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
static byte		pkt_command[256];
static u16b		pcommand_energy_cost[256];

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

int send_stats_info(connection_type *ct)
{
	u32b i;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_STATS) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}
	if (cq_printf(&ct->wbuf, "%ud%ul%ul", A_MAX, 0, 0) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	for (i = 0; i < A_MAX; i++)
	{
		/* Transfer other fields here */
		if (cq_printf(&ct->wbuf, "%s", stat_names[i]) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
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

	name_size = p_info[z_info->p_max-1].name + strlen(p_name + p_info[z_info->p_max-1].name) + 1;
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
	
	name_size = c_info[z_info->c_max-1].name + strlen(c_name + c_info[z_info->c_max-1].name) + 1;
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
	u32b i;

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

int send_option_info_DEPRECATED(connection_type *ct, int id)
{
	const option_type *opt_ptr = &option_info[id];

	if (cq_printf(&ct->wbuf, "%c%c%s%s", PKT_OPTION, 
		opt_ptr->o_page, opt_ptr->o_text, opt_ptr->o_desc) <= 0)
	{
		return 0;
	}
	return 1;
}
int send_option_info(connection_type *ct, player_type *p_ptr, int id)
{
	const option_type *opt_ptr = &option_info[id];

	if (!client_version_atleast(p_ptr->version,1,5,3)) return send_option_info_DEPRECATED(ct, id);

	if (cq_printf(&ct->wbuf, "%c" "%c%c%s%s", PKT_OPTION,
		opt_ptr->o_page, opt_ptr->o_norm,
		opt_ptr->o_text, opt_ptr->o_desc) <= 0)
	{
		return 0;
	}
	return 1;
}

/* XXX REMOVE ME XXX Remove at next protocol upgrade. */
int send_inventory_info_DEPRECATED(connection_type *ct)
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

int send_inventory_info(connection_type *ct)
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
		byte xpos = 0, ypos = 0;

		off += strlen(buf) + 1;
		if (i < INVEN_WIELD)
		{
			off = 0;
		}
		else
		{
			xpos = eq_pos[i-INVEN_WIELD][0];
			ypos = eq_pos[i-INVEN_WIELD][1];
		}

		my_strcpy(buf, mention_use(0, i), MAX_CHARS);

		if (cq_printf(&ct->wbuf, "%s%ul%c%c", buf, off, xpos, ypos) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			client_withdraw(ct);
		}
	}
	return 1;
}

int send_objflags_info(connection_type *ct)
{
	u32b off = 0;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_OBJFLAGS) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	if (cq_printf(&ct->wbuf, "%ud%ul%ul", MAX_OBJFLAGS_ROWS, MAX_OBJFLAGS_COLS, 0) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	return 1;
}


int send_floor_info(connection_type *ct)
{
	u32b off = 0;

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (cq_printf(&ct->wbuf, "%c%c", PKT_STRUCT_INFO, STRUCT_INFO_FLOOR) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
	}

	if (cq_printf(&ct->wbuf, "%ud%ul%ul", FLOOR_TOTAL, FLOOR_NEGATIVE ? 1 : 0, FLOOR_NEGATIVE ? -FLOOR_INDEX : FLOOR_INDEX) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		client_withdraw(ct);
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

int send_indication(player_type *p_ptr, int id, ...)
{
	connection_type *ct;

	const indicator_type *i_ptr = &indicators[(byte)id];
	int i = 0, n = 0;
	int start_pos;

	signed char tiny_c;
	s16b normal_c;
	s32b large_c;
	char* text_c;

	va_list marker;

	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	start_pos = ct->wbuf.len; /* begin cq "transaction" */

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
			ct->wbuf.len = start_pos; /* rollback */
			va_end( marker );
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

	if (cq_printf(&ct->wbuf, "%c" "%c%c%c%c" "%s%s" "%ud%c%ud%c", PKT_STREAM,
		s_ptr->pkt, s_ptr->addr, s_ptr->rle, s_ptr->flag,
		s_ptr->mark, s_ptr->window_desc,
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
	if (cq_printf(&ct->wbuf, "%c" "%b%ud%b", PKT_RESIZE, (byte)st, (u16b)y, (byte)x) <= 0)
	{
		client_withdraw(ct);
	}

	return 1;
}

int stream_char_raw(player_type *p_ptr, int st, int y, int x, byte a, char c, byte ta, char tc)
{
	connection_type *ct;
	const stream_type *stream = &streams[st];
	u16b l;
	int n;

	/* Programmer error */
	if (y > 127 || x > 255) { printf("stream_char is limited to y <= 127, x <= 255, you are using y %d, x %d\n", y, x); return -1; }

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!p_ptr->stream_hgt[st]) return 1;

	/* Header + Body (with or without transperancy) */
	l = ((y << 8) & 0x7F00) | (x & 0x00FF) | 0x8000;
	if (stream->flag & SF_TRANSPARENT)
		n = cq_printf(&ct->wbuf, "%c%d%c%c%c%c", stream->pkt, l, a, c, a, c);
	else
		n = cq_printf(&ct->wbuf, "%c%d%c%c", stream->pkt, l, a, c);
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
	cave_view_type *source = p_ptr->stream_cave[st] + y * MAX_WID;
	u16b l;
	int n;

	/* Programmer error */
	if (y > 127 || x > 255) { printf("stream_char is limited to y <= 127, x <= 255, you are using y %d, x %d\n", y, x); return -1; }

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!p_ptr->stream_hgt[st]) return 1;

	/* Header + Body (with or without transperancy) */
	l = ((y << 8) & 0x7F00) | (x & 0x00FF) | 0x8000;
	if (stream->flag & SF_TRANSPARENT)
		n = cq_printf(&ct->wbuf, "%c%ud%c%c%c%c", stream->pkt, l, source[x].a, source[x].c, p_ptr->trn_info[y][x].a, p_ptr->trn_info[y][x].c);
	else
		n = cq_printf(&ct->wbuf, "%c%ud%c%c", stream->pkt, l, source[x].a, source[x].c);
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

	/* Programmer error */
	if (as_y & 0x8000) { printf("stream_line is limited to y <= 32767, you are using y %d\n", as_y); return -1; }

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Do not send streams not subscribed to */
	if (!cols) return 1;

	/* Paranoia -- respect row bounds */
	if (as_y >= p_ptr->stream_hgt[st] && !(stream->flag & SF_MAXBUFFER)) return -1;

	/* Begin cq "transaction" */
	start_pos = ct->wbuf.len;

	/* Packet header */
	if (cq_printf(&ct->wbuf, "%c%ud", stream->pkt, as_y) <= 0)
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
int send_term_header(player_type *p_ptr, byte hint, cptr header)
{
	connection_type *ct;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (!cq_printf(&ct->wbuf, "%c%b%s", PKT_TERM_INIT, hint, header))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_term_writefile(connection_type *ct, byte fmode, cptr filename)
{
	if (ct == NULL) return -1;
	if (!cq_printf(&ct->wbuf, "%c" "%b%s", PKT_TERM_WRITE, fmode, filename))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_term_write(player_type *p_ptr, byte fmode, cptr filename)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	return send_term_writefile(ct, fmode, filename);
}

int send_cursor(player_type *p_ptr, byte vis, byte x, byte y)
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

int send_target_info(player_type *p_ptr, byte x, byte y, byte win, cptr str)
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

	/* HACK -- On first pass, just take note of the id and do nothing,
	 * on second pass, replace 'G'ain for priests/palladins. */
	if (cc_ptr->m_catch == 'G')
	{
		study_cmd_id = id; /* Remember for later */
		/* He has a player attached (LOGGED IN) */
		if ((int)ct->user != -1)
		{
			player_type *p_ptr = players->list[(int)ct->user]->data2;
			if (c_info[p_ptr->pclass].spell_book == TV_PRAYER_BOOK)
			{
				priest_study_cmd.pkt = cc_ptr->pkt;
				cc_ptr = &priest_study_cmd;
			}
		}
	}

	if (cq_printf(&ct->wbuf, "%c%c%c%d%ul%c%S%s", PKT_COMMAND,
		command_pkt[id], cc_ptr->scheme, cc_ptr->m_catch, cc_ptr->flag, cc_ptr->tval, cc_ptr->prompt, cc_ptr->display) <= 0)
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

	int start_pos = ct->wbuf.len; /* begin cq "transaction" */

	if (!it_ptr->tval[0] && !it_ptr->flag) return 1; /* Last one */

	if (cq_printf(&ct->wbuf, "%c%c%c", PKT_ITEM_TESTER,
		(byte)id, item_tester[id].flag) <= 0)
	{
		ct->wbuf.len = start_pos; /* rollback */
		return 0;
	}
	for (i = 0; i < MAX_ITH_TVAL; i++) 
	{ 
	 	if (cq_printf(&ct->wbuf, "%c", item_tester[id].tval[i]) <= 0)
		{
			ct->wbuf.len = start_pos; /* rollback */
			return 0;
		}
	}

	/* Ok */
	return 1;
}

int send_slash_fx(player_type *p_ptr, byte y, byte x, byte dir, byte fx)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (!p_ptr->supports_slash_fx) return 1;
	if (cq_printf(&ct->wbuf, "%c" "%c%c" "%c%b", PKT_SLASH_FX, y, x, dir, fx) <= 0)
	{
		/* No space in buffer, but we don't really care for this packet */
		return 0;
	}
	return 1;
}

int send_air_char(player_type *p_ptr, byte y, byte x, char a, char c, u16b delay, u16b fade)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%c" "%c%c" "%c%c" "%ud%ud", PKT_AIR, y, x, a, c, delay, fade) <= 0)
	{
		/* No space in buffer, but we don't really care for this packet */
		return 0;
	}
	return 1;
}

int send_floor_DEPRECATED(player_type *p_ptr, byte attr, int amt, byte tval, byte flag, byte s_tester, cptr name)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%c" "%c%c%d%c%b%b%s", PKT_FLOOR, 0, attr, amt, tval, flag, s_tester, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_floor(player_type *p_ptr, byte ga, char gc, byte attr, int amt, byte tval, byte flag, byte s_tester, cptr name, cptr name_one)
{
	connection_type *ct;
	/* Hack -- use old version of the function */
	if (!client_version_atleast(p_ptr->version, 1,5,3))
	{
		return send_floor_DEPRECATED(p_ptr, attr, amt, tval, flag, s_tester, name);
	}
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%b" "%c%c%c%c" "%d%c%b%b%s%s", PKT_FLOOR,
		0, ga, gc, attr,
		amt, tval, flag, s_tester, name, name_one) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_inven_DEPRECATED(player_type *p_ptr, char pos, byte attr, int wgt, int amt, byte tval, byte flag, byte s_tester, cptr name)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%c" "%c%c%ud%d%c%b%b%s", PKT_INVEN, pos, attr, wgt, amt, tval, flag, s_tester, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_inven(player_type *p_ptr, char pos, byte ga, char gc, byte attr, int wgt, int amt, byte tval, byte flag, byte s_tester, cptr name, cptr name_one)
{
	connection_type *ct;
	/* Hack -- use old version of the function */
	if (!client_version_atleast(p_ptr->version, 1,5,3))
	{
		return send_inven_DEPRECATED(p_ptr, pos, attr, wgt, amt, tval, flag, s_tester, name);
	}
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%b" "%c%c%c%c" "%ud%d%c%b%b%s%s", PKT_INVEN,
		pos, ga, gc, attr,
		wgt, amt, tval, flag, s_tester, name, name_one) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_equip(player_type *p_ptr, char pos, byte attr, int wgt, byte tval, byte flag, cptr name)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%c" "%c%c%ud%c%b%s", PKT_EQUIP, pos, attr, wgt, tval, flag, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_spell_info(player_type *p_ptr, u16b book, u16b i, byte flag, byte item_tester, cptr out_val)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (!cq_printf(&ct->wbuf, "%c" "%b%b%ud%ud%s", PKT_SPELL_INFO, flag, item_tester, book, i, out_val))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_ghost(player_type *p_ptr)
{
	connection_type *ct;
	s16b mode;

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	mode = PALIVE_ALIVE;
	if (p_ptr->ghost) mode = PALIVE_GHOST;
	else if (p_ptr->fruit_bat) mode = PALIVE_FRUITBAT;

	if (!cq_printf(&ct->wbuf, "%c%d", PKT_GHOST, mode))
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

int send_objflags(player_type *p_ptr, int line)
{
	connection_type *ct;
	//TODO: generalize this (merge with streams?)
	byte rle = ( p_ptr->use_graphics ? RLE_LARGE : RLE_CLASSIC );

	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	/* Header */
	if (cq_printf(&ct->wbuf, "%c%d", PKT_OBJFLAGS, line) <= 0)
	{
		client_withdraw(ct);
	}
	/* Body (39 grids of "cave") */
	if (cq_printc(&ct->wbuf, rle, p_ptr->hist_flags[line], MAX_OBJFLAGS_COLS) <= 0)
	{
		client_withdraw(ct);
	} 
	return 1;
}

/* XXX REMOVE ME XXX Remove at next protocol upgrade. */
int send_message_DEPRECATED(player_type *p_ptr, cptr msg, u16b typ)
{
	connection_type *ct;
	char buf[MAX_CHARS];

	/* Paranoia -- do not send to closed connection */
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

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

int send_message(player_type *p_ptr, cptr msg, u16b typ)
{
	connection_type *ct;
	char buf[MSG_LEN];

	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (msg == NULL)
		return 1;

	/* Hack -- use old version of the function */
	if (!client_version_atleast(p_ptr->version, 1,5,2))
	{
		return send_message_DEPRECATED(p_ptr, msg, typ);
	}

	/* Clip end of msg if too long */
	my_strcpy(buf, msg, MSG_LEN);

	if (!cq_printf(&ct->wbuf, "%c%ud%S", PKT_MESSAGE, typ, buf))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_message_repeat(player_type *p_ptr, u16b typ)
{
	connection_type *ct;

	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (!cq_printf(&ct->wbuf, "%c%ud", PKT_MESSAGE_REPEAT, typ))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_sound(player_type *p_ptr, u16b sound)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (!cq_printf(&ct->wbuf, "%c%ud", PKT_SOUND, sound))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_channel(player_type *p_ptr, char mode, u16b id, cptr name)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
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

	if (cq_scanf(&ct->rbuf, "%ud%c%s", &id, &mode, name) < 3) return 0;

	switch (mode)
	{
		case CHAN_JOIN:
		case CHAN_SELECT:

			/* If channel is real */
			if (name[0] == '#')
			{
				name[MAX_CHAN_LEN] = '\0';

				channel_join(p_ptr, name, TRUE);

				p_ptr->second_channel[0] = '\0';
			}
			/* If channel is virtual */
			else
			{
				my_strcpy(p_ptr->second_channel, name, MAX_CHARS);
			}

		break;
		case CHAN_LEAVE:

			channel_leave_id(p_ptr, id, FALSE);

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

	buf[0] = '\0';

	if (cq_scanf(&ct->rbuf, "%S", buf) < 1)
	{
		return 0;
	}

	player_talk(p_ptr, buf);

	return 1;
}

int recv_pass(connection_type *ct, player_type *p_ptr)
{
	char buf[MAX_CHARS];

	if (cq_scanf(&ct->rbuf, "%s", buf) < 1)
	{
		return 0;
	}

	my_strcpy(p_ptr->pass, buf, MAX_CHARS);

	/* BUG: the password is not actually saved until player_save,
	 * which can make things very confusing during login :( */

	return 1;
}

/* Default handler for all the gameplay commands. */
int recv_command(connection_type *ct, player_type *p_ptr)
{
	/* Hack -- remember position, and rewind to it upon failure */
	int fail = 0;
	int start_len = p_ptr->cbuf.len;

	/* Write header */
	if (cq_printf(&p_ptr->cbuf, "%c", next_pkt) <= 0)
	{
		fail = 1;
	}
	else
	/* Hack -- it's a "custom command" */
	if (next_pkt == PKT_COMMAND)
	{
		/* custom command 'id' is used in those, so we copy it first */
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
		p_ptr->cbuf.len = start_len;
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

	if (p_ptr == NULL)
	{
		/* TODO: verify if this can or cannot happen (by contract) */
		/* ... but better safe than sorry. */
		return -1;
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

		/* Hack -- do not allow new characters to be created? */
		if (cfg_instance_closed)
		{
			client_abort(ct, "No new characters can be created on this server.");
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

	id = p_ptr->infodata_sent[mode];

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
			while (id < OPT_MAX) if (!send_option_info(ct, p_ptr, id++)) break;
		break;
		default: break;
	}

	p_ptr->infodata_sent[mode] = id;

	/* OK */
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
	for (i = 0; i < A_MAX; i++)
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
	int n, local_size = 0;
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
		/* client_abort(ct, "Can't change visual info during gameplay"); */
		/* We can, see below! */
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
			local_size = 1024;
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

	/* Verify data (if changing during gameplay) */
	if (IS_PLAYING(p_ptr))
	{
		player_verify_visual(p_ptr);
		/* Redraw lots of things */
		p_ptr->redraw |= (PR_MAP | PR_FLOOR);
		p_ptr->window |= (PW_OVERHEAD | PW_MAP | PW_MONLIST);
		p_ptr->update |= (PU_VIEW | PU_LITE);
		p_ptr->redraw_inven |= (0xFFFFFFFFFFFFFFFFLL);
	}

	/* Ok */
	return 1;
}

int recv_options(connection_type *ct, player_type *p_ptr) {
	int i;
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

			/* Skip locked options */
			if (option_info[n].o_bit) continue;

			/* Skip birth options (if character is solid) */
			if (option_info[n].o_page == 1 && !p_ptr->new_game) continue;

			/* Real index is in the o_uid! */
			n = option_info[n].o_uid;

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
			case 0: p_ptr->use_graphics  = val; break;
			case 3: p_ptr->hitpoint_warn = (byte_hack)val; break;
			case 5: p_ptr->supports_slash_fx = (bool)val; break;
			default: break;
		}
		/* Hack -- light offsets for graphics mode */
		if (i >= 6 && i < 6 + 4 * 2 && client_version_atleast(p_ptr->version, 1,5,3))
		{
			/* 2 offsets (x, y) * 4 levels, starting at setting "6" */
			int level = (i - 6) / (2);
			int which = (i - 6) - (level * 2);
			p_ptr->graf_lit_offset[level][which] = (s16b)val;
		}
		/* Hack -- support OLD client graphics modes */
		if (i == 0 && !client_version_atleast(p_ptr->version, 1,5,3))
		{
			/* Back then, "1" meant Original, "2" Adam Bolt
			 * and "3" David Gervais, with very specific
			 * layouts. We *know* those layouts, so we can
			 * guess the offsets. */
			if (p_ptr->use_graphics == 1)
			{
				/* "lit floor" - "open floor" tiles */
				p_ptr->graf_lit_offset[0][0] = 0xCF - 0x80;
				p_ptr->graf_lit_offset[0][1] = 0x8F - 0x80;
				p_ptr->use_graphics = GRAPHICS_PLAIN;
			}
			if (p_ptr->use_graphics == 2)
			{
				p_ptr->graf_lit_offset[0][0] = 0;
				p_ptr->graf_lit_offset[0][1] = +2;
				p_ptr->use_graphics = GRAPHICS_TRANSPARENT;
			}
			if (p_ptr->use_graphics == 3)
			{
				p_ptr->graf_lit_offset[0][0] = 0;
				p_ptr->graf_lit_offset[0][1] = -1;
				p_ptr->use_graphics = GRAPHICS_TRANSPARENT;
			}
		}
	}

	/* Ok */
	return 1;
}

int recv_stream_size(connection_type *ct, player_type *p_ptr) {
	byte
		stg = 0,
		x = 0;
	u16b y = 0;
	byte st, addr;
	if (cq_scanf(&ct->rbuf, "%c%ud%c", &stg, &y, &x) < 3)
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

		/* SF_NEXT_GROUP also marks next group */
		if (st > stg && (streams[st].flag & SF_NEXT_GROUP)) break;

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
					setup_panel(p_ptr, TRUE);
					verify_panel(p_ptr);
					p_ptr->redraw |= (PR_MAP);
				}
			}
		}
		else
		{
			p_ptr->window_flag &= ~streams[st].window_flag;
			/* HACK -- check if player has disabled monster text window */
			monster_race_track_hack(p_ptr);
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

/* Hack -- pretend we got an "enterfeat" command */
int recv_enterfeat(player_type *p_ptr) {
	do_cmd_enterfeat(p_ptr);
	return 1;
}
/* Hack -- pretend we got a "pathfind" command */
int recv_pathfind(player_type *p_ptr) {
	byte x, y;
	if (cq_scanf(&p_ptr->cbuf, "%c%c", &y, &x) < 2) return 0;
	do_cmd_pathfind(p_ptr, y, x);
	return 1;
}

/* Hack -- translate mouse action into dungeon action, and enqueue
 * appropriate command. */
int recv_mouse_hack(player_type *p_ptr, int mod, int y, int x)
{
	int i;
	static int alter_cmd_id = -1;

	/* Do nothing if not even playing */
	if (!IS_PLAYING(p_ptr)) return 1;

	/* Right now, we only support 1 mouse button */
	if (!(mod & MCURSOR_LMB)) return 1;

	/* HACK! Find custom command id for "Alter" */
	if (alter_cmd_id == -1)
	for (i = 0; i < MAX_CUSTOM_COMMANDS; i++)
	{
		const custom_command_type *cmd_ptr;
		cmd_ptr = &custom_commands[i];
		if (!cmd_ptr->m_catch) break;
		if (!(cmd_ptr->flag & COMMAND_STORE)
		   && cmd_ptr->m_catch == '+')
		{
			alter_cmd_id = i;
		}
	}

	y = y + p_ptr->panel_row_min;
	x = x + p_ptr->panel_col_min;

	if (x < p_ptr->panel_col_min) x = p_ptr->panel_col_min;
	if (y < p_ptr->panel_row_min) y = p_ptr->panel_row_min;
	if (x > p_ptr->panel_col_max) x = p_ptr->panel_col_max;
	if (y > p_ptr->panel_row_max) y = p_ptr->panel_row_max;

	/* Hack -- queue '_' ? */
	if ((mod & MCURSOR_LMB) && (mod & MCURSOR_SHFT))
	{
		/* Grid offset is 0 (standing on) */
		if (p_ptr->px == x && p_ptr->py == y)
		{
			if (cq_printf(&p_ptr->cbuf, "%c", PKT_ENTER_FEAT) <= 0)
			{
				return -1;
			}
		}
		return 1;
	}

	/* Hack -- queue alter? */
	if ((mod & MCURSOR_LMB) && (mod & MCURSOR_KTRL))
	{
		/* Grid is nearby */
		if (ABS(p_ptr->px - x) <= 1 && ABS(p_ptr->py - y) <= 1)
		{
			int dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
			if (cq_printf(&p_ptr->cbuf, "%c" "%c%c", PKT_COMMAND,
				alter_cmd_id, (char)dir) <= 0)
			{
				return -1;
			}
		}
		return 1;
	}

	/* It's a pathfind request */
	if (cq_printf(&p_ptr->cbuf, "%c" "%c%c", PKT_PATHFIND,
		(char)y, (char)x) <= 0)
	{
		return -1;
	}
	return 1;
};


/* Client sent us some "mouse" action */
int recv_mouse(connection_type *ct, player_type *p_ptr) {
	byte mod, x, y;
	int n;
	char key;

	if (cq_scanf(&ct->rbuf, "%c%c%c", &mod, &x, &y) < 3) return 0;

	if ((mod & MCURSOR_META))
		target_set_interactive_mouse(p_ptr, mod,  y, x);
	else if (!(mod & MCURSOR_EMB))
		return recv_mouse_hack(p_ptr, mod, y, x);
	else if ((n = p_ptr->special_handler))
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
		p_ptr->window |= (PW_SPELL | PW_PLAYER | PW_MAP | PW_MONLIST | PW_ITEMLIST);
		p_ptr->update |= (PU_BONUS | PU_VIEW | PU_MANA | PU_HP);
		p_ptr->redraw_inven |= (0xFFFFFFFFFFFFFFFFLL);
		//TODO: check if there are more generic ways to apply those
	}
	return 1;
}

int recv_clear(connection_type *ct, player_type *p_ptr)
{
	/* Clear player's "command buffer" */
	cq_clear(&p_ptr->cbuf);
	/* Cancel pathfinder */
	p_ptr->running_withpathfind = FALSE;
	return 1;
}

int recv_party(connection_type *ct, player_type *p_ptr)
{
	char
		buf[160];
	s16b
		command;

	if (cq_scanf(&ct->rbuf, "%d%s", &command, buf) < 2)
	{
		return 0;
	}
	if (p_ptr && IS_PLAYING(p_ptr))
	{
		/* Hack -- silently fail in arena */
		if (p_ptr->arena_num != -1) return 1;

		switch (command)
		{
			case PARTY_CREATE:
			{
				party_create(p_ptr, buf);
				break;
			}

			case PARTY_ADD:
			{
				party_add(p_ptr, buf);
				break;
			}

			case PARTY_DELETE:
			{
				party_remove(p_ptr, buf);
				break;
			}

			case PARTY_REMOVE_ME:
			{
				party_leave(p_ptr);
				break;
			}

			case PARTY_HOSTILE:
			{
				add_hostility(p_ptr, buf);
				break;
			}

			case PARTY_PEACE:
			{
				remove_hostility(p_ptr, buf);
				break;
			}
		}
	}

	return 1;
}

int recv_suicide(connection_type *ct, player_type *p_ptr)
{
	/* Commit suicide */
	do_cmd_suicide(p_ptr);
	return 1;
}

int recv_target(connection_type *ct, player_type *p_ptr)
{
	char mode, dir;

	if (cq_scanf(&ct->rbuf, "%c%c", &mode, &dir) < 2)
	{
		return 0;
	}

	switch (mode)
	{
		case NTARGET_FRND:

			do_cmd_target_friendly(p_ptr, dir);

		break;
		case TARGET_KILL:

			do_cmd_target(p_ptr, dir);

		break;
		case NTARGET_LOOK:
		default:

			do_cmd_look(p_ptr, dir);

		break;
	}

	return 1;
}

int recv_locate(connection_type *ct, player_type *p_ptr)
{
	char dir;

	if (cq_scanf(&ct->rbuf, "%c", &dir) < 1)
	{
		return 0;
	}

	do_cmd_locate(p_ptr, dir);

	return 1;
}

int recv_confirm(connection_type *ct, player_type *p_ptr) {
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

	if (p_ptr->store_num > -1)
	{
		store_confirm(p_ptr);
	}
	else if (p_ptr->current_house > -1)
	{
		do_cmd_purchase_house(p_ptr, 0);
	}
	else
	{
		carry(p_ptr, 1, 1);
	}

	return 1;
}

/** Gameplay commands **/
/* Those return
	* -1 on critical error
	*  0 when lack energy
	*  1 on generic success
	*  2 success + we're certain player has spent energy
	*  3 success + all energy was drained (break loop)
*/
static int recv_walk(player_type *p_ptr) {
	char
		dir;
	/* The only critical error I can think of is broken queue */
	if (cq_scanf(&p_ptr->cbuf, "%c", &dir) < 1)
	{
		/* Impossible! :( */
		return -1;
	}

	/* New MAnghack */
	/* Exit store on every non-store command */
	p_ptr->store_num = -1;

	/* Classic MAnghack #2. */
	/* Always prefer last walk request */
	/* Note: there's another arguably more elegant way to do this:
	 * replace last walk request in "recv_command" or similar,
	 * the way older code did it */
	/* Here we test to see if there are *TWO* more walk requests queued up
	 * after the current one, and if so, we skip the current one. The reason
	 * we test for 2 commands (and not just 1), is classic MAngband sometimes
	 * allowed up to 2 walk requests to be executed consequently, so we emulate
	 * that behavior. */
	if ((cq_len(&p_ptr->cbuf) > 2)
	&& ((CQ_PEEK(&p_ptr->cbuf))[0] == PKT_WALK) /* next command */
	&& ((CQ_PEEK(&p_ptr->cbuf))[2] == PKT_WALK)) /* and the one after it */
	{
		return 1;
	}

	/* Classic MAnghack #1. */
	/* Disturb if running or resting */
	if (p_ptr->running || p_ptr->resting)
	{
		disturb(p_ptr, 0, 1);
		return 1;
	}

	/* Check energy */
	if (p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		/* Actually walk */
		do_cmd_walk(p_ptr, dir, option_p(p_ptr,ALWAYS_PICKUP));

		/* Hack -- add aggravating noise */
		set_noise(p_ptr, p_ptr->noise + (30 - p_ptr->skill_stl));

		/* Classic MAnghack #5. Reset built-up energy. */
		p_ptr->energy_buildup = 0;

		/* End turn */
		return 3;
	}

	/* Not enough energy */
	return 0;
}

static int recv_toggle_rest(player_type *p_ptr) {
	/* New MAnghack */
	/* Exit store on every non-store command */
	p_ptr->store_num = -1;

	if (p_ptr->resting)
	{
		disturb(p_ptr, 0, 1);
		return 1;
	}

	/* Don't rest if we are poisoned or at max hit points and max spell points */ 
	if ((p_ptr->poisoned) || ((p_ptr->chp == p_ptr->mhp) &&
	                          (p_ptr->csp == p_ptr->msp)))
	{
		return 1;
	}

	/* Check energy */
	if (p_ptr->energy >= level_speed(p_ptr->dun_depth))
	{
		/* Start resting */
		do_cmd_toggle_rest(p_ptr);

		/* Hack -- add aggravating noise */
		set_noise(p_ptr, p_ptr->noise + (30 - p_ptr->skill_stl));

		/* Classic MAnghack #5. Reset built-up energy. */
		p_ptr->energy_buildup = 0;

		/* End turn */
		return 3;
	}
	/* If we don't have enough energy to rest, disturb us (to stop
	 * us from running) and queue the command.
	 */
	disturb(p_ptr, 0, 1);

	/* Try again later */
	return 0;
}

/* Handle mouse click from the command buffer */
static int recv_mouse_click(player_type *p_ptr)
{
	u32b old_energy;
	byte mod, x, y;
	if (!IS_PLAYING(p_ptr)) return -1;

	if (cq_scanf(&p_ptr->cbuf, "%c%c%c", &mod, &x, &y) < 1)
	{
		/* Broken queue (impossible!) */
		return -1;
	}

	/* Remember how much energy player had */
	old_energy = p_ptr->energy;

	/* Reset "command_arg" (whatever that is) */
	p_ptr->command_arg = 0;

	/* RUN COMMAND */
	do_cmd_mouseclick(p_ptr, mod, y, x);

	/* Player has definitely spent energy */
	if (p_ptr->energy < old_energy)
	{
		/* Report it */
		return 2;
	}

	/* Done */
	return 1;
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
	byte i, tmp;
	char entry[60];
	u32b old_energy;

	if (IS_PLAYING(p_ptr))
	{
		player = Get_Ind[p_ptr->conn];
	}
	else
	{
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
		i = pkt_command[(char)next_pkt];
	}

	/* Undefined */
	if (i >= MAX_CUSTOM_COMMANDS)
	{
		printf("****** No known command [%d] '%c' PKT %d\n", i, '0', next_pkt); /* No command matches */
		return -1;
	}
	else if (!custom_commands[i].m_catch)
	{
		printf("****** Unknown command [%d] '%c' PKT %d\n", i, custom_commands[i].m_catch, next_pkt);
		return -1;
	}

	/* New MAnghack */
	/* Exit store on every non-store command */
	if (!(custom_commands[i].flag & COMMAND_STORE))
	{
		p_ptr->store_num = -1;
	}

	/* Does it cost energy? */
	if (custom_commands[i].energy_cost)
	{
		/* MAngband-specific: if we've JUST started running, pretend we don't have enough energy */
		if ((custom_commands[i].pkt == PKT_RUN) && p_ptr->running && p_ptr->ran_tiles == 0)
		{
			/* Report as lack of energy */
			return 0;
		}
		/* Not enough! ABORT! */
		if (p_ptr->energy < level_speed(p_ptr->dun_depth) / custom_commands[i].energy_cost)
		{
			/* Report lack of energy */
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

	/* Remember how much energy player had */
	old_energy = p_ptr->energy;

	/* Reset "command_arg" (whatever that is) */
	p_ptr->command_arg = 0;

	/* Call the callback ("execute command") */
#define S_ARG (custom_commands[i].do_cmd_callback)
#define S_EXEC(A, B, C) case SCHEME_ ## A: (*(void (*)B)S_ARG)C ; break;
	switch (custom_commands[i].scheme)
	{
		S_EXEC(	EMPTY,          	(player_type*),          	(p_ptr))
		S_EXEC(	ITEM,           	(player_type*, int),     	(p_ptr, item))
		S_EXEC(	DIR,            	(player_type*, int),     	(p_ptr, dir))
		S_EXEC(	VALUE,          	(player_type*, int),     	(p_ptr, value))
		S_EXEC(	SMALL,          	(player_type*, int),     	(p_ptr, value))
		S_EXEC(	STRING,         	(player_type*, char*),   	(p_ptr, entry))
		S_EXEC(	CHAR,           	(player_type*, char),    	(p_ptr, entry[0]))
		S_EXEC(	ITEM_DIR,       	(player_type*, int, int),	(p_ptr, item, dir))
		S_EXEC(	ITEM_VALUE,     	(player_type*, int, int),	(p_ptr, item, value))
		S_EXEC(	ITEM_SMALL,     	(player_type*, int, int),	(p_ptr, item, value))
		S_EXEC(	ITEM_STRING,    	(player_type*, int, char*),	(p_ptr, item, entry))
		S_EXEC(	ITEM_CHAR,      	(player_type*, int, char),	(p_ptr, item, entry[0]))
		S_EXEC(	DIR_VALUE,      	(player_type*, int, int),	(p_ptr, dir, value))
		S_EXEC(	DIR_SMALL,      	(player_type*, int, int),	(p_ptr, dir, value))
		S_EXEC(	DIR_STRING,     	(player_type*, int, char*),	(p_ptr, dir, entry))
		S_EXEC(	DIR_CHAR,       	(player_type*, int, char),	(p_ptr, dir, entry[0]))
		S_EXEC(	VALUE_STRING,   	(player_type*, int, char*),	(p_ptr, value, entry))
		S_EXEC(	VALUE_CHAR,     	(player_type*, int, char),	(p_ptr, value, entry[0]))
		S_EXEC(	SMALL_STRING,   	(player_type*, int, char*),	(p_ptr, value, entry))
		S_EXEC(	SMALL_CHAR,     	(player_type*, int, char),	(p_ptr, value, entry[0]))
		S_EXEC(	ITEM_DIR_VALUE, 	(player_type*, int, int, int),	(p_ptr, item, dir, value))
		S_EXEC(	ITEM_DIR_SMALL, 	(player_type*, int, int, int),	(p_ptr, item, dir, value))
		S_EXEC(	ITEM_DIR_STRING,	(player_type*, int, int, char*),(p_ptr, item, dir, entry))
		S_EXEC(	ITEM_DIR_CHAR,  	(player_type*, int, int, char),	(p_ptr, item, dir, entry[0]))
		S_EXEC(	ITEM_VALUE_STRING,	(player_type*, int, int, char*),(p_ptr, item, value, entry))
		S_EXEC(	ITEM_VALUE_CHAR,	(player_type*, int, int, char),	(p_ptr, item, value, entry[0]))
		S_EXEC(	ITEM_SMALL_STRING,	(player_type*, int, int, char*),(p_ptr, item, value, entry))
		S_EXEC(	ITEM_SMALL_CHAR,	(player_type*, int, int, char),	(p_ptr, item, value, entry[0]))

		S_EXEC(	PPTR_CHAR,         	(player_type*, char),      	(p_ptr, entry[0])) /* TODO: deprecate this */
	}
#undef S_ARG
#undef S_EXEC

	/* Player has definitely spent energy */
	if (p_ptr->energy < old_energy)
	{
		/* Report it */
		return 2;
	}

	/* Done */
	return 1;
}

int send_store_DEPRECATED(player_type *p_ptr, char pos, byte attr, s16b wgt, s16b number, long price, cptr name)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%b" "%c%c%d%d%ul%s", PKT_STORE, pos, attr, wgt, number, price, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_store(player_type *p_ptr, char pos, byte ga, char gc, byte attr, s16b wgt, s16b number, long price, cptr name)
{
	connection_type *ct;
	/* Hack -- use old version of the function */
	if (!client_version_atleast(p_ptr->version, 1,5,3))
	{
		return send_store_DEPRECATED(p_ptr, pos, attr, wgt, number, price, name);
	}
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%b" "%b%c%c%c%d%d%ul%s", PKT_STORE, pos, ga, gc, attr, wgt, number, price, name) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_store_info(player_type *p_ptr, byte flag, cptr name, char *owner, int items, long purse)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (cq_printf(&ct->wbuf, "%c%c%s%s%d%l", PKT_STORE_INFO, flag, name, owner, items, purse) <= 0)
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_confirm_request(player_type *p_ptr, byte type, cptr buf)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (!cq_printf(&ct->wbuf, "%c" "%c%c%s", PKT_CONFIRM, type, 0x00, buf))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_pickup_check(player_type *p_ptr, cptr buf)
{
	return send_confirm_request(p_ptr, 0x03, buf);
}

int send_store_sell(player_type *p_ptr, u32b price)
{
	char buf[80];
	sprintf(buf, "Accept %" PRId32 " gold?", price);
	return send_confirm_request(p_ptr, 0x01, buf);
}

int send_store_leave(player_type *p_ptr)
{
	connection_type *ct;
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];

	if (!cq_printf(&ct->wbuf, "%c", PKT_STORE_LEAVE))
	{
		client_withdraw(ct);
	}
	return 1;
}

int send_party_info(player_type *p_ptr)
{
	connection_type *ct;
	char *name = "";
	char *owner = "";
	if (p_ptr->conn == -1) return -1;
	ct = Conn[p_ptr->conn];
	if (p_ptr->party > 0)
	{
		name = parties[p_ptr->party].name;
		owner = parties[p_ptr->party].owner;
	}
	if (!cq_printf(&ct->wbuf, "%c" "%s%s", PKT_PARTY, name, owner))
	{
		client_withdraw(ct);
	}
	return 1;
}


/* This gets called before EVERY gameplay command. */
void do_cmd__before(player_type *p_ptr, byte pkt)
{
	/* Player is doing something? Reset AKF timer */
	p_ptr->afk_seconds = 0;

	/* Assume non-custom commands have their own hacks */
	if (pkt_command[pkt] >= MAX_CUSTOM_COMMANDS)
	{
		/* Do nothing */
		return;
	}
	/* Command is going to cost energy -- disturb if resting */
	if (pcommand_energy_cost[pkt] && p_ptr->resting)
	{
		disturb(p_ptr, 0, 1);
	}
}

/* This gets called after every *EXECUTED* gameplay command.
 * See "Gameplay commands" above for definition of "result".
 */
void do_cmd__after(player_type *p_ptr, byte pkt, int result)
{
	/* Some fatal error occured, we don't care */
	if (result <= -1) return;

	/* Paranoia -- player did not have enough energy to execute the command */
	if (result == 0) return;

	/* Hack -- after player does something, reset bubble color */
	p_ptr->bubble_colour = TERM_WHITE;
	p_ptr->bubble_change = turn; /* Delay next blink somewhat */

	/* Hack -- Add noise for commands that cost energy */
	if (pcommand_energy_cost[pkt])
	{
		int halve = 1;
		int v;
		
		/* If he did not spend any energy, only add half the noise */
		if (result == 1)
		{
			halve = 2;
		}
		v = (30 - p_ptr->skill_stl) / pcommand_energy_cost[pkt] / halve;
		set_noise(p_ptr, p_ptr->noise + v);
	}

	/* Classic MAnghack #5. Reset built-up energy. */
	if (pcommand_energy_cost[pkt]) /* For commands that cost energy */
	{
		p_ptr->energy_buildup = 0;
	}
}


/* New version of "process_pending_commands"
 *  for now, returns "-1" incase of an error..
 */
int process_player_commands(player_type *p_ptr)
{
	byte pkt;
	int result = 1;
	int start_pos = 0;

	/* parse */
	while ( cq_len(&p_ptr->cbuf) )
	{
		/* remember position */
		start_pos = p_ptr->cbuf.pos;
		/* read out command */
		next_pkt = pkt = CQ_GET(&p_ptr->cbuf);
		/* pre-execute hacks */
		do_cmd__before(p_ptr, pkt);
		/* execute command */
		result = (*pcommands[pkt])(p_ptr);
		/* post-execute hacks */
		if (result) do_cmd__after(p_ptr, pkt, result);
		/* not a "continuing success" */
		if (!(result >= 1 && result <= 2)) break;
	}
	/* not enough energy, step back */
	if (result == 0) p_ptr->cbuf.pos = start_pos;
	/* slide buffer to the left */
	else if (result >= 1) cq_slide(&p_ptr->cbuf);

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
		
		pkt_command[i] = MAX_CUSTOM_COMMANDS + 1; /* invalid value */
		pcommand_energy_cost[i] = 1; /* an OK default */
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
		pkt_command[pkt] = i;

		receiv[pkt] = recv_command;
		scheme[pkt] = custom_command_schemes[custom_commands[i].scheme];
		pcommand_energy_cost[pkt] = custom_commands[i].energy_cost;
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
