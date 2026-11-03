PROOF   = ih-payload
SOURCES = $(ROOT)/src/boot/image_helpers.c $(ROOT)/src/lib/bpak.c \
          $(ROOT)/tests/cbmc/stubs/crypto.c
UNWIND  = 33
# memcmp of up to 64 digest bytes
CBMC_EXTRA = --unwindset memcmp.0:65
include ../../Makefile.common
