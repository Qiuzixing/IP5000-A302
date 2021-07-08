/*
 * mutex.h
 *
 *  Created on: 2013-11-20
 *      Author: sid
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

#include "types.h"

class CMutex
{
public:
    CMutex();
    ~CMutex();
    void    Lock()      { RecursiveLock(); }
    void    Unlock()    { RecursiveUnlock(); }
    Bool    TryLock() {  return RecursiveTryLock(); }

private:
    void    RecursiveLock();
    void    RecursiveUnlock();
    Bool    RecursiveTryLock();


    pthread_mutex_t m_stMutex;
    pthread_t       m_stHolder;
    UInt32          m_u32HolderCount;
    
    friend class CCond;
};

class CMutexLocker
{
public:
    CMutexLocker(CMutex* i_pMutex)
        : m_pMutex(i_pMutex)
    {
        if (m_pMutex != NULL)
        {
            m_pMutex->Lock();
        }
    }

    ~CMutexLocker()
    {
        if (m_pMutex != NULL)
        {
            m_pMutex->Unlock();
        }
        m_pMutex = NULL;
    }

private:
    CMutex* m_pMutex;
};

#endif /* MUTEX_H_ */
