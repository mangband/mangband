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
eptr first_sender = NULL;
eptr first_timer = NULL;

/* Refrence lists */
int *Get_Conn;	/* Pass "Ind", get "ind" */
int *Get_Ind; 	/* Pass "ind", get "Ind" */
connection_type **Conn; /* Pass "ind", get "connection_type" */
connection_type **PConn; /* Pass "Ind", get "connection_type" */

/* Callbacks */
#define def_cb(A) int A(int data1, data data2)
def_cb(report_to_meta);
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
	MAX_INDICATORS, /* Must be replaced with real indicator count at run-time */
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
	C_MAKE(PConn, MAX_PLAYERS + 1, connection_type*);
	C_MAKE(Get_Conn, MAX_PLAYERS + 1, int);
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
	int PInd;
	/* Grab pointers */
	connection_type *ct = players->list[ind]->data1;
	player_type *p_ptr = players->list[ind]->data2;

	/* Add him to the end of the list */
	if (p_ptr->state != PLAYER_LEAVING)
	{
		p_max++;
		PInd = p_max;
	}
	/* Already on the list, get him */
	else
	{
		PInd = Get_Ind[ind];
	}

	p_list[PInd] = p_ptr;

	/* Fix various refrence lists */
	Get_Ind[ind] = PInd;
	Get_Conn[PInd] = ind;
	PConn[PInd] = ct; 

	/* Hack -- join '#public' channel */
	send_channel(PInd, CHAN_JOIN, 0, DEFAULT_CHANNEL);

	/* Hack -- send different 'G'ain command */
	if (c_info[p_ptr->pclass].spell_book == TV_PRAYER_BOOK)
	{
		send_custom_command_info(ct, study_cmd_id);
	}

	/* Mark him as playing */
	p_ptr->state = PLAYER_PLAYING;

	/* Setup his locaton */
	player_setup(PInd);
	setup_panel(PInd, TRUE);
	verify_panel(PInd);

	/* Hack, must find better place */
	prt_history(PInd);
	show_socials(PInd);

	/* Current party */
	send_party_info(PInd);

	/* Inform everyone */
	if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE)) /* unless it's hidden DM */
	{
		if (p_ptr->new_game)
		{
			msg_broadcast(PInd, format("%s begins a new game.", p_ptr->name));
		}
		else
		{
			msg_broadcast(PInd, format("%s has entered the game.", p_ptr->name));
		}
	}

	/* Discard "New game" marker */
	p_ptr->new_game = FALSE;

	return 0;
}

/* AUX function to reset various player states */
/* Note: this only deals with immediate network-related states, so we can
 * stop sending various stuff.
 * For "complete" player wipe, see "player_wipe()" in birth.c */
void player_abandon(player_type *p_ptr)
{
	int p_idx = Get_Ind[p_ptr->conn];
	int i;

	/* Leave all chat channels */
	/* No, let's keep them for now.
	channels_leave(p_idx);
	*/

	/* Unsubscribe from all streams */
	for (i = 0; i < MAX_STREAMS; i++)
	{
		p_ptr->stream_wid[i] = 0;
		p_ptr->stream_hgt[i] = 0;
	}

	/* Unschedule all indicators */
	p_ptr->redraw = 0;

	/* Stop all file perusal and interactivity */
	string_free(p_ptr->interactive_file);
	p_ptr->interactive_file = NULL;
	p_ptr->last_info_line = -1;
	p_ptr->special_handler = 0;
	p_ptr->special_file_type = 0;
	p_ptr->interactive_line = -1;

	/* Disable all kinds of tracking */
	p_ptr->health_who = 0;
	p_ptr->cursor_who = 0;
	p_ptr->target_set = FALSE;
	p_ptr->target_flag = 0;
	p_ptr->target_n = 0;
	p_ptr->monster_race_idx = 0;

	/* Disable message repeat */
	p_ptr->msg_last_type = MSG_MAX_ANGBAND;

	/* Forget about setup data */
	for (i = 0; i < 6; i++)
	{
		p_ptr->infodata_sent[i] = 0;
	}
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

	/* Leave all chat channels */
	channels_leave(p_idx);

	/* Leave everything else */
	player_abandon(p_ptr);

	/* Inform everyone */
	if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE)) /* unless hidden DM */
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

	/* Leave everything (before we destroyed all pointers) */
	player_abandon(p_ptr);

	/* If player was in town, hurry his disconnection up */
	if (!p_ptr->dun_depth || check_special_level(p_ptr->dun_depth))
	{
		/* LARGE value to trigger timeout ASAP */
		p_ptr->idle = 60;
		/* Alternatively, call player_leave(Ind)... */
	}

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
		c_ptr->user = ind;
	}
}
/* Fast termination of connection (used by shutdown routine) */


