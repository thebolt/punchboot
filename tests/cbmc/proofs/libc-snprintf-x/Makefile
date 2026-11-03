PROOF   = libc-snprintf-x
SOURCES = $(ROOT)/src/lib/libc/snprintf.c
UNWIND  = 13
# 10 decimal digits, 8 hex digits (each loop bounded by the value it prints)
CBMC_EXTRA = --unwindset unsigned_dec_print.0:11,unsigned_dec_print.1:11,unsigned_num_print.0:9,unsigned_num_print.1:9,unsigned_num_print.2:9
# The firmware libc itself: no string models, no builtins. The unsupported
# specifier branch logs, so the log stub is linked.
MODELS  = $(ROOT)/tests/cbmc/stubs/log.c
EXTRA_DEFS = -fno-builtin
# The libc relies on size_t wrapping and on the int to char conversions the C
# standard asks for. The functional properties in the harness pin the results
# down instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check --conversion-check
include ../../Makefile.common
