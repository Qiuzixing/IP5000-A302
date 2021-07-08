#include <log4cpp/log4cpp.hh>

int main()
{
	GBLog_Init("sample_log.conf");
	GBLog_Log(LL_DEBUG, "xxxxxxxxx %s %d", "test1", 100);
	GBLog_Log(LL_INFO, "yyyyyyyyy %s %d", "test2", 200);
	GBLog_Shutdown();
	return 0;
}

