/* File: main-win.c */

/* Purpose: Support for Windows Angband */

/*
 * Written (2.7.8) by Skirmantas Kligys (kligys@scf.usc.edu)
 *
 * Based loosely on "main-mac.c" and "main-xxx.c" and suggestions
 * by Ben Harrison (benh@voicenet.com).
 *
 * Upgrade to Angband 2.7.9v6 by Ben Harrison (benh@voicenet.com),
 * Ross E Becker (beckerr@cis.ohio-state.edu), and Chris R. Martin
 * (crm7479@tam2000.tamu.edu).
 *
 * Note that the "Windows" version requires several extra files, which
 * must be placed in various places.  These files are distributed in a
 * special "ext-win.zip" archive, with instructions on where to place
 * the various files.  For example, we require that all font files,
 * bitmap files, and sound files, be placed into the "lib/xtra/font/",
 * "/lib/xtra/graf/", and "lib/xtra/sound/" directories, respectively.
 *
 * See "h-config.h" for the extraction of the "WINDOWS" flag based
 * on the "_Windows", "__WINDOWS__", "__WIN32__", "WIN32", "__WINNT__",
 * or "__NT__" flags.  If your compiler uses a different compiler flag,
 * add it to "h-config.h", or, simply place it in the "Makefile".
 *
 *
 * This file still needs some work, possible problems are indicated by
 * the string "XXX XXX XXX" in any comment.
 *
 * XXX XXX XXX
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things.
 *
 * XXX XXX XXX
 * The user should be able to select the "bitmap" for a window independant
 * from the "font", and should be allowed to select any bitmap file which
 * is strictly smaller than the font file.  Currently, bitmap selection
 * always imitates the font selection unless the "Font" and "Graf" lines
 * are used in the "ANGBAND.INI" file.  This may require the addition
 * of a menu item for each window to select the bitmaps.
 *
 * XXX XXX XXX
 * The various "warning" messages assume the existance of the "screen.w"
 * window, I think, and only a few calls actually check for its existance,
 * this may be okay since "NULL" means "on top of all windows". (?)
 *
 * XXX XXX XXX
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.  These files *may* be available
 * at the ftp site somewhere.
 *
 * XXX XXX XXX
 * The "prepare menus" command should "gray out" any menu command which
 * is not allowed at the current time.  This will simplify the actual
 * processing of menu commands, which can assume the command is legal.
 *
 * XXX XXX XXX
 * Remove the old "FontFile" entries from the "*.ini" file, and remove
 * the entire section about "sounds", after renaming a few sound files.
 */


#include "c-angband.h"

#ifdef USE_WIN

#define MNU_SUPPORT

/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif

/*
 * XXX XXX XXX Hack -- broken sound libraries
 */
#ifdef BEN_HACK
# undef USE_SOUND
#endif


/*
 * Menu constants -- see "ANGBAND.RC"
 */

#define IDM_FILE_NEW			101
#define IDM_FILE_OPEN			102
#define IDM_FILE_SAVE			103
#define IDM_FILE_EXIT			104
#define IDM_FILE_QUIT			105

#define IDM_TEXT_SCREEN			201
#define IDM_TEXT_MIRROR			202
#define IDM_TEXT_RECALL			203
#define IDM_TEXT_CHOICE			204
#define IDM_TEXT_TERM_4			205
#define IDM_TEXT_TERM_5			206
#define IDM_TEXT_TERM_6			207
#define IDM_TEXT_TERM_7			208

#define IDM_WINDOWS_SCREEN		211
#define IDM_WINDOWS_MIRROR		212
#define IDM_WINDOWS_RECALL		213
#define IDM_WINDOWS_CHOICE		214
#define IDM_WINDOWS_TERM_4		215
#define IDM_WINDOWS_TERM_5		216
#define IDM_WINDOWS_TERM_6		217
#define IDM_WINDOWS_TERM_7		218

#define IDM_GRAPHICS_OFF	221
#define IDM_GRAPHICS_BIG_TILE	222
#define IDM_GRAPHICS_TILESET_1	223
#define IDM_GRAPHICS_TILESET_2	224
#define IDM_GRAPHICS_TILESET_3	225
#define IDM_GRAPHICS_TILESET_4	226
#define IDM_GRAPHICS_TILESET_5	227
#define IDM_GRAPHICS_TILESET_6	228
#define IDM_GRAPHICS_TILESET_7	229
#define IDM_GRAPHICS_TILESET_8	230

#define IDM_OPTIONS_SOUND		301
#define IDM_OPTIONS_MOUSE		302
#define IDM_OPTIONS_UNUSED		231
#define IDM_OPTIONS_SAVER		232

#define IDM_HELP_GENERAL		901
#define IDM_HELP_SPOILERS		902
#define IDM_HELP_ABOUT			903

/*
 * exclude parts of WINDOWS.H that are not needed
 */
#define NOSOUND           /* Sound APIs and definitions */
#define NOCOMM            /* Comm driver APIs and definitions */
#define NOLOGERROR        /* LogError() and related definitions */
#define NOPROFILER        /* Profiler APIs */
#define NOLFILEIO         /* _l* file I/O routines */
#define NOOPENFILE        /* OpenFile and related definitions */
#define NORESOURCE        /* Resource management */
#define NOATOM            /* Atom management */
#define NOLANGUAGE        /* Character test routines */
#define NOLSTRING         /* lstr* string management routines */
#define NODBCS            /* Double-byte character set routines */
#define NOKEYBOARDINFO    /* Keyboard driver routines */
#define NOCOLOR           /* COLOR_* color values */
#define NODRAWTEXT        /* DrawText() and related definitions */
#define NOSCALABLEFONT    /* Truetype scalable font support */
#define NOMETAFILE        /* Metafile support */
#define NOSYSTEMPARAMSINFO /* SystemParametersInfo() and SPI_* definitions */
#define NODEFERWINDOWPOS  /* DeferWindowPos and related definitions */
#define NOKEYSTATES       /* MK_* message key state flags */
#define NOWH              /* SetWindowsHook and related WH_* definitions */
#define NOCLIPBOARD       /* Clipboard APIs and definitions */
#define NOICONS           /* IDI_* icon IDs */
#define NOMDI             /* MDI support */
#define NOCTLMGR          /* Control management and controls */
#define NOHELP            /* Help support */

/*
 * exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */

/*
 * Include the "windows" support file
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCK2API_
//#include <winsock2.h>
#endif


/*
 * exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOTIMER        /* Timer support */
#define MMNOJOY          /* Joystick support */
#define MMNOMCI          /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

/*
 * Include the ??? files
 */
#include <mmsystem.h>
#include <commdlg.h>

/*
 * Include the support for loading bitmaps
 */
#ifdef USE_GRAPHICS
# include "win/readdib.h"
#endif

/*
 * Hack -- allow use of the Borg as a screen-saver
 */
#ifdef ALLOW_BORG
# define ALLOW_SCRSAVER
#endif

/*
 * Cannot include "dos.h", so we define some things by hand.
 */
#ifdef WIN32
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#else /* WIN32 */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#endif /* WIN32 */

/*
 * Silliness in WIN32 drawing routine
 */
#ifdef WIN32
# define MoveTo(H,X,Y) MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

/*
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif

/*
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK	0x00
#define VID_BLUE	0x01
#define VID_GREEN	0x02
#define VID_CYAN	0x03
#define VID_RED		0x04
#define VID_MAGENTA	0x05
#define VID_YELLOW	0x06
#define VID_WHITE	0x07

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/*
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK	0x00
#define VUD_BLUE	0x10
#define VUD_GREEN	0x20
#define VUD_CYAN	0x30
#define VUD_RED		0x40
#define VUD_MAGENTA	0x50
#define VUD_YELLOW	0x60
#define VUD_WHITE	0x70

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT	0x80


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 *
 * Note the use of "font_want" and "graf_want" for the names of the
 * font/graf files requested by the user, and the use of "font_file"
 * and "graf_file" for the currently active font/graf files.
 *
 * The "font_file" and "graf_file" are capitilized, and are of the
 * form "8X13.FON" and "8X13.BMP", while "font_want" and "graf_want"
 * can be in almost any form as long as it could be construed as
 * attempting to represent the name of a font or bitmap or file.
 */
struct _term_data
{
	term     t;

	cptr     s;

	HWND     w;

#ifdef USE_GRAPHICS
	
#endif

	DWORD    dwStyle;
	DWORD    dwExStyle;

	uint     keys;

	uint     rows;
	uint     cols;

	uint     pos_x;
	uint     pos_y;
	uint     size_wid;
	uint     size_hgt;
	uint     client_wid;
	uint     client_hgt;
	uint     size_ow1;
	uint     size_oh1;
	uint     size_ow2;
	uint     size_oh2;

	byte     visible;

	byte     size_hack;

	cptr     font_want;
	cptr     graf_want;

	cptr     font_file;

	HFONT    font_id;

	uint     font_wid;
	uint     font_hgt;
};

#ifdef USE_GRAPHICS
/*
* Flag set once "graphics" has been initialized
*/
static bool can_use_graphics = FALSE;

/*
* The global bitmap
*/
static DIBINIT infGraph;

/*
* The global bitmap mask
*/
static DIBINIT infMask;

#endif 
/*
 * Maximum number of windows XXX XXX XXX XXX
 */
#define MAX_TERM_DATA 8

/*
 * An array of term_data's
 */
static term_data win_data[MAX_TERM_DATA];

/*
 * Mega-Hack -- global "window creation" pointer
 */
static term_data *td_ptr;

/*
 * Even bigger hack than the above -- global edit control handle [grk]
 */
static HWND editmsg;
static HWND old_focus = NULL;
LRESULT APIENTRY SubClassFunc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
WNDPROC lpfnOldWndProc;

/*
 * Various boolean flags
 */
bool game_in_progress  = FALSE;  /* game in progress */
bool initialized       = FALSE;  /* note when "open"/"new" become valid */
bool paletted          = FALSE;  /* screen paletted, i.e. 256 colors */
bool colors16          = FALSE;  /* 16 colors screen, don't use RGB() */
bool use_mouse         = FALSE;  /* game accepts mouse */

/*
 * Saved instance handle
 */
static HINSTANCE hInstance;

/*
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/*
 * Black brush for the chat window edit control
 */
static HBRUSH hbrBlack;

/*
 * An icon
 */
static HICON hIcon;

/*
 * A palette
 */
static HPALETTE hPal;

#ifdef ALLOW_SCRSAVER

/*
 * The screen saver
 */
static HWND hwndSaver;

#endif

/*
 * Full path to ANGBAND.INI
 */
static cptr ini_file = NULL;

/*
 * Name of application
 */
static cptr AppName  = "ANGBAND";

/*
 * Name of sub-window type
 */
