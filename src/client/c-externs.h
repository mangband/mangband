/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */


/*
 * Not-so-Automatically generated "variable" declarations
 */

extern int max_macrotrigger;
extern cptr macro_template;
extern cptr macro_modifier_chr;
extern cptr macro_modifier_name[MAX_MACRO_MOD];
extern cptr macro_trigger_name[MAX_MACRO_TRIGGER];
extern cptr macro_trigger_keycode[2][MAX_MACRO_TRIGGER];

/* c-tables.c */
extern s16b ddx[10];
extern s16b ddy[10];
extern char hexsym[16];
extern byte ascii_to_color[128]; 
extern option_type local_option_info[MAX_OPTIONS];
extern cptr local_option_group[];
extern cptr ang_term_name[8];
extern cptr window_flag_desc[32];
extern cptr angband_sound_name[MSG_MAX];

/* set_focus.c */
extern void set_chat_focus( void );
extern void unset_chat_focus( void );
extern void stretch_chat_ctrl( void );
extern int win32_window_visible(int i);

/* z-term.c */
extern event_type inkey_ex(void);


/* variable.c */
extern char nick[MAX_CHARS];
extern char pass[MAX_CHARS];

extern char real_name[MAX_CHARS];

extern char server_name[MAX_CHARS];
extern int server_port;

extern object_type *inventory;
extern char **inventory_name;
extern char **inventory_name_one;
extern byte *inventory_secondary_tester;

extern object_type floor_item;
extern char floor_name[MAX_CHARS];
extern char floor_name_one[MAX_CHARS];
extern byte floor_secondary_tester;


extern indicator_type indicators[MAX_INDICATORS];
extern int known_indicators;
extern u32b indicator_window[MAX_INDICATORS];
extern char* str_coffers[MAX_COFFERS];
extern s32b coffers[MAX_COFFERS];
extern byte coffer_refs[MAX_INDICATORS];
extern int known_coffers;

extern store_type store;
extern owner_type store_owner;
extern char store_owner_name[MAX_CHARS];
extern char store_name[MAX_CHARS];
extern byte store_flag;
extern int store_prices[STORE_INVEN_MAX];
extern char store_names[STORE_INVEN_MAX][MAX_CHARS];
extern s16b store_num;

extern char spell_info[26][SPELLS_PER_BOOK+1][MAX_CHARS];
extern byte spell_flag[26 * (SPELLS_PER_BOOK+1)];
extern byte spell_test[26 * (SPELLS_PER_BOOK+1)];

extern char party_info[160];

extern client_setup_t Client_setup;
extern server_setup_t Setup;

extern s16b lag_mark;
extern s16b lag_minus;
extern bool redraw_lag_meter;
extern char health_track_num;
extern byte health_track_attr;

extern bool shopping;
extern bool shopping_buying; 
extern bool leave_store;
extern bool enter_store;

extern s16b last_line_info;
extern s16b cur_line;
extern s16b max_line;

extern cave_view_type* remote_info[16];
extern s16b last_remote_line[16];
extern cptr stream_desc[32];

extern cave_view_type sfx_info[MAX_HGT][MAX_WID];
extern s32b sfx_delay[MAX_HGT][MAX_WID];
extern void slashfx_dir_offset(int *x, int *y, int dir, bool invert);

extern cave_view_type air_info[MAX_HGT][MAX_WID];
extern s32b air_delay[MAX_HGT][MAX_WID];
extern s32b air_fade[MAX_HGT][MAX_WID];
extern bool air_updates;
extern bool air_refresh;

extern player_type player;
extern player_type *p_ptr;
extern s32b exp_adv;

extern bool prompt_quote_hack;

extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;

extern bool spellcasting;
extern int spellcasting_spell;

extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern item_tester_type item_tester[MAX_ITEM_TESTERS];
extern int known_item_testers;

extern int inven_out_index[256];

extern int special_line_type;
extern char special_line_header[MAX_CHARS];
extern bool special_line_onscreen;
#define interactive_mode special_line_onscreen

extern bool special_line_requested;
extern bool simple_popup_requested;
extern bool local_browser_requested;

extern bool confirm_requested;
extern byte confirm_type;
extern byte confirm_id;
extern char confirm_prompt[MAX_CHARS];

extern bool pause_requested;

#define enter_store_requested enter_store

extern bool escape_in_macro_triggers;

extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;

extern bool inkey_exit;
extern bool inkey_nonblock;

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


extern char *eq_name;
extern char *c_name;
extern char *c_text;
extern char *p_name;
extern player_race *race_info;
extern player_class *c_info;
extern s16b *eq_names;
extern byte *eq_xpos;
extern byte *eq_ypos;

