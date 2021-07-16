#ifndef UTILITIES_H
#define UTILITIES_H


/* Create server socket name:/tmp/mainswitch */
int create_unixsocket(const char *name);

/* send message to socket "name" */
ssize_t unixsock_send_message(int sfd, char *name, const char *buf, size_t msglen);

#endif // UTILITIES_H
