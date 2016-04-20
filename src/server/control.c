/*
 * Support for the "remote console".  It lets the server admin
 * perform server maintenance without requiring that the server
 * have a devoted tty.
 */
#include "mangband.h"
#include "net-server.h"

#define CONSOLE_AUTH 	1
#define CONSOLE_LISTEN	0
#define CONSOLE_WRITE	TRUE
#define CONSOLE_READ 	FALSE

typedef struct console_connection {
	connection_type	conn;	/* id */
	bool	auth;	/* logged in? */
	bool	listen; /* legacy listener */
	byte	on_channel[MAX_CHANNELS];
} console_connection;
typedef void (*console_cb) (connection_type *ct, char *params);
typedef struct console_command_ops {
	char*   	name;
	console_cb	call_back;
	char*   	comment;
} console_command_ops;
console_command_ops console_commands[];
int command_len;

eptr first_console = NULL;

int accept_console(int data1, data data2) {
	eptr new_connection;
	eptr new_console;
	bool old = (data1 == -1 ? TRUE : FALSE);
	connection_type *ct = NULL;
	console_connection *cn = NULL;	

	/* Add connection */
	if (!old)
	{
		new_connection = add_connection(first_connection, data1, console_read, console_close);
		if (!first_connection) first_connection = new_connection;
		ct = new_connection->data2;
	}
	/* Update connection */
	else
	{
		ct = (connection_type *)data2;
		ct->receive_cb = console_read;
		ct->close_cb = console_close;
	}

	/* Add console */
	MAKE(cn, console_connection);
	WIPE(cn, console_connection);
	new_console = e_add(first_console, ct, cn);
	if (!first_console) first_console = new_console;
	
	/* save pointer to console in connection */
	ct->uptr = (data)cn;

	/* Initial states */
	cn->auth = cn->listen = FALSE;

	/* Inform */
	cq_printf(&ct->wbuf, "%T", "Connected\n");

	return 0;
}
int console_close(int data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	console_connection *cn = ct->uptr;

	KILL(cn);

	return 0;
}
int console_read(int data1, data data2) { /* return -1 on error */
	connection_type *ct = data2;
	console_connection *cn = ct->uptr;//e_find(first_console, ct, NULL);

	int start_pos;
	int buflen;
	int i, j;

	char buf[1024];
	char *params;	

	/* Parse "read buffer" */
	while (	cq_len(&ct->rbuf) )
	{
		/* Ensure string is ready */
		start_pos = ct->rbuf.pos;
		if (cq_scanf(&ct->rbuf, "%T", buf) < 1)
		{
			/* String not ready */
			ct->rbuf.pos = start_pos;
			return 0;
		}

		/* Hijack input for login */
		if (cn->auth == FALSE)
		{
			/* Check for illegal accesses */
			if (!cfg_console_password || strcmp(buf, cfg_console_password))
			{
				/* Bail out! */
				plog(format("{CON} Authentication failure %s", ct->host_addr));				
				cq_printf(&ct->wbuf, "%T", "Invalid password\n");
				return -1;		
			}
			/* Move on */
			cn->auth = TRUE;
			cq_printf(&ct->wbuf, "%T", "Authenticated\n");
			continue;
		}

		/* Paranoia to ease ops-coder's life later */
		if (STRZERO(buf)) break;

		/* Split up command and params */
		if( (params = strstr(buf," ")) )
		{
			*params++ = '\0';
		}
		else
		{
			params = NULL;
		}

		/* Execute console command */
		buflen = strlen(buf);
		for (i = 0; i < command_len; i++) 
		{
			if (!strncmp(buf, console_commands[i].name, (j = strlen(console_commands[i].name)) ) && (buflen <= j || buf[j] == ' ')) 
			{
				(console_commands[i].call_back)(ct, params);				
				break;
			}
		}
	}

	/* Ok */
	return 0;
}

/*
 * Output some text to the console, if we are listening
 */
void console_print(char *msg, int chan)
{
	eptr console;
	for (console = first_console; console; console = console->next)
	{
		connection_type *ct = console->data1;
		console_connection *cn = console->data2;
		bool hint = FALSE;
		if( cn->on_channel[chan] || (chan == 0 && (hint = cn->listen)) )
		{
			if (!hint) cq_printf(&ct->wbuf, "%T%c", channels[chan].name, ' ');
			cq_printf(&ct->wbuf, "%T%c", msg, '\n');
		}
	}
}

/*
 * Return the list of players
 */
