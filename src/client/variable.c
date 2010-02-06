#include "c-angband.h"

/* Client global variables */

char nick[80];
char pass[80];

char real_name[80];

char server_name[80];
int server_port;

object_type *inventory; 	/* The client-side copy of the inventory */
char **inventory_name;  	/* The client-side copy of the inventory names */

object_type floor_item;
char floor_name[80]; 	/* Client-side copy of floor item */

store_type store;			/* The general info about the current store */
owner_type store_owner;			/* The current owner of the store */
char store_owner_name[MAX_CHARS]; /* Shop owner name */
char store_name[MAX_CHARS];			/* Shop name */
byte store_flag = 0;  		 	/* Shop type */
int store_prices[STORE_INVEN_MAX];	/* The prices of the items in the store */
char store_names[STORE_INVEN_MAX][80];	/* The names of the stuff in the store */
s16b store_num;				/* The current store number */

char spell_info[26][SPELLS_PER_BOOK+1][80];		/* Spell information */
byte spell_flag[26 * (SPELLS_PER_BOOK+1)];  	/* Spell flags */

char party_info[160];			/* Information about your party */
channel_type channels[MAX_CHANNELS];
s16b view_channel = 0;

server_setup_t Setup;				/* The information given to us by the server */
client_setup_t Client_setup;		/* The information we give to the server */

s16b lag_mark;
s16b lag_minus;
char health_track_num;
byte health_track_attr;

bool shopping;				/* Are we in a store? */
bool shopping_buying;		/* Are we buying? */ 
bool leave_store; 		/* Time to leave store */

s16b last_line_info;			/* Last line of info we've received */
s16b max_line;				/* Maximum amount of "special" info */
s16b cur_line;				/* Current displayed line of "special" info */

cave_view_type remote_info[8][MAX_TXT_INFO][80]; /* Local copies for Special Info */
s16b last_remote_line[8];

int lag_ok;				/* server understands lag-check packets */

player_type player;			/* The client-side copy of some of the player information */
player_type *p_ptr = &player;
player_type *Players = &player;

s32b exp_adv;				/* Amount of experience required to advance a level */

s16b command_see;
s16b command_gap;
s16b command_wrk;

bool item_tester_full;
byte item_tester_tval;
bool (*item_tester_hook)(object_type *o_ptr);
byte item_tester_tvals[MAX_ITEM_TESTERS][MAX_ITH_TVAL];
byte item_tester_flags[MAX_ITEM_TESTERS];

int special_line_type;
char special_line_header[80];
bool special_line_onscreen = TRUE;

bool inkey_base = FALSE;
bool inkey_xtra = FALSE;		/* See the "inkey()" function */
bool inkey_scan = FALSE;
bool inkey_flag = FALSE;

bool inkey_exit = FALSE;

bool first_escape = FALSE;

s16b macro__num;
cptr *macro__pat;
cptr *macro__act;
bool *macro__cmd;
char *macro__buf;

u16b message__next;
u16b message__last;
u16b message__head;
u16b message__tail;
u16b *message__ptr;
char *message__buf;
u16b *message__type;
u16b *message__count;

bool msg_flag;


int max_macrotrigger = 0;
cptr macro_template = NULL;
cptr macro_modifier_chr;
cptr macro_modifier_name[MAX_MACRO_MOD];
cptr macro_trigger_name[MAX_MACRO_TRIGGER];
cptr macro_trigger_keycode[2][MAX_MACRO_TRIGGER];


term *ang_term[8];
u32b window_flag[8];

byte color_table[256][4];

cptr ANGBAND_SYS;

cptr keymap_act[KEYMAP_MODES][256]; /* Keymaps for each "mode" associated with each keypress. */

s16b command_cmd;
s16b command_dir;

custom_command_type custom_command[MAX_CUSTOM_COMMANDS];
s16b custom_commands;

stream_type streams[MAX_STREAMS];
byte known_streams;

byte window_to_stream[ANGBAND_TERM_MAX];
byte known_window_streams;

s16b race;
s16b class;
s16b sex;
char ptitle[80];

s16b stat_order[6];			/* Desired order of stats */


bool topline_icky;
bool screen_icky;
s16b section_icky_col;
byte section_icky_row;
bool party_mode;
bool cursor_icky;
bool looking;

byte icky_levels; /* How many levels of ickyness nested -- DO NOT USE */

/* Player equipment refrence array */
char *eq_name;
s16b *eq_names;

/*
 * The player race arrays
 */
char *p_name;
player_race *race_info;
/*
 * The player class arrays
 */
char *c_name;
char *c_text;
player_class *c_info;

/*
 * Known limits
 */
maxima z_info;

s16b INVEN_TOTAL = 36;
s16b INVEN_WIELD = 24;
s16b INVEN_PACK  = 23;

cptr ANGBAND_GRAF = "none";
cptr ANGBAND_DIR;
cptr ANGBAND_DIR_APEX;
cptr ANGBAND_DIR_BONE;
cptr ANGBAND_DIR_DATA;
cptr ANGBAND_DIR_EDIT;
cptr ANGBAND_DIR_FILE;
cptr ANGBAND_DIR_HELP;
cptr ANGBAND_DIR_INFO;
cptr ANGBAND_DIR_SAVE;
cptr ANGBAND_DIR_USER;
cptr ANGBAND_DIR_PREF;
cptr ANGBAND_DIR_XTRA;
cptr ANGBAND_DIR_XTRA_SOUND;


int use_graphics;
bool big_tile;
bool use_sound;

cptr sound_file[MSG_MAX][SAMPLE_MAX];

cave_view_type trans[40][88];

option_type *option_info;
cptr *option_group;
int options_max;
int options_groups_max;

bool rogue_like_commands;
bool depth_in_feet;
bool auto_accept;
bool show_labels;
bool show_weights;
bool ring_bell;
bool use_color;

bool use_old_target;


int char_screen_mode;
bool target_recall;
char target_prompt[60];
