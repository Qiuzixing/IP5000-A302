#include "operation.h"
#include "types.h"
#include "debug.h"
#include <stdlib.h>
#include <cstring>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

CMutex COperation::s_DeviceMutex;

int COperation::FileToString(const char *i_pinFilePath, string &o_strDes)
{
    FILE *fp    = NULL;
    o_strDes   = "";
    fp = fopen(i_pinFilePath, "r");
    if(fp==NULL)
    {
        return -1;
    }
    char sTmp[1024]     = {0};
    int nRet            = 0;
    while(1)
    {

        nRet = fread(sTmp,1,sizeof(sTmp),fp);
        if(nRet<= 0)
        {
            break;
        }
        o_strDes.append(sTmp,nRet);
    }
    fclose(fp);
    return o_strDes.length();
}

int COperation::StringToFile(const char *i_pStrBuff, const char *i_pOutFileath)
{
    FILE *fp	= NULL;
	fp = fopen(i_pOutFileath,"w+");
	if(fp==NULL)
	{
		return -1;
	}
	SInt32 nRet = fwrite(i_pStrBuff,1,strlen(i_pStrBuff),fp);
	fclose(fp);
	chmod(i_pOutFileath, 0664);

	return 	nRet ;
}

bool  COperation::FileRename(const char *i_pFileName, const char *i_pFilePath, string &o_strRename)
{
    bool bFlag = false;
    string strSuffix = "[0]";
    string strTmp = i_pFileName;
    string strFilename = "";

    // 获取要判断的文件名带路径
    int n;
    for(n = strTmp.size(); n >= 0; n--)
    {
        if(strTmp[n] == '/')
        {
            strFilename = i_pFilePath + strTmp.substr(n);
            break;
        }

        if(n == 0)
        {
            strFilename = i_pFilePath + strTmp;
        }
    }

    // 获取文件名'.'的位置
    int nSuffixSite = strFilename.find(".");
    if(nSuffixSite == string::npos)
    {
        nSuffixSite = strFilename.size();
    }

    //sprintf(aAbsolutePath, "%s/%s", i_pFileName, i_pFilePath);
    BC_INFO_LOG("FileRename aAbsolutePath is <%s>", strFilename.c_str());

    // 如果文件存在，修改文件名
    while(1)
    {
        if(access(strFilename.c_str(), F_OK) == 0)
        {
            if(!bFlag)
            {
                strFilename.insert(nSuffixSite, strSuffix);
                bFlag = true;
            }

            strFilename.at(nSuffixSite+1) += 1;

            if(strFilename.at(nSuffixSite+1) > '9')
            {
                return false;
            }
        }
        else
        {
            break;
        }
    }
    o_strRename = strFilename;

    return true;
}


bool COperation::SetWebSecurityMode(const char *i_pMode)
{
    string strTmp = i_pMode;
    char szCmdStr[1024] = {0};

    snprintf(szCmdStr, sizeof(szCmdStr)-1, "./restart.sh web");
    printf("------------%s\n", szCmdStr);
    int ret = system(szCmdStr);
    if(ret < 0)
    {
        return false;
    }

    if(UpdateWebToConfigfile(WEB_CONFIG_FILE, "StartMode", strTmp))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool COperation::SetCertificate(const char *i_pCert)
{
    string strTmp = i_pCert;
    char szCmdStr[1024] = {0};

    snprintf(szCmdStr, sizeof(szCmdStr)-1, "./restart.sh web");
    printf("------------%s\n", szCmdStr);
    int ret = system(szCmdStr);
    if(ret < 0)
    {
        return false;
    }

    if(UpdateWebToConfigfile(WEB_CONFIG_FILE, "CERM", strTmp))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool COperation::UpdateEdidFile(const char *i_pFilePath)
{
    char aCmdStr[MAX_CMD_STR] = {0};

    string strRename = "";
	if(COperation::FileRename(i_pFilePath, DEFAULT_FILE_PATH "/edid", strRename))
    {
        BC_INFO_LOG("COperation::FileRename name is <%s>", strRename.c_str());

        // 将文件备份到路径下，并替换custom_edid.bin文件
        snprintf(aCmdStr, sizeof(aCmdStr) - 1, "cp -vf %s %s", i_pFilePath, strRename.c_str());
        int ret = system(aCmdStr);
        BC_INFO_LOG("UpdateEdidFile backup cmd is <%s>", aCmdStr);
        if(ret < 0)
        {
            BC_INFO_LOG("UpdateEdidFile backup failed");
            return false;
        }

        snprintf(aCmdStr, sizeof(aCmdStr) - 1, "mv -vf %s %s", i_pFilePath, DEFAULT_FILE_PATH "edid/custom_edid.bin");
        ret = system(aCmdStr);
        BC_INFO_LOG("UpdateEdidFile setedid cmd is <%s>", aCmdStr);
        if(ret < 0)
        {
            BC_INFO_LOG("UpdateEdidFile backup failed");
            return false;
        }
    }

    return true;
}

bool COperation::SetJsonFile(const char *i_pJsonData, const char *i_pFile)
{
    CMutexLocker locker(&s_DeviceMutex);
    int ret = StringToFile(i_pJsonData, i_pFile);
    if(ret < 0)
    {
        return false;
    }

    return true;
}

bool COperation::GetJsonFile(const char *i_pJsonFile, string& o_strContent)
{
    CMutexLocker locker(&s_DeviceMutex);
    int ret = FileToString(i_pJsonFile, o_strContent);
    if(ret < 0)
    {
        return false;
    }

    return true;
}

