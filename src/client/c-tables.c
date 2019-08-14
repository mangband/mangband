/* File: tables.c */

/* Purpose: Angband Tables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

/*
 * Only a little of the file is used for the client, the rest is commented out.
 */
#include "c-angband.h"



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
 * This table could be used to convert single char into TERM_ color.
 *
 * The mapping is "dwsorgbuDWvyRGBU" to DARK, WHITE, SLATE, ORANGE, RED, BLUE,
 *  UMBER, L_DARK, L_WHITE, VIOLET, YELLOW, L_RED, L_GREEN, L_BLUE, L_UMBER;
 *
 * Additional mappings might be introduced.
 */
byte ascii_to_color[128] = 
{
	/* NL  1  2  3  4  5  6  7 */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  8  9 LF 11 12 CR 14 15 */ 
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/* 16 17 18 19 20 21 22 23 */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/* 24 25 26 27 28 29 30 31 */	
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/* SP !  "  #  $  %  &  '  */	
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  (  )  *  +  ,  -  .  /  */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  0  1  2  3  4  5  6  7  */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  8  9  :  ;  <  =  >  ?  */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  @  A  B  C  D  E  F  G */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_BLUE,	TERM_L_WHITE,	TERM_L_DARK,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_GREEN,
	/*  H  I  J  K  L  M  N  O */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  P  Q  R  S  T  U  V  W */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_RED, 	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_UMBER,	TERM_L_WHITE,	TERM_L_WHITE,
	/*  X  Y  Z  [  \  ]  ^  _ */ 
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE, 
	/*  `  a  b  c  d  e  f  g */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_BLUE,  	TERM_L_WHITE,	TERM_DARK,  	TERM_L_WHITE,	TERM_L_WHITE,	TERM_GREEN,
	/*  h  i  j  k  l  m  n  o */ 
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_ORANGE,
 	/*  p  q  r  s  t  u  v  w */
	TERM_L_WHITE,	TERM_L_WHITE,	TERM_RED,   	TERM_SLATE, 	TERM_L_WHITE,	TERM_UMBER, 	TERM_VIOLET,	TERM_WHITE,
 	/*  x  y  z  {  |  }  ~ DL */
	TERM_L_WHITE,	TERM_YELLOW,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE,	TERM_L_WHITE
	/* 128 */
};

/*
 * Spells in each book (mage spells then priest spells)
 */
u32b spell_flags[3][9][2] =
{
	{
		/*** Mage spell books ***/
		{ 0x000001ff, 0x00000000 },
		{ 0x0003fe00, 0x00000000 },
		{ 0x07fc0000, 0x00000000 },
		{ 0xf8000000, 0x00000001 },
		{ 0x00000000, 0x0000003e },
		{ 0x00000000, 0x00001fc0 },
		{ 0x00000000, 0x0003e000 },
		{ 0x00000000, 0x00fc0000 },
		{ 0x00000000, 0xff000000 }
	},

	{
		/*** Priest spell books ***/
		{ 0x000000ff, 0x00000000 },
		{ 0x0000ff00, 0x00000000 },
		{ 0x01ff0000, 0x00000000 },
		{ 0x7e000000, 0x00000000 },
		{ 0x00000000, 0x03f00000 },
		{ 0x80000000, 0x0000000f },
		{ 0x00000000, 0x000001f0 },
		{ 0x00000000, 0x000fc000 },
		{ 0x00000000, 0x00003e00 }
    },

};

#if 0
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
 * Class titles for the player.
 *
 * The player gets a new title every five levels, so each class
 * needs only ten titles total.
 */
