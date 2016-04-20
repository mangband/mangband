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
extern bool cfg_mage_hp_bonus;
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
extern int player_textout;
extern party_type parties[MAX_PARTIES];
extern channel_type channels[MAX_CHANNELS];
extern house_type houses[MAX_HOUSES];
extern byte spell_flags[MAX_SPELL_REALMS][PY_MAX_SPELLS];
extern arena_type arenas[MAX_ARENAS];
extern int num_houses;
extern int num_arenas;
extern int chan_audit;
extern int chan_debug;
extern int chan_cheat;
extern long GetInd[];
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
extern u32b option_flag[8];
extern u32b option_mask[8];
extern u32b window_flag[8];
extern u32b window_mask[8];
/*extern term *ang_term[8];*/
extern s16b o_fast[MAX_O_IDX];
extern s16b m_fast[MAX_M_IDX];
extern cave_type ***cave;
extern wilderness_type *wild_info;
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
extern byte misc_to_attr[256];
extern char misc_to_char[256];
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
extern bool (*ang_sort_comp)(int Ind, vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(int Ind, vptr u, vptr v, int a, int b);
extern bool (*get_mon_num_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);
extern bool (*master_move_hook)(int Ind, char * parms);
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
extern void player_setup(int Ind);
extern void player_verify_visual(player_type *p_ptr);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int Depth, int y1, int x1, int y2, int x2);
extern bool player_can_see_bold(int Ind, int y, int x);
extern bool no_lite(int Ind);
extern void map_info(int Ind, int y, int x, byte *ap, char *cp, byte *tap, char *tcp, bool server);
extern int color_char_to_attr(char c);
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int Ind, int y, int x);
extern void note_spot_depth(int Depth, int y, int x);
extern void everyone_lite_spot(int Depth, int y, int x);
extern void everyone_forget_spot(int Depth, int y, int x);
extern void lite_spot(int Ind, int y, int x);
extern void prt_map(int Ind);
extern void display_map(int Ind, bool quiet);
extern void do_cmd_view_map(player_type *p_ptr, char query);
extern void forget_lite(int Ind);
extern void update_lite(int Ind);
extern void forget_view(int Ind);
extern void update_view(int Ind);
extern void forget_flow(void);
extern void update_flow(void);
extern void wiz_lite(int Ind);
extern void wiz_dark(int Ind);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int Depth, int y1, int x1, int y2, int x2);
extern bool projectable_wall(int Depth, int y1, int x1, int y2, int x2);
extern void scatter(int Depth, int *yp, int *xp, int y, int x, int d, int m);
extern void health_track(int Ind, int m_idx);
extern void update_health(int m_idx);
extern void monster_race_track(int Ind, int r_idx);
extern void disturb(int Ind, int stop_search, int flush_output);
extern bool is_quest(int level);
extern bool is_quest_level(int Ind, int level);
extern void update_players(void);
extern void update_cursor(int);
extern void cursor_track(int Ind, int m_idx);
extern int player_pict(int Ind, int who);

/* cmd1.c */
extern bool test_hit_fire(int chance, int ac, int vis);
extern bool test_hit_norm(int chance, int ac, int vis);
extern s16b critical_shot(int Ind, int weight, int plus, int dam);
extern s16b critical_norm(int Ind, int weight, int plus, int dam);
extern s16b tot_dam_aux(int Ind, object_type *o_ptr, int tdam, monster_type *m_ptr, bool ml);
extern s16b tot_dam_aux_player(object_type *o_ptr, int tdam, player_type *p_ptr);
extern void search(int Ind);
extern void carry(int Ind, int pickup, int confirm);
extern void py_attack(int Ind, int y, int x);
extern void move_player(int Ind, int dir, int do_pickup);
extern void run_step(int Ind, int dir);
extern int see_wall(int Ind, int dir, int y, int x);

