/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */


/*
 * Automatically generated "variable" declarations
 */

/* tables.c */
extern const custom_command_type custom_commands[MAX_CUSTOM_COMMANDS];
extern const stream_type streams[MAX_STREAMS];
extern item_tester_type item_tester[MAX_ITEM_TESTERS];
extern const indicator_type indicators[MAX_INDICATORS];
extern custom_command_type priest_study_cmd;
extern int study_cmd_id;
extern byte eq_pos[INVEN_TOTAL-INVEN_WIELD+1][2];
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern char hexsym[16];
extern byte adj_val_min[];
extern byte adj_val_max[];
extern const int adj_mag_study[];
extern const int adj_mag_mana[];
extern byte adj_mag_fail[];
extern const int adj_mag_stat[];
extern byte adj_chr_gold[];
extern byte adj_int_dev[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte trees_in_town;
extern const int adj_con_mhp[];
extern byte blows_table[12][12];
extern owner_type owners[MAX_STORES][MAX_OWNERS];
extern u16b extract_energy[200];
extern u16b level_speeds[128];
extern s32b player_exp[PY_MAX_LEVEL];
extern player_race race_info[MAX_RACES];
extern player_magic magic_info[MAX_CLASS];
extern byte chest_traps[64];
extern cptr player_title[MAX_CLASS][PY_MAX_LEVEL/5];
extern cptr color_names[16];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr stat_names_full[6];
extern cptr ang_term_name[8];
extern cptr window_flag_desc[32];
extern cptr option_group[];
extern option_type option_info[];
extern const cptr custom_command_schemes[SCHEME_LAST+1];

/* variable.c */
extern cptr copyright[6];
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte version_extra;
extern u16b shutdown_timer;
extern u32b sf_xtra;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern cptr arg_config_file;
extern bool arg_wizard;
extern bool arg_fiddle;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern bool server_generated;
extern bool server_dungeon;
extern bool server_state_loaded;
extern bool server_saved;
extern bool character_loaded;
extern bool character_died;
extern bool character_xtra;
extern u32b seed_flavor;
extern u32b seed_town;
extern s16b command_cmd;
extern s16b command_arg;
/*extern s16b command_rep;*/
extern s16b command_dir;
extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;
extern s16b command_new;
extern s16b choose_default;
extern bool create_up_stair;
extern bool create_down_stair;
extern bool msg_flag;
/*extern bool alive;
extern bool death;*/
/*extern s16b running;*/
extern s16b resting;
/*extern s16b cur_hgt;
extern s16b cur_wid;*/
/*extern s16b dun_level;*/
extern s16b *players_on_depth;
extern s16b special_levels[MAX_SPECIAL_LEVELS];
extern s16b num_repro;
extern s16b object_level;
extern char summon_kin_type;
extern s16b monster_level;
extern byte level_up_x[MAX_DEPTH];
extern byte level_up_y[MAX_DEPTH];
extern byte level_down_x[MAX_DEPTH];
extern byte level_down_y[MAX_DEPTH];
extern byte level_rand_x[MAX_DEPTH];
extern byte level_rand_y[MAX_DEPTH];
extern hturn turn;
extern hturn old_turn;
extern s32b player_id;
extern bool wizard;
extern bool to_be_wizard;
extern bool can_be_wizard;
/*extern u16b total_winner;*/
extern u16b panic_save;
/*extern u16b noscore;*/
extern s16b signal_count;
extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;
extern s16b coin_type;
extern bool opening_chest;
extern int use_graphics;
extern bool use_sound;
extern bool scan_monsters;
extern bool scan_objects;
/*extern s16b total_weight;*/
extern s16b inven_nxt;
/*extern s16b inven_cnt;
extern s16b equip_cnt;*/
extern s32b o_nxt;
extern s32b m_nxt;
extern s32b o_max;
extern s32b m_max;
extern s32b o_top;
extern s32b m_top;
extern s32b p_max;
extern maxima *z_info;
extern u32b eq_name_size;

extern bool cfg_report_to_meta;
extern char * cfg_meta_address;
extern char * cfg_report_address;
extern char * cfg_console_password;
extern bool cfg_console_local_only;
extern char * cfg_dungeon_master;
extern char * cfg_load_pref_file;
extern bool cfg_secret_dungeon_master;
extern s16b cfg_fps;
extern s32b cfg_tcp_port;
extern bool cfg_safe_recharge;
extern bool cfg_no_steal;
extern bool cfg_newbies_cannot_drop;
extern bool cfg_ghost_diving;
extern s32b cfg_level_unstatic_chance;
extern s32b cfg_retire_timer;
extern bool cfg_random_artifacts;
extern s16b cfg_preserve_artifacts;
extern bool cfg_ironman;
extern bool cfg_more_towns;
extern bool cfg_town_wall;
extern s32b cfg_unique_respawn_time;
extern s32b cfg_unique_max_respawn_time;
extern s16b cfg_max_townies;
extern s16b cfg_max_trees;
extern s16b cfg_max_houses;
extern bool cfg_chardump_color;
extern s16b cfg_pvp_hostility;
extern bool cfg_pvp_notify;
extern s16b cfg_pvp_safehostility;
extern s16b cfg_pvp_safedepth;
extern s16b cfg_pvp_saferadius;
extern s16b cfg_pvp_safelevel;
extern bool cfg_party_share_exp;
extern bool cfg_party_share_quest;
extern bool cfg_party_share_kill;
extern bool cfg_party_share_win;
extern s16b cfg_party_sharelevel;
extern bool cfg_instance_closed;

extern s16b hitpoint_warn;
extern s16b delay_factor;
extern bool dungeon_align;
extern s16b feeling;
extern s16b rating;
extern bool good_item_flag;
/*extern bool new_level_flag;*/
extern bool closing_flag;
/*extern s16b max_panel_rows, max_panel_cols;
extern s16b panel_row, panel_col;*/
/*extern s16b panel_row_min, panel_row_max;
extern s16b panel_col_min, panel_col_max;
extern s16b panel_col_prt, panel_row_prt;
extern s16b py;
extern s16b px;*/
/*extern s16b target_who;
extern s16b target_col;
extern s16b target_row;*/
/* extern s16b health_who; */
extern s16b recent_idx;
extern element_group* players;
extern int player_uid;
extern int player_euid;
extern int player_egid;
extern player_type* player_textout;
extern party_type parties[MAX_PARTIES];
extern channel_type channels[MAX_CHANNELS];
extern house_type houses[MAX_HOUSES];
extern byte spell_flags[MAX_SPELL_REALMS][PY_MAX_SPELLS];
extern byte spell_tests[MAX_SPELL_REALMS][PY_MAX_SPELLS];
extern arena_type arenas[MAX_ARENAS];
extern int num_houses;
extern int num_arenas;
extern int chan_audit;
extern int chan_debug;
extern int chan_cheat;
/*extern char player_name[32];
extern char player_base[32];
extern char died_from[80];
extern char history[4][60];
extern char savefile[1024];*/
/*extern s16b lite_n;
extern byte lite_y[LITE_MAX];
extern byte lite_x[LITE_MAX];
extern s16b view_n;
extern byte view_y[VIEW_MAX];
extern byte view_x[VIEW_MAX];
extern s16b temp_n;
extern byte temp_y[TEMP_MAX];
extern byte temp_x[TEMP_MAX];*/
extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;
extern s16b quark__num;
extern cptr *quark__str;
extern u16b message__next;
extern u16b message__last;
extern u16b message__head;
extern u16b message__tail;
extern u16b *message__ptr;
extern char *message__buf;
/*extern u32b option_flag[8];
extern u32b option_mask[8];
extern u32b window_flag[8];
extern u32b window_mask[8];*/
/*extern term *ang_term[8];*/
extern s16b o_fast[MAX_O_IDX];
extern s16b m_fast[MAX_M_IDX];
extern cave_type ***cave;
extern wilderness_type *wild_info;
extern hturn *turn_cavegen;
extern object_type *o_list;
extern monster_type *m_list;
extern player_type **p_list;
extern quest q_list[MAX_Q_IDX];
extern store_type *store;
/*extern object_type *inventory;*/
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[1024];
extern char misc_to_char[1024];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern byte keymap_cmds[128];
extern byte keymap_dirs[128];
extern byte color_table[256][4];
extern char text_screen[MAX_TEXTFILES][TEXTFILE__WID * TEXTFILE__HGT];
/*extern player_type *p_ptr;*/
/*extern player_race *rp_ptr;
extern player_class *cp_ptr;
extern player_magic *mp_ptr;*/
/*extern u32b spell_learned1;
extern u32b spell_learned2;
extern u32b spell_worked1;
extern u32b spell_worked2;
extern u32b spell_forgotten1;
extern u32b spell_forgotten2;
extern byte spell_order[64];
extern s16b player_hp[PY_MAX_LEVEL];*/
extern vault_type *v_info;
extern char *v_name;
extern char *v_text;
extern feature_type *f_info;
extern char *f_name;
extern char *f_text;
extern char *f_char_s;
extern byte *f_attr_s;
extern object_kind *k_info;
extern char *k_name;
extern char *k_text;
extern char *k_char_s;
extern byte *k_attr_s;
extern artifact_type *a_info;
extern char *a_name;
extern char *a_text;
extern ego_item_type *e_info;
extern char *e_name;
extern char *e_text;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern char *r_char_s;
extern byte *r_attr_s;
extern player_race *p_info;
extern char *p_name;
extern char *p_text;
extern player_class *c_info;
extern char *c_name;
extern char *c_text;
extern cptr store_names[MAX_STORES];
extern owner_type *b_info;
extern char *b_name;
extern char *b_text;
extern byte *g_info;
extern char *g_name;
extern char *g_text;
extern hist_type *h_info;
extern char *h_text;
extern flavor_type *flavor_info;
extern char *flavor_name;
extern char *flavor_text;
extern char *flavor_char_s;
extern byte *flavor_attr_s;
extern cptr ANGBAND_SYS;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_PREF;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_XTRA;
extern cptr ANGBAND_DIR_GAME;
extern cptr ANGBAND_DIR_TEXT;
extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern bool (*get_mon_num_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);
extern bool (*master_move_hook)(player_type *p_ptr, char * parms);
extern char * cfg_bind_name;
extern const player_sex sex_info[MAX_SEXES];

/*
 * Automatically generated "function declarations"
 */

/* birth.c */
extern player_type* player_alloc(void);
extern void player_free(player_type *p_ptr);
extern void player_wipe(player_type *p_ptr);
extern void player_net_wipe(player_type *p_ptr, int reach);
extern bool player_birth(int ind, int race, int pclass, int sex, int stat_order[]);
extern void server_birth(void);
extern void player_setup(player_type *p_ptr);
extern void player_verify_visual(player_type *p_ptr);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int Depth, int y1, int x1, int y2, int x2);
extern bool player_can_see_bold(player_type *p_ptr, int y, int x);
extern bool no_lite(player_type *p_ptr);
extern void map_info(player_type *p_ptr, int y, int x, byte *ap, char *cp, byte *tap, char *tcp, bool server);
extern int color_char_to_attr(char c);
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void cave_set_feat(int Depth, int y, int x, int feat);
extern void spot_updates(int Depth, int y, int x, u32b updates);
extern void note_spot(player_type *p_ptr, int y, int x);
extern void note_spot_depth(int Depth, int y, int x);
extern void everyone_lite_spot(int Depth, int y, int x);
extern void everyone_forget_spot(int Depth, int y, int x);
extern void lite_spot(player_type *p_ptr, int y, int x);
extern void prt_map(player_type *p_ptr);
extern void display_map(player_type *p_ptr, bool quiet);
extern void do_cmd_view_map(player_type *p_ptr, char query);
extern void forget_lite(player_type *p_ptr);
extern void update_lite(player_type *p_ptr);
extern void forget_view(player_type *p_ptr);
extern void update_view(player_type *p_ptr);
extern void forget_flow(void);
extern void update_flow(void);
extern void wiz_lite(player_type *p_ptr);
extern void wiz_dark(player_type *p_ptr);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int Depth, int y1, int x1, int y2, int x2);
extern bool projectable_wall(int Depth, int y1, int x1, int y2, int x2);
extern void scatter(int Depth, int *yp, int *xp, int y, int x, int d, int m);
extern void health_track(player_type *p_ptr, int m_idx);
extern void update_health(int m_idx);
extern void monster_race_track(player_type *p_ptr, int r_idx);
extern void disturb(player_type *p_ptr, int stop_search, int flush_output);
extern bool is_quest(int level);
extern bool is_quest_level(player_type *p_ptr, int level);
extern void update_players(void);
extern void update_cursor(int);
extern void cursor_track(player_type *p_ptr, int m_idx);
extern int player_pict(player_type *p_ptr, player_type *who);
extern player_type* player_on_cave_p(cave_type *c_ptr);
extern player_type* player_on_cave(int Depth, int y, int x);


