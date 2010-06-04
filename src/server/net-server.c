/*
 * MAngband Server code
 *
 * Copyright (c) 2010 MAngband Project Team.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of the "Angband licence" with an extra clause:
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply. 
 * Clause: You are not allowed to use this code in software which is not
 * distributed in source form.
 *
 */
#include "mangband.h"
#include "net-server.h"

#define ONE_SECOND	1000000 /* 1 million "microseconds" */

int ticks = 0;

/* List heads */
eptr first_connection = NULL;
eptr first_listener = NULL;
eptr first_caller = NULL;
eptr first_timer = NULL;

/* Refrence lists */
int *Get_Conn;	/* Pass "Ind", get "ind" */
int *Get_Ind; 	/* Pass "ind", get "Ind" */
connection_type **Conn; /* Pass "ind", get "connection_type" */
connection_type **PConn; /* Pass "Ind", get "connection_type" */

/* Callbacks */
#define def_cb(A) int A(data data1, data data2)
def_cb(second_tick); 
def_cb(dungeon_tick);
def_cb(accept_client);
def_cb(client_login);
def_cb(client_read);
def_cb(client_close);
def_cb(hub_read);
def_cb(hub_close);

static int		(*handlers[256])(connection_type *ct, player_type *p_ptr);
static cptr		schemes[256];

bool client_names_ok(char *nick_name, char *real_name, char *host_name);

/* Scheme to use for parsing next packet */ 
cptr next_scheme = NULL;
byte next_pkt = 0;

server_setup_t serv_info = 
{
	0,
	0,
	0,
	0,
	
	0,
	0,
	0,
	0,
	
	0,
	0,
	0,
	0
};

/* Grab memory */
void alloc_server_memory()
{
	/* Make Player List */
	MAKE(players, element_group);
	eg_init(players, MAX_PLAYERS);

	/* Make Legacy List */	
	C_MAKE(p_list, MAX_PLAYERS + 1, player_type*);
	Players = p_list;

	/* Make refrence lists */
	C_MAKE(Conn, MAX_PLAYERS, connection_type*);
	C_MAKE(PConn, MAX_PLAYERS, connection_type*);
	C_MAKE(Get_Conn, MAX_PLAYERS, int);
	C_MAKE(Get_Ind, MAX_PLAYERS, int);
}

/* Release it */
void free_server_memory() {
	/* Player list */
	if (players) eg_free(players);
	KILL(players);

	/* Legacy player-list */
	Players = NULL;
	KILL(p_list);

	/* Refrence lists */
	KILL(Conn);
	KILL(PConn);
	KILL(Get_Conn);
	KILL(Get_Ind);
}

/* Player enters active gameplay */
int player_enter(int ind) 
{
	/* Grab pointers */
	connection_type *ct = players->list[ind]->data1;
	player_type *p_ptr = players->list[ind]->data2;

// SUPER-HACK! Remove this asap:
p_ptr->screen_wid = 80;
p_ptr->screen_hgt = 20;

	/* Inform client */
	send_play(ct, 1);

	/* Add him to the end of the list */
	p_max++;
	p_list[p_max] = p_ptr;

	/* Fix various refrence lists */
	Get_Ind[ind] = p_max;
	Get_Conn[p_max] = ind;
	PConn[p_max] = ct; 

	/* Mark him as ready */
	p_ptr->state = PLAYER_PLAYING;

	/* Setup his locaton */
	player_setup(p_max);

	return 0;
}

/* Player leaves active gameplay (Remove player from p_list) */
int player_leave(int p_idx)
{
	player_type *p_ptr = p_list[p_idx];
	int ind = Get_Conn[p_idx];
	int saved = 0;

	/* Be paranoid */
	if (cave[p_ptr->dun_depth])
	{
		/* There's nobody on this space anymore */
		cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0;
		/* Forget his lite and viewing area */
		//forget_lite(Ind);TODO--test if this is really needed?
		//forget_view(Ind);TODO--test if this is really needed?
		/* Show everyone his disappearance */
		everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);
	}

	/* Try to save his character */
	saved = save_player(p_idx);

	/* Inform everyone */
	msg_broadcast(p_idx, format("%s has left the game.", p_ptr->name));

	/* This player has no connection attached (orphaned) */
	if (ind == -1)
	{
		/* Free player memory */
		cq_free(&p_ptr->cbuf);
		player_free(p_ptr);
	}

	/* If this player is not last on the list, perform a switcheroo */
	if (p_idx != p_max)
	{	
		/* Grab last player and his connection index */
		p_ptr = p_list[p_max];
		ind = p_list[p_max]->conn;

		/* If he has a connection, switch it too */
		if (ind != -1)
		{
			/* Update "Ind-by-ind" and "connection-by-Ind" */			
			Get_Ind[ind] = p_idx;
			PConn[p_idx] = players->list[ind]->data1;
		}

		/* Put him in current player's place */
		p_list[p_idx] = p_ptr;

		/* Switch index on grid */
		if (cave[p_ptr->dun_depth]) /* Cave is allocated */
			cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0 - p_idx;

		/* Update "ind-by-Ind" */
		Get_Conn[p_idx] = ind;
	}

	/* Reduce list */
	p_list[p_max] = NULL;
	p_max--;

	/* Recalculate player-player visibility */
	update_players();

	//TODO: Tell meta..
	return saved;
}

