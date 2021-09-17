/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：BC_Common_Log.h 
* 文件说明：
* 作    者：Heyman 
* 版本信息：1.0 
* 设计日期：2020-08-21 
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/************************************c语言通用日志框架************************************
*
*   
*
*   定义显示的日志级别
*   #define BC_COMMON_LOG_LEVEL
*       0. BC_COMMON_LOG_LEVEL_FATAL   
*       1. BC_COMMON_LOG_LEVEL_ERROR   
*       2. BC_COMMON_LOG_LEVEL_WARNING 
*       3. BC_COMMON_LOG_LEVEL_INFO    
*       4. BC_COMMON_LOG_LEVEL_DEBUG   
*       5. BC_COMMON_LOG_LEVEL_VERBOSE 
*   function：(使用printf输出格式带换行符)
*           BC_LOGV(type,format,...)
*           BC_LOGD(type,format,...)
*           BC_LOGI(type,format,...)
*           BC_LOGW(type,format,...)
*           BC_LOGE(type,format,...)
*           BC_LOGF(type,format,...)
*   eg. BC_LOGV("LOG",hellowrd");
*       int a=1;
*       BC_LOGE("LOG","a=%d",a);
*****************************************************************************************/
#pragma once
#ifndef __BC_COMMON_LOG_PRINT_H_
#define __BC_COMMON_LOG_PRINT_H_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if __linux__
#include <sys/time.h>
#endif
#define ENABLE_BC_LOGO4CPP                           0
#define ENABLE_BC_NTLOG                              0
#define ENABLE_BC_EXTERNAL_LOG                       0

#if ENABLE_BC_LOGO4CPP
#include "BC_logo4cpp.h"
#endif

#if ENABLE_BC_NTLOG
#include "BC_NTLog.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*改变此行控制日志输出级别*/
#define BC_COMMON_LOG_LEVEL   BC_COMMON_LOG_LEVEL_VERBOSE

#define BC_COMMON_LOG_LEVEL_FATAL     0   /*致命错误*/
#define BC_COMMON_LOG_LEVEL_ERROR     1   /*错误*/
#define BC_COMMON_LOG_LEVEL_WARNING   2   /*警告*/
#define BC_COMMON_LOG_LEVEL_INFO      3   /*信息*/
#define BC_COMMON_LOG_LEVEL_DEBUG     4   /*调试*/
#define BC_COMMON_LOG_LEVEL_VERBOSE   5   /*所有*/

#define TITLE_VERBOSE       "VERBOSE"
#define TITLE_DEBUG         "DEBUG"
#define TITLE_INFO          "INFO"
#define TITLE_WARNING       "WARN"
#define TITLE_ERROR         "ERROR"
#define TITLE_FATAL         "FATAL"

#ifndef BC_COMMON_LOG_LEVEL
#define BC_COMMON_LOG_LEVEL    BC_COMMON_LOG_LEVEL_VERBOSE
#endif

#define BC_COMMON_LOG_NOOP    (void) 0

#define BC_LOG_NONE                  		"\033[m"
#define BC_LOG_RED                         	"\033[0;32;31m"
#define BC_LOG_LIGHT_RED           			"\033[1;31m"
#define BC_LOG_GREEN                   		"\033[0;32;32m"
#define BC_LOG_LIGHT_GREEN     				"\033[1;32m"
#define BC_LOG_BLUE                       	"\033[0;32;34m"
#define BC_LOG_LIGHT_BLUE         			"\033[1;34m"
#define BC_LOG_DARY_GRAY         			"\033[1;30m"
#define BC_LOG_CYAN                       	"\033[0;36m"
#define BC_LOG_LIGHT_CYAN         			"\033[1;36m"
#define BC_LOG_PURPLE                 		"\033[0;35m"
#define BC_LOG_LIGHT_PURPLE   				"\033[1;35m"
#define BC_LOG_BROWN                 	 	"\033[0;33m"
#define BC_LOG_YELLOW                 		"\033[1;33m"
#define BC_LOG_LIGHT_GRAY        			"\033[0;37m"
#define BC_LOG_WHITE                    	"\033[1;37m"

