PROOF   = uuid-roundtrip
SOURCES = $(ROOT)/src/lib/uuid/unpack.c $(ROOT)/src/lib/libc/snprintf.c \
          $(ROOT)/src/lib/libc/strlen.c $(ROOT)/src/lib/libc/string.c $(ROOT)/src/lib/libc/memcmp.c $(ROOT)/src/lib/libc/memset.c
# The unparse format string is 47 characters
UNWIND  = 50
CBMC_EXTRA = --unwindset unsigned_dec_print.0:11,unsigned_dec_print.1:11,unsigned_num_print.0:9,unsigned_num_print.1:9,unsigned_num_print.2:9
# The vendored uuid code and the firmware libc, with the log stub only
MODELS  = $(ROOT)/tests/cbmc/stubs/log.c
EXTRA_DEFS = -fno-builtin
EXTRA_INC = -I $(ROOT)/src/lib -I $(ROOT)/src/arch/$(if $(filter 64,$(WORD)),armv8a,armv7a)/include
# The uuid code truncates on purpose ((unsigned char)tmp when packing) and
# the libc relies on size_t wrapping (while (len--)). The functional
# properties in the harness pin the results down instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check --conversion-check
# 500-2050 s: the digit count of each conversion is symbolic, so every later
# write is at a symbolic offset
TIMEOUT ?= 3600
include ../../Makefile.common

# unparse.c is included by the harness (see there)
$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/lib/uuid/unparse.c
