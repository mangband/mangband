#include "c-angband.h"

#include "../common/net-basics.h"
#include "../common/net-imps.h"

#define ONE_SECOND	1000000 /* 1 million "microseconds" */

/* Keepalive counters */
int sent_pings = 0;
int recd_pings = 0;

/* List heads */
eptr first_connection = NULL;
eptr first_listener = NULL;
eptr first_caller = NULL;
eptr first_timer = NULL;

/* Pointers */
eptr meta_caller = NULL;
eptr meta_connection = NULL;
eptr server_caller = NULL;
eptr server_connection = NULL;

connection_type *meta = NULL;
connection_type *serv = NULL;

/* Global Flags */
char *meta_buf;
int meta_buf_max;
s16b meta_connected = 0;
s16b connected = 0;
s16b state = 0;

static int		(*handlers[256])(connection_type *ct);
static cptr		(schemes[256]);

byte last_pkt; /* last_pkt is used for debug purposes only */ 
byte next_pkt;
cptr next_scheme;

server_setup_t serv_info;

byte indicator_refs[256]; /* PKT to ID: Indicators */
byte stream_ref[256]; ;/* PKT to ID: Streams */

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

	e_release_all(first_timer, 0, 1);
}

/* Iteration of the Loop */
void network_loop()
{
	//first_listener = handle_listeners(first_listener);
	first_connection = handle_connections(first_connection);
	first_caller = handle_callers(first_caller);
	first_timer = handle_timers(first_timer, static_timer(0));

	network_pause(1000); /* 0.001 ms "sleep" */
}

int client_close(int data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	/* 0_0` */
	quit(NULL);
	return 0;
}

int client_read(int data1, data data2) { /* return -1 on error */
	cq queue;
	connection_type *ct = (connection_type *)data2;

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
		last_pkt = next_pkt;

		/* Unable to continue */
		if (result != 1) break;
	}
#ifdef DEBUG
	if (result == -1) printf("Error parsing packet %d\n", last_pkt);
#endif
	/* Not enough bytes */
	if (result == 0) 
	{
		/* load */
		ct->rbuf.pos = start_pos;
	}
	/* Slide buffer to the left */
	cq_slide(&ct->rbuf);

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
	serv = (connection_type *)server_connection->data2;
	
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

	/* Early failure, probably DNS error */
	if (server_caller == NULL) return -1;

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

