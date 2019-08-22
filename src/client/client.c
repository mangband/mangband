/* Main client module */

/*
 * This is intentionally minimal, as system specific stuff may
 * need be done in "main" (or "WinMain" for Windows systems).
 * The real non-system-specific initialization is done in
 * "c-init.c".
 */

#if !defined(USE_WIN) && !defined(USE_CRB)
#include "c-angband.h"

/* This is needed on some platforms to replace main via dark magic */
/* TODO: See if it breaks anything. Also, where is our ON_OSX define? */
#if defined(USE_SDL2) || defined(USE_SDL)
#include <SDL.h>
#endif

#ifdef __APPLE__
// This is for setting the "root" path to the app's current dir on Mac OS X
// see below
#include "CoreFoundation/CoreFoundation.h"
#endif

static void read_credentials(void)
{
#ifdef SET_UID
	int player_uid;
	struct passwd *pw;
#endif

#ifdef WINDOWS
	char buffer[20] = {'\0'};
	DWORD bufferLen = sizeof(buffer);
#endif

	/* Initial defaults */
	strcpy(nick, "PLAYER");
	strcpy(pass, "passwd");
	strcpy(real_name, "PLAYER");

	/* Get login name if a UNIX machine */
#ifdef SET_UID
	/* Get player UID */
	player_uid = getuid();

	/* Get password entry */
	if ((pw = getpwuid(player_uid)))
	{
		/* Pull login id */
		my_strcpy(nick, pw->pw_name, MAX_CHARS);

		/* Cut */
		nick[MAX_NAME_LEN] = '\0';

		/* Copy to real name */
		my_strcpy(real_name, nick, MAX_CHARS);
	}
#endif

	/* Get user name from WINDOWS machine! */
#ifdef WINDOWS
	if ( GetUserName(buffer, &bufferLen) ) 
	{
		/* Cut */
		buffer[MAX_NAME_LEN] = '\0';
		
		/* Copy to real name */
		my_strcpy(real_name, buffer, MAX_CHARS);
	}
#endif
}

/*
 * --help, --version and main()
 */
