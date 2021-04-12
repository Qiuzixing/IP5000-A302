#include <aspeed/features.h>

#if SUPPORT_HDCP_REPEATER

#include <linux/module.h>
#include <asm/semaphore.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/drivers/aes.h>
#include "hace_def.h"


static unsigned int	table_initialized = 0;
static u8 pow_tab[256];
static u8 log_tab[256];
static u8 sbx_tab[256];
static u8 isb_tab[256];
static u32 rco_tab[10];
static u32 ft_tab[4][256];
static u32 it_tab[4][256];
static u32 fl_tab[4][256];
static u32 il_tab[4][256];

typedef struct _AST_AES_Context_
{
    unsigned char	valid;
	unsigned char	block_mode;
	unsigned int	key_length;//16: 128 bits; 24: 192 bits; 32: 256 bits
	unsigned int	expanded_key_length;
	unsigned int	IV_length;
	unsigned char	*context_buffer_VA;
	unsigned char	*context_buffer_PA;
} AST_AES_Context, *pAST_AES_Context;
#define MAX_AES_USERS	1
static AST_AES_Context AES_Context_List[MAX_AES_USERS];

DECLARE_MUTEX(Lock);//AES engine lock
static unsigned char Current_Context_Idx = 0xFF;


static inline u8
byte(const u32 x, const unsigned n)
{
	return x >> (n << 3);
}
#define u32_in(x) le32_to_cpu(*(const u32 *)(x))

static inline u8 f_mult (u8 a, u8 b)
{
	u8 aa = log_tab[a], cc = aa + log_tab[b];

	return pow_tab[cc + (cc < aa ? 1 : 0)];
}
#define ff_mult(a,b)    (a && b ? f_mult(a, b) : 0)

#define ls_box(x)				\
    ( fl_tab[0][byte(x, 0)] ^			\
      fl_tab[1][byte(x, 1)] ^			\
      fl_tab[2][byte(x, 2)] ^			\
      fl_tab[3][byte(x, 3)] )
      
