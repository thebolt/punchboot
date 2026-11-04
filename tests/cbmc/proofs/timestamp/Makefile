PROOF   = timestamp
SOURCES = $(ROOT)/src/timestamp.c
UNWIND  = 4
PRE_INC = -I $(CURDIR)/cfg
# The tick wraps at 2^32, so the differences of timestamps are modulo 2^32 on
# purpose. Property 3 pins the result down.
CHECK_FLAGS_OFF = --unsigned-overflow-check
include ../../Makefile.common
