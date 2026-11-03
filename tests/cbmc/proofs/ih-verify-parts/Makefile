PROOF   = ih-verify-parts
# image_helpers.c is included by the harness (see the region symbols there)
SOURCES = $(ROOT)/src/lib/bpak.c \
          $(ROOT)/tests/cbmc/stubs/crypto.c
# One part and one metadata entry: every loop stops by its second pass (the
# unwinding assertions check it). With 33 the 32 unrolled passes made the
# solver take more than 30 minutes at 64 bit.
UNWIND  = 3
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/image_helpers.c
