#ifndef _AST_AES_H_
#define _AST_AES_H_

#define AST_AES_USER_HDCP	0

#define AST_AES_BLOCK_MODE_ECB	0
#define AST_AES_BLOCK_MODE_CBC	1
#define AST_AES_BLOCK_MODE_CFB	2
#define AST_AES_BLOCK_MODE_OFB	3
#define AST_AES_BLOCK_MODE_CTR	4

#define AST_AES_IV_LENGTH	16
#define AST_AES_EXPANDED_KEY_LENGTH_128	176
#define AST_AES_EXPANDED_KEY_LENGTH_192	208
#define AST_AES_EXPANDED_KEY_LENGTH_256	240

#define AST_AES_STATUS_SUCCESSS	0
#define AST_AES_STATUS_INTERRUPTED	1
#define AST_AES_STATUS_BUSY	2
#define AST_AES_STATUS_TIMEOUT	3
#define AST_AES_STATUS_NO_CONTEXT	4

unsigned int Register_AST_AES_Context(unsigned int idx, unsigned char block_mode);
void Deregister_AST_AES_Context(unsigned int idx);
//These routines can only be invoked in a nonatomic context to protect against reentry.
unsigned int Setup_AST_AES_Context(unsigned int idx, unsigned char *key, unsigned int key_length, unsigned char* IV, unsigned int IV_length);
unsigned int Reset_AST_AES_Context(unsigned int idx);
#define AST_AES_FLAG_WAIT	0x0001
#define AST_AES_FLAG_GET	0x0010
#define AST_AES_FLAG_SET	0x0020
#define AST_AES_FLAG_LOAD	0x0100
#define AST_AES_FLAG_SAVE	0x0200
unsigned int AST_AES_Acquire(unsigned int idx, unsigned short flags, unsigned char *IV, unsigned char IV_length);
void AST_AES_Release(void);
unsigned int AST_AES_Encrypt(unsigned char *plaintext, unsigned int length, unsigned char *ciphertext, unsigned short flags);
unsigned int AST_AES_Decrypt(unsigned char *ciphertext, unsigned int length, unsigned char *plaintext, unsigned short flags);

#endif//#ifndef _AST_AES_H_
