
/* File: variable.c */

/* Purpose: Angband variables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


/*
 * Hack -- Link a copyright message into the executable
 */
cptr copyright[6] =
{
	"Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
	"also Copyright (c) 1997 Keldon L. Jones",
	"",
	"This software may be copied and distributed for educational, research,",
	"and not for profit purposes provided that this copyright and statement",
	"are included in all such copies."
};


/*
 * Hack -- Link the "version" into the executable
 */
byte version_major = SERVER_VERSION_MAJOR;
byte version_minor = SERVER_VERSION_MINOR;
byte version_patch = SERVER_VERSION_PATCH;
byte version_extra = SERVER_VERSION_EXTRA;


u16b shutdown_timer;	/* Shutdown server in (seconds) */

/*
 * Hack -- Savefile information
 */
u32b sf_xtra;			/* Operating system info */
u32b sf_when;			/* Time when savefile created */
u16b sf_lives;			/* Number of past "lives" with this file */
u16b sf_saves;			/* Number of "saves" during this life */

/*
 * Hack -- Run-time arguments
 */
cptr arg_config_file = NULL;	/* Command arg -- Desired config file */
bool arg_wizard;		/* Command arg -- Enter wizard mode */
bool arg_fiddle;		/* Command arg -- Enter fiddle mode */
bool arg_force_original;	/* Command arg -- Force original keyset */
bool arg_force_roguelike;	/* Command arg -- Force roguelike keyset */

bool server_generated;		/* The character exists */
bool server_dungeon;		/* The character has a dungeon */
bool server_state_loaded;	/* The server state was loaded from a savefile */
bool server_saved;		/* The character was just saved to a savefile */

bool character_loaded;		/* The character was loaded from a savefile */
bool character_died;		/* The character in the savefile was dead */
bool character_xtra;		/* The game is in an icky startup mode */

u32b seed_flavor;		/* Hack -- consistent object colors */
u32b seed_town;			/* Hack -- consistent town layout */

/*s16b command_cmd;*/		/* Current "Angband Command" */

/*s16b command_arg;*/		/* Gives argument of current command */
/*s16b command_rep;*/		/* Gives repetition of current command */
/*s16b command_dir;*/		/* Gives direction of current command */

/*s16b command_see;*/		/* See "cmd1.c" */
/*s16b command_wrk;*/		/* See "cmd1.c" */
/*s16b command_gap = 50;*/	/* See "cmd1.c" */

/*s16b command_new;*/		/* Command chaining from inven/equip view */

bool create_up_stair;		/* Auto-create "up stairs" */
bool create_down_stair;		/* Auto-create "down stairs" */

bool msg_flag;			/* Used in msg_print() for "buffering" */

/*bool alive;*/			/* True if game is running */
/*bool death;*/			/* True if player has died */

/*s16b running;*/			/* Current counter for running, if any */
/*s16b resting;*/			/* Current counter for resting, if any */

/*s16b cur_hgt;	*/		/* Current dungeon height */
/*s16b cur_wid;	*/		/* Current dungeon width */
/*s16b dun_level;	*/		/* Current dungeon level */
s16b num_repro;			/* Current reproducer count */
s16b object_level;		/* Current object creation level */
s16b monster_level;		/* Current monster creation level */

byte trees_in_town;

byte level_up_y[MAX_DEPTH]; 	/* Where do players start if coming up? */
byte level_up_x[MAX_DEPTH];
byte level_down_y[MAX_DEPTH];	/* Where do players start if going down? */
byte level_down_x[MAX_DEPTH];
byte level_rand_y[MAX_DEPTH];	/* Where do players start if they tele level? */
byte level_rand_x[MAX_DEPTH];

s16b players_on_world[MAX_DEPTH + MAX_WILD];
s16b *players_on_depth=&(players_on_world[MAX_WILD]);  /* How many players are at each depth */

s16b special_levels[MAX_SPECIAL_LEVELS]; /* List of depths which are special static levels */

char summon_kin_type;		/* Hack -- See summon_specific() */

hturn turn;			/* Current game turn */
hturn turn_compact;		/* Last time m_list and o_list were compacted */
/*hturn old_turn;*/		/* Turn when level began (feelings) */

s32b player_id;			/* Current player ID */

/*bool wizard;*/			/* Is the player currently in Wizard mode? */
/*bool can_be_wizard;*/		/* Does the player have wizard permissions? */

/*u16b total_winner;*/		/* Semi-Hack -- Game has been won */

