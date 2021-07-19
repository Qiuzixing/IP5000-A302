#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>


extern bool g_bDeviceSleepMode; //  设备工作状态  TRUE：休眠  FALSE： 正常
extern bool g_bOSDMeunDisplay;  // 菜单的显示状态 TRUE:显示 FALSE:隐藏

#define MSGD_IP "127.0.0.1"
#define MSGD_FE_PORT 6850

#define INFOTYPE_RT 68500001
#define INFOTYPE_ST 68500002
#define INFOTYPE_OSD 68500003
#define INFOTYPE_GUI_ACTION 68500004

#define Small 0
#define Mid   1
#define Big   2

#define OSD_DOWN_TIME 	(20*1000)	// 20s

#define ALIGN_FLAG		(1 << 0)
#define	FONT_SIZE_FLAG	(1 << 1)
#define	FONT_COLOR_FLAG	(1 << 2)
#define	BG_COLOR_FLAG	(1 << 3)
#define	TEXT_FLAG		(1 << 4)
#define DISPLAY_LONG	(1 << 5)

#define XPOS 525
#define YPOS 325

#define OSD_XPOS 30
#define OSD_YPOS 30;


#define	TOP_LEFT		1
#define	TOP_MID			2
#define	TOP_RIGTH		3


#define	LEFT_MID		4
#define	CENTER			5
#define	RIGHT_MID		6

#define	BOTTOM_LEFT		7
#define	BOTTOM_MID		8
#define	BOTTOM_RIGHT	9


#define RESTART_APP		777

#endif // GLOBAL_H
