PROOF   = gpt
# gpt.c is included by the harness (to reach the static tables)
SOURCES = $(ROOT)/src/lib/uuid/conv.c
# The description loop runs 36 times
UNWIND  = 40
# gpt_write_tbl (only called to recover a corrupt table) is the gpt-write
# proof. Here it returns any value: it only rewrites the backup table and the
# CRC fields of the primary header, and the partitions are created from
# primary.part and primary.hdr.no_of_parts. Inlined, the table copies do not
# fit in memory (CBMC flattens the 128 entry arrays into bit vectors).
ABSTRACT = gpt_write_tbl
# memcpy checks its preconditions but copies nothing (stubs/string_models.c).
# The copies are between the two tables (header and entries), which both hold
# any value with at most GPT_PARTS entries; the fields the code uses after a
# copy are either assigned right after it or any value anyway, so the
# destination already covers what the copy would put there. Exact copies do
# not fit in memory.
EXTRA_DEFS = -DPB_CBMC_MEMCPY_CHECK_ONLY=0
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/drivers/partition/gpt.c
