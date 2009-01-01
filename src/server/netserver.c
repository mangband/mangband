/* The server side of the network stuff */

/* The following is a direct excerpt from the netserver.c
 * in the XPilot distribution.  Much of it is incorrect
 * in MAngband's case, but most of it is still correct.
 */



/*
 * This is the server side of the network connnection stuff.
 *
 * We try very hard to not let the game be disturbed by
 * players logging in.  Therefore a new connection
 * passes through several states before it is actively
 * playing.
 * First we make a new connection structure available
 * with a new socket to listen on.  This socket port
 * number is told to the client via the pack mechanism.
 * In this state the client has to send a packet to this
 * newly created socket with its name and playing parameters.
 * If this succeeds the connection advances to its second state.
 * In this second state the essential server configuration
 * like the map and so on is transmitted to the client.
 * If the client has acknowledged all this data then it
 * advances to the third state, which is the
 * ready-but-not-playing-yet state.  In this state the client
 * has some time to do its final initializations, like mapping
 * its user interface windows and so on.
 * When the client is ready to accept frame updates and process
 * keyboard events then it sends the start-play packet.
 * This play packet advances the connection state into the
 * actively-playing state.  A player structure is allocated and
 * initialized and the other human players are told about this new player.
 * The newly started client is told about the already playing players and
 * play has begun.
 * Apart from these four states there are also two intermediate states.
 * These intermediate states are entered when the previous state
 * has filled the reliable data buffer and the client has not
 * acknowledged all the data yet that is in this reliable data buffer.
 * They are so called output drain states.  Not doing anything else
 * then waiting until the buffer is empty.
 * The difference between these two intermediate states is tricky.
 * The second intermediate state is entered after the
 * ready-but-not-playing-yet state and before the actively-playing state.
 * The difference being that in this second intermediate state the client
 * is already considered an active player by the rest of the server
 * but should not get frame updates yet until it has acknowledged its last
 * reliable data.
 *
 * Communication between the server and the clients is only done
 * using UDP datagrams.  The first client/serverized version of XPilot
 * was using TCP only, but this was too unplayable across the Internet,
 * because TCP is a data stream always sending the next byte.
 * If a packet gets lost then the server has to wait for a
 * timeout before a retransmission can occur.  This is too slow
 * for a real-time program like this game, which is more interested
 * in recent events than in sequenced/reliable events.
 * Therefore UDP is now used which gives more network control to the
 * program.
 * Because some data is considered crucial, like the names of
 * new players and so on, there also had to be a mechanism which
 * enabled reliable data transmission.  Here this is done by creating
 * a data stream which is piggybacked on top of the unreliable data
 * packets.  The client acknowledges this reliable data by sending
 * its byte position in the reliable data stream.  So if the client gets
 * a new reliable data packet and it has not had this data before and
 * there is also no data packet missing inbetween, then it advances
 * its byte position and acknowledges this new position to the server.
 * Otherwise it discards the packet and sends its old byte position
 * to the server meaning that it detected a packet loss.
 * The server maintains an acknowledgement timeout timer for each
 * connection so that it can retransmit a reliable data packet
 * if the acknowledgement timer expires.
 */



#define SERVER

#include "angband.h"
#include "netserver.h"

#define MAX_SELECT_FD			1023
/* #define MAX_RELIABLE_DATA_PACKET_SIZE	1024 */
#define MAX_RELIABLE_DATA_PACKET_SIZE	512

#define MAX_MOTD_CHUNK			512
#define MAX_MOTD_SIZE			(30*1024)
#define MAX_MOTD_LOOPS			120

connection_t	*Conn = NULL;
int		max_connections = 0;
static server_setup_t		Setup;
static int		(*playing_receive[256])(int ind),
				(*setup_receive[256])(int ind);
int			login_in_progress;
static int		num_logins, num_logouts;
static long		Id;
int			NumPlayers;

int		MetaSocket = -1;


char *showtime(void)
{
	time_t		now;
	struct tm	*tmp;
	static char	month_names[13][4] = {
				"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
				"Bug"
			};
	static char	buf[80];

	time(&now);
	tmp = localtime(&now);
	sprintf(buf, "%02d %s %02d:%02d:%02d",
		tmp->tm_mday, month_names[tmp->tm_mon],
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	return buf;
}


/*
 * Initialize the function dispatch tables for the various client
 * connection states.  Some states use the same table.
 */
static void Init_receive(void)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		playing_receive[i] = Receive_undefined;
		setup_receive[i] = Receive_undefined;
	}

	setup_receive[PKT_CHAR_INFO]	= Receive_char_info;
	setup_receive[PKT_PLAY]			= Receive_play;
	setup_receive[PKT_KEEPALIVE]	= Receive_keepalive;
	setup_receive[PKT_OPTIONS]		= Receive_options;
	setup_receive[PKT_VERIFY]		= Receive_verify_visual;
	setup_receive[PKT_MOTD]			= Receive_motd;
	setup_receive[PKT_QUIT]			= Receive_quit;
	
	playing_receive[PKT_MOTD]		= Receive_motd;	
	playing_receive[PKT_VERIFY]	= Receive_discard;
	playing_receive[PKT_QUIT]		= Receive_quit;
	playing_receive[PKT_PLAY]		= Receive_play;

	playing_receive[PKT_KEEPALIVE]		= Receive_keepalive;
	playing_receive[PKT_WALK]		= Receive_walk;
	playing_receive[PKT_RUN]		= Receive_run;
	playing_receive[PKT_TUNNEL]		= Receive_tunnel;
	playing_receive[PKT_AIM_WAND]		= Receive_aim_wand;
	playing_receive[PKT_DROP]		= Receive_drop;
	playing_receive[PKT_FIRE]		= Receive_fire;
	playing_receive[PKT_STAND]		= Receive_stand;
	playing_receive[PKT_DESTROY]		= Receive_destroy;
	playing_receive[PKT_LOOK]		= Receive_look;
	playing_receive[PKT_SPELL]		= Receive_spell;
    playing_receive[PKT_OBSERVE]	= Receive_observe;

	playing_receive[PKT_OPEN]		= Receive_open;
	playing_receive[PKT_PRAY]		= Receive_pray;
	playing_receive[PKT_QUAFF]		= Receive_quaff;
	playing_receive[PKT_READ]		= Receive_read;
	playing_receive[PKT_SEARCH]		= Receive_search;
	playing_receive[PKT_TAKE_OFF]		= Receive_take_off;
	playing_receive[PKT_USE]		= Receive_use;
	playing_receive[PKT_THROW]		= Receive_throw;
	playing_receive[PKT_WIELD]		= Receive_wield;
	playing_receive[PKT_ZAP]		= Receive_zap;

	playing_receive[PKT_TARGET]		= Receive_target;
	playing_receive[PKT_TARGET_FRIENDLY]	= Receive_target_friendly;
	playing_receive[PKT_INSCRIBE]		= Receive_inscribe;
	playing_receive[PKT_UNINSCRIBE]		= Receive_uninscribe;
	playing_receive[PKT_ACTIVATE]		= Receive_activate;
	playing_receive[PKT_BASH]		= Receive_bash;
	playing_receive[PKT_DISARM]		= Receive_disarm;
	playing_receive[PKT_EAT]		= Receive_eat;
	playing_receive[PKT_FILL]		= Receive_fill;
	playing_receive[PKT_LOCATE]		= Receive_locate;
	playing_receive[PKT_MAP]		= Receive_map;
	playing_receive[PKT_SEARCH_MODE]	= Receive_search_mode;

	playing_receive[PKT_CLOSE]		= Receive_close;
	playing_receive[PKT_GAIN]		= Receive_gain;
	playing_receive[PKT_DIRECTION]		= Receive_direction;
	playing_receive[PKT_GO_UP]		= Receive_go_up;
	playing_receive[PKT_GO_DOWN]		= Receive_go_down;
	playing_receive[PKT_MESSAGE]		= Receive_message;
	playing_receive[PKT_CHANNEL]		= Receive_channel;
	playing_receive[PKT_ITEM]		= Receive_item;
	playing_receive[PKT_PURCHASE]		= Receive_purchase;

	playing_receive[PKT_SELL]		= Receive_sell;
	playing_receive[PKT_STORE_LEAVE]	= Receive_store_leave;
	playing_receive[PKT_STORE_CONFIRM]	= Receive_store_confirm;
	playing_receive[PKT_DROP_GOLD]		= Receive_drop_gold;
	playing_receive[PKT_REDRAW]		= Receive_redraw;
	playing_receive[PKT_REST]		= Receive_rest;
	playing_receive[PKT_SPECIAL_LINE]	= 	Receive_special_line;
	playing_receive[PKT_SYMBOL_QUERY]	= 	Receive_symbol_query;
	playing_receive[PKT_PARTY]		= Receive_party;
	playing_receive[PKT_GHOST]		= Receive_ghost;

	playing_receive[PKT_SPIKE]		= Receive_spike;
	playing_receive[PKT_OPTIONS]		= Receive_options;
	playing_receive[PKT_SUICIDE]		= Receive_suicide;
	playing_receive[PKT_MASTER]		= Receive_master;

	playing_receive[PKT_STEAL]		= Receive_steal;
	playing_receive[PKT_COMMAND]		= Receive_custom_command;
	
	playing_receive[PKT_CLEAR] = Receive_clear;
	playing_receive[PKT_CHANGEPASS] = Receive_pass;
}

int Init_setup(void)
{
	int n = 0;
	char buf[1024];
	FILE *fp;

	Setup.frames_per_second = cfg_fps;
	Setup.motd_len = 23 * 80;
	Setup.min_col = SCREEN_WID / 3 + 1;
	Setup.min_row = SCREEN_HGT / 2 + 1;
	Setup.max_col = MAX_WID;
	Setup.max_row = MAX_HGT;

	path_build(buf, 1024, ANGBAND_DIR_TEXT, "news.txt");

	/* Open the news file */
	fp = my_fopen(buf, "r");

	if (fp)
	{
		/* Dump the file into the buffer */
		while (0 == my_fgets(fp, buf, 1024) && n < 23)
		{
			strncpy(&Setup.motd[n * 80], buf, 80);
			n++;
		}

		my_fclose(fp);
	}
	
	return 0;
}
	

/*
 * Talk to the metaserver.
 *
 * This function is called on startup, on death, and when the number of players
 * in the game changes.
 */
