/*
 * Implements the generic TCP/IP networking.
 * Copyright (c) 2010 Vitaly Driedfruit. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY Vitaly Driedfruit ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Vitaly Driedfruit OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "angband.h"

//TODO: Wrap those into configure.ac!!
#ifdef WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
# ifndef _WINSOCK2API_
# include <winsock2.h>
# include <ws2tcpip.h>
# endif
#undef EWOULDBLOCK
#undef EINPROGRESS
#undef EALREADY
#undef EISCONN
#undef EINVAL
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS
#define EALREADY WSAEALREADY
#define EISCONN WSAEISCONN
#define EINVAL WSAEINVAL
#define sockerr WSAGetLastError()
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/fcntl.h>

#define sockerr errno
#define closesocket close
#endif


fd_set rd;
fd_set wd;
int nfds;
int cnfds;
int lnfds;
int crfds;
int refds;

fd_set* get_fd_set() { return &rd; }
int* get_fd_counter() { return &refds; }

struct sender_type {
	struct sockaddr_in addr;
	int send_fd;
	micro interval;
	micro delay;
	callback send_cb; /* return 1 for infinite, 0 for one-shot, 2 for smooth */
	cq wbuf;
};

struct caller_type {
	struct sockaddr_in addr;
	int port;
	int caller_fd;
	int remove;
	callback connect_cb;
	callback failure_cb; /* return 1 to try again */
};

eptr add_sender(eptr root, char *host, int port, micro interval, callback send_cb) {
	struct sender_type *new_s;
	struct hostent *hp;
	int senderfd;

	/* Resolve host into "hp" */
	if ((hp = gethostbyname(host)) == NULL)
	{
		/* plog("NAME RESOLUTION FAILED") */
		return (NULL);
	}

	/* Init socket */
	senderfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Set to non-blocking. */
	//unblockfd(callerfd);

	/* Allocate memory */
	new_s = (struct sender_type*) RNEW(struct sender_type);

	/* Set addr and others */
	new_s->addr.sin_family = AF_INET;
	new_s->addr.sin_port = htons(port);
	memcpy (&(new_s->addr.sin_addr), hp->h_addr, sizeof(new_s->addr.sin_addr));

	new_s->send_fd = senderfd;
	new_s->send_cb = send_cb;
	new_s->interval = interval;
	new_s->delay = interval;
	cq_init(&new_s->wbuf, PD_SMALL_BUFFER);

	/* Add to list */
	return e_add(root, NULL, new_s);
}

eptr add_caller(eptr root, char *host, int port, callback conn_cb, callback fail_cb) {
	struct caller_type *new_c;
	struct hostent *hp;
	int callerfd;

	/* Resolve addr */
	if ((hp = gethostbyname(host)) == NULL)
	{
		/* plog("NAME RESOLUTION FAILED") */
		return (NULL);
	}

	/* Init socket */
	callerfd = socket(AF_INET, SOCK_STREAM, 0);
	
	/* Set to non-blocking. */
	unblockfd(callerfd);

	/* Allocate memory */
	new_c = (struct caller_type*) RNEW(struct caller_type);

	/* Set addr and others */
	new_c->addr.sin_family = AF_INET;
	new_c->addr.sin_port = htons(port);
	memcpy (&(new_c->addr.sin_addr), hp->h_addr, sizeof(new_c->addr.sin_addr));

	new_c->port = port;
	new_c->connect_cb = conn_cb;
	new_c->failure_cb = fail_cb;
	new_c->caller_fd = callerfd;
	new_c->remove = 0; /* important */

	crfds = MATH_MAX(crfds, callerfd);

	/* Add to list */
	return e_add(root, NULL, new_c);
}