static cptr AngList  = "AngList";
static int next_graphics = 0;
static int loaded_graphics = 0;
/*
 * Directory names
 */
static cptr ANGBAND_DIR_XTRA_FONT;
#define ANGBAND_DIR_XTRA_HELP ".\\lib\\text"

#ifdef USE_SOUND

/*
 * Flag set once "sound" has been initialized
 */
static bool can_use_sound = FALSE;

#endif /* USE_SOUND */

/*
 * The Angband color set:
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 * Note that on B/W machines, all non-zero colors can be white (on black).
 *
 * Note that all characters are assumed to be drawn on a black background.
 * This may require calling "Term_wipe()" before "Term_text()", etc.
 *
 * XXX XXX XXX See "main-ibm.c" for a method to allow color editing
 *
 * XXX XXX XXX The color codes below were taken from "main-ibm.c".
 */
static COLORREF win_clr[16] =
{
    PALETTERGB(0x00, 0x00, 0x00),  /* 0 0 0  Dark */
    PALETTERGB(0xFF, 0xFF, 0xFF),  /* 4 4 4  White */
    PALETTERGB(0x90, 0x90, 0x90),  /* 2 2 2  Slate */
    PALETTERGB(0xFF, 0x80, 0x00),  /* 4 2 0  Orange */
    PALETTERGB(0xFF, 0x20, 0x20),  /* 3 0 0  Red (was 2,0,0) */
    PALETTERGB(0x20, 0x8D, 0x44),  /* 0 2 1  Green */
    PALETTERGB(0x20, 0x50, 0xC0),  /* 0 0 4  Blue */
    PALETTERGB(0x8D, 0x44, 0x20),  /* 2 1 0  Umber */
    PALETTERGB(0x60, 0x60, 0x60),  /* 1 1 1  Lt. Dark */
    PALETTERGB(0xC0, 0xC0, 0xC0),  /* 3 3 3  Lt. Slate */
    PALETTERGB(0xB0, 0x30, 0xFF),  /* 4 0 4  Violet (was 2,0,2) */
    PALETTERGB(0xEF, 0xDF, 0x40),  /* 4 4 0  Yellow */
    // PALETTERGB(0xFF, 0x70, 0x50),  /* 4 0 0  Lt. Red (was 4,1,3) */
    PALETTERGB(0xFF, 0x80, 0x80),  /* 4 0 0  Lt. Red (was 4,1,3) */
    PALETTERGB(0x30, 0xFF, 0x30),  /* 0 4 0  Lt. Green */
    PALETTERGB(0x30, 0xD0, 0xFF),  /* 0 4 4  Lt. Blue */
    PALETTERGB(0xFF, 0xA8, 0x20)   /* 3 2 1  Lt. Umber */


#if 0
	PALETTERGB(0x00, 0x00, 0x00),  /* 0 0 0  Dark */
	PALETTERGB(0xFF, 0xFF, 0xFF),  /* 4 4 4  White */
	PALETTERGB(0x8D, 0x8D, 0x8D),  /* 2 2 2  Slate */
	PALETTERGB(0xFF, 0x8D, 0x00),  /* 4 2 0  Orange */
	PALETTERGB(0xD7, 0x00, 0x00),  /* 3 0 0  Red (was 2,0,0) */
	PALETTERGB(0x00, 0x8D, 0x44),  /* 0 2 1  Green */
	PALETTERGB(0x00, 0x00, 0xFF),  /* 0 0 4  Blue */
	PALETTERGB(0x8D, 0x44, 0x00),  /* 2 1 0  Umber */
	PALETTERGB(0x54, 0x54, 0x54),  /* 1 1 1  Lt. Dark */
	PALETTERGB(0xD7, 0xD7, 0xD7),  /* 3 3 3  Lt. Slate */
	PALETTERGB(0xFF, 0x00, 0xFF),  /* 4 0 4  Violet (was 2,0,2) */
	PALETTERGB(0xFF, 0xFF, 0x00),  /* 4 4 0  Yellow */
	PALETTERGB(0xFF, 0x44, 0xD7),  /* 4 0 0  Lt. Red (was 4,1,3) */
	PALETTERGB(0x00, 0xFF, 0x00),  /* 0 4 0  Lt. Green */
	PALETTERGB(0x00, 0xFF, 0xFF),  /* 0 4 4  Lt. Blue */
	PALETTERGB(0xD7, 0x8D, 0x44)   /* 3 2 1  Lt. Umber */
#endif
};


/*
 * Palette indices for 16 colors
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static BYTE win_pal[16] =
{
	VID_BLACK,			/* Dark */
	VID_WHITE,			/* White */
	VID_CYAN,			/* Slate XXX */
	VID_RED | VID_BRIGHT,	/* Orange XXX */
	VID_RED,			/* Red */
	VID_GREEN,			/* Green */
	VID_BLUE,			/* Blue */
	VID_YELLOW,			/* Umber XXX */
	VID_BLACK | VID_BRIGHT,	/* Light Dark */
	VID_CYAN | VID_BRIGHT,	/* Light Slate XXX */
	VID_MAGENTA,		/* Violet XXX */
	VID_YELLOW | VID_BRIGHT,	/* Yellow */
	VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
	VID_GREEN | VID_BRIGHT,	/* Light Green */
	VID_BLUE | VID_BRIGHT,	/* Light Blue */
	VID_YELLOW			/* Light Umber XXX */
};

/* [grk]
 * A gross hack to allow the client to scroll the dungeon display.
 * This is required for large graphical tiles where we cant have an
 * 66x22 tile display except in very high screen resolutions.
 * When the server supports player recentering this can go.
 *
 * When we receive a request to plot a tile at a location, we
 * shift the x-coordinate by this value. If the resultant
 * x-coordinate is negative we just ignore it and plot nothing.
 *
 * We only need scrolling along the x axis.
 */
static char x_offset = 0;

/* Hack -- set focus to chat message control */
void set_chat_focus( void )
{
	old_focus = GetFocus();
	SetFocus(editmsg);
}

void unset_chat_focus( void )
{
	/* Set focus back to original window */
	if(old_focus) SetFocus(old_focus);
}

void set_graphics_next(int mode)
{
	// char buf[1024];

	next_graphics = mode;
	if (next_graphics != use_graphics)
	{
		MessageBox(NULL, "You need to restart MAngband in order for the changes to take effect","MAngband",MB_OK);	
		
		/*
		save_prefs();
		*/

		/* Access the "graphic" mappings */
		//sprintf(buf, "%s-%s.prf", (use_graphics ? "graf" : "font"), ANGBAND_SYS);

		/* Load the file */
		//process_pref_file(buf);
	}
}

void stretch_chat_ctrl( void )
{
	/* Resize the edit control */
	SetWindowPos(editmsg, 0, 2, win_data[4].client_hgt-21,
	             win_data[4].client_wid-6, 20,
	             SWP_NOZORDER);
}

int win32_window_visible(int i)
{
	return (bool)win_data[i].visible;
}

/*
 * Hack -- given a pathname, point at the filename
 */
static cptr extract_file_name(cptr s)
{
	cptr p;

	/* Start at the end */
	p = s + strlen(s) - 1;

	/* Back up to divider */
	while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

	/* Return file name */
	return (p+1);
}



/*
 * Check for existance of a file
 */
static bool check_file(cptr s)
{
	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Prohibit directory */
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Prohibit directory */
	if (attrib & FA_DIREC) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Check for existance of a directory
 */
static bool check_dir(cptr s)
{
	int i;

	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

	/* Check length */
	i = strlen(path);

	/* Remove trailing backslash */
	if (i && (path[i-1] == '\\')) path[--i] = '\0';

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Require directory */
	if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Require directory */
	if (!(attrib & FA_DIREC)) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Validate a file
 */
static void validate_file(cptr s)
{
	/* Verify or fail */
	if (!check_file(s))
	{
		quit_fmt("Cannot find required file:\n%s", s);
	}
}


/*
 * Validate a directory
 */
static void validate_dir(cptr s)
{
	/* Verify or fail */
	if (!check_dir(s))
	{
		quit_fmt("Cannot find required directory:\n%s", s);
	}
}


/*
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
	RECT        rc;

	/* Paranoia */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	/* Paranoia */
	if (conn_state && td == &win_data[0])
	{
/*		if (td->cols < Setup.min_col) td->cols = Setup.min_col;
		if (td->rows < Setup.min_row) td->rows = Setup.min_row;
		if (td->cols > Setup.max_col + SCREEN_CLIP_X) td->cols = Setup.max_col + SCREEN_CLIP_X;
		if (td->rows > Setup.max_row + SCREEN_CLIP_Y) td->rows = Setup.max_row + SCREEN_CLIP_Y; */
	}

	/* Window sizes */
	td->client_wid = td->cols * td->font_wid + td->size_ow1 + td->size_ow2;
	td->client_hgt = td->rows * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;

	/* Fake window size */
	rc.left = rc.top = 0;
	rc.right = rc.left + td->client_wid;
	rc.bottom = rc.top + td->client_hgt;

	/* Adjust */
	AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

	/* Total size */
	td->size_wid = rc.right - rc.left;
	td->size_hgt = rc.bottom - rc.top;

	/* See CreateWindowEx */
	if (!td->w) return;

	/* Extract actual location */
	GetWindowRect(td->w, &rc);

	/* Save the location */
	td->pos_x = rc.left;
	td->pos_y = rc.top;
}


/*
 * Write the "preference" data for single term
 */
static void save_prefs_aux(term_data *td, cptr sec_name)
{
	char buf[32];

	RECT rc;

	if (!td->w) return;

	/* Visible (Sub-windows) */
	if (td != &win_data[0])
	{
		//strcpy(buf, td->visible ? "1" : "0");
		conf_set_int(sec_name, "Visible", td->visible);
	}

	/* Desired font */
	if (td->font_file)
	{
		/* Short-hand */
		if (!strncmp(td->font_file, ANGBAND_DIR_XTRA_FONT, strlen(ANGBAND_DIR_XTRA_FONT)))
		{
			strcpy(buf, td->font_file + strlen(ANGBAND_DIR_XTRA_FONT) + 1);
			conf_set_string(sec_name, "Font", buf);
		}
		else
		/* Full path */
		conf_set_string(sec_name, "Font", td->font_file);
	}

	/* Bad Hack :( -- since we allow slight overhead, make sure we're in bounds */
/*	if (td == &win_data[0] && Setup.max_col && !(window_flag[0] & PW_PLAYER_2) && td->cols > Setup.max_col) td->cols = Setup.max_col; // Compact
	if (td == &win_data[0] && Setup.max_row && !(window_flag[0] & PW_STATUS)   && td->rows > Setup.max_row) td->rows = Setup.max_row; // Status line	 */

	/* Current size (x) */
	//wsprintf(buf, "%d", td->cols);
	conf_set_int(sec_name, "Columns", td->cols);

	/* Current size (y) */
	//wsprintf(buf, "%d", td->rows);
	conf_set_int(sec_name, "Rows", td->rows);

	/* Acquire position */
	if (GetWindowRect(td->w, &rc))
	{
		/* Current position (x) */
		//wsprintf(buf, "%d", rc.left);
		conf_set_int(sec_name, "PositionX", rc.left);

		/* Current position (y) */
		//wsprintf(buf, "%d", rc.top);
		conf_set_int(sec_name, "PositionY", rc.top);
	}
}


/*
 * Write the "preference" data to the .INI file
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
#ifdef USE_GRAPHICS
	conf_set_int("Windows32", "Graphics", next_graphics);
#endif
#ifdef USE_SOUND
	conf_set_int("Windows32", "Sound", use_sound);
#endif
	conf_set_int("Windows32", "GameMouse", use_mouse);

	save_prefs_aux(&win_data[0], "Main window");

	/* XXX XXX XXX XXX */

	save_prefs_aux(&win_data[1], "Mirror window");

	save_prefs_aux(&win_data[2], "Recall window");

	save_prefs_aux(&win_data[3], "Choice window");

	save_prefs_aux(&win_data[4], "Term-4 window");

	save_prefs_aux(&win_data[5], "Term-5 window");

	save_prefs_aux(&win_data[6], "Term-6 window");

	save_prefs_aux(&win_data[7], "Term-7 window");
}