const static char frontends[][8] = {
#ifdef USE_SDL
	"sdl",
#endif
#ifdef USE_SDL2
	"sdl2",
#endif
#ifdef USE_X11
	"x11",
#endif
#ifdef USE_GCU
	"gcu",
#endif
};
static int num_frontends = sizeof(frontends) / 8;
static void show_version()
{
	int i;
	printf("mangclient (MAngband Client) %d.%d.%d",
		CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_PATCH);
	if (CLIENT_VERSION_EXTRA == 1) printf("alpha");
	else if (CLIENT_VERSION_EXTRA == 2) printf("beta");
	else if (CLIENT_VERSION_EXTRA == 3) printf("devel");
	printf("\n");
	printf("Copyright (C) 2011-2019 MAngband Project Team.\n");
	printf("License: MAngband license, see <LICENSE> file.\n");
	printf("Compiled with ");
	for (i = 0; i < num_frontends; i++)
	{
		if (i) printf("%s", i == num_frontends - 1 ? " and " : ", ");
		printf("'%s'", frontends[i]);
	}
	if (!i) printf("no(!)");
	printf(" display modules.\n");
}
static void show_help()
{
	int i;
	printf("Usage: %s [OPTIONS] [SERVER [PORT]]\n", argv0);
	printf("\n");
	printf("SERVER can be a hostname or an IPv4 address. PORT can also be supplied.\n");
	printf("If SERVER is not specified, %s will try to find available servers via the metaserver.\n", argv0);
	printf("\n");
	printf("Options\n");
	printf("  -mDISPLAY                 Pick display module. \n");
	printf("                            Available modules are ");
	for (i = 0; i < num_frontends; i++)
	{
		if (i) printf("%s", i == num_frontends - 1 ? " and " : ", ");
		printf("'%s'", frontends[i]);
	}
	if (!i) printf(" ..NONE..");
	printf(".\n");
	printf("      --config PATH         Config file location.\n");
	printf("      --libdir PATH         Readable asset dir location.\n");
	printf("      --userdir PATH        Writable user dir location.\n");
	printf("      --nick NICKNAME       Character name to use.\n");
	printf("\nDisplay modules might support additional arguments.\n");
#ifdef USE_SDL
	{ extern const char help_sdl[];
	printf("%s", help_sdl); }
#endif
#ifdef USE_SDL2
	{ extern const char help_sdl2[];
	printf("%s", help_sdl2); }
#endif
#ifdef USE_X11
	{ extern const char help_x11[];
	printf("%s", help_x11); }
#endif
#ifdef USE_GCU
	{ extern const char help_gcu[];
	printf("%s", help_gcu); }
#endif
}
static bool exit_promptly(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "--version"))
		{
			show_version();
			return TRUE;
		}
		else if (!strcmp(argv[i], "--help"))
		{
			show_help();
			return TRUE;
		}
	}
	return FALSE;
}
/* FIXME: This "parser" is horrible */
static void pick_module(char *buf, size_t len, int argc, char *argv[])
{
	int i, j;
	for (i = 0; i < num_frontends; i++)
	{
		char tmp[32];
		sprintf(tmp, "-m%s", frontends[i]);
		for (j = 0; j < argc; j++)
		{
			if(!strcmp(argv[j], tmp))
			{
				strcpy(argv[j], "");/* <- probably illegal */
				my_strcpy(buf, frontends[i], len);
				break;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	char prefer_module[16] = { 0 };
	bool done = FALSE;

	/* Save the program name */
	argv0 = argv[0];

	/* Do --version and --help */
	if (exit_promptly(argc, argv)) return 0;

	/* Save command-line arguments */
	clia_init(argc, (const char**)argv);

	/* Client Config-file */
	conf_init(NULL);

	/* Pick preferred module */
	pick_module(prefer_module, sizeof(prefer_module), argc, argv);

#ifdef __APPLE__
	// Set our "cwd" to the directory the application bundle is in on OS X
	char path[PATH_MAX];
	CFURLRef res = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	CFURLGetFileSystemRepresentation(res, TRUE, (UInt8 *)path, PATH_MAX);
	CFRelease(res);
	chdir(path);
#endif

	/* Setup the file paths */
	init_stuff();

	/* Attempt to initialize a visual module */
#ifdef USE_SDL
	/* Attempt to use the "main-sdl.c" support */
	if (!done && (!strcmp(prefer_module, "sdl") || STRZERO(prefer_module)))
	{
		extern errr init_sdl(void);
		if (0 == init_sdl()) done = TRUE;
		if (done) ANGBAND_SYS = "sdl";
	}
#endif
#ifdef USE_SDL2
	if (!done && (!strcmp(prefer_module, "sdl2") || STRZERO(prefer_module)))
	{
		extern errr init_sdl2(void);
		if (init_sdl2() == 0) done = TRUE;
		if (done) ANGBAND_SYS = "sdl2";
	}
#endif


#ifdef USE_XAW
	/* Attempt to use the "main-xaw.c" support */
	if (!done)
	{
		extern errr init_xaw(void);
		if (0 == init_xaw()) done = TRUE;
		if (done) ANGBAND_SYS = "xaw";
	}
#endif

#ifdef USE_X11
	/* Attempt to use the "main-x11.c" support */
	if (!done && (!strcmp(prefer_module, "x11") || STRZERO(prefer_module)))
	{
		extern errr init_x11(int argc, char **argv);
		if (0 == init_x11(argc,argv)) done = TRUE;
		if (done) ANGBAND_SYS = "x11";
	}
#endif

#ifdef USE_GCU
	/* Attempt to use the "main-gcu.c" support */
	if (!done && (!strcmp(prefer_module, "gcu") || STRZERO(prefer_module)))
	{
		extern errr init_gcu(void);
		if (0 == init_gcu()) done = TRUE;
		if (done) ANGBAND_SYS = "gcu";
	}
#endif

#ifdef USE_IBM
	/* Attempt to use the "main_ibm.c" support */
	if (!done)
	{
		extern errr init_ibm(void);
		if (0 == init_ibm()) done = TRUE;
		if (done) ANGBAND_SYS = "ibm";
	}
#endif

#ifdef USE_EMX
	/* Attempt to use the "main-emx.c" support */
	if (!done)
	{
		extern errr init_emx(void);
		if (0 == init_emx()) done = TRUE;
		if (done) ANGBAND_SYS = "emx";
	}
#endif

	/* No visual module worked */
	if (!done)
	{
		printf("Unable to initialize a display module!\n");
		exit(1);
	}

	/* Attempt to read default name/real name from OS */
	read_credentials();

	/** Initialize client and run main loop **/
	client_init();

	return 0;
}

#endif
