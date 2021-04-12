/* Display Definition */
#define CRT1				0x00
#define CRT2				0x01

/* Default Seting */
#if 0//defined in crt.h
#define CRT_LOW_THRESHOLD_VALUE         0x12
#define CRT_HIGH_THRESHOLD_VALUE        0x1E
#endif

/* Reg. Definition */
#define ASPEED_GRAPHIC_VA_BASE	IO_ADDRESS(ASPEED_GRAPHIC_BASE)
#define ASPEED_SCU_VA_BASE	IO_ADDRESS(ASPEED_SCU_BASE)

#define AST3000_VGA1_CTLREG	0x00
#define AST3000_VGA1_CTLREG2	0x04
#define AST3000_VGA1_STATUSREG	0x08 
#define AST3000_VGA1_PLL	0x0C
#define AST3000_VGA1_HTREG	0x10
#define AST3000_VGA1_HRREG	0x14
#define AST3000_VGA1_VTREG	0x18
#define AST3000_VGA1_VRREG	0x1C
#define AST3000_VGA1_STARTADDR	0x20
#define AST3000_VGA1_OFFSETREG	0x24
#define AST3000_VGA1_THRESHOLD	0x28

#define AST3000_VGA2_CTLREG	0x60
#define AST3000_VGA2_CTLREG2	0x64
#define AST3000_VGA2_STATUSREG	0x68 
#define AST3000_VGA2_PLL	0x6C
#define AST3000_VGA2_HTREG	0x70
#define AST3000_VGA2_HRREG	0x74
#define AST3000_VGA2_VTREG	0x78
#define AST3000_VGA2_VRREG	0x7C
#define AST3000_VGA2_STARTADDR	0x80
#define AST3000_VGA2_OFFSETREG	0x84
#define AST3000_VGA2_THRESHOLD	0x88

/* Macro */
#define IO_READ(r)		( *((volatile unsigned int *)(r)) )
#define IO_WRITE(r, v)		( (*((volatile unsigned *) (r))) = ((unsigned int)(v)) )
