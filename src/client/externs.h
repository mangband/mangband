/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */


/*
 * Not-so-Automatically generated "variable" declarations
 */

/* c-tables.c */
extern s16b ddx[10];
extern s16b ddy[10];
extern char hexsym[16];
extern option_type local_option_info[MAX_OPTIONS];
extern cptr local_option_group[];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr ang_term_name[8];
extern cptr window_flag_desc[32];

/* set_focus.c */
extern void set_chat_focus( void );
extern void unset_chat_focus( void );
extern void stretch_chat_ctrl( void );




/* variable.c */
extern char nick[80];
extern char pass[80];

extern char real_name[80];

extern char server_name[80];
extern int server_port;

extern object_type inventory[INVEN_TOTAL];
extern char inventory_name[INVEN_TOTAL][80];

extern object_type floor_item;
extern char floor_name[80];

extern store_type store;
extern owner_type store_owner;
extern char store_owner_name[MAX_CHARS];
extern char store_name[MAX_CHARS];
extern byte store_flag;
extern int store_prices[STORE_INVEN_MAX];
extern char store_names[STORE_INVEN_MAX][80];
extern s16b store_num;

extern char spell_info[26][SPELLS_PER_BOOK+1][80];
extern byte spell_flag[26 * (SPELLS_PER_BOOK+1)];

extern char party_info[160];

extern server_setup_t Setup;
extern client_setup_t Client_setup;

s16b lag_mark;
s16b lag_minus;
char health_track_num;
byte health_track_attr;

extern bool shopping;
extern bool shopping_buying; 
extern bool leave_store;

extern s16b last_line_info;
extern s16b cur_line;
extern s16b max_line;

extern cave_view_type remote_info[8][MAX_TXT_INFO][80];
extern s16b last_remote_line[8];

extern player_type player;
extern player_type *p_ptr;
extern s32b exp_adv;

extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;

extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern byte item_tester_tvals[MAX_ITEM_TESTERS][MAX_ITH_TVAL];
extern byte item_tester_flags[MAX_ITEM_TESTERS];

extern int special_line_type;
extern char special_line_header[80];
extern bool special_line_onscreen;

extern bool inkey_base;
extern bool inkey_scan;
extern bool inkey_flag;

extern bool inkey_exit;

extern bool first_escape;

extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;

extern u16b message__next;
extern u16b message__last;
extern u16b message__head;
extern u16b message__tail;
extern u16b *message__ptr;
extern char *message__buf;
extern u16b *message__type;
extern u16b *message__count;


extern char *c_name;
extern char *c_text;
extern char *p_name;
extern player_race *race_info;
extern player_class *c_info;

extern maxima z_info;

extern bool msg_flag;
extern channel_type channels[MAX_CHANNELS];
extern s16b view_channel;

extern term *ang_term[8];
extern u32b window_flag[8];

extern byte color_table[256][4];

extern cptr ANGBAND_SYS;

extern cptr keymap_act[KEYMAP_MODES][256];

extern s16b command_cmd;
extern s16b command_dir;

extern custom_command_type custom_command[MAX_CUSTOM_COMMANDS];
extern s16b custom_commands;

extern s16b race;
extern s16b class;
extern s16b sex;
extern char ptitle[80];

extern s16b stat_order[6];

extern bool topline_icky;
extern bool screen_icky;
extern s16b section_icky_col;
extern byte section_icky_row;
extern bool party_mode;
extern bool cursor_icky;


extern cptr ANGBAND_GRAF;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_PREF;
extern cptr ANGBAND_DIR_XTRA;

extern int use_graphics;
extern bool big_tile;
extern bool use_sound;

extern cave_view_type trans[40][88];

extern option_type *option_info;
extern cptr *option_group;
extern int options_max;
extern int options_groups_max;

extern bool rogue_like_commands;
extern bool auto_accept;
extern bool depth_in_feet;
extern bool show_weights;
extern bool ring_bell;
extern bool use_color;

extern int char_screen_mode;
extern bool target_recall;
extern char target_prompt[60];
/*
 * Not-so-Automatically generated "function declarations"
 */

/* c-birth.c */
extern void get_char_name(void);
extern void get_char_info(void);
extern bool get_server_name(void);

