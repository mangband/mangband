/* File: tables.c */

/* Purpose: Angband Tables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "defines.h"

/*
 *  Global array of "custom commands".
 *
 *	Ends with an empty entry (used while iterating, do not dismiss).
 *
 * FORMAT:
 *  	key, PKT, SCHEME, energy_cost, (*do_cmd_callback)
 *  	(flags | flags | flags), 
 *  	tval, prompt
 * LEGEND:
 *  key - char, single keypress	:	'j'
 *  PKT - packet type to use	: To use default command set PKT_COMMAND
 * 								: To declare new command use PKT_COMMAND+n (up to MAX_CUSTOM_COMMANDS)
 *								: To overload existing command use it's PKT_ (i.e. PKT_EAT)
 *  SCHEME - see pack.h			:	 SCHEME CONTROLS BOTH PACKET PARSING
 *								: 		AND DO_CMD_CALLBACK ARGUMENTS
 *								:			IT *IS* IMPORTANT
 *  energy_cost - 0 or n		: If the command is free use 0
 *								: Use n to set 1/Nth of level_speed
 *								: i.e. 2 to take half a turn, 1 for full turn, 4 for 1/4
 *  (*do_cmd_callback) - a callback to one of the do_cmd functions, arguments depend on SCHEME
 *  (flags) - see defines.h
 *  tval - TVAL for item tester (probably would be used as some hack for other modes too)
 *  prompt - new-line separated string of prompts for each (flags) group.
 */
