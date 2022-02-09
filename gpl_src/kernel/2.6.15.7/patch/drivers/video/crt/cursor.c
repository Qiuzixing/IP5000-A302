/*
** cursor.c implements CRT's cursor over ip feature.
*/

#include <asm/io.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/util.h>
#include "astdebug.h"
#include "cursor_o_ip.h"
#include "cursor.h"
#include "network.h"

// ARGB4444 format
unsigned short test_pattern_8x8[] = {
	0x0FFF, 0x1FFF, 0x2FFF, 0x3777, 0x4777, 0x5777, 0x6777, 0x7888, 
	0x8FFF, 0xF000, 0xAFFF, 0xB777, 0xC777, 0xD777, 0xE777, 0xF888, 
	0x0FFF, 0x1FFF, 0x2FFF, 0x3FFF, 0x4777, 0x5777, 0x6777, 0x7888, 
	0x8FFF, 0x9FFF, 0xAFFF, 0xBFFF, 0xCFFF, 0xD777, 0xE777, 0xF888, 
	0x0FFF, 0x1FFF, 0x2FFF, 0x3FFF, 0x4FFF, 0x5FFF, 0x6FFF, 0x7888, 
	0x8FFF, 0x9FFF, 0xAFFF, 0xBFFF, 0xCFFF, 0xDFFF, 0xEFFF, 0xFFFF, 
	0x0FFF, 0x1FFF, 0x2777, 0x3FFF, 0x4FFF, 0x5FFF, 0x6FFF, 0x7FFF, 
	0x8FFF, 0x9777, 0xA777, 0xB777, 0xC777, 0xDFFF, 0xEFFF, 0xFFFF, 
};

unsigned short test_pattern1_8x8[] = {
	0xF777, 0xF777, 0xF777, 0xF777, 0xF777, 0xF777, 0xF777, 0xF777, 
	0xF777, 0xF000, 0xFFFF, 0xF777, 0xF777, 0xF777, 0xF777, 0xF888, 
	0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 0xF777, 0xF777, 0xF777, 0xF888, 
	0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF777, 0xF777, 0xF888, 
	0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF888, 
	0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xF777, 0xFFFF, 0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
	0xF777, 0xF777, 0xF777, 0xF777, 0xF777, 0xFFFF, 0xFFFF, 0xFFFF, 
};

// XRGB4444 format
unsigned short test_pattern_16x16[] = {
	0x8777, 0x8777, 0x8777, 0x8777, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x8777, 0xC888, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x8777, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x8777, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x8777, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x4777, 0x4FFF, 0x4FFF, 0x4FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x4FFF, 0x4FFF, 0x4FFF, 0x4FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0xCFFF, 0xCFFF, 0xCFFF, 0xCFFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0xCFFF, 0xCFFF, 0xCFFF, 0xCFFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0xCFFF, 0xCFFF, 0xCFFF, 0xCFFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0xCFFF, 0xCFFF, 0xCFFF, 0xCFFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
	0xCFFF, 0xCFFF, 0xCFFF, 0xCFFF, 0x4FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 0x8FFF, 
};


void csr_enable(struct s_crt_drv *crt, unsigned int enable)
{
	unsigned int ba_offset;
	struct s_csr_info *csr_info = crt->csr_info;

	CDBG("csr_enable(%d)\n", enable);
	BUG_ON(csr_info == NULL);

	//ToDo. Maybe we should validate current status before enable cursor
	if (crt->disp_select == CRT_2) {
		// Use CRT2
		ba_offset = 0x60;
	}
#if (CONFIG_AST1500_SOC_VER == 1)
	else if (crt->disp_select == CRT_1){
		// Use CRT1
		ba_offset = 0;
	}
#endif
	else {
		uerr("CRT's output port is not selected yet!\n");
		return;
	}

	// Disable both CRT1 and CRT2 cursor first as initialization
	// Output port may runtime switch, so turn both port off first makes life easier.
	ModSCUReg(CRT1_CONTROL_REG, HW_CURSOR_OFF, HW_CURSOR_ENABLE_MASK);
	ModSCUReg(CRT2_CONTROL_REG, HW_CURSOR_OFF, HW_CURSOR_ENABLE_MASK);
	csr_info->enable = HW_CURSOR_OFF;
	
	if (enable) {
		//Enable cursor
		ModSCUReg(CRT1_CONTROL_REG + ba_offset, HW_CURSOR_ON, HW_CURSOR_ENABLE_MASK);
		CDBG("enable(0x%08x)\n", GetSCUReg(CRT1_CONTROL_REG + ba_offset));
		csr_info->enable = HW_CURSOR_ON;
	}
}

