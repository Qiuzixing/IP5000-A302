/* Err Flags */
#define FLAG_AESTEST_FAIL  		0x00000001
#define FLAG_RC4TEST_FAIL  		0x00000002
#define FLAG_HASHTEST_FAIL  		0x00000004

/* Specific */
/*
#define DRAM_BASE			0x40000000
#define	CRYPTO_SRC_BASE			(DRAM_BASE + 0x100000)
#define	CRYPTO_DST_BASE			(DRAM_BASE + 0x200000)
#define	CRYPTO_CONTEXT_BASE		(DRAM_BASE + 0x300000)

#define	HASH_SRC_BASE			(DRAM_BASE + 0x400000)
#define	HASH_DST_BASE			(DRAM_BASE + 0x500000)
#define	HMAC_KEY_BASE			(DRAM_BASE + 0x600000)
*/
#define m08byteAlignment(x)		((x + 0x00000007) & 0xFFFFFFF8)
#define m16byteAlignment(x)		((x + 0x0000000F) & 0xFFFFFFF0)
#define m64byteAlignment(x)		((x + 0x0000003F) & 0xFFFFFFC0)

#define CRYPTO_ALIGNMENT		16
#define CRYPTO_MAX_SRC			(100+CRYPTO_ALIGNMENT)
#define CRYPTO_MAX_DST			(100+CRYPTO_ALIGNMENT)
#define CRYPTO_MAX_CONTEXT		(100+CRYPTO_ALIGNMENT)

#define HASH_ALIGNMENT			16
#define HMAC_KEY_ALIGNMENT	 	64
#define HASH_MAX_SRC			(100+HASH_ALIGNMENT)
#define HASH_MAX_DST			(32+HASH_ALIGNMENT)
#define HMAC_MAX_KEY			(64+HMAC_KEY_ALIGNMENT)

/* General */
#define HAC_REG_BASE 			0x1e6e3000

#define MAX_KEYLENGTH			100
#define MAX_TEXTLENGTH			100
#define MAX_AESTEXTLENGTH		256
#define MAX_RC4TEXTLENGTH		256
#define MAX_RC4KEYLENGTH		256

#define CRYPTOMODE_ECB			0x00
#define CRYPTOMODE_CBC			0x01
#define CRYPTOMODE_CFB			0x02
#define CRYPTOMODE_OFB			0x03
#define CRYPTOMODE_CTR			0x04

#define HASHMODE_MD5			0x00
#define HASHMODE_SHA1			0x01
#define HASHMODE_SHA256			0x02
#define HASHMODE_SHA224			0x03

#define MIXMODE_DISABLE                 0x00
#define MIXMODE_CRYPTO                  0x02
#define MIXMODE_HASH                    0x03

#define REG_CRYPTO_SRC_BASE_OFFSET	0x00
#define REG_CRYPTO_DST_BASE_OFFSET	0x04
#define REG_CRYPTO_CONTEXT_BASE_OFFSET	0x08
#define REG_CRYPTO_LEN_OFFSET		0x0C
#define REG_CRYPTO_CMD_BASE_OFFSET	0x10
//#define REG_CRYPTO_ENABLE_OFFSET	0x14
#define REG_CRYPTO_STATUS_OFFSET	0x1C

#define REG_HASH_SRC_BASE_OFFSET	0x20
#define REG_HASH_DST_BASE_OFFSET	0x24
#define REG_HASH_KEY_BASE_OFFSET	0x28
#define REG_HASH_LEN_OFFSET		0x2C
#define REG_HASH_CMD_OFFSET		0x30
//#define REG_HASH_ENABLE_OFFSET		0x14
#define REG_HASH_STATUS_OFFSET		0x1C

#define HASH_BUSY			0x01
#define CRYPTO_BUSY			0x02

//#define ENABLE_HASH			0x01
//#define DISABLE_HASH			0x00
//#define ENABLE_CRYPTO			0x02
//#define DISABLE_CRYPTO			0x00

