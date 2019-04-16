/* Client initialization module */

/*
 * This file should contain non-system-specific code.  If a 
 * specific system needs its own "main" function (such as
 * Windows), then it should be placed in the "main-???.c" file.
 */

#include "c-angband.h"

char host_name[80];

static void init_arrays(void)
{
	/* Macro variables */
	C_MAKE(macro__pat, MACRO_MAX, cptr);
	C_MAKE(macro__act, MACRO_MAX, cptr);
	C_MAKE(macro__cmd, MACRO_MAX, bool);

	/* Macro action buffer */
	C_MAKE(macro__buf, 1024, char);

	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u16b);
	C_MAKE(message__buf, MESSAGE_BUF, char);
	C_MAKE(message__type, MESSAGE_MAX, u16b);
	C_MAKE(message__count, MESSAGE_MAX, u16b);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;

	/* Initialize room for the store's stock */
	C_MAKE(store.stock, STORE_INVEN_MAX, object_type);

	/* Clear client_setup */
	Client_setup.k_attr = NULL;
}

/*
 * Initialize and verify the file paths.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * PKGDATADIR, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the PKGDATADIR constant.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 *
 */
void init_stuff(void)
{
	char path[1024];

	cptr tail;

	/* Get the environment variable */
	tail = getenv("ANGBAND_PATH");

	/* Use the angband_path, or a default */
	my_strcpy(path, tail ? tail : PKGDATADIR, 1024);

	/* Read/Write path from config file */
	my_strcpy(path, conf_get_string("MAngband", "LibDir", path), 1024);
	conf_set_string("MAngband", "LibDir", path);

	/* Read path from command-line */
	clia_read_string(path, 1024, "libdir");

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) my_strcat(path, PATH_SEP, 1024);

	/* Initialize */
	init_file_paths(path);

	/* -------------------------- */
	/* Overwrite ANGBAND_DIR_USER */

	/* Read/Write path from config file or CLI */
	my_strcpy(path, conf_get_string("MAngband", "UserDir", ""), 1024);
	clia_read_string(path, 1024, "userdir");

	/* We're onto something */
	if (!STRZERO(path))
	{
		/* Hack -- Add a path separator (only if needed) */
		if (!suffix(path, PATH_SEP)) my_strcat(path, PATH_SEP, 1024);

		/* Overwrite "user" dir */
		my_strcat(path, "user", 1024);
		string_free(ANGBAND_DIR_USER);
		ANGBAND_DIR_USER = string_make(path);
	}

}

/* Init minor arrays */
void init_minor(void)
{
	int i;
	/* Chat channels */
	//C_MAKE(p_ptr->on_channel, MAX_CHANNELS, byte);
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		channels[i].name[0] = '\0';
		channels[i].id = 0;
		channels[i].num = 0;
		p_ptr->on_channel[i] = FALSE;
	}
	p_ptr->main_channel = 0;

	/* Term channels */
	p_ptr->remote_term = NTERM_WIN_OVERHEAD;

	/* Server-defined network behavior */
	known_indicators = 0;
	known_item_testers = 0;
	custom_commands = 0;
	known_options = 0;
	known_streams = 0;
	stream_groups = 0;
	for (i = 0; i < MAX_STREAMS; i++)
	{
		stream_group[i] = 0;
		p_ptr->stream_cave[i] = NULL;
		p_ptr->stream_wid[i] = 0;
		p_ptr->stream_hgt[i] = 0;
	}
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		window_to_stream[i] = NTERM_WIN_NONE;
		remote_info[i] = NULL;
	}

	/* Redraw flags */
	p_ptr->redraw = 0;
}

/* Init info arrays */
void init_info(void)
{
	/* k_info */
	C_MAKE(Client_setup.k_attr, z_info.k_max, byte);
	C_MAKE(Client_setup.k_char, z_info.k_max, char);
	C_MAKE(p_ptr->k_attr, z_info.k_max, byte);
	C_MAKE(p_ptr->k_char, z_info.k_max, char);
	/* d_info hack */
	C_MAKE(p_ptr->d_attr, z_info.k_max, byte);
	C_MAKE(p_ptr->d_char, z_info.k_max, char);

	/* r_info */
	C_MAKE(Client_setup.r_attr, z_info.r_max, byte);
	C_MAKE(Client_setup.r_char, z_info.r_max, char);
	C_MAKE(p_ptr->r_attr, z_info.r_max, byte);
	C_MAKE(p_ptr->r_char, z_info.r_max, char);

	/* f_info */
	C_MAKE(Client_setup.f_attr, z_info.f_max, byte);
	C_MAKE(Client_setup.f_char, z_info.f_max, char);
	C_MAKE(p_ptr->f_attr, z_info.f_max, byte);
	C_MAKE(p_ptr->f_char, z_info.f_max, char);

	/* pr_info */
	C_MAKE(p_ptr->pr_attr, (z_info.c_max+1)*z_info.p_max, byte);
	C_MAKE(p_ptr->pr_char, (z_info.c_max+1)*z_info.p_max, char);
}

