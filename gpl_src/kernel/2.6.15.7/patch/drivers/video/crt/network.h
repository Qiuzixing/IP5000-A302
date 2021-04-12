#ifndef __COIP_NETWORK_H__
#define __COIP_NETWORK_H__

#include <linux/tcp.h>

void skt_shutdown(struct socket *tcp_socket);
int skt_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags);

#endif //#ifndef __COIP_NETWORK_H__

