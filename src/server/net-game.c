/*
 * MAngband Network protocol
 *
 * Copyright (c) 2010 MAngband Project Team.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of the "Angband licence" with an extra clause:
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply. 
 * +  You are not allowed to use this code in software which is not
 *    distributed in source form.
 */
#include "mangband.h"
#include "net-server.h"

static int		(*pcommands[256])(player_type *p_ptr);

int send_play(connection_type *ct, byte mode) 
{
	return cq_printf(&ct->wbuf, "%c%b", PKT_PLAY, mode);
}

int send_quit(connection_type *ct, char *reason) 
{
	printf("Sending quit msg: %s\n", reason);
	return cq_printf(&ct->wbuf, "%c%S", PKT_QUIT, reason);
}

int send_server_info(connection_type *ct)
{
	if (!cq_printf(&ct->wbuf, "%c%d%d%d%d", PKT_BASIC_INFO, serv_info.val1, serv_info.val2, serv_info.val3, serv_info.val4))
	{
		client_withdraw(ct);
	}
	
	return 1;
}

int send_char_info(connection_type *ct, player_type *p_ptr)
{
	return cq_printf(&ct->wbuf, "%c%d%d%d%d", PKT_CHAR_INFO, p_ptr->state, p_ptr->prace, p_ptr->pclass, p_ptr->male);
}

int send_race_info(connection_type *ct)
{
	u32b i, name_size;

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_RACE) <= 0)
	{
		client_withdraw(ct);
	}
	
	name_size = p_info[z_info->p_max-1].name + strlen(p_name + p_info[z_info->p_max-1].name);
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", z_info->p_max, name_size, z_info->fake_text_size) <= 0)
	{
		client_withdraw(ct);
	} 

	for (i = 0; i < z_info->p_max; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s%ul",  p_name + p_info[i].name, p_info[i].name) <= 0)
		{
			client_withdraw(ct);
		}
	}
	return 1;
}

int send_class_info(connection_type *ct)
{
	u32b i, name_size;

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_CLASS) <= 0)
	{
		client_withdraw(ct);
	}
	
	name_size = c_info[z_info->c_max-1].name + strlen(c_name + c_info[z_info->c_max-1].name);
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", z_info->c_max, name_size, z_info->fake_text_size) <= 0)
	{
		client_withdraw(ct);
	} 

	for (i = 0; i < z_info->c_max; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s%ul%c",  c_name + c_info[i].name, c_info[i].name, c_info[i].spell_book) <= 0)
		{
			client_withdraw(ct);
		}
	}

	return 1;	
}

int send_message(int Ind, cptr msg, u16b typ)
{
	connection_type *ct = PConn[Ind];
	char buf[80];

	if (!ct) return -1;

	if (msg == NULL)
		return 1;

	/* Clip end of msg if too long */
	strncpy(buf, msg, 78);
	buf[78] = '\0';

	return cq_printf(&ct->wbuf, "%c%ud%s", PKT_MESSAGE, typ, buf);
}


int recv_message(connection_type *ct, player_type *p_ptr)
{
	char buf[1024];

	int Ind = Get_Ind[p_ptr->conn];

	buf[0] = '\0';

	if (cq_scanf(&ct->rbuf, "%S", buf) < 1)
	{
		return 0;
	}

	player_talk(Ind, buf);

	return 1;
}


/* Default handler for all the gameplay commands. */
int recv_command(connection_type *ct, player_type *p_ptr) 
{
	/* Write header */
	cq_printf(&p_ptr->cbuf, "%c", next_pkt);

	/* Copy command to player's "command buffer" */
	if (cq_copyf(&ct->rbuf, next_scheme, &p_ptr->cbuf) <= 0)
	{
		/* Unable to... */
		return -1;
	}

	/* OK */
	return 1;
}

/* Undefined packet "handler" */
static int recv_undef(connection_type *ct, player_type *p_ptr) 
{ 
	client_abort(ct, "Undefined packet!");
}

int recv_keepalive(connection_type *ct, player_type *p_ptr) 
{
	s32b ctime;

	if (cq_scanf(&ct->rbuf, "%l", &ctime) < 1)
	{
		/* Not enough bytes */
		return 0;
	}

	cq_printf(&ct->wbuf, "%c%l", PKT_KEEPALIVE, ctime);

	/* Ok */
	return 1;
}

