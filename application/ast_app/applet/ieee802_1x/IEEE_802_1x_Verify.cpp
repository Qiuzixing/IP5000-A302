
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "IEEE_802_1x_Verify.h"

#include "wpa_ctrl.h"
#include "file.h"
#include "log.h"

#define WPA_SERVER 			"wpa_supplicant_802_1x" 
#define WPA_DAEMON			"wpa_daemon"

#define WPA_CONFIG 			"wpa_supplicant_802_1x.conf"
#define WPA_CTRL_FILE   	"wpa_supplicant/eth0"
#define _802_1X_LOG			"8021.log"


#define BUF_SIZE			1024

using namespace std;

void ClrAuthInfo(T_AuthInfo *i_tAuthInfo)
{
	memset((void*)i_tAuthInfo->strUser.c_str(), 0, i_tAuthInfo->strUser.size());
	memset((void*)i_tAuthInfo->strPasswd.c_str(), 0, i_tAuthInfo->strPasswd.size());
	memset((void*)i_tAuthInfo->strEapType.c_str(), 0, i_tAuthInfo->strEapType.size());
	memset((void*)i_tAuthInfo->strCaCert.c_str(), 0, i_tAuthInfo->strCaCert.size());
	memset((void*)i_tAuthInfo->strClientCert.c_str(), 0, i_tAuthInfo->strClientCert.size());
	memset((void*)i_tAuthInfo->strPrivateKey.c_str(), 0, i_tAuthInfo->strPrivateKey.size());
	memset((void*)i_tAuthInfo->strPrivateKeyPasswd.c_str(), 0, i_tAuthInfo->strPrivateKeyPasswd.size());
	memset((void*)i_tAuthInfo->strPhase2Type.c_str(), 0, i_tAuthInfo->strPhase2Type.size());
}

void PthreadHandle::MsgHandle(ieee802_1x_Status Status)
{
	switch (Status)
	{
		case IEEE_802_1X_SUCCESS:
		{
			GBLog_Log(LL_INFO, "\n\n>>>>>>>>>>>>>auth success <<<<<<<<<<<<<<<<<<\n\n");
			m_s32SuccessNum++;
			break;
		}
		case IEEE_802_1X_FAIL:
		{
			GBLog_Log(LL_INFO, "\n\n>>>>>>>>>>>>>auth  fail <<<<<<<<<<<<<<<<<<<<<\n\n");
			m_s32FailNum++;
			break;
		}
		case IEEE_802_1X_ERROR_TIMEOUT:
		{
			GBLog_Log(LL_INFO, "\n\n>>>>>>>>>>>>>auth timeout <<<<<<<<<<<<<<<<<<<\n\n");
			m_s32TimeoutNum++;
			break;
		}
		default:
		{
			GBLog_Log(LL_INFO, "\n\n>>>>>>>>>>>>>>auth error! Status = %d <<<<<<<<<\n\n", Status);
			m_s32OtherNum++;
			break;
		}
	}
	
	--m_s32AllNum;

	m_s32Flg = 1;
	
	GBLog_Log(LL_INFO, "\n=======================================================\n"
			"SuccessNum=%d FailNum=%d TimeoutNum=%d OtherNum=%d RemainNum=%d\n"
			"============================================================\n",
			m_s32SuccessNum, m_s32FailNum, m_s32TimeoutNum, m_s32OtherNum, m_s32AllNum);
}

void *EventHandle(void * i_pArg) 
{
	PthreadHandle *pPoint = (PthreadHandle *)i_pArg;
	int err;

	while(1)
	{
		err = pPoint->m_objMsgQueue.MsgRecv( &(pPoint->m_tMsg) );
		if (err == -1)
		{
			GBLog_Log(LL_INFO, "messagte recv fail");
			pthread_exit((void *)-1);
		}
		
		if (MSG_MAGIC == pPoint->m_tMsg.mtype)
		{
			//停止定时器
			pPoint->m_objTimer.TimerStop();
			
			if (MSG_EXIT == pPoint->m_tMsg.mtext[0])
			{
				GBLog_Log(LL_INFO, "Recv Exit Message");
				break;		
			}
			else if (1 == pPoint->m_s32ConnectState)
			{
				//给一个函数来处理
				pPoint->MsgHandle( (ieee802_1x_Status)pPoint->m_tMsg.mtext[0] );
				pPoint->m_s32ConnectState = 0;
			}
			else
			{
				GBLog_Log(LL_INFO, "Discard It!");
			}
		}
	}
	return NULL;
}

//u32Num想要认证的次数
PthreadHandle::PthreadHandle(unsigned int u32Num)
{
	m_s32SuccessNum = 0;
	m_s32FailNum = 0;
	m_s32TimeoutNum = 0;
	m_s32OtherNum = 0;
	m_s32AllNum = u32Num;
	m_s32Flg = 0;
	m_s32ConnectState = 0;

	m_tMsg.mtype = 0;
	m_tMsg.mtext[0] = 0;
}