/* c-cmd.c */
extern void process_command(void);
extern void cmd_custom(byte i);
extern void cmd_tunnel(void);
extern void cmd_walk(void);
extern void cmd_run(void);
extern void cmd_stay(void);
extern void cmd_map(void);
extern void cmd_locate(void);
extern void cmd_search(void);
extern void cmd_toggle_search(void);
extern void cmd_rest(void);
extern void cmd_go_up(void);
extern void cmd_go_down(void);
extern void cmd_open(void);
extern void cmd_close(void);
extern void cmd_bash(void);
extern void cmd_disarm(void);
extern void cmd_inven(void);
extern void cmd_equip(void);
extern void cmd_drop(void);
extern void cmd_drop_gold(void);
extern void cmd_wield(void);
extern void cmd_take_off(void);
extern void cmd_destroy(void);
extern void cmd_inscribe(void);
extern void cmd_uninscribe(void);
extern void cmd_describe(void);
extern void cmd_spike(void);
extern void cmd_steal(void);
extern void cmd_quaff(void);
extern void cmd_read_scroll(void);
extern void cmd_aim_wand(void);
extern void cmd_use_staff(void);
extern void cmd_zap_rod(void);
extern void cmd_refill(void);
extern void cmd_eat(void);
extern void cmd_activate(void);
extern int cmd_target_interactive(int mode);
extern int cmd_target(void);
extern int cmd_target_friendly(void);
extern void cmd_look(void);
extern void cmd_character(void);
extern void cmd_artifacts(void);
extern void cmd_uniques(void);
extern void cmd_players(void);
extern void cmd_high_scores(void);
extern void cmd_help(void);
extern void cmd_query_symbol(void);
extern void cmd_chat();
extern void cmd_message(void);
extern void cmd_chat_close(int i);
extern void cmd_chat_cycle(int dir);
extern void cmd_party(void);
extern void cmd_fire(void);
extern void cmd_throw(void);
extern void cmd_browse(void);
extern void cmd_study(void);
extern void cmd_cast(void);
extern void cmd_pray(void);
extern void cmd_ghost(void);
extern void cmd_load_pref(void);
extern void cmd_redraw(void);
extern void cmd_purchase_house(void);
extern void cmd_suicide(void);
extern void cmd_master(void);
extern void cmd_master_aux_level(void);
extern void cmd_master_aux_build(void);
extern void cmd_master_aux_summon(void);
extern void cmd_observe(void);

/* c-files.c */
extern void text_to_ascii(char *buf, cptr str);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fclose(FILE *fff);
extern void init_stuff();
extern void init_file_paths(char *path);
extern errr process_pref_file(cptr buf);
extern errr process_pref_file_command(char *buf);
extern void show_motd(void);
extern void peruse_file(void);
extern errr Save_options(void);
extern void conf_init(void* param);									/* Client config section */
extern void conf_save();
extern void conf_timer(int ticks);
extern bool conf_section_exists(cptr section);
extern cptr conf_get_string(cptr section, cptr name, cptr default_value);
extern s32b conf_get_int(cptr section, cptr name, s32b default_value);
extern void conf_set_string(cptr section, cptr name, cptr value);
extern void conf_set_int(cptr section, cptr name, s32b value);

/* c-init.c */
extern bool client_ready(void);
extern void initialize_all_pref_files(void);
extern void client_init(char *argv1);

/* c-inven.c */
extern s16b index_to_label(int i);
extern bool item_tester_okay(object_type *o_ptr);
extern bool c_get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor);
extern bool c_check_item(int *item, byte tval);
extern bool c_get_spike(void);

/* c-util.c */
extern void move_cursor(int row, int col);
extern void flush(void);
extern void flush_now(void);
extern void set_graphics(int mode);
extern void macro_add(cptr pat, cptr act, bool cmd_flag);
extern char inkey(void);
extern void bell(void);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern bool get_string(cptr prompt, char *buf, int len);
extern bool get_com(cptr prompt, char *command);
extern void request_command(bool shopping);
extern int target_dir(char ch);
extern bool c_get_dir(char *dp, cptr prompt, bool allow_target, bool allow_friend);
extern bool get_dir(int *dp);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern bool get_check(cptr prompt);
extern s16b message_num(void);
extern cptr message_last(void);
extern cptr message_str(s16b age);
extern u16b message_type(s16b age);
extern void c_message_add(cptr msg, u16b type);
extern void c_message_del(s16b age);
extern void c_msg_print_aux(cptr msg, u16b type);
extern void c_msg_print(cptr msg);
extern s32b c_get_quantity(cptr prompt, s32b max);
extern errr path_build(char *buf, int max, cptr path, cptr file);
extern bool askfor_aux(char *buf, int len, char private);
extern void clear_from(int row);
extern int caveprt(cave_view_type* src, int len, s16b x, s16b y);
extern int cavecpy(cave_view_type* dest, cave_view_type* src, int len);
extern int cavestr(cave_view_type* dest, cptr src, byte attr, int max_col);
extern void prt_num(cptr header, int num, int row, int col, byte color);
extern void prt_lnum(cptr header, s32b num, int row, int col, byte color);
extern void interact_macros(void);
extern void do_cmd_options(void);
extern void do_cmd_options_birth(void);
extern bool get_string_masked(cptr prompt, char *buf, int len);

