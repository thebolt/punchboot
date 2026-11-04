PROOF   = boot-atf
# atf.c is included by the harness (to reach the addresses it keeps). The
# driver only builds for aarch64 (BOOT_ATF depends on ARCH_ARMV8): 64 bit
# only (see the words file), with __aarch64__ as the compiler defines it.
SOURCES = $(ROOT)/src/lib/bpak.c
EXTRA_INC = -I $(ROOT)/include/fdt -I $(ROOT)/src/arch/armv8a/include
EXTRA_DEFS = -D__aarch64__=1
# 4 parts and 5 metadata entries in use, 3 device tree nodes: every loop
# stops by its sixth pass (the unwinding assertions check it)
UNWIND  = 7
# strlen of the 36 character device UUID, strcmp with "chosen"
CBMC_EXTRA = --unwindset strlen.0:38,strcmp.0:8
# 1250-1450 s alone, about 4 GB
TIMEOUT ?= 2400
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/atf.c $(ROOT)/tests/cbmc/stubs/boot_driver.h