u16b panic_save;		/* Track some special "conditions" */
u16b noscore;			/* Track various "cheating" conditions */

s16b signal_count = 0;		/* Hack -- Count interupts */

#if 0 /* moved to client */
bool inkey_base;		/* See the "inkey()" function */
bool inkey_xtra;		/* See the "inkey()" function */
bool inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */
#endif

s16b coin_type;			/* Hack -- force coin type */

bool opening_chest;		/* Hack -- prevent chest generation */

int use_graphics;		/* Hack -- Assume no graphics mapping */

bool use_sound;			/* Hack -- Assume no special sounds */

bool scan_monsters;		/* Hack -- optimize multi-hued code, etc */
bool scan_objects;		/* Hack -- optimize multi-hued code, etc */

/*s16b total_weight;*/		/* Total weight being carried */

/*s16b inven_nxt;*/			/* Hack -- unused */

/*s16b inven_cnt;*/			/* Number of items in inventory */
/*s16b equip_cnt;*/			/* Number of items in equipment */

s32b o_nxt = 1;			/* Object free scanner */
s32b m_nxt = 1;			/* Monster free scanner */

s32b o_max = 1;			/* Object heap size */
s32b m_max = 1;			/* Monster heap size */

s32b o_top = 0;			/* Object top size */
s32b m_top = 0;			/* Monster top size */

s32b p_max = 0;			/* Player heap size */ 

/*
 * Server options, set in mangband.cfg
 */

bool cfg_report_to_meta = 0;
char * cfg_meta_address = NULL;
char * cfg_report_address = NULL;
char * cfg_bind_name = NULL;
char * cfg_console_password = NULL;
bool cfg_console_local_only = FALSE;
char * cfg_dungeon_master = NULL;
char * cfg_load_pref_file = NULL;
bool cfg_secret_dungeon_master = 0;
s16b cfg_fps = 12;
s32b cfg_tcp_port = 18346;
bool cfg_safe_recharge = FALSE;
bool cfg_no_steal = 0;
bool cfg_newbies_cannot_drop = 0;
bool cfg_ghost_diving = 0;
s32b cfg_level_unstatic_chance = 60;
bool cfg_random_artifacts = 0; /* No randarts by default */
s16b cfg_preserve_artifacts = 3;
s32b cfg_retire_timer = -1;
bool cfg_ironman = 0;
bool cfg_more_towns = 0;
bool cfg_town_wall = 0;
s32b cfg_unique_respawn_time = 300;
s32b cfg_unique_max_respawn_time = 50000;
s16b cfg_max_townies = 100;
s16b cfg_max_trees = 100;
s16b cfg_max_houses = 0;
bool cfg_chardump_color = FALSE;
s16b cfg_pvp_hostility = 2;
bool cfg_pvp_notify = FALSE;
s16b cfg_pvp_safehostility = 3;
s16b cfg_pvp_safedepth = -1;
s16b cfg_pvp_saferadius = -1;
s16b cfg_pvp_safelevel = -1;
bool cfg_party_share_exp = TRUE;
bool cfg_party_share_quest = FALSE;
bool cfg_party_share_kill = FALSE;
bool cfg_party_share_win = TRUE;
s16b cfg_party_sharelevel = -1;
bool cfg_instance_closed = FALSE;




bool dungeon_align; 	/* Most recent align request */
s16b feeling;			/* Most recent feeling */
s16b rating;			/* Level's current rating */

bool good_item_flag;		/* True if "Artifact" on this level */

/*bool new_level_flag;*/		/* Start a new level */

/*bool closing_flag;*/		/* Dungeon is closing */


/* Dungeon size info */
/*s16b max_panel_rows, max_panel_cols;
s16b panel_row, panel_col;
s16b panel_row_min, panel_row_max;
s16b panel_col_min, panel_col_max;*/
/*s16b panel_col_prt, panel_row_prt;*/

/* Player location in dungeon */
/* This is now put in the player info --KLJ-- */
/*s16b py;
s16b px;*/

/* Targetting variables */
/* ... in player info */
/*s32b target_who;
s16b target_col;
s16b target_row;*/

/* Health bar variable -DRS- */
/*s16b health_who;*/

/* Monster recall race */
/*s16b recent_idx;*/



/* Collection of connections/players */
element_group* players = NULL;

/* User info in z-file.c
int player_uid = 0;
int player_euid = 0;
int player_egid = 0;
*/

/* The array of players */
player_type **Players;

