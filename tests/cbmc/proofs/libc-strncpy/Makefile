PROOF   = libc-strncpy
SOURCES = $(ROOT)/src/lib/libc/string.c
UNWIND  = 9
# The firmware libc itself: no string models, no builtins
MODELS  =
EXTRA_DEFS = -fno-builtin
# The libc relies on size_t wrapping (while (len--), memmove's overlap test)
# and on the int to char conversions the C standard asks for (memset, strchr).
# The functional properties in the harness pin the results down instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check --conversion-check
include ../../Makefile.common