/* cmd2.c */
extern bool house_inside(int Ind, int house);
extern bool house_owned(int house);
extern bool house_owned_by(int Ind, int house);
extern bool set_house_owner(int Ind, int house);
extern bool create_house(int Ind);
extern int houses_owned(int Ind);
extern void disown_house(int house);
extern void do_cmd_go_up(int Ind);
extern void do_cmd_go_down(int Ind);
extern void do_cmd_search(int Ind);
extern void do_cmd_toggle_search(int Ind);
extern void do_cmd_alter(int Ind, int dir);
extern void do_cmd_open(int Ind, int dir);
extern void do_cmd_close(int Ind, int dir);
extern void do_cmd_tunnel(int Ind, int dir);
extern void do_cmd_disarm(int Ind, int dir);
extern void do_cmd_bash(int Ind, int dir);
extern void do_cmd_spike(int Ind, int dir);
extern void do_cmd_walk(int Ind, int dir, int pickup);
extern void do_cmd_stay(int Ind);
extern int do_cmd_run(int Ind, int dir);
extern void do_cmd_hold(int Ind);
extern void do_cmd_hold_or_stay(int Ind, int pickup);
extern void do_cmd_toggle_rest(int Ind);
/*extern void do_cmd_rest(void);*/
extern void do_cmd_fire(int Ind, int item, int dir);
extern void do_cmd_throw(int Ind, int item, int dir);
extern void do_cmd_purchase_house(int Ind, int dir);
extern int pick_house(int Depth, int y, int x);

/* cmd3.c */
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_wield(int Ind, int item);
extern void do_cmd_takeoff(int Ind, int item);
extern void do_cmd_drop(int Ind, int item, int quantity);
extern void do_cmd_drop_gold(int Ind, s32b amt);
extern void do_cmd_destroy(int Ind, int item, int quantity);
extern void do_cmd_observe(int Ind, int item);
extern void do_cmd_uninscribe(int Ind, int item);
extern void do_cmd_inscribe(int Ind, int item, cptr inscription);
extern void do_cmd_steal(int Ind, int dir);
extern void do_cmd_refill(int Ind, int item);
extern void do_cmd_target(int Ind, char dir);
extern void do_cmd_target_friendly(int Ind, char dir);
extern void do_cmd_look(int Ind, char dir);
extern void do_cmd_locate(int Ind, int dir);
extern void do_cmd_query_symbol(int Ind, char sym);
extern void describe_floor_tile(cave_type *c_ptr, cptr out_val, int Ind, bool active, byte cave_flag);
extern void do_cmd_monster_desc_aux(int Ind, int r_idx, bool quiet);
extern void do_cmd_monster_desc_all(int Ind, char sym);
extern bool ang_sort_comp_monsters(int Ind, vptr u, vptr v, int a, int b);
extern void ang_sort_swap_u16b(int Ind, vptr u, vptr v, int a, int b);

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
extern void do_cmd_feeling(int Ind);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);
extern void do_cmd_check_artifacts(int Ind, int line);
extern void do_cmd_check_uniques(int Ind, int line);
extern void do_cmd_check_players(int Ind, int line);
extern void do_cmd_check_other(player_type *p_ptr, int line);
extern void do_cmd_knowledge(player_type *p_ptr, char query);
extern void do_cmd_interactive(player_type *p_ptr, char query);
extern void do_cmd_interactive_input(player_type *p_ptr, char query);
extern void common_peruse(player_type *p_ptr, char query);

/* cmd5.c */
extern void do_cmd_browse(int Ind, int book);
extern void do_cmd_study(int Ind, int book, int spell);
extern void do_cmd_cast_pre(int Ind, int book, int dir, int spell);
extern void do_cmd_cast(int Ind, int book, int spell);
extern void do_cmd_cast_fin(int Ind, bool tried);
extern void do_cmd_pray_pre(int Ind, int book, int dir, int spell);
extern void do_cmd_pray(int Ind, int book, int spell);
extern void show_ghost_spells(int Ind);
extern void do_cmd_ghost_power_pre(int Ind, int dir, int ability);
extern void do_cmd_ghost_power(int Ind, int ability);
extern void do_cmd_ghost_power_fin(int Ind);


/* cmd6.c */
extern void do_cmd_eat_food(int Ind, int item);
extern void do_cmd_quaff_potion(int Ind, int item);
extern void do_cmd_read_scroll(int Ind, int item);
extern void do_cmd_read_scroll_end(int Ind, int item, bool ident);
extern void do_cmd_read_scroll_on(int Ind, int item, int item2);
extern void do_cmd_aim_wand(int Ind, int item, int dir);
extern void do_cmd_use_staff(int Ind, int item);
extern void do_cmd_use_staff_discharge(int Ind, int item, bool ident);
extern void do_cmd_zap_rod_pre(int Ind, int item, int dir);
extern void do_cmd_zap_rod(int Ind, int item);
extern void do_cmd_zap_rod_discharge(int Ind, int dir, bool ident);
extern void do_cmd_activate(int Ind, int item);
extern void do_cmd_activate_dir(int Ind, int item, int dir);
extern void do_cmd_refill_potion(int Ind, int item);

