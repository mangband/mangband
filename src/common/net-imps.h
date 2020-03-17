#ifndef __NET_IMPLEMENTS_H_
#define __NET_IMPLEMENTS_H_

#define TV_SEC(A) (A / 1000000)
#define TV_MSEC(A) (A / 1000)
#define TV_SET(A,B) {A.tv_sec = 0;A.tv_usec=B;}

/* struct sender_type -- see imps.c */
/* struct caller_type -- see imps.c */
typedef struct listener_type listener_type;
typedef struct connection_type connection_type;
typedef struct timer_type timer_type;
struct listener_type {
	int port;
	int listen_fd;	
	callback accept_cb;
};
struct connection_type {
	int conn_fd;
	callback receive_cb; /* return -1 if you disliked his input */
	callback send_cb; /* Unused, unless using connection wrappers */
	callback close_cb;
	int close;
	char host_addr[24];
	cq rbuf;
	cq wbuf;
	int user; /* User-defined data, unused by us */
	data uptr;
	cq wsrbuf; /* Unused, additional read buffer for connection wrapping */
};
struct timer_type {
	micro interval;
	micro delay;
	callback timeout_cb; /* return 1 for infinite, 0 for one-shot, 2 for smooth */
};

extern eptr add_sender(eptr root, char *host, int port, micro interval, callback send_cb);
extern eptr add_caller(eptr root, char *addr, int port, callback conn_cb, callback fail_cb);
extern eptr add_listener(eptr root, int port, callback cb);
extern eptr add_timer(eptr root, int interval, callback timeout);
extern eptr add_connection(eptr root, int fd, callback read, callback close);

extern eptr handle_senders(eptr root, micro microsec);
extern eptr handle_listeners(eptr root);
extern eptr handle_connections(eptr root);
extern eptr handle_callers(eptr root);
extern eptr handle_timers(eptr root, long microsec);
extern micro static_timer(int id);

extern void network_reset(void);
extern void network_pause(long timeout);
extern void denaglefd(int fd);
extern  int islocalfd(int fd);
extern  int fillhostname(char *str, int len);
extern void unblockfd(int fd);

extern void e_release_all(eptr node, int d1, int d2);

#endif