bool Report_to_meta(int flag)
{
	static sockbuf_t meta_buf;
	static char local_name[1024];
	static int init = 0;
	int bytes, i;
	char buf[1024], temp[100];
	bool hidden_dungeon_master = 0;

	/* Abort if the user doesn't want to report */
	if (!cfg_report_to_meta)
		return FALSE;

	/* If this is the first time called, initialize our hostname */
	if (!init)
	{
		/* Never do this again */
		init = 1;

		/* Get our hostname */
		if( cfg_report_address )
		{
			strncpy( local_name, cfg_report_address, 1024 );
		}
		else
		{
			if ( cfg_bind_name )
			{
				strncpy( local_name, cfg_bind_name, 1024 );
			}
			else
			{
				GetLocalHostName( local_name, 1024 );
			}
		}
		strcat(local_name, ":");
		sprintf(temp, "%d", (int) cfg_tcp_port);
		strcat(local_name, temp);
	}

	strcpy(buf, local_name);

	if (flag & META_START)
	{
		if ((MetaSocket = CreateDgramSocket(0)) == -1)
		{
			quit("Couldn't create meta-server Dgram socket\n");
		}

		if (SetSocketNonBlocking(MetaSocket, 1) == -1)
		{
			quit("Can't make socket non-blocking\n");
		}

		if (Sockbuf_init(&meta_buf, MetaSocket, SERVER_SEND_SIZE,
			SOCKBUF_READ | SOCKBUF_WRITE | SOCKBUF_DGRAM) == -1)
		{
			quit("No memory for sockbuf buffer\n");
		}

		Sockbuf_clear(&meta_buf);

		strcat(buf, " Number of players: 0  ");
	}

	else if (flag & META_DIE)
	{
		strcat(buf, " ");
	}

	else if (flag & META_UPDATE)
	{
	    hidden_dungeon_master=0;
		strcat(buf, " Number of players: ");

		/* Hack -- If cfg_secret_dungeon_master is enabled, determine
		 * if the DungeonMaster is playing, and if so, reduce the
		 * number of players reported.
		 */

		for (i = 1; i <= NumPlayers; i++)
		{
            if (Players[i]->dm_flags & DM_SECRET_PRESENCE) hidden_dungeon_master++;
		}

		/* tell the metaserver about everyone except hidden dungeon_masters */
		sprintf(temp, "%d ", NumPlayers - hidden_dungeon_master);
		strcat(buf, temp);

		/* if someone other than a dungeon master is playing */
		if (NumPlayers - hidden_dungeon_master)
		{
			strcat(buf, "Names: ");
			for (i = 1; i <= NumPlayers; i++)
			{
				/* handle the cfg_secret_dungeon_master option */
                if (Players[i]->dm_flags & DM_SECRET_PRESENCE) continue;
				strcat(buf, Players[i]->basename);
				strcat(buf, " ");
			}
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

	if (!(flag & META_DIE))
		strcat(buf, temp);

	/* If we haven't setup the meta connection yet, abort */
	if (MetaSocket == -1)
		return FALSE;

	Sockbuf_clear(&meta_buf);

	Packet_printf(&meta_buf, "%S", buf);

	if ((bytes = DgramSend(MetaSocket, cfg_meta_address, 8800, meta_buf.buf, meta_buf.len) == -1))
	{
		plog("Couldn't send info to meta-server!");
		return FALSE;
	}

	return TRUE;
}

	 
/*
 * Initialize the connection structures.
 */
int Setup_net_server(void)
{
	size_t size;

	Init_receive();

	if (Init_setup() == -1)
		return -1;

	/*
	 * The number of connections is limited by the number of bases
	 * and the max number of possible file descriptors to use in
	 * the select(2) call minus those for stdin, stdout, stderr,
	 * the contact socket, and the socket for the resolver library routines.
	 */

	max_connections = MAX_SELECT_FD - 5;
	size = max_connections * sizeof(*Conn);
	if ((Conn = (connection_t *) malloc(size)) == NULL)
		quit("Cannot allocate memory for connections");

	memset(Conn, 0, size);

	C_MAKE(Players, max_connections, player_type *);

	/* Tell the metaserver that we're starting up */
	Report_to_meta(META_START);

	plog(format("Server is running version %04x\n", SERVER_VERSION));

	return 0;
}

/* The contact socket */
static int Socket;
static sockbuf_t ibuf;

/* The contact socket now uses TCP.  This breaks backwards
 * compatibility, but is a good thing.
 */

void setup_contact_socket(void)
{
	plog("Create TCP socket..."); 
	while ((Socket = CreateServerSocket(cfg_tcp_port)) == -1)
	{
#ifdef WINDOWS
  Sleep(1);
#else
		sleep(1);
#endif
	}
	plog("Set Non-Blocking..."); 
	if (SetSocketNonBlocking(Socket, 1) == -1)
	{
		plog("Can't make contact socket non-blocking");
	}
	if (SocketLinger(Socket) == -1)
	{
		plog("Couldn't set SO_LINGER on the socket");
	}

	if (Sockbuf_init(&ibuf, Socket, SERVER_SEND_SIZE,
		SOCKBUF_READ | SOCKBUF_WRITE ) == -1)
	{
		quit("No memory for contact buffer");
	}

	install_input(Contact, Socket, 0);
}

static int Reply(char *host_addr, int fd)
{
	int result;

	if ((result = DgramWrite(fd, ibuf.buf, ibuf.len)) == -1)
	{
		GetSocketError(ibuf.sock);
	}	

	return result;
}

static int Limit_connections(char *nick_name, char *real_name, char *host_name, char *addr)
{
	player_type *p_ptr;
	connection_t *connp;
	int i;

	for (i = 0; i < max_connections; i++)
	{
		connp = &Conn[i];

		if (connp->state == CONN_FREE)
					continue;
		if (connp->state == CONN_CONSOLE)
					continue;

		if (strcasecmp(connp->nick, nick_name) == 0)
		{
			Destroy_connection(i, "resume connection");
			return SUCCESS;
		}
	}

	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr = Players[i];
		if (strcasecmp(p_ptr->name, nick_name) == 0)
		{
			/*plog(format("%s %s", Players[i]->name, nick_name));*/

			/* The following code allows you to "override" an
			 * existing connection by connecting again  -Crimson */

			 /* If uncommented, this code can tighten up the security
			  * by checking if the IP, username and hostname match. 
			  * Surpassed by real password check.
			if ((!strcasecmp(p_ptr->realname, real_name)) 
					&& (!strcasecmp(p_ptr->addr, addr))
					&& (!strcasecmp(p_ptr->hostname, host_name))
			){ 	// return E_IN_USE;
			*/
				Destroy_connection(p_ptr->conn, "resume connection");
				return SUCCESS;
		}

		/* All restrictions on the number of allowed players from one IP have 
		* been removed at this time. -APD
		*
		* and put back
		* -- Crimson
		*/
#ifdef LIMIT_PLAYER_CONNECTIONS

		if (
            // !strcasecmp(Players[i]->realname, real_name) &&
			!strcasecmp(Players[i]->addr, addr) && 
            // !strcasecmp(Players[i]->hostname, host_name) &&
             strcasecmp(nick_name, cfg_dungeon_master) &&
		)
		{
			return E_TWO_PLAYERS;
		}
#endif
	}

	return SUCCESS;
}
static int Check_names(char *nick_name, char *real_name, char *host_name, char *addr)
{
	char *ptr;

	if (real_name[0] == 0 || host_name[0] == 0 || nick_name[0] < 'A' ||
		nick_name[0] > 'Z')
		return E_INVAL;

	/* Any wierd characters here, bail out.  We allow letters, numbers and space */
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if ( (*ptr == 32) || ((*ptr >= 97) && (*ptr <= 122)) || ((*ptr >= 65) && (*ptr <= 90))
		|| ((*ptr >= 48) && (*ptr <= 57)) )
		{
			/* ok */
		} else {
			return E_INVAL;
		}
	}
	
	for (ptr = &nick_name[strlen(nick_name)]; ptr-- > nick_name; )
	{
		if (isascii(*ptr) && isspace(*ptr))
			*ptr = '\0';
		else break;
	}

	return SUCCESS;
}
static void Contact_cancel(int fd, char *reason)
{
	plog(reason);
	remove_input(fd);
	close(fd);
}	
static void Contact(int fd, int arg)
{
	int bytes, newsock;
	int race, class, sex , ret;
	bool need_info = FALSE;
	u16b version = 0;
	u16b conntype = 0;
	unsigned int len;
	char	status,
		real_name[MAX_CHARS],
		nick_name[MAX_CHARS],
		host_name[MAX_CHARS],
		pass_word[MAX_CHARS],
		host_addr[24];
    struct sockaddr_in sin;
    
	/* Create a TCP socket for communication with whoever contacted us */
	/* Hack -- check if this data has arrived on the contact socket or not.
	 * If it has, then we have not created a connection with the client yet, 
	 * and so we must do so.
	 */

	if (fd == Socket)
	{
		if ((newsock = SocketAccept(fd)) == -1)
		{
			/* We couldn't accept the socket connection. This is bad because we can't
			 * handle this situation correctly yet.  For the moment, we just log the
			 * error and quit */
			plog(format("Could not accept TCP Connection, socket error = %d",errno));
			quit("Couldn't accept TCP connection.");
		}
		install_input(Contact, newsock, 2);

		return;
	}

	/*
	 * Someone connected to us, now try and decipher the message
	 */
	Sockbuf_clear(&ibuf);
	if ((bytes = DgramReceiveAny(fd, ibuf.buf, ibuf.size)) <= 1)
	{
		/* If 0 bytes have been sent than the client has probably closed
		 * the connection
		 */
		if (bytes == 0)
		{
			remove_input(fd);
		}
		/* On Windows we may get a socket error without errno being set */
		else if (bytes < 0 && errno == 0)
		{
			remove_input(fd);
		}
		else if (bytes < 0 && errno != EWOULDBLOCK && errno != EAGAIN &&
			errno != EINTR)
		{
			/* Clear the error condition for the contact socket */
			GetSocketError(fd);
		}
		return;
	}
	ibuf.len = bytes;

	/* Get the IP address of the client, without using the broken DgramLastAddr() */
	len = sizeof sin;
	if (getpeername(fd, (struct sockaddr *) &sin, &len) >= 0)
		strcpy(host_addr, inet_ntoa(sin.sin_addr));  

	/* Read first data he sent us -- connection type */
	if (Packet_scanf(&ibuf, "%hu", &conntype) <= 0)
	{
		plog(format("Incomplete handshake from %s", host_addr));
		remove_input(fd);
		close(fd);
		return;
	}
	
	/* Convert connection type */
	conntype = connection_type_ok(conntype);

	/* For console, switch routines */
	if(conntype == CONNTYPE_CONSOLE)
	{
		/* Hack -- check local access */
		if (cfg_console_local_only && sin.sin_addr.s_addr != htonl(INADDR_LOOPBACK))
		{
			Contact_cancel(fd, format("Non-local console attempt from %s", host_addr));
			return;
		}
		/* Try moving to console handlers */	
		if ((ret = Setup_connection(0, 0, host_addr, 0, 0,	FALSE, 0, 0, 0, conntype, 0, fd)) > -1)
			NewConsole(fd, -(ret+1) );
		else
			Contact_cancel(fd, format("Unable to setup console connection for %s", host_addr));
		return;
	}
	/* For players - continue, otherwise - abort */
	else if (conntype != CONNTYPE_PLAYER)
	{
		Contact_cancel(fd, format("Invalid connection type requested from %s", host_addr));
		return;			
	}
		
	/* Read next data he sent us -- client version */
	if (Packet_scanf(&ibuf, "%hu", &version) <= 0)
	{
		Contact_cancel(fd, format("Incompatible version packet from %s", host_addr));
		return;
	}
	
	if (!client_version_ok(version)) 
	{
		status = E_VERSION;
	}
	else
	{
		/* His version was correct and he's a player. Let's try to read the string */
		if (Packet_scanf(&ibuf, "%s%s%s%s", real_name, host_name, nick_name, pass_word) <= 0)
		{
			Contact_cancel(fd, format("Incomplete handshake from %s", host_addr));
			return;
		}
	
		/* Paranoia */
		nick_name[sizeof(nick_name) - 1] = '\0';
		host_name[sizeof(host_name) - 1] = '\0';
		real_name[sizeof(real_name) - 1] = '\0';
		pass_word[sizeof(pass_word) - 1] = '\0';
		race = class = sex = 0;
			
#if 0
		plog(format("Received contact from %s:%d.\n", host_name, port);
		plog(format("Address: %s.\n", host_addr);
		plog(format("Info: real_name %s, port %hu, nick %s, host %s, version %hu\n", real_name, port, nick_name, host_name, version);  
#endif

		/* Check if his names are valid */
		if ( (status = Check_names(nick_name, real_name, host_name, host_addr)) )
		{
			status = E_INVAL;
		}
		/* Check if character with this name exists */
		else if (!lookup_player_id(nick_name))
		{ 
			status = E_NEED_INFO;
		}
		/* Survived those two tests? */ 
		if (!status)
		{
			/* Test if his password is matching */
			ret = scoop_player(nick_name, pass_word, &race, &class, &sex);

			if (ret == -2)					status = E_BAD_PASS; 	/* Incorrect Password */
			if (ret == -1)					status = E_READ_ERR;		/* Technical Error */
			if (ret == 0)					status = SUCCESS;			/* Real char exists */
			if (ret > 0)					status = E_NEED_INFO;	/* Nickname exists, but not real char OR New player.. whatever */
		}
	} 
	
	/* Save for later */
	if (status == E_NEED_INFO) 
	{
		need_info = TRUE;
		status = SUCCESS;
	}
	
	/* Let's see if he's allready connected */
	if (!status)
	{
		status = Limit_connections(nick_name, real_name, host_name, host_addr);
	}

	/* SETUP THE CONNECTION ! */
	if (!status)
	{
		if (NumPlayers >= MAX_SELECT_FD)
			status = E_GAME_FULL;
			
		else if ((ret = Setup_connection(real_name, nick_name, host_addr, host_name, pass_word, need_info, race, class, sex, conntype, version, fd)) == -1) 
			status = E_SOCKET;

		if (ret == -2)
			status = E_GAME_FULL;

		/* Log the players connection */			
		if (ret != -1)
			plog(format("Welcome %s=%s@%s (%s) (version %04x)", nick_name,	real_name, host_name, host_addr, version));
	}
	
	/* Clear buffer */
	Sockbuf_clear(&ibuf);

	/* Send reply */
	Packet_printf(&ibuf, "%c", status);

	Reply(host_addr, fd);
}
		
static void Conn_set_state(connection_t *connp, int state, int drain_state)
{
	static int num_conn_busy;
	static int num_conn_playing;

	if ((connp->state & (CONN_PLAYING | CONN_READY)) != 0)
		num_conn_playing--;
	else if (connp->state == CONN_FREE)
		num_conn_busy++;

	connp->state = state;
	connp->start = turn;

	if (connp->state == CONN_PLAYING)
	{
		num_conn_playing++;
		connp->timeout = IDLE_TIMEOUT;
	}
	else if (connp->state == CONN_SETUP)
		connp->timeout = SETUP_TIMEOUT;
	else if (connp->state == CONN_FREE)
	{
		num_conn_busy--;
		connp->timeout = IDLE_TIMEOUT;
	}
	login_in_progress = num_conn_busy - num_conn_playing;
}


/*
 * Delete a player's information and save his game
 */
static void Delete_player(int Ind)
{
	player_type *p_ptr = Players[Ind];
	char buf[255];

	/* Be paranoid */
	if (cave[p_ptr->dun_depth])
	{
		/* There's nobody on this space anymore */
		cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0;

		/* Forget his lite and viewing area */
		forget_lite(Ind);
		forget_view(Ind);

		/* Show everyone his disappearance */
		everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);
	}
	
	/* Leave chat channels */
	channels_leave(Ind);

	/* Try to save his character */
	save_player(Ind);

	/* If he was actively playing, tell everyone that he's left */
	/* handle the cfg_secret_dungeon_master option */
	if (p_ptr->alive && !p_ptr->death && 
	    !(p_ptr->dm_flags & DM_SECRET_PRESENCE))
	{
		sprintf(buf, "%s has left the game.", p_ptr->name);
		msg_broadcast(Ind, buf);
	}

	/* Swap entry number 'Ind' with the last one */
	/* Also, update the "player_index" on the cave grids */
	if (Ind != NumPlayers)
	{
		p_ptr			= Players[NumPlayers];
		if (cave[p_ptr->dun_depth])
			cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0 - Ind;
		Players[NumPlayers]	= Players[Ind];
		Players[Ind]		= p_ptr;
		p_ptr			= Players[NumPlayers];
	}

	GetInd[Conn[Players[Ind]->conn].id] = Ind;
	GetInd[Conn[Players[NumPlayers]->conn].id] = NumPlayers;

	/* Recalculate player-player visibility */
	update_players();

	if (p_ptr)
	{
		if (p_ptr->inventory)
			C_KILL(p_ptr->inventory, INVEN_TOTAL, object_type);

		KILL(p_ptr, player_type);
	}

	/* Clear the player slot previously used */
	Players[NumPlayers] = NULL;

	/* Update the number of players */
	NumPlayers--;

	/* Tell the metaserver about the loss of a player */	
	Report_to_meta(META_UPDATE);
}


/*
 * Cleanup a connection.  The client may not know yet that it is thrown out of
 * the game so we send it a quit packet if our connection to it has not already
 * closed.  If our connection to it has been closed, then connp->w.sock will
 * be set to -1.
 */
bool Destroy_connection(int ind, char *reason)
{
	connection_t		*connp = &Conn[ind];
	int			id, len, sock;
	char			pkt[MAX_CHARS];

/* 	if(ind ==0 ) { return TRUE; }; */

	if (connp->state == CONN_FREE)
	{
		errno = 0;
		plog(format("Cannot destroy empty connection (\"%s\")", reason));
		return TRUE;
	}

	sock = connp->w.sock;
	if (sock != -1)
	{
		remove_input(sock);
	}

	if (connp->conntype == CONNTYPE_PLAYER) 
	{
		strncpy(&pkt[1], reason, sizeof(pkt) - 3);
		pkt[sizeof(pkt) - 2] = '\0';
		pkt[0] = PKT_QUIT;
		len = strlen(pkt) + 2;
		pkt[len - 1] = PKT_END;
		pkt[len] = '\0';
		/*len++;*/
		if (sock != -1)
		{
			if (DgramWrite(sock, pkt, len) != len)
			{
				GetSocketError(sock);
				DgramWrite(sock, pkt, len);
			}
		}
		
		plog(format("Goodbye %s=%s@%s (\"%s\")",
			connp->nick ? connp->nick : "",
			connp->real ? connp->real : "",
			connp->host ? connp->host : "",
			reason));
	}

	Conn_set_state(connp, CONN_FREE, CONN_FREE);

	if (connp->id != -1)
	{
		id = connp->id;
		/*connp->id = -1;*/
		/*Players[GetInd[id]]->conn = NOT_CONNECTED;*/
		Delete_player(GetInd[id]);
	}
	if (connp->real != NULL)
		free(connp->real);
	if (connp->nick != NULL)
		free(connp->nick);
	if (connp->addr != NULL)
		free(connp->addr);
	if (connp->host != NULL)
		free(connp->host);
	Sockbuf_cleanup(&connp->w);
	Sockbuf_cleanup(&connp->r);
	Sockbuf_cleanup(&connp->c);
	Sockbuf_cleanup(&connp->q);
	memset(connp, 0, sizeof(*connp));

	num_logouts++;

	if (sock != -1)
	{
		DgramClose(sock);
	}

	return TRUE;
}

int Check_connection(char *real, char *nick, char *addr)
{
	int i;
	connection_t *connp;

	for (i = 0; i < max_connections; i++)
	{
		connp = &Conn[i];
		if (connp->state == CONN_SETUP)
			if (strcasecmp(connp->nick, nick) == 0)
			{
				if (!strcmp(real, connp->real)
					&& !strcmp(addr, connp->addr))
						return 1;
				return -1;
			}
	}
	return -1;
}


/*
 * After a TCP "Contact" was made we shall see if we have
 * room for more connections and create one.
 */
int Setup_connection(char *real, char *nick, char *addr, char *host, char *pass,
			bool need_info, int race, int class, int sex, u16b conntype, unsigned version, int fd)
{
	int i, free_conn_index = max_connections, my_port, sock;
	connection_t *connp;
	bool memory_error = FALSE;

	for (i = 0; i < max_connections; i++)
	{
		connp = &Conn[i];
		if (connp->state == CONN_FREE)
		{
			if (free_conn_index == max_connections)
				free_conn_index = i;
			continue;
		}
		if (connp->state == CONN_CONSOLE || conntype == CONNTYPE_CONSOLE)
			continue;

		if (strcasecmp(connp->nick, nick) == 0)
		{
			if (connp->state == CONN_SETUP
				&& strcmp(real, connp->real) == 0
				&& version == connp->version)
					return 1;
			else return -1;
		}
	}

	if (free_conn_index >= max_connections)
	{
		plog(format("Full house for %s(%s)@%s", real, nick, host));
		return -2;
	}
	connp = &Conn[free_conn_index];

	// A TCP connection already exists with the client, use it.
	sock = fd;

	if ((my_port = GetPortNum(sock)) == 0)
	{
		plog("Cannot get port from socket");
		DgramClose(sock);
		return -1;
	}
	if (SetSocketNonBlocking(sock, 1) == -1)
	{
		plog("Cannot make client socket non-blocking");
	}
	if (SetSocketNoDelay(Socket, 1) == -1)
	{
		plog("Can't set TCP_NODELAY on the socket");
	}
	if (SocketLinger(sock) == -1)
	{
		plog("Couldn't set SO_LINGER on the socket");
	}
	if (SetSocketReceiveBufferSize(sock, SERVER_RECV_SIZE + 256) == -1)
		plog(format("Cannot set receive buffer size to %d", SERVER_RECV_SIZE + 256));
	if (SetSocketSendBufferSize(sock, SERVER_SEND_SIZE + 256) == -1)
		plog(format("Cannot set send buffer size to %d", SERVER_SEND_SIZE + 256));

	Sockbuf_init(&connp->w, sock, SERVER_SEND_SIZE, SOCKBUF_WRITE);
	Sockbuf_init(&connp->r, sock, SERVER_RECV_SIZE, SOCKBUF_WRITE | SOCKBUF_READ);
	Sockbuf_init(&connp->c, -1, MAX_SOCKBUF_SIZE, SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK);
	Sockbuf_init(&connp->q, -1, MAX_SOCKBUF_SIZE, SOCKBUF_WRITE | SOCKBUF_READ | SOCKBUF_LOCK);

	connp->id = -1;
	connp->conntype = conntype;
	connp->addr = strdup(addr);

	if (connp->w.buf == NULL || connp->r.buf == NULL || connp->c.buf == NULL || connp->q.buf == NULL || connp->addr == NULL)
		memory_error = TRUE;
		
	if (conntype == CONNTYPE_PLAYER)
	{
		connp->real = strdup(real);
		connp->nick = strdup(nick);
		connp->host = strdup(host);
		connp->pass = strdup(pass);
		connp->version = version;
		connp->char_state = ( need_info ? 0 : 1 );
		connp->race = race;
		connp->class = class;
		connp->sex = sex;
		connp->start = turn;
	
		connp->timeout = SETUP_TIMEOUT;		
		
		if (connp->real == NULL || connp->nick == NULL || connp->pass == NULL || connp->host == NULL)
			memory_error = TRUE;		
	}
	
	if (memory_error)	
	{
		plog("Not enough memory for connection");
		Destroy_connection(free_conn_index, "no memory");
		return -1;
	}

	if (conntype == CONNTYPE_CONSOLE)
	{
		connp->console_authenticated = FALSE;
		connp->console_listen = FALSE;

		Conn_set_state(connp, CONN_CONSOLE, CONN_CONSOLE);
	}
	
	// Non-players leave now
	if (conntype != CONNTYPE_PLAYER)
		return free_conn_index;	
		
	Conn_set_state(connp, CONN_SETUP, CONN_SETUP);

	// Remove the contact input handler
	remove_input(sock);
	// Install the game input handler
	install_input(Handle_input, sock, free_conn_index);

	return free_conn_index;
}


int console_buffer(int ind, bool read)
{
	connection_t		*connp = &Conn[ind];
	if (read) return (int)&connp->r;
	return (int)&connp->w;
}
bool Conn_is_alive(int ind)
{
	connection_t		*connp = &Conn[ind];
	if (!connp) return FALSE;
	if (connp->state != CONN_CONSOLE) return FALSE;
	return TRUE;
}
bool Conn_get_console_setting(int ind, int set)
{
	connection_t		*connp = &Conn[ind];
	if (set) return (bool)connp->console_authenticated;
	return (bool)connp->console_listen;
}
void Conn_set_console_setting(int ind, int set, bool val)
{
	connection_t		*connp = &Conn[ind];
	if (set) connp->console_authenticated = val;
	else connp->console_listen = val;
}

	


/*
 * Sync the named options from the array of options.
 *
 * This is a crappy way of doing things....
 */
static void sync_options(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Do the dirty work */
	p_ptr->carry_query_flag = p_ptr->options[3];
	p_ptr->use_old_target = p_ptr->options[4];
	p_ptr->always_pickup = p_ptr->options[5];
	p_ptr->stack_force_notes = p_ptr->options[8];
	p_ptr->stack_force_costs = p_ptr->options[9];
	p_ptr->find_ignore_stairs = p_ptr->options[16];
	p_ptr->find_ignore_doors = p_ptr->options[17];
	p_ptr->find_cut = p_ptr->options[18];
	p_ptr->find_examine = p_ptr->options[19];
	p_ptr->disturb_move = p_ptr->options[20];
	p_ptr->disturb_near = p_ptr->options[21];
	p_ptr->disturb_panel = p_ptr->options[22];
	p_ptr->disturb_state = p_ptr->options[23];
	p_ptr->disturb_minor = p_ptr->options[24];
	p_ptr->disturb_other = p_ptr->options[25];
	p_ptr->stack_allow_items = p_ptr->options[30];
	p_ptr->stack_allow_wands = p_ptr->options[31];
	p_ptr->view_perma_grids = p_ptr->options[34];
	p_ptr->view_torch_grids = p_ptr->options[35];
	p_ptr->view_reduce_lite = p_ptr->options[44];
	p_ptr->view_reduce_view = p_ptr->options[45];
	p_ptr->view_yellow_lite = p_ptr->options[56];
	p_ptr->view_bright_lite = p_ptr->options[57];
	p_ptr->view_granite_lite = p_ptr->options[58];
	p_ptr->view_special_lite = p_ptr->options[59];
	
	/* Read the "no ghost" option, allow it to be changed at level 1 */
	if (p_ptr->lev <= 1)
	{
		/* Get the clients setting */
		p_ptr->no_ghost = p_ptr->options[28];
	}
	
}
/* Same as above, but for settings
 * Yeah, still kinda crappy */
static int sync_settings(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];
	player_type *p_ptr = Players[ind];

	/* Resize */	
	if (connp->Client_setup.settings[1] != p_ptr->screen_wid ||  connp->Client_setup.settings[2] != p_ptr->screen_hgt)
	{
		p_ptr->screen_wid = connp->Client_setup.settings[1];
		p_ptr->screen_hgt = connp->Client_setup.settings[2];
		
		if (p_ptr->screen_hgt < Setup.min_row || p_ptr->screen_hgt > Setup.max_row || 
		    p_ptr->screen_wid < Setup.min_col || p_ptr->screen_wid > Setup.max_col)
		{	
			errno = 0;
			Destroy_connection(p_ptr->conn, "Incompatible screen size");
			return -1;
		}

		setup_panel(ind, TRUE);
		verify_panel(ind);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);
		
		/* Very important - ack term resize */
		Send_term_resize_ack(ind);
	}
	
	return 1;
}
/*
 * A client has requested to start active play.
 * See if we can allocate a player structure for it
 * and if this succeeds update the player information
 * to all connected players.
 */
