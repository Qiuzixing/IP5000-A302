ifeq ($(CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE),2)
   zreladdr-y	:= 0x80008000
params_phys-y	:= 0x80000100
initrd_phys-y	:= 0x80800000
endif

ifeq ($(CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE),1)
   zreladdr-y	:= 0x40008000
params_phys-y	:= 0x40000100
initrd_phys-y	:= 0x40800000
endif