const custom_command_type custom_commands[MAX_CUSTOM_COMMANDS] = 
{	
	/*** Simpliest, one-off commands ***/
	{ /* Go Up by stairs */
		'<', PKT_GO_UP, SCHEME_EMPTY, 1, (cccb)do_cmd_go_up,
		(0),		0, ""
	},
	{ /* Go Down by stairs */
		'>', PKT_GO_DOWN, SCHEME_EMPTY, 1, (cccb)do_cmd_go_down,
		(0),		0, ""
	},
	{ /* Search */
		's', PKT_SEARCH, SCHEME_EMPTY, 1, (cccb)do_cmd_search,
		(0),		0, ""
	},
	{ /* Toggle Search */
		'S', PKT_SEARCH_MODE, SCHEME_EMPTY, 1, (cccb)do_cmd_toggle_search,
		(0),		0, ""
	},
	{ /* Repeat Feeling */
		KTRL('F'), (char)(PKT_COMMAND+1), SCHEME_EMPTY, 1, (cccb)do_cmd_feeling,
		(0),		0, ""
	},

	/*** Simple grid altering commands ***/
	{ /* Alter */
		'+', PKT_ALTER, SCHEME_DIR, 1, (cccb)do_cmd_alter,
		(COMMAND_TARGET_DIR),		0, ""
	},
	{ /* Tunnel */
		'T', PKT_TUNNEL, SCHEME_DIR, 1, (cccb)do_cmd_tunnel,
		(COMMAND_TARGET_DIR),		0, ""
	},
	{ /* Bash a door */
		'B', PKT_BASH, SCHEME_DIR, 1, (cccb)do_cmd_bash,
		(COMMAND_TARGET_DIR),		0, ""
	},
	{ /* Disarm a trap or chest */
		'D', PKT_DISARM, SCHEME_DIR, 1, (cccb)do_cmd_disarm,
		(COMMAND_TARGET_DIR),		0, ""
	},
	{ /* Open door or chest */
		'o', PKT_OPEN, SCHEME_DIR, 1, (cccb)do_cmd_open,
		(COMMAND_TARGET_DIR),		0, ""
	},
	{ /* Close door */
		'c', PKT_CLOSE, SCHEME_DIR, 1, (cccb)do_cmd_close,
		(COMMAND_TARGET_DIR),		0, ""
	},

	/*** Complex grid altering ***/
	{ /* Spike door */
		'j', PKT_SPIKE, SCHEME_DIR, 1, (cccb)do_cmd_spike,
		(COMMAND_ITEM_QUICK | COMMAND_ITEM_INVEN | COMMAND_TARGET_DIR),
		TV_SPIKE, "You have no spikes!"
	},
	{ /* Steal (MAngband-specific) */
		'J', PKT_STEAL, SCHEME_DIR, 1, (cccb)do_cmd_steal,
		(COMMAND_TARGET_DIR),
		0, "Touch in what "
	},
	{ /* Purchase/Sell/Examine House (MAngband-specific) */
		'h', (char)PKT_COMMAND, SCHEME_DIR, 1, (cccb)do_cmd_purchase_house,
		(COMMAND_TARGET_DIR),
		0, "Knock in what "
	},

	/*** Inventory commands ***/
	{ /* Wear/Wield Item */
		'w', PKT_WIELD, SCHEME_ITEM, 1, (cccb)do_cmd_wield,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		item_test(WEAR), "Wear/Wield which item? "
	},
	{ /* Takeoff */
		't', PKT_TAKE_OFF, SCHEME_ITEM, 1, (cccb)do_cmd_takeoff,
		(COMMAND_ITEM_EQUIP),
		0, "Takeoff which item? "
	},
	{ /* Drop Item */
		'd', PKT_TAKE_OFF, SCHEME_ITEM_VALUE, 1, (cccb)do_cmd_drop,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_AMMOUNT),
		0, "Drop what? \nHow much? "
	},
	{ /* Inscribe Item */
		'{', PKT_INSCRIBE, SCHEME_ITEM_STRING , 0, (cccb)do_cmd_inscribe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR | COMMAND_NEED_STRING),
		0, "Inscribe what? \nInscription: "
	},
	{ /* Uninscribe what?  */
		'}', PKT_UNINSCRIBE, SCHEME_ITEM, 0, (cccb)do_cmd_uninscribe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR),
		0, "Uninscribe what? "
	},
	{ /* Observe/Examine item  */
		'I', (char)PKT_OBSERVE, SCHEME_ITEM, 0, (cccb)do_cmd_observe,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_EQUIP | COMMAND_ITEM_FLOOR),
		0, "Examine what? "
	},

	/*** Inventory "usage" commands ***/
	/* Magic devices */
	{ /* Read scroll */
		'r', PKT_READ, SCHEME_ITEM, 1, (cccb)do_cmd_read_scroll,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_SCROLL, "Read which scroll? "
	},
	{ /* Aim wand */
		'a', PKT_AIM_WAND, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_aim_wand,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_WAND, "Aim which wand? "
	},
	{ /* Use staff */
		'u', PKT_USE, SCHEME_ITEM, 1, (cccb)do_cmd_use_staff,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_STAFF, "Use which staff? "
	},
	{ /* Zap rod */
		'z', PKT_ZAP, SCHEME_ITEM, 1, (cccb)do_cmd_zap_rod,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_ROD, "Use which rod? "
	},
	{ /* Activate */
		'A', PKT_ACTIVATE, SCHEME_ITEM, 1, (cccb)do_cmd_activate,
		(COMMAND_ITEM_EQUIP),
		item_test(ACTIVATE), "Activate what? "
	},
	/* Common items */
	{ /* Refill */
		'F', PKT_FILL, SCHEME_ITEM, 1, (cccb)do_cmd_refill,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		item_test(REFILL), "Refill with which light? "
	},
	{ /* Drink */
		'q', PKT_QUAFF, SCHEME_ITEM, 1, (cccb)do_cmd_quaff_potion,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_POTION, "Quaff which potion? "
	},
	{ /* Eat */
		'E', PKT_EAT, SCHEME_ITEM, 1, (cccb)do_cmd_eat_food,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR),
		TV_FOOD, "Eat what? "
	},

	/*** Firing and throwing ***/
	{ /* Fire an object */
		'f', PKT_FIRE, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_fire,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_TARGET_ALLOW),
		item_test(AMMO), "Fire which ammo? "
	},
	{ /* Throw an object */
		'v', PKT_THROW, SCHEME_ITEM_DIR, 1, (cccb)do_cmd_throw,
		(COMMAND_ITEM_INVEN | COMMAND_ITEM_FLOOR | COMMAND_TARGET_ALLOW),
		0, "Throw what? "
	},

	/*** Spell-casting ***/
	{ /* Study spell */
		'G', PKT_STUDY, SCHEME_ITEM_SMALL, 0, (cccb)do_cmd_study,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET),
		TV_MAGIC_BOOK, "You cannot gain spells!\nGain from which book? \nSpells\nStudy which spell? "
	},
	{ /* Cast spell */
		'm', PKT_SPELL, SCHEME_ITEM_DIR_SMALL, 0, (cccb)do_cmd_cast_pre,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET | 
		 COMMAND_TARGET_ALLOW | COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		TV_MAGIC_BOOK, "You cannot cast spells!\nCast from what book? \nSpells\nCast which spell? "
	},
	{ /* Use ghost power */
		'U', PKT_GHOST, SCHEME_DIR_SMALL, 0, (cccb)do_cmd_ghost_power_pre,
		(COMMAND_TEST_DEAD | COMMAND_SPELL_CUSTOM | COMMAND_SPELL_RESET | COMMAND_TARGET_ALLOW | 
		 COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		(10), "You are not undead.\nPowers\nUse which power? "
	},
	{ /* Cast cleric spell */
		'p', PKT_PRAY, SCHEME_ITEM_DIR_SMALL, 0, (cccb)do_cmd_pray_pre,
		(COMMAND_TEST_SPELL | COMMAND_ITEM_INVEN | COMMAND_SPELL_BOOK | COMMAND_SPELL_RESET | 
		 COMMAND_TARGET_ALLOW | COMMAND_TARGET_FRIEND | COMMAND_SECOND_DIR | COMMAND_NEED_SECOND),
		TV_PRAYER_BOOK, "Pray hard enough and your prayers may be answered.\nPray from what book? \nPrayers\nPray which prayer? "
	},	

	/*** Miscellaneous; MAngband-specific ***/
	{ /* 'Social' */
		KTRL('S'), PKT_COMMAND, SCHEME_DIR_SMALL, 0, (cccb)do_cmd_social,
		(COMMAND_SPELL_CUSTOM | COMMAND_SPELL_RESET | COMMAND_SPELL_INDEX | COMMAND_TARGET_ALLOW),
		(12), "Socials\nDo what? "
	},
#if 0
	{ /* Suicide */
		'Q', PKT_SUICIDE, SCHEME_CHAR, 1, (cccb)do_cmd_suicide,
		(COMMAND_NEED_CONFIRM | COMMAND_NEED_CHAR),
		0, "Please verify SUICIDE by typing the '@' sign: \nDo you really want to commit suicide? "
	},
#endif
	{ /* Drop Gold */
		'$', PKT_DROP_GOLD, SCHEME_VALUE, 1, (cccb)do_cmd_drop_gold,
		(COMMAND_NEED_VALUE),
		0, "How much gold? "
	},
	{ /* Symbol Query */
		'/', PKT_SYMBOL_QUERY, SCHEME_CHAR, 0, (cccb)do_cmd_query_symbol,
		(COMMAND_NEED_CHAR),
		0, "Symbol: "
	},

	/* End-of-array */
	{ 0,0,0,0,0,0,0 }
};

