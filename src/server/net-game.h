/*
 * This file should not be included in the makefile, it will be called
 * manually when needed from the source.
 * */ 

#ifdef PACKET
	PACKET(PKT_KEEPALIVE,	"%l",       	recv_keepalive)
	PACKET(PKT_CHAR_INFO,	NULL,       	recv_char_info)
	PACKET(PKT_PLAY,    	"%c",       	recv_play)

	PACKET(PKT_BASIC_INFO,	"%c%d",     	recv_basic_request)
	PACKET(PKT_VISUAL_INFO,	NULL,       	recv_visual_info)
	PACKET(PKT_RESIZE,    	"c%c%c",    	recv_stream_size)
	PACKET(PKT_OPTIONS,    	NULL,       	recv_options)
	PACKET(PKT_SETTINGS,   	NULL,       	recv_settings)

	PACKET(PKT_TERM_INIT,	"%c",       	recv_term_init)
	PACKET(PKT_KEY,     	"%c",       	recv_term_key)

	PACKET(PKT_REDRAW,    	"",         	recv_redraw)
	PACKET(PKT_CLEAR,    	"",         	recv_clear)

	PACKET(PKT_MESSAGE,    	"%s",       	recv_message)
	PACKET(PKT_CHANNEL,    	"%ud%c%s",     	recv_channel)

	PACKET(PKT_WALK,    	"%c",       	recv_command)
	PACKET(PKT_REST,    	"",         	recv_command)
#endif

#ifdef PCOMMAND
	PCOMMAND(PKT_WALK,    	"%c",       	recv_walk)
	PCOMMAND(PKT_REST,    	"",         	recv_toggle_rest)
#endif