int PthreadHandle::Init(int i_s32Msec)
{
	int err;
	if ( m_objMsgQueue.QueueInit() != 0)
	{
		return -1;
	}
	
	if (m_objTimer.TimerInit(&m_objMsgQueue, i_s32Msec) != 0)
	{
		return -1;
	}
	
	err =  pthread_create(&m_PthId, NULL, EventHandle, this);
	if (err != 0)
	{
		GBLog_Log(LL_INFO, "pthread_creat fail:%s\n", strerror(errno));
		return -1;
	}
	return 0;
	
}

int PthreadHandle::Deinit()
{
	if (m_PthId > 0L)
	{
		m_objMsgQueue.MsgSend(MSG_EXIT);
		pthread_join(m_PthId, NULL);
		m_PthId = 0;
	}
	
	m_objTimer.TimerDeinit();

	if (m_objMsgQueue.m_s32QueueId != -1)
	{
		m_objMsgQueue.QueueDeinit();
		m_objMsgQueue.m_s32QueueId = -1;
	}
	
	return 0;
}

IEEE_802_1X_Verify::IEEE_802_1X_Verify(unsigned int i_u32Num, string i_strDir, string i_strLog4Conf):m_objRcvMsgThread(i_u32Num)
{
	m_strLog4Conf = i_strLog4Conf;
	m_str802_1XDir = i_strDir;
	if (m_str802_1XDir[m_str802_1XDir.size()-1] != '/')
	{
		m_str802_1XDir += "/";
	}

	m_strWPA_SERVER		= 	m_str802_1XDir + WPA_SERVER;
	m_strWPA_DAEMON 	=	m_str802_1XDir + WPA_DAEMON;
	m_strWPA_CONFIG_FILE =	m_str802_1XDir + WPA_CONFIG;
	m_strWPA_CTRL_FILE	=	m_str802_1XDir + WPA_CTRL_FILE;
	m_str802_1X_Log     =   m_str802_1XDir + _802_1X_LOG;
}

int IEEE_802_1X_Verify::WriteCfgFile(const T_AuthInfo *i_tAuthInfo)
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::WriteCfgFile enter");
	int err;
	string dest;
	char buf[BUF_SIZE] = {'\0'};

	dest = "ctrl_interface=";
	dest += m_str802_1XDir;
	dest += "wpa_supplicant\n";
	
	dest +=  "network={\n";
	dest +=  "key_mgmt=IEEE8021X\n";

	if (!i_tAuthInfo->strEapType.empty())
	{
		sprintf(buf, "eap=%s\n", i_tAuthInfo->strEapType.c_str());
		dest +=  buf;
	}
	if (!i_tAuthInfo->strUser.empty())
	{
		sprintf(buf, "identity=\"%s\"\n", i_tAuthInfo->strUser.c_str());
		dest += buf;
	}
	if (!i_tAuthInfo->strPasswd.empty())
	{
		sprintf(buf, "password=\"%s\"\n", i_tAuthInfo->strPasswd.c_str());
		dest +=  buf;
	}
	if (!i_tAuthInfo->strCaCert.empty())
	{
		sprintf(buf, "ca_cert=\"%s\"\n", i_tAuthInfo->strCaCert.c_str());
		dest +=  buf; 
	}
	if (!i_tAuthInfo->strClientCert.empty())
	{
		sprintf(buf, "client_cert=\"%s\"\n", i_tAuthInfo->strClientCert.c_str());
		dest +=  buf;
	}
	if (!i_tAuthInfo->strPrivateKey.empty())
	{
		sprintf(buf, "private_key=\"%s\"\n", i_tAuthInfo->strPrivateKey.c_str());
		dest += buf;
	}
	if (!i_tAuthInfo->strPrivateKeyPasswd.empty())
	{
		sprintf(buf, "private_key_passwd=\"%s\"\n", i_tAuthInfo->strPrivateKeyPasswd.c_str());
		dest += buf;
	}
	if (!i_tAuthInfo->strPhase2Type.empty())
	{
		sprintf(buf, "phase2=\"auth=%s\"\n", i_tAuthInfo->strPhase2Type.c_str());
		dest += buf;
	}
	
	dest += "}\n";
	GBLog_Log(LL_INFO, "%s", dest.c_str());
	
	m_pFileDescri = fopen(m_strWPA_CONFIG_FILE.c_str(), "w+");
	if (NULL == m_pFileDescri)
	{
		GBLog_Log(LL_INFO, "fail:%s\n", strerror(errno));
		return -1;	
	}

	err = fwrite(dest.c_str(), 1, dest.length(), m_pFileDescri);
	if(err != dest.length())
	{
		GBLog_Log(LL_INFO, "fail:%s\n", strerror(errno));
		return -1;
	}

	if (-1 == fclose(m_pFileDescri))
	{
		GBLog_Log(LL_INFO, "fail:%s\n", strerror(errno));
		return -1;
	}
	
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::WriteCfgFile exit");
	return 0;
	
}