// Return HW_CURSOR_ON or HW_CURSOR_OFF
unsigned int csr_is_enable(struct s_crt_drv *crt)
{
#if 1
	/* Due to the issue that chip's enable bit doesn't update right after setting,
	  we shadow this bit in driver level. */
	if (crt->csr_info == NULL)
		return HW_CURSOR_OFF;
	
	return crt->csr_info->enable;
#else
	unsigned int ba_offset;
	unsigned int ret;


	//ToDo. Maybe we should validate current status before enable cursor
	if (crt->disp_select == OUTPUT_SELECT_DIGITAL) {
		// Use CRT2
		ba_offset = 0x60;
	} else if (crt->disp_select == OUTPUT_SELECT_ANALOG){
		// Use CRT1
		ba_offset = 0;
	} else {
		uerr("CRT's output port is not selected yet!\n");
		return 0;
	}

	ret = GetSCUReg(CRT1_CONTROL_REG + ba_offset);

	// return 1 if enable, otherwise return 0.
	return (ret & HW_CURSOR_ENABLE_MASK);
#endif
}


static void csr_free_mem(struct s_crt_drv *crt)
{
	struct s_csr_info *csr_info;
	unsigned int i;

	csr_info = crt->csr_info;
	if (csr_info == NULL)
		return;

	crt->csr_info = NULL;

	if (csr_info->tmp64x64 != NULL) {
		kfree(csr_info->tmp64x64);
	}

	if (csr_info->csr_cache != NULL) {
		kfree(csr_info->csr_cache);
	}

	if (csr_info->csr_bitmap_cache != NULL) {
		for (i = 0; i < CSR_MAX_CACHE_NUM; i++) {
			if (csr_info->csr_bitmap_cache[i] != NULL)
				kfree(csr_info->csr_bitmap_cache[i]);		
		}

		kfree(csr_info->csr_bitmap_cache);
	}

	kfree(csr_info);
}

static void csr_allocate_mem(struct s_crt_drv *crt)
{
	struct s_csr_info *csr_info;
	unsigned int i;

	crt->csr_info = csr_info = kzalloc(sizeof(struct s_csr_info), GFP_KERNEL);
	if (csr_info == NULL) {
		uerr("Failed to allocate csr_info\n");
		goto fail;
	}
	//Allocate memory for cursor pattern cache
	csr_info->csr_bitmap_cache = kzalloc(CSR_MAX_CACHE_NUM * sizeof(unsigned char*), GFP_KERNEL);
	if (csr_info->csr_bitmap_cache == NULL) {
		uerr("Failed to allocate csr_bitmap_cache[]\n");
		goto fail;
	}
	for (i = 0; i < CSR_MAX_CACHE_NUM; i++) {
		// ToDo. Shell we use DMA helper function instead.
		csr_info->csr_bitmap_cache[i] = kzalloc(CSR_PATTERN_SIZE, GFP_KERNEL);
		if (csr_info->csr_bitmap_cache[i] == NULL) {
			uerr("Failed to allocate csr_bitmap_cache[%d][]\n", i);
			goto fail;
		}
	}

	// Allocate cache headers
	csr_info->csr_cache = kzalloc(CSR_MAX_CACHE_NUM * sizeof(struct s_csr_cache), GFP_KERNEL);
	if (csr_info->csr_cache == NULL) {
		uerr("Failed to allocate csr_cache\n");
		goto fail;
	}

	for (i = 0; i < CSR_MAX_CACHE_NUM; i++) {
		csr_info->csr_cache[i].bitmap = csr_info->csr_bitmap_cache[i];
	}

	csr_info->tmp64x64 = kzalloc(CSR_PATTERN_SIZE, GFP_KERNEL);
	if (csr_info->tmp64x64 == NULL) {
		uerr("Failed to allocate tmp64x64\n");
		goto fail;
	}
	return;

fail:
	csr_free_mem(crt);
}