extern maxima z_info;

extern s16b INVEN_TOTAL;
extern s16b INVEN_WIELD;
extern s16b INVEN_PACK;

extern s16b FLOOR_INDEX;
extern bool FLOOR_NEGATIVE;
extern s16b FLOOR_TOTAL;

extern u16b MAX_OBJFLAGS_ROWS;
extern u16b MAX_OBJFLAGS_COLS;

extern bool msg_flag;
extern channel_type channels[MAX_CHANNELS];
extern s16b view_channel;

extern term *ang_term[8];
extern u32b window_flag[8];
extern u32b window_flag_o[8];

extern byte color_table[256][4];

extern cptr ANGBAND_SYS;

extern cptr ANGBAND_FON;
extern cptr ANGBAND_FONTNAME;
extern cptr ANGBAND_GRAFNAME;

extern byte mousemap[0xFF];

extern cptr keymap_act[KEYMAP_MODES][256];

extern s16b command_cmd;
extern s16b command_dir;
extern event_type command_cmd_ex;

extern custom_command_type custom_command[MAX_CUSTOM_COMMANDS];
extern int custom_commands;

extern stream_type streams[MAX_STREAMS];
extern int known_streams;

extern byte window_to_stream[ANGBAND_TERM_MAX];
extern byte stream_group[MAX_STREAMS];
extern byte stream_groups;


extern s16b race;
extern s16b pclass;
extern s16b sex;
extern char ptitle[80];

extern byte A_MAX;
extern cptr* stat_names;
extern s16b stat_order[A_CAP];

extern bool flip_inven;
extern s16b flip_charsheet;

extern bool topline_icky;
extern bool screen_icky;
extern s16b section_icky_col;
extern byte section_icky_row;
extern bool party_mode;
extern bool cursor_icky;
extern bool looking;
extern bool in_item_prompt;

extern byte icky_levels;

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
extern cptr ANGBAND_DIR_XTRA_SOUND;
extern cptr ANGBAND_DIR_XTRA_GRAF;

extern int use_graphics;
extern bool big_tile;
extern bool use_sound;

extern cptr sound_file[MSG_MAX][SAMPLE_MAX];

extern cave_view_type trans[40][88];

extern option_type *option_info;
extern cptr *option_group;
extern int options_max;
extern int options_groups_max;
extern int known_options;

extern bool rogue_like_commands;
extern bool auto_accept;
extern bool auto_itemlist;
extern bool auto_showlist;
extern bool depth_in_feet;
extern bool show_labels;
extern bool show_weights;
extern bool wrap_messages;
extern bool ring_bell;
extern bool use_color;

extern bool ignore_birth_options;

extern int char_screen_mode;
extern bool target_recall;
extern char target_prompt[60];

extern byte hitpoint_warn_toggle;
/*
 * Not-so-Automatically generated "function declarations"
 */

/* c-birth.c */
extern void get_char_name(void);
extern void get_char_info(void);
extern bool get_server_name(void);

/* c-cmd.c */
extern void process_command(void);
extern void process_requests(void);
extern void cmd_custom(byte i);
extern void cmd_interactive(byte line_type, bool use_anykey);
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
extern void cmd_chat(void);
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
extern void cmd_mouseclick(void);
extern void toggle_inven_equip(void);
/* c-cmd0.c */
extern void do_cmd_port(void);
extern char do_cmd_menu(void);
extern void cmd_init(void);
extern void do_cmd_term_mousepress(u32b termflag, int x, int y, int button);
extern void cmd_term_mousepress(int i, int x, int y, int button);
extern void do_cmd_use_item(int item, bool agressive);
extern void cmd_use_item(void);
extern bool command_tester_okay(int custom_command_id, int item);
extern char* macro_find_by_action(cptr buf);
extern custom_command_type *match_custom_command(char cmd, bool shop);
extern int command_to_display_name(char cmd, char *dst, size_t len);
extern char command_from_keystroke(char *buf);
extern char command_by_item(int item, bool agressive);
extern int command_as_keystroke(char cmd, char *dst, size_t len);
extern int item_as_keystroke(int item, char cmd, char *dst, size_t len, byte ctxt_flag);
extern int spell_as_keystroke(int spell, int book, char cmd, char *dst, size_t len, byte ctxt_flag);


