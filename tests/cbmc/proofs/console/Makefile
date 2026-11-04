PROOF   = console
# console.c is included by the harness (to reach the muted ranges)
SOURCES =
UNWIND  = 9
# The harness has its own console_uart_mute (console.c) and printf
EXTRA_DEFS = -DPB_VERSION=\"cbmc\"
MODELS  = $(ROOT)/tests/cbmc/stubs/string_models.c
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/console.c
