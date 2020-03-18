/* File: tables.c */

/* Purpose: Angband Tables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"

/*
 * Global array of "custom commands".
 *
 *   Ends with an empty entry (used while iterating, do not dismiss).
 *
 * FORMAT:
 *      key, PKT, SCHEME, energy_cost, (*do_cmd_callback)
 *      (flags | flags | flags),
 *      tval, prompt, display
 * LEGEND:
 *  key - char, single keypress : 'j'
 *  PKT - packet type to use    : To use default command set PKT_COMMAND
 *                              : To declare new command use PKT_UNDEFINED
 *                              : To overload existing command use it's PKT_ (i.e. PKT_EAT)
 *  SCHEME - see pack.h         : SCHEME CONTROLS BOTH PACKET PARSING
 *                              : AND do_cmd_callback ARGUMENTS
 *                              : IT *IS* IMPORTANT
 *  energy_cost - 0 or n        : If the command is free, use 0
 *                              : Use n to set 1/Nth of level_speed
 *                              : i.e. 2 to take half a turn, 1 for full turn, 4 for 1/4
 *  (*do_cmd_callback) - a callback to one of the "do_cmd_???" functions, arguments depend on SCHEME
 *  (flags) - see defines.h     : Each flag group requires a related prompt string
 *  tval - TVAL for item tester : Normally, a single TVAL (i.e. TVAL_POTION)
 *                              : For complex tests, use `item_test(???)` (see mdefines.h)
 *                              : For interactive mode commands, specifies SPECIAL_FILE_??? define
 *  prompt - new-line separated string of prompts for each (flags) group.
 *  display - human-friendly name of the command.
 */
const custom_command_type custom_commands[MAX_CUSTOM_COMMANDS] =
{
	/*** Moving around ***/
#if 0
	{ /* Walk 1 grid */
		';', PKT_WALK, SCHEME_DIR, 1, (cccb)do_cmd_walk,
		(COMMAND_TARGET_DIR),		0, "", "Walk"
	},
#endif
	{ /* Start running */
		'.', PKT_RUN, SCHEME_DIR, 1, (cccb)do_cmd_run,
		(COMMAND_TARGET_DIR),		0, "", "Run"
	},
	{ /* Stand still */
		',', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_stay,
		(0),		0, "", "Stay"
	},
	{ /* Hold still */
		'g', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_hold,
		(0),		0, "", "Get item"
	},

	/*** Simpliest, one-off commands ***/
	{ /* Go Up by stairs */
		'<', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_go_up,
		(0),		0, "", "Go upstairs"
	},
	{ /* Go Down by stairs */
		'>', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_go_down,
		(0),		0, "", "Go downstairs"
	},
#if 0
	{ /* Toggle Rest */
		'R', PKT_REST, SCHEME_EMPTY, 1, (cccb)do_cmd_toggle_rest,
		(0),		0, "", "Rest"
	},
#endif
	{ /* Search */
		's', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_search,
		(0),		0, "", "Search"
	},
	{ /* Toggle Search */
		'S', PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_toggle_search,
		(0),		0, "", "Searching mode"
	},
	{ /* Repeat Feeling */
		KTRL('F'), PKT_UNDEFINED, SCHEME_EMPTY, 1, (cccb)do_cmd_feeling,
		(0),		0, "", "Repeat level feeling"
	},

	/*** Simple grid altering commands ***/
	{ /* Alter */
		'+', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_alter,
		(COMMAND_TARGET_DIR),		0, "", "Alter"
	},
	{ /* Tunnel */
		'T', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_tunnel,
		(COMMAND_TARGET_DIR),		0, "", "Tunnel"
	},
	{ /* Bash a door */
		'B', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_bash,
		(COMMAND_TARGET_DIR),		0, "", "Bash"
	},
	{ /* Disarm a trap or chest */
		'D', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_disarm,
		(COMMAND_TARGET_DIR),		0, "", "Disarm"
	},
	{ /* Open door or chest */
		'o', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_open,
		(COMMAND_TARGET_DIR),		0, "", "Open"
	},
	{ /* Close door */
		'c', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_close,
		(COMMAND_TARGET_DIR),		0, "", "Close"
	},

	/*** Complex grid altering ***/
	{ /* Spike door */
		'j', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_spike,
		(COMMAND_ITEM_QUICK | COMMAND_ITEM_INVEN | COMMAND_TARGET_DIR),
		TV_SPIKE, "You have no spikes!", "Spike door"
	},
	{ /* Steal (MAngband-specific) */
		'J', PKT_UNDEFINED, SCHEME_DIR, 1, (cccb)do_cmd_steal,
		(COMMAND_TARGET_DIR),
		0, "Touch in what ", "Steal"
	},
	{ /* Purchase/Sell/Examine House (MAngband-specific) */
		'h', PKT_COMMAND, SCHEME_DIR, 1, (cccb)do_cmd_purchase_house,
		(COMMAND_TARGET_DIR),
		0, "Knock in what ", "Buy/sell house"
	},

	/*** Inventory commands ***/
	{ /* Wear/Wield Item */
		'w', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_wield,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		item_test(WEAR), "Wear/Wield which item? ", "Wear/Wield"
	},
	{ /* Takeoff */
		't', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_takeoff,
		(COMMAND_ITEM_EQUIP),
		0, "Takeoff which item? ", "Takeoff item"
	},
	{ /* Drop Item */
		'd', PKT_UNDEFINED, SCHEME_ITEM_VALUE, 1, (cccb)do_cmd_drop,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_AMMOUNT),
		0, "Drop what? \nHow much? ", "Drop item"
	},
	{ /* Destroy Item */
		'k', PKT_UNDEFINED, SCHEME_ITEM_VALUE, 1, (cccb)do_cmd_destroy,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR | COMMAND_ITEM_AMMOUNT |
		 COMMAND_NEED_CONFIRM | COMMAND_PROMPT_ITEM ),
		0, "Destroy what? \nHow many? \nReally destroy ", "Destroy item"
	},
	{ /* Inscribe Item */
		'{', PKT_UNDEFINED, SCHEME_ITEM_STRING , 0, (cccb)do_cmd_inscribe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR | COMMAND_NEED_STRING),
		0, "Inscribe what? \nInscription: ", "Inscribe item"
	},
	{ /* Uninscribe what?  */
		'}', PKT_UNDEFINED, SCHEME_ITEM, 0, (cccb)do_cmd_uninscribe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR),
		0, "Uninscribe what? ", "Uninscribe item"
	},
	{ /* Observe/Examine item  */
		'I', PKT_UNDEFINED, SCHEME_ITEM, 0, (cccb)do_cmd_observe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR),
		0, "Examine what? ", "Inspect item"
	},

	/*** Inventory "usage" commands ***/
	/* Magic devices */
	{ /* Read scroll */
		'r', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_read_scroll_on,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_ITEM_RESET | COMMAND_NEED_SECOND | COMMAND_SECOND_DIR),
		TV_SCROLL, "Read which scroll? ", "Read scroll"
	},
	{ /* Aim wand */
		'a', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_aim_wand,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_TARGET_ALLOW),
		TV_WAND, "Aim which wand? ", "Aim wand"
	},
	{ /* Use staff */
		'u', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_use_staff_pre,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_ITEM_RESET | COMMAND_NEED_SECOND | COMMAND_SECOND_DIR),
		TV_STAFF, "Use which staff? ", "Use staff"
	},
	{ /* Zap rod */
		'z', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_zap_rod_pre,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_ITEM_RESET | COMMAND_TARGET_ALLOW
		|/*item2->*/(COMMAND_NEED_SECOND | COMMAND_SECOND_DIR)/*<-item2*/),
		TV_ROD, "Use which rod? ", "Zap rod"
	},
	{ /* Activate */
		'A', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_activate_dir,
		(COMMAND_ITEM_EQUIP | COMMAND_ITEM_RESET | COMMAND_TARGET_ALLOW),
		item_test(ACTIVATE), "Activate what? ", "Activate item"
	},
	/* Common items */
	{ /* Refill */
		'F', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_refill,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		item_test(REFILL), "Refill with which light? ", "Refill light"
	},
	{ /* Drink */
		'q', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_quaff_potion,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_POTION, "Quaff which potion? ", "Quaff potion"
	},
	{ /* Eat */
		'E', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_eat_food,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_FOOD, "Eat what? ", "Eat food"
	},

	/*** Firing and throwing ***/
	{ /* Fire an object */
		'f', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_fire,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_TARGET_ALLOW),
		item_test(AMMO), "Fire which ammo? ", "Fire missile"
	},
	{ /* Throw an object */
		'v', PKT_UNDEFINED, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_throw,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_TARGET_ALLOW),
		0, "Throw what? ", "Throw item"
	},

	/*** Spell-casting ***/
	{ /* Study spell */
		'G', PKT_UNDEFINED, SCHEME_ITEM_SMALL, 1, (cccb)do_cmd_study,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET),
		TV_MAGIC_BOOK, "You cannot gain spells!\nGain from which book? \nSpells\nStudy which spell? ", "Study spell"
	},
	/* NOTE: see, overload for priests, below */
	{ /* Cast spell */
		'm', PKT_UNDEFINED, SCHEME_ITEM_DIR_SMALL, 1, (cccb)do_cmd_cast_pre,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET | 
		 COMMAND_TARGET_ALLOW | COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		TV_MAGIC_BOOK, "You cannot cast spells!\nCast from what book? \nSpells\nCast which spell? ", "Cast spell"
	},
	{ /* Use ghost power */
		'U', PKT_UNDEFINED, SCHEME_DIR_SMALL, 1, (cccb)do_cmd_ghost_power_pre,
		(COMMAND_TEST_DEAD | COMMAND_SPELL_CUSTOM | COMMAND_SPELL_RESET | COMMAND_TARGET_ALLOW | 
		 COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		(10), "You are not undead.\nPowers\nUse which power? ", "Ghost power"
	},
	{ /* Cast cleric spell */
		'p', PKT_UNDEFINED, SCHEME_ITEM_DIR_SMALL, 1, (cccb)do_cmd_pray_pre,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET | 
		 COMMAND_TARGET_ALLOW | COMMAND_TARGET_FRIEND | COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		TV_PRAYER_BOOK, "Pray hard enough and your prayers may be answered.\nPray from what book? \nPrayers\nPray which prayer? ", "Cast prayer"
	},

	/*** Knowledge query ***/
	{ /* Display monster list */
		'[', PKT_UNDEFINED, SCHEME_EMPTY, 0, (cccb)do_cmd_monlist,
		(0),
		0, "Symbol: ", "Display visible monster list"
	},
	{ /* Display item list */
		']', PKT_UNDEFINED, SCHEME_EMPTY, 0, (cccb)do_cmd_itemlist,
		(0),
		0, "", "Display visible item list"
	},
	{ /* Help */
		'?', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_HELP, "Help", "See Help"
	},
#if 1
	{ /* Knowledge */
		'#', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_KNOWLEDGE, "Knowledge", "See Knowledge"
	},
#else
	{ /* Scores */
		'#', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_SCORES, "Highscores", "See Highscores"
	},
#endif
	{ /* Artifacts */
		'~', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_ARTIFACT, "Artifacts", "List artifacts"
	},
	{ /* Uniques */
		'|', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_UNIQUE, "Uniques", "List uniques"
	},
	{ /* Players */
		'@', PKT_COMMAND, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_PLAYER, "Players", "Show online players"
	},

	/*** Miscellaneous; MAngband-specific ***/
	{ /* 'Social' */
		KTRL('S'), PKT_COMMAND, SCHEME_DIR_SMALL, 0, (cccb)do_cmd_social,
		(COMMAND_SPELL_CUSTOM | COMMAND_SPELL_RESET | COMMAND_SPELL_INDEX | COMMAND_TARGET_ALLOW),
		(12), "Socials\nDo what? ", "Socialize"
	},
	{ /* 'DM Menu' */
		'&', PKT_COMMAND, SCHEME_DIR_SMALL, 0, (cccb)do_cmd_interactive,
		(COMMAND_INTERACTIVE),
		SPECIAL_FILE_MASTER, "Dungeon Master", "DM Menu"
	},
	{ /* Mini-Map' */
		'M', PKT_UNDEFINED, SCHEME_PPTR_CHAR, 0, (cccb)do_cmd_view_map,
		(COMMAND_INTERACTIVE | COMMAND_INTERACTIVE_ANYKEY),
		99, "Mini-Map", "Display mini-map"
	},	
#if 0
	{ /* Suicide */
		'Q', PKT_UNDEFINED, SCHEME_CHAR, 1, (cccb)do_cmd_suicide,
		(COMMAND_NEED_CONFIRM | COMMAND_NEED_CHAR),
		0, "Please verify SUICIDE by typing the '@' sign: \nDo you really want to commit suicide? ", "Commit suicide"
	},
#endif
	{ /* Drop Gold */
		'$', PKT_UNDEFINED, SCHEME_VALUE, 1, (cccb)do_cmd_drop_gold,
		(COMMAND_NEED_VALUE),
		0, "How much gold? ", "Drop gold"
	},
	{ /* Symbol Query */
		'/', PKT_UNDEFINED, SCHEME_CHAR, 0, (cccb)do_cmd_query_symbol,
		(COMMAND_NEED_CHAR),
		0, "Symbol: ", "Symbol query"
	},
#if 1
	{ /* Refill bottle */
		KTRL('G'), PKT_UNDEFINED, SCHEME_ITEM, 0, (cccb)do_cmd_refill_potion,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_BOTTLE, "Refill which bottle? ", "Refill bottle"
	},
#endif

	/*** Store/shopping commands ***/
	{ /* [Get]/Purchase item */
		'p', PKT_UNDEFINED, SCHEME_ITEM_VALUE_STRING, 1, (cccb)store_purchase,
		(COMMAND_STORE | COMMAND_ITEM_STORE | COMMAND_ITEM_AMMOUNT),
		0, "Which item are you interested in? \nHow many? ", "Purchase"
	},
	{ /* [Drop]/Sell item */
		's', PKT_UNDEFINED, SCHEME_ITEM_VALUE, 1, (cccb)store_sell,
		(COMMAND_STORE | COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_AMMOUNT),
		0, "Sell what? \nHow many? ", "Sell"
	},
	{ /* [Examine]/Look at item */
		'l', PKT_UNDEFINED, SCHEME_ITEM, 1, (cccb)do_cmd_observe,
		(COMMAND_STORE | COMMAND_ITEM_STORE),
		0, "Which item do you want to examine? ", "Examine store item"
	},

#ifdef DEBUG
	{ /* Temporary debug command */
		'Z', PKT_UNDEFINED, SCHEME_STRING, 0, (cccb)file_character_server,
		(COMMAND_NEED_STRING),
		0, "Dump name: ", "Debug command"
	},
#endif

	/* End-of-array */
	{ 0 }
};
int study_cmd_id = -1; /* Set during init, to replace with: */
/* A special version for priests: */
custom_command_type priest_study_cmd =
	{ /* Study spell */
		'G', PKT_UNDEFINED, SCHEME_ITEM_SMALL, 1, (cccb)do_cmd_study,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK),
		TV_PRAYER_BOOK, "You cannot gain prayers!\nGain from which book? ", "Study prayer"
	};



