PROOF   = boot-linux
# linux.c is included by the harness (to reach the addresses it keeps)
SOURCES = $(ROOT)/src/lib/bpak.c
EXTRA_INC = -I $(ROOT)/include/fdt \
            -I $(ROOT)/src/arch/$(if $(filter 64,$(WORD)),armv8a,armv7a)/include
# 3 parts and 3 metadata entries in use, 3 device tree nodes: every loop
# stops by its fourth pass (the unwinding assertions check it)
UNWIND  = 5
# strlen of the 36 character device UUID, strcmp with "chosen"
CBMC_EXTRA = --unwindset strlen.0:38,strcmp.0:8
# 500-700 s alone, about 4 GB
TIMEOUT ?= 1800
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/linux.c $(ROOT)/tests/cbmc/stubs/boot_driver.h