/* cmd1.c */
extern bool test_hit_fire(int chance, int ac, int vis);
extern bool test_hit_norm(int chance, int ac, int vis);
extern s16b critical_shot(player_type *p_ptr, int weight, int plus, int dam);
extern s16b critical_norm(player_type *p_ptr, int weight, int plus, int dam);
extern s16b tot_dam_aux(player_type *p_ptr, object_type *o_ptr, int tdam, monster_type *m_ptr, bool ml);
extern s16b tot_dam_aux_player(object_type *o_ptr, int tdam, player_type *p_ptr);
extern void search(player_type *p_ptr);
extern void carry(player_type *p_ptr, int pickup, int confirm);
extern void py_attack(player_type *p_ptr, int y, int x);
extern void move_player(player_type *p_ptr, int dir, int do_pickup);
extern void run_step(player_type *p_ptr, int dir);
extern int see_wall(player_type *p_ptr, int dir, int y, int x);

/* cmd2.c */
extern bool house_inside(player_type *p_ptr, int house);
extern bool house_owned(int house);
extern bool house_owned_by(player_type *p_ptr, int house);
extern bool set_house_owner(player_type *p_ptr, int house);
extern bool create_house(player_type *p_ptr);
extern int houses_owned(player_type *p_ptr);
extern void disown_house(int house);
extern void do_cmd_go_up(player_type *p_ptr);
extern void do_cmd_go_down(player_type *p_ptr);
extern void do_cmd_search(player_type *p_ptr);
extern void do_cmd_toggle_search(player_type *p_ptr);
extern void do_cmd_alter(player_type *p_ptr, int dir);
extern void do_cmd_open(player_type *p_ptr, int dir);
extern void do_cmd_close(player_type *p_ptr, int dir);
extern void do_cmd_tunnel(player_type *p_ptr, int dir);
extern void do_cmd_disarm(player_type *p_ptr, int dir);
extern void do_cmd_bash(player_type *p_ptr, int dir);
extern void do_cmd_spike(player_type *p_ptr, int dir);
extern void do_cmd_walk(player_type *p_ptr, int dir, int pickup);
extern void do_cmd_stay(player_type *p_ptr);
extern int do_cmd_run(player_type *p_ptr, int dir);
extern void do_cmd_hold(player_type *p_ptr);
extern void do_cmd_hold_or_stay(player_type *p_ptr, int pickup, int take_stairs);
extern void do_cmd_enterfeat(player_type *p_ptr);
extern void do_cmd_toggle_rest(player_type *p_ptr);
extern void do_cmd_pathfind(player_type *p_ptr, int y, int x);
/*extern void do_cmd_rest(void);*/
extern void do_cmd_fire(player_type *p_ptr, int item, int dir);
extern void do_cmd_throw(player_type *p_ptr, int item, int dir);
extern void do_cmd_purchase_house(player_type *p_ptr, int dir);
extern int pick_house(int Depth, int y, int x);
extern void do_cmd_mouseclick(player_type *p_ptr, int mod, int y, int x);