/* c-spell.c */
extern int get_spell(int *sn, cptr p, cptr prompt, int *bn, bool known);
extern void show_browse(int book);
extern void do_study(int book);
extern void do_cast(int book);
extern void do_pray(int book);
extern void do_ghost(void);

/* c-store.c */
extern void display_inventory(void);
extern void display_store(void);

/* c-xtra1.c */
extern void show_inven(void);
extern void show_equip(void);
extern void fix_message(void);
extern void display_player(int screen_mode);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void fix_floor(void);
extern bool message_color(cptr msg, byte *ap);
extern int find_whisper_tab(cptr msg, char *text);
extern void prt_map_easy(void);
extern void prt_player_hack(bool force);

/* c-xtra2.c */
extern void do_cmd_messages(void);

/* client.c */

/* netclient.c */
extern int conn_state;			/* Similar to server's connp->state */
extern sockbuf_t rbuf, cbuf, wbuf, qbuf;
extern int ticks;
extern int lag_ok;
extern void update_ticks();
extern void do_keepalive();
extern void net_term_resize(int rows, int cols);
extern bool Net_Send(int Socket, sockbuf_t* ibuf);
extern bool Net_WaitReply(int Socket, sockbuf_t* ibuf, int retries);
extern int Net_setup(void);
extern int Net_verify(void);
extern int Net_init(int port);
extern void Net_cleanup(void);
extern int Net_flush(void);
extern int Net_fd(void);
extern int Net_input(void);
extern int Net_packet(void);
extern int Flush_queue(void);

extern int Send_motd(int offset);
extern int Send_play(int mode);

extern int Send_custom_command(byte i, char item, char dir, s32b value, char *entry);

extern int Send_search(void);
extern int Send_walk(int dir);
extern int Send_run(int dir);
extern int Send_drop(int item, int amt);
extern int Send_drop_gold(s32b amt);
extern int Send_tunnel(int dir);
extern int Send_stay(void);
extern int Send_toggle_search(void);
extern int Send_rest(void);
extern int Send_go_up(void);
extern int Send_go_down(void);
extern int Send_open(int dir);
extern int Send_close(int dir);
extern int Send_bash(int dir);
extern int Send_disarm(int dir);
extern int Send_wield(int item);
extern int Send_take_off(int item);
extern int Send_destroy(int item, int amt);
extern int Send_inscribe(int item, cptr buf);
extern int Send_uninscribe(int item);
extern int Send_spike(int dir);
extern int Send_quaff(int item);
extern int Send_read(int item);
extern int Send_aim(int item, int dir);
extern int Send_use(int item);
extern int Send_zap(int item);
extern int Send_fill(int item);
extern int Send_eat(int item);
extern int Send_activate(int item);
extern int Send_target_interactive(int mode, char dir);
extern int Send_chan(cptr channel);
extern int Send_msg(cptr message);
extern int Send_fire(int item, int dir);
extern int Send_throw(int item, int dir);
extern int Send_item(int item);
extern int Send_direction(int dir);
extern int Send_gain(int book, int spell);
extern int Send_cast(int book, int spell);
extern int Send_pray(int book, int spell);
extern int Send_ghost(int ability);
extern int Send_map(void);
extern int Send_locate(int dir);
extern int Send_store_purchase(int item, int amt, u32b price);
extern int Send_store_sell(int item, int amt);
extern int Send_store_leave(void);
extern int Send_store_confirm(void);
extern int Send_redraw(void);
extern int Send_special_line(int type, int line);
extern int Send_party(s16b command, cptr buf);
extern int Send_suicide(void);
extern int Send_options(bool settings);
extern int Send_master(s16b command, cptr buf);
extern int Send_observe(int item);
extern int Send_pass(cptr newpass);



/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifndef HAS_MEMSET
/* util.c */
extern char *memset(char*, int, huge);
#endif

#ifndef HAS_STRICMP
/* util.c */
extern int stricmp(cptr a, cptr b);
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void main(void); */
#endif

#ifdef WINDOWS
/* main-win.c */
/* extern int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, ...); */
#endif