/* The party information */
party_type parties[MAX_PARTIES];

/* Chat channels */
channel_type channels[MAX_CHANNELS];
int chan_audit, chan_debug, chan_cheat;

/* Arenas information */ 
arena_type arenas[MAX_ARENAS];
int num_arenas;

/* The information about houses */
house_type houses[MAX_HOUSES];
int num_houses;

/* This information will help client */
byte spell_flags[MAX_SPELL_REALMS][PY_MAX_SPELLS];
byte spell_tests[MAX_SPELL_REALMS][PY_MAX_SPELLS];

/* Player for doing text_out */
player_type* player_textout = NULL;

/* Current player's character name */
/* In the player info --KLJ-- */
/*char player_name[32];*/

/* Stripped version of "player_name" */
/* In the player info --KLJ-- */
/*char player_base[32];*/

/* What killed the player */
/* In the player info --KLJ-- */
/*char died_from[80];*/

/* Hack -- Textual "history" for the Player */
/* In the player info --KLJ-- */
/*char history[4][60];*/

/* Buffer to hold the current savefile name */
/*char savefile[1024];*/


/*
 * Array of grids lit by player lite (see "cave.c")
 */
/*s16b lite_n;
byte lite_y[LITE_MAX];
byte lite_x[LITE_MAX];*/

/*
 * Array of grids viewable to the player (see "cave.c")
 *
 * This is now in the player info, so that every player sees the cave
 * differently --KLJ--
 */
/*s16b view_n;
byte view_y[VIEW_MAX];
byte view_x[VIEW_MAX];*/

/*
 * Array of grids for use by various functions (see "cave.c")
 */ /* In player info */
/*s16b temp_n;
byte temp_y[TEMP_MAX];
byte temp_x[TEMP_MAX];*/


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
cptr *macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
cptr *macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool *macro__cmd;

/*
 * Current macro action [1024]
 */
char *macro__buf;


/*
 * The number of quarks
 */
s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
cptr *quark__str;


/*
 * The next "free" index to use
 */
u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
u16b message__last;

/*
 * The next "free" offset
 */
u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
u16b message__tail;

/*
 * The array of offsets, by index [MESSAGE_MAX]
 */
u16b *message__ptr;

/*
 * The array of chars, by offset [MESSAGE_BUF]
 */
char *message__buf;


/*
 * The array of normal options
 */ /* on client / player info */
/*u32b option_flag[8];
u32b option_mask[8];*/


/*
 * The array of window options
 */ /* on client */
/*u32b window_flag[8];
u32b window_mask[8];*/


/*
 * The array of window pointers
 */
/*term *ang_term[8];*/


/*
 * The array of indexes of "live" objects
 */
s16b o_fast[MAX_O_IDX];

/*
 * The array of indexes of "live" monsters
 */
s16b m_fast[MAX_M_IDX];


/*
 * The array of "cave grids" [MAX_WID][MAX_HGT].
 * Not completely allocated, that would be inefficient
 * Not completely hardcoded, that would overflow memory
 */
/* For wilderness, I am hacking this to extend in the negative direction.
   I currently have a huge number (4096?) of wilderness levels allocated.
   -APD-
*/ 
cave_type **world[MAX_DEPTH+MAX_WILD]; 
cave_type ***cave = &world[MAX_WILD];
wilderness_type world_info[MAX_WILD+1];
wilderness_type *wild_info=&(world_info[MAX_WILD]);

/*
 * Track turns which were used for dungeon generation
 */
hturn turn_worldgen[MAX_DEPTH+MAX_WILD];
hturn *turn_cavegen = &turn_worldgen[MAX_WILD];

/*
 * The array of dungeon items [MAX_O_IDX]
 */
object_type *o_list;

/*
 * The array of dungeon monsters [MAX_M_IDX]
 */
monster_type *m_list;

/*
 * The array of dungeon players [MAX_PLAYERS]
 */
player_type **p_list;

/*
 * Hack -- Quest array
 *
 * Also moved to player info 
 */
/* quest q_list[MAX_Q_IDX]; */


/*
 * The stores [MAX_STORES]
 */
store_type *store;

/*
 * The player's inventory [INVEN_TOTAL]
 *
 * This is now in the player info --KLJ--
 */
/* object_type *inventory; */


/*
 * The size of "alloc_kind_table" (at most MAX_K_IDX * 4)
 */
s16b alloc_kind_size;

/*
 * The entries in the "kind allocator table"
 */
