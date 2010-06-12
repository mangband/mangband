#include "c-angband.h"

#include "../common/net-basics.h"
#include "../common/net-imps.h"

/* List heads */
eptr first_connection = NULL;
eptr first_listener = NULL;
eptr first_caller = NULL;
eptr first_timer = NULL;

/* Pointers */
eptr meta_caller = NULL;
eptr server_caller = NULL;
eptr server_connection = NULL;

connection_type *serv = NULL;

/* Global Flags */
byte connected = 0;
s16b state = 0;

static int		(*handlers[256])(connection_type *ct);
static cptr		(schemes[256]);

byte next_pkt;
cptr next_scheme;

server_setup_t serv_info;

/* Init */
void setup_tables(void);
void setup_network_client()
{
	/* Prepare FD_SETS */
	network_reset();
	/* Setup tables..? */
	// can do it later
}
void cleanup_network_client()
{
	e_release_all(first_connection, 0, 1);
	e_release_all(first_caller, 0, 1);
}

/* Iteration of the Loop */
void network_loop()
{
	//first_listener = handle_listeners(first_listener);
	first_connection = handle_connections(first_connection);
	first_caller = handle_callers(first_caller);
	//first_timer = handle_timers(first_timer, static_timer(0));

	network_pause(1000); /* 0.001 ms "sleep" */
}

int client_close(int data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	/* 0_0` */
	quit(NULL);
}

int client_read(int data1, data data2) { /* return -1 on error */
	cq queue;
	connection_type *ct = data2;

	/* parse */
	int result = 1;
	int start_pos;
	while (	cq_len(&ct->rbuf) )
	{
		/* save */
		start_pos = ct->rbuf.pos; 
		next_pkt = CQ_GET(&ct->rbuf);
		next_scheme = schemes[next_pkt];

		result = (*handlers[next_pkt])(ct);

		/* Unable to continue */
		if (result != 1) break;
	}
	/* Not enough bytes */
	if (result == 0) 
	{
		/* load */
		ct->rbuf.pos = start_pos;
	}
	/* Slide buffer to the left */
#if 0
	/* Fast version */
	else if (result == 1)
	{
		CQ_CLEAR(&ct->rbuf);
	}
#else
	/* Slow, but safer version */
	if (ct->rbuf.pos)
	{
		char buf[PD_SMALL_BUFFER];
		strncpy(buf, &ct->rbuf.buf[ct->rbuf.pos], ct->rbuf.len);
		strncpy(ct->rbuf.buf, buf, ct->rbuf.len);
		ct->rbuf.len -= ct->rbuf.pos;
		ct->rbuf.pos = 0;
	}	
#endif

	return result;
}

					/* data1 is (int)fd */
int connected_to_server(int data1, data data2) {
	int fd = (int)data1;

	/* Unset 'caller' */
	server_caller = NULL;

	/* Setup 'connection' */
	server_connection = add_connection(first_connection, fd, client_read, client_close);
	if (!first_connection) first_connection = server_connection;

	/* Disable Nagle's algorithm */
	denaglefd(fd);

	/* Set usability pointer */	
	serv = server_connection->data2;
	
	/* Prepare packet-handling tables */
	setup_tables();

	/* Is connected! */
	connected = 1;

	/* OK */	
	return 1;
}

/* Return 1 to continue, 0 to cancel */
int failed_connection_to_server(int data1, data data2) {
	/* Ask user */ 
	int r = client_failed();
	if (r == 0) connected = -1;
	return r;  
}

int call_server(char *server_name, int server_port)
{
	server_caller = add_caller(first_caller, server_name, server_port, connected_to_server, failed_connection_to_server);
	if (first_caller == NULL) first_caller = server_caller;

	/* Unset */	
	connected = 0;

	/* Try */	
	while (!connected) 
	{
		network_loop();
	}

	/* Will be either 1 either -1 */
	return connected;
}


int send_play(byte mode) {
	return cq_printf(&serv->wbuf, "%c%c", PKT_PLAY, mode);
}

int send_char_info() {
	int	n, i;
	if (n = cq_printf(&serv->wbuf, "%c%ud%ud%ud", PKT_CHAR_INFO, race, pclass, sex) <= 0)
	{
		return n;
	}

	/* Send the desired stat order */
	for (i = 0; i < 6; i++)
	{
		n = cq_printf(&serv->wbuf, "%d", stat_order[i]);
		if (n < 0) 
		{
			return n;
		}
	}

	return 1;
}

