PROOF   = delay
SOURCES = $(ROOT)/src/delay.c
UNWIND  = 4
# The tick is a 32-bit counter that wraps, and the timeouts compute the end
# and the elapsed time modulo 2^32 on purpose. The harness pins the results
# down instead (properties 2 and 3).
CHECK_FLAGS_OFF = --unsigned-overflow-check
include ../../Makefile.common