/* Init */
void setup_network_server()
{
	/** Add timers **/
	/* Dungeon Turn */
	first_timer = add_timer(NULL, (ONE_SECOND / cfg_fps), (callback)dungeon_tick);
	/* Every Second */
	add_timer(first_timer, (ONE_SECOND), (callback)second_tick);

	/** Add UDP */
	/* Meta-server */
	first_sender = add_sender(NULL, cfg_meta_address, 8800, ONE_SECOND * 4, report_to_meta);

	/** Prepare FD_SETS **/
	network_reset();

	/** Add listeners **/
	/* Game */
	first_listener = add_listener(NULL, cfg_tcp_port, (callback)accept_client);
	if (!first_listener)
	{
		quit("Unable to create server interface");
	}
	/* Console */
	add_listener(first_listener, cfg_tcp_port + 1, accept_console);
	if (!first_listener)
	{
		quit("Unable to create console interface");
	}

	/** Allocate some memory */
	alloc_server_memory();	

	/** Setup packet handling functions **/
	setup_tables(handlers, schemes);
}

/* Player commands */
/* Usually, "process_player_commands" is triggered from within
   the "dungeon()" tick. However, classic MAngband would not wait
   for the next turn and execute the command immediately as it
   arrived. So this little function flushes them all right after
   we handled network.
*/
void post_process_players(void)
{
	int Ind;
	for (Ind = 1; Ind < NumPlayers + 1; Ind++)
	{
		player_type *p_ptr = Players[Ind];
		
		/* HACK -- Do not proccess while changing levels */
		if (p_ptr->new_level_flag == TRUE) continue;
		
		/* Try to execute any commands on the command queue. */
		(void) process_player_commands(Ind);
	}
	/* Next loop flushes all potential update flags Players have set
	 * for each other. */
	for (Ind = 1; Ind < NumPlayers + 1; Ind++)
	{
		player_type *p_ptr = Players[Ind];
		
		/* Recalculate and schedule updates */
		handle_stuff(Ind);
	}
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
		first_sender = handle_senders(first_sender, static_timer(1));
		first_timer = handle_timers(first_timer, static_timer(0));

		post_process_players(); /* Execute all commands */

		network_pause(2000); /* 0.002 ms "sleep" */
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
	e_release_all(first_sender, 0, 1);

	/* Release memory */
	free_server_memory();
}

/* Send one last packet to meta.
 * The round-about way of doing this has to do with the way "UDP Senders" are
 * handled. We can't call any netcode DIRECTLY, but we can trick it to flush. */
void report_to_meta_die(void)
{
	u16b old_shutdown_timer;
	/* "report_to_meta" checks for "shutdown_timer", so let's force it */
	old_shutdown_timer = shutdown_timer;
	shutdown_timer = 1; /* YES, we *are* shutting down! */
	/* Flush -- send actual UDP packet */
	/* ONE_SECOND * 60 must be >= sender's delay interval */
	first_sender = handle_senders(first_sender, ONE_SECOND * 60);
	/* Paranoia -- restore shutdown_timer */
	shutdown_timer = old_shutdown_timer;
}

