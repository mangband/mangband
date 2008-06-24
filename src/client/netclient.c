/* The client side of the networking stuff */

/* The file is very messy, and probably has a lot of stuff
 * that isn't even used.  Some massive work needs to be done
 * on it, but it's a low priority for me.
 */

/* I've made this file even worse as I have converted things to TCP.
 * rbuf, cbuf, wbuf, and qbuf are used very inconsistently.
 * rbuf is basically temporary storage that we read data into, and also
 * where we put the data before we call the command processing functions.
 * cbuf is the "command buffer".  qbuf is the "old command queue". wbuf
 * isn't used much.  The use of these buffers should probably be 
 * cleaned up.  -Alex
 */



#define CLIENT
#include "angband.h"
#include "netclient.h"

#ifndef WIN32
#include <unistd.h>
#endif

int conn_state;			/* Similar to server's connp->state */
static u32b last_keepalive;

int			ticks = 0; // Keeps track of time in 100ms "ticks"
u32b		mticks = 0; // Keeps track of time in 1ms "ticks"
static bool		request_redraw;

sockbuf_t	rbuf, cbuf, wbuf, qbuf;
static int		(*receive_tbl[256])(void),
					(*setup_tbl[256])(void);
static long		last_send_anything,
			last_keyboard_change,
			last_keyboard_ack,
			//talk_resend,
			reliable_offset,
			reliable_full_len,
			latest_reliable;
static char		talk_pend[1024], initialized = 0;

/*
 * Initialize the function dispatch tables.
 * There are two tables.  One for the semi-important unreliable
 * data like frame updates.
 * The other one is for the reliable data stream, which is
 * received as part of the unreliable data packets.
 */
static void Receive_init(void)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		receive_tbl[i] = NULL;
		setup_tbl[i] = NULL;
	}
	
	setup_tbl[PKT_QUIT]		= Receive_quit;
	setup_tbl[PKT_BASIC_INFO] = Receive_basic_info;
	setup_tbl[PKT_MOTD] = Receive_motd;
	setup_tbl[PKT_PLAY] = Receive_play;
	setup_tbl[PKT_CHAR_INFO] = Receive_char_info_conn;
	setup_tbl[PKT_KEEPALIVE]	= Receive_keepalive;
	setup_tbl[PKT_END]		= Receive_end;

	receive_tbl[PKT_MOTD] = Receive_motd;
	receive_tbl[PKT_QUIT]		= Receive_quit;
	receive_tbl[PKT_START]		= Receive_start;
	receive_tbl[PKT_END]		= Receive_end;

/*	receive_tbl[PKT_CHAR]		= Receive_char;*/
	

	/*reliable_tbl[PKT_LEAVE]	= Receive_leave;*/

	receive_tbl[PKT_QUIT]		= Receive_quit;
	receive_tbl[PKT_STAT]		= Receive_stat;
    receive_tbl[PKT_MAXSTAT]		= Receive_maxstat;
	receive_tbl[PKT_HP]		= Receive_hp;
	receive_tbl[PKT_AC]		= Receive_ac;
	receive_tbl[PKT_INVEN]		= Receive_inven;
	receive_tbl[PKT_EQUIP]		= Receive_equip;
	receive_tbl[PKT_CHAR_INFO]	= Receive_char_info;
	receive_tbl[PKT_VARIOUS]	= Receive_various;
	receive_tbl[PKT_PLUSSES]	= Receive_plusses;
	receive_tbl[PKT_EXPERIENCE]	= Receive_experience;
	receive_tbl[PKT_GOLD]		= Receive_gold;
	receive_tbl[PKT_SP]		= Receive_sp;
	receive_tbl[PKT_HISTORY]	= Receive_history;
	receive_tbl[PKT_OBJFLAGS]	= Receive_objflags;
	receive_tbl[PKT_CHAR]		= Receive_char;
	receive_tbl[PKT_MESSAGE]	= Receive_message;
	receive_tbl[PKT_STATE]		= Receive_state;
	receive_tbl[PKT_TITLE]		= Receive_title;
	receive_tbl[PKT_DEPTH]		= Receive_depth;
	receive_tbl[PKT_CONFUSED]	= Receive_confused;
	receive_tbl[PKT_POISON]		= Receive_poison;
	receive_tbl[PKT_STUDY]		= Receive_study;
	receive_tbl[PKT_FOOD]		= Receive_food;
	receive_tbl[PKT_FEAR]		= Receive_fear;
	receive_tbl[PKT_SPEED]		= Receive_speed;
	receive_tbl[PKT_CUT]		= Receive_cut;
	receive_tbl[PKT_BLIND]		= Receive_blind;
	receive_tbl[PKT_STUN]		= Receive_stun;
	receive_tbl[PKT_ITEM]		= Receive_item;
	receive_tbl[PKT_SPELL_INFO]	= Receive_spell_info;
	receive_tbl[PKT_DIRECTION]	= Receive_direction;
	receive_tbl[PKT_FLUSH]		= Receive_flush;
	receive_tbl[PKT_LINE_INFO]	= Receive_line_info;
	receive_tbl[PKT_SPECIAL_OTHER]	= Receive_special_other;
	receive_tbl[PKT_STORE]		= Receive_store;
	receive_tbl[PKT_STORE_INFO]	= Receive_store_info;
	receive_tbl[PKT_PLAYER_STORE_INFO]	= Receive_player_store_info;
	receive_tbl[PKT_SELL]		= Receive_sell;
	receive_tbl[PKT_TARGET_INFO]	= Receive_target_info;
	receive_tbl[PKT_SOUND]		= Receive_sound;
	receive_tbl[PKT_MINI_MAP]	= Receive_line_info;
	/* reliable_tbl[PKT_MINI_MAP]	= Receive_mini_map; */
	receive_tbl[PKT_SPECIAL_LINE]	= Receive_special_line;
	receive_tbl[PKT_FLOOR]		= Receive_floor;
	receive_tbl[PKT_PICKUP_CHECK]	= Receive_pickup_check;
	receive_tbl[PKT_PARTY]		= Receive_party;
	receive_tbl[PKT_COMMAND]	= Receive_custom_command;
	receive_tbl[PKT_SKILLS]		= Receive_skills;
	receive_tbl[PKT_PAUSE]		= Receive_pause;
	receive_tbl[PKT_CURSOR]		= Receive_cursor;
	receive_tbl[PKT_MONSTER_HEALTH]	= Receive_monster_health;
	receive_tbl[PKT_KEEPALIVE]	= Receive_keepalive;
}

int Send_verify_visual(int type)
{
	int	n, i, size;
	byte *attr_ref;
	char *char_ref;
	switch (type) {
		case 0:
			size = MAX_FLVR_IDX;
			attr_ref = &Client_setup.flvr_x_attr;
			char_ref = &Client_setup.flvr_x_char;
			break;
		case 1:
			size = MAX_F_IDX;
			attr_ref = &Client_setup.f_attr;
			char_ref = &Client_setup.f_char;
			break;
		case 2:
			size = MAX_K_IDX;
 			attr_ref = &Client_setup.k_attr;
  			char_ref = &Client_setup.k_char;
  			break;
  		case 3:
			size = MAX_R_IDX;
			attr_ref = &Client_setup.r_attr;
			char_ref = &Client_setup.r_char;
			break;
		case 4:
	 		size = 128;
			attr_ref = &Client_setup.tval_attr;
			char_ref = &Client_setup.tval_char;
			break;
		case 5:
			size = 256;
			attr_ref =	&Client_setup.misc_attr;
			char_ref =	&Client_setup.misc_char;
			break;
		default:
			return 0;
	}

	if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_VERIFY, (byte)type, size)) <= 0)
	{
		return n;
	}
	
	for (i = 0; i < size; i++)
	{
		if ((n = Packet_printf(&wbuf, "%c%c", attr_ref[i], char_ref[i])) <= 0)
		{
			return n;
		}
	}
	

	return 1;
} 
/*
 * NET VERIFY LIVES !!!  
 */