void MsgCb(char * i_pMsg, size_t i_Len)
{
	GBLog_Log(LL_INFO, "MsgCB: %s", i_pMsg);
}

int  IEEE_802_1X_Verify::WpaCtrlCmd(string i_strCtrlCmd)
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::WpaCtrlCmd enter");
	int err = 0;
	char Reply[BUF_SIZE] = {0};
	
	if (m_pWpaCtrlFd == NULL)

	{
		m_pWpaCtrlFd = wpa_ctrl_open(m_strWPA_CTRL_FILE.c_str());
		if (m_pWpaCtrlFd == NULL)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_open  fail");
			return -1;
		}
	}

	size_t s32Size = BUF_SIZE -1;
	err = wpa_ctrl_request(m_pWpaCtrlFd, i_strCtrlCmd.c_str(), i_strCtrlCmd.length(), Reply, &s32Size,  MsgCb);
	
	do {
		if (err == -1)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_request fail");
			break;
		}
		
		else if (err == -2)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_request timeout");
			break;
		}
		
	} while(0);
		
	Reply[s32Size] = '\0';
	GBLog_Log(LL_INFO, "%s :ctrl response = %s", i_strCtrlCmd.c_str(), Reply);
	usleep(5 * 1000);
	
	return err;
}

int IEEE_802_1X_Verify::WPA_RunServer()
{
	GBLog_Log(LL_INFO, "WPA_RunServer() enter!");
	int err = 0;
	int count = 0;

	string cmd;	
	cmd = "-i eth0 -D wired -B -c ";
	cmd += m_strWPA_CONFIG_FILE;
	cmd += " -f ";
	cmd += m_str802_1X_Log; 
	
	//需要配置文件，才能拉的器服务程序
	IEEE802_1X_MD5_Method("none", "none");
	
	//服务程序不在，拉起服务程序
	if (m_objExecProgrem.ProgramIsExisting(m_str802_1XDir, WPA_SERVER) == false)	
	{
		//执行服务程序
		//err = 
		m_objExecProgrem.RunProgram(m_strWPA_SERVER, cmd, 1);
		if (m_objExecProgrem.ProgramIsExisting(m_str802_1XDir, WPA_SERVER) == false)
		{
			GBLog_Log(LL_INFO, "Startup  WPA_SERVER Fail");
			return -1;
		}
		else
		{
			GBLog_Log(LL_INFO, "Startup  WPA_SERVER Success");
		}
	}	

	while (!FileIsExist(m_strWPA_CONFIG_FILE.c_str()))
	{
		usleep(10 * 1000);
		if(count++ > 20)
		{
			err = -1;
			break;
		}
	}
	
	if (Disconnect() != 0)
		err = -1;
	
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::WPA_RunServer exit");
	return err;
}

//守护进程
int IEEE_802_1X_Verify::WPA_RunDaemon()
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::WPA_RunDaemon() enter");
	int err = 0;

	string strProgram = m_str802_1XDir + WPA_DAEMON;
	
	string strParam;
	strParam = m_strLog4Conf + " ";
	strParam += m_str802_1XDir;
	strParam += " ";
	strParam += WPA_SERVER;
	strParam += " ";
	strParam += m_strWPA_CONFIG_FILE;
	strParam += " ";
	strParam += m_str802_1X_Log; 
	
	if (m_objExecProgrem.ProgramIsExisting(m_str802_1XDir, WPA_DAEMON) == false)
	{
		m_objExecProgrem.RunProgram(strProgram, strParam, 0);
		//没拉起守护进程
		if (m_objExecProgrem.ProgramIsExisting(m_str802_1XDir, WPA_DAEMON) == false)
		{
			GBLog_Log(LL_INFO, "Startup Daemon Fail");
			err = -1;
		}
		else
			GBLog_Log(LL_INFO, "Startup Daemon Success");
	}

	return err;	
}

int IEEE_802_1X_Verify::Init(int i_s32Msec)
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Init enter");

	m_pWpaCtrlFd = NULL;
	m_pFileDescri = NULL;

	if (m_objRcvMsgThread.Init(i_s32Msec) != 0)
	{
		return -1;
	}

	/*
	if (WPA_RunServer() != 0)
	{
		return -1;
	}

	WPA_RunDaemon();
	if (WPA_RunDaemon() != 0)
	{
		return -1;
	}
	*/
	return 0;
}

