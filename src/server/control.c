/*
 * Support for the "remote console".  It lets the server admin
 * perform server maintenance without requiring that the server
 * have a devoted tty.
 */

#include "mangband.h"

#define CONSOLE_AUTH 	1
#define CONSOLE_LISTEN	0
#define CONSOLE_WRITE	TRUE
#define CONSOLE_READ 	FALSE

typedef void (*console_cb) (int ind, char *params);
typedef struct console_command_ops {
	char*			name;
	console_cb	call_back;
	char*			comment;
} console_command_ops;
extern console_command_ops console_commands[];
extern int command_len;
 
/*
 * Output some text to the console, if we are listening
 */
void console_print(char *msg, int chan)
{
	int i;
	sockbuf_t *console_buf_w;
	byte *chan_ptr;
	bool hint;
	for (i = 0; i < max_connections; i++)
	{
		if (Conn_is_alive(i)) {
			chan_ptr = Conn_get_console_channels(i);
			hint = FALSE;
			 if( chan_ptr[chan] || (chan == 0 && (hint = Conn_get_console_setting(i, CONSOLE_LISTEN))) )
			 {
			 	console_buf_w = (sockbuf_t*)console_buffer(i, FALSE);
			 	if (!hint) 
			 	{	/* Name channel */
			 		Packet_printf(console_buf_w, "%s", channels[chan].name);
			 		Packet_printf(console_buf_w, "%s", " ");
			 	}
			 	Packet_printf(console_buf_w, "%s%c",msg,'\n');
				Sockbuf_flush(console_buf_w);
			 }
		}
	}
}

/*
 * Return the list of players
 */
static void console_who(int ind, char *useless)
{
	int k, num = 0;
	char brave[15];

	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);

	/* Ugly Hack -- Count players */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];
		if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE))
			num++;
	}

	/* Packet header */
	Packet_printf(console_buf_w, "%s",format("%d players online\n", num));
	
	/* Scan the player list */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];

		/* Add an entry */
		(p_ptr->no_ghost) ? strcpy(brave,"brave \0") : strcpy(brave,"\0"); 
		Packet_printf(console_buf_w, "%s",format("%s is a %slevel %d %s %s at %d ft\n", 
			p_ptr->name, brave, p_ptr->lev, p_name + p_info[p_ptr->prace].name,
			c_name + c_info[p_ptr->pclass].name, p_ptr->dun_depth*50));
			
	}
	Sockbuf_flush(console_buf_w);
}

/*
 * Utility function, change locally as required when testing
 */
static void console_debug(int ind, char *useless)
{
	return;
}

/*
 * Start listening to game server messages
 */
static void console_listen(int ind, char *channel)
{
	int i;
	byte *chan;
	if (channel && !STRZERO(channel)) {
		chan = Conn_get_console_channels(ind);
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			if (!strcmp(channels[i].name, channel))
			{
				chan[i] = 1; 
				break;
			}
		}
	}
	Conn_set_console_setting(ind, CONSOLE_LISTEN, TRUE);
}

/*
 * Return information about a specific player
 */