#if ENABLE_BC_LOGO4CPP
#if ENABLE_BC_NTLOG
#define  BC_COMMON_LOG_PRINT(level,type,fmt,...)\
	do{\
		BC_Log4cpp_Output((int)level,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		BC_NTLog_Output(strAppModuleName,(int)level,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
	}while(0)
#else
#define  BC_COMMON_LOG_PRINT(level,type,fmt,...)    \
	do{	\
		BC_Log4cpp_Output((int)level,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);	\
	}while(0)
#endif

#define BC_COMMON_LOG_VERBOSE(type,fmt,...) BC_COMMON_LOG_PRINT(ELOG_LVL_VERBOSE,type,fmt,##__VA_ARGS__)
#define BC_COMMON_LOG_DEBUG(type,fmt,...)  BC_COMMON_LOG_PRINT(ELOG_LVL_DEBUG,type,fmt,##__VA_ARGS__)
#define BC_COMMON_LOG_INFO(type,fmt,...)   BC_COMMON_LOG_PRINT(ELOG_LVL_INFO,type,fmt,##__VA_ARGS__)
#define BC_COMMON_LOG_WARN(type,fmt,...)   BC_COMMON_LOG_PRINT(ELOG_LVL_WARN,type,fmt,##__VA_ARGS__)
#define BC_COMMON_LOG_ERROR(type,fmt,...)  BC_COMMON_LOG_PRINT(ELOG_LVL_ERROR,type,fmt,##__VA_ARGS__)
#define BC_COMMON_LOG_FATAL(type,fmt,...)  BC_COMMON_LOG_PRINT(ELOG_LVL_ASSERT,type,fmt,##__VA_ARGS__)

#define BC_LOGV(type,format,...)    BC_COMMON_LOG_VERBOSE(type,format,##__VA_ARGS__)
#define BC_LOGD(type,format,...)    BC_COMMON_LOG_DEBUG(type,format,##__VA_ARGS__)
#define BC_LOGI(type,format,...)    BC_COMMON_LOG_INFO(type,format,##__VA_ARGS__)
#define BC_LOGW(type,format,...)    BC_COMMON_LOG_WARN(type,format,##__VA_ARGS__)
#define BC_LOGE(type,format,...)    BC_COMMON_LOG_ERROR(type,format,##__VA_ARGS__)
#define BC_LOGF(type,format,...)    BC_COMMON_LOG_FATAL(type,format,##__VA_ARGS__)
#else

#if __linux__
#ifndef _TIME_POINTER_
#define _TIME_POINTER_
		 static time_t timep;
		 static struct tm *ptm;
		 static struct timeval tv;
#endif
#else
	static struct
    {
        unsigned long microsecond;
        unsigned char second;
        unsigned char minite;
        unsigned char hour;
        unsigned char data;
    }log_time;
	rt_tick_t log_tick_value;
#endif

static char strCommonLogColor[16]={0};						
static char strCommonLogLevel[16]={0};

#if __linux__
#define  BC_COMMON_LOG_PRINT(level,type,fmt,...)    \
	do{	\
		switch(level)\
		{\
			case BC_COMMON_LOG_LEVEL_FATAL:\
				{\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_FATAL);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_LIGHT_RED);\
				}\
				break;\
			case BC_COMMON_LOG_LEVEL_ERROR:\
				{\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_ERROR);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_RED);\
				}\
				break;\
			case BC_COMMON_LOG_LEVEL_WARNING:\
				{\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_WARNING);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_YELLOW);\
				}\
				break;\
			case BC_COMMON_LOG_LEVEL_INFO:\
				{\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_INFO);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_CYAN);\
				}\
				break;\
			case BC_COMMON_LOG_LEVEL_DEBUG:\
				{										\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_DEBUG);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_GREEN);\
				}\
				break;									\
			case BC_COMMON_LOG_LEVEL_VERBOSE:\
			default:\
				{\
					snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_VERBOSE);\
					snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_LIGHT_GRAY);\
				}\
				break;\
		}\
		gettimeofday(&tv, NULL);\
        time(&timep);\
        ptm = localtime(&timep);\
        fprintf(stdout,\
        "%s[%4d-%02d-%02d %02d:%02d:%02d,%03d] [%s] [%s] [%s] [%d] " fmt BC_LOG_NONE "\n",\
        strCommonLogColor,\
        ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,(int)(tv.tv_usec / 1000),\
        strCommonLogLevel,type,__FUNCTION__,__LINE__,##__VA_ARGS__);\
	}while(0)