/* Item testers */
item_tester_type item_tester[MAX_ITEM_TESTERS] =
{
	/* item_tester_hook_wear (ITH_WEAR) */
	{
		{ 0 },	
		(ITF_WEAR),
	},
	/* item_tester_hook_weapon (ITH_WEAPON) */
	{
		{ TV_SWORD, TV_HAFTED, TV_POLEARM, TV_DIGGING, TV_BOW, TV_BOLT, TV_ARROW, TV_SHOT, 0 }, 
		(0),
	},
	/* item_tester_hook_armour (ITH_ARMOR) */
	{
		{ TV_DRAG_ARMOR, TV_HARD_ARMOR, TV_SOFT_ARMOR, TV_SHIELD, TV_CLOAK, TV_CROWN, TV_HELM, TV_BOOTS, TV_GLOVES, 0 }, 
		(0),
	},
	/* item_tester_hook_ammo (ITH_AMMO) */
	{
		{ TV_BOLT, TV_ARROW, TV_SHOT, 0 },
		(0),
	},
	/* item_tester_hook_recharge (ITH_RECHARGE) */
	{
		{ TV_STAFF, TV_WAND, 0 },
		(0),
	},
	/* item_tester_hook_activate (ITH_ACTIVATE) */
	{
		{ 0 },
		(ITF_ACT),
	},
	/* item_tester_refill_lantern (ITH_REFILL) */
	{
		{ 0 },/*{ TV_FLASK, TV_LITE, 0 },*/
		(ITF_FUEL),
	},
	/* item_tester_refill_torch (ITH_REFILL) */
	{
		{ 0 },/*{ TV_LITE, 0 } */
		(ITF_FUEL),
	},

	/* End of array */
	{
		{ 0 }, 0
	}
};

/*
 * Global array for "data streams"
 *
*	byte pkt;
*
*	byte addr;
*
*	byte rle;
*	byte flag;
*
*	byte min_row;
*	byte min_col;
*	byte max_row;
*	byte max_col;	
*
*	u32b window_flag;
*	cptr mark;
*	cptr display_name;
 */
#define STREAM_PKT(A) PKT_STREAM + 1 + STREAM_ ## A
#define MIN_WID SCREEN_WID / 3 + 1
#define MIN_HGT SCREEN_HGT / 2 + 1
const stream_type streams[MAX_STREAMS] = 
{
	{	/* 0 */
		STREAM_PKT(DUNGEON_ASCII),	NTERM_WIN_OVERHEAD,	RLE_CLASSIC,
		(0),
		MIN_HGT, MIN_WID, MAX_HGT, MAX_WID,
		0, "DUNGEON_ASCII", "Display the dungeon"
	},
	{	/* 1 */
		STREAM_PKT(DUNGEON_GRAF1),	NTERM_WIN_OVERHEAD,	RLE_LARGE,
		(0),
		MIN_HGT, MIN_WID, MAX_HGT, MAX_WID,
		0, "DUNGEON_GRAF1", ""
	},
	{	/* 2 */
		STREAM_PKT(DUNGEON_GRAF2),	NTERM_WIN_OVERHEAD,	RLE_LARGE,
		(SF_TRANSPARENT),
		MIN_HGT, MIN_WID, MAX_HGT, MAX_WID,
		0, "DUNGEON_GRAF2", ""
	},
	{	/* 3 */
		STREAM_PKT(MINIMAP_ASCII),	NTERM_WIN_OVERHEAD,	RLE_CLASSIC,
		(SF_OVERLAYED | SF_NEXT_GROUP | SF_HIDE),
		MIN_HGT, MIN_WID, MAX_HGT, MAX_WID, 
		0, "MINIMAP_ASCII", ""
	},
	{	/* 4 */
		STREAM_PKT(MINIMAP_GRAF),	NTERM_WIN_OVERHEAD,	RLE_LARGE,	
		(SF_OVERLAYED),
		MIN_HGT, MIN_WID, MAX_HGT, MAX_WID,
		0, "MINIMAP_GRAF", ""
	},
	{	/* 5 */
		STREAM_PKT(BGMAP_ASCII),	NTERM_WIN_MAP,  	RLE_CLASSIC,
		(0),
		20, 80, 24, 80,
		PW_MAP, "BGMAP_ASCII", "Display mini-map"
	},
	{	/* 6 */
		STREAM_PKT(BGMAP_GRAF), 	NTERM_WIN_MAP,  	RLE_LARGE,
		(0),
		20, 80, 24, 80,
		PW_MAP, "BGMAP_GRAF", ""
	},
	{	/* 7 */
		STREAM_PKT(SPECIAL_MIXED),	NTERM_WIN_SPECIAL,	RLE_CLASSIC,
		(SF_MAXBUFFER),
		20, 80, MAX_TXT_INFO, 80,
		0, "SPECIAL_MIXED", "Display special info"
	},
	{	/* 8 */
		STREAM_PKT(SPECIAL_TEXT),	NTERM_WIN_SPECIAL,	RLE_COLOR,
		(SF_MAXBUFFER),
		20, 80, MAX_TXT_INFO, 80,
		0, "SPECIAL_TEXT", ""
	},
	{	/* 9 */
		STREAM_PKT(MONSTER_TEXT),	NTERM_WIN_MONSTER,	RLE_COLOR,
		(0),
		20, 80, 22, 80,
		0, "MONSTER_TEXT", "Display monster recall"
	},
	{	/* 10 */
		STREAM_PKT(MONLIST_TEXT),	NTERM_WIN_MONLIST,	RLE_COLOR,
		(0),
		20, 80, 22, 80,
		0, "MONLIST_TEXT", "Display monster list"
	},
	{	/* 11 */
		STREAM_PKT(ITEMLIST_TEXT),	NTERM_WIN_ITEMLIST,	RLE_COLOR,
		(0),
		20, 80, 22, 80,
		0, "ITEMLIST_TEXT", "Display dungeon item list"
	},
#if 0
	{	/* 11 */
		/* Note: by re-using NTERM_WIN_SPECIAL, we seriously strain the
		 * "stream" concept. Here, it will only work because
		 * width 80 == stream7 width 80. Same width.
		 * height 255 > stream7 height 20. Larger height.
		 * So we effectively redefine the buffer to be similar, but larger, so
		 * streams 7 and 8 do not feel any ill-effects.
		 * NOTE: This will ONLY WORK if the client has a special hack
		 * for this situation! */
		STREAM_PKT(FILE_TEXT),	NTERM_WIN_SPECIAL, 	RLE_COLOR,
		(0),
		255, 80, 255, 80,
		0, "FILE_TEXT"
	},
#endif
	/* Tail */
	{	0	}
};

/*
 * Global array of "indicators"
 */
/*
	byte pkt;
	bool tiny;
	byte coffer;

	byte win;
	u16b row;
	u16b col;

	u32b flag;
	cptr prompt;
	u64b redraw;
	cptr mark;
*/
#define INDICATOR_PKT(A, T, N) PKT_INDICATOR + 1 + IN_ ## A, INDITYPE_ ## T, N
#define INDICATOR_CLONE(T, N) 0, IN_ ## T, N
const indicator_type indicators[MAX_INDICATORS] = 
{
	{
		INDICATOR_PKT(RACE, STRING, 0), 	IPW_1,	ROW_RACE,	COL_RACE,
		(0), "\aB%s",
		(PR_MISC), "race_"
	},
	{
		INDICATOR_PKT(CLASS, STRING, 0),	IPW_1,	ROW_CLASS,	COL_CLASS,
		(0), "\aB%s",
		(PR_MISC), "class_"
	},
	{
		INDICATOR_PKT(TITLE, STRING, 0),	IPW_1,	ROW_TITLE,	COL_TITLE,
		(0), "             \r\aB%s",
		(PR_TITLE), "title_"
	},
	{
		INDICATOR_PKT(LEVEL, TINY, 2),  	IPW_1,	ROW_LEVEL,	COL_LEVEL,
		(IN_STRIDE_LARGER | IN_STOP_ONCE | IN_VT_COLOR_RESET),
		"LEVEL \aG%6d\f\r\vLevel \ay%6d",
		(PR_LEV), "level"
	},
	{
		INDICATOR_PKT(EXP, LARGE, 3),   	IPW_1,	ROW_EXP,	COL_EXP,
		(IN_STRIDE_LARGER | IN_STOP_ONCE | IN_VT_COLOR_RESET), "EXP \aG%8ld\f\r\vExp \ay%8ld",
		(PR_EXP), "exp"
	},
	{
		INDICATOR_PKT(GOLD, LARGE, 1),    	IPW_1,	ROW_GOLD,	COL_GOLD,
		(0), "AU \aG%9ld",
		(PR_GOLD), "gold"
	},
#if 1
	/* Stats, classic way */
	{
		INDICATOR_PKT(STAT0, NORMAL, 3), 	IPW_1,	ROW_STAT+0,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Str:  \ay%\vSTR:  \aG%\vSTR:  \aU%",
		(PR_STATS), "stat0"
	},
	{
		INDICATOR_PKT(STAT1, NORMAL, 3), 	IPW_1,	ROW_STAT+1,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Int:  \ay%\vINT:  \aG%\vINT:  \aU%",
		(PR_STATS), "stat1"
	},
	{
		INDICATOR_PKT(STAT2, NORMAL, 3), 	IPW_1,	ROW_STAT+2,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Wis:  \ay%\vWIS:  \aG%\vWIS:  \aU%",
		(PR_STATS), "stat2"
	},
	{
		INDICATOR_PKT(STAT3, NORMAL, 3), 	IPW_1,	ROW_STAT+3,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Dex:  \ay%\vDEX:  \aG%\vDEX:  \aU%",
		(PR_STATS), "stat3"
	},
	{
		INDICATOR_PKT(STAT4, NORMAL, 3), 	IPW_1,	ROW_STAT+4,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Con:  \ay%\vCON:  \aG%\vCON:  \aU%",
		(PR_STATS), "stat4"
	},
	{
		INDICATOR_PKT(STAT5, NORMAL, 3), 	IPW_1,	ROW_STAT+5,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_COLOR_RESET | IN_VT_FF),
		"Chr:  \ay%\vCHR:  \aG%\vCHR:  \aU%",
		(PR_STATS), "stat5"
	},
