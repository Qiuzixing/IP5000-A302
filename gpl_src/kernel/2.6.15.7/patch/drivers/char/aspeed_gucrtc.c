#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/rtc.h>
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/arch/platform.h>
#include <asm/arch/vic.h>


#define RTC_BASE_VA		ASPEED_RTC_VA_BASE

#define RTC_CNTR_STS		(RTC_BASE_VA + 0x00)
#define RTC_ALARM		(RTC_BASE_VA + 0x04)
#define RTC_RELOAD		(RTC_BASE_VA + 0x08)
#define RTC_CONTROL		(RTC_BASE_VA + 0x0C)
#define RTC_RESTART		(RTC_BASE_VA + 0x10)
#define RTC_RESET		(RTC_BASE_VA + 0x14)


#define UMVP_READ_REG(r)		(*((volatile unsigned int *) (r)))
#define UMVP_WRITE_REG(r,v)		(*((volatile unsigned int *) (r)) = ((unsigned int)   (v)))

#define DAY_VAL(r)	((r & 0xfffe0000) >> 17)
#define HOUR_VAL(r)	((r & 0x0001f000) >> 12)
#define MIN_VAL(r)	((r & 0x00000fc0) >>  6)
#define SEC_VAL(r)	((r & 0x0000003f) 		)

#define RTC_IS_OPEN	   0x01
#define UMVP_RTC_MAJOR 77
#define UMVP_RTC_DEV_NAME "rtc"


//static unsigned long periodic_frequency; not used currently
static unsigned long rtc_status = 0;
static unsigned long rtc_irq_sec_happen = 0;
static unsigned long rtc_irq_alarm_happen = 0;
DECLARE_WAIT_QUEUE_HEAD (rtc_wait_queue);
extern spinlock_t rtc_lock;


static void rtc_set_current_time(unsigned int nDDDD, unsigned int nHH,
							unsigned int nMM, unsigned int nSS);
static void rtc_set_alarm_time(unsigned int nHH, unsigned int nMM, unsigned int nSS);

static void rtc_set_current_time2(struct rtc_time *rtc_tm);
static void rtc_get_current_time2(struct rtc_time *rtc_tm);							

static int rtc_open(struct inode *inode, struct file *file)
{
	if(rtc_status & RTC_IS_OPEN)
		goto out_busy;

	rtc_status |= RTC_IS_OPEN;	/* not allow any other to open */

	rtc_irq_sec_happen = 0;
	rtc_irq_alarm_happen = 0;
	
	return 0;

out_busy:
	return -EBUSY;
}

static int rtc_release(struct inode *inode, struct file *file)
{
	rtc_status = 0; /* allow another to open */
	return (0);
}

static ssize_t rtc_read(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
	unsigned long data;
	ssize_t retval;
	
	if (count < sizeof(unsigned))
		return -EINVAL;

	__set_current_state(TASK_INTERRUPTIBLE);

	/*
	 * Note : Though this driver provides poll() for select()
	 *        system call to query whether or not this device
	 *        is ready for read, this read function shalln't block
	 *        until rtc_irq_sec_happen or rtc_irq_alarm_happen
	 *        gets TRUE. This is because the user application 
	 *        might not call select() system call before calling
	 *        fread() or read().
	 */
	while (1)
	{
		data = rtc_irq_sec_happen;
		spin_unlock_irq (&rtc_lock);
		if (data != 0)
		{
			rtc_irq_sec_happen = 0;
			break;
		}
		spin_lock_irq (&rtc_lock);
		
		data = rtc_irq_alarm_happen;
		spin_unlock_irq (&rtc_lock);
		if (data != 0)
		{
			rtc_irq_alarm_happen = 0;
			break;
		}
		spin_lock_irq (&rtc_lock);
	}

	if (count < sizeof(unsigned long))
		retval = put_user(data, (unsigned int __user *)buf) /*?: sizeof(int)*/; 
	else
		retval = put_user(data, (unsigned long __user *)buf) /*?: sizeof(long)*/;

	current->state = TASK_RUNNING;

	return retval;
	
}

static unsigned int rtc_poll(struct file *pFile, struct poll_table_struct *pTbl)
{
	if (rtc_irq_sec_happen || rtc_irq_alarm_happen)
    	return (POLLIN | POLLRDNORM); /* readable */

    poll_wait(pFile, &rtc_wait_queue, pTbl);

    return (POLLIN | POLLRDNORM); /* readable */
}

