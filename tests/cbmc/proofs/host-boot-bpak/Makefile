PROOF   = host-boot-bpak
HOST    = 1
P       = $(ROOT)/tools/punchboot
SOURCES = $(ROOT)/src/wire.c $(P)/error.c $(P)/api_boot.c $(P)/api_device.c \
          $(ROOT)/src/lib/bpak.c
# At most 2 parts and 1 metadata entry, 8 payload bytes sent in chunks of
# at least 1 byte
UNWIND  = 10
CBMC_EXTRA = --object-bits 10
include ../../Makefile.common
