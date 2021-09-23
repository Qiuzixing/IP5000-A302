
#ifndef __802_1X_VERIFY_H__
#define __802_1X_VERIFY_H__

#include <stdio.h>
#include <pthread.h>
#include <string>

#include "Exec.h"
#include "MsgQueue.h"
#include "Timers.h"

typedef enum 
{
	IEEE_802_1X_SUCCESS 		= 1,
	IEEE_802_1X_FAIL			,
	IEEE_802_1X_ERROR_TIMEOUT	,
	IEEE_802_1X_ERROR_INIT		,
	IEEE_802_1X_ERROR_INTERNAL	,
}ieee802_1x_Status;

typedef struct 
{
	std::string strUser;
	std::string strPasswd;
	std::string strEapType;
	std::string strCaCert;
	std::string strClientCert;
	std::string strPrivateKey;
	std::string strPrivateKeyPasswd;
	std::string strPhase2Type;
}T_AuthInfo;

void ClearAuthInfo(T_AuthInfo *i_tAuthInfo);

class PthreadHandle
{
public:
	PthreadHandle(unsigned int u32Num);
	int Init(int i_s32Msec);
	int Deinit();
	void MsgHandle(ieee802_1x_Status Status);
	friend void *EventHandle(void *i_pArg);
	
	int m_s32SuccessNum;
	int m_s32FailNum;
	int m_s32TimeoutNum;
	int m_s32OtherNum;
	int m_s32AllNum;
	int m_s32Flg;
	int m_s32ConnectState;
	MessageQueue m_objMsgQueue;
	Timers m_objTimer;

private:
	pthread_t m_PthId;
	struct MsgBuf m_tMsg;
	
};

class IEEE_802_1X_Verify
{
public:
	PthreadHandle m_objRcvMsgThread;
	
	IEEE_802_1X_Verify(unsigned int i_u32Num, std::string i_strDir, std::string i_strLog4Conf); //认证次数，测试可设置自己想要达到的次数，实际应用时设置为1
	int Init(int i_s32Msec);
	int Deinit();
	int Connect();
	int Disconnect();
	int IEEE802_1X_MD5_Method(std::string i_strUser, std::string i_strPasswd);
	int IEEE802_1X_GTC_Method(std::string i_strUser, std::string i_strPasswd);
	int IEEE802_1X_TLS_Method(std::string i_strCaCert, std::string i_strClientCert,
									std::string i_strPrivateKey, std::string i_strPrivateKeyPasswd);
	int IEEE802_1X_TTLS_Method(std::string i_strUser, std::string i_strPasswd, 
									std::string i_strCaCert);
	int IEEE802_1X_PEAP_CHAPV2_Method(std::string i_strUser, std::string i_strPasswd, 
											  std::string i_strCaCert);
	int IEEE802_1X_PEAP_GTC_Method(std::string i_strUser, std::string i_strPasswd, 
										   std::string i_strCaCert);
private:
	int WriteCfgFile(const T_AuthInfo *i_tAuthInfo);
	int WpaCtrlCmd(std::string i_strCtrlCmd);
	int WPA_RunServer();
	int WPA_RunDaemon();
	
	FILE *m_pFileDescri;
	struct wpa_ctrl *m_pWpaCtrlFd;
	Exec m_objExecProgrem;
	
	std::string m_str802_1XDir;
	std::string m_strWPA_SERVER;
	std::string m_strWPA_DAEMON;
	std::string m_strWPA_CTRL_FILE;
	std::string m_strWPA_CONFIG_FILE;
	std::string m_str802_1X_Log;
	std::string m_strLog4Conf;
};

#endif

