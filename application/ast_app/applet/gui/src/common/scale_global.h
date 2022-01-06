#ifndef APP_GLOBAL_H
#define APP_GLOBAL_H

extern int g_nScreenWidth;              // 当前屏幕宽度
extern int g_nScreenHeight;             // 当前屏幕高度

extern float g_fScaleScreen;            // 屏幕缩放因子

const int g_nMaxPhysicsWidth  = 16000;  // 最大物理屏幕宽度
const int g_nMaxPhysicsHeight = 9000;   // 最大物理屏幕高度

const int g_nStdScreenWidth  = 1920;    // 标准显示屏幕宽度
const int g_nStdScreenHeight = 1080;    // 标准显示屏幕高度

const int g_nframebufferWidth = 1280;
const int g_nframebufferHeight = 720;

const int g_nOsdMenuWidth = 270;
const int g_nButtonWidth = 135;

const int g_nButtonHeight = 30;
const int g_nItemRowHeight = 30;


const int TOP_TOOLBAR_HEIGHT 	= 39;
const int BOTTOM_TOOLBAR_HEIGHT = 39;
const int SIDERBAR_WIDTH 		= 210;
const int SIDERBAR_HEIGHT 		= 760;
const int SIDER_BUTTON_WIDTH 	= 55;
const int SIDER_BUTTON_HEIGHT 	= 150;

const int g_nLayoutDockWidget = 210; 	// 布局窗口界面宽度
const int g_nLayoutItemWidth = 200;     // 布局选项宽度
const int g_nLayoutItemHeight = 85;     // 布局选项高度

const int g_nLayoutMgrItemWidth = 150;  // 管理布局设置界面宽度
const int g_nLayoutMgrItemHeight = 120; // 管理布局设置界面高度


#endif // APP_GLOBAL_H