void csr_dump_cache(struct s_crt_drv *crt)
{
	unsigned int i;
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cache *cache;

	if (csr_info == NULL) {
		uerr("coip function is not initialized yet\n");
		return;
	}

	for (i = 0; i < 2/*CSR_MAX_CACHE_NUM*/; i++) {
		cache = &csr_info->csr_cache[i];
		printk("cache[%d]----------\n", i);
		printk("hdr.index=%d, format=0x%08x, w=%d, h=%d, hot_x=%d, hot_y=%d, size=%d\n", 
			cache->header.index,
			cache->header.format,
			cache->header.width,
			cache->header.height,
			cache->header.hotspot_offset_x,
			cache->header.hotspot_offset_y,
			cache->header.bitmap_size);
		printk("ptn_offset_x=%d, ptn_offset_y=%d, bitmap=%p\n",
			cache->pattern_offset_x,
			cache->pattern_offset_y,
			cache->bitmap);
#if 0
	    for (i = 0; i< 64*64*2; i++) {
	        if (i%16 == 0)
	            printk("   ");
	        printk("%02x ", (unsigned char ) cache->bitmap[i]);
	        if (i%4 == 3)
	            printk("| ");
	        if (i%16 == 15)
	            printk("\n");
	    }
	    printk("\n");
#endif
		udump(cache->bitmap, 64*64*2);
	}
}


static void copy_pattern(
	struct s_csr_info *csr_info, 
	unsigned char *src, 
	struct s_csr_shape *header)
{
	struct s_csr_cache *cache;
	unsigned int i;
	unsigned char *ptr;
	
	cache = &(csr_info->csr_cache[header->index]);

	memcpy(&cache->header, header, sizeof(struct s_csr_shape));
	cache->pattern_offset_x = 64 - cache->header.width;
	cache->pattern_offset_y = 64 - cache->header.height;

	ptr = cache->bitmap;
	// To the start point to be copied
	ptr += cache->pattern_offset_y * 64 * 2 + cache->pattern_offset_x * 2;

	for (i = 0; i < cache->header.height; i++) {
		memcpy(ptr, src, cache->header.width * 2);
		ptr += 64 * 2;
		src += cache->header.width * 2;
	}
}

static void create_default_cursor(struct s_csr_info *csr_info)
{
	struct s_csr_shape header;

	header.index = 0;
	header.format = CSR_FORMAT_ARGB4444;
	header.width = 8;
	header.height = 8;
	header.hotspot_offset_x = 1;
	header.hotspot_offset_y = 1;
	header.bitmap_size = header.width * header.height * 2;
	copy_pattern(csr_info, (unsigned char*)test_pattern1_8x8, &header);


	header.index = 1;
	header.format = CSR_FORMAT_XRGB4444;
	header.width = 16;
	header.height = 16;
	header.hotspot_offset_x = 1;
	header.hotspot_offset_y = 1;
	header.bitmap_size = header.width * header.height * 2;
	copy_pattern(csr_info, (unsigned char*)test_pattern_16x16, &header);


}


int rx_CMD_CACHE_SHAPE(struct s_crt_drv *crt, unsigned int bytes)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cmd_cache_shape shape_hdr;
	unsigned int ret;

	CDBG("rx_CMD_CACHE_SHAPE(%d)\n", bytes);
	BUG_ON(csr_info == NULL);

	if (bytes < sizeof(shape_hdr)) {
		uerr("incorrect size!!%d but required %d\n", bytes, sizeof(shape_hdr));
		return sizeof(shape_hdr);
	}

