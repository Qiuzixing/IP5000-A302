/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

#include "uds_ipc.h"
#include "debug.h"

#define MAX_PAYLOAD 8192

struct ipc_srv_s {
	int epfd;
	int set_fd;
	int set_listen_fd;
	int get_fd;
	int get_listen_fd;
	int query_fd;
	int query_listen_fd;
	int reply_fd;
	int reply_listen_fd;
	int is_waiting_for_reply;
	int is_waiting_for_set;
	int is_waiting_for_query;
};
typedef int (*event_handler_func)(struct ipc_srv_s *);
void on_set(struct ipc_srv_s *c);
void on_query(struct ipc_srv_s *c);

/* Start of global variables. */
struct ipc_srv_s the_srv;
unsigned char buffer[MAX_PAYLOAD];

void deliver_msg(struct ipc_srv_s *c)
{
	int nbytes;

	if (!c->get_fd || !c->set_fd) {
		err("[%d] un-expected ipc_server state!?\n", __LINE__);
		goto done;
	}
	nbytes = read(c->set_fd, buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		err("read failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	buffer[nbytes] = 0;

	nbytes = write(c->get_fd, buffer, nbytes);
	if (nbytes < 0) {
		err("write failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}

done:
	return;
}

void query_msg(struct ipc_srv_s *c)
{
	int nbytes;

	if (!c->query_fd || !c->get_fd) {
		err("[%d] un-expected ipc_server state!?\n", __LINE__);
		goto done;
	}
	nbytes = read(c->query_fd, buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		err("read failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	buffer[nbytes] = 0;

	nbytes = write(c->get_fd, buffer, nbytes);
	if (nbytes < 0) {
		err("write failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}

	c->is_waiting_for_reply = 1;
done:
	return;
}

void reply_msg(struct ipc_srv_s *c)
{
	int nbytes;

	if (!c->query_fd || !c->reply_fd) {
		err("[%d] un-expected ipc_server state!?\n", __LINE__);
		goto done;
	}
	nbytes = read(c->reply_fd, buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		err("read failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	buffer[nbytes] = 0;

	/* write back to queryer channel. */
	nbytes = write(c->query_fd, buffer, nbytes);
	if (nbytes < 0) {
		err("write failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}

done:
	return;
}

void on_get(struct ipc_srv_s *c)
{
	int get_fd;
	struct epoll_event ep_event;

	/* We assume there can be ONLY one get request at a time. */
	if (c->get_fd) {
		err("get_fd already exists?!\n");
		goto done;
	}
	get_fd = accept(c->get_listen_fd, NULL, NULL);
	if (get_fd < 0) {
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	c->get_fd = get_fd;

	/* get request is ready. Wait for set request ready. */
	if (!c->is_waiting_for_set) {
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_set;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->set_listen_fd, &ep_event) < 0) {
			/* Ignore if already added. */
			if (errno != EEXIST) {
				err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
				goto done;
			}
		}
		c->is_waiting_for_set = 1;
		/* Remove get request from epoll. Wait for set. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->get_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
	}

	/* accept new query only when not querying. */
	if (!c->is_waiting_for_reply && !c->is_waiting_for_query) {
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_query;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->query_listen_fd, &ep_event) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			goto done;
		}
		c->is_waiting_for_query = 1;
	}

done:
	return;
}

void on_set(struct ipc_srv_s *c)
{
	int set_fd;
	struct epoll_event ep_event;

	/*
	** There can be multiple set request coming. However, it can be queued
	** into Unix domain socket protocol buffer without blocking.
	** So, ipc_server accept set request one at a time and only accpet it
	** when there is a get request.
	*/
	if (c->set_fd) {
		err("set_fd already exists?!\n");
		goto out;
	}
	set_fd = accept(c->set_listen_fd, NULL, NULL);
	if (set_fd < 0) {
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	c->set_fd = set_fd;

	deliver_msg(c);

done:
	if (c->get_fd) {
		close(c->get_fd);
		c->get_fd = 0;
	}
	if (c->set_fd) {
		close(c->set_fd);
		c->set_fd = 0;
	}

	if (c->is_waiting_for_set) {
		/* Remove set request from epoll. Wait for get request first. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->set_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
		c->is_waiting_for_set = 0;
		/* Start accepting get request again. */
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_get;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->get_listen_fd, &ep_event) < 0) {
			/* Ignore if already added. */
			if (errno != EEXIST) {
				err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
				//goto done;
			}
		}
	}

	if (c->is_waiting_for_query) {
		/* Remove query request from epoll. Wait for get or reply request. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->query_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
		c->is_waiting_for_query = 0;
	}

out:
	return;
}

void on_reply(struct ipc_srv_s *c)
{
	int reply_fd;
	struct epoll_event ep_event;

	if (c->reply_fd) {
		err("reply_fd already exists?!\n");
		goto done;
	}
	reply_fd = accept(c->reply_listen_fd, NULL, NULL);
	if (reply_fd < 0) {
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	c->reply_fd = reply_fd;

	reply_msg(c);

done:
	if (c->query_fd) {
		close(c->query_fd);
		c->query_fd = 0;
	}
	if (c->reply_fd) {
		close(c->reply_fd);
		c->reply_fd = 0;
	}
	/* FIXME. We can't prevent redundent reply message. So, query/reply MUST be in pair. */
	if (c->is_waiting_for_reply) {
		/* Remove reply request from epoll. Wait for get request. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->reply_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
		/* query_fd will be closed anyway, so the reply is gone. */
		c->is_waiting_for_reply = 0;
	}

	/*
	 * accept new query when c->is_waiting_for_set is true
	 * because it means we already got 'get' before and ready for 'set' or 'query'
	 *
	 */
	if (c->is_waiting_for_set) {
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_query;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->query_listen_fd, &ep_event) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			goto done;
		}
		c->is_waiting_for_query = 1;
	}

	return;
}

void on_query(struct ipc_srv_s *c)
{
	int query_fd;
	struct epoll_event ep_event;

	/*
	** We assume query/reply happens in pair and in sequence.
	** User is allowed to set/get in between query and reply.
	*/
	if (c->query_fd) {
		err("query_fd already exists?!\n");
		goto done;
	}
	query_fd = accept(c->query_listen_fd, NULL, NULL);
	if (query_fd < 0) {
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto done;
	}
	c->query_fd = query_fd;

	query_msg(c);

done:
	/* get is done here. */
	if (c->get_fd) {
		close(c->get_fd);
		c->get_fd = 0;
	}
	/* We keep query_fd for replying to queryer later. */

	if (c->is_waiting_for_set) {
		/* Remove set request from epoll. Wait for get request first. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->set_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
		c->is_waiting_for_set = 0;
		/* Start accepting get request again. */
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_get;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->get_listen_fd, &ep_event) < 0) {
			/* Ignore if already added. */
			if (errno != EEXIST) {
				err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
				//goto done;
			}
		}
	}

	if (c->is_waiting_for_query) {
		/* Remove query request from epoll. Wait for get or reply request. */
		if (epoll_ctl(c->epfd, EPOLL_CTL_DEL, c->query_listen_fd, NULL) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
		c->is_waiting_for_query = 0;
	}
	/* If query_msg() is sent, c->is_waiting_for_reply will be true. */
	if (c->is_waiting_for_reply) {
		/* Wait for reply request. */
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = (void *)on_reply;
		if (epoll_ctl(c->epfd, EPOLL_CTL_ADD, c->reply_listen_fd, &ep_event) < 0) {
			err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
			//goto done;
		}
	}
	return;
}

int start_ipc_server(char *set_ch, char *get_ch, char *query_ch, char *reply_ch)
{
	struct ipc_srv_s *c = &the_srv;
	struct epoll_event ep_event;
	int set_listen_fd = -1, get_listen_fd = -1;
	int query_listen_fd = -1, reply_listen_fd = -1;
	int epfd = -1, ne;
	event_handler_func handler;

	memset(c, 0, sizeof(struct ipc_srv_s));

	/* TODO increase create_uds_server()'s listen max connection number. */
	set_listen_fd = create_uds_server(set_ch);
	if (set_listen_fd < 0) {
		goto done;
	}

	get_listen_fd = create_uds_server(get_ch);
	if (get_listen_fd < 0) {
		goto done;
	}

	query_listen_fd = create_uds_server(query_ch);
	if (query_listen_fd < 0) {
		goto done;
	}

	reply_listen_fd = create_uds_server(reply_ch);
	if (reply_listen_fd < 0) {
		goto done;
	}

	/* About size '5', Since Linux 2.6.8, the size argument is ignored, but must be greater than zero */
	epfd = epoll_create(5);
	if (epfd < 0) {
		err("[%d] epoll_create1 failed (%d:%s)", __LINE__, errno, strerror(errno));
		goto done;
	}

	ep_event.events = EPOLLIN;
	ep_event.data.ptr = (void *)on_get;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, get_listen_fd, &ep_event) < 0) {
		err("[%d] epoll_ctl failed (%d:%s)", __LINE__, errno, strerror(errno));
		goto done;
	}

	c->set_listen_fd = set_listen_fd;
	c->get_listen_fd = get_listen_fd;
	c->query_listen_fd = query_listen_fd;
	c->reply_listen_fd = reply_listen_fd;
	c->epfd = epfd;
	memset(&ep_event, 0, sizeof(ep_event));

	for (;;) {
		ne = epoll_wait(epfd, &ep_event, 1, -1);
		if (ne < 0) {
			/* something wrong. */
			perror("epoll_wait failed?!");
			break;
		}
		/* case for epoll/node_query timeout. */
		if (ne == 0) {
			/* Should never happen */
			continue;
		}
		/* Handle fd events. */
		handler = (event_handler_func)ep_event.data.ptr;
		handler(c);
	}

done:
	if (set_listen_fd > 0)
		close(set_listen_fd);
	if (get_listen_fd > 0)
		close(get_listen_fd);
	if (query_listen_fd > 0)
		close(query_listen_fd);
	if (reply_listen_fd > 0)
		close(reply_listen_fd);
	if (epfd > 0)
		close(epfd);
}

int main(int argc, char *argv[])
{
	if (argc != 5) {
		/* print usage */
		err("4 input required!\n");
		return -1;
	}

	start_ipc_server(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