/* Timer */
int send_keepalive(u32b last_keepalive);
int keepalive_timer(int data1, data data2) {
	if (recd_pings == sent_pings) 
	{
		send_keepalive(sent_pings++);
		static_timer(1); //reset timer
	}
	//TODO: test this!
	else lag_mark = 10000;

	return 1;
}
void setup_keepalive_timer() 
{
	/* Create timer */
	eptr timer = add_timer(first_timer, ONE_SECOND, keepalive_timer);
	if (!first_timer) first_timer = timer;

	/* Reset counters */
	sent_pings = 0;
	recd_pings = 0;
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

int send_stream_size(byte id, int rows, int cols) {
	return cq_printf(&serv->wbuf, "%c%c%c%c", PKT_RESIZE, id, (byte)rows, (byte)cols);
}

int send_visual_info(byte type) {
	int	n, i, size;
	byte *attr_ref;
	char *char_ref;
	switch (type) 
	{
		case VISUAL_INFO_FLVR:
			size = MAX_FLVR_IDX;
			attr_ref = Client_setup.flvr_x_attr;
			char_ref = Client_setup.flvr_x_char;
			break;
		case VISUAL_INFO_F:
			size = z_info.f_max;
			attr_ref = Client_setup.f_attr;
			char_ref = Client_setup.f_char;
			break;
		case VISUAL_INFO_K:
			size = z_info.k_max;
 			attr_ref = Client_setup.k_attr;
  			char_ref = Client_setup.k_char;
  			break;
  		case VISUAL_INFO_R:
			size = z_info.r_max;
			attr_ref = Client_setup.r_attr;
			char_ref = Client_setup.r_char;
			break;
		case VISUAL_INFO_TVAL:
	 		size = 128;
			attr_ref = Client_setup.tval_attr;
			char_ref = Client_setup.tval_char;
			break;
		case VISUAL_INFO_MISC:
			size = 256;
			attr_ref =	Client_setup.misc_attr;
			char_ref =	Client_setup.misc_char;
			break;
		case VISUAL_INFO_PR:
			size = (z_info.c_max + 1) * z_info.p_max;
			attr_ref =	p_ptr->pr_attr;
			char_ref =	p_ptr->pr_char;
			break;
		default:
			return 0;
	}

	if (cq_printf(&serv->wbuf, "%c%c%d", PKT_VISUAL_INFO, type, size) <= 0)
	{
		return 0;
	}
	if (cq_printac(&serv->wbuf, RLE_NONE, attr_ref, char_ref, size) <= 0)
	{
		return 0;
	}

	return 1;
}

int send_options(void)
{
	byte next = 0;
	byte bit = 0;
	int i, n;

	if ((n = cq_printf(&serv->wbuf, "%c", PKT_OPTIONS)) < 0) return n;

	/* Pack each option as bit. Send every 8 options as byte */
	for (i = 0; i < options_max; i++)
	{
		if (p_ptr->options[i] == TRUE) next |= (1L << bit);
		bit++;
		if (bit > 7)
		{
			if ((n = cq_printf(&serv->wbuf, "%b", next)) < 0) return n;
			next = 0;
			bit = 0;
		}
	}

	/* Leftovers */
	if (bit != 0) 
	{
		if ((n = cq_printf(&serv->wbuf, "%b", next)) < 0) return n;
	}

	return 1;
}
int send_settings(void)
{
	byte next = 0;
	byte bit = 0;
	int i, n;

	if ((n = cq_printf(&serv->wbuf, "%c", PKT_SETTINGS)) < 0) return n;

	for (i = 0; i < 16; i++)
	{
		if ((n = cq_printf(&serv->wbuf, "%d", Client_setup.settings[i])) < 0) return n;
	}

	return 1;
}
int send_msg(cptr message)
{
	int n;

	if (view_channel != p_ptr->main_channel)
	{
		p_ptr->main_channel = view_channel;

		if ((n = send_channel(CHAN_SELECT, channels[view_channel].id, channels[view_channel].name)) <= 0)
			return n;
	}

	return cq_printf(&serv->wbuf, "%c%S", PKT_MESSAGE, message);
}

int send_interactive(byte type)
{
	return cq_printf(&serv->wbuf, "%c%c", PKT_TERM_INIT, type);
}
int send_term_key(char key)
{
	return cq_printf(&serv->wbuf, "%c%c", PKT_KEY, key);
}

int send_redraw(void)
{
	return cq_printf(&serv->wbuf, "%c", PKT_REDRAW);
}

int send_clear(void)
{
	return cq_printf(&serv->wbuf, "%c", PKT_CLEAR);
}

int send_channel(char mode, u16b id, cptr name)
{
	return cq_printf(&serv->wbuf, "%c%ud%c%s", PKT_CHANNEL, id, mode, name);
}

/* Gameplay commands */
int send_walk(char dir)
{
	return cq_printf(&serv->wbuf, "%c%c", PKT_WALK, dir);
}

int send_rest(void)
{
	return cq_printf(&serv->wbuf, "%c", PKT_REST);
}

int send_party(s16b command, cptr buf)
{
        return cq_printf(&serv->wbuf, "%c%d%s", PKT_PARTY, command, buf);
}

int send_suicide(void)
{
	return cq_printf(&serv->wbuf, "%c", PKT_SUICIDE);
}

int send_target_interactive(int mode, char dir)
{
	char c_mode;

	c_mode = (mode & TARGET_FRND ? NTARGET_FRND : (mode & TARGET_KILL ? NTARGET_KILL : NTARGET_LOOK));

	return cq_printf(&serv->wbuf, "%c%c%c", PKT_LOOK, c_mode, dir);
}

int send_locate(char dir)
{
	return cq_printf(&serv->wbuf, "%c" "%c", PKT_LOCATE, dir);
}

/* Custom command */
int send_custom_command(byte i, char item, char dir, s32b value, char *entry)
{
	custom_command_type *cc_ptr = &custom_command[i];
	int 	n;

	/* Command header */
	if (cc_ptr->pkt == (char)PKT_COMMAND)
		n = cq_printf(&serv->wbuf, "%c%c", PKT_COMMAND, i);
	else
		n = cq_printf(&serv->wbuf, "%c", cc_ptr->pkt);
	if (n <= 0) /* Error ! */
		return 0;

#define S_START case SCHEME_EMPTY: n = (1
#define S_WRITE(A) ); break; case SCHEME_ ## A: n = cq_printf(&serv->wbuf, ( CCS_ ## A ),
#define S_DONE ); break;

	/* Command body */
	switch (cc_ptr->scheme)
	{
		S_START \
		S_WRITE(ITEM)           	item\
		S_WRITE(DIR)            	dir\
		S_WRITE(VALUE)          	value\
		S_WRITE(SMALL)          	(byte)value\
		S_WRITE(STRING)         	entry\
		S_WRITE(CHAR)           	entry[0]\
		S_WRITE(DIR_VALUE)      	dir, value\
		S_WRITE(DIR_SMALL)      	dir, (byte)value\
		S_WRITE(DIR_STRING)     	dir, entry\
		S_WRITE(DIR_CHAR)       	dir, entry[0]\
		S_WRITE(VALUE_STRING)   	value, entry\
		S_WRITE(VALUE_CHAR)     	value, entry[0]\
		S_WRITE(SMALL_STRING)   	(byte)value, entry\
		S_WRITE(SMALL_CHAR)     	(byte)value, entry[0]\
		S_WRITE(ITEM_DIR)       	item, dir\
		S_WRITE(ITEM_VALUE)     	item, value\
		S_WRITE(ITEM_SMALL)     	item, (byte)value\
		S_WRITE(ITEM_STRING)    	item, entry\
		S_WRITE(ITEM_CHAR)      	item, entry[0]\
		S_WRITE(ITEM_DIR_VALUE) 	item, dir, value\
		S_WRITE(ITEM_DIR_SMALL) 	item, dir, (byte)value\
		S_WRITE(ITEM_DIR_STRING) 	item, dir, entry\
		S_WRITE(ITEM_DIR_CHAR)  	item, dir, entry[0]\
		S_WRITE(ITEM_VALUE_STRING)	item, value, entry\
		S_WRITE(ITEM_VALUE_CHAR) 	item, value, entry[0]\
		S_WRITE(ITEM_SMALL_STRING)	item, (byte)value, entry\
		S_WRITE(ITEM_SMALL_CHAR) 	item, (byte)value, entry[0]\
		S_WRITE(FULL)           	item, dir, value, entry\
		S_WRITE(PPTR_CHAR)         	entry[0]\
		S_DONE

	}
	if (n <= 0) /* Error ! */
		return 0;

#undef S_START
#undef S_WRITE
#undef S_DONE

	return 1;
}

int recv_store(void)
{
	int	
		price;
	char	
		name[1024];
	byte	
		pos,attr;
	s16b	
		wgt, num;

	if (cq_scanf(&serv->rbuf, "%c%c%d%d%ul%s", &pos, &attr, &wgt, &num, &price, name) < 6) return 0;
	

	store.stock[pos].sval = attr;
	store.stock[pos].weight = wgt;
	store.stock[pos].number = num;
	store_prices[(int) pos] = price;
	strncpy(store_names[(int) pos], name, 80);

	/* Make sure that we're in a store */
	if (shopping)
	{
		if (shopping_buying) inkey_exit = TRUE; /* Cancel input */
		display_inventory();
	}

	return 1;
}

int recv_store_info(void)
{
	s16b		
		num_items;
	s32b	
		max_cost;
	byte 	
		flag;

	if (cq_scanf(&serv->rbuf, "%c%s%s%d%l", &flag, store_name, store_owner_name, &num_items, &max_cost) < 5) return 0;
	

	store_flag = flag;
	store.stock_num = num_items;
	store_owner.max_cost = max_cost;

	/* Only enter "display_store" if we're not already shopping */
	if (!shopping)
		display_store();
	else
		display_inventory();

	return 1;
}
/* Undefined packet "handler" */
int recv_undef(connection_type *ct) {

	printf("Undefined packet %d came from server! Last packet was: %d\n", next_pkt, last_pkt);

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
#else
	/* make sure it's the same one we sent... */
	if (cticks == sent_pings - 1) 
	{
		if (state == PLAYER_PLAYING) 
		{
			micro time_passed = static_timer(1);
			time_passed /= 100;
			lag_mark = (s16b)time_passed;
			recd_pings++;
		} 
	}
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
	return 1;
}

int recv_basic_info(connection_type *ct) {

	if (cq_scanf(&ct->rbuf, "%b%b%b%b", &serv_info.val1, &serv_info.val2, &serv_info.val3, &serv_info.val4) < 4) 
	{
		/* Not enough bytes */
		return 0;
	}
	if (cq_scanf(&ct->rbuf, "%ul%ul%ul%ul", &serv_info.val9, &serv_info.val10, &serv_info.val11, &serv_info.val12) < 4) 
	{
		/* Not enough bytes */
		return 0;
	}

	z_info.k_max = serv_info.val9;
	z_info.r_max = serv_info.val10;
	z_info.f_max = serv_info.val11;

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
	if (mode == PLAYER_EMPTY) client_login();
	else state = mode;

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
	p_ptr->prace = (byte)race;
	p_ptr->pclass = (byte)pclass;
	p_ptr->male = (byte)sex;

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

	off = fake_name_size = fake_text_size = max = typ = 0;

	if (cq_scanf(&ct->rbuf, "%c%ud%ul%ul", &typ, &max, &fake_name_size, &fake_text_size) < 4)
	{
		/* Not ready */
		return 0;
	}

	/* Which struct */
	switch (typ)
	{
		/* Option groups */
		case STRUCT_INFO_OPTGROUP:
			/* Alloc */
			C_MAKE(option_group, max, cptr);
			options_groups_max = max;
			/* (Hack: max options) */
			C_MAKE(option_info, fake_name_size, option_type);
			options_max = fake_name_size;

			/* Fill */
			for (i = 0; i < max; i++)
			{
								
				if (cq_scanf(&ct->rbuf, "%s", &name) < 1)
				{
					return 0;
				}
				
				/* Transfer */
				option_group[i] = string_make(name);
			}
		break;
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

				my_strcpy(p_name + off, name, fake_name_size - off);
			
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
					return 0;
				}

				my_strcpy(c_name + off, name, fake_name_size - off);
			
				pc_ptr->name = off;
				/* Transfer other fields here */
				pc_ptr->spell_book = spell_book;
			}
		break;
		/* Inventory slots */
		case STRUCT_INFO_INVEN:
		{
			s16b last_off = -1;
			/* Alloc */
			C_MAKE(eq_name, fake_name_size, char);
			C_MAKE(eq_names, max, s16b);
			C_MAKE(inventory_name, max, char*);
			C_MAKE(inventory, max, object_type);
			INVEN_TOTAL = max;
			INVEN_WIELD = fake_text_size;
			
			/* Read extra */
			fake_text_size = 0;
			if (cq_scanf(&ct->rbuf, "%ul", &fake_text_size) < 1)
			{
				return 0;
			}
			INVEN_PACK = fake_text_size;

			/* Fill */
			for (i = 0; i < max; i++) 
			{
				C_MAKE(inventory_name[i], 80, char);

				off = 0;

				if (cq_scanf(&ct->rbuf, "%s%ul", &name, &off) < 2)
				{
					return n;
				}

				if (last_off != off)
				{
					printf("Saving %s at pos %d\n", name, off);
					my_strcpy(eq_name + off, name, fake_name_size - off);
				} else printf("Not Saving %s at pos %d\n", name, off);

				eq_names[i] = last_off = (s16b)off;
			}
		}		
	}	
	
	return 1;
}