static void console_who(connection_type* ct, char *useless)
{
	int k, num = 0;
	char brave[15];

	/* Ugly Hack -- Count players */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];
		if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE))
			num++;
	}

	/* Packet header */
	cq_printf(&ct->wbuf, "%T",format("%d players online\n", num));
	
	/* Scan the player list */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];

		/* Add an entry */
		(p_ptr->no_ghost) ? strcpy(brave,"brave \0") : strcpy(brave,"\0"); 
		cq_printf(&ct->wbuf,"%T", format("%s is a %slevel %d %s %s at %d ft\n", 
			p_ptr->name, brave, p_ptr->lev, p_name + p_info[p_ptr->prace].name,
			c_name + c_info[p_ptr->pclass].name, p_ptr->dun_depth*50));
			
	}
}

/*
 * Return the list of connections
 */
static void console_conn(connection_type* ct, char *useless)
{
	char conn_type[MAX_PLAYERS];
	eptr iter;
	int j = 0;
	cq_printf(&ct->wbuf, "%T", "Listing connections\n");

	for (iter = first_connection; iter; iter = iter->next)
	{
		char buf[80];
		connection_type* c_ptr = iter->data2; 
		j++;
		sprintf(buf, "Connection %d - %s\n", j, c_ptr->host_addr);
		cq_printf(&ct->wbuf, "%T", buf);
	}
}

/*
 * Utility function, change locally as required when testing
 */
static void console_debug(connection_type* ct, char *useless)
{
	return;
}

/*
 * Start listening to game server messages
 */
static void console_listen(connection_type* ct, char *channel)
{
	console_connection *cn = ct->uptr;
	int i;
	if (channel && !STRZERO(channel)) {
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			if (!strcmp(channels[i].name, channel))
			{
				cn->on_channel[i] = 1; 
				break;
			}
		}
	}
	cn->listen = TRUE;
}

/*
 * Return information about a specific player
 */
static void console_whois(connection_type* ct, char *name)
{
	int i, len;
	u16b major, minor, patch, extra;
	char brave[15]; //output[1024]; 
	player_type *p_ptr, *p_ptr_search;

	p_ptr = 0;

	/* Find this player */
	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr_search = Players[i];
		len = strlen(p_ptr_search->name);
		if (!my_strnicmp(p_ptr_search->name, name, len))
		{
			p_ptr = p_ptr_search;
		}
	}
	if (!p_ptr)
	{
		cq_printf(&ct->wbuf, "%T", "No such player\n");
		return;
	}
	
	/* Output player information */

	/* General character description */
	(p_ptr->no_ghost) ? strcpy(brave,"brave \0") : strcpy(brave,"\0"); 
	cq_printf(&ct->wbuf, "%T", format("%s is a %slevel %d %s %s at %d ft\n", 
		p_ptr->name, brave, p_ptr->lev, p_name + p_info[p_ptr->prace].name,
		c_name + c_info[p_ptr->pclass].name, p_ptr->dun_depth*50));
	
	/* Breakup the client version identifier */
	major = (p_ptr->version & 0xF000) >> 12;
	minor = (p_ptr->version & 0xF00) >> 8;
	patch = (p_ptr->version & 0xF0) >> 4;
	extra = (p_ptr->version & 0xF);

	/* Player connection info */
	cq_printf(&ct->wbuf, "%T", format("(%s@%s [%s] v%d.%d.%d.%d)\n", 
		p_ptr->realname, p_ptr->hostname, p_ptr->addr, major, minor, patch, extra));

	/* Other interesting factoids */
	if ( p_ptr->lives > 0 )
		cq_printf(&ct->wbuf, "%T", format("Has resurected %d times.\n", p_ptr->lives));
	if ( p_ptr->max_dlv == 0 )
		cq_printf(&ct->wbuf, "%T", format("Has never left the town!\n"));
	else
		cq_printf(&ct->wbuf, "%T", format("Has ventured down to %d ft\n", p_ptr->max_dlv*50));
	i = p_ptr->msg_hist_ptr-1;
	if( i >= 0 )
	{
		if (!STRZERO(p_ptr->msg_log[i]))
		{
			cq_printf(&ct->wbuf, "%T", format("Last message: %s\n", p_ptr->msg_log[i]));
		}
	}
}

static void console_message(connection_type* ct, char *buf)
{
	/* Send the message */
	player_talk(0, buf);
}

