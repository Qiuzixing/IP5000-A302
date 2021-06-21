#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net/if.h>
#include <net/if_arp.h>

#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
 #include <errno.h>
#include <pthread.h>
#include "lldpcontrol.h"
#include "ctlconfig.h"
#include "lldpctl.h"
#include "lldp-const.h"
#define MUTEX_INIT(lock) pthread_mutex_init(&lock,NULL)
#define MUTEX_LOCK(lock) pthread_mutex_lock(&lock)
#define MUTEX_UNLOCK(lock) pthread_mutex_unlock(&lock)
#define MUTEX_DESTROY(lock) pthread_mutex_destroy(&lock)

typedef struct _LLDP_ControlMng_T
{
	
	int initFlag;
	pthread_mutex_t selfLock;
	lldpctl_conn_t *conn;
	char ifname[32];
}LLDP_ControlMng_T;


static LLDP_ControlMng_T gs_lldpCtlMng = {0};

#if 1
int get_local_ip(const char *eth_inf, char *ip)
{

	struct ifaddrs *ifaddr = NULL;
	struct ifaddrs *ifa = NULL;
	int family;
	char *host = NULL;
	struct sockaddr_in sin;
	 if (getifaddrs(&ifaddr) == -1) {
	    printf(" getifaddrs err\n");
	    return -1;
	}
  
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	if (ifa->ifa_addr == NULL)
	     continue;

	 family = ifa->ifa_addr->sa_family;

	 if (strcmp(ifa->ifa_name, eth_inf) != 0)
	     continue;
	 if (family == AF_INET) {
		printf("%s get ip info \n",eth_inf);
		memcpy(&sin,ifa->ifa_addr,sizeof( struct sockaddr_in));
		memcpy(ip,&sin.sin_addr.s_addr,sizeof(in_addr_t));
		freeifaddrs(ifaddr);
		return 0;
	     }
	     
	 
	}
	printf(" %s get ip err\n",eth_inf);
	freeifaddrs(ifaddr);
	return -1;
 


}
#else
int get_local_ip(const char *eth_inf, char *ip)
{
	int sd;
	struct sockaddr_in sin;
	struct sockaddr addr;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
	    printf("socket error: %s\n", strerror(errno));
	    return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
	    printf("ioctl error: %s\n", strerror(errno));
	    close(sd);
	    return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	memcpy(ip,&sin.sin_addr.s_addr,sizeof(in_addr_t));
	close(sd);
	return 0;
}
#endif
static int S_GetLldpCtlInitFlag()
{

	return gs_lldpCtlMng.initFlag;
}

