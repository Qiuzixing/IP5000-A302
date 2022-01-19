#include "global.h"

#define I_WEB_PIPE          "/tmp/overlay_f"

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
    while(flock(fileno(m_fp), LOCK_EX) == -1)
    {
        usleep(100 * 1000);
        continue;
    }

    qDebug("LOCK FINISHED");

}

void CFileMutex::UnLock()
{
    int nret = -1;

    nret = flock(fileno(m_fp), LOCK_UN);
    qDebug() << "###UnLock::nrest###" << nret;
    if(nret == 0)
    {

    }
}