int Net_verify()
{
	int i;
	/* There are 5 char/attr streams, go throu all of them */
	for (i = 0; i < 6; i++)
	{
		Send_verify_visual(i);
	}
	return 0;
}


/*
 * Open the datagram socket and allocate the network data
 * structures like buffers.
 * Currently there are three different buffers used:
 * 1) wbuf is used only for sending packets (write/printf).
 * 2) rbuf is used for receiving packets in (read/scanf).
 * 3) cbuf is used to copy the reliable data stream
 *    into from the raw and unreliable rbuf packets.
 */
int Net_init(int fd)
{
	int		 sock;

	/*signal(SIGPIPE, SIG_IGN);*/

	Receive_init();

	sock = fd;

	wbuf.sock = sock;
	//if (SetSocketNonBlocking(sock, 1) == -1)
	//{
	//	plog("Can't make socket non-blocking");
	//	return -1;
	//}
	
	if (SetSocketNoDelay(sock, 1) == -1)
	{
		plog("Can't set TCP_NODELAY on socket");
		return -1;
	}

	if (SetSocketSendBufferSize(sock, CLIENT_SEND_SIZE + 256) == -1)
		plog(format("Can't set send buffer size to %d: error %d", CLIENT_SEND_SIZE + 256, errno));
	if (SetSocketReceiveBufferSize(sock, CLIENT_RECV_SIZE + 256) == -1)
		plog(format("Can't set receive buffer size to %d", CLIENT_RECV_SIZE + 256));


	/* reliable data buffer, not a valid socket filedescriptor needed */
	if (Sockbuf_init(&cbuf, -1, CLIENT_RECV_SIZE,
		SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK) == -1)
	{
		plog(format("No memory for control buffer (%u)", CLIENT_RECV_SIZE));
		return -1;
	}

	/* queue buffer, not a valid socket filedescriptor needed */
	if (Sockbuf_init(&qbuf, -1, CLIENT_RECV_SIZE,
		SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK) == -1)
	{
		plog(format("No memory for queue buffer (%u)", CLIENT_RECV_SIZE));
		return -1;
	}

#if 0
	/* read buffer */
	if (Sockbuf_init(&rbuf, sock, CLIENT_RECV_SIZE,
		SOCKBUF_READ | SOCKBUF_DGRAM) == -1)
	{
		plog(format("No memory for read buffer (%u)", CLIENT_RECV_SIZE));
		return -1;
	}

	/* write buffer */
	if (Sockbuf_init(&wbuf, sock, CLIENT_SEND_SIZE,
		SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1)
	{
		plog(format("No memory for write buffer (%u)", CLIENT_SEND_SIZE));
		return -1;
	}
#endif
	/* read buffer */
	if (Sockbuf_init(&rbuf, sock, CLIENT_RECV_SIZE,
		SOCKBUF_READ | SOCKBUF_WRITE) == -1)
	{
		plog(format("No memory for read buffer (%u)", CLIENT_RECV_SIZE));
		return -1;
	}

	/* write buffer */
	if (Sockbuf_init(&wbuf, sock, CLIENT_SEND_SIZE,
		SOCKBUF_WRITE) == -1)
	{
		plog(format("No memory for write buffer (%u)", CLIENT_SEND_SIZE));
		return -1;
	}

	Setup.ready = Setup.wait = FALSE;

	/* reliable data byte stream offset */
	reliable_offset = 0;

	/* Initialized */
	initialized = 1;
	
	/* Advance State*/
	conn_state = CONN_SETUP;

	return 0;
}


/*
 * Cleanup all the network buffers and close the datagram socket.
 * Also try to send the server a quit packet if possible.
 */
void Net_cleanup(void)
{
	int	sock = wbuf.sock;
	char	ch;

	if (sock > 2)
	{
		ch = PKT_QUIT;
		if (DgramWrite(sock, &ch, 1) != 1)
		{
			GetSocketError(sock);
			DgramWrite(sock, &ch, 1);
		}
		Term_xtra(TERM_XTRA_DELAY, 50);

		DgramClose(sock);
	}

	Sockbuf_cleanup(&rbuf);
	Sockbuf_cleanup(&cbuf);
	Sockbuf_cleanup(&wbuf);
	Sockbuf_cleanup(&qbuf);

	// Make sure that we won't try to write to the socket again,
	// after our connection has closed
	wbuf.sock = -1;
}


/*
 * Flush the network output buffer if it has some data in it.
 * Called by the main loop before blocking on a select(2) call.
 */
int Net_flush(void)
{
	//if (talk_resend < last_turns)
	//	Send_msg(NULL);
	if (wbuf.len == 0)
	{
		wbuf.ptr = wbuf.buf;
		return 0;
	}
	if (Sockbuf_flush(&wbuf) == -1)
		return -1;
	Sockbuf_clear(&wbuf);
	last_send_anything = ticks;
	return 1;
}


/*
 * Return the socket filedescriptor for use in a select(2) call.
 */
int Net_fd(void)
{
	if (!initialized)
		return -1;
	return rbuf.sock;
}




/*
 * Process a packet which most likely is a frame update,
 * perhaps with some reliable data in it.
 */
int Net_packet(void)
{
	int		type,
			prev_type = 0,
			result;

	int (**ack_tbl)(void) = receive_tbl;
	if (conn_state == CONN_SETUP) ack_tbl = setup_tbl;
	/* Hack -- copy cbuf to rbuf since this is where this function
	 * expects the data to be.
	 */
	Sockbuf_clear(&rbuf);

	/* Hack -- assume that we have already processed any data that
	 * cbuf.ptr is pointing past.
	 */
	Sockbuf_advance(&cbuf, cbuf.ptr - cbuf.buf);
	if (Sockbuf_write(&rbuf, cbuf.ptr, cbuf.len) != cbuf.len)
	{
		plog("Can't copy reliable data to buffer");
		return -1;
	}
	Sockbuf_clear(&cbuf);

	/* Process all of the received client updates */
	while (rbuf.buf + rbuf.len > rbuf.ptr)
	{
		type = (*rbuf.ptr & 0xFF);
		if (ack_tbl[type] == NULL)
		{
			errno = 0;
			/* The player really doesn't need to know about this */
#ifdef DEBUG			
			plog(format("Received unknown packet type (%d, %d, %d), dropping", type, prev_type, conn_state));
#endif
			Sockbuf_clear(&rbuf);
			break;
		}
		else if ((result = (*ack_tbl[type])()) <= 0)
		{
			if (result == -1)
			{
				if (type != PKT_QUIT)
				{
					errno = 0;
					plog(format("Processing packet type (%d, %d) failed", type, prev_type));
				}
				return -1;
			}
			if (result == -2)
			{
				ack_tbl = (conn_state == CONN_PLAYING ? receive_tbl : setup_tbl);
				continue;
			}
			Sockbuf_clear(&rbuf);
			break;
		}
		prev_type = type;
	}
	return 0;
}


/*
 * Read a packet into one of the input buffers.
 * If it is a frame update then we check to see
 * if it is an old or duplicate one.  If it isn't
 * a new frame then the packet is discarded and
 * we retry to read a packet once more.
 * It's a non-blocking read.
 */
static int Net_read(void)
{
	int	n;

	for (;;)
	{
		/* Wait for data to appear -- Dave Thaler */
		while (!SocketReadable(Net_fd()));

		if ((n = Sockbuf_read(&rbuf)) == -1)
		{
			plog("Net input error");
			return -1;
		}
		if (rbuf.len <= 0)
		{
			Sockbuf_clear(&rbuf);
			return 0;
		}

		/* If this is the end of a chunk of info, quit reading stuff */
		if (rbuf.ptr[rbuf.len - 1] == PKT_END)
			return 1;
	}
}


/*
 * Read frames from the net until there are no more available.
 * If the server has floaded us with frame updates then we should
 * discard everything except the most recent ones.  The X server
 * may be too slow to keep up with the rate of the XPilot server
 * or there may have been a network hickup if the net is overloaded.
 */
int Net_input(void)
{
	int	n;

	/* First, clear the buffer */
	Sockbuf_clear(&rbuf);

	/* Get some new data */
	if ((n = Net_read()) <= 0)
	{
		return n;
	}

	/* Write the received data to the command buffer */
	if (Sockbuf_write(&cbuf, rbuf.ptr, rbuf.len) != rbuf.len)
	{
		plog("Can't copy reliable data to buffer in Net_input");
		return -1;
	}

	n = Net_packet();

	Sockbuf_clear(&rbuf);

	if (n == -1)
		return -1;

	return 1;
}

int Flush_queue(void)
{
	short	len;

	if (!initialized) return 0;

	len = qbuf.len;

	if (cbuf.ptr > cbuf.buf)
		Sockbuf_advance(&cbuf, cbuf.ptr - cbuf.buf);
	if (Sockbuf_write(&cbuf, qbuf.ptr, len) != len)
	{
		errno = 0;
		plog("Can't copy queued data to buffer");
		qbuf.ptr += len;
		Sockbuf_advance(&qbuf, qbuf.ptr - qbuf.buf);
		return -1;
	}
	Sockbuf_clear(&qbuf);

	Net_packet();

#if 0
/*	reliable_offset += len; */
	qbuf.ptr += len;
	Sockbuf_advance(&qbuf, qbuf.ptr - qbuf.buf);
#endif
	/* If a redraw has been requested, send the request */
	if (request_redraw)
	{
		Send_redraw();
		request_redraw = FALSE;
	}

	return 1;
}

bool Net_Send(int Socket, sockbuf_t* ibuf)
{
	int bytes;
	/* Send the info */
	if ((bytes = DgramWrite(Socket, ibuf->buf, ibuf->len) == -1))
	{
		return FALSE;
	}

	return TRUE;
}

bool Net_WaitReply(int Socket, sockbuf_t* ibuf, int max_retries)
{
	int retries;
	/* Listen for reply */
	for (retries = 0; retries < max_retries; retries++)
	{
		/* Set timeout */
		SetTimeout(1, 0);

		/* Wait for info */
		if (!SocketReadable(Socket)) continue;

		/* Read reply */
		if(DgramRead(Socket, ibuf->buf, ibuf->size) <= 0)
		{
			/*printf("DgramReceiveAny failed (errno = %d)\n", errno);*/
			continue;
		}

		break;
	}
	
	if (retries >= max_retries)
	{
		return FALSE;
	}

	return TRUE;
}	

/*
 * Receive the beginning of a new frame update packet,
 * which contains the loops number.
 */
int Receive_start(void)
{
	int	n;
	long	loops;
	byte	ch;
	long	key_ack;

	if ((n = Packet_scanf(&rbuf, "%c%ld%ld", &ch, &loops, &key_ack)) <= 0)
			return n;

	/*
	if (last_turns >= loops)
	{
		printf("ignoring frame (%ld)\n", last_turns - loops);
		return 0;
	}
	last_turns = loops;
	*/
	if (key_ack > last_keyboard_ack)
	{
		if (key_ack > last_keyboard_change)
		{
			printf("Premature keyboard ack by server (%ld,%ld,%ld)\n",
				last_keyboard_change, last_keyboard_ack, key_ack);
			return 0;
		}
		else last_keyboard_ack = key_ack;
	}
#if 0
	if ((n = Handle_start(loops)) == -1)
		return -1;
#endif

	return 1;
}

/*
 * Receive the end of a new frame update packet,
 * which should contain the same loops number
 * as the frame head.  If this terminating packet
 * is missing then the packet is corrupt or incomplete.
 */
int Receive_end(void)
{
	int	n;
	byte	ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
		return n;

#if 0
	if ((n = Handle_end(loops)) == -1)
		return -1;
#endif

	return 1;
}



int Send_motd(int off)
{
	int	n;
	int offset = off;
	
	if (offset < 0) offset = 0;

	if ((n = Packet_printf(&wbuf, "%c%ld", PKT_MOTD, offset)) <= 0)
	{
		return n;
	}

	if (off == 0)
	{
		Setup.ready = FALSE;
		Setup.wait = TRUE;
	}

	return 1;
}

int Send_char_info()
{
	int	n, i;
	if ((n = Packet_printf(&wbuf, "%c%hu%hu%hu", PKT_CHAR_INFO, race, class, sex)) <= 0)
	{
		return n;
	}

	/* Send the desired stat order */
	for (i = 0; i < 6; i++)
	{
			n = Packet_printf(&wbuf, "%hd", stat_order[i]);
			if (n < 0) {
						return n;
			}
	}

	return 1;
}

int Send_play(int mode)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_PLAY, (byte)mode)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_ack(long rel_loops)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%ld%ld", PKT_ACK,
		reliable_offset, rel_loops)) <= 0)
	{
		if (n == 0)
			return 0;
		plog("Can't ack reliable data");
		return -1;
	}

	return 1;
}

