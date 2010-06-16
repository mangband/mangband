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
# endif
#undef EWOULDBLOCK
#undef EINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS
#define sockerr WSAGetLastError()
#define socklen_t int
#else
#include <netdb.h> 
#include <netinet/tcp.h>
#define sockerr errno
#define closesocket close
#endif

fd_set rd;
int nfds;
int cnfds;
int lnfds;
int crfds;

struct caller_type {
	struct sockaddr_in addr;
	int port;
	int caller_fd;
	int remove;
	callback connect_cb;
	callback failure_cb; /* return 1 to try again */
};

eptr add_caller(eptr root, char *host, int port, callback conn_cb, callback fail_cb) {
	struct caller_type *new_c;
	struct hostent *hp;
	int callerfd;
	
	/* Init socket */
	callerfd = socket(AF_INET, SOCK_STREAM, 0);
	
	/* Set to non-blocking. */
	unblockfd(callerfd);

	/* Allocate memory */
	MAKE(new_c, struct caller_type);

	/* Set addr and others */
	new_c->addr.sin_family = AF_INET;
	new_c->addr.sin_port = htons(port);
	hp = gethostbyname(host);
	memcpy (&(new_c->addr.sin_addr), hp->h_addr, hp->h_length);

	new_c->port = port;
	new_c->connect_cb = conn_cb;
	new_c->failure_cb = fail_cb;
	new_c->caller_fd = callerfd;
	new_c->remove = 0; /* important */

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

	/* Enable address reuse */
	on = 1;
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );   

	/* Set to non-blocking. */
	unblockfd(listenfd);

	/* Bind & Listen */
	WIPE(&servaddr, struct sockaddr_in);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
		plog("BIND FAILED");
	   	return(NULL);
	}

	if (listen(listenfd,1024) < 0) {
		plog("LISTEN FAILED");
		return(NULL);
	}

	/* Allocate memory */
	MAKE(new_l, struct listener_type);

	new_l->port = port;
	new_l->accept_cb = cb;
	new_l->listen_fd = listenfd;

	/* Add to list */	
	return e_add(root, NULL, new_l);
}

eptr add_connection(eptr root, int fd, callback read, callback close) {
	struct connection_type *new_c;
	struct sockaddr_in sin;
	int len = sizeof sin;

	/* Allocate memory */
	MAKE(new_c, struct connection_type);

	new_c->conn_fd = fd;
	new_c->receive_cb = read;
	new_c->close_cb = close;
	new_c->close = 0;
	new_c->uptr = NULL;
	cq_init(&new_c->wbuf, PD_SMALL_BUFFER);
	cq_init(&new_c->rbuf, PD_SMALL_BUFFER);

	if (getpeername(fd, (struct sockaddr *) &sin, &len) >= 0)
		inet_ntop(AF_INET, &sin.sin_addr, new_c->host_addr, 24);

	/* Add to list */
	return e_add(root, NULL, new_c);
}

eptr add_timer(eptr root, int interval, callback timeout) {
	struct timer_type *new_t;

	/* Allocate memory */
	MAKE(new_t, struct timer_type);

	new_t->interval = interval;
	new_t->delay = interval;
	new_t->timeout_cb = timeout;

	/* Add to list */	
	return e_add(root, NULL, new_t);
}