/* Item testers */
byte item_tester_tvals[MAX_ITEM_TESTERS][MAX_ITH_TVAL] = 
{
	/* item_tester_hook_wear (ITH_WEAR) */
	{ 0 },
	/* item_tester_hook_weapon (ITH_WEAPON) */
	{ TV_SWORD, TV_HAFTED, TV_POLEARM, TV_DIGGING, TV_BOW, TV_BOLT, TV_ARROW, TV_SHOT, 0 },
	/* item_tester_hook_armour (ITH_ARMOR) */
	{ TV_DRAG_ARMOR, TV_HARD_ARMOR, TV_SOFT_ARMOR, TV_SHIELD, TV_CLOAK, TV_CROWN, TV_HELM, TV_BOOTS, TV_GLOVES, 0 },
	/* item_tester_hook_ammo (ITH_AMMO) */
	{ TV_BOLT, TV_ARROW, TV_SHOT, 0 },
	/* item_tester_hook_recharge (ITH_RECHARGE) */
	{ TV_STAFF, TV_WAND, 0 },
	/* item_tester_hook_activate (ITH_ACTIVATE) */
	{ 0 },
	/* item_tester_refill_lantern (ITH_REFILL) */
	{ 0 }, /*{ TV_FLASK, TV_LITE, 0 },*/
	/* item_tester_refill_torch (ITH_REFILL) */
	{ 0 }, /*{ TV_LITE, 0 } */

	{ 0 }, /* End of array */
};
byte item_tester_flags[MAX_ITEM_TESTERS] = 
{
	/* item_tester_hook_wear (ITH_WEAR) */
	(ITF_WEAR),
	/* item_tester_hook_weapon (ITH_WEAPON) */
	0,
	/* item_tester_hook_armour (ITH_ARMOR) */
	0,
	/* item_tester_hook_ammo (ITH_AMMO) */
	0,
	/* item_tester_hook_recharge (ITH_RECHARGE) */
	0,
	/* item_tester_hook_activate (ITH_ACTIVATE) */
	(ITF_ACT),
	/* item_tester_refill_lantern (ITH_REFILL_LANTERN) */
	(ITF_FUEL),
	/* item_tester_refill_torch (ITH_REFILL_TORCH) */
	(ITF_FUEL),

	0, /* End of array */
};


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