/* cmd3.c */
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_wield(player_type *p_ptr, int item);
extern void do_cmd_takeoff(player_type *p_ptr, int item);
extern void do_cmd_drop(player_type *p_ptr, int item, int quantity);
extern void do_cmd_drop_gold(player_type *p_ptr, s32b amt);
extern void do_cmd_destroy(player_type *p_ptr, int item, int quantity);
extern void do_cmd_observe(player_type *p_ptr, int item);
extern void do_cmd_uninscribe(player_type *p_ptr, int item);
extern void do_cmd_inscribe(player_type *p_ptr, int item, cptr inscription);
extern void do_cmd_steal(player_type *p_ptr, int dir);
extern void do_cmd_refill(player_type *p_ptr, int item);
extern void do_cmd_target(player_type *p_ptr, char dir);
extern void do_cmd_target_friendly(player_type *p_ptr, char dir);
extern void do_cmd_look(player_type *p_ptr, char dir);
extern void do_cmd_locate(player_type *p_ptr, int dir);
extern void do_cmd_query_symbol(player_type *p_ptr, char sym);
extern void do_cmd_monlist(player_type *p_ptr);
extern void do_cmd_itemlist(player_type *p_ptr);
extern void describe_floor_tile(cave_type *c_ptr, cptr out_val, player_type *p_ptr, bool active, byte cave_flag);
extern void do_cmd_monster_desc_aux(player_type *p_ptr, int r_idx, bool quiet);
extern void do_cmd_monster_desc_all(player_type *p_ptr, char sym);
extern bool ang_sort_comp_monsters(void *player_context, vptr u, vptr v, int a, int b);
extern void ang_sort_swap_u16b(void *player_context, vptr u, vptr v, int a, int b);

