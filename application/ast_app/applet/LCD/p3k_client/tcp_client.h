
#ifndef __TCP_CLIENT_H_
#define __TCP_CLIENT_H_

#ifdef __cplusplus
extern "C"{
#endif

int tcp_client_init(char *ip, int port);
int send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int size);
	int tcp_client_deinit();


#ifdef __cplusplus
}
#endif

#endif


