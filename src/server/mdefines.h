/* File: defines.h */

/* Purpose: server-side global constants and macro definitions */


/*
 * Current version number of MAngband.
 */

#define SERVER_VERSION_MAJOR	1
#define SERVER_VERSION_MINOR	5
#define SERVER_VERSION_PATCH	3

/*
 * This value specifys the suffix to the version info sent to the metaserver.
 *
 * 0 - nothing
 * 1 - "alpha"
 * 2 - "beta"
 * 3 - "development"
 */
#define SERVER_VERSION_EXTRA	0


/*
 * This value is a single 16-bit number holding the version info
 */

#define SERVER_VERSION ( \
  SERVER_VERSION_MAJOR << 12 | SERVER_VERSION_MINOR << 8 \
  | SERVER_VERSION_PATCH << 4 | SERVER_VERSION_EXTRA)


/*
 * The maximum number of player ID's
 */
#define MAX_ID 4096


/*
 * This is very important...
 *
 * This is the number of "frames" to produce per second.  It determines
 * the speed of the game.
 */
#define FPS 12

/* maximum respawn time for uniques.... from japanese patch */
#define COME_BACK_TIME_MAX 600

/*
 * A "stack" of items is limited to less than 100 items (hard-coded).
 */
#define MAX_STACK_SIZE			100
/*
 * Total number of owners per store (see "store.c", etc)
 */
#define MAX_OWNERS	4

/*
 * Maximum number of player "race" types (see "table.c", etc)
 */
#define MAX_RACES           11

/*
 * Maximum number of player "class" types (see "table.c", etc)
 */
#define MAX_CLASS            6

/*
 * Maximum number of player "sex" types (see "table.c", etc)
 */
#define MAX_SEXES            2

/*
 * Total number of stores (see "store.c", etc)
 */
#define MAX_STORES	9

/*
 * Maximum number of parties to allow.  If, while trying to create a new
 * party, you get a "No empty party slot" or somesuch message, increase
 * this number.  However, you should NEVER decrease this number after a
 * server has been run, or all sorts of bad things could happen.
 */
#define MAX_PARTIES	256

/*
 * Maximum number of houses available.
 */
#define MAX_HOUSES	1024

/*
 * Total number of arenas
 */
#define MAX_ARENAS	10

/*
 * Number of entries in the player name hash table.
 * This must be a power of 2!
 */
#define NUM_HASH_ENTRIES	256


/* Maximum number of items in ITEMLIST window/command */
#define MAX_ITEMLIST 256

/*
 * Indexes of the various "stats" (hard-coded by savefiles, etc).
 */
#define A_STR	0
#define A_INT	1
#define A_WIS	2
#define A_DEX	3
#define A_CON	4
#define A_CHR	5

/*
 * Total number of stats.
 */
#define A_MAX	6


/*
 * Player race constants (hard-coded by save-files, arrays, etc)
 */
#define RACE_HUMAN		0
#define RACE_HALF_ELF	1
#define RACE_ELF		2
#define RACE_HOBBIT		3
#define RACE_GNOME		4
#define RACE_DWARF		5
#define RACE_HALF_ORC	6
#define RACE_HALF_TROLL	7
#define RACE_DUNADAN	8
#define RACE_HIGH_ELF	9
#define RACE_KOBOLD		10

/*
 * Player class constants (hard-coded by save-files, arrays, etc)
 */
#define CLASS_WARRIOR	0
#define CLASS_MAGE		1
#define CLASS_PRIEST	2
#define CLASS_ROGUE		3
#define CLASS_RANGER	4
#define CLASS_PALADIN	5

/*
 * Misc constants
 */
#define SERVER_SAVE	10		/* Minutes between server saves */
#define TOWN_DAWN		50000	/* Number of turns from dawn to dawn XXX */
#define GROW_TREE	5000		/* How often to grow a new tree in town */
#define GROW_CROPS	5000		/* How often to grow a bunch of new vegetables in wilderness */
#define BREAK_GLYPH		550		/* Rune of protection resistance */
#define BTH_PLUS_ADJ    3       /* Adjust BTH per plus-to-hit */
#define MON_MULT_ADJ	8		/* High value slows multiplication */
#define MON_SUMMON_ADJ	2		/* Adjust level of summoned creatures */
#define MON_DRAIN_LIFE	2		/* Percent of player exp drained per hit */
#define USE_DEVICE      3		/* x> Harder devices x< Easier devices     */

/* Resistance panel */
#define MAX_OBJFLAGS_ROWS 13
#define MAX_OBJFLAGS_COLS 39

/*
 * There is a 1/20 (5%) chance of inflating the requested object_level
 * during the creation of an object (see "get_obj_num()" in "object.c").
 * Lower values yield better objects more often.
 */
#define GREAT_OBJ	20

/*
 * There is a 1/50 (2%) chance of inflating the requested monster_level
 * during the creation of a monsters (see "get_mon_num()" in "monster.c").
 * Lower values yield harder monsters more often.
 */
#define NASTY_MON	50		/* 1/chance of inflated monster level */



/*
 * Refueling constants
 */
#define FUEL_TORCH	5000	/* Maximum amount of fuel in a torch */
#define FUEL_LAMP	15000   /* Maximum amount of fuel in a lantern */


/*
 * More maximum values
 */
#define MAX_SIGHT	20	/* Maximum view distance */
#define MAX_RANGE	18	/* Maximum range (spells, etc) */

/*
 * Maximum number of picked up/stolen objects a monster can carry
 */
#define MAX_MONSTER_BAG 25 /* Undef to unlimit */

/*
 * There is a 1/160 chance per round of creating a new monster
 */
#define MAX_M_ALLOC_CHANCE	160

/*
 * Normal levels get at least 14 monsters
 */
#define MIN_M_ALLOC_LEVEL	14

/*
 * The town starts out with 4 residents during the day
 */
#define MIN_M_ALLOC_TD		4

/*
 * The town starts out with 8 residents during the night
 */
#define MIN_M_ALLOC_TN		8


/* Macros for determing if it is night or day */

#define		IS_DAY	 ((turn.turn % (10L * TOWN_DAWN)) <= (10L * TOWN_DAWN / 2))
#define		IS_NIGHT ((turn.turn % (10L * TOWN_DAWN)) > (10L * TOWN_DAWN / 2))


/*
 * A monster can only "multiply" (reproduce) if there are fewer than 100
 * monsters on the level capable of such spontaneous reproduction.  This
 * is a hack which prevents the "m_list[]" array from exploding due to
 * reproducing monsters.  Messy, but necessary.
 */
#define MAX_REPRO	100




/*
 * Player "food" crucial values
 */
#define PY_FOOD_MAX		15000	/* Food value (Bloated) */
#define PY_FOOD_FULL	10000	/* Food value (Normal) */
#define PY_FOOD_ALERT	2000	/* Food value (Hungry) */
#define PY_FOOD_WEAK	1000	/* Food value (Weak) */
#define PY_FOOD_FAINT	500		/* Food value (Fainting) */
#define PY_FOOD_STARVE	100		/* Food value (Starving) */

/*
 * Player regeneration constants
 */
#define PY_REGEN_NORMAL		197		/* Regen factor*2^16 when full */
#define PY_REGEN_WEAK		98		/* Regen factor*2^16 when weak */
#define PY_REGEN_FAINT		33		/* Regen factor*2^16 when fainting */
#define PY_REGEN_HPBASE		1442	/* Min amount hp regen*2^16 */
#define PY_REGEN_MNBASE		524		/* Min amount mana regen*2^16 */


/*
 * Time bubble scale factors in percentage terms
 */
#define MAX_TIME_SCALE	1000
#define MIN_TIME_SCALE	10
#define RUNNING_FACTOR	500		/* Increase time by this percentage when running */
#define NORMAL_TIME		100		/* 100% */
/*
 * When player is low on HP, divide time factor by CONSTANT_TIME_FACTOR.
 * To enable varying health-based factor (e.g. 50% health = 50% time slowdown)
 * undefine CONSTANT_TIME_FACTOR.
 */
#define CONSTANT_TIME_FACTOR	5	/* N times slower */


/*
 * Maximum number of options and option groups
 */
#define	OPT_MAX             45
#define	MAX_OPTION_GROUPS   4

/*
 * Available Options
 *
 * It is very important that all options stay together
 * without any gaps, OPT_MAX must not exceed them too.
 * Order, on the other hand, is not important.
 *
 *  *** WRONG ***			*** CORRECT ***
 * OPT_MAX       64			OPT_MAX       2
 * OPT_NO_GHOST  5          OPT_NO_GHOST  0
 * OPT_SUN_LIGHT 9          OPT_SUN_LIGHT 1
 */
#define OPT_NO_GHOST    		0
#define OPT_DEPTH_IN_FEET   	1
#define OPT_DUNGEON_ALIGN   	2
#define OPT_AVOID_OTHER 		3
#define OPT_USE_COLOR   		4
#define OPT_AUTO_SCUM       	5
#define OPT_EXPAND_LOOK 		6
#define OPT_SHOW_DETAILS    	7
#define OPT_CARRY_QUERY_FLAG	8
#define OPT_ALWAYS_PICKUP   	9
#define OPT_USE_OLD_TARGET  	10
#define OPT_STACK_FORCE_NOTES	11
#define OPT_STACK_FORCE_COSTS	12
#define OPT_STACK_ALLOW_ITEMS	13
#define OPT_STACK_ALLOW_WANDS	14
#define OPT_FIND_IGNORE_STAIRS	15
#define OPT_FIND_IGNORE_DOORS	16
#define OPT_FIND_CUT        	17
#define OPT_FIND_EXAMINE    	18
#define OPT_DISTURB_MOVE    	19
#define OPT_DISTURB_NEAR    	20
#define OPT_DISTURB_PANEL   	21
#define OPT_DISTURB_STATE   	22
#define OPT_DISTURB_MINOR   	23
#define OPT_DISTURB_OTHER   	24
#define OPT_VIEW_PERMA_GRIDS	25
#define OPT_VIEW_TORCH_GRIDS	26
#define OPT_VIEW_REDUCE_LITE	27
#define OPT_VIEW_REDUCE_VIEW	28
#define OPT_VIEW_YELLOW_LITE	29
#define OPT_VIEW_BRIGHT_LITE	30
#define OPT_VIEW_GRANITE_LITE	31
#define OPT_VIEW_SPECIAL_LITE	32
#define OPT_VIEW_ORANGE_LITE	33
#define OPT_EASY_ALTER			34
#define OPT_ALERT_HITPOINT		35
#define OPT_HILITE_LEADER   	36
#define OPT_PAUSE_AFTER_DETECT 	37
#define OPT_DISTURB_LOOK    	38
#define OPT_UNSETH_BONUS    	39
#define OPT_EXPAND_INSPECT	40
#define OPT_ENERGY_BUILDUP	41
#define OPT_MONSTER_RECOIL	42
#define OPT_BUMP_OPEN   	43
#define OPT_PICKUP_INVEN	44
#define option_p(A,B) (A->options[OPT_ ## B])


/*
 * Buffers for ".txt" text files
 */
#define MAX_TEXTFILES 2
#define TEXTFILE__WID 80
#define TEXTFILE__HGT 23
#define TEXTFILE_MOTD 0
#define TEXTFILE_TOMB 1


/*
 * indicators / print-fields defines
 */
#define MAX_INDICATORS 64

/* Those IDs must match the order of indicators[] array from tables.c */
/* Player compact */
#define IN_RACE     	0
#define IN_CLASS    	1
#define IN_TITLE    	2
#define IN_LEVEL    	3
#define IN_EXP      	4
#define IN_GOLD     	5
#define IN_STAT0    	6
#define IN_STAT1    	7
#define IN_STAT2    	8
#define IN_STAT3    	9
#define IN_STAT4    	10
#define IN_STAT5    	11
#define IN_ARMOR    	12
#define IN_HP       	13
#define IN_SP       	14
#define IN_MON_HEALTH	15
#define IN_CUT      	16
/* Status line */
#define IN_FOOD     	17
#define IN_BLIND    	18
#define IN_STUN     	19
#define IN_CONFUSED 	20
#define IN_AFRAID   	21
#define IN_POISONED 	22
#define IN_STATE    	23
#define IN_SPEED    	24
#define IN_STUDY    	25
#define IN_DEPTH    	26
#define IN_OPPOSE   	27
/* Charsheet */
#define IN_VARIOUS    	28
#define IN_SKILLS    	29
#define IN_SKILLS2    	30
#define IN_PLUSSES    	31
#define IN_HISTORY0 	32
#define IN_HISTORY1 	33
#define IN_HISTORY2 	34
#define IN_HISTORY3 	35
#define IN_NAME     	36
#define IN_GENDER   	37

/*** Screen Locations ***/

#define ROW_RACE    	1
#define COL_RACE    	0	/* <race name> */

#define ROW_CLASS   	2
#define COL_CLASS   	0	/* <class name> */

#define ROW_TITLE   	3
#define COL_TITLE   	0	/* <title> or <mode> */

#define ROW_LEVEL   	4
#define COL_LEVEL   	0	/* "LEVEL xxxxxx" */

#define ROW_EXP     	5
#define COL_EXP     	0	/* "EXP xxxxxxxx" */

#define ROW_GOLD    	6
#define COL_GOLD    	0	/* "AU xxxxxxxxx" */

#define ROW_EQUIPPY 	14
#define COL_EQUIPPY 	0	/* equippy chars */

#define ROW_LAG     	7
#define COL_LAG     	0	/* "LAG xxxxxxxx" */

#define ROW_STAT    	8
#define COL_STAT    	0	/* "xxx   xxxxxx" */
/* takes up 6 rows... */

#define ROW_AC      	15
#define COL_AC      	0	/* "Cur AC xxxxx" */

#define ROW_MAXHP   	16
#define COL_MAXHP   	0	/* "Max HP xxxxx" */

#define ROW_CURHP   	17
#define COL_CURHP   	0	/* "Cur HP xxxxx" */

#define ROW_MAXSP   	18
#define COL_MAXSP   	0	/* "Max SP xxxxx" */

#define ROW_CURSP   	19
#define COL_CURSP   	0	/* "Cur SP xxxxx" */

#define ROW_INFO    	20
#define COL_INFO    	0	/* "xxxxxxxxxxxx" */

#define ROW_CUT     	21
#define COL_CUT     	0	/* <cut> */

#define ROW_STUN    	22
#define COL_STUN    	0	/* <stun> */

#define ROW_HUNGRY  	-1
#define COL_HUNGRY  	0	/* "Weak" / "Hungry" / "Full" / "Gorged" */

#define ROW_BLIND   	-1
#define COL_BLIND   	7	/* "Blind" */

#define ROW_CONFUSED	-1
#define COL_CONFUSED	13	/* "Confused" */

#define ROW_AFRAID  	-1
#define COL_AFRAID  	22	/* "Afraid" */

#define ROW_POISONED	-1
#define COL_POISONED	29	/* "Poisoned" */

#define ROW_STATE   	-1
#define COL_STATE   	38	/* <state> */

#define ROW_SPEED   	-1
#define COL_SPEED   	49	/* "Slow (-NN)" or "Fast (+NN)" */

#define ROW_STUDY   	-1
#define COL_STUDY   	64	/* "Study" */

#define ROW_DEPTH   	-1
#define COL_DEPTH   	70	/* "Lev NNN" / "NNNN ft" */

#define ROW_OPPOSE_ELEMENTS	-1
#define COL_OPPOSE_ELEMENTS	80	/* "Acid Elec Fire Cold Pois" */


/*
 * item_tester_hook defines
 */