static int S_SetLldpCtlCtlInitFlag(int flag)
{
	gs_lldpCtlMng.initFlag = 0;
	if(flag > 0)
	{
		gs_lldpCtlMng.initFlag = 1;
	}
	return 0;
}
static lldpctl_conn_t* S_GetLLdpConnHandle()
{


	return gs_lldpCtlMng.conn;
}
static LLDP_ControlMng_T *S_GetLLdpCtlMngHandle()
{

	return &gs_lldpCtlMng;
}
static int S_GetSingleItem(lldpctl_atom_t *port,LLDP_FrameItem_S* item)
{
	if(port == NULL || item == NULL)
	{
		return -1;
	}
	char *t_str = NULL;
	lldpctl_atom_t *chassis = lldpctl_atom_get(port, lldpctl_k_port_chassis);

	if(!chassis)
	{
		return -1;
	}
	item->port.p_subtype= lldpctl_atom_get_int(port,lldpctl_k_port_id_subtype);
	item->pvid = lldpctl_atom_get_int(port,lldpctl_k_port_vlan_pvid);
	t_str = lldpctl_atom_get_str(port,lldpctl_k_port_id);
	if(t_str)
	{
		memcpy(item->port.p_id, t_str,strlen(t_str));
		t_str = NULL;
	}
	t_str = lldpctl_atom_get_str(port,lldpctl_k_port_descr);
	if(t_str)
	{
		memcpy(item->portDesc, t_str,strlen(t_str));
		t_str = NULL;
	}
	
	item->maxFSize = 0;
	
	item->sec  =  lldpctl_atom_get_int(port,lldpctl_k_port_ttl);

	item->macPhy.autoNegoSup= lldpctl_atom_get_int(port, lldpctl_k_port_dot3_autoneg_support);
	
	item->macPhy.autoNegoStatus=lldpctl_atom_get_int(port, lldpctl_k_port_dot3_autoneg_enabled);
	

	item->chassis.c_subtype =  lldpctl_atom_get_int(chassis,lldpctl_k_chassis_id_subtype);
	
  	t_str = lldpctl_atom_get_str(chassis,lldpctl_k_chassis_id);
	if(t_str)
	{
		memcpy(item->chassis.c_id,t_str ,strlen(t_str));
		t_str = NULL;
	}
	
	t_str = lldpctl_atom_get_str(chassis,lldpctl_k_chassis_name);
	if(t_str)
	{
		memcpy(item->systemName,t_str ,strlen(t_str));
		t_str = NULL;
	}
	
	t_str = lldpctl_atom_get_str(chassis,lldpctl_k_chassis_descr);
	if(t_str)
	{
		memcpy(item->systemDesc,t_str ,strlen(t_str));
		t_str = NULL;
	}
	lldpctl_atom_dec_ref(chassis);
	//lldpctl_atom_dec_ref(port);

	
	return 0;
}
static int S_DefaultLLdpCtl()
{

	lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	lldpctl_atom_t * t_cfgAtom = NULL; 
	int ret = 0;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(t_handle == NULL)
	{
		return -1;
	}
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		printf("[%s]get config err\n",__FUNCTION__);
		return -1;
	}
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		lldpctl_atom_dec_ref(t_cfgAtom);
		printf("[%s]get interfaces err\n",__FUNCTION__);
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	
	lldpctl_atom_dec_ref(t_interfaces);
	
	if(!lldpctl_atom_set_int(t_cfgAtom, lldpctl_k_config_chassis_mgmt_advertise,0)) //MANAGE ment 不要
	{
		printf("---lldpctl_k_config_chassis_mgmt_advertise  err\n");
	}
	if(!lldpctl_atom_set_int(t_cfgAtom, lldpctl_k_config_paused,0))
	{
		
		printf("[%s]set lldpctl_k_config_paused err\n",__FUNCTION__);
		
	}
	lldpctl_atom_dec_ref(t_cfgAtom);
	if(t_portAtom)
	{
		
		
		if(!lldpctl_atom_set_int(t_portAtom, lldpctl_k_port_dot3_mfs, 1522))
		{
			printf("---default lldpctl_k_port_dot3_mfs err\n");
		}
		if(!lldpctl_atom_set_int(t_portAtom, lldpctl_k_port_vlan_pvid, 99)) 
		{
			printf("--default lldpctl_k_port_vlan_tx  err\n");
		}
		lldpctl_atom_dec_ref(t_portAtom);
	}
	else
	{
			printf("---default port get  err\n");
		   return -1;
	}
	LLDP_ControlSystemName("Kramer");
	LLDP_ControlSystemDesc("Kramer");
	ret = LLDP_ControlRightUpdate();
	
	return ret;
}

