
#include <unistd.h>
#include <stdlib.h>

#include "Exec.h"
#include "log.h"

using namespace std;

#define SLEEP_TIME1      60
#define SLEEP_TIME2      15


#define WPA_SERVER 			"wpa_supplicant_802_1x"
#define WPA_CONFIG 			"wpa_supplicant_802_1x.conf"

int InitDaemon()
{
	int err;
	pid_t Pid;
	Pid = fork();
	if (Pid < 0)
	{
		GBLog_Log(LL_INFO, "fork fail");
		return -1;
	}
	if (Pid > 0)
	{
		exit(0);
	}
	setsid();
	chdir("/");
	return 0;
}

int main(int argc, char **argv)
{
	int i = 0;
	int ret = 0;
	for (i = 1; i < argc; i++)
	{
		GBLog_Log(LL_INFO, "argv[%d]=%s\n", i, argv[i]);
	}
	
	string strLog4Conf   = argv[1];
	string str802_1xDir  = argv[2];
	string strWPA_SERVER = argv[3];
	string strWPAConf    = argv[4];
	string str802Log     = argv[5];
		

	string strParam;
	strParam = "-i eth0 -D wired -B -c ";
	strParam += strWPAConf;
	strParam += " -f";
	strParam += str802Log;
	
	GBLog_Log(LL_INFO, "WPA_Daemon enter");

	if (InitDaemon() != 0)
	{
		return -1;
	}

	while(1)
	{
		sleep(SLEEP_TIME1);
		if (!Exec::ProgramIsExisting(str802_1xDir, strWPA_SERVER))
		{
			sleep(SLEEP_TIME2);
			if (!Exec::ProgramIsExisting(str802_1xDir, strWPA_SERVER))
			{
				GBLog_Log(LL_INFO, "WPA_SERVER not exist");
				Exec::RunProgram(str802_1xDir + strWPA_SERVER, strParam, 1);
				
				if (!Exec::ProgramIsExisting(str802_1xDir, strWPA_SERVER))
				{
					GBLog_Log(LL_INFO, "Daemon Startup WPA_SERVER Fail");
					ret = -1;
					break;
				}
			}
		}
	}

	return ret;
	
}




