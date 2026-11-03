PROOF   = bio-child
SOURCES = $(ROOT)/src/bio.c \
          $(ROOT)/src/lib/uuid/copy.c $(ROOT)/src/lib/uuid/compare.c \
          $(ROOT)/src/lib/uuid/unpack.c
UNWIND  = 4
# uuid_copy and uuid_compare loop over the 16 bytes, strncpy over the
# 36 character description
CBMC_EXTRA = --unwindset uuid_copy.0:17,memcmp.0:17,strncpy.0:38,strncpy.1:38
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: ../bio/harness.c
