PROOF   = libc-strtoul-overflow
SOURCES = $(ROOT)/src/lib/libc/strtoul.c
UNWIND  = 10
# The firmware libc itself: no string models, no builtins
MODELS  =
EXTRA_DEFS = -fno-builtin
# The libc relies on size_t wrapping and on the int to char conversions the C
# standard asks for. The functional properties in the harness pin the results
# down instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check --conversion-check
# strtoul.c includes <arch/arch.h>
EXTRA_INC = -I $(ROOT)/src/arch/$(if $(filter 64,$(WORD)),armv8a,armv7a)/include
include ../../Makefile.common