int send_login(u16b version, char* real_name, char* host_name, char* user_name, char* pass_word) {
	return cq_printf(&serv->wbuf, "%c%ud%s%s%s%s", PKT_LOGIN, version, real_name, host_name, user_name, pass_word);
}

int send_handshake(u16b conntype) {
	return cq_printf(&serv->wbuf, "%ud", conntype);
}

int send_keepalive(u32b last_keepalive) {
	return cq_printf(&serv->wbuf, "%c%l", PKT_KEEPALIVE, last_keepalive);
}

int send_request(byte mode, u16b id) {
	return cq_printf(&serv->wbuf, "%c%c%ud", PKT_BASIC_INFO, mode, id);
}



int send_msg(cptr message)
{
	return cq_printf(&serv->wbuf, "%c%S", PKT_MESSAGE, message);
}

/* Gameplay commands */
int send_walk(char dir)
{
	return cq_printf(&serv->wbuf, "%c%c", PKT_WALK, dir);
}





/* Undefined packet "handler" */
int recv_undef(connection_type *ct) {

	printf("Undefined packet %d came from server!\n", next_pkt);

	/* Disconnect client! */
	return -1;

}

/* Keepalive packet "handler" */
int recv_keepalive(connection_type *ct) {

	s32b
		cticks = 0;
	if (cq_scanf(&ct->rbuf, "%l", &cticks) < 1)
	{
		return 0;
	}
#if 0
	/* make sure it's the same one we sent... */
	if(cticks == last_keepalive) {
		if (conn_state == CONN_PLAYING) {
			lag_mark = (mticks - last_sent);
			p_ptr->redraw |= PR_LAG_METER;
		} 
		last_keepalive=0;
	};
#endif
#if 1

#endif 
	/* Ok */
	return 1;
}

/* Quit packet, server is disconnecting us */
int recv_quit(connection_type *ct) {
	char
		reason[MSG_LEN];

	if (cq_scanf(&ct->rbuf, "%S", reason) < 1) 
	{
		strcpy(reason, "unknown reason");
	}

	quit(format("Quitting: %s", reason));
}

int recv_basic_info(connection_type *ct) {

	if (cq_scanf(&ct->rbuf, "%d%d%d%d", &serv_info.val1, &serv_info.val2, &serv_info.val3, &serv_info.val4) < 4) 
	{
		/* Not enough bytes */
		return 0;
	}

	/* Ok */
	return 1;
}

/* Play packet, server is promoting us */
int recv_play(connection_type *ct) {
	byte
		mode = 0;

	if (cq_scanf(&ct->rbuf, "%b", &mode) < 1) 
	{
		/* Not enough bytes */
		return 0;
	}

	/* React */
	if (mode == 0) client_setup();
	if (mode == 1) state = PLAYER_SHAPED;

	/* Ok */
	return 1;
}

/* Character info packet, important at setup stage */
int recv_char_info(connection_type *ct) {
		state = 0;
		race = 0;
		pclass = 0;
		sex = 0;
	if (cq_scanf(&ct->rbuf, "%d%d%d%d", &state, &race, &pclass, &sex) < 4)
	{
		/* Not enough bytes */
		return 0;
	}

	p_ptr->state = state;
	p_ptr->prace = race;
	p_ptr->pclass = pclass;
	p_ptr->male = sex;

	/* Ok */
	return 1;
}

