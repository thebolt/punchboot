PROOF   = uuid-parse
SOURCES = $(ROOT)/src/lib/uuid/parse.c $(ROOT)/src/lib/uuid/pack.c \
          $(ROOT)/src/lib/libc/strtoul.c $(ROOT)/src/lib/libc/strlen.c $(ROOT)/src/lib/libc/string.c $(ROOT)/src/lib/libc/memcmp.c $(ROOT)/src/lib/libc/memset.c
UNWIND  = 40
# The vendored uuid code and the firmware libc, with the log stub only
MODELS  = $(ROOT)/tests/cbmc/stubs/log.c
EXTRA_DEFS = -fno-builtin
EXTRA_INC = -I $(ROOT)/src/arch/$(if $(filter 64,$(WORD)),armv8a,armv7a)/include
# The uuid code truncates on purpose ((unsigned char)tmp when packing) and
# the libc relies on size_t wrapping (while (len--)). The functional
# properties in the harness pin the results down instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check --conversion-check
include ../../Makefile.common