/* Connection leaves it's player behind. */
void player_drop(int ind)
{
	connection_type *c_ptr = players->list[ind]->data1;
	player_type *p_ptr = players->list[ind]->data2;
	int p_idx = Get_Ind[ind];

	/* Actively playing */
	if (p_ptr->state == PLAYER_PLAYING)
	{
		/* No more */
		p_ptr->state = PLAYER_LEAVING;
		/* Break "Ind" lists */
		PConn[p_idx] = NULL;
		Get_Conn[p_idx] = -1;
		Get_Ind[ind] = -1;
	}

	/* No longer part of the list */
	p_ptr->conn = -1; 

	/** Remove connection from master-list (LOGOUT) **/
	ind = eg_del(players, ind);

	/* Update remaining portions of pointer lists */
	if (ind > -1) for (; ind < players->num; ind++) 
	{
		/* Pick a player and a connection by "ind" */
		c_ptr = players->list[ind]->data1;
		p_ptr = players->list[ind]->data2;
		/* He is in-game */
		if (p_ptr->state == PLAYER_PLAYING)
		{
			/* Update in-game lists */
			p_idx = Get_Ind[p_ptr->conn];
		 	Get_Ind[ind] = p_idx; /* Player-by-index */ 
			Get_Conn[p_idx] = ind; /* Index-by-player */
			PConn[p_idx] = c_ptr; /* Connection-by-player pointer */ 
		}
		/* Connection-by-index pointer */
		Conn[ind] = c_ptr;
		/* Set "ind" index to both of them */
		p_ptr->conn = ind;
		c_ptr->user = (data)ind;
	}
}
/* Fast termination of connection (used by shutdown routine) */


/* Init */
void setup_network_server()
{
	/** Add timers **/
	/* Dungeon Turn */
	first_timer = add_timer(NULL, (ONE_SECOND / cfg_fps + 250), (callback)dungeon_tick);
	/* Every Second */
	add_timer(first_timer, (ONE_SECOND), (callback)second_tick);

	/** Prepare FD_SETS **/
	network_reset();

	/** Add listeners **/
	/* Game */
	first_listener = add_listener(NULL, cfg_tcp_port, (callback)accept_client);
	/* Console */
	add_listener(first_listener, cfg_tcp_port + 1, accept_console);

	/** Allocate some memory */
	alloc_server_memory();	

	/** Setup packet handling functions **/
	setup_tables(handlers, schemes);
}

/* Infinite Loop */
void network_loop()
{
	shutdown_timer = 0;
	plog("Entering network loop...");
	while (1)
	{
		first_listener = handle_listeners(first_listener);
		first_connection = handle_connections(first_connection);
		first_caller = handle_callers(first_caller);
		first_timer = handle_timers(first_timer, static_timer(0));

		network_pause(200000); /* 0.2 ms "sleep" */
	}
}

/* Close */
void close_network_server()
{
	/* Remove timers, listeners and connections */
	e_release_all(first_timer, 0, 1);
	e_release_all(first_listener, 0, 1);

	/* TODO: Inform meta ...? */	

	/* Close all connections and callers (?) */
	e_release_all(first_connection, 0, 1);
	e_release_all(first_caller, 0, 1);

	/* Release memory */
	free_server_memory();
}


int second_tick(data data1, data data2) {
	int i;

	/* plog("A Second Passed"); */ ticks = 0;

	/* Update shutdown timer */
	if (shutdown_timer) 
	{
		shutdown_timer--;
		/* It's time */
		if (!shutdown_timer) 
		{ 
			shutdown_server();
			return 0;
		}
	}

	/* Add 1 second to everyone's idle timer */
	for (i = 1; i < p_max + 1; i++) 
	{
		/* Increase and test for timeout */
		if (p_list[i]->idle++ > 15) 
		{
			/* This player has a connection attached */
			if (p_list[i]->conn != -1)
			{
				connection_type *ct = PConn[i];
				client_kill(ct, "Ping timeout");
			}
			/* Remove him from game*/
			player_leave(i);
		}
	}

	return 1;
}