eptr handle_connections(eptr root) {
	char mesg[PD_SMALL_BUFFER];
	eptr iter;
	int connfd, n, to_close = 0;
	struct connection_type *ct;
	struct sockaddr_in cliaddr;
	static socklen_t clilen = sizeof(cliaddr);

	for (iter=root; iter; iter=iter->next) {
		ct = (connection_type*)iter->data2;
		connfd = ct->conn_fd;	

		FD_SET(connfd, &rd);

		/* Receive (Connection is not yet closed) */
		if (!ct->close)
		{
			n = recvfrom(connfd,mesg,PD_SMALL_BUFFER,0,(struct sockaddr *)&cliaddr,&clilen);
			if (n > 0) 
			{
				/* Got 'n' bytes */
				if (cq_nwrite(&ct->rbuf, mesg, n))
				{
					n = ct->receive_cb(0, ct);

					/* Error while handling input */
					if (n < 0) ct->close = 1;
				}
				/* Error while filling buffer */
				else ct->close = 1;
			}
			/* Error while receiving */
			else if (n == 0 || sockerr != EWOULDBLOCK) ct->close = 1;	
		}
		/* Send */
		if (cq_len(&ct->wbuf))
		{
			n = cq_read(&ct->wbuf, &mesg[0], PD_SMALL_BUFFER);
			n = sendto(connfd,mesg,n,0,(struct sockaddr *)&cliaddr,clilen);

			/* Error while sending */
			if (n <= 0) ct->close = 1;
		}

		/* Done for? */
		to_close += ct->close;
	}
	if (to_close) cnfds = 0;		
	while (to_close) {
		for (iter=root; iter; iter=iter->next) {
			ct = (connection_type*)iter->data2;
			if (ct->close)
			{
				closesocket(ct->conn_fd);
				FD_CLR(ct->conn_fd, &rd);
				ct->close_cb(0, ct);
				free(ct);
				if (e_del(&root, iter) == 0)
					root = NULL;
				to_close--;
				break;
			} else {
				cnfds = MATH_MAX(cnfds, ct->conn_fd);
			}
		}
	}
	/*nfds = MATH_MAX(lnfds, cnfds);*/
	return root;
}

eptr handle_callers(eptr root) {
	eptr iter;
	int to_remove = 0;

	for (iter=root; iter; iter=iter->next) {
		struct caller_type *ct = iter->data2;
		int callerfd = ct->caller_fd;
		int n = 0;

		FD_SET(callerfd, &rd);
		n = connect(callerfd, (struct sockaddr *)&ct->addr, sizeof(ct->addr));
#ifdef WINDOWS
		if (sockerr == WSAEISCONN) n = 0;
#endif
		if (n == 0) ct->connect_cb(callerfd, (data)ct);
		else if (sockerr == EINPROGRESS) continue;
		else {
			n = ct->failure_cb(callerfd, (data)ct);
			if (n) continue;
			closesocket(callerfd);
		}

		to_remove += (ct->remove = 1);
	}
	if (to_remove) crfds = 0;
	while (to_remove) {
		for (iter=root; iter; iter=iter->next) {
			struct caller_type *ct = iter->data2;
			if (ct->remove)
			{
				FD_CLR(ct->caller_fd, &rd);
				free(ct);
				if (e_del(&root, iter) == 0)
					root = NULL;
				to_remove--;
				break;
			} else {
				crfds = MATH_MAX(crfds, ct->caller_fd);
			}
		}
	}
	
	return root;
}

