#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#define LDAP_STATUS_OK              (0L)
#define LDAP_STATUS_DN_PASS_ERR     (0x31L)
#define LDAP_STATUS_CONN_ERR        (-1L)

void send_notify_msg(char *ip, int port, unsigned char status);

#endif // ifndef __NOTIFY_H__