alloc_entry *alloc_kind_table;


/*
 * The size of "alloc_race_table" (at most MAX_R_IDX)
 */
s16b alloc_race_size;

/*
 * The entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;



/*
 * Specify attr/char pairs for visual special effects
 * Be sure to use "index & 0x400" to avoid illegal access
 */
byte misc_to_attr[1024];
char misc_to_char[1024];
/*
 * Specify attr/char pairs for inventory items (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];
char tval_to_char[128];

/*
 * Simple keymap method, see "init.c" and "cmd6.c".
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte keymap_cmds[128];
byte keymap_dirs[128];


/*
 * Global table of color definitions
 * Be sure to use "index & 0xFF" to avoid illegal access
 */
byte color_table[256][4];

/*
 * Static arrays to hold text screen loaded from TEXTFILEs.
 */
char text_screen[MAX_TEXTFILES][TEXTFILE__WID * TEXTFILE__HGT];

/*** Player information ***/

/*
 * Static player info record
 *
 * Don't need this anymore --KLJ--
 */
/*static player_type p_body;*/

/*
 * Pointer to the player info
 *
 * Or this --KLJ--
 */
/*player_type *p_ptr = &p_body;*/

/*
 * Pointer to the player tables (race, class, magic)
 *
 * This is in the player info now --KLJ--
 */
/*player_race *rp_ptr;
player_class *cp_ptr;
player_magic *mp_ptr;*/



/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 *
 * This is in the player info now --KLJ--
 */
/*s16b player_hp[PY_MAX_LEVEL];*/


/* Length of equipment slot names combined */
u32b eq_name_size;

/*
 * Structure (not array) of size limits
 */
maxima *z_info;

/*
 * The vault generation arrays
 */
vault_type *v_info;
char *v_name;
char *v_text;

/*
 * The terrain feature arrays
 */
feature_type *f_info;
char *f_name;
char *f_text;
char *f_char_s; /* copy of f_info characters */
byte *f_attr_s; /* copy of f_info attributes */

/*
 * The object kind arrays
 */
object_kind *k_info;
char *k_name;
char *k_text;
char *k_char_s; /* copy of k_info characters */
byte *k_attr_s; /* copy of k_info attributes */

/*
 * The artifact arrays
 */
artifact_type *a_info;
char *a_name;
char *a_text;

/*
 * The ego-item arrays
 */
ego_item_type *e_info;
char *e_name;
char *e_text;


/*
 * The monster race arrays
 */
monster_race *r_info;
char *r_name;
char *r_text;
char *r_char_s; /* copy of r_info characters */
byte *r_attr_s; /* copy of r_info attributes */


/*
 * The player race arrays
 */
player_race *p_info;
char *p_name;
char *p_text;

/*
 * The player class arrays
 */
player_class *c_info;
char *c_name;
char *c_text;

/*
 * The shop owner arrays
 */
owner_type *b_info;
char *b_name;
char *b_text;

/*
 * The racial price adjustment arrays
 */
byte *g_info;
char *g_name;
char *g_text;

/*
 * The player history arrays
 */
hist_type *h_info;
char *h_text;

/*
 * The object flavor arrays
 */
flavor_type *flavor_info;
char *flavor_name;
char *flavor_text;
char *flavor_char_s; /* copy of flavor_info characters */
byte *flavor_attr_s; /* copy of flavor_info attributes */

/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
cptr ANGBAND_DIR;


/*
 * Various data files for the game, such as the high score and
 * the mangband logs.
 */
cptr ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_EDIT;

/*
 * Various user editable text files (ascii), such as the help and greeting
 * screen. These files may be portable between platforms
 */
cptr ANGBAND_DIR_HELP;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_SAVE;

/*
 * Bone files for player ghosts (ascii)
 * These files are portable between platforms
 * (Used for death dumps in MAngband)
 */
cptr ANGBAND_DIR_BONE;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_PREF;


/*
 * Total Hack -- allow all items to be listed (even empty ones)
 * This is only used by "do_cmd_inven_e()" and is cleared there.
 */
bool item_tester_full;


/*
 * Here is a "pseudo-hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
byte item_tester_tval;


/*
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
bool (*item_tester_hook)(object_type*);





/*
 * Hack -- function hook to restrict "get_mon_num_prep()" function
 */
bool (*get_mon_num_hook)(int r_idx);



/*
 * Hack -- function hook to restrict "get_obj_num_prep()" function
 */
bool (*get_obj_num_hook)(int k_idx);