/*
 * Open all relevant pref files.
 */
void initialize_all_pref_files(void)
{
	char buf[1024];

	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "user.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "character" pref file */
	sprintf(buf, "%s.prf", nick);
	buf[0] = tolower(buf[0]);

	/* Process that file */
	process_pref_file(buf);
}

int get_bone_decision() {
#if 0
	char k;
	clear_from(20);
	put_str("R - reroll, Any other key - restart", 21, 1);
	Term_fresh();
	k = inkey();
	if (k == 'r' || k == 'R') return 1;
#endif
	return 0;
}

/*
 * Sync a piece of server data via "send_request()" call.
 */
static void sync_data_piece(int rq, int* ask, int rcv, int max, bool* ready)
{
	if (rcv < max)
	{
		if ((*ask) < rcv)
		{
			send_request(rq, rcv);
			(*ask) = rcv;
		}
		(*ready) = FALSE;
	}
}
/*
 * Sync required data structures.
 * Returns TRUE when data is ready for active gameplay,
 * FALSE when not.
 */
bool sync_data(void)
{
	bool data_ready = TRUE;

	static int asked_indicators = -1;
	static int asked_streams = -1;
	static int asked_commands = -1;
	static int asked_testers = -1;
	static int asked_options = -1;

	/* Indicators */
	sync_data_piece(RQ_INDI, &asked_indicators, known_indicators, serv_info.val1, &data_ready);

	/* Streams */
	sync_data_piece(RQ_STRM, &asked_streams, known_streams, serv_info.val2, &data_ready);

	/* Commands */
	sync_data_piece(RQ_CMDS, &asked_commands, custom_commands, serv_info.val3, &data_ready);

	/* Item Testerers */
	sync_data_piece(RQ_ITEM, &asked_testers, known_item_testers, serv_info.val4, &data_ready);

	/* Options */
	sync_data_piece(RQ_OPTS, &asked_options, known_options, options_max, &data_ready);

	return data_ready;
}

/* HACK! Declare Game_loop so we can call it */
static void Game_loop(void);

/*
 * Send handshake to the server and do the loop
 */
static void Setup_loop()
{
	int old_state = -1;
	u16b conntype = CONNTYPE_PLAYER;

	bool asked_game = FALSE;

	bool data_sent = FALSE;
	bool data_ready = FALSE;
	bool char_ready = FALSE;

	send_handshake(conntype);

	ignore_birth_options = FALSE;

	do
	{
		/* Do networking */
		network_loop();

		/** Give main-xxx a chance to handle OS events, since this is a tight loop **/
		Term_xtra(TERM_XTRA_EVENT, /* Blocking=*/FALSE);

		/* Check and Prepare data */
		data_ready = sync_data();

		/* Check and Prepare character */
		if (old_state != state)
		{
#ifdef DEBUG
			printf("Changing SetupState=%d (was=%d)\n", state, old_state);
#endif
			/* Handshake complete */
			if (state == PLAYER_EMPTY)
			{
				/* Send login */
				client_login();
			}
			/* No character is ready */
			if (state == PLAYER_NAMED)
			{
				/* Generate one */
				get_char_info();
				send_char_info();
				send_play(PLAY_ROLL);
			}
			/* Character is dead! */
			if (state == PLAYER_BONE)
			{
				if (get_bone_decision())
				{
					/* Ask for similar one */
					send_play(PLAY_REROLL);
				}
				else
				{
					/* Generate new one */
					send_play(PLAY_RESTART);
				}
			}
			if (old_state < PLAYER_SHAPED && state >= PLAYER_SHAPED)
			{
				//client_setup();
			}
			/* Character is ready for rolling */
			if (state == PLAYER_SHAPED)
			{
				/* Let's roll */
				send_play(PLAY_ROLL);
			}
			/* Character is ready to play */
			if (state == PLAYER_READY)
			{
				char_ready = TRUE;
			}
			/* Character was leaving the game */
			if (state == PLAYER_LEAVING)
			{
				char_ready = TRUE;
			}
			old_state = state;
		}
		if (state >= PLAYER_FULL && data_sent == FALSE)
		{
			if (data_ready == TRUE)
			{
				client_setup();
				data_sent = TRUE;
			}
		}
		else
		if (state >= PLAYER_FULL && data_ready == TRUE)
		{
			if (asked_game == FALSE)
			{
				send_play(PLAY_ENTER);
			}
			asked_game = TRUE;
		}
	} while (!(char_ready && data_ready && data_sent));

#ifdef DEBUG
    printf("Starting with %d indicators, %d commands and %d streams\n", known_indicators, custom_commands, known_streams);
#endif
	client_ready();
	send_play(PLAY_PLAY);

	/* Advance to next loop */
	Term_clear();
	Term_fresh();

	Game_loop();
}

