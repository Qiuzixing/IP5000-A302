#include "operation.h"
#include "types.h"
#include "debug.h"
#include <stdlib.h>
#include <cstring>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/file.h>
#include <unistd.h>
#include <crypt.h>


CMutex COperation::s_DeviceMutex;

CFileMutex::CFileMutex(const char *i_pFile)
:
m_file(i_pFile),
m_fp(NULL),
m_bisLock(false)
{}

CFileMutex::~CFileMutex()
{
    if(m_fp != NULL)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

int CFileMutex::Init()
{
    string::size_type site = m_file.rfind(".");
    if(site == string::npos)
    {
        return -1;
    }

    string strFile = m_file.substr(0, site);
    strFile += ".lck";

    m_fp = fopen(strFile.c_str(),"a");
    if(m_fp == NULL)
    {
        BC_ERROR_LOG("Init fopen failed!");
        return -1;
    }

    return 0;
}


void CFileMutex::Lock()
{
    int nret = -1;
    while(!m_bisLock)
    {
        nret = flock(fileno(m_fp), LOCK_EX);
        if(nret == 0)
        {
            m_bisLock = true;
        }
    }
}

void CFileMutex::UnLock()
{
    int nret = -1;
    while(m_bisLock)
    {
        nret = flock(fileno(m_fp), LOCK_UN);
        if(nret == 0)
        {
            m_bisLock = false;
        }
    }

    fclose(m_fp);
    m_fp = NULL;
}

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
    fflush(fp);
    fsync(fileno(fp));

	fclose(fp);
	chmod(i_pOutFileath, 0664);

	return 	nRet ;
}

int COperation::AssertDirExists(const char *i_pDir)
{
    if(access(i_pDir, 0) == -1)
    {
        char szCmd[128] = {0};
        sprintf(szCmd, "mkdir -p %s", i_pDir);
        return My_System(szCmd);
    }

    return 0;
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
    if(UpdateWebToConfigfile(DEFAULT_WEB_CONFIG_FILE, "StartMode", strTmp))
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
    if(UpdateWebToConfigfile(DEFAULT_WEB_CONFIG_FILE, "CERM", strTmp))
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
    CFileMutex fmutex(i_pFile);
    if(fmutex.Init() != 0)
    {
        return false;
    }
    fmutex.Lock();

    int ret = StringToFile(i_pJsonData, i_pFile);
    if(ret < 0)
    {
        return false;
    }

    fmutex.UnLock();
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

bool COperation::SetPassword(const char *i_pNewPasswd)
{
    CMutexLocker locker(&s_DeviceMutex);

    char szCmdStr[MAX_CMD_STR] = {0};
    snprintf(szCmdStr, sizeof(szCmdStr) - 1, "echo \"admin:%s\" | chpasswd -m", i_pNewPasswd);
    if(0 == My_System(szCmdStr))
    {
        SavePasswordToParam("admin");
        return true;
    }
    else
    {
        return false;
    }
}

int COperation::SavePasswordToParam(const char *i_pUsrName)
{
    int rst = 0;
    FILE* pf = fopen("/etc/passwd", "r");
    if (NULL == pf)
    {
        rst = -1;
        //goto out;
        return rst;
    }
    char line[128];
    ::memset(line, 0, sizeof(line));
    char* user;
    char* salt;
    while (NULL != fgets(line, sizeof(line), pf))
    {
        char *save_ptr;
        char *token;
        //token ��ʾ���� ":" ǰ���һ�Σ�save_ptr ָ�� ":" �����һ��
        token = strtok_r(line, ":", &save_ptr);
        if (!token)
            break;
		//ƥ�� i_pUsrName
        if (0 != ::strcmp(i_pUsrName, token))
            continue;

        BC_INFO_LOG("i_pUsrName  : %s",token);
		//������NULL���Զ���ʼ��һ�Σ�token ����������
        token = strtok_r(NULL, ":", &save_ptr);
        if (!token)
            break;
		salt = token;
		break;
    }
    if (NULL == salt)
    {
        rst = -1;
        //goto out;
        fclose(pf);
        pf = NULL;
        return rst;
    }
	int argc = 4;
	char* argv[4] = {(char *)"astparam", (char *)"s", (char *)"passwd", salt};
	//�������������Ҫ����Щ�ļ� make ����
	extern char* astparam(int argc, char** argv);
	(void)astparam(argc, argv);
	argc = 2;
	argv[1] = (char *)"save";
	(void)astparam(argc, argv);

//out:
    if (NULL != pf)
    {
        fclose(pf);
        pf = NULL;
    }
    return rst;
}

bool COperation::VerifyPassword(char *i_pUsername, char *i_pPasswd)
{
    CMutexLocker locker(&s_DeviceMutex);

    if(0 == JudgePasswdEncrypt(i_pUsername, i_pPasswd))
    {
        return true;
    }
    else
    {
        return false;
    }
}

int COperation::JudgePasswdEncrypt(char *username, char *passwd)
{
    FILE* pf = fopen("/etc/passwd", "r");
    if (NULL == pf)
    {
        return -1;
    }
    char line[64];
    memset(line, 0, sizeof(line));
    char* user;
    char* salt;
    while (NULL != ::fgets(line, sizeof(line), pf))
    {
        char *save_ptr;
        char *token;
        token = strtok_r(line, ":", &save_ptr);
        if (!token)
            break;

        if (0 != strcmp(username, token))
            continue;
		//printf("%s : ",token);
        token = strtok_r(NULL, ":", &save_ptr);
        if (!token)
            break;

        salt = token;
        //printf("%s \n",salt);
    }
    if (NULL == salt)
    {
    	//printf("salt = NULL \n");
        fclose(pf);
        pf = NULL;
        return -1;
    }
    fclose(pf);
    pf = NULL;
    if(NULL == salt)
    	BC_INFO_LOG("salt is null\n");
    else
    	BC_INFO_LOG("slat:%s\n", salt);


    struct crypt_data data;
    memset(&data, 0, sizeof(data));

    char * encrypted_passwd = NULL;
    encrypted_passwd = crypt_r(passwd, salt, &data);
    if(NULL == encrypted_passwd)
    {
    	BC_INFO_LOG("encrypted_passwd is null\n");
    	return -1;
    }
    else
    {
    	BC_INFO_LOG("encrypted_passwd is :%s\n", encrypted_passwd);
    }
    return strncmp(encrypted_passwd, salt,::strlen(encrypted_passwd)>::strlen(salt)?::strlen(encrypted_passwd): ::strlen(salt));
}