static int Enter_player(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	int i;
	char buf[255];

	if (Id >= MAX_ID)
	{
		errno = 0;
		plog(format("Id too big (%d)", Id));
		return -2;
	}

	for (i = 1; i < NumPlayers + 1; i++)
	{
		if (strcasecmp(Players[i]->name, connp->nick) == 0)
		{
			errno = 0;
			plog(format("Name already in use %s", connp->nick));
			Destroy_connection(ind, "not login"); 
			return -1;
		}
	}

	/* XXX - HACK - Ensure his settings are allowed, disconnect otherwise */
	if (connp->Client_setup.settings[1] < Setup.min_col || connp->Client_setup.settings[1] > Setup.max_col || 
	    connp->Client_setup.settings[2] < Setup.min_row || connp->Client_setup.settings[2] > Setup.max_row)
	{
		Destroy_connection(ind, format("Incompatible screen size %dx%d (min %dx%d, max %dx%d).", 
		connp->Client_setup.settings[1], connp->Client_setup.settings[2], Setup.min_col, Setup.min_row, Setup.max_col, Setup.max_row)); 
		return -1;
	}

	if (!player_birth(NumPlayers + 1, connp->nick, connp->pass, ind, connp->race, connp->class, connp->sex, connp->stat_order))
	{
		/* Failed, connection destroyed */
		return -2;
	}

	p_ptr = Players[NumPlayers + 1];
	strcpy(p_ptr->realname, connp->real);
	strcpy(p_ptr->hostname, connp->host);
	strcpy(p_ptr->addr, connp->addr);
	p_ptr->version = connp->version;

	/* temporary hack to turn on maximize for everybody */
	p_ptr->maximize=1;
	
	/* Initialise message ptr before we start sending messages */
	p_ptr->msg_hist_ptr = 0;


	/* Copy the client preferences to the player struct */
	for (i = 0; i < 64; i++)
	{
		p_ptr->options[i] = connp->Client_setup.options[i];
	}

	for (i = 0; i < MAX_FLVR_IDX; i++) 
	{ 
		if (!connp->Client_setup.flvr_x_attr[i]) connp->Client_setup.flvr_x_attr[i] = flavor_info[i].x_attr;
		if (!connp->Client_setup.flvr_x_char[i]) connp->Client_setup.flvr_x_char[i] = flavor_info[i].x_char;
	}

	for (i = 0; i < MAX_F_IDX; i++)
	{
		p_ptr->f_attr[i] = connp->Client_setup.f_attr[i];
		p_ptr->f_char[i] = connp->Client_setup.f_char[i];

		if (!p_ptr->f_attr[i]) p_ptr->f_attr[i] = f_info[i].z_attr;
		if (!p_ptr->f_char[i]) p_ptr->f_char[i] = f_info[i].z_char;
	}

	for (i = 0; i < MAX_K_IDX; i++)
	{
		p_ptr->k_attr[i] = connp->Client_setup.k_attr[i];
		p_ptr->k_char[i] = connp->Client_setup.k_char[i];

		if (!p_ptr->k_attr[i]) p_ptr->k_attr[i] = (k_info[i].flavor ? connp->Client_setup.flvr_x_attr[k_info[i].flavor]: k_info[i].x_attr);
		if (!p_ptr->k_char[i]) p_ptr->k_char[i] = (k_info[i].flavor ? connp->Client_setup.flvr_x_char[k_info[i].flavor]: k_info[i].x_char);
		
		if (!p_ptr->d_attr[i]) p_ptr->d_attr[i] = (k_info[i].flavor ? connp->Client_setup.flvr_x_attr[k_info[i].flavor]: k_info[i].d_attr);
		if (!p_ptr->d_char[i]) p_ptr->d_char[i] = (k_info[i].flavor ? connp->Client_setup.flvr_x_char[k_info[i].flavor]: k_info[i].d_char);
	}

	for (i = 0; i < MAX_R_IDX; i++)
	{
		p_ptr->r_attr[i] = connp->Client_setup.r_attr[i];
		p_ptr->r_char[i] = connp->Client_setup.r_char[i];

		if (!p_ptr->r_attr[i]) p_ptr->r_attr[i] = r_info[i].x_attr;
		if (!p_ptr->r_char[i]) p_ptr->r_char[i] = r_info[i].x_char;
	}
	
	for (i = 0; i < 128; i++) 
	{
		p_ptr->tval_attr[i] = connp->Client_setup.tval_attr[i];
		p_ptr->tval_char[i] = connp->Client_setup.tval_char[i];
				
		if (!p_ptr->tval_attr[i]) p_ptr->tval_attr[i] = tval_to_attr[i]; 
		if (!p_ptr->tval_char[i]) p_ptr->tval_char[i] = tval_to_char[i];
	}
	
	for (i = 0; i < 256; i++) 
	{
		p_ptr->misc_attr[i] = connp->Client_setup.misc_attr[i];
		p_ptr->misc_char[i] = connp->Client_setup.misc_char[i];
	}

	/* Hack -- graphic option */
	p_ptr->use_graphics = connp->Client_setup.settings[0];

	/* Hack -- dungeon screen size */
	p_ptr->screen_wid = connp->Client_setup.settings[1];
	p_ptr->screen_hgt = connp->Client_setup.settings[2];
	
	setup_panel(NumPlayers + 1, TRUE);

	sync_options(NumPlayers + 1);

	GetInd[Id] = NumPlayers + 1;

	NumPlayers++;
	connp->id = Id++;
	
	Send_game_start_conn(ind);	

	//Conn_set_state(connp, CONN_READY, CONN_PLAYING);
	Conn_set_state(connp, CONN_PLAYING, CONN_PLAYING);

#ifdef COMMAND_OVERLOAD
	/* Send custom commands */
	Send_custom_commands(NumPlayers);
#endif
	/* Send party information */
	Send_party(NumPlayers);
	
	/* Send channel */
	Send_channel(NumPlayers, 0, NULL);

	/* Send him his history */
	prt_history(NumPlayers);	
	
	/* Hack -- terminate the data stream sent to the client 
	if (Packet_printf(&connp->c, "%c", PKT_END) <= 0)
	{
		Destroy_connection(p_ptr->conn, "write error");
		return -1;
	}

	if (Send_reliable(ind) == -1)
	{
		Destroy_connection(ind, "Couldn't send prepared data");
		return -1;
	}*/ 
	

	num_logins++;

	/* Handle the cfg_secret_dungeon_master option */
	if (p_ptr->dm_flags & DM_SECRET_PRESENCE) return 0;

	/* Tell everyone about our new player */
	if(p_ptr->exp == 0)
	{
		sprintf(buf, "%s begins a new game.", p_ptr->name);
	}
	else 
	{
		sprintf(buf, "%s has entered the game.", p_ptr->name);
	}
	
	msg_broadcast(i, buf);
	
	/* Tell the meta server about the new player */
	Report_to_meta(META_UPDATE);

	return 0;
}

