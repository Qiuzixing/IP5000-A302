#ifndef _AES_H_
#define _AES_H_

void EnableAESEncryption(u8 CompressMode, u8 IsAutoMode, u32 *key);
void aes_expand_key(unsigned char *expanded_key, unsigned char *key);

#endif /* #ifndef _AES_H_ */

