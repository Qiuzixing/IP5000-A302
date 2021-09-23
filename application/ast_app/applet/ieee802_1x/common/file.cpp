
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#include "file.h"
#include "log.h"

bool FileIsExist(const char *file_path)
{
	bool ret = true;
	do {
		if (file_path == NULL)
		{
			GBLog_Log(LL_INFO, "bad param");
			ret = false;
			break;
		}
		
		if (access(file_path, F_OK))
		{
			GBLog_Log(LL_INFO, "%s not exit", file_path);
			ret = false;
			break;
		}
		
	}while(0);

	return ret;
}