again:
	ret = skt_xmit(0, csr_info->sock, (char *) &shape_hdr, sizeof(shape_hdr), 0);
	if (ret != sizeof(shape_hdr)) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv data, %d\n", ret);
		return ret;
	}

	bytes -= sizeof(shape_hdr);
	if (bytes != shape_hdr.shape.bitmap_size) {
		uerr("incorrect size!!%d but required %d\n", bytes, shape_hdr.shape.bitmap_size);
		return shape_hdr.shape.bitmap_size;
	}

again1:
	ret = skt_xmit(0, csr_info->sock, (char *)(csr_info->tmp64x64), bytes, 0);
	if (ret != bytes) {
		if (ret == -EFAULT)
			goto again1;
		uerr("recv data, %d\n", ret);
		return ret;
	}

	copy_pattern(csr_info, csr_info->tmp64x64, &shape_hdr.shape);

	// Update to screen if asked.
	if (shape_hdr.hotspot_x != -1) {
		csr_chg_shape(crt, shape_hdr.hotspot_x, 
		               shape_hdr.hotspot_y, 
		               shape_hdr.shape.index);
	}

	return 0;
}

int rx_CMD_CHANGE_SHAPE(struct s_crt_drv *crt, unsigned int bytes)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cmd_change_shape cmd;
	unsigned int ret;

	CDBG("rx_CMD_CHANGE_SHAPE(%d)\n", bytes);
	BUG_ON(csr_info == NULL);
	
	if (bytes != sizeof(cmd)) {
		uerr("incorrect size!!%d but required %d\n", bytes, sizeof(cmd));
		return sizeof(cmd);
	}
	
again:
	ret = skt_xmit(0, csr_info->sock, (char *) &cmd, bytes, 0);
	if (ret != bytes) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv data, %d\n", ret);
		return ret;
	}

	csr_chg_shape(crt, cmd.hotspot_x, cmd.hotspot_y, cmd.index);

	return 0;
}


int rx_CMD_ENABLE(struct s_crt_drv *crt, unsigned int bytes)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cmd_enable en;
	unsigned int ret;

	CDBG("rx_CMD_ENABLE(%d)\n", bytes);
	BUG_ON(csr_info == NULL);
	
	if (bytes != sizeof(en)) {
		uerr("incorrect size!!%d but required %d\n", bytes, sizeof(en));
		return sizeof(en);
	}
	
again:
	ret = skt_xmit(0, csr_info->sock, (char *) &en, sizeof(en), 0);
	if (ret != sizeof(en)) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv data, %d\n", ret);
		return ret;
	}

	csr_enable(crt, en.enable);

	return 0;
}


int rx_CMD_MOVE(struct s_crt_drv *crt, unsigned int bytes)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cmd_move cmd;
	unsigned int ret;

	CDBG("rx_CMD_MOVE(%d)\n", bytes);
	BUG_ON(csr_info == NULL);
	
	if (bytes != sizeof(cmd)) {
		uerr("incorrect size!!%d but required %d\n", bytes, sizeof(cmd));
		return sizeof(cmd);
	}
	
again:
	ret = skt_xmit(0, csr_info->sock, (char *) &cmd, bytes, 0);
	if (ret != bytes) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv data, %d\n", ret);
		return ret;
	}

	csr_move(crt, cmd.hotspot_x, cmd.hotspot_y);

	return 0;
}


