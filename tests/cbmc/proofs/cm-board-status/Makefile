PROOF   = cm-board-status
# cm_main.c is included by the harness (to reach its static state)
SOURCES = $(ROOT)/src/wire.c $(ROOT)/tests/cbmc/stubs/cm_env.c \
          $(ROOT)/src/lib/uuid/copy.c
UNWIND  = 3
CBMC_EXTRA = --unwindset memcmp.0:65,uuid_copy.0:17
EXTRA_DEFS = -DPB_VERSION=\"cbmc\"
# Every function without a body returns any value and fills its pointer
# arguments with any value
UNDEFINED = havoc,params:.*
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/cm/cm_main.c ../cm-handlers/harness.c