int recv_option_info(connection_type *ct) 
{
	option_type *opt_ptr;
	int n;

	char desc[MAX_CHARS];
	char name[MAX_CHARS];
	byte
		opt_page = 0;

	if (cq_scanf(&ct->rbuf, "%c%s%s", &opt_page, name, desc) < 3)
	{
		return 0;
	}

	/* Grab */
	opt_ptr = &option_info[known_options];

	/* Fill */
	opt_ptr->o_page = opt_page;
	opt_ptr->o_text = string_make(name);
	opt_ptr->o_desc = string_make(desc);
	opt_ptr->o_set = 0;

	/* Link to local */
	for (n = 0; local_option_info[n].o_desc; n++)
	{ 
		if (!strcmp(local_option_info[n].o_text, name))
		{
			local_option_info[n].o_set = known_options;
			opt_ptr->o_set = n;
		}
	}

	known_options++;

	return 1;
}

/* Read and update specific indicator */
int recv_indicator(connection_type *ct) {

	indicator_type *i_ptr; 
	int id = indicator_refs[next_pkt];
	int i, coff;

	signed char tiny_c;
	s16b normal_c;
	s32b large_c;
	char* text_c;

	/* Error -- unknown indicator */
	if (id > known_indicators) return -1;

	i_ptr = &indicators[id];
	coff = coffer_refs[id];

	/* Read (i_ptr->amnt) values of type (i_ptr->type) */
	for (i = 0; i < i_ptr->amnt; i++) 
	{
		/* Read */
		s32b val = 0, n = 0;
		if (i_ptr->type  == INDITYPE_TINY)
		{
			n = cq_scanf(&ct->rbuf, "%c", &tiny_c);
			val = (s32b)tiny_c;
		} 
		else if (i_ptr->type == INDITYPE_NORMAL)
		{
			n = cq_scanf(&ct->rbuf, "%d", &normal_c);
			val = (s32b)normal_c;
		}
		else if (i_ptr->type == INDITYPE_LARGE)
		{
			n = cq_scanf(&ct->rbuf, "%l", &large_c);
			val = (s32b)large_c;
		}

		/* Error ? */
		if (n < 1) return 0;

		/* Save */
		coffers[coff + i] = val;
	}

	/* Schedule redraw */
	p_ptr->redraw |= i_ptr->redraw;

	/* Update *all* windows that have this indicator */
	for (i = 0; i < known_indicators; i++)
		if (indicators[i].redraw == i_ptr->redraw)
			p_ptr->window |= indicator_window[i];

	return 1;
}

