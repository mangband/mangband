/* File: config.h */

/* Purpose: Angband specific configuration stuff */

/* Note : Much of the functionality of this file is being phased into
 * mangband.cfg.  This will allow server reconfiguration without recompilation.
 */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * Look through the following lines, and where a comment includes the
 * tag "OPTION:", examine the associated "#define" statements, and decide
 * whether you wish to keep, comment, or uncomment them.  You should not
 * have to modify any lines not indicated by "OPTION".
 *
 * Note: Also examine the "system" configuration file "h-config.h"
 * and the variable initialization file "variable.c".  If you change
 * anything in "variable.c", you only need to recompile that file.
 *
 * And finally, remember that the "Makefile" will specify some rather
 * important compile time options, like what visual module to use.
 */

/* 
 * define this if you wish to limit players to one character playing
 * at a time on the server.  Note this doesn't stop them from just
 * using two computers side by side, but most won't have this.
#define LIMIT_PLAYER_CONNECTIONS
 */

/*
 * OPTION: Set the metaserver address.  The metaserver keeps track of
 * all running servers so people can find an active server with other
 * players on.  Define this to be an empty string if you don't want to
 * report to a metaserver.
 */
#define	META_ADDRESS "mangband.org"  


/*
 * OPTION: Set a vhost bind address.  This is used in two ways.
 * For one, it sets the name the metaserver will list for the IP.
 * This is useful if your IP has a different reverse address (or no 
 * reverse address), and you'd like the metaserver to list you by 
 * the established name here.  
 *
 * This is also useful if you have multiple IP's on a single box, 
 * and care which one the server binds too, (for name purposes, perhaps).
 * Note that this would allow multiple servers to run on a single
 * machine as well.
 *
#define	BIND_NAME "tester"
#define	BIND_IP "192.168.1.199"
*/


/*
 * OPTION: Disconnect starving players.  If defined, starving players
 * will be kicked out of the game (to protect their characters), given
 * they've been AFK for at least the specified number of seconds.
 */
#define DISCONNECT_STARVING (60 * 3)

/*
 * OPTION: Allow the use of "sound" in various places.
 */
#define USE_SOUND

/*
 * OPTION: Allow the use of "graphics" in various places
 */
#define USE_GRAPHICS

/*
 * OPTION: Default fonts (when using X11)
 */
#define DEFAULT_X11_FONT_SCREEN		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_MIRROR		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_RECALL		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_CHOICE		DEFAULT_X11_FONT



/*
 * OPTION: Hack -- Macintosh stuff
 */
#ifdef MACINTOSH

/* Do not handle signals */
/* also available using the -Z switch on startup for mangband */
# undef HANDLE_SIGNALS

#endif

/*
 * OPTION: See the Makefile(s), where several options may be declared.
 *
 * Some popular options include "USE_GCU" (allow use with Unix "curses"),
 * "USE_X11" (allow basic use with Unix X11), "USE_XAW" (allow use with
 * Unix X11 plus the Athena Widget set), and "USE_CAP" (allow use with
 * the "termcap" library, or with hard-coded vt100 terminals).
 *
 * The old "USE_NCU" option has been replaced with "USE_GCU".
 *
 * Several other such options are available for non-unix machines,
 * such as "MACINTOSH", "WINDOWS", "USE_IBM", "USE_EMX".
 *
 * You may also need to specify the "system", using defines such as
 * "SOLARIS" (for Solaris), etc, see "h-config.h" for more info.
 */


/*
 * OPTION: Include some debugging code.  Note that this option may
 * result in a server that crashes more frequently, as a core dump
 * will be done instead of killing the bad connection.
 */
/* #define DEBUG */


/*
 * OPTION: define "SPECIAL_BSD" for using certain versions of UNIX
 * that use the 4.4BSD Lite version of Curses in "main-gcu.c"
 */
/* #define SPECIAL_BSD */


/*
 * OPTION: Use the POSIX "termios" methods in "main-gcu.c"
 */
/* #define USE_TPOSIX */

/*
 * OPTION: Use the "termio" methods in "main-gcu.c"
 */
/* #define USE_TERMIO */

/*
 * OPTION: Use the icky BSD "tchars" methods in "main-gcu.c"
 */
