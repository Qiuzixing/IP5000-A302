#ifndef __OSCOND_H__
#define __OSCOND_H__

#include <pthread.h>

#include "mutex.h"

class CCond
{
public:
    CCond();
    ~CCond();

    inline void     Signal();
    inline void     Wait(CMutex* i_pMutex, SInt32 i_s32TimeoutInMilSecs = 0);
    inline void     Broadcast();

private:

    pthread_cond_t 	m_stCondition;
    void           	TimedWait(CMutex* i_pMutex, SInt32 i_s32TimeoutInMilSecs);
};

inline void CCond::Wait(CMutex* i_pMutex, SInt32 i_s32TimeoutInMilSecs)
{
    this->TimedWait(i_pMutex, i_s32TimeoutInMilSecs);
}

inline void CCond::Signal()
{
    pthread_cond_signal(&m_stCondition);
}

inline void CCond::Broadcast()
{
    pthread_cond_broadcast(&m_stCondition);
}

#endif //__OSCOND_H__