#define ITH_WEAR        	1
#define ITH_WEAPON      	2
#define ITH_ARMOR       	3
#define ITH_AMMO        	4
#define ITH_RECHARGE    	5
#define ITH_ACTIVATE    	6
#define ITH_REFILL      	7
#define item_test(A) (TV_MAX + (ITH_ ## A))
#define ITEM_ANY        	0

/*
 * item_tester_flag defines
 */
#define ITF_WEAR	0x01 /* Can wear/wield item */
#define ITF_ACT 	0x02 /* Can activate item */
#define ITF_FUEL 	0x04 /* Can be used as fuel */
#define ITF_FLAG_0 	0x08

#define ITF_FLAG_1 	ITEM_ASK_AIM  /* Reserved */
#define ITF_FLAG_2 	ITEM_ASK_ITEM /* Reserved */
#define ITF_FLAG_3 	0x40 /* Variant-specific */
#define ITF_FLAG_4 	0x80 /* Variant-specific */

/*
 * Chat Channels defines
 */
/* channel modes */
#define	CM_KEYLOCK	0x01 /* +k More similar to IRC's +A */
#define	CM_SECRET	0x02 /* +s */
#define	CM_MODERATE 0x04 /* +m */
#define	CM_SERVICE 	0x08 /* +! service channel */
#define	CM_PLOG 	0x10 /* +p log to plog */
/* user-channel modes */
#define	UCM_EAR  	0x01
#define	UCM_VOICE	0x02
#define	UCM_OPER 	0x04
#define	UCM_BAN  	0x08

#define UCM_LEAVE 	(UCM_EAR | UCM_VOICE | UCM_OPER)
#define on_channel(P,I) ((P)->on_channel[(I)] & UCM_EAR)

/* can_talk(p_ptr, channel_index) test:
 * Line 1. Present on channel
 * Line 2. Not banned
 * Line 3. Not moderated
 * Line 4. Moderated, BUT
 * Line 5.  user has Voice or Op */
#define can_talk(P,I) \
	(on_channel((P),(I)) && \
	!((P)->on_channel[(I)] & UCM_BAN) && \
	(!(channels[(I)].mode & CM_MODERATE) || ( \
	(channels[(I)].mode & CM_MODERATE) && \
		( (P)->on_channel[(I)] & UCM_VOICE || (P)->on_channel[(I)] & UCM_OPER ) \
	)))

#define clog(C,M) if (chan_ ## C) msg_channel(chan_ ## C, (M))
#define audit(M) clog(audit, (M))
#define debug(M) clog(debug, (M))
#define cheat(M) clog(cheat, (M))

/* Transitional networking helpers */
#define ConnPlayers(IND) players->list[IND]->data2
#define NumPlayers p_max
#define Players p_list

/*
 * Stream-related
 */
#define STREAM_DUNGEON_ASCII	0
#define STREAM_DUNGEON_GRAF1	1
#define STREAM_DUNGEON_GRAF2	2
#define STREAM_MINIMAP_ASCII	3
#define STREAM_MINIMAP_GRAF 	4
#define STREAM_BGMAP_ASCII  	5
#define STREAM_BGMAP_GRAF   	6
#define STREAM_SPECIAL_MIXED 	7
#define STREAM_SPECIAL_TEXT  	8
#define STREAM_MONSTER_TEXT  	9
#define STREAM_MONLIST_TEXT  	10
#define STREAM_ITEMLIST_TEXT	11
#define STREAM_FILE_TEXT    	12

#define Stream_line(I,S,L) stream_line_as(Players[I],S,L,L)
#define Stream_line_p(P,S,L) stream_line_as(P,S,L,L)

#define DUNGEON_STREAM_p(P) ((P)->use_graphics > GRAPHICS_PLAIN ? STREAM_DUNGEON_GRAF2 : ((P)->use_graphics ? STREAM_DUNGEON_GRAF1 : STREAM_DUNGEON_ASCII ))
#define MINIMAP_STREAM_p(P) ((P)->use_graphics ? STREAM_MINIMAP_GRAF : STREAM_MINIMAP_ASCII)
#define BGMAP_STREAM_p(P) ((P)->use_graphics ? STREAM_BGMAP_GRAF : STREAM_BGMAP_ASCII)

#define Send_char(I,X,Y,A,C) stream_char_raw(Players[I],STREAM_SPECIAL_MIXED,Y,X,A,C,A,C)
#define Send_char_p(P,X,Y,A,C) stream_char_raw(P,STREAM_SPECIAL_MIXED,Y,X,A,C,A,C)
#define Send_tile(P,X,Y,A,C,TA,TC) stream_char_raw((P),DUNGEON_STREAM_p((P)),Y,X,A,C,TA,TC)
#define Stream_tile(I,P,Y,X) stream_char(Players[I],DUNGEON_STREAM_p(P),Y,X);
#define Stream_tile_p(P,Y,X) stream_char(P,DUNGEON_STREAM_p(P),Y,X);

/*
 * The types of special file perusal.
 */
#define SPECIAL_FILE_NONE	0
#define SPECIAL_FILE_OTHER	1
#define SPECIAL_FILE_ARTIFACT	2
#define SPECIAL_FILE_PLAYER	3
#define SPECIAL_FILE_UNIQUE	4
#define SPECIAL_FILE_SCORES	5
#define SPECIAL_FILE_HELP	6
#define SPECIAL_FILE_KNOWLEDGE	7
#define SPECIAL_FILE_HOUSES	8
#define SPECIAL_FILE_OBJECT	9
#define SPECIAL_FILE_KILL	10
#define SPECIAL_FILE_HISTORY	11
#define SPECIAL_FILE_SELF	12
#define SPECIAL_FILE_MASTER	13
#define SPECIAL_FILE_INPUT	14


/*
 * Miscelanious hacks
 */
/* Hack -- see if player is playing the game already */
#define IS_PLAYING(P) ((P)->state == PLAYER_PLAYING ? TRUE : FALSE)
/* Hack -- check if object is owned by player */
#define obj_own_p(P,O) ((!(O)->owner_id || (P)->id == (O)->owner_id))
/* Hack -- shorthand alias for "check_prevent_inscription" */
#define CPI(P,M) (P)->prevents[(byte)(M)]
/* Hack -- shorthand alias for "check_guard_inscription" */
#define CGI(O,M) check_guard_inscription( (O)->note, (M) )
/* Hack -- overloaded guard-inscriptions */
#define protected_p(P,O,M) (!is_dm_p((P)) && !obj_own_p((P), (O)) && CGI((O), (M)))
/* Hack -- check guard inscription and abort (chunk of code) */
#define __trap(P,X) if ((X)) { msg_print((P), "The item's inscription prevents it."); return; }
/* Hack -- ensure a variable fits into ddx/ddy array bounds */
#define VALID_DIR(D) ((D) > 0 && (D) < 10)

/*
 * Per-player artifact states
 */
#define ARTS_NOT_FOUND	0
#define ARTS_FOUND  	1
#define ARTS_ABANDONED	2
#define ARTS_SOLD   	3

#define set_artifact_p(P, A, I) if ((P)->a_info[(A)] < (I)) (P)->a_info[(A)] = (I)

/*
 * Monster sorting flags
 */
#define SORT_EXP	0x0001
#define SORT_LEVEL	0x0002
#define SORT_PKILL	0x0004
#define SORT_TKILL	0x0008
#define SORT_EASY	0x000F
#define SORT_RARITY	0x0010
#define SORT_RICH	0x0020
#define SORT_UNIQUE	0x0040
#define SORT_QUEST	0x0080


/*
 * MAngband-specific miscellaneous hacks
 */
#define PURSE_MULTIPLIER 5 /* For store owners */
#define RIFT_RESIST_TELEPORT 127 /* For monsters */
#define PROJECTED_CHANCE_RATIO 50 /* (1.5) For players */

/*
 * Dungeon master flags
 */
#define DM_IS_MASTER    	0x00000001
#define DM_SECRET_PRESENCE	0x00000002	/* cfg_secret_dungeon_master helper */
#define DM_CAN_MUTATE_SELF	0x00000004	/* This option allows change of the DM_ options */
#define DM_CAN_ASSIGN   	0x00000008
#define DM___MENU       	0x000000F0	/* Dungeon Master Menu: (shortcut to set all) */
#define DM_CAN_BUILD    	0x00000010  /*  building menu */
#define DM_LEVEL_CONTROL	0x00000020	/*  static/unstatic level */
#define DM_CAN_SUMMON   	0x00000040	/*  summon monsters */
#define DM_CAN_GENERATE 	0x00000080	/*  generate vaults / items */
#define DM_MONSTER_FRIEND	0x00000100
#define DM_INVULNERABLE 	0x00000200
#define DM_GHOST_HANDS  	0x00000400	/* Can interact with world (like open/close doors) even as a ghost */
#define DM_GHOST_BODY   	0x00000800	/* Can carry/wield items even as a ghost */
#define DM_NEVER_DISTURB	0x00001000
#define DM_SEE_LEVEL    	0x00002000	/* See all level */
#define DM_SEE_MONSTERS 	0x00004000	/* Free ESP + Monster spoilers */
#define DM_SEE_PLAYERS  	0x00008000	/* Full Info + "Player spoilers" */
#define DM_HOUSE_CONTROL	0x00010000	/* Can reset houses */
#define DM_KEEP_LITE    	0x00020000	/* Lite radius never changes */
#define DM_OBJECT_CONTROL	0x00040000	/* Can reset objects */
#define DM_ARTIFACT_CONTROL	0x00080000	/* Can reset artifacts */
#define DM_MISC_XXX1    	0x10000000
#define DM_MISC_XXX2    	0x20000000	/* DM_MISC_XXX -- For grouping some minor features together */
#define DM_MISC_XXX3    	0x40000000
#define DM_MISC_XXX4    	0x80000000

#define is_dm(IND) \
	((Players[IND]->dm_flags & DM_IS_MASTER) ? TRUE : FALSE)

#define is_dm_p(P) \
	(((P)->dm_flags & DM_IS_MASTER) ? TRUE : FALSE)