void cmd_handler(struct s_crt_drv *crt)
{
	int ret;
	struct s_cursor_header hdr;
	struct s_csr_info *csr_info = crt->csr_info;

	CDBG("cmd_handler start\n");
	while (1) {
		// Receive header
		//memset(&hdr, 0, sizeof(hdr));
again:
		ret = skt_xmit(0, csr_info->sock, (char *) &hdr, sizeof(hdr), 0);
		if (ret != sizeof(hdr)) {
			if (ret == -EFAULT)
				goto again;
			uerr("recv a header, %d\n", ret);
			break;
		}
		// validate request
		if ((hdr.cmd_type & CURSOR_VER_MASK) != CURSOR_VER) {
			uerr("invalid version number (0x%08x)\n", hdr.cmd_type & CURSOR_VER_MASK);
			break;
		}
		
		// parse and execute command
		switch (hdr.cmd_type) {
		case CSR_CMD_ENABLE:
			ret = rx_CMD_ENABLE(crt, hdr.data_size);
			break;
		case CSR_CMD_CACHE_SHAPE:
			ret = rx_CMD_CACHE_SHAPE(crt, hdr.data_size);
			break;
		case CSR_CMD_CHANGE_SHAPE:
			ret = rx_CMD_CHANGE_SHAPE(crt, hdr.data_size);
			break;
		case CSR_CMD_MOVE:
			ret = rx_CMD_MOVE(crt, hdr.data_size);
			break;
		default:
			//ToDo. Data corruption recovery
			uerr("unknown cmd_type\n");
			ret = -1;
			break;
		}

		if (ret)
			break;
	}

//down:
	// Shutdown socket
	skt_shutdown(csr_info->sock);
	csr_info->sock = NULL;
	csr_enable(crt, 0);
	CDBG("cmd_handler stop\n");
}


void csr_initial(struct s_crt_drv *crt, struct socket *sock)
{
	struct s_csr_info *csr_info;
	//If sockfd == NULL, it will start as testing mode

	//Allocate memory
	if (crt->csr_info == NULL) {
		csr_allocate_mem(crt);
		if (crt->csr_info == NULL)
			BUG();
	}
	csr_info = crt->csr_info;

	if (csr_info->sock != NULL) {
		uerr("Re-initial coip function without stopping it first?!\n");
		BUG();
	}

	if (sock == NULL) {
		// Initial all cached cursor pattern under testing mode
		create_default_cursor(csr_info);
	} else {
		CDBG("kernel socket (%p)\n", sock);
		csr_info->sock = sock;
	}

	//init tcp receive thread
	if (csr_info->wq == NULL) {
		csr_info->wq = create_singlethread_workqueue("CoIP");
		INIT_WORK(&csr_info->cmd_work, (void (*)(void*))cmd_handler, crt);
	} else {
		PREPARE_WORK(&csr_info->cmd_work, (void (*)(void*))cmd_handler, crt);
	}

	// Start cmd_handler
	if (sock != NULL)
		queue_work(csr_info->wq, &csr_info->cmd_work);


}


static inline void cal_hw_offset_position(
	struct s_csr_cache *cache,
	unsigned int hotspot_x,
	unsigned int hotspot_y,
	unsigned int *hw_offset_x,
	unsigned int *hw_offset_y,
	unsigned int *hw_pos_x,
	unsigned int *hw_pos_y)
{
	// For x
	if (hotspot_x >= cache->header.hotspot_offset_x) {
		*hw_pos_x = hotspot_x - cache->header.hotspot_offset_x;
		*hw_offset_x = cache->pattern_offset_x;
	} else {
		*hw_pos_x = 0;
		*hw_offset_x = cache->pattern_offset_x + cache->header.hotspot_offset_x - hotspot_x;
	}

	// For y
	if (hotspot_y >= cache->header.hotspot_offset_y) {
		*hw_pos_y = hotspot_y - cache->header.hotspot_offset_y;
		*hw_offset_y = cache->pattern_offset_y;
	} else {
		*hw_pos_y = 0;
		*hw_offset_y = cache->pattern_offset_y + cache->header.hotspot_offset_y - hotspot_y;
	}

	*hw_pos_x &= 0xFFF;
	*hw_offset_x &= 0x3F;
	*hw_pos_y &= 0xFFF;
	*hw_offset_y &= 0x3F;
}


unsigned int csr_get_hw_offset(struct s_crt_drv *crt)
{
	unsigned int bao;

	if (crt->disp_select == CRT_2) {
		// Use CRT2
		bao = 0x60;
	}
#if (CONFIG_AST1500_SOC_VER == 1)
	else if (crt->disp_select == CRT_1){
		// Use CRT1
		bao = 0;
	}
#endif
	else {
		uerr("CRT's output port is not selected yet!\n");
		return 0;
	}

	return GetSCUReg(CRT1_CURSOR_OFFSET+ bao);
}