/* 
 * Decodes a (possibly) RLE-encoded stream of attr/char pairs
 *
 * See "rle_encode" for possible "mode" descriptions.
 *
 * Note -- if "draw" is -1, the packets will be read for no effect 
 * Note -- setting "lineref" to NULL enables to-screen drawing, in that 
 * 		  case 'draw' is used to convey the Y coordinate
 */ 
int rle_decode(sockbuf_t* buf, cave_view_type* lineref, int max_col, int mode, s16b draw)
{
	int	x, i;
	char c;
	byte a, n;
	s16b y = draw;
		
	for (x = 0; x < max_col; x++)
	{
		/* Read the char/attr pair */
		Packet_scanf(buf, "%c%c", &c, &a);

		/* RLE-II - Test for 0xFF in the attribute */
		if ((mode == RLE_LARGE) && (a == 0xFF))
		{
			/* Get the number of repetitions */
			n = c;
			
			/* Read the attr/char pair */
			Packet_scanf(buf, "%c%c", &c, &a);
		}
		/* RLE-I - Check for bit 0x40 on the attribute */
		else if ((mode == RLE_CLASSIC) && (a & 0x40))
		{
			/* First, clear the bit */
			a &= ~(0x40);

			/* Read the number of repetitions */
			Packet_scanf(buf, "%c", &n);
		}
		else
		{
			/* No RLE, just one instance */
			n = 1;
		}

		/* Draw a character n times */
		if (draw >= 0)
		{
			for (i = 0; i < n; i++)
			{
				if (lineref) 
				{
					/* Memorize */
					lineref[x+i].a = a;
					lineref[x+i].c = c;
				} 
				else if (c)
				{				
					/* Don't draw on screen if character is 0 */
					Term_draw(x + i, y, a, c);
				}
			}
		}
		/* Reset 'x' to the correct value */
		x += n - 1;

		/* hack -- if x > allowed, assume we have received corrupted data,
		 * flush our buffers 
		if (x > max_col) 
		{
			Sockbuf_clear(&rbuf);
			Sockbuf_clear(&cbuf);
		}
		*/ 
	}
}