int recv_indicator_str(connection_type *ct) {
	indicator_type *i_ptr;
	int id = indicator_refs[next_pkt];
	char buf[MAX_CHARS]; 

	/* Error -- unknown indicator */
	if (id > known_indicators) return -1;

	i_ptr = &indicators[id];

	/* Read the string */
	if (cq_scanf(&ct->rbuf, "%s", buf) < 1) return 0;

	/* Store the string in indicator's 'prompt' */
	my_strcpy((char*)str_coffers[id], buf, MAX_CHARS);

	/* Schedule redraw */
	p_ptr->redraw |= i_ptr->redraw;

	/* Update *all* windows that have this indicator */
	for (i = 0; i < known_indicators; i++)
		if (indicators[i].redraw == i_ptr->redraw)
			p_ptr->window |= indicator_window[i];

	return 1;
}

/* Learn about certain indicator from server */
int recv_indicator_info(connection_type *ct) {
	byte
		pkt = 0,
		type = 0,
		amnt = 0,
		win = 0;
	char buf[MSG_LEN]; //TODO: check this 
	char mark[MAX_CHARS];
	s16b row = 0,
		col = 0;
	u32b flag = 0;
	int n;

	indicator_type *i_ptr;

	if (cq_scanf(&ct->rbuf, "%c%c%c%c%d%d%ul%S%s", &pkt, &type, &amnt, &win, &row, &col, &flag, buf, mark) < 9) return 0;

	/* Check for errors */
	if (known_indicators >= MAX_INDICATORS)
	{
		plog("No more indicator slots! (MAX_INDICATORS)");
		return -1;
	}
	if (known_coffers + amnt + 1 >= MAX_COFFERS)
	{
		plog("Not enougth coffer slots! (MAX_COFFERS)");
		return -1;
	}

	/* Get it */
	i_ptr = &indicators[known_indicators];

	i_ptr->pkt = pkt;
	i_ptr->type = type;
	i_ptr->amnt = amnt;
	i_ptr->redraw = (1L << (known_indicators));
	i_ptr->row = row;
	i_ptr->col = col;
	i_ptr->flag = flag;
	i_ptr->win = win;

	i_ptr->mark = strdup(mark);

	n = strlen(buf) + 1;
	C_MAKE(i_ptr->prompt, n, char);
	my_strcpy((char*)i_ptr->prompt, buf, n);

	str_coffers[known_indicators] = NULL;

	/* Set local window_flag */
	/* TODO: make it a ref. array in c-tables */
	indicator_window[known_indicators] = 0;
	if (win & IPW_1) indicator_window[known_indicators] |= PW_PLAYER_2;
	if (win & IPW_2) indicator_window[known_indicators] |= PW_STATUS;
	if (win & IPW_3) indicator_window[known_indicators] |= PW_PLAYER_0;
	if (win & IPW_4) indicator_window[known_indicators] |= PW_PLAYER_3;
	if (win & IPW_5) indicator_window[known_indicators] |= PW_PLAYER_1;

	/* Indicator takes place of a PKT */
	if (pkt)
	{
		handlers[pkt] = ((type != INDITYPE_STRING) ? recv_indicator : recv_indicator_str);
		schemes[pkt] = NULL; /* HACK */

		indicator_refs[pkt] = known_indicators;
		coffer_refs[known_indicators] = known_coffers;

		/* Indicator has it's own string coffer */
		if (type == INDITYPE_STRING)
		{
			C_MAKE(str_coffers[known_indicators], MAX_CHARS, char);
			str_coffers[known_indicators][0] = '\0';
		}
	}
	/* A 'hollow' indicator, which is just a clone */
	else
	{
		/* Note: in indicator clones "type" is used as source indicator id,
		 * and "amnt" is used as coffer sub-offset. For example:
		 * Indicator [0, 12, 2] is a clone (pkt "0"), refers indicator 12 ("12"),
		 * and uses indicator 12's coffer 1 ("2" - const 1). */
		unsigned int ind = type;
		unsigned int offset = amnt - 1;
		/* Perform it's own error-checking. */
		if (ind >= known_indicators)
		{
			plog("Attempting to clone indicator too far!");
			return -1;
		}
		if (indicators[ind].type == INDITYPE_STRING)
		{
			/* Allow string indicators to be flaky about regular coffers */
			offset = 0;
		}
		else if (offset >= indicators[ind].amnt)
		{
			plog("Attempting to clone coffer too far!");
			return -1;
		}
		i_ptr->type = indicators[ind].type;	/* Actual type is cloned */
		i_ptr->amnt = indicators[ind].amnt - offset; /* Actual ammount is source_ammount - offset */
		str_coffers[known_indicators] = str_coffers[ind]; /* String pointer is source string */
		i_ptr->redraw = indicators[ind].redraw;
		coffer_refs[known_indicators] = coffer_refs[ind] + offset;

		amnt = 0;
	}

	known_coffers += amnt;
	known_indicators++;

	/* Register a possible local overload */
	register_indicator(known_indicators - 1);

	return 1;
}

