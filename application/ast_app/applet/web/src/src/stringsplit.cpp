/****************************************************************************
* 版权信息：（C）2013，深圳市广平正科技有限责任公司版权所有
* 系统名称：
* 文件名称：stringsplit.cpp
* 文件说明：
* 作    者：zhangem
* 版本信息：V1.0
* 设计日期：2015-1-22
* 修改记录：
* 日    期		版    本		修改人 		修改摘要
****************************************************************************/
/**************************** 条件编译选项和头文件 ****************************/
#include "stringsplit.h"

/********************************** 宏、常量 **********************************/

/********************************** 数据类型 **********************************/

/************************************ 变量 ************************************/

/********************************** 函数实现 **********************************/
string DeleteChar(const string& i_strOrig, char i_chToDelete)
{
    char* pDest = new char[i_strOrig.length()+1];
    int nPosInDest = 0;
    for (int i = 0; i < i_strOrig.length(); i++)
    {
        if (i_strOrig[i] != i_chToDelete)
        {
            pDest[nPosInDest++] = i_strOrig[i];
        }
    }
    pDest[nPosInDest] = '\0';

    string strDest = pDest;

    delete []pDest;

    return strDest;
}

string ReplaceStr(string& i_strOrig, const string& i_strOldValue,const string& i_strNewValue)
{
    string strOrig = i_strOrig;
    for(string::size_type nPos = 0;   nPos!=string::npos; nPos+=i_strNewValue.length())
    {
        if ((nPos=strOrig.find(i_strOldValue,nPos))!=string::npos)
        {
            strOrig.replace(nPos,i_strOldValue.length(),i_strNewValue);
        }
        else
        {
            break;
        }
    }
    return strOrig;
}


/**********************************  类实现 **********************************/
CStringSpliter::CStringSpliter(const string& i_strOrigStr)
:
m_strOrig(i_strOrigStr)
{
}

CStringSpliter::~CStringSpliter()
{
}

void CStringSpliter::Split(const string& i_strSplitStr)
{
    int nStartPos = 0;
    string strSubStr = "";
    int nPos = 0;
    do
    {
        nPos = m_strOrig.find(i_strSplitStr, nStartPos);
        if (string::npos == nPos)
        {
            strSubStr = m_strOrig.substr(nStartPos);
        }
        else
        {
            strSubStr = m_strOrig.substr(nStartPos, nPos-nStartPos);
            nStartPos = nPos + i_strSplitStr.length();
        }

        if (!strSubStr.empty())
        {
            m_vecSplitStr.push_back(strSubStr);
        }
    }while(string::npos != nPos);
}

int CStringSpliter::Size()
{
    return m_vecSplitStr.size();
}


const string& CStringSpliter::operator[](int i_nPos) const
{
    return m_vecSplitStr[i_nPos];
}

string& CStringSpliter::operator[](int i_nPos)
{
    return m_vecSplitStr[i_nPos];
}

vector<string> CStringSpliter::GetVec()
{
    return m_vecSplitStr;
}




