#
# Makefile for the linux kernel.
#

# Common support (must be linked before board specific support)
obj-y += mm.o irq.o timer.o ast1500.o aes.o ast-scu_ast15xx.o ast-scu_ast1500.o ast-scu_ast1510.o ast-scu_ast1520.o

# Specific board support
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_AST1500_GPIO) += gpio.o

# Dynamic power management
obj-$(CONFIG_DPM)			+= aspeed_dpm.o
