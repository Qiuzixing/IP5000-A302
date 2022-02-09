#ifndef __PROCESS_JSON_H__
#define __PROCESS_JSON_H__


#include <iostream>
#include <string>
#include "json.h"
#include <fstream>
using namespace std;

typedef struct ConfInfoParam
{
	string auth_mode;
	string web_rootpath;
	string web_httpport;
	string web_httpsport;
	string web_cermpath;
	string web_startmode;
	string log_confpath;
	string web_numthread;
	string web_keepaliveenable;
	string web_keepalivetimems;
	string web_ws_pingpongenable;
	string web_ws_pingpongtimems;
	string ldap_URI;
	string ldap_BaseDN;
	string ldap_BindDN;
	string ldap_Attr;
	string ldap_CaPem;
}ConfInfoParam;


/*
demon： 从json格式的数据流中获取 value					数据结构为 嵌套结构
json内容如下：
{
    "date":{
		"year":2020
		"mon":9
		"day":27
    }
    "student":"xxx"
    "num":66666
    "age":18
}
*/
void GetValueFromJsonstream();

/*
    //json内容如下： 							数据结构为 多子集结构
    {
    "name": "json″,
    "array": [
        {
            "cpp": "jsoncpp"
        },
        {
            "java": "jsoninjava"
        },
        {
            "php": "support"
        }
    ]
    }
    */

void readStrProJson();



/*
	从文件中读取 json 数据
*/
typedef struct WEBCONF
{
	string ROOT;
	string PORT;
	string CERM;
}WEBCONF;
int ReadStrFromFile(const char* filename,WEBCONF * conf);






bool GetConfParamfromJson(const char * i_jsonfile,ConfInfoParam * i_webparam);


bool UpdateWebToConfigfile(const char * i_jsonfile, const char *i_option, string i_startmode);







#endif
