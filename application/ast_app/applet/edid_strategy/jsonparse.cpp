#include "jsonparse.h"
#include <iostream>
#include <string.h>
#include <string> 
#include <fstream>
#include "json/json.h"

using namespace std;

JsonFileStruct g_jsonfilestruct;


static void InitJsonFileStruct()
{
	memset(&g_jsonfilestruct,0,sizeof(JsonFileStruct));
}

static void PrintJsonStruct()
{
	printf("[%s]\n[%s]\n[%s]\n[%s]\n[%s]\n",g_jsonfilestruct.buf_0,g_jsonfilestruct.buf_1,
											g_jsonfilestruct.buf_2,g_jsonfilestruct.buf_3,g_jsonfilestruct.buf_4);
}

bool ParseJsonFile(const char * i_modejsonfile)
{
	InitJsonFileStruct();
	
	Json::Reader reader;  
	Json::Value root; 
	std::ifstream is;  
	is.open (i_modejsonfile, std::ios::binary);
	if(!is.is_open())
	{
		printf("open %s fail !!!\n",i_modejsonfile);
	    return false;
	}
	if(reader.parse(is, root))  
	{   
		if(!root["0"].empty())
		{
			string buf0 = root["0"].asString();
			if(buf0.length() > sizeof(g_jsonfilestruct.buf_0))
			{
				printf("Warning : buf_0 'size is too small\n");
			}
			memcpy(g_jsonfilestruct.buf_0,buf0.c_str(),sizeof(g_jsonfilestruct.buf_0) - 1);//because the string needs to end with '\0'
		}
		if(!root["1"].empty())
		{
			string buf1 = root["1"].asString();
			if(buf1.length() > sizeof(g_jsonfilestruct.buf_1))
			{
				printf("Warning : buf_1 'size is too small\n");
			}
			memcpy(g_jsonfilestruct.buf_1,buf1.c_str(),sizeof(g_jsonfilestruct.buf_1) - 1);
		}
		if(!root["2"].empty())
		{
			string buf2 = root["2"].asString();
			if(buf2.length() > sizeof(g_jsonfilestruct.buf_2))
			{
				printf("Warning : buf_2 'size is too small\n");
			}
			memcpy(g_jsonfilestruct.buf_2,buf2.c_str(),sizeof(g_jsonfilestruct.buf_2) - 1);
		}
		if(!root["3"].empty())
		{
			string buf3 = root["3"].asString();
			if(buf3.length() > sizeof(g_jsonfilestruct.buf_3))
			{
				printf("Warning : buf_3 'size is too small\n");
			}
			memcpy(g_jsonfilestruct.buf_3,buf3.c_str(),sizeof(g_jsonfilestruct.buf_3) - 1);
		}
		if(!root["4"].empty())
		{
			string buf4 = root["4"].asString();
			if(buf4.length() > sizeof(g_jsonfilestruct.buf_4))
			{
				printf("Warning : buf_4 'size is too small\n");
			}
			memcpy(g_jsonfilestruct.buf_4,buf4.c_str(),sizeof(g_jsonfilestruct.buf_4) - 1);
		}
        
	}
	else
	{
		printf("%s data is not json data\n",i_modejsonfile);
		is.close(); 
		return false;
	}
	is.close(); 

	//PrintJsonStruct();
	return true; 
}


static void SetStructBuf0Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_0,0,sizeof(g_jsonfilestruct.buf_0));
	strncpy(g_jsonfilestruct.buf_0,i_value,sizeof(g_jsonfilestruct.buf_0));
}
static void SetStructBuf1Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_1,0,sizeof(g_jsonfilestruct.buf_1));
	strncpy(g_jsonfilestruct.buf_1,i_value,sizeof(g_jsonfilestruct.buf_1));
}
static void SetStructBuf2Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_2,0,sizeof(g_jsonfilestruct.buf_2));
	strncpy(g_jsonfilestruct.buf_2,i_value,sizeof(g_jsonfilestruct.buf_2));
}
static void SetStructBuf3Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_3,0,sizeof(g_jsonfilestruct.buf_3));
	strncpy(g_jsonfilestruct.buf_3,i_value,sizeof(g_jsonfilestruct.buf_3));
}
static void SetStructBuf4Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_4,0,sizeof(g_jsonfilestruct.buf_4));
	strncpy(g_jsonfilestruct.buf_4,i_value,sizeof(g_jsonfilestruct.buf_4));
}

static char * GetStructBuf0Value()
{
	return g_jsonfilestruct.buf_0;
}
static char * GetStructBuf1Value()
{
	return g_jsonfilestruct.buf_1;
}

static char * GetStructBuf2Value()
{
	return g_jsonfilestruct.buf_2;
}

static char * GetStructBuf3Value()
{
	return g_jsonfilestruct.buf_3;
}

static char * GetStructBuf4Value()
{
	return g_jsonfilestruct.buf_4;
}

void SetStructBufValue(E_Buf_Name i_name,const char * i_value)
{
	if(e_buf_0 == i_name)
	{
		SetStructBuf0Value(i_value);
	}
	else if(e_buf_1 == i_name)
	{
		SetStructBuf1Value(i_value);
	}
	else if(e_buf_2 == i_name)
	{
		SetStructBuf2Value(i_value);
	}
	else if(e_buf_3 == i_name)
	{
		SetStructBuf3Value(i_value);
	}
	else if(e_buf_4 == i_name)
	{
		SetStructBuf4Value(i_value);
	}
	else
	{
		printf("Unknow Type\n");
	}
}

char * GetStructBufValue(E_Buf_Name i_name)
{
	if(e_buf_0 == i_name)
	{
		return GetStructBuf0Value();
	}
	else if(e_buf_1 == i_name)
	{
		return GetStructBuf1Value();
	}
	else if(e_buf_2 == i_name)
	{
		return GetStructBuf2Value();
	}
	else if(e_buf_3 == i_name)
	{
		return GetStructBuf3Value();
	}
	else if(e_buf_4 == i_name)
	{
		return GetStructBuf4Value();
	}
	else
	{
		printf("Unknow Type\n");
		return NULL;
	}
}


static int StringToFile(const char *instrBuff, const char *poutFileath)
{
	FILE *fp	= NULL;
	fp = fopen(poutFileath,"w+");
	if(fp==NULL)
	{
		return -1;
	}
	int nRet = fwrite(instrBuff,1,strlen(instrBuff),fp);
	fflush(fp);
	fclose(fp);	
	return 	nRet ;
}



bool SaveStruct2File(const char * i_jsonfilename)
{
	//PrintJsonStruct();

	Json::Value root;
	root["0"] = g_jsonfilestruct.buf_0;
	root["1"] = g_jsonfilestruct.buf_1;
	root["2"] = g_jsonfilestruct.buf_2;
	root["3"] = g_jsonfilestruct.buf_3;
	root["4"] = g_jsonfilestruct.buf_4;

	Json::StyledWriter fw;
	string m_strjson = fw.write(root);

	StringToFile(m_strjson.c_str(),i_jsonfilename);

	return true;
}


