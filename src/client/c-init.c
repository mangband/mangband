/* Client initialization module */

/*
 * This file should contain non-system-specific code.  If a 
 * specific system needs its own "main" function (such as
 * Windows), then it should be placed in the "main-???.c" file.
 */

#include "c-angband.h"

static int Socket;

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
	strcpy(path, tail ? tail : PKGDATADIR);

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) strcat(path, PATH_SEP);

	/* Read/Write path from config file */
	strncpy(path, conf_get_string("MAngband", "LibDir", path), 1024);
	conf_set_string("MAngband", "LibDir", path);

	/* Initialize */
	init_file_paths(path);
}

/* Init minor arrays */
void init_minor(void)
{
	int i;
	/* Chat channels */
	C_MAKE(p_ptr->on_channel, MAX_CHANNELS, byte);
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		channels[i].name[0] = '\0';
		channels[i].id = channels[i].num = 0;
		p_ptr->on_channel[i] = FALSE; 
	}
	p_ptr->main_channel = 0;
	
	/* Term channels */
	p_ptr->remote_term = NTERM_WIN_OVERHEAD;

	/* Server-defined network behavior */
	known_streams = 0;
	known_window_streams = 0;
	window_to_stream[0] = 0;
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


/*
 * Loop, looking for net input and responding to keypresses.
 */
static void Input_loop(void)
{
	int	netfd, result;

	if (Net_flush() == -1)
		return;

	if ((netfd = Net_fd()) == -1)
	{
		plog("Bad socket filedescriptor");
		return;
	}

	for (;;)
	{
		// Send out a keepalive packet if need be
		// do_keepalive();

		if (Net_flush() == -1)
		{
			plog("Bad net flush");
			return;
		}

		/* Set the timeout on the network socket */
		/* This polling should probably be replaced with a select
		 * call that combines the net and keyboard input.
		 * This REALLY needs to be replaced since under my Linux
		 * system calling usleep seems to have a 10 ms overhead
		 * attached to it.
		 */
		//SetTimeout(0, 1000000 / Setup.frames_per_second);
		SetTimeout(0, 1000000 / 1000);

		/* Only take input if we got some */
		if (SocketReadable(netfd))
		{
			if ((result = Net_input()) == -1)
			{
				/*plog("Bad net input"); */
				return;
			}
		}

		/* See if we have a command waiting */
		request_command(FALSE);

		/* Process any commands we got */
		while (command_cmd)
		{
			/* Process it */
			process_command();

			/* XXX Unused */
			//last_sent = time(NULL);

			/* Clear previous command */
			command_cmd = 0;

			/* Ask for another command */
			request_command(FALSE);
		}

		// Update our internal timer, which is used to figure out when
		// to send keepalive packets.
		update_ticks();
		
		/* Redraw status etc if necessary */
		if (p_ptr->redraw)
		{
			redraw_stuff();
		}

		/* Hack -- don't redraw the screen until we have all of it */
		if (last_line_info < Term->hgt - SCREEN_CLIP_Y) continue;

		/* Flush input (now!) */
		flush_now();

		/* Update the screen */
		Term_fresh();

		/* Redraw windows if necessary */
		if (p_ptr->window)
		{
			window_stuff();
		}
		do_keepalive(); // [grk] - wasn't keeping connection alive

	}
}	

/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(cptr s)
{
	int j;

	Net_cleanup();
	SocketCloseAll();

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
	int i;
	/* Graphics */
	Client_setup.settings[0] = use_graphics;

	/* Dungeon Size */
	Client_setup.settings[1] = Term->wid;
	Client_setup.settings[2] = Term->hgt - SCREEN_CLIP_L; // Top Line
	Client_setup.settings[1] -= DUNGEON_OFFSET_X; // Compact
	Client_setup.settings[2] -= DUNGEON_OFFSET_Y; // Status line

	/* Hitpoint warning */
	Client_setup.settings[3] = p_ptr->hitpoint_warn;
	
	/* Terminal subscriptions */
	Client_setup.settings[4] = 0;
	for (i = 0; i < 8; i++)
		Client_setup.settings[4] |= window_flag[i];
}


/* Trick "net_term_manage" into subscribing by passing an empty array */
void init_subscriptions() 
{
	int i;

	u32b empty_flag[ANGBAND_TERM_MAX];

	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		empty_flag[i] = 0L;
	}

	net_term_manage(&empty_flag[0], &window_flag[0], FALSE);
}


/*
 * Client is ready to play call-back
 */
bool client_ready()
{
	/* Send request for MOTD to read (optional) */
	Send_motd(0); // pass -1 to receive motd off-screen 
	
	/* Initialize the pref files */
	initialize_all_pref_files();

	gather_settings();

	Send_options(TRUE);

	/* Send visual preferences */
	Net_verify();

	/* Subscribe to data streams */
	init_subscriptions();

	/* Hack -- don't enter the game if waiting for motd */
	if (Setup.wait && !Setup.ready)
	{
		return FALSE;
	}		
		
	/* Request gameplay */
	Send_play(1);
	
	return TRUE;
}

/*
 * Initialize everything, contact the server, and start the loop.
 */