int recv_play(connection_type *ct, player_type *p_ptr) 
{
	byte 
		mode = 0;
	if (cq_scanf(&ct->rbuf, "%c", &mode) < 1) 
	{
		/* Not enough bytes */
		return 0;
	}

	if (p_ptr == NULL || p_ptr->state < PLAYER_BONE)
	{
		/* ....? Some kind of error */
	}

	/* Ready for rolling */
	if (p_ptr->state == PLAYER_SHAPED)
	{
		player_birth(ct->user, p_ptr->prace, p_ptr->pclass, p_ptr->male, p_ptr->stat_order);
		p_ptr->state = PLAYER_FULL;
	}

	/* Imagine we're good to go */
	if (p_ptr->state == PLAYER_FULL)
	{
		player_enter(ct->user);
	}

	return 1;
}

int recv_basic_request(connection_type *ct, player_type *p_ptr) {
	char mode;
	u16b id;

	if (cq_scanf(&ct->rbuf, "%c%ud", &mode, &id) < 2) 
	{
		/* Not enough bytes */
		return 0;
	}

	switch (mode) 
	{
		/* TODO: Populate with actual info sendouts */
		default: break;
	}

	return 1;
}

int recv_char_info(connection_type *ct, player_type *p_ptr) {
	int i;
	
	/* TODO: Ensure p_ptr->state is correct of char_info and bail if not */
	//return -1;

	if (cq_scanf(&ct->rbuf, "%d%d%d", &p_ptr->prace, &p_ptr->prace, &p_ptr->male) < 3) 
	{
		/* Not enough bytes */
		return 0;
	}

	/* Read the stat order */
	for (i = 0; i < 6; i++)
	{
		p_ptr->stat_order[i] = 0;
		if (cq_scanf(&ct->rbuf, "%d", &p_ptr->stat_order[i]) < 1) 
		{
			/* Not enough bytes */
			return 0;
		}
	}

	/* Have Template */
	p_ptr->state = PLAYER_SHAPED;

	/* Ready for rolling */
	if (p_ptr->state == PLAYER_SHAPED)
	{
		player_birth(ct->user, p_ptr->prace, p_ptr->pclass, p_ptr->male, p_ptr->stat_order);
		p_ptr->state = PLAYER_FULL;
	}

	/* Inform client */
	send_char_info(ct, p_ptr);

	return 1;
}

/** Gameplay commands **/
/* Those return 
	* -1 on critical error
	*  0 when lack energy
	*  1 or 2 on success
	*  2 when all energy was drained (break loop)
*/
static int recv_walk(player_type *p_ptr) {
	int Ind = Get_Ind[p_ptr->conn];

	char
		dir;
	/* The only critical error I can think of is broken queue */
	if (cq_scanf(&p_ptr->cbuf, "%c", &dir) < 1)
	{
		/* Impossible! :( */
		return -1;
	}

	/* Check energy */
	if (p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		/* Actually walk */
		do_cmd_walk(Ind, dir, option_p(p_ptr,ALWAYS_PICKUP));

		/* End turn */
		return 2;
	}

	/* Old MAngband code does several hacks to ensure smoother running, disturbing, etc...
	 *  TODO !!! */

	/* Not enough energy */
	return 0;
}

/* New version of "process_pending_commands"
 *  for now, returns "-1" incase of an error..
 */
int process_player_commands(int p_idx)
{
	player_type *p_ptr = p_list[p_idx]; 

	byte pkt;
	int result = 1;
	int start_pos = 0;

	/* parse */
	while (	cq_len(&p_ptr->cbuf) )
	{
		/* remember position */
		start_pos = p_ptr->cbuf.pos;
		/* read out and execute command */
		pkt = CQ_GET(&p_ptr->cbuf);
		result = (*pcommands[pkt])(p_ptr);
		/* not a "continuing success" */
		if (result != 1) break;
	}
	/* not enough energy, step back */
	if (result == 0) p_ptr->cbuf.pos = start_pos;

	/* ... */
	return result;
}


/* Setup receivers */
void setup_tables(sccb receiv[256], cptr *scheme)
{
	int i;

	for (i = 0; i < 256; i++) {
		receiv[i] = recv_undef;
		scheme[i] = NULL;
	}

#define PACKET(PKT, SCHEME, FUNC) \
	receiv[PKT] = FUNC; \
	scheme[PKT] = SCHEME;
#define PCOMMAND(PKT, SCHEME, FUNC) \
	pcommands[PKT] = FUNC;
#include "net-game.h"
#undef PACKET
}

void free_tables() 
{
	/* No tables... */
}
