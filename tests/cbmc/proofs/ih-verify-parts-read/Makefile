PROOF   = ih-verify-parts-read
# image_helpers.c is included by the harness (part_load_addr is static)
SOURCES = $(ROOT)/src/lib/bpak.c \
          $(ROOT)/tests/cbmc/stubs/crypto.c
UNWIND  = 33
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/image_helpers.c