/* c-files.c */
extern void text_to_ascii(char *buf, size_t max, cptr str);
extern void init_stuff(void);
extern void init_file_paths(char *path);
extern void free_file_paths(void);
extern void import_user_pref_files(cptr dest_path);
extern errr process_pref_file(cptr buf);
extern errr process_pref_file_command(char *buf);
extern void show_motd(void);
extern void show_recall(byte win, cptr prompt);
extern void prepare_popup(int line_type, bool use_anykey);
extern void show_popup(void);
extern void show_remote_peruse(s16b line);
extern void show_file_peruse(s16b line);
extern void stash_remote_info(void);
extern void peruse_file(void);
extern errr Save_options(void);
extern void conf_init(void* param);	/* Client config section */
extern void conf_done(void);
extern void conf_save(void);
extern void conf_timer(int ticks);
extern bool conf_section_exists(cptr section);
extern cptr conf_get_string(cptr section, cptr name, cptr default_value);
extern s32b conf_get_int(cptr section, cptr name, s32b default_value);
extern void conf_set_string(cptr section, cptr name, cptr value);
extern void conf_set_int(cptr section, cptr name, s32b value);
extern void conf_append_section(cptr section, cptr filename);
extern void clia_init(int argc, const char *argv[]);
extern int clia_find(const char *key); 
extern bool clia_cpy_string(char *dst, int len, int i);
extern bool clia_cpy_int(s32b *dst, int i);
extern bool clia_read_string(char *dst, int len, const char *key);
extern bool clia_read_int(s32b *dst, const char *key);
extern bool clia_read_bool(bool *dst, const char *key);

/* c-init.c */
extern bool sync_data(void);
extern bool client_login(void);
extern bool client_ready(void);
extern bool client_setup(void);
extern void wipe_visual_prefs(void);
extern void initialize_all_pref_files(void);
extern void client_init(void);
extern  int client_failed(void);
extern void gather_settings(void);
extern void flush_updates(void);
extern void init_subscriptions(void);
extern void quit_hook(cptr str);

/* c-inven.c */
extern char index_to_label(int i);
extern bool item_tester_okay(object_type *o_ptr);
extern bool item_tester_hack(object_type *o_ptr, int i); /* Do not use this */
extern bool c_get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor_);
extern bool c_check_item(int *item, byte tval);
extern byte c_secondary_tester(int item);
extern int count_items_by_name(char* name, bool inven, bool equip, bool floor_);
extern void do_cmd_inscribe_auto(char *item_name, char *inscription);
extern void do_cmd_uniscribe_by_tag(char cmd, char tag);

/* c-util.c */
extern void move_cursor(int row, int col);
extern void flush(void);
extern void flush_now(void);
extern void set_graphics(int mode);
extern int sound_count(int val);
extern void macro_add(cptr pat, cptr act, bool cmd_flag);
extern int macro_find_exact(cptr pat);
extern errr macro_trigger_free(void);
extern void ascii_to_text(char *buf, size_t len, cptr src);
extern char inkey(void);
extern void bell(void);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern int prt_multi(int sx, int sy, int n, int m, int attr, cptr msg);
extern bool get_string(cptr prompt, char *buf, int len);
extern bool get_com(cptr prompt, char *command);
extern bool get_com_ex(cptr prompt, char *command, event_type *xe);
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
extern void msg_flush(void);
extern void c_msg_print_aux(cptr msg, u16b type);
extern void c_msg_print(cptr msg);
extern s32b c_get_quantity(cptr prompt, s32b max);
extern bool askfor_aux(char *buf, int len, char m_private);
extern void clear_from(int row);
extern int cavedraw(cave_view_type* src, int len, s16b x, s16b y);
extern int caveprt(cave_view_type* src, int len, s16b x, s16b y);
extern int cavemem(cave_view_type* src, int len, s16b x, s16b y);
extern int caveclr(cave_view_type* dest, int len);
extern int cavecpy(cave_view_type* dest, cave_view_type* src, int len);
extern int cavestr(cave_view_type* dest, cptr src, byte attr, int max_col);
extern void mem_line(int y, int x, int cols);
extern void show_line(int y, s16b cols, bool mem, int st);
extern void show_char(s16b y, s16b x, byte a, char c, byte ta, char tc, bool mem);
extern void update_air(void);
extern void update_slashfx(void);
extern void prt_num(cptr header, int num, int row, int col, byte color);
extern void prt_lnum(cptr header, s32b num, int row, int col, byte color);
extern void interact_macros(void);
extern void do_cmd_options(void);
extern void do_cmd_options_birth(void);
extern bool get_string_masked(cptr prompt, char *buf, int len);
#ifdef USE_SOUND
extern void load_sound_prefs(void);
#endif