/* ... */
int read_stream_char(byte st, byte addr, bool trn, bool mem, s16b y, s16b x)
{
	int 	n;
	byte
		a = 0,
		ta = 0;
	char
		c = 0,
		tc = 0;

	cave_view_type *dest = stream_cave(st, y);

	if (cq_scanf(&serv->rbuf, "%c%c", &a, &c) < 2) return 0;

	if (trn && cq_scanf(&serv->rbuf, "%c%c", &ta, &tc) < 2) return 0;

	dest[x].a = a;
	dest[x].c = c;

	if (y > last_remote_line[addr]) 
		last_remote_line[addr] = y; 

	if (addr == NTERM_WIN_OVERHEAD)
		show_char(y, x, a, c, ta, tc, mem);

	return 1;
}
int recv_stream(connection_type *ct) {
	s16b	cols, y = 0;
	s16b	*line;
	byte	addr, id;
	cave_view_type	*dest;

	stream_type 	*stream;

	if (cq_scanf(&ct->rbuf, "%d", &y) < 1) return 0;

	id = stream_ref[next_pkt];
	stream = &streams[id];
	addr = stream->addr;

	/* Hack -- single char */
	if (y & 0xFF00)	return 
		read_stream_char(id, addr, (stream->flag & SF_TRANSPARENT), !(stream->flag & SF_OVERLAYED), (y & 0x00FF), (y >> 8)-1 );

	if (y > p_ptr->stream_hgt[id]) 
	{
		plog("Stream out of bounds");
		return -1;
	}

	cols = p_ptr->stream_wid[id];
	dest = p_ptr->stream_cave[id] + y * cols;
 	line = &last_remote_line[addr];

	/* Decode the secondary attr/char stream */
	if ((stream->flag & SF_TRANSPARENT))
	{
		if (cq_scanc(&ct->rbuf, stream->rle, p_ptr->trn_info[y], cols) < cols) return 0;
	}
	/* OR clear it ! */ 
	else if (stream->flag & SF_OVERLAYED)
		caveclr(p_ptr->trn_info[y], cols);

	/* Decode the attr/char stream */
	if (cq_scanc(&ct->rbuf, stream->rle, dest, cols) < cols) return 0;

	/* Check the min/max line count */
	if ((*line) < y)
		(*line) = y;
	/* TODO: test this approach -- else if (y == 0) (*line) = 0; */

	/* Put data to screen ? */		
	if (addr == NTERM_WIN_OVERHEAD)
		show_line(y, cols, !(stream->flag & SF_OVERLAYED));

	return 1;
}

int recv_stream_size(connection_type *ct) {
	byte
		stg = 0,
		x = 0,
		y = 0;
	byte	st, addr;

	if (cq_scanf(&ct->rbuf, "%c%c%c", &stg, &y, &x) < 3) return 0;

	/* Ensure it is valid and start from there */
	if (stg >= known_streams) { printf("invalid stream %d (known - %d)\n", stg, known_streams); return 1;}

	/* Fetch target "window" */	
	addr = streams[stg].addr;

	/* (Re)Allocate memory */
	if (remote_info[addr])
	{
		KILL(remote_info[addr]);
	}
	C_MAKE(remote_info[addr], (y+1) * x, cave_view_type);
	last_remote_line[addr] = 0;

	/* Affect the whole group */
	for (st = stg; st < known_streams; st++)
	{
		/* Stop when we move on to the next group */
		if (streams[st].addr != addr) break;

		/* Save new size */
		p_ptr->stream_wid[st] = x;
		p_ptr->stream_hgt[st] = y;

		/* Save pointer */
		p_ptr->stream_cave[st] = remote_info[addr];
	}

	/* HACK - Dungeon display resize */
	if (addr == NTERM_WIN_OVERHEAD)
	{
		/* Redraw status line */
		Term_erase(0, y-1, x);
		p_ptr->redraw |= PR_STATUS;
		/* Redraw compact */
		p_ptr->redraw |= PR_COMPACT;
	}


	return 1;
}

