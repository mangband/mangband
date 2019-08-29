/*
 * Client-to-Server packet definitions.
 */
/*
 * This file should not be compiled with the rest of the source,
 * it is included manually when needed.
 */

/*
 * "System" Packets.
 *
 * Those are executed upon arrival on the network.
 * NULL for scheme means the scheme varies and is probably hard-coded
 * into the recv_ function itself, while "" stands for empty scheme.
 */
#ifdef PACKET
	PACKET(PKT_KEEPALIVE,	"%l",   	recv_keepalive)
	PACKET(PKT_CHAR_INFO,	NULL,   	recv_char_info)
	PACKET(PKT_PLAY,	"%c",   	recv_play)

	PACKET(PKT_BASIC_INFO,	"%c%d", 	recv_basic_request)
	PACKET(PKT_VISUAL_INFO,	NULL,   	recv_visual_info)
	PACKET(PKT_RESIZE,	"c%c%c",	recv_stream_size)
	PACKET(PKT_OPTIONS,	NULL,   	recv_options)
	PACKET(PKT_SETTINGS,	NULL,   	recv_settings)

	PACKET(PKT_TERM_INIT,	"%c",   	recv_term_init)
	PACKET(PKT_KEY, 	"%c",   	recv_term_key)
	PACKET(PKT_CURSOR,	"%c%c%c",	recv_mouse)
	PACKET(PKT_LOOK,	"%c%c", 	recv_target)
	PACKET(PKT_LOCATE,	"%c",   	recv_locate)

	PACKET(PKT_REDRAW,	"",     	recv_redraw)
	PACKET(PKT_CLEAR,	"",     	recv_clear)
	PACKET(PKT_CONFIRM,	"%c%c", 	recv_confirm)

	PACKET(PKT_SUICIDE,	"",     	recv_suicide)
	PACKET(PKT_CHANGEPASS,	"%s",   	recv_pass)
	PACKET(PKT_MESSAGE,	"%s",   	recv_message)
	PACKET(PKT_CHANNEL,	"%ud%c%s",	recv_channel)
	PACKET(PKT_PARTY,	"%d%s", 	recv_party)

	PACKET(PKT_COMMAND,	NULL,   	recv_command)
#endif

/*
 * Player commands.
 *
 * Those are first handled by the "recv_command" routine,
 * then copied to the player's command buffer, and are then
 * executed when needed energy (if any) is available.
 *
 * Note: each PCOMMAND definition automatically invokes a
 * PACKET(..., NULL, recv_command) definition.
 *
 * Note: the rest of player commands are defined via
 * "custom_commands" mechanism in tables.c
 */
#ifdef PCOMMAND
	PCOMMAND(PKT_WALK,	"%c",   	recv_walk)
	PCOMMAND(PKT_REST,	"",     	recv_toggle_rest)
	PCOMMAND(PKT_PATHFIND,	"%c%c", 	recv_pathfind)
	PCOMMAND(PKT_ENTER_FEAT,"",     	recv_enterfeat)
#endif
