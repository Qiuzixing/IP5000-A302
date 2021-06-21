#ifndef __LLDPCONTROL_H__
#define __LLDPCONTROL_H__




#ifdef __cplusplus
extern "C" {
#endif
#include "list.h"

//定义CHASSIS包subtype 
#define LLDPCTL_CHASSISID_SUBTYPE_MACADDR 4   //mac地址
#define LLDPCTL_CHASSISID_SUBTYPE_IPADDR 5     //ip地址
#define LLDPCTL_CHASSISID_SUBTYPE_IFNAME 6    //网卡名
#define LLDPCTL_CHASSISID_SUBTYPE_LOCAL 7     //自定义

//定义PORTid包subtype 
#define LLDPCTL_PORTID_SUBTYPE_MAC 3    //mac地址
#define LLDPCTL_PORTID_SUBTYPE_IFNAME 5  //网卡名
#define LLDPCTL_PORTID_SUBTYPE_LOCAL 7  //自定义

#define LLDPCTL_RXTX_UNCHANGED 0  
#define LLDPCTL_TX_ONLY 1  //只发送
#define LLDPCTL_RX_ONLY 2 //只接收
#define LLDPCTL_RXTX_DISABLE 3 //都不执行
#define LLDPCTL_RXTX_BOTH 4  //都执行

#define MAX_CHASSIS_ID_LEN 64
#define MAX_PORT_ID_LEN 64
#define MAX_PORT_DESC_LEN 256
#define MAX_SYSTEMNAME_LEN 256
#define MAX_SYSTEM_DESC_LEN 512
typedef struct _LLDP_ChassisId_S
{
	int c_subtype ; //subtype
	char c_id[MAX_CHASSIS_ID_LEN];  //id comment
}LLDP_ChassisId_S;
typedef struct _LLDP_PortId_S
{
	int p_subtype ;//subtype
	char p_id[MAX_PORT_ID_LEN];//id comment
}LLDP_PortId_S;

typedef struct _LLDP_MACPHY_S
{
	int autoNegoSup; //auto-negotiation supported
	int autoNegoStatus; //auto-negotiation status
}LLDP_MACPHY_S;
typedef struct  _LLDP_FrameItem_S
{
	int sec;     //ttl间隔
	int maxFSize;  //保留
	int pvid;  
	LLDP_ChassisId_S chassis;   //chassis包信息
	LLDP_PortId_S port;      //portid 包信息
	LLDP_MACPHY_S  macPhy;//mac/phy信息只读
	char portDesc[MAX_PORT_DESC_LEN];
	char systemName[MAX_SYSTEMNAME_LEN]; //系统名称
	char systemDesc[MAX_SYSTEM_DESC_LEN];  //系统描述
}LLDP_FrameItem_S;


typedef struct _LLDP_FrameItemList_S
{
	LLDP_FrameItem_S item;
	struct listnode _itemlist;
}LLDP_FrameItemList_S;
/********************************
功能说明: 控制接口初始化,初始化时会立即发送一条LLDP包
输入参数:ifName 网口名例"eth0"
输出参数:无
返回值:0 成功,其他:失败
**********************************/
int LLDP_ControlInit(char* ifName);

/********************************
功能说明: 控制接口去初始化
输入参数:ifName 网口名例"eth0"
输出参数:无
返回值:0 成功,其他:失败
**********************************/
int LLDP_ControlUInit(char* ifName);




/********************************
功能说明: 设置Chassis 包信息
输入参数:subtype:idsubtype类型,类型为mac,ip,ifname时id可为NULL,id:自定义Chassisid内容
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlChassisId(int subType,char*id);


/********************************
功能说明: 设置LLDP portId包信息
输入参数:subtype:idsubsubtype类型,类型为mac,ifname时id可为NULL,id:自定义portid内容
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlPortId(int subType,char*id);

/********************************
功能说明: 设置LLDP portId包信息
输入参数:desc:port描述字符串
输出参数:无
返回值:0 成功,其他:失败
**********************************/
int LLDP_ControlPortDesc(char*desc); 

/********************************
功能说明:设置SystemName   名称,例"Ubuntu"
输入参数:name:系统名称字符串
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlSystemName(char*name);

/********************************
功能说明: 设置系统描述信息
输入参数:desc: 系统描述字符穿
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlSystemDesc(char*desc);

/********************************
功能说明: 设置LLDP 定期发送间隔TTL
输入参数:sec 单位为秒
输出参数:无
返回值:0 成功,其他:失败
**********************************/
int LLDP_ControlInterval(int sec);



/********************************
功能说明: 设置LLDP MAC/PHY 包信息
输入参数:macphy 信息结构体
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlMacPhy(LLDP_MACPHY_S*macphy);


/********************************
功能说明:更新发送信息，调用此接口会立即更新发送LLDP包
输入参数:无
输出参数:无
返回值:0 成功,其他:失败**********************************/

int LLDP_ControlRightUpdate();




/********************************
功能说明:获取当前发现的所有邻居
输入参数:无
输出参数:number
返回值:NULL 失败 ,!NULL 获取到 数据地址
**********************************/
LLDP_FrameItemList_S* LLDP_ControlGetNeighbors(int *number);


/********************************
功能说明:获取当前发现的所有邻居
输入参数:无
输出参数:number
返回值:NULL 失败 ,!NULL 获取到 数据地址
**********************************/
int LLDP_ControlGetLocalTLVinfo(LLDP_FrameItem_S*item);

/********************************
功能说明:获取neighbors后，调用此函数进行释放空间
输入参数:neighbos 列表地址
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlReleaseNeighbors(LLDP_FrameItemList_S*neighbors);


/********************************
功能说明:设置能发现的最多的邻居数目
输入参数:count :最大数目值
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int  LLDP_ControlMaxNeighbors(int count);


/********************************
功能说明: 设置设备监听发送标志
输入参数:flag  标志 1.TX ONLY 2.RX ONLY 3,NO 4,RXTX
输出参数:无
返回值:0 成功,其他:失败
**********************************/

int LLDP_ControlRecvSendFlag(int flag);

/********************************
功能说明: 获取设备监听发送标志
输入参数:无
输出参数:flag  标志 1.TX ONLY 2.RX ONLY 3,NO 4,RXTX
返回值:0 成功,其他:失败
**********************************/
int LLDP_ControlGetRecvSendFlag(int* flag);

#ifdef __cplusplus
}
#endif

#endif