int dungeon_tick(data data1, data data2) {
	/* plog("The Clock Ticked"); */ ticks++;

	/* Game Turn */
	dungeon();
	return 2;
}
					/* data1 is (int)fd */
int accept_client(data data1, data data2) {
	eptr new_connection;
	int fd = (int)data1;

	/* Add connection */
	new_connection = add_connection(first_connection, fd, hub_read, hub_close);
	if (!first_connection) first_connection = new_connection;

	/* Disable Nagle's algorithm */
	denaglefd(fd);   

	return 0;
}

int hub_read(data data1, data data2) { /* return -1 on error */
	/* char *recv = data1; // Unused */
	connection_type *ct = (connection_type*)data2;

	/* int len = strlen(recv); // Unused */
	int okay = 0;

	u16b
		conntype = 0;
	if (cq_scanf(&ct->rbuf, "%ud", &conntype) < 1)
	{
		conntype = CONNTYPE_ERROR;
	}
	else
	{
		conntype = connection_type_ok(conntype);
	}

	switch (conntype) 
	{
		case CONNTYPE_PLAYER: 

			ct->receive_cb = client_login;
			ct->close_cb = client_close;

			ct->user = (data)-1;

			send_play(ct, 0);

		break;
		case CONNTYPE_CONSOLE:
		
			accept_console((data)ct, NULL);

		break;
		case CONNTYPE_OLDPLAYER:
			okay = -1;
			cq_printf(&ct->wbuf, "%c", 0x01);
			plog(format("Legacy connection requested from %s", ct->host_addr));	
		break;
		case CONNTYPE_ERROR:
			okay = -1;
			plog(format("Invalid connection type requested from %s", ct->host_addr));
		break;
	}

	cq_clear(&ct->rbuf);
	return okay;
}
int hub_close(data data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	debug(format("{HUB} Lost connection to %s", ct->host_addr));
	return 0;
}


int client_read(data data1, data data2) { /* return -1 on error */
	connection_type *ct = data2;
	player_type *p_ptr = players->list[(int)ct->user]->data2; 

	byte pkt;
	int result;
	int start_pos;

	/* Reset timeout timer */
	p_ptr->idle = 0;

	/* Parse "read buffer" */
	while (	cq_len(&ct->rbuf) )
	{
		start_pos = ct->rbuf.pos;
		pkt = CQ_GET(&ct->rbuf);
		next_pkt = pkt;
		next_scheme = schemes[pkt];
		result = (*handlers[pkt])(ct, p_ptr);

		/* Do not continue */
		if (result != 1) break;
	}

	/* Not enough bytes */
	if (result == 0) ct->rbuf.pos = start_pos;

	/* Returning "-1" kills the connection */
	return result;
}

/* Hack -- imagine "recv_login" and "client_read" rolled into one. */
int client_login(data data1, data data2) { /* return -1 on error */
	connection_type *ct = data2;
	/* char *recv = data1; // Unused */
	player_type *p_ptr = NULL;

	byte pkt;
	int start_pos;

	u16b
		version = 0;
	char
		real_name[MAX_CHARS],
		host_name[MAX_CHARS],		
		nick_name[MAX_CHARS],
		pass_word[MAX_CHARS];

	if (!cq_len(&ct->rbuf)) return 0;

	start_pos = ct->rbuf.pos;
	pkt = CQ_GET(&ct->rbuf);

	/* Only accept login packet at this stage */
	if (pkt != PKT_LOGIN) return -1;

	/* Note: all variables (except char/byte) passed to cq_scanf MUST be initialized */
	real_name[0] = host_name[0] = nick_name[0] = pass_word[0] = '\0';

	if (cq_scanf(&ct->rbuf, "%ud%s%s%s%s", &version, real_name, host_name, nick_name, pass_word) < 5)
	{
		/* Not enough bytes */
		ct->rbuf.pos = start_pos;	
		return 0;
	}

	/* Test for Resume, Drop, Block........ */
	// ............. nothing here yet, TODO:!

	/** LOGIN **/
	if (!client_version_ok(version))
	{ 
		client_abort(ct, "Incompatible client version.");
	}
	if (!client_names_ok(nick_name, real_name, host_name))
	{ 
		client_abort(ct, "The server didn't like your nickname, realname, or hostname.");
	}

	/* Allocate memory */
	p_ptr = player_alloc();

	/* Copy his name and connection info */
	strcpy(p_ptr->name, nick_name);
	strcpy(p_ptr->pass, pass_word);
	p_ptr->version = version;

	/* Verify his name and create a savefile name */
	if (!process_player_name(p_ptr, TRUE))
	{ 
		player_free(p_ptr); /* Unalloc back */
		client_abort(ct, "Unacceptable nickname");
	}

	p_ptr->state = PLAYER_NAMED;

	/* Attempt to load from a savefile */
	if (!load_player(p_ptr))
	{
		player_free(p_ptr); /* Unalloc back */
		client_abort(ct, "Error loading savefile");
	}

	/* If loaded a character */
	if (character_loaded)
	{
		p_ptr->state = PLAYER_BONE;
		if (p_ptr->death == FALSE)
			p_ptr->state = PLAYER_FULL;
	}
	else
	{
		p_ptr->state = PLAYER_SHAPED;
	}

	/* ADD TO LIST */
	p_ptr->conn = eg_add(players, ct, p_ptr);
	ct->user = (data)p_ptr->conn;
	Conn[p_ptr->conn] = ct;

	/* Init "command buffer" */
	cq_init(&p_ptr->cbuf, PD_SMALL_BUFFER);

	/* Advance to next stage */
	ct->receive_cb = client_read;

	/* Since LOGIN is the first command ever, it's a good time to send basics */
	send_race_info(ct);
	send_class_info(ct);
	send_server_info(ct);

	/* Finally send char info: */
	send_char_info(ct, p_ptr);

	/* Return "1" to sustain connection */
	return 1;
}

