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
static byte		command_pkt[256];

int send_play(connection_type *ct, byte mode) 
{
	if (!cq_printf(&ct->wbuf, "%c%b", PKT_PLAY, mode))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_quit(connection_type *ct, const char *reason) 
{
	/* Initial attempt might fail due to buffer overflow */
	if (cq_printf(&ct->wbuf, "%c%S", PKT_QUIT, reason) > 0) return 1;

	/* In this case, clear the buffer and try again */
	cq_clear(&ct->wbuf);
	cq_printf(&ct->wbuf, "%c%S", PKT_QUIT, reason);

	/* We did all we could, but it's an error */
	return -1;
}

int send_server_info(connection_type *ct)
{
	/* Begin cq "transaction" */
	int start_pos = ct->wbuf.len;

	if (!cq_printf(&ct->wbuf, "%c%b%b%b%b", PKT_BASIC_INFO, serv_info.val1, serv_info.val2, serv_info.val3, serv_info.val4))
	{
		ct->wbuf.len = start_pos; /* rewind */
		client_withdraw(ct);
	}
	if (!cq_printf(&ct->wbuf, "%ul%ul%ul%ul", serv_info.val9, serv_info.val10, serv_info.val11, serv_info.val12))
	{
		ct->wbuf.len = start_pos; /* rewind */
		client_withdraw(ct);
	}

	/* OK */
	return 1;
}

int send_char_info(connection_type *ct, player_type *p_ptr)
{
	if (!cq_printf(&ct->wbuf, "%c%d%d%d%d", PKT_CHAR_INFO, p_ptr->state, p_ptr->prace, p_ptr->pclass, p_ptr->male))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_race_info(connection_type *ct)
{
	u32b i, name_size;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_RACE) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}
	
	name_size = p_info[z_info->p_max-1].name + strlen(p_name + p_info[z_info->p_max-1].name);
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", z_info->p_max, name_size, z_info->fake_text_size) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	} 

	for (i = 0; i < z_info->p_max; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s%ul",  p_name + p_info[i].name, p_info[i].name) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
	}
	return 1;
}

int send_class_info(connection_type *ct)
{
	u32b i, name_size;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_CLASS) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}
	
	name_size = c_info[z_info->c_max-1].name + strlen(c_name + c_info[z_info->c_max-1].name);
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", z_info->c_max, name_size, z_info->fake_text_size) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	} 

	for (i = 0; i < z_info->c_max; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s%ul%c",  c_name + c_info[i].name, c_info[i].name, c_info[i].spell_book) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
	}

	return 1;	
}

int send_optgroups_info(connection_type *ct)
{
	u32b i, name_size;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_OPTGROUP) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}
	
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", MAX_OPTION_GROUPS, OPT_MAX, 0) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	} 

	for (i = 0; i < MAX_OPTION_GROUPS; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s",  option_group[i]) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
	}

	return 1;
}

int send_option_info(connection_type *ct, int id)
{
	const option_type *opt_ptr = &option_info[id];

	if (cq_printf(&ct->wbuf, "%c%c%s%s", PKT_OPTION, 
		opt_ptr->o_page, opt_ptr->o_text, opt_ptr->o_desc) <= 0)
	{
		return 0;
	}
	return 1;
}

int send_inventory_info(connection_type *ct, int id)
{
	u32b i, off = 0;
	char buf[80];

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_INVEN) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	if (cq_printf(&ct->wbuf, "%ud%ul%ul%ul", INVEN_TOTAL, eq_name_size, INVEN_WIELD, INVEN_PACK) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	} 

	buf[0] = '\0';
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		off += strlen(buf) + 1;
		if (i < INVEN_WIELD) 
		{
			off = 0;
		}

		my_strcpy(buf, mention_use(0, i), MAX_CHARS);

		if (cq_printf(&ct->wbuf, "%s%ul", buf, off) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
	}
	return 1;
}

int send_indicator_info(connection_type *ct, int id)
{
	const indicator_type *i_ptr = &indicators[id];
	if (!i_ptr->mark) return 1; /* Last one */

	if (cq_printf(&ct->wbuf, "%c%c%c%c%c%d%d%ul%S%s", PKT_INDICATOR,
		i_ptr->pkt, i_ptr->type, i_ptr->amnt,
		i_ptr->win, i_ptr->row, i_ptr->col,
		i_ptr->flag, i_ptr->prompt, i_ptr->mark) <= 0)
	{
		/* Hack -- instead of "client_withdraw(ct);", we simply */
		return 0;
	}

	/* Ok */
	return 1;
}

int send_indication(int Ind, byte id, ...)
{
	connection_type *ct = PConn[Ind];
	const indicator_type *i_ptr = &indicators[id];
	int i = 0, n;

	signed char tiny_c;
	s16b normal_c;
	s32b large_c;
	char* text_c;

	va_list marker;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (!ct) return -1;

	if (!cq_printf(&ct->wbuf, "%c", i_ptr->pkt))
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	va_start( marker, id );

	do
	{
		if (i_ptr->type == INDITYPE_TINY)
		{
			tiny_c = (signed char) va_arg (marker, unsigned int);
			n = cq_printf(&ct->wbuf, "%c", tiny_c);
		}
		else if (i_ptr->type == INDITYPE_NORMAL)
		{
			normal_c = (s16b) va_arg (marker, unsigned int);
			n = cq_printf(&ct->wbuf, "%d", normal_c);
		}
		else if (i_ptr->type == INDITYPE_LARGE)
		{
			large_c = (s32b) va_arg (marker, s32b);
			n = cq_printf(&ct->wbuf, "%l", large_c);
		}
		else if (i_ptr->type == INDITYPE_STRING)
		{
			text_c = (char*) va_arg (marker, char*);
			n = cq_printf(&ct->wbuf, "%s", text_c);
		}
		/* Result */
		if (!n)
		{
			client_withdraw(ct);
		}		
	} while (++i < i_ptr->amnt);

	va_end( marker );

	return 0;
}