/* #define USE_TCHARS */



/*
 * OPTION: Use the "curs_set()" call in "main-gcu.c".
 * Hack -- This option will not work on most BSD machines
 */
#if defined(SYS_V) || defined(linux) || defined(__FreeBSD__)
# define USE_CURS_SET
#endif



/*
 * OPTION: Default font (when using X11).
 */
#define DEFAULT_X11_FONT		"9x15"

/*
 * OPTION: for multi-user machines running the game setuid to some other
 * user (like 'games') this SAFE_SETUID option allows the program to drop
 * its privileges when saving files that allow for user specified pathnames.
 * This lets the game be installed system wide without major security
 * concerns.  There should not be any side effects on any machines.
 *
 * This will handle "gids" correctly once the permissions are set right.
 * --VERY DANGEROUS IN MANGBAND.
 */
#define SAFE_SETUID


/*
 * This flag enables the "POSIX" methods for "SAFE_SETUID".
 */
#ifdef _POSIX_SAVED_IDS
# define SAFE_SETUID_POSIX
#endif


/*
 * This "fix" is from "Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>"
 * It prevents problems on (non-Solaris) Suns using "SAFE_SETUID".
 */
#if defined(sun) && !defined(SOLARIS)
# undef SAFE_SETUID_POSIX
#endif


/*
 * OPTION: Verify savefile Checksums (Angband 2.7.0 and up)
 * This option can help prevent "corruption" of savefiles, and also
 * stop intentional modification by amateur users.
 */
#define VERIFY_CHECKSUMS


/*
 * OPTION: Forbid the use of "fiddled" savefiles.  As far as I can tell,
 * a fiddled savefile is one with an internal timestamp different from
 * the actual timestamp.  Thus, turning this option on forbids one from
 * copying a savefile to a different name.  Combined with disabling the
 * ability to save the game without quitting, and with some method of
 * stopping the user from killing the process at the tombstone screen,
 * this should prevent the use of backup savefiles.  It may also stop
 * the use of savefiles from other platforms, so be careful.
 */
/* #define VERIFY_TIMESTAMP */


/*
 * OPTION: Forbid the "savefile over-write" cheat, in which you simply
 * run another copy of the game, loading a previously saved savefile,
 * and let that copy over-write the "dead" savefile later.  This option
 * either locks the savefile, or creates a fake "xxx.lok" file to prevent
 * the use of the savefile until the file is deleted.  Not ready yet.
 */
/* #define VERIFY_SAVEFILE */


/*
 * OPTION: Use wider corrdiors (room for two people abreast).
 */
#define WIDE_CORRIDORS

/*
 * OPTION: Do not switch to manual targeting if there are no 
 * targets in the vicinity of player (un-Angband), display a prompt instead
 */
#define NOTARGET_PROMPT

/*
 * OPTION: Enable experimental command overloading.
 * Command overloads eat more bandwitch, but give more flexibility.
 */
#define COMMAND_OVERLOAD
 
/*
 * OPTION: Hack -- Compile in support for "Cyborg" mode
 */
/* #define ALLOW_BORG */

/*
 * OPTION: Hack -- Compile in support for "Wizard Commands"
 */
/* #define ALLOW_WIZARD */

/*
 * OPTION: Hack -- Compile in support for "Spoiler Generation"
 */
/* #define ALLOW_SPOILERS */


/*
 * OPTION: Allow "do_cmd_colors" at run-time
 */
#define ALLOW_COLORS

/*
 * OPTION: Allow "do_cmd_visuals" at run-time
 */
#define ALLOW_VISUALS

/*
 * OPTION: Allow "do_cmd_macros" at run-time
 */
#define ALLOW_MACROS


/*
 * OPTION: Allow characteres to be "auto-rolled"
 * --broken in mangband.
 */
/* #define ALLOW_AUTOROLLER */


/*
 * OPTION: Allow monsters to "flee" when hit hard
 */
#define ALLOW_FEAR

/*
 * OPTION: Allow monsters to "flee" from strong players
 */
#define ALLOW_TERROR


/*
 * OPTION: Allow parsing of the ascii template files in "init.c".
 * This must be defined if you do not have valid binary image files.
 * It should be usually be defined anyway to allow easy "updating".
 */