/* */
int recv_struct_info(connection_type *ct)
{
	char 	ch;
	int 	i, n;
	byte 	typ;
	u16b 	max;
	char 	name[MAX_CHARS];
	u32b 	off, fake_name_size, fake_text_size;
	byte	spell_book;

	typ = max = off = fake_name_size = fake_text_size = 0;

	if (cq_scanf(&ct->rbuf, "%c%ud%ul%ul", &typ, &max, &fake_name_size, &fake_text_size) < 4)
	{
		/* Not ready */
		return 0;
	}

	/* Which struct */
	switch (typ)
	{
#if 0	
		/* Option groups */
		case STRUCT_INFO_OPTGROUP:
			/* Alloc */
			C_MAKE(option_group, max, cptr);
			options_groups_max = max;
			
			/* Fill */
			for (i = 0; i < max; i++)
			{
				if ((n = Packet_scanf(&rbuf, "%s", name)) <= 0)
				{
					return n;
				}
				
				/* Transfer */
				option_group[i] = string_make(name);
			}
		break;
		/* Options */
		case STRUCT_INFO_OPTION:
			/* Alloc */
			C_MAKE(option_info, max, option_type);
			options_max = max;
			
			/* Fill */
			for (i = 0; i < max; i++)
			{
				option_type *opt_ptr;
				byte opt_page;
				char desc[MAX_CHARS];

				opt_ptr = &option_info[i];
				opt_page = 0;
				
				
				if ((n = Packet_scanf(&rbuf, "%c%s%s", &opt_page, name, desc)) <= 0)
				{
					return n;
				}
				
				/* Transfer */
				opt_ptr->o_page = opt_page;
				opt_ptr->o_text = string_make(name);
				opt_ptr->o_desc = string_make(desc);
				opt_ptr->o_set = 0;
				/* Link to local */
				for (n = 0; local_option_info[n].o_desc; n++)
				{
					if (!strcmp(local_option_info[n].o_text, name))
					{
						local_option_info[n].o_set = i;
						opt_ptr->o_set = n;
					}				
				}
			}
		break;
#endif		
		/* Player Races */
		case STRUCT_INFO_RACE:
			/* Alloc */
			C_MAKE(p_name, fake_name_size, char);
			C_MAKE(race_info, max, player_race);
			z_info.p_max = max;
			
			/* Fill */
			for (i = 0; i < max; i++) 
			{
				player_race *pr_ptr = NULL;
				pr_ptr = &race_info[i];
	
				off = 0;
						
				if (cq_scanf(&ct->rbuf, "%s%ul", &name, &off) < 2)
				{
					return 0;
				}

				strcpy(p_name + off, name);
			
				pr_ptr->name = off;
				/* Transfer other fields here */
			}
		break;
		/* Player Classes */
		case STRUCT_INFO_CLASS:
			/* Alloc */
			C_MAKE(c_name, fake_name_size, char);
			C_MAKE(c_info, max, player_class);
			z_info.c_max = max;
			
			/* Fill */
			for (i = 0; i < max; i++) 
			{
				player_class *pc_ptr = NULL;
				pc_ptr = &c_info[i];
	
				off = spell_book = 0;
						
				if (cq_scanf(&ct->rbuf, "%s%ul%c", &name, &off, &spell_book) < 3)
				{
					return n;
				}

				strcpy(c_name + off, name);
			
				pc_ptr->name = off;
				/* Transfer other fields here */
				pc_ptr->spell_book = spell_book;
			}
		break;
	}	
	
	return 1;
}


int recv_message(connection_type *ct) {
	char 
		mesg[80];
	u16b 
		type = 0;
	if (cq_scanf(&ct->rbuf, "%ud%s", &type, mesg) < 2) return 0;

	c_message_add(mesg, type);	

	return 1;
}




void setup_tables()
{
	/* Setup receivers */
	int i;
	for (i = 0; i < 256; i++) {
		handlers[i] = recv_undef;
		schemes[i] = NULL;	
	}

#define PACKET(PKT, SCHEME, FUNC) \
	handlers[PKT] = FUNC; \
	schemes[PKT] = SCHEME;
#include "net-client.h"
#undef PACKET

}