/* Actually execute commands from the client command queue 
 * (return TRUE incase there was an error)
 */
int process_pending_commands(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;	
	int player, type, result, (**receive_tbl)(int ind) = playing_receive, old_energy = 0;
	int last_pos, data_advance = 0; // Hack to buffer data in SETUP state
	int num_players_start = NumPlayers; // Hack to see if we have quit in this function

	if (connp->state & (CONN_PLAYING | CONN_READY))
		receive_tbl = &playing_receive[0];
	else if (connp->state & CONN_SETUP)
		receive_tbl = &setup_receive[0];
	
	// Hack -- take any pending commands from the command que connp->q
	// and move them to connp->r, where the Receive functions get their
	// data from.
	Sockbuf_clear(&connp->r);
	if (connp->q.len > 0)
	{
		if (Sockbuf_write(&connp->r, connp->q.ptr, connp->q.len) != connp->q.len)
		{
			errno = 0;
			Destroy_connection(ind, "Can't copy queued data to buffer");
			return TRUE;
		}
		//connp->q.ptr += connp->q.len;
		//Sockbuf_advance(&connp->q, connp->q.ptr - connp->q.buf);
		Sockbuf_clear(&connp->q);
	}

	// If we have no commands to execute return
	if (connp->r.len <= 0)
		return FALSE;

	// Get the player pointer
	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	// Hack -- if our player id has not been set then do WITHOUT player
	else
	{
		while ((connp->r.ptr < connp->r.buf + connp->r.len))
		{
			/* Store all data for future, incase a command reports it lacks bytes! */
			if (Sockbuf_write(&connp->q, connp->r.ptr, (connp->r.len - data_advance)) !=  (connp->r.len - data_advance))
			{
				errno = 0;
				Destroy_connection(ind, "Can't copy read data to queue buffer");
				return TRUE;
			}
			last_pos = connp->r.ptr;
			type = (connp->r.ptr[0] & 0xFF);
			result = (*receive_tbl[type])(ind);
			if ((connp->r.ptr - last_pos) > 0) 
				data_advance += (int)(connp->r.ptr - last_pos);
			connp->start = turn;
			if (result == 0)
				return TRUE;
			Sockbuf_clear(&connp->q);
			if (result == -1)
				return TRUE;
		}
		return FALSE;
	}

	// Attempt to execute every pending command. Any command that fails due
	// to lack of energy will be put into the queue for next turn by the
	// respective receive function. 		

	while ((connp->r.ptr < connp->r.buf + connp->r.len))
	{
		/* Make sure we don't do anything on an unallocated level. */
		if (p_ptr->new_level_flag) return FALSE;

		type = (connp->r.ptr[0] & 0xFF);
		result = (*receive_tbl[type])(ind);
		if (connp->state == CONN_PLAYING)
		{
			connp->start = turn;
		}
		if (result == -1)
			return TRUE;

		// We didn't have enough energy to execute an important command.
		if (result == 0) 
		{
			/* Hack -- if we tried to do something while resting, wake us up.
			 */
			if (p_ptr->resting) disturb(player, 0, 0);

			/* If we didn't have enough energy to execute this
			 * command, in order to ensure that our important
			 * commands execute in the proper order, stop
			 * processing any commands that require energy. We
			 * assume that any commands that don't require energy
			 * (such as quitting, or talking) should be executed
			 * ASAP.
			 */
			/* Mega-Hack -- save our old energy and set our energy
			 * to 0.  This will allow us to execute "out of game"
			 * actions such as talking while we wait for enough
			 * energy to execute our next queued in game action.
			 */
			if (p_ptr->energy) 
			{
				old_energy = p_ptr->energy;
				p_ptr->energy = 0;
			}
		}
	}
	/* Restore our energy if neccecary. */

	/* Make sure that the player structure hasn't been deallocated in this
	 * time due to a quit request.  Mega-Hack : to do this we check if the number
	 * of players has changed while this loop has been executing.  This would be
	 * a BAD thing to do if we ever went multithreaded.
	 */
	if (NumPlayers == num_players_start)
		if (!p_ptr->energy) p_ptr->energy = old_energy;

	return FALSE;

}

/*
 * Process a client packet.
 * The client may be in one of several states,
 * therefore we use function dispatch tables for easy processing.
 * Some functions may process requests from clients being
 * in different states.
 * The behavior of this function has been changed somewhat.  New commands are now
 * put into a command queue, where they will be executed later.
 */
void Handle_input(int fd, int arg)
{
	int ind = arg, player, old_numplayers = NumPlayers;
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	int (**receive_tbl)(int ind);

	if (connp->state & (CONN_PLAYING | CONN_READY))
		receive_tbl = &playing_receive[0];
	else if (connp->state == CONN_SETUP)
		receive_tbl = &setup_receive[0];
	else {
		if (connp->state != CONN_FREE)
			Destroy_connection(ind, "not input");
		return;
	}

#if 0
	/* Clear connp->r, which will be our new queue */
	Sockbuf_clear(&connp->r);
	
	/* Put any old commands at the beginning of the new queue we are reading into */
	if (connp->q.len > 0)
	{
		if (connp->r.ptr > connp->r.buf)
			Sockbuf_advance(&connp->r, connp->r.ptr - connp->r.buf);
		if (Sockbuf_write(&connp->r, connp->q.ptr, connp->q.len) != connp->q.len)
		{
			errno = 0;
			Destroy_connection(ind, "Can't copy queued data to buffer");
			return;
		}

		connp->q.ptr += connp->q.len;
		Sockbuf_advance(&connp->q, connp->q.ptr - connp->q.buf);
	}
	Sockbuf_clear(&connp->q);
#endif

	
	/* Mega-Hack */
	if (connp->id != -1 && Players[GetInd[connp->id]]->new_level_flag) return;

	// Reset the buffer we are reading into
	Sockbuf_clear(&connp->r);

	// Read in the data
	if (Sockbuf_read(&connp->r) <= 0)
	{
		// Check to make sure that an EAGAIN error didn't occur.  Sometimes on
		// Linux when receiving a lot of traffic EAGAIN will occur on recv and
		// Sockbuf_read will return 0.
#ifndef WINDOWS
		if (errno != EAGAIN)
		{
			// If this happens, the the client has probably closed his TCP connection.
			do_quit(ind, 0);
		}
#else
		/* On windows, we frequently get EWOULDBLOCK return codes, i.e.
		 * there is no data yet, but there may be in a moment. Without
		 * this check clients frequently get disconnected */
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK))
		{
			// If this happens, the the client has probably closed his TCP connection.
			do_quit(ind, 0);
		}
#endif
		
		//Destroy_connection(ind, "input error");
		return;
	}

	// Add this new data to the command queue
	if (Sockbuf_write(&connp->q, connp->r.ptr, connp->r.len) != connp->r.len)
	{
		errno = 0;
		Destroy_connection(ind, "Can't copy queued data to buffer");
		return;
	}

	// Execute any new commands immediatly if possobile
	process_pending_commands(ind);

	/* Experimental hack -- to reduce perceived latency, flush our network
	 * info right now so the player sees the results of his actions as soon
	 * as possobile.  Everyone else will see him move at most one game turn
	 * later, which is usually < 100 ms.
	 */

	/* Hack -- don't update the player info if the number of players since
	 * the beginning of this function call has changed, which might indicate
	 * that our player has left the game.
	 */
	if ((old_numplayers == NumPlayers) && (connp->state == CONN_PLAYING))
	{
		// Update the players display if neccecary and possobile
		if (connp->id != -1)
		{
			player = GetInd[connp->id];
			p_ptr = Players[player];

			/* Notice stuff */
			if (p_ptr->notice) notice_stuff(player);

			/* Update stuff */
			if (p_ptr->update) update_stuff(player);

			/* Redraw stuff */
			if (p_ptr->redraw) redraw_stuff(player);

			/* Window stuff */
			if (p_ptr->window) window_stuff(player);
		}
	}

 
	//PKT_END makes client pause with the net input and move to keyboard 
	//so it's important to apply it at the end
	if (connp->c.len > 0)
	{
		if (Packet_printf(&connp->c, "%c", PKT_END) <= 0)
		{
			//Destroy_connection(p_ptr->conn, "write error");
			Destroy_connection(ind, "write error");
			return;
		}
		Send_reliable(ind);
	}
	
