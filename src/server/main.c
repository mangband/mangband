/* File: main.c */

/* Purpose: initialization, main() function and main loop */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 *
 * Unnecessary, as the server doesn't open any "terms".  --KLJ--
 */
static void quit_hook(cptr s)
{
	cleanup_angband();
#if 0
	int j;

	/* Scan windows */
	for (j = 8 - 1; j >= 0; j--)
	{
		/* Unused */
		if (!ang_term[j]) continue;

		/* Nuke it */
		term_nuke(ang_term[j]);
	}
#endif
}



/*
 * Set the stack size (for the Amiga)
 */
#ifdef AMIGA
# include <dos.h>
__near long __stack = 32768L;
#endif


/*
 * Set the stack size and overlay buffer (see main-286.c")
 */
#ifdef USE_286
# include <dos.h>
extern unsigned _stklen = 32768U;
extern unsigned _ovrbuffer = 0x1500;
#endif

/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * PKGDATADIR, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the PKGDATADIR constant.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 *
 * Note that the "path" must be "Angband:" for the Amiga, and it
 * is ignored for "VM/ESA", so I just combined the two.
 */
static void init_stuff(void)
{
	char path[1024];
	char path_wr[1024];

#if defined(AMIGA) || defined(VM)

	/* Hack -- prepare "path" */
	strcpy(path, "Angband:");

#else /* AMIGA / VM */

	cptr tail;

	/* Get the environment variable */
	tail = getenv("ANGBAND_PATH");

	/* Use the angband_path, or a default */
	my_strcpy(path, tail ? tail : PKGDATADIR, 1024);

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) strcat(path, PATH_SEP);

	/* Repeat for writable paths */
	my_strcpy(path_wr, tail ? tail : LOCALSTATEDIR, 1024);
	if (!suffix(path_wr, PATH_SEP)) strcat(path_wr, PATH_SEP);


#endif /* AMIGA / VM */

	/* Initialize */
	init_file_paths(path, path_wr);
}

/*
 * Server logging hook.
 * We should be cautious, as we may be called from a signal handler in a panic.
 */
static void server_log(cptr str)
{
	char buf[16];
	time_t t;
	struct tm *local;
	
	/* Grab the time */
	time(&t);
	local = localtime(&t);
	strftime(buf, 16, "%d%m%y %H%M%S", local);
	
	/* Output the message timestamped */
	fprintf(stderr,"%s %s\n", buf, str);
}