void client_init(char *argv1)
{
	sockbuf_t ibuf;
	unsigned char status;
	int trycount;
	char host_name[80], trymsg[80], c, *s;
	u16b version = CLIENT_VERSION;
	u16b conntype = CONNTYPE_PLAYER;
	bool done = 0;

	/* Setup the file paths */
	/*init_stuff(); -- Moved elsewhere */

	/* Initialize various arrays */
	init_arrays();
	
	/* Initialize minor arrays */
	init_minor();

	GetLocalHostName(host_name, 80);

	/* Set the "quit hook" */
	// Hmm trapping this here, overwrites any quit_hook that the main-xxx.c code
	// may have. So for the windows client, we disable this. The main-win.c file
	// does this stuff anyway [grk]
#ifndef WINDOWS
	quit_aux = quit_hook;
#endif
	server_port = conf_get_int("MAngband", "Port", 18346);

	/* Check whether we should query the metaserver */
	if (argv1 == NULL)
	{
		/* Query metaserver */
		if (!get_server_name())
			quit("No server specified.");
	}
	else
	{
		/* Set the server's name */
		strcpy(server_name, argv1);
		/* Set server port */
		s = strchr(server_name, ':');
		if (s) 
		{
		    sscanf(s, ":%d", &server_port);
		    strcpy (s, "\0");
		}
	}

	/* Fix "localhost" */
	if (!strcmp(server_name, "localhost"))
		strcpy(server_name, host_name);

	/* Default nickname and password */
	strcpy(nick, conf_get_string("MAngband", "nick", nick));
	strcpy(pass, conf_get_string("MAngband", "pass", pass));
	
	
	/* Get character name and pass */

	get_char_name();

	/* Capitalize the name */
	nick[0] = toupper(nick[0]);

	// Create the net socket and make the TCP connection
	if ((Socket = CreateClientSocket(server_name, server_port)) == -1)
	{
	    while (!done) {
		/* Prompt for auto-retry [grk] */
		put_str("Couldn't connect to server, keep trying? [Y/N]", 21, 1);
		/* Make sure the message is shown */
		Term_fresh();
		c = 0;
		while (c != 'Y' && c!='y' && c != 'N' && c != 'n')
			/* Get a key */
			c = inkey();

		/* If we dont want to retry, exit with error */
		if(c=='N' || c=='n')
			quit("That server either isn't up, or you mistyped the hostname.\n");

		/* ...else, keep trying until socket connected */
		trycount = 1;
		while( (Socket = CreateClientSocket(server_name, server_port)) == -1)
		{
			if (trycount > 200) break;
			/* Progress Message */
			sprintf(trymsg, "Connecting to server [%i]                      ",trycount++);
			put_str(trymsg, 21, 1);
			/* Make sure the message is shown */
			Term_redraw(); /* Hmm maybe not the proper way to force an os poll */
			Term_flush();
		}
		if (Socket != -1) done = 1;
	    }
	}

	/* Create a socket buffer */
	if (Sockbuf_init(&ibuf, Socket, CLIENT_SEND_SIZE,
		SOCKBUF_READ | SOCKBUF_WRITE) == -1)
	{
		quit("No memory for socket buffer\n");
	}

	/* Make it non-blocking 
	if (SetSocketNonBlocking(Socket, 1) == -1)
	{	
		quit("Can't make socket non-blocking\n");
	} */

	/* Clear it */
	Sockbuf_clear(&ibuf);

	/* Put the contact info in it */
	Packet_printf(&ibuf, "%hu", conntype);
	Packet_printf(&ibuf, "%hu", version);
	Packet_printf(&ibuf, "%s%s%s%s", real_name, host_name, nick, pass);
	
	/* Send it */
	if (!Net_Send(Socket, &ibuf))
		quit("Couldn't send contact information\n");

	/* Wait for reply */
	if (!Net_WaitReply(Socket, &ibuf, 10))
		quit("Server didn't respond!\n");

	/* Read what he sent */
	Packet_scanf(&ibuf, "%c", &status);
	
	/* Some error */
	if (status)
	{
		/* The server didn't like us.... */
		switch (status)
		{
			case E_NEED_INFO: break;
			case E_VERSION:
				quit("This version of the client will not work with that server.");
			case E_SOCKET:
				quit("Socket error");
			case E_BAD_PASS:
				quit("The password you supplied is incorrect");
			case E_READ_ERR:
				quit("There was an error accessing your savefile");
			case E_GAME_FULL:
				quit("Sorry, the game is full.  Try again later.");
			case E_IN_USE:
				quit("That nickname is already in use.  If it is your nickname, wait 30 seconds and try again.");
			case E_INVAL:
				quit("The server didn't like your nickname, realname, or hostname.");
			case E_TWO_PLAYERS:
				quit("There is already another character from this user/machine on the server.");
			default:
				quit(format("Connection failed with status %d.", status));
		}
	}
	
	/* Server agreed to talk, initialize the buffers */
	if (Net_init(Socket) == -1)
	{
		quit("Network initialization failed!\n");
	}
	
	/* Send request for anything needed for play	*/
	Send_play(0);

	/* Main loop */
	Input_loop();

	/* Cleanup network stuff */
	Net_cleanup();

	/* Quit, closing term windows */
	quit(NULL);
}
