PROOF   = host-part-rw
HOST    = 1
P       = $(ROOT)/tools/punchboot
SOURCES = $(ROOT)/src/wire.c $(P)/error.c $(P)/api_partition.c $(P)/api_device.c \
          $(P)/api_stream.c
# Up to 2 partitions and 3 chunks plus the end of the file (the device and
# file bounds in the harness); memcmp of a 16 byte UUID, strncpy of the
# 36 character description
UNWIND  = 5
CBMC_EXTRA = --unwindset memcmp.0:17,strncpy.0:37,strncpy.1:37 --object-bits 10
# 400-420 s alone, over 600 s next to another proof
TIMEOUT ?= 1200
include ../../Makefile.common