//		Sockbuf_clear(&connp->r);
}

// This function is used for sending data to clients who do not yet have
// Player structures allocated, and for timing out players who have been
// idle for a while.
int Net_input(void)
{
	int i, ind, num_reliable = 0, input_reliable[MAX_SELECT_FD];
	connection_t *connp;
	char msg[MSG_LEN];

	for (i = 0; i < max_connections; i++)
	{
		connp = &Conn[i];

		if (connp->state == CONN_FREE)
			continue;
		if (connp->state == CONN_CONSOLE)
			continue;
		if (connp->start + connp->timeout * cfg_fps < turn)
		{
            /*if (connp->state & (CONN_PLAYING | CONN_READY))
			{
				sprintf(msg, "%s mysteriously disappeared!",
					connp->nick);
                Set_message(msg);
            }*/
			sprintf(msg, "timeout %02x", connp->state);
			Destroy_connection(i, msg);
			continue;
		}

		// Make sure that the player we are looking at is not already in the
		// game.  If he is already in the game then we will send him data
		// in the function Net_output.
		if (connp->id != -1) continue;

/*		if (connp->r.len > 0)
			Sockbuf_clear(&connp->r); */
#if 0
		if (connp->state != CONN_PLAYING)
		{
			input_reliable[num_reliable++] = i;
		}
#endif
	}

	for (i = 0; i < num_reliable; i++)
	{
		ind = input_reliable[i];
		connp = &Conn[ind];

		if (connp->state & (CONN_READY | CONN_SETUP | CONN_PLAYING))
		{
			if (connp->c.len > 0)
				if (Send_reliable(ind) == -1)
					continue;
		}
	}

	if (num_logins | num_logouts)
		num_logins = num_logouts = 0;

	return login_in_progress;
}

int Net_output(void)
{
	int	i;
	connection_t *connp;
	player_type *p_ptr;

	for (i = 1; i <= NumPlayers; i++)
	{
		p_ptr = Players[i];

		if (p_ptr->new_level_flag) continue;

		connp = &Conn[p_ptr->conn];

		/* XXX XXX XXX Mega-Hack -- Redraw player's spot every time */
		/* This keeps the network connection "active" even if nothing is */
		/* happening -- KLJ */
		/* This has been changed to happen less often, operating at close to 3 
		 * times a second to keep the BGP routing tables happy.  
		 * I had originally changed this to about once every 2 seconds, 
		 * but apparently it was doing bad things, as the inactivity in the UDP
		 * stream was causing us to loose priority in the routing tables.
		 * Thanks to Crimson for explaining this. -APD
		 */
		
		/* to keep a good UDP connection, send data that requests a response
		 * every 1/4 of a second
		 */

		/* Hack -- add the index to our turn, so we don't send all the players
		 * reliable data simultaniously.  This should hopefully "spread out"
		 * the incoming data a little so it doesn't all happen in a semi-
		 * synchronized way.
		 */

		/*
		if (!((turn + i) % 4)) 
		{
			lite_spot(i, p_ptr->py, p_ptr->px); 
			if (Send_reliable(p_ptr->conn) == -1)
				return -1;	
		}
		*/

		/* otherwise, send normal data if there is any */
		//else 
		//{
	//		  Tell the client that this is the end  
	//		  
	//		If we have any data to send to the client, terminate it
	//		and send it to the client.
			if (connp->c.len > 0)
			{
				if (Packet_printf(&connp->c, "%c", PKT_END) <= 0)
				{
					Destroy_connection(p_ptr->conn, "write error");
					continue;
				}
				Send_reliable(p_ptr->conn);
			}
			// Flush the output buffers 
		//	if (Sockbuf_flush(&connp->w) == -1)
		//		return -1;
		//}

		//Sockbuf_clear(&connp->w);
	}

	/* Every fifteen seconds, update the info sent to the metaserver */
	if (!(turn % (15 * cfg_fps)))
		Report_to_meta(META_UPDATE);

	return 1;
}


/*
 * Encodes and sends an attr/char pairs stream using one of the following "mode"s:
 * 
 * RLE_NONE - no encoding is performed, the stream is sent as is.
 * RLE_CLASSIC - classic mangband encoding, the attr is ORed with 0x40, 
 * 					uses 3 bytes per repetition
 * RLE_LARGE - a more traffic-consuming routine, which is using a separate
 * 				byte to mark the repetition spot (making it 4 bytes per rep.), 
 * 				but it can be used to transfer high-bit attr/chars
 *  
 * "lineref" is a pointer to an attr/char array, and "max_col" is specifying it's size  
 *
 * Note! To sucessfully decode, client MUST use the same "mode"
 */
int rle_encode(sockbuf_t* buf, cave_view_type* lineref, int max_col, int mode)
{
	int x1, i, n, a;
	char c;
	/* count bytes */ int b = 0;

	/* Each column */
	for (i = 0; i < max_col; i++)
	{
		/* Obtain the char/attr pair */
		c = (lineref[i]).c;
		a = (lineref[i]).a;
		
		/* Start looking here */
		x1 = i + 1;

		/* Start with count of 1 */
		n = 1;

		/* Count repetitions of this grid */
		while (mode && lineref[x1].c == c &&
			lineref[x1].a == a && x1 < max_col)
		{
			/* Increment count and column */
			n++;
			x1++;
		}

		/* RLE-II if there at least 3 similar grids in a row */
		if (mode == RLE_LARGE && n >= 3)
		{
			/* Output the info - number of repetitons, and FF to mark the spot */
			Packet_printf(buf, "%c%c%c%c", (byte)n, 0xFF, c, a);

			/* Start again after the run */
			i = x1 - 1;	
			/* count bytes */ b+=4;
		}
		/* RLE-I if there at least 2 similar grids in a row */
		else if (mode == RLE_CLASSIC && n >= 2)
		{
			/* Set bit 0x40 of a */
			a |= 0x40;

			/* Output the info */
			Packet_printf(buf, "%c%c%c", c, a, n);
			
			/* Start again after the run */
			i = x1 - 1;
			/* count bytes */ b+=3;
		}
		else
		{
			/* Normal, single grid */
			Packet_printf(buf, "%c%c", c, a);
			/* count bytes */ b+=2;
		}
				
	}
	/* report total bytes */ return b;
}

/*
 * Send a reply to a special client request.
 * Not used consistently everywhere.
 * It could be used to setup some form of reliable
 * communication from the client to the server.
 */
int Send_reply(int ind, int replyto, int result)
{
	connection_t *connp = &Conn[ind];
	int n;

	n = Packet_printf(&connp->c, "%c%c%c", PKT_REPLY, replyto, result);
	if (n == -1)
	{
		Destroy_connection(ind, "write error");
		return -1;
	}

	return n;
}

int Send_leave(int ind, int id)
{
	connection_t *connp = &Conn[ind];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for leave info (%d,%d)",
			connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd", PKT_LEAVE, id);
}

// Actually quit. This was seperated as a hack to allow us to
// "quit" when a quit packet has not been received, such as when
// our TCP connection is severed.  The tellclient argument
// specifies whether or not we should try to send data to the
// client informing it about the quit event.
void do_quit(int ind, bool tellclient)
{
	int player, depth = 0;
	connection_t * connp = &Conn[ind];

	if (connp->id != -1) 
	{
		player = GetInd[connp->id];
		depth = Players[player]->dun_depth;
	}

	if (!tellclient)
	{
		/* Close the socket */
#ifdef WINDOWS
		SocketClose(connp->w.sock);
#else
		close(connp->w.sock);
#endif

		/* No more packets from a player who is quitting */
		remove_input(connp->w.sock);

		/* Disable all output and input to and from this player */
		connp->w.sock = -1;
	}

	/* If we are close to the center of town, exit quickly. */
	if (depth <= 0 ? wild_info[depth].radius <= 2 : 0)
	{
		Destroy_connection(ind, "client quit");
	}
	// Otherwise wait for the timeout
}


static int Receive_quit(int ind)
{
	int player, n, depth = 0;
	connection_t *connp = &Conn[ind];
	char ch;

	if (connp->id != -1) 
	{
		player = GetInd[connp->id];
		depth = Players[player]->dun_depth;
	}
	if ((n = Packet_scanf(&connp->r, "%c", &ch)) != 1)
	{
		errno = 0;
		Destroy_connection(ind, "receive error");
		return -1;
	}
	
	do_quit(ind, 0);
	
	return 1;
}

static int Receive_play(int ind)
{
	connection_t *connp = &Conn[ind];
	unsigned char ch, mode;
	int n;
	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &mode)) != 2)
	{
		errno = 0;
		plog("Cannot receive play packet");
		Destroy_connection(ind, "receive error");
		return -1;
	}
	if (connp->state != CONN_SETUP)
	{
			errno = 0;
			plog(format("Connection not in setup state (%02x)", connp->state));
			Destroy_connection(ind, "not setup");
			return -1;
	}

	/* Just asking */
	if (mode == 0)
	{
	
		Send_basic_info_conn(ind);
		
		Send_race_info_conn(ind);
		Send_class_info_conn(ind);

		Send_char_info_conn(ind);

	} else {
		/* Trying to start gameplay! */
		if ((n = Enter_player(ind)) == -2)
		{
			errno = 0;
			plog(format("Enable to play (%02x)", connp->state));
			Destroy_connection(ind, "cant play");
		}
		if (n < 0)
		{
			/* The connection has already been destroyed */
			return -1;
		}
	}
	return 2;
}

int Send_reliable(int ind)
{
	connection_t *connp = &Conn[ind];
	int num_written;

	/* Hack -- make sure we have a valid socket to write to.  -1 is used to
	 * specify a player that has disconnected but is still "in game".
	 */
	if (connp->w.sock == -1) return 0;

	if (Sockbuf_write(&connp->w, connp->c.buf, connp->c.len) != connp->c.len)
	{
		plog("Cannot write reliable data");
		Destroy_connection(ind, "write error");
		return -1;
	}
	if ((num_written = Sockbuf_flush(&connp->w)) < 0)
	{
		plog(format("Cannot flush reliable data (%d)", num_written));
		Destroy_connection(ind, "flush error");
		return -1;
	}
	Sockbuf_clear(&connp->c);
	return num_written;
}

static int Receive_discard(int ind)
{
	connection_t *connp = &Conn[ind];

	errno = 0;
	plog(format("Discarding packet %d while in state %02x",
		connp->r.ptr[0], connp->state));
	connp->r.ptr = connp->r.buf + connp->r.len;

	return 0;
}

static int Receive_undefined(int ind)
{
	connection_t *connp = &Conn[ind];

	errno = 0;
	plog(format("Unknown packet type %s (%d,%02x)",connp->nick, connp->r.ptr[0], connp->state));
	Destroy_connection(ind, "undefined packet"); 
	return -1;
}

int Send_plusses(int ind, int tohit, int todam)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for plusses (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd", PKT_PLUSSES, tohit, todam);
}


int Send_oppose(int ind, int acid, int elec, int fire, int cold, int pois)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for oppose (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd%hd%hd%hd", PKT_OPPOSE, acid, elec, fire, cold, pois);
}

int Send_ac(int ind, int base, int plus)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for ac (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd", PKT_AC, base, plus);
}

int Send_experience(int ind, int lev, int max, int cur, s32b adv)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for experience (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu%d%d%d", PKT_EXPERIENCE, lev, 
			max, cur, adv);
}

int Send_gold(int ind, s32b au)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for gold (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%d", PKT_GOLD, au);
}

int Send_hp(int ind, int mhp, int chp)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for hp (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%hd%hd", PKT_HP, mhp, chp);
}

int Send_sp(int ind, int msp, int csp)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for sp (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd", PKT_SP, msp, csp);
}