#else
	/* Stats, modern way */
	{
		INDICATOR_PKT(STAT0, NORMAL, 3), 	IPW_1,	ROW_STAT+0,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"STR!  \aG%\vSTR:  \aG%\vStr:  \ay%",
		(PR_STATS), "stat0"
	},
	{
		INDICATOR_PKT(STAT1, NORMAL, 3), 	IPW_1,	ROW_STAT+1,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"INT!  \aG%\vINT:  \aG%\vInt:  \ay%",
		(PR_STATS), "stat1"
	},
	{
		INDICATOR_PKT(STAT2, NORMAL, 3), 	IPW_1,	ROW_STAT+2,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"WIS!  \aG%\vWIS:  \aG%\vWis:  \ay%",
		(PR_STATS), "stat2"
	},
	{
		INDICATOR_PKT(STAT3, NORMAL, 3), 	IPW_1,	ROW_STAT+3,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"DEX!  \aG%\vDEX:  \aG%\vDex:  \ay%",
		(PR_STATS), "stat3"
	},
	{
		INDICATOR_PKT(STAT4, NORMAL, 3), 	IPW_1,	ROW_STAT+4,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"CON!  \aG%\vCON:  \aG%\vCon:  \ay%",
		(PR_STATS), "stat4"
	},
	{
		INDICATOR_PKT(STAT5, NORMAL, 3), 	IPW_1,	ROW_STAT+5,	COL_STAT,
		(IN_STOP_ONCE | IN_TEXT_STAT | IN_STRIDE_LARGER | IN_VT_COLOR_RESET | IN_VT_FF),
		"CHR!  \aG%\vCHR:  \aG%\vChr:  \ay%",
		(PR_STATS), "stat5"
	},
#endif
	{
		INDICATOR_PKT(ARMOR, NORMAL, 3),   	IPW_1,	ROW_AC,	COL_AC,
		(0), "Cur AC \aG%5d",
		(PR_ARMOR), "armor"
	},
#if 1
	/* Classic HP/SP indicators */
	{
		INDICATOR_PKT(HP, NORMAL,	2),     IPW_1,	ROW_MAXHP,	COL_MAXHP,
		(0), "Cur HP \a@%5d\f\n\r\awMax HP \aG%5d", 
		(PR_HP), "hp"
	},
	{
		INDICATOR_PKT(SP, NORMAL,	2),     IPW_1,	ROW_MAXSP,	COL_MAXSP,
		(0), "Cur SP \a@%5d\f\n\r\awMax SP \aG%5d", 
		(PR_MANA), "sp"
	},
#else
	/* Modern (1-line) HP/SP indicators */
	{
		INDICATOR_PKT(HP, NORMAL, 2),     	IPW_1,	ROW_CURHP,	COL_CURHP,
		(0), "HP \a@% 4d\f\aw/\aG% 4d",
		(PR_HP), "hp"
	},
	{
		INDICATOR_PKT(SP, NORMAL, 2),     	IPW_1,	ROW_CURSP,	COL_CURSP,
		(0), "SP \a#% 4d\f\aw/\aG% 4d",
		(PR_MANA), "sp"
	},
#endif
	{
		INDICATOR_PKT(MON_HEALTH, TINY, 2),	IPW_1,	ROW_INFO,	COL_INFO,
		(IN_TEXT_CUT), "\a![----------]\a \r\f\t%**********", 
		(PR_HEALTH), "track"	
	},
	{
		INDICATOR_PKT(CUT, TINY, 1),    	IPW_2,	ROW_CUT, COL_CUT,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"            \v\ayGraze       \v\ayLight cut   \v\aoBad cut     \v\aoNasty cut   \v\arSevere cut  \v\arDeep gash   \v\aRMortal wound",
		(PR_CUT),  "cut"
	},
	{
		INDICATOR_PKT(FOOD, TINY, 1),   	IPW_2,	ROW_HUNGRY, COL_HUNGRY,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"\arWeak  \v\aoWeak  \v\ayHungry\v\aG      \v\aGFull  \v\agGorged",
		 (PR_HUNGER),  "hunger"	
	},
	{
		INDICATOR_PKT(BLIND, TINY, 1),   	IPW_2,	ROW_BLIND, COL_BLIND,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"     \v\aoBlind",
		(PR_BLIND),  "blind"
	},
	{
		INDICATOR_PKT(STUN, TINY, 1),   	IPW_2,	ROW_STUN, COL_STUN,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"            \v\aoStun        \v\aoHeavy stun  \v\arKnocked out ",
		(PR_STUN),  "stun"
	},
	{
		INDICATOR_PKT(CONFUSED, TINY, 1),   	IPW_2,	ROW_CONFUSED, COL_CONFUSED,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"        \v\aoConfused",
		(PR_CONFUSED),  "confused"
	},
	{
		INDICATOR_PKT(AFRAID, TINY, 1),     	IPW_2,	ROW_AFRAID, COL_AFRAID,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"      \v\aoAfraid",
		(PR_AFRAID),  "afraid"
	},
	{
		INDICATOR_PKT(POISONED, TINY, 1),   	IPW_2,	ROW_POISONED, COL_POISONED,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"        \v\aoPoisoned",
		(PR_POISONED),  "poisoned"
	},
	{
		INDICATOR_PKT(STATE, TINY, 3),      	IPW_2,	ROW_STATE,	COL_STATE,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_EMPTY | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"%\v\arParalyzed!\f%\v\awSearching  \v\aDStlth Mode\f\aw          \v\awResting   ",
		(PR_STATE),  "state"
	},
	{
		INDICATOR_PKT(SPEED, NORMAL, 1),  	IPW_2,	ROW_SPEED,	COL_SPEED,
		(IN_STOP_ONCE | IN_STOP_EMPTY | IN_STRIDE_POSITIVE | IN_VT_STRIDE_FLIP | IN_VT_CR),
		"\v             \v\aUSlow ( \b%d)\v\aGFast (+ \b%d)",
		(PR_SPEED), "speed"	
	},
	{
		INDICATOR_PKT(STUDY, TINY, 1),   	IPW_2,	ROW_STUDY, COL_STUDY,
		(IN_STOP_ONCE | IN_TEXT_LABEL | IN_STRIDE_POSITIVE | IN_VT_DEC_VALUE),
		"     \vStudy",
		(PR_STUDY),  "study"
	},
	{
		INDICATOR_PKT(DEPTH, TINY, 1),   	IPW_2,	ROW_DEPTH, COL_DEPTH,
		(0),
		"Lev \aw%3d",
		(PR_DEPTH),  "depth"
	},
	{
		INDICATOR_PKT(OPPOSE, TINY, 5), 	IPW_2,	ROW_OPPOSE_ELEMENTS,	COL_OPPOSE_ELEMENTS,
		(IN_AUTO_CUT | IN_TEXT_LABEL | IN_STRIDE_NONZERO | IN_VT_DEC_VALUE),
		"     \v\asAcid \f     \v\abElec \f     \v\arFire \f     \v\awCold \f     \v\agPois ",
		(PR_OPPOSE_ELEMENTS), "oppose"
	},
	/** Character sheet **/
	{
		INDICATOR_PKT(VARIOUS, NORMAL, 4),   	IPW_3 | IPW_4,  	2,	32,
		(IN_VT_CR | IN_VT_LF | IN_VT_FF | IN_VT_COLOR_RESET),
		"Age             \aB%6ld\vHeight          \aB%6ld\vWeight          \aB%6ld\vSocial Class    \aB%6ld",
		(PR_VARIOUS), "various"
	},
	{
		INDICATOR_PKT(SKILLS, NORMAL, 16),   	IPW_3,  	16,	1,
		(IN_TEXT_LIKERT | IN_VT_COLOR_RESET | IN_VT_FF),
		"Fighting    :\t%\t\t\t\t\t\t\t\t\t\t\t\t\v\fPerception  :\t%\r\n\v\fBows/Throw  :\t%\t\t\t\t\t\t\t\t\t\t\t\t\v\fSearching   :\t%\r\n\v\fSaving Throw:\t%\t\t\t\t\t\t\t\t\t\t\t\t\v\fDisarming   :\t%\r\n\v\fStealth     :\t%\t\t\t\t\t\t\t\t\t\t\t\t\v\fMagic Device:\t%",
		(PR_SKILLS), "skills"
	},
	{
		INDICATOR_PKT(SKILLS2, NORMAL, 3),   	IPW_3,  	16,	55,
		(IN_VT_COLOR_RESET | IN_VT_FF | IN_VT_CR | IN_VT_LF),
		"Blows/Round:\t\t%d\vShots/Round:\t\t%d\v\nInfra-Vision:\t%d feet",
		(PR_SKILLS), "skills2"
	},
	{
		INDICATOR_PKT(PLUSSES, NORMAL, 2),   	IPW_3 | IPW_4,  	9,	1,
		(IN_VT_COLOR_RESET | IN_VT_CR | IN_VT_LF | IN_VT_FF),
		"+ To Hit    \t\t\t\aB%6ld\v+ To Damage \t\t\t\aB%6ld", 
		(PR_PLUSSES), "plusses"
	},
	/* Those 4 indicators should be merged into one (once code allows it) */
	{
		INDICATOR_PKT(HISTORY0, STRING, 0), 	IPW_4,	16+0,	10,
		(0), "%s",
		(0), "history0_"
	},
	{
		INDICATOR_PKT(HISTORY1, STRING, 0), 	IPW_4,	16+1,	10,
		(0), "%s",
		(0), "history1_"
	},
	{
		INDICATOR_PKT(HISTORY2, STRING, 0), 	IPW_4,	16+2,	10,
		(0), "%s",
		(0), "history2_"
	},
	{
		INDICATOR_PKT(HISTORY3, STRING, 0), 	IPW_4,	16+3,	10,
		(0), "%s",
		(0), "history3_"
	},
	/* Name and gender indicators */
	{
		INDICATOR_PKT(NAME, STRING, 0), 	IPW_3 | IPW_4 | IPW_5,	2,	1,
		(0), "Name        : \aB%s",
		(0), "hist_name_"
	},
	{
		INDICATOR_PKT(GENDER, STRING, 0), 	IPW_3 | IPW_4 | IPW_5,	3,	1,
		(0), "Sex         : \aB%s",
		(0), "hist_gender_"
	},

	/** Clones **/
	{
		INDICATOR_CLONE(RACE, 0), 	IPW_3 | IPW_4 | IPW_5,	4,	1,
		(0), "Race        : \aB%s",
		(PR_MISC), "hist_race_"
	},
	{
		INDICATOR_CLONE(CLASS, 0), 	IPW_3 | IPW_4 | IPW_5,	5,	1,
		(0), "Class       : \aB%s",
		(PR_MISC), "hist_class_"
	},
	{
		INDICATOR_CLONE(HP, 2),   	IPW_3 | IPW_4,  	9,	52,
		(0),
		"\vMax Hit Points  	 \aG%6ld", 
		(PR_HP), "hist_mhp"
	},
 	{
		INDICATOR_CLONE(HP, 1),   	IPW_3 | IPW_4,     10,	52,
		(IN_VT_FF),
		"Cur Hit Points    \a@%6ld",
		(PR_HP), "hist_chp"
	},
	{
		INDICATOR_CLONE(SP, 2),   	IPW_3 | IPW_4,     11,	52,
		(0),
		"\vMax SP (Mana)     \aG%6ld",
		(PR_HP), "hist_msp"
	},
	{
		INDICATOR_CLONE(SP, 1),   	IPW_3 | IPW_4,     12,	52,
		(IN_VT_FF),
		"Cur SP (Mana)     \a#%6ld",
		(PR_HP), "hist_csp"
	},
	{
		INDICATOR_CLONE(ARMOR, 3),   IPW_3 | IPW_4, 	11,	1,
		(0),
		"+ To AC        \aB%6ld",
		(PR_ARMOR), "hist_toac"
	},
	{
		INDICATOR_CLONE(ARMOR, 2),   IPW_3 | IPW_4, 	12,	1,
		(0),
		"  Base AC      \aB%6ld",
		(PR_ARMOR), "hist_baseac"
	},
	{
		INDICATOR_CLONE(LEVEL, 1), IPW_3 | IPW_4,	9,	28,
		(0),
		"Level      \aG%9ld",
		(PR_LEV), "hist_level"
	},
	{
		INDICATOR_CLONE(EXP, 1),   IPW_3 | IPW_4, 	10,	28,
		(IN_STRIDE_LARGER | IN_STOP_ONCE | IN_VT_COLOR_RESET),
		"Experience    \aG%6ld\f\r\vExperience    \ay%6ld",
		(PR_EXP), "hist_cexp"
	},
	{
		INDICATOR_CLONE(EXP, 1),   IPW_3 | IPW_4, 	11,	28,
		(0),
		"Max Exp       \aG%6ld",
		(PR_EXP), "hist_mexp"
	},
	{
		INDICATOR_CLONE(EXP, 3),   IPW_3 | IPW_4, 	12,	28,
		(0),
		"Exp to Adv.  \aG%7ld",
		(PR_EXP), "hist_aexp"
	},
	{
		INDICATOR_CLONE(GOLD, 1),   IPW_3 | IPW_4, 	13,	28,
		(0),
		"Gold       \aG%9ld",
		(PR_GOLD), "hist_gold"
	},
	{
		INDICATOR_CLONE(GOLD, 1),   IPW_6, 	19,	53,
		(0),
		"Gold Remaining:\aG%9ld",
		(PR_GOLD), "store_gold"
	},
	/* alternative way to create verbose stat indicators
 	{
		INDICATOR_CLONE(STAT0, 1),   	IPW_3,     2+0,	61,
		(IN_STOP_STRIDE | IN_TEXT_PRINTF | (IN_STRIDE_LESSER) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\vStr: \ay%d\f or \a;%d\v \v\f"
		"\rSTR: \a;%d\v\f (of \b%d)",
		(PR_STATS), "hist_stat0"
	}, */
	/* Stats, Verbose. Displays both injured and uninjured values. */
	{
		/* STR, Verbose */
		INDICATOR_CLONE(STAT0, 1),   	IPW_3 | IPW_4 | IPW_5,     2+0,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fSTR: \a;%\v \v\rStr: \ay%\f \a;%",
		(PR_STATS), "hist_stat0"
	},
	{
		/* INT, Verbose */
		INDICATOR_CLONE(STAT1, 1),   	IPW_3 | IPW_4 | IPW_5,     2+1,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fINT: \a;%\v \v\rInt: \ay%\f \a;%",
		(PR_STATS), "hist_stat1"
	},
	{
		/* WIS, Verbose */
		INDICATOR_CLONE(STAT2, 1),   	IPW_3 | IPW_4 | IPW_5,     2+2,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fWIS: \a;%\v \v\rWis: \ay%\f \a;%",
		(PR_STATS), "hist_stat2"
	},
	{
		/* DEX, Verbose */
		INDICATOR_CLONE(STAT3, 1),   	IPW_3 | IPW_4 | IPW_5,     2+3,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fDEX: \a;%\v \v\rDex: \ay%\f \a;%",
		(PR_STATS), "hist_stat3"
	},
	{
		/* CON, Verbose */
		INDICATOR_CLONE(STAT4, 1),   	IPW_3 | IPW_4 | IPW_5,     2+4,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fCON: \a;%\v \v\rCon: \ay%\f \a;%",
		(PR_STATS), "hist_stat4"
	},
	{
		/* CHR, Verbose */
		INDICATOR_CLONE(STAT5, 1),   	IPW_3 | IPW_4 | IPW_5,     2+5,	61,
		(IN_STOP_STRIDE | IN_TEXT_STAT | (IN_STRIDE_LESSER | IN_STRIDE_NOT) | IN_VT_STRIDE_FLIP | IN_VT_COLOR_RESET | IN_VT_COFFER_RESET),
		"\v\fCHR: \a;%\v \v\rChr: \ay%\f \a;%",
		(PR_STATS), "hist_stat5"
	},

	/* Tail */
	{	0	}
};