/* control.c */
extern void NewConsole(int fd, int arg);
extern bool InitNewConsole(int write_fd);
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
extern void display_player(int Ind);
extern errr file_character(cptr name, bool full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern void read_times(void);
extern void show_news(void);
extern errr show_file(int Ind, cptr name, cptr what, int line, int color);
extern int file_peruse_next(player_type *p_ptr, char query, int next);
extern void common_file_peruse(player_type *p_ptr, char query);
extern void copy_file_info(player_type *p_ptr, cptr name, int line, int color);
extern void do_cmd_help(int Ind, int line);
extern bool process_player_name(player_type *p_ptr, bool sf);
extern int process_player_name_aux(cptr name, cptr basename, bool sf);
extern void get_name(int Ind);
extern void do_cmd_suicide(int Ind);
extern void do_cmd_save_game(int Ind);
extern long total_points(int Ind);
extern void display_scores(int from, int to);
extern void add_high_score(int Ind);
extern void close_game(void);
extern void exit_game_panic(void);
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);
extern void kingly(int Ind);
extern void setup_exit_handler(void);
extern errr get_rnd_line(cptr file_name, int entry, char *output);
extern errr file_character_server(int Ind, cptr name);

/* generate.c */
extern void alloc_dungeon_level(int Depth);
extern void dealloc_dungeon_level(int Depth);
extern void generate_cave(int Ind, int Depth,int auto_scum);
extern void build_vault(int Depth, int yval, int xval, int ymax, int xmax, cptr data);

/* wilderness.c */
extern int world_index(int world_x, int world_y);
extern void wild_cat_depth(int Depth, char *buf); 
extern void init_wild_info(void);
extern void wild_apply_day(int Depth);
extern void wild_apply_night(int Depth);
extern int determine_wilderness_type(int Depth);
extern void wilderness_gen(int Depth);
extern void wild_add_monster(int Depth);

/* init-txt.c */
extern errr init_v_info_txt(FILE *fp, char *buf);
extern errr init_f_info_txt(FILE *fp, char *buf);
extern errr init_k_info_txt(FILE *fp, char *buf);
extern errr init_a_info_txt(FILE *fp, char *buf);
extern errr init_e_info_txt(FILE *fp, char *buf);
extern errr init_r_info_txt(FILE *fp, char *buf);

/* init.c */
extern void init_file_paths(char *path);
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
extern bool make_attack_normal(int Ind, int m_idx);
extern bool make_attack_spell(int Ind, int m_idx);
extern void process_monsters(void);

/* mon-desc.c */
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);

/* monster.c */
extern void describe_monster(int Ind, int m_ind, bool spoilers);
extern void delete_monster_idx(int i);
extern void delete_monster(int Depth, int y, int x);
extern void compact_monsters(int size);
extern void wipe_m_list(int Depth);
extern s16b m_pop(void);
extern errr get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void monster_desc(int Ind, char *desc, int m_idx, int mode);
extern void lore_do_probe(int Ind, int m_idx);
extern void lore_treasure(int Ind, int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool dist);
extern void update_monsters(bool dist);
extern void update_player(int Ind);
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

/* monster2.c */
extern bool is_detected(u32b flag, u32b esp);
extern s16b monster_carry(int Ind, int m_idx, object_type *j_ptr);
extern bool monster_can_carry(int m_idx);
extern bool summon_specific_okay_aux(int r_idx, int summon_type);
extern void display_monlist(int Ind);

