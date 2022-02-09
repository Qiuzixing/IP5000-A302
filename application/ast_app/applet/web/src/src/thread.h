/*
 * thread.h
 *
 *  Created on: 2014-01-09
 *      Author: sid
 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "types.h"

class CThread
{
public:
    static void         Initialize(UInt32 i_u32StackSizeBytes = 1536 * 1024);  // Call before calling any other functions 1.5M
    static pthread_t    GetCurrentThreadID(void)            { return ::pthread_self(); }
    static CThread*     GetCurrent(void);
    static void         ThreadYield(void);
    static void         Sleep(UInt32 i_u32Msec);
    static void         SetUser(const char* i_pszUser)      { ::strncpy(s_szUser, i_pszUser, sizeof(s_szUser) - 1); s_szUser[sizeof(s_szUser) - 1] = 0; }
    static void         SetGroup(const char* i_pszGroup)    { ::strncpy(s_szGroup, i_pszGroup, sizeof(s_szGroup) - 1); s_szGroup[sizeof(s_szGroup) - 1] = 0; }
    static void         SetPersonality(const char* i_pszUser, const char* i_pszGroup)   { SetUser(i_pszUser); SetGroup(i_pszGroup); }
    static void         WrapSleep(Bool i_bwrapSleep)        { s_bWrapSleep = i_bwrapSleep; }

private:
    static void*        _Entry(void* inThread);

    static pthread_key_t    s_tMainKey;
    static char             s_szUser[128];
    static char             s_szGroup[128];
    static Bool             s_bWrapSleep;
    static pthread_attr_t   s_tThreadAttr;

public:
    CThread(void);
    CThread(SInt32 i_s32ThreadPolicy, SInt32 i_s32SchedPriority);
    virtual         ~CThread(void);
    virtual void    Entry(void) = 0;
    void            Start(void);

    void            Join(void);
    void            SendStopRequest(void)               { m_bStopRequested = TRUE; }
    Bool            IsStopRequested(void)               { return m_bStopRequested; }
    void            SetThreadData(void* i_pThreadData)  { m_pThreadData = i_pThreadData; }
    void*           GetThreadData(void)                 { return m_pThreadData; }
    void            StopAndWaitForThread(void);
    Bool            SwitchPersonality(void);
    Bool            ResetStopRequest();
    void            SetAffinity(int cpu, char *pMsg);
    void            SetName(const char *pName);

private:
    Bool        m_bStopRequested;
    Bool        m_bJoined;
    pthread_t   m_tThreadID;
    void*       m_pThreadData;
    SInt32      m_s32ThreadPolicy;
    SInt32      m_s32SchedPriority;
};

#endif /* __THREAD_H__ */