/*
 * Global array of equipment slot X/Y positions for advanced
 * interfaces. 0 is 0.0f, 128 is 0.5f, 255 is 1.0f.
 */
#define __F(X) MAX(MIN(127 + X * 64, 255), 0)
byte eq_pos[INVEN_TOTAL - INVEN_WIELD + 1][2] = {
		{ __F(-2), __F( 0) },	/* INVEN_WIELD */
		{ __F(-2), __F( 1) },	/* INVEN_BOW */
		{ __F( 1), __F( 1) },	/* INVEN_LEFT */
		{ __F(-1), __F( 1) },	/* INVEN_RIGHT */
		{ __F( 0), __F(-1) },	/* INVEN_NECK */
		{ __F( 2), __F( 0) },	/* INVEN_LITE */
		{ __F( 0), __F( 0) },	/* INVEN_BODY */
		{ __F( 1), __F(-1) },	/* INVEN_OUTER */
		{ __F( 1), __F( 0) },	/* INVEN_ARM */
		{ __F( 0), __F(-2) },	/* INVEN_HEAD */
		{ __F(-1), __F( 0) },	/* INVEN_HANDS */
		{ __F( 0), __F( 2) },	/* INVEN_FEET */
};
#undef __F

/*
 * Global array for looping through the "keypad directions"
 */
s16b ddd[9] =
{ 2, 8, 6, 4, 3, 1, 9, 7, 5 };

/*
 * Global arrays for converting "keypad direction" into offsets
 */
s16b ddx[10] =
{ 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };

s16b ddy[10] =
{ 0, 1, 1, 1, 0, 0, 0, -1, -1, -1 };

/*
 * Global arrays for optimizing "ddx[ddd[i]]" and "ddy[ddd[i]]"
 */
s16b ddx_ddd[9] =
{ 0, 0, 1, -1, 1, -1, 1, -1, 0 };

s16b ddy_ddd[9] =
{ 1, -1, 0, 0, 1, 1, -1, -1, 0 };



/*
 * Global array for converting numbers to uppercase hecidecimal digit
 * This array can also be used to convert a number to an octal digit
 */
