#include <sys/time.h>
#include <assert.h>

#include "cond.h"
#include "mutex.h"
#include "thread.h"

CCond::CCond()
{
    pthread_condattr_t stCondAttr;
    pthread_condattr_init(&stCondAttr);
#ifdef USING_CLOCK_MONOTONIC
    pthread_condattr_setclock(&stCondAttr, CLOCK_MONOTONIC);
#endif
    SInt32 ret = pthread_cond_init(&m_stCondition, &stCondAttr);
    assert(ret == 0);
}

CCond::~CCond()
{
    pthread_cond_destroy(&m_stCondition);
}

void CCond::TimedWait(CMutex* i_pMutex, SInt32 i_s32TimeoutInMilSecs)
{
    struct timespec stTimeSpec;
    struct timeval  stTimeValue;
    struct timezone stTimeZone;
    SInt32 s32Seconds;
    SInt32 s32MicroSeconds;

    // These platforms do refcounting manually, and wait will release the mutex,
    // so we need to update the counts here

    i_pMutex->m_u32HolderCount--;
    i_pMutex->m_stHolder = 0;

    if (i_s32TimeoutInMilSecs == 0)
    {
        (void)pthread_cond_wait(&m_stCondition, &i_pMutex->m_stMutex);
    }
    else
    {
#ifdef USING_CLOCK_MONOTONIC
        clock_gettime(CLOCK_MONOTONIC, &stTimeSpec);
        s32Seconds = i_s32TimeoutInMilSecs / 1000;
        i_s32TimeoutInMilSecs = i_s32TimeoutInMilSecs - (s32Seconds * 1000);
        assert(i_s32TimeoutInMilSecs < 1000);
        
        stTimeSpec.tv_sec += s32Seconds;
        stTimeSpec.tv_nsec += i_s32TimeoutInMilSecs*1000000;
#else
        gettimeofday(&stTimeValue, &stTimeZone);
        s32Seconds = i_s32TimeoutInMilSecs / 1000;
        i_s32TimeoutInMilSecs = i_s32TimeoutInMilSecs - (s32Seconds * 1000);
        i_s32TimeoutInMilSecs = ((i_s32TimeoutInMilSecs <= 10) ? 5 : (i_s32TimeoutInMilSecs-(i_s32TimeoutInMilSecs%10)-2)); // adjust for hisi kernel
        assert(i_s32TimeoutInMilSecs < 1000);
        s32MicroSeconds = i_s32TimeoutInMilSecs * 1000;
        assert(stTimeValue.tv_usec < 1000000);
        stTimeSpec.tv_sec = stTimeValue.tv_sec + s32Seconds;
        stTimeSpec.tv_nsec = (stTimeValue.tv_usec + s32MicroSeconds) * 1000;
        assert(stTimeSpec.tv_nsec < 2000000000);
        if (stTimeSpec.tv_nsec > 999999999)
        {
            stTimeSpec.tv_sec++;
            stTimeSpec.tv_nsec -= 1000000000;
        }
#endif
        (void)pthread_cond_timedwait(&m_stCondition, &i_pMutex->m_stMutex, &stTimeSpec);
    }

    i_pMutex->m_u32HolderCount++;
    i_pMutex->m_stHolder = pthread_self();
}