static int rtc_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
{
	register unsigned int s;
	
	switch (cmd) {
	case RTC_AIE_OFF:	/* alarm off */
		{
			disable_irq(INT_RTC_ALARM);
			return 0;
		}
	case RTC_AIE_ON:	/* alarm on */
		{
			enable_irq(INT_RTC_ALARM);
			return 0;
		}
	case RTC_UIE_OFF:	/* update off */
		{
			disable_irq(INT_RTC_SEC);
			return 0;
		}
	case RTC_UIE_ON:	/* update on */
		{
			enable_irq(INT_RTC_SEC);
			return 0;
		}
	case RTC_PIE_OFF:	/* periodic off */
		{
			/* no such function */
			return 0;
		}
	case RTC_PIE_ON:	/* periodic on */
		{
			/* no such function */
			return 0;
		}
	case RTC_ALM_READ:	/* read alarm */
		{
			s = UMVP_READ_REG(RTC_ALARM);
			//printk(" %02d:%02d:%02d\n", HOUR_VAL(s), MIN_VAL(s), SEC_VAL(s));
			break;
		}
	case RTC_ALM_SET:	/* set alarm */
		{
			unsigned int hrs, min, sec;
			struct rtc_time alm_tm;

			if (copy_from_user(&alm_tm, (struct rtc_time __user *)arg,
				   sizeof(struct rtc_time)))
			return -EFAULT;

			hrs = alm_tm.tm_hour;
			min = alm_tm.tm_min;
			sec = alm_tm.tm_sec;
			rtc_set_alarm_time(hrs, min, sec);
			return 0;
		}
	case RTC_RD_TIME:	/* read time */
		{
			struct rtc_time rtc_tm;

#if 0
			s = UMVP_READ_REG(RTC_CNTR_STS);

			rtc_tm.tm_year = 0;
			rtc_tm.tm_mon = 0;   /* tm_mon starts at zero */
			rtc_tm.tm_mday = DAY_VAL(s);
			rtc_tm.tm_hour = HOUR_VAL(s);
			rtc_tm.tm_min = MIN_VAL(s);
			rtc_tm.tm_sec = SEC_VAL(s);
#endif

			rtc_get_current_time2(&rtc_tm);

			s = copy_to_user((struct rtc_time __user *)arg,
							&rtc_tm,
							 sizeof(struct rtc_time));

			return s;
		}
	case RTC_SET_TIME:	/* set time */
		{
			struct rtc_time rtc_tm;
			//unsigned int yrs, mon, day, hrs, min, sec;
			if (copy_from_user(&rtc_tm, (struct rtc_time __user *)arg,
				   sizeof(struct rtc_time)))
			return -EFAULT;

			rtc_set_current_time2(&rtc_tm);
			
#if 0                        
			yrs = rtc_tm.tm_year + 1900;
			mon = rtc_tm.tm_mon + 1;   /* tm_mon starts at zero */
			day = rtc_tm.tm_mday;
			hrs = rtc_tm.tm_hour;
			min = rtc_tm.tm_min;
			sec = rtc_tm.tm_sec;
			rtc_set_current_time(day, hrs, min, sec);
#endif

			return 0;
		}
	case RTC_IRQP_READ:	/* read periodic alarm frequency */
		{
			//no such function!
			//return put_user(periodic_frequency, (unsigned long __user *)arg);
			break;
		}
	case RTC_IRQP_SET:	/* set periodic alarm frequency */
		{
			//no such function!
			return 0;
		}
	default:
			return -ENOTTY;
	}
		
	return 0;
}

	
static struct file_operations rtc_fops = {
	.open       = rtc_open,
	.release	= rtc_release,
	.read		= rtc_read,
	.poll		= rtc_poll,
	.ioctl		= rtc_ioctl,
};

static irqreturn_t rtc_isr(int irq, void *devid, struct pt_regs *regs)
{
	vic_clear_intr(irq);

	if (irq == INT_RTC_SEC)
		rtc_irq_sec_happen = 1;
	else if (irq == INT_RTC_ALARM)
		rtc_irq_alarm_happen = 1;

	wake_up_interruptible(&rtc_wait_queue); // wake up sleeping processes

	return (IRQ_HANDLED);
}

void rtc_set_alarm_time(unsigned int nHH, unsigned int nMM, unsigned int nSS)
{
	if (nHH < 24 && nMM < 60 && nSS < 60)
	{
		UMVP_WRITE_REG(RTC_ALARM, (nHH << 12)+(nMM << 6)+nSS);
	}
}

void rtc_set_current_time(unsigned int nDDDD, unsigned int nHH,
							unsigned int nMM, unsigned int nSS)
{
	unsigned int s1;

	UMVP_WRITE_REG(RTC_RELOAD, (nDDDD<<17) + (nHH<<12) + (nMM<<6) + nSS);
	UMVP_WRITE_REG(RTC_RESTART, 0x5A);
	do {
	   s1 = UMVP_READ_REG(RTC_CONTROL);
	} while (s1 & 0x20);
	
}

/* 
 * Support perpetual calendar
 */
#define TM_YEAR_BASE 1900

/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
# define __isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

const unsigned short int __yday[2] = { 365, 366 };
  
const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };
  