int LLDP_ControlInit(char* ifName)
{
	LLDP_ControlMng_T* t_handle =NULL;
	int ret = 0;
	t_handle = S_GetLLdpCtlMngHandle();
	if(t_handle == NULL)
	{
		
		return -1;
	}
	if(t_handle->initFlag == 1)
	{
		printf("[%d %s]: already init\n",__LINE__,__FUNCTION__);
		return 0;
	}
	t_handle->conn =NULL;
	t_handle->conn = lldpctl_new_name(DAEMON_SOCKET_PATH, NULL,NULL, NULL);
	if(t_handle->conn == NULL)
	{
		printf("[%d %s]:  err connect  lldpd.socket\n",__LINE__,__FUNCTION__);
		return -1;
	}
	#if 1
	MUTEX_INIT(t_handle->selfLock);
	t_handle->initFlag = 1;
	memcpy(t_handle->ifname,ifName,strlen(ifName));
	//默认设置
	ret = S_DefaultLLdpCtl();
	//printf("-----------lld pport %s-----init  %s-----\n",t_handle->ifname,ret?"fail":"success");
	#endif
	return ret;
}
int LLDP_ControlUInit(char* ifName)
{
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	
	if(!t_handle->initFlag)
	{
		return 0;
	}
	if(t_handle->conn)
	{
		lldpctl_release(t_handle->conn);
		t_handle->conn = NULL;
	}
	MUTEX_DESTROY(t_handle->selfLock);
	t_handle->initFlag = 0;

	return 0;
}
int LLDP_ControlChassisId(int subType,char*id)
{
	char t_cidStr[128] = {0};
	char t_ip_net[64] ={0};
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	char ch = 1;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		return -1;
	}
	//本身库控制函数只支持调用设置MAC LOCAL,这里在设置ip与ifname时做了改动
	switch(subType)
	{
		case LLDPCTL_CHASSISID_SUBTYPE_MACADDR:
		{
			if(!lldpctl_atom_set_str(t_cfgAtom,lldpctl_k_config_cid_string,NULL))
			{
				MUTEX_UNLOCK(t_handle->selfLock);
				lldpctl_atom_dec_ref(t_cfgAtom);
				return -1;
			}
			MUTEX_UNLOCK(t_handle->selfLock);
			lldpctl_atom_dec_ref(t_cfgAtom);
			return 0;
		}
		case LLDPCTL_CHASSISID_SUBTYPE_IPADDR:
		{
			memset(t_ip_net,0,sizeof(t_ip_net));
			get_local_ip(t_handle->ifname,t_ip_net);
			sprintf(t_cidStr,"%s%c%s","ip-addr",ch,t_ip_net);
		}
		break;
		case LLDPCTL_CHASSISID_SUBTYPE_IFNAME:
			sprintf(t_cidStr,"%s%s","if-name",t_handle->ifname);
		break;
		case LLDPCTL_CHASSISID_SUBTYPE_LOCAL:
			if(id != NULL)
			sprintf(t_cidStr,"%s",id);
		break;
		default:
			printf("unknow chassisid subtype %d\n",subType);
			return -1;
	}
	if(!lldpctl_atom_set_str(t_cfgAtom,lldpctl_k_config_cid_string,t_cidStr))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}