eptr add_listener(eptr root, int port, callback cb) {
	struct listener_type *new_l;
	struct sockaddr_in servaddr;
	int listenfd;
	int on;

	/* Init socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		plog_fmt("SOCKET CREATEION FAILED for port %d", port);
		return (NULL);
	}

	/* Enable address reuse */
	on = 1;
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on) );

	/* Set to non-blocking. */
	unblockfd(listenfd);

	/* Bind & Listen */
	WIPE(&servaddr, struct sockaddr_in);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
		plog_fmt("BIND FAILED for port %d", port);
		closesocket(listenfd);
		return(NULL);
	}

	if (listen(listenfd,1024) < 0) {
		plog_fmt("LISTEN FAILED for port %d", port);
		closesocket(listenfd);
		return(NULL);
	}

	/* Allocate memory */
	new_l = (struct listener_type*) RNEW(struct listener_type);

	new_l->port = port;
	new_l->accept_cb = cb;
	new_l->listen_fd = listenfd;

	/* Add to list */
	return e_add(root, NULL, new_l);
}

eptr add_connection(eptr root, int fd, callback read, callback close) {
	struct connection_type *new_c;
	struct sockaddr_in sin;
	socklen_t len = sizeof sin;

	/* Allocate memory */
	new_c = (struct connection_type*) RNEW(struct connection_type);

	new_c->conn_fd = fd;
	new_c->receive_cb = read;
	new_c->send_cb = NULL;
	new_c->close_cb = close;
	new_c->close = 0;
	new_c->uptr = NULL;
	cq_init(&new_c->wbuf, PD_LARGE_BUFFER);
	cq_init(&new_c->rbuf, PD_LARGE_BUFFER);

	if (getpeername(fd, (struct sockaddr *) &sin, &len) >= 0)
	{
#ifdef HAVE_INET_NTOP
		inet_ntop(AF_INET, &sin.sin_addr, new_c->host_addr, 24);
#else
		strcpy(new_c->host_addr, (char*)inet_ntoa(sin.sin_addr));
#endif
	}

	cnfds = MAX(cnfds, new_c->conn_fd);

	/* Add to list */
	return e_add(root, NULL, new_c);
}

eptr add_timer(eptr root, int interval, callback timeout) {
	struct timer_type *new_t;

	/* Allocate memory */
	new_t = (struct timer_type*) RNEW(struct timer_type);

	new_t->interval = interval;
	new_t->delay = interval;
	new_t->timeout_cb = timeout;

	/* Add to list */
	return e_add(root, NULL, new_t);
}

eptr handle_connections(eptr root) {
	char mesg[PD_LARGE_BUFFER];
	eptr iter;
	int connfd, n, to_close = 0;
	struct connection_type *ct;

	for (iter=root; iter; iter=iter->next) {
		ct = (connection_type*)iter->data2;
		connfd = ct->conn_fd;

		FD_SET(connfd, &rd);

		/* /Connection is not yet closed/ */
		if (!ct->close)
		{
			/* Receive */
			n = PD_LARGE_BUFFER;/* Paranoia */
			n = MAX(1, MIN(cq_space(&ct->rbuf), PD_LARGE_BUFFER)); /* n = [1=>"bytes left in buffer"=>PD_LARGE_BUFFER] */
			n = recvfrom(connfd, mesg, n, 0, NULL, 0);
			if (n > 0)
			{
				/* Got 'n' bytes */
				n = cq_nwrite(&ct->rbuf, mesg, n);
				/* Error while filling buffer */
				if (n <= 0) ct->close = 1;
			}
			/* Error while receiving */
			else if (n == 0 || sockerr != EWOULDBLOCK) ct->close = 1;
		}
		/* Handle input */
		if (!ct->close && cq_len(&ct->rbuf))
		{
			n = ct->receive_cb(0, ct);
			/* Error while handling input */
			if (n < 0) ct->close = 1;
		}
		/* Send */
		if (cq_len(&ct->wbuf))
		{
			/* Hack -- call connection wrapper (if any) */
			if (ct->send_cb)
			{
				/* It's job is to fill the "mesg" buffer, instead of
				 * doing a simple "cq_read" below. */
				void *old_ptr = ct->uptr;
				ct->uptr = &mesg[0];
				n = ct->send_cb(0, ct);
				ct->uptr = old_ptr;
			} else

			n = cq_read(&ct->wbuf, &mesg[0], PD_LARGE_BUFFER);
			n = sendto(connfd,mesg,n,0, NULL,0);

			/* Error while sending */
			if (n <= 0) ct->close = 1;
		}

		/* Done for? */
		to_close += ct->close;
	}
	if (to_close) {
		while (to_close) {
			for (iter=root; iter; iter=iter->next) {
				ct = (connection_type*)iter->data2;
				if (ct->close)
				{
					closesocket(ct->conn_fd);
					FD_CLR(ct->conn_fd, &rd);
					ct->close_cb(0, ct);
					cq_free(&ct->rbuf);
					cq_free(&ct->wbuf);
					FREE(ct);
					e_del(&root, iter);
					to_close--;
					break;
				}
			}
		}
		cnfds = 0;
		for (iter=root; iter; iter=iter->next) {
			ct = (connection_type*)iter->data2;
			cnfds = MATH_MAX(cnfds, ct->conn_fd);
		}
	}
	/*nfds = MATH_MAX(lnfds, cnfds);*/
	return root;
}

