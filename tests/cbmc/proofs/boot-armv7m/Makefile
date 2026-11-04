PROOF   = boot-armv7m
# armv7m_baremetal.c is included by the harness (to reach the address it keeps)
SOURCES = $(ROOT)/src/lib/bpak.c
EXTRA_INC = -I $(ROOT)/src/arch/armv7m/include
# 3 parts in use, then an empty slot: every loop stops by its fourth pass
UNWIND  = 5
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/armv7m_baremetal.c
