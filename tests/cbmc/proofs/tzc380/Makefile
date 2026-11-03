PROOF   = tzc380
# tzc380.c is included by the harness (with the mmio stubs)
SOURCES =
# The size_log2 loop runs up to 64 times
UNWIND  = 66
# The 64-bit base is split into two 32-bit register words on purpose; the
# harness checks both words exactly instead
CHECK_FLAGS_OFF = --conversion-check
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/drivers/tzc/tzc380.c
