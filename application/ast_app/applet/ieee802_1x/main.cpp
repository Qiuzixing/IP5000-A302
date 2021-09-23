
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <string>
#include "IEEE_802_1x_Verify.h"
#include "log.h"

using namespace std;

typedef struct {
	string strUser;
	string strPasswd;
	string strCACert;
	string strClientCert;
	string strClientKey;
	string strLog4Conf;
	string str802_1X_Dir;
	string strAuthMethod;
}T_ConfInfo;


void help(const char* i_pName)
{
    printf("%s [options]\n", i_pName);
    printf("default: [%s -u admin -p admin -c /802_1x/certs/ca.pem -l v/certs/client.pem -k /802_1x/certs/client.key"\
			"-d /802_1x  -m md5", i_pName);
    printf("    -h --help               show this help\n");
    printf("    -u --use                authentication usename\n");
    printf("    -p --password           authentication password\n");
    printf("    -c --ca cert            ca certificate\n");
    printf("    -t --client cert        client certificate\n");
    printf("    -k --private key        client private key\n");
	printf("    -d --directory          802_1x directory\n");
	printf("    -m --method             choose which auth method\n");
}

void conf_auth_info(int argc, char **argv, T_ConfInfo *ConfInfo)
{
    static struct option longOpts[] =
    {
        { "help",       	no_argument, NULL, 'h' },
        { "use",			required_argument, 	NULL, 	'u' },
		{ "password",		required_argument, 	NULL, 	'p' },
		{ "ca_cert",		required_argument, 	NULL, 	'c' },
		{ "client_certs",	required_argument, 	NULL, 	't' },
		{ "client_key",		required_argument, 	NULL, 	'k' },
		{ "directory",		required_argument, 	NULL, 	'd' },
		{ "method",			required_argument, 	NULL, 	'm' },
        { NULL, 0, NULL, 0 }
    };
		
    while(1)
    {
        int c;
        int index = 0;

        c = getopt_long(argc, argv, "hu:p:c:t:k:l:d:m:", longOpts, &index);
        if (c == -1)
            break;

        switch (c){
            case 'h':
                help(argv[0]);
                exit(-1);
            case 'u':
				ConfInfo->strUser= optarg; // = optarg .append(optarg)
                break;
			case 'p':
				ConfInfo->strPasswd= optarg;
				break;
			case 'c':
				ConfInfo->strCACert= optarg;
				break;
			case 't':
				ConfInfo->strClientCert= optarg;
				break;
			case 'k':
				ConfInfo->strClientKey= optarg;
				break;
			case 'd':
				ConfInfo->str802_1X_Dir= optarg;
				break;
			case 'm':
				ConfInfo->strAuthMethod= optarg;
				break;
            default:
				help(argv[0]);
                exit(-1);
			
        }
    }
	
	printf( "-u %s\n", ConfInfo->strUser.c_str());
    printf( "-p %s\n", ConfInfo->strPasswd.c_str());
	printf( "-c %s\n", ConfInfo->strCACert .c_str());
	printf( "-t %s\n", ConfInfo->strClientCert.c_str());
	printf( "-k %s\n", ConfInfo->strClientKey.c_str());
	printf( "-l %s\n", ConfInfo->strLog4Conf.c_str());
	printf( "-d %s\n", ConfInfo->str802_1X_Dir.c_str());
	printf( "-m %s\n", ConfInfo->strAuthMethod.c_str());
 
}

int ChooseMethod(IEEE_802_1X_Verify *i_objIEEE_802_1X, T_ConfInfo * ConfInfo)
{
	int err = -1;
	do {
		//1.md5方式
		if (ConfInfo->strAuthMethod == "md5")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_MD5_Method(ConfInfo->strUser, ConfInfo->strPasswd);
			break;
		}
		
		//2.gtc方式
		if (ConfInfo->strAuthMethod == "gtc")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_GTC_Method(ConfInfo->strUser, ConfInfo->strPasswd);
			break;
		}
		
		//3.tls方式
		if (ConfInfo->strAuthMethod == "tls")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_TLS_Method(ConfInfo->strCACert,
															ConfInfo->strClientCert, 
															ConfInfo->strClientKey,
															"whatever");
			break;
		}
		
		//4.ttls方式
		if (ConfInfo->strAuthMethod == "ttls")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_TTLS_Method(ConfInfo->strUser,
															ConfInfo->strPasswd,
															ConfInfo->strCACert);
			break;
		}
		
		//5.mschapv2方式
		if (ConfInfo->strAuthMethod == "peap_mschapv2")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_PEAP_CHAPV2_Method(ConfInfo->strUser, 
																	ConfInfo->strPasswd,
																	ConfInfo->strCACert);
			break;
		}
		
		//6.peap_gtc方式
		if (ConfInfo->strAuthMethod == "peap_gtc")
		{
			err = i_objIEEE_802_1X->IEEE802_1X_PEAP_GTC_Method(ConfInfo->strUser, 
																ConfInfo->strPasswd,
																ConfInfo->strCACert);
			break;
		}

		GBLog_Log(LL_INFO, ">>>>>>>>>>>>>>>>>>>No this Auth Method<<<<<<<<<<<<<<");
		
	}while(0);
	
	return err;
}


int main(int argc, char **argv)
{
	T_ConfInfo ConfInfo;
	conf_auth_info(argc, argv, &ConfInfo);
		
	IEEE_802_1X_Verify IEEE_802_1X(100, ConfInfo.str802_1X_Dir, ConfInfo.strLog4Conf);

	do {
		//1.初始化，定时器为60秒，60秒后即为超时。
		if (IEEE_802_1X.Init(1 * 1000) != 0)
		{
			GBLog_Log(LL_INFO, "IEEE_802_1X.Init fail");
			break;
		}

		//2.选择认证方式。
		if (ChooseMethod(&IEEE_802_1X, &ConfInfo) != 0)
		{
			GBLog_Log(LL_INFO, "ChooseMethod fail");
			break;
		}

		//3.开始认证
		if (IEEE_802_1X.Connect() != 0)
		{
			GBLog_Log(LL_INFO, "Connect fail");
			break;
		}
		else
		{
			GBLog_Log(LL_INFO, "Connect OK");
			break;
		}

		// //4.多次认证，测试稳定性,
		// while(1)
		// {
		// 	usleep(1 *1000);
		// 	if (IEEE_802_1X.m_objRcvMsgThread.m_s32AllNum != 0)	//m_s32AllNum为认证总次数，主要是检验程序的稳定性，实际认证设置为1就好了
		// 	{
		// 		if (IEEE_802_1X.m_objRcvMsgThread.m_s32Flg == 1) //表示上一次认证已经完成，可以开始下一轮认证。
		// 		{
		// 			IEEE_802_1X.Disconnect();
		// 			IEEE_802_1X.Connect();
		// 			IEEE_802_1X.m_objRcvMsgThread.m_s32Flg = 0;
		// 		}
		// 	}
		// 	else
		// 	{

		// 		break;
		// 	}
		// }

	}while (0);

	IEEE_802_1X.Deinit();
	GBLog_Log(LL_INFO, "main exit!");
		
	return 0;
	
}