bool net_term_clamp(byte win, byte *y, byte *x)
{
	stream_type* st_ptr;
	s16b nx = (*x);
	s16b ny = (*y);
	s16b xoff = 0;
	s16b yoff = 0;

	st_ptr = &streams[window_to_stream[win]];

	/* Shift expectations */
	if (st_ptr->addr == NTERM_WIN_OVERHEAD) 
	{
		yoff = SCREEN_CLIP_L;
		if (st_ptr->flag & SF_KEEP_X)	xoff += DUNGEON_OFFSET_X;
		if (st_ptr->flag & SF_KEEP_Y)	yoff += DUNGEON_OFFSET_Y;
	} 
	else 
	{
		if (st_ptr->flag & SF_KEEP_X)	xoff = SCREEN_CLIP_X;
		if (st_ptr->flag & SF_KEEP_Y)	yoff = SCREEN_CLIP_Y;
	}	

	/* Perform actual clamping */
	if (nx < st_ptr->min_col + xoff) nx = st_ptr->min_col + xoff; 
	if (nx > st_ptr->max_col + xoff) nx = st_ptr->max_col + xoff; 

	if (ny < st_ptr->min_row + yoff) ny = st_ptr->min_row + yoff; 
	if (ny > st_ptr->max_row + yoff) ny = st_ptr->max_row + yoff;

	/* Compare old and new values */
	if (nx != (*x) || ny != (*y))
	{
		/* Update */
		(*x) = (byte)nx;
		(*y) = (byte)ny;
		/* Return change */
		return TRUE;
	}
	/* Return no change */
	return FALSE;
}
/*
 * Manage Stream Subscriptions.
 * This function tests ALL active windows for changes.
 *  The changes are provided in form of 2 sets of flags - old and new.  It is caller's 
 *  responsibility to prepare those sets. An empty (zero-filled) array for old_flags is 
 *  acceptable. Both arrays shouldn't exceed ANGBAND_TERM_MAX. 
 *  If 'clear' is set to TRUE, each affected window is also cleared.
 * Returns a local window update mask.
 *  Applying it to p-ptr->window should redraw all affected windows.
 */
u32b net_term_manage(u32b* old_flag, u32b* new_flag, bool clear)
{
	int j;
	int k;

	/* Track changes */
	s16b st_y[MAX_STREAMS];
	s16b st_x[MAX_STREAMS];
	u32b st_flag = 0;

	/* Clear changes */
	for (j = 0; j < MAX_STREAMS; j++)
	{
		/* Default change is 'unchanged' */
		st_y[j] = -1;
		st_x[j] = -1;
	}

	/* Now, find actual changes by comparing old and new */ 
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!ang_term[j]) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Determine stream groups affected by this window */
		for (k = 0; k < stream_groups; k++) 
		{
			byte st = stream_group[k];
			stream_type* st_ptr = &streams[st];

			/* The stream is unchanged or turned off */
			if (st_y[st] <= 0)
			{
				/* It's now active */
				if ((new_flag[j] & st_ptr->window_flag))
				{
					/* It wasn't active or it's size changed. Subscribe! */
					if (!(old_flag[j] & st_ptr->window_flag) || Term->wid != p_ptr->stream_wid[st] || Term->hgt != p_ptr->stream_hgt[st]) 
					{
						st_y[st] = Term->hgt;
						st_x[st] = Term->wid;
					}
				}
				/* Trying to turn it off */
				else if ((old_flag[j] & st_ptr->window_flag))
				{
					st_y[st] = 0;
					st_x[st] = 0;
				}
			}
		}

		/* Ignore visible changes */
		if (clear)
		{
			/* Erase */ 
			Term_clear();

			/* Refresh */
			Term_fresh();
		}

		/* Restore */
		Term_activate(old);
	}

	/* Send subscriptions */
	for (j = 0; j < known_streams; j++) 
	{
		/* A change is scheduled */
		if (st_y[j] != -1) 
		{
			/* We try to subscribe/resize */
			if (st_y[j]) 
			{ 
				/* HACK -- Dungeon Display Offsets */
				if (streams[j].addr == NTERM_WIN_OVERHEAD)
				{
					/* Compact display */
					st_x[j] = st_x[j] - DUNGEON_OFFSET_X;
					/* Status and top line */
					st_y[j] = st_y[j] - SCREEN_CLIP_L - DUNGEON_OFFSET_Y;
				}

				/* Test bounds */
				if (st_x[j] < streams[j].min_col) st_x[j] = streams[j].min_col;
				if (st_x[j] > streams[j].max_col) st_x[j] = streams[j].max_col;
				if (st_y[j] < streams[j].min_row) st_y[j] = streams[j].min_row;
				if (st_y[j] > streams[j].max_row) st_y[j] = streams[j].max_row;
				/* If we changed nothing, bail out */
				if (st_x[j] == p_ptr->stream_wid[j] && st_y[j] == p_ptr->stream_hgt[j]) continue;
			}

			/* Send it! */
			Send_stream_size(j, st_y[j], st_x[j]);

			/* Toggle update flag */
			st_flag |= streams[j].window_flag;
		}
	}

	/* Return update flags */
	return st_flag;
}
/* Helper caller for "net_term_manage" */
u32b net_term_update(bool clear) { return net_term_manage(window_flag, window_flag, clear); }