/*
 * Load preference for a single term
 */
static void load_prefs_aux(term_data *td, cptr sec_name)
{
	char tmp[128];

	/* Visibility (Sub-window) */
	if (td != &win_data[0])
	{
		/* Extract visibility */
		td->visible = (conf_get_int(sec_name, "Visible", td->visible) != 0);
	}

	/* Desired font, with default */
	strcpy(tmp, conf_get_string(sec_name, "Font", "8X13.FON"));
	td->font_want = string_make(extract_file_name(tmp));

	/* Desired graf, with default */
	//strcpy(tmp, conf_get_string(sec_name, "Graf", GFXBMP[use_graphics]));
	//td->graf_want = string_make(extract_file_name(tmp));

	/* Window size */
	td->cols = conf_get_int(sec_name, "Columns", td->cols);
	td->rows = conf_get_int(sec_name, "Rows", td->rows);

	/* Window position */
	td->pos_x = conf_get_int(sec_name, "PositionX", td->pos_x);
	td->pos_y =	conf_get_int(sec_name, "PositionY", td->pos_y);
}

/*
 * Load the preferences from the .INI file
 */
static void load_prefs(void)
{
	char buffer[20] = {'\0'};
	DWORD bufferLen = sizeof(buffer);

#ifdef USE_GRAPHICS
	/* Extract the "use_graphics" flag */
	loaded_graphics = conf_get_int("Windows32", "Graphics", 0);
	set_graphics(loaded_graphics);
	set_graphics_next(loaded_graphics);
#endif

#ifdef USE_SOUND
	/* Extract the "use_sound" flag */
	use_sound = (conf_get_int("Windows32", "Sound", 0) != 0);
#endif

	/* Extract the "use_mouse" flag */
	use_mouse = (conf_get_int("Windows32", "GameMouse", 1) == 1);

	/* Load window prefs */
	load_prefs_aux(&win_data[0], "Main window");

	/* XXX XXX XXX XXX */

	load_prefs_aux(&win_data[1], "Mirror window");

	load_prefs_aux(&win_data[2], "Recall window");

	load_prefs_aux(&win_data[3], "Choice window");

	load_prefs_aux(&win_data[4], "Term-4 window");

	load_prefs_aux(&win_data[5], "Term-5 window");

	load_prefs_aux(&win_data[6], "Term-6 window");

	load_prefs_aux(&win_data[7], "Term-7 window");

	/* Pull nick/pass */
	my_strcpy(nick, conf_get_string("MAngband", "nick", "PLAYER"), MAX_CHARS);
	my_strcpy(pass, conf_get_string("MAngband", "pass", "passwd"), MAX_CHARS);
	my_strcpy(server_name, conf_get_string("MAngband", "host", ""), MAX_CHARS);

	/* Pull username from Windows */
	if ( GetUserName(buffer, &bufferLen) ) {
		/* Cut */
		buffer[16] = '\0';
		strcpy(real_name, buffer);

	}
	else
	{
		/* XXX Default real name */
		strcpy(real_name, "PLAYER");
	}
}


/*
 * Create the new global palette based on the bitmap palette
 * (if any) from the given bitmap xxx, and the standard 16
 * entry palette derived from "win_clr[]" which is used for
 * the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 */
static int new_palette(void)
{
	HPALETTE       hBmPal;
	HPALETTE       hNewPal;
	HDC            hdc;
	int            i, nEntries;
	int            pLogPalSize;
	int            lppeSize;
	LPLOGPALETTE   pLogPal;
	LPPALETTEENTRY lppe;


	/* Cannot handle palettes */
	if (!paletted) return (TRUE);

	/* No palette */
	hBmPal = NULL;

	/* No bitmap */
	lppeSize = 0;
	lppe = NULL;
	nEntries = 0;

#ifdef USE_GRAPHICS

	/* Check the bitmap palette */
	hBmPal = infGraph.hPalette;

	/* Use the bitmap */
	if (hBmPal)
	{
		lppeSize = 256*sizeof(PALETTEENTRY);
		lppe = (LPPALETTEENTRY)ralloc(lppeSize);
		nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
		if (nEntries == 0) {
			plog("Corrupted bitmap palette");
			FREE(lppe);
			return (FALSE);
		}
		else if (nEntries > 220) {
			plog("Bitmap must have no more than 220 colors");
			FREE(lppe);
			return (FALSE);
		}
	}

#endif

	/* Size of palette */
	pLogPalSize = sizeof(LOGPALETTE) + (16+nEntries)*sizeof(PALETTEENTRY);

	/* Allocate palette */
	pLogPal = (LPLOGPALETTE)ralloc(pLogPalSize);

	/* Version */
	pLogPal->palVersion = 0x300;

	/* Make room for bitmap and normal data */
	pLogPal->palNumEntries = nEntries + 16;

	/* Save the bitmap data */
	for (i = 0; i < nEntries; i++)
	{
		pLogPal->palPalEntry[i] = lppe[i];
	}

	/* Save the normal data */
	for (i = 0; i < 16; i++)
	{
		LPPALETTEENTRY p;

		/* Access the entry */
		p = &(pLogPal->palPalEntry[i+nEntries]);

		/* Save the colors */
		p->peRed = GetRValue(win_clr[i]);
		p->peGreen = GetGValue(win_clr[i]);
		p->peBlue = GetBValue(win_clr[i]);

		/* Save the flags */
		p->peFlags = PC_NOCOLLAPSE;
	}

	/* Free something */
	if (lppe) FREE(lppe);

	/* Create a new palette, or fail */
	hNewPal = CreatePalette(pLogPal);
	if (!hNewPal) quit("Cannot create palette");

	/* Free the palette */
	FREE(pLogPal);


	hdc = GetDC(win_data[0].w);
	SelectPalette(hdc, hNewPal, 0);
	i = RealizePalette(hdc);
	ReleaseDC(win_data[0].w, hdc);
	if (i == 0) quit("Cannot realize palette");

	/* Check windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		hdc = GetDC(win_data[i].w);
		SelectPalette(hdc, hNewPal, 0);
		ReleaseDC(win_data[i].w, hdc);
	}

	/* Delete old palette */
	if (hPal) DeleteObject(hPal);

	/* Save new palette */
	hPal = hNewPal;

	/* Success */
	return (TRUE);
}


#ifdef USE_SOUND
/*
 * Initialize sound
 */
static bool init_sound(void)
{
	/* Initialize once */
	if (!can_use_sound)
	{
		/* Load the prefs */
		load_sound_prefs();

		/* Sound available */
		can_use_sound = TRUE;
	}

	/* Result */
	return (can_use_sound);
}
#endif /* USE_SOUND */


/*
 * Resize a window
 */
static void term_window_resize(term_data *td)
{

	/* Require window */
	if (!td->w) return;

	/* Resize the window */
	SetWindowPos(td->w, 0, 0, 0,
	             td->size_wid, td->size_hgt,
	             SWP_NOMOVE | SWP_NOZORDER);

	/* Redraw later */
	InvalidateRect(td->w, NULL, TRUE);
}

/*
 * See if any other term is already using font_file
 */
static bool term_font_inuse(term_data* td)
{
	int i;
	bool used = FALSE;

	/* Scan windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Check "screen" */
		if ((td != &win_data[i]) &&
		    (win_data[i].font_file) &&
		    (streq(win_data[i].font_file, td->font_file)))
		{
			used = TRUE;
			break;
		}
	}
	return used;
}

/*
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 */
static errr term_force_font(term_data *td, cptr name)
{
	int i;

	int wid, hgt;

	cptr s;

	char base[16];

	char base_font[16];

	char buf[1024];


	/* Forget the old font (if needed) */
	if (td->font_id) DeleteObject(td->font_id);

	/* Forget old font */
	if (td->font_file)
	{
		bool used = term_font_inuse(td);

		/* Remove unused font resources */
		if (!used) RemoveFontResource(td->font_file);

		/* Free the old name */
		string_free(td->font_file);

		/* Forget it */
		td->font_file = NULL;
	}


	/* No name given */
	if (!name) return (1);

	/* Extract the base name (with suffix) */
	s = extract_file_name(name);

	/* Extract font width */
	wid = atoi(s);

	/* Default font height */
	hgt = 0;

	/* Copy, capitalize, remove suffix, extract width */
	for (i = 0; (i < 16 - 1) && s[i] && (s[i] != '.'); i++)
	{
		/* Capitalize */
		base[i] = FORCEUPPER(s[i]);

		/* Extract "hgt" when found */
		if (base[i] == 'X') hgt = atoi(s+i+1);
	}

	/* Terminate */
	base[i] = '\0';


	/* Build base_font */
	strcpy(base_font, base);
	strcat(base_font, ".FON");


	/* Access the font file */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, base_font);


	/* Verify file */
	if (!check_file(buf)) return (1);


	/* Save new font name */
	td->font_file = string_make(buf);

	/* If this font is used for the first time */
	if (!term_font_inuse(td))
	{
		/* Load the new font or quit */
		if (!AddFontResource(buf))
		{
			quit_fmt("Font file corrupted:\n%s", buf);
		}
	}

	/* Create the font XXX XXX XXX Note use of "base" */
	td->font_id = CreateFont(hgt, wid, 0, 0, FW_DONTCARE, 0, 0, 0,
	                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         FIXED_PITCH | FF_DONTCARE, base);


	/* Hack -- Unknown size */
	if (!wid || !hgt)
	{
		HDC         hdcDesktop;
		HFONT       hfOld;
		TEXTMETRIC  tm;

		/* all this trouble to get the cell size */
		hdcDesktop = GetDC(HWND_DESKTOP);
		hfOld = SelectObject(hdcDesktop, td->font_id);
		GetTextMetrics(hdcDesktop, &tm);
		SelectObject(hdcDesktop, hfOld);
		ReleaseDC(HWND_DESKTOP, hdcDesktop);

		/* Font size info */
		wid = tm.tmAveCharWidth;
		hgt = tm.tmHeight;
	}

	/* Save the size info */
	td->font_wid = wid;
	td->font_hgt = hgt;


	/* Analyze the font */
	term_getsize(td);

	/* Resize the window */
	term_window_resize(td);

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * Force the use of a new "graf file" for a term_data
 *
 * This function is never called before the windows are ready
 *
 * This function returns zero only if everything succeeds.
 */