// Transitional network hacks
#define msg_format_p(P, M, ...) plog("msg_format_p unimplemented\n")
#define msg_print_p(P, M) plog("msg_print_p unimplemented\n")
#define Send_term_info(IND, FLAG, ARG) send_term_info(Players[Ind], FLAG, ARG)
#define Send_special_other(IND, HEADER) send_term_header(Players[Ind], HEADER)
#define Destroy_connection(IND, A) plog("Destroy_connection unimplemented\n")
#define Send_target_info(IND, X, Y, W, STR) send_target_info(Players[Ind], X, Y, W, STR)
#define Send_direction(IND) plog("Send_direction unimplemented\n")
#define Send_item_request(IND, tval_hook) plog("Send_item_request unimplemented\n")
#define Send_cursor(IND, vis, x, y) send_cursor(Players[Ind], vis, x, y)
#define Send_store(IND, pos, attr, wgt, number, price, name) plog("Send_store unimplemented\n")
#define Send_store_info(IND, flag, name, owner, items, purse) plog("Send_store_info unimplemented\n")
#define Send_flush(IND) plog("Send_flush unimplemented\n")
#define Send_pause(PLR) send_term_info(PLR, NTERM_HOLD, NTERM_PAUSE)
#define Send_party(IND) plog("Send_party unimplemented\n")
#define Send_store_leave(IND) plog("Send_store_leave unimplemented\n")
#define Send_store_sell(IND, price) send_store_sell(Ind, price)
#define Send_pickup_check(IND, buf) send_confirm_request(Ind, 0x03, buf)

/* net-server.c */
extern int *Get_Ind;
extern void setup_network_server();
extern void network_loop();
extern void close_network_server();
extern int player_leave(int p_idx);
extern int player_kill(int p_idx, cptr reason);

/* net-game.c */
extern int process_player_commands(int ind);
extern int stream_char_raw(player_type *p_ptr, int st, int y, int x, byte a, char c, byte ta, char tc);
extern int stream_char(player_type *p_ptr, int st, int y, int x);
extern int stream_line_as(player_type *p_ptr, int st, int y, int x);
extern int send_term_info(player_type *p_ptr, byte flag, u16b line);
extern int send_term_header(player_type *p_ptr, cptr header);
extern int send_cursor(player_type *p_ptr, char vis, char x, char y);
extern int send_target_info(player_type *p_ptr, char x, char y, byte win, cptr str);
extern int send_character_info(player_type *p_ptr);
extern int send_floor(int Ind, byte attr, int amt, byte tval, byte flag, cptr name);
extern int send_inven(int Ind, char pos, byte attr, int wgt, int amt, byte tval, byte flag, cptr name);
extern int send_equip(int Ind, char pos, byte attr, int wgt, byte tval, byte flag, cptr name);
extern int send_spell_info(int Ind, u16b book, u16b id, byte flag, cptr desc);
extern int send_ghost(player_type *p_ptr);
extern int send_inventory_info(connection_type *ct);
extern int send_floor_info(connection_type *ct);
extern int send_indication(int Ind, byte id, ...);
extern int send_objflags(int Ind, int line);
extern int send_message(int Ind, cptr msg, u16b typ);
extern int send_message_repeat(int Ind, u16b typ);
extern int send_sound(int Ind, int sound);
extern int send_channel(int Ind, char mode, u16b id, cptr name);
extern int send_store(int Ind, char pos, byte attr, s16b wgt, s16b number, long price, cptr name);
extern int send_store_info(int Ind, byte flag, cptr name, char *owner, int items, long purse);
extern int send_store_sell(int Ind, u32b price);
extern int send_confirm_request(int Ind, byte type, cptr buf);



