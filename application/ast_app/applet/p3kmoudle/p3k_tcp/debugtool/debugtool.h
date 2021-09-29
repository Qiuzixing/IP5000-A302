#ifndef __DEBUGTOOL_H__
#define __DEBUGTOOL_H__

#include <stdio.h>
#include <stdarg.h>

#define _DBG_ON
//#define _DBG_LEVEL_INFO_
#define _DBG_LEVEL_WARN_

//define _DBG_ to open debug
#ifdef _DBG_ON
    #ifdef _DBG_LEVEL_INFO_    
/* 如果定义了调试级别为INFO， 那么同时打开WARN， ERR开关*/
        #ifndef _DBG_LEVEL_WARN_
            #define _DBG_LEVEL_WARN_
        #endif
        
        #ifndef _DBG_LEVEL_ERR_
            #define DBG_LEVER_ERR
        #endif    
    #endif
    /* 如果定义了调试级别为WARN， 那么同时打开ERR开关*/
    #ifdef _DBG_LEVEL_WARN_    
        #ifndef _DBG_LEVEL_ERR_
            #define _DBG_LEVEL_ERR_
        #endif    
    #endif
//_DBG_LEVEL_ERR_ for default
    #ifndef _DBG_LEVEL_ERR_
        #define _DBG_LEVEL_ERR_
    #endif
#endif


void __DBG_INFO (const char* file,const char*func, int line, const char *format, ...);
void __DBG_WARN (const char* file,const char*func, int line, const char *format, ...);
void __DBG_ERR (const char* file, const char*func, int line, const char *format, ...);
//redirect print to files
void  px_dbg_file(FILE *dbgfile) ;
//if closed debug file clear; 
void px_dpg_file_clean();
#ifdef _DBG_ON
    #ifdef _DBG_LEVEL_INFO_
        #define DBG_InfoMsg(format, args...) __DBG_INFO(__FILE__,__FUNCTION__, __LINE__, format, ##args)    
    #else
        #define DBG_InfoMsg(format, args...)    
    #endif
#else 
    #define DBG_InfoMsg(format, args...)    
#endif
/*************************************************/
#ifdef _DBG_ON
    #ifdef _DBG_LEVEL_WARN_
        #define DBG_WarnMsg(format, args...) __DBG_WARN(__FILE__,__FUNCTION__, __LINE__, format, ##args)
    #else
        #define DBG_WarnMsg(format, args...)    
    #endif
#else 
    #define DBG_WarnMsg(format, args...)    
#endif

/**********************************************/
#ifdef _DBG_ON
    #ifdef _DBG_LEVEL_ERR_
        #define DBG_ErrMsg(format, args...) __DBG_ERR(__FILE__,__FUNCTION__, __LINE__,format, ##args)
    #else
        #define DBG_ErrMsg(format, args...)
    #endif
#else 
    #define DBG_ErrMsg(format, args...)
#endif


#endif
