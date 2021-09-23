
#ifndef __EXEC_H__
#define __EXEC_H__

#include <string>
class Exec
{
public:
	
	static int RunProgram(std::string i_strProgrom, std::string i_strParam, int i_s32WaitFlg);
	static bool ProgramIsExisting(std::string i_strPath, std::string i_strName);
private:
	
};

#endif