int LLDP_ControlInterval(int sec)
{
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag || sec <= 0)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("[%s]get config err\n",__FUNCTION__);
		return -1;
	}
	
	if(!lldpctl_atom_set_int(t_cfgAtom,lldpctl_k_config_tx_interval,sec))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		printf("[%s]set lldpctl_k_config_tx_interval err\n",__FUNCTION__);
		return -1;
	}
	if(!lldpctl_atom_set_int(t_cfgAtom,lldpctl_k_config_tx_hold,1))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		printf("[%s]set lldpctl_k_config_tx_interval err\n",__FUNCTION__);
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}
int LLDP_ControlPortId(int subType,char*id)
{
	int t_tmp = 0;
	char t_id[128]={0};
	lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return -1;
	}
	switch(subType)
	{
		case LLDPCTL_PORTID_SUBTYPE_MAC:
			t_tmp = LLDP_PORTID_SUBTYPE_LLADDR;
		break;
		case LLDPCTL_PORTID_SUBTYPE_IFNAME:
			t_tmp = LLDP_PORTID_SUBTYPE_IFNAME;
		break;
		case LLDPCTL_PORTID_SUBTYPE_LOCAL:
		{
			memcpy(t_id,id,strlen(id));
			if(!lldpctl_atom_set_str(t_portAtom, lldpctl_k_port_id,t_id))
			{
				MUTEX_UNLOCK(t_handle->selfLock);
				lldpctl_atom_dec_ref(t_portAtom);
				return -1;
			}
			lldpctl_atom_dec_ref(t_portAtom);
			MUTEX_UNLOCK(t_handle->selfLock);
			return 0;
		}
		default:
		printf("unknow portid subtype %d\n",subType);		
		return -1;
	}
	if(!lldpctl_atom_set_int(t_portAtom, lldpctl_k_port_id_subtype, t_tmp))
	{
		lldpctl_atom_dec_ref(t_portAtom);
		MUTEX_UNLOCK(t_handle->selfLock);
		return -1;
	}
	lldpctl_atom_dec_ref(t_portAtom);
	MUTEX_UNLOCK(t_handle->selfLock);
	return 0;
}
int LLDP_ControlPortDesc(char*desc)
{
      lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return -1;
	}
	if(!lldpctl_atom_set_str(t_portAtom, lldpctl_k_port_descr, desc))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_portAtom);
		printf("%s set port desc err\n",__FUNCTION__);
		return -1;
	}
	lldpctl_atom_dec_ref(t_portAtom);
	return 0;
}
int LLDP_ControlSystemName(char*name)
{
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		
		return -1;
	}
	if(!lldpctl_atom_set_str(t_cfgAtom, lldpctl_k_config_hostname,name))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}
int LLDP_ControlSystemDesc(char*desc)
{
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		
		return -1;
	}
	if(!lldpctl_atom_set_str(t_cfgAtom, lldpctl_k_config_description,desc))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}
int LLDP_ControlRightUpdate()
{
	
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("[%s]get config err\n",__FUNCTION__);
		return -1;
	}
	
	if(!lldpctl_atom_set_int(t_cfgAtom, lldpctl_k_config_tx_interval,-1))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		printf("[%s]set lldpctl_k_config_tx_interval err\n",__FUNCTION__);
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}

int LLDP_ControlMacPhy(LLDP_MACPHY_S*macphy)
{
	printf(" LLDP_ControlMacPhy just for wait\n");
	return 0;
}
LLDP_FrameItemList_S* LLDP_ControlGetNeighbors(int *number)
{
	lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	lldpctl_atom_t *neighbors = NULL;
	LLDP_FrameItemList_S *tmp = NULL;
	LLDP_FrameItemList_S *head = NULL;
	int t_count = 0;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return NULL;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return NULL;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return NULL;
	}
	value = NULL;
	neighbors=lldpctl_atom_get(t_portAtom,lldpctl_k_port_neighbors);
	if(neighbors == NULL)
	{
		lldpctl_atom_dec_ref(t_portAtom);	
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port neighbors err\n");
		return NULL;
	}
	head = (LLDP_FrameItemList_S*)malloc(sizeof(LLDP_FrameItemList_S));
	if(!head)
	{
		printf("%s %d creat list err\n",__FUNCTION__,__LINE__);
	}
	memset(head,0,sizeof(LLDP_FrameItemList_S));
	list_init(&head->_itemlist);
	lldpctl_atom_foreach(neighbors, value)
	{
		
		tmp =(LLDP_FrameItemList_S*)malloc(sizeof(LLDP_FrameItemList_S));
		if(!tmp)
		{
			continue;
		}
		memset(tmp,0,sizeof(LLDP_FrameItemList_S));
		if(0 != S_GetSingleItem(value,&tmp->item))
		{
			continue;
		}
	
		list_add_tail(&head->_itemlist, &tmp->_itemlist);
		
		t_count++;
		tmp =NULL;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(neighbors);	
 	lldpctl_atom_dec_ref(t_portAtom);	

	
 	*number = t_count;
	
	return head;
}



