#include "global.h"

int FileLock(const char *i_pFile)
{
    string strTmp = i_pFile;
    string::size_type site = strTmp.rfind(".");
    if(site == string::npos)
    {
        return -1;
    }

    string strFile = strTmp.substr(0, site);
    strFile += ".lck";

    FILE *fp        = NULL;
        fp = ::fopen(strFile.c_str(),"a");
    if(fp == NULL)
    {
        qDebug("FileLock fopen failed!");
        return -1;
    }

    int nret = ::flock(fileno(fp), LOCK_EX);
    qDebug() << "nret:" << nret;

    ::fclose(fp);
    if(nret == 0)
    {
        return 0;
    }
    else
    {
        ::remove(strFile.c_str());
        qDebug("FileLock failed!");
        return -1;
    }
}

int FileUnLock(const char *i_pFile)
{
    string strTmp = i_pFile;
    string::size_type site = strTmp.rfind(".");
    if(site == string::npos)
    {
        return -1;
    }

    string strFile = strTmp.substr(0, site);
    strFile += ".lck";

    FILE *fp        = NULL;
        fp = ::fopen(strFile.c_str(),"a");
    if(fp == NULL)
    {
        qDebug("FileUnLock fopen failed!");
        return -1;
    }

    int nret = ::flock(fileno(fp), LOCK_UN);
    qDebug() << "nret:" << nret;
    ::fclose(fp);

    //::remove(strFile.c_str());

    if(nret == 0)
    {
        return 0;
    }
    else
    {
        qDebug("FileUnLock failed!");
        return -1;
    }
}

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
        qDebug("Init fopen failed!");
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
