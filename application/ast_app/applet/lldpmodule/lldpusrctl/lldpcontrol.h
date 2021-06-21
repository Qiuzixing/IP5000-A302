#ifndef __LLDPCONTROL_H__
#define __LLDPCONTROL_H__




#ifdef __cplusplus
extern "C" {
#endif
#include "list.h"

//����CHASSIS��subtype 
#define LLDPCTL_CHASSISID_SUBTYPE_MACADDR 4   //mac��ַ
#define LLDPCTL_CHASSISID_SUBTYPE_IPADDR 5     //ip��ַ
#define LLDPCTL_CHASSISID_SUBTYPE_IFNAME 6    //������
#define LLDPCTL_CHASSISID_SUBTYPE_LOCAL 7     //�Զ���

//����PORTid��subtype 
#define LLDPCTL_PORTID_SUBTYPE_MAC 3    //mac��ַ
#define LLDPCTL_PORTID_SUBTYPE_IFNAME 5  //������
#define LLDPCTL_PORTID_SUBTYPE_LOCAL 7  //�Զ���

#define LLDPCTL_RXTX_UNCHANGED 0  
#define LLDPCTL_TX_ONLY 1  //ֻ����
#define LLDPCTL_RX_ONLY 2 //ֻ����
#define LLDPCTL_RXTX_DISABLE 3 //����ִ��
#define LLDPCTL_RXTX_BOTH 4  //��ִ��

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
	int sec;     //ttl���
	int maxFSize;  //����
	int pvid;  
	LLDP_ChassisId_S chassis;   //chassis����Ϣ
	LLDP_PortId_S port;      //portid ����Ϣ
	LLDP_MACPHY_S  macPhy;//mac/phy��Ϣֻ��
	char portDesc[MAX_PORT_DESC_LEN];
	char systemName[MAX_SYSTEMNAME_LEN]; //ϵͳ����
	char systemDesc[MAX_SYSTEM_DESC_LEN];  //ϵͳ����
}LLDP_FrameItem_S;


typedef struct _LLDP_FrameItemList_S
{
	LLDP_FrameItem_S item;
	struct listnode _itemlist;
}LLDP_FrameItemList_S;
/********************************
����˵��: ���ƽӿڳ�ʼ��,��ʼ��ʱ����������һ��LLDP��
�������:ifName ��������"eth0"
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/
int LLDP_ControlInit(char* ifName);

/********************************
����˵��: ���ƽӿ�ȥ��ʼ��
�������:ifName ��������"eth0"
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/
int LLDP_ControlUInit(char* ifName);




/********************************
����˵��: ����Chassis ����Ϣ
�������:subtype:idsubtype����,����Ϊmac,ip,ifnameʱid��ΪNULL,id:�Զ���Chassisid����
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlChassisId(int subType,char*id);


/********************************
����˵��: ����LLDP portId����Ϣ
�������:subtype:idsubsubtype����,����Ϊmac,ifnameʱid��ΪNULL,id:�Զ���portid����
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlPortId(int subType,char*id);

/********************************
����˵��: ����LLDP portId����Ϣ
�������:desc:port�����ַ���
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/
int LLDP_ControlPortDesc(char*desc); 

/********************************
����˵��:����SystemName   ����,��"Ubuntu"
�������:name:ϵͳ�����ַ���
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlSystemName(char*name);

/********************************
����˵��: ����ϵͳ������Ϣ
�������:desc: ϵͳ�����ַ���
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlSystemDesc(char*desc);

/********************************
����˵��: ����LLDP ���ڷ��ͼ��TTL
�������:sec ��λΪ��
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/
int LLDP_ControlInterval(int sec);



/********************************
����˵��: ����LLDP MAC/PHY ����Ϣ
�������:macphy ��Ϣ�ṹ��
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlMacPhy(LLDP_MACPHY_S*macphy);


/********************************
����˵��:���·�����Ϣ�����ô˽ӿڻ��������·���LLDP��
�������:��
�������:��
����ֵ:0 �ɹ�,����:ʧ��**********************************/

int LLDP_ControlRightUpdate();




/********************************
����˵��:��ȡ��ǰ���ֵ������ھ�
�������:��
�������:number
����ֵ:NULL ʧ�� ,!NULL ��ȡ�� ���ݵ�ַ
**********************************/
LLDP_FrameItemList_S* LLDP_ControlGetNeighbors(int *number);


/********************************
����˵��:��ȡ��ǰ���ֵ������ھ�
�������:��
�������:number
����ֵ:NULL ʧ�� ,!NULL ��ȡ�� ���ݵ�ַ
**********************************/
int LLDP_ControlGetLocalTLVinfo(LLDP_FrameItem_S*item);

/********************************
����˵��:��ȡneighbors�󣬵��ô˺��������ͷſռ�
�������:neighbos �б��ַ
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlReleaseNeighbors(LLDP_FrameItemList_S*neighbors);


/********************************
����˵��:�����ܷ��ֵ������ھ���Ŀ
�������:count :�����Ŀֵ
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int  LLDP_ControlMaxNeighbors(int count);


/********************************
����˵��: �����豸�������ͱ�־
�������:flag  ��־ 1.TX ONLY 2.RX ONLY 3,NO 4,RXTX
�������:��
����ֵ:0 �ɹ�,����:ʧ��
**********************************/

int LLDP_ControlRecvSendFlag(int flag);

/********************************
����˵��: ��ȡ�豸�������ͱ�־
�������:��
�������:flag  ��־ 1.TX ONLY 2.RX ONLY 3,NO 4,RXTX
����ֵ:0 �ɹ�,����:ʧ��
**********************************/
int LLDP_ControlGetRecvSendFlag(int* flag);

#ifdef __cplusplus
}
#endif

#endif