/*
 * Propogate all pending updates to the screen
 */
void flush_updates()
{
	/* Redraw status etc if necessary */
	if (p_ptr->redraw)
	{
		redraw_stuff();
	}

	/* Redraw windows if necessary */
	if (p_ptr->window)
	{
		window_stuff();
	}

	/* Redraw air? */
	if (air_updates)
	{
		update_air();
	}

	/* Hack -- don't redraw the screen until we have all of it */
	//if (last_line_info < Term->hgt - SCREEN_CLIP_Y) continue;
	/* Update the screen */
	Term_fresh();
}

/*
 * Loop, looking for net input and responding to keypresses.
 */
static void Game_loop(void)
{
	while (1)
	{
		/* Do networking */
		network_loop();	

		/* See if we have a command waiting */
		request_command(FALSE);

		/* Process any commands we got */
		while (command_cmd)
		{
			/* Process it */
			process_command();

			/* Clear previous command */
			command_cmd = 0;

			/* Ask for another command */
			request_command(FALSE);
		}

		/* Process server-side requests */
		process_requests();

		/* Flush input (now!) */
		flush_now();

		/* Flush output */
		flush_updates();
	}
}

/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
void quit_hook(cptr s)
{
	int j;

	cleanup_network_client();

	/* Nuke each term */
	for (j = 8 - 1; j >= 0; j--)
	{
		/* Unused */
		if (!ang_term[j]) continue;

		/* Nuke it */
		term_nuke(ang_term[j]);
	}

	conf_save();
}

void gather_settings()
{
	/* Graphics */
	Client_setup.settings[0] = use_graphics;

	/* Hitpoint warning */
	Client_setup.settings[3] = p_ptr->hitpoint_warn;
}


/* Trick "net_term_manage" into subscribing by passing an empty array */
void init_subscriptions() 
{
	int i, n;

	u32b empty_flag[ANGBAND_TERM_MAX];

	stream_type* st_ptr;
	int group_top = 0;

#ifdef PMSG_TERM
	/* Force PW_MESSAGE_CHAT on Term-4 */
	window_flag[PMSG_TERM] = PW_MESSAGE_CHAT;
#endif

	/* Fill stream_groups, window_to_stream, stream_to, stream_desc, etc */
#if 1
	for (i = 0; i < known_streams; i++)
	{
		/* Handle single streams */
		st_ptr = &streams[i];

		/* Special case: unresizable stream */
		if ((st_ptr->min_col == st_ptr->max_col)
		 && (st_ptr->min_row == st_ptr->max_row)
		 && (st_ptr->max_row != 0))
		{
			/* Auto-subscribe */
			st_ptr->flag |= SF_AUTO;
		}

		/* Handle stream groups */
		if (i && ((streams[i-1].addr != st_ptr->addr) ||
		          (st_ptr->flag & SF_NEXT_GROUP)))
		{
			group_top = i;
			stream_groups++;
			window_to_stream[st_ptr->addr] = i;
		} else if (!i) {
			window_to_stream[st_ptr->addr] = i;
		}

		/* Set stream_cave reference */
		p_ptr->stream_cave[i] = remote_info[group_top];

		/* Set stream-to-streamgroup reference */
		stream_group[i] = group_top;
	}

	/* Advance some streams to the UI */
	n = 0;
	st_ptr = NULL;
	for (i = 0; i < known_streams; i++)
	{
		if (&streams[stream_group[i]] == st_ptr)
			continue;

		/* Get top member */
		st_ptr = &streams[stream_group[i]];

		/* Hidden stream */
		if (st_ptr->flag & SF_HIDE)
		{
			st_ptr->window_flag = (1L << n);
			continue;
		}

		/* Find an unused slot */
		while (window_flag_desc[n] && n < 32) n++;

		/* Found */
		if (!window_flag_desc[n])
		{
			st_ptr->window_flag = (1L << n);
			/* HACK! Enforce Dungeon View on window 0 */
			if (st_ptr->addr == NTERM_WIN_OVERHEAD) window_flag[0] |= (1L << n);
			/* HACK! Enforce Special View on window 0 */
			if (st_ptr->addr == NTERM_WIN_SPECIAL) window_flag[0] |= (1L << n);
			/* Save "string" */
			window_flag_desc[n] = st_ptr->window_desc;
		}
	}
#endif

	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		empty_flag[i] = 0L;
	}

	net_term_manage(&empty_flag[0], &window_flag[0], FALSE);
}


