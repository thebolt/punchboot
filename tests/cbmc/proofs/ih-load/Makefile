PROOF   = ih-load
SOURCES = $(ROOT)/src/boot/image_helpers.c $(ROOT)/src/lib/bpak.c \
          $(ROOT)/tests/cbmc/stubs/crypto.c
UNWIND  = 33
# Up to 3 chunks per part (the chunk size is any value)
# The load addresses are raw memory, not CBMC objects: the hash stub does
# not check that it can read them
EXTRA_DEFS = -DPB_CBMC_MAX_HASHED=6 -DPB_CBMC_HASH_RAW_ADDRESSES
include ../../Makefile.common
