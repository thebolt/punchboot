# Decisions

Short records of decisions and why they were made. Add new ones at the bottom,
and never edit old ones; supersede them with a new entry instead.

## D-001: Keep the bpak file format unchanged for now (2026-10-22)

The modernization works around the bpak format. Refactoring how punchboot
*uses* bpak is fine, as long as the on-disk and on-wire `struct bpak_header`
stays compatible.

## D-002: AI working context lives in `doc/dev/` (2026-10-22)

`CLAUDE.md` stays small and points to files that are loaded on demand.
`doc/source/` remains the user-facing Sphinx documentation (removed, D-020). `doc/dev/` is
developer and AI context: status, roadmap, findings, decisions, architecture
notes and a dated log.

## D-003: Release flow (2026-10-22)

Releases are made from master. Bump `version.txt` in a PR, then create a GitHub
release with tag `vX.Y.Z` and generated notes. The tag triggers
`punchboot-tools.yml`, which uploads to PyPI.

## D-004: Host-native test setup replaces the QEMU suite (2026-10-22)

Tests run on the host: CMocka unit tests, plus pytest integration tests against punchboot built
as a native process (`plat/host`, `board/host`). They are built with CMake/CTest, while the
firmware for targets stays on Make + Kconfig. The 43 bash/QEMU tests are ported one by one, and
the QEMU suite is removed when everything is covered. A small QEMU smoke tier for
arch-specific code may come back later.
Why: the QEMU suite was slow (~15 min), serial, and broke with toolchain upgrades (the gcov hack).
It also couldn't use sanitizers or a debugger.

## D-005: Tooling for the host tests (2026-10-22)

CMake + CTest, CMocka for C unit tests, and pytest driving the real `punchboot` Python package for
integration tests (this also tests the host library and the Python bindings).

## D-006: CI runs the host tests in the dev container; QEMU suite removed (2026-10-22)

`build.yml` runs everything through `scripts/pbdev`, the same as locally: the host tests (default
and sanitizers), coverage (gcovr, D-012), a compile check of the firmware for the boards that
build (jiffy, imx8dxlevk), and Coverity on master. The bash/QEMU suite, `src/board/test`,
`src/plat/qemu` and `configs/test_defconfig` were removed once all tests were ported. The virtio
drivers are kept for now; they would be needed for a QEMU smoke tier.

## D-007: Unit tests fake the neighbouring modules; known bugs are not encoded as passing (2026-10-22)

A unit test links one module (sometimes two) and fakes what it calls: the image helpers for
`boot.c`, the boot flow, root of trust and SLC for `cm_main.c`, the crypto providers through the
crypto API, and a scripted transport for command mode. The ramdisk fake injects I/O errors and the
platform fake has a controllable clock. A path that hits a known open bug is not tested; a comment
names the finding. In pytest, a known bug gets a `xfail(strict=True)` test that names the finding,
so the fix shows up as an XPASS.
Why: the integration tests can't reach most error paths, and a test that passes on buggy behaviour
would have to be changed by the fix.

## D-008: Never break backward compatibility of the wire protocol or the bpak format (2026-10-22)

Supersedes D-001. This is permanent, not "for now". Released hosts must keep working with new
devices, new hosts with released devices, and every image that a released bpak CLI produced and a
released punchboot accepted must still be accepted. Extensions are allowed when they are additive
and opt-in (append-only enums, fields taken from reserved bytes where zero keeps the old
behaviour, new commands that old devices reject cleanly, capabilities). Rejecting images that were
always invalid is the only exception, and needs its own decision entry. `compatibility.md` lists
what is frozen, how to extend, and the guards.
Why: devices in the field are not updated in step with the tools, factory lines pin tool versions,
and signed images outlive both.

## D-009: The boot source is a kind plus a bio partition; only the platform decides it (2026-10-22)

