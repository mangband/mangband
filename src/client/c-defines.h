/* File: c-defines.h */

/* Purpose: client constants and macro definitions */

/*
 * Current version number of the MAngband client
 */

#define CLIENT_VERSION_MAJOR	1
#define CLIENT_VERSION_MINOR	5
#define CLIENT_VERSION_PATCH	3

/*
 * This value specifys the suffix to the version info sent to the metaserver.
 *
 * 0 - nothing
 * 1 - "alpha"
 * 2 - "beta"
 * 3 - "development"
 */
#define CLIENT_VERSION_EXTRA	0


/*
 * This value is a single 16-bit number holding the version info
 */

#define CLIENT_VERSION ( CLIENT_VERSION_MAJOR << 12 | CLIENT_VERSION_MINOR << 8 \
  | CLIENT_VERSION_PATCH << 4 | CLIENT_VERSION_EXTRA)


/*
 * PMSG_TERM hack. This is used on Windows client to force Term-4
 * into chat mode (and forbid any other terms from using it).
 */
#ifdef USE_WIN
#define PMSG_TERM 4
#endif

/*
 * Maximum number of macro trigger names
 */
#define MAX_MACRO_TRIGGER 200
#define MAX_MACRO_MOD 12

/* Indicators */
#define MAX_INDICATORS	64
#define MAX_COFFERS 	255

/* Sound */
#define SAMPLE_MAX 16

/* Get cave by stream */
#define stream_cave(I, L) (p_ptr->stream_cave[(I)] + (L) * p_ptr->stream_wid[(I)])

/* Speed of air layer fadeout effects */
#define AIR_FADE_THRESHOLD 10

/* Speed of slash fx effect */
#define SLASH_FX_THRESHOLD 500

/*** SERVER DEFINES ***/
/* Sometimes, we just copy defines from server.
 * Why not have them in common/ ? Because they ultimately are different values
 */ //TODO: remove those!!!
#define TV_MAX		100

/*
 * Compact display (shows Race, Class, Title, HP etc; on the left)
 * 	Tradittionaly it is 13 characters wide.
 *	Status line (shows Depth, Study at the bottom)
 *		1 row
 * Top Line (shows lots of stuff) - 1 row
 *
 * Depending if player enabled or disabled compact/status,
 * different offsets should be used when drawing map on screen.
 */
#define SCREEN_CLIP_X	13
#define SCREEN_CLIP_Y	2
#define SCREEN_CLIP_L	1
#define DUNGEON_OFFSET_X \
	((window_flag_o[0] & PW_PLAYER_2) ? SCREEN_CLIP_X : 0)
#define DUNGEON_OFFSET_Y \
	(1)
#define DUNGEON_CLIP_X \
	(0)
#define DUNGEON_CLIP_Y \
	((window_flag_o[0] & PW_STATUS) ? SCREEN_CLIP_L : 0)

/*** Screen Locations ***/

/*
 * Some screen locations for various display routines
 * Currently, row 8 and 15 are the only "blank" rows.
 * That leaves a "border" around the "stat" values.
 */

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

#define ROW_HUNGRY  	(Term->hgt - 1)
#define COL_HUNGRY  	0	/* "Weak" / "Hungry" / "Full" / "Gorged" */

#define ROW_BLIND   	(Term->hgt - 1)
#define COL_BLIND   	7	/* "Blind" */

#define ROW_CONFUSED	(Term->hgt - 1)
#define COL_CONFUSED	13	/* "Confused" */

#define ROW_AFRAID  	(Term->hgt - 1)
#define COL_AFRAID  	22	/* "Afraid" */

#define ROW_POISONED	(Term->hgt - 1)
#define COL_POISONED	29	/* "Poisoned" */

#define ROW_STATE   	(Term->hgt - 1)
#define COL_STATE   	38	/* <state> */

#define ROW_SPEED   	(Term->hgt - 1)
#define COL_SPEED   	49	/* "Slow (-NN)" or "Fast (+NN)" */

#define ROW_STUDY   	(Term->hgt - 1)
#define COL_STUDY   	64	/* "Study" */

#define ROW_DEPTH   	(Term->hgt - 1)
#define COL_DEPTH   	70	/* "Lev NNN" / "NNNN ft" */

#define ROW_OPPOSE_ELEMENTS	(Term->hgt - 1)
#define COL_OPPOSE_ELEMENTS	80	/* "Acid Elec Fire Cold Pois" */

/* Bitflags for "ctxt_flag" argument in
 * spell_as_keystroke(), item_as_keystroke() functions.
 */
#define CTXT_WITH_CMD  0x01 /* If set, include command keystrokes */
#define CTXT_WITH_ITEM 0x02 /* If set, include item selection keystrokes */
#define CTXT_WITH_DIR  0x08 /* If set, include dir selection keystrokes */
#define CTXT_FULL (CTXT_WITH_CMD | CTXT_WITH_ITEM | CTXT_WITH_DIR)
#define CTXT_PREFER_SHORT 0x10 /* Prefer short spell names ('a') */
#define CTXT_PREFER_NAME  0x20 /* Prefer long name ("Scroll of Xuzzy") */
#define CTXT_PROJECTING   0x40 /* Projecting a spell/pryaer (uppercase) */
#define CTXT_FORCE_TARGET 0x80 /* Always include "*t" at the end */