/* cmd4.c */
extern void do_cmd_redraw(void);
extern void do_cmd_change_name(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colors(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(player_type *p_ptr);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);
extern void do_cmd_check_artifacts(player_type *p_ptr, int line);
extern void do_cmd_check_uniques(player_type *p_ptr, int line);
extern void do_cmd_check_players(player_type *p_ptr, int line);
extern void do_cmd_check_other(player_type *p_ptr, int line);
extern void do_cmd_knowledge(player_type *p_ptr, char query);
extern void do_cmd_interactive(player_type *p_ptr, char query);
extern void do_cmd_interactive_input(player_type *p_ptr, char query);
extern void common_peruse(player_type *p_ptr, char query);

/* cmd5.c */
extern void do_cmd_browse(player_type *p_ptr, int book);
extern void do_cmd_study(player_type *p_ptr, int book, int spell);
extern void do_cmd_cast_pre(player_type *p_ptr, int book, int dir, int spell);
extern void do_cmd_cast(player_type *p_ptr, int book, int spell);
extern void do_cmd_cast_fin(player_type *p_ptr, bool tried);
extern void do_cmd_pray_pre(player_type *p_ptr, int book, int dir, int spell);
extern void do_cmd_pray(player_type *p_ptr, int book, int spell);
extern void show_ghost_spells(player_type *p_ptr);
extern void do_cmd_ghost_power_pre(player_type *p_ptr, int dir, int ability);
extern void do_cmd_ghost_power(player_type *p_ptr, int ability);
extern void do_cmd_ghost_power_fin(player_type *p_ptr);


/* cmd6.c */
extern void do_cmd_eat_food(player_type *p_ptr, int item);
extern void do_cmd_quaff_potion(player_type *p_ptr, int item);
extern void do_cmd_read_scroll(player_type *p_ptr, int item);
extern void do_cmd_read_scroll_end(player_type *p_ptr, int item, bool ident);
extern void do_cmd_read_scroll_on(player_type *p_ptr, int item, int item2);
extern void do_cmd_aim_wand(player_type *p_ptr, int item, int dir);
extern void do_cmd_use_staff_pre(player_type *p_ptr, int item, int item2);
extern void do_cmd_use_staff(player_type *p_ptr, int item);
extern void do_cmd_use_staff_discharge(player_type *p_ptr, int item, bool ident);
extern void do_cmd_zap_rod_pre(player_type *p_ptr, int item, int dir);
extern void do_cmd_zap_rod(player_type *p_ptr, int item);
extern void do_cmd_zap_rod_discharge(player_type *p_ptr, int dir, bool ident);
extern void do_cmd_activate(player_type *p_ptr, int item);
extern void do_cmd_activate_dir(player_type *p_ptr, int item, int dir);
extern void do_cmd_refill_potion(player_type *p_ptr, int item);

/* pathfind.c */
extern bool findpath(player_type *p_ptr, int y, int x);

/* control.c */
extern void console_print(char *msg, int chan);

/* dungeon.c */
extern bool is_boring(byte feat);
extern void play_game(bool new_game);
extern void shutdown_server(void);
extern void dungeon(void);
extern bool check_special_level(s16b special_depth);
extern int find_player_name(char *name);
extern int find_player(s32b id);
extern int count_players(int Depth);

/* files.c */
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern s16b tokenize(char *buf, s16b num, char **tokens);
extern void display_player(player_type *p_ptr);
extern errr file_character(cptr name, bool full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern void read_times(void);
extern void show_news(void);
extern errr show_file(player_type *p_ptr, cptr name, cptr what, int line, int color);
extern int file_peruse_next(player_type *p_ptr, char query, int next);
extern void common_file_peruse(player_type *p_ptr, char query);
extern void copy_file_info(player_type *p_ptr, cptr name, int line, int color);
extern void do_cmd_help(player_type *p_ptr, int line);
extern int rewrite_player_name(char *wptr, char *bptr, const char *nick_name);
extern bool process_player_name(player_type *p_ptr, bool sf);
extern int process_player_name_aux(cptr name, cptr basename, bool sf);
/*extern void get_name(player_type *p_ptr);*/
extern void do_cmd_suicide(player_type *p_ptr);
extern void do_cmd_save_game(player_type *p_ptr);
extern long total_points(player_type *p_ptr);
extern void display_scores(player_type *p_ptr, int to);
extern void add_high_score(player_type *p_ptr);
extern void close_game(void);
extern void exit_game_panic(void);
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);
extern void kingly(player_type *p_ptr);
extern void setup_exit_handler(void);
extern errr get_rnd_line(cptr file_name, int entry, char *output);
extern errr file_character_server(player_type *p_ptr, cptr name);

/* generate.c */
extern void alloc_dungeon_level(int Depth);
extern void dealloc_dungeon_level(int Depth);
extern void generate_cave(player_type *p_ptr, int Depth, int auto_scum);
extern void build_vault(int Depth, int yval, int xval, int ymax, int xmax, cptr data);
extern void place_closed_door(int Depth, int y, int x);

/* wilderness.c */
extern int world_index(int world_x, int world_y);
extern void wild_cat_depth(int Depth, char *buf); 
extern void init_wild_info(void);
extern void wild_apply_day(int Depth);
extern void wild_apply_night(int Depth);
extern int determine_wilderness_type(int Depth);
extern void wilderness_gen(int Depth);
extern void wild_add_monster(int Depth);
extern void wild_grow_crops(int Depth);
extern void do_cmd_plant_seed(player_type *p_ptr, int item);

/* init-txt.c */
extern errr init_v_info_txt(FILE *fp, char *buf);
extern errr init_f_info_txt(FILE *fp, char *buf);
extern errr init_k_info_txt(FILE *fp, char *buf);
extern errr init_a_info_txt(FILE *fp, char *buf);
extern errr init_e_info_txt(FILE *fp, char *buf);
extern errr init_r_info_txt(FILE *fp, char *buf);
extern errr parse_file(struct parser *p, const char *filename);

/* init.c */
extern void init_file_paths(char *path, char *path_wr);
extern void free_file_paths(void);
extern void init_some_arrays(void);
extern void load_server_cfg(void);
extern void unload_server_cfg(void);
extern void cleanup_angband(void);

/* load1.c */
/*extern errr rd_savefile_old(void);*/

/* load2.c */
extern errr rd_savefile_new(player_type *p_ptr);
extern errr rd_server_savefile(void);
extern errr rd_savefile_new_scoop_aux(char *sfile, char *pass_word);
extern bool rd_dungeon_special_ext(int Depth, cptr levelname);

/* melee1.c */
/* melee2.c */
extern bool make_attack_normal(player_type *p_ptr, int m_idx);
extern bool make_attack_spell(player_type *p_ptr, int m_idx);
extern void process_monsters(void);

/* mon-desc.c */
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);

/* monster.c */
extern void describe_monster(player_type *p_ptr, int m_ind, bool spoilers);
extern void delete_monster_idx(int i);
extern void delete_monster(int Depth, int y, int x);
extern void compact_monsters(int size);
extern void wipe_m_list(int Depth);
extern s16b m_pop(void);
extern errr get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void monster_desc(player_type *p_ptr, char *desc, int m_idx, int mode);
extern void lore_do_probe(player_type *p_ptr, int m_idx);
extern void lore_treasure(player_type *p_ptr, int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool dist);
extern void update_monsters(bool dist);
extern void update_player(player_type *p_ptr);
extern void update_players(void);
extern bool place_monster_aux(int Depth, int y, int x, int r_idx, bool slp, bool grp);
extern bool place_monster(int Depth, int y, int x, bool slp, bool grp);
extern bool alloc_monster(int Depth, int dis, int slp);
extern bool summon_specific(int Depth, int y1, int x1, int lev, int type);
extern bool multiply_monster(int m_idx);
extern void update_smart_learn(int m_idx, int what);
extern void setup_monsters(void);
extern int race_index(char * name);
extern bool summon_specific_race(int Depth, int y1, int x1, int r_idx, unsigned char num);
extern bool summon_specific_race_somewhere(int Depth, int r_idx, unsigned char num);
extern int race_index_fuzzy(char * name);
extern int monster_richness(int r_idx);
extern u16b rand_mimic_kind(int r_idx);

/* monster2.c */
extern bool is_detected(u32b flag, u32b esp);
extern void reveal_mimic(int m_idx);
extern void forget_monster(player_type *p_ptr, int m_idx, bool deleted);
extern s16b monster_carry(player_type *p_ptr, int m_idx, object_type *j_ptr);
extern bool monster_can_carry(int m_idx);
extern bool summon_specific_okay_aux(int r_idx, int summon_type);
extern void display_monlist(player_type *p_ptr);

// Transitional network hacks
#define Send_term_info(IND, FLAG, ARG) send_term_info(Players[Ind], FLAG, ARG)
#define Send_special_other(PLR, HEADER) send_term_header(PLR, NTERM_POP, HEADER)
#define Send_direction(IND) plog("Send_direction unimplemented\n")
#define Send_item_request(IND, tval_hook) plog("Send_item_request unimplemented\n")
#define Send_flush(IND) plog("Send_flush unimplemented\n")
#define Send_pause(PLR) send_term_info(PLR, NTERM_HOLD, NTERM_PAUSE)