`plat_get_boot_source()` reports where pb was loaded from: `STORAGE` with the bio partition UUID of
the boot slot, or `USB`, `UART`, `DEBUGGER`, `UNKNOWN`. Every boot slot is a bio partition,
whatever the medium (eMMC boot partitions, NOR flash regions on the imxrt, the A/B offsets in the
MCX N internal flash), so any number of slots (A/B, primary/backups/factory recovery) works without
changing the API; the partition name says what the slot is. Drivers only report facts about their
device (`mmc_get_boot_part_config()`: what BOOT_PARTITION_ENABLE was at init). The platform turns
them into a boot source, because only it knows its ROM (fallbacks, serial download). Platforms that
don't know return `-PB_ERR_NOT_SUPPORTED`, and `UNKNOWN` when they can't tell this boot. New kinds
are appended. Not on the wire yet; exposing it to the host is a wire addition (D-008).
Why: an eMMC-only "booted from BOOT0/BOOT1" call was wrong for USB boots, for other ROMs and for
other media.

## D-011: Command mode is open without authentication in SLC EOL (2026-10-23)

Authentication is only enforced in `CONFIGURATION_LOCKED`. In `EOL` every command is open, as in
`NOT_CONFIGURED` and `CONFIGURATION`. Closes F-002 as intended.
Why: Jonas confirmed this is the intended EOL behaviour (2026-10-23). Note that it also opens
`cmd_board` (F-063) and everything else in EOL, so bounds checks on host input matter there too.

## D-012: No Codecov; coverage is a gcovr report in CI (2026-10-23)

The coverage job prints the gcovr summary and uploads the HTML report as a workflow artifact.
Nothing is uploaded to Codecov, and coverage is not a gate.
Why: the number only covers what the host build compiles, so most driver and platform code is not
in it. Codecov's default targets failed PR #65 because `mmc_core.c` and `rom_log.c` entered the
report for the first time (-7.5% project), the upload was non-fatal and the statuses not required,
and it needed `id-token: write`. What matters is that each fix has a test that fails without it.

## D-013: Hardware init without hardware is refactored behind a golden register trace (2026-10-28)

Code that can only run on a board we don't have (first: the i.MX8M LPDDR4 init) gets a host unit
test that runs it against a device model behind a fake `pb/mmio.h` and compares every register
write to a checked-in trace. Cosmetic commits keep the trace identical. A commit that means to
change what reaches the hardware regenerates the trace and says why in its message.
Why: there is no imx8mevk in the lab, and a DRAM init that is wrong in one bit fails silently or
not at all until it is on a board. The trace proves that a cleanup changed nothing; it does not
prove the sequence is right.

## D-014: DDR IP code lives in `src/drivers/ddr`, selected by the platforms (2026-10-28)

The Synopsys uMCTL2 controller and the firmware-trained DWC DDR PHY are drivers
(`DRIVERS_DDR_UMCTL2`, `DRIVERS_DDR_DWC_PHY`, hidden Kconfig symbols). Platforms select what their
SoC has; the i.MX8M SoCs do it through `PLAT_IMX8M_COMMON`, which `PLAT_IMX8MQ` (and later
`PLAT_IMX8MP`) select. The drivers take the base address (and the PHY its CSR stride and firmware)
from the caller and know nothing about the SoC. The init sequence, resets, clocks and the TF-A
handover stay in the platform, with one `ddr_<soc>.c` per SoC behind `ddr_soc.h`.
Why: the i.MX8M Plus has the same controller and PHY as the 8MQ, and the controller is also in the
ZynqMP and Allwinner H6/H616. The Allwinner H5 has neither (Synopsys names at Allwinner offsets,
a PUB-style PHY), so its DRAM code stays in its platform.

## D-017: MMC/SD drivers are tested against standard-based card models (2026-10-28)

`mmc_core` and the host controller drivers are tested on the host against models of an e.MMC
(JESD84-B51) and an SD card behind a bus interface, and the uSDHC against a register model from
its RM (`tests/unit/models/`, `testing.md`). The models behave like the device and log every
host action the standard or RM does not allow, with the section; tests fail on any. A bug the
bench finds gets a finding, and its test is skipped (`SKIP_OPEN`) until the fix enables it.
Why: the drivers are shared by five boards on four SoCs and only two boards are in the lab. The
first run found bugs on every board that hardware testing hadn't shown (R1b without busy wait,
commands above the clock limit in DDR52, ignored status errors). The models are a reading of the
documents, not the documents: each rule cites its section so it can be checked, and electrical
timing (sampling points, DLL taps) still needs a board.

## D-018: The next release is 3.0.0, for the porting API, not the wire (2026-10-29)

