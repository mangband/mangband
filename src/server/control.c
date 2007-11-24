/*
 * Support for the "remote console".  It lets the server admin
 * perform server maintenance without requiring that the server
 * have a devoted tty.
 */

#include "angband.h"

static sockbuf_t console_buf;
static bool console_authenticated;
static bool console_listen;

/*
 * Output some text to the console, if we are listening
 */
void console_print(char *msg)
{
	printf("Listener heard: %s\n",msg);
	if (console_listen)
	{
		Packet_printf(&console_buf, "%s%c",msg,'\n');
		Sockbuf_flush(&console_buf);
	}
}

/*
 * Return the list of players
 */
static void console_status()
{
	int k;

	/* Packet header */
	Packet_printf(&console_buf, "%s",format("%d players online\n", NumPlayers));
	
	/* Scan the player list */
	for (k = 1; k <= NumPlayers; k++)
	{
		player_type *p_ptr = Players[k];

		/* Add an entry */
		Packet_printf(&console_buf, "%s",format("%s is a level %d %s %s at %d ft\n", 
			p_ptr->name, p_ptr->lev, race_info[p_ptr->prace].title,
			class_info[p_ptr->pclass].title, p_ptr->dun_depth*50));
			
	}
	Sockbuf_flush(&console_buf);
}

static void console_message(char *buf)
{
	/* Send the message */
	player_talk(0, buf);
}

static void console_kick_player(char *name)
{
	int i;

	/* Check the players in the game */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check name */
		if (!strcmp(name, Players[i]->name))
		{
			/* Kick him */
			Destroy_connection(Players[i]->conn, "kicked out");

			/* Success */
			Packet_printf(&console_buf, "%s", "Kicked player\n");
			Sockbuf_flush(&console_buf);
			return;
		}
	}

	/* Failure */
	Packet_printf(&console_buf, "%s", "No such player\n");
	Sockbuf_flush(&console_buf);

}

static void console_reload_server_preferences(void)
{
	/* Reload the server preferences */
	load_server_cfg();

	/* Let mangconsole know that the command was a success */
	/* Packet header */
	Packet_printf(&console_buf, "%s", "Reloaded\n");

	/* Write the output */
	DgramReply(console_buf.sock, console_buf.ptr, console_buf.len);
}

static void console_shutdown(void)
{
	/* Packet header */
	Packet_printf(&console_buf, "%s", "Server shutdown\n");

	/* Write the output */
	Sockbuf_flush(&console_buf);

	/* Shutdown */
	shutdown_server();
}

/*
 * This is the response function when incoming data is received on the
 * control pipe.
 */
void NewConsole(int read_fd, int arg)
{
	char ch, passwd[80], buf[1024], cmd[16];
	char *params;
	int i, j, bytes, buflen;
	static int newsock = 0;

	/* Make a TCP connection */
	/* Hack -- check if this data has arrived on the contact socket or not.
	 * If it has, then we have not created a connection with the client yet, 
	 * and so we must do so.
	 */
	if (read_fd == ConsoleSocket)
	{
		// Hack -- make sure that two people haven't tried to use mangconsole
		// at the same time.  Since I am currently too lazy to support this,
		// we will remove the input of the first person when the second person
		// connects.
		if (newsock) remove_input(newsock);
		if ((newsock = SocketAccept(read_fd)) == -1)
		{
			quit("Couldn't accept TCP connection.\n");
		}
		console_buf.sock = newsock;
		install_input(NewConsole, newsock, 2);
		console_authenticated = FALSE;
		console_listen = FALSE;
		Packet_printf(&console_buf, "%s","Connected\n");
		Sockbuf_flush(&console_buf);

		return;
	}


	/* Clear the buffer */
	Sockbuf_clear(&console_buf);
	/* Read the message */
	bytes = DgramReceiveAny(read_fd, console_buf.buf, console_buf.size);
	printf("got %d bytes\n",bytes);

	/* Check for errors or our TCP connection closing */
	if (bytes <= 0)
	{
		/* If this happens our TCP connection has probably been severed.
		 * Remove the input.
		 */
		//s_printf("Error reading from console socket\n");
		remove_input(newsock);
		newsock = 0;

		return;
	}

	/* Set length */
	console_buf.len = bytes;

	/* Acquire sender's address */
//	strcpy(host_name, DgramLastname()); 

	/* Get the password if not authenticated */
	if(!console_authenticated)
	{
		Packet_scanf(&console_buf, "%N",passwd); 

		/* Check for illegal accesses */
		if (strcmp(passwd, cfg_console_password))
		{
			/* Clear buffer */
			Sockbuf_clear(&console_buf);
	
			/* Put an "illegal access" reply in the buffer */
			Packet_printf(&console_buf, "%s", "Invalid password\n");
			
			/* Send it */
			DgramWrite(read_fd, console_buf.buf, console_buf.len);

			/* Log this to the local console */
			s_printf("Illegal console command from %s.\n", DgramLastname());

			return;
		}
		else 
		{
			/* Clear buffer */
			Sockbuf_clear(&console_buf);
			console_authenticated = TRUE;
			Packet_printf(&console_buf, "%s","Authenticated\n");
			Sockbuf_flush(&console_buf);
		}
	}

	/* Acquire command in the form: <command> <params> */
	Packet_scanf(&console_buf, "%N", buf);
	buflen = strlen(buf);
	for(i=0;i<sizeof(cmd);i++)
	{
		if (i < buflen)
		{
			if (buf[i] == ' ') {
				cmd[i] = '\0';
				params = &buf[i+1];
				break;
			} else {
				cmd[i] = buf[i];
			}
		}
	}

	/* Clear buffer */
	Sockbuf_clear(&console_buf);

	/* Determine what the command is */
	if (!strncmp(cmd,"listen",6)) 
	{
		console_listen = TRUE;
	}
	else if (!strncmp(cmd,"status",6)) 
	{
		console_status();
	}
	else if (!strncmp(cmd,"shutdown",8))
	{
		console_shutdown();
	}
	else if (!strncmp(cmd,"msg",3))
	{
		console_message(params);
	}
	else if (!strncmp(cmd,"kick",4))
	{
		console_kick_player(params);
	}
	else if (!strncmp(cmd,"reload",6))
	{
		console_reload_server_preferences();
	}
	
}

/*
 * Initialize the stuff for the new console
 */
bool InitNewConsole(int write_fd)
{
	/* Initialize buffer */
	if (Sockbuf_init(&console_buf, write_fd, 8192, SOCKBUF_READ | SOCKBUF_WRITE))
	{
		/* Failed */
		s_printf("No memory for console buffer.\n");

		return FALSE;
	}

	return TRUE;
}