unsigned int csr_get_hw_pos(struct s_crt_drv *crt)
{
	unsigned int bao;

	if (crt->disp_select == CRT_2) {
		// Use CRT2
		bao = 0x60;
	}
#if (CONFIG_AST1500_SOC_VER == 1)
	else if (crt->disp_select == CRT_1){
		// Use CRT1
		bao = 0;
	}
#endif
	else {
		uerr("CRT's output port is not selected yet!\n");
		return 0;
	}

	return GetSCUReg(CRT1_CURSOR_POSITION+ bao);
}

static inline void _csr_mov(
	struct s_csr_cache *cache, 
	unsigned int x, 
	unsigned int y, 
	unsigned int bao)
{
	unsigned int offset_x, offset_y, pos_x, pos_y;

	// caculate offset and position
	cal_hw_offset_position(cache, x, y, &offset_x, &offset_y, &pos_x, &pos_y);

	// set cursor offset
	SetSCUReg(CRT1_CURSOR_OFFSET + bao, (offset_x | (offset_y << 8)));

	// set cursor position/kick 
	SetSCUReg(CRT1_CURSOR_POSITION + bao, (pos_x | (pos_y << 16)));
}

void csr_move(struct s_crt_drv *crt, unsigned int x, unsigned int y)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cache *cache;
	unsigned int bao; //base address offset
	unsigned int index;

	BUG_ON(csr_info == NULL);

	index = csr_info->cur_idx;
	cache = &(csr_info->csr_cache[index]);

	if (cache->header.index != index) {
		uerr("The cursor cache[%d] may not be valid\n", index);
		BUG();
	}

	if (crt->disp_select == CRT_2) {
		// Use CRT2
		bao = 0x60;
	}
#if (CONFIG_AST1500_SOC_VER == 1)
	else if (crt->disp_select == CRT_1){
		// Use CRT1
		bao = 0;
	}
#endif
	else {
		uerr("CRT's output port is not selected yet!\n");
		return;
	}

	_csr_mov(cache, x, y, bao);
}

void csr_chg_shape(struct s_crt_drv *crt, unsigned int x, unsigned int y, unsigned int index)
{
	struct s_csr_info *csr_info = crt->csr_info;
	struct s_csr_cache *cache;
	unsigned int bao; //base address offset

	BUG_ON(csr_info == NULL);

	cache = &(csr_info->csr_cache[index]);

	if (cache->header.index != index) {
		uerr("The cursor cache[%d] may not be valid\n", index);
		BUG();
	}

	if (crt->disp_select == CRT_2) {
		// Use CRT2
		bao = 0x60;
	}
#if (CONFIG_AST1500_SOC_VER == 1)
	else if (crt->disp_select == CRT_1){
		// Use CRT1
		bao = 0;
	}
#endif
	else {
		uerr("CRT's output port is not selected yet!\n");
		return;
	}

	// set cursor pattern format
	ModSCUReg(CRT1_CONTROL_REG + bao, 
		cache->header.format | csr_is_enable(crt), 
		CURSOR_PATTERN_FORMAT_MASK | HW_CURSOR_ENABLE_MASK);

	// set pattern base address
	SetSCUReg(CRT1_CURSOR_PATTERN + bao, virt_to_phys(cache->bitmap));

	_csr_mov(cache, x, y, bao);

	csr_info->cur_idx = index;
}

void csr_to_console_screen(struct s_crt_drv *crt)
{
	struct s_csr_info *csr_info = crt->csr_info;

	if (csr_info == NULL)
		return;

	//Disable cursor, no matter how.
#if (CONFIG_AST1500_SOC_VER == 1)
	ModSCUReg(CRT1_CONTROL_REG, HW_CURSOR_OFF, HW_CURSOR_ENABLE_MASK);
#endif
	ModSCUReg(CRT2_CONTROL_REG, HW_CURSOR_OFF, HW_CURSOR_ENABLE_MASK);
}


void csr_to_decode_screen(struct s_crt_drv *crt)
{
	struct s_csr_info *csr_info = crt->csr_info;

	if (csr_info == NULL)
		return;

	csr_enable(crt, csr_info->enable);
}