#define ALLOW_TEMPLATES

/*
 * OPTION: Allow loading of pre-2.7.0 savefiles.  Note that it takes
 * about 15K of code in "save-old.c" to parse the old savefile format.
 * Angband 2.8.0 will ignore a lot of info from pre-2.7.0 savefiles.
 * --broken in mangband.
 */
/* #define ALLOW_OLD_SAVEFILES */

/* Use the new houseless code for Wilderness levels, including invaders. */
/*
#define DEVEL_TOWN_COMPATIBILITY 0
*/


/*
 * OPTION: Delay the loading of the "f_text" array until it is actually
 * needed, saving ~1K, since "feature" descriptions are unused.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_F_TEXT */

/*
 * OPTION: Delay the loading of the "k_text" array until it is actually
 * needed, saving ~1K, since "object" descriptions are unused.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_K_TEXT */

/*
 * OPTION: Delay the loading of the "a_text" array until it is actually
 * needed, saving ~1K, since "artifact" descriptions are unused.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_A_TEXT */

/*
 * OPTION: Delay the loading of the "e_text" array until it is actually
 * needed, saving ~1K, since "ego-item" descriptions are unused.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_E_TEXT */

/*
 * OPTION: Delay the loading of the "r_text" array until it is actually
 * needed, saving ~60K, but "simplifying" the "monster" descriptions.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_R_TEXT */

/*
 * OPTION: Delay the loading of the "v_text" array until it is actually
 * needed, saving ~1K, but "destroying" the "vault" generation.
 * --broken in mangband.
 */
/* #define DELAY_LOAD_V_TEXT */


/*
 * OPTION: Handle signals
 */
#define HANDLE_SIGNALS


/*
 * OPTION: Hack -- Windows stuff
 */
#if defined(WINDOWS) || defined(__WIN32__)

/* Do not handle signals */
#undef HANDLE_SIGNALS

#endif


/*
 * Allow "Wizards" to yield "high scores"
 * --broken in mangband.
 */
/* #define SCORE_WIZARDS */

/*
 * Allow "Borgs" to yield "high scores"
 * --broken in mangband.
 */
/* #define SCORE_BORGS */

/*
 * Allow "Cheaters" to yield "high scores"
 * --broken in mangband.
 */
/* #define SCORE_CHEATERS */




/*
 * OPTION: Allow use of the "flow_by_smell" and "flow_by_sound"
 * software options, which enable "monster flowing".
 * --broken in mangband.
 */
/* #define MONSTER_FLOW */


/*
 * OPTION: Maximum flow depth when using "MONSTER_FLOW"
 */
#define MONSTER_FLOW_DEPTH 32



/*
 * OPTION: Allow use of extended spell info	-DRS-
 */
#define DRS_SHOW_SPELL_INFO

/*
 * OPTION: Allow use of the monster health bar	-DRS-
 */
#define DRS_SHOW_HEALTH_BAR


/*
 * OPTION: Enable the "smart_learn" and "smart_cheat" options.
 * They let monsters make more "intelligent" choices about attacks
 * (including spell attacks) based on their observations of the
 * player's reactions to previous attacks.  The "smart_cheat" option
 * lets the monster know how the player would react to an attack
 * without actually needing to make the attack.  The "smart_learn"
 * option requires that a monster make a "failed" attack before
 * learning that the player is not harmed by that attack.
 *
 * This adds about 3K to the memory and about 5K to the executable.
 * --broken in mangband.
 */
/* #define DRS_SMART_OPTIONS */



/*
 * OPTION: Enable the "track_follow" and "track_target" options.
 * They let monsters follow the player's foot-prints, or remember
 * the player's recent locations.  This code has been removed from
 * the current version because it is being rewritten by Billy, and
 * until it is ready, it will not work.  Do not define this option.
 * --broken in mangband.
 */
/* #define WDT_TRACK_OPTIONS */





/*
 * OPTION: Set the "default" path to the angband "lib" directory.
 *
 * See "main.c" for usage, and note that this value is only used on
 * certain machines, primarily Unix machines.  If this value is used,
 * it will be over-ridden by the "ANGBAND_PATH" environment variable,
 * if that variable is defined and accessable.  The final slash is
 * optional, but it may eventually be required.
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the "lib" folder, for example, "/tmp/angband/lib/"
 * or "/usr/games/lib/angband/", or "/pkg/angband/lib".
 */