char hexsym[16] =
{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


/*
 * Stat Table (INT/WIS) -- Number of half-spells per level
 */
const int adj_mag_study[] =
{
	  0	/* 3 */,
	  0	/* 4 */,
	 10	/* 5 */,
	 20	/* 6 */,
	 30	/* 7 */,
	 40	/* 8 */,
	 50	/* 9 */,
	 60	/* 10 */,
	 70	/* 11 */,
	 80	/* 12 */,
	 85	/* 13 */,
	 90	/* 14 */,
	 95	/* 15 */,
	100	/* 16 */,
	105	/* 17 */,
	110	/* 18/00-18/09 */,
	115	/* 18/10-18/19 */,
	120	/* 18/20-18/29 */,
	130	/* 18/30-18/39 */,
	140	/* 18/40-18/49 */,
	150	/* 18/50-18/59 */,
	160	/* 18/60-18/69 */,
	170	/* 18/70-18/79 */,
	180	/* 18/80-18/89 */,
	190	/* 18/90-18/99 */,
	200	/* 18/100-18/109 */,
	210	/* 18/110-18/119 */,
	220	/* 18/120-18/129 */,
	230	/* 18/130-18/139 */,
	240	/* 18/140-18/149 */,
	250	/* 18/150-18/159 */,
	250	/* 18/160-18/169 */,
	250	/* 18/170-18/179 */,
	250	/* 18/180-18/189 */,
	250	/* 18/190-18/199 */,
	250	/* 18/200-18/209 */,
	250	/* 18/210-18/219 */,
	250	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- extra half-mana-points per level
 */
const int adj_mag_mana[] =
{
	  0	/* 3 */,
	 10	/* 4 */,
	 20	/* 5 */,
	 30	/* 6 */,
	 40	/* 7 */,
	 50	/* 8 */,
	 60	/* 9 */,
	 70	/* 10 */,
	 80	/* 11 */,
	 90	/* 12 */,
	100	/* 13 */,
	110	/* 14 */,
	120	/* 15 */,
	130	/* 16 */,
	140	/* 17 */,
	150	/* 18/00-18/09 */,
	160	/* 18/10-18/19 */,
	170	/* 18/20-18/29 */,
	180	/* 18/30-18/39 */,
	190	/* 18/40-18/49 */,
	200	/* 18/50-18/59 */,
	225	/* 18/60-18/69 */,
	250	/* 18/70-18/79 */,
	300	/* 18/80-18/89 */,
	350	/* 18/90-18/99 */,
	400	/* 18/100-18/109 */,
	450	/* 18/110-18/119 */,
	500	/* 18/120-18/129 */,
	550	/* 18/130-18/139 */,
	600	/* 18/140-18/149 */,
	650	/* 18/150-18/159 */,
	700	/* 18/160-18/169 */,
	750	/* 18/170-18/179 */,
	800	/* 18/180-18/189 */,
	800	/* 18/190-18/199 */,
	800	/* 18/200-18/209 */,
	800	/* 18/210-18/219 */,
	800	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
byte adj_mag_fail[] =
{
	99	/* 3 */,
	99	/* 4 */,
	99	/* 5 */,
	99	/* 6 */,
	99	/* 7 */,
	50	/* 8 */,
	30	/* 9 */,
	20	/* 10 */,
	15	/* 11 */,
	12	/* 12 */,
	11	/* 13 */,
	10	/* 14 */,
	9	/* 15 */,
	8	/* 16 */,
	7	/* 17 */,
	6	/* 18/00-18/09 */,
	6	/* 18/10-18/19 */,
	5	/* 18/20-18/29 */,
	5	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	4	/* 18/70-18/79 */,
	4	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	3	/* 18/100-18/109 */,
	2	/* 18/110-18/119 */,
	2	/* 18/120-18/129 */,
	2	/* 18/130-18/139 */,
	2	/* 18/140-18/149 */,
	1	/* 18/150-18/159 */,
	1	/* 18/160-18/169 */,
	1	/* 18/170-18/179 */,
	1	/* 18/180-18/189 */,
	1	/* 18/190-18/199 */,
	0	/* 18/200-18/209 */,
	0	/* 18/210-18/219 */,
	0	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- failure rate adjustment
 */
const int adj_mag_stat[] =
{
	-5	/* 3 */,
	-4	/* 4 */,
	-3	/* 5 */,
	-3	/* 6 */,
	-2	/* 7 */,
	-1	/* 8 */,
	 0	/* 9 */,
	 0	/* 10 */,
	 0	/* 11 */,
	 0	/* 12 */,
	 0	/* 13 */,
	 1	/* 14 */,
	 2	/* 15 */,
	 3	/* 16 */,
	 4	/* 17 */,
	 5	/* 18/00-18/09 */,
	 6	/* 18/10-18/19 */,
	 7	/* 18/20-18/29 */,
	 8	/* 18/30-18/39 */,
	 9	/* 18/40-18/49 */,
	10	/* 18/50-18/59 */,
	11	/* 18/60-18/69 */,
	12	/* 18/70-18/79 */,
	15	/* 18/80-18/89 */,
	18	/* 18/90-18/99 */,
	21	/* 18/100-18/109 */,
	24	/* 18/110-18/119 */,
	27	/* 18/120-18/129 */,
	30	/* 18/130-18/139 */,
	33	/* 18/140-18/149 */,
	36	/* 18/150-18/159 */,
	39	/* 18/160-18/169 */,
	42	/* 18/170-18/179 */,
	45	/* 18/180-18/189 */,
	48	/* 18/190-18/199 */,
	51	/* 18/200-18/209 */,
	54	/* 18/210-18/219 */,
	57	/* 18/220+ */
};


/*
 * Stat Table (CHR) -- payment percentages
 */
byte adj_chr_gold[] =
{
	130	/* 3 */,
	125	/* 4 */,
	122	/* 5 */,
	120	/* 6 */,
	118	/* 7 */,
	116	/* 8 */,
	114	/* 9 */,
	112	/* 10 */,
	110	/* 11 */,
	108	/* 12 */,
	106	/* 13 */,
	104	/* 14 */,
	103	/* 15 */,
	102	/* 16 */,
	101	/* 17 */,
	100	/* 18/00-18/09 */,
	99	/* 18/10-18/19 */,
	98	/* 18/20-18/29 */,
	97	/* 18/30-18/39 */,
	96	/* 18/40-18/49 */,
	95	/* 18/50-18/59 */,
	94	/* 18/60-18/69 */,
	93	/* 18/70-18/79 */,
	92	/* 18/80-18/89 */,
	91	/* 18/90-18/99 */,
	90	/* 18/100-18/109 */,
	89	/* 18/110-18/119 */,
	88	/* 18/120-18/129 */,
	87	/* 18/130-18/139 */,
	86	/* 18/140-18/149 */,
	85	/* 18/150-18/159 */,
	84	/* 18/160-18/169 */,
	83	/* 18/170-18/179 */,
	82	/* 18/180-18/189 */,
	81	/* 18/190-18/199 */,
	80	/* 18/200-18/209 */,
	80	/* 18/210-18/219 */,
	80	/* 18/220+ */
};


/*
 * Stat Table (INT) -- Magic devices
 */
byte adj_int_dev[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	4	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	6	/* 18/60-18/69 */,
	6	/* 18/70-18/79 */,
	7	/* 18/80-18/89 */,
	7	/* 18/90-18/99 */,
	8	/* 18/100-18/109 */,
	9	/* 18/110-18/119 */,
	10	/* 18/120-18/129 */,
	11	/* 18/130-18/139 */,
	12	/* 18/140-18/149 */,
	13	/* 18/150-18/159 */,
	14	/* 18/160-18/169 */,
	15	/* 18/170-18/179 */,
	16	/* 18/180-18/189 */,
	17	/* 18/190-18/199 */,
	18	/* 18/200-18/209 */,
	19	/* 18/210-18/219 */,
	20	/* 18/220+ */
};


/*
 * Stat Table (WIS) -- Saving throw
 */
byte adj_wis_sav[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	3	/* 18/30-18/39 */,
	3	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	5	/* 18/70-18/79 */,
	5	/* 18/80-18/89 */,
	6	/* 18/90-18/99 */,
	7	/* 18/100-18/109 */,
	8	/* 18/110-18/119 */,
	9	/* 18/120-18/129 */,
	10	/* 18/130-18/139 */,
	11	/* 18/140-18/149 */,
	12	/* 18/150-18/159 */,
	13	/* 18/160-18/169 */,
	14	/* 18/170-18/179 */,
	15	/* 18/180-18/189 */,
	16	/* 18/190-18/199 */,
	17	/* 18/200-18/209 */,
	18	/* 18/210-18/219 */,
	19	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- disarming
 */
byte adj_dex_dis[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	4	/* 18/00-18/09 */,
	4	/* 18/10-18/19 */,
	4	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	5	/* 18/60-18/69 */,
	6	/* 18/70-18/79 */,
	6	/* 18/80-18/89 */,
	7	/* 18/90-18/99 */,
	8	/* 18/100-18/109 */,
	8	/* 18/110-18/119 */,
	8	/* 18/120-18/129 */,
	8	/* 18/130-18/139 */,
	8	/* 18/140-18/149 */,
	9	/* 18/150-18/159 */,
	9	/* 18/160-18/169 */,
	9	/* 18/170-18/179 */,
	9	/* 18/180-18/189 */,
	9	/* 18/190-18/199 */,
	10	/* 18/200-18/209 */,
	10	/* 18/210-18/219 */,
	10	/* 18/220+ */
};


/*
 * Stat Table (INT) -- disarming
 */
byte adj_int_dis[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	1	/* 8 */,
	1	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	2	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	3	/* 18/00-18/09 */,
	3	/* 18/10-18/19 */,
	3	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	4	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	6	/* 18/60-18/69 */,
	7	/* 18/70-18/79 */,
	8	/* 18/80-18/89 */,
	9	/* 18/90-18/99 */,
	10	/* 18/100-18/109 */,
	10	/* 18/110-18/119 */,
	11	/* 18/120-18/129 */,
	12	/* 18/130-18/139 */,
	13	/* 18/140-18/149 */,
	14	/* 18/150-18/159 */,
	15	/* 18/160-18/169 */,
	16	/* 18/170-18/179 */,
	17	/* 18/180-18/189 */,
	18	/* 18/190-18/199 */,
	19	/* 18/200-18/209 */,
	19	/* 18/210-18/219 */,
	19	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to ac (plus 128)
 */
byte adj_dex_ta[] =
{
	128 + -4	/* 3 */,
	128 + -3	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 1	/* 15 */,
	128 + 1	/* 16 */,
	128 + 1	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 2	/* 18/20-18/29 */,
	128 + 2	/* 18/30-18/39 */,
	128 + 2	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 3	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to dam (plus 128)
 */
byte adj_str_td[] =
{
	128 + -2	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 5	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 10	/* 18/140-18/149 */,
	128 + 11	/* 18/150-18/159 */,
	128 + 12	/* 18/160-18/169 */,
	128 + 13	/* 18/170-18/179 */,
	128 + 14	/* 18/180-18/189 */,
	128 + 15	/* 18/190-18/199 */,
	128 + 16	/* 18/200-18/209 */,
	128 + 18	/* 18/210-18/219 */,
	128 + 20	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to hit (plus 128)
 */
byte adj_dex_th[] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + -1	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 3	/* 18/00-18/09 */,
	128 + 3	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 4	/* 18/50-18/59 */,
	128 + 4	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to hit (plus 128)
 */
byte adj_str_th[] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 0	/* 16 */,
	128 + 0	/* 17 */,
	128 + 1	/* 18/00-18/09 */,
	128 + 1	/* 18/10-18/19 */,
	128 + 1	/* 18/20-18/29 */,
	128 + 1	/* 18/30-18/39 */,
	128 + 1	/* 18/40-18/49 */,
	128 + 1	/* 18/50-18/59 */,
	128 + 1	/* 18/60-18/69 */,
	128 + 2	/* 18/70-18/79 */,
	128 + 3	/* 18/80-18/89 */,
	128 + 4	/* 18/90-18/99 */,
	128 + 5	/* 18/100-18/109 */,
	128 + 6	/* 18/110-18/119 */,
	128 + 7	/* 18/120-18/129 */,
	128 + 8	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- weight limit in deca-pounds
 */
byte adj_str_wgt[] =
{
	5	/* 3 */,
	6	/* 4 */,
	7	/* 5 */,
	8	/* 6 */,
	9	/* 7 */,
	10	/* 8 */,
	11	/* 9 */,
	12	/* 10 */,
	13	/* 11 */,
	14	/* 12 */,
	15	/* 13 */,
	16	/* 14 */,
	17	/* 15 */,
	18	/* 16 */,
	19	/* 17 */,
	20	/* 18/00-18/09 */,
	22	/* 18/10-18/19 */,
	24	/* 18/20-18/29 */,
	26	/* 18/30-18/39 */,
	28	/* 18/40-18/49 */,
	30	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	30	/* 18/70-18/79 */,
	30	/* 18/80-18/89 */,
	30	/* 18/90-18/99 */,
	30	/* 18/100-18/109 */,
	30	/* 18/110-18/119 */,
	30	/* 18/120-18/129 */,
	30	/* 18/130-18/139 */,
	30	/* 18/140-18/149 */,
	30	/* 18/150-18/159 */,
	30	/* 18/160-18/169 */,
	30	/* 18/170-18/179 */,
	30	/* 18/180-18/189 */,
	30	/* 18/190-18/199 */,
	30	/* 18/200-18/209 */,
	30	/* 18/210-18/219 */,
	30	/* 18/220+ */
};


/*
 * Stat Table (STR) -- weapon weight limit in pounds
 */
byte adj_str_hold[] =
{
	4	/* 3 */,
	5	/* 4 */,
	6	/* 5 */,
	7	/* 6 */,
	8	/* 7 */,
	10	/* 8 */,
	12	/* 9 */,
	14	/* 10 */,
	16	/* 11 */,
	18	/* 12 */,
	20	/* 13 */,
	22	/* 14 */,
	24	/* 15 */,
	26	/* 16 */,
	28	/* 17 */,
	30	/* 18/00-18/09 */,
	30	/* 18/10-18/19 */,
	35	/* 18/20-18/29 */,
	40	/* 18/30-18/39 */,
	45	/* 18/40-18/49 */,
	50	/* 18/50-18/59 */,
	55	/* 18/60-18/69 */,
	60	/* 18/70-18/79 */,
	65	/* 18/80-18/89 */,
	70	/* 18/90-18/99 */,
	80	/* 18/100-18/109 */,
	80	/* 18/110-18/119 */,
	80	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	80	/* 18/140-18/149 */,
	90	/* 18/150-18/159 */,
	90	/* 18/160-18/169 */,
	90	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	90	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (STR) -- digging value
 */
byte adj_str_dig[] =
{
	0	/* 3 */,
	0	/* 4 */,
	1	/* 5 */,
	2	/* 6 */,
	3	/* 7 */,
	4	/* 8 */,
	4	/* 9 */,
	5	/* 10 */,
	5	/* 11 */,
	6	/* 12 */,
	6	/* 13 */,
	7	/* 14 */,
	7	/* 15 */,
	8	/* 16 */,
	8	/* 17 */,
	9	/* 18/00-18/09 */,
	10	/* 18/10-18/19 */,
	12	/* 18/20-18/29 */,
	15	/* 18/30-18/39 */,
	20	/* 18/40-18/49 */,
	25	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	35	/* 18/70-18/79 */,
	40	/* 18/80-18/89 */,
	45	/* 18/90-18/99 */,
	50	/* 18/100-18/109 */,
	55	/* 18/110-18/119 */,
	60	/* 18/120-18/129 */,
	65	/* 18/130-18/139 */,
	70	/* 18/140-18/149 */,
	75	/* 18/150-18/159 */,
	80	/* 18/160-18/169 */,
	85	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	95	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (STR) -- help index into the "blow" table
 */
byte adj_str_blow[] =
{
	3	/* 3 */,
	4	/* 4 */,
	5	/* 5 */,
	6	/* 6 */,
	7	/* 7 */,
	8	/* 8 */,
	9	/* 9 */,
	10	/* 10 */,
	11	/* 11 */,
	12	/* 12 */,
	13	/* 13 */,
	14	/* 14 */,
	15	/* 15 */,
	16	/* 16 */,
	17	/* 17 */,
	20 /* 18/00-18/09 */,
	30 /* 18/10-18/19 */,
	40 /* 18/20-18/29 */,
	50 /* 18/30-18/39 */,
	60 /* 18/40-18/49 */,
	70 /* 18/50-18/59 */,
	80 /* 18/60-18/69 */,
	90 /* 18/70-18/79 */,
	100 /* 18/80-18/89 */,
	110 /* 18/90-18/99 */,
	120 /* 18/100-18/109 */,
	130 /* 18/110-18/119 */,
	140 /* 18/120-18/129 */,
	150 /* 18/130-18/139 */,
	160 /* 18/140-18/149 */,
	170 /* 18/150-18/159 */,
	180 /* 18/160-18/169 */,
	190 /* 18/170-18/179 */,
	200 /* 18/180-18/189 */,
	210 /* 18/190-18/199 */,
	220 /* 18/200-18/209 */,
	230 /* 18/210-18/219 */,
	240 /* 18/220+ */
};


/*
 * Stat Table (DEX) -- index into the "blow" table
 */
byte adj_dex_blow[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	1	/* 10 */,
	1	/* 11 */,
	1	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	1	/* 16 */,
	1	/* 17 */,
	1	/* 18/00-18/09 */,
	2	/* 18/10-18/19 */,
	2	/* 18/20-18/29 */,
	2	/* 18/30-18/39 */,
	2	/* 18/40-18/49 */,
	3	/* 18/50-18/59 */,
	3	/* 18/60-18/69 */,
	4	/* 18/70-18/79 */,
	4	/* 18/80-18/89 */,
	5	/* 18/90-18/99 */,
	6	/* 18/100-18/109 */,
	7	/* 18/110-18/119 */,
	8	/* 18/120-18/129 */,
	9	/* 18/130-18/139 */,
	10	/* 18/140-18/149 */,
	11	/* 18/150-18/159 */,
	12	/* 18/160-18/169 */,
	14	/* 18/170-18/179 */,
	16	/* 18/180-18/189 */,
	18	/* 18/190-18/199 */,
	20	/* 18/200-18/209 */,
	20	/* 18/210-18/219 */,
	20	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- chance of avoiding "theft" and "falling"
 */
byte adj_dex_safe[] =
{
	0	/* 3 */,
	1	/* 4 */,
	2	/* 5 */,
	3	/* 6 */,
	4	/* 7 */,
	5	/* 8 */,
	5	/* 9 */,
	6	/* 10 */,
	6	/* 11 */,
	7	/* 12 */,
	7	/* 13 */,
	8	/* 14 */,
	8	/* 15 */,
	9	/* 16 */,
	9	/* 17 */,
	10	/* 18/00-18/09 */,
	10	/* 18/10-18/19 */,
	15	/* 18/20-18/29 */,
	15	/* 18/30-18/39 */,
	20	/* 18/40-18/49 */,
	25	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	35	/* 18/70-18/79 */,
	40	/* 18/80-18/89 */,
	45	/* 18/90-18/99 */,
	50	/* 18/100-18/109 */,
	60	/* 18/110-18/119 */,
	70	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	90	/* 18/140-18/149 */,
	100	/* 18/150-18/159 */,
	100	/* 18/160-18/169 */,
	100	/* 18/170-18/179 */,
	100	/* 18/180-18/189 */,
	100	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (CON) -- base regeneration rate
 */
byte adj_con_fix[] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	0	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	1	/* 16 */,
	1	/* 17 */,
	2	/* 18/00-18/09 */,
	2	/* 18/10-18/19 */,
	2	/* 18/20-18/29 */,
	2	/* 18/30-18/39 */,
	2	/* 18/40-18/49 */,
	3	/* 18/50-18/59 */,
	3	/* 18/60-18/69 */,
	3	/* 18/70-18/79 */,
	3	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	4	/* 18/100-18/109 */,
	4	/* 18/110-18/119 */,
	5	/* 18/120-18/129 */,
	6	/* 18/130-18/139 */,
	6	/* 18/140-18/149 */,
	7	/* 18/150-18/159 */,
	7	/* 18/160-18/169 */,
	8	/* 18/170-18/179 */,
	8	/* 18/180-18/189 */,
	8	/* 18/190-18/199 */,
	9	/* 18/200-18/209 */,
	9	/* 18/210-18/219 */,
	9	/* 18/220+ */
};


/*
 * Stat Table (CON) -- extra 1/100th hitpoints per level
 */
const int adj_con_mhp[] =
{
	-250	/* 3 */,
	-150	/* 4 */,
	-100	/* 5 */,
	 -75	/* 6 */,
	 -50	/* 7 */,
	 -25	/* 8 */,
	 -10	/* 9 */,
	  -5	/* 10 */,
	   0	/* 11 */,
	   5	/* 12 */,
	  10	/* 13 */,
	  25	/* 14 */,
	  50	/* 15 */,
	  75	/* 16 */,
	 100	/* 17 */,
	 150	/* 18/00-18/09 */,
	 175	/* 18/10-18/19 */,
	 200	/* 18/20-18/29 */,
	 225	/* 18/30-18/39 */,
	 250	/* 18/40-18/49 */,
	 275	/* 18/50-18/59 */,
	 300	/* 18/60-18/69 */,
	 350	/* 18/70-18/79 */,
	 400	/* 18/80-18/89 */,
	 450	/* 18/90-18/99 */,
	 500	/* 18/100-18/109 */,
	 550	/* 18/110-18/119 */,
	 600	/* 18/120-18/129 */,
	 650	/* 18/130-18/139 */,
	 700	/* 18/140-18/149 */,
	 750	/* 18/150-18/159 */,
	 800	/* 18/160-18/169 */,
	 900	/* 18/170-18/179 */,
	1000	/* 18/180-18/189 */,
	1100	/* 18/190-18/199 */,
	1250	/* 18/200-18/209 */,
	1250	/* 18/210-18/219 */,
	1250	/* 18/220+ */
};


/*
 * This table is used to help calculate the number of blows the player can
 * make in a single round of attacks (one player turn) with a normal weapon.
 *
 * This number ranges from a single blow/round for weak players to up to six
 * blows/round for powerful warriors.
 *
 * Note that certain artifacts and ego-items give "bonus" blows/round.
 *
 * First, from the player class, we extract some values:
 *
 *    Warrior --> num = 6; mul = 5; div = MAX(30, weapon_weight);
 *    Mage    --> num = 4; mul = 2; div = MAX(40, weapon_weight);
 *    Priest  --> num = 5; mul = 3; div = MAX(35, weapon_weight);
 *    Rogue   --> num = 5; mul = 3; div = MAX(30, weapon_weight);
 *    Ranger  --> num = 5; mul = 4; div = MAX(35, weapon_weight);
 *    Paladin --> num = 5; mul = 4; div = MAX(30, weapon_weight);
 *
 * To get "P", we look up the relevant "adj_str_blow[]" (see above),
 * multiply it by "mul", and then divide it by "div", rounding down.
 *
 * To get "D", we look up the relevant "adj_dex_blow[]" (see above),
 * note especially column 6 (DEX 18/101) and 11 (DEX 18/150).
 *
 * The player gets "blows_table[P][D]" blows/round, as shown below,
 * up to a maximum of "num" blows/round, plus any "bonus" blows/round.
 */
byte blows_table[12][12] =
{
	/* P/D */
	/* 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11+ */

	/* 0  */
	{  1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   3 },

	/* 1  */
	{  1,   1,   1,   1,   2,   2,   3,   3,   3,   4,   4,   4 },

	/* 2  */
	{  1,   1,   2,   2,   3,   3,   4,   4,   4,   5,   5,   5 },

	/* 3  */
	{  1,   2,   2,   3,   3,   4,   4,   4,   5,   5,   5,   5 },

	/* 4  */
	{  1,   2,   2,   3,   3,   4,   4,   5,   5,   5,   5,   5 },

	/* 5  */
	{  2,   2,   3,   3,   4,   4,   5,   5,   5,   5,   5,   6 },

	/* 6  */
	{  2,   2,   3,   3,   4,   4,   5,   5,   5,   5,   5,   6 },

	/* 7  */
	{  2,   3,   3,   4,   4,   4,   5,   5,   5,   5,   5,   6 },

	/* 8  */
	{  3,   3,   3,   4,   4,   4,   5,   5,   5,   5,   6,   6 },

	/* 9  */
	{  3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6 },

	/* 10 */
	{  3,   3,   4,   4,   4,   4,   5,   5,   5,   6,   6,   6 },

	/* 11+ */
	{  3,   3,   4,   4,   4,   4,   5,   5,   6,   6,   6,   6 },
};


#if 0

/*
 * This is the "old" table used to calculate multiple blows.
 *
 * Note that this table used a different indexing scheme to determine "P"
 */

byte old_blows_table[11][12] =
{
	/* P/D */
	/* 3,  10, /01, /50, /90,/100,/101,/110,/120,/130,/140,/150 */

	/* 0+ */
	{  1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   3},

	/* 2+ */
	{  1,   1,   1,   1,   2,   2,   3,   3,   3,   3,   3,   4},

	/* 3+ */
	{  1,   1,   1,   2,   2,   3,   4,   4,   4,   4,   4,   5},

	/* 4+ */
	{  1,   1,   2,   2,   3,   3,   4,   4,   4,   5,   5,   5},

	/* 6+ */
	{  1,   2,   2,   3,   3,   4,   4,   4,   5,   5,   5,   5},

	/* 8+ */
	{  1,   2,   2,   3,   4,   4,   4,   5,   5,   5,   5,   5},

	/* 10+ */
	{  2,   2,   3,   3,   4,   4,   5,   5,   5,   5,   5,   6},

	/* 13+ */
	{  2,   3,   3,   3,   4,   4,   5,   5,   5,   5,   5,   6},

	/* 15+ */
	{  3,   3,   3,   4,   4,   4,   5,   5,   5,   5,   6,   6},

	/* 18+ */
	{  3,   3,   3,   4,   4,   4,   5,   5,   5,   5,   6,   6},

	/* 20+ */
	{  3,   3,   4,   4,   4,   4,   5,   5,   5,   6,   6,   6}
};

#endif



/*
 * Store owners (exactly four "possible" owners per store, chosen randomly)
 * { name, purse, max greed, min greed, haggle_per, tolerance, race, unused }
 */
cptr store_names[MAX_STORES] = 
{
	"General store",
	"Armoury",
	"Weapon Smith",
	"Temple",
	"Alchemist",
	"Magic Shop",
	"Black Market",
	"Your home",
	"The Back Room"
};




/*
 * This table allows quick conversion from "speed" to "energy"
 * The basic function WAS ((S>=110) ? (S-110) : (100 / (120-S)))
 * Note that table access is *much* quicker than computation.
 *
 * Note that the table has been changed at high speeds.  From
 * "Slow (-40)" to "Fast (+30)" is pretty much unchanged, but
 * at speeds above "Fast (+30)", one approaches an asymptotic
 * effective limit of 50 energy per turn.  This means that it
 * is relatively easy to reach "Fast (+30)" and get about 40
 * energy per turn, but then speed becomes very "expensive",
 * and you must get all the way to "Fast (+50)" to reach the
 * point of getting 45 energy per turn.  After that point,
 * furthur increases in speed are more or less pointless,
 * except to balance out heavy inventory.
 *
 * Note that currently the fastest monster is "Fast (+30)".
 *
 * It should be possible to lower the energy threshhold from
 * 100 units to 50 units, though this may interact badly with
 * the (compiled out) small random energy boost code.  It may
 * also tend to cause more "clumping" at high speeds.
 */
u16b extract_energy[200] =
{
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* Slow */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* S-50 */     100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
	/* S-40 */     200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
	/* S-30 */     200,  200,  200,  200,  200,  200,  200,  300,  300,  300,
	/* S-20 */     300,  300,  300,  300,  300,  400,  400,  400,  400,  400,
	/* S-10 */     500,  500,  500,  500,  600,  600,  700,  700,  800,  900,
	/* Norm */    1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
	/* F+10 */    2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900,
	/* F+20 */    3000, 3100, 3200, 3300, 3400, 3500, 3600, 3600, 3700, 3700,
	/* F+30 */    3800, 3800, 3900, 3900, 4000, 4000, 4000, 4100, 4100, 4100,
	/* F+40 */    4200, 4200, 4200, 4300, 4300, 4300, 4400, 4400, 4400, 4400,
	/* F+50 */    4500, 4500, 4500, 4500, 4500, 4600, 4600, 4600, 4600, 4600,
	/* F+60 */    4700, 4700, 4700, 4700, 4700, 4800, 4800, 4800, 4800, 4800,
	/* F+70 */    4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900,
	/* Fast */    4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900, 4900,
};


/*
 * This table provides for different game speeds at different
 * dungeon depths.  Shallower depths are faster, allowing for
 * easier town navigation.  Deeper depths are slow, hopefully
 * make deep combat less of a test of reflexs.
 */
u16b level_speeds[128] =
{
	 7500,  9000,  9100,  9200,  9300,  9400,  9500,  9600,  9700,  9800, /* Town - 450' */
	 9900, 10000, 10000, 10000, 10000, 10000, 10100, 10200, 10300, 10400, /* 500' - 950' */
	10500, 10600, 10700, 10800, 10900, 11000, 11100, 11200, 11300, 11400, /* 1000' - 1450' */
	11500, 11600, 11700, 11800, 11900, 12000, 12100, 12200, 12300, 12400, /* 1500' - 1950' */
	12500, 12600, 12700, 12800, 12900, 13000, 13100, 13200, 13300, 13400, /* 2000' - 2450' */
	13500, 13700, 13800, 13900, 14000, 14200, 14300, 14400, 14600, 14800, /* 2500' - 2950' */
	15000, 15200, 15400, 15600, 15800, 16000, 16200, 16400, 16600, 16800, /* 3000' - 3450' */
	17000, 17200, 17400, 17600, 17800, 18000, 18200, 18400, 18600, 18800, /* 3500' - 3950' */
	19000, 19200, 19400, 19600, 19800, 20000, 20000, 20000, 20000, 20000, /* 4000' - 4450' */
	20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, /* 4500' - 4950' */
	20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, /* 5000' - 5450' */
	20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, /* 5500' - 5950' */
	20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000                /* 6000' - 6350' */
};





/*
 * Base experience levels, may be adjusted up for race and/or class
 */
s32b player_exp[PY_MAX_LEVEL] =
{
	10,
	25,
	45,
	70,
	100,
	140,
	200,
	280,
	380,
	500,
	650,
	850,
	1100,
	1400,
	1800,
	2300,
	2900,
	3600,
	4400,
	5400,
	6800,
	8400,
	10200,
	12500,
	17500,
	25000,
	35000L,
	50000L,
	75000L,
	100000L,
	150000L,
	200000L,
	275000L,
	350000L,
	450000L,
	550000L,
	700000L,
	850000L,
	1000000L,
	1250000L,
	1500000L,
	1800000L,
	2100000L,
	2400000L,
	2700000L,
	3000000L,
	3500000L,
	4000000L,
	4500000L,
	5000000L
};


/*
 * Player Sexes
 *
 *	Title,
 *	Winner
 */
const player_sex sex_info[MAX_SEXES] =
{
	{
		"Female",
		"Queen"
	},

	{
		"Male",
		"King"
	}
};







/*
 * Each chest has a certain set of traps, determined by pval
 * Each chest has a "pval" from 1 to the chest level (max 55)
 * If the "pval" is negative then the trap has been disarmed
 * The "pval" of a chest determines the quality of its treasure
 * Note that disarming a trap on a chest also removes the lock.
 */
byte chest_traps[64] =
{
	0,					/* 0 == empty */
	(CHEST_POISON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),			/* 5 == best small wooden */
	0,
	(CHEST_POISON),
	(CHEST_POISON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_POISON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_SUMMON),			/* 15 == best large wooden */
	0,
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_PARALYZE),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_SUMMON),
	(CHEST_PARALYZE),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_EXPLODE),			/* 25 == best small iron */
	0,
	(CHEST_POISON | CHEST_LOSE_STR),
	(CHEST_POISON | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_PARALYZE),
	(CHEST_POISON | CHEST_SUMMON),
	(CHEST_SUMMON),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),	/* 35 == best large iron */
	0,
	(CHEST_SUMMON),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_POISON | CHEST_PARALYZE),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_POISON | CHEST_PARALYZE),	/* 45 == best small steel */
	0,
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_STR),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE),
	(CHEST_POISON | CHEST_PARALYZE),	/* 55 == best large steel */
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
};








/*
 * Hack -- the "basic" color names (see "TERM_xxx")
 */
cptr color_names[16] =
{
	"Dark",
	"White",
	"Slate",
	"Orange",
	"Red",
	"Green",
	"Blue",
	"Umber",
	"Light Dark",
	"Light Slate",
	"Violet",
	"Yellow",
	"Light Red",
	"Light Green",
	"Light Blue",
	"Light Umber",
};



/*
 * Abbreviations of healthy stats
 */
cptr stat_names[A_MAX] =
{
	"STR: ", "INT: ", "WIS: ", "DEX: ", "CON: ", "CHR: "
};

/*
 * Abbreviations of damaged stats
 */
cptr stat_names_reduced[A_MAX] =
{
	"Str: ", "Int: ", "Wis: ", "Dex: ", "Con: ", "Chr: "
};

/*
 * Full stat names
 */
cptr stat_names_full[A_MAX] =
{
	"strength",
	"intelligence",
	"wisdom",
	"dexterity",
	"constitution",
	"charisma"
};


/*
 * Standard window names
 */
cptr ang_term_name[8] =
{
	"Angband",
	"Mirror",
	"Recall",
	"Choice",
	"Term-4",
	"Term-5",
	"Term-6",
	"Term-7"
};


/*
 * Certain "screens" always use the main screen, including News, Birth,
 * Dungeon, Tomb-stone, High-scores, Macros, Colors, Visuals, Options.
 *
 * Later, special flags may allow sub-windows to "steal" stuff from the
 * main window, including File dump (help), File dump (artifacts, uniques),
 * Character screen, Small scale map, Previous Messages, Store screen, etc.
 *
 * The "ctrl-i" (tab) command flips the "Display inven/equip" and "Display
 * equip/inven" flags for all windows.
 *
 * The "ctrl-g" command (or pseudo-command) should perhaps grab a snapshot
 * of the main screen into any interested windows.
 */
cptr window_flag_desc[32] =
{
	"Display inven/equip",
	"Display equip/inven",
	"Display spell list",
	"Display character",
	NULL,
	NULL,
	"Display messages",
	"Display overhead view",
	"Display monster recall",
	"Display object recall",
	NULL,
	"Display snap-shot",
	NULL,
	NULL,
	"Display borg messages",
	"Display borg status",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};



/*
 * Option Screen Sets:
 *
 *	(define MAX_OPTION_GROUPS to match)
 */
cptr option_group[] = 
{
	"Birth Options",
	"Dungeon / Inventory Options",
	"Running / Disturbance Options",
	"Lighting Options",
	NULL
};
/* 
 * *** Options ***
 */
#define OPT_INFO(A) NULL, ((byte)OPT_ ## A)
option_type option_info[] =
{
	/*** Birth Options ***/
	{ OPT_INFO(NO_GHOST),     		FALSE,	1,	0, 0,
	"no_ghost",	    			"Death is permanent" },

	{ OPT_INFO(UNSETH_BONUS),  		FALSE,	1,	0, 0,
	"unset_class_bonus",		"Discard extra speed and hit points" },

	{ OPT_INFO(ENERGY_BUILDUP),		TRUE,	1,	0, 0,
	"energy_buildup",		"Get additional attack after being idle" },

	{ OPT_INFO(MONSTER_RECOIL),		TRUE,	1,	0, 0,
	"monster_recoil",		"Monsters recover for a turn after attacking you" },

	/*** Game-play ***/
	/* Dungeon */
	{ OPT_INFO(AUTO_SCUM),				FALSE,	2,	0, 0,
	"auto_scum",    			"Auto-scum for good levels" },	

	{ OPT_INFO(DUNGEON_ALIGN),		TRUE,	2,	0, 0,
	"dungeon_align",    		"Generate dungeons with aligned rooms" },

	{ OPT_INFO(CARRY_QUERY_FLAG), 	FALSE,	2,	0, 0,
	"carry_query_flag",	    	"Prompt before picking things up" },

	{ OPT_INFO(ALWAYS_PICKUP),		FALSE,	2,	0, 0,
	"always_pickup",    		"Pick things up by default" },

	{ OPT_INFO(PICKUP_INVEN),		FALSE,	2,	0, 0,
	"pickup_inven",    		"Pick up items matching inventory" },

	{ OPT_INFO(EASY_ALTER),			TRUE,	2,	0, 0,
	"easy_alter",    			"Open/Disarm doors/traps on movement" },

	{ OPT_INFO(BUMP_OPEN),			FALSE,	2,	0, 0,
	"bump_open",    			"Open doors on movement" },

	/* Targeting */
	{ OPT_INFO(EXPAND_LOOK),			FALSE,	2,	0, 0,
	"expand_look",  			"Expand the power of the look command" },

	{ OPT_INFO(USE_OLD_TARGET),		FALSE,	2,	0, 0,
	"use_old_target",   		"Use old target by default" },

	{ OPT_INFO(SHOW_DETAILS), 		TRUE,	2,	0, 0,
	"show_details",	    		"Show details on monster recall" },

	{ OPT_INFO(PAUSE_AFTER_DETECT),	TRUE,	2,	0, 0,
	"pause_after_detect",  		"Freeze screen after detecting monsters" },
	/* Stacking */
	{ OPT_INFO(STACK_ALLOW_ITEMS),	TRUE,	2,	0, 0,
	"stack_allow_items",    	"Allow weapons and armor to stack" },

	{ OPT_INFO(STACK_ALLOW_WANDS),	TRUE,	2,	0, 0,
	"stack_allow_wands",    	"Allow wands/staffs/rods to stack" },

	{ OPT_INFO(STACK_FORCE_NOTES),	FALSE,	2,	0, 0,
	"stack_force_notes",    	"Merge inscriptions when stacking" },

	{ OPT_INFO(STACK_FORCE_COSTS),	FALSE,	2,	0, 0,
	"stack_force_costs",    	"Merge discounts when stacking" },

	{ OPT_INFO(EXPAND_INSPECT),	FALSE,	2,	0, 0,
	"expand_inspect",    	"Compare equipment when examining items" },

	/*** Running Options ***/
	{ OPT_INFO(FIND_IGNORE_STAIRS),	TRUE,	3,	0, 0,
	"find_ignore_stairs",   	"Run past stairs" },

	{ OPT_INFO(FIND_IGNORE_DOORS),	TRUE,	3,	0, 0,
	"find_ignore_doors",    	"Run through open doors" },

	{ OPT_INFO(FIND_CUT), 			TRUE,	3,	0, 0,
	"find_cut",	    			"Run past known corners" },

	{ OPT_INFO(FIND_EXAMINE), 		TRUE,	3,	0, 0,
	"find_examine", 			"Run into potential corners" },

	{ OPT_INFO(DISTURB_MOVE), 		TRUE,	3,	0, 0,
	"disturb_move", 			"Disturb whenever any monster moves" },

	{ OPT_INFO(DISTURB_NEAR), 		TRUE,	3,	0, 0,
	"disturb_near", 			"Disturb whenever viewable monster moves" },

	{ OPT_INFO(DISTURB_PANEL),		TRUE,	3,	0, 0,
	"disturb_panel",    		"Disturb whenever map panel changes" },

	{ OPT_INFO(DISTURB_STATE),		TRUE,	3,	0, 0,
	"disturb_state",    		"Disturb whenever player state changes" },

	{ OPT_INFO(DISTURB_MINOR),		TRUE,	3,	0, 0,
	"disturb_minor",    		"Disturb whenever boring things happen" },

	{ OPT_INFO(DISTURB_OTHER),		TRUE,	3,	0, 0,
	"disturb_other",    		"Disturb whenever various things happen" },

	{ OPT_INFO(DISTURB_LOOK),		TRUE,	3,	0, 0,
	"disturb_look",     		"Stop looking around when disturbed" },

	{ OPT_INFO(ALERT_HITPOINT),		FALSE,	3,	0, 0,
	"alert_hitpoint",    		"Display a message whenever hit points are low" },

	/*** Lighting Options ***/
	{ OPT_INFO(VIEW_PERMA_GRIDS),    	TRUE,	4,	0, 0,
	"view_perma_grids", 		"Map remembers all perma-lit grids" },

	{ OPT_INFO(VIEW_TORCH_GRIDS),    	FALSE,	4,	0, 0,
	"view_torch_grids", 		"Map remembers all torch-lit grids" },

	{ OPT_INFO(VIEW_REDUCE_LITE), 	FALSE,	4,	0, 0,
	"view_reduce_lite", 		"Reduce lite-radius when running" },

	{ OPT_INFO(VIEW_REDUCE_VIEW), 	FALSE,	4,	0, 0,
	"view_reduce_view", 		"Reduce view-radius in town" },
		
	{ OPT_INFO(VIEW_YELLOW_LITE), 	FALSE,	4,	0, 0,
	"view_yellow_lite", 		"Use special colors for torch-lit grids" },

	{ OPT_INFO(VIEW_ORANGE_LITE), 	TRUE,	4,	0, 0,
	"view_orange_lite",     	"Use orange color for torch-lit grids (Haloween)" },

	{ OPT_INFO(VIEW_BRIGHT_LITE), 	FALSE,	4,	0, 0,
	"view_bright_lite", 		"Use special colors for 'viewable' grids" },

	{ OPT_INFO(VIEW_GRANITE_LITE),	FALSE,	4,	0, 0,
	"view_granite_lite",    	"Use special colors for wall grids" },

	{ OPT_INFO(VIEW_SPECIAL_LITE),	FALSE,	4,	0, 0,
	"view_special_lite",    	"Use special colors for floor grids" },

	{ OPT_INFO(AVOID_OTHER),      	FALSE,	4,	0, 0,
	"avoid_other",      		"Avoid processing weird colors" },

	{ OPT_INFO(HILITE_LEADER),     	FALSE,	4,	0, 0,
	"hilite_leader",      		"Use special color for party leader" },

	/*** Hidden Options ***/
	{ OPT_INFO(USE_COLOR),    		TRUE,	0,	0, 0,
	"use_color",    			"Use color if possible" },

	{ OPT_INFO(DEPTH_IN_FEET),    	TRUE,	0,	0, 0,
	"depth_in_feet",    		"Show dungeon level in feet" },	

	/*** End of Table ***/

	{ 0 }
};
#if 0
option_type option_info[] =
{
	/*** User-Interface ***/

	{ &rogue_like_commands,	FALSE,	1,	0, 0,
	"rogue_like_commands",	"Rogue-like commands" },

	{ &quick_messages,	 	FALSE,	1,	0, 1,
	"quick_messages",		"Activate quick messages" },

	{ &other_query_flag,	FALSE,	1,	0, 2,
	"other_query_flag",		"Prompt for various information" },

	{ &carry_query_flag,	FALSE,	1,	0, 3,
	"carry_query_flag",		"Prompt before picking things up" },

	{ &use_old_target,		FALSE,	1,	0, 4,
	"use_old_target",		"Use old target by default" },

	{ &always_pickup,		TRUE,	1,	0, 5,
	"always_pickup",		"Pick things up by default" },

	{ &always_repeat,		TRUE,	1,	0, 6,
	"always_repeat",		"Repeat obvious commands" },

	{ &depth_in_feet,		FALSE,	1,	0, 7,
	"depth_in_feet",		"Show dungeon level in feet" },

	{ &stack_force_notes,	FALSE,	1,	0, 8,
	"stack_force_notes",	"Merge inscriptions when stacking" },

	{ &stack_force_costs,	FALSE,	1,	0, 9,
	"stack_force_costs",	"Merge discounts when stacking" },

	{ &show_labels,			TRUE,	1,	0, 10,
	"show_labels",			"Show labels in object listings" },

	{ &show_weights,		FALSE,	1,	0, 11,
	"show_weights",			"Show weights in object listings" },

	{ &show_choices,		FALSE,	1,	0, 12,
	"show_choices",			"Show choices in certain sub-windows" },

	{ &show_details,		FALSE,	1,	0, 13,
	"show_details",			"Show details in certain sub-windows" },

	{ &ring_bell,			TRUE,	1,	0, 14,
	"ring_bell",			"Audible bell (on errors, etc)" },

	{ &use_color,			TRUE,	1,	0, 15,
	"use_color",			"Use color if possible (slow)" },


	/*** Disturbance ***/

	{ &find_ignore_stairs,	TRUE,	2,	0, 16,
	"find_ignore_stairs",	"Run past stairs" },

	{ &find_ignore_doors,	TRUE,	2,	0, 17,
	"find_ignore_doors",	"Run through open doors" },

	{ &find_cut,			TRUE,	2,	0, 18,
	"find_cut",				"Run past known corners" },

	{ &find_examine,		TRUE,	2,	0, 19,
	"find_examine",			"Run into potential corners" },

	{ &disturb_move,		TRUE,	2,	0, 20,
	"disturb_move",			"Disturb whenever any monster moves" },

	{ &disturb_near,		TRUE,	2,	0, 21,
	"disturb_near",			"Disturb whenever viewable monster moves" },

	{ &disturb_panel,		TRUE,	2,	0, 22,
	"disturb_panel",		"Disturb whenever map panel changes" },

	{ &disturb_state,		TRUE,	2,	0, 23,
	"disturb_state",		"Disturb whenever player state changes" },

	{ &disturb_minor,		TRUE,	2,	0, 24,
	"disturb_minor",		"Disturb whenever boring things happen" },

	{ &disturb_other,		TRUE,	2,	0, 25,
	"disturb_other",		"Disturb whenever various things happen" },

	{ &alert_hitpoint,		FALSE,	2,	0, 26,
	"alert_hitpoint",		"Alert user to critical hitpoints" },

	{ &alert_failure,		FALSE,	2,	0, 27,
	"alert_failure",		"Alert user to various failures" },


	/*** Game-Play ***/

	{ &no_ghost,			FALSE,	3,	1, 0,
	"no_ghost",				"Death is permanent" },

	{ &auto_scum,			FALSE,	3,	1, 1,
	"auto_scum",			"Auto-scum for good levels" },

	{ &stack_allow_items,	TRUE,	3,	1, 2,
	"stack_allow_items",	"Allow weapons and armor to stack" },

	{ &stack_allow_wands,	TRUE,	3,	1, 3,
	"stack_allow_wands",	"Allow wands/staffs/rods to stack" },

	{ &expand_look,			FALSE,	3,	1, 4,
	"expand_look",			"Expand the power of the look command" },

	{ &expand_list,			FALSE,	3,	1, 5,
	"expand_list",			"Expand the power of the list commands" },

	{ &view_perma_grids,	TRUE,	3,	1, 6,
	"view_perma_grids",		"Map remembers all perma-lit grids" },

	{ &view_torch_grids,	FALSE,	3,	1, 7,
	"view_torch_grids",		"Map remembers all torch-lit grids" },

	{ &dungeon_align,		TRUE,	3,	1, 8,
	"dungeon_align",		"Generate dungeons with aligned rooms" },

	{ &dungeon_stair,		TRUE,	3,	1, 9,
	"dungeon_stair",		"Generate dungeons with connected stairs" },

	{ &flow_by_sound,		FALSE,	3,	1, 10,
	"flow_by_sound",		"Monsters chase current location (v.slow)" },

	{ &flow_by_smell,		FALSE,	3,	1, 11,
	"flow_by_smell",		"Monsters chase recent locations (v.slow)" },

	{ &track_follow,		FALSE,	3,	1, 12,
	"track_follow",			"Monsters follow the player (broken)" },

	{ &track_target,		FALSE,	3,	1, 13,
	"track_target",			"Monsters target the player (broken)" },

	{ &smart_learn,			FALSE,	3,	1, 14,
	"smart_learn",			"Monsters learn from their mistakes" },

	{ &smart_cheat,			FALSE,	3,	1, 15,
	"smart_cheat",			"Monsters exploit players weaknesses" },


	/*** Efficiency ***/

	{ &view_reduce_lite,	FALSE,	4,	1, 16,
	"view_reduce_lite",		"Reduce lite-radius when running" },

	{ &view_reduce_view,	FALSE,	4,	1, 17,
	"view_reduce_view",		"Reduce view-radius in town" },

	{ &avoid_abort,			FALSE,	4,	1, 18,
	"avoid_abort",			"Avoid checking for user abort" },

	{ &avoid_other,			FALSE,	4,	1, 19,
	"avoid_other",			"Avoid processing special colors" },

	{ &flush_failure,		TRUE,	4,	1, 20,
	"flush_failure",		"Flush input on various failures" },

	{ &flush_disturb,		FALSE,	4,	1, 21,
	"flush_disturb",		"Flush input whenever disturbed" },

	{ &flush_command,		FALSE,	4,	1, 22,
	"flush_command",		"Flush input before every command" },

	{ &fresh_before,		TRUE,	4,	1, 23,
	"fresh_before",			"Flush output before every command" },

	{ &fresh_after,			FALSE,	4,	1, 24,
	"fresh_after",			"Flush output after every command" },

	{ &fresh_message,		FALSE,	4,	1, 25,
	"fresh_message",		"Flush output after every message" },

	{ &compress_savefile,	TRUE,	4,	1, 26,
	"compress_savefile",	"Compress messages in savefiles" },

	{ &hilite_player,		FALSE,	4,	1, 27,
	"hilite_player",		"Hilite the player with the cursor" },

	{ &view_yellow_lite,	FALSE,	4,	1, 28,
	"view_yellow_lite",		"Use special colors for torch-lit grids" },

	{ &view_bright_lite,	FALSE,	4,	1, 29,
	"view_bright_lite",		"Use special colors for 'viewable' grids" },

	{ &view_granite_lite,	FALSE,	4,	1, 30,
	"view_granite_lite",	"Use special colors for wall grids (slow)" },

	{ &view_special_lite,	FALSE,	4,	1, 31,
	"view_special_lite",	"Use special colors for floor grids (slow)" },


	/*** End of Table ***/

	{ 0 }
};
#endif

const cptr custom_command_schemes[SCHEME_LAST+1] = 
{
	CCS_EMPTY,
	CCS_FULL,

	CCS_ITEM,
	CCS_DIR,
	CCS_VALUE,
	CCS_SMALL,
	CCS_STRING,
	CCS_CHAR,

	CCS_ITEM_DIR,
	CCS_ITEM_VALUE,
	CCS_ITEM_SMALL,
	CCS_ITEM_STRING,
	CCS_ITEM_CHAR,

	CCS_DIR_VALUE,
	CCS_DIR_SMALL,
	CCS_DIR_STRING,
	CCS_DIR_CHAR,

	CCS_VALUE_STRING,
	CCS_VALUE_CHAR,
	CCS_SMALL_STRING,
	CCS_SMALL_CHAR,

	CCS_ITEM_DIR_VALUE,
	CCS_ITEM_DIR_SMALL,
	CCS_ITEM_DIR_STRING,
	CCS_ITEM_DIR_CHAR,

	CCS_ITEM_VALUE_STRING,
	CCS_ITEM_VALUE_CHAR,
	CCS_ITEM_SMALL_STRING,
	CCS_ITEM_SMALL_CHAR,
};

