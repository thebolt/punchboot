PROOF   = cm-gate
# cm_main.c is included by the harness (to reach its static state)
SOURCES = $(ROOT)/src/wire.c
UNWIND  = 2
EXTRA_DEFS = -DPB_VERSION=\"cbmc\"
# Every function without a body fails when called, and the path ends there
# (see the harness)
UNDEFINED = assert-false-assume-false
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/cm/cm_main.c