int old_Receive_reliable(void)
{
	int	n;
	short	len;
	byte	ch;
	long	rel, rel_loops, full_len;

	if ((n = Packet_scanf(&rbuf, "%c%hd%ld%ld%ld", &ch, &len, &rel, &rel_loops, &full_len)) == -1)
		return -1;

	if (n == 0)
	{
		errno = 0;
		plog("Incomplete reliable data packet");
		return 0;
	}

	if (len <= 0)
	{
		errno = 0;
		plog(format("Bad reliable data length (%d)", len));
		return -1;
	}

	if (full_len <= 0)
	{
		errno = 0;
		plog(format("Bad reliable data full length (%d)", reliable_full_len));
		return -1;
	}

	/* Track maximum full length */
	if (full_len > reliable_full_len || rel_loops > latest_reliable)
	{
		reliable_full_len = full_len;
		latest_reliable = rel_loops;
		//last_turns = rel_loops;
	}

	if (rbuf.ptr + len > rbuf.buf + rbuf.len)
	{
		errno = 0;
		plog(format("Not all reliable data in packet (%d, %d, %d)",
			rbuf.ptr - rbuf.buf, len, rbuf.len));
		rbuf.ptr += len;
		Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
		return -1;
	}
	if (rel > reliable_offset)
	{
		rbuf.ptr += len;
		Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
		if (Send_ack(rel_loops) == -1)
			return -1;
		return 1;
	}
	if (rel + len <= reliable_offset)
	{
		rbuf.ptr += len;
		Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
		if (Send_ack(rel_loops) == -1)
			return -1;
		return 1;
	}
	if (rel < reliable_offset)
	{
		len -= reliable_offset - rel;
		rbuf.ptr += reliable_offset - rel;
		rel = reliable_offset;
	}
	if (cbuf.ptr > cbuf.buf)
		Sockbuf_advance(&cbuf, cbuf.ptr - cbuf.buf);
	if (Sockbuf_write(&cbuf, rbuf.ptr, len) != len)
	{
		errno = 0;
		plog("Can't copy reliable data to buffer");
		rbuf.ptr += len;
		Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
		return -1;
	}
	reliable_offset += len;
	rbuf.ptr += len;
	Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
	if (Send_ack(rel_loops) == -1)
		return -1;
	return 1;
}

int Receive_reliable(void)
{
	if (Sockbuf_write(&cbuf, rbuf.ptr, rbuf.len) != rbuf.len)
	{
		plog("Can't copy reliable data to buffer");
		return -1;
	}
	Sockbuf_clear(&rbuf);
	return 1;

}


int Receive_reply(int *replyto, int *result)
{
	int	n;
	byte	type, ch1, ch2;

	n = Packet_scanf(&cbuf, "%c%c%c", &type, &ch1, &ch2);
	if (n <= 0)
		return n;
	if (n != 3 || type != PKT_REPLY)
	{
		plog("Can't receive reply packet");
		return 1;
	}
	*replyto = ch1;
	*result = ch2;
	return 1;
}

int Receive_quit(void)
{
	unsigned char		pkt;
	sockbuf_t		*sbuf;
	char			reason[MAX_CHARS];

	if (rbuf.ptr < rbuf.buf + rbuf.len)
		sbuf = &rbuf;
	else sbuf = &cbuf;
	if (Packet_scanf(sbuf, "%c", &pkt) != 1)
	{
		errno = 0;
		plog("Can't read quit packet");
	}
	else
	{
		if (Packet_scanf(sbuf, "%s", reason) <= 0)
			strcpy(reason, "unknown reason");
		errno = 0;
		quit(format("Quitting: %s", reason));
	}
	return -1;
}

