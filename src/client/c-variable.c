#include "c-angband.h"

/* Client global variables */

char nick[MAX_CHARS];
char pass[MAX_CHARS];

char real_name[MAX_CHARS];

char server_name[MAX_CHARS];
int server_port;

object_type *inventory; 	/* The client-side copy of the inventory */
char **inventory_name;  	/* The client-side copy of the inventory names */
char **inventory_name_one;	/* Client-side copy of inventory names (singular) */
byte *inventory_secondary_tester;/* Secondary item tester (if known) */

object_type floor_item;
char floor_name[MAX_CHARS];     /* Client-side copy of floor item */
char floor_name_one[MAX_CHARS]; /* Client-side copy of floor item (singular) */
byte floor_secondary_tester;

indicator_type indicators[MAX_INDICATORS];
int known_indicators;

u32b indicator_window[MAX_INDICATORS]; /* Client-side mapping of indicators->PW_ flags */
char *str_coffers[MAX_COFFERS]; /* Client-side copy of string variables */
s32b coffers[MAX_COFFERS]; /* Client-side copy of various player variables */
byte coffer_refs[MAX_INDICATORS]; /* ID to INDEX: Coffers */
int known_coffers;

store_type store;			/* The general info about the current store */
owner_type store_owner;			/* The current owner of the store */
char store_owner_name[MAX_CHARS]; /* Shop owner name */
char store_name[MAX_CHARS];			/* Shop name */
byte store_flag = 0;  		 	/* Shop type */
int store_prices[STORE_INVEN_MAX];	/* The prices of the items in the store */
char store_names[STORE_INVEN_MAX][MAX_CHARS];	/* The names of the stuff in the store */
s16b store_num;				/* The current store number */

char spell_info[26][SPELLS_PER_BOOK+1][MAX_CHARS];		/* Spell information */
byte spell_flag[26 * (SPELLS_PER_BOOK+1)];  	/* Spell flags */
byte spell_test[26 * (SPELLS_PER_BOOK+1)];	/* TV filter for item-related spells */

char party_info[160];			/* Information about your party */
channel_type channels[MAX_CHANNELS];
s16b view_channel = 0;

server_setup_t Setup;				/* The information given to us by the server */
client_setup_t Client_setup;		/* The information we give to the server */

s16b lag_mark;
s16b lag_minus;
bool redraw_lag_meter = FALSE;
char health_track_num;
byte health_track_attr;

bool shopping;				/* Are we in a store? */
bool shopping_buying;		/* Are we buying? */ 
bool leave_store; 		/* Time to leave store */
bool enter_store; 		/* Time to enter store */

s16b last_line_info;			/* Last line of info we've received */
s16b max_line;				/* Maximum amount of "special" info */
s16b cur_line;				/* Current displayed line of "special" info */

cave_view_type* remote_info[16]; /* Local copies for Special Info */
s16b last_remote_line[16];
cptr stream_desc[32];

cave_view_type sfx_info[MAX_HGT][MAX_WID] = { 0 };
s32b sfx_delay[MAX_HGT][MAX_WID] = { 0 };

cave_view_type air_info[MAX_HGT][MAX_WID] = { 0 };
s32b air_delay[MAX_HGT][MAX_WID] = { 0 };
s32b air_fade[MAX_HGT][MAX_WID] = { 0 };
bool air_updates = TRUE;
bool air_refresh = TRUE;

int lag_ok;				/* server understands lag-check packets */

player_type player;			/* The client-side copy of some of the player information */
player_type *p_ptr = &player;
player_type *Players = &player;

s32b exp_adv;				/* Amount of experience required to advance a level */

bool prompt_quote_hack = FALSE; /* Allow '"' to be used in place of '\r' */

s16b command_see;
s16b command_gap;
s16b command_wrk;

bool spellcasting = FALSE; /* Selecting a magic book */
int spellcasting_spell = -1; /* Select-by-name spell */

int inven_out_index[256] = { -2 }; /* Last line->a) index mappings from show_equip() */

bool item_tester_full;
byte item_tester_tval;
bool (*item_tester_hook)(object_type *o_ptr);
item_tester_type item_tester[MAX_ITEM_TESTERS];
int known_item_testers;

//int special_line_type; /* Which interactive terminal we're interacting with */
char special_line_header[MAX_CHARS];
bool special_line_onscreen = FALSE;

bool special_line_requested = FALSE;
bool simple_popup_requested = FALSE;
bool local_browser_requested = FALSE;

bool confirm_requested = FALSE;
char confirm_prompt[MAX_CHARS];
byte confirm_id;
byte confirm_type;

bool pause_requested = FALSE;

bool inkey_base = FALSE;
bool inkey_xtra = FALSE;		/* See the "inkey()" function */
bool inkey_scan = FALSE;
bool inkey_flag = FALSE;

bool inkey_exit = FALSE;
bool inkey_nonblock = FALSE;	/* Set to TRUE for a single non-blocking read. */

bool first_escape = TRUE;

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
u32b window_flag_o[8]; /* Flags as user intended, no state modifications */

byte color_table[256][4];

cptr ANGBAND_SYS;

cptr ANGBAND_FON = "";
cptr ANGBAND_FONTNAME = "";
cptr ANGBAND_GRAFNAME = "";

byte mousemap[0xFF] = { 0 };

cptr keymap_act[KEYMAP_MODES][256]; /* Keymaps for each "mode" associated with each keypress. */

s16b command_cmd;
s16b command_dir;
event_type command_cmd_ex; /* Gives additional information of current command */

custom_command_type custom_command[MAX_CUSTOM_COMMANDS];
int custom_commands;

stream_type streams[MAX_STREAMS];
int known_streams;

byte window_to_stream[ANGBAND_TERM_MAX];
byte stream_group[MAX_STREAMS];
byte stream_groups;

s16b race;
s16b pclass;
s16b sex;
char ptitle[80];

byte A_MAX = 0;
cptr *stat_names;
s16b stat_order[A_CAP];			/* Desired order of stats */

bool flip_inven = FALSE;
s16b flip_charsheet = 0;

bool topline_icky;
bool screen_icky;
s16b section_icky_col;
byte section_icky_row;
bool party_mode;
bool cursor_icky;
bool looking;
bool in_item_prompt = FALSE;

byte icky_levels; /* How many levels of ickyness nested -- DO NOT USE */

/* Player equipment refrence array */
char *eq_name;
s16b *eq_names;
byte *eq_xpos;
byte *eq_ypos;

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

s16b FLOOR_INDEX = -1;
bool FLOOR_NEGATIVE = TRUE;
s16b FLOOR_TOTAL = 1;

u16b MAX_OBJFLAGS_ROWS = 13;
u16b MAX_OBJFLAGS_COLS = 39;


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
cptr ANGBAND_DIR_XTRA_GRAF;


int use_graphics;
bool big_tile;
bool use_sound;

cptr sound_file[MSG_MAX][SAMPLE_MAX];

cave_view_type trans[40][88];

option_type *option_info;
cptr *option_group;
int options_max;
int options_groups_max;
int known_options;

bool rogue_like_commands;
bool depth_in_feet;
bool auto_accept;
bool auto_itemlist;
bool auto_showlist;
bool show_labels;
bool show_weights;
bool wrap_messages;
bool ring_bell;
bool use_color;

bool use_old_target;

bool ignore_birth_options;

bool escape_in_macro_triggers = FALSE;

int char_screen_mode;
bool target_recall;
char target_prompt[60];

byte hitpoint_warn_toggle = 0;
