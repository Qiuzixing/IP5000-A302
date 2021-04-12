#ifndef __CURSOR_O_IP_H__
#define __CURSOR_O_IP_H__


#define CURSOR_VER_1    (0xAD010000)
#define CURSOR_VER      (CURSOR_VER_1)
#define CURSOR_VER_MASK (0xFFFF0000)

#define CSR_MAX_CACHE_NUM 32 //index from 0 to 31

/* cmd_types used in  "struct s_cursor_header" */
#define CSR_CMD_ENABLE         (CURSOR_VER | 0x0001)
#define CSR_CMD_CACHE_SHAPE    (CURSOR_VER | 0x0002)
#define CSR_CMD_CHANGE_SHAPE   (CURSOR_VER | 0x0003)
#define CSR_CMD_MOVE           (CURSOR_VER | 0x0004)

/* cursor format */
#define CSR_FORMAT_XRGB4444    (0 << 10)
#define CSR_FORMAT_ARGB4444    (1 << 10)

struct s_cursor_header
{
    unsigned int cmd_type;
    unsigned int data_size;
    unsigned char data[];
} __attribute__ ((packed));

struct s_csr_shape
{
    unsigned int index; // The index which will be used to recall the cursor shape. Numbers from 0 to (CSR_MAX_CACHE_NUM-1).
    unsigned int format; //Will be CSR_FORMAT_XRGB4444 or CSR_FORMAT_ARGB4444
    unsigned int width; // width of this cursor shape
    unsigned int height; //height of this cursor shape
    unsigned int hotspot_offset_x; // x hot spot offset relative to left-top of the cursor shape
    unsigned int hotspot_offset_y; // y hot spot offset relative to left-top of the cursor shape
    unsigned int bitmap_size; //The size of this bipmap will be width*height*2 bytes
    unsigned char bitmap[]; //The size of this bipmap will be width*height*2 bytes
} __attribute__ ((packed));

struct s_csr_cmd_enable
{
    unsigned int enable; // 1 enable, 0 disable
} __attribute__ ((packed));

struct s_csr_cmd_cache_shape
{
    unsigned int hotspot_x; // -1 means just cache the shape without update screen cursor's shape and position.
    unsigned int hotspot_y;
    struct s_csr_shape shape;
} __attribute__ ((packed));

struct s_csr_cmd_change_shape
{
    unsigned int hotspot_x;
    unsigned int hotspot_y;
    unsigned int index; //The shape index to be changed to
} __attribute__ ((packed));

struct s_csr_cmd_move
{
    unsigned int hotspot_x;
    unsigned int hotspot_y;
} __attribute__ ((packed));


/*
** Programming Guide
*/
#if 0 //Here is the programming guide
- Every cursor over ip command sent from host must start with a "struct s_cursor_header". 
  The "data" field in header will be different per request command.
- To enable cursor over ip, CSR_CMD_ENABLE command must be used with ((struct s_csr_cmd_enable*)(s_cursor_header.data))->enable = 1;
- CSR_CMD_CACHE_SHAPE command is used to save and cache a cursor pattern. The saved cursor pattern can be recalled by "index".
  The optional "hot_spot_x" and "hot_spot_y" members indicate the position to be draw on the screen.
- CSR_CMD_CHANGE_SHAPE command is used to recall an existing cached cursor shape previously saved by CSR_CMD_CACHE_SHAPE command.
- CSR_CMD_MOVE command is used to move current cursor shape to the indicated position.
#endif

#endif //#ifndef __CURSOR_O_IP__