static void console_whois(int ind, char *name)
{
	int i, len;
	u16b major, minor, patch, extra;
	char brave[15]; //output[1024]; 
	player_type *p_ptr, *p_ptr_search;
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	p_ptr = 0;

	/* Find this player */
	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr_search = Players[i];
		len = strlen(p_ptr_search->name);
		if (!strncasecmp(p_ptr_search->name, name, len))
		{
			p_ptr = p_ptr_search;
		}
	}
	if (!p_ptr)
	{
		Packet_printf(console_buf_w, "%s%c","No such player",'\n');
		Sockbuf_flush(console_buf_w);
		return;
	}
	
	/* Output player information */

	/* General character description */
	(p_ptr->no_ghost) ? strcpy(brave,"brave \0") : strcpy(brave,"\0"); 
	Packet_printf(console_buf_w, "%s",format("%s is a %slevel %d %s %s at %d ft\n", 
		p_ptr->name, brave, p_ptr->lev, p_name + p_info[p_ptr->prace].name,
		c_name + c_info[p_ptr->pclass].name, p_ptr->dun_depth*50));
	
	/* Breakup the client version identifier */
	major = (p_ptr->version & 0xF000) >> 12;
	minor = (p_ptr->version & 0xF00) >> 8;
	patch = (p_ptr->version & 0xF0) >> 4;
	extra = (p_ptr->version & 0xF);

	/* Player connection info */
	Packet_printf(console_buf_w, "%s",format("(%s@%s [%s] v%d.%d.%d.%d)\n", 
		p_ptr->realname, p_ptr->hostname, p_ptr->addr, major, minor, patch, extra));
				
	/* Other interesting factoids */
	if ( p_ptr->lives > 0 )
		Packet_printf(console_buf_w, "%s",format("Has resurected %d times.\n", p_ptr->lives));
	if ( p_ptr->max_dlv == 0 )
		Packet_printf(console_buf_w, "%s",format("Has never left the town!\n"));
	else
		Packet_printf(console_buf_w, "%s",format("Has ventured down to %d ft\n", p_ptr->max_dlv*50));
	i = p_ptr->msg_hist_ptr-1;
	if( i >= 0 )
	{
		if (p_ptr->msg_log[i])
		{
			Packet_printf(console_buf_w, "%s",format("Last message: %s\n", p_ptr->msg_log[i]));
		}
	}
		

	Sockbuf_flush(console_buf_w);
	
}

static void console_message(int ind, char *buf)
{
	/* Send the message */
	player_talk(0, buf);
}

static void console_kick_player(int ind, char *name)
{
	int i, len;
	player_type *p_ptr, *p_ptr_search;
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	p_ptr = 0;

	/* Check the players in the game */
	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr_search = Players[i];
		len = strlen(p_ptr_search->name);
		if (!strncasecmp(p_ptr_search->name, name, len))
		{
			p_ptr = p_ptr_search;
			break;
		}
	}

	/* Check name */
	if (p_ptr)
	{
		/* Kick him */
		Destroy_connection(p_ptr->conn, "kicked out");
		/* Success */
		Packet_printf(console_buf_w, "%s", "Kicked player\n");
		Sockbuf_flush(console_buf_w);
		return;
	}
	else
	{
		/* Failure */
		Packet_printf(console_buf_w, "%s", "No such player\n");
		Sockbuf_flush(console_buf_w);
	}

}

/*
 * Test the integrity of the RNG
 */
static void console_rng_test(int ind, char *useless)
{
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	int i;
	u32b outcome;
	/* This is the expected outcome, generated on our reference platform */
	u32b reference = 0x0D3E5371;
	
	bool randquick = Rand_quick;
	u32b randvalue = Rand_value;
	u16b randplace = Rand_place;
	u32b randstate[RAND_DEG];

	/* Don't run this if any players are connected */
	if(NumPlayers > 0)
	{
		Packet_printf(console_buf_w, "%s", "Can't run the RNG test with players connected!\n");
		Sockbuf_flush(console_buf_w);
		return;
	}
	
	/* Preserve current RNG state */
	for( i=0; i<RAND_DEG; i++ ) randstate[i] = Rand_state[i];

	/* Initialise to a known state */
	Rand_quick = FALSE;
	Rand_value = 0;
	Rand_place = 0;
	Rand_state_init(0xDEADDEAD);
	outcome = 0;

	/* Let the operator know we are busy */
	Packet_printf(console_buf_w, "%s", "Torturing the RNG for 100 million iterations...\n");
	Sockbuf_flush(console_buf_w);

	/* Torture the RNG for a hundred million iterations */
	for(i=0;i<100000000;i++)
	{
		/* Flip between the quick and the complex */
		Rand_quick = (i % 2);
		outcome ^= Rand_mod(0x0FFFFFFF);
		outcome ^= Rand_div(0x0FFFFFFF);	
	}

	/* Display the results */
	if(outcome == reference)
	{
		Packet_printf(console_buf_w, "%s","RNG is working perfectly\n");
	} else {
		Packet_printf(console_buf_w, "%s","RNG integrity check FAILED\n");
		Packet_printf(console_buf_w, "%s",
			format("Outcome was 0x%08X, expected 0x%08X\n",outcome, reference));
	}
	Sockbuf_flush(console_buf_w);
	
	/* Restore the RNG state */
	Rand_quick = randquick;
	Rand_value= randvalue;
	Rand_place = randplace;
	for( i=0; i<RAND_DEG; i++ ) Rand_state[i] = randstate[i];
}

