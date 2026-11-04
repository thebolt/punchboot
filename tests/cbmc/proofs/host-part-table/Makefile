PROOF   = host-part-table
HOST    = 1
P       = $(ROOT)/tools/punchboot
SOURCES = $(ROOT)/src/wire.c $(P)/error.c $(P)/api_partition.c $(P)/api_device.c \
          $(P)/api_stream.c $(ROOT)/src/lib/bpak.c
# Up to 3 entries; the description check loops 37 times, strncpy 36
UNWIND  = 4
CBMC_EXTRA = --unwindset terminated.0:38,strncpy.0:37,strncpy.1:37
include ../../Makefile.common