int Send_game_start_conn(int ind)
{
	connection_t *connp = &Conn[ind];
	if (connp->state != CONN_SETUP)
	{
		errno = 0;
		plog(format("Connection not ready for play packet (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c", PKT_PLAY);
}
int Send_motd_conn(int ind, int offset)
{
	connection_t *connp = &Conn[ind];
	int i;
	int max;

	 
	max = MAX_MOTD_CHUNK;
	if (offset + max > Setup.motd_len) max = Setup.motd_len - offset;
	if (offset > Setup.motd_len) offset = Setup.motd_len;
		
	if (Packet_printf(&connp->c, "%c%ld%ld", PKT_MOTD, max, offset) <= 0)
	{
		Destroy_connection(ind, format("write error"));
		return -1;
	}
	
	for (i = offset; i < offset + max; i++)
	{
		if (Packet_printf(&connp->c, "%c", Setup.motd[i]) <= 0)
		{
			Destroy_connection(ind, "write error");
			return -1;
		}
	}
	
	return 1; 
} 
int Send_race_info_conn(int ind)
{
	connection_t *connp = &Conn[ind];
	u32b i, name_size;
	if (!BIT(connp->state, CONN_SETUP))
	{
		errno = 0;
		plog(format("Connection not ready for race info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	if (Packet_printf(&connp->c, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_RACE) <= 0)
	{
		Destroy_connection(ind, "write error");
		return -1;
	}
	
	name_size = p_info[z_info->p_max-1].name + strlen(p_name + p_info[z_info->p_max-1].name);
	if (Packet_printf(&connp->c, "%hu%lu%lu", z_info->p_max, name_size, z_info->fake_text_size) <= 0)
	{
		Destroy_connection(ind, "write error");
		return -1;
	} 

	for (i = 0; i < z_info->p_max; i++)
	{
		/* Transfer other fields here */
		if (Packet_printf(&connp->c, "%s%lu",  p_name + p_info[i].name, p_info[i].name) <= 0)
		{
			Destroy_connection(ind, "write error");
			return -1;
		}
	}
	return 1;
}
int Send_class_info_conn(int ind)
{
	connection_t *connp = &Conn[ind];
	u32b i, name_size;
	if (!BIT(connp->state, CONN_SETUP))
	{
		errno = 0;
		plog(format("Connection not ready for class info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	if (Packet_printf(&connp->c, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_CLASS) <= 0)
	{
		Destroy_connection(ind, "write error");
		return -1;
	}
	
	name_size = c_info[z_info->c_max-1].name + strlen(c_name + c_info[z_info->c_max-1].name);
	if (Packet_printf(&connp->c, "%hu%lu%lu", z_info->c_max, name_size, z_info->fake_text_size) <= 0)
	{
		Destroy_connection(ind, "write error");
		return -1;
	} 

	for (i = 0; i < z_info->c_max; i++)
	{
		/* Transfer other fields here */
		if (Packet_printf(&connp->c, "%s%lu%c",  c_name + c_info[i].name, c_info[i].name, c_info[i].spell_book) <= 0)
		{
			Destroy_connection(ind, "write error");
			return -1;
		}
	}
	return 1;	
}
int Send_basic_info_conn(int ind)
{
	connection_t *connp = &Conn[ind];
	if (!BIT(connp->state, CONN_SETUP))
	{
		errno = 0;
		plog(format("Connection not ready for basic info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd%hd%c%c%c%c%hd", PKT_BASIC_INFO, Setup.frames_per_second, 3, 1, Setup.min_col, Setup.min_row, Setup.max_col, Setup.max_row, 0);
}
int Send_char_info_conn(int ind)
{
	connection_t *connp = &Conn[ind];

	if (!BIT(connp->state, CONN_SETUP))
	{
		errno = 0;
		plog(format("Connection not ready for char info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd%hd%hd", PKT_CHAR_INFO, connp->char_state, connp->race, connp->class, connp->sex);
}
int Send_term_resize_ack(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for term resize ack (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c%c%c", PKT_ACK, 0, Players[ind]->screen_wid, Players[ind]->screen_hgt);
}

int Send_char_info(int ind, int race, int class, int sex)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for char info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd%hd%hd", PKT_CHAR_INFO, race, class, sex);
}

int Send_various(int ind, int hgt, int wgt, int age, int sc)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for various (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu%hu%hu%hu", PKT_VARIOUS, hgt, wgt, age, sc);
}

int Send_stat(int ind, int stat, int max, int cur)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for stat (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c%hd%hd", PKT_STAT, stat, max, cur);
}

int Send_maxstat(int ind, int stat, int max)
{
    connection_t *connp = &Conn[Players[ind]->conn];

    if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
    {
        errno = 0;
        plog(format("Connection not ready for maxstat (%d.%d.%d)",
            ind, connp->state, connp->id));
        return 0;
    }

    return Packet_printf(&connp->c, "%c%c%hd", PKT_MAXSTAT, stat, max);
}

int Send_objflags(int Ind, int line)
{
	connection_t *connp = &Conn[Players[Ind]->conn];
	player_type *p_ptr = Players[Ind];
	
	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for objflags (%d.%d.%d)",
			Ind, connp->state, connp->id));
		return 0;
	}
	
	/* Put a header on the packet */
	Packet_printf(&connp->c, "%c%hu", PKT_OBJFLAGS, line);

	/* Encode and send the attr/char stream */
	rle_encode(&connp->c, p_ptr->hist_flags[line], 13, RLE_CLASSIC); 	

	/* No RLE mode
	for (i = 0; i < 13; i++) {
	
		Packet_printf(&connp->c, "%c%c", Players[Ind]->hist_flags[line][i].a, Players[Ind]->hist_flags[line][i].c);
		
	}
	*/
	return 1;
}

int Send_history(int ind, int line, cptr hist)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for history (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu%s", PKT_HISTORY, line, hist);
}

int Send_floor(int ind, byte attr, int amt, byte tval, cptr name)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for floor item (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c%hd%c%s", PKT_FLOOR, attr, amt, tval, name);
}

int Send_inven(int ind, char pos, byte attr, int wgt, int amt, byte tval, cptr name)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for inven (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c%c%hu%hd%c%s", PKT_INVEN, pos, attr, wgt, amt, tval, name);
}

int Send_equip(int ind, char pos, byte attr, int wgt, byte tval, cptr name)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for equip (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c%c%hu%c%s", PKT_EQUIP, pos, attr, wgt, tval, name);
}

int Send_title(int ind, cptr title)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for title (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%s", PKT_TITLE, title);
}

int Send_depth(int ind, int depth)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for depth (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu", PKT_DEPTH, depth);
}

int Send_food(int ind, int food)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for food (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu", PKT_FOOD, food);
}

int Send_blind(int ind, bool blind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for blind (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c", PKT_BLIND, blind);
}

int Send_confused(int ind, bool confused)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for confusion (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c", PKT_CONFUSED, confused);
}

int Send_fear(int ind, bool fear)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for fear (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c", PKT_FEAR, fear);
}

int Send_poison(int ind, bool poisoned)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for poison (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c", PKT_POISON, poisoned);
}

int Send_state(int ind, bool paralyzed, bool searching, bool resting)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for state (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu%hu%hu", PKT_STATE, paralyzed, searching, resting);
}

int Send_speed(int ind, int speed)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for speed (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hd", PKT_SPEED, speed);
}

int Send_study(int ind, bool study)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for study (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%c", PKT_STUDY, study);
}

int Send_cut(int ind, int cut)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for cut (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu", PKT_CUT, cut);
}

int Send_stun(int ind, int stun)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for stun (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu", PKT_STUN, stun);
}

int Send_direction(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for direction (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c", PKT_DIRECTION);
}

int Send_message(int ind, cptr msg, u16b typ)
{
	connection_t *connp = &Conn[Players[ind]->conn];
	char buf[80];

	if (msg == NULL)
		return 1;

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for message (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	/* Clip end of msg if too long */
	strncpy(buf, msg, 78);
	buf[78] = '\0';

	return Packet_printf(&connp->c, "%c%s%hd", PKT_MESSAGE, buf, typ);
}

int Send_char(int ind, int x, int y, byte a, char c, byte ta, char tc)
{
	if (!BIT(Conn[Players[ind]->conn].state, CONN_PLAYING | CONN_READY))
		return 0;

	if (Players[ind]->use_graphics > 1)
		return Packet_printf(&Conn[Players[ind]->conn].c, "%c%c%c%c%c%c%c", PKT_CHAR, x, y, a, c, ta, tc);
	else
		return Packet_printf(&Conn[Players[ind]->conn].c, "%c%c%c%c%c", PKT_CHAR, x, y, a, c);
}

int Send_spell_info(int ind, int book, int i, cptr out_val)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for spell info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c%hu%hu%s", PKT_SPELL_INFO, book, i, out_val);
}


int Send_item_request(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for item request (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c", PKT_ITEM);
}

int Send_flush(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for flush (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	return Packet_printf(&connp->c, "%c", PKT_FLUSH);
}

/*
 * As an attempt to lower bandwidth requirements, each line is run length
 * encoded.  Non-encoded grids are sent as normal, but if a grid is
 * repeated at least twice, then bit 0x40 of the attribute is set, and
 * the next byte contains the number of repetitions of the previous grid.
 */
int Send_line_info(int ind, int y)
{
	player_type *p_ptr = Players[ind];
	connection_t *connp = &Conn[p_ptr->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for line info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	
	/* Put a header on the packet */
	Packet_printf(&connp->c, "%c%hd", PKT_LINE_INFO, y);

	if (p_ptr->use_graphics > 1)
	{
		/* Encode and transperancy attr/char stream */
		rle_encode(&connp->c, p_ptr->trn_info[y], p_ptr->screen_wid, RLE_LARGE );
	}

	/* Encode and send the attr/char stream */
	rle_encode(&connp->c, p_ptr->scr_info[y], p_ptr->screen_wid, ( p_ptr->use_graphics ? RLE_LARGE : RLE_CLASSIC ) );


	/* Hack -- Prevent buffer overruns by flushing after each line sent */
	/* Send_reliable(Players[ind]->conn); */
	
	return 1;
}

int Send_mini_map(int ind, int y)
{
	player_type *p_ptr = Players[ind];
	connection_t *connp = &Conn[p_ptr->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for minimap (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}
	    
	/* Packet header */
	Packet_printf(&connp->c, "%c%hd", PKT_MINI_MAP, y);
	
	rle_encode(&connp->c, p_ptr->scr_info[y], p_ptr->screen_wid, ( p_ptr->use_graphics ? RLE_LARGE : RLE_CLASSIC ) ); 

	/* Hack -- Prevent buffer overruns by flushing after each line sent */
	/* Send_reliable(Players[ind]->conn); */
	
	return 1;
}

int Send_store(int ind, char pos, byte attr, int wgt, int number, int price, cptr name)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for store item (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c%c%hd%hd%d%s", PKT_STORE, pos, attr, wgt, number, price, name);
}

int Send_store_info(int ind, int num, int owner, int items)
{
	connection_t *connp = &Conn[Players[ind]->conn];

    int count = items;

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for store info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

    return Packet_printf(&connp->c, "%c%hd%hd%hd", PKT_STORE_INFO, num, owner, count);
}

int Send_player_store_info(int ind, char *name, char *owner, int items)
{
	connection_t *connp = &Conn[Players[ind]->conn];

    int count = items;

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for store info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

    return Packet_printf(&connp->c, "%c%s%s%hd", PKT_PLAYER_STORE_INFO, name, owner, count);
}

int Send_store_sell(int ind, int price)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for sell price (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%d", PKT_SELL, price);
}

int Send_target_info(int ind, int x, int y, cptr str)
{
	connection_t *connp = &Conn[Players[ind]->conn];
	char buf[80];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for target info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	/* Copy */
	strncpy(buf, str, 79);

	/* Paranoia -- Add null */
	buf[79] = '\0';

	return Packet_printf(&connp->c, "%c%c%c%s", PKT_TARGET_INFO, x, y, buf);
}

int Send_sound(int ind, int sound)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for sound (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c", PKT_SOUND, sound);
}

/*
 * Hack this should be loaded from some cool structure!!! 
 *
 *	XXX XXX HARDCODED COMMANDS HERE:
 */
int Send_custom_command(int ind, int command)
{
	connection_t *connp = &Conn[Players[ind]->conn];
	s16b trigger;
	byte tval = 0;
	u32b flag = 0;
	cptr prompt = "";
	char buf[60];
	buf[59] = '\0';
	
	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for custom command (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	switch (command) 
	{
		case 1:
			trigger = 'J';
			flag = (COMMAND_ITEM_NONE | COMMAND_TARGET_DIR);
			prompt = "Touch in what ";
			break;
		case 0:
			trigger = 'E';
			tval = TV_FOOD;
			flag = (COMMAND_ITEM_INVENT | COMMAND_ITEM_FLOOR | COMMAND_ITEM_NORMAL | COMMAND_TARGET_NONE);
			prompt = "Eat what? ";
			break;
		default:
			return -1;
	}

	strncpy(buf, prompt, 59);
	return Packet_printf(&connp->c, "%c%hd%lu%c%S", PKT_COMMAND, trigger, flag, tval, buf);
}
//Second evil function
void Send_custom_commands(int ind)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		Send_custom_command(ind, i);
	} 
}

int Send_special_line(int ind, int max, int line, byte attr, cptr buf)
{
	connection_t *connp = &Conn[Players[ind]->conn];
	char temp[80];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for special line (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	strncpy(temp, buf, 79);
	temp[79] = '\0';
	return Packet_printf(&connp->c, "%c%hd%hd%c%s", PKT_SPECIAL_LINE, max, line, attr, temp);
}

int Send_pickup_check(int ind, cptr buf)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for pickup check (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%s", PKT_PICKUP_CHECK, buf);
}

/* adding ultimate quick and dirty hack here so geraldo can play his 19th lvl char
   with the 80 character party name...... 
   -APD-
*/

int Send_party(int ind)
{
	player_type *p_ptr = Players[ind];
	connection_t *connp = &Conn[p_ptr->conn];
	char buf[160];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection nor ready for party info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	sprintf(buf, "Party: %s", parties[p_ptr->party].name);

	/* MEGA hack */
	
	buf[60] = 0;

	if (p_ptr->party > 0)
	{
		strcat(buf, "     Owner: ");
		strcat(buf, parties[p_ptr->party].owner);
	}

	return Packet_printf(&connp->c, "%c%s", PKT_PARTY, buf);
}

int Send_channel(int ind, int n, cptr virtual)
{
	player_type *p_ptr = Players[ind];
	connection_t *connp = &Conn[p_ptr->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection nor ready for chat channel info (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	} 

	return Packet_printf(&connp->c, "%c%c%s", PKT_CHANNEL, n, ( virtual ? virtual : channels[n].name));
}

int Send_special_other(int ind, char *header)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for special other (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%s", PKT_SPECIAL_OTHER, header);
}

int Send_skills(int ind)
{
	player_type *p_ptr = Players[ind];
	connection_t *connp = &Conn[p_ptr->conn];
	s16b skills[11];
	int i, tmp;
	object_type *o_ptr;

	/* Fighting skill */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];
	tmp = p_ptr->to_h + o_ptr->to_h;
	skills[0] = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);

	/* Shooting skill */
	o_ptr = &p_ptr->inventory[INVEN_BOW];
	tmp = p_ptr->to_h + o_ptr->to_h;
	skills[1] = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);

	/* Basic abilities */
	skills[2] = p_ptr->skill_sav;
	skills[3] = p_ptr->skill_stl;
	skills[4] = p_ptr->skill_fos;
	skills[5] = p_ptr->skill_srh;
	skills[6] = p_ptr->skill_dis;
	skills[7] = p_ptr->skill_dev;

	/* Number of blows */
	skills[8] = p_ptr->num_blow;
	skills[9] = p_ptr->num_fire;

	/* Infravision */
	skills[10] = p_ptr->see_infra;

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for skills (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	Packet_printf(&connp->c, "%c", PKT_SKILLS);

	for (i = 0; i < 11; i++)
	{
		Packet_printf(&connp->c, "%hd", skills[i]);
	}

	return 1;
}
	

int Send_pause(int ind)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for skills (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c", PKT_PAUSE);
}


int Send_cursor(int ind, char vis, char x, char y)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for cursor position (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c%c%c", PKT_CURSOR, vis, x, y);
}


int Send_monster_health(int ind, int num, byte attr)
{
	connection_t *connp = &Conn[Players[ind]->conn];

	if (!BIT(connp->state, CONN_PLAYING | CONN_READY))
	{
		errno = 0;
		plog(format("Connection not ready for monster health bar (%d.%d.%d)",
			ind, connp->state, connp->id));
		return 0;
	}

	return Packet_printf(&connp->c, "%c%c%c", PKT_MONSTER_HEALTH, num, attr);
}

/*
 * Return codes for the "Receive_XXX" functions are as follows:
 *
 * -1 --> Some error occured
 *  0 --> The action was queued (not enough energy)
 *  1 --> The action was ignored (not enough energy)
 *  2 --> The action completed successfully
 *
 *  Every code except for 1 will cause the input handler to stop
 *  processing actions.
 */

// This does absolutly nothing other than keep our connection active.
static int Receive_keepalive(int ind)
{
        int n;
        connection_t *connp = &Conn[ind];
        char ch;

        s32b ctime;

        if ((n = Packet_scanf(&connp->r, "%c%ld", &ch, &ctime)) <= 0)
        {
           if (n == -1) Destroy_connection(ind, "read error");
           return n;
        }

        Packet_printf(&connp->c, "%c%ld", PKT_KEEPALIVE, ctime);
        return 2;
}

static int Receive_verify_visual(int ind)
{
	connection_t *connp = &Conn[ind];

	int n, i, type, size, local_size;	
	char ch;
	byte at;
	bool discard = FALSE;
	char *char_ref;
	byte *attr_ref;
	type=size=0;
	if ((n = Packet_scanf(&connp->r, "%c%c%hd", &ch, &type, &size)) <= 0)
	{
		if (n == -1) Destroy_connection(ind, "read error");
		return n;
	}
	
	/* Gather type */
	switch (type) {
		case 0:
			local_size = MAX_FLVR_IDX;
			attr_ref = connp->Client_setup.flvr_x_attr;
			char_ref = connp->Client_setup.flvr_x_char;
			break;
		case 1:
			local_size = MAX_F_IDX;
			attr_ref = connp->Client_setup.f_attr;
			char_ref = connp->Client_setup.f_char;
			break;
		case 2:
			local_size = MAX_K_IDX;
 			attr_ref = connp->Client_setup.k_attr;
  			char_ref = connp->Client_setup.k_char;
  			break;
  		case 3:
			local_size = MAX_R_IDX;
			attr_ref = connp->Client_setup.r_attr;
			char_ref = connp->Client_setup.r_char;
			break;
		case 4:
	 		local_size = 128;
			attr_ref = connp->Client_setup.tval_attr;
			char_ref = connp->Client_setup.tval_char;
			break;
		case 5:
			local_size = 256;
			attr_ref = connp->Client_setup.misc_attr;
			char_ref = connp->Client_setup.misc_char;
			break;
		default:
			discard = TRUE;
	}
	if (local_size != size)
	{
		discard = TRUE;
	}
	
	/* Finally read the data */
	for (i = 0; i < size; i++)
	{
		if ((n = Packet_scanf(&connp->r, "%c%c", &at, &ch)) <= 0)
		{
			if (n == -1) Destroy_connection(ind, "read error");
			return n;
		}
		if (!discard)
		{
			attr_ref[i] = at; 
			char_ref[i] = ch;
		}
	}
	
	return 2;
}
static int Receive_motd(int ind)
{
	connection_t *connp = &Conn[ind];

	int n, off;	
	char ch;
	
	if ((n = Packet_scanf(&connp->r, "%c%ld", &ch, &off)) <= 0)
	{
		if (n == -1) Destroy_connection(ind, "read error");
		return n;
	}

	Send_motd_conn(ind, off);

	return 2;
}

static int Receive_char_info(int ind)
{
	connection_t *connp = &Conn[ind];

	int n, i;	
	char ch;

	if ((n = Packet_scanf(&connp->r, "%c%hu%hu%hu", &ch, &connp->race, &connp->class, &connp->sex)) <= 0)
	{
		if (n == -1) Destroy_connection(ind, "read error");
		return n;
	}

	/* Read the stat order */
	for (i = 0; i < 6; i++)
	{
		if ((n = Packet_scanf(&connp->r, "%hd", &connp->stat_order[i])) == -1) {
			Destroy_connection(ind, "misread stat order");
			return n;
		}
	}

	/* Have Template */
	connp->char_state = 1;

	Send_char_info_conn(ind);

	return 2;
}
static int Receive_walk(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;
	char ch, dir;

    int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	/* Disturb if running or resting */
	if (p_ptr->running || p_ptr->resting)
	{
		disturb(player, 0, 0);
		return 1;
	}

	if (player && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_walk(player, dir, p_ptr->always_pickup);
		return 2;
	}
	else
	{
		// Otherwise discared the walk request.
		//if (!connp->q.len && p_ptr->autoattack)
		// If we have no commands queued, then queue our walk request.
		// Note that ch might equal PKT_RUN, since Receive_run will
		// sometimes call this function.
		if (!connp->q.len)
		{
			Packet_printf(&connp->q, "%c%c", PKT_WALK, dir);
			return 0;
		}
		else
		{
			// If we have a walk command queued at the end of the queue,
			// then replace it with this queue request.  
			if (connp->q.buf[connp->q.len - 2] == PKT_WALK)
			{
				connp->q.len -= 2;
				Packet_printf(&connp->q, "%c%c", PKT_WALK, dir);
				return 0;
			}
		}
	}

	return 1;
}

static int Receive_run(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int i, n, player;
	char dir;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	/* If not the dungeon master, who can always run */
	if (!(p_ptr->dm_flags & DM_NEVER_DISTURB)) 
	{
		/* Check for monsters in sight or confusion */
		for (i = 0; i < m_max; i++)
		{
			/* Check this monster */
			/* Level 0 monsters do not disturb */
			if ((p_ptr->mon_los[i] && !m_list[i].csleep && r_info[m_list[i].r_idx].level) || (p_ptr->confused))
			{
				// Treat this as a walk request
				// Hack -- send the same connp->r "arguments" to Receive_walk
				// Hack -- Always allow running in town.
				if(p_ptr->dun_depth) {	
					return Receive_walk(ind);
				}
			}
		}
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	/* Disturb if we want to change directions */
	//if (dir != p_ptr->find_current) disturb(player, 0, 0);

	/* Hack -- Fix the running in '5' bug */
	if (dir == 5)
		return 1;

	// If we don't want to queue the command, return now.
	if ((n = do_cmd_run(player,dir)) == 2)
	{
		return -1;
	}
	// If do_cmd_run returns a 0, then there wasn't enough energy
	// to execute the run command.  Queue the run command if desired.
	else if (n == 0)
	{
		// Only buffer a run request if we have no previous commands
		// buffered, and it is a new direction or we aren't already
		// running.
		if (((!connp->q.len) && (dir != p_ptr->find_current)) || (!p_ptr->running))
		{
			Packet_printf(&connp->q, "%c%c", ch, dir);
				return 0;
		}
	}

	return 1;
}

static int Receive_tunnel(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_tunnel(player, dir);
		return 2;
	}

	return 1;
}

static int Receive_aim_wand(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd%c", &ch, &item, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_aim_wand(player, item, dir);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd%c", ch, item, dir);
		return 0;
	}

	return 1;
}

static int Receive_drop(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player; 
	s16b item, amt;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &item, &amt)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_drop(player, item, amt);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd%hd", ch, item, amt);
		return 0;
	}

	return 1;
}

static int Receive_fire(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir = 5;

	int n, player;
	s16b item;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c%hd", &ch, &dir, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	/* Check confusion */
	if (p_ptr->confused)
	{
		/* Change firing direction */
		while (dir == 5)
			dir = ddd[rand_int(8)];
	}

	if (connp->id != -1 && p_ptr->energy >= (level_speed(p_ptr->dun_depth) / p_ptr->num_fire))
	{
		do_cmd_fire(player, dir, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c%hd", ch, dir, item);
		return 0;
	}

	return 1;
}

static int Receive_stand(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
	{
		do_cmd_stay(player, 1);
		return 2;
	}

	return -1;
}

static int Receive_destroy(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item, amt;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &item, &amt)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_destroy(player, item, amt);
		return 2;
	}
	else if (player)
	{
		// I am immortalizing this commented out line so others can see
		// the utter stupidity of the famous "destroying-things-ocasionally-
		// crashesg- the-server" bug.  The horrobile thing is I didn't
		// investigate this for several weeks. 
		// The Moral : Listen seriously to bugreports
		//Packet_printf(&connp->q, "%c%hd%hd", &ch, &item, &amt);
		Packet_printf(&connp->q, "%c%hd%hd", ch, item, amt);
		return 0;
	}

	return 1;
}

static int Receive_observe(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_observe(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_look(int ind)
{
    connection_t *connp = &Conn[ind];
    player_type *p_ptr;

    char ch, dir;

    int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

    if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
    {
        if (n == -1)
            Destroy_connection(ind, "read error");
        return n;
    }

    if (connp->id != -1)
        do_cmd_look(player, dir);

    return 1;
}

static int Receive_spell(int ind)
{
    connection_t *connp = &Conn[ind];
    player_type *p_ptr;

    char ch;

    int n, player;

    s16b book, spell;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

    if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &book, &spell)) <= 0)
    {
        if (n == -1)
            Destroy_connection(ind, "read error");
        return n;
    }

    if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
    {
		do_cmd_cast(player, book, spell);
		return 2;
	}
	else if (player)
	{
		p_ptr->current_spell = -1;
		Packet_printf(&connp->q, "%c%hd%hd", ch, book, spell);
		return 0;
	}

	return 1;
}

static int Receive_open(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_open(player, dir);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c", ch, dir);
		return 0;
	}

	return 1;
}

static int Receive_pray(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	s16b book, prayer;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &book, &prayer)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_pray(player, book, prayer);
		return 2;
	}
	else if (player)
	{
		p_ptr->current_spell = -1;
		Packet_printf(&connp->q, "%c%hd%hd", ch, book, prayer);
		return 0;
	}

	return 1;
}

static int Receive_ghost(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	s16b ability;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &ability)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_ghost_power(player, ability);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, ability);
		return 0;
	}

	return 1;
}

static int Receive_quaff(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_quaff_potion(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_read(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_read_scroll(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_search(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_search(player);
		return 2;
	}

	return 1;
}

static int Receive_take_off(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_takeoff(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_use(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_use_staff(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_throw(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;
	s16b item;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c%hd", &ch, &dir, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_throw(player, dir, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c%hd", ch, dir, item);
		return 0;
	}

	return 1;
}

static int Receive_wield(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_wield(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_zap(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_zap_rod(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_target(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;
	s16b dir;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_target(player, dir);

	return 1;
}

static int Receive_target_friendly(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;
	s16b dir;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_target_friendly(player, dir);

	return 1;
}


static int Receive_inscribe(int ind)
{
    s16b item;

    char inscription[80];

	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c%hd%s", &ch, &item, inscription)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_inscribe(player, item, inscription);

	return 1;
}

static int Receive_uninscribe(int ind)
{
    s16b item;

	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_uninscribe(player, item);

	return 1;
}

static int Receive_activate(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_activate(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_bash(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_bash(player, dir);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c", ch, dir);
		return 0;
	}

	return 1;
}

static int Receive_disarm(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;


	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_disarm(player, dir);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c", ch, dir);
		return 0;
	}

	return 1;
}

static int Receive_eat(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	s16b item;
	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_eat_food(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

		
static int Receive_fill(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	s16b item;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_refill(player, item);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd", ch, item);
		return 0;
	}

	return 1;
}

static int Receive_locate(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch, dir;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_locate(player, dir);

	return 1;
}

static int Receive_map(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_view_map(player);

	return 1;
}

static int Receive_search_mode(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		do_cmd_toggle_search(player);

	return 1;
}

static int Receive_close(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_close(player, dir);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%c", ch, dir);
		return 0;
	}

	return 1;
}

static int Receive_gain(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	s16b book, spell;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &book, &spell)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_study(player, book, spell);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%hd%hd", ch, book, spell);
		return 0;
	}

	return 1;
}

static int Receive_go_up(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_go_up(player);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c", ch);
		return 0;
	}

	return 1;
}

static int Receive_go_down(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_go_down(player);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c", ch);
		return 0;
	}

	return 1;
}


static int Receive_direction(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch, dir;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		Handle_direction(player, dir);

	return 1;
}

static int Receive_item(int ind)
{
    s16b item;

	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%hd", &ch, &item)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1)
		Handle_item(player, item);

	return 1;
}


static int change_password(Ind, buf)
int Ind;
char * buf;
{

	player_type *p_ptr = Players[Ind];
	if(p_ptr) {
		strncpy(p_ptr->pass, buf, MAX_PASS_LEN);
		return 0;
	}
	return -1;
}


static int Receive_pass(int ind)
{
	connection_t *connp = &Conn[ind];

	char ch, buf[1024];

	int n, player;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%S", &ch, buf)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	change_password(player, buf);

	return 1;
}

static int Receive_message(int ind)
{
	connection_t *connp = &Conn[ind];

	char ch, buf[1024];

	int n, player;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	buf[0] = '\0';

	if ((n = Packet_scanf(&connp->r, "%c%S", &ch, buf)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	player_talk(player, buf);

	return 1;
}

static int Receive_channel(int ind)
{
	connection_t *connp = &Conn[ind];

	char ch, buf[1024];

	int n, player;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	buf[0] = '\0';

	if ((n = Packet_scanf(&connp->r, "%c%S", &ch, buf)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (buf[0] == '-')
	{
		channel_leave(player, buf + 1);
	}
	else if (buf[0] == '#')
	{
		channel_join(player, buf, TRUE);
		/* Hack: Secondary channel */
		Players[player]->second_channel[0] = '\0';
	}	else if (player) strncpy(Players[player]->second_channel, buf, 80);
		/* EndHack */

	return 1;
}
	
static int Receive_purchase(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;
	int n, player;
	s16b item, amt;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &item, &amt)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player && p_ptr->store_num > -1)
		store_purchase(player, item, amt);
	else if (player)
		do_cmd_purchase_house(player, item);

	return 1;
}

static int Receive_sell(int ind)
{
    s16b item, amt;

	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%hd%hd", &ch, &item, &amt)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
		store_sell(player, item, amt);

	return 1;
}

static int Receive_store_leave(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
		p_ptr = Players[player];
    }
    else player = 0;
    
	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (!player) return -1;

        /* Update stuff */
        p_ptr->update |= (PU_VIEW | PU_LITE);
        p_ptr->update |= (PU_MONSTERS);

        /* Redraw stuff */
        p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA);

        /* Redraw map */
        p_ptr->redraw |= (PR_MAP);

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD);

	/* Update store info */
	p_ptr->store_num = -1;

	return 1;
}

static int Receive_store_confirm(int ind)
{
	connection_t *connp = &Conn[ind];
    player_type *p_ptr;

	char ch;

	int n, player;

    if (connp->id != -1)
    {
        player = GetInd[connp->id];
        p_ptr = Players[player];
    }
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (!player)
		return -1;

	if (p_ptr->store_num > -1)
		store_confirm(player);
	else
		do_cmd_purchase_house(player, 0);

	return 1;
}

static int Receive_drop_gold(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	char ch;
	int n, player;
	s32b amt;

	if (connp->id != -1) 
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%ld", &ch, &amt)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		do_cmd_drop_gold(player, amt);
		return 2;
	}
	else if (player)
	{
		Packet_printf(&connp->q, "%c%ld", ch, amt);
		return 0;
	}

	return 1;
}

static int Receive_custom_command(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	s16b item, value;
	int n, player;
	char ch, i, dir;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c%hd%c%hd", &ch, &i, &item, &dir, &value)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}
	
	do_cmd_custom(player, i, item, dir, value);

	return 1;
}

static int Receive_spike(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}
	
	do_cmd_spike(player, dir);
	
	return 1;
}

static int Receive_steal(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;

	char ch, dir;

	int n, player;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &dir)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}
	
	if (!cfg_no_steal) {
		if (connp->id != -1 && p_ptr->energy >= level_speed(p_ptr->dun_depth))
		{
			do_cmd_steal(player, dir);
			return 2;
		}
		else if (player)
		{
			Packet_printf(&connp->q, "%c%c", ch, dir);
			return 0;
		}
	}
	else 
		/* handle the option to disable player/player stealing */
		msg_print(player, "Your pathetic attempts at stealing fail.\n");

	return 1;
}
	
static int Receive_redraw(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	int player, n;
	char ch;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
	{
		p_ptr->store_num = -1;
		p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_FLOOR );
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);
	p_ptr->update |= (PU_BONUS | PU_VIEW | PU_MANA | PU_HP);
	}

	return 1;
}

