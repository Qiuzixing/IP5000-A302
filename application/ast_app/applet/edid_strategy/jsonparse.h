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
	char buf_5[STRUCT_BUF_NORMAL_LEN];
	char buf_6[STRUCT_BUF_NORMAL_LEN];
	char buf_7[STRUCT_BUF_NORMAL_LEN];
}JsonFileStruct;

enum E_Buf_Name
{
	e_buf_0 = 0,
	e_buf_1,
	e_buf_2,
	e_buf_3,
	e_buf_4,
	e_buf_5,
	e_buf_6,
	e_buf_7
};

bool ParseJsonFile(const char * i_modejsonfile,unsigned char *last_index);

void SetStructBufValue(E_Buf_Name i_name,const char * i_value);

char * GetStructBufValue(E_Buf_Name i_name);

void reorder_edidlist(E_Buf_Name index);

bool SaveStruct2File(const char * i_jsonfilename);

#endif