int report_to_meta(int data1, data data2) {
	static char local_name[1024];
	static int init = 0;
	char buf[1024], temp[100];
	cq *out = (cq*)data2;
	int k, num = 0;

	/* Abort if the user doesn't want to report */
	if (!cfg_report_to_meta) return 1;
	
	/* If this is the first time called, initialize our hostname */
	if (!init)
	{
		plog("Reporting to meta-server...");

		/* Never do this again */
		init = 1;

		/* Get our hostname */
		if (cfg_report_address)
		{
			my_strcpy(local_name, cfg_report_address, 1024);
		}
		else
		{
			if (cfg_bind_name)
			{
				my_strcpy(local_name, cfg_bind_name, 1024);
			}
			else
			{
				fillhostname(local_name, 1024);
			}
		}
		/* Add :port number */
		my_strcat(local_name, format(":%d", (int)cfg_tcp_port), 1024);
	}

	/* Start with our address */
	strcpy(buf, local_name);

	/* Hack -- if we're shutting down, don't send player list and version */
	if (shutdown_timer) 
	{
		/* Send address + whitepace, which metaserver recognizes as death report */
		strcat(buf, " ");
		cq_write(out, buf);
		return 1;
	}

	/* Ugly Hack -- Count players */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];
		if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE))
			num++;
	}

	/* 'Number of players' */
	strcat(buf, " Number of players: ");
	sprintf(temp, "%d ", num);
	strcat(buf, temp);

	/* Scan the player list */
	if (num) 
	{
		/* List player names */
		strcat(buf, "Names: ");

		for (k = 1; k <= NumPlayers; k++)
		{
			/* Hide dungeon master */
			if (Players[k]->dm_flags & DM_SECRET_PRESENCE) continue;
			/* Add an entry */
			strcat(buf, Players[k]->basename);
			strcat(buf, " ");
		}
	}

	/* Append the version number */
#ifndef SVNREV
    if (cfg_ironman)
    	sprintf(temp, "Version: %d.%d.%d Ironman ", SERVER_VERSION_MAJOR, 
    	SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);
    else
    	sprintf(temp, "Version: %d.%d.%d ", SERVER_VERSION_MAJOR, 
    	SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);
	/* Append the additional version info */
	if (SERVER_VERSION_EXTRA == 1)
		strcat(temp, "alpha");
	if (SERVER_VERSION_EXTRA == 2)
		strcat(temp, "beta");
	if (SERVER_VERSION_EXTRA == 3)
		strcat(temp, "development");
#else
    if (cfg_ironman)
    	sprintf(temp, "Revision: %d Ironman ", atoi(SVNREV));
    else
    	sprintf(temp, "Revision: %d ", atoi(SVNREV));
#endif
	strcat(buf, temp);

	/* Send it */
	cq_write(out, buf);

	return 1;
}

int second_tick(int data1, data data2) {
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

int dungeon_tick(int data1, data data2) {
	/* plog("The Clock Ticked"); */ ticks++;

	/* Game Turn */
	dungeon();
	return 2;
}
					/* data1 is (int)fd */
int accept_client(int data1, data data2) {
	eptr new_connection;
	int fd = (int)data1;

	/* Add connection */
	new_connection = add_connection(first_connection, fd, hub_read, hub_close);
	if (!first_connection) first_connection = new_connection;

	/* Disable Nagle's algorithm */
	denaglefd(fd);   

	return 0;
}

int hub_read(int data1, data data2) { /* return -1 on error */
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

			ct->user = -1;

			send_play(ct, PLAYER_EMPTY);

		break;
		case CONNTYPE_CONSOLE:
		
			/* evil hack -- chop trailing \n before reading password */
			if (cq_len(&ct->rbuf) && cq_peek(&ct->rbuf)[0] == '\n')
				ct->rbuf.pos++;
		
			okay = accept_console(-1, (data)ct);

		break;
		case CONNTYPE_OLDPLAYER:
			okay = -1;
			cq_printf(&ct->wbuf, "%c%c%d", 0, 0x01, 0);
			debug(format("Legacy connection requested from %s", ct->host_addr));
		break;
		case CONNTYPE_ERROR:
			okay = -1;
			debug(format("Invalid connection type requested from %s", ct->host_addr));
		break;
	}

	/* cq_clear(&ct->rbuf); */
	return okay;
}
int hub_close(int data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	debug(format("{HUB} Lost connection to %s", ct->host_addr));
	return 0;
}


