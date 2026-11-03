PROOF   = ih-verify-parts
# image_helpers.c is included by the harness (see the region symbols there)
SOURCES = $(ROOT)/src/lib/bpak.c \
          $(ROOT)/tests/cbmc/stubs/crypto.c
UNWIND  = 33
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/image_helpers.c
