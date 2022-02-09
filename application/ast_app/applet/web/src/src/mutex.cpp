/*
 * mutex.cpp
 *
 *  Created on: 2013Äê11ÔÂ20ÈÕ
 *      Author: sid
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mutex.h"

pthread_once_t          g_stMutexAttrInit = PTHREAD_ONCE_INIT;
pthread_mutexattr_t*    g_pMutexAttr = NULL;

void InitMutexAttr()
{
    g_pMutexAttr = (pthread_mutexattr_t*)malloc(sizeof(pthread_mutexattr_t));
    memset(g_pMutexAttr, 0, sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(g_pMutexAttr);
}

CMutex::CMutex():
m_stHolder(0),
m_u32HolderCount(0)
{
    assert(0 == pthread_once(&g_stMutexAttrInit, InitMutexAttr));
    assert(0 == pthread_mutex_init(&m_stMutex, g_pMutexAttr));
}

CMutex::~CMutex()
{
    assert(0 == m_u32HolderCount);
    pthread_mutex_destroy(&m_stMutex);
}

void CMutex::RecursiveLock()
{
    if (pthread_self() == m_stHolder)
    {
        m_u32HolderCount++;
        return;
    }
    pthread_mutex_lock(&m_stMutex);
    assert(0 == m_stHolder);
    m_stHolder = pthread_self();
    m_u32HolderCount++;
    assert(1 == m_u32HolderCount);
}

void CMutex::RecursiveUnlock()
{
    assert(pthread_self() == m_stHolder);
    assert(m_u32HolderCount > 0);
    m_u32HolderCount--;
    if (m_u32HolderCount == 0)
    {
        m_stHolder = 0;
        pthread_mutex_unlock(&m_stMutex);
    }
}

Bool CMutex::RecursiveTryLock()
{
    if (pthread_self() == m_stHolder)
    {
        m_u32HolderCount++;
        return TRUE;
    }
    SInt32 s32Err = pthread_mutex_trylock(&m_stMutex);
    if (s32Err != 0)
    {
        return FALSE;
    }

    assert(m_stHolder == 0);
    m_stHolder = pthread_self();
    m_u32HolderCount++;
    assert(m_u32HolderCount == 1);
    return TRUE;
}
