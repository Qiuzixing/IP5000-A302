#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "utilities.h"

/* Create unix domain socket etc: name:/tmp/mainswitch */
int create_unixsocket(const char *name)
{
    struct sockaddr_un svaddr;
    int sfd = -1;

    assert(NULL != name);
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0); /* Create server socket */
    if (sfd == -1) {
        //perror("socket.");
        goto FAIL;
    }

    /* Construct well-known address and bind server socket to it */
    if (remove(name) == -1 && errno != ENOENT) {
        //fprintf(stderr, "remove-%s", name);
        close(sfd);
        goto FAIL;
    }

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, name, sizeof(svaddr.sun_path) - 1);
    if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1) {
        //perror("bind.");
        close(sfd);
        goto FAIL;;
    }

    return sfd;

FAIL:
    return -1;
}

/* send message to socket "name" */
ssize_t unixsock_send_message(int sfd, char *name, const char *buf, size_t msglen)
{
    struct sockaddr_un svaddr;
    ssize_t sendbytes = 0;
    assert(NULL != name);
    /* Construct address of server */
    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, name, sizeof(svaddr.sun_path) - 1);

    sendbytes = sendto(sfd, buf, msglen, 0, (struct sockaddr *) &svaddr,
                       sizeof(struct sockaddr_un));
    return sendbytes;
}
