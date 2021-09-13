#include "jsonparse.h"
#include <iostream>
#include <string.h>
#include <string> 
#include <fstream>
#include "json/json.h"

using namespace std;

JsonFileStruct g_jsonfilestruct;
#define EDID_FILE_MAX 8

static void InitJsonFileStruct()
{
	memset(&g_jsonfilestruct,0,sizeof(JsonFileStruct));
}

void PrintJsonStruct()
{
	printf("[%s]\n[%s]\n[%s]\n[%s]\n[%s]\n",g_jsonfilestruct.buf_0,g_jsonfilestruct.buf_1,
											g_jsonfilestruct.buf_2,g_jsonfilestruct.buf_3,g_jsonfilestruct.buf_4);
}

bool ParseJsonFile(const char * i_modejsonfile,unsigned char *last_index)
{
	InitJsonFileStruct();
	int i = 0;
	Json::Reader reader;  
	Json::Value root1; 
	std::ifstream is;  
	is.open (i_modejsonfile, std::ios::binary);
	if(!is.is_open())
	{
		printf("open %s fail !!!\n",i_modejsonfile);
	    return false;
	}
	
	if(reader.parse(is, root1))  
	{   
		if(!root1["edid_list"].empty())
		{
			Json::Value& root = root1["edid_list"];
			if(!root["0"].empty())
			{
				string buf0 = root["0"].asString();
				if(buf0.length() > sizeof(g_jsonfilestruct.buf_0))
				{
					printf("Warning : buf_0 'size is too small\n");
				}
				if(buf0.length() == 0)
				{
					*last_index = 0;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_0,buf0.c_str(),sizeof(g_jsonfilestruct.buf_0) - 1);//because the string needs to end with '\0'
				}
			}
			
			if(!root["1"].empty())
			{
				string buf1 = root["1"].asString();
				if(buf1.length() > sizeof(g_jsonfilestruct.buf_1))
				{
					printf("Warning : buf_1 'size is too small\n");
				}
				if(buf1.length() == 0)
				{
					*last_index = 1;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_1,buf1.c_str(),sizeof(g_jsonfilestruct.buf_1) - 1);
				}
			}
			
			if(!root["2"].empty())
			{
				string buf2 = root["2"].asString();
				if(buf2.length() > sizeof(g_jsonfilestruct.buf_2))
				{
					printf("Warning : buf_2 'size is too small\n");
				}
				if(buf2.length() == 0)
				{
					*last_index = 2;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_2,buf2.c_str(),sizeof(g_jsonfilestruct.buf_2) - 1);
				}
			}

			if(!root["3"].empty())
			{
				string buf3 = root["3"].asString();
				if(buf3.length() > sizeof(g_jsonfilestruct.buf_3))
				{
					printf("Warning : buf_3 'size is too small\n");
				}
				if(buf3.length() == 0)
				{
					*last_index = 3;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_3,buf3.c_str(),sizeof(g_jsonfilestruct.buf_3) - 1);
				}
			}

			if(!root["4"].empty())
			{
				string buf4 = root["4"].asString();
				if(buf4.length() > sizeof(g_jsonfilestruct.buf_4))
				{
					printf("Warning : buf_4 'size is too small\n");
				}
				if(buf4.length() == 0)
				{
					*last_index = 4;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_4,buf4.c_str(),sizeof(g_jsonfilestruct.buf_4) - 1);
				}
			}

			if(!root["5"].empty())
			{
				string buf5 = root["5"].asString();
				if(buf5.length() > sizeof(g_jsonfilestruct.buf_5))
				{
					printf("Warning : buf_5 'size is too small\n");
				}
				if(buf5.length() == 0)
				{
					*last_index = 5;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_5,buf5.c_str(),sizeof(g_jsonfilestruct.buf_5) - 1);
				}
			}

			if(!root["6"].empty())
			{
				string buf6 = root["6"].asString();
				if(buf6.length() > sizeof(g_jsonfilestruct.buf_6))
				{
					printf("Warning : buf_6 'size is too small\n");
				}
				if(buf6.length() == 0)
				{
					*last_index = 6;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_6,buf6.c_str(),sizeof(g_jsonfilestruct.buf_6) - 1);
				}
			}

			if(!root["7"].empty())
			{
				string buf7 = root["7"].asString();
				if(buf7.length() > sizeof(g_jsonfilestruct.buf_7))
				{
					printf("Warning : buf_7 'size is too small\n");
				}
				if(buf7.length() == 0)
				{
					*last_index = 7;
					return true;
				}
				else
				{
					memcpy(g_jsonfilestruct.buf_7,buf7.c_str(),sizeof(g_jsonfilestruct.buf_7) - 1);
				}
			}
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

static void SetStructBuf5Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_5,0,sizeof(g_jsonfilestruct.buf_5));
	strncpy(g_jsonfilestruct.buf_5,i_value,sizeof(g_jsonfilestruct.buf_5));
}
static void SetStructBuf6Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_6,0,sizeof(g_jsonfilestruct.buf_6));
	strncpy(g_jsonfilestruct.buf_6,i_value,sizeof(g_jsonfilestruct.buf_6));
}
static void SetStructBuf7Value(const char * i_value)
{
	memset(g_jsonfilestruct.buf_7,0,sizeof(g_jsonfilestruct.buf_7));
	strncpy(g_jsonfilestruct.buf_7,i_value,sizeof(g_jsonfilestruct.buf_7));
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

static char * GetStructBuf5Value()
{
	return g_jsonfilestruct.buf_5;
}

static char * GetStructBuf6Value()
{
	return g_jsonfilestruct.buf_6;
}

static char * GetStructBuf7Value()
{
	return g_jsonfilestruct.buf_7;
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
	else if(e_buf_5 == i_name)
	{
		SetStructBuf5Value(i_value);
	}
	else if(e_buf_6 == i_name)
	{
		SetStructBuf6Value(i_value);
	}
	else if(e_buf_7 == i_name)
	{
		SetStructBuf7Value(i_value);
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
	else if(e_buf_5 == i_name)
	{
		return GetStructBuf5Value();
	}
	else if(e_buf_6 == i_name)
	{
		return GetStructBuf6Value();
	}
	else if(e_buf_7 == i_name)
	{
		return GetStructBuf7Value();
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
	root["5"] = g_jsonfilestruct.buf_5;
	root["6"] = g_jsonfilestruct.buf_6;
	root["7"] = g_jsonfilestruct.buf_7;

	Json::Value root1;
	root1["edid_list"] = root;

	Json::StyledWriter fw;
	string m_strjson = fw.write(root1);

	StringToFile(m_strjson.c_str(),i_jsonfilename);

	return true;
}

void reorder_edidlist(E_Buf_Name index)
{
	switch(index)
	{
		case e_buf_0:
		case e_buf_1:
			SetStructBuf1Value(g_jsonfilestruct.buf_2);
		case e_buf_2:
			SetStructBuf2Value(g_jsonfilestruct.buf_3);
		case e_buf_3:
			SetStructBuf3Value(g_jsonfilestruct.buf_4);
		case e_buf_4:
			SetStructBuf4Value(g_jsonfilestruct.buf_5);
		case e_buf_5:
			SetStructBuf5Value(g_jsonfilestruct.buf_6);
		case e_buf_6:
			SetStructBuf6Value(g_jsonfilestruct.buf_7);
		case e_buf_7:
			SetStructBuf7Value("");
			break;
		default:
			break;

	}
}