/* Store price maxes (purse) have now been increased by 5x from Angband 3.0.6 */
owner_type owners[MAX_STORES][MAX_OWNERS] =
{
	{
		/* General store */
		{ "Bilbo the Friendly",		5000*5,		170, 108,  5, 15, RACE_HOBBIT},
		{ "Rincewind the Chicken",	10000*5,	175, 108,  4, 12, RACE_HUMAN},
		{ "Snafu the Midget",		20000*5,	170, 107,  5, 15, RACE_GNOME},
		{ "Lyar-el the Comely",		30000*5,	165, 107,  6, 18, RACE_ELF},
	},
	{
		/* Armoury */
		{ "Kon-Dar the Ugly",		5000*5,		210, 115,  5,  7, RACE_HALF_ORC},
		{ "Darg-Low the Grim",		10000*5,	190, 111,  4,  9, RACE_HUMAN},
		{ "Decado the Handsome",	25000*5,  	200, 112,  4, 10, RACE_DUNADAN},
		{ "Mauglin the Grumpy",		30000*5,	200, 112,  4,  5, RACE_DWARF},
	},
	{
		/* Weapon Smith */
		{ "Ithyl-Mak the Beastly",	5000*5,		210, 115,  6,  6, RACE_HALF_TROLL},
		{ "Arndal Beast-Slayer",	10000*5,	185, 110,  5,  9, RACE_HALF_ELF},
		{ "Tarl Beast-Master",		25000*5,	190, 115,  5,  7, RACE_HOBBIT},
		{ "Oglign Dragon-Slayer",	30000*5,	195, 112,  4,  8, RACE_DWARF},
	},
	{
		/* Temple */
		{ "Ludwig the Humble",		15000*5,	175, 109,  6, 15, RACE_HUMAN},
		{ "Gunnar the Paladin",		20000*5,	185, 110,  5, 23, RACE_HUMAN},
		{ "Delilah the Pure",		25000*5,	180, 107,  6, 20, RACE_ELF},
		{ "Keldon the Wise",		30000*5,	185, 109,  5, 15, RACE_DWARF},
	},
	{
		/* Alchemist */
		{ "Mauser the Chemist",		10000*5,	190, 111,  5,  8, RACE_HALF_ELF},
		{ "Wizzle the Chaotic",		10000*5,	190, 110,  6,  8, RACE_HOBBIT},
		{ "Ga-nat the Greedy",		15000*5,	200, 116,  6,  9, RACE_GNOME},
		{ "Vella the Slender",		15000*5,	220, 111,  4,  9, RACE_HUMAN},
	},
	{
		/* Magic Shop */
		{ "Ariel the Sorceress",	15000*5,	200, 110,  7,  8, RACE_HALF_ELF},
		{ "Buggerby the Great",		20000*5,	215, 113,  6, 10, RACE_GNOME},
		{ "Inglorian the Mage",		25000*5,	200, 110,  7, 10, RACE_HUMAN},
		{ "Luthien Starshine",		30000*5,	175, 110,  5, 11, RACE_HIGH_ELF},
	},
	{
		/* Black Market */
		{ "Lo-Hak the Awful",		15000*5,	250, 150, 10,  5, RACE_HALF_TROLL},
		{ "Histor the Goblin",		20000*5,	250, 150, 10,  5, RACE_HALF_ORC},
		{ "Durwin the Shifty",		25000*5,	250, 150, 10,  5, RACE_HALF_ORC},
		{ "Drago the Fair",			30000*5,	250, 150, 10,  5, RACE_ELF},
	},
	{
		/* Home */
		{ "Your home",				0,      100, 100,  0, 99, 99},
		{ "Your home",				0,      100, 100,  0, 99, 99},
		{ "Your home",				0,      100, 100,  0, 99, 99},
		{ "Your home",				0,      100, 100,  0, 99, 99}
	}
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
byte extract_energy[200] =
{
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* S-50 */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* S-40 */     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	/* S-30 */     2,  2,  2,  2,  2,  2,  2,  3,  3,  3,
	/* S-20 */     3,  3,  3,  3,  3,  4,  4,  4,  4,  4,
	/* S-10 */     5,  5,  5,  5,  6,  6,  7,  7,  8,  9,
	/* Norm */    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	/* F+10 */    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	/* F+20 */    30, 31, 32, 33, 34, 35, 36, 36, 37, 37,
	/* F+30 */    38, 38, 39, 39, 40, 40, 40, 41, 41, 41,
	/* F+40 */    42, 42, 42, 43, 43, 43, 44, 44, 44, 44,
	/* F+50 */    45, 45, 45, 45, 45, 46, 46, 46, 46, 46,
	/* F+60 */    47, 47, 47, 47, 47, 48, 48, 48, 48, 48,
	/* F+70 */    49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
	/* Fast */    49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
};


/*
 * This table provides for different game speeds at different
 * dungeon depths.  Shallower depths are faster, allowing for
 * easier town navigation.  Deeper depths are slow, hopefully
 * make deep combat less of a test of reflexs.
 */
byte level_speeds[128] =
{
	 75,  90,  91,  92,  93,  94,  95,  96,  97,  98, /* Town - 450' */
	 99, 100, 100, 100, 100, 100, 101, 102, 103, 104, /* 500' - 950' */
	105, 106, 107, 108, 109, 110, 111, 112, 113, 114, /* 1000' - 1450' */
	115, 116, 117, 118, 119, 120, 121, 122, 123, 124, /* 1500' - 1950' */
	125, 126, 127, 128, 129, 130, 131, 132, 133, 134, /* 2000' - 2450' */
	135, 137, 138, 139, 140, 142, 143, 144, 146, 148, /* 2500' - 2950' */
	150, 152, 154, 156, 158, 160, 162, 164, 166, 168, /* 3000' - 3450' */
	170, 172, 174, 176, 178, 180, 182, 184, 186, 188, /* 3500' - 3950' */
	190, 192, 194, 196, 198, 200, 200, 200, 200, 200, /* 4000' - 4450' */
	200, 200, 200, 200, 200, 200, 200, 200, 200, 200, /* 4500' - 4950' */
	200, 200, 200, 200, 200, 200, 200, 200, 200, 200, /* 5000' - 5450' */
	200, 200, 200, 200, 200, 200, 200, 200, 200, 200, /* 5500' - 5950' */
	200, 200, 200, 200, 200, 200, 200, 200            /* 6000' - 6350' */
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
 * Hack -- the "basic" sound names (see "SOUND_xxx")
 */
cptr sound_names[SOUND_MAX] =
{
	"",
	"hit",
	"miss",
	"flee",
	"drop",
	"kill",
	"level",
	"death",
};



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
#define OPT_INFO(A) ((byte)OPT_ ## A)
option_type option_info[] =
{
	/*** Birth Options ***/
	{ OPT_INFO(NO_GHOST),     		FALSE,	1,	0, 0,
	"no_ghost",	    			"Death is permanent" },
	
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

	{ OPT_INFO(EASY_ALTER),			TRUE,	2,	0, 0,
	"easy_alter",    			"Open/Disarm doors/traps on movement" },
	/* Targeting */
	{ OPT_INFO(EXPAND_LOOK),			FALSE,	2,	0, 0,
	"expand_look",  			"Expand the power of the look command" },

	{ OPT_INFO(USE_OLD_TARGET),		FALSE,	2,	0, 0,
	"use_old_target",   		"Use old target by default" },

	{ OPT_INFO(SHOW_DETAILS), 		TRUE,	2,	0, 0,
	"show_details",	    		"Show details on monster recall" },
	/* Stacking */
	{ OPT_INFO(STACK_ALLOW_ITEMS),	TRUE,	2,	0, 0,
	"stack_allow_items",    	"Allow weapons and armor to stack" },

	{ OPT_INFO(STACK_ALLOW_WANDS),	TRUE,	2,	0, 0,
	"stack_allow_wands",    	"Allow wands/staffs/rods to stack" },

	{ OPT_INFO(STACK_FORCE_NOTES),	FALSE,	2,	0, 0,
	"stack_force_notes",    	"Merge inscriptions when stacking" },

	{ OPT_INFO(STACK_FORCE_COSTS),	FALSE,	2,	0, 0,
	"stack_force_costs",    	"Merge discounts when stacking" },

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


	/*** Hidden Options ***/
	{ OPT_INFO(USE_COLOR),    		TRUE,	0,	0, 0,
	"use_color",    			"Use color if possible" },

	{ OPT_INFO(DEPTH_IN_FEET),    	TRUE,	0,	0, 0,
	"depth_in_feet",    		"Show dungeon level in feet" },	

	/*** End of Table ***/

	{ 0,			0, 0, 0, 0,
	NULL,			NULL }
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

	{ NULL,			0, 0, 0, 0,
	NULL,			NULL }
};
#endif
#define _TILE(A,C) {(A),(byte)(C)}
/* Hack -- player images for graphic mode */
cave_view_type player_presets[3][MAX_CLASS+1][MAX_RACES+1] = {
	/* Standart 8x8 */
	{ 
		/* Warrior */
		{
			_TILE(0x8C,0x80), /* Human */
			_TILE(0x8C,0x81), /* Half-Elf */
			_TILE(0x8C,0x82), /* Elf */
			_TILE(0x8C,0x83), /* Hobbit */
			_TILE(0x8C,0x84), /* Gnome */
			_TILE(0x8C,0x85), /* Dwarf */
			_TILE(0x8C,0x86), /* Half-Orc */
			_TILE(0x8C,0x87), /* Half-Troll */
			_TILE(0x8C,0x88), /* Dunadan */
			_TILE(0x8C,0x89), /* High-Elf */
			_TILE(0xA4,0x92), /* Kobold */
		},	
		/* Mage */
		{
			_TILE(0x8C,0x8A), /* Human */
			_TILE(0x8C,0x8B), /* Half-Elf */
			_TILE(0x8C,0x8C), /* Elf */
			_TILE(0x8C,0x8D), /* Hobbit */
			_TILE(0x8C,0x8E), /* Gnome */
			_TILE(0x8C,0x8F), /* Dwarf */
			_TILE(0x8C,0x90), /* Half-Orc */
			_TILE(0x8C,0x91), /* Half-Troll */
			_TILE(0x8C,0x92), /* Dunadan */
			_TILE(0x8C,0x93), /* High-Elf */
			_TILE(0xA4,0x91), /* Kobold */
		},
		/* Priest */
		{
			_TILE(0x8C,0x94), /* Human */
			_TILE(0x8C,0x95), /* Half-Elf */
			_TILE(0x8C,0x96), /* Elf */
			_TILE(0x8C,0x97), /* Hobbit */
			_TILE(0x8C,0x98), /* Gnome */
			_TILE(0x8C,0x99), /* Dwarf */
			_TILE(0x8C,0x9A), /* Half-Orc */
			_TILE(0x8C,0x9B), /* Half-Troll */
			_TILE(0x8C,0x9C), /* Dunadan */
			_TILE(0x8C,0x9D), /* High-Elf */
			_TILE(0xA6,0x91), /* Kobold */
		},
		/* Rogue */
		{
			_TILE(0x8C,0x9E), /* Human */
			_TILE(0x8C,0x9F), /* Half-Elf */
			_TILE(0x8D,0x80), /* Elf */
			_TILE(0x8D,0x81), /* Hobbit */
			_TILE(0x8D,0x82), /* Gnome */
			_TILE(0x8D,0x83), /* Dwarf */
			_TILE(0x8D,0x84), /* Half-Orc */
			_TILE(0x8D,0x85), /* Half-Troll */
			_TILE(0x8D,0x86), /* Dunadan */
			_TILE(0x8D,0x87), /* High-Elf */
			_TILE(0xA7,0x91), /* Kobold */
		},
		/* Ranger */
		{
			_TILE(0x8D,0x88), /* Human */
			_TILE(0x8D,0x89), /* Half-Elf */
			_TILE(0x8D,0x8A), /* Elf */
			_TILE(0x8D,0x8B), /* Hobbit */
			_TILE(0x8D,0x8C), /* Gnome */
			_TILE(0x8D,0x8D), /* Dwarf */
			_TILE(0x8D,0x8E), /* Half-Orc */
			_TILE(0x8D,0x8F), /* Half-Troll */
			_TILE(0x8D,0x90), /* Dunadan */
			_TILE(0x8D,0x91), /* High-Elf */
			_TILE(0xA8,0x91), /* Kobold */
		},
		/* Palladin */
		{
			_TILE(0x8D,0x92), /* Human */
			_TILE(0x8D,0x93), /* Half-Elf */
			_TILE(0x8D,0x94), /* Elf */
			_TILE(0x8D,0x95), /* Hobbit */
			_TILE(0x8D,0x96), /* Gnome */
			_TILE(0x8D,0x97), /* Dwarf */
			_TILE(0x8D,0x98), /* Half-Orc */
			_TILE(0x8D,0x99), /* Half-Troll */
			_TILE(0x8D,0x9A), /* Dunadan */
			_TILE(0x8D,0x9B), /* High-Elf */
			_TILE(0xA9,0x91), /* Kobold */
		},
		/* Special */
		{
			_TILE(0x91,0x82), /* Ghost */
			_TILE(0x96,0x98), /* Fruit bat */
		}
	},
	/* New 16x16 */
	{ 
		/* Warrior */
		{
			_TILE(0x92,0x80), /* Human */
			_TILE(0x92,0x81), /* Half-Elf */
			_TILE(0x92,0x82), /* Elf */
			_TILE(0x92,0x83), /* Hobbit */
			_TILE(0x92,0x84), /* Gnome */
			_TILE(0x92,0x85), /* Dwarf */
			_TILE(0x92,0x86), /* Half-Orc */
			_TILE(0x92,0x87), /* Half-Troll */
			_TILE(0x92,0x88), /* Dunadan */
			_TILE(0x92,0x89), /* High-Elf */
			_TILE(0x92,0x92), /* Kobold */
		},
		/* Mage */
		{
			_TILE(0x93,0x80), /* Human */
			_TILE(0x93,0x81), /* Half-Elf */
			_TILE(0x93,0x82), /* Elf */
			_TILE(0x93,0x83), /* Hobbit */
			_TILE(0x93,0x84), /* Gnome */
			_TILE(0x93,0x85), /* Dwarf */
			_TILE(0x93,0x86), /* Half-Orc */
			_TILE(0x93,0x87), /* Half-Troll */
			_TILE(0x93,0x88), /* Dunadan */
			_TILE(0x93,0x89), /* High-Elf */
			_TILE(0x93,0x92), /* Kobold */
		},
		/* Priest */
		{
			_TILE(0x94,0x80), /* Human */
			_TILE(0x94,0x81), /* Half-Elf */
			_TILE(0x94,0x82), /* Elf */
			_TILE(0x94,0x83), /* Hobbit */
			_TILE(0x94,0x84), /* Gnome */
			_TILE(0x94,0x85), /* Dwarf */
			_TILE(0x94,0x86), /* Half-Orc */
			_TILE(0x94,0x87), /* Half-Troll */
			_TILE(0x94,0x88), /* Dunadan */
			_TILE(0x94,0x89), /* High-Elf */
			_TILE(0x94,0x92), /* Kobold */
		},
		/* Rogue */
		{
			_TILE(0x95,0x80), /* Human */
			_TILE(0x95,0x81), /* Half-Elf */
			_TILE(0x95,0x82), /* Elf */
			_TILE(0x95,0x83), /* Hobbit */
			_TILE(0x95,0x84), /* Gnome */
			_TILE(0x95,0x85), /* Dwarf */
			_TILE(0x95,0x86), /* Half-Orc */
			_TILE(0x95,0x87), /* Half-Troll */
			_TILE(0x95,0x88), /* Dunadan */
			_TILE(0x95,0x89), /* High-Elf */
			_TILE(0x95,0x92), /* Kobold */
		},
		/* Ranger */
		{
			_TILE(0x96,0x80), /* Human */
			_TILE(0x96,0x81), /* Half-Elf */
			_TILE(0x96,0x82), /* Elf */
			_TILE(0x96,0x83), /* Hobbit */
			_TILE(0x96,0x84), /* Gnome */
			_TILE(0x96,0x85), /* Dwarf */
			_TILE(0x96,0x86), /* Half-Orc */
			_TILE(0x96,0x87), /* Half-Troll */
			_TILE(0x96,0x88), /* Dunadan */
			_TILE(0x96,0x89), /* High-Elf */
			_TILE(0x96,0x92), /* Kobold */
		},
		/* Palladin */
		{
			_TILE(0x97,0x80), /* Human */
			_TILE(0x97,0x81), /* Half-Elf */
			_TILE(0x97,0x82), /* Elf */
			_TILE(0x97,0x83), /* Hobbit */
			_TILE(0x97,0x84), /* Gnome */
			_TILE(0x97,0x85), /* Dwarf */
			_TILE(0x97,0x86), /* Half-Orc */
			_TILE(0x97,0x87), /* Half-Troll */
			_TILE(0x97,0x88), /* Dunadan */
			_TILE(0x97,0x89), /* High-Elf */
			_TILE(0x97,0x92), /* Kobold */
		},
		/* Special */
		{
			_TILE(0x9F,0x9E), /* Ghost */
			_TILE(0xA5,0x8F), /* Fruit bat */
		}
	},
	/* David Gervais 16x16 */
	{ 
		/* Warrior */
		{
			_TILE(0x83,0x87), /* Human */
			_TILE(0x83,0x8F), /* Half-Elf */
			_TILE(0x83,0x97), /* Elf */
			_TILE(0x83,0x9F), /* Hobbit */
			_TILE(0x83,0xC7), /* Gnome */
			_TILE(0x83,0xAF), /* Dwarf */
			_TILE(0x83,0xB7), /* Half-Orc */
			_TILE(0x83,0xBF), /* Half-Troll */
			_TILE(0x83,0xA7), /* Dunadan */
			_TILE(0x83,0xCF), /* High-Elf */
			_TILE(0x83,0xD7), /* Kobold */
		},
		/* Mage */
		{
			_TILE(0x83,0x81), /* Human */
			_TILE(0x83,0x88), /* Half-Elf */
			_TILE(0x83,0x91), /* Elf */
			_TILE(0x83,0x98), /* Hobbit */
			_TILE(0x83,0xC1), /* Gnome */
			_TILE(0x83,0xA8), /* Dwarf */
			_TILE(0x83,0xB1), /* Half-Orc */
			_TILE(0x83,0xB8), /* Half-Troll */
			_TILE(0x83,0xA1), /* Dunadan */
			_TILE(0x83,0xC8), /* High-Elf */
			_TILE(0x83,0xD1), /* Kobold */
		},
		/* Priest */
		{
			_TILE(0x83,0x84), /* Human */
			_TILE(0x83,0x8C), /* Half-Elf */
			_TILE(0x83,0x94), /* Elf */
			_TILE(0x83,0x9C), /* Hobbit */
			_TILE(0x83,0xC4), /* Gnome */
			_TILE(0x83,0xAC), /* Dwarf */
			_TILE(0x83,0xB4), /* Half-Orc */
			_TILE(0x83,0xBC), /* Half-Troll */
			_TILE(0x83,0xA4), /* Dunadan */
			_TILE(0x83,0xCC), /* High-Elf */
			_TILE(0x83,0xD3), /* Kobold */
		},
		/* Rogue */
		{
			_TILE(0x83,0x86), /* Human */
			_TILE(0x83,0x8E), /* Half-Elf */
			_TILE(0x83,0x96), /* Elf */
			_TILE(0x83,0x9E), /* Hobbit */
			_TILE(0x83,0xC6), /* Gnome */
			_TILE(0x83,0xAE), /* Dwarf */
			_TILE(0x83,0xB6), /* Half-Orc */
			_TILE(0x83,0xBE), /* Half-Troll */
			_TILE(0x83,0xA6), /* Dunadan */
			_TILE(0x83,0xCE), /* High-Elf */
			_TILE(0x83,0xD6), /* Kobold */
		},
		/* Ranger */
		{
			_TILE(0x83,0x85), /* Human */
			_TILE(0x83,0x8D), /* Half-Elf */
			_TILE(0x83,0x95), /* Elf */
			_TILE(0x83,0x9D), /* Hobbit */
			_TILE(0x83,0xC5), /* Gnome */
			_TILE(0x83,0xAD), /* Dwarf */
			_TILE(0x83,0xB5), /* Half-Orc */
			_TILE(0x83,0xBD), /* Half-Troll */
			_TILE(0x83,0xA5), /* Dunadan */
			_TILE(0x83,0xCD), /* High-Elf */
			_TILE(0x83,0xD5), /* Kobold */
		},
		/* Palladin */
		{
			_TILE(0x83,0x83), /* Human */
			_TILE(0x83,0x8B), /* Half-Elf */
			_TILE(0x83,0x93), /* Elf */
			_TILE(0x83,0x9B), /* Hobbit */
			_TILE(0x83,0xC3), /* Gnome */
			_TILE(0x83,0xAB), /* Dwarf */
			_TILE(0x83,0xB3), /* Half-Orc */
			_TILE(0x83,0xBB), /* Half-Troll */
			_TILE(0x83,0xA3), /* Dunadan */
			_TILE(0x83,0xCB), /* High-Elf */
			_TILE(0x83,0xD3), /* Kobold */
		},
		/* Special */
		{
			_TILE(0x94,0x96), /* Ghost */
			_TILE(0x90,0x81), /* Fruit bat */
		}
	}
};
