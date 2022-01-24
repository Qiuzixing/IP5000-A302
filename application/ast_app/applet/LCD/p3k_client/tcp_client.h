
#ifndef __TCP_CLIENT_H_
#define __TCP_CLIENT_H_

#ifdef __cplusplus
extern "C"{
#endif

int tcp_client_init(char *ip, int port);
int old_send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int size);
int send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int recv_size, char *head, int head_size);

int tcp_client_deinit();


#ifdef __cplusplus
}
#endif

#endif


