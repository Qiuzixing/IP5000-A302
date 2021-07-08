/****************************************************************************
* 版权信息：（C）2013，深圳市广平正科技有限责任公司版权所有
* 系统名称：
* 文件名称：stringsplit.h
* 文件说明：
* 作    者：zhangem
* 版本信息：V1.0
* 设计日期：2015-1-22
* 修改记录：
* 日    期		版    本		修改人 		修改摘要
****************************************************************************/
/**************************** 条件编译选项和头文件 ****************************/
#ifndef  __STRINGSPLIT_H__
#define  __STRINGSPLIT_H__

#include <string>
#include <vector>

using namespace std;
/********************************** 常量和宏 **********************************/

/********************************** 数据类型 **********************************/

/********************************** 函数声明 **********************************/
string  DeleteChar(const string& i_strOrig, char i_chToDelete);
string  ReplaceStr(string& i_strOrig, const string& i_strOldValue,const string& i_strNewValue);

/*********************************** 类定义 ***********************************/

class CStringSpliter
{
public:
    CStringSpliter(const string& i_strOrigStr);
    ~CStringSpliter();

    void            Split(const string& i_strSplitStr);
    int             Size();
    const string&   operator[](int i_nPos) const;
    string&         operator[](int i_nPos);
    vector<string>  GetVec();
private:
    string  m_strOrig;
    vector<string>  m_vecSplitStr;
};

#endif