static void console_reload(int ind, char *mod)
{
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	bool done = FALSE;

	if (mod && !strcmp(mod, "config"))
	{
		/* Reload the server preferences */
		load_server_cfg();

		done = TRUE;
	}
	else if (mod && !strcmp(mod, "news"))
	{
		/* Reload the news file */
		Init_setup();

		done = TRUE;
	}
	
	/* Let mangconsole know that the command was a success */
	if (done)
	{
		/* Packet header */
		Packet_printf(console_buf_w, "%s", "Reloaded\n");
	}
	else
	{
		/* Packet header */
		Packet_printf(console_buf_w, "%s", "Reload failed\n");
	}
	
	/* Write the output */
	Sockbuf_flush(console_buf_w);
}

static void console_shutdown(int ind, char *useless)
{
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	
	/* Packet header */
	Packet_printf(console_buf_w, "%s", "Server shutdown\n");

	/* Write the output */
	Sockbuf_flush(console_buf_w);

	/* Shutdown */
	shutdown_server();
}

/* Return list of available console commands */
static void console_help(int ind, char *name)
{
	sockbuf_t *console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	int i;
	bool done = FALSE;
	
	/* Root */
	if (!name || name[0] == ' ' || name[0] == '\0')
	{
		for (i = 0; i < command_len; i++) 
		{
			Packet_printf(console_buf_w, "%s", console_commands[i].name);
			Packet_printf(console_buf_w, "%s", " ");
		}
		Packet_printf(console_buf_w, "%s", "\n");
		done = TRUE;
	}
	/* Specific command */
	else
	{
		for (i = 0; i < command_len; i++) 
		{
			/* Found it */
			if (!strcmp(console_commands[i].name, name))
			{
				Packet_printf(console_buf_w, "%s", console_commands[i].name);
				Packet_printf(console_buf_w, "%s", " ");
				Packet_printf(console_buf_w, "%s", console_commands[i].comment);
				Packet_printf(console_buf_w, "%s", "\n");
				done = TRUE;
			}
		}
	}
	
	if (!done)
		Packet_printf(console_buf_w, "%s", "Unrecognized command\n");	
	
	Sockbuf_flush(console_buf_w);
} 

/*
 * This is the response function when incoming data is received on the
 * control pipe.
 */
