/*
 * This file should not be included in the makefile, it will be called
 * manually when needed from the source.
 * */ 

#ifdef PACKET
	PACKET(PKT_KEEPALIVE,	"%l",   	recv_keepalive)
	PACKET(PKT_PLAY,	"%c",   	recv_play)
	PACKET(PKT_QUIT,	"%S",   	recv_quit)
	PACKET(PKT_BASIC_INFO,	NULL,   	recv_basic_info)
	PACKET(PKT_CHAR_INFO,	"%d%d%d%d",	recv_char_info)
	PACKET(PKT_STRUCT_INFO,	NULL,   	recv_struct_info)

	PACKET(PKT_INDICATOR,	NULL,   	recv_indicator_info)
	PACKET(PKT_STREAM,	NULL,   	recv_stream_info)
	PACKET(PKT_RESIZE,	NULL,   	recv_stream_size)
	PACKET(PKT_COMMAND,	NULL,   	recv_custom_command_info)
	PACKET(PKT_ITEM_TESTER,	NULL,   	recv_item_tester_info)
	PACKET(PKT_OPTION,	NULL,   	recv_option_info)

	PACKET(PKT_TERM,	"%b%ud",	recv_term_info)
	PACKET(PKT_TERM_INIT,	"%s",   	recv_term_header)
	PACKET(PKT_CURSOR,	"%c%c%c",	recv_cursor)
	PACKET(PKT_TARGET_INFO,	"%c%c%c%s",	recv_target_info)

	PACKET(PKT_CHANNEL,	"%ud%c%s",	recv_channel)
	PACKET(PKT_MESSAGE,	"%s%d", 	recv_message)
	PACKET(PKT_MESSAGE_REPEAT,"%d", 	recv_message_repeat)
	PACKET(PKT_SOUND,	"%c",   	recv_sound)
	PACKET(PKT_GHOST,	"%d",   	recv_ghost)
	PACKET(PKT_FLOOR,	"%c%c%d%c%c%s",   	recv_floor)
	PACKET(PKT_INVEN,	"%c%c%ud%d%c%c%s",	recv_inven)
	PACKET(PKT_EQUIP,	"%c%c%ud%c%c%s",	recv_equip)
	PACKET(PKT_SPELL_INFO,	"%c%ud%ud%s",   	recv_spell_info)
	PACKET(PKT_OBJFLAGS,	NULL,   	recv_objflags)
	PACKET(PKT_STORE,	"%c%c%d%d%ul%s",	recv_store)
	PACKET(PKT_STORE_INFO,	"%c%s%s%d%l",   	recv_store_info)
	PACKET(PKT_CONFIRM,	"%c%c%s",	recv_confirm_request)
#endif