eptr handle_callers(eptr root) {
	eptr iter;
	int to_remove = 0;

	for (iter=root; iter; iter=iter->next) {
		struct caller_type *ct = (struct caller_type *)iter->data2;
		int callerfd = ct->caller_fd;
		int n = 0;
		int err = 0;

		/* if (FD_ISSET(callerfd, &wd)) {
			//this is a good place to check if socket is connected
		} */

		FD_SET(callerfd, &wd);
		n = connect(callerfd, (struct sockaddr *)&ct->addr, sizeof(ct->addr));
		err = sockerr;
		#ifdef WINDOWS
		if (err == EINVAL) {
			int errVal;
			int errLen = sizeof(int);
			getsockopt(callerfd, SOL_SOCKET, SO_ERROR, (char*)&errVal, &errLen);
			if (errVal) {
				err = errVal;
			} else {
				err = EWOULDBLOCK;
			}
		}
		#endif
		if (n == 0 || err == EISCONN)
			ct->connect_cb(callerfd, (data)ct);
		else if (err == EALREADY) continue;
		else if (err == EINPROGRESS) continue;
		else if (err == EWOULDBLOCK) continue;
		else {
			n = ct->failure_cb(callerfd, (data)ct);
			if (n) continue;
			closesocket(callerfd);
		}

		to_remove += (ct->remove = 1);
	}
	if (to_remove) {
		while (to_remove) {
			for (iter=root; iter; iter=iter->next) {
				struct caller_type *ct = (struct caller_type *)iter->data2;
				if (ct->remove)
				{
					FD_CLR(ct->caller_fd, &wd);
					FREE(ct);
					e_del(&root, iter);
					to_remove--;
					break;
				}
			}
		}
		crfds = 0;
		for (iter=root; iter; iter=iter->next) {
			struct caller_type *ct = (struct caller_type *)iter->data2;
			crfds = MATH_MAX(crfds, ct->caller_fd);
		}
	}
	
	return root;
}

eptr handle_listeners(eptr root) {
	struct sockaddr_in cliaddr;
	static socklen_t clilen = sizeof(cliaddr);

	eptr iter;
	int connfd, err;

	for (iter=root; iter; iter=iter->next) {
		struct listener_type *lt = (struct listener_type *)iter->data2;
		int listenfd = lt->listen_fd;	

		FD_SET (listenfd, &rd);
		lnfds = MATH_MAX(listenfd, lnfds);

		connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
		if (connfd == -1) continue;

		unblockfd(connfd);

		/* cnfds = MATH_MAX(connfd, cnfds); */

		err = lt->accept_cb(connfd, lt);
		if (err) {
			closesocket(connfd);
			/* FD_CLR(connfd, &rd); */
		}

	}
	return root;
}

/* Returns microseconds since last time this function was called */
micro static_timer(int id) {
	static micro times[5] = { 0, 0, 0, 0, 0 };

	micro passed;
#ifndef WINDOWS /* TODO: HAVE_GETTIMEOFDAY */
	micro microsec;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	microsec = tv.tv_sec * 1000000 + tv.tv_usec;
#else
	micro microsec;
	LARGE_INTEGER PerformanceCount, Frequency;
	__int64 tv;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&PerformanceCount);
	tv = (PerformanceCount.QuadPart * 1000000) / Frequency.QuadPart;
	microsec = tv;
