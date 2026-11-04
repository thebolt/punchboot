# Formal verification (CBMC)

Bounded model checking of the central firmware code with [CBMC](https://github.com/diffblue/cbmc)
(D-022). A proof checks every input up to a bound, so a passing proof means: no failed property
for any input within the bound, under the assumptions and stubs listed for that proof. It is not
an unbounded proof, and it says nothing about stubbed code (see Trust boundary).

How to run and add proofs: `tests/cbmc/README.md`.

## Setup

- Harnesses live in `tests/cbmc/proofs/<name>/`, shared rules in `tests/cbmc/Makefile.common`.
- Every proof runs at both word sizes of the firmware targets: `WORD=32` (ILP32, i.MX6UL/RT) and
  `WORD=64` (LP64, i.MX8). goto-cc builds with `-m32`/`-m64` against the firmware libc headers
  (`include/libc`, `include/libc/aarch{32,64}`), not the host libc, so the type widths are those
  of the targets. `tests/cbmc/common.h` asserts the pointer and `size_t` width.
- `config.h` is generated from `configs/host_defconfig` like the host build.
- `LOGLEVEL=0`: the log macros compile away.
- The string functions (`memcpy`, `memmove`, `memset`, `memcmp`) come from
  `tests/cbmc/stubs/string_models.c`. They assert the C preconditions (valid for n bytes, no
  overlap for memcpy). CBMC's own models are not used: CBMC preprocesses them with the host libc
  headers, which do not exist for 32-bit builds. The real `src/lib/libc` is verified by its own
  proofs (planned, Phase 1).
- Default checks: bounds, pointer, pointer-primitive, signed and unsigned overflow, conversion,
  undefined shift, pointer overflow, division by zero, and unwinding assertions (a too-low loop
  bound fails the proof). A proof may turn a check off with `CHECK_FLAGS_OFF` and a comment why.
- Vacuity: every harness has `PB_COVER()` goals for the paths that must be reachable. `make cover`
  fails if one is unreachable or if there are none. In the cover build the harness assertions are
  compiled out, because CBMC turns assertions into assumptions there and a known failure would
  cut every path after it.
- Globals start as zero in CBMC like in C. Harnesses give stub state and module state any value
  explicitly where that matters (`pb_ocotp_havoc()`).
- The log macros print through `tests/cbmc/stubs/log.c` (no output, arguments still evaluated).
- `char` is unsigned like on ARM (`-funsigned-char`, asserted in `common.h`).
- Hangs are failures: `--no-self-loops-to-assumptions` keeps `while (1) {}` a loop, so its
  unwinding assertion fails, instead of CBMC turning it into `assume(false)`.
- Harnesses compile with `-Werror`: an implicit declaration makes CBMC assume an `int` return.
- A proof can replace a function that has its own proof by one that returns any value
  (`ABSTRACT`, goto-instrument), give every function without a body a failing body
  (`UNDEFINED = assert-false-assume-false`, used by `cm-gate`: any handler call is a failure),
  or override a Kconfig value with a `config.h` wrapper (`PRE_INC`, used by `ramlog`).
- What makes CBMC 6.6 too slow or run out of memory here, and the way around it:
  - Symbolic division (by a block size, by 10 in `snprintf`): constant block sizes (`bio`,
    `bio-4k`, `stubs/cm_env.c`), the exact decimal digits only for values below 100000.
  - Symbolic sized writes into large arrays (the command buffers): the buffers get any contents
    up front, and the read stubs only check the buffer.
  - An assumption on a value is not a constant for symbolic execution: the command (`cm-*`) and
    the GPT entry count are assigned, so the other handlers and paths drop out.
  - Large structs (the 16 KiB GPT tables) are bit vectors: only the bytes the code reads are
    symbolic, counts are assigned (a constant) not assumed, and the paths through
    `gpt_ptbl_init` are separate proofs.
  - `--arrays-uf-always` crashes CBMC 6.6 (an invariant violation); kissat as external solver
    was not faster.
- Mutants: `tests/cbmc/mutants/*.patch` are bugs that a proof must find (only a counterexample
  counts, not a build error). `scripts/pbdev cbmc --mutants` applies each to a scratch copy.
- Not in CI: the proofs take hours, so they are run by hand (`scripts/pbdev cbmc`, then
  `--mutants`) after a change to proven code. No CI job or gate for now.
- CBMC 6.6.0 (Debian trixie package in the dev container).

## Status

| Proof | Target | Properties | Bounds | Known failures | Time (32/64) |
|---|---|---|---|---|---|
| `crc` | `src/lib/crc.c` | Memory safety; one step equals the bitwise CRC-32 (so all lengths, by induction); incremental `crc32(a‖b)` | len ≤ 16, incremental ≤ 2 | | 22 s / 22 s |
| `wire` | `src/wire.c` | `valid_command` is exactly magic + range; only IDENTIFIER_READ and AUTHENTICATE need no auth; `init_*2` size checks, no write outside the struct | none (fully symbolic structs) | | 3 s / 5 s |
| `bpak-header` | `bpak_valid_header` (pre-auth) | No UB for any 4 KiB header; OK implies magic, hash kind, alignment without wrap (F-149) and metadata bounds for the slots before the first empty one | none | | 55 s / 55 s |
| `bpak-signature` | `bpak_copyz_signature` (pre-auth) | No access outside the header or the output buffer; OK only for 0 < sz ≤ min(512, buffer); signature copied, area and size zeroed; header unchanged on error | buffer ≤ 1 KiB | | 5 s / 5 s |
| `bpak-getters` | `bpak_get_meta`, `_anyref`, `bpak_get_part` | Returned entry is a header slot before the first empty one with the requested id/ref, metadata in bounds (F-148) | valid header | | 83 s / 83 s |
| `bpak-part` | `bpak_part_size`, `bpak_part_offset` | No wrap; the size exact when it fits `size_t`; offset = 4096 + the 64 bit sum of the earlier part sizes (F-148, F-149) | valid header; offset for the first 3 slots | | 42 s / 223 s |
| `der-ecsig` | `der_ecsig_to_rs` (pre-auth, CAAM) | No access outside sig, r, s for any input; OK implies a SEQUENCE | sig ≤ 24 bytes, curve length ≤ 8 (see harness) | | 28 s / 38 s |
| `der-pubkey` | `der_ec_public_key_data` (trusted keystore) | Memory safety within the given key length (F-066) | key ≤ 160 bytes | | 3 s / 5 s |
| `slc` | `src/slc.c` | Without a config or callback: NOT_SUPPORTED, no NULL call | none | | 1 s / 1 s |
| `rot` | `src/rot.c` | `rot_init` accepts exactly a matching key map; status/revoke act on the requested key; `rot_get_dsa_key` only returns a key the board reports OK, with its data and kind; no error codes as counts or ids (F-153) | 3 keys | | 1 s / 1 s |
| `rot-imx6ul` | i.MX6UL `rot_helpers.c`, `slc_helpers.c` | Revoked key reads as revoked; revoke burns only the key's bit; LOCKED exactly when HAB reports secure boot; lock fuse burnt only from an unlocked state | param1 < 4 | | 1 s / 1 s |
| `rot-imx6ul-seq` | i.MX6UL `rot_helpers.c`, `slc_helpers.c` | Over any sequence of revokes, key status reads and locks on the same fuses: a revoked key never reads OK again, the lock fuse stays burnt | 3 steps, 4 SRK slots, any fuses at the start | | 1 s / 1 s |
| `rot-imx8x` | i.MX8X `rot_helpers.c`, `slc_helpers.c` | REVOKED exactly when the key's SECO bit is set; LOCKED exactly when SECO reports OEM closed; fail closed when SECO can't be asked; forward only with the SRK fused | param1 < 4 | | 1 s / 1 s |
| `slc-imxrt` | i.MX RT `slc_helpers.c`, `rot_helpers.c` | LOCKED exactly when SEC_CONFIG is set; NOT_CONFIGURED means no SRK bits; no revocation support | none | | 1 s / 1 s |
| `libc-*` (18 proofs) | Every function in `src/lib/libc/` string.h: `memcpy` (`string.c` and the unused `memcpy.c`), `memmove`, `memset`, `memcmp`, `memchr`, `strlen`, `strnlen`, `strcmp`, `strncmp` (`strcmp.c` and the unused `strncmp.c`), `strchr`, `strrchr`, `strcpy`, `strncpy`, `strlcpy`, `strspn`, `strcspn` | The definition in C11 7.24 (POSIX for `strnlen`, `strlcpy`), clause cited in each harness: results, what is written and what is left alone, no access outside what the standard lets the function touch | buffers ≤ 6 bytes | | ≤ 3 s each, `memmove` 45 s |
| `libc-strtoul`, `libc-strtoul-overflow` | `strtoul` | C11 7.22.1.4 written out as a reference parser (white-space, sign, 0x, base 0, subject sequence, negation, `ULONG_MAX`), value and `endptr` compared for any string; exact value or `ULONG_MAX` for 7-digit numbers | strings ≤ 6 bytes; bases 0, 2, 8, 10, 16, 36 | | 50 s; 180 s |
| `libc-snprintf-{s,u,d,x,08x}` | `snprintf` | C11 7.21.6.5: at most n - 1 characters and a NUL, returns the full length, the text the conversion defines | n ≤ 12 | | see CI |
| `libc-snprintf-unsupported` | `snprintf` | Returns for `%c`, `%X`, `%zu`, `%2u` | n ≤ 8 | | 1 s |
| `uuid-parse` | `uuid_parse` | Accepts exactly the canonical 8-4-4-4-12 hex form; the bytes are the hex digits in order | strings ≤ 38 bytes | | 80 s |
| `uuid-roundtrip` | `uuid_unparse`, `uuid_parse` (with the firmware `snprintf`, `strtoul`) | Any UUID unparses to a 36 character lower case canonical string and parses back to itself | none | | 500-1800 s (timeout 2400 s) |
| `uuid-pack` | uuid pack/unpack, compare, copy, clear, is_null | unpack∘pack is the identity; compare orders like the bytes; copy, clear, is_null | none | | 1 s |
| `bio`, `bio-4k` | `src/bio.c` | Read, write and erase reach the HAL only inside the device with the absolute LBA (F-055); sizes and getters exact (F-157); invalid handles rejected; lookup by UUID | one device, any LBAs, 512 / 4096 byte blocks | | 70 s / 110 s |
| `bio-child` | `bio_allocate_parent` | A partition lies inside its disk and has its flags | a fixed disk | | see CI |
| `gpt`, `gpt-recover-primary`, `gpt-recover-backup` | `gpt_ptbl_init` (pre-auth, on disk contents) | No access outside the tables for any contents, both recovery paths; at most 128 entries (F-056); partitions inside the disk and not overlapping (F-006) | 3 entries in use; `gpt_write_tbl` and table copies abstracted | | 1-2 s |
| `gpt-write`, `gpt-write-128` | `gpt_write_tbl` | Every write inside the disk, CRCs inside the tables (F-162) | 3 (recovered) / 128 (installed) entries | | 1 s |
| `ab-state` | `src/boot/ab_state.c` | A valid state after init for any disk state; boots A, B or nothing; an unverified system only with an attempt left (counter goes down by one) or by the speculative rollback; board registers inside the array | any state, any CRC | | 2 s |
| `ramlog` | `src/ramlog.c` | Nothing before init; putc at head % size; read copies exactly what is in the ring after offset | 1 KiB log, reads up to 64 bytes | | 6 s / 10 s |
| `tzc380` | `src/drivers/tzc/tzc380.c` | A region is programmed only with a valid number, power-of-two size >= 32 KiB inside the address width, aligned base, SP bits only; exactly base and attribute registers | any config register | | 1 s |
| `delay` | `src/delay.c` | Init refuses exactly 0 and 0xffffffff; a timeout expires exactly when its time has passed, for any start tick, also when the tick wrapped between polls (F-172); `pb_delay_us` returns after the delay | one poll < 2^32 us after the init; delay ≤ 2 tick steps | | 1 s / 1 s |
| `timestamp` | `src/timestamp.c` | The table is never overrun; `ts_total` is the first to last difference, 0 without timestamps (F-173) | table of 2, ≤ 3 calls | | 0 s / 0 s |
| `device-uuid` | `src/device_uuid.c`, `uuid3.c` | A version 3 RFC 4122 UUID, the same on every call after the first success; the platform's error returned, nothing cached after a failure | two calls; the platform keeps to its length contract | | 1 s / 1 s |
| `console` | `src/console.c` | The early RAM log replay reads only inside the log and writes out exactly the bytes still in it that were not muted, in order, each once; after `console_init` the UART gets output exactly when not muted | ≤ 5 steps of ≤ 3 bytes or a (un)mute, a RAM log of ≤ 8 bytes at any position | | 97 s / 108 s |
| `main` | `src/main.c`, `src/self_test.c` | Nothing boots unless plat and board init succeeded; self tests between them and the boot; command mode then a reset after a returning boot; every self test once, in order, after a watchdog kick | a firmware build with self tests (jiffy), ≤ 2 tests | | 1 s / 1 s |
| `usbd` | `src/drivers/usb/usbd.c`, `pb_dev_cls.c` string descriptors | For any setup packets (pre-auth), during enumeration and mid-transfer: every EP0 answer inside its descriptor and at most `wLength` (F-175); SET_CONFIGURATION gets the interface's endpoints; an interrupted transfer restarts as it was; the serial number from a written UUID (F-176); connected only once configured | the descriptor layout and lengths of `pb_dev_cls.c`, a HAL with every op (all three UDC drivers) | | 17 s / 17 s |
| `ih-auth` | `boot_image_auth_header` (pre-auth) | OK only when dsa_verify verified; the header's signature, digest length of its hash kind; the whole header hashed once with the signature zeroed; the key only if not revoked | any header | | 7 s |
| `ih-verify-parts` | `boot_image_verify_parts` | OK means every region (stack, data, bss, code, no_init, any layout) was checked with the part's whole range, which does not wrap, and where the code found a region clear it is (F-065) | one part, its load address in the first metadata entry; `CHECK_OVERLAP` records its range and checks each call | | 10 s / 25 s |
| `ih-verify-parts-read` | `part_load_addr` | For any metadata layout: OK means the pb-load-addr entry is a u64, the address is its value, and the part fits above it (F-065) | one part, 4 metadata entries | | 240 s / 260 s |
| `ih-load` | `boot_image_load_and_hash` | Hashes exactly what was read, chunk by chunk; OK means every chunk read and hashed | 2 parts of up to 3 chunks | | 270 s / 640 s |
| `ih-copy` | `boot_image_copy_and_hash` | Copies inside source and destination (F-088); hashes the copy | one part up to 512 bytes, buffers up to 256 | | 5 s |
| `ih-payload` | `boot_image_verify_payload` | OK only for a known hash kind and a matching digest of its length | any header | | 5 s |
| `boot-linux` | `boot_driver_linux_prepare` | The kernel, ramdisk and dtb addresses are their u64 load address entries (F-168); libfdt only on the dtb part after the header check, with the tree inside the part; the patched tree is cleaned; initrd cells not truncated | 3 parts and entries at fixed offsets (`stubs/boot_driver.h`), any ids, sizes and config; libfdt as a contract | | 480-600 s / 550-1000 s (timeout 1800 s) |
| `boot-atf` | `boot_driver_atf_prepare` (aarch64 only) | The ATF, BL33, BL32, ramdisk and dtb addresses are their u64 load address entries and the image sizes their part sizes, under 4 GiB (F-168); OP-TEE arch read only if it holds a u32; libfdt as in `boot-linux` | 4 parts, an OP-TEE arch entry; 64 bit only (`words`) | | 1250-1500 s (timeout 2400 s) |
| `boot` | `src/boot/boot.c` | prepare, late_boot and jump only for the header authenticated, checked, loaded and verified in this boot_load | two boots, any sources; image helpers abstracted (`ih-*`) | | 9 s |
| `cm-gate` | `pb_command_parse` | A command that needs authentication does nothing unauthenticated unless the SLC is a valid unlocked state (fails closed, D-023) | any command bytes and slc; every external call fails | | 1 s |
| `cm-*` (14 proofs, one per command) | the command handlers in `cm_main.c` | Authenticated, any command bytes, host data and results of the called modules: no undefined behaviour, every transport read and write inside a valid buffer | the called modules as contracts (`stubs/cm_env.c`: bio with 512 byte blocks, up to 17 keys, ramlog), everything else any value | | 4-50 s |
| `crypto` | `src/crypto.c` | hash calls only between hash_init and hash_final, to the provider that has the algorithm, with the caller's arguments; dsa_verify's result and verified are the provider's, NOT_SUPPORTED without one; tables never overflow | providers with any algorithm bits | | 1 s |
| `host-api` | the host API calls with one exchange (`tools/punchboot/api_*.c`) | For any bytes a malicious device sends and any transport error: no undefined behaviour, only the caller's buffers; the version, boot and board status end inside the caller's buffer, board command and log read sizes fit it (F-169) | output buffers ≤ 512 bytes | | 158 s / 177 s |
| `host-part-table` | `pb_api_partition_read_table` | At most the caller's entries, every description terminated, a failed malloc refused (F-169) | ≤ 3 entries | | 43 s / 42 s |
| `host-part-rw` | `pb_api_partition_write`, `pb_api_partition_read` | No division by the device's buffer count, both terminate, a successful write sent the whole file (F-169) | chunk 0 or ≤ 8 KiB, ≤ 2 partitions of ≤ 3 chunks, file ≤ partition + 1 | | 344 s / 354 s |
| `host-boot-bpak` | `pb_api_boot_bpak_image` | For any file of any size: nothing read outside it, the transfer terminates (F-169) | header + 8 bytes, ≤ 2 parts, 1 metadata entry | | 23 s / 30 s |

Checks turned off: `crc` has no unsigned overflow check (`while (size--)` wraps on purpose). The
libc and uuid proofs have no unsigned overflow and conversion checks: the code relies on size_t
wrapping and on the conversions the C standard asks for (`(unsigned char)c`), and the functional
properties pin the results down instead.

Known failures are listed per proof in `expected_failures.txt`. A proof fails on any other
failure, and also when a known one stops failing (then the list is updated and the proof becomes
the fix's regression test). No proof has one now: the last ones, implementation-defined
conversions in `pb_wire_init_result` and `efi_crc32`, are fixed (F-167).

## Trust boundary

What the proofs assume rather than prove. Each stub states its contract in its file.

| Not verified | Why | Assumed |
|---|---|---|
| String functions in proofs other than libc | Replaced by models | C standard semantics |
| Mbed TLS, CAAM and other crypto drivers | Out of scope | `stubs/crypto.c`: any result, a digest of the asked length; the dispatch is the `crypto` proof |
| littlefs, libfdt | Vendored, large | littlefs: not used before an image is verified; libfdt: only on the signed device tree, inside its part (`boot-linux`, `boot-atf`) |
| USB, MMC, UART hardware, SoC code | Hardware | The transport reads into and writes from the buffers it is given (`cm-handlers`) |
| Host transports (`usb.c`, `socket.c`), the Python wrapper | libusb, CPython | A read fills the whole buffer or fails (`stubs/host_device.h`); `usb.c` does not check the length yet (F-170) |

## Plan

From the approved plan (2026-11-02), ordered by attacker reachability:

- **Phase 0 (done):** tooling, CI job, vacuity and mutation checks, `crc` and `wire`.
- **Phase 1:** done: `bpak`, `der`, `slc`/`rot` and the platform helpers (found F-148..F-153).
  Then every function in `src/lib/libc` against its C standard definition, and `uuid` (found
  F-154..F-156, confirmed F-089).
- **Phase 2:** done: `bio`, `gpt`, `image_helpers`, `ab_state`, `ramlog`, `tzc380` (found
  F-157, F-158, F-162, F-163; confirmed F-055, F-065, F-006). All of them are fixed (D-024,
  D-025); F-065 has hardening left that is not a parser bug.
- **Phase 3:** done: `boot.c`, the auth gate, one proof per command handler, crypto dispatch
  (confirmed F-070, F-074; found F-159, F-161, F-164..F-166). The boot drivers `linux.c` and
  `atf.c` (found F-168); libfdt is in the trust boundary (signed input only).
- **The rest of `src/*.c`:** done: `delay.c` (found F-172), `timestamp.c` (F-173),
  `device_uuid.c`, `console.c`, `main.c` and `self_test.c` (`main`).
- **Drivers:** the USB device core and class (`usbd`, found F-175, F-176), which handle setup
  packets from any host. The UDC, MMC, UART, fuse and crypto drivers program hardware and are not
  proven. Postponed: the i.MX6UL ROM log decoder (`rom_log.c`, jiffy's `dump-rom-log`): with the
  firmware `snprintf` even one event line is too big for CBMC 6.6 (over 16k objects, then
  timeouts); by reading, every line is under 72 of the 128 bytes it is given.
- **Phase 4:** host-side parsing in `tools/punchboot` done (`host-*`, found F-169; host mode in
  `Makefile.common`). Sequence properties: `rot-imx6ul-seq` runs revoke, status and lock in any
  order on the same fuses. The others follow from the per-call proofs by induction, because each
  starts from any state: the fuse stub only adds bits (revocation and locking are monotonic on
  every platform), `ab-state` starts from any state on disk and never wraps the counter, and only
  `cmd_auth` writes `authenticated` (`cm-auth`; the gate is `cm-gate`). `authenticated` is kept
  over a transport restart (F-174, open). Left: harnesses reused as fuzz targets.
- Not done: cbmc-viewer reports. No CI job or gate: the proofs are run by hand for now.

Findings from proofs go in `findings.md`, each fix in its own `bugfix:` commit with the proof as
its regression test. Fixes that change accepted images or wire behaviour need a decision first
(D-008).