/* Learn about certain stream from server */
int recv_stream_info(connection_type *ct) {
	byte
		pkt = 0,
		addr = 0,
		flag = 0,
		rle = 0;
	byte
		min_col = 0,
		min_row = 0,
		max_col = 0,
		max_row = 0;
	char buf[MSG_LEN]; //TODO: check this 
	int n;

	stream_type *s_ptr;

	buf[0] = '\0';

	if (cq_scanf(&ct->rbuf, "%c%c%c%c%s%c%c%c%c", &pkt, &addr, &rle, &flag, buf, 
			&min_row, &min_col, &max_row, &max_col) < 9) return 0;

	/* Check for errors */
	if (known_streams >= MAX_STREAMS)
	{
		plog("No more stream slots! (MAX_STREAMS)");
		return -1;
	}

	/* Get it */
	s_ptr = &streams[known_streams];
	WIPE(s_ptr, stream_type);

	s_ptr->pkt = pkt;
	s_ptr->addr = addr;	
	s_ptr->rle = rle;

	s_ptr->flag = flag;

	/*s_ptr->scr = (!addr ? p_ptr->scr_info : remote_info[addr] );
	s_ptr->trn = (!trn ? NULL : p_ptr->trn_info);*/

	s_ptr->min_row = min_row;
	s_ptr->min_col = min_col;
	s_ptr->max_row = max_row;
	s_ptr->max_col = max_col;

	if (!STRZERO(buf))
	{
		s_ptr->mark = strdup(buf);
	}


	handlers[pkt] = recv_stream;
	schemes[pkt] = NULL; /* HACK */

	stream_ref[pkt] = known_streams;

	known_streams++;	


	return 1;
}

/* Network/Terminals */
int recv_term_info(connection_type *ct) {
	byte win,
		flag = 0;
	u16b
		line = 0;

	if (cq_scanf(&ct->rbuf, "%b", &flag) < 1) return 0;

	/* For principal modes, grab additional parameter */
	if ((flag & 0xF0) && cq_scanf(&ct->rbuf, "%ud", &line) < 1) return 0;

	/* Change terminal id */
	if (flag & NTERM_ACTIVATE)
	{
		p_ptr->remote_term = (byte)line;
		line = 0; /* Paranoia - reset line just in case */
	}

	/* Grab terminal id */
	win = p_ptr->remote_term;

	/* Reset counter */	
	if (flag & NTERM_CLEAR)
	{
		last_remote_line[win] = 0;
	}

	/* Refresh window */
	if (flag & NTERM_FRESH)
	{
		p_ptr->window |= streams[window_to_stream[win]].window_flag;
	}

	/* Icky test */
	if ((flag & NTERM_ICKY) && !screen_icky) return 1;

	/* Change terminal state */	
	if (flag & NTERM_HOLD)
	{
		if (line == NTERM_ESCAPE)
		{
			inkey_exit = TRUE;
		}
		if (line == NTERM_PUSH && screen_icky)
		{
			icky_levels++;
		}
		if (line == NTERM_PULL && icky_levels)
		{
			icky_levels--;
		}
	}

	/* Copy nterm contents to screen */
	if (flag & NTERM_FLUSH)
	{
		u16b wid, hgt, off, n;
		byte st;

		off = 0;
		st = window_to_stream[win];
		hgt = last_remote_line[win] + 1;
		wid = p_ptr->stream_wid[st];

		/* HACK: */
		if (!win) off = DUNGEON_OFFSET_X;

		for (n = line; n < hgt; n++)
		{
			caveprt(stream_cave(st, n), wid, off, n );
		}
	}

	/* Browse NTerm contents locally */
	if (flag & NTERM_BROWSE)
	{
		show_peruse(line);
	}
	/* Pop-up NTerm contents */
	if (flag & NTERM_POP)
	{
		show_popup();
	}
	/* Clear screen */
	if (flag & NTERM_CLEAR)
	{
		Term_clear();
	}
	/* Refresh screen */
	if (flag & NTERM_FRESH)
	{
		Term_fresh();
	}
	return 1;
}
int recv_term_header(connection_type *ct) {
	char buf[80];

	if (cq_scanf(&ct->rbuf, "%s", buf) < 1) return 0;

	/* Save header */
	strcpy(special_line_header, buf);

	/* Enable perusal mode */
	special_line_type = TRUE;

	/* Ignore it if we're busy */
	if (screen_icky || looking) return 1;

	/* Prepare popup route */
	special_line_requested = TRUE;

	/* NOTE! WE NOW BREAK THE NETWORK CYCLE! */
	return 2;
}

int recv_cursor(connection_type *ct) {
	char vis, x, y;

	if (cq_scanf(&ct->rbuf, "%c%c%c", &vis, &x, &y) < 3) return 0;

	if (cursor_icky)
	{
		x += DUNGEON_OFFSET_X;
		Term_consolidate_cursor(vis, x, y);
	}

	return 1;
}