#endif
/*
	printf("OLD: %ld\n", times[id]);
	printf("NEW: %ld\n", microsec);
	printf("DIF: %ld\n", microsec);
*/
	passed = (!times[id] ? 0 : microsec - times[id]);
	times[id] = microsec;

	return passed;
}

eptr handle_senders(eptr root, micro microsec) {
	eptr iter;
	int n, to_close = 0;
	char mesg[PD_SMALL_BUFFER];

	for (iter=root; iter; iter=iter->next) {
		struct sender_type *sender = (struct sender_type *)iter->data2;
		sender->delay -= microsec;
		while (sender->delay <= 0) {
			sender->delay += sender->interval;
			n = sender->send_cb((int)TV_SEC(sender->interval), &sender->wbuf);
			if (!n) {
				cq_clear(&sender->wbuf);
				sender->interval = 0;
				to_close++;
			} else if (n != 2) 
				sender->delay = sender->interval;
		}
		/* Send */
		if (cq_len(&sender->wbuf))
		{
			n = cq_read(&sender->wbuf, &mesg[0], PD_SMALL_BUFFER);
			n = sendto(sender->send_fd,mesg,n,0,(struct sockaddr *)&sender->addr,sizeof(struct sockaddr));

			/* Error while sending */
			if (n <= 0) {
				sender->interval = 0;
				to_close++;
			}
		}
	}
	while (to_close) {
		for (iter=root; iter; iter=iter->next) {
			struct sender_type *sender = (struct sender_type *)iter->data2;
			if (!sender->interval)
			{
				closesocket(sender->send_fd);
				cq_free(&sender->wbuf);
				FREE(sender);
				e_del(&root, iter);
				to_close--;
				break;
			}
		}
	}
	return root;
}

eptr handle_timers(eptr root, micro microsec) {
	eptr iter;
	int n, to_close = 0;
	for (iter=root; iter; iter=iter->next) {
		struct timer_type *timer = (struct timer_type *)iter->data2;
		timer->delay -= microsec;
		while (timer->delay <= 0) {
			timer->delay += timer->interval;
			n = timer->timeout_cb((int)(microsec / 1000000), timer);
			if (!n) {
				timer->interval = 0;
				to_close++;
			} else if (n != 2) 
				timer->delay = timer->interval;
		}
	}
	while (to_close) {
		for (iter=root; iter; iter=iter->next) {
			struct timer_type *timer = (struct timer_type *)iter->data2;
			if (!timer->interval)
			{
				FREE(timer);
				e_del(&root, iter);
				to_close--;
				break;
			}
		}
	}
	return root;
}

void network_reset() {
#ifdef WINDOWS
	WSADATA wsadata;

	/* Initialize WinSock */
	WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif

	FD_ZERO (&rd);
	FD_ZERO (&wd);
	nfds = cnfds = lnfds = crfds = refds = 0;
}

void network_done() {
#ifdef WINDOWS
	WSACleanup();
#endif
}

void network_pause(micro timeout) {
#ifndef HAVE_SELECT
	usleep(timeout);
#else
	struct timeval tv = { 0, 0 };

	TV_SET(tv, timeout); /* 200000 = 0.2 seconds */

	nfds = MATH_MAX(lnfds, cnfds);
	nfds = MATH_MAX(nfds, crfds);
	nfds = MATH_MAX(nfds, refds);

	select(nfds + 1, &rd, &wd, NULL, &tv);
#endif
}

/* Set socket as non-blocking */
void unblockfd(int fd) {
#ifdef WINDOWS
	u_long iMode = 1;
	ioctlsocket(fd, FIONBIO, &iMode);
#else
	fcntl(fd, F_SETFL, O_NONBLOCK);
#endif
}

/* Disable Nagle's algorithm (required for MAngband) */
void denaglefd(int fd) {
#ifdef WINDOWS
       DWORD
#else
       int
#endif
       on = 1;
       setsockopt( fd, IPPROTO_TCP, TCP_NODELAY,
#ifdef WINDOWS
       (const char*)
#else
       (const void*)
#endif
       &on, sizeof(on) );
}

