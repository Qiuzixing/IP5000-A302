
#ifndef _OSDDATA_H_
#define _OSDDATA_H_

#if OSD_FROM_GUI
u8 *OSDDATA[] = {};
#define OSD_DATA_NUM 0

#else //#if OSD_FROM_GUI

u8 osd_data_0[] =
{
	#include "osd_0.h"
};
u8 osd_data_1[] =
{
	#include "osd_1.h"
};
u8 osd_data_2[] =
{
	#include "osd_2.h"
};
u8 osd_data_3[] =
{
	#include "osd_3.h"
};
u8 osd_data_4[] =
{
	#include "osd_4.h"
};
u8 osd_data_5[] =
{
	#include "osd_5.h"
};
u8 osd_data_6[] =
{
	#include "osd_6.h"
};
u8 osd_data_7[] =
{
	#include "osd_7.h"
};
u8 osd_data_8[] =
{
	#include "osd_8.h"
};
u8 osd_data_9[] =
{
	#include "osd_9.h"
};

u8 *OSDDATA[] =
{
	osd_data_0,
	osd_data_1,
	osd_data_2,
	osd_data_3,
	osd_data_4,
	osd_data_5,
	osd_data_6,
	osd_data_7,
	osd_data_8,
	osd_data_9,
};

#define OSD_DATA_NUM 10
#endif

#if 0
BYTE OSDDATA[4][352*50*2+64] =
{
	{
		#include "osd_0.h"
	},
	{
		#include "osd_1.h"
	},
	{
		#include "osd_2.h"
	},
	{
		#include "osd_3.h"
	},
//#include "osddata0.h"
//#include "osddata1.h"
//#include "osddata2.h"
//#include "osddata3.h"
//#include "osddata4.h"
//#include "osddata5.h"
};
#endif

#endif