static errr term_force_graf(term_data *td, graphics_mode *gm)
{
	int i, is_png = 0;

	int wid, hgt;

	cptr s;

	char buf[1024];

	/* No name */
	if (!gm->file) return (1);

	/* Extract the base name (with suffix) */
	s = gm->file;

	/* Extract font width */
	wid = gm->cell_width;

	/* Default font height */
	hgt = gm->cell_height;

	/* Require actual sizes */
	if (!wid || !hgt) return (1);

	/* Check if we need PNG loader */
	if (isuffix(s, ".png")) is_png = TRUE;

	/* Access the graf file */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_GRAF, gm->file);

	/* Verify file */
	if (!check_file(buf)) return (1);

	/* Note: PNG loader will extract the mask from the alpha
	 * channel, or assume colorkey is at pixel0,0 */
	if (is_png)
	{
		/* Load the png or quit */
 		if (!ReadDIB2_PNG(td->w, buf, &infGraph, &infMask, FALSE)) 
		{
			quit_fmt("Cannot read file '%s'", buf);
		}
	}
	else
	{
		/* Load the bitmap or quit */
		if (!ReadDIB(td->w, buf, &infGraph))
		{
			quit_fmt("Bitmap corrupted:\n%s", buf);
		}

		/* Load mask, if appropriate */
		if (!STRZERO(gm->mask))
		{
			/* Access the mask file */
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, gm->mask);

			/* Load the bitmap or quit */
			if (!ReadDIB(win_data[0].w, buf, &infMask))
			{
				quit_fmt("Cannot read bitmap mask file '%s'", buf);
			}
		}
	}
	/* Save the new sizes */
	infGraph.CellWidth = wid;
	infGraph.CellHeight = hgt;

	/* Copy the picture from the bitmap to the window */
//	BitBlt(hdc, x2, y2, w1, h1, hdcSrc, x1, y1, SRCCOPY);

	/* Activate a palette */
	if (!new_palette())
	{
		/* Free bitmap XXX XXX XXX */

		/* Oops */
		plog("Cannot activate palette!");
		return (FALSE);
	}
	/* Graphics available */
	can_use_graphics = use_graphics;

	/* Success */
	return (can_use_graphics);
}

#endif


/*
 * Allow the user to change the font (and graf) for this window.
 *
 * XXX XXX XXX This is only called for non-graphic windows
 */
static void term_change_font(term_data *td)
{
	OPENFILENAME ofn;
	TCHAR fullFileName[2048];
	char tmp[1024] = "";

	/* Extract a default if possible */
	if (td->font_file) strcpy(tmp, td->font_file);

	/* No default? Let's build it */
	if (STRZERO(tmp))
	{
		strnfmt(tmp, 1024, "%s%s", ANGBAND_DIR_XTRA_FONT, "\\*.fon");
	}

	/* Resolve absolute path */
	if (_fullpath(fullFileName, tmp, 2048) == NULL)
	{
		/* Complete and utter despair... */
		strcpy(fullFileName, "\\*.fon");
	}

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = win_data[0].w;
	ofn.lpstrFilter = "Font Files (*.fon)\0*.fon\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = fullFileName;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = NULL;

	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "fon";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* Force the font */
		if (term_force_font(td, fullFileName))
		{
			/* Oops */
			(void)term_force_font(td, "8X13.FON");
		}
	}
}


#ifdef USE_GRAPHICS

#if 0 /* Manual change disabled */
/*
 * Allow the user to change the graf (and font) for a window
 *
 * XXX XXX XXX This is only called for graphic windows, and
 * changes the font and the bitmap for the window, and is
 * only called if "use_graphics" is true.
 */
static void term_change_bitmap(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[128] = "";

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = win_data[0].w;
	ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_XTRA_GRAF;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "bmp";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* XXX XXX XXX */

		/* Force the requested font and bitmap */
		if (
		    term_force_graf(td, tmp))
		{
			graphics_mode *gm;
			/* Force the "standard" font */
			//(void)term_force_font(td, "8X13.FON");

			/* Force the "standard" bitmap */
			gm = get_graphics_mode((byte)use_graphics);
			(void)term_force_graf(td, gm ? gm->file : "");
		}
	}
}
#endif

#endif


/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Restore the term */
	Term_activate(term_screen);
}
/*
 * Hack -- resize term_data
 */
static void term_data_resize(term_data *td, bool redraw)
{
	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the contents */
	if (redraw)	Term_redraw();

	/* Resize! */
	Term_resize(td->cols, td->rows);

	/* Restore the term */
	Term_activate(term_screen);
}





/*** Function hooks needed by "Term" ***/

/*
 * Interact with the User
 */
static errr Term_user_win(int n)
{
	/* Success */
	return (0);
}


/*
 * React to global changes
 */
static errr Term_xtra_win_react(void)
{
	int i;

/*	I added this USE_GRAPHICS because we lost color support as well. -GP */
#ifdef USE_GRAPHICS
	static old_use_graphics = FALSE;


	/* XXX XXX XXX Check "color_table[]" */


	/* Simple color */
	if (colors16)
	{
		/* Save the default colors */
		for (i = 0; i < 16; i++)
		{
			/* Simply accept the desired colors */
			win_pal[i] = color_table[i][0];
		}
	}

	/* Complex color */
	else
	{
		COLORREF code;

		byte rv, gv, bv;

		bool change = FALSE;

		/* Save the default colors */
		for (i = 0; i < 16; i++)
		{
			/* Extract desired values */
			rv = color_table[i][1];
			gv = color_table[i][2];
			bv = color_table[i][3];

			/* Extract a full color code */
			code = PALETTERGB(rv, gv, bv);

			/* Activate changes */
			if (win_clr[i] != code)
			{
				/* Note the change */
				change = TRUE;

				/* Apply the desired color */
				win_clr[i] = code;
			}
		}

		/* Activate the palette if needed */
		if (change) (void)new_palette();
	}
#endif	/* no color support -gp */

#ifdef USE_SOUND
    /* Initialize sound (if needed) */
    if (use_sound && !init_sound())
    {
        /* Warning */
        plog("Cannot initialize sound!");

        /* Cannot enable */
        use_sound = FALSE;
    }
#endif /* USE_SOUND */

#ifdef USE_GRAPHICS

	/* XXX XXX XXX Check "use_graphics" */
	if (use_graphics && !old_use_graphics)
	{
		/* 
		 * Code for reacting to graphical changes.
		 */
	}

	/* Remember */
	old_use_graphics = use_graphics;

#endif


	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *old = Term;

		term_data *td = &win_data[i];

		/* Skip non-changes XXX XXX XXX */
		if ((td->cols == td->t.wid) && (td->rows == td->t.hgt)) continue;

		/* Activate */
		Term_activate(&td->t);


		InvalidateRect(td->w, NULL, TRUE);
		/* Hack -- Resize the term */
		//Term_resize(td->cols, td->rows);

		/* Redraw the contents */
		Term_redraw();

		/* Restore */
		Term_activate(old);
	}


	/* Success */
	return (0);
}


/*
 * Process at least one event
 */
