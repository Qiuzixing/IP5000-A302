#define	CMDQ_MODE	0x00
#define MMIO_MODE	0x01

struct astfb_dfbinfo {

	unsigned long ulFBSize;
	unsigned long ulFBPhys;

	unsigned long ulCMDQSize;
	unsigned long ulCMDQOffset;
	unsigned long ul2DMode;
	
};

#define ASTFB_GET_DFBINFO	_IOR(0xF3,0x00,struct astfb_dfbinfo)
