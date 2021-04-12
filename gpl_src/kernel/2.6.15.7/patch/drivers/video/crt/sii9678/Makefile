#
# Makefile for the SiliconImage 9678/9679 device driver
#
DRIVER=sii9678
obj-$(CONFIG_AST1500_DIGITAL_SINK) += $(DRIVER).o
obj-$(CONFIG_AST1500_DIGITAL_SOURCE) += $(DRIVER).o

$(DRIVER)-objs := ast_main.o si_drv_adapter.o si_drv_adapter_internal.o si_drv_adapter_flash.o si_drv_adapter_tx.o