int LLDP_ControlGetLocalTLVinfo(LLDP_FrameItem_S*item)
{
	lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	lldpctl_atom_t *chassis = NULL;
	LLDP_FrameItemList_S *tmp = NULL;

	LLDP_ControlMng_T* t_handle =NULL;
	char *t_str = NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	if(item == NULL)
	{
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return -1;
	}

	item->port.p_subtype= lldpctl_atom_get_int(t_portAtom,lldpctl_k_port_id_subtype);
	t_str = lldpctl_atom_get_str(t_portAtom,lldpctl_k_port_id);
	if(t_str)
	{
		memcpy(item->port.p_id, t_str,strlen(t_str));
		t_str = NULL;
	}
	
	S_GetSingleItem(t_portAtom,item);
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_portAtom);
	return 0;
}

int LLDP_ControlReleaseNeighbors(LLDP_FrameItemList_S*neighbors)
{
	struct listnode *node = NULL;
	LLDP_FrameItemList_S *item = NULL;
	list_for_each(node, &(neighbors->_itemlist)){
		item = (LLDP_FrameItemList_S*)node_to_item(node, LLDP_FrameItemList_S, _itemlist);
		list_remove(&(item->_itemlist));
		free(item);
		item = NULL;
	}
	free(neighbors);
	return 0;
}

int LLDP_ControlMaxNeighbors(int count)
{
	lldpctl_atom_t * t_cfgAtom = NULL; 
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag || count <= 0)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	
	MUTEX_LOCK(t_handle->selfLock);
	t_cfgAtom = lldpctl_get_configuration(t_handle->conn);
	if(!t_cfgAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("[%s]get config err\n",__FUNCTION__);
		return -1;
	}
	
	if(!lldpctl_atom_set_int(t_cfgAtom, lldpctl_k_config_max_neighbors,count))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_cfgAtom);
		printf("[%s]set lldpctl_k_config_tx_interval err\n",__FUNCTION__);
		return -1;
	}
	MUTEX_UNLOCK(t_handle->selfLock);
	lldpctl_atom_dec_ref(t_cfgAtom);
	return 0;
}

int LLDP_ControlRecvSendFlag(int flag)
{
	 lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return -1;
	}
	if(!lldpctl_atom_set_int(t_portAtom, lldpctl_k_port_status, flag))
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		lldpctl_atom_dec_ref(t_portAtom);
		printf("%s set  rx tx flag err\n",__FUNCTION__);
		return -1;
	}
	lldpctl_atom_dec_ref(t_portAtom);
	return 0;


}

int LLDP_ControlGetRecvSendFlag(int* flag)
{
	 lldpctl_atom_t * t_portAtom = NULL; 
	lldpctl_atom_t *t_interfaces = NULL;
	lldpctl_atom_t *value = NULL;
	LLDP_ControlMng_T* t_handle =NULL;
	t_handle = S_GetLLdpCtlMngHandle();
	if(!t_handle->initFlag)
	{
		printf("  please do LLDP_ControlInit first\n");
		return -1;
	}
	MUTEX_LOCK(t_handle->selfLock);
	t_interfaces = lldpctl_get_interfaces(t_handle->conn);
	if(!t_interfaces)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get interfaces err\n");
		return -1;
	}
	lldpctl_atom_foreach(t_interfaces,value)
	{
		if(lldpctl_atom_get_str(value, lldpctl_k_interface_name) &&
			!strcmp(lldpctl_atom_get_str(value, lldpctl_k_interface_name),t_handle->ifname))
		{
			
			t_portAtom = lldpctl_get_port(value);
			break;
		}
	}
	lldpctl_atom_dec_ref(t_interfaces);
	if(!t_portAtom)
	{
		MUTEX_UNLOCK(t_handle->selfLock);
		printf("get port info err\n");
		return -1;
	}
	*flag=lldpctl_atom_get_int(t_portAtom, lldpctl_k_port_status);
	
	lldpctl_atom_dec_ref(t_portAtom);
	return 0;

}