eptr handle_listeners(eptr root) {
	struct sockaddr_in cliaddr;
	static socklen_t clilen = sizeof(cliaddr);

	eptr iter;
	int connfd;

	for (iter=root; iter; iter=iter->next) {
		struct listener_type *lt = iter->data2;
		int listenfd = lt->listen_fd;	

		FD_SET (listenfd, &rd);
		lnfds = MATH_MAX(listenfd, lnfds);

		connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
		if (connfd == -1) continue;

		unblockfd(connfd);

		cnfds = MATH_MAX(connfd, cnfds);

		lt->accept_cb(connfd, lt);
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
	micro microsec = 0;
	FILETIME tv;
	SYSTEMTIME tv2;
	GetSystemTimeAsFileTime(&tv);
	FileTimeToSystemTime(&tv, &tv2);
	microsec = tv2.wSecond * 1000000 + tv2.wMilliseconds * 1000;
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

eptr handle_timers(eptr root, micro microsec) {
	eptr iter;
	int n, to_close = 0;
	for (iter=root; iter; iter=iter->next) {
		struct timer_type *timer = iter->data2;
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
			struct timer_type *timer = iter->data2;
			if (!timer->interval)
			{
				free(timer);
				if (e_del(&root, iter) == 0)
					root = NULL;
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
}

void network_pause(micro timeout) {
	struct timeval tv = { 0, 0 };

	TV_SET(tv, timeout); /* 200000 = 0.2 seconds */

	nfds = MATH_MAX(lnfds, cnfds);
	nfds = MATH_MAX(nfds, crfds);

	select (nfds + 1, &rd, NULL, NULL, &tv);
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
	char on = 1;
	setsockopt( fd, SOL_SOCKET, TCP_NODELAY, &on, sizeof(on) );   
}

/* Get local machine hostname */
int fillhostname(char *str, int len) {
	return gethostname(str, len);
}

#define PACK_PTR_8(PT, VAL) * PT ++ = VAL 
#define PACK_PTR_16(PT, VAL) * PT ++ = (char)(VAL >> 8), * PT ++ = (char)VAL
#define PACK_PTR_32(PT, VAL) * PT ++ = (char)(VAL >> 24), * PT ++ = (char)(VAL >> 16), * PT ++ = (char)(VAL >> 8), * PT ++ = (char)VAL
#define PACK_PTR_STR(PT, VAL) while ((* PT ++ = * VAL ++) != '\0')

const cptr pf_errors[] = {
"", /* 0 */
"Unrecognized format", /* 1 */
"No space in buffer", /* 2 */
"",
};
int cq_printf(cq *charq, char *str, ...) {
	int error = 0, bytes = 0, str_size = 0;
	va_list marker;

	signed char s8b;
	unsigned char u8b;
	u16b _u16b;
	s16b _s16b;
	u32b _u32b;
	s32b _s32b;
	char *text;

	char *start, *wptr, *end;

	va_start( marker, str );

	start = wptr = &charq->buf[charq->len];
	end = &charq->buf[charq->max];

#define PF_ERROR_SIZE(SIZE) if (wptr + SIZE > end) { error = 2; break; }
#define PF_ERROR_FRMT default: { error = 1; break; }
#define PACK_8(VAL) PACK_PTR_8(wptr, VAL)
#define PACK_16(VAL) PACK_PTR_16(wptr, VAL)
#define PACK_32(VAL) PACK_PTR_32(wptr, VAL)
#define PACK_STR(VAL) PACK_PTR_STR(wptr, VAL)

	while (!error && *str++ == '%') {
		switch(*str++) {
			case 'b': {/* hack, same as '%uc' */
				PF_ERROR_SIZE(1)
				u8b = (unsigned char) va_arg (marker, unsigned int);
				PACK_8(u8b);
				break;}
			case 'c': {
				PF_ERROR_SIZE(1)
				s8b = (signed char) va_arg (marker, signed int);
				PACK_8(s8b);
				break;}
			case 'd': {
				PF_ERROR_SIZE(2)
				_s16b = (s16b) va_arg (marker, signed int);
				PACK_16(_s16b);
				break;}
			case 'l': {
				PF_ERROR_SIZE(4)
				_s32b = (s32b) va_arg (marker, s32b);
				PACK_32(_s32b);
				break;}
			case 'u': { /* unsigned */
				switch (*str++) {
			    	case 'c': {
						PF_ERROR_SIZE(1)
						u8b = (unsigned char) va_arg (marker, unsigned int);
						PACK_8(u8b);
						break;}
			    	case 'd': {
			    		PF_ERROR_SIZE(2)
						_u16b = (u16b) va_arg (marker, unsigned int);
						PACK_16(_u16b);
						break;}
					case 'l': {
						PF_ERROR_SIZE(4)
						_u32b = va_arg (marker, u32b);
						PACK_32(_u32b);
			    		break;}
			    	PF_ERROR_FRMT
				}
				break;}
			case 'n': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text);
				PF_ERROR_SIZE(str_size+1);
				PACK_8(str_size);
				PACK_STR(text);
				break;}
			case 'N': {
				text = (char*) va_arg (marker, char *);
				str_size = strlen(text);
				PF_ERROR_SIZE(str_size+2);
				PACK_16(str_size);
				PACK_STR(text);
				break;}
			case 's': {
				text = (char*) va_arg (marker, char *);
				//str_size = MAX_CHARS;
				str_size = MIN(strlen(text)+1, MAX_CHARS);
				PF_ERROR_SIZE(str_size);
				PACK_STR(text);
				break;}
			case 'S': {
				text = (char*) va_arg (marker, char *);
				str_size = MIN(strlen(text)+1, MSG_LEN);
				//str_size = MSG_LEN;
				PF_ERROR_SIZE(str_size);
				PACK_STR(text);
				break;}
			PF_ERROR_FRMT
		}
	}

	if (error) {
		plog(format("Error in cq_printf('...%s'): %s [%d.%d]\n", str, pf_errors[error], str_size, charq->len)); 
		bytes = 0;
	} else {
		bytes = (wptr - start);
		charq->len += bytes;
	}

	va_end( marker );
	return bytes;
}

#define UNPACK_PTR_8(PT, VAL) * PT = * VAL ++ 
#define UNPACK_PTR_16(PT, VAL) * PT = * VAL ++ << 8, * PT |= (* VAL ++ & 0xFF)
#define UNPACK_PTR_32(PT, VAL) * PT = * VAL ++ << 24, * PT |= (* VAL ++ & 0xFF) << 16, * PT |= (* VAL ++ & 0xFF) << 8, * PT |= (* VAL ++ & 0xFF)

int cq_scanf(cq *charq, char *str, ...) {
	int error = 0, found = 0, str_size = 0;
	va_list marker;

	signed char *s8b;
	unsigned char *u8b;
	u16b *_u16b;
	s16b *_s16b;
	u32b *_u32b;
	s32b *_s32b;
	char *_text = {'\0'};
	
	char *start, *rptr, *end;

	va_start( marker, str );

	start = rptr = &charq->buf[charq->pos];
	end = &charq->buf[charq->len];

#define SF_ERROR_SIZE(SIZE) if (rptr + SIZE > end) { error = 2; break; }
#define SF_ERROR_FRMT default: { error = 1; break; }
#define UNPACK_8(PT) UNPACK_PTR_8(PT, rptr)
#define UNPACK_16(PT) UNPACK_PTR_16(PT, rptr)
#define UNPACK_32(PT) UNPACK_PTR_32(PT, rptr)
	
	while (!error && *str++ == '%') {
		found++;
		switch(*str++) {
			case 'b': /* hack, same as '%uc' */
				SF_ERROR_SIZE(1)
				u8b = (unsigned char*) va_arg (marker, unsigned int*);
				UNPACK_8(u8b);
			break;
			case 'c': {
				SF_ERROR_SIZE(1)
				s8b = (signed char*) va_arg (marker, signed int*);
				UNPACK_8(s8b);
				break;}
			case 'd': {
				SF_ERROR_SIZE(2)
				_s16b = (s16b*) va_arg (marker, signed int*);
				UNPACK_16(_s16b);
				break;}
			case 'l': {
				SF_ERROR_SIZE(4)
				_s32b = (s32b*) va_arg (marker, s32b*);
				UNPACK_32(_s32b);
				break;}
			case 'u': {/* unsigned */
				switch (*str++) {
			    	case 'c': {
			    		SF_ERROR_SIZE(1)
						u8b = (unsigned char*) va_arg (marker, unsigned int*);
						UNPACK_8(u8b);
						break;}
			    	case 'd': {
			    		SF_ERROR_SIZE(2)
						_u16b = (u16b*) va_arg (marker, unsigned int*);
						UNPACK_16(_u16b);
						break;}
					case 'l': {
						SF_ERROR_SIZE(4)
						_u32b = (u32b*) va_arg (marker, u32b*);
						UNPACK_32(_u32b);
			    		break;}
					SF_ERROR_FRMT	
				}
				break;}
			case 'n': {
				SF_ERROR_SIZE(1)
				_text = (char*) va_arg (marker, char*);
				UNPACK_8(&str_size);
				SF_ERROR_SIZE(str_size+1);
				while(str_size--) *_text++ = *rptr++;
				break;}
			case 'N': {
				SF_ERROR_SIZE(1)
				_text = (char*) va_arg (marker, char*);
				UNPACK_16(&str_size);
				SF_ERROR_SIZE(str_size+1);
				while(str_size--) *_text++ = *rptr++;
				break;}
			case 's': {
				_text = (char*) va_arg (marker, char*);
				//unsigned char str_size;
				str_size = MIN(strlen(rptr), MAX_CHARS);
				SF_ERROR_SIZE(str_size);
				while((*_text++ = *rptr++) != '\0') ;
				break;}
			case 'S': {
				_text = (char*) va_arg (marker, char*);
				//unsigned char str_size;
				str_size = MIN(strlen(rptr), MSG_LEN);
				SF_ERROR_SIZE(str_size);
				while((*_text++ = *rptr++) != '\0') ;
				break;}
			case 'T': {/* HACK! unlimited \n-terminated string (\r==\n here)*/
				_text = (char*) va_arg (marker, char*);
				while(*rptr != '\0' && *rptr != '\n' && *rptr != '\r')
				 *_text++ = *rptr++;
				 *_text = '\0';
				while(*rptr != '\0') *rptr++;
				break;}
			SF_ERROR_FRMT
		}
	}
	
	if (error) {
		found = 0;
		plog(format("Error in cq_scanf('...%s'): %s [%d]\n", str, pf_errors[error], str_size));
	} else {
		charq->pos += (rptr - start);
	}

	va_end( marker );
	return found;
}

/* Hack -- a function to apply cq_scanf into cq_printf. 
 * Must be similar to the functions above */
int cq_copyf(cq *src, const char *str, cq *dst) {
	int error = 0, found = 0, str_size = 0;

	char *rstart, *rptr, *rend;
	char *wstart, *wptr, *wend;

	rstart = rptr = &src->buf[src->pos];
	rend = &src->buf[src->len];

	wstart = wptr = &dst->buf[dst->len];
	wend = &dst->buf[dst->max];

#define CF_ERROR_SIZE(SIZE) if (rptr + SIZE > rend || wptr + SIZE > wend) { error = 2; break; }
#define CF_ERROR_FRMT default: { error = 1; break; }

#define REPACK_8 *wptr++ = *rptr++;
#define REPACK_16 REPACK_8; REPACK_8  
#define REPACK_32 REPACK_16; REPACK_16

	while (!error && *str++ == '%') {
		found++;
		switch(*str++) {
			case 'b': /* hack, same as '%uc' */
				CF_ERROR_SIZE(1)
				REPACK_8
			break;
			case 'c': {
				CF_ERROR_SIZE(1)
				REPACK_8
				break;}
			case 'd': {
				CF_ERROR_SIZE(2)
				REPACK_16
				break;}
			case 'l': {
				CF_ERROR_SIZE(4)
				REPACK_32
				break;}
			case 'u': {/* unsigned */
				switch (*str++) {
			    	case 'c': {
			    		CF_ERROR_SIZE(1)
			    		REPACK_8
						break;}
			    	case 'd': {
			    		CF_ERROR_SIZE(2)
			    		REPACK_16
						break;}
					case 'l': {
						CF_ERROR_SIZE(4)
						REPACK_32
			    		break;}
					CF_ERROR_FRMT	
				}
				break;}
			case 'n': {
				CF_ERROR_SIZE(1)
				UNPACK_8(&str_size);
				CF_ERROR_SIZE(str_size+1)
				while(str_size--) *wptr++ = *rptr++;
				break;}
			case 'N': {
				CF_ERROR_SIZE(1)
				UNPACK_16(&str_size);
				CF_ERROR_SIZE(str_size+1)
				while(str_size--) *wptr++ = *rptr++;
				break;}
			case 's': {
				CF_ERROR_SIZE(MAX_CHARS)
				while((*wptr++ = *rptr++) != '\0') ;
				break;}
			case 'S': {
				CF_ERROR_SIZE(MSG_LEN)
				while((*wptr++ = *rptr++) != '\0') ;
				break;}				
			CF_ERROR_FRMT
		}
	}

	if (error) {
		found = 0;
		plog(format("Error in cq_copyf('...%s'): %s [%d.%d.%d]\n", str, pf_errors[error], str_size, src->len, dst->len));
	} else {
		dst->len += (wptr - wstart);	
		src->pos += (rptr - rstart);
	}

	return found;
}

/* Delete all nodes from "root" while deleting their "data" */
void e_release_all(eptr node, int d1, int d2) {
	eptr next = NULL;
	for (; node; node = next) {
		next = node->next;
		/* "!= NULL" saves us 1 syscall (or is useless?) */
		if (d1 && node->data1 != NULL) free(node->data1);
		if (d2 && node->data2 != NULL) free(node->data2);
		e_free_aux(node);
	}
}