/* c-spell.c */
extern int get_spell(int *sn, cptr p, cptr prompt, int *bn, bool known, bool bookless);
extern errr get_spell_by_name(int *bn, int *sn, bool inven, bool equip, bool books);
extern void show_browse(int book);
extern void do_study(int book);
extern void do_cast(int book);
extern void do_pray(int book);
extern void do_ghost(void);

/* c-store.c */
extern void display_inventory(void);
extern void display_store(void);
extern int get_store_stock(int *citem, cptr prompt);

/* c-xtra1.c */
extern int register_indicator(int id);
extern void show_inven(void);
extern void show_equip(void);
extern byte find_chat_window(void);
extern void fix_message(void);
extern void display_player(int screen_mode);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void fix_floor(void);
extern bool message_color(cptr msg, byte *ap);
extern int find_whisper_tab(cptr msg, char *text);
extern void prt_map_easy(void);
extern void prt_player_hack(void);
extern void schedule_redraw(u32b filter);
extern void redraw_indicators(u32b filter);


/* c-xtra2.c */
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_chat_open(int id, cptr name);
extern void do_chat_select(int id);
extern void do_chat_close(int id);
extern void do_handle_message(cptr mesg, u16b type);


/* client.c */

/* net-client.c */
extern s16b state;
extern bool net_term_clamp(byte win, byte *y, byte *x);
extern u32b net_term_manage(u32b* old_flag, u32b* new_flag, bool clear);
extern u32b net_term_update(bool clear);
extern void net_visuals_update(void);
extern void setup_keepalive_timer(void);
extern void setup_network_client(void);
extern void cleanup_network_client(void);
extern void network_loop(void);
extern int call_metaserver(char *server_name, int server_port, char *buf, int buflen);
extern int call_server(char *server_name, int server_port);
extern server_setup_t serv_info;
extern int send_handshake(u16b conntype);
extern int send_login(u16b version, char* real_name, char* host_name, char* user_name, char* pass_word);
extern int send_pass(char *new_password);
extern int send_settings(void);
extern int send_options(void);
extern int send_play(byte mode);
extern int send_suicide(void);
extern int send_char_info(void);
extern int send_keepalive(u32b last_keepalive);
extern int send_request(byte mode, u16b id);
extern int send_visual_info(byte type);
extern int send_msg(cptr message);
extern int send_redraw(void);
extern int send_clear(void);
extern int send_channel(char mode, u16b id, cptr name);
extern int send_walk(char dir);
extern int send_rest(void);
extern int send_custom_command(byte i, char item, char dir, s32b value, char *entry);
extern int send_party(s16b command, cptr buf);
extern int send_store_leave(void);
extern int send_target_interactive(int mode, char dir);
extern int send_locate(char dir);
extern int send_confirm(byte type, byte id);
extern int send_interactive(byte type);
extern int send_term_key(char key);
extern int send_mouse(byte mod, byte x, byte y);

//TRANSITIONAL HACKAGE:
#define conn_state state
#define update_ticks() plog("update_ticks unimplemented!")
#define do_keepalive() plog("do_keepalive unimplemented!")
#define Flush_queue() flush_updates()
#define Send_run(dir) plog("Send_run unimplemented!")
#define Send_drop_gold(amt) plog("Send_drop_gold unimplemented!")
#define Send_stay() plog("Send_stay unimplemented!")
#define Send_rest() plog("Send_rest unimplemented!")
#define Send_destroy(item, amt) plog("Send_destroy unimplemented!")
#define Send_target_interactive(mode, dir) send_target_interactive(mode, dir)
#define Send_item(item) plog("Send_item unimplemented!")
#define Send_direction(dir) plog("Send_direction unimplemented!")
#define Send_gain(book, spell) plog("Send_gain unimplemented!")
#define Send_cast(book, spell) plog("Send_cast unimplemented!")
#define Send_pray(book, spell) plog("Send_pray unimplemented!")
#define Send_ghost(ability) plog("Send_ghost unimplemented!")
#define Send_store_purchase(item, amt, price) plog("Send_store_purchase unimplemented!")
#define Send_store_sell(item, amt) plog("Send_store_sell unimplemented!")
#define Send_store_confirm() plog("Send_store_confirm unimplemented!")
#define Send_special_line(type, line) plog("Send_special_line unimplemented!")
#define Send_interactive(type) plog("Send_interactive unimplemented!")
#define Send_term_key(ch) plog("Send_term_key unimplemented!")
#define Send_master(command, buf) plog("Send_master unimplemented!")
#define Send_observe(item) plog("Send_observe unimplemented!")


/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void main(void); */
#endif

#ifdef WINDOWS
/* main-win.c */
/* extern int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, ...); */
#endif
