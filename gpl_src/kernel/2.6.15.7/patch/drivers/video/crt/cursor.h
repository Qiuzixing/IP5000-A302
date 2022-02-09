#ifndef __CURSOR_H__
#define __CURSOR_H__

#include <linux/tcp.h>
#include <linux/workqueue.h>
#include "cursor_o_ip.h"
#include <asm/arch/drivers/crt.h>

#define CSR_PATTERN_SIZE  (64*64*2) // 64 pixel * 64 pixel * 2 bytes per pixel = 8KB

struct s_csr_cache
{
	struct s_csr_shape header;
	unsigned int pattern_offset_x; // = 64 - header.width
	unsigned int pattern_offset_y; // = 64 - header.height
	unsigned char *bitmap;
};



struct s_csr_info
{
	/* Due to the issue that chip's enable bit doesn't update right after setting,
	  we shadow this bit in driver level. */
	unsigned int enable; // HW_CURSOR_ON / HW_CURSOR_OFF
	//allocated memory for cursor pattern
	unsigned char **csr_bitmap_cache;
	// cursor shape cache information
	struct s_csr_cache *csr_cache;
	unsigned int cur_idx;
	struct workqueue_struct *wq;
	struct work_struct cmd_work;
	struct socket *sock;
	unsigned char *tmp64x64;
};

void csr_initial(struct s_crt_drv *crt, struct socket *sock);
void csr_enable(struct s_crt_drv *crt, unsigned int enable);
unsigned int csr_is_enable(struct s_crt_drv *crt);
void csr_chg_shape(struct s_crt_drv *crt, unsigned int x, unsigned int y, unsigned int index);
void csr_move(struct s_crt_drv *crt, unsigned int x, unsigned int y);
unsigned int csr_get_hw_pos(struct s_crt_drv *crt);
unsigned int csr_get_hw_offset(struct s_crt_drv *crt);
unsigned int csr_is_enable(struct s_crt_drv *crt);
void csr_enable(struct s_crt_drv *crt, unsigned int enable);
void csr_dump_cache(struct s_crt_drv *crt);
void csr_to_console_screen(struct s_crt_drv *crt);
void csr_to_decode_screen(struct s_crt_drv *crt);


#define CDBG(fmt, args...) \
	do { \
		if (crt->debug_flag & CDBG_INFO) \
			printk(KERN_INFO "crt: " fmt , ## args); \
	} while(0)


#endif //#ifndef __CURSOR_H__