/* net-server.c */
extern void setup_network_server();
extern void network_loop();
extern void close_network_server();
extern void report_to_meta_die(void);
extern int player_leave(int p_idx);
extern int player_disconnect(player_type *p_ptr, cptr reason);

/* net-game.c */
extern int process_player_commands(player_type *p_ptr);
extern int stream_char_raw(player_type *p_ptr, int st, int y, int x, byte a, char c, byte ta, char tc);
extern int stream_char(player_type *p_ptr, int st, int y, int x);
extern int stream_line_as(player_type *p_ptr, int st, int y, int x);
extern int send_term_info(player_type *p_ptr, byte flag, u16b line);
extern int send_term_header(player_type *p_ptr, byte hint, cptr header);
extern int send_term_writefile(connection_type *ct, byte fmode, cptr filename);
extern int send_term_write(player_type *p_ptr, byte fmode, cptr filename);
extern int send_cursor(player_type *p_ptr, byte vis, byte x, byte y);
extern int send_target_info(player_type *p_ptr, byte x, byte y, byte win, cptr str);
extern int send_character_info(player_type *p_ptr);
extern int send_slash_fx(player_type *p_ptr, byte y, byte x, byte dir, byte fx);
extern int send_air_char(player_type *p_ptr, byte y, byte x, char a, char c, u16b delay, u16b fade);
extern int send_floor(player_type *p_ptr, byte a, char c, byte attr, int amt, byte tval, byte flag, byte s_tester, cptr name, cptr name_one);
extern int send_inven(player_type *p_ptr, char pos, byte a, char c, byte attr, int wgt, int amt, byte tval, byte flag, byte s_tester, cptr name, cptr name_one);
extern int send_equip(player_type *p_ptr, char pos, byte attr, int wgt, byte tval, byte flag, cptr name);
extern int send_spell_info(player_type *p_ptr, u16b book, u16b id, byte flag, byte item_tester, cptr desc);
extern int send_ghost(player_type *p_ptr);
extern int send_inventory_info_DEPRECATED(connection_type *ct);
extern int send_inventory_info(connection_type *ct);
extern int send_objflags_info(connection_type *ct);
extern int send_floor_info(connection_type *ct);
extern int send_indication(player_type *p_ptr, int id, ...);
extern int send_objflags(player_type *p_ptr, int line);
extern int send_party_info(player_type *p_ptr);
extern int send_message(player_type *p_ptr, cptr msg, u16b typ);
extern int send_message_repeat(player_type *p_ptr, u16b typ);
extern int send_sound(player_type *p_ptr, u16b sound);
extern int send_channel(player_type *p_ptr, char mode, u16b id, cptr name);
extern int send_store(player_type *p_ptr, char pos, byte ga, char gc, byte attr, s16b wgt, s16b number, long price, cptr name);
extern int send_store_info(player_type *p_ptr, byte flag, cptr name, char *owner, int items, long purse);
extern int send_store_sell(player_type *p_ptr, u32b price);
extern int send_store_leave(player_type *p_ptr);
extern int send_confirm_request(player_type *p_ptr, byte type, cptr buf);
extern int send_pickup_check(player_type *p_ptr, cptr buf);



