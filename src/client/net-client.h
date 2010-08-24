/*
 * This file should not be included in the makefile, it will be called
 * manually when needed from the source.
 * */ 

#ifdef PACKET
	PACKET(PKT_KEEPALIVE,  	"%l",   	recv_keepalive)
	PACKET(PKT_PLAY,    	"%c",   	recv_play)
	PACKET(PKT_QUIT,    	"%S",   	recv_quit)
	PACKET(PKT_BASIC_INFO,	"%d%d%d%d",	recv_basic_info)
	PACKET(PKT_CHAR_INFO,	"%d%d%d%d",	recv_char_info)
	PACKET(PKT_STRUCT_INFO,	NULL,   	recv_struct_info)

	PACKET(PKT_INDICATOR,	NULL,   	recv_indicator_info)
	PACKET(PKT_STREAM,  	NULL,   	recv_stream_info)
	PACKET(PKT_RESIZE,     	NULL,   	recv_stream_size)
	PACKET(PKT_COMMAND,  	NULL,   	recv_custom_command_info)
	PACKET(PKT_ITEM_TESTER,	NULL,   	recv_item_tester_info)

	PACKET(PKT_MESSAGE, 	"%s%d",   	recv_message)
#endif