static void console_kick_player(connection_type* ct, char *name)
{
	int i, len;
	player_type *p_ptr, *p_ptr_search;
	p_ptr = 0;

	/* Check the players in the game */
	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr_search = Players[i];
		len = strlen(p_ptr_search->name);
		if (!my_strnicmp(p_ptr_search->name, name, len))
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
		cq_printf(&ct->wbuf, "%T", "Kicked player\n");
		return;
	}
	else
	{
		/* Failure */
		cq_printf(&ct->wbuf, "%T", "No such player\n");
	}

}

/*
 * Test the integrity of the RNG
 */
static void console_rng_test(connection_type* ct, char *useless)
{
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
		cq_printf(&ct->wbuf, "%T", "Can't run the RNG test with players connected!\n");
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
	cq_printf(&ct->wbuf, "%T", "Torturing the RNG for 100 million iterations...\n");
	//DOESN'T WORK RIGHT WITHOUT IMMEDIATE FLUSH...

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
		cq_printf(&ct->wbuf, "%T", "RNG is working perfectly\n");
	} else {
		cq_printf(&ct->wbuf, "%T", "RNG integrity check FAILED\n");
		cq_printf(&ct->wbuf, "%T",
			format("Outcome was 0x%08X, expected 0x%08X\n",outcome, reference));
	}

	/* Restore the RNG state */
	Rand_quick = randquick;
	Rand_value= randvalue;
	Rand_place = randplace;
	for( i=0; i<RAND_DEG; i++ ) Rand_state[i] = randstate[i];
}

static void console_reload(connection_type* ct, char *mod)
{
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
		//Init_setup();
		//TODO....

		done = TRUE;
	}
	
	/* Let mangconsole know that the command was a success */
	if (done)
	{
		/* Packet header */
		cq_printf(&ct->wbuf, "%T", "Reloaded\n");
	}
	else
	{
		/* Packet header */
		cq_printf(&ct->wbuf, "%T", "Reload failed\n");
	}
}

static void console_shutdown(connection_type* ct, char *when)
{
	int min = 0;
	//if (!my_stricmp(when, "NOW")) min = 0;
	//else if (IS_VALID_NUMBER(when)) min = when

	/* Now */
	if (min == 0)
	{
		/* Tell */
		cq_printf(&ct->wbuf, "%T", "Server shutdown\n");

		/* Shutdown */
		shutdown_server();
	} 
	else
	/* Delayed */
	{
		/* Set timer (in seconds) */
		shutdown_timer= min * 60;
	}
}

/* Return list of available console commands */
static void console_help(connection_type* ct, char *name)
{
	int i;
	bool done = FALSE;

	/* Root */
	if (!name || name[0] == ' ' || name[0] == '\0')
	{
		for (i = 0; i < command_len; i++)
		{
			cq_printf(&ct->wbuf, "%T", console_commands[i].name);
			cq_printf(&ct->wbuf, "%T", " ");
		}
		cq_printf(&ct->wbuf, "%T", "\n");
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
				cq_printf(&ct->wbuf, "%T", console_commands[i].name);
				cq_printf(&ct->wbuf, "%T", " ");
				cq_printf(&ct->wbuf, "%T", console_commands[i].comment);
				cq_printf(&ct->wbuf, "%T", "\n");
				done = TRUE;
			}
		}
	}

	if (!done)
		cq_printf(&ct->wbuf, "%T", "Unrecognized command\n");

}

console_command_ops console_commands[] = {
	{ "help",      console_help,        "[TOPIC]\nExplain a command or list all avaliable"	},
	{ "listen",    console_listen,      "[CHANNEL]\nAttach self to #public or specified"	},
	{ "who",       console_who,         "\nList players"                                	},
	{ "conn",      console_conn,         "\nList connections"                           	},
	{ "shutdown",  console_shutdown,    "[TIME|NOW]\nKill server in TIME minutes or 'NOW'"	},
	{ "msg",       console_message,     "MESSAGE\nBroadcast a message"                  	},
	{ "kick",      console_kick_player, "PLAYERNAME\nKick player from the game"         	},
	{ "reload",    console_reload,      "config|news\nReload mangband.cfg or news.txt"  	},
	{ "whois",     console_whois,       "PLAYERNAME\nDetailed player information"       	},
	{ "rngtest",   console_rng_test,    "\nPerform RNG test"                            	},
	{ "debug",     console_debug,       "\nUnused"                                      	},
};
int command_len = sizeof(console_commands) / sizeof(console_command_ops);