static errr Term_xtra_win_event(int v)
{
	MSG msg;

	/* Wait for an event */
	if (v)
	{
		/* Block */
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Check for an event */
	else
	{
		/* Check */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Success */
	return 0;
}


/*
 * Process all pending events
 */
static errr Term_xtra_win_flush(void)
{
	MSG msg;

	/* Process all pending events */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Success */
	return (0);
}


/*
 * Hack -- clear the screen
 *
 * XXX XXX XXX Make this more efficient
 */
static errr Term_xtra_win_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	HDC  hdc;
	RECT rc;

	/* Rectangle to erase */
	rc.left   = td->size_ow1;
	rc.right  = rc.left + td->cols * td->font_wid;
	rc.top    = td->size_oh1;
	rc.bottom = rc.top + td->rows * td->font_hgt;

	/* Erase it */
	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Hack -- make a noise
 */
static errr Term_xtra_win_noise(void)
{
	MessageBeep(MB_ICONASTERISK);
	return (0);
}


/*
 * Hack -- make a sound
 */
static errr Term_xtra_win_sound(int v)
{
#ifdef USE_SOUND
	char buf[MSG_LEN];
	int s = sound_count(v);

	/* Illegal sound */
	if (!s) return (-1);

	/* Random sample */
	s = randint0(s);

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_SOUND, sound_file[v][s]);

	/* Play the sound */
	return PlaySound(buf, NULL, SND_FILENAME | SND_ASYNC);
#else
	/* Oops */
	return (1);
#endif /* USE_SOUND */
}


/*
 * Delay for "x" milliseconds
 */
int Term_xtra_win_delay(int v)
{

#ifdef WIN32

	/* Sleep */
	Sleep(v);

#else /* WIN32 */

	DWORD t;
	MSG   msg;

	/* Final count */
	t = GetTickCount() + v;

	/* Wait for it */
	while (GetTickCount() < t)
	{
		/* Handle messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#endif /* WIN32 */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a bell sound */
		case TERM_XTRA_NOISE:
		return (Term_xtra_win_noise());

		/* Make a special sound */
		case TERM_XTRA_SOUND:
		return (Term_xtra_win_sound(v));

		/* Process random events */
		case TERM_XTRA_BORED:
		return (Term_xtra_win_event(0));

		/* Process an event */
		case TERM_XTRA_EVENT:
		return (Term_xtra_win_event(v));

		/* Flush all events */
		case TERM_XTRA_FLUSH:
		return (Term_xtra_win_flush());

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		return (Term_xtra_win_clear());

		/* React to global changes */
		case TERM_XTRA_REACT:
		return (v == 0 ? Term_xtra_win_react() : 0);

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
		return (Term_xtra_win_delay(v));
	}

	/* Oops */
	return 1;
}



/*
 * Low level graphics (Assumes valid input).
 *
 * Erase a "block" of "n" characters starting at (x,y).
 */
static errr Term_wipe_win(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	HDC  hdc;
	RECT rc;

#ifdef USE_GRAPHICS
	/* [grk] Client-side scrolling support
	 * This is a kludge see declaration of x_offset */
//	if(td == &win_data[0]){
//		x = x - x_offset;
//		if(x+n<13) return 0;
//		if(x<13){
//			n = n + (x-13);
//			x = 13;
//		}
//	}
#endif
	/* Rectangle to erase in client coords */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + n * td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_win(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	RECT   rc;
	HDC    hdc;

	/* Frame the grid */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(win_data[0].w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(win_data[0].w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw a "special" attr/char at the given location.
 *
 * XXX XXX XXX We use the "Term_pict_win()" function for "graphic" data,
 * which are encoded by setting the "high-bits" of both the "attr" and
 * the "char" data.  We use the "attr" to represent the "row" of the main
 * bitmap, and the "char" to represent the "col" of the main bitmap.  The
 * use of this function is induced by the "higher_pict" flag.
 *
 * If we are called for anything but the "screen" window, or if the global
 * "use_graphics" flag is off, we simply "wipe" the given grid.
 */
// static errr Term_pict_win(int x, int y, byte a, char c)
static errr Term_pict_win(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp) 
{
	term_data *td = (term_data*)(Term->data);

#ifdef USE_GRAPHICS

	HDC  hdc;
	HDC hdcSrc;
	HDC hdcMask;
	HBITMAP hbmSrcOld;
	int row, col;
	int i;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2;
	int x3, y3;
	graphics_mode *gm;

	/* Paranoia -- handle weird requests */
	if (!use_graphics)
	{
		/* First, erase the grid */
		return (Term_wipe_win(x, y, 1));
	}

	gm = get_graphics_mode((byte)use_graphics);

	Term_wipe_win(x, y, n);

#ifdef USE_GRAPHICS
	/* [grk] Client-side scrolling support
	 * This is a kludge see declaration of x_offset */
//	if(td == &win_data[0]){
//		x = x - x_offset;
//		if(x<13) return 0;
//	}
#endif

	/* Extract picture info */
//	row = ((int)ap & 0x7F);
//	col = ((int)cp & 0x7F);
	
	/* Size of bitmap cell */
	w1 = infGraph.CellWidth;
	h1 = infGraph.CellHeight;

	/* Location of bitmap cell */
//	x1 = col * w1;
//	y1 = row * h1;

	/* Size of window cell */
	w2 = td->font_wid;
	h2 = td->font_hgt;

	/* Location of window cell */
	x2 = x * w2 + td->size_ow1;
	y2 = y * h2 + td->size_oh1;

	/* Info */
	hdc = GetDC(td->w);

	/* More info */
	hdcSrc = CreateCompatibleDC(hdc);
	hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

	if (gm->transparent)
	{
		hdcMask = CreateCompatibleDC(hdc);
		SelectObject(hdcMask, infMask.hBitmap);
	}
	else
	{
		hdcMask = NULL;
	}
	
	/* Draw attr/char pairs */
	for (i = 0; i < n; i++, x2 += w2)
	{
		byte a = ap[i];
		char c = cp[i];

		/* Extract picture */
		int row = (a & 0x7F);
		int col = (c & 0x7F);

		/* Location of bitmap cell */
		x1 = col * w1;
		y1 = row * h1;

		if (gm->transparent)
		{
			x3 = (tcp[i] & 0x7F) * w1;
			y3 = (tap[i] & 0x7F) * h1;

			/* Perfect size */
			if ((w1 == w2) && (h1 == h2))
			{
				/* Copy the terrain picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, w2, h2, hdcSrc, x3, y3, SRCCOPY);

				/* Only draw if terrain and overlay are different */
				if ((x1 != x3) || (y1 != y3))
				{
					/* Mask out the tile */
					BitBlt(hdc, x2, y2, w2, h2, hdcMask, x1, y1, SRCAND);

					/* Draw the tile */
					BitBlt(hdc, x2, y2, w2, h2, hdcSrc, x1, y1, SRCPAINT);
				}
			}

			/* Need to stretch */
			else
			{
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the terrain picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, w2, h2, hdcSrc, x3, y3, w1, h1, SRCCOPY);

				/* Only draw if terrain and overlay are different */
				if ((x1 != x3) || (y1 != y3))
				{
					/* Mask out the tile */
					StretchBlt(hdc, x2, y2, w2, h2, hdcMask, x1, y1, w1, h1, SRCAND);

					/* Draw the tile */
					StretchBlt(hdc, x2, y2, w2, h2, hdcSrc, x1, y1, w1, h1, SRCPAINT);
				}
			}
		}
		else
		{
			/* Perfect size */
			if ((w1 == w2) && (h1 == h2))
			{
				/* Copy the picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, w2, h2, hdcSrc, x1, y1, SRCCOPY);
			}

			/* Need to stretch */
			else
			{
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, w2, h2, hdcSrc, x1, y1, w1, h1, SRCCOPY);
			}
		}
	}

	/* Release */
	SelectObject(hdcSrc, hbmSrcOld);
	DeleteDC(hdcSrc);

	if (gm->transparent)
	{
		/* Release */
		SelectObject(hdcMask, hbmSrcOld);
		DeleteDC(hdcMask);
	}

	/* Release */
	ReleaseDC(td->w, hdc);

#else

	/* Just erase this grid */
	return (Term_wipe_win(x, y, 1));

#endif

	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", above.
 *
 * XXX XXX XXX Note that this function assumes the font is monospaced.
 *
 * XXX XXX XXX One would think there is a more efficient method for
 * telling a window what color it should be using to draw with, but
 * perhaps simply changing it every time is not too inefficient.
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
	term_data *td = (term_data*)(Term->data);
	RECT rc;
	HDC  hdc;

#ifdef USE_GRAPHICS
	/* [grk] Client-side scrolling support
	 * This is a kludge see declaration of x_offset */
//	if(td == &win_data[0] && (y!=0) ){
//		if(x>13){
//			x = x - x_offset;
//			if(x<13) return 0;
//		}
//	}
#endif

	/* Location */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + n * td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	/* Acquire DC */
	hdc = GetDC(td->w);

	/* Background color */
	SetBkColor(hdc, RGB(0, 0, 0));

	/* Foreground color */
	if (colors16)
	{
		SetTextColor(hdc, PALETTEINDEX(win_pal[a&0x0F]));
	}
	else
	{
		SetTextColor(hdc, win_clr[a&0x0F]);
	}

	/* Use the font */
	SelectObject(hdc, td->font_id);

	/* Dump the text */
	ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
	           s, n, NULL);

	/* Release DC */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*** Other routines ***/


/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	t->soft_cursor = TRUE;

	/* Use "Term_pict" for "graphic" data */
	t->higher_pict = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Prepare the template hooks */
	t->user_hook = Term_user_win;
	t->xtra_hook = Term_xtra_win;
	t->wipe_hook = Term_wipe_win;
	t->curs_hook = Term_curs_win;
	t->pict_hook = Term_pict_win;
	t->text_hook = Term_text_win;

	/* Remember where we came from */
	t->data = (vptr)(td);
}


/*
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * XXX XXX XXX Need to work on the default window positions
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette. (?)
 */
static void init_windows(void)
{
	int i;
	static char version[20];
	term_data *td;
	HFONT editfont;

	/* Main window */
	td = &win_data[0];
	WIPE(td, term_data);

    sprintf(version, "Mangband %d.%d.%d",
			CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_PATCH);
	td->s = version;
	td->keys = 1024;
	td->rows = 24;
	td->cols = 80;
	td->visible = TRUE;
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh1 = 2;
	td->size_oh2 = 2;
	td->pos_x = 0;
	td->pos_y = 0;

	/* Sub-windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Mirror window */
		td = &win_data[i];
		WIPE(td, term_data);
		td->s = ang_term_name[i];
		td->keys = 16;
		td->rows = 24;
		td->cols = 80;
		td->visible = TRUE;
		td->size_ow1 = 1;
		td->size_ow2 = 1;
		td->size_oh1 = 1;
		/* Hack - give term4 space for edit control [grk] */
		if(i!=4) td->size_oh2 = 1;
		else td->size_oh2 = 30;
		td->pos_x = 0;
		td->pos_y = 0;
	}


	/* Load .INI preferences */
	load_prefs();


	/* Need these before term_getsize gets called */
	win_data[0].dwStyle = (WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | 
	                   WS_MINIMIZEBOX | WS_CAPTION | WS_VISIBLE);
	win_data[0].dwExStyle = 0;

	/* Windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		win_data[i].dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU);
		win_data[i].dwExStyle = (WS_EX_TOOLWINDOW);
	}


	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (term_force_font(&win_data[i], win_data[i].font_want))
		{
			(void)term_force_font(&win_data[i], "8X13.FON");
		}
	}


	/* Screen window */
	td_ptr = &win_data[0];
	td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AppName,
	                           td_ptr->s, td_ptr->dwStyle,
	                           td_ptr->pos_x, td_ptr->pos_y,
	                           td_ptr->size_wid, td_ptr->size_hgt,
	                           HWND_DESKTOP, NULL, hInstance, NULL);
	if (!td_ptr->w) quit("Failed to create Angband window");
	td_ptr = NULL;
	term_data_link(&win_data[0]);
	ang_term[0] = &win_data[0].t;

	/* Windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td_ptr = &win_data[i];
		td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AngList,
		                           td_ptr->s, td_ptr->dwStyle,
		                           td_ptr->pos_x, td_ptr->pos_y,
		                           td_ptr->size_wid, td_ptr->size_hgt,
		                           HWND_DESKTOP, NULL, hInstance, NULL);
		if (!td_ptr->w) quit("Failed to create sub-window");
		if (td_ptr->visible)
		{
			td_ptr->size_hack = TRUE;
			ShowWindow(td_ptr->w, SW_SHOW);
			td_ptr->size_hack = FALSE;
		}
		td_ptr = NULL;
		term_data_link(&win_data[i]);
		ang_term[i] = &win_data[i].t;
	}

	/* hack [grk] */
	editmsg = CreateWindowEx(WS_EX_STATICEDGE,"EDIT",NULL,WS_CHILD|ES_AUTOHSCROLL|ES_OEMCONVERT|WS_VISIBLE,
						 2,win_data[4].client_hgt-24,win_data[4].client_wid-8,20,win_data[4].w,NULL,hInstance,NULL);
	editfont=CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH,"Arial");  
	SendMessage(editmsg, WM_SETFONT, (int)editfont, (int)NULL );
	stretch_chat_ctrl();

	SendMessage(editmsg, EM_LIMITTEXT, 590, 0L);
	lpfnOldWndProc = (WNDPROC)SetWindowLongPtr(editmsg, GWL_WNDPROC, (DWORD) SubClassFunc);

	/* Activate the screen window */
	SetActiveWindow(win_data[0].w);

	/* Bring main screen back to top */
	SetWindowPos(win_data[0].w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


#ifdef USE_GRAPHICS

	/* Handle "graphics" mode */
	if (use_graphics)
	{
		/* XXX XXX XXX Force the "standard" bitmap */
		graphics_mode *gm = get_graphics_mode((byte)use_graphics);

		(void)term_force_graf(&win_data[0], gm);
	}

#endif

#ifdef USE_SOUND
	/* Handle sound module */
	Term_xtra_win_react();
#endif

	/* New palette XXX XXX XXX */
	(void)new_palette();


	/* Create a "brush" for drawing the "cursor" */
	hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);

	/* Create a "brush" for drawing the chat window edit control background */
	hbrBlack = CreateSolidBrush(0);

	/* Process pending messages */
	(void)Term_xtra_win_flush();
}