The release after v2.7.1 (2.8.0 was never tagged) is 3.0.0, built up on branch `work-3.0.0`.
The major bump is for the board and driver porting API, which breaks out-of-tree boards: the
request-based MMC HAL with one `mmc_device` per card, the `imx_usdhc` setup, `PLAT_IMX8M_COMMON`
and the DDR drivers in `src/drivers/ddr` (D-014), the DWC3 and ChipIdea UDC rework, Kconfig and
defconfig changes. The wire protocol and bpak stay compatible (D-008): 3.0 hosts work with 2.x
devices and the other way round, and every image 2.x accepted is still accepted. The release
notes say both first.
Why: Jonas wants a major release for the size of the change (new SoC and EVK, MMC/SD, DWC3,
CAAM, RAM log, many fixes). A major version normally reads as "incompatible", so without the
note users could think 2.x and 3.x tools and devices must not be mixed, and `punchboot~=2.7` or
`<3` pins on PyPI won't pick up the fixes.

## D-019: No boot-time self tests; on-target checks go through command mode (2026-10-29)

`CONFIG_SELF_TEST`, `DECLARE_SELF_TEST`, `src/self_test.c` and the four SHA-256 known-answer
tests in `crypto.c` are removed in 3.0.0, with `hash_print()`, which only they used.
Why: the tests had run nothing since `--gc-sections` (F-129) and nobody noticed, every defconfig
had them off, and the host unit tests cover the same vectors in CI. Boot is fail-closed anyway: a
wrong hash fails the signature check and pb doesn't boot, so the tests added a message, not
safety, at boot time cost on every boot. They also didn't test ECDSA, the dangerous failure.
Checking the crypto engines on silicon (CAAM, DCP) is done from the host against a board, the way
CAAM was checked on jiffy and the 8MP EVK, and belongs in a future hardware-in-the-loop tier.

## D-020: Slimming is limited to dead files and subsystems while new boards come in (2026-10-29, temporary)

Until the boards that are on their way are in, code is not removed just because no board in
the tree uses it: drivers, library and porting API, helpers, macros, register/pin/clock headers
and board hooks stay, because new boards and examples will need them. Jonas brought back
imx_gpio, imx_usbdcd, `hash_copy_update`, `bio_get_part_by_uu_str`, `usbd_xfer_cancel`, the
`mmc_*` read/write/info/extcsd calls, `boot_image_copy_and_hash`, `utils_def.h` and the
`imx_cdns3_udc.h` include in imx8dxlevk the same day. Ask Jonas before removing anything of
that kind, and hold the porting API simplifications in F-145 until the new boards show what
they need.
What still goes: whole subsystems nobody enables, docs nobody builds, files nothing references,
config nothing reads, dead scripts. The first round (3.0.0) removed littlefs, the Sphinx and
Read the Docs tree (`doc/source`, `.readthedocs.yml`; user docs start from the README, F-044),
the unreferenced presentation and image in `doc/`, unused TF-A asm helpers and asm macros,
headers nothing includes, always-on and unread Kconfig options, Makefile leftovers and dead
scripts. Boards that don't build (F-033, F-051), the ATF boot driver (F-146) and virtio
(F-052) wait for Jonas.
Why: Jonas wants the code slimmed down; dead code is read, grepped, reviewed and kept
compiling, and a stub doc tree looks like documentation (`--gc-sections` already kept most of
it out of the binaries). But more boards are coming, and Jonas expects to add things to the
boards or add examples that use what looks unused today.
Revisit: when the incoming boards are in, decide whether to go back to removing code without
an in-tree user.

## D-021: The host tools are pure Python (2026-10-29)