int send_stream_info(connection_type *ct, int id)
{
	const stream_type *s_ptr = &streams[id];
	if (!s_ptr->pkt) return 1; /* Last one */

	if (cq_printf(&ct->wbuf, "%c%c%c%c%c%s%c%c%c%c", PKT_STREAM,
 		s_ptr->pkt, s_ptr->addr, s_ptr->rle, s_ptr->flag, s_ptr->mark, 
 		s_ptr->min_row, s_ptr->min_col, s_ptr->max_row, s_ptr->max_col) <= 0)
	{
		/* Hack -- instead of "client_withdraw(ct);", we simply */
		return 0;
	}

	/* Ok */
	return 1;
}

int send_stream_size(connection_type *ct, int st, int y, int x)
{
	if (!ct) return -1;

	/* Acknowledge new size for stream */
	if (cq_printf(&ct->wbuf, "%c%c%c%c", PKT_RESIZE, (byte)st, (byte)y, (byte)x) <= 0)
	{
		client_withdraw(ct);
	}

	return 1;
}

int stream_char_raw(player_type *p_ptr, int st, int y, int x, byte a, char c, byte ta, char tc)
{
	connection_type *ct;
	const stream_type *stream = &streams[st];
	int n;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!p_ptr->stream_hgt[st]) return 1;

	/* Header + Body (with or without transperancy) */
	if (stream->flag & SF_TRANSPARENT)
		n = cq_printf(&ct->wbuf, "%c%d%c%c%c%c", stream->pkt, y | ((x+1) << 8), a, c, a, c);
	else
		n = cq_printf(&ct->wbuf, "%c%d%c%c", stream->pkt, y | ((x+1) << 8), a, c);		
	if (n <= 0)
	{
		client_withdraw(ct);
	}

	/* Ok */
	return 1;
}

int stream_char(player_type *p_ptr, int st, int y, int x)
{
	connection_type *ct;
	const stream_type *stream = &streams[st];
	cave_view_type *source 	= p_ptr->stream_cave[st] + y * MAX_WID;
	s16b l;
	int n;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!p_ptr->stream_hgt[st]) return 1;

	/* Header + Body (with or without transperancy) */
	l = y | ((x+1) << 8);
	if (stream->flag & SF_TRANSPARENT)
		n = cq_printf(&ct->wbuf, "%c%d%c%c%c%c", stream->pkt, l, source[x].a, source[x].c, p_ptr->trn_info[y][x].a, p_ptr->trn_info[y][x].c);
	else
		n = cq_printf(&ct->wbuf, "%c%d%c%c", stream->pkt, l, source[x].a, source[x].c);
	if (n <= 0)
	{
		client_withdraw(ct);
	}

	/* Ok */
	return 1;
}

int stream_line_as(player_type *p_ptr, int st, int y, int as_y)
{
	connection_type *ct;
	const stream_type *stream = &streams[st];
	cave_view_type *source;
	int start_pos;

	s16b	cols = p_ptr->stream_wid[st];
	byte	rle = stream->rle;
	byte	trn = (stream->flag & SF_TRANSPARENT);
	source 	= p_ptr->stream_cave[st] + y * MAX_WID;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!cols) return 1;

	/* Begin cq "transaction" */
	start_pos = ct->wbuf.len;

	/* Packet header */
	if (cq_printf(&ct->wbuf, "%c%d", stream->pkt, as_y) <= 0)
	{
		ct->wbuf.len = start_pos; /* rewind */
		client_withdraw(ct);
	}
	/* (Secondary) */
	if (trn && cq_printc(&ct->wbuf, rle, p_ptr->trn_info[y], cols) <= 0)
	{
		ct->wbuf.len = start_pos; /* rewind */
		client_withdraw(ct);
	}
	/* Packet body */
	if (cq_printc(&ct->wbuf, rle, source, cols) <= 0)
	{
		ct->wbuf.len = start_pos; /* rewind */
		client_withdraw(ct);
	}

int ii;
printf(">>%2d:%2d>",st,as_y);
for (ii = 0; ii < cols; ii++) printf("%c", source[ii].c);
printf("\n");

	/* Ok */
	return 1;
}