static char* has_addr_aux(cptr pmsgbuf, int mode, char* addrbuf, size_t addrmax, size_t* addrlen)
{
	int offset,breakpoint,nicklen;
	char * startmsg;
	char search_for = ':';
	offset = 0;
	if (mode == 1)
	{
		search_for = ' ';
		if (pmsgbuf[0] != '#')
		{
			*addrlen = 0;
			return pmsgbuf;
		}
	}
	for (startmsg = pmsgbuf; *startmsg ; startmsg++ ) {
		if( *startmsg==search_for ) break;
	};
	if( *startmsg  && (startmsg-pmsgbuf<addrmax) ) {
		strncpy(addrbuf,pmsgbuf,(startmsg-pmsgbuf)+1);
		nicklen=strlen(addrbuf);
		startmsg+=1;
		while (*startmsg == ' ') startmsg+=1;
	} else {
		startmsg=pmsgbuf;
		nicklen=0;
	};
	*addrlen = nicklen;
	return startmsg;
}
static char* has_addr(cptr msg, char* addrbuf, size_t addrmax, size_t* addrlen)
{
	char *r;
	/* Check channel */
	r = has_addr_aux(msg, 1, addrbuf, addrmax, addrlen);
	if (*addrlen > 0) return r;

	/* Check privmsg */
	r = has_addr_aux(msg, 0, addrbuf, addrmax, addrlen);
	if (*addrlen > 0) return r;

	/* OK, got nothing */
	*addrlen = 0;
	return msg;
}

/* hack - edit control subclass [grk] */
LRESULT APIENTRY SubClassFunc(   HWND hWnd,
               UINT Message,
               WPARAM wParam,
               LPARAM lParam)
{
	char pmsgbuf[1000]; /* overkill */
	char pmsg[MSG_LEN + 2];
	char nickbuf[30];

	/* Allow ESCAPE to return focus to main window. */
	if( Message == WM_KEYDOWN ) {
		if( wParam == VK_ESCAPE ){
		unset_chat_focus();
		return 0;
		}
		if (wParam == 33) { /* PGUP */
			cmd_chat_cycle(-1);
			return 0;
		}
		if (wParam == 34) { /* PGDN */
			cmd_chat_cycle(+1);
			return 0;
		}
	}

	if ( Message == WM_CHAR ) {
		/* Is this RETURN ? */
		if( wParam == 13 || wParam == 10000) {
			int CUTOFF=0;
			int msglen=0;
			memset(nickbuf,0,22);

			/* Get the controls text and send it */
			msglen = GetWindowText(editmsg, pmsgbuf, 999);

			/* Send the text in chunks of CUTOFF characters,
			   or nearest break before CUTOFF chars */

			if( msglen == 0 ){
			    unset_chat_focus();
			    return 0;
			}
			/* Max message length depends on our nick (known)
			 * and recepient nick (unknown, assume max length) */
			CUTOFF = MSG_LEN - strlen(nick) - 3; /* 3 is for decorative symbols */
			CUTOFF -= (MAX_NAME_LEN+1); /* +1 for ':' symbol in private messages */
/*RLS*/
			if( msglen < CUTOFF ){
				send_msg(pmsgbuf);
			} else{
				int offset,breakpoint,nicklen;
				char * startmsg;
				offset = 0;

				/* see if this was a privmsg, if so, pull off the nick */

				startmsg = has_addr(pmsgbuf, nickbuf, 29, &nicklen);

				/* now deal with what's left */

				while(msglen>0){
					memset(pmsg,0,MSG_LEN);

					if(msglen<CUTOFF){
						breakpoint=msglen;
					} else{
						/* try to find a breaking char */
						for(breakpoint=CUTOFF; breakpoint>0; breakpoint--) {
							if( startmsg[offset+breakpoint] == ' ' ) break;
							if( startmsg[offset+breakpoint] == ',' ) break;
							if( startmsg[offset+breakpoint] == '.' ) break;
							if( startmsg[offset+breakpoint] == ';' ) break;
						};
						if(!breakpoint) breakpoint=CUTOFF; /* nope */
					}

					/* if we pulled off a nick above, prepend it. */
					if(nicklen) strncpy(pmsg,nickbuf,nicklen);

					/* stash in this part of the msg */
					strncat(pmsg, startmsg+offset, breakpoint);
					msglen -= breakpoint;
					offset += breakpoint;
					send_msg(pmsg);
					//Net_flush();
				}
			}

			/* Clear the message box */
			pmsgbuf[0] = 0;
			SetWindowText(editmsg, pmsgbuf);
			unset_chat_focus();
			return 0;
		}
	}

	return CallWindowProc(lpfnOldWndProc, hWnd, Message, wParam, lParam);
}


/*
 * Hack -- disables new and open from file menu
 */
static void disable_start(void)
{
	HMENU hm = GetMenu(win_data[0].w);

	EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}


/*
 * Prepare the menus
 *
 * XXX XXX XXX See the updated "main-mac.c" for a much nicer
 * system, basically, you explicitly disable any menu option
 * which the user should not be allowed to use, and then you
 * do not have to do any checking when processing the menu.
 */
static void setup_menus(void)
{
	int i;

	HMENU hm = GetMenu(win_data[0].w);
#ifdef MNU_SUPPORT
	/* Save player */
	EnableMenuItem(hm, IDM_FILE_SAVE,
	               MF_BYCOMMAND | MF_ENABLED | MF_GRAYED);


	/* Exit with save */
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | MF_ENABLED | MF_GRAYED);


	/* Window font options */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Window font */
		EnableMenuItem(hm, IDM_TEXT_SCREEN + i,
		               MF_BYCOMMAND | (win_data[i].visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
	}

	/* Window options */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Window */
		CheckMenuItem(hm, IDM_WINDOWS_SCREEN + i,
		              MF_BYCOMMAND | (win_data[i].visible ? MF_CHECKED : MF_UNCHECKED));
	}
#endif
#ifdef USE_GRAPHICS
	/* Item "Graphics - Off" */
	CheckMenuItem(hm, IDM_GRAPHICS_OFF,
	              MF_BYCOMMAND | (!next_graphics ? MF_CHECKED : MF_UNCHECKED));

	/* Item "Graphics - Tileset N" */
	for (i = 0; i < 8; i++)
	{
		/* Replace "Tileset N" string with actual tileset name */
		if ((1 + i) <= get_num_graphics_modes())
		{
			graphics_mode *gm = get_graphics_mode((byte)(1 + i));
			MENUITEMINFO menuitem = { sizeof(MENUITEMINFO) };
			GetMenuItemInfo(hm, IDM_GRAPHICS_TILESET_1 + i, FALSE, &menuitem);
			menuitem.fMask = MIIM_TYPE | MIIM_DATA;
			menuitem.dwTypeData = gm->menuname;
			SetMenuItemInfo(hm, IDM_GRAPHICS_TILESET_1 + i, FALSE, &menuitem);
		}

		/* Item "Graphics - Tileset N" */
		CheckMenuItem(hm, IDM_GRAPHICS_TILESET_1 + i,
		              MF_BYCOMMAND | (next_graphics == 1 + i ? MF_CHECKED : MF_UNCHECKED));

		/* XXX Delete unused menu entries */
		if ((1 + i) > get_num_graphics_modes())
		RemoveMenu(hm, IDM_GRAPHICS_TILESET_1 + i, MF_BYCOMMAND);
	}

	/* Item "Graphics - Respect Tile Size?" */
	CheckMenuItem(hm, IDM_GRAPHICS_BIG_TILE,
	              MF_BYCOMMAND | (big_tile ? MF_CHECKED : MF_UNCHECKED));
#endif
#ifdef USE_SOUND
	/* Item "Sound" */
	CheckMenuItem(hm, IDM_OPTIONS_SOUND,
	              MF_BYCOMMAND | (use_sound ? MF_CHECKED : MF_UNCHECKED));
#endif

	/* Item "Mouse" */
	CheckMenuItem(hm, IDM_OPTIONS_MOUSE,
	              MF_BYCOMMAND | (use_mouse ? MF_CHECKED : MF_UNCHECKED));

#ifdef BEN_HACK
	/* Item "Colors 16" */
	EnableMenuItem(hm, IDM_OPTIONS_UNUSED,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif

#ifndef ALLOW_SCRSAVER
	/* Item "Run as Screensaver" */
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif

}


/*
 * Check for double clicked (or dragged) savefile
 *
 * Apparently, Windows copies the entire filename into the first
 * piece of the "command line string".  Perhaps we should extract
 * the "basename" of that filename and append it to the "save" dir.
 */
static void check_for_save_file(LPSTR cmd_line)
{
	char *s, *p;

	/* First arg */
	s = cmd_line;

	/* Second arg */
	p = strchr(s, ' ');

	/* Tokenize, advance */
	if (p) *p++ = '\0';

	/* No args */
	if (!*s) return;

/* More stuff moved to server -GP
	// Extract filename
	strcat(savefile, s);

	// Validate the file
	validate_file(savefile);
*/
	/* Game in progress */
	game_in_progress = TRUE;

	/* No restarting */
	disable_start();

	/* Play game */
/*	play_game(FALSE);	-changed to network call -GP */
}

BOOL CALLBACK dDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch( uMsg ){
		case WM_COMMAND:
			EndDialog(hwndDlg,0);
	}
	return 0;
}