cptr player_title[MAX_CLASS][PY_MAX_LEVEL/5] =
{
	/* Warrior */
	{
		"Rookie",
		"Soldier",
		"Mercenary",
		"Veteran",
		"Swordsman",
		"Champion",
		"Hero",
		"Baron",
		"Duke",
		"Lord",
	},

	/* Mage */
	{
		"Novice",
		"Apprentice",
		"Trickster",
		"Illusionist",
		"Spellbinder",
		"Evoker",
		"Conjurer",
		"Warlock",
		"Sorcerer",
		"Mage Lord",
	},

	/* Priest */
	{
		"Believer",
		"Acolyte",
		"Adept",
		"Curate",
		"Canon",
		"Lama",
		"Patriarch",
		"Priest",
		"High Priest",
		"Priest Lord",
	},

	/* Rogues */
	{
		"Vagabond",
		"Cutpurse",
		"Robber",
		"Burglar",
		"Filcher",
		"Sharper",
		"Low Thief",
		"High Thief",
		"Master Thief",
		"Assassin",
	},

	/* Rangers */
	{
		"Runner",
		"Strider",
		"Scout",
		"Courser",
		"Tracker",
		"Guide",
		"Pathfinder",
		"Low Ranger",
		"High Ranger",
		"Ranger Lord",
	},

	/* Paladins */
	{
		"Gallant",
		"Keeper",
		"Protector",
		"Defender",
		"Warder",
		"Knight",
		"Guardian",
		"Low Paladin",
		"High Paladin",
		"Paladin Lord",
    }

};
#endif


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
 * Standard sound (and message) names
 */
cptr angband_sound_name[MSG_MAX] =
{
	"",
	"hit",
	"miss",
	"flee",
	"drop",
	"kill",
	"level",
	"death",
	"study",
	"teleport",
	"shoot",
	"quaff",
	"zap_rod",
	"walk",
	"tpother",
	"hitwall",
	"eat",
	"store1",
	"store2",
	"store3",
	"store4",
	"dig",
	"opendoor",
	"shutdoor",
	"tplevel",
	"bell",
	"nothing_to_open",
	"lockpick_fail",
	"stairs_down", 
	"hitpoint_warn",
	"act_artifact", 
	"use_staff", 
	"destroy", 
	"mon_hit", 
	"mon_touch", 
	"mon_punch", 
	"mon_kick", 
	"mon_claw", 
	"mon_bite", 
	"mon_sting", 
	"mon_butt", 
	"mon_crush", 
	"mon_engulf", 
	"mon_crawl", 
	"mon_drool", 
	"mon_spit", 
	"mon_gaze", 
	"mon_wail", 
	"mon_spore", 
	"mon_beg", 
	"mon_insult", 
	"mon_moan", 
	"recover", 
	"blind", 
	"confused", 
	"poisoned", 
	"afraid", 
	"paralyzed", 
	"drugged", 
	"speed", 
	"slow", 
	"shield", 
	"blessed", 
	"hero", 
	"berserk", 
	"prot_evil", 
	"invuln", 
	"see_invis", 
	"infrared", 
	"res_acid", 
	"res_elec", 
	"res_fire", 
	"res_cold", 
	"res_pois", 
	"stun", 
	"cut", 
	"stairs_up", 
	"store_enter", 
	"store_leave", 
	"store_home", 
	"money1", 
	"money2", 
	"money3", 
	"shoot_hit", 
	"store5", 
	"lockpick", 
	"disarm", 
	"identify_bad", 
	"identify_ego", 
	"identify_art", 
	"breathe_elements", 
	"breathe_frost", 
	"breathe_elec", 
	"breathe_acid", 
	"breathe_gas", 
	"breathe_fire", 
	"breathe_confusion", 
	"breathe_disenchant", 
	"breathe_chaos", 
	"breathe_shards", 
	"breathe_sound", 
	"breathe_light", 
	"breathe_dark", 
	"breathe_nether", 
	"breathe_nexus", 
	"breathe_time", 
	"breathe_inertia", 
	"breathe_gravity", 
	"breathe_plasma", 
	"breathe_force", 
	"summon_monster", 
	"summon_angel", 
	"summon_undead", 
	"summon_animal", 
	"summon_spider", 
	"summon_hound", 
	"summon_hydra", 
	"summon_demon", 
	"summon_dragon", 
	"summon_gr_undead", 
	"summon_gr_dragon", 
	"summon_gr_demon", 
	"summon_ringwraith", 
	"summon_unique", 
	"wield", 
	"cursed", 
	"pseudo_id", 
	"hungry", 
	"notice", 
	"ambient_day", 
	"ambient_nite", 
	"ambient_dng1", 
	"ambient_dng2", 
	"ambient_dng3", 
	"ambient_dng4", 
	"ambient_dng5", 
	"mon_create_trap", 
	"mon_shriek", 
	"mon_cast_fear", 
	"hit_good", 
	"hit_great", 
	"hit_superb", 
	"hit_hi_great", 
	"hit_hi_superb", 
	"cast_spell", 
	"pray_prayer",
	"kill_unique",
	"kill_king",
	"drain_stat",
	"multiply"
	,"",
	"player_talk",
	"player_yell",
	"player_social",
	"player_spell",
	"player_prayer",
	"player_action",
	"monster_action",
};