static int Receive_rest(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	int player, n;
	char ch;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)	
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
	{
		/* If we are already resting, cancel the rest. */
		/* Waking up takes no energy, although we will still be drowsy... */
		if (p_ptr->resting)
		{
			disturb(player, 0, 0);
			return 2;
		}

		/* Don't rest if we are poisoned or at max hit points and max spell points */
		if ((p_ptr->poisoned) || ((p_ptr->chp == p_ptr->mhp) &&
					(p_ptr->csp == p_ptr->msp)))
		{
			return 2;
		}

		/* Resting takes a lot of energy! */
		if ((p_ptr->energy) >= (level_speed(p_ptr->dun_depth)*2)-1)
		{

			/* Set flag */
			p_ptr->resting = TRUE;

			/* Make sure we aren't running */
			p_ptr->running = FALSE;

			/* Take a lot of energy to enter "rest mode" */
			p_ptr->energy -= (level_speed(p_ptr->dun_depth)*2)-1;

			/* Redraw */
			p_ptr->redraw |= (PR_STATE);
			return 2;
		}
		/* If we don't have enough energy to rest, disturb us (to stop
		 * us from running) and queue the command.
		 */
		else
		{
			disturb(player, 0, 0);
			Packet_printf(&connp->q, "%c", ch);
			return 0;
		}
	}

	return 1;
}