void perform_sanity_check(void)
{
	char buf[128];
	u64b tmp64u, _tmp64u;
	u32b tmp32u, _tmp32u;
	u16b tmp16u, _tmp16u;
	byte tmp08u, _tmp08u;
/* execessive
	s64b tmp64s, _tmp64s;
	s32b tmp32s, _tmp32s;
	s16b tmp16s, _tmp16s;
	char tmp08s, _tmp08s;
*/

#ifdef DEBUG
#ifdef SCNu8
	printf("byte %d <u> %s %s\n", (int)sizeof(byte), PRIu8, SCNu8);
	printf("char %d <u> %s %s\n", (int)sizeof(char), PRId8, SCNd8);
#else
	printf("byte %d <!> %s %s\n", (int)sizeof(byte), PRIu8, SCNu16);
	printf("char %d <!> %s %s\n", (int)sizeof(char), PRId8, SCNd16);
#endif
	printf("u16b %d <u> %s %s\n", (int)sizeof(u16b), PRIu16, SCNu16);
	printf("s16b %d <u> %s %s\n", (int)sizeof(s16b), PRId16, SCNd16);
	printf("u32b %d <u> %s %s\n", (int)sizeof(u32b), PRIu32, SCNu32);
	printf("s32b %d <u> %s %s\n", (int)sizeof(s32b), PRId32, SCNd32);
	printf("u64b %d <u> %s %s\n", (int)sizeof(u64b), PRIu64, SCNu64);
	printf("s64b %d <u> %s %s\n", (int)sizeof(s64b), PRId64, SCNd64);
#endif
	if (sizeof(byte) < 1) quit("sizeof(byte) < 1");
	if (sizeof(u16b) < 2) quit("sizeof(u16b) < 2");
	if (sizeof(u32b) < 4) quit("sizeof(u32b) < 4");
	if (sizeof(u64b) < 8) quit("sizeof(u64b) < 8");

	tmp08u =/* tmp08s =*/ 0xF7;
	tmp16u =/* tmp16s =*/ 0xF7F7;
	tmp32u =/* tmp32s =*/ 0xF7F7F7F7;
	tmp64u =/* tmp64s =*/ 0xF7F7F7F7F7F7F7F7LL;

	sprintf(buf, "%" PRIu8, tmp08u);
#ifndef SCNu8
	sscanf(buf, "%" SCNu16, &_tmp16u);
	_tmp08u = (byte)_tmp16u;
#else
	sscanf(buf, "%" SCNu8, &_tmp08u);
#endif
	if (tmp08u != _tmp08u) quit_fmt("printf/sscanf mismatch for byte");

	strnfmt(buf, sizeof(buf), "%" PRIu8, tmp08u);
	_tmp08u = 0;
#ifndef SCNu8
	sscanf(buf, "%" SCNu16, &_tmp16u);
	_tmp08u = (byte)_tmp16u;
#else
	sscanf(buf, "%" SCNu8, &_tmp08u);
#endif
	if (tmp08u != _tmp08u) quit_fmt("strnfmt/sscanf mismatch for byte");


	sprintf(buf, "%" PRIu16, tmp16u);
	sscanf(buf, "%" SCNu16, &_tmp16u);
	if (tmp16u != _tmp16u) quit_fmt("printf/sscanf mismatch for u16b %s/%s", PRIu16, SCNu16);

	strnfmt(buf, sizeof(buf), "%" PRIu16, tmp16u);
	_tmp16u = 0;
	sscanf(buf, "%" SCNu16, &_tmp16u);
	if (tmp16u != _tmp16u) quit_fmt("strnfmt/sscanf mismatch for u16b %s/%s", PRIu16, SCNu16);


	sprintf(buf, "%" PRIu32, tmp32u);
	sscanf(buf, "%" SCNu32, &_tmp32u);
	if (tmp32u != _tmp32u) quit_fmt("printf/sscanf mismatch for u32b %s/%s", PRIu32, SCNu32);
	
	strnfmt(buf, sizeof(buf), "%" PRIu32, tmp32u);
	_tmp32u = 0;
	sscanf(buf, "%" SCNu32, &_tmp32u);
	if (tmp32u != _tmp32u) quit_fmt("strnfmt/sscanf mismatch for u32b %s/%s", PRIu32, SCNu32);


	sprintf(buf, "%" PRIu64, tmp64u);
	sscanf(buf, "%" SCNu64, &_tmp64u);
	if (tmp64u != _tmp64u) quit_fmt("printf/sscanf mismatch for u64b %s/%s", PRIu64, SCNu64);

	strnfmt(buf, sizeof(buf), "%" PRIu64, tmp64u);
	_tmp64u = 0;
	sscanf(buf, "%" SCNu64, &_tmp64u);
	if (tmp64u != _tmp64u) quit_fmt("strnfmt/sscanf mismatch for u64b %s/%s", PRIu64, SCNu64);
}

void show_version()
{
	cptr version_modifiers[] = {
		"", "alpha", "beta", "devel"
	};
	printf("MAngband Server %d.%d.%d %s\n",
		SERVER_VERSION_MAJOR,
		SERVER_VERSION_MINOR,
		SERVER_VERSION_PATCH,
		version_modifiers[SERVER_VERSION_EXTRA]);
	puts("Copyright (c) 2007-2016 MAngband Project Team");
	puts("Compiled with:");
#ifdef CONFIG_PATH
	printf("    Config path: %s\n", CONFIG_PATH);
#endif
	printf("     PKGDATADIR: %s\n", PKGDATADIR);
	printf("  LOCALSTATEDIR: %s\n", LOCALSTATEDIR);
	/* Actually abort the process */
	quit(NULL);
}