/* Get local machine hostname */
int fillhostname(char *str, int len) {
	return gethostname(str, len);
}

/* islocalfd helper functions */
static int cmp_sockaddr4(struct sockaddr_in *a, struct sockaddr_in *b)
{
	return (ntohl(a->sin_addr.s_addr) == ntohl(b->sin_addr.s_addr));
}
static int firstoctet_sockaddr4(struct sockaddr_in *addr)
{
	return ((ntohl(addr->sin_addr.s_addr) >> 24) & 0xFF);
}

/* Check if address belongs to a local network. */
/* Only deals vs IP4 addresses for now... :( */
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>  /* Linux/BSD getiffaddr */
#else
#ifdef WINDOWS
#include <iphlpapi.h> /* Windows GetAdapterAddresses */
#endif
#endif
int islocalfd(int fd)
{
	int ok = 0;
	struct sockaddr_in remote_addr;
	struct sockaddr_in *check_addr;
	socklen_t len = sizeof(remote_addr);
	getpeername(fd, (struct sockaddr *)&remote_addr, &len);
	
	/* Any address starting with 127. is a local address */
	if (firstoctet_sockaddr4(&remote_addr) == 127)
	{
		return 1;
	}
#ifdef HAVE_GETIFADDRS
  {
	/* Simplest version, should work on UNIXes and MinGW */
	struct ifaddrs *ifap, *if_ptr;

	getifaddrs (&ifap);
	for (if_ptr = ifap; if_ptr; if_ptr = if_ptr->ifa_next) {
		if (if_ptr->ifa_addr->sa_family == AF_INET) {
			check_addr = (struct sockaddr_in *) if_ptr->ifa_addr;
			//printf("Interface: %s\tAddress: %s\n", if_ptr->ifa_name, inet_ntoa(check_addr->sin_addr));
			if (cmp_sockaddr4(check_addr, &remote_addr))
			{
				ok = 1;
				break;
			}
		}
	}
	freeifaddrs(ifap);
  }
#else
#ifdef WINDOWS
  {
	/* Windows-only version */
	DWORD rv, size;
	PIP_ADAPTER_ADDRESSES ifap, if_ptr;
	PIP_ADAPTER_UNICAST_ADDRESS ua;

	/* First call to get required size */
	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
	if (rv != ERROR_BUFFER_OVERFLOW) {
		/* Fatal error */
		return 0;
	}
	/* Second call to populate the structs */
	ifap = (PIP_ADAPTER_ADDRESSES)malloc(size);
	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, ifap, &size);
	if (rv != ERROR_SUCCESS) {
		/* Fatal error */
		free(ifap);
		return 0;
	}
	/* This is similar to getifaddr iteration from UNIX */
	for (if_ptr = ifap; if_ptr != NULL; if_ptr = if_ptr->Next) {
	for (ua = if_ptr->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
		if (ua->Address.lpSockaddr->sa_family == AF_INET) {
			check_addr = (struct sockaddr_in*) ua->Address.lpSockaddr;
			//printf("Interface: %wS\tAddress: %s\n", aa->FriendlyName, inet_ntoa(check_addr->sin_addr));
			if (cmp_sockaddr4(check_addr, &remote_addr))
			{
				ok = 1;
				break;
			}
		}
	} if (ok) break; }
	free(ifap);
  }
#else
  {
	/* Worst-case scenario, revert to getpeername/getsockname.
	 * This will not handle multi-homed machines (>1 NIC) correctly. */
	//TODO: replace with ioctl-based interface enumeration..?
	struct sockaddr_in local_addr;
	getsockname(fd, (struct sockaddr *)&local_addr, &len);
	ok = cmp_sockaddr4(&remote_addr, &local_addr);
  }
#endif
#endif
	return ok;
}

/* Delete all nodes from "root" while deleting their "data" */
void e_release_all(eptr node, int d1, int d2) {
	eptr next = NULL;
	for (; node; node = next) {
		next = node->next;
		/* "!= NULL" saves us 1 syscall (or is useless?) */
		if (d1 && node->data1 != NULL) FREE(node->data1);
		if (d2 && node->data2 != NULL) FREE(node->data2);
		e_free_aux(node);
	}
}