static int Receive_special_line(int ind)
{
	connection_t *connp = &Conn[ind];
	int player, n;
	char ch, type;
	s16b line;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%c%hd", &ch, &type, &line)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
	{
		switch (type)
		{
			case SPECIAL_FILE_NONE:
				Players[player]->special_file_type = FALSE;
				break;
			case SPECIAL_FILE_UNIQUE:
				do_cmd_check_uniques(player, line);
				break;
			case SPECIAL_FILE_ARTIFACT:
				do_cmd_check_artifacts(player, line);
				break;
			case SPECIAL_FILE_PLAYER:
				do_cmd_check_players(player, line);
				break;
			case SPECIAL_FILE_OTHER:
				do_cmd_check_other(player, line);
				break;
			case SPECIAL_FILE_SCORES:
				display_scores(player, line);
				break;
			case SPECIAL_FILE_HELP:
				do_cmd_help(player, line);
				break;
		}
	}

	return 1;
}

static int Receive_symbol_query(int ind)
{
	connection_t *connp = &Conn[ind];
	int player, n;
	char ch, sym;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;
		
	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &sym)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	/* Perform query */
	do_cmd_query_symbol(player, sym);

	return 1;
}

static int Receive_options(int ind)
{
	connection_t *connp = &Conn[ind];
	player_type *p_ptr;
	int player, i, n;
	char ch, settings;

	if (connp->id != -1)
	{
		player = GetInd[connp->id];
		p_ptr = Players[player];
	}
	else
	{
		player = 0;
		p_ptr = NULL;
	}

	if ((n = Packet_scanf(&connp->r, "%c%c", &ch, &settings)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (settings)
	{
		for (i = 0; i < 16; i++)
		{
			n = Packet_scanf(&connp->r, "%hd", &connp->Client_setup.settings[i]);
			if (n <= 0)
			{
				Destroy_connection(ind, "read error");
				return n;
			}
		}
	}		


	for (i = 0; i < 64; i++)
	{
		n = Packet_scanf(&connp->r, "%c", &connp->Client_setup.options[i]);
		if (n <= 0)
		{
			Destroy_connection(ind, "read error");
			return n;
		}
		if (player)
		{
			p_ptr->options[i] = connp->Client_setup.options[i];
		}
	}

	/* Sync named options */
	if (player)
	{	
		sync_options(player);
		return sync_settings(player);
	}

	return 1;
}

static int Receive_suicide(int ind)
{
	connection_t *connp = &Conn[ind];
	int player, n;
	char ch;

	if (connp->id != -1)
		player = GetInd[connp->id];
	else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c", &ch)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	/* Commit suicide */
	do_cmd_suicide(player);

	return 1;
}

static int Receive_party(int ind)
{
	connection_t *connp = &Conn[ind];
	int player, n;
	char ch, buf[160];
	s16b command;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	if ((n = Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
	{
		switch (command)
		{
			case PARTY_CREATE:
			{
				party_create(player, buf);
				break;
			}

			case PARTY_ADD:
			{
				party_add(player, buf);
				break;
			}

			case PARTY_DELETE:
			{
				party_remove(player, buf);
				break;
			}

			case PARTY_REMOVE_ME:
			{
				party_leave(player);
				break;
			}

			case PARTY_HOSTILE:
			{
				add_hostility(player, buf);
				break;
			}

			case PARTY_PEACE:
			{
				remove_hostility(player, buf);
				break;
			}
		}
	}

	return 1;
}

static int Receive_clear(int ind)
{
	char ch;
	int n;
	connection_t *connp = &Conn[ind];

	/* Remove the clear command from the queue */
	if ((n = Packet_scanf(&connp->r, "%c", &ch)) != 1)
	{
		errno = 0;
		plog("Cannot receive clear packet");
		Destroy_connection(ind, "receive error");
		return -1;
	}

	/* Clear any queued commands prior to this clear request */
	Sockbuf_clear(&connp->q);

	return 2;
}


void Handle_direction(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	if (!dir)
	{
		p_ptr->current_spell = -1;
		p_ptr->current_rod = -1;
		p_ptr->current_activation = -1;
		return;
	}

	if (p_ptr->current_spell != -1)
	{
		if (p_ptr->ghost)
			do_cmd_ghost_power_aux(Ind, dir);
		else if (p_ptr->cp_ptr->spell_book == TV_MAGIC_BOOK)
			do_cmd_cast_aux(Ind, dir);
		else if (p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK)
			do_cmd_pray_aux(Ind, dir);
		else p_ptr->current_spell = -1;
	}
	else if (p_ptr->current_rod != -1)
		do_cmd_zap_rod_dir(Ind, dir);
	else if (p_ptr->current_activation != -1)
		do_cmd_activate_dir(Ind, dir);
}
		
void Handle_item(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];
	int i;
	bool ident = FALSE; /* scroll exposed itself */ 

	if (p_ptr->current_spell != -1) {
	    do_cmd_cast_aux(Ind, item);
	    return;
	}
	if ((p_ptr->current_enchant_h > 0) || (p_ptr->current_enchant_d > 0) ||
             (p_ptr->current_enchant_a > 0))
	{
		ident = enchant_spell_aux(Ind, item, p_ptr->current_enchant_h,
			p_ptr->current_enchant_d, p_ptr->current_enchant_a);
	}
	else if (p_ptr->current_identify)
	{
		ident = ident_spell_aux(Ind, item);
	}
	else if (p_ptr->current_star_identify)
	{
		ident = identify_fully_item(Ind, item);
	}
	else if (p_ptr->current_recharge)
	{
		ident = recharge_aux(Ind, item, p_ptr->current_recharge);
	}
    else if (p_ptr->current_artifact)
    {
		ident = create_artifact_aux(Ind, item);
    }

	if (p_ptr->current_scroll != -1) {
		do_cmd_read_scroll_end(Ind, p_ptr->current_scroll, ident);
	}
	if (p_ptr->current_staff != -1) {
		do_cmd_use_staff_discharge(Ind, p_ptr->current_staff, ident);
	}

	for (i = 0; i < INVEN_PACK; i++) inven_item_optimize(Ind, i);
}



/* receive a dungeon master command */
static int Receive_master(int ind)
{
	connection_t *connp = &Conn[ind];
	int player, n;
	char ch, buf[160];
	s16b command;

	if (connp->id != -1) player = GetInd[connp->id];
		else player = 0;

	/* Make sure this came from the dungeon master.  Note that it may be
	 * possobile to spoof this, so probably in the future more advanced
	 * authentication schemes will be neccecary. -APD
	 */

	if ((n = Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)) <= 0)
	{
		if (n == -1)
			Destroy_connection(ind, "read error");
		return n;
	}

	if (player)
	{
		switch (command)
		{
			case MASTER_LEVEL:
			{
				if (!(Players[player]->dm_flags & DM_LEVEL_CONTROL)) return 2;
				master_level(player, buf);
				break;
			}

			case MASTER_BUILD:
			{
				if (!(Players[player]->dm_flags & DM_CAN_BUILD)) return 2;
				master_build(player, buf);
				break;
			}

			case MASTER_SUMMON:
			{
				if (!(Players[player]->dm_flags & DM_CAN_SUMMON)) return 2;
				master_summon(player, buf);
				break;
			}
		
			case MASTER_GENERATE:
			{
				if (!(Players[player]->dm_flags & DM_CAN_GENERATE)) return 2;
				master_generate(player, buf);
				break;
			}
			
			case MASTER_PLAYER:
			{
				master_player(player, buf);
				break;
			}
		}
	}

	return 2;
}

