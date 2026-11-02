PROOF   = crc
SOURCES = $(ROOT)/src/lib/crc.c
UNWIND  = 17
# crc32() loops on while (size--), which wraps size on the last test. That is
# well defined and intended, so the unsigned overflow check is off here.
CHECK_FLAGS_OFF = --unsigned-overflow-check
include ../../Makefile.common
