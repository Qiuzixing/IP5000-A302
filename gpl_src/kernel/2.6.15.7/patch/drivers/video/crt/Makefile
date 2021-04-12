#
# Makefile for the ASPEED framebuffer device driver
#

obj-$(CONFIG_CRT_ASPEED) += display.o
obj-$(CONFIG_AST1500_DIGITAL_SOURCE) += cat6613/
obj-$(CONFIG_AST1500_DIGITAL_SINK) += cat6613/
#obj-$(CONFIG_AST1500_DIGITAL_SOURCE) += SiI9136/
obj-$(CONFIG_AST1500_DIGITAL_SINK) += SiI9136/
obj-$(CONFIG_AST1500_DIGITAL_SINK) += sii9678/
obj-$(CONFIG_AST1500_DIGITAL_SOURCE) += sii9678/


display-objs := xmiter_hal.o crt_main.o crt.o cursor.o network.o EDID.o edid_v1.o edid_v2.o
