/*
 * This file should not be included in the makefile, it will be called
 * manually when needed from the source.
 * */ 

#ifdef PACKET
	PACKET(PKT_KEEPALIVE,  	"%l",   	recv_keepalive)
	PACKET(PKT_PLAY,    	"%c",   	recv_play)
	PACKET(PKT_QUIT,    	"%S",   	recv_quit)
	PACKET(PKT_CHAR_INFO,	"%d%d%d%d",	recv_char_info)
	PACKET(PKT_STRUCT_INFO,	NULL,   	recv_struct_info)
	
	PACKET(PKT_MESSAGE, 	"%s%d",   	recv_message)
#endif