#define CRYPTO_SYNC_MODE_MASK		0x03
#define CRYPTO_SYNC_MODE_ASYNC		0x00
#define CRYPTO_SYNC_MODE_PASSIVE	0x02
#define CRYPTO_SYNC_MODE_ACTIVE		0x03

#define CRYPTO_AES128			0x00
#define CRYPTO_AES192			0x04
#define CRYPTO_AES256			0x08

#define CRYPTO_AES_ECB			0x00
#define CRYPTO_AES_CBC			0x10
#define CRYPTO_AES_CFB			0x20
#define CRYPTO_AES_OFB			0x30
#define CRYPTO_AES_CTR			0x40

#define CRYPTO_ENCRYPTO			0x80
#define CRYPTO_DECRYPTO			0x00

#define CRYPTO_AES			0x000
#define CRYPTO_RC4			0x100

#define CRYPTO_ENABLE_RW		0x000
#define CRYPTO_ENABLE_CONTEXT_LOAD	0x000
#define CRYPTO_ENABLE_CONTEXT_SAVE	0x000

#define HASH_SYNC_MODE_MASK		0x03
#define HASH_SYNC_MODE_ASYNC		0x00
#define HASH_SYNC_MODE_PASSIVE		0x02
#define HASH_SYNC_MODE_ACTIVE		0x03

#define HASH_READ_SWAP_ENABLE		0x04
#define HMAC_SWAP_CONTROL_ENABLE	0x08

#define HASH_ALG_SELECT_MASK		0x70
#define HASH_ALG_SELECT_MD5		0x00
#define HASH_ALG_SELECT_SHA1		0x20
#define HASH_ALG_SELECT_SHA224		0x40
#define HASH_ALG_SELECT_SHA256		0x50

#define HAC_ENABLE			0x80
#define HAC_DIGEST_CAL_ENABLE		0x180
#define HASH_INT_ENABLE			0x200

/* AES */
#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 erk[64];     /* encryption round keys */
    uint32 drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
}
aes_context;

typedef struct
{
    int	  aes_mode;
    int	  key_length;
	
    uint8 key[32];	/* as iv in CTR mode */
    uint8 plaintext[64];
    uint8 ciphertext[64];
    
}
aes_test;

aes_test aestest[] = {
    { CRYPTOMODE_ECB, 128,
     {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c, '\0'},
     {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34, '\0'},
     {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32, '\0'} },
    {0xFF, 0xFF, NULL, NULL, NULL},		/* End Mark */
};

/* RC4 */
typedef struct
{
    uint8 key[32];
    uint8 data[64];
}
rc4_test;

rc4_test rc4test[] = {
    {{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, '\0'},
     {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, '\0'}}, 	
    {{0xff}, {0xff}},				/* End Mark */	
};

/* Hash */
typedef struct
{
    int	  hash_mode;
    int	  digest_length;
	
    uint8 input[64];
    uint8 digest[64];
    
}
hash_test;
	
hash_test hashtest[] = {
    {HASHMODE_SHA1, 20,
     "abc",
     {0x53, 0x20, 0xb0, 0x8c, 0xa1, 0xf5, 0x74, 0x62, 0x50, 0x71, 0x89, 0x41, 0xc5, 0x0a, 0xdf, 0x4e, 0xbb, 0x55, 0x76, 0x06, '\0'}},
    {0xFF, 0xFF, NULL, NULL},			/* End Mark */ 
};

/* HMAC */
typedef struct
{
    int	  hash_mode;
    int	  key_length;
    int	  digest_length;

    uint8 key[100];	
    uint8 input[64];
    uint8 digest[64];
    
}
hmac_test;
	
hmac_test hmactest[] = {
    {HASHMODE_SHA1, 64, 20,
     {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, '\0' },
     "Sample #1",
     {0xbf, 0x39, 0xda, 0xb1, 0x7d, 0xc2, 0xe1, 0x23, 0x0d, 0x28, 0x35, 0x3b, 0x8c, 0xcb, 0x14, 0xb6, 0x22, 0x02, 0x65, 0xb3, '\0'}},
    {0xFF, 0xFF, NULL, NULL, NULL},			/* End Mark */ 
};
		