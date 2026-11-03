PROOF   = ramlog
# ramlog.c is included by the harness (to reach the static log)
SOURCES =
UNWIND  = 3
PRE_INC = -I $(CURDIR)/cfg
# head counts the bytes written and wraps at 2^32 on purpose: the size is a
# power of two, so head % size stays continuous. The ring position is checked
# by the harness instead.
CHECK_FLAGS_OFF = --unsigned-overflow-check
# ramlog.c includes <arch/arch.h>
EXTRA_INC = -I $(ROOT)/src/arch/$(if $(filter 64,$(WORD)),armv8a,armv7a)/include
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/ramlog.c
