/* Main client module */

/*
 * This is intentionally minimal, as system specific stuff may
 * need be done in "main" (or "WinMain" for Windows systems).
 * The real non-system-specific initialization is done in
 * "c-init.c".
 */

#if !defined(USE_WIN) && !defined(USE_CRB)
#include "c-angband.h"

#ifdef USE_SDL
/* This is needed on some platforms to replace main via dark magic */
/* TODO: See if it breaks anything. Also, where is our ON_OSX define? */
#include <SDL.h>
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

int main(int argc, char *argv[])
{
	bool done = FALSE;

	/* Save the program name */
	argv0 = argv[0];

	/* Save command-line arguments */
	clia_init(argc, (const char**)argv);

	/* Client Config-file */
	conf_init(NULL);

	/* Setup the file paths */
	init_stuff();

	/* Attempt to initialize a visual module */
#ifdef USE_SDL
	/* Attempt to use the "main-sdl.c" support */
	if (!done)
	{
		extern errr init_sdl(void);
		if (0 == init_sdl()) done = TRUE;
		if (done) ANGBAND_SYS = "sdl";
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
	if (!done)
	{
		extern errr init_x11(int argc, char **argv);
		if (0 == init_x11(argc,argv)) done = TRUE;
		if (done) ANGBAND_SYS = "x11";
	}
#endif

#ifdef USE_GCU
	/* Attempt to use the "main-gcu.c" support */
	if (!done)
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
