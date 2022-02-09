
#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__


int udp_init();
int udp_bind(int fd, char *addr, int port);
int send_event(int fd, char *addr, int port);
int recv_event(int fd, char *recv_buf);

#endif