#else
#define  BC_COMMON_LOG_PRINT(level,type,fmt,...)    \
		do{ \
			switch(level)\
			{\
				case BC_COMMON_LOG_LEVEL_FATAL:\
					{\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_FATAL);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_LIGHT_RED);\
					}\
					break;\
				case BC_COMMON_LOG_LEVEL_ERROR:\
					{\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_ERROR);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_RED);\
					}\
					break;\
				case BC_COMMON_LOG_LEVEL_WARNING:\
					{\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_WARNING);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_YELLOW);\
					}\
					break;\
				case BC_COMMON_LOG_LEVEL_INFO:\
					{\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_INFO);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_CYAN);\
					}\
					break;\
				case BC_COMMON_LOG_LEVEL_DEBUG:\
					{										\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_DEBUG);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_GREEN);\
					}\
					break;									\
				case BC_COMMON_LOG_LEVEL_VERBOSE:\
				default:\
					{\
						snprintf(strCommonLogLevel,sizeof(strCommonLogLevel),"%s",TITLE_VERBOSE);\
						snprintf(strCommonLogColor,sizeof(strCommonLogColor),"%s",BC_LOG_LIGHT_GRAY);\
					}\
					break;\
			}\
			log_tick_value = rt_tick_get();\
			log_time.microsecond = log_tick_value * (1000 / RT_TICK_PER_SECOND) % 1000;\
            log_tick_value /= RT_TICK_PER_SECOND;\
            log_time.second = log_tick_value % 60;\
            log_tick_value /= 60;\
            log_time.minite = log_tick_value % 60;\
            log_tick_value /= 60;\
            log_time.hour = log_tick_value % 24;\
            log_tick_value /= 24;\
            log_time.data = log_tick_value % 30;\
			rt_kprintf(\
			"%s[%04d %02d:%02d:%02d,%03d] [%s] [%s] [%s] [%d] " fmt BC_LOG_NONE "\n",\
        	strCommonLogColor,\
        	log_time.data,log_time.hour,log_time.minite,log_time.second,log_time.microsecond,\
        	strCommonLogLevel,type,__FUNCTION__,__LINE__,##__VA_ARGS__);\
		}while(0)
#endif

#if BC_COMMON_LOG_LEVEL_VERBOSE <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_VERBOSE(type,fmt,...)	\
	do{	\
		BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_VERBOSE,type,fmt,##__VA_ARGS__);	\
		BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_VERBOSE,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
	}while(0)
#else
#define BC_COMMON_LOG_VERBOSE(type,fmt,...) BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_VERBOSE,type,fmt,##__VA_ARGS__)
#endif
#else
#define BC_COMMON_LOG_VERBOSE(...) BC_COMMON_LOG_NOOP
#endif

#define BC_LOGV(type,format,...)    BC_COMMON_LOG_VERBOSE(type,format,##__VA_ARGS__)

