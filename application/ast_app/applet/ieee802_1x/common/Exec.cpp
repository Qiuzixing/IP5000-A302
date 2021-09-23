
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h> 

#include "Exec.h"
#include "log.h"

#define SIZE_256			256

using namespace std;

bool Exec::ProgramIsExisting(string i_strPath, string i_strName)
{
	GBLog_Log(LL_INFO, "Exec::ProgramIsExisting enter");
	int err = false;
	FILE *Fd;
	string Cmd;
	string Text;
	char buf[SIZE_256] = {'\0', };
	
	Cmd = "ps -ef | grep ";
	Cmd	+= i_strName;
	Cmd += " | grep -v grep";

	Fd = popen(Cmd.c_str(), "r");
	if (Fd == NULL)
	{
		GBLog_Log(LL_INFO, "popen fail");
		return false;
	}
	while(!feof(Fd))
	{
		fgets(buf, SIZE_256, Fd);
		Text = buf;
		if(Text.find(i_strName.c_str()) != -1)
		{
			err = true;
			break;
		}
	}
	pclose(Fd);
	
	if (!err)
	{
		GBLog_Log(LL_INFO, "%s no exist", i_strName.c_str());
	}
	
	return err;
}


int Exec::RunProgram(string i_strProgrom, string i_strParam, int i_s32WaitFlg)
{
	GBLog_Log(LL_INFO, "Exec::RunProgram %s enter", i_strProgrom.c_str());
	pid_t m_Pid;
	m_Pid = fork();
	if (m_Pid < 0)
	{
		GBLog_Log(LL_INFO, "fork fail:%s", strerror(errno));
		return -1;
	}

	if (m_Pid == 0)
	{
		int i = 0;
		char *p = NULL;
		char *argv[20];
		char *Progrom = strdup(i_strProgrom.c_str());
		char *buf = strdup(i_strParam.c_str());
		
		argv[0] = Progrom;
		i = 1;
		p = strtok(buf, " ");
		while (p)
		{
			argv[i] = p;
			i++;
			p = strtok(NULL, " ");
		}
		argv[i] = NULL;
		
		if (execv(Progrom, argv))
		{
			GBLog_Log(LL_INFO,"stratup %s error: %s", Progrom, strerror(errno));
			//printf("stratup %s error: %s", Progrom, strerror(errno));
		}
		
		free(buf);
		free(Progrom);
		exit(0);
	}

	if (i_s32WaitFlg)
	{
		waitpid(m_Pid, NULL, 0);
	}
	return 0;
}