#ifndef PKGDATADIR
# define PKGDATADIR DEFAULT_PATH
#endif
#ifndef LOCALSTATEDIR
# define LOCALSTATEDIR DEFAULT_PATH
#endif


/*
 * On multiuser systems, add the "uid" to savefile names
 */
#ifdef SET_UID
# define SAVEFILE_USE_UID
#endif


/*
 * OPTION: Check the "time" against "lib/file/hours.txt"
 * --not used in mangband
 */
/* #define CHECK_TIME */

/*
 * OPTION: Check the "load" against "lib/file/load.txt"
 * This may require the 'rpcsvs' library
 * --not used in mangband
 */
/* #define CHECK_LOAD */


/*
 * OPTION: For some brain-dead computers with no command line interface,
 * namely Macintosh, there has to be some way of "naming" your savefiles.
 * The current "Macintosh" hack is to make it so whenever the character
 * name changes, the savefile is renamed accordingly.  But on normal
 * machines, once you manage to "load" a savefile, it stays that way.
 * Macintosh is particularly weird because you can load savefiles that
 * are not contained in the "lib:save:" folder, and if you change the
 * player's name, it will then save the savefile elsewhere.  Note that
 * this also gives a method of "bypassing" the "VERIFY_TIMESTAMP" code.
 */
#if defined(MACINTOSH) || defined(WINDOWS) || defined(AMIGA)
# define SAVEFILE_MUTABLE
#endif


/*
 * OPTION: Capitalize the "user_name" (for "default" player name)
 * This option is only relevant on SET_UID machines.
 */
#define CAPITALIZE_USER_NAME



/*
 * OPTION: Shimmer Multi-Hued monsters/objects
 */
#define SHIMMER_MONSTERS
#define SHIMMER_OBJECTS



/*
 * Hack -- Special "ancient machine" versions
 */
#if defined(USE_286) || defined(ANGBAND_LITE_MAC)
# ifndef ANGBAND_LITE
#  define ANGBAND_LITE
# endif
#endif

/*
 * OPTION: Attempt to minimize the size of the game
 */
#ifndef ANGBAND_LITE
/* #define ANGBAND_LITE */
#endif

/*
 * Hack -- React to the "ANGBAND_LITE" flag
 */
#ifdef ANGBAND_LITE
# undef ALLOW_COLORS
# undef ALLOW_VISUALS
# undef ALLOW_MACROS
# undef MONSTER_FLOW
# undef WDT_TRACK_OPTIONS
# undef DRS_SMART_OPTIONS
# undef ALLOW_OLD_SAVEFILES
# undef ALLOW_BORG
# undef ALLOW_WIZARD
# undef ALLOW_SPOILERS
# undef ALLOW_TEMPLATES
# undef DELAY_LOAD_R_TEXT
# define DELAY_LOAD_R_TEXT
#endif



/*
 * OPTION: Attempt to prevent all "cheating"
 */
/* #define VERIFY_HONOR */


/*
 * React to the "VERIFY_HONOR" flag
 */
#ifdef VERIFY_HONOR
# define VERIFY_SAVEFILE
# define VERIFY_CHECKSUMS
# define VERIFY_TIMESTAMPS
#endif


/*
 * OPTION: Use "blocking getch() calls" in "main-gcu.c".
 * Hack -- Note that this option will NOT work on many BSD machines
 * Currently used whenever available, if you get a warning about
 * "nodelay()" undefined, then make sure to undefine this.
 */
#if defined(SYS_V) || defined(AMIGA) || defined(linux)
# define USE_GETCH
#endif

/*
 * OPTION: for the AFS distributed file system, define this to ensure that
 * the program is secure with respect to the setuid code.  This option has
 * not been tested (to the best of my knowledge).  This option may require
 * some weird tricks with "player_uid" and such involving "defines".
 * Note that this option used the AFS library routines Authenticate(),
 * bePlayer(), beGames() to enforce the proper priviledges.
 * You may need to turn "SAFE_SETUID" off to use this option.
 */
/* #define SECURE */