#define loop4(i)                                    \
{   t = ror32(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= round_keys[4 * i];     round_keys[4 * i + 4] = t;    \
    t ^= round_keys[4 * i + 1]; round_keys[4 * i + 5] = t;    \
    t ^= round_keys[4 * i + 2]; round_keys[4 * i + 6] = t;    \
    t ^= round_keys[4 * i + 3]; round_keys[4 * i + 7] = t;    \
}

#define loop6(i)                                    \
{   t = ror32(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= round_keys[6 * i];     round_keys[6 * i + 6] = t;    \
    t ^= round_keys[6 * i + 1]; round_keys[6 * i + 7] = t;    \
    t ^= round_keys[6 * i + 2]; round_keys[6 * i + 8] = t;    \
    t ^= round_keys[6 * i + 3]; round_keys[6 * i + 9] = t;    \
    t ^= round_keys[6 * i + 4]; round_keys[6 * i + 10] = t;   \
    t ^= round_keys[6 * i + 5]; round_keys[6 * i + 11] = t;   \
}

#define loop8(i)                                    \
{   t = ror32(t,  8); ; t = ls_box(t) ^ rco_tab[i];  \
    t ^= round_keys[8 * i];     round_keys[8 * i + 8] = t;    \
    t ^= round_keys[8 * i + 1]; round_keys[8 * i + 9] = t;    \
    t ^= round_keys[8 * i + 2]; round_keys[8 * i + 10] = t;   \
    t ^= round_keys[8 * i + 3]; round_keys[8 * i + 11] = t;   \
    t  = round_keys[8 * i + 4] ^ ls_box(t);    \
    round_keys[8 * i + 12] = t;                \
    t ^= round_keys[8 * i + 5]; round_keys[8 * i + 13] = t;   \
    t ^= round_keys[8 * i + 6]; round_keys[8 * i + 14] = t;   \
    t ^= round_keys[8 * i + 7]; round_keys[8 * i + 15] = t;   \
}

#define star_x(x) (((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x1b)
#define imix_col(y,x)       \
    u   = star_x(x);        \
    v   = star_x(u);        \
    w   = star_x(v);        \
    t   = w ^ (x);          \
   (y)  = u ^ v ^ w;        \
   (y) ^= ror32(u ^ t,  8) ^ \
          ror32(v ^ t, 16) ^ \
          ror32(t,24)

static void gen_tabs (void)
{
	u32 i, t;
	u8 p, q;

	/* log and power tables for GF(2**8) finite field with
	   0x011b as modular polynomial - the simplest primitive
	   root is 0x03, used here to generate the tables */

	for (i = 0, p = 1; i < 256; ++i) {
		pow_tab[i] = (u8) p;
		log_tab[p] = (u8) i;

		p ^= (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	log_tab[1] = 0;

	for (i = 0, p = 1; i < 10; ++i) {
		rco_tab[i] = p;

		p = (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	for (i = 0; i < 256; ++i) {
		p = (i ? pow_tab[255 - log_tab[i]] : 0);
		q = ((p >> 7) | (p << 1)) ^ ((p >> 6) | (p << 2));
		p ^= 0x63 ^ q ^ ((q >> 6) | (q << 2));
		sbx_tab[i] = p;
		isb_tab[p] = (u8) i;
	}

	for (i = 0; i < 256; ++i) {
		p = sbx_tab[i];

		t = p;
		fl_tab[0][i] = t;
		fl_tab[1][i] = rol32(t, 8);
		fl_tab[2][i] = rol32(t, 16);
		fl_tab[3][i] = rol32(t, 24);

		t = ((u32) ff_mult (2, p)) |
		    ((u32) p << 8) |
		    ((u32) p << 16) | ((u32) ff_mult (3, p) << 24);

		ft_tab[0][i] = t;
		ft_tab[1][i] = rol32(t, 8);
		ft_tab[2][i] = rol32(t, 16);
		ft_tab[3][i] = rol32(t, 24);

		p = isb_tab[i];

		t = p;
		il_tab[0][i] = t;
		il_tab[1][i] = rol32(t, 8);
		il_tab[2][i] = rol32(t, 16);
		il_tab[3][i] = rol32(t, 24);

		t = ((u32) ff_mult (14, p)) |
		    ((u32) ff_mult (9, p) << 8) |
		    ((u32) ff_mult (13, p) << 16) |
		    ((u32) ff_mult (11, p) << 24);

		it_tab[0][i] = t;
		it_tab[1][i] = rol32(t, 8);
		it_tab[2][i] = rol32(t, 16);
		it_tab[3][i] = rol32(t, 24);
	}
}

static void aes_set_key(unsigned char block_mode, const u8 *in_key, unsigned int key_len, unsigned char *round_keys)
{
	u32 i, t, u, v, w;

	round_keys[0] = u32_in (in_key);
	round_keys[1] = u32_in (in_key + 4);
	round_keys[2] = u32_in (in_key + 8);
	round_keys[3] = u32_in (in_key + 12);

	switch (key_len) {
	case 16:
		t = round_keys[3];
		for (i = 0; i < 10; ++i)
			loop4 (i);
		break;

	case 24:
		round_keys[4] = u32_in (in_key + 16);
		t = round_keys[5] = u32_in (in_key + 20);
		for (i = 0; i < 8; ++i)
			loop6 (i);
		break;

	case 32:
		round_keys[4] = u32_in (in_key + 16);
		round_keys[5] = u32_in (in_key + 20);
		round_keys[6] = u32_in (in_key + 24);
		t = round_keys[7] = u32_in (in_key + 28);
		for (i = 0; i < 7; ++i)
			loop8 (i);
		break;
	}

	if ((block_mode == AST_AES_BLOCK_MODE_ECB) || (block_mode == AST_AES_BLOCK_MODE_CBC))
	{
		for (i = 4; i < key_len + 24; ++i) {
			imix_col (round_keys[i], round_keys[i]);
		}
	}
}

unsigned int Register_AST_AES_Context(unsigned int idx, unsigned char block_mode)
{
	printk("Register_AST_AES_Context(%d,%d)\n", idx, block_mode);
	if ((IO_READ(SCU_BASE + SCU_RESET_CONTROL) & SCU_HACE_REST_CONTROL_MASK) || (IO_READ(SCU_BASE + SCU_CLOCK_CONTROL) & SCU_HACE_CLOCK_CONTROL_MASK))
	{
		unsigned long flags;
		printk("HACE not initialized yet!!!\n");
		local_irq_save(flags);
		if (!(IO_READ(SCU_BASE + SCU_PROTECTION_KEY) & 1))
		{
			printk("SCU is locked.\n");
			IO_WRITE(SCU_BASE + SCU_PROTECTION_KEY, 0x1688a8a8);
		}
		IO_WRITE((SCU_BASE + SCU_CLOCK_CONTROL), IO_READ(SCU_BASE + SCU_CLOCK_CONTROL) & ~(SCU_HACE_CLOCK_CONTROL_MASK | SCU_RESERVED_CLOCK_CONTROL_MASK));
		barrier();
		IO_WRITE((SCU_BASE + SCU_RESET_CONTROL), IO_READ(SCU_BASE + SCU_RESET_CONTROL) & ~SCU_HACE_REST_CONTROL_MASK);
		local_irq_restore(flags);
	}
	AES_Context_List[idx].context_buffer_VA = dma_alloc_coherent(NULL, AST_AES_IV_LENGTH + AST_AES_EXPANDED_KEY_LENGTH_256, (dma_addr_t *)&AES_Context_List[idx].context_buffer_PA, GFP_KERNEL);
	if (!AES_Context_List[idx].context_buffer_VA)
	{
		printk("failed to allocate AES context buffer\n");
		return AST_AES_STATUS_NO_CONTEXT;
	}
	printk("AES context buffer PA = %08X\n", (unsigned int)(AES_Context_List[idx].context_buffer_PA));
	if (((unsigned int)(AES_Context_List[idx].context_buffer_PA)) & 0x7)
	{
		printk("AES context buffer not aligned!!!\n");
		BUG();
	}
	AES_Context_List[idx].block_mode = block_mode;
	if (!table_initialized)
	{
		table_initialized = 1;
		gen_tabs();
	}
	return AST_AES_STATUS_SUCCESSS;
}
EXPORT_SYMBOL(Register_AST_AES_Context);

//Users must invoke Reset_AST_AES_Context() before Deregister_AST_AES_Context();
void Deregister_AST_AES_Context(unsigned int idx)
{
	if (AES_Context_List[idx].context_buffer_VA)
	{
		dma_free_coherent(NULL, AST_AES_IV_LENGTH + AST_AES_EXPANDED_KEY_LENGTH_256, AES_Context_List[idx].context_buffer_VA, (dma_addr_t)AES_Context_List[idx].context_buffer_PA);
		AES_Context_List[idx].context_buffer_VA = NULL;
	}
}
EXPORT_SYMBOL(Deregister_AST_AES_Context);

unsigned int Setup_AST_AES_Context(unsigned int idx, unsigned char *key, unsigned int key_length, unsigned char* IV, unsigned int IV_length)
{
	printk("--->Setup_AST_AES_Context\n");
	if (down_interruptible(&Lock))
		return AST_AES_STATUS_INTERRUPTED;
	if (AES_Context_List[idx].valid)
	{
		printk("context in use!!!\n");
		up(&Lock);
		return AST_AES_STATUS_NO_CONTEXT;
	}
	if (IV_length <= AST_AES_IV_LENGTH)
	{
		memcpy(AES_Context_List[idx].context_buffer_VA, IV, IV_length);
		memset(&AES_Context_List[idx].context_buffer_VA[IV_length], 0, AST_AES_IV_LENGTH - IV_length);
	}
	else
		memcpy(AES_Context_List[idx].context_buffer_VA, IV, AST_AES_IV_LENGTH);
	AES_Context_List[idx].IV_length = IV_length;
	//to do:expand key
	aes_set_key(AES_Context_List[idx].block_mode, key, key_length, &AES_Context_List[idx].context_buffer_VA[AST_AES_IV_LENGTH]);
	AES_Context_List[idx].key_length = key_length;
	switch (key_length)
	{
	case 16:
		AES_Context_List[idx].expanded_key_length = AST_AES_EXPANDED_KEY_LENGTH_128;
		break;
	case 24:
		AES_Context_List[idx].expanded_key_length = AST_AES_EXPANDED_KEY_LENGTH_192;
		break;
	case 32:
		AES_Context_List[idx].expanded_key_length = AST_AES_EXPANDED_KEY_LENGTH_256;
		break;
	default:
		printk("invalid AES key length (%u)\n", key_length);
		BUG();
	}
	AES_Context_List[idx].valid = 1;
	//force reload of current context
	if (Current_Context_Idx == idx)
		Current_Context_Idx = 0xFF;
	up(&Lock);
	{
		unsigned int i;
		printk("IV:\n");
		for (i = 0; i < AST_AES_IV_LENGTH; i++)
		{
			if ((i & 0xF) == 0xF)
				printk("%02X\n", AES_Context_List[idx].context_buffer_VA[i]);
			else
				printk("%02X ", AES_Context_List[idx].context_buffer_VA[i]);
		}
		printk("round keys:\n");
		for (i = 0; i < AES_Context_List[idx].expanded_key_length; i++)
		{
			if ((i & 0xF) == 0xF)
				printk("%02X\n", AES_Context_List[idx].context_buffer_VA[AST_AES_IV_LENGTH + i]);
			else
				printk("%02X ", AES_Context_List[idx].context_buffer_VA[AST_AES_IV_LENGTH + i]);
		}
		printk("\n");
	}
	printk("Setup_AST_AES_Context<---\n");
	return AST_AES_STATUS_SUCCESSS;
}
EXPORT_SYMBOL(Setup_AST_AES_Context);

unsigned int Reset_AST_AES_Context(unsigned int idx)
{
	printk("--->Reset_AST_AES_Context\n");
	if (down_interruptible(&Lock))
		return AST_AES_STATUS_INTERRUPTED;
	AES_Context_List[idx].valid = 0;
	up(&Lock);
	printk("Reset_AST_AES_Context<---\n");
	return AST_AES_STATUS_SUCCESSS;
}
EXPORT_SYMBOL(Reset_AST_AES_Context);

unsigned int AST_AES_Acquire(unsigned int idx, unsigned short flags, unsigned char *IV, unsigned char IV_length)
{
	if (flags & AST_AES_FLAG_WAIT)
	{
		if (down_interruptible(&Lock))
			return AST_AES_STATUS_INTERRUPTED;
	}
	else
	{
		if (down_trylock(&Lock))
			return AST_AES_STATUS_BUSY;
	}
	
	if (!AES_Context_List[idx].valid)
	{
		up(&Lock);
		return AST_AES_STATUS_NO_CONTEXT;
	}
	
	if (idx != Current_Context_Idx)
	{
		Current_Context_Idx = idx;
		//update new context base
		IO_WRITE(HACE_BASE + CRYPTO_CONTEXT_BUFFER_BASE, AES_Context_List[idx].context_buffer_PA);
		//load context
		{
			unsigned int command = AES_DES | DISABLE_SAVE_CONTEXT | ENABLE_LOAD_CONTEXT | DISABLE_DATA_OPERATION | AES_ENGINE;
			switch (AES_Context_List[idx].key_length)
			{
			case 16:
				command |= AES_KEY_LENGTH_128;
				break;
			case 24:
				command |= AES_KEY_LENGTH_192;
				break;
			case 32:
				command |= AES_KEY_LENGTH_256;
			}
			command |= (AES_Context_List[idx].block_mode << BLOCK_CIPHER_MODE_SELECT_SHIFT);
			IO_WRITE(HACE_BASE + CRYPTO_ENGINE_COMMAND, command);
		}
	}
	
	if (flags & AST_AES_FLAG_GET) 
	{
		memcpy(IV, &AES_Context_List[idx].context_buffer_VA[AST_AES_IV_LENGTH - IV_length], IV_length);
	}
	if (flags & AST_AES_FLAG_SET) 
	{
		memcpy(&AES_Context_List[idx].context_buffer_VA[AST_AES_IV_LENGTH - IV_length], IV, IV_length);
	}
	
	return AST_AES_STATUS_SUCCESSS;
}
EXPORT_SYMBOL(AST_AES_Acquire);

void AST_AES_Release(void)
{
	up(&Lock);
}
EXPORT_SYMBOL(AST_AES_Release);

//to do:
//1. Fine tune timeout value.

//For encryption, IV within a context is maintained by the AES engine.
unsigned int AST_AES_Encrypt(unsigned char *plaintext, unsigned int length, unsigned char *ciphertext, unsigned short flags)
{
	unsigned int i, timeout;
	unsigned int command = (IO_READ(HACE_BASE + CRYPTO_ENGINE_COMMAND) &
		(AES_KEY_LENGTH_SELECT_MASK | BLOCK_CIPHER_MODE_SELECT_MASK | CRYPTO_ALGORITHM_SELECT_MASK | CRYPTO_ENGINE_SELECT_MASK));
	command |= (DATA_ENCRYPTION | ENABLE_DATA_OPERATION);//do encryption; enable data operation.
	if (flags & AST_AES_FLAG_LOAD)
	{
		command |= ENABLE_LOAD_CONTEXT;//enable load context.
	}
	else
		command |= DISABLE_LOAD_CONTEXT;//disable load context.
	if (flags & AST_AES_FLAG_SAVE)
	{
		command |= ENABLE_SAVE_CONTEXT;//enable save context.
	}
	else
		command |= DISABLE_SAVE_CONTEXT;//disable save context.

	if (((unsigned int)plaintext) & 0x7)
	{
		printk("AES data buffer not aligned!!!\n");
		BUG();
	}
	IO_WRITE(HACE_BASE + CRYPTO_DATA_SOURCE_BASE, plaintext);
	IO_WRITE(HACE_BASE + CRYPTO_DATA_DESTINATION_BASE, ciphertext);
	IO_WRITE(HACE_BASE + CRYPTO_DATA_LENGTH_BASE, length);
	barrier();
	//fire
//	printk("%08X\n", command);
//	printk("%02X ", AES_Context_List[Current_Context_Idx].context_buffer_VA[AST_AES_IV_LENGTH - 1]);
	IO_WRITE(HACE_BASE + CRYPTO_ENGINE_COMMAND, command);
	timeout = length;
	for (i = 0; i < timeout; i++)
	{
#if 1
		if ((IO_READ(HACE_BASE + HAC_ENGINE_STATUS) & CRYPTO_ENGINE_STATUS_MASK) == CRYPTO_ENGINE_BUSY)
		{
			schedule();
		}
		else
			break;
#else
		if ((IO_READ(HACE_BASE + HAC_ENGINE_STATUS) & CRYPTO_ENGINE_STATUS_MASK) == CRYPTO_ENGINE_IDLE)
			break;
#endif
	}
	if (i == timeout)
	{
		printk("AST_AES_Encrypt timeouted (%u)\n", length);
		return AST_AES_STATUS_TIMEOUT;
	}
	else
	{
		return AST_AES_STATUS_SUCCESSS;
	}
}
EXPORT_SYMBOL(AST_AES_Encrypt);

unsigned int AST_AES_Decrypt(unsigned char *ciphertext, unsigned int length, unsigned char *plaintext, unsigned short flags)
{
	unsigned int i, timeout;
	unsigned int command = (IO_READ(HACE_BASE + CRYPTO_ENGINE_COMMAND) &
		(AES_KEY_LENGTH_SELECT_MASK | BLOCK_CIPHER_MODE_SELECT_MASK | CRYPTO_ALGORITHM_SELECT_MASK | CRYPTO_ENGINE_SELECT_MASK));
	command |= (DATA_DECRYPTION | ENABLE_DATA_OPERATION);//do decryption; enable data operation.
	if (flags & AST_AES_FLAG_LOAD)
	{
		command |= ENABLE_LOAD_CONTEXT;//enable load context.
	}
	else
		command |= DISABLE_LOAD_CONTEXT;//disable load context.
	if (flags & AST_AES_FLAG_SAVE)
	{
		command |= ENABLE_SAVE_CONTEXT;//enable save context.
	}
	else
		command |= DISABLE_SAVE_CONTEXT;//disable save context.

	if (((unsigned int)ciphertext) & 0x7)
	{
		printk("AES data buffer not aligned!!!\n");
		BUG();
	}
	IO_WRITE(HACE_BASE + CRYPTO_DATA_SOURCE_BASE, ciphertext);
	IO_WRITE(HACE_BASE + CRYPTO_DATA_DESTINATION_BASE, plaintext);
	IO_WRITE(HACE_BASE + CRYPTO_DATA_LENGTH_BASE, length);
	barrier();
	//fire
//	printk("%02X ", AES_Context_List[Current_Context_Idx].context_buffer_VA[AST_AES_IV_LENGTH - 1]);
	IO_WRITE(HACE_BASE + CRYPTO_ENGINE_COMMAND, command);
	timeout = length;
	for (i = 0; i < timeout; i++)
	{
#if 1
		if ((IO_READ(HACE_BASE + HAC_ENGINE_STATUS) & CRYPTO_ENGINE_STATUS_MASK) == CRYPTO_ENGINE_BUSY)
			schedule();
		else
			break;
#else
		if ((IO_READ(HACE_BASE + HAC_ENGINE_STATUS) & CRYPTO_ENGINE_STATUS_MASK) == CRYPTO_ENGINE_IDLE)
			break;
#endif
	}
	if (i == timeout)
	{
		printk("AST_AES_Decrypt timeouted (%u)\n", length);
		return AST_AES_STATUS_TIMEOUT;
	}
	else
	{
		return AST_AES_STATUS_SUCCESSS;
	}
}
EXPORT_SYMBOL(AST_AES_Decrypt);

#endif//#if SUPPORT_HDCP_REPEATER