/*
 * Some machines can actually parse command line args
 *
 * XXX XXX XXX The "-c", "-d", and "-i" options should probably require
 * that their "arguments" do NOT end in any path separator.
 *
 * The "path" options should probably be simplified into some form of
 * "-dWHAT=PATH" syntax for simplicity.
 */
int main(int argc, char *argv[])
{
	bool new_game = FALSE;
	int catch_signals = TRUE;

	/* Setup our logging hook */
	plog_aux = server_log;	

	/* Save the "program name" */
	argv0 = argv[0];

#ifdef WINDOWS
	/* Load our debugging library on Windows, to give us nice stack dumps */
	/* We use exchndl.dll from the mingw-utils package */
	LoadLibrary("exchndl.dll");
	
	/* Initialise WinSock */
#endif

#ifdef USE_286
	/* Attempt to use XMS (or EMS) memory for swap space */
	if (_OvrInitExt(0L, 0L))
	{
		_OvrInitEms(0, 0, 64);
	}
#endif

	/* Sanity check */
	perform_sanity_check();

	/* Get the file paths */
	init_stuff();

	/* Process the command line arguments */
	for (--argc, ++argv; argc > 0; --argc, ++argv)
	{
		/* Require proper options */
		if (argv[0][0] != '-') goto usage;

		/* Analyze option */
		switch (argv[0][1])
		{
			case 'c':
			case 'C':
			arg_config_file = string_make(&argv[0][2]);
			break;

			case 'e':
			case 'E':
			ANGBAND_DIR_EDIT = string_make(&argv[0][2]);
			break;

			case 'p':
			case 'P':
			ANGBAND_DIR_PREF = string_make(&argv[0][2]);
			break;

			case 't':
			case 'T':
			ANGBAND_DIR_HELP = string_make(&argv[0][2]);
			break;

			case 'd':
			case 'D':
			ANGBAND_DIR_DATA = string_make(&argv[0][2]);
			break;

#ifndef VERIFY_SAVEFILE
			case 's':
			case 'S':
			ANGBAND_DIR_SAVE = string_make(&argv[0][2]);
			break;
#endif

			case 'b':
			case 'B':
			ANGBAND_DIR_BONE = string_make(&argv[0][2]);
			break;

			case 'r':
			case 'R':
			new_game = TRUE;
			break;

			case 'F':
			case 'f':
			arg_fiddle = TRUE;
			break;

			case 'Z':
			case 'z':
			catch_signals = FALSE;
			break;

			case 'v':
				show_version();
			break;

			case 'h':
			default:
			usage:

			/* Note -- the Term is NOT initialized */
			puts("Usage: mangband [options]");
			puts("  -r       Reset the server");
			puts("  -z       Don't catch signals");
			puts("  -C<file> Use config file <file>");
			puts("  -e<path> Look for edit files in the directory <path>");
			puts("  -t<path> Look for help files in the directory <path>");
			puts("  -p<path> Look for pref files in the directory <path>");
			puts("  -d<path> Look for data files in the directory <path>");
			puts("  -s<path> Look for save files in the directory <path>");
			puts("  -b<path> Look for bone files in the directory <path>");

			/* Actually abort the process */
			quit(NULL);
		}
	}

	/* Note we are starting up */
	plog("Game Restarted");

	/* Tell "quit()" to call "Term_nuke()" */
	quit_aux = quit_hook;

	/* Catch nasty signals */
	if (catch_signals == TRUE)
		signals_init();

	/* Catch nasty "signals" on Windows */
#ifdef WINDOWS
#ifndef HANDLE_SIGNALS
	setup_exit_handler();
#endif
#endif

	/* Load the mangband.cfg options */
	load_server_cfg();

	/* Test existance of 'news.txt' and 'scores.raw' */
	show_news();

	/* Initialize the arrays */
	init_some_arrays();


	/* Prepare the game */
	play_game(new_game);

	/* Set up the network server */
	setup_network_server();

	/* Loop forever */
	network_loop();

	/* This should never, ever happen */
	plog("FATAL ERROR network_loop() returned!");

	/* Close stuff */
	close_game();

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}
