PROOF   = boot
# boot.c is included by the harness (to reach the static header pointer)
SOURCES = $(ROOT)/src/lib/uuid/copy.c $(ROOT)/src/lib/uuid/clear.c \
          $(ROOT)/src/lib/uuid/isnull.c $(ROOT)/src/lib/uuid/compare.c \
          $(ROOT)/src/lib/uuid/unpack.c
UNWIND  = 3
CBMC_EXTRA = --unwindset uuid_copy.0:17,uuid_is_null.0:17,memcmp.0:7
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/boot/boot.c