#if 0
/*
 * Abbreviations of healthy stats
 */
cptr stat_names[6] =
{
	"STR: ", "INT: ", "WIS: ", "DEX: ", "CON: ", "CHR: "
};

/*
 * Abbreviations of damaged stats
 */
cptr stat_names_reduced[6] =
{
	"Str: ", "Int: ", "Wis: ", "Dex: ", "Con: ", "Chr: "
};
#endif

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
	"Display player (basic)",
	"Display player (extra)",
	"Display player (compact)",
	NULL,//"Display map view",
	"Display messages",
	NULL,//"Display overhead view",
	NULL,//"Display monster recall",
	NULL,//"Display object recall",
	NULL,//"Display monster list",
	"Display status",
	"Display chat messages",
	"Display spell list",
	"Display player (history)",//"Display borg messages",
	NULL,//"Display special info",//"Display borg status",
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
 * Available Options
 *
 * Option Screen Sets:
 *
 *	Set 1: User Interface
 */
cptr local_option_group[] =
{
	"User-interace options",
	NULL /* End */
};
option_type local_option_info[] =
{
	/*** User-Interface ***/
	{ &rogue_like_commands,	0,FALSE,	1,	0, 0,
	"rogue_like_commands",  	"Rogue-like commands" },

	{ &ring_bell,			0,TRUE,	1,	0, 0,
	"ring_bell",			"Audible bell (on errors, etc)" },

	{ &use_color,			0,TRUE,	1,	0, 0,
	"use_color",			"Use color if possible" },

	{ &depth_in_feet,		0,TRUE,	1,	0, 0,
	"depth_in_feet",		"Show dungeon level in feet" },

	{ &show_labels, 		0,FALSE,	1,	0, 0,
	"show_labels",			"Show labels in equipment listings" },

	{ &show_weights,		0,TRUE,	1,	0, 0,
	"show_weights",			"Show weights in object listings" },

	{ &wrap_messages,		0,TRUE,	1,	0, 0,
	"wrap_messages",		"Wrap long messages in sub-windows" },

	{ &auto_accept, 		0,FALSE,	1,	0, 0,
	"auto_accept",			"Always say Yes to Yes/No prompts" },

	{ &auto_itemlist,   		0,FALSE,	1,	0, 0,
	"auto_itemlist",		"Show listings when prompting for an item" },

	{ &auto_showlist,   		0,FALSE,	1,	0, 0,
	"auto_showlist",		"Show listings when prompting for a spell" },


	/*** End of Table ***/

	{ NULL,			0, 0, 0, 0, 0,
	NULL,			NULL }
};

#if 0
	/*** User-Interface ***/

/* 00 */{ &rogue_like_commands,	FALSE,	1,	0, 0,
	"rogue_like_commands",	"Rogue-like commands" },

/* 01 */{ &quick_messages,	 	FALSE,	1,	0, 1,
	"quick_messages",		"Activate quick messages" },

/* 02 */{ &other_query_flag,	FALSE,	1,	0, 2,
	"other_query_flag",		"Prompt for various information" },

/* 03 */{ &carry_query_flag,	FALSE,	1,	0, 3,
	"carry_query_flag",		"Prompt before picking things up" },