#define dm_flag(IND,F) \
	((Players[(IND)]->dm_flags & & DM_ ## F) ? TRUE : FALSE)

#define dm_flag_p(P,F) \
	(((P)->dm_flags & DM_ ## F) ? TRUE : FALSE)

#define c_put_p(P,A,C,Y,X) { (P)->info[(Y)][(X)].a = (A); (P)->info[(Y)][(X)].c = (C); }
#define c_put(IND,A,C,Y,X) c_put_p(Players[IND],A,C,Y,X)

/*
 * Maximum number of "normal" pack slots, and the index of the "overflow"
 * slot, which can hold an item, but only temporarily, since it causes the
 * pack to "overflow", dropping the "last" item onto the ground.  Since this
 * value is used as an actual slot, it must be less than "INVEN_WIELD" (below).
 * Note that "INVEN_PACK" is probably hard-coded by its use in savefiles, and
 * by the fact that the screen can only show 23 items plus a one-line prompt.
 */
#define INVEN_PACK		23

/*
 * Indexes used for various "equipment" slots (hard-coded by savefiles, etc).
 */
#define INVEN_WIELD		24
#define INVEN_BOW       25
#define INVEN_LEFT      26
#define INVEN_RIGHT     27
#define INVEN_NECK      28
#define INVEN_LITE      29
#define INVEN_BODY      30
#define INVEN_OUTER     31
#define INVEN_ARM       32
#define INVEN_HEAD      33
#define INVEN_HANDS     34
#define INVEN_FEET      35

/*
 * Total number of inventory slots (hard-coded).
 */
#define INVEN_TOTAL	36

/*
 * This defines our way to index floor items
 * during client-server communications.
 */
#define FLOOR_INDEX     (-11)
#define FLOOR_NEGATIVE  TRUE
#define FLOOR_TOTAL     1

/*
 * Maximum number of objects allowed in a single dungeon grid.
 *
 * The main-screen has a minimum size of 24 rows, so we can always
 * display 23 objects + 1 header line.
 * MAngband-specific: we do not support floor piles, so this is set to "1",
 * same as FLOOR_TOTOAL.
 */
#define MAX_FLOOR_STACK			1/*23*/


/* Object origin kinds */
/* NOTE: do not change this unless you intend to break savefiles */
enum {
	ORIGIN_NONE = 0,
	ORIGIN_FLOOR,			/* found on the dungeon floor */
	ORIGIN_DROP,			/* normal monster drops */
	ORIGIN_CHEST,			/* from chest (depth should be copied) */
	ORIGIN_DROP_SPECIAL,		/* from monsters in special rooms */
	ORIGIN_DROP_PIT,		/* from monsters in pits/nests */
	ORIGIN_DROP_VAULT,		/* from monsters in vaults */
	ORIGIN_SPECIAL,			/* on the floor of a special room */
	ORIGIN_PIT,			/* on the floor of a pit/nest */
	ORIGIN_VAULT,			/* on the floor of a vault */
	ORIGIN_LABYRINTH,		/* on the floor of a labyrinth */
	ORIGIN_CAVERN,			/* on the floor of a cavern */
	ORIGIN_RUBBLE,			/* found under rubble */
	ORIGIN_MIXED,			/* stack with mixed origins */
	ORIGIN_STATS,			/* ^ only the above are considered by main-stats */
	ORIGIN_ACQUIRE,			/* called forth by scroll */
	ORIGIN_DROP_BREED,		/* from breeders */
	ORIGIN_DROP_SUMMON,		/* from combat summons */
	ORIGIN_STORE,			/* something you bought */
	ORIGIN_STOLEN,			/* stolen by monster (used only for gold) */
	ORIGIN_BIRTH,			/* objects created at character birth */
	ORIGIN_DROP_UNKNOWN,		/* drops from unseen foes */
	ORIGIN_CHEAT,			/* created by wizard mode */
	ORIGIN_DROP_POLY,		/* from polymorphees */
	ORIGIN_DROP_WIZARD,		/* from wizard mode summons */
	ORIGIN_WILD_DWELLING,		/* from wilderness dwellings */

	ORIGIN_MAX
};

#define ORIGIN_SIZE FLAG_SIZE(ORIGIN_MAX)
#define ORIGIN_BYTES 4 /* savefile bytes - room for 32 origin types */


/*
 * Legal restrictions for "summon_specific()"
 */
#define SUMMON_ANIMAL		1
#define SUMMON_SPIDER		2
#define SUMMON_HOUND		3
#define SUMMON_HYDRA		4
#define SUMMON_ANGEL		5
#define SUMMON_DEMON		6
#define SUMMON_UNDEAD		7
#define SUMMON_DRAGON		8
#define SUMMON_HI_UNDEAD	9
#define SUMMON_HI_DRAGON	10
#define SUMMON_HI_DEMON		11
#define SUMMON_WRAITH		12
#define SUMMON_UNIQUE		13
#define SUMMON_KIN			14
#define SUMMON_ORC			15



/*** Terrain Feature Indexes (see "lib/edit/f_info.txt") ***/

/* Nothing */
#define FEAT_NONE		0x00

/* Various */
#define FEAT_FLOOR		0x01
#define FEAT_INVIS		0x02
#define FEAT_GLYPH		0x03
#define FEAT_OPEN		0x04
#define FEAT_BROKEN		0x05
#define FEAT_LESS		0x06
#define FEAT_MORE		0x07

/* Shops */
#define FEAT_SHOP_HEAD	0x08
#define FEAT_SHOP_TAIL	0x0F

/* Traps */
#define FEAT_TRAP_HEAD	0x10
#define FEAT_TRAP_TAIL	0x1F

/* Doors */
#define FEAT_DOOR_HEAD	0x20
#define FEAT_DOOR_TAIL	0x2F

/* Extra */
#define FEAT_SECRET	0x30
#define FEAT_RUBBLE	0x31

/* Seams */
#define FEAT_MAGMA	0x32
#define FEAT_QUARTZ	0x33
#define FEAT_MAGMA_H	0x34
#define FEAT_QUARTZ_H	0x35
#define FEAT_MAGMA_K	0x36
#define FEAT_QUARTZ_K	0x37

/* Walls */
#define FEAT_WALL_EXTRA	0x38
#define FEAT_WALL_INNER	0x39
#define FEAT_WALL_OUTER	0x3A
#define FEAT_WALL_SOLID	0x3B
#define FEAT_PERM_EXTRA	0x3C
#define FEAT_PERM_INNER	0x3D
#define FEAT_PERM_OUTER	0x3E
#define FEAT_PERM_SOLID	0x3F

/* adding various wilderness features here.
feat_perm is used for an "invisible" outside wall
that keeps many algorithms happy.
-APD- */
#define FEAT_DRAWBRIDGE	0x50
#define FEAT_LOGS			0x60
#define FEAT_PERM_CLEAR	0x70
#define FEAT_PVP_ARENA 	0x5F

/* Trees */
#define FEAT_TREE			0x61
#define FEAT_EVIL_TREE  0x62

/* Wilds */
#define FEAT_DIRT				0x40
#define FEAT_GRASS			0x41
#define FEAT_CROP				0x42
#define FEAT_LOOSE_DIRT		0x44
#define FEAT_WATER			0x4C
#define FEAT_MUD				0x48

/* Crops */
#define FEAT_CROP_HEAD			0x81
#define FEAT_CROP_POTATO		0x81
#define FEAT_CROP_CABBAGE		0x82
#define FEAT_CROP_CARROT		0x83
#define FEAT_CROP_BEET			0x84
#define FEAT_CROP_SQUASH		0x85
#define FEAT_CROP_CORN			0x86
#define FEAT_CROP_MUSHROOM		0x87
#define FEAT_CROP_TAIL			0x88

/* Special "home doors" */
#define FEAT_HOME_OPEN	0x51
#define FEAT_HOME_HEAD	0x71
#define FEAT_HOME_TAIL	0x78


/*** Artifact indexes (see "lib/edit/a_info.txt") ***/

/* Lites */
#define ART_GALADRIEL		1
#define ART_ELENDIL			2
#define ART_THRAIN			3
#define ART_PALANTIR		7

/* Amulets */
#define ART_CARLAMMAS		4
#define ART_INGWE			5
#define ART_DWARVES			6
#define ART_ELESSAR		14
#define ART_EVENSTAR		15

/* Rings */
#define ART_BARAHIR			8
#define ART_TULKAS			9
#define ART_NARYA			10
#define ART_NENYA			11
#define ART_VILYA			12
#define ART_POWER			13

/* Dragon Scale */
#define ART_RAZORBACK		16
#define ART_BLADETURNER		17
#define ART_MEDIATOR		18

/* Hard Armour */
#define ART_SOULKEEPER		19
#define ART_ISILDUR			20
#define ART_ROHIRRIM		21
#define ART_BELEGENNON		22
#define ART_CELEBORN		23
#define ART_ARVEDUI			24
#define ART_CASPANION		25

/* Soft Armour */
#define ART_HIMRING		26
#define ART_HITHLOMIR		27
#define ART_THALKETTOTH		28

/* Shields */
#define ART_GILGALAD		29
#define ART_THORIN			30
#define ART_CELEGORM		31
#define ART_ANARION			32

/* Helms and Crowns */
#define ART_CELEBRIMBOR		33
#define ART_MORGOTH			34
#define ART_BERUTHIEL		35
#define ART_THRANDUIL		36
#define ART_THENGEL			37
#define ART_HAMMERHAND		38
#define ART_DOR				39
#define ART_HOLHENNETH		40
#define ART_GORLIM			41
#define ART_GONDOR			42
#define ART_NUMENOR		43

/* Cloaks */
#define ART_COLLUIN			44
#define ART_HOLCOLLETH		45
#define ART_THINGOL			46
#define ART_THORONGIL		47
#define ART_COLANNON		48
#define ART_LUTHIEN			49
#define ART_TUOR			50

/* Gloves */
#define ART_EOL			51
#define ART_CAMBELEG		52
#define ART_CAMMITHRIM		53
#define ART_PAURHACH		54
#define ART_PAURNIMMEN		55
#define ART_PAURAEGEN		56
#define ART_PAURNEN			57
#define ART_CAMLOST			58
#define ART_FINGOLFIN		59

/* Boots */
#define ART_FEANOR			60
#define ART_DAL				61
#define ART_THROR			62
#define ART_WORMTONGUE		63

/* Swords */
#define ART_MAEDHROS		64
#define ART_ANGRIST			65
#define ART_NARTHANC		66
#define ART_NIMTHANC		67
#define ART_DETHANC			68
#define ART_RILIA			69
#define ART_BELANGIL		70
#define ART_CALRIS			71
#define ART_ARUNRUTH		72
#define ART_GLAMDRING		73
#define ART_AEGLIN			74
#define ART_ORCRIST			75
#define ART_GURTHANG		76
#define ART_ZARCUTHRA		77
#define ART_MORMEGIL		78
#define ART_GONDRICAM		79
#define ART_CRISDURIAN		80
#define ART_AGLARANG		81
#define ART_RINGIL			82
#define ART_ANDURIL			83
#define ART_ANGUIREL		84
#define ART_ELVAGIL			85
#define ART_FORASGIL		86
#define ART_CARETH			87
#define ART_STING			88
#define ART_HARADEKKET		89
#define ART_GILETTAR		90
#define ART_DOOMCALLER		91

/* Polearms */
#define ART_MELKOR		92
#define ART_THEODEN			93
#define ART_PAIN			94
#define ART_OSONDIR			95
#define ART_TIL				96
#define ART_AEGLOS			97
#define ART_OROME			98
#define ART_NIMLOTH			99
#define ART_EORLINGAS		100
#define ART_DURIN			101
#define ART_EONWE			102
#define ART_BALLI			103
#define ART_LOTHARANG		104
#define ART_MUNDWINE		105
#define ART_BARUKKHELED		106
#define ART_WRATH			107
#define ART_ULMO			108
#define ART_AVAVIR			109
#define ART_HURIN		110

/* Hafted */
#define ART_GROND			111
#define ART_TOTILA			112
#define ART_THUNDERFIST		113
#define ART_BLOODSPIKE		114
#define ART_FIRESTAR		115
#define ART_TARATOL			116
#define ART_AULE			117
#define ART_NAR				118
#define ART_ERIRIL			119
#define ART_OLORIN			120
#define ART_DEATHWREAKER	121
#define ART_TURMIL			122
#define ART_GOTHMOG		123

/* Bows */
#define ART_BELTHRONDING	124
#define ART_BARD			125
#define ART_CUBRAGOL		126
#define ART_UMBAR		127
#define ART_AMROD		128
#define ART_AMRAS		129

/* Digging Tools */
#define ART_NAIN		130
#define ART_EREBOR		131

/* Miscellaneous new stuff */
#define ART_FUNDIN		132
#define ART_AZAGHAL		133
#define ART_HARADRIM		134
#define ART_NARSIL		135
#define ART_EOWYN		136

/* Randarts */
#define ART_RANDART		137

/* Randart rarity */
#define RANDART_RARITY	60

/* Option: randarts can be generated */
#define RANDART


/*** Ego-Item indexes (see "lib/edit/e_info.txt") ***/

/* Nothing */
/* xxx */
/* xxx */
/* xxx */

/* Body Armor */
#define EGO_RESIST_ACID		4
#define EGO_RESIST_ELEC		5
#define EGO_RESIST_FIRE		6
#define EGO_RESIST_COLD		7
#define EGO_RESISTANCE		8
#define EGO_ELVENKIND		9
#define EGO_ARMR_VULN		10
#define EGO_PERMANENCE		11
#define EGO_ARMR_DWARVEN	12
/* xxx */
/* xxx */
/* xxx */

/* Shields */
#define EGO_ENDURE_ACID		16
#define EGO_ENDURE_ELEC		17
#define EGO_ENDURE_FIRE		18
#define EGO_ENDURE_COLD		19
#define EGO_ENDURANCE		20
#define EGO_SHIELD_ELVENKIND	21
#define EGO_SHIELD_PRESERVATION	22
#define EGO_SHIELD_VULN		23

/* Crowns and Helms */
#define EGO_INTELLIGENCE	24
#define EGO_WISDOM			25
#define EGO_BEAUTY			26
#define EGO_MAGI			27
#define EGO_MIGHT			28
#define EGO_LORDLINESS		29
#define EGO_SEEING			30
#define EGO_INFRAVISION		31
#define EGO_LITE			32
#define EGO_TELEPATHY		33
#define EGO_REGENERATION	34
#define EGO_TELEPORTATION	35
#define EGO_SERENITY		36
#define EGO_NITE_DAY		37
#define EGO_DULLNESS		38
#define EGO_SICKLINESS		39
#define EGO_STUPIDITY		136
#define EGO_NAIVETY		137
#define EGO_UGLINESS		138


/* Cloaks */
#define EGO_PROTECTION		40
#define EGO_STEALTH			41
#define EGO_AMAN			42
#define EGO_CLOAK_MAGI		43
#define EGO_ENVELOPING		44
#define EGO_VULNERABILITY	45
#define EGO_IRRITATION		46
/* xxx */

/* Gloves */
#define EGO_FREE_ACTION		48
#define EGO_SLAYING			49
#define EGO_AGILITY			50
#define EGO_POWER			51
#define EGO_GLOVES_THIEVERY	52
#define EGO_GAUNTLETS_COMBAT	53
#define EGO_WEAKNESS		54
#define EGO_CLUMSINESS		55

/* Boots */
#define EGO_SLOW_DESCENT	56
#define EGO_QUIET			57
#define EGO_MOTION			58
#define EGO_SPEED			59
#define EGO_STABILITY		60
#define EGO_NOISE			61
#define EGO_SLOWNESS		62
#define EGO_ANNOYANCE		63

/* Weapons */
#define EGO_HA				64
#define EGO_DF				65
#define EGO_BLESS_BLADE		66
#define EGO_GONDOLIN		67
#define EGO_WEST			68
#define EGO_ATTACKS			69
#define EGO_FURY			70
/* xxx */
#define EGO_BRAND_ACID		72
#define EGO_BRAND_ELEC		73
#define EGO_BRAND_FIRE		74
#define EGO_BRAND_COLD		75
#define EGO_BRAND_POIS		76
/* xxx */
/* xxx */
/* xxx */
#define EGO_SLAY_ANIMAL		80
#define EGO_SLAY_EVIL		81
#define EGO_SLAY_UNDEAD		82
#define EGO_SLAY_DEMON		83
#define EGO_SLAY_ORC		84
#define EGO_SLAY_TROLL		85
#define EGO_SLAY_GIANT		86
#define EGO_SLAY_DRAGON		87
#define EGO_KILL_ANIMAL		88
#define EGO_KILL_EVIL		89
#define EGO_KILL_UNDEAD		90
#define EGO_KILL_DEMON		83
#define EGO_KILL_ORC		84
#define EGO_KILL_TROLL		85
#define EGO_KILL_GIANT		86
#define EGO_KILL_DRAGON		95
/* xxx */
/* xxx */
/* xxx */
/* xxx */
#define EGO_DIGGING			100
#define EGO_DIGGER_EARTHQUAKE		101
#define EGO_MORGUL			102
/* xxx */

/* Bows */
#define EGO_ACCURACY		104
#define EGO_VELOCITY		105
#define EGO_BOW_LORIEN		106
#define EGO_CROSSBOW_HARAD	107
#define EGO_EXTRA_MIGHT		108
#define EGO_EXTRA_SHOTS		109
#define EGO_SLING_BUCKLAND	110
#define EGO_NAZGUL			111

/* Ammo */
#define EGO_HURT_ANIMAL		112
#define EGO_HURT_EVIL		113
#define EGO_HURT_UNDEAD		114
#define EGO_HURT_DEMON		115
#define EGO_HURT_ORC		116
#define EGO_HURT_TROLL		117
#define EGO_HURT_GIANT		118
#define EGO_HURT_DRAGON		119
#define EGO_AMMO_HOLY		120
#define EGO_AMMO_VENOM		121
#define EGO_FLAME			122
#define EGO_FROST			123
#define EGO_WOUNDING		124
#define EGO_BACKBITING		125

/* Broken items */
#define EGO_SHATTERED		126
#define EGO_BLASTED			127

/* MAngband specific ego items */
/* Shields */
#define EGO_SHIELD_AVARI	128


/* Cloaks */
#define EGO_CLOAK_TELERI	129
#define EGO_CLOAK_RES		131
#define EGO_CLOAK_LORDLY_RES	130

/* Gloves */
#define EGO_ISTARI			132

/* Boots */
#define EGO_MIRKWOOD		133

/* Missile launchers */
#define EGO_LOTHLORIEN		106 /* note: this is angband,
				     *	for mangband set to 134 */
#define EGO_NUMENOR		135



/*** Object "tval" and "sval" codes ***/


/*
 * The values for the "tval" field of various objects.
 *
 * This value is the primary means by which items are sorted in the
 * player inventory, followed by "sval" and "cost".
 *
 * Note that a "BOW" with tval = 19 and sval S = 10*N+P takes a missile
 * weapon with tval = 16+N, and does (xP) damage when so combined.  This
 * fact is not actually used in the source, but it kind of interesting.
 *
 * Note that as of 2.7.8, the "item flags" apply to all items, though
 * only armor and weapons and a few other items use any of these flags.
 */

#define TV_SKELETON      1	/* Skeletons ('s') */
#define TV_BOTTLE		 2	/* Empty bottles ('!') */
#define TV_JUNK          3	/* Sticks, Pottery, etc ('~') */

#define TV_SPIKE         5	/* Spikes ('~') */
#define TV_CHEST         7	/* Chests ('~') */
#define TV_SHOT			16	/* Ammo for slings */
#define TV_ARROW        17	/* Ammo for bows */
#define TV_BOLT         18	/* Ammo for x-bows */
#define TV_BOW          19	/* Slings/Bows/Xbows */
#define TV_DIGGING      20	/* Shovels/Picks */
#define TV_HAFTED       21	/* Priest Weapons */
#define TV_POLEARM      22	/* Axes and Pikes */
#define TV_SWORD        23	/* Edged Weapons */
#define TV_BOOTS        30	/* Boots */
#define TV_GLOVES       31	/* Gloves */
#define TV_HELM         32	/* Helms */
#define TV_CROWN        33	/* Crowns */
#define TV_SHIELD       34	/* Shields */
#define TV_CLOAK        35	/* Cloaks */
#define TV_SOFT_ARMOR   36	/* Soft Armor */
#define TV_HARD_ARMOR   37	/* Hard Armor */
#define TV_DRAG_ARMOR	38	/* Dragon Scale Mail */
#define TV_LITE         39	/* Lites (including Specials) */
#define TV_AMULET       40	/* Amulets (including Specials) */
#define TV_RING         45	/* Rings (including Specials) */
#define TV_STAFF        55
#define TV_WAND         65
#define TV_ROD          66
#define TV_SCROLL       70
#define TV_POTION       75
#define TV_FLASK        77
#define TV_FOOD         80
#define TV_MAGIC_BOOK   90
#define TV_PRAYER_BOOK  91
#define TV_GOLD         100	/* Gold can only be picked up by players */


/* Maximum "tval" */
#define TV_MAX		100


/* The "sval" codes for TV_GOLD (mostly calculated at runtime) */
#define SV_PLAYER_GOLD	9

/* The "sval" codes for TV_SHOT/TV_ARROW/TV_BOLT */
#define SV_AMMO_LIGHT		0	/* pebbles */
#define SV_AMMO_NORMAL		1	/* shots, arrows, bolts */
#define SV_AMMO_HEAVY		2	/* seeker arrows and bolts */
#define SV_AMMO_MITHRIL			3	/* mithril shots, arrows, bolts */
#define SV_AMMO_MAGIC			4	/* magic shots, arrows, bolts */

/* The "sval" codes for TV_BOW (note information in "sval") */
#define SV_SLING			2	/* (x2) */
#define SV_SHORT_BOW		12	/* (x2) */
#define SV_LONG_BOW			13	/* (x3) */
#define SV_LIGHT_XBOW		23	/* (x3) */
#define SV_HEAVY_XBOW		24	/* (x4) */

/* The "sval" codes for TV_DIGGING */
#define SV_SHOVEL			1
#define SV_GNOMISH_SHOVEL	2
#define SV_DWARVEN_SHOVEL	3
#define SV_PICK				4
#define SV_ORCISH_PICK		5
#define SV_DWARVEN_PICK		6
#define SV_MATTOCK			7

/* The "sval" values for TV_HAFTED */
#define SV_WHIP				2	/* 1d3 */
#define SV_QUARTERSTAFF			3	/* 1d9 */
#define SV_MACE					5	/* 2d4 */
#define SV_BALL_AND_CHAIN		6	/* 2d4 */
#define SV_WAR_HAMMER			8	/* 3d3 */
#define SV_LUCERN_HAMMER		10	/* 2d5 */
#define SV_MORNING_STAR			12	/* 2d6 */
#define SV_FLAIL				13	/* 2d6 */
#define SV_LEAD_FILLED_MACE		15	/* 3d4 */
#define SV_TWO_HANDED_FLAIL		18	/* 3d6 */
#define SV_MACE_OF_DISRUPTION	20	/* 5d8 */
#define SV_GROND				50	/* 3d4 */

/* The "sval" values for TV_POLEARM */
#define SV_SPEAR				2	/* 1d6 */
#define SV_AWL_PIKE				4	/* 1d8 */
#define SV_TRIDENT				5	/* 1d9 */
#define SV_PIKE					8	/* 2d5 */
#define SV_BEAKED_AXE			10	/* 2d6 */
#define SV_BROAD_AXE			11	/* 2d6 */
#define SV_GLAIVE				13	/* 2d6 */
#define SV_HALBERD				15	/* 3d4 */
#define SV_SCYTHE				17	/* 5d3 */
#define SV_LANCE				20	/* 2d8 */
#define SV_BATTLE_AXE			22	/* 2d8 */
#define SV_GREAT_AXE			25	/* 4d4 */
#define SV_LOCHABER_AXE			28	/* 3d8 */
#define SV_SCYTHE_OF_SLICING	30	/* 8d4 */

/* The "sval" codes for TV_SWORD */
#define SV_BROKEN_DAGGER		1	/* 1d1 */
#define SV_BROKEN_SWORD			2	/* 1d2 */
#define SV_DAGGER				4	/* 1d4 */
#define SV_MAIN_GAUCHE			5	/* 1d5 */
#define SV_RAPIER				7	/* 1d6 */
#define SV_SMALL_SWORD			8	/* 1d6 */
#define SV_SHORT_SWORD			10	/* 1d7 */
#define SV_SABRE				11	/* 1d7 */
#define SV_CUTLASS				12	/* 1d7 */
#define SV_TULWAR				15	/* 2d4 */
#define SV_BROAD_SWORD			16	/* 2d5 */
#define SV_LONG_SWORD			17	/* 2d5 */
#define SV_SCIMITAR				18	/* 2d5 */
#define SV_KATANA				20	/* 3d4 */
#define SV_BASTARD_SWORD		21	/* 3d4 */
#define SV_TWO_HANDED_SWORD		25	/* 3d6 */
#define SV_EXECUTIONERS_SWORD	28	/* 4d5 */
#define SV_BLADE_OF_CHAOS		30	/* 6d5 */

/* The "sval" codes for TV_SHIELD */
#define SV_SMALL_LEATHER_SHIELD		2
#define SV_SMALL_METAL_SHIELD		3
#define SV_LARGE_LEATHER_SHIELD		4
#define SV_LARGE_METAL_SHIELD		5
#define SV_ORCISH_SHIELD                7
#define SV_SHIELD_OF_DEFLECTION		10

/* The "sval" codes for TV_HELM */
#define SV_HARD_LEATHER_CAP		2
#define SV_METAL_CAP			3
#define SV_IRON_HELM			5
#define SV_STEEL_HELM			6
#define SV_IRON_CROWN			10
#define SV_GOLDEN_CROWN			11
#define SV_JEWELED_CROWN		12
#define SV_MORGOTH				50

/* The "sval" codes for TV_BOOTS */
#define SV_PAIR_OF_SOFT_LEATHER_BOOTS	2
#define SV_PAIR_OF_HARD_LEATHER_BOOTS	3
#define SV_PAIR_OF_METAL_SHOD_BOOTS     6
#define SV_PAIR_OF_WITAN_BOOTS          8

/* The "sval" codes for TV_CLOAK */
#define SV_CLOAK					1
#define SV_KOLLA                                        3
#define SV_SHADOW_CLOAK				6

/* The "sval" codes for TV_GLOVES */
#define SV_SET_OF_LEATHER_GLOVES	1
#define SV_SET_OF_GAUNTLETS			2
#define SV_SET_OF_ELVEN_GLOVES			4
#define SV_SET_OF_CESTI				5

/* The "sval" codes for TV_SOFT_ARMOR */
#define SV_FILTHY_RAG				1
#define SV_ROBE						2
#define SV_SOFT_LEATHER_ARMOR		4
#define SV_SOFT_STUDDED_LEATHER		5
#define SV_HARD_LEATHER_ARMOR		6
#define SV_HARD_STUDDED_LEATHER		7
#define SV_LEATHER_SCALE_MAIL		11

/* The "sval" codes for TV_HARD_ARMOR */
#define SV_RUSTY_CHAIN_MAIL			1	/* 14- */
#define SV_METAL_SCALE_MAIL			3	/* 13 */
#define SV_CHAIN_MAIL				4	/* 14 */
#define SV_AUGMENTED_CHAIN_MAIL		6	/* 16 */
#define SV_DOUBLE_CHAIN_MAIL		7	/* 16 */
#define SV_BAR_CHAIN_MAIL			8	/* 18 */
#define SV_METAL_BRIGANDINE_ARMOUR	9	/* 19 */
#define SV_PARTIAL_PLATE_ARMOUR		12	/* 22 */
#define SV_METAL_LAMELLAR_ARMOUR	13	/* 23 */
#define SV_FULL_PLATE_ARMOUR		15	/* 25 */
#define SV_RIBBED_PLATE_ARMOUR		18	/* 28 */
#define SV_MITHRIL_CHAIN_MAIL		20	/* 28+ */
#define SV_MITHRIL_PLATE_MAIL		25	/* 35+ */
#define SV_ADAMANTITE_PLATE_MAIL	30	/* 40+ */

/* The "sval" codes for TV_DRAG_ARMOR */
#define SV_DRAGON_BLACK			1
#define SV_DRAGON_BLUE			2
#define SV_DRAGON_WHITE			3
#define SV_DRAGON_RED			4
#define SV_DRAGON_GREEN			5
#define SV_DRAGON_MULTIHUED		6
#define SV_DRAGON_SHINING		10
#define SV_DRAGON_LAW			12
#define SV_DRAGON_BRONZE		14
#define SV_DRAGON_GOLD			16
#define SV_DRAGON_CHAOS			18
#define SV_DRAGON_BALANCE		20
#define SV_DRAGON_POWER			30

/* The sval codes for TV_LITE */
#define SV_LITE_TORCH		0
#define SV_LITE_LANTERN		1
#define SV_LITE_DWARVEN		2
#define SV_LITE_FEANOR		3
#define SV_LITE_GALADRIEL	4
#define SV_LITE_ELENDIL		5
#define SV_LITE_THRAIN		6
#define SV_LITE_PALANTIR		7

/* The "sval" codes for TV_AMULET */
#define SV_AMULET_DOOM			0
#define SV_AMULET_TELEPORT		1
#define SV_AMULET_ADORNMENT		2
#define SV_AMULET_SLOW_DIGEST	3
#define SV_AMULET_RESIST_ACID	4
#define SV_AMULET_SEARCHING		5
#define SV_AMULET_WISDOM		6
#define SV_AMULET_CHARISMA		7
#define SV_AMULET_THE_MAGI		8
#define SV_AMULET_SUSTENANCE		9
#define SV_AMULET_CARLAMMAS		10
#define SV_AMULET_INGWE			11
#define SV_AMULET_DWARVES		12
#define SV_AMULET_ESP			13
#define SV_AMULET_RESIST		14
#define SV_AMULET_REGEN			15
#define SV_AMULET_ELESSAR		16
#define SV_AMULET_EVENSTAR		17
#define SV_AMULET_DEVOTION		18
#define SV_AMULET_WEPMASTERY		19
#define SV_AMULET_TRICKERY		20
#define SV_AMULET_INFRA			21
#define SV_AMULET_RESIST_ELEC		22
#define SV_AMULET_THE_MOON              23
#define SV_AMULET_TERKEN		24
#define SV_AMULET_SPEED			25

/* The sval codes for TV_RING */
#define SV_RING_WOE				0
#define SV_RING_AGGRAVATION		1
#define SV_RING_WEAKNESS		2
#define SV_RING_STUPIDITY		3
#define SV_RING_TELEPORTATION	4
#define SV_RING_SLOW_DIGESTION	6
#define SV_RING_FEATHER_FALL	7
#define SV_RING_RESIST_FIRE		8
#define SV_RING_RESIST_COLD		9
#define SV_RING_SUSTAIN_STR		10
#define SV_RING_SUSTAIN_INT		11
#define SV_RING_SUSTAIN_WIS		12
#define SV_RING_SUSTAIN_DEX		13
#define SV_RING_SUSTAIN_CON		14
#define SV_RING_SUSTAIN_CHR		15
#define SV_RING_PROTECTION		16
#define SV_RING_ACID			17
#define SV_RING_FLAMES			18
#define SV_RING_ICE				19
#define SV_RING_RESIST_POIS		20
#define SV_RING_FREE_ACTION		21
#define SV_RING_SEE_INVIS		22
#define SV_RING_SEARCHING		23
#define SV_RING_STR				24
#define SV_RING_INT				25
#define SV_RING_DEX				26
#define SV_RING_CON				27
#define SV_RING_ACCURACY		28
#define SV_RING_DAMAGE			29
#define SV_RING_SLAYING			30
#define SV_RING_SPEED			31
#define SV_RING_BARAHIR			32
#define SV_RING_TULKAS			33
#define SV_RING_NARYA			34
#define SV_RING_NENYA			35
#define SV_RING_VILYA			36
#define SV_RING_POWER			37
#define SV_RING_LIGHTNING		38

/* The "sval" codes for TV_STAFF */
#define SV_STAFF_DARKNESS		0
#define SV_STAFF_SLOWNESS		1
#define SV_STAFF_HASTE_MONSTERS	2
#define SV_STAFF_SUMMONING		3
#define SV_STAFF_TELEPORTATION	4
#define SV_STAFF_IDENTIFY		5
#define SV_STAFF_REMOVE_CURSE	6
#define SV_STAFF_STARLITE		7
#define SV_STAFF_LITE			8
#define SV_STAFF_MAPPING		9
#define SV_STAFF_DETECT_GOLD	10
#define SV_STAFF_DETECT_ITEM	11
#define SV_STAFF_DETECT_TRAP	12
#define SV_STAFF_DETECT_DOOR	13
#define SV_STAFF_DETECT_INVIS	14
#define SV_STAFF_DETECT_EVIL	15
#define SV_STAFF_CURE_LIGHT		16
#define SV_STAFF_CURING			17
#define SV_STAFF_HEALING		18
#define SV_STAFF_THE_MAGI		19
#define SV_STAFF_SLEEP_MONSTERS	20
#define SV_STAFF_SLOW_MONSTERS	21
#define SV_STAFF_SPEED			22
#define SV_STAFF_PROBING		23
#define SV_STAFF_DISPEL_EVIL	24
#define SV_STAFF_POWER			25
#define SV_STAFF_HOLINESS		26
#define SV_STAFF_BANISHMENT		27
#define SV_STAFF_EARTHQUAKES	28
#define SV_STAFF_DESTRUCTION	29

/* The "sval" codes for TV_WAND */
#define SV_WAND_HEAL_MONSTER	0
#define SV_WAND_HASTE_MONSTER	1
#define SV_WAND_CLONE_MONSTER	2
#define SV_WAND_TELEPORT_AWAY	3
#define SV_WAND_DISARMING		4
#define SV_WAND_TRAP_DOOR_DEST	5
#define SV_WAND_STONE_TO_MUD	6
#define SV_WAND_LITE			7
#define SV_WAND_SLEEP_MONSTER	8
#define SV_WAND_SLOW_MONSTER	9
#define SV_WAND_CONFUSE_MONSTER	10
#define SV_WAND_FEAR_MONSTER	11
#define SV_WAND_DRAIN_LIFE		12
#define SV_WAND_POLYMORPH		13
#define SV_WAND_STINKING_CLOUD	14
#define SV_WAND_MAGIC_MISSILE	15
#define SV_WAND_ACID_BOLT		16
#define SV_WAND_ELEC_BOLT		17
#define SV_WAND_FIRE_BOLT		18
#define SV_WAND_COLD_BOLT		19
#define SV_WAND_ACID_BALL		20
#define SV_WAND_ELEC_BALL		21
#define SV_WAND_FIRE_BALL		22
#define SV_WAND_COLD_BALL		23
#define SV_WAND_WONDER			24
#define SV_WAND_ANNIHILATION	25
#define SV_WAND_DRAGON_FIRE		26
#define SV_WAND_DRAGON_COLD		27
#define SV_WAND_DRAGON_BREATH	28

/* The "sval" codes for TV_ROD */
#define SV_ROD_DETECT_TRAP		0
#define SV_ROD_DETECT_DOOR		1
#define SV_ROD_IDENTIFY			2
#define SV_ROD_RECALL			3
#define SV_ROD_ILLUMINATION		4
#define SV_ROD_MAPPING			5
#define SV_ROD_DETECTION		6
#define SV_ROD_PROBING			7
#define SV_ROD_CURING			8
#define SV_ROD_HEALING			9
#define SV_ROD_RESTORATION		10
#define SV_ROD_SPEED			11
#define SV_ROD_TELEPORT_AWAY	13
#define SV_ROD_DISARMING		14
#define SV_ROD_LITE				15
#define SV_ROD_SLEEP_MONSTER	16
#define SV_ROD_SLOW_MONSTER		17
#define SV_ROD_DRAIN_LIFE		18
#define SV_ROD_POLYMORPH		19
#define SV_ROD_ACID_BOLT		20
#define SV_ROD_ELEC_BOLT		21
#define SV_ROD_FIRE_BOLT		22
#define SV_ROD_COLD_BOLT		23
#define SV_ROD_ACID_BALL		24
#define SV_ROD_ELEC_BALL		25
#define SV_ROD_FIRE_BALL		26
#define SV_ROD_COLD_BALL		27

/* The "sval" codes for TV_SCROLL */
#define SV_SCROLL_DARKNESS				0
#define SV_SCROLL_AGGRAVATE_MONSTER		1
#define SV_SCROLL_CURSE_ARMOR			2
#define SV_SCROLL_CURSE_WEAPON			3
#define SV_SCROLL_SUMMON_MONSTER		4
#define SV_SCROLL_SUMMON_UNDEAD			5
#define SV_SCROLL_CREATE_ARTIFACT 		6
#define SV_SCROLL_TRAP_CREATION			7
#define SV_SCROLL_PHASE_DOOR			8
#define SV_SCROLL_TELEPORT				9
#define SV_SCROLL_TELEPORT_LEVEL		10
#define SV_SCROLL_WORD_OF_RECALL		11
#define SV_SCROLL_IDENTIFY				12
#define SV_SCROLL_STAR_IDENTIFY			13
#define SV_SCROLL_REMOVE_CURSE			14
#define SV_SCROLL_STAR_REMOVE_CURSE		15
#define SV_SCROLL_ENCHANT_ARMOR			16
#define SV_SCROLL_ENCHANT_WEAPON_TO_HIT	17
#define SV_SCROLL_ENCHANT_WEAPON_TO_DAM	18
#define SV_SCROLL_STAR_ENCHANT_ARMOR	20
#define SV_SCROLL_STAR_ENCHANT_WEAPON	21
#define SV_SCROLL_RECHARGING			22
#define SV_SCROLL_LIGHT					24
#define SV_SCROLL_MAPPING				25
#define SV_SCROLL_DETECT_GOLD			26
#define SV_SCROLL_DETECT_ITEM			27
#define SV_SCROLL_DETECT_TRAP			28
#define SV_SCROLL_DETECT_DOOR			29
#define SV_SCROLL_DETECT_INVIS			30
#define SV_SCROLL_SATISFY_HUNGER		32
#define SV_SCROLL_BLESSING				33
#define SV_SCROLL_HOLY_CHANT			34
#define SV_SCROLL_HOLY_PRAYER			35
#define SV_SCROLL_MONSTER_CONFUSION		36
#define SV_SCROLL_PROTECTION_FROM_EVIL	37
#define SV_SCROLL_RUNE_OF_PROTECTION	38
#define SV_SCROLL_TRAP_DOOR_DESTRUCTION	39
#define SV_SCROLL_STAR_DESTRUCTION		41
#define SV_SCROLL_DISPEL_UNDEAD			42
#define SV_SCROLL_BANISHMENT			44
#define SV_SCROLL_MASS_BANISHMENT		45
#define SV_SCROLL_ACQUIREMENT			46
#define SV_SCROLL_STAR_ACQUIREMENT		47
#define SV_SCROLL_LIFE				48
#define SV_SCROLL_CREATE_HOUSE			49

/* The "sval" codes for TV_POTION */
#define SV_POTION_WATER				0
#define SV_POTION_APPLE_JUICE		1
#define SV_POTION_SLIME_MOLD		2
#define SV_POTION_SLOWNESS			4
#define SV_POTION_SALT_WATER		5
#define SV_POTION_POISON			6
#define SV_POTION_BLINDNESS			7
#define SV_POTION_CONFUSION			9
#define SV_POTION_SLEEP				11
#define SV_POTION_LOSE_MEMORIES		13
#define SV_POTION_RUINATION			15
#define SV_POTION_DEC_STR			16
#define SV_POTION_DEC_INT			17
#define SV_POTION_DEC_WIS			18
#define SV_POTION_DEC_DEX			19
#define SV_POTION_DEC_CON			20
#define SV_POTION_DEC_CHR			21
#define SV_POTION_DETONATIONS		22
#define SV_POTION_DEATH				23
#define SV_POTION_INFRAVISION		24
#define SV_POTION_DETECT_INVIS		25
#define SV_POTION_SLOW_POISON		26
#define SV_POTION_CURE_POISON		27
#define SV_POTION_BOLDNESS			28
#define SV_POTION_SPEED				29
#define SV_POTION_RESIST_HEAT		30
#define SV_POTION_RESIST_COLD		31
#define SV_POTION_HEROISM			32
#define SV_POTION_BERSERK_STRENGTH	33
#define SV_POTION_CURE_LIGHT		34
#define SV_POTION_CURE_SERIOUS		35
#define SV_POTION_CURE_CRITICAL		36
#define SV_POTION_HEALING			37
#define SV_POTION_STAR_HEALING		38
#define SV_POTION_LIFE				39
#define SV_POTION_RESTORE_MANA		40
#define SV_POTION_RESTORE_EXP		41
#define SV_POTION_RES_STR			42
#define SV_POTION_RES_INT			43
#define SV_POTION_RES_WIS			44
#define SV_POTION_RES_DEX			45
#define SV_POTION_RES_CON			46
#define SV_POTION_RES_CHR			47
#define SV_POTION_INC_STR			48
#define SV_POTION_INC_INT			49
#define SV_POTION_INC_WIS			50
#define SV_POTION_INC_DEX			51
#define SV_POTION_INC_CON			52
#define SV_POTION_INC_CHR			53
#define SV_POTION_AUGMENTATION			55
#define SV_POTION_ENLIGHTENMENT			56
#define SV_POTION_STAR_ENLIGHTENMENT	57
#define SV_POTION_SELF_KNOWLEDGE		58
#define SV_POTION_EXPERIENCE			59

/* The "sval" codes for TV_FOOD */
#define SV_FOOD_POISON			0
#define SV_FOOD_BLINDNESS		1
#define SV_FOOD_PARANOIA		2
#define SV_FOOD_CONFUSION		3
#define SV_FOOD_HALLUCINATION	4
#define SV_FOOD_PARALYSIS		5
#define SV_FOOD_WEAKNESS		6
#define SV_FOOD_SICKNESS		7
#define SV_FOOD_STUPIDITY		8
#define SV_FOOD_NAIVETY			9
#define SV_FOOD_UNHEALTH		10
#define SV_FOOD_DISEASE			11
#define SV_FOOD_CURE_POISON		12
#define SV_FOOD_CURE_BLINDNESS	13
#define SV_FOOD_CURE_PARANOIA	14
#define SV_FOOD_CURE_CONFUSION	15
#define SV_FOOD_CURE_SERIOUS	16
#define SV_FOOD_RESTORE_STR		17
#define SV_FOOD_RESTORE_CON		18
#define SV_FOOD_RESTORING		19

/* crops */
#define	SV_FOOD_POTATO			20
#define SV_FOOD_HEAD_OF_CABBAGE		21
#define SV_FOOD_CARROT			22
#define SV_FOOD_BEET			23
#define	SV_FOOD_SQUASH			24
#define	SV_FOOD_EAR_OF_CORN		25

#define SV_FOOD_BISCUIT			32
#define SV_FOOD_JERKY			33
#define SV_FOOD_RATION			35
#define SV_FOOD_SLIME_MOLD		36
#define SV_FOOD_WAYBREAD		37
#define SV_FOOD_PINT_OF_ALE		38
#define SV_FOOD_PINT_OF_WINE		39

/* The "sval" codes for TV_JUNK */
#define SV_HOUSE_FOUNDATION		9

/*
 * Special "sval" limit -- first "normal" food
 */
#define SV_FOOD_MIN_FOOD	20

/*
 * Special "sval" limit -- first "aimed" rod
 */
#define SV_ROD_MIN_DIRECTION	12

/*
 * Special "sval" limit -- first "large" chest
 */
#define SV_CHEST_MIN_LARGE	4

/*
 * Special "sval" limit -- first "good" magic/prayer book
 */
#define SV_BOOK_MIN_GOOD	4


/*
 * Special "sval" value -- unknown "sval"
 */
#define SV_UNKNOWN			255


/*
 * Some bit-flags for the "smart" field
 */
#define SM_RES_ACID		0x00000001
#define SM_RES_ELEC		0x00000002
#define SM_RES_FIRE		0x00000004
#define SM_RES_COLD		0x00000008
#define SM_RES_POIS		0x00000010
#define SM_RES_NETH		0x00000020
#define SM_RES_LITE		0x00000040
#define SM_RES_DARK		0x00000080
#define SM_RES_FEAR		0x00000100
#define SM_RES_CONF		0x00000200
#define SM_RES_CHAOS	0x00000400
#define SM_RES_DISEN	0x00000800
#define SM_RES_BLIND	0x00001000
#define SM_RES_NEXUS	0x00002000
#define SM_RES_SOUND	0x00004000
#define SM_RES_SHARD	0x00008000
#define SM_OPP_ACID		0x00010000
#define SM_OPP_ELEC		0x00020000
#define SM_OPP_FIRE		0x00040000
#define SM_OPP_COLD		0x00080000
#define SM_OPP_POIS		0x00100000
#define SM_OPP_XXX1		0x00200000
#define SM_OPP_XXX2		0x00400000
#define SM_OPP_XXX3		0x00800000
#define SM_IMM_ACID		0x01000000
#define SM_IMM_ELEC		0x02000000
#define SM_IMM_FIRE		0x04000000
#define SM_IMM_COLD		0x08000000
#define SM_IMM_XXX5		0x10000000
#define SM_IMM_XXX6		0x20000000
#define SM_IMM_FREE		0x40000000
#define SM_IMM_MANA		0x80000000

/*** General flag values ***/


/*
 * Special cave grid flags
 */
#define CAVE_MARK	0x01 	/* memorized feature */
#define CAVE_GLOW	0x02 	/* self-illuminating */
#define CAVE_ICKY	0x04 	/* part of a vault */
#define CAVE_ROOM	0x08 	/* part of a room */
#define CAVE_LITE	0x10 	/* lite flag  */
#define CAVE_VIEW	0x20 	/* view flag */
#define CAVE_TEMP	0x40 	/* temp flag */
#define CAVE_XTRA	0x80 	/* misc flag */



/*
 * Bit flags for the "project()" function
 *
 *   JUMP: Jump directly to the target location (this is a hack)
 *   BEAM: Work as a beam weapon (affect every grid passed through)
 *   THRU: Continue "through" the target (used for "bolts"/"beams")
 *   STOP: Stop as soon as we hit a monster (used for "bolts")
 *   GRID: Affect each grid in the "blast area" in some way
 *   ITEM: Affect each object in the "blast area" in some way
 *   KILL: Affect each monster in the "blast area" in some way
 *   HIDE: Hack -- disable "visual" feedback from projection
 */
#define PROJECT_JUMP	0x01
#define PROJECT_BEAM	0x02
#define PROJECT_THRU	0x04
#define PROJECT_STOP	0x08
#define PROJECT_GRID	0x10
#define PROJECT_ITEM	0x20
#define PROJECT_KILL	0x40
#define PROJECT_HIDE	0x80

/*
 * Bit flags for the "enchant()" function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOAC    0x04

/*** General index values ***/


/*
 * Spell types used by project(), and related functions.
 */
#define GF_ELEC         1
#define GF_POIS         2
#define GF_ACID         3
#define GF_COLD         4
#define GF_FIRE         5
#define GF_BOULDER      8  /* Thrown boulder */
#define GF_BOLT         9  /* Crossbow shot */
#define GF_MISSILE      10 /* Magic missile */
#define GF_ARROW        11
#define GF_PLASMA       12
#define GF_HOLY_ORB     13
#define GF_WATER        14
#define GF_LITE         15
#define GF_DARK         16
#define GF_LITE_WEAK	17
#define GF_DARK_WEAK	18
#define GF_SHARDS       20
#define GF_SOUND        21
#define GF_CONFUSION    22
#define GF_FORCE        23
#define GF_INERT        24 /* GF_INERTIA */
#define GF_MANA         26
#define GF_METEOR       27
#define GF_ICE          28
#define GF_CHAOS        30
#define GF_NETHER       31
#define GF_DISENCHANT   32
#define GF_NEXUS        33
#define GF_TIME         34
#define GF_GRAVITY      35
#define GF_KILL_WALL	40
#define GF_KILL_DOOR	41
#define GF_KILL_TRAP	42
#define GF_MAKE_WALL	45
#define GF_MAKE_DOOR	46
#define GF_MAKE_TRAP	47
#define GF_OLD_CLONE	51
#define GF_OLD_POLY		52
#define GF_OLD_HEAL		53
#define GF_OLD_SPEED	54
#define GF_OLD_SLOW		55
#define GF_OLD_CONF		56
#define GF_OLD_SLEEP	57
#define GF_OLD_DRAIN	58
#define GF_AWAY_UNDEAD	61
#define GF_AWAY_EVIL	62
#define GF_AWAY_ALL	63
#define GF_TURN_UNDEAD	64
#define GF_TURN_EVIL	65
#define GF_TURN_ALL	66
#define GF_DISP_UNDEAD	67
#define GF_DISP_EVIL	68
#define GF_DISP_ALL	69
#define	GF_HEAL_PLAYER	70
#define GF_PROJECT_SPELL 71
#define GF_PROJECT_PRAYER 72

#define GF_MAX 73

/*
 * Some things which induce learning
 */
#define DRS_ACID	1
#define DRS_ELEC	2
#define DRS_FIRE	3
#define DRS_COLD	4
#define DRS_POIS	5
#define DRS_NETH	6
#define DRS_LITE	7
#define DRS_DARK	8
#define DRS_FEAR	9
#define DRS_CONF	10
#define DRS_CHAOS	11
#define DRS_DISEN	12
#define DRS_BLIND	13
#define DRS_NEXUS	14
#define DRS_SOUND	15
#define DRS_SHARD	16
#define DRS_FREE	30
#define DRS_MANA	31



/*
 * Hack -- first "normal" artifact in the artifact list.  All of
 * the artifacts with indexes from 1 to 15 are "special" (lights,
 * rings, amulets), and the ones from 16 to 127 are "normal".
 */
#define ART_MIN_NORMAL		16


/*
 * Hack -- special "xtra" object powers
 */

/* Sustain one stat */
#define EGO_XTRA_SUSTAIN	1

/* High resist */
#define EGO_XTRA_POWER		2

/* Special ability */
#define EGO_XTRA_ABILITY	3





/*** Object flag values ***/


/*
 * Chest trap flags (see "tables.c")
 */
#define CHEST_LOSE_STR		0x01
#define CHEST_LOSE_CON		0x02
#define CHEST_POISON		0x04
#define CHEST_PARALYZE		0x08
#define CHEST_EXPLODE		0x10
#define CHEST_SUMMON		0x20



/*
 * Special "Item Flags"
 */
#define ID_SENSE	0x01	/* Item has been "sensed" */
#define ID_FIXED	0x02	/* Item has been "haggled" */
#define ID_EMPTY	0x04	/* Item charges are known */
#define ID_KNOWN	0x08	/* Item abilities are known */
#define ID_RUMOUR	0x10	/* Item background is known */
#define ID_MENTAL	0x20	/* Item information is known */
#define ID_CURSED	0x40	/* Item is temporarily cursed */
#define ID_BROKEN	0x80	/* Item is permanently worthless */



/*
 * As of 2.7.8, the "object flags" are valid for all objects, and as
 * of 2.7.9, these flags are not actually stored with the object.
 *
 * Note that "flags1" contains all flags dependant on "pval" (including
 * stat bonuses, but NOT stat sustainers), plus all "extra attack damage"
 * flags (SLAY_XXX and BRAND_XXX).
 *
 * Note that "flags2" contains all "resistances" (including "Stat Sustainers",
 * actual immunities, and resistances).  Note that "Hold Life" is really an
 * "immunity" to ExpLoss, and "Free Action" is "immunity to paralysis".
 *
 * Note that "flags3" contains everything else -- including the three "CURSED"
 * flags, and the "BLESSED" flag, several "item display" parameters, some new
 * flags for powerful Bows, and flags which affect the player in a "general"
 * way (LITE, TELEPATHY, SEE_INVIS, SLOW_DIGEST, REGEN, FEATHER), including
 * all the "general" curses (TELEPORT, AGGRAVATE, EXP_DRAIN).  It also has
 * four new flags called "ITEM_IGNORE_XXX" which lets an item specify that
 * it can not be affected by various forms of destruction.  This is NOT as
 * powerful as actually granting resistance/immunity to the wearer.
 */

#define TR1_STR             0x00000001L /* STR += "pval" */
#define TR1_INT             0x00000002L /* INT += "pval" */
#define TR1_WIS             0x00000004L /* WIS += "pval" */
#define TR1_DEX             0x00000008L /* DEX += "pval" */
#define TR1_CON             0x00000010L /* CON += "pval" */
#define TR1_CHR             0x00000020L /* CHR += "pval" */
#define TR1_XXX1            0x00000040L /* (reserved) */
#define TR1_XXX2            0x00000080L /* (reserved) */
#define TR1_STEALTH         0x00000100L /* Stealth += "pval" */
#define TR1_SEARCH          0x00000200L /* Search += "pval" */
#define TR1_INFRA           0x00000400L /* Infra += "pval" */
#define TR1_TUNNEL          0x00000800L /* Tunnel += "pval" */
#define TR1_SPEED           0x00001000L /* Speed += "pval" */
#define TR1_BLOWS           0x00002000L /* Blows += "pval" */
#define TR1_SHOTS           0x00004000L /* Shots += "pval" */
#define TR1_MIGHT           0x00008000L /* Might += "pval" */
#define TR1_SLAY_ANIMAL     0x00010000L /* Weapon slays animals */
#define TR1_SLAY_EVIL       0x00020000L /* Weapon slays evil */
#define TR1_SLAY_UNDEAD     0x00040000L /* Weapon slays undead */
#define TR1_SLAY_DEMON      0x00080000L /* Weapon slays demon */
#define TR1_SLAY_ORC        0x00100000L /* Weapon slays orc */
#define TR1_SLAY_TROLL      0x00200000L /* Weapon slays troll */
#define TR1_SLAY_GIANT      0x00400000L /* Weapon slays giant */
#define TR1_SLAY_DRAGON     0x00800000L /* Weapon slays dragon */
#define TR1_KILL_DRAGON     0x01000000L /* Weapon kills dragon */
#define TR1_KILL_DEMON      0x02000000L /* Weapon kills demon */
#define TR1_KILL_UNDEAD     0x04000000L /* Weapon "kills" undead */
#define TR1_BRAND_POIS      0x08000000L /* Weapon has poison brand */
#define TR1_BRAND_ACID      0x10000000L /* Weapon has acid brand */
#define TR1_BRAND_ELEC      0x20000000L /* Weapon has elec brand */
#define TR1_BRAND_FIRE      0x40000000L /* Weapon has fire brand */
#define TR1_BRAND_COLD      0x80000000L /* Weapon has cold brand */

#define TR2_SUST_STR        0x00000001L /* Sustain STR */
#define TR2_SUST_INT        0x00000002L /* Sustain INT */
#define TR2_SUST_WIS        0x00000004L /* Sustain WIS */
#define TR2_SUST_DEX        0x00000008L /* Sustain DEX */
#define TR2_SUST_CON        0x00000010L /* Sustain CON */
#define TR2_SUST_CHR        0x00000020L /* Sustain CHR */
#define TR2_XXX1            0x00000040L /* (reserved) */
#define TR2_XXX2            0x00000080L /* (reserved) */
#define TR2_XXX3            0x00000100L /* (reserved) */
#define TR2_XXX4            0x00000200L /* (reserved) */
#define TR2_XXX5            0x00000400L /* (reserved) */
#define TR2_XXX6            0x00000800L /* (reserved) */
#define TR2_IM_ACID         0x00001000L /* Immunity to acid */
#define TR2_IM_ELEC         0x00002000L /* Immunity to elec */
#define TR2_IM_FIRE         0x00004000L /* Immunity to fire */
#define TR2_IM_COLD         0x00008000L /* Immunity to cold */
#define TR2_RES_ACID        0x00010000L /* Resist acid */
#define TR2_RES_ELEC        0x00020000L /* Resist elec */
#define TR2_RES_FIRE        0x00040000L /* Resist fire */
#define TR2_RES_COLD        0x00080000L /* Resist cold */
#define TR2_RES_POIS        0x00100000L /* Resist poison */
#define TR2_RES_FEAR        0x00200000L /* Resist fear */
#define TR2_RES_LITE        0x00400000L /* Resist lite */
#define TR2_RES_DARK        0x00800000L /* Resist dark */
#define TR2_RES_BLIND       0x01000000L /* Resist blind */
#define TR2_RES_CONFU       0x02000000L /* Resist confusion */
#define TR2_RES_SOUND       0x04000000L /* Resist sound */
#define TR2_RES_SHARD       0x08000000L /* Resist shards */
#define TR2_RES_NEXUS       0x10000000L /* Resist nexus */
#define TR2_RES_NETHR       0x20000000L /* Resist nether */
#define TR2_RES_CHAOS       0x40000000L /* Resist chaos */
#define TR2_RES_DISEN       0x80000000L /* Resist disenchant */

#define TR3_SLOW_DIGEST     0x00000001L /* Slow digest */
#define TR3_FEATHER         0x00000002L /* Feather Falling */
#define TR3_LITE            0x00000004L /* Perma-Lite */
#define TR3_REGEN           0x00000008L /* Regeneration */
#define TR3_TELEPATHY       0x00000010L /* Telepathy */
#define TR3_SEE_INVIS       0x00000020L /* See Invis */
#define TR3_FREE_ACT        0x00000040L /* Free action */
#define TR3_HOLD_LIFE       0x00000080L /* Hold life */
#define TR3_XXX1            0x00000100L
#define TR3_XXX2            0x00000200L
#define TR3_XXX3            0x00000400L
#define TR3_XXX4            0x00000800L
#define TR3_IMPACT          0x00001000L /* Earthquake blows */
#define TR3_TELEPORT        0x00002000L /* Random teleportation */
#define TR3_AGGRAVATE       0x00004000L /* Aggravate monsters */
#define TR3_DRAIN_EXP       0x00008000L /* Experience drain */
#define TR3_IGNORE_ACID     0x00010000L /* Item ignores Acid Damage */
#define TR3_IGNORE_ELEC     0x00020000L /* Item ignores Elec Damage */
#define TR3_IGNORE_FIRE     0x00040000L /* Item ignores Fire Damage */
#define TR3_IGNORE_COLD     0x00080000L /* Item ignores Cold Damage */
#define TR3_XXX5            0x00100000L /* (reserved) */
#define TR3_XXX6            0x00200000L /* (reserved) */
#define TR3_BLESSED         0x00400000L /* Item has been blessed */
#define TR3_ACTIVATE        0x00800000L /* Item can be activated */
#define TR3_INSTA_ART       0x01000000L /* Item makes an artifact */
#define TR3_EASY_KNOW       0x02000000L /* Item is known if aware */
#define TR3_HIDE_TYPE       0x04000000L /* Item hides description */
#define TR3_SHOW_MODS       0x08000000L /* Item shows Tohit/Todam */
#define TR3_XXX7            0x10000000L /* (reserved) */
#define TR3_LIGHT_CURSE     0x20000000L /* Item has Light Curse */
#define TR3_HEAVY_CURSE     0x40000000L /* Item has Heavy Curse */
#define TR3_PERMA_CURSE     0x80000000L /* Item has Perma Curse */


/*
 * Hack -- flag set 1 -- mask for "pval-dependant" flags.
 * Note that all "pval" dependant flags must be in "flags1".
 */
#define TR1_PVAL_MASK \
	(TR1_STR | TR1_INT | TR1_WIS | TR1_DEX | \
	 TR1_CON | TR1_CHR | TR1_XXX1 | TR1_XXX2 | \
	 TR1_STEALTH | TR1_SEARCH | TR1_INFRA | TR1_TUNNEL | \
	 TR1_SPEED | TR1_BLOWS | TR1_SHOTS | TR1_MIGHT)


/*
 * Flag set 3 -- mask for "ignore element" flags.
 */
#define TR3_IGNORE_MASK \
	(TR3_IGNORE_ACID | TR3_IGNORE_ELEC | TR3_IGNORE_FIRE | \
	 TR3_IGNORE_COLD )

/*
 * Hack -- special "xtra" object flag info (type)
 */
#define OBJECT_XTRA_TYPE_SUSTAIN	1
#define OBJECT_XTRA_TYPE_RESIST		2
#define OBJECT_XTRA_TYPE_POWER		3

/*
 * Hack -- special "xtra" object flag info (what flag set)
 */
#define OBJECT_XTRA_WHAT_SUSTAIN	2
#define OBJECT_XTRA_WHAT_RESIST		2
#define OBJECT_XTRA_WHAT_POWER		3

/*
 * Hack -- special "xtra" object flag info (base flag value)
 */
#define OBJECT_XTRA_BASE_SUSTAIN	TR2_SUST_STR
#define OBJECT_XTRA_BASE_RESIST		TR2_RES_POIS
#define OBJECT_XTRA_BASE_POWER		TR3_SLOW_DIGEST

/*
 * Hack -- special "xtra" object flag info (number of flags)
 */
#define OBJECT_XTRA_SIZE_SUSTAIN	6
#define OBJECT_XTRA_SIZE_RESIST		12
#define OBJECT_XTRA_SIZE_POWER		8


/*** Class flags ***/

#define CF_EXTRA_SHOT		0x00000001L	/* Extra shots */
#define CF_BRAVERY_30		0x00000002L	/* Gains resist fear at plev 30 */
#define CF_BLESS_WEAPON		0x00000004L	/* Requires blessed/hafted weapons */
#define CF_CUMBER_GLOVE		0x00000008L	/* Gloves disturb spellcasting */
#define CF_ZERO_FAIL		0x00000010L /* Fail rates can reach 0% */
#define CF_BEAM				0x00000020L /* Higher chance of spells beaming */
#define CF_CHOOSE_SPELLS	0x00000040L	/* Allow choice of spells */
#define CF_PSEUDO_ID_HEAVY	0x00000080L /* Allow heavy pseudo-id */
#define CF_PSEUDO_ID_IMPROV	0x00000100L /* Pseudo-id improves quicker with player-level */
#define CF_XXX10			0x00000200L
#define CF_XXX11			0x00000400L
#define CF_XXX12			0x00000800L
#define CF_XXX13			0x00001000L
#define CF_XXX14			0x00002000L
#define CF_XXX15			0x00004000L
#define CF_XXX16			0x00008000L
#define CF_BACK_STAB   		0x00010000L /* Extra damage to sleeping or fleeing creatures */
#define CF_STEALTH_MODE		0x00020000L /* Trade speed for stealth instead of searching */
#define CF_STEALING_IMPROV 	0x00040000L /* Stealing improves quicker with player-level */
#define CF_SPEED_BONUS 		0x00080000L /* Gain 1 additional speed point every 5 levels */
#define CF_HP_BONUS			0x00100000L /* Gain 1 additional hit point on levelup */
#define CF_XXX22			0x00200000L
#define CF_XXX23			0x00400000L
#define CF_XXX24			0x00800000L
#define CF_XXX25			0x01000000L
#define CF_XXX26			0x02000000L
#define CF_XXX27			0x04000000L
#define CF_XXX28			0x08000000L
#define CF_XXX29			0x10000000L
#define CF_XXX30			0x20000000L
#define CF_XXX31			0x40000000L
#define CF_XXX32			0x80000000L


/*** Monster blow constants ***/


/*
 * New monster blow methods
 */
#define RBM_HIT		1
#define RBM_TOUCH	2
#define RBM_PUNCH	3
#define RBM_KICK	4
#define RBM_CLAW	5
#define RBM_BITE	6
#define RBM_STING	7
#define RBM_XXX1	8
#define RBM_BUTT	9
#define RBM_CRUSH	10
#define RBM_ENGULF	11
#define RBM_XXX2	12
#define RBM_CRAWL	13
#define RBM_DROOL	14
#define RBM_SPIT	15
#define RBM_XXX3	16
#define RBM_GAZE	17
#define RBM_WAIL	18
#define RBM_SPORE	19
#define RBM_XXX4	20
#define RBM_BEG		21
#define RBM_INSULT	22
#define RBM_MOAN	23
#define RBM_XXX5	24


/*
 * New monster blow effects
 */
#define RBE_HURT		1
#define RBE_POISON		2
#define RBE_UN_BONUS	3
#define RBE_UN_POWER	4
#define RBE_EAT_GOLD	5
#define RBE_EAT_ITEM	6
#define RBE_EAT_FOOD	7
#define RBE_EAT_LITE	8
#define RBE_ACID		9
#define RBE_ELEC		10
#define RBE_FIRE		11
#define RBE_COLD		12
#define RBE_BLIND		13
#define RBE_CONFUSE		14
#define RBE_TERRIFY		15
#define RBE_PARALYZE	16
#define RBE_LOSE_STR	17
#define RBE_LOSE_INT	18
#define RBE_LOSE_WIS	19
#define RBE_LOSE_DEX	20
#define RBE_LOSE_CON	21
#define RBE_LOSE_CHR	22
#define RBE_LOSE_ALL	23
#define RBE_SHATTER		24
#define RBE_EXP_10		25
#define RBE_EXP_20		26
#define RBE_EXP_40		27
#define RBE_EXP_80		28
#define RBE_HALLU		29


/*** Monster flag values (hard-coded) ***/


/*
 * New monster race bit flags
 */
#define RF1_UNIQUE			0x00000001	/* Unique Monster */
#define RF1_QUESTOR			0x00000002	/* Quest Monster */
#define RF1_MALE			0x00000004	/* Male gender */
#define RF1_FEMALE			0x00000008	/* Female gender */
#define RF1_CHAR_CLEAR		0x00000010	/* Absorbs symbol */
#define RF1_CHAR_MULTI		0x00000020	/* Changes symbol */
#define RF1_ATTR_CLEAR		0x00000040	/* Absorbs color */
#define RF1_ATTR_MULTI		0x00000080	/* Changes color */
#define RF1_FORCE_DEPTH		0x00000100	/* Start at "correct" depth */
#define RF1_FORCE_MAXHP		0x00000200	/* Start with max hitpoints */
#define RF1_FORCE_SLEEP		0x00000400	/* Start out sleeping */
#define RF1_FORCE_EXTRA		0x00000800	/* Start out something */
#define RF1_FRIEND			0x00001000	/* Arrive with a friend */
#define RF1_FRIENDS			0x00002000	/* Arrive with some friends */
#define RF1_ESCORT			0x00004000	/* Arrive with an escort */
#define RF1_ESCORTS			0x00008000	/* Arrive with some escorts */
#define RF1_NEVER_BLOW		0x00010000	/* Never make physical blow */
#define RF1_NEVER_MOVE		0x00020000	/* Never make physical move */
#define RF1_RAND_25			0x00040000	/* Moves randomly (25%) */
#define RF1_RAND_50			0x00080000	/* Moves randomly (50%) */
#define RF1_ONLY_GOLD		0x00100000	/* Drop only gold */
#define RF1_ONLY_ITEM		0x00200000	/* Drop only items */
#define RF1_DROP_60			0x00400000	/* Drop an item/gold (60%) */
#define RF1_DROP_90			0x00800000	/* Drop an item/gold (90%) */
#define RF1_DROP_1D2		0x01000000	/* Drop 1d2 items/gold */
#define RF1_DROP_2D2		0x02000000	/* Drop 2d2 items/gold */
#define RF1_DROP_3D2		0x04000000	/* Drop 3d2 items/gold */
#define RF1_DROP_4D2		0x08000000	/* Drop 4d2 items/gold */
#define RF1_DROP_GOOD		0x10000000	/* Drop good items */
#define RF1_DROP_GREAT		0x20000000	/* Drop great items */
#define RF1_DROP_USEFUL		0x40000000	/* Drop "useful" items */
#define RF1_DROP_CHOSEN		0x80000000	/* Drop "chosen" items */

/*
 * New monster race bit flags
 */
#define RF2_STUPID			0x00000001	/* Monster is stupid */
#define RF2_SMART			0x00000002	/* Monster is smart */
#define RF2_XXX1			0x00000004	/* (?) */
#define RF2_XXX2			0x00000008	/* (?) */
#define RF2_INVISIBLE		0x00000010	/* Monster avoids vision */
#define RF2_COLD_BLOOD		0x00000020	/* Monster avoids infra */
#define RF2_EMPTY_MIND		0x00000040	/* Monster avoids telepathy */
#define RF2_WEIRD_MIND		0x00000080	/* Monster avoids telepathy? */
#define RF2_MULTIPLY		0x00000100	/* Monster reproduces */
#define RF2_REGENERATE		0x00000200	/* Monster regenerates */
#define RF2_XXX3			0x00000400	/* (?) */
#define RF2_XXX4			0x00000800	/* (?) */
#define RF2_POWERFUL		0x00001000	/* Monster has strong breath */
#define RF2_XXX5			0x00002000	/* (?) */
#define RF2_XXX7			0x00004000	/* (?) */
#define RF2_XXX6			0x00008000	/* (?) */
#define RF2_OPEN_DOOR		0x00010000	/* Monster can open doors */
#define RF2_BASH_DOOR		0x00020000	/* Monster can bash doors */
#define RF2_PASS_WALL		0x00040000	/* Monster can pass walls */
#define RF2_KILL_WALL		0x00080000	/* Monster can destroy walls */
#define RF2_MOVE_BODY		0x00100000	/* Monster can move monsters */
#define RF2_KILL_BODY		0x00200000	/* Monster can kill monsters */
#define RF2_TAKE_ITEM		0x00400000	/* Monster can pick up items */
#define RF2_KILL_ITEM		0x00800000	/* Monster can crush items */
#define RF2_WANDERER		0x01000000	/* Town wanderers */
#define RF2_BRAIN_2			0x02000000
#define RF2_BRAIN_3			0x04000000
#define RF2_BRAIN_4			0x08000000
#define RF2_BRAIN_5			0x10000000
#define RF2_BRAIN_6			0x20000000
#define RF2_BRAIN_7			0x40000000
#define RF2_BRAIN_8			0x80000000

/*
 * New monster race bit flags
 */
#define RF3_ORC				0x00000001	/* Orc */
#define RF3_TROLL			0x00000002	/* Troll */
#define RF3_GIANT			0x00000004	/* Giant */
#define RF3_DRAGON			0x00000008	/* Dragon */
#define RF3_DEMON			0x00000010	/* Demon */
#define RF3_UNDEAD			0x00000020	/* Undead */
#define RF3_EVIL			0x00000040	/* Evil */
#define RF3_ANIMAL			0x00000080	/* Animal */
#define RF3_XXX1			0x00000100	/* (?) */
#define RF3_XXX2			0x00000200	/* (?) */
#define RF3_XXX3			0x00000400	/* Non-Vocal (?) */
#define RF3_XXX4			0x00000800	/* Non-Living (?) */
#define RF3_HURT_LITE		0x00001000	/* Hurt by lite */
#define RF3_HURT_ROCK		0x00002000	/* Hurt by rock remover */
#define RF3_HURT_FIRE		0x00004000	/* Hurt badly by fire */
#define RF3_HURT_COLD		0x00008000	/* Hurt badly by cold */
#define RF3_IM_ACID			0x00010000	/* Resist acid a lot */
#define RF3_IM_ELEC			0x00020000	/* Resist elec a lot */
#define RF3_IM_FIRE			0x00040000	/* Resist fire a lot */
#define RF3_IM_COLD			0x00080000	/* Resist cold a lot */
#define RF3_IM_POIS			0x00100000	/* Resist poison a lot */
#define RF3_XXX5			0x00200000	/* Immune to (?) */
#define RF3_RES_NETH		0x00400000	/* Resist nether a lot */
#define RF3_RES_WATE		0x00800000	/* Resist water */
#define RF3_RES_PLAS		0x01000000	/* Resist plasma */
#define RF3_RES_NEXU		0x02000000	/* Resist nexus */
#define RF3_RES_DISE		0x04000000	/* Resist disenchantment */
#define RF3_XXX6			0x08000000	/* Resist (?) */
#define RF3_NO_FEAR			0x10000000	/* Cannot be scared */
#define RF3_NO_STUN			0x20000000	/* Cannot be stunned */
#define RF3_NO_CONF			0x40000000	/* Cannot be confused */
#define RF3_NO_SLEEP		0x80000000	/* Cannot be slept */

/*
 * New monster race bit flags
 */
#define RF4_SHRIEK			0x00000001	/* Shriek for help */
#define RF4_XXX2			0x00000002	/* (?) */
#define RF4_XXX3			0x00000004	/* (?) */
#define RF4_XXX4			0x00000008	/* (?) */
#define RF4_ARROW_1			0x00000010	/* Fire an arrow (light) */
#define RF4_ARROW_2			0x00000020	/* Fire an arrow (heavy) */
#define RF4_ARROW_3			0x00000040	/* Fire missiles (light) */
#define RF4_ARROW_4			0x00000080	/* Fire missiles (heavy) */
#define RF4_BR_ACID			0x00000100	/* Breathe Acid */
#define RF4_BR_ELEC			0x00000200	/* Breathe Elec */
#define RF4_BR_FIRE			0x00000400	/* Breathe Fire */
#define RF4_BR_COLD			0x00000800	/* Breathe Cold */
#define RF4_BR_POIS			0x00001000	/* Breathe Poison */
#define RF4_BR_NETH			0x00002000	/* Breathe Nether */
#define RF4_BR_LITE			0x00004000	/* Breathe Lite */
#define RF4_BR_DARK			0x00008000	/* Breathe Dark */
#define RF4_BR_CONF			0x00010000	/* Breathe Confusion */
#define RF4_BR_SOUN			0x00020000	/* Breathe Sound */
#define RF4_BR_CHAO			0x00040000	/* Breathe Chaos */
#define RF4_BR_DISE			0x00080000	/* Breathe Disenchant */
#define RF4_BR_NEXU			0x00100000	/* Breathe Nexus */
#define RF4_BR_TIME			0x00200000	/* Breathe Time */
#define RF4_BR_INER			0x00400000	/* Breathe Inertia */
#define RF4_BR_GRAV			0x00800000	/* Breathe Gravity */
#define RF4_BR_SHAR			0x01000000	/* Breathe Shards */
#define RF4_BR_PLAS			0x02000000	/* Breathe Plasma */
#define RF4_BR_WALL			0x04000000	/* Breathe Force */
#define RF4_BR_MANA			0x08000000	/* Breathe Mana */
#define RF4_XXX5			0x10000000
#define RF4_XXX6			0x20000000
#define RF4_XXX7			0x40000000
#define RF4_BOULDER			0x80000000	/* Throw a boulder */

/*
 * New monster race bit flags
 */
#define RF5_BA_ACID			0x00000001	/* Acid Ball */
#define RF5_BA_ELEC			0x00000002	/* Elec Ball */
#define RF5_BA_FIRE			0x00000004	/* Fire Ball */
#define RF5_BA_COLD			0x00000008	/* Cold Ball */
#define RF5_BA_POIS			0x00000010	/* Poison Ball */
#define RF5_BA_NETH			0x00000020	/* Nether Ball */
#define RF5_BA_WATE			0x00000040	/* Water Ball */
#define RF5_BA_MANA			0x00000080	/* Mana Storm */
#define RF5_BA_DARK			0x00000100	/* Darkness Storm */
#define RF5_DRAIN_MANA		0x00000200	/* Drain Mana */
#define RF5_MIND_BLAST		0x00000400	/* Blast Mind */
#define RF5_BRAIN_SMASH		0x00000800	/* Smash Brain */
#define RF5_CAUSE_1			0x00001000	/* Cause Light Wound */
#define RF5_CAUSE_2			0x00002000	/* Cause Serious Wound */
#define RF5_CAUSE_3			0x00004000	/* Cause Critical Wound */
#define RF5_CAUSE_4			0x00008000	/* Cause Mortal Wound */
#define RF5_BO_ACID			0x00010000	/* Acid Bolt */
#define RF5_BO_ELEC			0x00020000	/* Elec Bolt (unused) */
#define RF5_BO_FIRE			0x00040000	/* Fire Bolt */
#define RF5_BO_COLD			0x00080000	/* Cold Bolt */
#define RF5_BO_POIS			0x00100000	/* Poison Bolt (unused) */
#define RF5_BO_NETH			0x00200000	/* Nether Bolt */
#define RF5_BO_WATE			0x00400000	/* Water Bolt */
#define RF5_BO_MANA			0x00800000	/* Mana Bolt */
#define RF5_BO_PLAS			0x01000000	/* Plasma Bolt */
#define RF5_BO_ICEE			0x02000000	/* Ice Bolt */
#define RF5_MISSILE			0x04000000	/* Magic Missile */
#define RF5_SCARE			0x08000000	/* Frighten Player */
#define RF5_BLIND			0x10000000	/* Blind Player */
#define RF5_CONF			0x20000000	/* Confuse Player */
#define RF5_SLOW			0x40000000	/* Slow Player */
#define RF5_HOLD			0x80000000	/* Paralyze Player */

/*
 * New monster race bit flags
 */
#define RF6_HASTE			0x00000001	/* Speed self */
#define RF6_XXX1			0x00000002	/* Speed a lot (?) */
#define RF6_HEAL			0x00000004	/* Heal self */
#define RF6_XXX2			0x00000008	/* Heal a lot (?) */
#define RF6_BLINK			0x00000010	/* Teleport Short */
#define RF6_TPORT			0x00000020	/* Teleport Long */
#define RF6_XXX3			0x00000040	/* Move to Player (?) */
#define RF6_XXX4			0x00000080	/* Move to Monster (?) */
#define RF6_TELE_TO			0x00000100	/* Move player to monster */
#define RF6_TELE_AWAY		0x00000200	/* Move player far away */
#define RF6_TELE_LEVEL		0x00000400	/* Move player vertically */
#define RF6_XXX5			0x00000800	/* Move player (?) */
#define RF6_DARKNESS		0x00001000	/* Create Darkness */
#define RF6_TRAPS			0x00002000	/* Create Traps */
#define RF6_FORGET			0x00004000	/* Cause amnesia */
#define RF6_XXX6			0x00008000	/* ??? */
#define RF6_S_KIN			0x00010000	/* Summon Kin */
#define RF6_S_HI_DEMON			0x00020000	/*  Summon Greater Demons */
#define RF6_S_MONSTER		0x00040000	/* Summon Monster */
#define RF6_S_MONSTERS		0x00080000	/* Summon Monsters */
#define RF6_S_ANIMAL			0x00100000	/* Summon Animals */
#define RF6_S_SPIDER		0x00200000	/* Summon Spiders */
#define RF6_S_HOUND			0x00400000	/* Summon Hounds */
#define RF6_S_HYDRA			0x00800000	/* Summon Hydras */
#define RF6_S_ANGEL			0x01000000	/* Summon Angel */
#define RF6_S_DEMON			0x02000000	/* Summon Demon */
#define RF6_S_UNDEAD		0x04000000	/* Summon Undead */
#define RF6_S_DRAGON		0x08000000	/* Summon Dragon */
#define RF6_S_HI_UNDEAD		0x10000000	/* Summon Greater Undead */
#define RF6_S_HI_DRAGON		0x20000000	/* Summon Ancient Dragon */
#define RF6_S_WRAITH		0x40000000	/* Summon Unique Wraith */
#define RF6_S_UNIQUE		0x80000000	/* Summon Unique Monster */


/*
 * Some flags are obvious
 */
#define RF1_OBVIOUS_MASK \
	(RF1_UNIQUE | RF1_QUESTOR | RF1_MALE | RF1_FEMALE | \
	 RF1_FRIEND | RF1_FRIENDS | RF1_ESCORT | RF1_ESCORTS)

/*
 * "race" flags
 */
#define RF3_RACE_MASK \
	(RF3_ORC | RF3_TROLL | RF3_GIANT | RF3_DRAGON | \
	 RF3_DEMON | RF3_UNDEAD | RF3_EVIL | RF3_ANIMAL)



/*
	Different types of terrain, used for the wilderness.
	-APD-

	HACK -- I am temporarily using these numbers to determine
	how many monsters to generate.
*/

#define		WILD_LAKE		0
#define		WILD_GRASSLAND		1
#define		WILD_FOREST		2
#define		WILD_SWAMP		20
#define		WILD_DENSEFOREST	15
#define		WILD_WASTELAND		7
#define		WILD_UNDEFINED		8
#define		WILD_CLONE		9 /* we should copy the terrain type of a neighbor */
#define		WILD_TOWN		10

/* different buildings */
#define		WILD_LOG_CABIN		0
#define		WILD_ROCK_HOME		1
#define		WILD_PERM_HOME		2
#define		WILD_SHACK		3
#define		WILD_TOWN_HOME		4
#define		WILD_ARENA		8

/* types of crops */
#define		WILD_CROP_POTATO	0
#define		WILD_CROP_CABBAGE	1
#define		WILD_CROP_CARROT	2
#define		WILD_CROP_BEET		3
#define		WILD_CROP_MUSHROOM	4
#define		WILD_CROP_SQUASH	5
#define		WILD_CROP_CORN		6

/* used for wilderness generation */
#define		DIR_NORTH		0
#define		DIR_EAST		1
#define		DIR_SOUTH		2
#define		DIR_WEST		3

/* wilderness flags */

#define		WILD_F_GENERATED	1
#define		WILD_F_INHABITED	2
#define		WILD_F_IN_MEMORY	4


/*
 * Hack -- choose "intelligent" spells when desperate
 */

#define RF4_INT_MASK \
   0L

#define RF5_INT_MASK \
  (RF5_HOLD | RF5_SLOW | RF5_CONF | RF5_BLIND | RF5_SCARE)

#define RF6_INT_MASK \
   (RF6_BLINK |  RF6_TPORT | RF6_TELE_LEVEL | RF6_TELE_AWAY | \
    RF6_HEAL | RF6_HASTE | RF6_TRAPS | \
    RF6_S_MONSTER | RF6_S_MONSTERS | \
   	RF6_S_ANIMAL | RF6_S_SPIDER | RF6_S_HOUND | RF6_S_HYDRA | \
    RF6_S_ANGEL | RF6_S_DRAGON | RF6_S_UNDEAD | RF6_S_DEMON | \
    RF6_S_HI_DRAGON | RF6_S_HI_UNDEAD | RF6_S_WRAITH | RF6_S_UNIQUE)

/*
 * Artifact activation index
 */
#define ACT_ILLUMINATION        0
#define ACT_MAGIC_MAP           1
#define ACT_CLAIRVOYANCE        2
#define ACT_PROT_EVIL           3
#define ACT_DISP_EVIL           4
#define ACT_HEAL1               5
#define ACT_HEAL2               6
#define ACT_CURE_WOUNDS         7
#define ACT_HASTE1              8
#define ACT_HASTE2              9
#define ACT_FIRE1               10
#define ACT_FIRE2               11
#define ACT_FIRE3               12
#define ACT_FROST1              13
#define ACT_FROST2              14
#define ACT_FROST3              15
#define ACT_FROST4              16
#define ACT_FROST5              17
#define ACT_ACID1               18
#define ACT_RECHARGE1           19
#define ACT_SLEEP               20
#define ACT_LIGHTNING_BOLT      21
#define ACT_ELEC2               22
#define ACT_BANISHMENT          23
#define ACT_MASS_BANISHMENT     24
#define ACT_IDENTIFY            25
#define ACT_DRAIN_LIFE1         26
#define ACT_DRAIN_LIFE2         27
#define ACT_BIZZARE             28
#define ACT_STAR_BALL           29
#define ACT_RAGE_BLESS_RESIST   30
#define ACT_PHASE               31
#define ACT_TRAP_DOOR_DEST      32
#define ACT_DETECT              33
#define ACT_RESIST              34
#define ACT_TELEPORT            35
#define ACT_RESTORE_LIFE        36
#define ACT_MISSILE             37
#define ACT_ARROW               38
#define ACT_REM_FEAR_POIS       39
#define ACT_STINKING_CLOUD      40
#define ACT_STONE_TO_MUD        41
#define ACT_TELE_AWAY           42
#define ACT_WOR                 43
#define ACT_CONFUSE             44
#define ACT_PROBE               45
#define ACT_FIREBRAND           46
#define ACT_STARLIGHT           47
#define ACT_MANA_BOLT           48
#define ACT_BERSERKER           49

#define ACT_MAX                 50

/* Login constants */
#define BAD_PASSWORD 35


/*** Macro Definitions ***/


/*
 * Determine if a given inventory item is "aware"
 */
#define object_aware_p(PLR, T) \
    ((PLR)->kind_aware[(T)->k_idx])

/*
 * Determine if a given inventory item is "tried"
 */
#define object_tried_p(PLR, T) \
    ((PLR)->kind_tried[(T)->k_idx])

/*
 * Determine if a given inventory item is "known"
 * Test One -- Check for special "known" tag
 * Test Two -- Check for "Easy Know" + "Aware"
 */
#define object_known_p(PTR, T) \
	(((T)->ident & (ID_KNOWN)) || \
	 ((k_info[(T)->k_idx].flags3 & (TR3_EASY_KNOW)) && \
	  (PTR)->kind_aware[(T)->k_idx]))


#define object_felt_or_known_p(IND, T) \
    (((T)->ident & ID_SENSE) || \
    object_known_p(IND, T))

/*
 * Return the "attr" for a given item.
 * Allow user redefinition of "aware" items.
 * Default to the "flavor" attr for unaware items
 */
#define object_attr_p(PLR, T) \
    (object_kind_attr_p((PLR), (T)->k_idx))

/*
 * Return the "char" for a given item.
 * Allow user redefinition of "aware" items.
 * Default to the "flavor" char for unaware items
 */
#define object_char_p(PLR, T) \
    (object_kind_char_p((PLR), (T)->k_idx))

/*
 * Return the "attr" for a given object kind.
 * Allow user redefinition of "aware" items.
 * Default to the "flavor" attr for unaware items.
 */
#define object_kind_attr_p(PLR, K_IDX) \
    (((PLR)->kind_aware[(K_IDX)]) ? \
     ((PLR)->k_attr[(K_IDX)]) : \
     ((PLR)->d_attr[(K_IDX)]))

/*
 * Return the "char" for a given object kind.
 * Allow user redefinition of "aware" items.
 * Default to the "flavor" char for unaware items.
 */
#define object_kind_char_p(PLR, K_IDX) \
    (((PLR)->kind_aware[(K_IDX)]) ? \
     ((PLR)->k_char[(K_IDX)]) : \
     ((PLR)->d_char[(K_IDX)]))

/* Server-side versions of the above macros.
 * The difference, is that while those *will* take
 * player's awareness of a given object into account,
 * they will completely ignore player-side visual mappings */

#define object_attr_s(PLR, T) \
    (object_kind_attr_s((PLR), (T)->k_idx))

#define object_char_s(PLR, T) \
    (object_kind_char_p((PLR), (T)->k_idx))

#define object_kind_attr_s(PLR, K_IDX) \
    (((PLR)->kind_aware[(K_IDX)]) ? \
     (k_attr_s[(K_IDX)]) : \
     (flavor_attr_s[ k_info[(K_IDX)].flavor ]))

#define object_kind_char_s(PLR, K_IDX) \
    (((PLR)->kind_aware[(K_IDX)]) ? \
     (k_char_s[(K_IDX)]) : \
     (flavor_char_s[ k_info[(K_IDX)].flavor ]))

/* Copy object */
#define object_copy(D,S) COPY((D), (S), object_type);


/*
 * Artifacts use the "name1" field
 */
#define artifact_p(T) \
        ((T)->name1 ? TRUE : FALSE)
#define true_artifact_p(T) \
	((T)->name1 ? ((T)->name3 ? FALSE : TRUE) : FALSE)
#define randart_p(T) \
	((T)->name1 == ART_RANDART ? TRUE : FALSE)


/* Get pointer to correct artifact structure */
#if defined(RANDART)
#define artifact_ptr(O) \
	((O)->name1 == ART_RANDART ? randart_make((O)) \
	 : &a_info[(O)->name1])
#else
#define artifact_ptr(O) \
	(&a_info[(O)->name1])
#endif


/*
 * Ego-Items use the "name2" field
 */
#define ego_item_p(T) \
        ((T)->name2 ? TRUE : FALSE)


/*
 * Broken items.
 */
#define broken_p(T) \
        ((T)->ident & ID_BROKEN)

/*
 * Cursed items.
 */
#define cursed_p(T) \
        ((T)->ident & ID_CURSED)



/*
 * Determines if a map location is fully inside the outer walls
 */
#define in_bounds(DEPTH,Y,X) \
   ((DEPTH ? (((Y) > 0) && ((X) > 0) && ((Y) < MAX_HGT-1) && ((X) < MAX_WID-1)) \
           : (((Y) > 0) && ((X) > 0) && ((Y) < MAX_HGT-1) && ((X) < MAX_WID-1))))

/*
 * Determines if a map location is fully inside the outer walls
 * This is more than twice as expensive as "in_bounds()", but
 * often we need to exclude the outer walls from calculations.
 */
#define in_bounds_fully(Y,X) \
	(((Y) > 0) && ((Y) < MAX_HGT-1) && \
	 ((X) > 0) && ((X) < MAX_WID-1))


/*
 * Determines if a map location is on or inside the outer walls
 */
#define in_bounds2(DEPTH,Y,X) \
   ((DEPTH ? (((Y) >= 0) && ((X) >= 0) && ((Y) < MAX_HGT) && ((X) < MAX_WID)) \
      : (((Y) > 0) && ((X) > 0) && ((Y) < MAX_HGT) && ((X) < MAX_WID))))


/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(Y,X)" always implies "in_bounds2(Y,X)".
 */
#define panel_contains(P_PTR,Y,X) \
  (((Y) >= (P_PTR)->panel_row_min) && ((Y) <= (P_PTR)->panel_row_max) && \
   ((X) >= (P_PTR)->panel_col_min) && ((X) <= (P_PTR)->panel_col_max))


/*
 * Determine if a "legal" grid is a "floor" grid
 *
 * Line 1 -- forbid doors, rubble, seams, walls
 *
 * Note that the terrain features are split by a one bit test
 * into those features which block line of sight and those that
 * do not, allowing an extremely fast single bit check below.
 */
#define cave_floor_bold(DEPTH,Y,X) \
    (!(cave[DEPTH][Y][X].feat & 0x20))

/*
 * Determine if a grid is an "naked" wilderness grid
 * Line 1 -- forbid in dungeon
 * Line 2-3 -- forbid non-terrain
 */
#define cave_wild_bold(DEPTH,Y,X) \
	(!(DEPTH > 0) && \
	(((cave[DEPTH][Y][X].feat >= FEAT_DIRT) && \
	  (cave[DEPTH][Y][X].feat <= FEAT_LOOSE_DIRT)) || \
	(((cave[DEPTH][Y][X].feat >= FEAT_CROP_HEAD) && \
	  (cave[DEPTH][Y][X].feat <= FEAT_CROP_TAIL)))))


/*
 * Determine if a "legal" grid is a "clean" floor grid
 *
 * Line 1 -- forbid non-floors
 * Line 2 -- forbid non-terrain
 * Line 2 -- forbid normal objects
 */
#define cave_clean_bold(DEPTH,Y,X) \
   (((cave[DEPTH][Y][X].feat == FEAT_FLOOR) || \
     (cave_wild_bold(DEPTH,Y,X))) && \
    (!cave[DEPTH][Y][X].o_idx))

/*
 * Determine if a "legal" grid is an "empty" floor grid
 *
 * Line 1 -- forbid doors, rubble, seams, walls
 * Line 2 -- forbid normal monsters
 * Line 3 -- forbid any player
 */
#define cave_empty_bold(DEPTH,Y,X) \
    (cave_floor_bold(DEPTH,Y,X) && \
     !(cave[DEPTH][Y][X].m_idx))

/*
 * Determine if a "legal" grid is an "naked" floor grid
 *
 * Line 1 -- forbid non-wild floors
 * Line 2 -- forbid non-floors
 * Line 3 -- forbid normal objects
 * Line 4 -- forbid normal monsters
 * Line 5 -- forbid any player... MISSING ?
 */
#define cave_naked_bold(DEPTH,Y,X) \
	(((cave_wild_bold(DEPTH,Y,X))	|| \
	  (cave[DEPTH][Y][X].feat == FEAT_FLOOR)) && \
	!(cave[DEPTH][Y][X].o_idx) && \
	!(cave[DEPTH][Y][X].m_idx))


/*
 * Determine if a "legal" grid is "permanent"
 *
 * Line 1 -- perma-walls in town
 * Line 2-3 -- perma-walls
 * Line 4-5 -- stairs
 * Line 6-7 -- shop doors
 * Lines 8-9 -- home doors
 */
#define cave_perma_bold(DEPTH,Y,X) \
    ((cave[DEPTH][Y][X].feat == FEAT_PERM_CLEAR) || \
    ((cave[DEPTH][Y][X].feat >= FEAT_PERM_EXTRA) && \
      (cave[DEPTH][Y][X].feat <= FEAT_PERM_SOLID)) || \
     ((cave[DEPTH][Y][X].feat == FEAT_LESS) || \
      (cave[DEPTH][Y][X].feat == FEAT_MORE)) || \
     ((cave[DEPTH][Y][X].feat >= FEAT_SHOP_HEAD) && \
      (cave[DEPTH][Y][X].feat <= FEAT_SHOP_TAIL)) || \
     ((cave[DEPTH][Y][X].feat >= FEAT_HOME_HEAD) && \
      (cave[DEPTH][Y][X].feat <= FEAT_HOME_TAIL)))

/*
 * Is a given location "valid" for placing things?
 *
 * Permanent grids are never "valid" (see above).
 *
 * Hack -- a grid with an artifact in it is never valid.
 *
 * This function is often "combined" with "cave_floor_bold(Y,X)"
 * or one of the other similar macros above.
 *
 * Line 1 -- forbid perma-grids
 * Line 2-3 -- forbid grids containing artifacts
 * Line 4 -- forbit house doors
 */
#define cave_valid_bold(DEPTH,Y,X) \
    (!cave_perma_bold(DEPTH,Y,X) && \
     (!cave[DEPTH][Y][X].o_idx || \
      !artifact_p(&o_list[cave[DEPTH][Y][X].o_idx])))



/*
 * Grid based version of "cave_floor_bold()"
 */
#define cave_floor_grid(C) \
    (!((C)->feat & 0x20))


/*
 * Grid based version of "cave_wild_bold()"
 */
#define cave_wild_grid(C) \
	(((C)->feat >= FEAT_DIRT) && \
	 ((C)->feat <= FEAT_LOOSE_DIRT))


/*
 * Grid based version of "cave_clean_bold()"
 */
#define cave_clean_grid(C) \
    (((C)->feat == FEAT_FLOOR) || \
     (cave_wild_grid(C)) && \
     (!(C)->o_idx))

/*
 * Grid based version of "cave_empty_bold()"
 */
#define cave_empty_grid(C) \
    (cave_floor_grid(C) && \
     !((C)->m_idx))

/*
 * Grid based version of "cave_empty_bold()"
 */
#define cave_naked_grid(C) \
    ((((C)->feat == FEAT_FLOOR) || \
     		cave_wild_grid(C)) && \
     !((C)->o_idx) && \
     			!((C)->m_idx)))

/*
 * Grid based version of "cave_perma_bold()"
 */
#define cave_perma_grid(C) \
    (((C)->feat >= FEAT_PERM_EXTRA) || \
     (((C)->feat == FEAT_LESS) || \
      ((C)->feat == FEAT_MORE)) || \
     (((C)->feat >= FEAT_SHOP_HEAD) && \
      ((C)->feat <= FEAT_SHOP_TAIL)))


/*
 * Grid based version of "cave_valid_bold()"
 */
#define cave_valid_grid(C) \
    (!cave_perma_grid(C) && \
     (!(C)->o_idx || \
      !artifact_p(&o_list[(C)->o_idx])))



/*
 * Determine if a "legal" grid is within "los" of the player
 *
 * Note the use of comparison to zero to force a "boolean" result
 */
#define player_has_los_bold(PLR,Y,X) \
    ((PLR->cave_flag[Y][X] & CAVE_VIEW) != 0)

/*
 * Convert an "attr"/"char" pair into a "pict" (P)
 */
#define PICT(A,C) \
	((((u16b)(A)) << 8) | ((byte)(C)))

/*
 * Convert a "pict" (P) into an "attr" (A)
 */
#define PICT_A(P) \
	((byte)((P) >> 8))

/*
 * Convert a "pict" (P) into an "char" (C)
 */
#define PICT_C(P) \
	((char)((byte)(P)))

/*
 * Determine if a dungeon level of specific "depth" is a town.
 */
#define level_is_town(DEPTH) \
	((DEPTH) == 0 || (cfg_more_towns && check_special_level((DEPTH))))


/*
 * Get index for a player
 */
#define player_index(PLR) ( (PLR)->Ind )

/*
 * Compare two players
 */
#define same_player(PLR1, PLR2) ( (PLR1) == (PLR2) )

/*
 * Iterate over all players
 * ITER must be a defined "int".
 * PLR must be a defined "player_type*".
 */
#define foreach_player(ITER, PLR) \
	for (\
		(ITER) = 1;\
		((ITER) <= NumPlayers) && ((PLR) = Players[(ITER)]); \
		(ITER)++ \
	    )

/* Define iterator to use with "foreach_player" */
#define player_iterator(ITER, PLR) \
	int (ITER); \
	player_type* (PLR);

/*
 * Hack -- Prepare to use the "Secure" routines
 */
#if defined(SET_UID) && defined(SECURE)
extern int PlayerUID;
# define getuid() PlayerUID
# define geteuid() PlayerUID
#endif