void rtc_set_current_time2(struct rtc_time *rtc_tm)
{
  	/* Get Parameters */	
  	unsigned int i, yrs, mon, day, hrs, min, sec;
  
  	yrs = rtc_tm->tm_year;
  	mon = rtc_tm->tm_mon;
  	day = rtc_tm->tm_mday;
  	hrs = rtc_tm->tm_hour;
  	min = rtc_tm->tm_min;
  	sec = rtc_tm->tm_sec;
				
  	/* Ensure that mon is in range, and set year accordingly.  */
  	int mon_remainder = mon % 12;
  	int negative_mon_remainder = mon_remainder < 0;
  	int mon_years = mon / 12 - negative_mon_remainder;
  	int year = yrs + mon_years;
  	int yday = 0;

  	/* The other values need not be in range:
     	   the remaining code handles minor overflows correctly,
     	   assuming int and time_t arithmetic wraps around.
     	   Major overflows are caught at the end.  */

  	/* Calculate day of year from year, month, and day of month.
     	   The result need not be in range.  */
  	for (i=0; i<year; i++)
            yday += __yday[__isleap (i + TM_YEAR_BASE)];
      
  	yday += ((__mon_yday[__isleap (year + TM_YEAR_BASE)]
	       [mon_remainder + 12 * negative_mon_remainder])
	       + day - 1);

	/* Set to H/W RTC */
   	rtc_set_current_time(yday, hrs, min, sec);	
}	

void rtc_get_current_time2(struct rtc_time *rtc_tm)
{
	register unsigned int s;
	unsigned int day, mday, yday;
	unsigned int yrs =0, mon = 11;
	
	/* Get from RTC Register */
	s = UMVP_READ_REG(RTC_CNTR_STS);
	day = DAY_VAL(s) + 1;
	
	/* Calculate year */
	do {
	    yday = __yday[__isleap (yrs + TM_YEAR_BASE)];
	    if (day >= yday)
	    {
	        day -= yday;
	        yrs++;
	    }    
	    else
	        break;
	        
	} while (1);
	
	/* Calculate mon */	
	do {
	    mday = __mon_yday[__isleap (yrs + TM_YEAR_BASE)][mon];
	    if (mday > day)
	        mon--;  
	    else
	    {
	    	day -= mday;
	        break;
	    }    
	        
	} while (1);
	
	rtc_tm->tm_year = yrs;
	rtc_tm->tm_mon = mon;
	rtc_tm->tm_mday = day;	
	rtc_tm->tm_hour = HOUR_VAL(s);
	rtc_tm->tm_min = MIN_VAL(s);
	rtc_tm->tm_sec = SEC_VAL(s);	
}

int __init rtc_init(void)
{
	int ttt=-ENOMEM;

	vic_set_intr_trigger(INT_RTC_SEC, vicc_edge_activeBoth);
	vic_set_intr_trigger(INT_RTC_DAY, vicc_edge_activeBoth);
	vic_set_intr_trigger(INT_RTC_HOUR, vicc_edge_activeBoth);
	vic_set_intr_trigger(INT_RTC_MIN, vicc_edge_activeBoth);
	vic_set_intr_trigger(INT_RTC_ALARM, vicc_edge_activeRaising);


	/* register ISR */
	if (request_irq(INT_RTC_SEC, rtc_isr, SA_INTERRUPT, "RTC_SEC", NULL))
	{
		printk("unable to register interrupt INT_RTC_SEC = %d\n", INT_RTC_SEC);
		return (-1);
	}

/*
	if (request_irq(INT_RTC_MIN, rtc_isr, SA_INTERRUPT, "RTC_MIN", NULL))
	{
		printk("unable to register interrupt INT_RTC_MIN = %d\n", INT_RTC_MIN);
		return (-1);
	}

	if (request_irq(INT_RTC_HOUR, rtc_isr, SA_INTERRUPT, "RTC_HOUR", NULL))
	{
		printk("unable to register interrupt INT_RTC_HOUR = %d\n", INT_RTC_HOUR);
		return (-1);
	}

	if (request_irq(INT_RTC_DAY, rtc_isr, SA_INTERRUPT, "RTC_DAY", NULL))
	{
		printk("unable to register interrupt INT_RTC_DAY = %d\n", INT_RTC_DAY);
		return (-1);
	}
*/

	if (request_irq(INT_RTC_ALARM, rtc_isr, SA_INTERRUPT, "RTC_ALARM", NULL))
	{
		printk("unable to register interrupt INT_RTC_ALARM = %d\n", INT_RTC_ALARM);
		return (-1);
	}


	/* start the RTC from dddd:hh:mm:ss = 0000:00:00:00 */
	UMVP_WRITE_REG(RTC_RESET, 0x99);
	rtc_set_current_time(0, 0, 0, 0);
	rtc_set_alarm_time(0, 0, 0);
	UMVP_WRITE_REG(RTC_CONTROL, 0x1f);


    /* device registration*/
	devfs_mk_cdev(MKDEV(UMVP_RTC_MAJOR,0),S_IFCHR|S_IRUSR|S_IWUSR,"rtc");
    ttt = register_chrdev(UMVP_RTC_MAJOR, "rtc", &rtc_fops);

	return (0);
}

void __exit rtc_exit(void)
{
    disable_irq(INT_RTC_SEC);
    free_irq(INT_RTC_SEC, NULL);
}

module_init(rtc_init);
module_exit(rtc_exit);