/* 04 */{ &use_old_target,		FALSE,	1,	0, 4,
	"use_old_target",		"Use old target by default" },

/* 05 */{ &always_pickup,		TRUE,	1,	0, 5,
	"always_pickup",		"Pick things up by default" },

/* 06 */{ &always_repeat,		TRUE,	1,	0, 6,
	"always_repeat",		"Repeat obvious commands" },

/* 07 */{ &depth_in_feet,		FALSE,	1,	0, 7,
	"depth_in_feet",		"Show dungeon level in feet" },

/* 08 */{ &stack_force_notes,	FALSE,	1,	0, 8,
	"stack_force_notes",	"Merge inscriptions when stacking" },

/* 09 */{ &stack_force_costs,	FALSE,	1,	0, 9,
	"stack_force_costs",	"Merge discounts when stacking" },

/* 10 */{ &show_labels,			TRUE,	1,	0, 10,
	"show_labels",			"Show labels in object listings" },

/* 11 */{ &show_weights,		FALSE,	1,	0, 11,
	"show_weights",			"Show weights in object listings" },

/* 12 */{ &show_choices,		FALSE,	1,	0, 12,
	"show_choices",			"Show choices in certain sub-windows" },

/* 13 */{ &show_details,		FALSE,	1,	0, 13,
	"show_details",			"Show details in certain sub-windows" },

/* 14 */{ &ring_bell,			TRUE,	1,	0, 14,
	"ring_bell",			"Audible bell (on errors, etc)" },

/* 15 */{ &use_color,			TRUE,	1,	0, 15,
	"use_color",			"Use color if possible (slow)" },


	/*** Disturbance ***/

/* 16 */{ &find_ignore_stairs,	TRUE,	2,	0, 16,
	"find_ignore_stairs",	"Run past stairs" },

/* 17 */{ &find_ignore_doors,	TRUE,	2,	0, 17,
	"find_ignore_doors",	"Run through open doors" },

/* 18 */{ &find_cut,			TRUE,	2,	0, 18,
	"find_cut",				"Run past known corners" },

/* 19 */{ &find_examine,		TRUE,	2,	0, 19,
	"find_examine",			"Run into potential corners" },

/* 20 */{ &disturb_move,		TRUE,	2,	0, 20,
	"disturb_move",			"Disturb whenever any monster moves" },

/* 21 */{ &disturb_near,		TRUE,	2,	0, 21,
	"disturb_near",			"Disturb whenever viewable monster moves" },

/* 22 */{ &disturb_panel,		TRUE,	2,	0, 22,
	"disturb_panel",		"Disturb whenever map panel changes" },

/* 23 */{ &disturb_state,		TRUE,	2,	0, 23,
	"disturb_state",		"Disturb whenever player state changes" },

/* 24 */{ &disturb_minor,		TRUE,	2,	0, 24,
	"disturb_minor",		"Disturb whenever boring things happen" },

/* 25 */{ &disturb_other,		TRUE,	2,	0, 25,
	"disturb_other",		"Disturb whenever various things happen" },

/* 26 */{ &alert_hitpoint,		FALSE,	2,	0, 26,
	"alert_hitpoint",		"Alert user to critical hitpoints" },

/* 27 */{ &alert_failure,		FALSE,	2,	0, 27,
	"alert_failure",		"Alert user to various failures" },


	/*** Game-Play ***/

/* 28 */{ &no_ghost,			FALSE,	3,	1, 0,
	"no_ghost",				"Death is permanent" },

/* 29 */{ &auto_scum,			FALSE,	3,	1, 1,
	"auto_scum",			"Auto-scum for good levels" },

/* 30 */{ &stack_allow_items,	TRUE,	3,	1, 2,
	"stack_allow_items",	"Allow weapons and armor to stack" },

/* 31 */{ &stack_allow_wands,	TRUE,	3,	1, 3,
	"stack_allow_wands",	"Allow wands/staffs/rods to stack" },

/* 32 */{ &expand_look,			FALSE,	3,	1, 4,
	"expand_look",			"Expand the power of the look command" },