int Receive_stat(void)
{
	int	n;
	char	ch;
	char	stat;
	s16b	max, cur;

	if ((n = Packet_scanf(&rbuf, "%c%c%hd%hd", &ch, &stat, &max, &cur)) <= 0)
	{
		return n;
	}

	p_ptr->stat_top[(int) stat] = max;
	p_ptr->stat_use[(int) stat] = cur;

	if (!screen_icky && !shopping)
        prt_stat(stat, max, cur, (p_ptr->stat_max[(int) stat] == 18+100));
	else
		if ((n = Packet_printf(&qbuf, "%c%c%hd%hd", ch, stat, max, cur)) <= 0)
		{
			return n;
		}


	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_maxstat(void)
{
    int	n;
    char	ch;
    char	stat;
    s16b	max;

    if ((n = Packet_scanf(&rbuf, "%c%c%hd", &ch, &stat, &max)) <= 0)
    {
        return n;
    }

    p_ptr->stat_max[(int) stat] = max;

    if (!screen_icky && !shopping)
        prt_stat(stat, p_ptr->stat_top[(int) stat], p_ptr->stat_use[(int) stat], (max == 18+100));
    else
        if ((n = Packet_printf(&qbuf, "%c%c%hd", ch, stat, max)) <= 0)
        {
            return n;
        }


    /* Window stuff */
    p_ptr->window |= (PW_PLAYER);

    return 1;
}
int Receive_hp(void)
{
	int	n;
	char 	ch;
	s16b	max, cur;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd", &ch, &max, &cur)) <= 0)
	{
		return n;
	}

	p_ptr->mhp = max;
	p_ptr->chp = cur;

	if (!screen_icky && !shopping)
		prt_hp(max, cur);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd%hd", ch, max, cur)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_ac(void)
{
	int	n;
	char	ch;
	s16b	base, plus;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd", &ch, &base, &plus)) <= 0)
	{
		return n;
	}

	p_ptr->dis_ac = base;
	p_ptr->dis_to_a = plus;

	if (!screen_icky && !shopping)
		prt_ac(base + plus);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd%hd", ch, base, plus)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_floor(void)
{
	int	n;
	char	ch;
	byte tval, attr;
	s16b amt;
	char name[80];

	if ((n = Packet_scanf(&rbuf, "%c%c%hd%c%s", &ch, &attr, &amt, &tval, name)) <= 0)
	{
		return n;
	}

	/* Remember for later */
	floor_amt = amt;
	floor_tval = tval;
	floor_attr = attr;
	
	strncpy(floor_name, name, 79);
	fix_floor();
	return 1;
}

int Receive_inven(void)
{
	int	n;
	char	ch;
	char pos, attr, tval;
	s16b wgt, amt;
	char name[80];

	if ((n = Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%s", &ch, &pos, &attr, &wgt, &amt, &tval, name)) <= 0)
	{
		return n;
	}

	/* Hack -- The color is stored in the sval, since we don't use it for anything else */
	inventory[pos - 'a'].sval = attr;
	inventory[pos - 'a'].tval = tval;
	inventory[pos - 'a'].weight = wgt;
	inventory[pos - 'a'].number = amt;

	strncpy(inventory_name[pos - 'a'], name, 79);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	return 1;
}

int Receive_equip(void)
{
	int	n;
	char 	ch;
	char pos, attr, tval;
	s16b wgt;
	char name[80];

	if ((n = Packet_scanf(&rbuf, "%c%c%c%hu%c%s", &ch, &pos, &attr, &wgt, &tval, name)) <= 0)
	{
		return n;
	}

	inventory[pos - 'a' + INVEN_WIELD].sval = attr;
	inventory[pos - 'a' + INVEN_WIELD].tval = tval;
	inventory[pos - 'a' + INVEN_WIELD].weight = wgt;
	inventory[pos - 'a' + INVEN_WIELD].number = 1;


	strncpy(inventory_name[pos - 'a' + INVEN_WIELD], name, 79);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP);

	return 1;
}

/* Ready to play */
int Receive_play(void)
{
	int n;
	char ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}
	
	conn_state = CONN_PLAYING;

	return -2;
}
int Receive_motd(void)
{
	int n, i, off, len;
	char ch;
	bool show = FALSE;

	len = off = 0;

	if ((n = Packet_scanf(&rbuf, "%c%ld%ld", &ch, &len, &off)) <= 0)
	{
		return n;
	}

	for (i = 0; i < len; i++)
	{
		if ((n = Packet_scanf(&rbuf, "%c", &Setup.motd[off+i])) <= 0)
		{
			return n;
		}
	}

	if (len == 0)
	{
		Setup.ready = TRUE;
		
		if (conn_state == CONN_SETUP)
		{
			if (Setup.wait) {
				Send_play(1);
				show_motd();
				Setup.wait = FALSE;
			}
		}
	}
	else 
	{
		/* Request continuation */		
		Send_motd(off + i);
	}

	return 1;
}
int Receive_basic_info(void)
{
	int n;
	int graf, snd, tmp3, tmp4, tmp5, tmp6, tmp7;
	char ch;

	/* Clear any old info */
	graf = snd = tmp3 = tmp4 = tmp5 = tmp6 = tmp7 = 0;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%hd%hd%hd%hd", &ch, &Setup.frames_per_second, &graf, &snd, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7)) <= 0)
	{
		return n;
	}

	/* Server has restrictions on graphics */
	if (graf < use_graphics) 
	{
		set_graphics(graf);
	}

	return 1;
}
int Receive_char_info_conn(void)
{
	int	n, mode;
	char	ch;

	/* Clear any old info */
	race = class = sex = 0;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%hd%hd", &ch, &mode, &race, &class, &sex)) <= 0)
	{
		return n;
	}

	/* No character */
	if (mode == 0)
	{
		get_char_info();
		Send_char_info();
	}

	p_ptr->prace = race;
	p_ptr->pclass = class;
	p_ptr->male = sex;
	
	/* Hack -- assume ready */
	if (Setup.frames_per_second && mode)
	{
			client_ready();
	}
	
	return 1;
}

int Receive_char_info(void)
{
	int	n;
	char	ch;
	static bool pref_files_loaded = FALSE;

	/* Clear any old info */
	race = class = sex = 0;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%hd", &ch, &race, &class, &sex)) <= 0)
	{
		return n;
	}

	p_ptr->prace = race;
	p_ptr->pclass = class;
	p_ptr->male = sex;

	if (!screen_icky && !shopping)
		prt_basic();
	else
		if ((n = Packet_printf(&qbuf, "%c%hd%hd%hd", ch, race, class, sex)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_various(void)
{
	int	n;
	char	ch;
	s16b	hgt, wgt, age, sc;

	if ((n = Packet_scanf(&rbuf, "%c%hu%hu%hu%hu", &ch, &hgt, &wgt, &age, &sc)) <= 0)
	{
		return n;
	}

	p_ptr->ht = hgt;
	p_ptr->wt = wgt;
	p_ptr->age = age;
	p_ptr->sc = sc;

	/*printf("Received various info: height %d, weight %d, age %d, sc %d\n", hgt, wgt, age, sc);*/

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_plusses(void)
{
	int	n;
	char	ch;
	s16b	dam, hit;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd", &ch, &hit, &dam)) <= 0)
	{
		return n;
	}

	p_ptr->dis_to_h = hit;
	p_ptr->dis_to_d = dam;

	/*printf("Received plusses: +%d tohit +%d todam\n", hit, dam);*/

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_experience(void)
{
	int	n;
	char	ch;
	int	max, cur, adv;
	s16b	lev;

	if ((n = Packet_scanf(&rbuf, "%c%hu%d%d%d", &ch, &lev, &max, &cur, &adv)) <= 0)
	{
		return n;
	}

	p_ptr->lev = lev;
	p_ptr->max_exp = max;
	p_ptr->exp = cur;
	exp_adv = adv;

	if (!screen_icky && !shopping)
		prt_level(lev, max, cur, adv);
	else
		if ((n = Packet_printf(&qbuf, "%c%hu%d%d%d", ch, lev, max, cur, adv)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_gold(void)
{
	int	n;
	char	ch;
	int	gold;

	if ((n = Packet_scanf(&rbuf, "%c%d", &ch, &gold)) <= 0)
	{
		return n;
	}

	p_ptr->au = gold;

	if (shopping)
	{
	        char out_val[64];

	        prt("Gold Remaining: ", 19, 53);

	        sprintf(out_val, "%9ld", (long) gold);
	        prt(out_val, 19, 68);
	}
	else if (!screen_icky)
		prt_gold(gold);
	else
		if ((n = Packet_printf(&qbuf, "%c%d", ch, gold)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_sp(void)
{
	int	n;
	char	ch;
	s16b	max, cur;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd", &ch, &max, &cur)) <= 0)
	{
		return n;
	}

	p_ptr->msp = max;
	p_ptr->csp = cur;

	if (!screen_icky && !shopping)
		prt_sp(max, cur);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd%hd", ch, max, cur)) <= 0)
		{
			return n;
		}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_objflags(void)
{
	char	ch, c;
	int	n, x, i;
	s16b	y;
	byte	a;
	
	if ((n = Packet_scanf(&rbuf, "%c%hu", &ch, &y)) <= 0)
	{
		return n;
	}
	
	rle_decode(&rbuf, &p_ptr->hist_flags[y], 13, RLE_CLASSIC, 0);
	
	/* No RLE mode
	for (x = 0; x < 13; x++)
	{
		
		Packet_scanf(&rbuf, "%c%c", &a, &c);
		p_ptr->hist_flags[y][x].a = a;
		p_ptr->hist_flags[y][x].c = c;
		
	}
	*/

	return 1;
}


int Receive_history(void)
{
	int	n;
	char	ch;
	s16b	line;
	char	buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%hu%s", &ch, &line, buf)) <= 0)
	{
		return n;
	}

	strcpy(p_ptr->history[line], buf);

	/*printf("Received history line %d: %s\n", line, buf);*/

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_char(void)
{
	int	n;
	char	ch;
	char	x, y;
	char	a, c;
	char	tap,tcp;

	if ((n = Packet_scanf(&rbuf, "%c%c%c%c%c", &ch, &x, &y, &a, &c)) <= 0)
	{
		return n;
	}

	if (use_graphics > 1) {
		if ((n = Packet_scanf(&rbuf, "%c%c", &tap, &tcp)) <= 0)
		{
			return n;
		}
		if (p_ptr->trn_info[y][x].a != tap || p_ptr->trn_info[y][x].c != tcp) 
		{
			p_ptr->trn_info[y][x].a = tap;
			p_ptr->trn_info[y][x].c = tcp;
			/* Hack -- force refresh of that grid no matter what */
			Term->scr->a[y][x] = 0;
			Term->scr->c[y][x] = 0;
			Term->old->a[y][x] = 0;
			Term->old->c[y][x] = 0;
		}
	}

	if (!screen_icky && !shopping)
	{
		Term_draw(x, y, a, c);

		/* Put the cursor there */
		Term_gotoxy(x, y);
	}
	/* Queue for later */
	else if (use_graphics > 1)
	{
		if ((n = Packet_printf(&qbuf, "%c%c%c%c%c%c%c", ch, x, y, a, c, tap, tcp)) <= 0)
		{
			return n;
		}
	}
	else
	{
		if ((n = Packet_printf(&qbuf, "%c%c%c%c%c", ch, x, y, a, c)) <= 0)
		{
			return n;
		}
	}

	return 1;
}

int Receive_message(void)
{
	int	n, c;
	char	ch;
	char	buf[1024], search[1024], *ptr;

	if ((n = Packet_scanf(&rbuf, "%c%s", &ch, buf)) <= 0)
	{
		return n;
	}

	/* XXX Mega-hack -- because we are not using checksums, sometimes under
	 * heavy traffic Receive_line_input receives corrupted data, which will cause
	 * the run-length encoded algorithm to exit prematurely.  Since there is no 
	 * end of line_info tag, the packet interpretor assumes the line_input data
	 * is finished, and attempts to parse the next byte as a new packet type.
	 * Since the ascii value of '.' (a frequently updated character) is 46, 
	 * which equals PKT_MESSAGE, if corrupted line_info data is received this function
	 * may get called wihtout a valid string.  To try to prevent the client from
	 * displaying a garbled string and messing up our screen, we will do a quick
	 * sanity check on the string.  Note that this might screw up people using
	 * international character sets.  (Such as the Japanese players)
	 * 
	 * A better solution for this would be to impliment some kind of packet checksum.
	 * -APD
	 */

	/* perform a sanity check on our string */
	/* Hack -- ' ' is numericall the lowest charcter we will probably be trying to
	 * display.  This might screw up international character sets.
	 */
	for (c = 0; c < n; c++) if (buf[c] < ' ') return 1;

/*	printf("Message: %s\n", buf);*/

	sprintf(search, "%s] ", nick);

	if (strstr(buf, search) != 0)
	{
		ptr = strstr(talk_pend, strchr(buf, ']') + 2);
// [grk] hack not needed for WIN32 client
#ifndef WINDOWS
		ptr = strtok(ptr, "\t");
		ptr = strtok(NULL, "\t");
#endif
		if (ptr) strcpy(talk_pend, ptr);
		else strcpy(talk_pend, "");
	}

	if (!topline_icky && (party_mode || shopping || !screen_icky))
	{
		c_msg_print(buf);
	}
	else
	{
		c_message_add(buf);
	}
	/*
		if ((n = Packet_printf(&qbuf, "%c%s", ch, buf)) <= 0)
		{
			return n;
		}
	*/
	return 1;
}

int Receive_state(void)
{
	int	n;
	char	ch;
	s16b	paralyzed, searching, resting;

	if ((n = Packet_scanf(&rbuf, "%c%hu%hu%hu", &ch, &paralyzed, &searching, &resting)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_state((bool)paralyzed, (bool)searching, (bool)resting);
	else
		if ((n = Packet_printf(&qbuf, "%c%hu%hu%hu", ch, paralyzed, searching, resting)) <= 0)
		{
			return n;
		}
	
	return 1;
}

int Receive_title(void)
{
	int	n;
	char	ch;
	char	buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%s", &ch, buf)) <= 0)
	{
		return n;
	}

	/* XXX -- Extract "ghost-ness" */
	p_ptr->ghost = streq(buf, "Ghost");

	if (!screen_icky && !shopping)
		prt_title(buf);
	else
		if ((n = Packet_printf(&qbuf, "%c%s", ch, buf)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_depth(void)
{
	int	n;
	char	ch;
	s16b	depth;

	if ((n = Packet_scanf(&rbuf, "%c%hu", &ch, &depth)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_depth(depth);
	else
		if ((n = Packet_printf(&qbuf, "%c%hu", ch, depth)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_confused(void)
{
	int	n;
	char	ch;
	bool	confused;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &confused)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_confused(confused);
	else
		if ((n = Packet_printf(&qbuf, "%c%c", ch, confused)) <= 0)
		{
			return n;
		}

	return 1;
}
	
int Receive_poison(void)
{
	int	n;
	char	ch;
	bool	poison;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &poison)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_poisoned(poison);
	else
		if ((n = Packet_printf(&qbuf, "%c%c", ch, poison)) <= 0)
		{
			return n;
		}

	return 1;
}
	
int Receive_study(void)
{
	int	n;
	char	ch;
	bool	study;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &study)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_study(study);
	else
		if ((n = Packet_printf(&qbuf, "%c%c", ch, study)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_food(void)
{
	int	n;
	char	ch;
	u16b	food;

	if ((n = Packet_scanf(&rbuf, "%c%hu", &ch, &food)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_hunger(food);
	else
		if ((n = Packet_printf(&qbuf, "%c%hu", ch, food)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_fear(void)
{
	int	n;
	char	ch;
	bool	afraid;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &afraid)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_afraid(afraid);
	else
		if ((n = Packet_printf(&qbuf, "%c%c", ch, afraid)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_speed(void)
{
	int	n;
	char	ch;
	s16b	speed;

	if ((n = Packet_scanf(&rbuf, "%c%hd", &ch, &speed)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_speed(speed);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd", ch, speed)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_keepalive(void)
{
	int	n;
	char	ch;
	s32b	cticks;


	if ((n = Packet_scanf(&rbuf, "%c%ld", &ch, &cticks)) <= 0)
	{
		return n;
	}
	/* make sure it's the same one we sent... */

	if(cticks == last_keepalive) {
		if (!screen_icky && !shopping && conn_state == CONN_PLAYING) {
			prt_lag(cticks,mticks-cticks);
		} 
		last_keepalive=0;
	};

	return 1;
}

int Receive_cut(void)
{
	int	n;
	char	ch;
	s16b	cut;

	if ((n = Packet_scanf(&rbuf, "%c%hd", &ch, &cut)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_cut(cut);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd", ch, cut)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_blind(void)
{
	int	n;
	char	ch;
	bool	blind;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &blind)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_blind(blind);
	else
		if ((n = Packet_printf(&qbuf, "%c%c", ch, blind)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_stun(void)
{
	int	n;
	char	ch;
	s16b	stun;

	if ((n = Packet_scanf(&rbuf, "%c%hd", &ch, &stun)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !shopping)
		prt_stun(stun);
	else
		if ((n = Packet_printf(&qbuf, "%c%hd", ch, stun)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_item(void)
{
	char	ch;
	int	n, item;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !topline_icky)
	{
		c_msg_print(NULL);
		item_tester_tval = 0;

		if (!c_get_item(&item, "Which item? ", TRUE, TRUE, TRUE))
		{
			return 1;
		}

		Send_item(item);
	}
	else
		if ((n = Packet_printf(&qbuf, "%c", ch)) <= 0)
		{
			return n;
		}
	
	return 1;
}

int Receive_spell_info(void)
{
	char	ch;
	int	n;
	u16b	book, line;
	char	buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%hu%hu%s", &ch, &book, &line, buf)) <= 0)
	{
		return n;
	}

    /* Save the info... */
	strcpy(spell_info[book][line], buf);

    /* ... and wipe the next line */
    if (line < 8) spell_info[book][line+1][0] = '\0';

	return 1;
}

int Receive_direction(void)
{
	char	ch;
	int	n, dir = 0;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}

	if (!screen_icky && !topline_icky && !shopping)
	{
		/* Ask for a direction */
		get_dir(&dir);

		/* Send it back */
		if ((n = Packet_printf(&wbuf, "%c%c", PKT_DIRECTION, dir)) <= 0)
		{
			return n;
		}
	}
	else
		if ((n = Packet_printf(&qbuf, "%c", ch)) <= 0)
		{
			return n;
		}

	return 1;
}

int Receive_flush(void)
{
	char	ch;
	int	n;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}

	/* Flush the terminal */
	Term_fresh();

	/* Wait */
	Term_xtra(TERM_XTRA_DELAY, 1);

	return 1;
}


int Receive_line_info(void)
{
	char	ch, c, n;
	int	x, i;
	s16b	y;
	byte	a;
	bool	draw = FALSE;

	if ((n = Packet_scanf(&rbuf, "%c%hd", &ch, &y)) <= 0)
	{
		return n;
	}

	/* If this is the mini-map then we can draw if the screen is icky */
	if (ch == PKT_MINI_MAP || (!screen_icky && !shopping))
		draw = TRUE;
	
	/* Check the max line count */
	if (y > last_line_info)
		last_line_info = y;

	if (ch != PKT_MINI_MAP && use_graphics > 1)
	{
		/* Decode the secondary attr/char stream */		
		rle_decode(&rbuf, p_ptr->trn_info[y], 80, RLE_LARGE , 0 );
	}

	/* Decode the attr/char stream */		
	rle_decode(&rbuf, NULL, 80, (use_graphics ? RLE_LARGE : RLE_CLASSIC), ( draw ? y : -1) );

	/* Request a redraw if the screen was icky */
	if (screen_icky)
		request_redraw = TRUE;

	return 1;
}

/*
 * Note that this function does not honor the "screen_icky"
 * flag, as it is only used for displaying the mini-map,
 * and the screen should be icky during that time.
 */
int Receive_mini_map(void)
{
	char	ch, c;
	int	n, x;
	s16b	y;
	byte	a;

	if ((n = Packet_scanf(&rbuf, "%c%hd", &ch, &y)) <= 0)
	{
		return n;
	}

	/* Check the max line count */
	if (y > last_line_info)
		last_line_info = y;

	for (x = 0; x < 80; x++)
	{
		Packet_scanf(&rbuf, "%c%c", &c, &a);

		/* Don't draw anything if "char" is zero */
		/* Only draw if the screen is "icky" */
		if (c && screen_icky && x < 80 - 12)
			Term_draw(x + 12, y, a, c);
	}

	return 1;
}

int Receive_special_other(void)
{
	int	n;
	char	ch;
	char buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%s", &ch, buf)) <= 0)
	{
		return n;
	}
	
	/* Set file perusal header */
	strcpy(special_line_header, buf);

	/* Set file perusal method to "other" */
	special_line_type = SPECIAL_FILE_OTHER;

	/* Peruse the file we're about to get */
	peruse_file();

	return 1;
}

int Receive_store(void)
{
	int	n, price;
	char	ch, pos, name[1024];
	byte	attr;
	s16b	wgt, num;

	if ((n = Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%s", &ch, &pos, &attr, &wgt, &num, &price, name)) <= 0)
	{
		return n;
	}

	store.stock[pos].sval = attr;
	store.stock[pos].weight = wgt;
	store.stock[pos].number = num;
	store_prices[(int) pos] = price;
	strncpy(store_names[(int) pos], name, 80);

	/* Make sure that we're in a store */
	if (shopping)
	{
		display_inventory();
	}

	return 1;
}

int Receive_store_info(void)
{
	int	n;
	char	ch;
	s16b	owner_num, num_items;

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%hd", &ch, &store_num, &owner_num, &num_items)) <= 0)
	{
		return n;
	}

	store.stock_num = num_items;
	/* The shopkeeper of the BM looks after his "back room" */
	if (store_num != 8)
	{
		store_owner = owners[store_num][owner_num];
	}
	else
	{
		/* BM shopkeeper looks after this */
		store_owner = owners[6][owner_num];
	}

	/* Only enter "display_store" if we're not already shopping */
	if (!shopping)
		display_store();
	else
		display_inventory();

	return 1;
}

int Receive_player_store_info(void)
{
	int	n;
	char	ch;
	s16b	owner_num, num_items;

	if ((n = Packet_scanf(&rbuf, "%c%s%s%hd", &ch, player_store_name, player_owner, &num_items)) <= 0)
	{
		return n;
	}

	store_num = 8;
	store.stock_num = num_items;

	/* Only enter "display_store" if we're not already shopping */
	if (!shopping)
		display_store();
	else
		display_inventory();

	return 1;
}

int Receive_sell(void)
{
	int	n, price;
	char	ch, buf[1024];

	if ((n = Packet_scanf(&rbuf, "%c%d", &ch, &price)) <= 0)
	{
		return n;
	}

	/* Tell the user about the price */
	sprintf(buf, "Accept %d gold? ", price);

	if (get_check(buf))
		Send_store_confirm();

	return 1;
}

int Receive_target_info(void)
{
	int	n;
	char	ch, x, y, buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%c%c%s", &ch, &x, &y, buf)) <= 0)
	{
		return n;
	}

	if (buf[0] == '\0') target_position = TRUE;

	/* Print the message */
	prt(buf, 0, 0);

	/* Move the cursor */
	Term_gotoxy(x, y);
	if (cursor_icky)
		Term_consolidate_cursor(TRUE, x, y);

	return 1;
}

int Receive_sound(void)
{
	int	n;
	char	ch, sound;

	if ((n = Packet_scanf(&rbuf, "%c%c", &ch, &sound)) <= 0)
	{
		return n;
	}

	/* Make a sound (if allowed) */
	if (use_sound) Term_xtra(TERM_XTRA_SOUND, sound);

	return 1;
}

int Receive_custom_command(void)
{
	int n;
	char ch, tval;
	s16b catch;
	u32b flag;
	
	custom_command_type *cc_ptr;

	char buf[60];
	buf[0] = '\0';
	if ((n = Packet_scanf(&rbuf, "%c%hd%lu%c%S", &ch, &catch, &flag, &tval, buf)) <= 0)
	{
		return n;
	}

	if (custom_commands >= MAX_CUSTOM_COMMANDS) return 0;

	cc_ptr = &custom_command[custom_commands];
	WIPE(cc_ptr, custom_command_type);

	cc_ptr->catch = catch;
	cc_ptr->flag = flag;
	cc_ptr->tval = tval;
	buf[59] = '\0'; 
	strcat(cc_ptr->prompt, buf);

	custom_commands++;	

	return 1;
}

int Receive_special_line(void)
{
	int	n;
	char	ch, attr;
	s16b	max, line;
	char	buf[80];

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%c%s", &ch, &max, &line, &attr, buf)) <= 0)
	{
		return n;
	}

	if (!screen_icky) return 0;

	/* Maximum */
	max_line = max;

	/* Hack -- decide to go popup/fullon mode */
	if (line == 0)
	{	
		if (max > (SCREEN_HGT - 2)/2 || special_line_type != SPECIAL_FILE_OTHER) 
		{
			/* Clear the screen */
			Term_clear();
	
			/* Show a general "title" + header */
			special_line_header[60] = '\0';
	      prt(format("[Mangband %d.%d.%d] %60s",
				VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, special_line_header), 0, 0);
	
			/* Prompt (check if we have extra pages) */
			if (max > (SCREEN_HGT - 2)) 
				prt("[Press Space to advance, or ESC to exit.]", 23, 0);
			else
				prt("[Press ESC to exit.]", 23, 0);
				
		} else {
			/* Clear the screen */
			for (n = 0; n < max_line + 5; n++)
				Term_erase(0, n, 80);
			
			/* Show a specific "title" -- header */
			c_put_str(TERM_YELLOW, special_line_header, 0, 0);

			/* Prompt */
			c_put_str(TERM_L_BLUE, "[Press any key to continue]", max_line + 3, 0);
		}
	}
	
		

	/* Print out the info */
	c_put_str(attr, buf, line + 2, 0);

	return 1;
}

int Receive_pickup_check(void)
{
	int	n;
	char	ch, buf[180];

	if ((n = Packet_scanf(&rbuf, "%c%s", &ch, buf)) <= 0)
	{
		return n;
	}

	/* Get a check */
	if (get_check(buf))
	{
		/* Pick it up */
		Send_stay();
	}

	return 1;
}


int Receive_party(void)
{
	int n;
	char ch, buf[160];

	if ((n = Packet_scanf(&rbuf, "%c%s", &ch, buf)) <= 0)
	{
		return n;
	}

	/* Copy info */
	strcpy(party_info, buf);

	/* Re-show party info */
	if (party_mode)
	{
		Term_erase(0, 13, 255);
		Term_putstr(0, 13, -1, TERM_WHITE, party_info);
		Term_putstr(0, 11, -1, TERM_WHITE, "Command: ");
	}

	return 1;
}

int Receive_skills(void)
{
	int	n, i;
	s16b tmp[11];
	char	ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}

	/* Read into skills info */
	for (i = 0; i < 11; i++)
	{
		if ((n = Packet_scanf(&rbuf, "%hd", &tmp[i])) <= 0)
		{
			return n;
		}
	}

	/* Store */
	p_ptr->skill_thn = tmp[0];
	p_ptr->skill_thb = tmp[1];
	p_ptr->skill_sav = tmp[2];
	p_ptr->skill_stl = tmp[3];
	p_ptr->skill_fos = tmp[4];
	p_ptr->skill_srh = tmp[5];
	p_ptr->skill_dis = tmp[6];
	p_ptr->skill_dev = tmp[7];
	p_ptr->num_blow = tmp[8];
	p_ptr->num_fire = tmp[9];
	p_ptr->see_infra = tmp[10];

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return 1;
}

int Receive_cursor(void)
{
	int n;
	char ch;
	char vis, x, y;

	if ((n = Packet_scanf(&rbuf, "%c%c%c%c", &ch, &vis, &x, &y)) <= 0)
	{
		return n;
	}
	
	if (cursor_icky)
		Term_consolidate_cursor(vis, x, y);

	return 1;
}


int Receive_pause(void)
{
	int n;
	char ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0)
	{
		return n;
	}

	/* Show the most recent changes to the screen */
	Term_fresh();

	/* Flush any pending keystrokes */
	Term_flush();

	/* The screen is icky */
	screen_icky = TRUE;

	/* Wait */
	inkey();

	/* Screen isn't icky any more */
	screen_icky = FALSE;

	/* Flush queue */
	Flush_queue();

	/* Show the most recent changes to the screen */
	Term_fresh();

	return 1;
}


int Receive_monster_health(void)
{
	int n;
	char ch, num;
	byte attr;

	if ((n = Packet_scanf(&rbuf, "%c%c%c", &ch, &num, &attr)) <= 0)
	{
		return n;
	}

	if (!screen_icky)
	{
		/* Draw the health bar */
		health_redraw(num, attr);
	}
	else
	{
		if ((n = Packet_printf(&qbuf, "%c%c%c", ch, num, attr)) <= 0)
		{
			return n;
		}
	}

	return 1;
}

int Send_search(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_SEARCH)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_walk(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_WALK, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_run(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_RUN, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_drop(int item, int amt)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_DROP, item, amt)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_drop_gold(s32b amt)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%ld", PKT_DROP_GOLD, amt)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_tunnel(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_TUNNEL, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_stay(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STAND)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_keepalive(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%ld", PKT_KEEPALIVE,last_keepalive=mticks)) <= 0)
	{
		return n;
	}
	
	return 1;
}

int Send_toggle_search(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_SEARCH_MODE)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_rest(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_REST)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_go_up(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_GO_UP)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_go_down(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_GO_DOWN)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_open(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_OPEN, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_close(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_CLOSE, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_bash(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_BASH, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_disarm(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_DISARM, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_wield(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_WIELD, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_take_off(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_TAKE_OFF, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_destroy(int item, int amt)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_DESTROY, item, amt)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_observe(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_OBSERVE, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_inscribe(int item, cptr buf)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_INSCRIBE, item, buf)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_uninscribe(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_UNINSCRIBE, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_custom_command(byte i, int item, char dir, int value)
{
	int 	n;
	printf("com [%c/%d] >>> item=%d, dir=%d, value=%d\n", i,i,item,dir,value);
	if ((n = Packet_printf(&wbuf, "%c%c%hd%c%hd", PKT_COMMAND, i, item, dir, value)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_spike(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_SPIKE, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_steal(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_STEAL, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_quaff(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_QUAFF, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_read(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_READ, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_aim(int item, int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%c", PKT_AIM_WAND, item, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_use(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_USE, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_zap(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ZAP, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_fill(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_FILL, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_eat(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_EAT, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_activate(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ACTIVATE, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_target(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_TARGET, dir)) <= 0)
	{
		return n;
	}

	return 1;
}


int Send_target_friendly(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_TARGET_FRIENDLY, dir)) <= 0)
	{
		return n;
	}

	return 1;
}


int Send_look(int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_LOOK, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_msg(cptr message)
{
	int	n;

	if (message && strlen(message))
	{
// [grk] WIN32 client hack to allow us to send messages longer than 58 chars
#ifndef WINDOWS
		if (strlen(talk_pend))
			strcat(talk_pend, "\t");
		strcat(talk_pend, message);
#else
		strcpy(talk_pend, message);
#endif
	}

	//talk_resend = last_turns + 36;

#ifndef WINDOWS
	if (!strlen(talk_pend)) return 1;
#endif

	if ((n = Packet_printf(&wbuf, "%c%S", PKT_MESSAGE, talk_pend)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_pass(cptr newpass)
{
	int	n;

	if (newpass && strlen(newpass))
	{
	    if ((n = Packet_printf(&wbuf, "%c%S", PKT_CHANGEPASS, newpass)) <= 0)
	    {
		return n;
	    }
	
	}

	return 1;
}


int Send_fire(int item, int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_FIRE, dir, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_throw(int item, int dir)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_THROW, dir, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_item(int item)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ITEM, item)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_gain(int book, int spell)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_GAIN, book, spell)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_cast(int book, int spell)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SPELL, book, spell)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_pray(int book, int spell)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_PRAY, book, spell)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_ghost(int ability)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_GHOST, ability)) <= 0)
	{
		return n;
	}
	
	return 1;
}

int Send_map(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_MAP)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_locate(int dir)
{
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_LOCATE, dir)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_store_purchase(int item, int amt)
{
	int 	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_PURCHASE, item, amt)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_store_sell(int item, int amt)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SELL, item, amt)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_store_leave(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STORE_LEAVE)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_store_confirm(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STORE_CONFIRM)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_redraw(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_REDRAW)) <= 0)
	{
		return n;
	}
	
	/* Hack -- Clear the screen */
	Term_clear();

	return 1;
}

int Send_special_line(int type, int line)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_SPECIAL_LINE, type, line)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_party(s16b command, cptr buf)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_PARTY, command, buf)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_purchase_house(int dir)
{
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_PURCHASE, dir, 0)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_suicide(void)
{
	int n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_SUICIDE)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_options(bool settings)
{
	int i, n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_OPTIONS, (byte)settings )) <= 0)
	{
		return n;
	}

	if (settings)
	{
		/* Send each setting */
		for (i = 0; i < 16; i++)
		{
			Packet_printf(&wbuf, "%hd", Client_setup.settings[i]);
		}
	}

	/* Send each option */
	for (i = 0; i < 64; i++)
	{
		Packet_printf(&wbuf, "%c", Client_setup.options[i]);
	}

	return 1;
}

int Send_master(s16b command, cptr buf)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_MASTER, command, buf)) <= 0)
	{
		return n;
	}

	return 1;
}

int Send_clear(void)
{
	int	n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_CLEAR)) <= 0)
	{
		return n;
	}

	return 1;
}

// Update the current time, which is stored in 100 ms "ticks".
// I hope that Windows systems have gettimeofday on them by default.
// If not there should hopefully be some simmilar efficient call with the same
// functionality. 
// I hope this doesn't prove to be a bottleneck on some systems.  On my linux system
// calling gettimeofday seems to be very very fast.
void update_ticks()
{
	struct timeval cur_time;
	int newticks;
	float scale = 100000;
	int mins,hours;

// [grk] We do this slightly differently on WIN32 

#ifdef WINDOWS
	LPSYSTEMTIME lpst;
	SYSTEMTIME st;
	lpst = &st;
	GetSystemTime(lpst);

	cur_time.tv_usec = lpst->wMilliseconds; 
	cur_time.tv_sec = lpst->wSecond; 
	mins = lpst->wMinute; 
	hours = lpst->wHour; 
	scale = 100;
#else
/* 	
	hours = time(NULL) % 86400;
	mins = time(NULL) % 3600;
*/
	gettimeofday(&cur_time, NULL);
#endif

	// Set the new ticks to the old ticks rounded down to the number of seconds.
	newticks = ticks-(ticks%10);
	// Find the new least significant digit of the ticks
	newticks += cur_time.tv_usec / scale;

	// Assume that it has not been more than one second since this function was last called
	if (newticks < ticks) newticks += 10;
	ticks = newticks;	
	/*RLS*/
	mticks = (long)(hours*3600*100) + 
		(long)(mins*60*100) +
		(long)(cur_time.tv_sec*100) +
		cur_time.tv_usec/((scale/100)/10) ;
/* 
	mticks = (long)(ticks*1000 ) + cur_time.tv_usec/(scale/100) ;
	mticks = (long) ticks;
*/
}

/* Write a keepalive packet to the output queue if it has been two seconds
 * since we last sent anything.  
 * Note that if the loop that is calling this function doesn't flush the
 * network output before calling this function again very bad things could
 * happen, such as an overflow of our send queue.
 */
void do_keepalive()
{
static u32b last_sent;
	// Check to see if it has been 2 seconds since we last sent anything.  Assume
	// that each game turn lasts 100 ms.
	//if ((ticks - last_send_anything) >= 10)
		if ((mticks - last_sent) > 10000)  /* 1 second */
		{
			if(last_keepalive) { 
				if (!screen_icky && !shopping && conn_state == CONN_PLAYING) {
					prt_lag(999999999, mticks-last_keepalive); 
				} 
				last_keepalive=0;
			};
			Send_keepalive(); 
			last_sent=mticks;
		}
}

