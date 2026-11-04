PROOF   = main
# main.c and self_test.c are included by the harness (to place the self test
# section)
SOURCES =
UNWIND  = 4
PRE_INC = -I $(CURDIR)/cfg
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/main.c $(ROOT)/src/self_test.c
