PROOF   = host-api
HOST    = 1
P       = $(ROOT)/tools/punchboot
SOURCES = $(ROOT)/src/wire.c $(P)/error.c $(P)/api_device.c $(P)/api_misc.c $(P)/api_slc.c \
          $(P)/api_authentication.c $(P)/api_boot.c $(P)/api_partition.c $(P)/api_stream.c \
          $(P)/api_board.c $(ROOT)/src/lib/bpak.c
UNWIND  = 2
# strnlen of the version in the 504 byte response, is_string over a 512 byte
# buffer
CBMC_EXTRA = --unwindset strnlen.0:505,is_string.0:513 --object-bits 10
include ../../Makefile.common