void NewConsole(int read_fd, int arg)
{
	char passwd[80], buf[1024];
	char *params;
	int bytes, buflen, ind;
	int newsock = 0;
	int i, j;

	sockbuf_t *console_buf_w = NULL;
	sockbuf_t *console_buf_r = NULL;
	
	if (arg < 0) 
	{
		ind = abs(arg) - 1;
		arg = 1;
	} 
	else 
	{
		ind = arg;
		arg = 0;	
	}
	
	console_buf_w = (sockbuf_t*)console_buffer(ind, CONSOLE_WRITE);
	console_buf_r = (sockbuf_t*)console_buffer(ind, CONSOLE_READ);

	/* Make a TCP connection */
	/* Hack -- check if this data has arrived on the contact socket or not.
	 * If it has, then we have not created a connection with the client yet, 
	 * and so we must do so.
	 */
	if (arg)
	{
		newsock = read_fd;

		if (newsock) remove_input(newsock);

		console_buf_r->sock = console_buf_w->sock = newsock;

		if (SetSocketNonBlocking(newsock, 1) == -1)
		{
			plog("Can't make contact socket non-blocking");
		}
		
		install_input(NewConsole, newsock, ind);
		
		Conn_set_console_setting(ind, CONSOLE_AUTH, FALSE);
		Conn_set_console_setting(ind, CONSOLE_LISTEN, FALSE);

		Sockbuf_clear(console_buf_w);
		Packet_printf(console_buf_w, "%s","Connected\n");
		Sockbuf_flush(console_buf_w);

		return;
	}

	newsock = console_buf_r->sock;

	/* Clear the buffer */
	Sockbuf_clear(console_buf_r);
	/* Read the message */
	bytes = DgramReceiveAny(read_fd, console_buf_r->buf, console_buf_r->size);

	/* If this happens our TCP connection has probably been severed. Remove the input. */
	if(!bytes && errno != EAGAIN && errno != EWOULDBLOCK)
	{
		Destroy_connection(ind, "console down");
		return;
	}
	if (bytes < 0)
	{
		/* Hack - ignore these errors */
		if(errno == EAGAIN || errno == EINTR)
		{ 
			GetSocketError(newsock);
			return;
		}
		/* We have a socket error, disconnect */
		Destroy_connection(ind, "console down");
		return;
	}

	/* Set length */
	console_buf_r->len = bytes;

	/* Get the password if not authenticated */
	if(!Conn_get_console_setting(ind, CONSOLE_AUTH))
	{
		Packet_scanf(console_buf_r, "%N",passwd);

		/* Hack: comply with telnet */
		buflen = strlen(passwd);
		if (buflen && passwd[buflen-1] == '\r') passwd[buflen-1] = '\0';
		
		/* Check for illegal accesses */
		if (!cfg_console_password || strcmp(passwd, cfg_console_password))
		{
			/* Clear buffer */
			Sockbuf_clear(console_buf_w);
	
			/* Put an "illegal access" reply in the buffer */
			Packet_printf(console_buf_w, "%s", "Invalid password\n");
			
			/* Send it */
			DgramWrite(read_fd, console_buf_w->buf, console_buf_w->len);

			/* Log this to the local console */
			plog(format("Incorrect console password from %s.", DgramLastname()));
			
			/* Kill him */
			Destroy_connection(ind, "console down");
			return;
		}
		else 
		{
			/* Clear buffer */
			Sockbuf_clear(console_buf_w);
			Conn_set_console_setting(ind, CONSOLE_AUTH, TRUE);
			Packet_printf(console_buf_w, "%s","Authenticated\n");
			Sockbuf_flush(console_buf_w);
			return;
		}
	}

	/* Acquire command in the form: <command> <params> */
	Packet_scanf(console_buf_r, "%N", buf);
	buflen = strlen(buf);

	/* Hack: comply with telnet */
	if (buflen && buf[buflen-1] == '\r') buf[buflen-1] = '\0';
		
	/* Split up command and params */
	if( (params = strstr(buf," ")) )
	{
		*params++ = '\0';
	}
	else
	{
		params = NULL;//buf;
	}

	/* Clear buffer */
	Sockbuf_clear(console_buf_r);
	
	/* Paranoia to ease ops-coder's life later */
	if (STRZERO(buf)) return;
	
	/* Execute console command */
	buflen = strlen(buf);
	for (i = 0; i < command_len; i++) 
	{
	 	if (!strncmp(buf, console_commands[i].name, (j = strlen(console_commands[i].name)) ) && (buflen <= j || buf[j] == ' ')) 
	 	{
			(console_commands[i].call_back)(ind, params);				
	 		break;
	 	}
	}
	
}

console_command_ops console_commands[] = {
	{ "help",      console_help,        "[TOPIC]\nExplain a command or list all avaliable"	},
  	{ "listen",    console_listen,      "[CHANNEL]\nAttach self to #public or specified"	},
  	{ "who",       console_who,         "\nList players"												},
  	{ "shutdown",  console_shutdown,    "\nKill server"												},
  	{ "msg",       console_message,     "MESSAGE\nBroadcast a message"							},
  	{ "kick",      console_kick_player, "PLAYERNAME\nKick player from the game"				},
  	{ "reload",    console_reload,      "config|news\nReload mangband.cfg or news.txt"		},
  	{ "whois",     console_whois,       "PLAYERNAME\nDetailed player information"				},
  	{ "rngtest",   console_rng_test,    "\nPerform RNG test"											},
  	{ "debug",     console_debug,       "\nUnused"														},
};
int command_len = sizeof(console_commands) / sizeof(console_command_ops);