int recv_target_info(connection_type *ct) {
	char x, y, buf[80], *s;
	byte win;

	if (cq_scanf(&ct->rbuf, "%c%c%c%s", &x, &y, &win, buf) < 4) return 0;

	if (!looking) return 1;

	/* Hack -- information recall */
	s = strchr(buf, '['); /* Store prompt starting at '[' character */
	show_recall(win, s); /* Show/Hide recall window */

	if (!target_recall)
	{
		prt(buf, 0, 0);
	}

	/* Move the cursor */
	if (cursor_icky)
	{
		x += DUNGEON_OFFSET_X;
		Term_consolidate_cursor(TRUE, x, y);
	}

	return 1;
}

int recv_channel(connection_type *ct) {
	u16b
		id;
	char
		mode,
		name[MAX_CHARS];
	
	if (cq_scanf(&ct->rbuf, "%ud%c%s", &id, &mode, name) < 3) return 0;

	switch (mode)
	{
		case CHAN_JOIN:

			do_chat_open(id, name);

		break;
		case CHAN_SELECT:

			do_chat_select(id);

		break;
		case CHAN_LEAVE:

			do_chat_close(id);

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

	do_handle_message(mesg, type);

	return 1;
}

int recv_message_repeat(connection_type *ct) {

	char 
		mesg[80];
	u16b 
		type = 0;

	if (cq_scanf(&ct->rbuf, "%ud", &type) < 1) return 0;

	strcpy(mesg, message_last());

	do_handle_message(mesg, type);

	return 1;
}

int recv_sound(connection_type *ct)
{
	char 
		sound;

	if (cq_scanf(&ct->rbuf, "%c", &sound) < 1) return 0;

	/* Make a sound (if allowed) */
	if (use_sound) Term_xtra(TERM_XTRA_SOUND, sound);

	return 1;
}

int recv_custom_command_info(connection_type *ct) {
	byte
		pkt = 0,
		tval = 0,
		scheme = 0;
	char buf[MSG_LEN]; //TODO: check this 
	s16b m_catch = 0;
	u32b flag = 0;
	int n;

	custom_command_type *cc_ptr;

	if (cq_scanf(&ct->rbuf, "%c%c%d%ul%c%S", &pkt, &scheme, &m_catch, &flag, &tval, buf) < 6) return 0;

	/* Check for errors */
	if (known_indicators >= MAX_CUSTOM_COMMANDS)
	{
		plog("No more command slots! (MAX_CUSTOM_COMMANDS)");
		return -1;
	}
	if (scheme >= MAX_SCHEMES)
	{
		plog("Undefined CC scheme!");
		return -1;
	}

	/* Get it */
	cc_ptr = &custom_command[custom_commands];
	WIPE(cc_ptr, custom_command_type);

	cc_ptr->m_catch = m_catch;
	cc_ptr->pkt = pkt;
	cc_ptr->scheme = scheme;
	cc_ptr->flag = flag;
	cc_ptr->tval = tval;

	/* Replace \n with \0 before applying to ->prompt */
	for (n = 0; n < strlen(buf)+1; n++) 
	{
		if (buf[n] == '\n') buf[n] = '\0';
	}
	my_strcpy(cc_ptr->prompt, buf, sizeof(cc_ptr->prompt));

	custom_commands++;

	return 1;
}

int recv_item_tester_info(connection_type *ct) {
	byte
		id = 0,
		flag = 0,
		tval = 0;
	int n;

	item_tester_type *it_ptr;

	if (cq_scanf(&ct->rbuf, "%c%c", &id, &flag) < 2) return 0;

	/* Check for errors */
	if (id >= MAX_ITEM_TESTERS)
	{
		plog("No more item_tester slots! (MAX_ITEM_TESTERS)");
		return -1;
	}

	/* Get it */
	it_ptr = &item_tester[id];
	WIPE(it_ptr, item_tester_type);

	it_ptr->flag = flag;

	for (n = 0; n < MAX_ITH_TVAL; n++) 
	{
		tval = 0;
		if (cq_scanf(&ct->rbuf, "%c", &tval) < 1) return 0;
		it_ptr->tval[n] = tval;
	}

	if (id + 1 > known_item_testers) known_item_testers = id + 1;

	return 1;
}


int recv_floor(connection_type *ct)
{
	int	n;
	char	ch;
	byte tval, attr;
	byte flag;
	s16b amt;
	char name[80];

	if (cq_scanf(&ct->rbuf, "%c%d%c%c%s", &attr, &amt, &tval, &flag, name) < 5)
	{
		return 0;
	}

	/* Remember for later */
	floor_item.sval = attr; /* Hack -- Store "attr" in "sval" */
	floor_item.tval = tval;
	floor_item.ident = flag; /* Hack -- Store "flag" in "ident" */
	floor_item.number = amt;

	strncpy(floor_name, name, 79);
	fix_floor();
	return 1;
}

int recv_inven(connection_type *ct)
{
	int	n;
	char	ch;
	char pos, attr, tval;
	byte flag;
	s16b wgt, amt;
	char name[80];

	if (cq_scanf(&ct->rbuf, "%c%c%ud%d%c%c%s", &pos, &attr, &wgt, &amt, &tval, &flag, name) < 7)
	{
		return 0;
	}

	/* Hack -- The color is stored in the sval, since we don't use it for anything else */
	inventory[pos - 'a'].sval = attr;
	inventory[pos - 'a'].tval = tval;
	inventory[pos - 'a'].ident = flag;
	inventory[pos - 'a'].weight = wgt;
	inventory[pos - 'a'].number = amt;

	strncpy(inventory_name[pos - 'a'], name, 79);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	return 1;
}

int recv_equip(connection_type *ct)
{
	int	n;
	char 	ch;
	char pos, attr, tval;
	byte flag;
	s16b wgt;
	char name[80];

	if (cq_scanf(&ct->rbuf, "%c%c%ud%c%c%s", &pos, &attr, &wgt, &tval, &flag, name) < 6)
	{
		return 0;
	}

	inventory[pos - 'a' + INVEN_WIELD].sval = attr;
	inventory[pos - 'a' + INVEN_WIELD].tval = tval;
	inventory[pos - 'a' + INVEN_WIELD].ident = flag;
	inventory[pos - 'a' + INVEN_WIELD].weight = wgt;
	inventory[pos - 'a' + INVEN_WIELD].number = 1;


	strncpy(inventory_name[pos - 'a' + INVEN_WIELD], name, 79);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP);

	return 1;
}