int send_term_info(player_type *p_ptr, byte flag, u16b line)
{
	connection_type *ct;
	int n;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Special hack when principal mode is "activate" */
	if (flag & NTERM_ACTIVATE)
	{
		/* Do not change terminals too often */
		if (p_ptr->remote_term == (byte)line) return 1;
		/* Ensure change */
		p_ptr->remote_term = (byte)line;
	}

	/* Send (with additional parameter?) */
	if (flag & 0xF0)
		n = cq_printf(&ct->wbuf, "%c%b%ud", PKT_TERM, flag, line);
	else
		n = cq_printf(&ct->wbuf, "%c%b", PKT_TERM, flag);

	if (n <= 0)
	{
		client_withdraw(ct);
	}

	return n;
}
int send_term_header(player_type *p_ptr, cptr header)
{
	connection_type *ct;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (!cq_printf(&ct->wbuf, "%c%s", PKT_TERM_INIT, header))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_cursor(player_type *p_ptr, char vis, char x, char y)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (!cq_printf(&ct->wbuf, "%c" "%c%c%c", PKT_CURSOR, vis, x, y))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_target_info(player_type *p_ptr, char x, char y, byte win, cptr str)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (!cq_printf(&ct->wbuf, "%c" "%c%c%c%s", PKT_TARGET_INFO, x, y, win, str))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_custom_command_info(connection_type *ct, int id)
{
	const custom_command_type *cc_ptr = &custom_commands[id];

	if (!cc_ptr->m_catch) return 1; /* Last one */

	if (cq_printf(&ct->wbuf, "%c%c%c%d%ul%c%S", PKT_COMMAND,
		command_pkt[id], cc_ptr->scheme, cc_ptr->m_catch, cc_ptr->flag, cc_ptr->tval, cc_ptr->prompt) <= 0)
	{
		/* Hack -- instead of "client_withdraw(ct);", we simply */
		return 0;
	}

	/* Ok */
	return 1;
}

int send_item_tester_info(connection_type *ct, int id)
{
	const item_tester_type *it_ptr = &item_tester[id];
	int i;

	if (!it_ptr->tval[0] && !it_ptr->flag) return 1; /* Last one */

	if (cq_printf(&ct->wbuf, "%c%c%c", PKT_ITEM_TESTER,
		(byte)id, item_tester[id].flag) <= 0)
	{
		return 0;
	}
	for (i = 0; i < MAX_ITH_TVAL; i++) 
	{ 
	 	if (cq_printf(&ct->wbuf, "%c", item_tester[id].tval[i]) <= 0)
		{
			return 0;
		} 
	}  

	/* Ok */
	return 1;
}

int send_floor(int Ind, byte attr, int amt, byte tval, byte flag, cptr name)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (cq_printf(&ct->wbuf, "%c%c%d%c%c%s", PKT_FLOOR, attr, amt, tval, flag, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_inven(int Ind, char pos, byte attr, int wgt, int amt, byte tval, byte flag, cptr name)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (cq_printf(&ct->wbuf, "%c%c%c%ud%d%c%c%s", PKT_INVEN, pos, attr, wgt, amt, tval, flag, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_equip(int Ind, char pos, byte attr, int wgt, byte tval, byte flag, cptr name)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (cq_printf(&ct->wbuf, "%c%c%c%ud%c%c%s", PKT_EQUIP, pos, attr, wgt, tval, flag, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_spell_info(int Ind, u16b book, u16b i, byte flag, cptr out_val)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (!cq_printf(&ct->wbuf, "%c%c%ud%ud%s", PKT_SPELL_INFO, flag, book, i, out_val))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_character_info(player_type *p_ptr)
{
	connection_type *ct;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	return send_char_info(ct, p_ptr);
}

int send_objflags(int Ind, int line)
{
	connection_type *ct = PConn[Ind];
	//TODO: generalize this (merge with streams?)
	byte rle = ( Players[Ind]->use_graphics ? RLE_LARGE : RLE_CLASSIC );

	if (!ct) return -1;

	/* Header */
	if (cq_printf(&ct->wbuf, "%c%d", PKT_OBJFLAGS, line) <= 0)
	{
		client_withdraw(ct);
	}
	/* Body (39 grids of "cave") */
	if (cq_printc(&ct->wbuf, rle, Players[Ind]->hist_flags[line], 39) <= 0)
	{
		client_withdraw(ct);
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

	if (!cq_printf(&ct->wbuf, "%c%ud%s", PKT_MESSAGE, typ, buf))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_message_repeat(int Ind, u16b typ)
{
	connection_type *ct = PConn[Ind];

	if (!ct) return -1;

	if (!cq_printf(&ct->wbuf, "%c%ud", PKT_MESSAGE_REPEAT, typ))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_sound(int Ind, int sound)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (!cq_printf(&ct->wbuf, "%c%c", PKT_SOUND, sound))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_channel(int Ind, char mode, u16b id, cptr name)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (!cq_printf(&ct->wbuf, "%c%ud%c%s", PKT_CHANNEL, id, mode, name))
	{
		client_withdraw(ct);
	}
	return 1;
}

int recv_channel(connection_type *ct, player_type *p_ptr)
{
	u16b
		id;
	char
		mode,
		name[MAX_CHARS];

	int Ind = Get_Ind[p_ptr->conn];

	if (cq_scanf(&ct->rbuf, "%ud%c%s", &id, &mode, name) < 3) return 0;

	switch (mode)
	{
		case CHAN_JOIN:
		case CHAN_SELECT:

			/* If channel is real */
			if (name[0] == '#')
			{
				name[MAX_CHAN_LEN] = '\0';

				channel_join(Ind, name, TRUE);

				p_ptr->second_channel[0] = '\0';
			}
			/* If channel is virtual */
			else
			{
				my_strcpy(p_ptr->second_channel, name, MAX_CHARS);
			}

		break;
		case CHAN_LEAVE:

			channel_leave_id(Ind, id, FALSE);

		break;
		default:
			/*client_abort(ct, "Unrecognized channel mode.");*/
		break;
	}

	return 1;
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
	/* Hack -- remember position, and rewind to it upon failure */
	int fail = 0;
	int start_pos = p_ptr->cbuf.pos;

	/* Write header */
	if (cq_printf(&p_ptr->cbuf, "%c", next_pkt) <= 0)
	{
		fail = 1;
	}
	else
	/* Hack -- it's a "custom command" */
	if (next_pkt == PKT_COMMAND)
	{
		/* custtom command 'id' is used in those, so we copy it first */
		if (cq_copyf(&ct->rbuf, "%c", &p_ptr->cbuf) < 1)
		{
			/* Unable to... */
			fail = 1;
		}
		else
		{
			/* and the actual 'scheme' should be picked from a pcommand buffer */
			byte cmd = ct->rbuf.buf[ct->rbuf.pos-1];
			/* "cmd" is the 'id' we just copied above */
			next_scheme = custom_command_schemes[custom_commands[cmd].scheme];
		}
	}

	/* Copy command to player's "command buffer" */
	if (!fail && next_scheme && cq_copyf(&ct->rbuf, next_scheme, &p_ptr->cbuf) < 0)
	{
		/* Unable to... */
		fail = 1;
	}

	if (fail)
	{
		/* Rewind, report lack of energy */
		p_ptr->cbuf.pos = start_pos;
		return 0;
	}

	/* OK */
	return 1;
}

/* Undefined packet "handler" */
static int recv_undef(connection_type *ct, player_type *p_ptr) 
{ 
	client_abort(ct, format("Undefined packet '%d'!", next_pkt));
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
	/* Client asks for a hard restart */
	if (mode == PLAY_RESTART)
	{
		/* Prerequisites: */
		if (p_ptr->state != PLAYER_BONE)
		{
			client_abort(ct, "Character not suitable for restarting!");
		}

		/* Do it */
		player_net_wipe(p_ptr, 0);

		/* Consequences: */
		p_ptr->state = PLAYER_NAMED;
	}
	/* Client asks for soft restart */
	else if (mode == PLAY_REROLL)
	{
		/* Prerequisites: */
		if (p_ptr->state != PLAYER_BONE)
		{
			client_abort(ct, "Character not suitable for rerolling!");
		}

		/* Do it */
		player_net_wipe(p_ptr, 1);

		/* Consequences: */
		p_ptr->state = PLAYER_SHAPED;
	}
	/* Client asks for a (re)roll */
	else if (mode == PLAY_ROLL)
	{
		/* Prerequisites: */
		if (p_ptr->state != PLAYER_SHAPED) 
		{
			client_abort(ct, "Character not suitable for rolling!");
		}

		/* Do it */
		player_birth(ct->user, p_ptr->prace, p_ptr->pclass, p_ptr->male, p_ptr->stat_order);

		/* Consequences: */
		p_ptr->state = PLAYER_FULL;
	}
	/* Client asks for to enter game */
	else if (mode == PLAY_ENTER)
	{
		/* Prerequisites: */
		if (p_ptr->state != PLAYER_FULL) 
		{
			client_abort(ct, "Character not ready to enter a game!");
		}

		/* Do it */
		player_verify_visual(p_ptr);

		/* Consequences: */
		p_ptr->state = PLAYER_READY;
	}
	/* Client asks to return to previously abandoned gameplay */
	else if (mode == PLAY_PLAY && p_ptr->state == PLAYER_LEAVING)
	{
		/* Prerequisites: */
		if (p_ptr->screen_wid == 0 || p_ptr->screen_hgt == 0) 
		{
			client_abort(ct, format("Viewscreen not ready to play a game! [%dx%d]",p_ptr->screen_wid,p_ptr->screen_hgt));
		}

		/* Do it */
		player_verify_visual(p_ptr);
		player_enter(ct->user);
	}
	/* Client asks for active gameplay */
	else if (mode == PLAY_PLAY)
	{
		/* Prerequisites: */
		if (p_ptr->state != PLAYER_READY)
		{
			client_abort(ct, "Character not ready to play a game!");
		}
		if (p_ptr->screen_wid == 0 || p_ptr->screen_hgt == 0) 
		{
			client_abort(ct, format("Viewscreen not ready to play a game! [%dx%d]",p_ptr->screen_wid,p_ptr->screen_hgt));
		}

		/* Show MOTD */
		show_motd(p_ptr);

		/* Do it */
		player_enter(ct->user);

		/* Consequences: */
		p_ptr->state = PLAYER_PLAYING;
	}

	/* Inform client */
	send_play(ct, p_ptr->state);

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
		case BASIC_INFO_INDICATORS:
			while (id < MAX_INDICATORS) if (!send_indicator_info(ct, id++)) break;
		break;
		case BASIC_INFO_STREAMS:
			while (id < MAX_STREAMS) if (!send_stream_info(ct, id++)) break;
		break;
		case BASIC_INFO_COMMANDS:
			while (id < MAX_CUSTOM_COMMANDS) if (!send_custom_command_info(ct, id++)) break;
		break;
		case BASIC_INFO_ITEM_TESTERS:
			while (id < MAX_ITEM_TESTERS) if (!send_item_tester_info(ct, id++)) break;
		break;
		case BASIC_INFO_OPTIONS:
			while (id < OPT_MAX) if (!send_option_info(ct, id++)) break;
		break;
		default: break;
	}

	return 1;
}

int recv_char_info(connection_type *ct, player_type *p_ptr) {
	int i;
	
	/* TODO: Ensure p_ptr->state is correct of char_info and bail if not */
	if (p_ptr->state != PLAYER_NAMED)
	{ 
		client_abort(ct, "Character not ready to be modified!");
	}

	if (cq_scanf(&ct->rbuf, "%d%d%d", &p_ptr->prace, &p_ptr->pclass, &p_ptr->male) < 3) 
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

	/* Inform client */
	send_char_info(ct, p_ptr);

	return 1;
}

int recv_visual_info(connection_type *ct, player_type *p_ptr) {
	int n, i, local_size;
	byte at;
	char *char_ref;
	byte *attr_ref = NULL;
	byte
		type = 0;
	u16b
		size = 0;
	if (cq_scanf(&ct->rbuf, "%c%d", &type, &size) < 2)
	{
		/* Not enough bytes */
		return 0;
	}

	if (IS_PLAYING(p_ptr))
	{
		client_abort(ct, "Can't change visual info during gameplay");
	}

	/* Gather type */
	switch (type) 
	{
		case VISUAL_INFO_FLVR:
			local_size = MAX_FLVR_IDX;//MIN(MAX_FLVR_IDX, z_info->flavor_max)
			attr_ref = p_ptr->flvr_attr;
			char_ref = p_ptr->flvr_char;
			break;
		case VISUAL_INFO_F:
			local_size = z_info->f_max;
			attr_ref = p_ptr->f_attr;
			char_ref = p_ptr->f_char;
			break;
		case VISUAL_INFO_K:
			local_size = z_info->k_max;
 			attr_ref = p_ptr->k_attr;
  			char_ref = p_ptr->k_char;
  			break;
  		case VISUAL_INFO_R:
			local_size = z_info->r_max;
			attr_ref = p_ptr->r_attr;
			char_ref = p_ptr->r_char;
			break;
		case VISUAL_INFO_TVAL:
	 		local_size = 128;
			attr_ref = p_ptr->tval_attr;
			char_ref = p_ptr->tval_char;
			break;
		case VISUAL_INFO_MISC:
			local_size = 256;
			attr_ref = p_ptr->misc_attr;
			char_ref = p_ptr->misc_char;
			break;
		case VISUAL_INFO_PR:
			local_size = (z_info->c_max + 1) * z_info->p_max;
			attr_ref = p_ptr->pr_attr;
			char_ref = p_ptr->pr_char;
			break;
		default: break;
	}
	/* Ensure size is compatible */
	if (local_size != size)
	{
		attr_ref = NULL;
	}

	/* Finally, Read the data */
	if ((n = cq_scanac(&ct->rbuf, RLE_NONE, attr_ref, char_ref, size)) < size)
	{
		/* Not enough bytes */
		return 0;
	}

	/* Ok */
	return 1;
}

int recv_options(connection_type *ct, player_type *p_ptr) {
	int n, i, j;
	byte next, bit;

	for (i = 0; i < OPT_MAX; i += 8)
	{
		next = 0;
		if (cq_scanf(&ct->rbuf, "%b", &next) < 1)
		{
			/* Not enough bytes */
			return 0;
		}
		/* Unpack 8 options */
		for (bit = 0; bit < 8; bit++)
		{
			int n = i + bit;
			if (n >= OPT_MAX) break;

			/* Real index is in the o_uid! */
			n = option_info[n].o_uid;

			/* Skip locked options */
			if (option_info[n].o_bit) continue;

			/* Skip birth options */
			if (option_info[n].o_page == 1 && IS_PLAYING(p_ptr)) continue;
			/* Set */
			if (next & (1L << bit))
			{
				p_ptr->options[n] = TRUE;
			}
			else
			{
				p_ptr->options[n] = FALSE;
			}
		}
	}

	/* Ok */
	return 1;
}

int recv_settings(connection_type *ct, player_type *p_ptr) {
	int i;

	for (i = 0; i < 16; i ++)
	{
		s16b val = 0;
		if (cq_scanf(&ct->rbuf, "%d", &val) < 1)
		{
			/* Not enough bytes */
			return 0;
		}
		switch (i)
		{
			case 0:	p_ptr->use_graphics  = val; break;
			case 3:	p_ptr->hitpoint_warn = val; break; 
			default: break;
		}
	}

	/* Ok */
	return 1;
}

int recv_stream_size(connection_type *ct, player_type *p_ptr) {
	int Ind = Get_Ind[p_ptr->conn];
	byte
		stg = 0,
		y = 0,
		x = 0;
	byte st, addr;
	if (cq_scanf(&ct->rbuf, "%c%c%c", &stg, &y, &x) < 3) 
	{
		/* Not enough bytes */
		return 0;
	}

	/* Set stream group starting point */
	if (stg < MAX_STREAMS) addr = streams[stg].addr;

	/* Do the whole group */
	for (st = stg; st < MAX_STREAMS; st++)
	{
		/* Stop when we move to the next group */
		if (streams[st].addr != addr) break;

		/* Test bounds (if we're subscribing) */
		if (y)
		{
			if (y < streams[st].min_row) y = streams[st].min_row;
			if (y > streams[st].max_row) y = streams[st].max_row;
			if (x < streams[st].min_col) x = streams[st].min_col;
			if (x > streams[st].max_col) x = streams[st].max_col;
		}

		/* Set width and height */
		p_ptr->stream_wid[st] = x;
		p_ptr->stream_hgt[st] = y;

		/* Subscribe / Unsubscribe */
		if (y) 
		{
			p_ptr->window_flag |= streams[st].window_flag;
			p_ptr->window |= streams[st].window_flag; /* + Schedule actual update */
			/* HACK! Resizing dungeon view! */
			if (streams[st].addr == NTERM_WIN_OVERHEAD)
			{
				p_ptr->screen_wid = p_ptr->stream_wid[0];
				p_ptr->screen_hgt = p_ptr->stream_hgt[0];
				if (IS_PLAYING(p_ptr))
				{
					setup_panel(Ind, TRUE);
					verify_panel(Ind);
					p_ptr->redraw |= (PR_MAP);
				}
			}
		}
		else
		{
			p_ptr->window_flag &= ~streams[st].window_flag;
		}
	}

	/* Ack it */
	send_stream_size(ct, stg, y, x);

	return 1;
}
/* */
int recv_term_init(connection_type *ct, player_type *p_ptr)
{
	byte
		type = 0;
	int Ind = Get_Ind[p_ptr->conn];		
	int n;
	if (cq_scanf(&ct->rbuf, "%c", &type) < 1)
	{
		/* Not enough bytes */
		return 0;
	}

	/* Hack */
	if (type == 0 && !IS_PLAYING(p_ptr)) type = SPECIAL_FILE_HELP;


	p_ptr->special_file_type = type;
	for (n = 0; n < MAX_CUSTOM_COMMANDS; n++)
	{
		const custom_command_type *cc_ptr = &custom_commands[n];
		if (cc_ptr->m_catch == 0) break;
		if ((cc_ptr->tval == type) &&
			(cc_ptr->flag & COMMAND_INTERACTIVE) &&
			(cc_ptr->do_cmd_callback)) 
		{
			p_ptr->special_handler = n;
			(*(void (*)(player_type*, char))(cc_ptr->do_cmd_callback))(p_ptr, 0);
			return 1;
		}
			
	}

	do_cmd_interactive(p_ptr, 0);

	return 1;
}
/* */
int recv_term_key(connection_type *ct, player_type *p_ptr)
{
	byte
		key = 0;
	int Ind = Get_Ind[p_ptr->conn];		
	int n;
	if (cq_scanf(&ct->rbuf, "%c", &key) < 1)
	{
		/* Not enough bytes */
		return 0;
	}

	if ((n = p_ptr->special_handler))
		(*(void (*)(player_type*, char))(custom_commands[n].do_cmd_callback))(p_ptr, key);
	else if (p_ptr->special_file_type)
		do_cmd_interactive(p_ptr, key);

	return 1;
}

/* Client asks all-new data, so we enqueue ALL the updates */
int recv_redraw(connection_type *ct, player_type *p_ptr)
{
	if (p_ptr->state == PLAYER_PLAYING)
	{
		p_ptr->store_num = -1; //TODO: check if this is really necessary/okay?
		p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_FLOOR);
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER | PW_MAP | PW_MONLIST);
		p_ptr->update |= (PU_BONUS | PU_VIEW | PU_MANA | PU_HP);
		//TODO: check if there are more generic ways to apply those
	}
	return 1;
}

int recv_clear(connection_type *ct, player_type *p_ptr)
{
	/* Clear player's "command buffer" */
	cq_clear(&p_ptr->cbuf);
	return 1;
}

int recv_party(connection_type *ct, player_type *p_ptr)
{
	int Ind = Get_Ind[p_ptr->conn];

	char
		buf[160];
	s16b
		command;

	if (cq_scanf(&ct->rbuf, "%d%s", &command, buf) < 2)
	{
	    return 0;
	}
	if (Ind)
	{
		/* Hack -- silently fail in arena */
		if (p_ptr->arena_num != -1) return 1;

		switch (command)
		{
			case PARTY_CREATE:
			{
				party_create(Ind, buf);
				break;
			}

			case PARTY_ADD:
			{
				party_add(Ind, buf);
				break;
			}

			case PARTY_DELETE:
			{
				party_remove(Ind, buf);
				break;
			}

			case PARTY_REMOVE_ME:
			{
				party_leave(Ind);
				break;
			}

			case PARTY_HOSTILE:
			{
				add_hostility(Ind, buf);
				break;
			}

			case PARTY_PEACE:
			{
				remove_hostility(Ind, buf);
				break;
			}
		}
	}

	return 1;
}

int recv_suicide(connection_type *ct, player_type *p_ptr)
{
	int Ind = Get_Ind[p_ptr->conn];

	/* Commit suicide */
	do_cmd_suicide(Ind);
	return 1;
}

int recv_target(connection_type *ct, player_type *p_ptr)
{
	char mode, dir;
	int Ind;

	Ind = Get_Ind[p_ptr->conn];

	if (cq_scanf(&ct->rbuf, "%c%c", &mode, &dir) < 2)
	{
		return 0;
	}

	switch (mode)
	{
		case NTARGET_FRND:

			do_cmd_target_friendly(Ind, dir);

		break;
		case TARGET_KILL:

			do_cmd_target(Ind, dir);

		break;
		case NTARGET_LOOK:
		default:

			do_cmd_look(Ind, dir);

		break;
	}

    return 1;
}

int recv_locate(connection_type *ct, player_type *p_ptr)
{
	char dir;
	int Ind;

	Ind = Get_Ind[p_ptr->conn];

	if (cq_scanf(&ct->rbuf, "%c", &dir) < 1)
	{
		return 0;
	}

	do_cmd_locate(Ind, dir);

	return 1;
}

int recv_confirm(connection_type *ct, player_type *p_ptr) {
	int Ind;
	byte type;
	byte id;

	if (cq_scanf(&ct->rbuf, "%c%c", &type, &id) < 2)
	{
		return 0;
	}

	/* TODO: increment and compare */
	if (id != 0) return 1;

	/* Don't do any "offline" purchases */
	if (p_ptr->state != PLAYER_PLAYING) return 1;

	Ind = Get_Ind[p_ptr->conn];

	if (p_ptr->store_num > -1)
	{
		store_confirm(Ind);
	}
	else if (p_ptr->current_house > -1)
	{
		do_cmd_purchase_house(Ind, 0);
	}
	else
	{
		carry(Ind, 1, 1);
	}

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

	/* Classic MAnghack #2. */
	/* Always prefer last walk request */
	/* Note: there's another arguably more elegant way to do this:
	 * replace last walk request in "recv_command" or similar,
	 * the way older code did it */
	if (cq_len(&p_ptr->cbuf) && (CQ_PEEK(&p_ptr->cbuf))[0] == PKT_WALK)
	{
		return 1;
	}

	/* Classic MAnghack #1. */
	/* Disturb if running or resting */
	if (p_ptr->running || p_ptr->resting)
	{
		disturb(Ind, 0, 0);
		return 1;
	}

	/* Check energy */
	if (p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		/* Actually walk */
		do_cmd_walk(Ind, dir, option_p(p_ptr,ALWAYS_PICKUP));

		/* End turn */
		return 2;
	}

	/* Not enough energy */
	return 0;
}

static int recv_toggle_rest(player_type *p_ptr) {
	int Ind = Get_Ind[p_ptr->conn];

	if (p_ptr->resting)
	{
		disturb(Ind, 0, 0);
		return 1;
	}	

	/* Don't rest if we are poisoned or at max hit points and max spell points */ 
	if ((p_ptr->poisoned) || ((p_ptr->chp == p_ptr->mhp) && 
	                       	(p_ptr->csp == p_ptr->msp)))
	{
		return 1;
	}

	/* Check energy */
	if ((p_ptr->energy) >= (level_speed(p_ptr->dun_depth)))
	{
		/* Start resting */
		do_cmd_toggle_rest(Ind);

		/* End turn */
		return 2;
	}
	/* If we don't have enough energy to rest, disturb us (to stop
	 * us from running) and queue the command.
	 */
	disturb(Ind, 0, 0);

	/* Try again later */ 
	return 0;
}

/* By the time we're parsing command queue, we're guaranteed to have all the bytes,
 * so it's not neccessary to do error checks on that. */
static int recv_custom_command(player_type *p_ptr)
{
	s32b value;
	int n, player;
	char
		dir,
		item;
	byte id;
	byte i, j, tmp;
	char entry[60];

	if (IS_PLAYING(p_ptr))
	{
		player = Get_Ind[p_ptr->conn];
	}
	else
	{
		printf("Not playing\n");
		return -1;
	}

	/* Direct? */
	if (next_pkt == (char)PKT_COMMAND)
	{
		if (cq_scanf(&p_ptr->cbuf, "%c", &i) < 1)
		{
			printf("Weird error\n");
			return -1;
		}
	}
	/* Find */
	else
	{
		/* TODO: replace this with lookup table */
		i = MAX_CUSTOM_COMMANDS + 1;
		for (j = 0; j < MAX_CUSTOM_COMMANDS; j++)
		{
			if (command_pkt[j] == (char)next_pkt)
			{
				i = j;
				break;
			}
		}
	}

	/* Undefined */
	if (i > MAX_CUSTOM_COMMANDS || !custom_commands[i].m_catch)
	{
		printf("****** Unknown command [%d] '%c' PKT %d\n", i, custom_commands[i].m_catch, next_pkt);
		return -1;
	}

	/* Does it cost energy? */
	if (custom_commands[i].energy_cost)
	{
		/* Not enough! ABORT! */
		if (p_ptr->energy < level_speed(p_ptr->dun_depth) / custom_commands[i].energy_cost)
		{
			/* Report lack of energy */
			printf("Lack energy\n");
			return 0;
		} 
	}

	/* Read the arguments from command buffer */
#define S_START case SCHEME_EMPTY: n = (1
#define S_READ(A) ); break; case SCHEME_ ## A: cq_scanf(&p_ptr->cbuf, (CCS_ ## A),
#define S_SET(A) ); A; (tmp=0
#define S_DONE ); break;
	switch (custom_commands[i].scheme)
	{
		S_START

		S_READ( FULL )          	&item, &dir, &value, entry

		S_READ( ITEM )          	&item
		S_READ( DIR )           	&dir
		S_READ( VALUE )         	&value
		S_READ( SMALL )         	&tmp            	S_SET(value=tmp)
		S_READ( STRING )        	entry
		S_READ( CHAR )          	&entry[0]

		S_READ( ITEM_DIR )      	&item, &dir
		S_READ( ITEM_VALUE )    	&item, &value
		S_READ( ITEM_SMALL )    	&item, &tmp     	S_SET(value=tmp)
		S_READ( ITEM_STRING )   	&item, entry
		S_READ( ITEM_CHAR )     	&item, &entry[0]

		S_READ( DIR_VALUE )     	&dir, &value
		S_READ( DIR_SMALL )     	&dir, &tmp      	S_SET(value=tmp)
		S_READ( DIR_STRING )    	&dir, entry
		S_READ( DIR_CHAR )      	&dir, &entry[0]

		S_READ( VALUE_STRING )  	&value, entry
		S_READ( VALUE_CHAR )    	&value, &entry[0]
		S_READ( SMALL_STRING )  	&tmp, entry     	S_SET(value=tmp)
		S_READ( SMALL_CHAR )    	&tmp, &entry[0] 	S_SET(value=tmp)

		S_READ( ITEM_DIR_VALUE )	&item, &dir, &value
		S_READ( ITEM_DIR_SMALL )	&item, &dir, &tmp	S_SET(value=tmp)
		S_READ( ITEM_DIR_STRING )	&item, &dir, entry
		S_READ( ITEM_DIR_CHAR ) 	&item, &dir, &entry[0]

		S_READ( ITEM_VALUE_STRING )	&item, &value, entry
		S_READ( ITEM_VALUE_CHAR )	&item, &value, &entry[0]
		S_READ( ITEM_SMALL_STRING )	&item, &tmp, entry  	S_SET(value=tmp)
		S_READ( ITEM_SMALL_CHAR )	&item, &tmp, &entry[0]	S_SET(value=tmp)

		S_READ( PPTR_CHAR )        	&entry[0]

		S_DONE
	}
#undef S_START
#undef S_READ
#undef S_SET
#undef S_DONE


	/* Call the callback ("execute command") */
#define S_ARG (custom_commands[i].do_cmd_callback) 
#define S_EXEC(A, B, C) case SCHEME_ ## A: (*(void (*)B)S_ARG)C ; break;
	switch (custom_commands[i].scheme)
	{
		S_EXEC( EMPTY,          	(int),                  	(player))
		S_EXEC( ITEM,           	(int, char),            	(player, item))
		S_EXEC(	DIR,            	(int, char),            	(player, dir))
		S_EXEC(	VALUE,          	(int, int),             	(player, value))
		S_EXEC(	SMALL,          	(int, int),             	(player, value))
		S_EXEC(	STRING,         	(int, char*),           	(player, entry))
		S_EXEC(	CHAR,           	(int, char),            	(player, entry[0]))
		S_EXEC(	ITEM_DIR,       	(int, char, char),      	(player, item, dir))
		S_EXEC(	ITEM_VALUE,     	(int, char, int),       	(player, item, value))
		S_EXEC(	ITEM_SMALL,     	(int, char, int),       	(player, item, value))
		S_EXEC(	ITEM_STRING,    	(int, char, char*),     	(player, item, entry))
		S_EXEC(	ITEM_CHAR,      	(int, char, char),      	(player, item, entry[0]))
		S_EXEC(	DIR_VALUE,      	(int, char, int),       	(player, dir, value))
		S_EXEC(	DIR_SMALL,      	(int, char, int),       	(player, dir, value))
		S_EXEC(	DIR_STRING,     	(int, char, char*),     	(player, dir, entry))
		S_EXEC(	DIR_CHAR,       	(int, char, char),      	(player, dir, entry[0]))
		S_EXEC(	VALUE_STRING,   	(int, int, char*),      	(player, value, entry))
		S_EXEC(	VALUE_CHAR,     	(int, int, char),       	(player, value, entry[0]))
		S_EXEC(	SMALL_STRING,   	(int, int, char*),      	(player, value, entry))
		S_EXEC(	SMALL_CHAR,     	(int, int, char),       	(player, value, entry[0]))
		S_EXEC(	ITEM_DIR_VALUE, 	(int, char, char, int), 	(player, item, dir, value))
		S_EXEC(	ITEM_DIR_SMALL, 	(int, char, char, int), 	(player, item, dir, value))
		S_EXEC(	ITEM_DIR_STRING,	(int, char, char, char*),	(player, item, dir, entry))
		S_EXEC(	ITEM_DIR_CHAR,  	(int, char, char, char),	(player, item, dir, entry[0]))
		S_EXEC(	ITEM_VALUE_STRING,	(int, char, int, char*),	(player, item, value, entry))
		S_EXEC(	ITEM_VALUE_CHAR,	(int, char, int, char), 	(player, item, value, entry[0]))
		S_EXEC(	ITEM_SMALL_STRING,	(int, char, int, char*),	(player, item, value, entry))
		S_EXEC(	ITEM_SMALL_CHAR, 	(int, char, int, char), 	(player, item, value, entry[0]))

		S_EXEC(	PPTR_CHAR,         	(player_type*, char),      	(p_ptr, entry[0]))
	}
#undef S_ARG 
#undef S_EXEC

	/* Done */
	return 1;
}

int send_store(int Ind, char pos, byte attr, s16b wgt, s16b number, long price, cptr name) 
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (cq_printf(&ct->wbuf, "%c%c%c%d%d%ul%s", PKT_STORE, pos, attr, wgt, number, price, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_store_info(int Ind, byte flag, cptr name, char *owner, int items, long purse)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (cq_printf(&ct->wbuf, "%c%c%s%s%d%l", PKT_STORE_INFO, flag, name, owner, items, purse) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_confirm_request(int Ind, byte type, cptr buf)
{
	connection_type *ct = PConn[Ind];
	if (!ct) return -1;
	if (!cq_printf(&ct->wbuf, "%c" "%c%c%s", PKT_CONFIRM, type, 0x00, buf))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_store_sell(int Ind, u32b price)
{
	char buf[80];
	sprintf(buf, "Accept %" PRId32 " gold?", price);
	return send_confirm_request(Ind, 0x01, buf);
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
		next_pkt = pkt = CQ_GET(&p_ptr->cbuf);
		result = (*pcommands[pkt])(p_ptr);
		/* not a "continuing success" */
		if (result != 1) break;
	}
	/* not enough energy, step back */
	if (result == 0) p_ptr->cbuf.pos = start_pos;
	/* slide buffer to the left */
	else if (result == 1) cq_slide(&p_ptr->cbuf);

	/* ... */
	return result;
}


/* Setup receivers */
void setup_tables(sccb receiv[256], cptr *scheme)
{
	int i;
	byte next_free = 0;

	/* Clear packet and command handlers */ 
	for (i = 0; i < 256; i++) {
		receiv[i] = recv_undef;
		scheme[i] = NULL;
		pcommands[i] = NULL;
	}

	/* Set default handlers */
#define PACKET(PKT, SCHEME, FUNC) \
	receiv[PKT] = FUNC; \
	scheme[PKT] = SCHEME;
#define PCOMMAND(PKT, SCHEME, FUNC) \
	pcommands[PKT] = FUNC; \
	receiv[PKT] = recv_command; \
	scheme[PKT] = SCHEME;
#include "net-game.h"
#undef PACKET
#undef PCOMMAND

	/* Setup custom commands */
	for (i = 0; i < MAX_CUSTOM_COMMANDS; i++)
	{
		byte pkt = (byte)custom_commands[i].pkt;
		if (!custom_commands[i].m_catch) break;
		if (pkt == PKT_UNDEFINED)
		{
			while (receiv[next_free] != recv_undef && next_free < 255) next_free++;
			pkt = next_free;
			if (pkt == 255) plog("ERROR! Out of custom command slots!");
		}
		pcommands[pkt] = recv_custom_command;
		command_pkt[i] = pkt;

		receiv[pkt] = recv_command;
		scheme[pkt] = custom_command_schemes[custom_commands[i].scheme];
	}
	/* 'Count' commands */
	serv_info.val3 = i;

	/* Count indicators */
	i = 0;
	while (i < MAX_INDICATORS && !(!indicators[i].pkt && !indicators[i].amnt)) i++;
	if (i >= 254) plog("ERROR! Out of indicator PKTs!");
	serv_info.val1 = i;	
	
	/* Count streams */
	i = 0;
	while (i < MAX_STREAMS && streams[i].pkt != 0) i++;
	serv_info.val2 = i;

	/* Count item_testers */
	i = 0;
	while (i < MAX_ITEM_TESTERS && !(!item_tester[i].tval[0] && !item_tester[i].flag)) i++;
	serv_info.val4 = i;

	/* 'Count' info */
	serv_info.val9 = z_info->k_max;
	serv_info.val10 = z_info->r_max;
	serv_info.val11 = z_info->f_max;
}

void free_tables() 
{
	/* No tables... */
}