int IEEE_802_1X_Verify::Connect()
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Connect enter");
	if (WpaCtrlCmd("RECONFIGURE"))
	{
		GBLog_Log(LL_INFO, "CtrlCmd: RECONFIGURE Fail");
		return -1;
	}

	if (WpaCtrlCmd("LOGON"))
	{
		GBLog_Log(LL_INFO, "CtrlCmd: LOGON Fail");
		return -1;
	}
	if (WpaCtrlCmd("RECONNECT"))
	{
		GBLog_Log(LL_INFO, "CtrlCmd: RECONNECT Fail");
		return -1;
	}
	
	m_objRcvMsgThread.m_s32ConnectState = 1;
	
	m_objRcvMsgThread.m_objTimer.TimerStart(m_strWPA_CTRL_FILE); 
	
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Connect exit");
	return 0;
}

int IEEE_802_1X_Verify::Disconnect()
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Disconnect enter");
	if (WpaCtrlCmd("LOGOFF"))
	{
		GBLog_Log(LL_INFO, "CtrlCmd: LOGOFF Fail");
		return -1;
	}

	if (WpaCtrlCmd("DISCONNECT"))
	{
		GBLog_Log(LL_INFO, "CtrlCmd: DISCONNECT Fail");
		return -1;
	}
	
	m_objRcvMsgThread.m_s32ConnectState = 0;
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Disconnect exit");
	return 0;
}

int IEEE_802_1X_Verify::IEEE802_1X_MD5_Method(std::string i_strUser, std::string i_strPasswd)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = i_strUser;
	tUserAuthInfo.strPasswd = i_strPasswd;
	tUserAuthInfo.strEapType = "MD5";
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}

int IEEE_802_1X_Verify::IEEE802_1X_GTC_Method(std::string i_strUser, std::string i_strPasswd)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = i_strUser;
	tUserAuthInfo.strPasswd = i_strPasswd;
	tUserAuthInfo.strEapType = "GTC";
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}

int IEEE_802_1X_Verify::IEEE802_1X_TLS_Method(std::string i_strCaCert, std::string i_strClientCert,
													std::string i_strPrivateKey, std::string i_strPrivateKeyPasswd)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = "none";
	tUserAuthInfo.strPasswd = "none";
	tUserAuthInfo.strEapType = "TLS";
	tUserAuthInfo.strCaCert = i_strCaCert;
	tUserAuthInfo.strClientCert	= i_strClientCert;
	tUserAuthInfo.strPrivateKey	= i_strPrivateKey;
	tUserAuthInfo.strPrivateKeyPasswd =	i_strPrivateKeyPasswd;
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}

int IEEE_802_1X_Verify::IEEE802_1X_TTLS_Method(std::string i_strUser, std::string i_strPasswd, 
													std::string i_strCaCert)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = i_strUser;
	tUserAuthInfo.strPasswd = i_strPasswd;
	tUserAuthInfo.strEapType = "TTLS";
	tUserAuthInfo.strCaCert = i_strCaCert;
	tUserAuthInfo.strPhase2Type	= "PAP";
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}
							
int IEEE_802_1X_Verify::IEEE802_1X_PEAP_CHAPV2_Method(std::string i_strUser, std::string i_strPasswd, 
																std::string i_strCaCert)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = i_strUser;
	tUserAuthInfo.strPasswd = i_strPasswd;
	tUserAuthInfo.strEapType = "PEAP";
	tUserAuthInfo.strCaCert = i_strCaCert;
	tUserAuthInfo.strPhase2Type	= "MSCHAPV2";	
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}

int IEEE_802_1X_Verify::IEEE802_1X_PEAP_GTC_Method(std::string i_strUser, std::string i_strPasswd, 
															std::string i_strCaCert)
{
	T_AuthInfo tUserAuthInfo;
	ClrAuthInfo(&tUserAuthInfo);
	tUserAuthInfo.strUser = i_strUser;
	tUserAuthInfo.strPasswd = i_strPasswd;
	tUserAuthInfo.strEapType = "PEAP";
	tUserAuthInfo.strCaCert = i_strCaCert;
	tUserAuthInfo.strPhase2Type	= "GTC";
	if (WriteCfgFile(&tUserAuthInfo) != 0)
	{
		return -1;
	}
	return 0;
}

int IEEE_802_1X_Verify::Deinit()
{
	GBLog_Log(LL_INFO, "IEEE_802_1X_Verify::Deinit");
	if (m_objRcvMsgThread.Deinit() != 0)
	{
		GBLog_Log(LL_INFO, "m_objRcvMsgThread.Deinit fail");
	}
		

	if (m_pWpaCtrlFd != NULL)
	{
		wpa_ctrl_close(m_pWpaCtrlFd);
	}
		
	return 0;
}