int recv_spell_info(connection_type *ct)
{
	char
		flag;
	u16b
		book,
		line;
	char buf[MSG_LEN];//TODO: verify this

	if (cq_scanf(&ct->rbuf, "%c%ud%ud%s", &flag, &book, &line, buf) < 4)
	{
		return 0;
	}

	if (line >= SPELLS_PER_BOOK)
	{
		plog(format("Spell out of bounds! Getting %d, SPELLS_PER_BOOK=%d!", line, SPELLS_PER_BOOK));
		return -1;
	}

	/* Save the info */
	strcpy(spell_info[book][line], buf);
	spell_flag[book * SPELLS_PER_BOOK + line] = flag;

	/* and wipe the next line */
	spell_info[book][line+1][0] = '\0';

	/* Update spell list */
	p_ptr->window |= PW_SPELL;

	return 1;
}

int recv_objflags(connection_type *ct)
{
	s16b
		y = 0;
	byte rle = ( use_graphics ? RLE_LARGE : RLE_CLASSIC );

	/* Header (line number) */
	if (cq_scanf(&ct->rbuf, "%d", &y) < 1)
	{
		return 0;
	}

	/* Body (39 grids of cave) */
	if (cq_scanc(&ct->rbuf, rle, p_ptr->hist_flags[y], 39) < 39)
	{ 
		return 0;
	}

	//TODO: re-evalute those
	/* Update relevant displays */
	p_ptr->redraw |= PR_EQUIPPY;
	p_ptr->window |= (PW_PLAYER_1);

	/* Ok */
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


/* META-SERVER STUFF */
int meta_close(int data1, data data2) {
	connection_type *ct = (connection_type*)data2;
	memcpy(meta_buf, ct->rbuf.buf, MIN(ct->rbuf.len, meta_buf_max));
	meta_connected = ct->rbuf.len;
	return 0;
}
int meta_read(int data1, data data2) { /* return -1 on error */
	connection_type *ct = (connection_type *)data2;
	return 0;
}
int connected_to_meta(int data1, data data2) {
	int fd = (int)data1;

	/* Unset 'caller' */
	meta_caller = NULL;

	/* Setup 'connection' */
	meta_connection = add_connection(first_connection, fd, meta_read, meta_close);
	if (!first_connection) first_connection = meta_connection;

	/* Set usability pointer */
	meta = (connection_type *)meta_connection->data2;

	/* OK */
	meta_connected = 1;
	return 1;
}

/* Return 1 to continue, 0 to cancel */
int failed_connection_to_meta(int data1, data data2) {
	/* NOT OK */
	meta_connected = -1;
	return 0;
}

int call_metaserver(char *server_name, int server_port, char *buf, int buflen)
{
	meta_caller = add_caller(first_caller, server_name, server_port, connected_to_meta, failed_connection_to_meta);
	if (first_caller == NULL) first_caller = meta_caller;

	/* Early failure, probably DNS error */
	if (meta_caller == NULL) return -1;

	/* Unset */
	meta_connected = 0;

	meta_buf = buf;
	meta_buf_max = buflen;

	/* Try */
	while (!meta_connected)
	{
		network_loop();
		network_pause(100000); /* 0.1 ms "sleep" */
	}
	/* Will be either 1 either -1 */

	/* Now let's try reading */
	if (meta_connected == 1)
	{
		meta_connected = 0;
		while (!meta_connected)
		{
			network_loop();
		}
	}
	/* Will be either 2 either 1 either -1 */

	return meta_connected;
}
/* END OF META-SERVER STUFF */


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

	/* Hack -- if stream is hidden from UI, auto-subscribe..? */
	for (k = 0; k < stream_groups; k++)
	{
		byte st = stream_group[k];
		stream_type* st_ptr = &streams[st];

		if (st_ptr->flag & SF_HIDE)
		{
			st_x[st] = st_ptr->min_col;
			st_y[st] = st_ptr->min_row;
		}
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

			/* Hack -- if stream is hidden from UI, don't touch it */
			if (st_ptr->flag & SF_HIDE) continue;

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
			send_stream_size(j, st_y[j], st_x[j]);

			/* Toggle update flag */
			st_flag |= streams[j].window_flag;
		}
	}

	/* Return update flags */
	return st_flag;
}
/* Helper caller for "net_term_manage" */
u32b net_term_update(bool clear) { return net_term_manage(window_flag, window_flag, clear); }
