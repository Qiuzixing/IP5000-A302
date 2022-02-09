/*
 * thread.cpp
 *
 *  Created on: 2014-1-9
 *      Author: sid
 */

#include <pthread.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "thread.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define THREAD_DEBUG 0

//#define gettid() syscall(__NR_gettid)
#define gettid() syscall(SYS_gettid)

pthread_key_t   CThread::s_tMainKey = 0;
pthread_attr_t  CThread::s_tThreadAttr;

char CThread::s_szUser[128]     = "";
char CThread::s_szGroup[128]    = "";
Bool CThread::s_bWrapSleep      = TRUE;

void CThread::Initialize(UInt32 i_u32StackSizeBytes)
{
    (void)pthread_key_create(&s_tMainKey, NULL);
    (void)pthread_attr_init(&s_tThreadAttr);
    (void)pthread_attr_setscope(&s_tThreadAttr, PTHREAD_SCOPE_SYSTEM);
    (void)pthread_attr_setstacksize(&s_tThreadAttr, i_u32StackSizeBytes);
}

CThread* CThread::GetCurrent()
{
    return (CThread*)pthread_getspecific(CThread::s_tMainKey);
}

void CThread::ThreadYield()
{
    // on platforms who's threading is not pre-emptive yield
    // to another thread
    (void)sched_yield();
}

void CThread::Sleep(UInt32 i_u32Msec)
{
    ::usleep(i_u32Msec * 1000);
}

void* CThread::_Entry(void* i_pThread)
{
    CThread* pThread = (CThread*)i_pThread;
    pThread->m_tThreadID = (pthread_t)pthread_self();

    struct sched_param tThreadParam;
    
#if (THREAD_DEBUG) 
    SInt32 s32ThreadPolicy = SCHED_OTHER;
    pthread_getschedparam(pThread->m_tThreadID, &s32ThreadPolicy, &tThreadParam);

    printf(" Thread 0x%lx DEBUG: Default policy is %s, priority is %d\n", pThread->m_tThreadID,
        (s32ThreadPolicy == SCHED_FIFO ? "FIFO"
         : (s32ThreadPolicy == SCHED_RR ? "RR"
            : (s32ThreadPolicy == SCHED_OTHER ? "OTHER"
               : "unknown"))),
        tThreadParam.sched_priority);
#endif

    tThreadParam.sched_priority = pThread->m_s32SchedPriority;
    
    pthread_setschedparam(pThread->m_tThreadID, pThread->m_s32ThreadPolicy, &tThreadParam);

#if (THREAD_DEBUG) 
    printf(" Thread 0x%lx DEBUG: set thread at %s/%d\n", pThread->m_tThreadID,
        (pThread->m_s32ThreadPolicy == SCHED_FIFO ? "FIFO"
         : (pThread->m_s32ThreadPolicy == SCHED_RR ? "RR"
            : (pThread->m_s32ThreadPolicy == SCHED_OTHER ? "OTHER"
               : "unknown"))),
         tThreadParam.sched_priority);

    pthread_getschedparam(pThread->m_tThreadID, &s32ThreadPolicy, &tThreadParam);

    printf(" Thread 0x%lx DEBUG: already set the thread policy is %s, priority is %d\n", pThread->m_tThreadID,
        (s32ThreadPolicy == SCHED_FIFO ? "FIFO"
         : (s32ThreadPolicy == SCHED_RR ? "RR"
            : (s32ThreadPolicy == SCHED_OTHER ? "OTHER"
               : "unknown"))),
        tThreadParam.sched_priority);
#endif

    (void)pthread_setspecific(CThread::s_tMainKey, pThread);
    (void)pThread->SwitchPersonality();
    //
    // Run the thread
    pThread->Entry();
    return NULL;
}

CThread::CThread(void)
    : m_bStopRequested(FALSE), m_bJoined(FALSE), m_tThreadID(0), m_pThreadData(NULL), m_s32ThreadPolicy(SCHED_OTHER), m_s32SchedPriority(0)
{
}

CThread::CThread(SInt32 i_s32ThreadPolicy, SInt32 i_s32SchedPriority)
    : m_bStopRequested(FALSE), m_bJoined(FALSE), m_tThreadID(0), m_pThreadData(NULL), m_s32ThreadPolicy(i_s32ThreadPolicy), m_s32SchedPriority(i_s32SchedPriority)
{
}

CThread::~CThread(void)
{
    this->StopAndWaitForThread();
}

void CThread::Start(void)
{
    pthread_attr_t* pThreadAttr = &s_tThreadAttr;
    SInt32 s32Err = pthread_create((pthread_t*)&m_tThreadID, pThreadAttr, _Entry, (void*)this);
    assert(0 == s32Err);
}

void CThread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    assert(!m_bJoined);
    m_bJoined = TRUE;
    if (0 != m_tThreadID)
    {
        (void)pthread_join(m_tThreadID, NULL);
    }
}

void CThread::StopAndWaitForThread(void)
{
    m_bStopRequested = TRUE;
    if (!m_bJoined)
    {
        Join();
    }
}

Bool CThread::SwitchPersonality()
{
    if (::strlen(s_szGroup) > 0)
    {
        struct group* gr = ::getgrnam(s_szGroup);
        if (gr == NULL || ::setgid(gr->gr_gid) == -1)
        {
            return FALSE;
        }
    }

    if (::strlen(s_szUser) > 0)
    {
        struct passwd* pw = ::getpwnam(s_szUser);
        if (pw == NULL || ::setuid(pw->pw_uid) == -1)
        {
            return FALSE;
        }
    }

    return TRUE;
}

Bool CThread::ResetStopRequest()
{
    m_bStopRequested = FALSE;
    return TRUE;
}

void CThread::SetAffinity(int cpu, char *pMsg)
{
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);

    cpu_set_t mask;

    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

    if (sched_setaffinity(0, &mask) == -1)
    {
        printf("sched_setaffinity failed \n");
        return;
    }
    
//#if (THREAD_DEBUG) 
#if 1
    CPU_ZERO(&mask);
    if (sched_getaffinity(0, &mask) == -1)
    {
        printf("sched_getaffinity failed\n");
        return;
    }
    for (int i = 0; i < cpu_num; i++)
    {
        if (CPU_ISSET(i, &mask))
        {
            if(NULL == pMsg)
                printf("thread %lu run on processor %d\n", gettid(), i);
            else
                printf("[%s] thread %lu run on processor %d\n", pMsg, gettid(), i);
        }
    }
#endif
}

void CThread::SetName(const char *pName)
{
    if(NULL == pName)
        return;

    prctl(PR_SET_NAME, (unsigned long)(pName), 0, 0);

    return;
    
}
