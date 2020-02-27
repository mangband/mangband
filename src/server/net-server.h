#ifndef _NET_SERVER_H
#define _NET_SERVER_H

#include "../common/net-basics.h"
#include "../common/net-imps.h"

/* For websocket handshake: */
#include "../common/base64encode.h"
#include "../common/sha1.h"

/* 
 * Note: this file is not to be included with all the rest of the .h files.
 * It is manually included from "net-server.c" and "net-game.c"
 */

/* Helper(s) */
typedef int (*sccb)	(connection_type *ct, player_type *p_ptr); /* "Static Command Call-Back" */ 

/** net-server.c **/
extern server_setup_t serv_info;
extern eptr first_connection;

extern cptr next_scheme;
extern byte next_pkt;

extern int *Get_Conn;	/* Pass "Ind", get "ind" */
extern int *Get_Ind; 	/* Pass "ind", get "Ind" */
extern connection_type **Conn; /* Pass "ind", get "connection_type" */
extern connection_type **PConn; /* Pass "Ind", get "connection_type" */

#define client_abort(CT, REASON) send_quit(CT, REASON); return -1
extern int client_kill(connection_type *ct, cptr reason);
#ifdef DEBUG
#define client_withdraw(CT) plog_fmt("Withdrawing client %s! Error '%s' on %s : %d", (CT)->host_addr, cq_error(&((CT)->wbuf)), __FILE__, __LINE__); client_kill(CT, "write error"); return -1
#else
#define client_withdraw(CT) client_kill(CT, "write error"); return -1
#endif

extern int player_enter(int ind);
extern int player_leave(int p_idx);
extern void player_drop(int ind);

extern bool client_names_ok(char *nick_name, char *real_name, char *host_name);
extern bool client_version_ok(u16b version);
extern u16b connection_type_ok(u16b version);
extern bool client_version_atleast(u16b version, int at_major, int at_minor, int at_patch);


/** net-game.c **/
/* Setup */
extern void setup_tables(sccb receiv[256], cptr *playing_schemes);
/* Send */
extern int send_server_info(connection_type *ct);
extern int send_play(connection_type *ct, byte mode);
extern int send_quit(connection_type *ct, const char *reason);
extern int send_char_info(connection_type *ct, player_type *p_ptr);
extern int send_stats_info(connection_type *ct);
extern int send_race_info(connection_type *ct);
extern int send_class_info(connection_type *ct);
extern int send_optgroups_info(connection_type *ct);
extern int send_options_info(connection_type *ct, player_type *p_ptr, int id);
extern int send_indicator_info(connection_type *ct, int id);
extern int send_custom_command_info(connection_type *ct, int id);
/* Receive */
//Not really needed .. //
//extern int recv_undef(connection_type *ct, player_type *p_ptr);

/** control.c **/
int accept_console(int data1, data data2);
int console_read(int data1, data data2);
int console_close(int data1, data data2);

#endif