/* obj-info.c */
/* object1.c */
/* object2.c */
extern void object_info_screen(const object_type *o_ptr);
extern void flavor_init(void);
extern void reset_visuals(void);
extern void flavor_copy(char *buf, u16b flavor, object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_flags_known(int Ind, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_desc(int Ind, char *buf, const object_type *o_ptr, int pref, int mode);
extern void object_desc_store(int Ind, char *buf, object_type *o_ptr, int pref, int mode);
extern bool identify_fully_aux(int Ind, object_type *o_ptr);
extern s16b index_to_label(int i);
extern s16b label_to_inven(int Ind, int c);
extern s16b label_to_equip(int Ind, int c);
extern s16b wield_slot(int Ind, object_type *o_ptr);
extern cptr mention_use(int Ind, int i);
extern cptr describe_use(int Ind, int i);
extern void inven_item_charges(int Ind, int item);
extern void inven_item_describe(int Ind, int item);
extern void inven_item_increase(int Ind, int item, int num);
extern void inven_item_optimize(int Ind, int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern void floor_item_notify(int Ind, s16b o_idx, bool force);
extern bool inven_carry_okay(int Ind, object_type *o_ptr);
extern s16b inven_carry(player_type *p_ptr, object_type *o_ptr);
extern bool item_tester_okay(object_type *o_ptr);
extern void display_inven(int Ind);
extern void display_equip(int Ind);
/*extern void show_inven(void);
extern void show_equip(void);
extern void toggle_inven_equip(void);
extern bool get_item(int Ind, int *cp, cptr pmt, bool equip, bool inven, bool floor);*/
extern void delete_object_idx(int i);
extern void delete_object_ptr(object_type * o_ptr);
extern void delete_object(int Depth, int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(int Depth);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern byte object_tester_flag(int Ind, object_type *o_ptr);
extern void object_known(object_type *o_ptr);
extern void object_aware(player_type *p_ptr, object_type *o_ptr);
extern void object_tried(int Ind, object_type *o_ptr);
extern s32b object_value(int Ind, object_type *o_ptr);
extern bool object_similar_floor(object_type *o_ptr, object_type *j_ptr);
extern bool object_similar(player_type *p_ptr, object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(player_type *p_ptr, object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void invwipe(object_type *o_ptr);
extern void invcopy(object_type *o_ptr, int k_idx);
extern bool check_ego(object_type *o_ptr, int level, int power, int idx);
extern void apply_magic(int Depth, object_type *o_ptr, int lev, bool okay, bool good, bool great);
extern bool place_object(int Depth, int y, int x, bool good, bool great, u16b quark);
extern void acquirement(int Depth, int y1, int x1, int num, bool great);
extern void place_trap(int Depth, int y, int x);
extern void place_gold(int Depth, int y, int x);
extern void process_objects(void);
extern void drop_near(object_type *o_ptr, int chance, int Depth, int y, int x);
extern void pick_trap(int Depth, int y, int x);
extern cptr item_activation(object_type *o_ptr);
extern void combine_pack(int Ind);
extern void reorder_pack(int Ind);
extern void setup_objects(void);
extern void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
extern void reduce_charges(object_type *o_ptr, int amt);
extern void object_own(player_type *p_ptr, object_type *o_ptr);
extern object_type* player_get_item(player_type *p_ptr, int item, int *idx);

/* party.c */
extern int party_lookup(cptr name);
extern bool player_in_party(int party_id, int Ind);
extern int party_create(int Ind, cptr name);
extern int party_add(int adder, cptr name);
extern int party_remove(int remover, cptr name);
extern void party_leave(int Ind);
extern void party_msg(int party_id, cptr msg);
extern void party_msg_format(int party_id, cptr fmt, ...);
extern void party_msg_format_near(int Ind, u16b type, cptr fmt, ...);
extern void party_gain_exp(int Ind, int party_id, s32b amount, int m_idx);
extern int party_mark_members(int Ind, int m_idx);
extern void party_share_hurt(int Ind, int Ind2);
extern bool add_hostility(int Ind, cptr name);
extern bool remove_hostility(int Ind, cptr name);
extern bool check_hostile(int attacker, int target);
extern cptr lookup_player_name(int id);
extern int lookup_player_id(cptr name);
extern void add_player_name(cptr name, int id);
extern void delete_player_id(int id);
extern void delete_player_name(cptr name);
extern void wipe_player_names();
extern int player_id_list(int **list);
extern bool pvp_okay(int attacker, int target, int mode);
extern int ego_kind_index_fuzzy(char * name);
extern int item_kind_index_fuzzy(char * name);
extern bool place_specific_object(int Depth, int y1, int x1, object_type *forge, int lev, int num);

/* save.c */
extern bool save_player(int Ind);
extern int scoop_player(char *nick, char *pass);
extern bool load_player(player_type *p_ptr);
extern bool load_server_info(void);
extern bool save_server_info(void);
extern bool wr_dungeon_special_ext(int Depth, cptr levelname);


/* spells1.c */
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int Ind, int dis);
extern void teleport_player_to(int Ind, int ny, int nx);
extern void teleport_player_level(int Ind);
extern void take_hit(int Ind, int damage, cptr kb_str);
extern void acid_dam(int Ind, int dam, cptr kb_str);
extern void elec_dam(int Ind, int dam, cptr kb_str);
extern void fire_dam(int Ind, int dam, cptr kb_str);
extern void cold_dam(int Ind, int dam, cptr kb_str);
extern bool inc_stat(int Ind, int stat);
extern bool dec_stat(int Ind, int stat, int amount, int permanent);
extern bool res_stat(int Ind, int stat);
extern bool apply_disenchant(int Ind, int mode);
extern bool project(int who, int rad, int Depth, int y, int x, int dam, int typ, int flg);
extern bool project_hook(int Ind, int typ, int dir, int dam, int flg);

/* spells2.c */
extern void wipe_spell(int Depth, int cy, int cx, int r);
extern bool hp_player(int Ind, int num);
extern bool hp_player_quiet(int Ind, int num);
extern bool warding_glyph(int Ind);
extern bool do_dec_stat(int Ind, int stat);
extern bool do_res_stat(int Ind, int stat);
extern bool do_inc_stat(int Ind, int stat);
extern void identify_pack(int Ind);
extern void message_pain(int Ind, int m_idx, int dam);
extern bool remove_curse(int Ind);
extern bool remove_all_curse(int Ind);
extern bool restore_level(int Ind);
extern void self_knowledge(int Ind, bool spoil);
extern bool lose_all_info(int Ind);
extern void map_area(int Ind);
extern bool detect_treasure(int Ind);
extern bool detect_objects_magic(int Ind);
extern bool detect_invisible(int Ind, bool pause);
extern bool detect_evil(int Ind);
extern bool detect_creatures(int Ind, bool pause);
extern bool detection(int Ind);
extern bool detect_objects_normal(int Ind);
extern bool detect_trap(int Ind);
extern bool detect_sdoor(int Ind);
extern void stair_creation(int Ind);
extern bool enchant(int Ind, object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int Ind, int num_hit, int num_dam, int num_ac);
extern bool enchant_spell_aux(int Ind, int item, int num_hit, int num_dam, int num_ac);
extern bool ident_spell(int Ind);
extern bool ident_spell_aux(int Ind, int item);
extern bool identify_fully(int Ind);
extern bool identify_fully_item(int Ind, int item);
extern bool recharge(int Ind, int num);
extern bool recharge_aux(int Ind, int item, int num);
extern bool speed_monsters(int Ind);
extern bool slow_monsters(int Ind);
extern bool sleep_monsters(int Ind);
extern void aggravate_monsters(int Ind, int who);
extern bool banishment(int Ind);
extern bool mass_banishment(int Ind);
extern bool probing(int Ind);
extern bool banish_evil(int Ind, int dist);
extern bool dispel_evil(int Ind, int dam);
extern bool dispel_undead(int Ind, int dam);
extern bool dispel_monsters(int Ind, int dam);
extern bool turn_undead(int Ind);
extern void destroy_area(int Depth, int y1, int x1, int r, bool full);
extern void earthquake(int Depth, int cy, int cx, int r);
extern void lite_room(int Ind, int Depth, int y1, int x1);
extern void unlite_room(int Ind, int Depth, int y1, int x1);
extern bool lite_area(int Ind, int dam, int rad);
extern bool unlite_area(int Ind, int dam, int rad);
extern bool fire_swarm(int Ind, int num, int typ, int dir, int dam, int rad);
extern bool fire_ball(int Ind, int typ, int dir, int dam, int rad);
extern bool fire_bolt(int Ind, int typ, int dir, int dam);
extern bool fire_beam(int Ind, int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int Ind, int prob, int typ, int dir, int dam);
extern bool lite_line(int Ind, int dir);
extern bool strong_lite_line(int Ind, int dir);
extern bool drain_life(int Ind, int dir, int dam);
extern bool wall_to_mud(int Ind, int dir);
extern bool destroy_door(int Ind, int dir);
extern bool disarm_trap(int Ind, int dir);
extern bool heal_monster(int Ind, int dir);
extern bool speed_monster(int Ind, int dir);
extern bool slow_monster(int Ind, int dir);
extern bool sleep_monster(int Ind, int dir);
extern bool confuse_monster(int Ind, int dir, int plev);
extern bool fear_monster(int Ind, int dir, int plev);
extern bool poly_monster(int Ind, int dir);
extern bool clone_monster(int Ind, int dir);
extern bool teleport_monster(int Ind, int dir);
extern bool alter_reality(int Ind, bool power);
extern bool heal_player_ball(int Ind, int dir, int dam);
extern bool project_spell_ball(int Ind, int dir, int spell);
extern bool project_prayer_ball(int Ind, int dir, int spell);
extern bool door_creation(int Ind);
extern bool trap_creation(int Ind);
extern bool destroy_doors_touch(int Ind);
extern bool sleep_monsters_touch(int Ind);
extern bool create_artifact(int Ind);
extern bool create_artifact_aux(int Ind, int item);
extern void brand_ammo(int Ind, int item);
extern void brand_weapon(int Ind);
extern bool brand_bolts(int Ind);
extern void ring_of_power(int Ind, int dir);
extern bool curse_armor(int Ind);
extern bool curse_weapon(int Ind);


/* store.c */
extern bool get_store_item(int Ind, int item, object_type *i_ptr);
extern int get_player_store_name(int num, char *name);
extern void store_purchase(int Ind, int item, int amt, cptr checksum);
extern void store_sell(int Ind, int item, int amt);
extern void store_confirm(int Ind);
extern void do_cmd_store(int Ind, int pstore);
extern void store_shuffle(int which);
extern void store_maint(int which);
extern void store_init(int which);
extern s32b player_price_item(int Ind, object_type *o_ptr);

/* util.c */
extern errr path_parse(char *buf, int max, cptr file);
extern errr path_temp(char *buf, int max);
extern errr path_build(char *buf, int max, cptr path, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fputs(FILE *fff, cptr buf, huge n);
extern errr my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern errr fd_copy(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);
extern void flush(void);
extern void bell(void);
extern void sound(int Ind, int num);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern void keymap_init(void);
extern void macro_add(cptr pat, cptr act, bool cmd_flag);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern s16b quark_add(cptr str);
extern void fill_prevent_inscription(bool *arr, s16b quark);
extern void update_prevent_inscriptions(int Ind);
extern bool check_guard_inscription( s16b quark, char what);
extern s16b message_num(void);
extern cptr message_str(s16b age);
extern void message_add(cptr msg);
extern void msg_print(int Ind, cptr msg);
extern void msg_print_aux(int Ind, cptr msg, u16b type);
extern void msg_broadcast(int Ind, cptr msg);
extern void msg_channel(int chan, cptr msg);
extern void msg_format(int Ind, cptr fmt, ...);
extern void msg_format_type(int Ind, u16b type, cptr fmt, ...);
extern void msg_print_near(int Ind, cptr msg);
extern void msg_format_near(int Ind, cptr fmt, ...);
extern void msg_print_complex_near(int Ind, int Ind2, u16b type, cptr msg);
extern void msg_format_complex_near(int Ind, int Ind2, u16b type, cptr fmt, ...);
extern void player_talk(int Ind, char *msg);
extern void channel_join(int Ind, cptr channel, bool quiet);
extern void channel_leave(int Ind, cptr channel);
extern void channel_leave_id(int Ind, int i, bool quiet);
extern void channels_leave(int Ind);
extern bool is_a_vowel(int ch);
extern void log_history_event(int Ind, char *msg, bool unique);
extern void history_wipe(history_event *evt);
extern cptr format_history_event(history_event *evt);
extern int color_text_to_attr(cptr name);
extern int color_opposite(int color);
extern cptr attr_to_text(byte a);
extern void send_prepared_info(player_type *p_ptr, byte win, byte stream);
extern void send_prepared_popup(int Ind, cptr header);
extern void text_out(cptr buf);
extern void text_out_c(byte a, cptr buf);
extern void text_out_init(int Ind);
extern void text_out_done();
extern void text_out_save();
extern void text_out_load();
extern void c_prt(player_type *p_ptr, byte attr, cptr str, int row, int col);
extern void prt(player_type *p_ptr, cptr str, int row, int col);
extern void clear_line(int Ind, int row);
extern void clear_from(int Ind, int row);
extern bool ask_for(int Ind, char query, char *buf); 
extern bool askfor_aux(int Ind, char query, char *buf, int row, int col, cptr prompt, cptr default_value, byte prompt_attr, byte input_attr);

/* xtra1.c */
extern void cnv_stat(int val, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern void notice_stuff(int Ind);
extern void update_stuff(int Ind);
extern void redraw_stuff(int Ind);
extern void window_stuff(int Ind);
extern void handle_stuff(int Ind);
extern void prt_history(int Ind);
extern void c_prt_status_line(player_type *p_ptr, cave_view_type *dest, int len);
extern void player_flags(int Ind, u32b *f1, u32b * f2, u32b *f3);

/* xtra2.c */
extern int get_player(int Ind, object_type *o_ptr);
extern void describe_player(int Ind, int who);
extern bool set_blind(int Ind, int v);
extern bool set_confused(int Ind, int v);
extern bool set_poisoned(int Ind, int v);
extern bool set_afraid(int Ind, int v);
extern bool set_paralyzed(int Ind, int v);
extern bool set_image(int Ind, int v);
extern bool set_fast(int Ind, int v);
extern bool set_slow(int Ind, int v);
extern bool set_shield(int Ind, int v);
extern bool set_blessed(int Ind, int v);
extern bool set_hero(int Ind, int v);
extern bool set_shero(int Ind, int v);
extern bool set_protevil(int Ind, int v);
extern bool set_invuln(int Ind, int v);
extern bool set_tim_invis(int Ind, int v);
extern bool set_tim_infra(int Ind, int v);
extern bool set_oppose_acid(int Ind, int v);
extern bool set_oppose_elec(int Ind, int v);
extern bool set_oppose_fire(int Ind, int v);
extern bool set_oppose_cold(int Ind, int v);
extern bool set_oppose_pois(int Ind, int v);
extern bool set_stun(int Ind, int v);
extern bool set_cut(int Ind, int v);
extern bool set_food(int Ind, int v);
extern void set_recall(int Ind, object_type * o_ptr);
extern void check_experience(int Ind);
extern void gain_exp(int Ind, s32b amount);
extern void lose_exp(int Ind, s32b amount);
extern void monster_death(int Ind, int m_idx);
extern void player_death(int Ind);
extern void resurrect_player(int Ind);
extern bool mon_take_hit(int Ind, int m_idx, int dam, bool *fear, cptr note);
extern bool adjust_panel(int Ind, int y, int x);
extern void panel_bounds(int Ind);
extern void verify_panel(int Ind);
extern void setup_panel(int Ind, bool adjust);
extern cptr look_mon_desc(int m_idx);
extern int pick_arena(int Depth, int y, int x);
extern int pick_arena_opponent(int Depth, int a);
extern void access_arena(int Ind, int y, int x);
extern void ang_sort_aux(int Ind, vptr u, vptr v, int p, int q);
extern void ang_sort(int Ind, vptr u, vptr v, int n);
extern void ang_sort_swap_distance(int Ind, vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_distance(int Ind, vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_value(int Ind, vptr u, vptr v, int a, int b);
extern void ang_sort_swap_value(int Ind, vptr u, vptr v, int a, int b);
extern bool target_able(int Ind, int m_idx);
extern bool target_okay(int Ind);
extern s16b target_pick(int Ind, int y1, int x1, int dy, int dx);
extern bool target_set_interactive(int Ind, int mode, char query);
extern bool get_aim_dir(int Ind, int *dp);
extern bool get_item(int Ind, int *cp, byte tval_hook);
extern bool confuse_dir(bool confused, int *dp);
extern int motion_dir(int y1, int x1, int y2, int x2);
extern bool do_scroll_life(int Ind);
extern bool do_restoreXP_other(int Ind);
extern int level_speed(int Ind);
extern int time_factor(int Ind);
extern int base_time_factor(int Ind, int slowest);
extern void show_motd(player_type *p_ptr);
extern void show_tombstone(player_type *p_ptr);
extern void wipe_socials();
extern void boot_socials();
extern void show_socials(int Ind);
extern void do_cmd_social(int Ind, int dir, int i);
extern void do_cmd_dungeon_master(int Ind, char query);

extern void master_new_hook(int Ind, char hook_q, s16b oy, s16b ox);

extern bool master_level(int Ind, char * parms);
extern bool master_build(int Ind, char * parms);
extern bool master_summon(int Ind, char * parms);
extern bool master_generate(int Ind, char * parms);
extern bool master_acquire(int Ind, char * parms);
extern bool master_player(int Ind, char * parms);

/*extern bool get_rep_dir(int *dp);*/

extern bool c_get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor);

/* x-spell.c */
extern cptr spell_names[MAX_SPELL_REALMS][PY_MAX_SPELLS];
extern magic_type ghost_spells[8];
extern void spells_init();
extern int get_spell_index(int Ind, const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern cptr get_spell_info(int Ind, int index);
extern byte get_spell_flag(int tval, int spell, byte player_flag);
extern bool cast_spell(int Ind, int tval, int index);
extern bool cast_spell_hack(int Ind, int tval, int index);

/* use-obj.c */
extern bool use_object(int Ind, object_type *o_ptr, int item, bool *ident);
extern bool use_object_current(int Ind);

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
