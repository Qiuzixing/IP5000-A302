#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_COMMANDNAME_LEN  32
#define MAX_PARAM_LEN      256
#define MAX_USR_STR_LEN   4*1024
typedef struct _P3K_SimpleCmdInfo_S
{
	char	 command[MAX_COMMANDNAME_LEN];  //ÃüÁî×Ö·û´®"KDS-AUD"
	char param[MAX_PARAM_LEN];    //²ÎÊý×Ö·û´® param1,param2,param3
	
}P3K_SimpleCmdInfo_S;
/*
typedef struct _P3K_SimpleCmdRespInfo_S
{
	int   userDataFlag;
	char	 command[MAX_COMMANDNAME_LEN];  //ÃüÁî×Ö·û´®"KDS-AUD"
	char param[MAX_PARAM_LEN];    //²ÎÊý×Ö·û´® param1,param2,param3
	char userData[MAX_USR_STR_LEN];
}P3K_SimpleCmdRespInfo_S;
*/
/*
typedef struct _P3K_SimpleSpecCmdInfo_S
{
	P3K_SimpleCmdInfo_S cmdinfo;
	char userDefine[MAX_USR_STR_LEN];
}P3K_SimpleSpecCmdInfo_S;
*/
#ifdef __cplusplus
}
#endif




#endif
