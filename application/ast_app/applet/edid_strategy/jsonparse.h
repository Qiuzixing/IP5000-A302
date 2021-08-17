#ifndef __JSONPARSE_H__
#define __JSONPARSE_H__

#include <stdio.h>

#define STRUCT_BUF_NORMAL_LEN			512


typedef struct JsonFileStruct
{
	char buf_0[STRUCT_BUF_NORMAL_LEN];
	char buf_1[STRUCT_BUF_NORMAL_LEN];
	char buf_2[STRUCT_BUF_NORMAL_LEN];
	char buf_3[STRUCT_BUF_NORMAL_LEN];
	char buf_4[STRUCT_BUF_NORMAL_LEN];
}JsonFileStruct;

enum E_Buf_Name
{
	e_buf_0 = 0,
	e_buf_1,
	e_buf_2,
	e_buf_3,
	e_buf_4
};


bool ParseJsonFile(const char * i_modejsonfile);

void SetStructBufValue(E_Buf_Name i_name,const char * i_value);

char * GetStructBufValue(E_Buf_Name i_name);



bool SaveStruct2File(const char * i_jsonfilename);

#endif