int client_read(int data1, data data2) { /* return -1 on error */
	connection_type *ct = (connection_type *)data2;
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

	/* Slide "read buffer" */
	else if (result == 1) cq_slide(&ct->rbuf);

	/* Returning "-1" kills the connection */
	return result;
}

/* Hack -- imagine "recv_login" and "client_read" rolled into one. */
int client_login(int data1, data data2) { /* return -1 on error */
	connection_type *ct = data2;
	/* char *recv = data1; // Unused */
	player_type *p_ptr = NULL;
	int Ind;

	byte pkt;
	int start_pos, i;

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

	/** BLOCK **/
	// ............. nothing here yet, TODO:!

	/** LOGIN **/
	if (!client_version_ok(version))
	{ 
#ifdef DEBUG
		debug(format("Rejecting %s for version %04x", ct->host_addr, version));
#endif
		client_abort(ct, "Incompatible client version.");
	}
	if (!client_names_ok(nick_name, real_name, host_name))
	{ 
#ifdef DEBUG
		debug(format("Rejecting %s for nick - %s, real - %s, host - %s", ct->host_addr, nick_name, real_name, host_name));
#endif
		client_abort(ct, "The server didn't like your nickname, realname, or hostname.");
	}
	if (scoop_player(nick_name, pass_word) < 0)
	{
#ifdef DEBUG
		debug(format("Rejecting %s for wrong password nick - %s", ct->host_addr, nick_name));
#endif
		client_abort(ct, "Incorrect password.");
	}

	/* DROP */
	/* See if a player with same nickname is already connected */
	for (i = 0; i < players->num; i++)
	{
		connection_type *q_ct = players->list[i]->data1;
		player_type *q_ptr = players->list[i]->data2;
		if (q_ct != ct && !ct->close && !strcmp(q_ptr->name, nick_name))
		{
			/* Keep player pointer */
			p_ptr = q_ptr;

			/* Drop other connection... */
			player_drop(i);
			client_kill(q_ct, "Reconnect from other location.");
			/* ...but manually detach it from player, as we're going to reuse it */
			q_ct->user = -1;
			q_ptr->conn = -1;
		}
	}

	if (!eg_can_add(players))
	{
		debug(format("Rejecting %s because players array is full, nick - %s", ct->host_addr, nick_name));
		client_abort(ct, "The server is full.");
	}

	/* RESUME/DROP */
	/* See if a player with same nickname is already playing */
	if ((Ind = find_player_name(nick_name)))
	{
		p_ptr = Players[Ind];

		/* Reset "command buffer" */
		cq_clear(&p_ptr->cbuf);
	}
	/* Reuse kept player from the DROP operation */
	else if (p_ptr)
	{
		Ind = (p_ptr->conn != -1) ? Get_Ind[p_ptr->conn] : 0;

		/* Reset "command buffer" */
		cq_clear(&p_ptr->cbuf);
	}
	/* NEW */
	else
	{
		/* Allocate memory */
		p_ptr = player_alloc();
		player_wipe(p_ptr);

		/* Copy his name and connection info */
		my_strcpy(p_ptr->name, nick_name, MAX_CHARS);
		my_strcpy(p_ptr->pass, pass_word, MAX_CHARS);
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

		/* Dead */
		if (character_died)
		{
			p_ptr->state = PLAYER_BONE;
		}
		/* Alive and well */
		else if (character_loaded)
		{
			p_ptr->state = PLAYER_FULL;
		}
		else
		{
			//p_ptr->state = PLAYER_SHAPED;
		}

		/* Init "command buffer" */
		cq_init(&p_ptr->cbuf, PD_SMALL_BUFFER);
	}

	/* ADD TO LIST */
	p_ptr->conn = eg_add(players, ct, p_ptr);
	ct->user = p_ptr->conn;
	Conn[p_ptr->conn] = ct;

	if (p_ptr->state == PLAYER_LEAVING)
	{
		/* Fix other lists */
		Get_Ind[p_ptr->conn] = Ind;
		Get_Conn[Ind] = p_ptr->conn;
		PConn[Ind] = ct;
	}

	/* Copy host/real names */
	my_strcpy(p_ptr->hostname, host_name, MAX_CHARS);
	my_strcpy(p_ptr->realname, real_name, MAX_CHARS);

	/* Advance to next stage */
	ct->receive_cb = client_read;

	/* Since LOGIN is the first command ever, it's a good time to send basics */
	send_race_info(ct);
	send_class_info(ct);
	send_server_info(ct);
	send_inventory_info(ct);
	send_objflags_info(ct);
	send_floor_info(ct);
	send_optgroups_info(ct);

	/* Finally send char info: */
	send_char_info(ct, p_ptr);

	/* Report */
	plog(format("Welcome %s=%s@%s (%s) version (%04x)", nick_name, real_name, host_name, ct->host_addr, version ));

	/* Return "1" to sustain connection */
	return 1;
}

