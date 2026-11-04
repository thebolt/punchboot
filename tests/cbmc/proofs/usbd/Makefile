PROOF   = usbd
# pb_dev_cls.c is included by the harness (to reach its string descriptors)
SOURCES = $(ROOT)/src/drivers/usb/usbd.c
UNWIND  = 38
# goto-cc warns that pb_dev_cls_init's &descriptors has an incompatible pointer
# type: CBMC types an initialized flexible array member differently (see the
# harness). It is the only warning; the harness does not use pb_dev_cls_init.
WERROR  =
include ../../Makefile.common

$(OUT).gb $(OUT).cover.gb: $(ROOT)/src/drivers/usb/pb_dev_cls.c
