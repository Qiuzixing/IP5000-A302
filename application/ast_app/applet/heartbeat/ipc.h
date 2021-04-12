#ifndef _IPC_H_
#define _IPC_H_

int hb_ipc_set(const unsigned char *ipc_ch, unsigned char *ipc_value);
int hb_ipc_query(const unsigned char *ipc_ch, unsigned char *ipc_value);

#endif /* #ifndef _IPC_H_ */