The `punchboot` package no longer has a C extension: the wire protocol, the transports and the
command sequences are Python (`tools/punchboot/`), USB goes through pyusb with libusb
from the libusb-package wheels (falling back to the system's libusb), and the C host library
(`tools/punchboot/*.c`, `api.h`) is gone. Nothing outside the extension used it. `_punchboot`
is gone too; `Session` and the exceptions keep their API.
Safety net (D-008): compatibility guard 4 first. The transcripts in `tests/compat/transcripts`
were recorded from the 2.x C library against pb-host, and the Python host must send the same
bytes, in the same transfer sizes, and return the same values. The structs in `wire.py` and
`bpak.py` are checked against `abi.txt`.
Why: one wheel for every platform and Python version instead of a cibuildwheel matrix with
vcpkg and patched Homebrew libusb bottles; Linux aarch64 for free; no memory-safety bugs in host
code (F-094) and no GIL held during USB I/O (F-009); ~550 lines of boilerplate (F-145) gone. A
32 MiB write to the 8MP EVK is as fast as with the C library (~28.5 MiB/s, within noise).
Cost: libusb now comes from someone else's wheels (libusb-package); a platform without them
needs a system libusb. There is no C host library for embedding anymore.
## D-022: Bounded model checking with CBMC for the central firmware code (2026-11-02)

Proofs live in `tests/cbmc`, run at both firmware word sizes (32 and 64 bit) against the firmware
libc headers, and are ordered by attacker reachability (`verification.md`). Results are stated as
"no failed property up to bound N under the listed assumptions", never as unbounded proofs.
Every harness has reachability goals, and every proof has a mutant it must find. The vendored
libc and uuid are verified too. The CI job is not blocking until the proofs have been stable.
Why: the unit and integration tests only check the inputs someone thought of. Most of the open
security findings are bounds or wrap bugs in code that parses untrusted input, which is what
bounded model checking finds. Note: D-022 and up may collide with other branches, renumber on
merge.

## D-023: Command mode authentication fails closed (2026-11-03)

Commands that need authentication are open without it only when the security life cycle is a
valid unlocked state: NOT_CONFIGURED, CONFIGURATION or EOL (D-011). A locked device, and a device
whose life cycle can't be read (`slc_read_status()` returns an error) or reads as an unknown
value, needs authentication.
Why: the gate only closed for CONFIGURATION_LOCKED, so a HAB or SECO error (F-152) opened every
command on a locked device (F-161, found by the CBMC `cm-gate` proof). A device whose SLC can't be
read can still be used with the auth token or password. The wire protocol is unchanged (D-008).


## D-024: Partition erase is checked; on-disk GPT entries that overlap are skipped (2026-11-03)

PART_ERASE erases inside the partition only (`start_lba` stays partition-relative on the wire, as
hosts send it), refuses a partition without BIO_FLAG_WRITABLE, and a block count of 0 erases the
whole partition. GPT entries that overlap an earlier accepted entry are skipped with an error
instead of becoming a second partition over the same blocks.
Why: on imxrt an erase from LBA 0 of any partition erased the start of NOR (F-055), pre-2.0 hosts
send a block count of 0 and got OK with nothing erased (F-075), and overlapping entries let one
partition write another's blocks (F-006). No valid host request or partition table changes
meaning, so this is not a D-008 break; the wire format is unchanged.

## D-025: Malformed signed image headers are rejected (2026-11-03)

`bpak_valid_header` rejects a part whose size + pad wraps, or whose offset doesn't fit an `off_t`.
The bpak getters stop at the first empty slot, as `bpak_valid_header` does. A part's pb-load-addr
must be a u64 and the part must fit in the address space above it, also where the Linux and
ATF drivers read it, and their device tree must fit its part (`fdt_totalsize`, F-168).
Why: these headers need the signing key, but the code that reads them read past the metadata
(F-148), mis-sized parts (F-149) and let CHECK_OVERLAP wrap, so a part could load over the
firmware (F-065). No tool produces such a header (bpak keeps entries packed, writes the load
address as a u64, and real parts are far below 2^63 bytes), so every image a release accepted is
still accepted (D-008; dtc writes `totalsize` as the size of the blob and bpak only pads after
it). A 32-bit device still accepts a bpak with parts over 4 GiB in command
mode; only booting one is refused.

## D-026: The host API does not trust the device's sizes (2026-11-04)

The host refuses a device that reports 0 stream buffers or a chunk size of 0, a board status
that leaves no room for a null character, and a version without one in the response; it
terminates the strings it returns itself. `pb_api_boot_bpak_image` refuses a bpak file that does
not hold its header and every part. Why: the host crashed, hung or read past its buffers on
such answers (F-169). `compatibility.md` asks the host to handle a zero in every caps field:
these two have been set (2 and `CM_BUF_SIZE`) by every firmware since they were added, and a
zero never worked (it divided by zero or hung), so refusing it with an error loses nothing.
The firmware sends board status text without a null character; the host now adds it, so a
status as large as the caller's buffer is refused (the wrapper's buffer is 1 KiB, the board
statuses are a line). `pb_api_boot_bpak` keeps its signature for C API users.