/* This function gets called when the connection is over */
int client_close(data data1, data data2) {
	connection_type *c_ptr = data2;
	int ind = (int)c_ptr->user;

	/* He has a player attached (LOGGED IN) */
	if (ind != -1)
	{
		player_type *p_ptr = players->list[ind]->data2;
		bool playing = (p_ptr->state == PLAYER_PLAYING ? TRUE : FALSE);		

		/* Split player from connection */
		player_drop(ind);
 
		/* Player is NOT in the game */
		if (!playing)
		{
			/* Free player memory */
			cq_free(&p_ptr->cbuf);
			player_free(p_ptr);
		}
	}

	return 0;
}

/* You call this function to kill a connection. 
 * The function is really simple, it only schedules events and does nothing.
 * 
 * Note:  The consequences of disconnect are handled in "client_close", see above.
 *
 * Note:  You should NOT call this function from inside a packet-handler. Instead, 
 *  [call "send_quit" manually and] just return "-1". The "client_abort" macro
 *  does exactly that. 
 * To recap: killing some other connection from code - "client_kill()" 
 *           killing own connection due to errors  - "client_abort()"
 */
int client_kill(connection_type *ct, cptr reason)
{
	/* Inform client */
	if (reason != NULL)
	{
		send_quit(ct, (char*)reason);
	}

	/* Schedule disconnect */
	ct->close = 1;

	return 0;
}
/* client_kill a connection refrenced by p_idx */
int player_kill(int p_idx, cptr reason)
{
	connection_type *ct = PConn[p_idx];
	if (ct != NULL)	return client_kill(ct, reason);
	return 0;
}

/*
 * Check if we like the names.
 */
bool client_names_ok(char *nick_name, char *real_name, char *host_name)
{
	char *ptr;
printf("%s - nick, %s - real, %s - host\n", nick_name, real_name, host_name);
	if (real_name[0] == 0 || host_name[0] == 0) return FALSE;

	/* Any wierd characters here, bail out.  We allow letters, numbers and space */
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if ( (*ptr == 32) || ((*ptr >= 97) && (*ptr <= 122)) || ((*ptr >= 65) && (*ptr <= 90))
		|| ((*ptr >= 48) && (*ptr <= 57)) )
		{
			/* ok */
		} else {
			return FALSE;
		}
	}
	
	/* Right-trim nick */
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if (isascii(*ptr) && isspace(*ptr))
			*ptr = '\0';
		else break;
	}

	return TRUE;
}

/*
 * Check if we can work with this version.
 * Add more interesting checks if needed
 */
bool client_version_ok(u16b version)
{
	if (version == SERVER_VERSION) 
		return TRUE;
	else
		return FALSE;
}

/*
 * Check if the given connection type is valid.
 */
u16b connection_type_ok(u16b conntype)
{
	if (conntype == CONNTYPE_PLAYER)
		return CONNTYPE_PLAYER;
	if (conntype == 8202 || conntype == 8205)
		return CONNTYPE_CONSOLE;
	if (conntype == CONNTYPE_OLDPLAYER)
		return CONNTYPE_OLDPLAYER;
	
	return CONNTYPE_ERROR;
}