/* obj-info.c */
/* object1.c */
/* object2.c */
extern void object_info_screen(const object_type *o_ptr);
extern void flavor_init(void);
extern void reset_visuals(void);
extern void flavor_copy(char *buf, u16b flavor, object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_flags_known(const player_type *p_ptr, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_desc(const player_type *p_ptr, char *buf, size_t buflen, const object_type *o_ptr, int pref, int mode);
extern void object_desc_one(const player_type *p_ptr, char *buf, size_t bufsize, object_type *o_ptr, int force, int mode);
extern void object_desc_store(const player_type *p_ptr, char *buf, object_type *o_ptr, int pref, int mode);
extern bool identify_fully_aux(player_type *p_ptr, object_type *o_ptr);
extern char index_to_label(int i);
extern s16b label_to_inven(player_type *p_ptr, int c);
extern s16b label_to_equip(player_type *p_ptr, int c);
extern s16b wield_slot(player_type *p_ptr, object_type *o_ptr);
extern cptr mention_use(player_type *p_ptr, int i);
extern cptr describe_use(player_type *p_ptr, int i);
extern void inven_item_charges(player_type *p_ptr, int item);
extern void inven_item_describe(player_type *p_ptr, int item);
extern void inven_item_increase(player_type *p_ptr, int item, int num);
extern void inven_item_optimize(player_type *p_ptr, int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern void floor_item_notify(player_type *p_ptr, s16b o_idx, bool force);
extern bool inven_drop_okay(player_type *p_ptr, object_type *o_ptr);
extern bool inven_carry_okay(player_type *p_ptr, object_type *o_ptr);
extern s16b inven_carry(player_type *p_ptr, object_type *o_ptr);
extern bool item_tester_okay(object_type *o_ptr);
extern void display_inven(player_type *p_ptr);
extern void display_equip(player_type *p_ptr);
/*extern void show_inven(void);
extern void show_equip(void);
extern void toggle_inven_equip(void);
extern bool get_item(player_type *p_ptr, int *cp, cptr pmt, bool equip, bool inven, bool floor);*/
extern void delete_object_idx(int i);
extern void delete_object_ptr(object_type * o_ptr);
extern void delete_object(int Depth, int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(int Depth);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern bool object_is_fuel(player_type *p_ptr, object_type *o_ptr, bool *fits);
extern byte object_tester_flag(player_type *p_ptr, object_type *o_ptr, byte *secondary_tester);
extern void object_known(object_type *o_ptr);
extern void object_aware(player_type *p_ptr, object_type *o_ptr);
extern void object_tried(player_type *p_ptr, object_type *o_ptr);
extern s32b object_value(player_type *p_ptr, object_type *o_ptr);
extern bool object_similar_floor(object_type *o_ptr, object_type *j_ptr);
extern bool object_similar(const player_type *p_ptr, const object_type *o_ptr, const object_type *j_ptr);
extern void object_absorb(player_type *p_ptr, object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void invwipe(object_type *o_ptr);
extern void invcopy(object_type *o_ptr, int k_idx);
extern bool check_ego(object_type *o_ptr, int level, int power, int idx);
extern void apply_magic(int Depth, object_type *o_ptr, int lev, bool okay, bool good, bool great);
extern object_type* place_object(int Depth, int y, int x, bool good, bool great, byte origin);
extern void acquirement(int Depth, int y1, int x1, int num, bool great);
extern void place_trap(int Depth, int y, int x);
extern void place_gold(int Depth, int y, int x);
extern void process_objects(void);
extern void drop_near(object_type *o_ptr, int chance, int Depth, int y, int x);
extern void pick_trap(int Depth, int y, int x);
extern cptr item_activation(object_type *o_ptr);
extern void combine_pack(player_type *p_ptr);
extern void reorder_pack(player_type *p_ptr);
extern void setup_objects(void);
extern void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
extern void reduce_charges(object_type *o_ptr, int amt);
extern void object_audit(player_type *p_ptr, object_type *o_ptr, int amt);
extern void object_own(player_type *p_ptr, object_type *o_ptr);
extern void artifact_notify(player_type *p_ptr, object_type *o_ptr);
extern object_type* player_get_item(player_type *p_ptr, int item, int *idx);
extern object_type* player_get_floor_item(player_type *p_ptr, int *idx);
extern void player_redraw_item(player_type *p_ptr, int item);
extern void player_redraw_fuel_items(player_type *p_ptr);
extern u16b rand_tval_kind(int tval);
extern void display_itemlist(player_type *p_ptr);

/* randart.c */
extern artifact_type *randart_make(const object_type *o_ptr);
extern void randart_name(const object_type *o_ptr, char *buffer);

/* party.c */
extern int party_lookup(cptr name);
extern bool player_in_party(int party_id, player_type *p_ptr);
extern int party_create(player_type *p_ptr, cptr name);
extern int party_add(player_type *p_ptr, cptr name);
extern int party_remove(player_type *p_ptr, cptr name);
extern void party_leave(player_type *p_ptr);
extern void party_msg(int party_id, cptr msg);
extern void party_msg_format(int party_id, cptr fmt, ...);
extern void party_msg_format_near(player_type *p_ptr, u16b type, cptr fmt, ...);
extern void party_gain_exp(player_type *p_ptr, int party_id, s32b amount, int m_idx);
extern int party_mark_members(player_type *p_ptr, int m_idx);
extern void party_share_hurt(player_type *p_ptr, player_type *q_ptr);
extern bool add_hostility(player_type *p_ptr, cptr name);
extern bool remove_hostility(player_type *p_ptr, cptr name);
extern bool check_hostile(player_type *attacker, player_type *target);
extern cptr lookup_player_name(int id);
extern int lookup_player_id(cptr name);
extern void add_player_name(cptr name, int id);
extern void delete_player_id(int id);
extern void delete_player_name(cptr name);
extern void wipe_player_names();
extern int player_id_list(int **list);
extern bool pvp_okay(player_type *attacker, player_type *target, int mode);
extern int ego_kind_index_fuzzy(char * name);
extern int item_kind_index_fuzzy(char * name);
extern bool place_specific_object(int Depth, int y1, int x1, object_type *forge, int lev, int num);

/* save.c */
extern bool save_player(player_type *p_ptr);
extern int scoop_player(char *nick, char *pass);
extern bool load_player(player_type *p_ptr);
extern bool load_server_info(void);
extern bool save_server_info(void);
extern bool wr_dungeon_special_ext(int Depth, cptr levelname);


/* spells1.c */
extern u16b default_bolt_pict(int typ, int dir, int *index);
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(player_type *p_ptr, int dis);
extern void teleport_player_to(player_type *p_ptr, int ny, int nx);
extern void teleport_player_level(player_type *p_ptr);
extern void take_hit(player_type *p_ptr, int damage, cptr kb_str);
extern void acid_dam(player_type *p_ptr, int dam, cptr kb_str);
extern void elec_dam(player_type *p_ptr, int dam, cptr kb_str);
extern void fire_dam(player_type *p_ptr, int dam, cptr kb_str);
extern void cold_dam(player_type *p_ptr, int dam, cptr kb_str);
extern bool inc_stat(player_type *p_ptr, int stat);
extern bool dec_stat(player_type *p_ptr, int stat, int amount, int permanent);
extern bool res_stat(player_type *p_ptr, int stat);
extern bool apply_disenchant(player_type *p_ptr, int mode);
extern bool project(int who, int rad, int Depth, int y, int x, int dam, int typ, int flg);
extern bool project_hook(player_type *p_ptr, int typ, int dir, int dam, int flg);

/* spells2.c */
extern void wipe_spell(int Depth, int cy, int cx, int r);
extern bool hp_player(player_type *p_ptr, int num);
extern bool hp_player_quiet(player_type *p_ptr, int num);
extern bool warding_glyph(player_type *p_ptr);
extern bool do_dec_stat(player_type *p_ptr, int stat);
extern bool do_res_stat(player_type *p_ptr, int stat);
extern bool do_inc_stat(player_type *p_ptr, int stat);
extern void identify_pack(player_type *p_ptr);
extern void message_pain(player_type *p_ptr, int m_idx, int dam);
extern bool remove_curse(player_type *p_ptr);
extern bool remove_all_curse(player_type *p_ptr);
extern bool restore_level(player_type *p_ptr);
extern void self_knowledge(player_type *p_ptr, bool spoil);
extern bool lose_all_info(player_type *p_ptr);
extern void map_area(player_type *p_ptr);
extern bool detect_treasure(player_type *p_ptr);
extern bool detect_objects_magic(player_type *p_ptr);
extern bool detect_invisible(player_type *p_ptr, bool pause);
extern bool detect_evil(player_type *p_ptr);
extern bool detect_creatures(player_type *p_ptr, bool pause);
extern bool detection(player_type *p_ptr);
extern bool detect_objects_normal(player_type *p_ptr);
extern bool detect_trap(player_type *p_ptr);
extern bool detect_sdoor(player_type *p_ptr);
extern void stair_creation(player_type *p_ptr);
extern bool enchant(player_type *p_ptr, object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(player_type *p_ptr, int num_hit, int num_dam, int num_ac, bool discount);
extern bool enchant_spell_aux(player_type *p_ptr, int item, int num_hit, int num_dam, int num_ac, bool discount);
extern bool ident_spell(player_type *p_ptr);
extern bool ident_spell_aux(player_type *p_ptr, int item);
extern bool identify_fully(player_type *p_ptr);
extern bool identify_fully_item(player_type *p_ptr, int item);
extern bool recharge(player_type *p_ptr, int num);
extern bool recharge_aux(player_type *p_ptr, int item, int num);
extern bool speed_monsters(player_type *p_ptr);
extern bool slow_monsters(player_type *p_ptr);
extern bool sleep_monsters(player_type *p_ptr);
extern void aggravate_monsters(player_type *p_ptr, int who);
extern bool banishment(player_type *p_ptr);
extern bool mass_banishment(player_type *p_ptr);
extern bool probing(player_type *p_ptr);
extern bool banish_evil(player_type *p_ptr, int dist);
extern bool dispel_evil(player_type *p_ptr, int dam);
extern bool dispel_undead(player_type *p_ptr, int dam);
extern bool dispel_monsters(player_type *p_ptr, int dam);
extern bool turn_undead(player_type *p_ptr);
extern void destroy_area(int Depth, int y1, int x1, int r, bool full);
extern void earthquake(int Depth, int cy, int cx, int r);
extern void lite_room(player_type *p_ptr, int Depth, int y1, int x1);
extern void unlite_room(player_type *p_ptr, int Depth, int y1, int x1);
extern bool lite_area(player_type *p_ptr, int dam, int rad);
extern bool unlite_area(player_type *p_ptr, int dam, int rad);
extern bool fire_swarm(player_type *p_ptr, int num, int typ, int dir, int dam, int rad);
extern bool fire_ball(player_type *p_ptr, int typ, int dir, int dam, int rad);
extern bool fire_bolt(player_type *p_ptr, int typ, int dir, int dam);
extern bool fire_beam(player_type *p_ptr, int typ, int dir, int dam);
extern bool fire_bolt_or_beam(player_type *p_ptr, int prob, int typ, int dir, int dam);
extern bool lite_line(player_type *p_ptr, int dir);
extern bool strong_lite_line(player_type *p_ptr, int dir);
extern bool drain_life(player_type *p_ptr, int dir, int dam);
extern bool wall_to_mud(player_type *p_ptr, int dir);
extern bool destroy_door(player_type *p_ptr, int dir);
extern bool disarm_trap(player_type *p_ptr, int dir);
extern bool heal_monster(player_type *p_ptr, int dir);
extern bool speed_monster(player_type *p_ptr, int dir);
extern bool slow_monster(player_type *p_ptr, int dir);
extern bool sleep_monster(player_type *p_ptr, int dir);
extern bool confuse_monster(player_type *p_ptr, int dir, int plev);
extern bool fear_monster(player_type *p_ptr, int dir, int plev);
extern bool poly_monster(player_type *p_ptr, int dir);
extern bool clone_monster(player_type *p_ptr, int dir);
extern bool teleport_monster(player_type *p_ptr, int dir);
extern bool alter_reality(player_type *p_ptr, bool power);
extern bool heal_player_ball(player_type *p_ptr, int dir, int dam);
extern bool project_spell_ball(player_type *p_ptr, int dir, int spell);
extern bool project_prayer_ball(player_type *p_ptr, int dir, int spell);
extern bool door_creation(player_type *p_ptr);
extern bool trap_creation(player_type *p_ptr);
extern bool destroy_doors_touch(player_type *p_ptr);
extern bool sleep_monsters_touch(player_type *p_ptr);
extern bool create_artifact(player_type *p_ptr);
extern bool create_artifact_aux(player_type *p_ptr, int item);
extern void brand_ammo(player_type *p_ptr, int item, bool discount);
extern void brand_weapon(player_type *p_ptr, bool discount);
extern bool brand_bolts(player_type *p_ptr, bool discount);
extern void ring_of_power(player_type *p_ptr, int dir);
extern bool curse_armor(player_type *p_ptr);
extern bool curse_weapon(player_type *p_ptr);


/* store.c */
extern bool get_store_item(player_type *p_ptr, int item, object_type *i_ptr);
extern int get_player_store_name(int num, char *name);
extern void store_purchase(player_type *p_ptr, int item, int amt, cptr checksum);
extern void store_sell(player_type *p_ptr, int item, int amt);
extern void store_confirm(player_type *p_ptr);
extern void do_cmd_store(player_type *p_ptr, int pstore);
extern void store_shuffle(int which);
extern void store_maint(int which);
extern void store_init(int which);
extern s32b player_price_item(player_type *p_ptr, object_type *o_ptr);

/* util.c */
extern errr path_temp(char *buf, int max);
extern void flush(void);
extern void bell(void);
extern void sound(player_type *p_ptr, int num);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern void keymap_init(void);
extern void macro_add(cptr pat, cptr act, bool cmd_flag);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern s16b quark_add(cptr str);
extern void fill_prevent_inscription(bool *arr, s16b quark);
extern void update_prevent_inscriptions(player_type *p_ptr);
extern bool check_guard_inscription( s16b quark, char what);
extern s16b message_num(void);
extern cptr message_str(s16b age);
extern void message_add(cptr msg);
extern void msg_print(player_type *p_ptr, cptr msg);
extern void msg_print_aux(player_type *p_ptr, cptr msg, u16b type);
extern void msg_broadcast(player_type *p_ptr, cptr msg);
extern void msg_channel(int chan, cptr msg);
extern void msg_format_p(player_type *p_ptr, cptr fmt, ...);
extern void msg_format(player_type *p_ptr, cptr fmt, ...);
extern void msg_format_type(player_type *p_ptr, u16b type, cptr fmt, ...);
extern void msg_print_near(player_type *p_ptr, cptr msg);
extern void msg_format_near(player_type *p_ptr, cptr fmt, ...);
extern void msg_print_complex_near(player_type *p_ptr, player_type *q_ptr, u16b type, cptr msg);
extern void msg_format_complex_near(player_type *p_ptr, player_type *q_ptr, u16b type, cptr fmt, ...);
extern void msg_format_complex_far(player_type *p_ptr, player_type *q_ptr, u16b type, cptr fmt, cptr sender, ...);
extern void msg_format_monster(int m_idx, player_type *q_ptr, u16b type, cptr fmt_vis, cptr fmt_inv, ...);
extern void player_talk(player_type *p_ptr, char *msg);
extern void channel_join(player_type *p_ptr, cptr channel, bool quiet);
extern void channel_leave(player_type *p_ptr, cptr channel);
extern void channel_leave_id(player_type *p_ptr, int i, bool quiet);
extern void channels_leave(player_type *p_ptr);
extern bool is_a_vowel(int ch);
extern void log_history_event(player_type *p_ptr, char *msg, bool unique);
extern void history_wipe(history_event *evt);
extern cptr format_history_event(history_event *evt);
extern int color_text_to_attr(cptr name);
extern int color_opposite(int color);
extern cptr attr_to_text(byte a);
extern void send_prepared_info(player_type *p_ptr, byte win, byte stream, byte extra_params);
extern void send_prepared_popup(player_type *p_ptr, cptr header);
extern void monster_race_track_hack(player_type *p_ptr);
extern void text_out(cptr buf);
extern void text_out_c(byte a, cptr buf);
extern void text_out_init(player_type *p_ptr);
extern void text_out_done();
extern void text_out_save();
extern void text_out_load();
extern void c_prt(player_type *p_ptr, byte attr, cptr str, int row, int col);
extern void prt(player_type *p_ptr, cptr str, int row, int col);
extern void clear_line(player_type *p_ptr, int row);
extern void clear_from(player_type *p_ptr, int row);
extern bool ask_for(player_type *p_ptr, char query, char *buf); 
extern bool askfor_aux(player_type *p_ptr, char query, char *buf, int row, int col, cptr prompt, cptr default_value, byte prompt_attr, byte input_attr);

/* xtra1.c */
extern void cnv_stat(int val, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern void notice_stuff(player_type *p_ptr);
extern void update_stuff(player_type *p_ptr);
extern void redraw_stuff(player_type *p_ptr);
extern void window_stuff(player_type *p_ptr);
extern void handle_stuff(player_type *p_ptr);
extern void prt_history(player_type *p_ptr);
extern void c_prt_status_line(player_type *p_ptr, cave_view_type *dest, int len);
extern void player_flags(player_type *p_ptr, u32b *f1, u32b * f2, u32b *f3);

/* xtra2.c */
extern int get_player(player_type *p_ptr, object_type *o_ptr);
extern void describe_player(player_type *p_ptr, player_type *who);
extern bool set_blind(player_type *p_ptr, int v);
extern bool set_confused(player_type *p_ptr, int v);
extern bool set_poisoned(player_type *p_ptr, int v);
extern bool set_afraid(player_type *p_ptr, int v);
extern bool set_paralyzed(player_type *p_ptr, int v);
extern bool set_image(player_type *p_ptr, int v);
extern bool set_fast(player_type *p_ptr, int v);
extern bool set_slow(player_type *p_ptr, int v);
extern bool set_shield(player_type *p_ptr, int v);
extern bool set_blessed(player_type *p_ptr, int v);
extern bool set_hero(player_type *p_ptr, int v);
extern bool set_shero(player_type *p_ptr, int v);
extern bool set_protevil(player_type *p_ptr, int v);
extern bool set_invuln(player_type *p_ptr, int v);
extern bool set_tim_invis(player_type *p_ptr, int v);
extern bool set_tim_infra(player_type *p_ptr, int v);
extern bool set_oppose_acid(player_type *p_ptr, int v);
extern bool set_oppose_elec(player_type *p_ptr, int v);
extern bool set_oppose_fire(player_type *p_ptr, int v);
extern bool set_oppose_cold(player_type *p_ptr, int v);
extern bool set_oppose_pois(player_type *p_ptr, int v);
extern bool set_stun(player_type *p_ptr, int v);
extern bool set_cut(player_type *p_ptr, int v);
extern bool set_food(player_type *p_ptr, int v);
extern void set_recall(player_type *p_ptr, object_type * o_ptr);
extern bool set_noise(player_type *p_ptr, int v);
extern void check_experience(player_type *p_ptr);
extern void gain_exp(player_type *p_ptr, s32b amount);
extern void lose_exp(player_type *p_ptr, s32b amount);
extern void monster_death(player_type *p_ptr, int m_idx);
extern void player_death(player_type *p_ptr);
extern void resurrect_player(player_type *p_ptr);
extern bool mon_take_hit(player_type *p_ptr, int m_idx, int dam, bool *fear, cptr note);
extern bool adjust_panel(player_type *p_ptr, int y, int x);
extern void panel_bounds(player_type *p_ptr);
extern void verify_panel(player_type *p_ptr);
extern void setup_panel(player_type *p_ptr, bool adjust);
extern cptr look_mon_desc(int m_idx);
extern int pick_arena(int Depth, int y, int x);
extern int pick_arena_opponent(player_type *p_ptr, int a);
extern void access_arena(player_type *p_ptr, int y, int x);
extern void ang_sort_swap_distance(void *player_context, vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_distance(void *player_context, vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_value(void *player_context, vptr u, vptr v, int a, int b);
extern void ang_sort_swap_value(void *player_context, vptr u, vptr v, int a, int b);
extern bool target_able(player_type *p_ptr, int m_idx);
extern bool target_okay(player_type *p_ptr);
extern s16b target_pick(player_type *p_ptr, int y1, int x1, int dy, int dx);
extern bool target_set_interactive(player_type *p_ptr, int mode, char query);
extern bool target_set_interactive_mouse(player_type *p_ptr, int mod, int y, int x);
extern bool get_aim_dir(player_type *p_ptr, int *dp);
extern bool get_item(player_type *p_ptr, int *cp, byte tval_hook);
extern bool confuse_dir(bool confused, int *dp);
extern int motion_dir(int y1, int x1, int y2, int x2);
extern bool do_scroll_life(player_type *p_ptr);
extern bool do_restoreXP_other(player_type *p_ptr);
extern u32b level_speed(int Depth);
extern int time_factor(player_type *p_ptr);
extern int base_time_factor(player_type *p_ptr, int slowest);
extern void show_motd(player_type *p_ptr);
extern void show_tombstone(player_type *p_ptr);
extern void wipe_socials();
extern void boot_socials();
extern void show_socials(player_type *p_ptr);
extern void do_cmd_social(player_type *p_ptr, int dir, int i);
extern void do_cmd_dungeon_master(player_type *p_ptr, char query);

extern void master_new_hook(player_type *p_ptr, char hook_q, s16b oy, s16b ox);

extern bool master_level(player_type *p_ptr, char * parms);
extern bool master_build(player_type *p_ptr, char * parms);
extern bool master_summon(player_type *p_ptr, char * parms);
extern bool master_generate(player_type *p_ptr, char * parms);
extern bool master_acquire(player_type *p_ptr, char * parms);
extern bool master_player(player_type *p_ptr, char * parms);

/*extern bool get_rep_dir(int *dp);*/

extern bool c_get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor);

/* x-spell.c */
extern cptr spell_names[MAX_SPELL_REALMS][PY_MAX_SPELLS];
extern magic_type ghost_spells[8];
extern void spells_init();
extern int get_spell_index(const player_type *p_ptr, const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern cptr get_spell_info(player_type *p_ptr, int index);
extern byte get_spell_flag(int tval, int spell, byte player_flag, byte *item_tester);
extern bool cast_spell(player_type *p_ptr, int tval, int index);
extern bool cast_spell_hack(player_type *p_ptr, int tval, int index);

/* use-obj.c */
extern bool use_object(player_type *p_ptr, object_type *o_ptr, int item, bool *ident);
extern bool use_object_current(player_type *p_ptr);

/*
 * Hack -- conditional (or "bizarre") externs
 */


#ifndef HAVE_MEMSET
/* util.c */
extern void *memset(void*, int, size_t);
#endif

#ifndef HAVE_STRICMP
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