/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
#ifdef	MNU_SUPPORT
	int i;

	/* Analyze */
	switch (wCmd)
	{
		/* Save and Exit */
		case IDM_FILE_EXIT:
		{
			quit(NULL);
			break;
		}


		case IDM_TEXT_SCREEN:
		{

#ifdef USE_GRAPHICS
			/* XXX XXX XXX */
			//if (use_graphics)
			//{
			//	term_change_bitmap(&win_data[0]);
			//	break;
			//}
#endif

			term_change_font(&win_data[0]);
			break;
		}

		/* Window fonts */
		case IDM_TEXT_MIRROR:
		case IDM_TEXT_RECALL:
		case IDM_TEXT_CHOICE:
		case IDM_TEXT_TERM_4:
		case IDM_TEXT_TERM_5:
		case IDM_TEXT_TERM_6:
		case IDM_TEXT_TERM_7:
		{
			i = wCmd - IDM_TEXT_SCREEN;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			term_change_font(&win_data[i]);

			break;
		}

		/* Window visibility */
		case IDM_WINDOWS_MIRROR:
		case IDM_WINDOWS_RECALL:
		case IDM_WINDOWS_CHOICE:
		case IDM_WINDOWS_TERM_4:
		case IDM_WINDOWS_TERM_5:
		case IDM_WINDOWS_TERM_6:
		case IDM_WINDOWS_TERM_7:
		{
			i = wCmd - IDM_WINDOWS_SCREEN;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			if (!win_data[i].visible)
			{
				win_data[i].visible = TRUE;
				ShowWindow(win_data[i].w, SW_SHOW);
				term_data_redraw(&win_data[i]);
			}
			else
			{
				win_data[i].visible = FALSE;
				ShowWindow(win_data[i].w, SW_HIDE);
			}

			break;
		}

		/* Hack -- unused */
		case IDM_OPTIONS_UNUSED:
		{
			/* XXX XXX XXX */
			break;
		}
/*	Currently no graphics options available. -GP */
#ifdef USE_GRAPHICS
		case IDM_GRAPHICS_OFF:
		{
			set_graphics_next(0);
			break;
		}
		case IDM_GRAPHICS_TILESET_1:
		case IDM_GRAPHICS_TILESET_2:
		case IDM_GRAPHICS_TILESET_3:
		case IDM_GRAPHICS_TILESET_4:
		case IDM_GRAPHICS_TILESET_5:
		case IDM_GRAPHICS_TILESET_6:
		case IDM_GRAPHICS_TILESET_7:
		case IDM_GRAPHICS_TILESET_8:
		{
			set_graphics_next(wCmd - IDM_GRAPHICS_TILESET_1 + 1);
			break;
			/* no support -GP */
			/* React to changes */
			//Term_xtra_win_react();

			/* Hack -- Force redraw */
			//Term_key_push(KTRL('R'));
		}
		case IDM_GRAPHICS_BIG_TILE:
		{
			static int old_font_wid, old_font_hgt;
			/*
			if (!big_tile) 
			{
				old_font_wid = win_data[0].font_wid;
				old_font_hgt = win_data[0].font_hgt;
				win_data[0].font_wid = infGraph.CellHeight;
				win_data[0].font_hgt = infGraph.CellHeight;
			} else {
				win_data[0].font_wid = old_font_wid;
				win_data[0].font_hgt = old_font_hgt;
			}
			*/
			x_offset = 20;
			big_tile = !big_tile;			

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));
			break;
		}
#endif
		case IDM_OPTIONS_SOUND:
		{
			use_sound = !use_sound;
			Term_xtra_win_react();
			break;
		}

		case IDM_OPTIONS_MOUSE:
		{
			use_mouse = !use_mouse;
			break;
		}

		case IDM_HELP_GENERAL:
		{
			char buf[1024];
			char tmp[1024];
			path_build(tmp, 1024, ANGBAND_DIR_XTRA_HELP, "angband.hlp");
			sprintf(buf, "winhelp.exe %s", tmp);
			WinExec(buf, SW_NORMAL);
			break;
		}

		case IDM_HELP_SPOILERS:
		{
			char buf[1024];
			char tmp[1024];
			path_build(tmp, 1024, ANGBAND_DIR_XTRA_HELP, "spoilers.hlp");
			sprintf(buf, "winhelp.exe %s", tmp);
			WinExec(buf, SW_NORMAL);
			break;
		}
		case IDM_HELP_ABOUT:
		{
			/*x_offset += 1;
			Term_redraw();*/

			DialogBox(hInstance, "ABOUT" ,win_data[0].w, dDialogProc );

			break;
		}

	}
#endif	/* MNU_SUPPORT */
}

#ifdef BEN_HACK
LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam);
#endif

LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	HDC             hdc;
	term_data      *td;
	MINMAXINFO FAR *lpmmi;
	RECT            rc;
	int             i;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	/* Handle message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(td_ptr));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			lpmmi = (MINMAXINFO FAR *)lParam;

			if (!td) return 1;  /* this message was sent before WM_NCCREATE */

			/* Minimum window size is 8x2 */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + (MAX_WID+SCREEN_CLIP_X) * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + (MAX_HGT+SCREEN_CLIP_Y) * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;

			/* Paranoia */
			rc.right  += (td->font_wid - 1);
			rc.bottom += (td->font_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x      = rc.right - rc.left;
			lpmmi->ptMaxSize.y      = rc.bottom - rc.top;

			/* Save maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			ValidateRect(hWnd, NULL);   /* why needed ?? */
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			BYTE KeyState = 0x00;
			bool enhanced = FALSE;
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Check for non-normal keys */
			if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN)) enhanced = TRUE;
			if ((wParam >= VK_F1) && (wParam <= VK_F12)) enhanced = TRUE;
			if ((wParam == VK_INSERT) || (wParam == VK_DELETE)) enhanced = TRUE;

			/* XXX XXX XXX */
			if (enhanced)
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" from bits 16..23 of lParam */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
#if defined(WM_XBUTTONDOWN)
		case WM_XBUTTONDOWN:
#endif
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int button = 1;

			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (!Term) return 1;  /* no active terminal yet */

			x = x / td->font_wid;
			y = y / td->font_hgt;

			/* Which button */
			if (uMsg == WM_LBUTTONDOWN) button = 1;
			if (uMsg == WM_MBUTTONDOWN) button = 2;
			if (uMsg == WM_RBUTTONDOWN) button = 3;
#if defined(WM_XBUTTONDOWN)
			if (uMsg == WM_XBUTTONDOWN) button = 3 + HIWORD(wParam);
#endif

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) button |= 16;
			if (GetKeyState(VK_SHIFT)   & 0x8000) button |= 32;
			if (GetKeyState(VK_MENU)    & 0x8000) button |= 64;

			if (!use_mouse) return 0; /* No in-game mouse */

			Term_mousepress(x, y, button);
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (!Term) return 1;  /* no active terminal yet */

			x = x / td->font_wid;
			y = y / td->font_hgt;

			Term_mousepress(x, y, 0);
			return 0;
		}

		case WM_INITMENU:
		{
			setup_menus();
			return 0;
		}

		case WM_CLOSE:
		case WM_QUIT:
		{
			quit(NULL);
			return 0;
		}

		case WM_COMMAND:
		{
			process_menus(LOWORD(wParam));
			return 0;
		}
/*
		case WM_ENTERSIZEMOVE:
		{
			return 0;
		}
*/		
		case WM_EXITSIZEMOVE:
		{
			term *old = Term;

			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (!conn_state) return 1;

			if (td->t.wid != td->cols || td->t.hgt != td->rows)
			{
				Term_resize(td->cols, td->rows);
				net_term_manage(window_flag, window_flag, FALSE);
			}
			
			return 0;
		}

		case WM_SIZE:
		{
			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (td->size_hack) return 1; /* was sent from WM_SIZE */

			switch (wParam)
			{
				case SIZE_MINIMIZED:
				{
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (win_data[i].visible) ShowWindow(win_data[i].w, SW_HIDE);
					}
					return 0;
				}

				case SIZE_MAXIMIZED:
				{
					/* fall through XXX XXX XXX */
				}

				case SIZE_RESTORED:
				{
					term *old = Term;

					td->size_hack = TRUE;

					td->cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
					td->rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;

					term_getsize(td);

					MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);

					td->size_hack = FALSE;

					/* Windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (win_data[i].visible) ShowWindow(win_data[i].w, SW_SHOWNOACTIVATE);
					}

					/* Changed size (before loggin in) */
					Term_resize(td->cols, td->rows);
					if (!conn_state)
					{
						net_term_manage(window_flag, window_flag, FALSE);
					}

					return 0;
				}
			}
			break;
		}

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return FALSE;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return FALSE;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, FALSE);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			return FALSE;
		}

		case WM_ACTIVATE:
		{
			if (wParam && !HIWORD(lParam))
			{
				for (i = 1; i < MAX_TERM_DATA; i++)
				{
					SetWindowPos(win_data[i].w, hWnd, 0, 0, 0, 0,
					             SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
				}
				SetFocus(hWnd);
				return 0;
			}
			break;
		}
	}


	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef BEN_HACK
LRESULT FAR PASCAL _export AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam);
#endif

LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
{
	term_data      *td;
	MINMAXINFO FAR *lpmmi;
	RECT            rc;
	PAINTSTRUCT     ps;
	HDC             hdc;
	int             i,j;
	char		pmsg[60];


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	/* Process message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(td_ptr));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case 0x133: /* WM_CTLCOLOREDIT */
		{
			SetTextColor((HDC)wParam,0x00ffffff);
			SetBkColor((HDC)wParam, 0);
			return (int)hbrBlack;
		}

                /* GRK, wasn't trapping this so vis menus got messed up */
                /* We fake a click on the visibilty menu when the close icon is clicked */

                case WM_CLOSE:
                        /* Which term is closing ? */
                        j=-1;
                        for(i=0; i<MAX_TERM_DATA;i++)
                          if(&win_data[i] == td) j = i;
                        /* Click its menu entry */
                        if(j != -1) process_menus(211+j);
                        return 0;

		case WM_GETMINMAXINFO:
		{
			if (!td) return 1;  /* this message was sent before WM_NCCREATE */

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* Minimum size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->font_hgt + td->size_oh1 + td->size_oh2;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save the minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->font_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right += (td->font_wid - 1);
			rc.bottom += (td->font_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x      = rc.right - rc.left;
			lpmmi->ptMaxSize.y      = rc.bottom - rc.top;

			/* Save the maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			/* Force redraw to remove artifacts */
			InvalidateRect(hWnd,NULL,TRUE);
			return 0;
		}
		case WM_SIZE:
		{
			term *old = Term;

			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (td->size_hack) return 1; /* was sent from inside WM_SIZE */

			td->size_hack = TRUE;

			td->cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
			td->rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;

			term_getsize(td);

			term_data_resize(td, FALSE);
			/* Gross hack -- find needed terminal number */
			for (i = 1; i < ANGBAND_TERM_MAX; i++) {
				if ((ang_term[i]) == &(td->t)) {
					p_ptr->window |= window_flag[i];
				}
			}
			window_stuff();

			MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);
			if(td == &win_data[4]) stretch_chat_ctrl();

			td->size_hack = FALSE;

			return 0;
		}

		case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			BYTE KeyState = 0x00;
			bool enhanced = FALSE;
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

            /* Which term getting the keypress ? */
            j=-1;
            for(i=0; i<MAX_TERM_DATA;i++)
              if(&win_data[i] == td) j = i;
              /* If this is term7 we are sending a player message */
              if(j == 7){
				/* Is this RETURN ? */
				if( wParam == 13){
					/* Get the controls text and send it */
					GetWindowText(editmsg, pmsg, 59);
					send_msg(pmsg);
				}
				/* If not return, ignore key */
				return 0;
			}
			
			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Check for non-normal keys */
			if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN)) enhanced = TRUE;
			if ((wParam >= VK_F1) && (wParam <= VK_F12)) enhanced = TRUE;
			if ((wParam == VK_INSERT) || (wParam == VK_DELETE)) enhanced = TRUE;

			/* XXX XXX XXX */
			if (enhanced)
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" from bits 16..23 of lParam */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return FALSE;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, FALSE);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			return 0;
		}

		case WM_NCLBUTTONDOWN:
		{
			if (wParam == HTSYSMENU)
			{
				/* Hide sub-windows */
				if (td != &win_data[0])
				{
					if (td->visible)
					{
						td->visible = FALSE;
						ShowWindow(td->w, SW_HIDE);
					}
				}
				return 0;
			}
			break;
		}

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef ALLOW_SCRSAVER

