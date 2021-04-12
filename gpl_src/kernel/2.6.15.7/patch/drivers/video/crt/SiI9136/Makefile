#
# Makefile for the ASPEED framebuffer device driver
#

obj-$(CONFIG_AST1500_DIGITAL_SOURCE) += sii9136.o
obj-$(CONFIG_AST1500_DIGITAL_SINK) += sii9136.o

sii9136-objs := SiI9136_main.o SiI9136_wrap.o ast_i2c.o TPI_Access.o tpidebug.o TPI.o AV_Config.o EDID.o HDCP.o SiI9136_300.o Globals.o VideoModeTable.o \
si_apiCEC.o  si_apiCpi.o
