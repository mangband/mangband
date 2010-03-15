/*
 * This file should not be included in the makefile, it will be called
 * manually when needed from the source.
 * */ 

#ifdef PACKET
	PACKET(PKT_KEEPALIVE,	"%l",       	recv_keepalive)
	PACKET(PKT_CHAR_INFO,	NULL,       	recv_char_info)
	PACKET(PKT_PLAY,    	"%c",       	recv_play)	

	PACKET(PKT_MESSAGE,    	"%s",       	recv_message)

	PACKET(PKT_WALK,    	"%c",       	recv_command)
#endif

#ifdef PCOMMAND
	PCOMMAND(PKT_WALK,    	"%c",       	recv_walk)
#endif