#if BC_COMMON_LOG_LEVEL_DEBUG <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_DEBUG(type,fmt,...)	\
		do{ \
			BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_DEBUG,type,fmt,##__VA_ARGS__);	\
			BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_DEBUG,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_COMMON_LOG_DEBUG(type,fmt,...)  BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_DEBUG,type,fmt,##__VA_ARGS__)
#endif
#else
#define  BC_COMMON_LOG_DEBUG(...)  BC_COMMON_LOG_NOOP
#endif

#define BC_LOGD(type,format,...)    BC_COMMON_LOG_DEBUG(type,format,##__VA_ARGS__)

#if BC_COMMON_LOG_LEVEL_INFO <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_INFO(type,fmt,...)	\
		do{ \
			BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_INFO,type,fmt,##__VA_ARGS__);	\
			BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_INFO,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_COMMON_LOG_INFO(type,fmt,...)   BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_INFO,type,fmt,##__VA_ARGS__)
#endif
#else
#define  BC_COMMON_LOG_INFO(...)   BC_COMMON_LOG_NOOP
#endif

#define BC_LOGI(type,format,...)    BC_COMMON_LOG_INFO(type,format,##__VA_ARGS__)

#if BC_COMMON_LOG_LEVEL_WARNING <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_WARN(type,fmt,...)	\
		do{ \
			BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_WARNING,type,fmt,##__VA_ARGS__); \
			BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_WARNING,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_COMMON_LOG_WARN(type,fmt,...)   BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_WARNING,type,fmt,##__VA_ARGS__)
#endif
#else
#define  BC_COMMON_LOG_WARN(...)   BC_COMMON_LOG_NOOP
#endif

#define BC_LOGW(type,format,...)    BC_COMMON_LOG_WARN(type,format,##__VA_ARGS__)

#if BC_COMMON_LOG_LEVEL_ERROR <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_ERROR(type,fmt,...)	\
		do{ \
			BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_ERROR,type,fmt,##__VA_ARGS__); \
			BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_ERROR,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_COMMON_LOG_ERROR(type,fmt,...)  BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_ERROR,type,fmt,##__VA_ARGS__)
#endif
#else
#define  BC_COMMON_LOG_ERROR(...)  BC_COMMON_LOG_NOOP
#endif

#define BC_LOGE(type,format,...)    BC_COMMON_LOG_ERROR(type,format,##__VA_ARGS__)

#if BC_COMMON_LOG_LEVEL_FATAL <= BC_COMMON_LOG_LEVEL
#if ENABLE_BC_NTLOG
#define BC_COMMON_LOG_FATAL(type,fmt,...)	\
		do{ \
			BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_FATAL,type,fmt,##__VA_ARGS__); \
			BC_NTLog_Output(strAppModuleName,BC_COMMON_LOG_LEVEL_FATAL,__FUNCTION__,__LINE__,(char *)type,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_COMMON_LOG_FATAL(type,fmt,...)  BC_COMMON_LOG_PRINT(BC_COMMON_LOG_LEVEL_FATAL,type,fmt,##__VA_ARGS__)
#endif
#else
#define  BC_COMMON_LOG_FATAL(...)  BC_COMMON_LOG_NOOP
#endif

#define BC_LOGF(type,format,...)    BC_COMMON_LOG_FATAL(type,format,##__VA_ARGS__)

#endif //ENABLE_BC_LOGO4CPP

#if ENABLE_BC_EXTERNAL_LOG
#if ENABLE_BC_LOGO4CPP
#if ENABLE_BC_NTLOG
//notice match format need [app][debuglev] if msg have't must add
/*BC_NTLog_Output(src,ELOG_LVL_INFO,NULL,-1,NULL,fmt,##__VA_ARGS__);\*/  
/*BC_NTLog_Output(NULL,-1,NULL,-1,NULL,fmt,##__VA_ARGS__);\*/           
#define  BC_EXTERNAL_LOG_PRINT(src,fmt,...)\
		do{\
			BC_Log4cpp_Output(ELOG_LVL_INFO,NULL,-1,NULL,fmt,##__VA_ARGS__);\
			BC_NTLog_Output(src,ELOG_LVL_INFO,NULL,-1,NULL,fmt,##__VA_ARGS__);\
		}while(0)
#else
#define  BC_EXTERNAL_LOG_PRINT(fmt,...)\
			do{\
				BC_Log4cpp_Output(ELOG_LVL_INFO,NULL,-1,NULL,fmt,##__VA_ARGS__);\
			}while(0)
#endif
#else
#define  BC_EXTERNAL_LOG_PRINT(fmt,...)    \
		do{ \
			gettimeofday(&tv, NULL);	\
			time(&timep);	\
			ptm =localtime(&timep); 		  \
			fprintf(stdout,\
			BC_LOG_CYAN "[%4d-%02d-%02d %02d:%02d:%02d,%03d]" BC_LOG_NONE fmt "\n",\
			ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,(int)(tv.tv_usec / 1000),\
			,##__VA_ARGS__);	\
		}while(0)

#endif

#define BC_LOG_EXTERNAL(src,format,...)  BC_EXTERNAL_LOG_PRINT(src,format,##__VA_ARGS__)

#endif //ENABLE_BC_EXTERNAL_LOG

#ifdef __cplusplus
}
#endif
#endif