#define MOUSE_SENS 40

LRESULT FAR PASCAL _export AngbandSaverProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;

	term_data *td;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	/* Process */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(td_ptr));
			break;
		}

		case WM_SETCURSOR:
		{
			SetCursor(NULL);
			return 0;
		}

#if 0
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE) break;

			/* else fall through */
		}
#endif

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYDOWN:
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (iMouse)
			{
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
				{
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
			}

			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);
			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			return 0;
		}
	}

	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif





/*** Temporary Hooks ***/


/*
 * Error message -- See "z-util.c"
 */
static void hack_plog(cptr str)
{
	/* Give a warning */
/*	if (str) c_msg_print(str); */
	if (str) MessageBox(NULL, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 */
static void hack_quit(cptr str)
{
	int i;

	/* Force saving of preferences on any quit [grk] */
	save_prefs();
	
	/* Give a warning */
	if (str && str[0]) MessageBox(NULL, str, "Error", MB_OK | MB_ICONSTOP);

	/* Sub-Windows */
	for (i = MAX_TERM_DATA - 1; i >= 1; i--)
	{
		term_force_font(&win_data[i], NULL);
		if (win_data[i].font_want) string_free(win_data[i].font_want);
		if (win_data[i].graf_want) string_free(win_data[i].graf_want);
		if (win_data[i].w) DestroyWindow(win_data[i].w);
		win_data[i].w = 0;
	}

#ifdef USE_GRAPHICS
	/* Free the bitmap stuff */
	FreeDIB(&infGraph);
	FreeDIB(&infMask);
#endif
	/* Forget grafmodes */
	close_graphics_modes();


	term_force_font(&win_data[0], NULL);
	if (win_data[0].font_want) string_free(win_data[0].font_want);
	if (win_data[0].graf_want) string_free(win_data[0].graf_want);
	if (win_data[0].w) DestroyWindow(win_data[0].w);
	win_data[0].w = 0;

	/* Unregister the classes */
	UnregisterClass(AppName, hInstance);

	/* Destroy the icon */
	if (hIcon) DestroyIcon(hIcon);

	/* Cleanup WinSock */
	cleanup_network_client();

	/* Exit */
	exit (0);
}


/*
 * Fatal error (see "z-util.c")
 */
static void hack_core(cptr str)
{
	/* Give a warning */
	if (str) MessageBox(NULL, str, "Error", MB_OK | MB_ICONSTOP);

	/* Quit */
	quit (NULL);
}


/*** Various hooks ***/


/*
 * Error message -- See "z-util.c"
 */
static void hook_plog(cptr str)
{
	/* Warning */
	if (str) MessageBox(win_data[0].w, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 *
 * A lot of this function should be handled by actually calling
 * the "term_nuke()" function hook for each window.  XXX XXX XXX
 */
static void hook_quit(cptr str)
{
	int i;

#ifdef USE_SOUND
	int j;
#endif /* USE_SOUND */

	/* Give a warning */
	if (str) MessageBox(win_data[0].w, str, "Error", MB_OK | MB_ICONSTOP);


	/* Save the preferences */
	save_prefs();

	/* Sub-Windows */
	for (i = MAX_TERM_DATA - 1; i >= 1; i--)
	{
		term_force_font(&win_data[i], NULL);
		if (win_data[i].font_want) string_free(win_data[i].font_want);
		if (win_data[i].graf_want) string_free(win_data[i].graf_want);
		if (win_data[i].w) DestroyWindow(win_data[i].w);
		win_data[i].w = 0;
	}

#ifdef USE_GRAPHICS
	/* Free the bitmap stuff */
	FreeDIB(&infGraph);
	FreeDIB(&infMask);
#endif

#ifdef USE_SOUND
	/* Free the sound names */
	for (i = 0; i < MSG_MAX; i++)
	{
		for (j = 0; j < SAMPLE_MAX; j++)
		{
			if (!sound_file[i][j]) break;

			string_free(sound_file[i][j]);
		}
	}
#endif /* USE_SOUND */

	term_force_font(&win_data[0], NULL);
	if (win_data[0].font_want) string_free(win_data[0].font_want);
	if (win_data[0].graf_want) string_free(win_data[0].graf_want);
	if (win_data[0].w) DestroyWindow(win_data[0].w);
	win_data[0].w = 0;


	/*** Free some other stuff ***/

	DeleteObject(hbrYellow);

	if (hPal) DeleteObject(hPal);

	UnregisterClass(AppName, hInstance);

	if (hIcon) DestroyIcon(hIcon);

	//network_done();

	/* Free strings */

	exit(0);
}



/*** Initialize ***/


/*
 * Init some stuff
 */
void static init_stuff_win(void)
{
	int   i;

	char path[1024];

#ifdef USE_GRAPHICS
	graphics_mode *gm;
#endif

	/* XXX XXX XXX */
	strcpy(path, conf_get_string("Windows32", "LibPath", ".\\lib"));

	/* Analyze the path */
	i = strlen(path);

	/* Require a path */
	if (!i) quit("LibPath shouldn't be empty in ANGBAND.INI");

	/* Nuke terminal backslash */
	if (i && (path[i-1] != '\\'))
	{
		path[i++] = '\\';
		path[i] = '\0';
	}

	/* Validate the path */
	validate_dir(path);


	/* Init the file paths */

	init_file_paths(path);
 
	/* Hack -- Validate the paths */

/*	validate_dir(ANGBAND_DIR_APEX); *//*on server */
/*	validate_dir(ANGBAND_DIR_EDIT); */
/*	validate_dir(ANGBAND_DIR_FILE); */
/*	validate_dir(ANGBAND_DIR_HELP); */
	validate_dir(ANGBAND_DIR_BONE);
	validate_dir(ANGBAND_DIR_PREF);
	validate_dir(ANGBAND_DIR_USER);
	validate_dir(ANGBAND_DIR_XTRA);	  /*sounds & graphics */


	// Build the "font" path
	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");

	// Allocate the path
	ANGBAND_DIR_XTRA_FONT = string_make(path);

	// Validate the "font" directory
	validate_dir(ANGBAND_DIR_XTRA_FONT);

	// Build the filename
	path_build(path, 1024, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

	// Hack -- Validate the basic font
	validate_file(path);


#ifdef USE_GRAPHICS

	/* Validate the "graf" directory */
	validate_dir(ANGBAND_DIR_XTRA_GRAF);

	/* Pick graphics mode */
	if ((gm = get_graphics_mode((byte)use_graphics)))
	{
		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_XTRA_GRAF, gm->file);

		/* Hack -- Validate the basic graf */
		validate_file(path);
	}
#endif


#ifdef USE_SOUND

	/* Validate the "sound" directory */
	validate_dir(ANGBAND_DIR_XTRA_SOUND);

#endif

}



int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPSTR lpCmdLine, int nCmdShow)
{
#ifdef USE_GRAPHICS
	int i;
#endif

	WNDCLASS wc;
	HDC      hdc;
	MSG      msg;
	//WSADATA wsadata;

	hInstance = hInst;  /* save in a global var */

	if (hPrevInst == NULL)
	{
		wc.style         = CS_CLASSDC;
		wc.lpfnWndProc   = AngbandWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4; /* one long pointer to term_data */
		wc.hInstance     = hInst;
		wc.hIcon         = hIcon = LoadIcon(hInst, AppName);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = AppName;
		wc.lpszClassName = AppName;

		if (!RegisterClass(&wc)) exit(1);

		wc.lpfnWndProc   = AngbandListProc;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = AngList;

		if (!RegisterClass(&wc)) exit(2);

#ifdef ALLOW_SCRSAVER

		wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
		wc.lpfnWndProc    = AngbandSaverProc;
		wc.hCursor        = NULL;
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = "WindowsScreenSaverClass";

		if (!RegisterClass(&wc)) exit(3);

#endif

	}

	/* Set hooks */
	quit_aux = hack_quit;
	core_aux = hack_core;
	plog_aux = hack_plog;

	/* Global client config */
	conf_init(hInstance);
	
	/* Prepare the filepaths */
	init_stuff_win();

	/* Initialize WinSock */
//	WSAStartup(MAKEWORD(1, 1), &wsadata);

	/* Determine if display is 16/256/true color */
	hdc = GetDC(NULL);
	colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
	paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
	ReleaseDC(NULL, hdc);
#ifdef USE_GRAPHICS
	/* Initialize "color_table" */
	for (i = 0; i < 16; i++)
	{
		/* Save the "complex" codes */
		color_table[i][1] = GetRValue(win_clr[i]);
		color_table[i][2] = GetGValue(win_clr[i]);
		color_table[i][3] = GetBValue(win_clr[i]);

		/* Save the "simple" code */
		color_table[i][0] = win_pal[i];
	}
#endif

	/* load the possible graphics modes */
	if (!init_graphics_modes("graphics.txt")) {
		plog("Graphics list load failed");
	}

	/* Prepare the windows */
	init_windows();

	/* Set the system suffix */
	ANGBAND_SYS = "win";

	/* We are now initialized */
	initialized = TRUE;

	/* Do network initialization, etc. */
	client_init();

	/* Process messages forever */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	cleanup_network_client();

	/* Paranoia */
	quit(NULL);

	/* Paranoia */
	return (0);
}

#endif /* _Windows */