/*
 * Client is ready to login call-back
 */
bool client_login()
{
	u16b version = CLIENT_VERSION;

	send_login(version, real_name, host_name, nick, pass);

	return TRUE;
}

/*
 * Client is ready to setup call-back
 */
bool client_setup()
{
	int i;

	/* Initialize info arrays */
	init_info();

	/* Initialize the pref files */
	initialize_all_pref_files();

	/* Send request for MOTD to read (optional) */
	//Send_motd(0); // pass -1 to receive motd off-screen

	gather_settings();

	send_settings();

	send_options();

	/* Send visual preferences */
	for (i = 0; i < VISUAL_INFO_PR + 1; i++)
	{
		send_visual_info(i);
	}

	/* Hack -- don't enter the game if waiting for motd */
	//if (Setup.wait && !Setup.ready)
	{
		//return FALSE;
	}

	/* Request gameplay */
	//Send_play(1);

	return TRUE;
}

/*
 * Client is ready to play call-back
 */
bool client_ready()
{
	/* Setup keepalive timer */
	setup_keepalive_timer();

	/* Subscribe to data streams */
	init_subscriptions();

	return TRUE;
}

/* Return 1 to continue, 0 to cancel */
int client_failed(void)
{
	static int try_count = 0;
	event_type chkey;
	char ch = 0;
	byte star = 0;

	if (try_count++ > 2000)
	{
		try_count = 0;
		/* Prompt for auto-retry [grk] */
		put_str("Couldn't connect to server, keep trying? [Y/N]", 21, 1);
		/* Make sure the message is shown */
		Term_fresh();

		while (ch != 'Y' && ch !='y' && ch != 'N' && ch != 'n')
		{
			/* Get a key */
			(void)(Term_inkey(&chkey, FALSE, TRUE));
			ch = chkey.key;
		}
		/* If we dont want to retry, exit with error */
		if(ch == 'N' || ch == 'n') return 0;
		put_str("Connecting to server                          ", 21, 1);
	}

	/* Hack -- show "animation" */
	star = (try_count % 300) / 100;
	put_str(".   ", 21, 21);
	if (star)
		do { put_str(".", 21, 22 + star); }
		while (star--);
	Term_fresh();

	return 1;
}

/*
 * Initialize everything, contact the server, and start the loop.
 */
void client_init(void)
{
	bool done = 0;

	/* Setup the file paths */
	/*init_stuff(); -- Moved elsewhere */

	/* Initialize various arrays */
	init_arrays();

	/* Initialize minor arrays */
	init_minor();

	/* Init network */
	setup_network_client();

	/* Fetch machine name */
	fillhostname(host_name, 80);

	/* Set the default "quit hook" */
	if (quit_aux == NULL) quit_aux = quit_hook;

	/* Default server host and port */
	server_port = conf_get_int("MAngband", "port", 18346);
	my_strcpy(server_name, conf_get_string("MAngband", "host", ""), sizeof(server_name));

	/* Read host/port from the command line */
	clia_read_string(server_name, sizeof(server_name), "host");
	clia_read_int(&server_port, "port");

	/* Check whether we should query the metaserver */
	if (STRZERO(server_name))
	{
		/* Query metaserver */
		if (!get_server_name())
			quit("No server specified.");
	}

	/* Fix "localhost" */
	if (!strcmp(server_name, "localhost"))
		strcpy(server_name, host_name);

	/* Default nickname and password */
	my_strcpy(nick, conf_get_string("MAngband", "nick", nick), MAX_CHARS);
	my_strcpy(pass, conf_get_string("MAngband", "pass", pass), MAX_CHARS);

	/* Nick from command line */
	clia_read_string(nick, MAX_CHARS, "nick");

	/* Get character name and pass */
	get_char_name();

	/* Create a "caller" socket which makes the TCP connection */
	if (call_server(server_name, server_port) == -1) 
	{
		quit("That server either isn't up, or you mistyped the hostname.\n");
	}

	/* Init setup */
	Setup_loop();

	/* Quit */
	quit(NULL);
}