/* 33 */{ &expand_list,			FALSE,	3,	1, 5,
	"expand_list",			"Expand the power of the list commands" },

/* 34 */{ &view_perma_grids,	TRUE,	3,	1, 6,
	"view_perma_grids",		"Map remembers all perma-lit grids" },

/* 35 */{ &view_torch_grids,	FALSE,	3,	1, 7,
	"view_torch_grids",		"Map remembers all torch-lit grids" },

/* 36 */{ &dungeon_align,		TRUE,	3,	1, 8,
	"dungeon_align",		"Generate dungeons with aligned rooms" },

/* 38 */{ &dungeon_stair,		TRUE,	3,	1, 9,
	"dungeon_stair",		"Generate dungeons with connected stairs" },

/* 39 */{ &flow_by_sound,		FALSE,	3,	1, 10,
	"flow_by_sound",		"Monsters chase current location (v.slow)" },

/* 40 */{ &flow_by_smell,		FALSE,	3,	1, 11,
	"flow_by_smell",		"Monsters chase recent locations (v.slow)" },

/* 41 */{ &track_follow,		FALSE,	3,	1, 12,
	"track_follow",			"Monsters follow the player (broken)" },

/* 42 */{ &track_target,		FALSE,	3,	1, 13,
	"track_target",			"Monsters target the player (broken)" },

/* 43 */{ &smart_learn,			FALSE,	3,	1, 14,
	"smart_learn",			"Monsters learn from their mistakes" },

/* 44 */{ &smart_cheat,			FALSE,	3,	1, 15,
	"smart_cheat",			"Monsters exploit players weaknesses" },


	/*** Efficiency ***/

/* 45 */{ &view_reduce_lite,	FALSE,	4,	1, 16,
	"view_reduce_lite",		"Reduce lite-radius when running" },

/* 46 */{ &view_reduce_view,	FALSE,	4,	1, 17,
	"view_reduce_view",		"Reduce view-radius in town" },

/* 47 */{ &avoid_abort,			FALSE,	4,	1, 18,
	"avoid_abort",			"Avoid checking for user abort" },

/* 48 */{ &avoid_other,			FALSE,	4,	1, 19,
	"avoid_other",			"Avoid processing special colors" },

/* 49 */{ &flush_failure,		TRUE,	4,	1, 20,
	"flush_failure",		"Flush input on various failures" },

/* 50 */{ &flush_disturb,		FALSE,	4,	1, 21,
	"flush_disturb",		"Flush input whenever disturbed" },

/* 51 */{ &flush_command,		FALSE,	4,	1, 22,
	"flush_command",		"Flush input before every command" },

/* 52 */{ &fresh_before,		TRUE,	4,	1, 23,
	"fresh_before",			"Flush output before every command" },

/* 53 */{ &fresh_after,			FALSE,	4,	1, 24,
	"fresh_after",			"Flush output after every command" },

/* 54 */{ &fresh_message,		FALSE,	4,	1, 25,
	"fresh_message",		"Flush output after every message" },

/* 55 */{ &compress_savefile,	TRUE,	4,	1, 26,
	"compress_savefile",	"Compress messages in savefiles" },

/* 56 */{ &hilite_player,		FALSE,	4,	1, 27,
	"hilite_player",		"Hilite the player with the cursor" },

/* 57 */{ &view_yellow_lite,	FALSE,	4,	1, 28,
	"view_yellow_lite",		"Use special colors for torch-lit grids" },

/* 58 */{ &view_bright_lite,	FALSE,	4,	1, 29,
	"view_bright_lite",		"Use special colors for 'viewable' grids" },

/* 59 */{ &view_granite_lite,	FALSE,	4,	1, 30,
	"view_granite_lite",	"Use special colors for wall grids (slow)" },

/* 60 */{ &view_special_lite,	FALSE,	4,	1, 31,
	"view_special_lite",	"Use special colors for floor grids (slow)" },


	/*** End of Table ***/

	{ NULL,			0, 0, 0, 0,
	NULL,			NULL }
};
#endif

