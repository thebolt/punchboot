PROOF   = gpt-write-128
# gpt.c is included by the harness (to reach the static tables)
SOURCES = $(ROOT)/src/lib/uuid/conv.c
UNWIND  = 4
# memcpy checks its preconditions but copies nothing (stubs/string_models.c).
# gpt_write_tbl copies the primary table to the backup; both hold any value
# and the same no_of_parts, and the backup header fields it uses are assigned
# right after the copy, so the destination already covers what the copy would
# put there. Exact copies do not fit in memory.
EXTRA_DEFS = -DPB_CBMC_MEMCPY_CHECK_ONLY=0
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/drivers/partition/gpt.c ../gpt/harness.c ../gpt-write/harness.c
