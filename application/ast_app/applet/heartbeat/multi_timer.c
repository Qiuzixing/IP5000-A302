#if 0
Reference to http://www.graphics-muse.org/wp/?p=868

The man page for timer_create() (used in applications, not in kernel code) gives a useful example of how to create and start high resolution timers.  However, it fails to mention that if you use multiple timers with a single signal then you need have a single signal handler that can identify which timer just went off and pass control to the timer-specific handler.

The way to do this is as follows.  Start with a function that creates and starts timers, makeTimer().

static int
makeTimer( char *name, timer_t *timerID, int expireMS, int intervalMS )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        fprintf(stderr, "%s: Failed to setup signal handling for %s.\n", PROG, name);
        return(-1);
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = intervalMS * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);

    return(0);
}

The function takes a pointer to a timer_t variable that will be filled with the timer ID created by timer_create().  This pointer is also saved in the sival_ptr variable right before calling timer_create().  In this function notice that we always use the SIGRTMIN signal, so expiration of any timer causes this signal to be raised.  The signal handler I've written for that signal is timerHandler.

static void
timerHandler( int sig, siginfo_t *si, void *uc )
{
    timer_t *tidp;
    tidp = si->si_value.sival_ptr;

    if ( *tidp == firstTimerID )
        firstCB(sig, si, uc);
    else if ( *tidp == secondTimerID )
        secondCB(sig, si, uc);
    else if ( *tidp == thirdTimerID )
        thirdCB(sig, si, uc);
}

The handler checks that the value stored in sival_ptr matches a given timerID variable.  The sival_ptr is the same as the one we set in makeTimer(), though here it lives in a different structure.  Obviously, it got copied from there to here on the way to this signal handler.  The point is that the timerID is what is used to determine which timer just went off and determine what to do next.

All that's left is to create a timer.  Here's an example.

timer_t firstTimerID;

static int
srtSchedule( void )
{
    int rc;
    rc = makeTimer("First Timer", &firstTimerID, 40, 40);
    return rc;
}

This schedules a timer to go off every 40 milliseconds.  On my board (an embedded PowerPC-based board) the high resolution timers have a resolution of 4ms (see clock_getres()) so 40ms shouldn't be a problem.  The callback associated with this timer, firstCB(), is not shown here because it's specific to the application.

There is a lot of other stuff you can do with these timers.  You can check if the time overrun should cause an adjustment in the timer.  That helps sync with the real time clock.  Also, as with interrupts in the kernel, you should keep signal handlers fast and efficient.  Don't do anything that takes a very long time to complete.  Typically giving a thread a kick to do something should be sufficient.  But that really depends on your application.
#endif
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "multi_timer.h"

static void timer_handler(int sig, siginfo_t *si, void *uc)
{
	struct mtimer_s *mtimer;

	//printf("multi timer works!\n");
	mtimer = (struct mtimer_s *)si->si_value.sival_ptr;
	/*
	** Bruce161201.
	** Even after timer was deleted, a pending timer event may still get fired.
	** We use mtimer->ref_cnt here to make sure bogus timer event will be ignored.
	*/
	if (!mtimer->ref_cnt) {
		dbg("timer was deleted!?");
		return;
	}

	mtimer->fired++;

	if (mtimer->handler)
		(mtimer->handler)(mtimer->priv_data);
}

void start_timer(struct mtimer_s *mtimer, int expireMS, int intervalMS)
{
	struct itimerspec its;

	if (!mtimer->ref_cnt) {
		dbg("starting a timer without creating it first?!\n");
		return;
	}

	mtimer->fired = 0;
	/* NOTE! Must convert large MS value into Sec value, otherwise the timer won't work. */
	its.it_interval.tv_sec = intervalMS / 1000;
	its.it_interval.tv_nsec = (intervalMS % 1000) * 1000000;
	its.it_value.tv_sec = expireMS / 1000;
	its.it_value.tv_nsec = (expireMS % 1000) * 1000000;
	/* timer_settime() will restart the timer if it already started. */
	if (timer_settime(mtimer->timer_id, 0, &its, NULL) < 0) {
		perror("timer_settime failed");
	}
}

void stop_timer(struct mtimer_s *mtimer)
{
	if (mtimer->ref_cnt)
		start_timer(mtimer, 0, 0);
}

int create_timer(struct mtimer_s *mtimer, mtimer_handler_func func, void *priv_data)
{
	struct sigevent         te;
	struct sigaction        sa;
	int                     sigNo = SIGRTMIN/*SIGALRM*/;
	timer_t *timerID = &mtimer->timer_id;

	/* Set up signal handler. */
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(sigNo, &sa, NULL) == -1) {
		return -1;
	}

	/* Set and enable alarm */
	te.sigev_notify = SIGEV_SIGNAL;
	te.sigev_signo = sigNo;
	te.sigev_value.sival_ptr = mtimer;
	timer_create(CLOCK_REALTIME, &te, timerID);
	/* mtimer->timer_id will be unique serial int starting from 0 */

	mtimer->handler = func;
	mtimer->priv_data = priv_data;
	mtimer->fired = 0;
	mtimer->ref_cnt++;
	if (mtimer->ref_cnt > 1)
		dbg("timer is created twice?!");

	return 0;
}

int delete_timer(struct mtimer_s *mtimer)
{
	if (mtimer->ref_cnt) {
		/* timer_delete will disarm armed timer before being deleted. */
		timer_delete(mtimer->timer_id);
	}

	memset(mtimer, 0, sizeof(struct mtimer_s));
	return 0;
}

#ifdef MULTI_TIMER_USAGE_SAMPLE
void use_timer(void)
{
	struct mtimer_s timer;

	timer.handler = on_timeup;
	timer.priv_data = (void *)100;
	create_timer(&timer);
	/* First time up after 100ms and then 50ms interval */
	start_timer(&timer, 100, 50);
	/* Restart timer */
	start_timer(&timer, 100, 0);
	stop_timer(&timer);
	delete_timer(&timer);
}
#endif

#ifdef SIMPLE_TIMER_EXAMPLE
static void alarm_handler(int signum)
{
	printf ("Timer fired!\n");
}

void simple_timer(long msec)
{
	struct itimerval timeout;

	timeout.it_value.tv_sec = 0;
	timeout.it_value.tv_usec = msec * 1000;
	timeout.it_interval.tv_sec = 0;
	timeout.it_interval.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &timeout, NULL)) {
		perror ("setitimer");
		return;
	}
}

void sample_call(void)
{
	/* initial */
	signal(SIGALRM, alarm_handler);
	/* fire timer. 50 msec. */
	simple_timer(50);
}
#endif