/* This function gets called when the connection is over */
int client_close(int data1, data data2) {
	connection_type *c_ptr = data2;
	int ind = (int)c_ptr->user;

	/* He has a player attached (LOGGED IN) */
	if (ind != -1)
	{
		player_type *p_ptr = players->list[ind]->data2;
		bool playing = (p_ptr->state == PLAYER_PLAYING ? TRUE : FALSE);		

		if (p_ptr->state == PLAYER_LEAVING) playing = TRUE;

		/* Report */
		plog(format("Goodbye %s=%s@%s (%s)", p_ptr->name, p_ptr->realname, p_ptr->hostname, c_ptr->host_addr));

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
/* client_kill a connection that belongs to player p_ptr */
int player_disconnect(player_type *p_ptr, cptr reason)
{
	/* Ensure player has a connection */
	if (p_ptr->conn > -1)
	{
		connection_type *ct = Conn[p_ptr->conn];
		/* Be very paranoid */
		if (ct != NULL) return client_kill(ct, reason);
	}
	return 0;
}

/*
 * Check if we like the names.
 */
bool client_names_ok(char *nick_name, char *real_name, char *host_name)
{
	char *ptr;

	/** Realname / Hostname **/
	if (real_name[0] == 0 || host_name[0] == 0) return FALSE;

	/* Replace weird characters with '?' */
	for (ptr = &real_name[strlen(real_name)]; ptr-- > real_name; )
	{
		if (!isascii(*ptr) || !isprint(*ptr)) *ptr = '?';
	}
	for (ptr = &host_name[strlen(host_name)]; ptr-- > host_name; )
	{
		if (!isascii(*ptr) || !isprint(*ptr)) *ptr = '?';
	}

	/** Playername **/
	if (nick_name[0] == '\0') return FALSE;

	/* Any wierd characters here, bail out.  We allow letters, numbers and space */
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if (!isascii(*ptr)) return FALSE;
		if (!(isalpha(*ptr) || isdigit(*ptr) || *ptr == ' '))
		{
			return FALSE;
		}
	}

	/* Can't start with space */
	if (nick_name[0] == ' ') return FALSE;

	/* Right-trim nick */
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if (isascii(*ptr) && isspace(*ptr))
			*ptr = '\0';
		else break;
	}

	/* Hack -- Reserved name */
	if (!my_stricmp(nick_name, "server")) return FALSE;

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
