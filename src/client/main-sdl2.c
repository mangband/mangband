/* File: main-sdl2.c */
/* Purpose: SDL2 frontend for mangband */
/* Original SDL2 client written by "kts of kettek (kettek1@kettek.net)". */
/* NOTES:
  2. Many files in 'lib/user' and 'lib/xtra' are essential. These files and their purpose(s) are:
    * 'lib/user/font-sdl2.prf'
      - used to create the initial color palette - if non-existent, all text will be black on black
    * 'lib/pref/pref-sdl2.prf'
      - keymap file for SDL2, required for arrow keys, also needs expansion
    * 'lib/xtra/font/AnonymousPro.ttf'
      - The default TTF font, licensed under the Open Font License
      - NOTE: This can be overridden in MAngband.ini as "font_file".
    * 'lib/xtra/font/ui-cmd.ttf'
       - "UI" font for Mobile UI. See 'ui-cmd.txt' for details.

TODO:
  * Actually use all possible settings in MAngband.ini
  * Figure out keymapping that doesn't require 'keymap-sdl2.prf'
  * Get rid of as much hard-coded values as possible!
*/
#ifdef USE_SDL2

#include "c-angband.h"

#include "main-sdl2.h"
#include "sdl-font.h"
#include "sdl-sound.h"

#ifdef WINDOWS
double fmin(double x, double y)
{
	return (x < y ? x : y);
}
#define WM_DESKTOP_OFFSET 64
#define WM_TITLEBAR_OFFSET 16
#else
#define WM_DESKTOP_OFFSET 0
#define WM_TITLEBAR_OFFSET 16
#endif
#ifdef MOBILE_UI
#undef WM_DESKTOP_OFFSET
#undef WM_TITLEBAR_OFFSET
#define WM_DESKTOP_OFFSET 0
#define WM_TITLEBAR_OFFSET 0
#endif

#ifdef MOBILE_UI
#define USE_LISENGLAS
#define USE_ICON_OVERLAY
#endif

/* Color definitions. Carefully copied from kettek's values. */
enum {
	GUI_COLOR_BACKGROUND,
	GUI_COLOR_BACKGROUND_ACTIVE,
	GUI_COLOR_MENU_BACKGROUND,
	GUI_COLOR_MENU_BORDER,
	GUI_COLOR_MENU_ITEM,
	GUI_COLOR_MENU_ITEM_HOVER,
	GUI_COLOR_MENU_ITEM_ACTIVE,
	GUI_COLOR_MENU_ITEM_ALWAYS,
	GUI_COLOR__MAX
};
static SDL_Color GUI_COLORS[GUI_COLOR__MAX] = {
	/*R     G     B     A */
	{ 0,    0,    0,    255 }, /* _BACKGROUND */
	{ 8,    16,   32,   255 }, /* _BACKGROUND_ACTIVE */
	{ 4,    16,   32,   255 }, /* _MENU_BACKGROUND */
	{ 32,   64,   128,  255 }, /* _MENU_BORDER */
	{ 0x8A, 0x8A, 0x8A, 255 }, /* _MENU_ITEM */
	{ 0xE0, 0xE0, 0xE0, 255 }, /* _MENU_ITEM_HOVER */
	{ 0x00, 0xC8, 0xFF, 255 }, /* _MENU_ITEM_ACTIVE */
	{ 0xFF, 0xFF, 0xFF, 255 }, /* _MENU_ITEM_ALWAYS */
};
#define GUI_COLOR(GCI) GUI_COLORS[GUI_COLOR_ ## GCI]
#define GUI_COLOR3(GCI) GUI_COLOR(GCI).r, GUI_COLOR(GCI).g, GUI_COLOR(GCI).b
#define GUI_COLOR4(GCI) GUI_COLOR3(GCI), GUI_COLOR(GCI).a

#define TERM_RGB(ATTR) color_table[ATTR][1],color_table[ATTR][2],color_table[ATTR][3]

/* plog() hook to display a message box. similar to WIN32 client */
static void hack_plog(cptr str)
{
	const SDL_MessageBoxButtonData buttons[] = {
	{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" } };
	const SDL_MessageBoxColorScheme colorScheme = { {
	{ GUI_COLOR3(BACKGROUND_ACTIVE) }, /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
	{ GUI_COLOR3(MENU_ITEM_ALWAYS)  }, /* [SDL_MESSAGEBOX_COLOR_TEXT] */
	{ GUI_COLOR3(MENU_BORDER)       }, /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
	{ GUI_COLOR3(MENU_BACKGROUND)   }, /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
	{ GUI_COLOR3(MENU_ITEM_HOVER)   }  /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
	} };
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		"MAngband", /* .title */
		str, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
		SDL_Log("error displaying message box");
	}
	printf("%s\n", str);
	return;
}

/* ==== Global data ==== */
static struct TermData terms[TERM_MAX];    // Our terminals
static struct FontData fonts[TERM_MAX];    // Our fonts, tied to the term limit
static struct PictData picts[TERM_MAX];    // Our picts, ^
static char default_font[128];
static int default_font_size = 12;
static char default_font_small[128];
static int default_font_small_size = 10;


#define MAX_QUICK_FONTS 24
char quick_font_names[MAX_QUICK_FONTS][1024] = {
	"nethack10x19-10.hex",
	"misc6x13.hex",
	""
};

#define MAX_FONT_SIZES 24
const int ttf_font_sizes[MAX_FONT_SIZES] = {
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 18, 20, 24, 30, 32, 36, 48, 0
};

#define MAX_ZOOM_LEVELS 6
static int zoom_levels[MAX_ZOOM_LEVELS] = {
	25,
	50,
	100,
	200,
	400,
	800,
};

/* Forward-declare */
void Term2_refresh_char(int x, int y);
bool Term2_cave_char(int x, int y, byte a, char c, byte ta, char tc);
void Term2_query_area_size(s16b *x, s16b *y, int st);
void Term2_screen_keyboard(int show, int hint);
void Term2_window_updated(u32b flags);
bool Term2_ask_dir(const char *prompt, bool allow_target, bool allow_friend);
bool Term2_ask_command(const char *prompt, bool shopping);
bool Term2_ask_item(const char *prompt, bool mode, bool inven, bool equip, bool onfloor);
bool Term2_ask_spell(const char *prompt, int realm, int book);
bool Term2_ask_confirm(const char *prompt);
bool Term2_ask_menu(menu_type *menu, menu_get_entry func, int row, int col);

/* Helpers */

/* There are some bugs in SDL2, so sadly, here's what we do: */
static void getRendererSize(TermData *td, int *w, int *h)
{
	//SDL_GetWindowSize(td->window, w, h);
#ifdef ON_OSX
	/* On OSX, this seems to be more reliable */
	SDL_GL_GetDrawableSize(td->window, w, h);
#else
	/* Assume there are no further bugs... */
	SDL_SetRenderTarget(td->renderer, NULL);
	SDL_GetRendererOutputSize(td->renderer, w, h);
#endif
}
static bool isHighDPI(void)
{
	int i = 0;
#if 0
	Uint32 hd = SDL_GetWindowFlags(terms[i].window);
	return (hd & SDL_WINDOW_ALLOW_HIGHDPI);
#else
	int winW, winH, highW, highH;
	SDL_GetWindowSize(terms[i].window, &winW, &winH);
	getRendererSize(&terms[i], &highW, &highH);
	return (winW == highW && winH == highH) ? FALSE : TRUE;
#endif
}
static void mobileForceOptions(void)
{
#ifdef USE_ICON_OVERLAY
	auto_itemlist = FALSE;
	auto_showlist = FALSE;
	auto_accept = FALSE;
#endif
}
static void mobileAutoLayout(void)
{
	TermData *td = &terms[0];
	int maxw, maxh;
	int w = td->font_data->w;
	int h = td->font_data->h;

	getRendererSize(td, &maxw, &maxh);

	maxh -= 64;
	maxw -= 64;
#ifdef MOBILE_UI
	if (SDL_IsTablet() || isHighDPI())
	{
		w *= 2;
		h *= 2;
		td->cell_mode = TERM_CELL_CUST;
		td->char_mode = TERM_CHAR_STRETCH;
		td->orig_w = td->cell_w = w;
		td->orig_h = td->cell_h = h;
	}
#endif
	td->x = 0;
	td->y = 0;
	td->need_render = TRUE;

	resizeTerm(td, MAX(maxw / w, 80), MAX(maxh / h, 24));
	refreshTerm(td);
}

/* Options: */
static bool sdl_game_mouse = TRUE;
static bool sdl_combine_arrowkeys = FALSE;
static int  sdl_combiner_delay = 50;
static bool ignore_keyboard_layout = FALSE;
static bool collapse_numpad_keys = FALSE;

  ////////////////////////////
 /* ==== Icon Overlay ==== */
////////////////////////////
static struct FontData tiny_font;
static struct FontData cmd_font;
static SDL_Texture *tx_tiny = NULL;
static SDL_Texture *tx_cmd = NULL;

enum IconContext {

	MICONS_SELECTOR_ROOT,
	MICONS_DIRECTION,
	MICONS_ITEM,
	MICONS_SPELL,
	MICONS_COMMANDS,
	MICONS_CONFIRMATION,
	MICONS_PASSED_MENU,

	MICONS_SYSTEM,
	MICONS_QUICKEST,
	MICONS_ALTER,
	MICONS_INVEN,
	MICONS_STORE,
	MICONS_EQUIP,
	MICONS_SPELLS,
	MICONS_SELECTOR_CLOSE,

};

/* TODO: move those to a struct? */
static int icon_hover = -1;
static int icon_pressed = -1;
static int icon_pressed_x = -1;
static int icon_pressed_y = -1;
static int term2_icon_context = -1;
static int term2_icon_pref = -1;
static int term2_item_pref = -1;
static int term2_spell_pref = -1;
static int micon_command_filter = MICONS_QUICKEST;
static bool micon_allow_target = FALSE;
static bool micon_allow_friend = FALSE;
static bool micon_allow_inven = FALSE;
static bool micon_allow_equip = FALSE;
static bool micon_allow_floor = FALSE;
static int micon_spell_realm = -1;
static int micon_spell_book = -1;
static menu_type* micon_passed_menu = NULL;
static menu_get_entry micon_passed_menu_resolver = NULL;
static int micon_passed_menu_row = 0;
static int micon_passed_menu_col = 0;

typedef struct IconData IconData;
struct IconData {
	SDL_Rect pos;
	int action;
	int sub_action;
	int draw;	/* Icon index from UI font (or -1) */
	byte a;     /* Attr on tileset (or 0) */
	byte c;     /* Char on tileset (or 0) */
};
static int num_icons = 0;
static IconData icons[256] = { 0 };

/* Quick-slots */
static IconData drag_icon = { 0 };
static bool dragging_icon = FALSE;
typedef struct SlotData SlotData;
struct SlotData {
	char macro_act[1024]; /* Macro to execute */
	char macro_item[1024]; /* Stand-alone item name (if any) */
	int item_counter; /* "cached" counter for items */
	int action;     /* IconAction to execute */
	int sub_action; /* IconAction argument */
	int draw;   /* Icon index from UI font */
	byte a;     /* Attr on tileset */
	byte c;     /* Char on tileset */
	byte attr;  /* Text color */
	char display[1024]; /* Text */
};
static SlotData drag_slot = { 0 };
static SlotData quick_slots[10] = { 0 };
/* End quick-slots */

#ifdef USE_LISENGLAS
  /////////////////////////
 /* ==== Lisenglas ==== */
/////////////////////////
static SDL_Texture *tx_circle = NULL;
static SDL_Texture *tx_lisen = NULL;
static SDL_Texture *lg_tex = NULL;

static bool lisen_enabled = FALSE;
static bool lisen_viable = FALSE;
static bool lisen_pressed = FALSE;
static int lisen_x = 0;
static int lisen_y = 0;
static int lisen_sx = 0;
static int lisen_sy = 0;
#endif

/* ==== Help ==== */
const char help_sdl2[] = "";

/* ==== Initialize function ==== */
/* init_sdl2
Our initializer function. Sets up SDL2 and creates our z-terms (windows).
*/
errr init_sdl2(void) {
	char buf[1024];
	int i;

	Uint32 init_flags = SDL_INIT_VIDEO;

	// **** Load in Configuration ****
	// The following global vars are set AFTER init_sdl2(), but as per below, we need 'em
	ANGBAND_SYS = "sdl2";

	// FIXME: this should be handled elsewhere, but we want colors now.
	strnfmt(buf, 1024, "font-%s.prf", ANGBAND_SYS);
	process_pref_file(buf);

#ifdef USE_SOUND
	init_flags |= SDL_INIT_AUDIO;
#endif

	// **** Initialize SDL libraries ****
	if (SDL_Init(init_flags) != 0)
	{
		plog_fmt("SDL_Init(): %s", SDL_GetError());
		return 1;
	}

	/* Activate msgbox hook */
	plog_aux = hack_plog;

	/* sdl-font */
	if (sdl_font_init() != 0) {
		plog("sdl_font_init(): fatal error");
		return 3;
	}

	/* load the possible graphics modes */
	init_graphics_modes("graphics.txt");

	/* set graphics mode */
	use_graphics = conf_get_int("SDL2", "Graphics", 0);
	if (use_graphics)
	{
		graphics_mode* gm = get_graphics_mode((byte)use_graphics);
		ANGBAND_GRAF = gm ? gm->pref : "none";
	}

#ifdef USE_SOUND
	/* Sound */
	load_sound_prefs();
	sdl_init_sound();
#endif

	/* Activate quit hook */
	quit_aux = quit_sdl2;

	/* Take keyboard layouts into account: */
	SDL_StartTextInput();

	// **** Load Preferences ****
	// FIXME: 0 is not guaranteed to be NULL, use a "clearTermData" func
	memset(terms, 0, sizeof(TermData)*TERM_MAX);

	loadConfig();

	// **** Merge command-line ****

	// **** Load Fonts and Picts ****
	memset(fonts, 0, TERM_MAX*sizeof(struct FontData));
	memset(picts, 0, TERM_MAX*sizeof(struct PictData));

	// **** Initialize z-terms ****
	for (i = 0; i < TERM_MAX; i++)
	{
		if (terms[i].config & TERM_IS_HIDDEN) continue;

		initTermData(&terms[i], ang_term_name[i], i, NULL);
		applyTermConf(&terms[i]);
		setTermTitle(&terms[i]);
		refreshTerm(&terms[i]);
		ang_term[i] = &(terms[i].t);
		if (terms[i].x < 0 || terms[i].y < 0
		 || terms[i].ren_rect.x < 0 || terms[i].ren_rect.y < 0)
		{
			termStack(i);
		}
		termConstrain(i);
	}

#ifdef USE_ICON_OVERLAY
	loadCmdFont("ui-cmd.ttf");
#  ifdef USE_SDL2_TTF
	loadTinyFont("5X8.FON");
#  else
	loadTinyFont("misc6x13.hex");
#  endif
#endif

#ifdef USE_LISENGLAS
	loadUiCircle();
#endif

	// **** Activate Main z-term and gooo ****
	Term_activate(&(terms[TERM_MAIN].t)); // set active Term to terms[TERM_MAIN]
	term_screen = Term;                   // set term_screen to terms[TERM_MAIN]

	/** Activate Term-2 hooks **/
	cave_char_aux = Term2_cave_char;
	query_size_aux = Term2_query_area_size;
	refresh_char_aux = Term2_refresh_char;
	screen_keyboard_aux = Term2_screen_keyboard;
	window_updated_aux = Term2_window_updated;
	z_ask_command_aux = Term2_ask_command;
	z_ask_dir_aux = Term2_ask_dir;
	z_ask_item_aux = Term2_ask_item;
	z_ask_spell_aux = Term2_ask_spell;
	z_ask_confirm_aux = Term2_ask_confirm;
	z_ask_menu_aux = Term2_ask_menu;

	return 0;
}

/* Our de-initializer */
void quit_sdl2(cptr s)
{
	/* save all values */
	saveConfig();

	/* Unload various things */
#ifdef USE_ICON_OVERLAY
	unloadCmdFont();
	unloadTinyFont();
#endif
#ifdef USE_LISENSGLAS
	unloadUiCircle();
#endif

	/* Call regular quit hook */
	quit_hook(s);

	/* Forget grafmodes */
	close_graphics_modes();

	/* Do SDL-specific stuff */
#ifdef USE_SOUND
	sdl_cleanup_sound();
#endif
	sdl_font_quit();
	SDL_Quit();
}

/* ==== Term related functions ==== */
static errr initTermData(TermData *td, cptr name, int id, cptr font)
{
	int width, height, key_queue;
	term *t = &td->t;

	td->id = id;
	if (name) strcpy(td->title, &name[0]);

	if (td->cell_w < 1) td->cell_w = 8;
	if (td->cell_h < 1) td->cell_h = 16;
	if (td->cols < 1) td->cols = 80;
	if (td->rows < 1) td->rows = (td->id == TERM_RECALL) ? 13 : 24;

	width = td->cell_w * td->cols;
	height = td->cell_h * td->rows;
	key_queue = 1024; // assume key queue of 1024 atm

	td->fb_w = 0;
	td->fb_h = 0;

	td->alt_framebuffer = NULL;
	td->alt_fb_w = td->alt_fb_h = 0;

	td->need_render = TRUE;

	// If this a virtual term, we assume that TERM_MAIN
	// has already been fully initialized and use its window and renderer
	if (td->config & TERM_IS_VIRTUAL && td->id != TERM_MAIN)
	{
		td->window = terms[TERM_MAIN].window;
		td->renderer = terms[TERM_MAIN].renderer;
		td->window_id = terms[TERM_MAIN].window_id;
		terms[TERM_MAIN].win_need_render = TRUE;
	}
	else
	{
		/* Initialize SDL2 window */
		int x = td->x;
		int y = td->y;
		Uint32 winflags = SDL_WINDOW_RESIZABLE;

		if (td->id == TERM_MAIN)
		{
			if (x < 0) x = SDL_WINDOWPOS_CENTERED;
			if (y < 0) y = SDL_WINDOWPOS_CENTERED;
			if (td->width <= 0 || td->height <= 0)
			{
				SDL_DisplayMode DM;
				SDL_GetCurrentDisplayMode(0, &DM);
				td->width = DM.w;
				td->height = DM.h - WM_DESKTOP_OFFSET;
			}

			if (td->width < width) td->width = width;
			if (td->height < height) td->height = height;

#ifdef MOBILE_UI
			winflags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
#else
			winflags = SDL_WINDOW_RESIZABLE;
#endif
		}

		if ((td->window = SDL_CreateWindow(td->title, x, y,
		     td->width, td->height, winflags)) == NULL)
		{
			plog_fmt("Could not create Window: %s", SDL_GetError());
			return 1;
		}
		td->window_id = SDL_GetWindowID(td->window);
		if ((td->renderer = SDL_CreateRenderer(td->window,
		-1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE)) == NULL)
		{
			plog_fmt("Could not create Renderer: %s", SDL_GetError());
			return 1;
		}
		/* Hack - force position */
		if (td->x >= 0 && td->y >= 0)
		{
			SDL_SetWindowPosition(td->window, td->x, td->y);
		}
		/* Or get position */
		else
		{
			SDL_GetWindowPosition(td->window, &td->x, &td->y);
		}
	}

	// Create our framebuffer via refreshing
	refreshTerm(td);

	/* Initialize Z-term stuff */
	term_init(t, td->cols, td->rows, key_queue);
	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;
	t->never_bored = TRUE; // dunno
	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	t->always_text = FALSE;

	/* Hooks */
	t->init_hook = initTermHook;
	t->nuke_hook = nukeTermHook;
	t->xtra_hook = xtraTermHook;
	t->curs_hook = cursTermHook;
	t->wipe_hook = wipeTermHook;
	t->text_hook = textTermHook;
	t->pict_hook = pictTermHook;
	t->data = (vptr)(td); // point our z-term to TermData
	//  Term_activate(t);
	td->config |= TERM_IS_ONLINE;

	return 0;
}

static errr applyTermConf(TermData *td) {
	char *font_file;
	int font_size;

	cptr fallback_font;
	int fallback_size;

	if (!STRZERO(td->font_file))
	{
		font_file = td->font_file;
	}
	else
	{
		font_file = (td->id == TERM_MAIN) ? default_font : default_font_small;
	}
	if (td->font_size != 0)
	{
		font_size = td->font_size;
	}
	else
	{
		font_size = (td->id == TERM_MAIN) ? default_font_size : default_font_small_size;
	}

	// unload/load fonts as needed
	if (td->font_data != NULL && strcmp(font_file, td->font_data->filename) != 0)
	{
		unloadFont(td);
	}

	if (td->font_data == NULL)
	{
		if (loadFont(td, font_file, font_size, (td->config & TERM_FONT_SMOOTH ? 1 : 0)) != 0)
		{
			// uhoh, let's try to load default
			if (loadFont(td, default_font, default_font_size, (td->config & TERM_FONT_SMOOTH ? 1 : 0)) != 0)
			{
				// UHOH, we even the default font doesn't work
				plog("Could not load any usable fonts!");
				quit(NULL);
			}
		}
	}

	// unload/load picts as needed
	if (td->pict_data != NULL && strcmp(td->pict_file, td->pict_data->filename) != 0)
	{
		unloadPict(td);
	}

	if (td->pict_file[0] != '\0')
	{
		if (loadPict(td, td->pict_file) != 0)
		{
			plog("Could not load pict file!");
		}
	}

	// disable zoom in ascii-mode
	if (!use_graphics)
	{
		td->config &= ~TERM_DO_SCALE;
	}
	// apply zoom settings
	if (!(td->config & TERM_DO_SCALE))
	{
		td->zoom = 100;
	} else {
		//force sane scale settings in zoom mode
		td->char_mode = TERM_CHAR_STATIC;
		td->pict_mode = TERM_PICT_STRETCH;
	}
#ifdef MOBILE_UI
	// set different settings
	if (SDL_IsTablet() || isHighDPI())
	{
		td->char_mode = TERM_CHAR_STRETCH;
	}
#endif

	if (td->zoom < 1) td->zoom = 1;
	if (td->zoom > 1000) td->zoom = 1000;
	// apply cell mode settings
	if (td->cell_mode == TERM_CELL_CUST)
	{
		setTermCells(td, td->orig_w, td->orig_h);
	}
	else if (td->cell_mode == TERM_CELL_PICT && td->pict_data != NULL)
	{
		setTermCells(td, td->pict_data->w, td->pict_data->h);
	}
	else if (td->font_data != NULL)
	{
		setTermCells(td, td->font_data->w, td->font_data->h);
	}
	return 0;
}

static errr setTermCells(TermData *td, int w, int h) {
	if (w <= 0 || h <= 0) return 1;
	td->cell_w = w;
	td->cell_h = h;
	// set our window to our cell size FIXME
	if (td->config & TERM_IS_VIRTUAL || td->id == TERM_MAIN)
	{
		td->ren_rect.w = td->cols * w;
		td->ren_rect.h = td->rows * h;
	}
	else
	{
		SDL_SetWindowSize(td->window, w*td->cols, h*td->rows);
	}
	return 0;
}

/* Here we have functions that are evil and cut right through
 * the heart of regular (M)Angband code flow. */

/* Our custom version of Term_queue_char() */
bool Term2_cave_char(int x, int y, byte a, char c, byte ta, char tc)
{
	TermData *td = (TermData*)(Term->data);

	/* If ALT.DUNGEON is off, defer to regular function. */
	if (!(td->config & TERM_DO_SCALE))
	{
		/* Actually, do nothing */
		return FALSE;
	}
#if 0
	/* Hack -- blank space */
	if (!c || c == ' ') {
		wipeTermCell_Cave(x, y);
		return TRUE;
	}
#endif
	if ((byte)c < 127) {
		char buf[2] = { c, 0 };
		(void)textTermHook_ALT(x, y, 1, (byte)a, (cptr)&buf);
		return TRUE;
	}

	(void)pictTermHook_ALT(x, y, 1, &a, &c, &ta, &tc);
	return TRUE;
}
void Term2_query_area_size(s16b *x, s16b *y, int st) {
	TermData *td = (TermData*)ang_term[0]->data;
	/* ALT.DUNGEON is OFF, do nothing */
	if (!(td->config & TERM_DO_SCALE))
	{
		return;
	}

	/* Non-main stream */
	if (streams[st].flag & SF_OVERLAYED) {
		return;
		*x = td->cols - DUNGEON_OFFSET_X;
		*y = td->rows - DUNGEON_OFFSET_Y - DUNGEON_CLIP_Y;
	}
	/* Dungeon stream */
	else {
		*x = td->dng_cols;
		*y = td->dng_rows;
	}
}
/* Memorize cave to re-show it on Term2_refresh */
cave_view_type mymem[2][MAX_HGT][MAX_WID] = { 0 };
/* Restore cave from memory */
static void Term2_refresh_char1(int x, int y)
{
	byte a, c, ta, tc;

	if (y < 0 || y >= MAX_HGT) return;
	if (x < 0 || x >= MAX_WID) return;

	a = mymem[1][y][x].a;
	c = mymem[1][y][x].c;
	ta = mymem[0][y][x].a;
	tc = mymem[0][y][x].c;

	Term2_cave_char(x, y, a, c, ta, tc);
}
/* Hook for refreshing slash effects. We will draw 4 nearby tiles. */
void Term2_refresh_char(int x, int y)
{
	int dir = sfx_info[y][x].a;
	int ox, oy;
	slashfx_dir_offset(&ox, &oy, dir, TRUE);
	Term2_refresh_char1(x + ox, y + oy);
	Term2_refresh_char1(x + ox * 0, y + oy);
	Term2_refresh_char1(x + ox, y + oy * 0);
	Term2_refresh_char1(x, y);
}
/* Hook for managing on-screen keyboard */
void Term2_screen_keyboard(int show, int hint)
{
	/* Does nothing on non-mobile platforms */
#ifdef MOBILE_UI
	if (show)
	{
		SDL_StartTextInput();
	}
	else
	{
		SDL_StopTextInput();
	}
#endif
}
#ifdef USE_ICON_OVERLAY
/* Forward-declare: */static void recountSlotItems(void);
#endif
void Term2_window_updated(u32b flags)
{
#ifdef USE_ICON_OVERLAY
	if ((flags & PW_INVEN) | (flags & PW_EQUIP))
	{
		recountSlotItems();
	}
#endif
}

bool Term2_ask_command(const char *prompt, bool shopping)
{
	term2_icon_context = MICONS_COMMANDS;
	if (shopping) term2_icon_context = MICONS_STORE;
	return FALSE;
}

bool Term2_ask_dir(const char *prompt, bool allow_target, bool allow_friend)
{
	term2_icon_context = MICONS_DIRECTION;
	micon_allow_target = allow_target;
	micon_allow_friend = allow_friend;
	return FALSE;
}
bool Term2_ask_item(const char *prompt, bool mode, bool inven, bool equip, bool onfloor)
{
	term2_icon_context = MICONS_ITEM;
	if (mode) term2_icon_context = MICONS_EQUIP;
	micon_allow_inven = inven;
	micon_allow_equip = equip;
	micon_allow_floor = onfloor;
	return FALSE;
}
bool Term2_ask_spell(const char *prompt, int realm, int book)
{
	term2_icon_context = MICONS_SPELL;
	micon_spell_realm = realm;
	micon_spell_book = book;
	return FALSE;
}
bool Term2_ask_confirm(const char *prompt)
{
	term2_icon_context = MICONS_CONFIRMATION;
	return FALSE;
}

bool Term2_ask_menu(menu_type *menu, menu_get_entry func, int row, int col)
{
	term2_icon_context = MICONS_PASSED_MENU;
	micon_passed_menu = menu;
	micon_passed_menu_resolver = func;
	micon_passed_menu_row = row;
	micon_passed_menu_col = col;
	if (menu == NULL) term2_icon_context = -1;
	return (FALSE);
}

void net_stream_clamp(int addr, int *x, int *y) {
	int i;
	for (i = 0; i < known_streams; i++) {
		if (streams[i].addr == addr) {
			if (*x < streams[i].min_col) *x = streams[i].min_col;
			if (*x > streams[i].max_col) *x = streams[i].max_col;
			if (*y < streams[i].min_row) *y = streams[i].min_row;
			if (*y > streams[i].max_row) *y = streams[i].max_row;
			return;
		}
	}
}
void refreshTermAlt(TermData *td) {
	int dw, dh;
	int dng_cols, dng_rows;

	int fw = td->cols - DUNGEON_OFFSET_X;
	int fh = td->rows - DUNGEON_OFFSET_Y - DUNGEON_CLIP_Y;

	int pix_w = fw * td->cell_w + 1;
	int pix_h = fh * td->cell_h + 1;

	pix_w = pix_w * 100 / td->zoom;
	pix_h = pix_h * 100 / td->zoom;

	dng_cols = pix_w / td->pict_data->w;
	dng_rows = pix_h / td->pict_data->h;

	if (!(td->config & TERM_DO_SCALE)) {
		if (td->alt_framebuffer) {
			SDL_DestroyTexture(td->alt_framebuffer);
			td->alt_framebuffer = NULL;
		}
		if (td->dng_cols || td->dng_rows) {
			td->need_redraw = TRUE;
		}
		td->dng_cols = 0;
		td->dng_rows = 0;
		td->alt_fb_w = 0;
		td->alt_fb_h = 0;
		if (td->need_redraw) {
			// (re)subscribe
			net_term_update(FALSE);
		}
		return;
	}

	net_stream_clamp((int)td->id, &dng_cols, &dng_rows);

	dw = (dng_cols) * td->pict_data->w;
	dh = (dng_rows) * td->pict_data->h;

	td->dng_rect.x = DUNGEON_OFFSET_X * td->cell_w;
	td->dng_rect.y = DUNGEON_OFFSET_Y * td->cell_h;

	if ((td->dng_cols == dng_cols) && (td->dng_rows == dng_rows)
	&& (td->alt_fb_w == dw) && (td->alt_fb_h == dh))
		return;

	if (td->alt_framebuffer) SDL_DestroyTexture(td->alt_framebuffer);

	td->dng_rect.w = dw;
	td->dng_rect.h = dh;

	td->dng_cols = dng_cols;
	td->dng_rows = dng_rows;

	if ((td->alt_framebuffer = SDL_CreateTexture(td->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dw, dh)) == NULL) {
		plog_fmt("refreshTermAlt: %s", SDL_GetError());
		return;
	 }

	td->alt_fb_w = dw;
	td->alt_fb_h = dh;

	// wipe the new framebuffer
	SDL_SetRenderTarget(td->renderer, td->alt_framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_RenderClear(td->renderer);

	// (re)subscribe
	net_term_update(FALSE);

	td->need_redraw = TRUE;
}

static errr refreshTerm(TermData *td) {
	int w, h;

	if (td->id == TERM_MAIN && td->font_data && td->pict_data)
	{
		refreshTermAlt(td);
	}

	if (td->config & TERM_IS_VIRTUAL || td->id == TERM_MAIN)
	{
		w = td->ren_rect.w;
		h = td->ren_rect.h;
	}
	else
	{
		getRendererSize(td, &w, &h);
		/* HACK -- on OSX, newly created window has size of 1x1,
		 * let's just say it's unusable... */
		if (w == 1 && h == 1) return -1;
	}

	if (td->fb_w == w && td->fb_h == h) return 0;
	if (td->framebuffer) SDL_DestroyTexture(td->framebuffer);

	if ((td->framebuffer = SDL_CreateTexture(td->renderer,
	SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)) == NULL)
	{
		plog_fmt("refreshTerm: %s", SDL_GetError());
		return 1;
	}

	td->need_redraw = TRUE;

	td->fb_w = w;
	td->fb_h = h;
	td->ren_rect.w = w;
	td->ren_rect.h = h;

	if (!(td->config & TERM_IS_VIRTUAL))
	{
		td->width = w;
		td->height = h;
	 }

	// refresh the renderer to match the new window dimensions
	SDL_RenderSetViewport(td->renderer, NULL);

	// wipe the new framebuffer
	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 0, 200, 0, 100);
	SDL_RenderClear(td->renderer);

	return 0;
}

static errr resizeWindow(TermData *td, int width, int height)
{
	//SDL_SetWindowSize(td->window, width, height);
	td->width = width;
	td->height = height;
#ifdef MOBILE_UI
	mobileAutoLayout();
#endif
	refreshTerm(td);
	return 0;
}
static errr resizeTerm(TermData *td, int cols, int rows)
{
	term *old_td;
	if (rows <= 0 || cols <= 0) return 1;
	if (rows == td->rows && cols == td->cols) return 0;

	td->rows = rows;
	td->cols = cols;

	if (td->config & TERM_IS_VIRTUAL || td->id == TERM_MAIN)
	{
		td->ren_rect.w = td->cell_w*cols; td->ren_rect.h = td->cell_h*rows;
	}
	else
	{
		SDL_SetWindowSize(td->window, td->cell_w*cols, td->cell_h*rows);
	}
	setTermTitle(td);

	// store current term, activate this, resize, and restore previous
	old_td = Term;
	Term_activate(&(td->t));
	Term_resize(cols, rows);
	Term_activate(old_td);

	return 0;
}
static errr setTermTitle(TermData *td)
{
	char buf[1024];
	strnfmt(buf, 1024, "%s (%dx%d)", td->title, td->cols, td->rows);
	/* If this is main window, do some extra hackage */
	if (td->window == terms[TERM_MAIN].window)
	{
		my_strcpy(buf, "MAngband", 1024);
		if (conn_state > 0)
		{
			strnfmt(buf, 1024, "MAngband - %s (%s)", nick, server_name);
		}
	}
	SDL_SetWindowTitle(td->window, buf);
	return 0;
}

/* loadFont
This function attempts to load and attach the given font name and font size to the TermData.
It first checks all existing FontData structures to see if a FontData with the same settings already exists, and if so, simply attaches that FontData. Otherwise, it will create the given FontData structure and attach it.
*/
static errr loadFont(TermData *td, cptr filename, int fontsize, int smoothing) {
	char *font_error = NULL;
	int i;
	for (i = 0; i < TERM_MAX; i++)
	{
		if ((strcmp(terms[i].font_file, filename) == 0)
		&& terms[i].font_size == fontsize
		&& (terms[i].config & TERM_FONT_SMOOTH) == (smoothing ? TERM_FONT_SMOOTH : 0) // eww
		&& terms[i].font_data != NULL)
		{
			attachFont(terms[i].font_data, td);
			return 0;
		}
	}

	// font data does not exist, let's create it in the first available FontData slot
	for (i = 0; i < TERM_MAX; i++)
	{
		if (fonts[i].surface == NULL)
		{
			if (fileToFont(&fonts[i], filename, fontsize, smoothing) != 0)
			{
				font_error = "Can't load font";
				break; //error
			}
			attachFont(&fonts[i], td);

			if (i == TERM_MAIN)
			{
				char *ext = strrchr(filename, '.');
				ANGBAND_FON = ext ? string_make(ext+1) : ANGBAND_FON;
				ANGBAND_FONTNAME = string_make(filename);

				net_visuals_update();
			}

			return 0;
		}
	}
	// plog_fmt("loadFont(): %s", font_error, NULL);
	return 1;
}

/* unloadFont
This function unloads the FontData from the TermData and will attempt to destroy the given FontData if it does not referenced by any Term
*/
static errr unloadFont(TermData *td)
{
	int i;
	FontData *fd = td->font_data;
	detachFont(td);
	for (i = 0; i < TERM_MAX; i++)
	{
		if (terms[i].font_data == fd)
		{
			// still referenced, bail
			return 0;
		}
	}

	// isn't referenced anymore, let's delete it!
	cleanFontData(fd);
	return 0;
}
/* attachFont
This function creates an SDL_Texture from the given FontData's surface, then sets
needed Term options.
*/
static errr attachFont(FontData *fd, TermData *td)
{
	if ((td->font_texture = SDL_CreateTextureFromSurface(td->renderer, fd->surface)) == NULL)
	{
		plog_fmt("Error: attachFont(): %s\n", SDL_GetError());
	        return 1;
	}

	SDL_SetTextureBlendMode(td->font_texture, SDL_BLENDMODE_BLEND);
	td->font_data = fd;
#ifdef MOBILE_UI
	mobileAutoLayout();
#endif
	return 0;
}

/* detachFont
This function destroys the SDL_Texture created by attachFont
*/
errr detachFont(TermData *td)
{
	if (td->font_texture) SDL_DestroyTexture(td->font_texture);
	td->font_texture = NULL;
	td->font_data = NULL;
	return 0;
}

/* loadPict
This function attempts to load and attach the given pict to the TermData.
It first checks all existing PictData structures to see if a PictData with the same settings already exists, and if so, simply attaches that PictData. Otherwise, it will create the given PictData structure and attach it.
*/
errr loadPict(TermData *td, cptr filename)
{
	int i;
	for (i = 0; i < TERM_MAX; i++)
	{
		if ((strcmp(terms[i].pict_file, filename) == 0)
		&& terms[i].pict_data != NULL)
		{
			attachPict(terms[i].pict_data, td);
			return 0;
		}
	}
	// pict data does not exist, let's create it in the first available PictData slot
	for (i = 0; i < TERM_MAX; i++)
	{
		if (picts[i].surface == NULL)
		{
			if (imgToPict(&picts[i], filename, terms[i].mask_file) != 0)
			{
				break; // error!
			}
			attachPict(&picts[i], td);
			return 0;
		}
	}
	plog_fmt("loadPict(): %s", "Could not load pict data!");
	return 1;
}

/* unloadPict
This function unloads the PictData from the TermData and will attempt to destroy the given PictData if it does not referenced by any Term
*/
static errr unloadPict(TermData *td)
{
	int i;
	PictData *pd = td->pict_data;
	detachPict(td);
	for (i = 0; i < TERM_MAX; i++)
	{
		if (terms[i].pict_data == pd)
		{
			// still referenced, bail
			return 0;
		}
	}
	// isn't referenced anymore, let's delete it!
	cleanPictData(pd);
	return 0;
}
/* attachPict
This function creates an SDL_Texture from the given PictData's surface. It then sets required
*/
static errr attachPict(PictData *pd, TermData *td)
{
	if ((td->pict_texture = SDL_CreateTextureFromSurface(td->renderer, pd->surface)) == NULL)
	{
		plog_fmt("attachPict Error: %s", SDL_GetError());
		return 1;
	}

	SDL_SetTextureBlendMode(td->pict_texture, SDL_BLENDMODE_BLEND);
	td->pict_data = pd;
	td->t.higher_pict = TRUE; // enable "pict" graphics
	return 0;
}
/* detachPict
This function "detaches' the given term's pict data - in effect, it destroys the SDL_Texture
and deactivates pict-related settings
*/
static errr detachPict(TermData *td)
{
	if (td->pict_texture) SDL_DestroyTexture(td->pict_texture);
	td->pict_texture = NULL;
	td->pict_data = NULL;
	td->t.higher_pict = FALSE;
	return 0;
}

/* ==== z-term hooks ==== */
static void initTermHook(term *t) {}
static void nukeTermHook(term *t)
{
	TermData *td = (TermData*)(t->data);

	// detach (free texture and NULL pointers) to font/pict if attached
	unloadFont(td);
	unloadPict(td);

	// destroy self
	if (td->framebuffer) SDL_DestroyTexture(td->framebuffer);
	if (td->alt_framebuffer) SDL_DestroyTexture(td->alt_framebuffer);
	td->framebuffer = td->alt_framebuffer = NULL;
	td->fb_w = td->fb_h = 0;
	td->alt_fb_w = td->alt_fb_h = 0;

	if (!(td->config & TERM_IS_VIRTUAL) || td->id == TERM_MAIN)
	{
		int i;
		for (i = 0; i < TERM_MAX; i++)
		{
			if (i == td->id) continue;
			if (terms[i].renderer == td->renderer) terms[i].renderer = NULL;
			if (terms[i].window == td->window) terms[i].renderer = NULL;
		}
		if (td->renderer) SDL_DestroyRenderer(td->renderer);
		td->renderer = NULL;
		if (td->window) SDL_DestroyWindow(td->window);
		td->window = NULL;
	}

	/* Term is no longer visible */
	td->config &= ~TERM_IS_ONLINE;
}

/* Rendering and GUI */
static int drag_x = 0;
static int drag_y = 0;
static int accum_x = 0;
static int accum_y = 0;
static int dragging = -1;
static int gripping = 0;
static float accum_pinch = 0;

static int alt_cursor_x = -1;
static int alt_cursor_y = -1;

static void renderWindow(TermData *mtd)
{
	SDL_Color *bgcol;
	int i, j;
	bgcol = &GUI_COLORS[menu_mode ? GUI_COLOR_BACKGROUND_ACTIVE : GUI_COLOR_BACKGROUND];
	SDL_SetRenderTarget(mtd->renderer, NULL);
	SDL_SetRenderDrawColor(mtd->renderer, bgcol->r, bgcol->g, bgcol->b, bgcol->a);
	SDL_RenderClear(mtd->renderer);

	// Render ALT.DUNGEON
	if (mtd->id == TERM_MAIN && mtd->alt_framebuffer)
	{
		SDL_Rect src = { 0, 0, mtd->alt_fb_w, mtd->alt_fb_h };
		SDL_Rect dst = {
			mtd->ren_rect.x + mtd->dng_rect.x,
			mtd->ren_rect.y + mtd->dng_rect.y,
			mtd->alt_fb_w, mtd->alt_fb_h
		};
		dst.w = dst.w * terms[0].zoom /100;
		dst.h = dst.h * terms[0].zoom /100;

		SDL_RenderCopy(terms[0].renderer, terms[0].alt_framebuffer, &src, &dst);
		/* Render cursor */
		if (mtd->pict_data && alt_cursor_y >= 0)
		{
			SDL_Rect cell_rect;
			cell_rect.w = mtd->pict_data->w * terms[0].zoom /100;
			cell_rect.h = mtd->pict_data->h * terms[0].zoom /100;
			cell_rect.x = dst.x + alt_cursor_x * cell_rect.w;
			cell_rect.y = dst.y + alt_cursor_y * cell_rect.h;
			SDL_SetRenderDrawColor(mtd->renderer, 128, 255, 64, 255);
			SDL_RenderDrawRect(mtd->renderer, &cell_rect);
		}

	}

	/* Render all terms */
	for (i = 0; i < TERM_MAX; i++)
	{
		TermData *td = &terms[i]; /* or TERM_MAX - 1 - i to invert */
		if (!(td->config & TERM_IS_ONLINE)) continue;
		if (td->window_id != mtd->window_id) continue;

		renderTerm(td);
	}

	/* Render MOBILE OVERLAY */
	if (mtd->id == TERM_MAIN && mtd->alt_framebuffer && mtd->pict_data)
	{
#ifdef USE_LISENGLAS
		renderLisenGlas(mtd);
#endif
#ifdef USE_ICON_OVERLAY
		renderIconOverlay(mtd);
#endif
	}

	SDL_RenderPresent(mtd->renderer);
}

static void renderTerm(TermData *td)
{
	SDL_Rect srect = { 0, 0, td->ren_rect.w, td->ren_rect.h };

	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(td->framebuffer, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(td->renderer, NULL);
	SDL_SetRenderDrawColor(td->renderer, 32, 32, 32, 255);

	SDL_RenderCopy(td->renderer, td->framebuffer, &srect, &td->ren_rect);

	renderGui(td);
}

static int looksLikeCave(int x, int y)
{
	TermData *td = (TermData*)(Term->data);
	/*printf("Print cave for row %d (term rows: %d, dungeon rows: %d)\n",
	y, td->rows, td->dng_rows);*/
	if (x >= DUNGEON_OFFSET_X && y >= DUNGEON_OFFSET_Y
	 && y < td->rows - DUNGEON_OFFSET_Y - DUNGEON_CLIP_Y)
	{
		if (state < PLAYER_PLAYING) return 0;
		if (screen_icky) return 0;
		if (section_icky_row > 0 && y < section_icky_row)
		{
			if (section_icky_col > 0 && x < section_icky_col)
				return 0;
			if (section_icky_col < 0 && x >= td->cols + section_icky_col)
				return 0;
		}
		return 1;
	}
	return 0;
}

static void altDungeonCutout(void)
{
	int y, x, sx, sy;
	TermData *td = (TermData*)(Term->data);
	if (!(td->config & TERM_DO_SCALE)) return;

	sx = DUNGEON_OFFSET_X;
	sy = DUNGEON_OFFSET_Y;

	if (state < PLAYER_PLAYING) return;
	if (screen_icky) return;

	for (y = sy; y < td->rows - DUNGEON_CLIP_Y; y++) {
		/* Hack -- forget tile we cutted out */
		byte *old_aa = Term->old->a[y];
		char *old_cc = Term->old->c[y];
		byte *old_taa = Term->old->ta[y];
		char *old_tcc = Term->old->tc[y];

		for (x = sx; x < td->cols; x++)
		{
			if (section_icky_row > 0 && y < section_icky_row)
			{
				if (section_icky_col > 0 && x < section_icky_col)
					continue;
				if (section_icky_col < 0 && x >= td->cols + section_icky_col)
					continue;
			}

			wipeTermCell_UI(x, y, 1);

			/* Hack -- forget tile we cutted out */
			old_aa[x] = 0;
			old_cc[x] = 0;
			old_taa[x] = 0;
			old_tcc[x] = 0;
		}
	}
}

static void rerender()
{
	int i;
	for (i = 0; i < TERM_MAX; i++) {
		if (!(terms[i].config & TERM_IS_ONLINE)) continue;
		if (terms[i].need_redraw) {
			term *old_td = Term;
			//if (i == TERM_MAIN) {
			//	p_ptr->redraw |= 0xffffffff;
			//}
			Term_activate(&(terms[i].t));
			Term_redraw();
			Term_activate(old_td);
			terms[i].need_redraw = FALSE;
			terms[i].need_render = TRUE;
		}
		if (terms[i].need_cutout) {
			altDungeonCutout();
			terms[i].need_cutout = FALSE;
		}
	}
	for (i = 0; i < TERM_MAX; i++) {
		if (!(terms[i].config & TERM_IS_ONLINE)) continue;
		if (terms[i].need_render) {
			if (terms[i].config & TERM_IS_VIRTUAL) {
				terms[TERM_MAIN].win_need_render = TRUE;
			} else {
				terms[i].win_need_render = TRUE;
			}
			terms[i].need_render = FALSE;
		}
	}
	for (i = 0; i < TERM_MAX; i++) {
		if (!(terms[i].config & TERM_IS_ONLINE)) continue;
		if (terms[i].win_need_render) {

			renderWindow(&terms[i]);
			terms[i].win_need_render = FALSE;
		}
	}
}

static void mustRerender(void)
{
	int i;
	for (i = 0; i < TERM_MAX; i++) {
		terms[i].need_render = TRUE;
	}
}

// MMM BEGIN
/* ==== Magical Mangband Menu ==== */
// NOTE: These Menus should be scrapped, but I wanted a quick, cross-platform method to access
// and modify configurations for the SDL2 client. It is mostly an ugly growth that sits on top
// of the regular system. I have kept all of the menu-related code separated down here so that
// removal could be done with ease. To remove, search for "MMM" and remove the relevant code. :)
// (I am thankful that of all the things in this file, I consider this the only "real" hack)
static int menu_mode = MENU_HIDE;	// Our extra "SDL2" menu mode
static int menu_x = -1, menu_y = -1;	// our menu x and y coordinates
static int menu_open = -1;
static int menu_open_term = -1;
static int menu_hover = -1;
static int menu_term = 0;

enum {
	MENU_NO_ACTION,

	MENU_SUB_ROOT,
	MENU_SUB_SOUND,
	MENU_SUB_WINDOW,
	MENU_SUB_GRAPHICS,
	MENU_SUB_WINDOWS,
	MENU_SUB_WINSIZE,
	MENU_SUB_ZOOM,
	MENU_SUB_FONTS,
	MENU_SUB_FONTSIZE,

	MENU_ACT_MIN,

	MENU_QUICK_FONT0,
	MENU_QUICK_FONT_LAST = MENU_QUICK_FONT0 + MAX_QUICK_FONTS,

	MENU_QUICK_GRAF0,
	MENU_QUICK_GRAF1,
	MENU_QUICK_GRAF2,
	MENU_QUICK_GRAF3,
	MENU_QUICK_GRAF4,
	MENU_QUICK_GRAF5,
	MENU_QUICK_GRAF6,
	MENU_QUICK_GRAF7,
	MENU_QUICK_GRAF8,

	MENU_ZOOM_OFF,
	MENU_ZOOM_LEVEL0,
	MENU_ZOOM_LEVEL1,
	MENU_ZOOM_LEVEL2,
	MENU_ZOOM_LEVEL3,
	MENU_ZOOM_LEVEL4,
	MENU_ZOOM_LEVEL5,
	MENU_ZOOM_LEVEL6,
	MENU_ZOOM_LEVEL7,
	MENU_ZOOM_LEVEL8,

	MENU_FONT_SIZE0,
	MENU_FONT_SIZE_LAST = MENU_FONT_SIZE0 + MAX_FONT_SIZES,

	MENU_ACT_CLOSE,
	MENU_ACT_TOGGLE_VIRTUAL,
	MENU_SHOW_WINDOW0,
	MENU_SHOW_WINDOW1,
	MENU_SHOW_WINDOW2,
	MENU_SHOW_WINDOW3,
	MENU_SHOW_WINDOW4,
	MENU_SHOW_WINDOW5,
	MENU_SHOW_WINDOW6,
	MENU_SHOW_WINDOW7,
	MENU_SHOW_WINDOW8,
	MENU_ACT_SOUND_OFF,
	MENU_ACT_SOUND_ON,

	MENU_ACT_TOGGLE_GAMEMOUSE,
	MENU_ACT_TOGGLE_ARROWCOMBINER,
	MENU_ACT_TOGGLE_IGNORELAYOUT,
	MENU_ACT_TOGGLE_COLLAPSENUMPAD,

	MENU_ACT_MAX,
};

static errr renderTButton(TermData *td, int cx, int cy, char *buf, int menuID) {
	int i = 0;
	int color = 8;
	int n = strlen(buf);
	int grid_w = td->font_data->w;
	int grid_h = td->font_data->h;
	int x = cx * grid_w + td->menu_rect.x;
	int y = cy * grid_h + td->menu_rect.y;
	int w = n * grid_w;
	int h = 1 * grid_h;

	if (menuID == MENU_NO_ACTION)
	{
		color = 1;
	}
	else if (menu_term == td->id
	 && menu_x >= x && menu_y >= y
	 && menu_x <= x + w - 1
	 && menu_y <= y + h - 1)
	{
		color = 9;
		menu_hover = menuID;
	}
	if (menu_open == menuID && menu_open_term == td->id)
	{
		color = 14;
	}

	sysText(td, cx, cy, n, color, buf);
	return 0;
}


static errr renderGui(TermData *td)
{
	SDL_Rect rect;
	int grid_w, grid_h;
	int width, height, i, y, growth;
	int ox, oy;
	int cx;
	int sub_x = 0, sub_w = 0;

	if (!menu_mode) return 0;

	grid_w = td->font_data->w;
	grid_h = td->font_data->h;

	ox = td->ren_rect.x;
	oy = td->ren_rect.y - grid_h;

	if (oy < 0) oy = 0;
	growth = td->ren_rect.y - oy;
	width  = grid_w * (td->cols);
	height = grid_h * (td->rows);

	// draw window border
	rect.x = ox;
	rect.y = oy;
	rect.w = width;
	rect.h = height + growth;

	if (dragging == td->id && gripping) {
		rect.w = td->resize_rect.w;
		rect.h = td->resize_rect.h + growth;
	}

	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BORDER));
	SDL_RenderDrawRect(td->renderer, &rect);

	// draw menu bar
	rect.h = grid_h * (1);
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BACKGROUND));
	SDL_RenderFillRect(td->renderer, &rect);
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BORDER));
	SDL_RenderDrawRect(td->renderer, &rect);

	/* Store menu_rect */
	td->menu_rect = rect;
	td->menu_rect.y = td->ren_rect.y - growth;

	/* Re-find hovering button */
	if (td->id == menu_term) menu_hover = -1;

	/* Start at the top */
	cx = 0;
	y = 0;

	// Main button
	if (td->id == TERM_MAIN)
	{
		renderTButton(td, cx, 0, "[@]", MENU_SUB_ROOT);
		if (menu_open == MENU_SUB_ROOT && menu_open_term == td->id) {
			char buf[128];
			int sx = cx - 1;
			int sy = y + 1;
			strnfmt(buf, 32, " [%c] In-Game Mouse      ", (sdl_game_mouse ? 'X' : '.'));
			renderTButton(td, sx, sy + 0, buf, MENU_ACT_TOGGLE_GAMEMOUSE);
			strnfmt(buf, 32, " [%c] Combine Arrow Keys ", (sdl_combine_arrowkeys ? 'X' : '.'));
			renderTButton(td, sx, sy + 1, buf, MENU_ACT_TOGGLE_ARROWCOMBINER);
			td->menu_rect.h += 2 * grid_h;
			sub_w = 24;
			sub_x = sx;
		}
		cx += 3 + 1;
	} else cx += 1;

	// Window name
	{
		renderTButton(td, cx, y, td->title, MENU_NO_ACTION);
		cx += strlen(td->title) + 1;
	}

	// Window size
	{
		char buf[128];
		int cols = td->cols;
		int rows = td->rows;
		if (gripping && dragging == td->id) {
			cols = td->resize_rect.w / td->cell_w;
			rows = td->resize_rect.h / td->cell_h;
		}
		strnfmt(buf, 128, "(%dx%d)", cols, rows);
		renderTButton(td, cx, y, buf, MENU_NO_ACTION);//MENU_SUB_WINSIZE);
		cx += strlen(buf) + 1;
	}

	// Zoom
	if (td->id == TERM_MAIN && use_graphics > 0) {
		char buf[128];
		int fit = !(td->config & TERM_DO_SCALE);
		if (fit) {
			strnfmt(buf, 128, "Zoom: FIT");
		} else {
			strnfmt(buf, 128, "Zoom: %3d%%", td->zoom);
		}
		renderTButton(td, cx, y, buf, MENU_SUB_ZOOM);
		if (menu_open == MENU_SUB_ZOOM && menu_open_term == td->id) {
			int j = 0;
			int sx = cx - 1;
			int sy = y + 1;
			int on;
			strnfmt(buf, 32, " [%c] Fit the grid ", (fit ? 'X' : '.'));
			renderTButton(td, sx, sy, buf, MENU_ZOOM_OFF);
			for (j = 0; j < MAX_ZOOM_LEVELS; j++) {
				on = 0;
				if (td->zoom == zoom_levels[j]) on = 1;
				if (fit) on = 0;
				strnfmt(buf, 32, " [%c] %3d %%        ", (on ? 'X' : '.'),
				zoom_levels[j]);
				renderTButton(td, sx, sy + j + 1, buf, MENU_ZOOM_LEVEL0 + j);
			}
			td->menu_rect.h += (j+1) * grid_h;
			sub_w = 18;
			sub_x = sx;
		}
		cx += strlen("Zoom: 000X") + 1;
	}

	// Font
	{
		renderTButton(td, cx, y, "Font", MENU_SUB_FONTS);
		if (menu_open == MENU_SUB_FONTS && menu_open_term == td->id) {
			int sx = cx - 1;
			int sy = y + 1;
			int j;
			for (j = 0; j < MAX_QUICK_FONTS; j++) {
				char buf[32], on;
				if (STRZERO(quick_font_names[j])) break;
				on = 0;
				if (!strcmp(quick_font_names[j], td->font_file)) on = 1;
				strnfmt(buf, 32, " [%c] %-12s ", (on ? 'X' : '.'),
				quick_font_names[j]);
				renderTButton(td, sx, sy + j, buf, MENU_QUICK_FONT0 + j);
				sub_w = MATH_MAX(sub_w, strlen(quick_font_names[j]) + 6);
			}
			td->menu_rect.h += j * grid_h;
			sub_x = sx;
		}
		cx += strlen("Font") + 1;
	}

	// Fontsize
	if (td->font_data->scalable)
	{
		renderTButton(td, cx, y, "Fontsize", MENU_SUB_FONTSIZE);
		if (menu_open == MENU_SUB_FONTSIZE && menu_open_term == td->id) {
			int sx = cx - 1;
			int sy = y + 1;
			int j;
			for (j = 0; j < MAX_FONT_SIZES; j++) {
				char buf[32], on;
				if (ttf_font_sizes[j] == 0) break;
				on = 0;
				if (ttf_font_sizes[j] == td->font_size) on = 1;
				strnfmt(buf, 32, " [%c] %2dpt ", (on ? 'X' : '.'),
				ttf_font_sizes[j], j);
				renderTButton(td, sx, sy + j, buf, MENU_FONT_SIZE0 + j);
				sub_w = 10;
			}
			td->menu_rect.h += j * grid_h;
			sub_x = sx;
		}
		cx += strlen("Fontsize") + 1;
	}

	// Graphics
	if (td->id == TERM_MAIN)
	{
		renderTButton(td, cx, y, "Graphics", MENU_SUB_GRAPHICS);
		if (menu_open == MENU_SUB_GRAPHICS && menu_open_term == td->id) {
			int sx = cx - 1;
			int sy = y + 1;
			int si = 0;
			int j;
			for (j = 0; 0 < get_num_graphics_modes(); j++) {
				char buf[32], on;
				graphics_mode *gm = get_graphics_mode((byte)j);
				if (!gm) break;
				on = 0;
				if (j && !strcmp(gm->file, td->pict_file)) on = 1;
				if (!j && use_graphics == 0) on = 1;
				if (j == use_graphics) on = 1;
				strnfmt(buf, 32, " [%c] %-12s ", (on ? 'X' : '.'),
				j ? gm->menuname : "Off");
				renderTButton(td, sx, sy + j, buf, MENU_QUICK_GRAF0 + j);
				sub_w = MATH_MAX(sub_w, strlen(gm->menuname) + 6);
			}
			td->menu_rect.h += j * grid_h;
			sub_x = sx;
		}
		cx += strlen("Graphics") + 1;
	}

	// Sound
	if (td->id == TERM_MAIN)
	{
		renderTButton(td, cx, y, "Sound", MENU_SUB_SOUND);
		if (menu_open == MENU_SUB_SOUND && menu_open_term == td->id) {
			int sx = cx - 1;
			int sy = y + 1;
			int si = 0;
			char buf[128];
			strnfmt(buf, 16, " [%c] %-12s ", use_sound ? 'X' : '.', "On");
			renderTButton(td, sx, sy + 0, buf, MENU_ACT_SOUND_ON);
			strnfmt(buf, 16, " [%c] %-12s ", !use_sound ? 'X' : '.', "Off");
			renderTButton(td, sx, sy + 1, buf, MENU_ACT_SOUND_OFF);
			td->menu_rect.h += 2 * grid_h;
			sub_w = 15;
			sub_x = sx;
		}
		cx += strlen("Sound") + 1;

	}

	// Windows
	if (td->id == TERM_MAIN)
	{
		renderTButton(td, cx, y, "Windows", MENU_SUB_WINDOWS);
		if (menu_open == MENU_SUB_WINDOWS && menu_open_term == td->id) {
			int sx = cx - 1;
			int sy = y + 1;
			int si = 0;
			int j;
			for (j = 0; j < TERM_MAX; j++) {
				char buf[16];
				strnfmt(buf, 16, " [%c] %-12s ",
				(terms[j].config & TERM_IS_ONLINE) ? 'X' : '.',
				ang_term_name[j]);
				renderTButton(td, sx, sy + j, buf, MENU_SHOW_WINDOW0 + j);
			}
			td->menu_rect.h += j * grid_h;
			sub_w = 15;
			sub_x = sx;
		}
		cx += strlen("Windows") + 1;
	}

	/* Right-hand buttons */
	if (td->id == TERM_MAIN) {
		cx = td->cols - 3;
	} else {
		cx = td->cols - 7;
	}

	// Toggle virtual
	if (td->id != 0)
	{
		renderTButton(td, cx, y, "[^]", MENU_ACT_TOGGLE_VIRTUAL);
		cx += strlen("[^]") + 1;
	}

	// Close
	{
		renderTButton(td, cx, y, "[X]", MENU_ACT_CLOSE);
		cx += strlen("[X]") + 1;
	}

	// draw menu bar border
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BORDER));
	SDL_RenderDrawRect(td->renderer, &rect);

	// draw submenu border
	rect.x = sub_x * grid_w + td->ren_rect.x;
	rect.y = 1 * grid_h + td->ren_rect.y - 1 - growth;
	rect.w = sub_w * grid_w;
	rect.h = td->menu_rect.h - 1 * grid_h + 2;
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BORDER));
	SDL_RenderDrawRect(td->renderer, &rect);

	// draw grip
	rect.x = width - grid_w * 1 + ox;
	rect.y = height - grid_h * 1 + oy + growth;
	rect.w = grid_w * 1;
	rect.h = grid_h * 1;
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BACKGROUND));
	SDL_RenderFillRect(td->renderer, &rect);
	td->menu_rect.y += growth;
	renderTButton(td, td->cols-1, td->rows-1, "\\", MENU_NO_ACTION);
	td->menu_rect.y -= growth;
	SDL_SetRenderDrawColor(td->renderer, GUI_COLOR4(MENU_BORDER));
	SDL_RenderDrawRect(td->renderer, &rect);
	td->grip_rect = rect;

	return 0;
}

static int guiTermMatch(int window_id, int x, int y) {
	int i, j;
	TermData *td;
	SDL_Rect *r;
	for (j = 0; j < TERM_MAX; j++) {
		i = 7 - j;
		td = &terms[i];
		if (!(terms[i].config & TERM_IS_ONLINE)) continue;
		if (terms[i].window_id != window_id) continue;

		r = &td->menu_rect;
		if (x >= r->x && x <= r->x + r->w
		&&  y >= r->y && y <= r->y + r->h) {
			return i;
		}
	}
	return -1;
}

static int guiGripMatch(int window_id, int x, int y) {
	int i, j;
	TermData *td;
	SDL_Rect *r;
	for (j = 0; j < TERM_MAX; j++) {
		i = 7 - j;
		td = &terms[i];
		if (!(terms[i].config & TERM_IS_ONLINE)) continue;
		if (terms[i].window_id != window_id) continue;

		r = &td->grip_rect;
		if (x >= r->x && x <= r->x + r->w
		&&  y >= r->y && y <= r->y + r->h) {
			return i;
		}
	}
	return -1;
}

static int guiDragStart(int i, int x, int y)
{
	TermData *td;
	td = &terms[i];

	drag_x = x;
	drag_y = y;

	dragging = i;

	if (gripping) {
		td->resize_rect.x = 0;
		td->resize_rect.y = 0;
		td->resize_rect.w = td->ren_rect.w;
		td->resize_rect.h = td->ren_rect.h;
	}

	return 1;
}

static int guiDragMotion(int mx, int my) {
	TermData *td;

	if (dragging < 0) return 0;

	td = &terms[dragging];

	if (gripping) {
		int w, h, cw, ch;
		byte cw_, ch_;
		int i = dragging;

		td->resize_rect.w = td->ren_rect.w + (accum_x - drag_x);
		td->resize_rect.h = td->ren_rect.h + (accum_y - drag_y);

		// resize to nearest whole cell
		w = td->resize_rect.w;
		h = td->resize_rect.h;
		cw = w/terms[i].cell_w;
		ch = h/terms[i].cell_h;

		cw_ = cw; ch_ = ch;
		net_term_clamp((byte)i, &ch_, &cw_);
		cw = cw_; ch = ch_;

		td->resize_rect.w = cw_ * terms[i].cell_w;
		td->resize_rect.h = ch_ * terms[i].cell_h;

		td->need_render = TRUE;

		return 1;
	}

	td->ren_rect.x = td->ren_rect.x + mx;
	td->ren_rect.y = td->ren_rect.y + my;

	if (mx || my) td->need_render = TRUE;

	termConstrain(td->id);
	return 1;
}

static bool guiDragStop() {
	if (dragging <= -1) {
		return 0;
	}
	if (gripping) {
		TermData *td = &terms[dragging];
		int i = dragging;

		// resize to nearest whole cell
		int w = td->resize_rect.w;
		int h = td->resize_rect.h;
		int cw = w/terms[i].cell_w;
		int ch = h/terms[i].cell_h;

		byte cw_ = cw, ch_ = ch;
		net_term_clamp((byte)i, &ch_, &cw_);
		cw = cw_; ch = ch_;

		resizeTerm(&terms[i], cw, ch);
		refreshTerm(&terms[i]);
		terms[i].need_redraw = TRUE;
	}
	gripping = 0;
	dragging = -1;
	return 1;
}

static int guiMenuReact(int window_id, int x, int y) {
	int i;
	i = guiTermMatch(window_id, x, y);
	if (i >= 0)
	{
		menu_term = i;
		//menu_hover = -1;
		menu_x = x;
		menu_y = y;
		terms[i].need_render = TRUE;
	} else {
		menu_term = -1;
	}
	return i;
}

static void guiMenuOff(void) {
	menu_open = -1;
	menu_open_term = -1;
	menu_hover = -1;
}


/* Populate SDL_Rect "screen" with term<id> PARENT dimensions */
static void termSizeScreen(SDL_Rect *screen, int term_id)
{
	TermData *td = &terms[term_id];

	/* Main TERM and NON-Virtual terms live inside WM screen */
	if (!(td->config & TERM_IS_VIRTUAL) || td->id == TERM_MAIN)
	{
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		screen->x = 0;
		screen->y = 0;
		screen->w = DM.w;
		screen->h = DM.h;
	}
	/* Virtual terms live inside parent non-virtual windows */
	else
	{
		SDL_GetWindowPosition(td->window, &screen->x, &screen->y);
		getRendererSize(td, &screen->w, &screen->h);
	}
}

/* Populate SDL_Rect "r" with term<id> dimenstions */
static void termSizeRect(SDL_Rect *r, int term_id)
{
	TermData *td = &terms[term_id];
	if (td->config & TERM_IS_VIRTUAL)
	{
		r->x = td->ren_rect.x;
		r->y = td->ren_rect.y;
		r->w = td->ren_rect.w;
		r->h = td->ren_rect.h;
	}
	else
	{
		r->x = td->x;
		r->y = td->y;
		r->w = td->width;
		r->h = td->height;
	}
}

/*
 * This function finds whether 2 SDL_Rects overlap.
 */
static bool rectsCollide(SDL_Rect *a, SDL_Rect *b) {
	if (a->y + a->h <= b->y) return(FALSE);
	if (b->y + b->h <= a->y) return(FALSE);
	if (a->x + a->w <= b->x) return(FALSE);
	if (b->x + b->w <= a->x) return(FALSE);
	return(TRUE);
}

static void termStack(int i) {
	TermData *td = &terms[i];
	SDL_Rect pos;
	SDL_Rect other;
	SDL_Rect screen;
	int j;
	bool found = FALSE;

	if (i == TERM_MAIN) found = TRUE;

	termSizeRect(&pos, i);
	termSizeScreen(&screen, i);

	/* Start at top-left */
	pos.x = 0;
	pos.y = 0;

	/* Hack - allow extra pixel for those 2 terms.
	 * This way, default fonts + default terms will fit on 1280 screen */
	if (td->id == TERM_CHOICE || td->id == TERM_RECALL)
	{
		pos.w -= 1;
	}

	while (!found) {
		/* Iterate throu all previous terminals to find non-occupied space */
		for (j = 0; j < TERM_MAX; j++) {
			found = TRUE;

			if (i == j) continue; /* Skip self */

			if (terms[j].config & TERM_IS_HIDDEN) continue; /* Skip offline terminals */

			if ((terms[j].config & TERM_IS_VIRTUAL) != (td->config & TERM_IS_VIRTUAL))
				continue; /* Do not collide virtual/non-virtual terms */

			termSizeRect(&other, j);


			/* Collision! */
			if (rectsCollide(&pos, &other))
			{
				/* Move right if there is still space */
				if (pos.x + pos.w + other.w < screen.w) {
					pos.x = pos.x + other.w;
					found = FALSE;
					break;
				}
				/* Perform a carriage return */
				if (pos.y + 1 + other.h < screen.h) {
					pos.y = pos.y + 1;
					pos.x = 0;
					found = FALSE;
					break;
				}
			}
		}
	}
	if (td->config & TERM_IS_VIRTUAL) {
		td->ren_rect.x = pos.x;
		td->ren_rect.y = pos.y;
	} else {
		td->x = pos.x;
		td->y = pos.y;
		SDL_SetWindowPosition(td->window, pos.x, pos.y);
	}
}

static void termConstrain(int i) {
	SDL_Rect screen;
	SDL_Rect t;
	TermData *td = &terms[i];
	int y_offset = 0;
	if (!(td->config & TERM_IS_ONLINE)) return;

	termSizeScreen(&screen, i);
	termSizeRect(&t, i);

	/* XXX HACK XXX Adjust window size for external WM titlebar */
	if (!(td->config & TERM_IS_VIRTUAL)) y_offset = WM_TITLEBAR_OFFSET;

	if (t.x < 0) t.x = 0;
	if (t.y < 0) t.y = 0; /* then right */
	if (t.x + t.w > screen.w) t.x = screen.w - t.w;
	if (t.y + t.h > screen.h - y_offset) t.y = screen.h - y_offset - t.h;
	if (t.x < 0) t.x = 0; /* left again */
	if (t.y < - y_offset) t.y = - y_offset;

	if (td->config & TERM_IS_VIRTUAL) {
		td->ren_rect.x = t.x;
		td->ren_rect.y = t.y;
	} else {
		td->x = t.x;
		td->y = t.y;
		SDL_SetWindowPosition(td->window, t.x, t.y);
	}
}

static void termClose(int i) {
	terms[i].config |= TERM_IS_HIDDEN;
	/* Hack -- if z-term active_flag is not set,
	 * the nuke hook won't be called... */
	terms[i].t.active_flag = TRUE;
	term_nuke(&(terms[i].t));
	ang_term[i] = NULL;
}

static void termSpawn(int i) {
	terms[i].config &= ~TERM_IS_HIDDEN;
	initTermData(&terms[i], ang_term_name[i], i, NULL);
	applyTermConf(&terms[i]);
	setTermTitle(&terms[i]);
	refreshTerm(&terms[i]);
	ang_term[i] = &(terms[i].t);
}


/* Needed for HIGHDPI/Retina displays */
static void normalizeMouseCoordinates(int i, int *x, int *y, int sx, int sy)
{
	int winW, winH, highW, highH;
	float scaleX, scaleY;

	SDL_GetWindowSize(terms[i].window, &winW, &winH);
	getRendererSize(&terms[i], &highW, &highH);

	scaleX = (float)highW / winW;
	scaleY = (float)highH / winH;

	*x = sx * scaleX;
	*y = sy * scaleY;
}

/* Track state of modifier keys (to use with mouse clicks) */
static int Noticemodkeypress(int sym, int b_pressed)
{
	static int pressed[3] = { 0 };
	int r = -1;
	switch (sym)
	{
		case SDLK_RCTRL:  case SDLK_LCTRL:  r = 0; break;
		case SDLK_RALT:   case SDLK_LALT:   r = 1; break;
		case SDLK_RSHIFT: case SDLK_LSHIFT: r = 2; break;
		default: return 0; break;
	}
	if (b_pressed == 0 || b_pressed == 1) pressed[r] = b_pressed;
	return pressed[r];
}

static void mobilePinch(int i, int dir)
{
#if 0 /* This doesn't work well :( */
	int zoom = terms[i].zoom;

	if (dir >= 1)
	{
		zoom *= 2;
		if (zoom > 200) zoom = 200;
	}
	else if (dir <= -1)
	{
		zoom /= 2;
		if (zoom < 50) zoom = 50;
	}

	if (terms[i].zoom == zoom) return;

	terms[i].zoom = zoom;

	applyTermConf(&terms[i]);
	refreshTerm(&terms[i]);
	termConstrain(i);
#endif
}

static void Term_multikeypress(char *buf)
{
	char *c = buf;
	while (*c) Term_keypress(*c++);
}

static void altCoord(int wx, int wy, int *x, int *y)
{
	int i = 0;
	int cx, cy;

	int acw = terms[i].pict_data ? terms[i].pict_data->w : terms[i].cell_w;
	int ach = terms[i].pict_data ? terms[i].pict_data->h : terms[i].cell_h;

	acw = acw * terms[i].zoom / 100;
	ach = ach * terms[i].zoom / 100;

	cx = (wx - terms[i].ren_rect.x - terms[i].dng_rect.x) /
	    (acw);
	cy = (wy - terms[i].ren_rect.y - terms[i].dng_rect.y) /
	    (ach);

	*x = cx;
	*y = cy;
}

static void handleMouseClick(int i, int wx, int wy, int sdlbutton)
{
	/* Hack -- user doesn't want to control the game with mouse */
	if (!sdl_game_mouse) return;

	if (i == TERM_MAIN)
	{
		int button = 0;
		int cx = (wx - terms[i].ren_rect.x) / terms[i].cell_w;
		int cy = (wy - terms[i].ren_rect.y) / terms[i].cell_h;
		if ((terms[i].config & TERM_DO_SCALE) && !screen_icky)
		{
			if (wx >= terms[i].ren_rect.x + terms[i].dng_rect.x
			 && wy >= terms[i].ren_rect.y + terms[i].dng_rect.y
			 && wx <= terms[i].ren_rect.x + terms[i].dng_rect.x + terms[i].dng_rect.w
			 && wy <= terms[i].ren_rect.y + terms[i].dng_rect.y + terms[i].dng_rect.h)
			{
				altCoord(wx, wy, &cx, &cy);
				cx += DUNGEON_OFFSET_X;
				cy += DUNGEON_OFFSET_Y;
			}
		}
		/* Indexes */
		if (sdlbutton == SDL_BUTTON_LEFT) button = 1;
		else if (sdlbutton == SDL_BUTTON_MIDDLE) button = 2;
		else if (sdlbutton == SDL_BUTTON_RIGHT) button = 3;
		else if (sdlbutton == SDL_BUTTON_X1) button = 4;
		else if (sdlbutton == SDL_BUTTON_X2) button = 5;

		/* Convention picked from main-win.c of V341 */
		if (Noticemodkeypress(SDLK_LCTRL, -1))  button |= 16;
		if (Noticemodkeypress(SDLK_LSHIFT, -1)) button |= 32;
		if (Noticemodkeypress(SDLK_LALT, -1))   button |= 64;

		/* Pass to Angband */
		Term_mousepress(cx, cy, button);
	}
}

static int matchWindow(int window_id)
{
	int i;
	for (i = 0; i < TERM_MAX; i++)
	{
		if (terms[i].window_id == window_id)
		{
			return i;		
		}
	}
	return -1;
}
static void handleWMEvent(SDL_Event *ev)
{
	int i;
	if (ev->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
	{
		i = matchWindow(ev->window.windowID);
		if (i >= 0)
		{
			//not a good idea
			//Term_activate(&(terms[i].t));
		}
	}
	else if (ev->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		i = matchWindow(ev->window.windowID);
		if (i >= 0)
		{
			// resize to nearest whole cell
			int w = ev->window.data1;
			int h = ev->window.data2;
			int cw = w/terms[i].cell_w;
			int ch = h/terms[i].cell_h;

			if (i == TERM_MAIN)
			{
				normalizeMouseCoordinates(0, &w, &h, w, h);
				resizeWindow(&terms[i], w, h);
			}
			else
			{
				byte cw_, ch_;
				cw_ = cw; ch_ = ch;
				net_term_clamp((byte)i, &ch_, &cw_);
				cw = cw_; ch = ch_;

				resizeTerm(&terms[i], cw, ch);
				refreshTerm(&terms[i]);
				//refreshTermAlt(&terms[i]);
			}
		}
	}
	else if (ev->window.event == SDL_WINDOWEVENT_MOVED)
	{
		i = matchWindow(ev->window.windowID);
		if (i >= 0)
		{
			int x = ev->window.data1;
			int y = ev->window.data2;

			terms[i].x = x;
			terms[i].y = y;
		}
	}
	else if (ev->window.event == SDL_WINDOWEVENT_EXPOSED)
	{
		for (i = 0; i < TERM_MAX; i++)
		{
			if (terms[i].window_id == ev->window.windowID)
			{
				terms[i].need_redraw = TRUE;
			}
		}
	}
	else if (ev->window.event == SDL_WINDOWEVENT_CLOSE)
	{
		for (i = 0; i < TERM_MAX; i++)
		{
			if (terms[i].window_id == ev->window.windowID)
			{
				if (i == TERM_MAIN)
				{
					quit("Sayonara!");
					return;
				}
				termClose(i);
			}
		}
	}
}

static void eventMouseCoordinates(SDL_Event *ev, int *x, int *y)
{
	if (ev->type == SDL_MOUSEMOTION)
	{
		normalizeMouseCoordinates(0, x, y, ev->motion.x, ev->motion.y);
	}
	if (ev->type == SDL_MOUSEBUTTONDOWN || ev->type == SDL_MOUSEBUTTONUP)
	{
		normalizeMouseCoordinates(0, x, y, ev->button.x, ev->button.y);
	}
}

static bool handleMouseEvent_Menu(SDL_Event *ev)
{
	int i;
	int wx, wy;

	eventMouseCoordinates(ev, &wx, &wy);

	if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.button == SDL_BUTTON_LEFT)
	{
		/* Start dragging? */
		i = guiTermMatch(ev->window.windowID, wx, wy);
		if (i >= 0 && terms[i].config & TERM_IS_VIRTUAL)
		{
			/* Matched virtual title bar */
			if (menu_hover <= 0)
			{
				guiDragStart(i, wx, wy);
			}
		}
		else
		{
			/* Start resizing ? */
			i = guiGripMatch(ev->window.windowID, wx, wy);
			if (i >= 0)
			{
				gripping = 1;
				guiDragStart(i, wx, wy);
			}
		}
		return TRUE;
	}
	else if (ev->type == SDL_MOUSEMOTION)
	{
		if (dragging > -1 && terms[dragging].window_id == ev->window.windowID)
		{
			accum_x = ev->motion.x;
			accum_y = ev->motion.y;
			guiDragMotion(ev->motion.xrel, ev->motion.yrel);
			return TRUE;
		}

		guiMenuReact(ev->window.windowID, wx, wy);
		return TRUE;
	}
	else if (ev->type == SDL_MOUSEBUTTONUP && ev->button.button == SDL_BUTTON_LEFT)
	{
		if (dragging > -1)
		{
			guiDragStop();
			return TRUE;
		}
		i = matchWindow(ev->window.windowID);
		if (i >= 0)
		{
			int cx = wx/(terms[i].cell_w ? terms[i].cell_w : 1);
			int cy = wy/(terms[i].cell_h ? terms[i].cell_h : 1);

			handleMenu(menu_term, cx, cy);
			return TRUE;
		}
	}
	return FALSE;
}

static void handleMouseEvent(SDL_Event *ev)
{
	int i;
	int wx, wy;

	int icon_drag_threshold = MICON_W / 2;
#ifdef MOBILE_UI
	/* Easier for fat fingers */
	icon_drag_threshold *= 2;
#endif
	if (menu_mode)
	{
		bool handled = handleMouseEvent_Menu(ev);
		if (handled) return;
	}

	eventMouseCoordinates(ev, &wx, &wy);

	if (ev->type == SDL_MOUSEBUTTONDOWN)
	{
		if (ev->button.button == SDL_BUTTON_LEFT)
		{
#ifdef USE_ICON_OVERLAY
			//icon_hover =
			icon_pressed = matchIcon(wx, wy);
			icon_pressed_x = wx;
			icon_pressed_y = wy;
#endif
#if defined(USE_LISENGLAS) && defined(USE_ICON_OVERLAY)
			if (icon_hover < 0)
				lisen_pressed = TRUE;
#endif
		}
	}
	else if (ev->type == SDL_MOUSEMOTION)
	{
#ifdef USE_ICON_OVERLAY
		if (icon_pressed > -1 && icon_pressed < num_icons
		&& icon_hover == icon_pressed)
		{
			if (!dragging_icon)
			{
				if ((ABS(icon_pressed_x - wx) > icon_drag_threshold)
				|| (ABS(icon_pressed_y - wy) > icon_drag_threshold))
				{
					dragging_icon = TRUE;
					drag_icon = icons[icon_pressed];
					icon_pressed = -1;
					terms[0].need_render = TRUE;
					return;
				}
			}
		}
		if (dragging_icon)
		{
			drag_icon.pos.x = wx - drag_icon.pos.w/2;
			drag_icon.pos.y = wy - drag_icon.pos.h/2;
			terms[0].need_render = TRUE;
		}

		icon_hover = matchIcon(wx, wy);
#endif
#ifdef USE_LISENGLAS
		if (lisen_pressed)
		{
			updateLisenglas(wx, wy);
		}
#endif

		/* Still here? Good, let's pass this event to Angband */
		{
			for (i = 0; i < TERM_MAX; i++)
			{
				if (terms[i].window_id == ev->window.windowID)
				{
					int w = MAX(terms[i].fb_w, terms[i].dng_rect.x + terms[i].alt_fb_w);
					int h = MAX(terms[i].fb_w, terms[i].dng_rect.y + terms[i].alt_fb_h);
					if (wx >= terms[i].ren_rect.x && wy >= terms[i].ren_rect.y
					 && wx <= terms[i].ren_rect.y + w
					 && wy <= terms[i].ren_rect.y + h)
					{
						handleMouseClick(i, wx, wy, 0);
						break;
					}
				}
			}
		}
	}
	else if (ev->type == SDL_MOUSEBUTTONUP)
	{
		if (ev->button.button == SDL_BUTTON_RIGHT)
		{
			if (menu_mode)
			{
				menu_mode = 0;
				guiDragStop();
				guiMenuOff();
			}
			else
			{
				menu_mode = 1;
				menu_term = -1;
				guiMenuReact(ev->window.windowID, wx, wy);
			}
			mustRerender();
			return;
		}
		if (ev->button.button == SDL_BUTTON_LEFT)
		{
#ifdef USE_LISENGLAS
			lisen_pressed = FALSE;
			lisen_viable = FALSE;
#endif
#ifdef USE_ICON_OVERLAY
			icon_pressed = -1;
			if (dragging_icon)
			{
				/* Only allow drop onto quickslots */
				if (icon_hover > -1 && icon_hover < num_icons
				&& icons[icon_hover].action == MICON_LOCAL_QUICK_SLOT)
				{
					handleMIcon(icons[icon_hover].action, icons[icon_hover].sub_action);
				}
				dragging_icon = FALSE;
				return;
			}
#ifdef MOBILE_UI
			/* On mobile, it's possible to press without hovering first,
			 * so we shall re-evalute the button we're over */
			icon_hover = matchIcon(wx, wy);
#endif
			if (icon_hover >= 0)
			{
				handleMIcon(icons[icon_hover].action, icons[icon_hover].sub_action);
			 	return;
			}
#endif
		}
		/* Still here? Good, let's pass this event to Angband */
		{
			for (i = 0; i < TERM_MAX; i++)
			{
				if (terms[i].window_id == ev->window.windowID)
				{
					int w = MAX(terms[i].fb_w, terms[i].dng_rect.x + terms[i].alt_fb_w);
					int h = MAX(terms[i].fb_w, terms[i].dng_rect.y + terms[i].alt_fb_h);
					if (wx >= terms[i].ren_rect.x && wy >= terms[i].ren_rect.y
					 && wx <= terms[i].ren_rect.y + w
					 && wy <= terms[i].ren_rect.y + h)
					{
						handleMouseClick(i, wx, wy, ev->button.button);
						break;
					}
				}
			}
		}
	}
}

/* Notification: */
static bool warn_wrong_layout = FALSE;

/* When keyboard layout is ignored, we do this: */
static int hackyShift(int key)
{
	if (key >= 'a' && key <= 'z') return key - 32;
	else if (key == '1') return '!';
	else if (key == '2') return '@';
	else if (key == '3') return '#';
	else if (key == '4') return '$';
	else if (key == '5') return '%';
	else if (key == '6') return '^';
	else if (key == '7') return '&';
	else if (key == '8') return '*';
	else if (key == '9') return '(';
	else if (key == '0') return '*';
	else if (key == '`') return '~';
	else if (key == '-') return '_';
	else if (key == '=') return '+';
	else if (key == '[') return '{';
	else if (key == ']') return '}';
	else if (key == ';') return ':';
	else if (key == '\'') return '"';
	else if (key == '\\') return '|';
	else if (key == '.') return '<';
	else if (key == ',') return '>';
	else if (key == '/') return '?';
	return key;
}

/* ==== Arrow Key Combiner ==== */
static SDL_Event prev_arrow = {0};
static Uint32 prev_arrow_timestamp = 0;
static int has_prev_arrow = FALSE;
static int combiner_recurse = 1;

static int isArrowKey(int key)
{
	return (key == SDLK_UP  || key == SDLK_DOWN
		|| key == SDLK_LEFT || key == SDLK_RIGHT);
}
static int isArrowCombinable(int key1, int key2)
{
	switch (key1)
	{
		case SDLK_UP:
		case SDLK_DOWN:
			return (key2 == SDLK_LEFT || key2 == SDLK_RIGHT);
		case SDLK_LEFT:
		case SDLK_RIGHT:
			return (key2 == SDLK_UP || key2 == SDLK_DOWN);
		default:
			break;
	}
	return 0;
}
static SDL_Keycode CombinedMovement(SDL_Keycode a, SDL_Keycode b)
{
	const SDL_Keycode keys9x9[4][4] = {
		{ SDLK_HOME, SDLK_UP, SDLK_PAGEUP },
		{ SDLK_LEFT, 0, SDLK_RIGHT },
		{ SDLK_END, SDLK_DOWN, SDLK_PAGEDOWN },
	};
	int _dx = 0, _dy = 0;
	if (a == SDLK_UP || b == SDLK_UP) _dy = -1;
	else if (a == SDLK_DOWN || b == SDLK_DOWN) _dy = 1;
	if (a == SDLK_LEFT || b == SDLK_LEFT) _dx = -1;
	else if (a == SDLK_RIGHT || b == SDLK_RIGHT) _dx = 1;
	if (_dx && _dy)
	{
		return keys9x9[_dy + 1][_dx + 1];
	}
	return 0;
}
static void handleKeyboardEvent(SDL_Event *ev);
static void StoreDelayedArrow(SDL_Event *ev)
{
	prev_arrow = *ev;
	prev_arrow_timestamp = SDL_GetTicks();
	has_prev_arrow = TRUE;
}
static void DropDelayedArrow()
{
	has_prev_arrow = FALSE;
}
static int isDelayedArrowRecent()
{
	if (has_prev_arrow) 
	{
		Uint32 diff = SDL_GetTicks() - prev_arrow_timestamp;
		if (diff < sdl_combiner_delay) return TRUE;
	}
	return FALSE;
}
static void PlayDelayedArrow()
{
	if (has_prev_arrow)
	{
		combiner_recurse = 0;
		handleKeyboardEvent(&prev_arrow);
		combiner_recurse = 1;
	}
}
static void TimeoutDelayedArrow()
{
	if (has_prev_arrow && !isDelayedArrowRecent())
	{
		PlayDelayedArrow();
		DropDelayedArrow();
	}
}
/* Main function responsible for combining arrow keys.
 * It's a bit verbose, and has a measurable amount of code duplication,
 * but it's easier to debug specific cases, by following specific branches this way.
 */
static int NoticeArrowKey(int key, SDL_Event *ev, int push)
{
	/* Disabled completely */
	if (!sdl_combine_arrowkeys) return ev->key.keysym.sym;
	/* Disabled temporarily */
	if (!combiner_recurse) return ev->key.keysym.sym;

	/* If we already have a delayed arrow key, then... */
	if (has_prev_arrow)
	{
		/* If new key is not an arrow, release */
		if (!isArrowKey(key))
		{
			PlayDelayedArrow();
			DropDelayedArrow();
		}
		else
		{
			/* If new key IS an arrow, let's examine the situation */
			int prev_key = prev_arrow.key.keysym.sym;
			/* If it's the same key */
			if (prev_key == key)
			{
				/* Which has been released */
				if (push == 0)
				{
					/* Play it, once */
					PlayDelayedArrow();
					DropDelayedArrow();
					return 0;
				}
				/* Which has been pressed again */
				else
				{
					/* Do nothing */
				}
			}
			/* If it's a different key */
			else
			{
				if (push == 0)
				{
					/* Do nothing */
				}
				else
				{
					/* Can we actually combine the two? */
					if (isArrowCombinable(prev_key, key) && isDelayedArrowRecent())
					{
						/* Let's combine! */
						ev->key.keysym.sym = CombinedMovement(prev_key, key);
						DropDelayedArrow();
					}
					/* We can't, flush the old one */
					else
					{
						PlayDelayedArrow();
						DropDelayedArrow();
						/* But keep the new one for later */
						StoreDelayedArrow(ev);
						return 0;
					}
				}
			}
		}
	}
	/* If we don't have a delayed arrow key, let's take this one */
	else if (isArrowKey(key) && push)
	{
		StoreDelayedArrow(ev);
		return 0;
	}

	return ev->key.keysym.sym;
}

static void handleKeyboardEvent(SDL_Event *ev)
{
	/* Handle text input */
	if (ev->type == SDL_TEXTINPUT)
	{
		/* This is a utf-8 string */
		char *c = ev->text.text;
		/* Hack -- if we ignore_keyboard_layout, don't use textinput */	
		if (ignore_keyboard_layout) *c = 0;
		while (*c)
		{
			int key = *c;
			/* We only want ascii input */
			if (*c & 0x80)
			{
				/* Some foreign keyboard layout, warn user! */
				warn_wrong_layout = TRUE;
				return;
			}
			/* Send as-is */
			Term_keypress(key);
			c++;
		}
		return;
	}

	/* Hack -- "Back" button */
	if (ev->key.keysym.scancode == SDL_SCANCODE_AC_BACK)
	{
#ifdef USE_ICON_OVERLAY
		handleMIcon(MICON_ESCAPE, 0);
#else
		Term_keypress(ESCAPE);
#endif
		return;
	}

	/* Key released */
	if (ev->type == SDL_KEYUP && ev->key.state == SDL_RELEASED)
	{
		int key = ev->key.keysym.sym;
		/* Track modifier keypresses, for mouse sake */
		if (key >= SDLK_LCTRL && key <= SDLK_RGUI)
		{
			Noticemodkeypress(key, 0);
			return;
		}
		/* Hack -- combine arrow keys */
		NoticeArrowKey(key, ev, 0);
	}

	/* Key pressed */
	if (ev->type == SDL_KEYDOWN && ev->key.state == SDL_PRESSED)
	{
		int key = ev->key.keysym.sym;
		int mod = ev->key.keysym.mod & 0x00000FFF;
		bool sent = FALSE;
		bool shift_only = ((mod & KMOD_SHIFT) && ((mod & ~KMOD_SHIFT) == 0));
		bool ctrl_only = ((mod & KMOD_CTRL) && ((mod & ~KMOD_CTRL) == 0));

		/* Track modifier keypresses, for mouse sake */
		if (key >= SDLK_LCTRL && key <= SDLK_RGUI)
		{
			Noticemodkeypress(key, 1);
			return;
		}

		/* Hack -- combine arrow keys */
		if ((key = NoticeArrowKey(key, ev, 1)) == 0) return;

		/* ASCII */
		if (key <= 127)
		{
			/* Space is already handled by text input, ignore */
			if (!ignore_keyboard_layout && key == ' ' && mod == 0) return;

			/* Printable charater */
			if (isgraph(key))
			{
				/* No modifiers */
				if (mod == 0)
				{
					/* Already handled by text input, ignore */
					if (!ignore_keyboard_layout) return;

					/* Send as-is */
					Term_keypress(key);
					sent = TRUE;
				}
				/* Only Shift modifier */
				else if (shift_only)
				{
					/* Already handled by text input, ignore */
					if (!ignore_keyboard_layout) return;

					/* Send uppercase */
					Term_keypress(hackyShift(key));
					sent = TRUE;
				}
				/* Only Ctrl modifier */
				else if (ctrl_only && isalpha(key))
				{
					/* Send ctrlcase */
					Term_keypress(KTRL(key));
					sent = TRUE;
				}
			}
			/* Control character with no modifiers */
			if ((iscntrl(key) || isspace(key)) && mod == 0)
			{
				/* Send as-is */
				Term_keypress(key);
				sent = TRUE;
			}
		}
		/* Send as macro */
		if (!sent)
		{
			char buf[32];

			/* Remove 0x4000000, which SDL assigns to all non-ascii keys */
			key &= 0x0000FFFF;

			/* Hack -- allow NumPad to be macroed separately */
			if ((ev->key.keysym.mod & KMOD_NUM) 
			&& ev->key.keysym.sym >= SDLK_KP_DIVIDE
			&& ev->key.keysym.sym <= SDLK_KP_PERIOD
			&& collapse_numpad_keys == FALSE)
			{
				/* Largest real SDL_Keysym is 0x11A, so this is "safe" */
				key |= 0xF00;
			}
				
			/* Format macro */
			strnfmt(buf, 32, "%c%s%s%s%s_%lX%c", 31,
				    ev->key.keysym.mod & KMOD_CTRL  ? "N" : "",
				    ev->key.keysym.mod & KMOD_SHIFT ? "S" : "",
				    ev->key.keysym.mod & KMOD_ALT   ? "O" : "",
				    ev->key.keysym.mod & KMOD_GUI   ? "M" : "",
				    (unsigned long) key, 13);
#ifdef DEBUG
			printf("Macro: ^_%s%s%s%s_%lX\\r\n",
				    ev->key.keysym.mod & KMOD_CTRL  ? "N" : "",
				    ev->key.keysym.mod & KMOD_SHIFT ? "S" : "",
				    ev->key.keysym.mod & KMOD_ALT   ? "O" : "",
				    ev->key.keysym.mod & KMOD_GUI   ? "M" : "",
				    (unsigned long) key);
#endif
			Term_multikeypress(buf);
			sent = TRUE;
		}
		if (!sent)
		{
			printf("Unhandled key %08x mod %08x\n", ev->key.keysym.sym, ev->key.keysym.mod);
		}
	}
}

static errr xtraTermHook(int n, int v) {
	term *old_td;
	TermData *td = (TermData*)(Term->data);
	SDL_Event event;
	int i;
	switch (n) {
	case TERM_XTRA_NOISE: // generic noise
		sdl_bell();
		return 0;
	case TERM_XTRA_SOUND:
#ifdef USE_SOUND
		/* Make a sound */
		i = sound_count(v);
		if (i) sdl_play_sound(v, randint0(i));
#endif
		return 0;
	case TERM_XTRA_BORED:
		rerender();
		return 0; // send "0" to Event processing
	case TERM_XTRA_LEVEL: // v of 0 = deactivate, v of 1 = activate
		return 0;
	case TERM_XTRA_EVENT:
	do {

		if (v)
		{
			if (!SDL_WaitEvent(&event)) return(0);
			v = 0;
		} else {
			if (!SDL_PollEvent(&event)) return(0);
		}
		if (event.type == SDL_QUIT)
		{
			quit("Goodbye.");
		}
		else if (event.type == SDL_KEYDOWN
				|| event.type == SDL_KEYUP
				|| event.type == SDL_TEXTINPUT)
		{
			handleKeyboardEvent(&event);
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			handleWMEvent(&event);
		}

		/* Gestures */
		else if (event.type == SDL_MULTIGESTURE)
		{
			accum_pinch += event.mgesture.dDist;
		}

		/* Touch events! */
		else if (event.type == SDL_FINGERDOWN)
		{

		}
		else if (event.type == SDL_FINGERMOTION)
		{

		}
		else if (event.type == SDL_FINGERUP)
		{
			if (SDL_GetNumTouchFingers(event.tfinger.touchId) == 0)
			{
				mobilePinch(0, (int)(accum_pinch*100)); accum_pinch = 0;
			}
		}

		/* Mouse events */
		else if (event.type == SDL_MOUSEBUTTONDOWN
				|| event.type == SDL_MOUSEMOTION
				|| event.type == SDL_MOUSEBUTTONUP)
		{
			handleMouseEvent(&event);
		}

	} while (SDL_PollEvent(NULL));
		/* Hack -- flush delayed arrow */
		TimeoutDelayedArrow();
	return 0; // okay, I guess?
	case TERM_XTRA_FLUSH:
		while (SDL_PollEvent(&event));
		return 0; // force a redraw?
	case TERM_XTRA_FRESH:
#ifdef USE_SOUND
		/* HACK !!! -- Terminate current sound if necessary */
		if (use_sound) sdl_play_sound_end(TRUE);
#endif
		td->need_render = TRUE;
		return 0;
	case TERM_XTRA_CLEAR:
		SDL_SetRenderTarget(td->renderer, td->framebuffer);
		SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255);
		SDL_RenderClear(td->renderer);
		SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
		return 0; // clear?
	case TERM_XTRA_REACT:
#ifdef MOBILE_UI
		if (v == TERM_XTRA_REACT_OPTIONS)
		{
			mobileForceOptions();
		}
#endif
		setTermTitle(&terms[TERM_MAIN]);
		refreshTerm(&terms[TERM_MAIN]);
		return 0; // react?
	case TERM_XTRA_FROSH: // TODO: refresh row "v"
		return 0;
	case TERM_XTRA_DELAY:
		SDL_Delay(v);
		return 0;
	}
	return 1;
}

static errr cursTermHook(int x, int y)
{
	TermData *td = (TermData*)(Term->data);
	SDL_Rect cell_rect =
	{
	     x * td->cell_w,
	     y * td->cell_h,
	     td->cell_w,
	     td->cell_h
	};

	/* Regular cursor */
	SDL_SetRenderDrawColor(td->renderer, 128, 255, 64, 255);
	SDL_RenderDrawRect(td->renderer, &cell_rect);

	/* Alt.Dungeon cursor */
	if (td->config & TERM_DO_SCALE)
	{
		if (looksLikeCave(x, y))
		{
			alt_cursor_x = x - DUNGEON_OFFSET_X;
			alt_cursor_y = y - DUNGEON_OFFSET_Y;
		}
	}

	return 0;
}

static void wipeTermCell_Cave(int x, int y)
{
	int n = 1;
	TermData *td = (TermData*)(Term->data);
	SDL_Rect cell_rect = { x*td->cell_w, y*td->cell_h, td->cell_w*n, td->cell_h };

	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 0, 255, 0, 50);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(td->renderer, &cell_rect);

}
static errr wipeTermCell_ALT(int x, int y)
{
	int n = 1;
	SDL_Rect alt_rect;
	TermData *td = (TermData*)(Term->data);

	if (td->alt_framebuffer == NULL) return -1;
	if (td->pict_data == NULL) return -1;

	alt_rect.x = x * td->pict_data->w;
	alt_rect.y = y * td->pict_data->h;
	alt_rect.w = td->pict_data->w * n;
	alt_rect.h = td->pict_data->h * 1;

	SDL_SetRenderTarget(td->renderer, td->alt_framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(td->renderer, &alt_rect);
	return 0;
}
static void wipeTermCell_UI(int x, int y, int cutout)
{
	int n = 1;
	TermData *td = (TermData*)(Term->data);
	SDL_Rect cell_rect = { x*td->cell_w, y*td->cell_h, td->cell_w*n, td->cell_h };

	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255 - cutout * 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(td->renderer, &cell_rect);
}

static errr wipeTermHook(int x, int y, int n)
{
	int i;
	TermData *td = (TermData*)(Term->data);

	for (i = 0; i < n; i++)
	{
		wipeTermCell_UI(x + i, y, 0);
	}

	if (td->config & TERM_DO_SCALE)
	{
		if ((x - DUNGEON_OFFSET_X == alt_cursor_x)
		 && (y - DUNGEON_OFFSET_Y == alt_cursor_y))
		{
			alt_cursor_x = -1;
			alt_cursor_y = -1;
		}
		if (looksLikeCave(x, y)) td->need_cutout = TRUE;
	}
	return 0;
}

static void textTermCell_Char(int x, int y, byte attr, char c)
{
	int w, h, offsetx, offsety;
	float r;

	TermData *td = (TermData*)(Term->data);
	struct FontData *fd = td->font_data;

	if (td->char_mode == TERM_CHAR_STRETCH)
	{
		w = td->cell_w;
		h = td->cell_h;
	}
	else if (td->char_mode == TERM_CHAR_SCALE)
	{
		w = td->font_data->w;
		h = td->font_data->h;
		r = fmin((float)td->cell_w / (float)w, (float)td->cell_h / (float)h);
		w *= r;
		h *= r;
	}
	else
	{
		w = td->font_data->w;
		h = td->font_data->h;
	}

	offsetx = (td->cell_w / 2) - (w/2);
	offsety = (td->cell_h / 2) - (h/2);

	/* XXX */
	{
		SDL_Rect cell_rect = { x*td->cell_w+offsetx, y*td->cell_h+offsety, w, h };
		int si = c;
		int row = si / 16;
		int col = si - (row*16);
		SDL_Rect char_rect = { col*fd->w, row*fd->h, fd->w, fd->h };

		SDL_RenderCopy(td->renderer, td->font_texture, &char_rect, &cell_rect);
	}
}

static errr textTermHook_ALT(int x, int y, int n, byte attr, cptr s)
{
	int i;
	int old_mode, old_cell_w, old_cell_h;
	TermData *td = (TermData*)(Term->data);
	if (!td->font_data || !td->pict_data) return -1;

	SDL_SetTextureColorMod(td->font_texture, TERM_RGB(attr));

	SDL_SetRenderTarget(td->renderer, td->alt_framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);

	/* Save old mode */
	old_mode = td->char_mode;
	old_cell_w = td->cell_w;
	old_cell_h = td->cell_h;

	/* Set static mode with our dimensions */
	td->char_mode = TERM_CHAR_STRETCH;
	td->cell_w = td->pict_data->w;
	td->cell_h = td->pict_data->h;

	for (i = 0; i < n; i++) {
		wipeTermCell_ALT(x + i, y);
//		SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
		textTermCell_Char(x + i, y, attr, s[i]);
	}

	/* Restore old mode */
	td->char_mode = old_mode;
	td->cell_w = old_cell_w;
	td->cell_h = old_cell_h;

	/* We need re-renders and cutouts */
	td->need_render = TRUE;
	//td->need_cutout = TRUE;

	return 0;
}


static errr textTermHook(int x, int y, int n, byte attr, cptr s)
{
	int i;
	bool probably_cave = FALSE;
	TermData *td = (TermData*)(Term->data);
	struct FontData *fd = td->font_data;
	SDL_SetTextureColorMod(td->font_texture, TERM_RGB(attr));

	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);

	for (i = 0; i < n; i++)
	{
		wipeTermCell_UI(x + i, y, 0);
		textTermCell_Char(x + i, y, attr, s[i]);
		if (!probably_cave) probably_cave = looksLikeCave(x + i, y);
	}

	if (td->config & TERM_DO_SCALE)
	{
		if (probably_cave) td->need_cutout = TRUE;
	}

	return 0;
}


static void pictTermCell_Tile(int x, int y, byte a, byte c, byte ta, byte tc)
{
	SDL_Rect cell_rect, terrain_rect, sprite_rect;
	int offsetx, offsety, w, h;
	float r;
	TermData *td = (TermData*)(Term->data);
	int sf_x = 0, sf_y = 0;

//  struct FontData *fd = td->font_data;
	if (td->pict_mode == TERM_PICT_STRETCH) {
		w = td->cell_w;
		h = td->cell_h;
	} else if (td->pict_mode == TERM_PICT_SCALE) {
		w = td->pict_data->w;
		h = td->pict_data->h;
		r = fmin((float)td->cell_w / (float)w, (float)td->cell_h / (float)h);
		w *= r;
		h *= r;
	} else {
		w = td->pict_data->w;
		h = td->pict_data->h;
	}

	offsetx = (td->cell_w / 2) - (w/2);
	offsety = (td->cell_h / 2) - (h/2);

	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);

	cell_rect.x = x * td->cell_w + offsetx;
	cell_rect.y = y * td->cell_h + offsety;
	cell_rect.w = w;
	cell_rect.h = h;

	sprite_rect.x = c*(td->pict_data->w);
	sprite_rect.y = a*(td->pict_data->h);
	sprite_rect.w = td->pict_data->w;
	sprite_rect.h = td->pict_data->h;

	terrain_rect.x = tc*(td->pict_data->w);
	terrain_rect.y = ta*(td->pict_data->h);
	terrain_rect.w = td->pict_data->w;
	terrain_rect.h = td->pict_data->h;

	/* Slash effect is active on this tile */
	if (sfx_delay[y][x] > 0)
	{
		int dir = sfx_info[y][x].a;
		int ox, oy;
		float halfW = (float)w / 4;
		float halfH = (float)h / 4;
		float prog = (float)( SLASH_FX_THRESHOLD - sfx_delay[y][x] ) / ( SLASH_FX_THRESHOLD);
		slashfx_dir_offset(&ox, &oy, dir, TRUE);
		sf_x = prog * halfW * ox;
		sf_y = prog * halfH * oy;
	}

	if (use_graphics > 1)
	{
		SDL_RenderCopy(td->renderer, td->pict_texture, &terrain_rect, &cell_rect);
		if (ta != a || tc != c)
		{
			cell_rect.x += sf_x;
			cell_rect.y += sf_y;

			SDL_RenderCopy(td->renderer, td->pict_texture, &sprite_rect, &cell_rect);
		}
	} else {
		SDL_RenderCopy(td->renderer, td->pict_texture, &sprite_rect, &cell_rect);
	}
}


/* Draw ALT.DUNGEON */
static errr pictTermHook_ALT(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	int i;
	int old_mode, old_cell_w, old_cell_h;
	Uint8 a, c, ta, tc;
	TermData *td = (TermData*)(Term->data);
	if (td->font_data == NULL || td->pict_data == NULL) return 1;

	SDL_SetRenderTarget(td->renderer, td->alt_framebuffer);

	/* Store old mode */
	old_mode = td->pict_mode;
	old_cell_w = td->cell_w;
	old_cell_h = td->cell_h;

	/* Set stretch mode */
	td->pict_mode = TERM_PICT_STRETCH;
	td->cell_w = td->pict_data->w;
	td->cell_h = td->pict_data->h;

	for (i = 0; i < n; i++)
	{
		a = (ap[i] & 0x7F);
		c = (cp[i] & 0x7F);
		ta = (tap[i] & 0x7F);
		tc = (tcp[i] & 0x7F);

		mymem[1][y][x + i].a = ap[i];
		mymem[1][y][x + i].c = cp[i];
		mymem[0][y][x + i].a = tap[i];
		mymem[0][y][x + i].c = tcp[i];

		wipeTermCell_ALT(x + i, y);
		pictTermCell_Tile(x + i, y, a, c, ta, tc);
	}

	/* Restore old mode */
	td->pict_mode = old_mode;
	td->cell_w = old_cell_w;
	td->cell_h = old_cell_h;

	td->need_render = TRUE;
	//td->need_cutout = TRUE;
	return 0;
}

static errr pictTermHook(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	int i;
	bool probably_cave = FALSE;
	Uint8 a,  c;
	Uint8 ta, tc;

	TermData *td = (TermData*)(Term->data);
	if (td->font_data == NULL || td->pict_data == NULL) return 1;

	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
	//  SDL_SetTextureBlendMode(td->pict_texture, SDL_BLENDMODE_BLEND);

	for (i = 0; i < n; i++)
	{
		a = (ap[i] & 0x7F);
		c = (cp[i] & 0x7F);
		ta = (tap[i] & 0x7F);
		tc = (tcp[i] & 0x7F);

		wipeTermCell_UI(x + i, y, 0);
		pictTermCell_Tile(x + i, y, a, c, ta, tc);

		if (!probably_cave) probably_cave = looksLikeCave(x + i, y);
	}

	if (td->config & TERM_DO_SCALE)
	{
		if (probably_cave) td->need_cutout = TRUE;
	}

	return 0;
}

/* handleMenu
    "i" is term_id
    "x" and "y" are unused!
*/
static errr handleMenu(int i, int x, int y) {
	TermData *td;

	if (i < 0) {
		guiMenuOff();
		return 0;
	}

	td = &terms[i];
	if (menu_hover == MENU_ACT_TOGGLE_VIRTUAL) {

		if (i == TERM_MAIN) return 0;
		/* Close it */
		termClose(i);
		/* Toggle virtuality */
		if (td->config & TERM_IS_VIRTUAL) {
			/* Become a real window */
			td->config &= ~TERM_IS_VIRTUAL;
			td->x = td->ren_rect.x + terms[TERM_MAIN].x;
			td->y = td->ren_rect.y + terms[TERM_MAIN].y - WM_TITLEBAR_OFFSET;
			td->ren_rect.x = 0;
			td->ren_rect.y = 0;
		} else {
			td->config |= TERM_IS_VIRTUAL;
			td->ren_rect.x = td->x - terms[TERM_MAIN].x;
			td->ren_rect.y = td->y - terms[TERM_MAIN].y + WM_TITLEBAR_OFFSET;
		}
		/* Invalidate framebuffer */
		td->fb_w = 0;
		td->fb_h = 0;
		/* Open it again */
		termSpawn(i);
		td->need_redraw = TRUE;
		
		/* Move it around */
		termConstrain(i);

		/* Also, redraw main window */
		terms[0].need_render = TRUE;
	}

	if (menu_hover == MENU_ACT_CLOSE) {
		if (i == TERM_MAIN) return 0;
		termClose(i);
	}

	if (menu_hover == MENU_ACT_SOUND_ON) {
		use_sound = TRUE;
	}

	else if (menu_hover == MENU_ACT_SOUND_OFF) {
#ifdef USE_SOUND
		if (use_sound) sdl_play_sound_end(TRUE);
#endif
		use_sound = FALSE;
	}

	if (menu_hover == MENU_ACT_TOGGLE_GAMEMOUSE) {
		sdl_game_mouse = !sdl_game_mouse;
	}
	if (menu_hover == MENU_ACT_TOGGLE_ARROWCOMBINER) {
		sdl_combine_arrowkeys = !sdl_combine_arrowkeys;
	}
	if (menu_hover == MENU_ACT_TOGGLE_COLLAPSENUMPAD) {
		collapse_numpad_keys = !collapse_numpad_keys;
	}
	if (menu_hover == MENU_ACT_TOGGLE_IGNORELAYOUT) {
		ignore_keyboard_layout = !ignore_keyboard_layout;
	}

	if (menu_hover >= MENU_QUICK_FONT0 && menu_hover <= MENU_QUICK_FONT_LAST) {
		int f = menu_hover - MENU_QUICK_FONT0;
		unloadFont(&terms[i]);
		strcpy(td->font_file, quick_font_names[f]);
		applyTermConf(&terms[i]);
		refreshTerm(&terms[i]);
		termConstrain(i);
		net_visuals_update();
	}

	if (menu_hover >= MENU_FONT_SIZE0 && menu_hover <= MENU_FONT_SIZE_LAST) {
		int f = menu_hover - MENU_FONT_SIZE0;
		unloadFont(&terms[i]);
		td->font_size = ttf_font_sizes[f];
		applyTermConf(&terms[i]);
		refreshTerm(&terms[i]);
		termConstrain(i);
	}

	if (menu_hover == MENU_QUICK_GRAF0) {
		use_graphics = 0;
//		unloadFont(&terms[i]);
		unloadPict(&terms[i]);
		strcpy(td->pict_file, "");
		applyTermConf(&terms[i]);
		ANGBAND_GRAF = "none";
		refreshTerm(&terms[i]);
		termConstrain(i);
		net_visuals_update();
	}

	if (menu_hover >= MENU_QUICK_GRAF1 && menu_hover <= MENU_QUICK_GRAF8) {
		int f = menu_hover - MENU_QUICK_GRAF0;
		graphics_mode* gm;
		use_graphics = f;
		gm = get_graphics_mode((byte)use_graphics);
//		unloadFont(&terms[i]);
		unloadPict(&terms[i]);
		strcpy(td->pict_file, gm->file);
		strcpy(td->mask_file, gm->mask);
		applyTermConf(&terms[i]);
		ANGBAND_GRAF = gm->pref;
		refreshTerm(&terms[i]);
		termConstrain(i);
		net_visuals_update();
	}

	if (menu_hover == MENU_ZOOM_OFF) {
		if (terms[i].config & TERM_DO_SCALE) {
			terms[i].config &= ~TERM_DO_SCALE;
			terms[i].char_mode = TERM_CHAR_STATIC;
			terms[i].pict_mode = TERM_PICT_STRETCH;
		} else {
			terms[i].config |= TERM_DO_SCALE;
			terms[i].char_mode = TERM_CHAR_STATIC;
			terms[i].pict_mode = TERM_PICT_STRETCH;
		}
		applyTermConf(&terms[i]);
		refreshTerm(&terms[i]);
		termConstrain(i);
	}

	if (menu_hover >= MENU_ZOOM_LEVEL0 && menu_hover <= MENU_ZOOM_LEVEL8) {
		int l = menu_hover - MENU_ZOOM_LEVEL0;
		terms[i].config |= TERM_DO_SCALE;
		terms[i].zoom = zoom_levels[l];
		applyTermConf(&terms[i]);
		refreshTerm(&terms[i]);
		termConstrain(i);
	}

	if (menu_hover >= MENU_SHOW_WINDOW0 && menu_hover <= MENU_SHOW_WINDOW0+8) {
		int j = menu_hover - MENU_SHOW_WINDOW0;
		if (j == 0) return 0;
		if (terms[j].config & TERM_IS_ONLINE) {
			termClose(j);
		} else {
			termSpawn(j);
			if (terms[j].x < 0 || terms[j].y < 0 || terms[j].ren_rect.x < 0 || terms[j].ren_rect.y < 0)
				termStack(j);
			termConstrain(j);
		}
	}
	
	/* Click-off */
	if ((menu_hover >= MENU_ACT_MIN && menu_hover <= MENU_ACT_MAX)
	|| menu_hover == -1)
	{
		menu_open = -1;
		menu_open_term = -1;
		menu_hover = -1;
	}
	/* Open submenu */
	else
	{
		menu_open = menu_hover;
		menu_open_term = menu_term;
		terms[menu_term].need_render = TRUE;
	}
	return 0;
}


static errr sysText(TermData *td, int x, int y, int n, byte attr, cptr s)
{
	int i;
	struct FontData *fd = td->font_data;
	SDL_SetTextureColorMod(td->font_texture, TERM_RGB(attr));

	for (i = 0; i < n; i++)
	{
		SDL_Rect cell_rect = {
			((x + i) * fd->w) + td->menu_rect.x,
			((y + 0) * fd->h) + td->menu_rect.y,
			fd->w, fd->h
		};
		int si = s[i];
		int row = si / 16;
		int col = si - (row * 16);
		SDL_Rect char_rect = { col*fd->w, row*fd->h, fd->w, fd->h };
		SDL_RenderCopy(td->renderer, td->font_texture, &char_rect, &cell_rect);
	}
	return 0;
}


#ifdef USE_LISENGLAS
/* ===== Lisenglass ===== */

static errr loadUiCircle()
{
	SDL_Surface *sf_circle;
	SDL_Color darkness = { 0, 0, 0, 0 };
	sf_circle = SDL_CreateCircleSurface32(96, 96, 48, &darkness);
	if (sf_circle == NULL) return -1;
	tx_circle = SDL_CreateTextureFromSurface(terms[0].renderer, sf_circle);
	//SDL_FreeSurface(sf_circle);
	if (tx_circle == NULL) return -1;
	return 0;
}
static void unloadUiCircle(void)
{
	if (tx_circle != NULL) SDL_DestroyTexture(tx_circle);
	tx_circle = NULL;
	if (tx_lisen != NULL) SDL_DestroyTexture(tx_lisen);
	tx_lisen = NULL;
}

static void updateLisenglas(int wx, int wy)
{
	int ax, ay;
	int lw = LISEN_W + LISEN_W/2;
	int lh = LISEN_H + LISEN_H/2;
	SDL_Rect *r = &terms[0].ren_rect;

	if (icon_hover >= 0 || screen_icky || state != PLAYER_PLAYING)
	{
		lisen_viable = FALSE;
		return;
	}

	altCoord(wx, wy, &ax, &ay);

	/* Is it even viable ? */
	lisen_viable = TRUE;
	if (ax < 0 || ay < 0
	|| ax > terms[0].dng_cols
	|| ay >= terms[0].dng_rows)
	{
		lisen_viable = FALSE;
		return;
	}

	/* Force-rerender */
	terms[0].need_render = TRUE;

	/* Center */
	lisen_x = wx - LISEN_W;
	lisen_y = wy - LISEN_H;

	/* Upwards */
	lisen_y -= lh;

	/* Clamp */
	if (lisen_y <= 0) lisen_y = 0;

	/* Finger inside */
	if (wy >= lisen_y && wy <= lisen_y + LISEN_H*2 + 32)
	{
#ifndef MOBILE_UI
		/* Downwards (this looks most logical) */
		lisen_x -= wy + LISEN_H - 32;
#else
		/* Leftwards (on mobile, we should never draw UNDER the finger) */
		lisen_x -= lh;
#endif
	}

	/* Clamp left/right */
	if (lisen_x <= 0) lisen_x = 0;
	if (lisen_x + LISEN_W*2 >= terms[0].width) lisen_x = terms[0].width - LISEN_W*2;

	/* Clamp source tile */
	lisen_sx = ax - 1;
	lisen_sy = ay - 1;
	if (lisen_sx < -1) lisen_sx = -1;
	if (lisen_sy < -1) lisen_sy = -1;
	if (lisen_sx >= terms[0].dng_cols-1) lisen_sx = terms[0].dng_cols-2;
	if (lisen_sy >= terms[0].dng_rows-1) lisen_sy = terms[0].dng_rows-2;
}

static void renderLisenGlas(TermData *td)
{
	int tile_w = td->pict_data->w;
	int tile_h = td->pict_data->h;

	SDL_Rect src = {
		lisen_sx * 32,
		lisen_sy * 32,
		32 * 3, 32 * 3
	};
	SDL_Rect dst = {
		lisen_x,
		lisen_y,
		32 * 3 * 2, 32 * 3 * 2
	};
	SDL_Rect dst2 = {
		0, 0, dst.w, dst.h
	};

	/* Highlighted tile in abolute coords */
	SDL_Rect main_tile = {
		(lisen_sx+1) * 32 + td->dng_rect.x + td->ren_rect.x,
		(lisen_sy+1) * 32 + td->dng_rect.y + td->ren_rect.y,
		32, 32
	};

	if (!lisen_pressed || !lisen_viable) return;

	if (tx_lisen == NULL)
	{
		tx_lisen = SDL_CreateTexture(td->renderer,
			SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
			32 * 9, 32 * 9);
		if (tx_lisen == NULL) return;
	}

	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);

	/* Draw lines */
	SDL_SetRenderTarget(td->renderer, NULL);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 128);
#if 0
	SDL_RenderDrawLine(td->renderer,
		lisen_x + LISEN_W,
		lisen_y,
		main_tile.x + main_tile.w / 2,
		main_tile.y);
	SDL_RenderDrawLine(td->renderer,
		lisen_x + LISEN_W,
		lisen_y + LISEN_H * 2,
		main_tile.x + main_tile.w / 2,
		main_tile.y + main_tile.h);
#endif
	SDL_RenderDrawLine(td->renderer,
		lisen_x + 1,
		lisen_y + LISEN_H,
		main_tile.x,
		main_tile.y + main_tile.h / 2);
	SDL_RenderDrawLine(td->renderer,
		lisen_x + LISEN_W * 2 - 1,
		lisen_y + LISEN_H,
		main_tile.x + main_tile.w,
		main_tile.y + main_tile.h / 2);

	SDL_RenderDrawRect(td->renderer, &main_tile);

	/* Draw on temp. texture */
	SDL_SetRenderTarget(td->renderer, tx_lisen);

	/* Clear it */
	//SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 0);
	//SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);
	//SDL_RenderFillRect(td->renderer, &dst2);

	/* Draw circle */
	if (1) {
		SDL_Rect csrc = { 0, 0, LISEN_W, LISEN_H };
		SDL_Rect cdst = { 0, 0, 96*2, 96*2 };
		SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
		SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 0);
		//SDL_SetRenderDrawColor(td->renderer, 255, 255, 0, 255);
		SDL_SetTextureBlendMode(tx_circle, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(td->renderer, tx_circle, &csrc, &cdst);
	}
	/* Draw copy of alt.dungeon */
	if (1) {
		SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 0);
		SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
		//SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 0);
		SDL_SetTextureBlendMode(td->alt_framebuffer, SDL_BLENDMODE_ADD);
		SDL_RenderCopy(td->renderer, td->alt_framebuffer, &src, &dst2);
	}

	/* Render to screen */
	SDL_SetRenderTarget(td->renderer, NULL);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(tx_lisen, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(td->renderer, tx_lisen, &dst2, &dst);
}
#endif /* USE_LISENGLAS */


#ifdef USE_ICON_OVERLAY
/* ===== Icon Overlay ===== */

static void renderIconOverlay(TermData *td)
{
	/* Let's figure out some good positions */
	int max_w, max_h;
	int left = 0;
	int top = td->cell_h;

	int icon_w = cmd_font.w;
	int icon_h = cmd_font.h;
	int icon_ws = cmd_font.w + MICON_S;
	int icon_hs = cmd_font.h + MICON_S;

	num_icons = 0;

	getRendererSize(td, &max_w, &max_h);

	/* Display Dragged Icon */
	if (dragging_icon)
	{
		SDL_Rect pos = drag_icon.pos;
#ifdef MOBILE_UI
		/* On mobile user's finger is convering the icon, move it */
		pos.x -= MICON_W;
		pos.y -= MICON_H;
#endif
		iconPict(&pos, drag_icon.a, drag_icon.c, FALSE);
		drawUiIcon(&pos, drag_icon.draw);
	}


	SDL_SetRenderTarget(td->renderer, NULL);
	SDL_SetTextureBlendMode(tx_cmd, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(tx_cmd, 200);

	if (section_icky_row)
		SDL_SetTextureAlphaMod(tx_cmd, 64);
	else
		SDL_SetTextureAlphaMod(tx_cmd, 200);

	/* Escape icon */
	{
		SDL_Rect pos = { 0, top, icon_w, icon_h };
		if (section_icky_row) pos.y = section_icky_row * td->cell_h;
		renderMIcon(&pos, &pos, MICON_ESCAPE, 0, 0x7F, 10);
	}
	/* KBD icon */
	{
		SDL_Rect pos = { max_w - icon_w, top, icon_w, icon_h };
		renderMIcon(&pos, &pos, MICON_TOGGLE_KEYBOARD, 0, MICO_KBD, 10);
	}
	/* Toggle overlay icon */
	if (term2_icon_context != MICONS_DIRECTION)
	{
		SDL_Rect pos = { max_w - icon_w, max_h - icon_h, icon_w, icon_h };
		renderMIcon(&pos, &pos, MICON_TOGGLE_OVERLAY, 0, MICO_FINGER_CLICK, 10);
	}

	/* Angband menu (show even on icky screens) */
	if (term2_icon_context == MICONS_PASSED_MENU)
	{
		SDL_Rect pane = {
			td->cell_w * micon_passed_menu_col,
			td->cell_h * micon_passed_menu_row, max_w, max_h };
		pane.w -= pane.x;
		pane.h -= pane.y;
		drawIconPanel_PassedMenu(&pane);
	}

	if (state != PLAYER_PLAYING) return;

	/* Quick-slots */
	if (term2_icon_context != MICONS_DIRECTION)
	{
		SDL_Rect pane2 = { (max_w - (icon_ws*10))/2, max_h - icon_hs, icon_ws * 10, icon_hs };
		drawIconPanel_Slots(&pane2);
	}

	if (screen_icky && !shopping) return;
	if (section_icky_row) return;

	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureColorMod(td->pict_texture, 255,255,255);
	SDL_SetTextureAlphaMod(td->pict_texture, 255);

	/** Gameplay icons **/
	/* Contextual */
	if (term2_icon_context == MICONS_DIRECTION)
	{
		SDL_Rect pane = { max_w - icon_ws * 3, max_h - icon_hs * 3, icon_ws * 3, icon_hs * 3 };
		drawIconPanel_Direction(&pane, micon_allow_target, micon_allow_friend);
	}
	else if (term2_icon_context == MICONS_CONFIRMATION)
	{
		SDL_Rect pane = { max_w / 2 - icon_ws * 2, max_h / 2 - icon_hs / 2, icon_ws * 3 * 2, icon_hs};
		drawIconPanel_Confirmation(&pane);
	}
	else if (term2_icon_context == MICONS_ITEM)
	{
		SDL_Rect pane2 = { max_w - (64) * 4 - 1, top + icon_hs, (64) * 4 + 1, (64) * 7 };
		drawIconPanel_Inventory(&pane2, micon_allow_inven, micon_allow_equip, micon_allow_floor);
	}
	else if (term2_icon_context == MICONS_EQUIP)
	{
		SDL_Rect pane2 = { max_w - (64) * 5, top + 64, (64) * 5, (64) * 5 };
		drawIconPanel_Equipment(&pane2, 0, 0, 0);
	}
	else if (term2_icon_context == MICONS_SPELL)
	{
		SDL_Rect pane2 = { max_w - (64+1) * 5, top + 64, (64+1) * 5, (64) * 5 };
		drawIconPanel_Spells(&pane2, micon_spell_realm, micon_spell_book);
	}
	else if (term2_icon_context == MICONS_STORE)
	{
		SDL_Rect pane = { max_w - 64, top + 64, MICON_W * 1, max_h };
		drawIconPanel_Commands(&pane, MICONS_STORE);
	}
	/* User pref */
	else if (term2_icon_pref == MICONS_INVEN)
	{
		SDL_Rect pane2 = { max_w - (64) * 4 - 1, top + icon_hs, (64) * 4 + 1, (64) * 7 };
		drawIconPanel_Inventory(&pane2, 0, 0, 0);

		micon_command_filter = MICONS_INVEN;

		SDL_Rect pane = { 0, top + 64, 32 * 9, max_h };
		drawIconPanel_Commands(&pane, micon_command_filter);
	}
	else if (term2_icon_pref == MICONS_EQUIP)
	{
		SDL_Rect pane2 = { max_w - (64) * 5, top + 64, (64) * 5, (64) * 5 };
		drawIconPanel_Equipment(&pane2, 0, 0, 0);

		micon_command_filter = MICONS_EQUIP;

		SDL_Rect pane = { 0, top + 64, 32 * 9, max_h };
		drawIconPanel_Commands(&pane, micon_command_filter);
	}
	else if (term2_icon_pref == MICONS_SPELLS)
	{
		SDL_Rect pane2 = { max_w - (64+1) * 5, top + 64, (64+1) * 5, (64) * 5 };
		drawIconPanel_Spells(&pane2, -1, -1);

		micon_command_filter = MICONS_SPELLS;

		SDL_Rect pane = { 0, top + 64, 32 * 9, max_h };
		drawIconPanel_Commands(&pane, micon_command_filter);
	}
	else if (term2_icon_pref == MICONS_SELECTOR_CLOSE)
	{
		/* Nothing */
	}
	else
	{
		SDL_Rect pane = { 0, top + 64, 32 * 9, max_h };
		drawIconPanel_Commands(&pane, micon_command_filter);

		SDL_Rect pane2 = { max_w - 32 - 32, top + 64, 32 + 16, max_h };
		drawIconPanel_Selector(&pane2, 0);
	}
}

static errr loadCmdFont(cptr filename)
{
#if 0
	if (imgToPict(&uipict, filename, NULL) != 0)
	{
		return -1;
	}
	tx_cmd = SDL_CreateTextureFromSurface(terms[0].renderer, cmd_pict.surface);
#else
	cleanFontData(&cmd_font);
	sdl_graf_prefer_rgba = TRUE;
	if (fileToFont(&cmd_font, filename, 55, 2) != 0)
	{
		return -1;
	}
	//printf("UI-CMD font size: %d x %d\n", cmd_font.w, cmd_font.h);
	tx_cmd = SDL_CreateTextureFromSurface(terms[0].renderer, cmd_font.surface);
#endif
	return 0;
}
static void unloadCmdFont(void)
{
	if (tx_cmd) SDL_DestroyTexture(tx_cmd);
	tx_cmd = NULL;
}

static errr loadTinyFont(cptr filename)
{
	cleanFontData(&tiny_font);
	sdl_graf_prefer_rgba = TRUE;
	if (fileToFont(&tiny_font, filename, 0, 0) != 0)
	{
		return -1;
	}
	tx_tiny = SDL_CreateTextureFromSurface(terms[0].renderer, tiny_font.surface);
	return 0;
}
static void unloadTinyFont(void)
{
	cleanFontData(&tiny_font);
	if (tx_tiny) SDL_DestroyTexture(tx_tiny);
	tx_tiny = NULL;
}

static errr iconText(int x, int y, byte attr, cptr s, int tiny)
{
	int i;
	int mult = 1;
	TermData *td = &terms[0];
	const char *c = s;
	struct FontData *fd = td->font_data;
	SDL_Texture *tx = td->font_texture;

	/* Use tiny font instead */
	if (tiny > 0)
	{
		fd = &tiny_font;
		tx = tx_tiny;
	}

#ifdef MOBILE_UI
	/* Hack -- change size */
	if (tiny == -2)
	{
		if (SDL_IsTablet() || isHighDPI())
		{
			mult = 2;
		}
	}
#endif

	/* Set color */
	SDL_SetTextureColorMod(tx, TERM_RGB(attr));

	/* Dump each character */
	while (*c)
	{
		SDL_Rect cell_rect = { x, y, fd->w*mult, fd->h*mult };
		int si = *c;
		int row = si / 16;
		int col = si - (row * 16);
		SDL_Rect char_rect = { col*fd->w, row*fd->h, fd->w, fd->h };
		SDL_RenderCopy(td->renderer, tx, &char_rect, &cell_rect);
		x += fd->w*mult;
		c++;
	}
	return 0;
}

static errr iconPict(SDL_Rect *pos, byte a, char c, bool remember)
{
	TermData *td = &terms[0];
	struct PictData *pd = td->pict_data;

	int col = c & 0x7F;
	int row = a & 0x7F;

	SDL_Rect src_rect = { col*pd->w, row*pd->h, pd->w, pd->h };
	SDL_Rect dst_rect = { pos->x, pos->y, pos->w, pos->h };

	SDL_RenderCopy(td->renderer, td->pict_texture, &src_rect, &dst_rect);

	if (remember)
	{
		IconData *id = &icons[num_icons];
		id->a = a;
		id->c = c;
	}

	return 0;
}

static void drawUiIcon(SDL_Rect *dst, int k)
{
	TermData *td = &terms[0];
	int sy = k / 16;
	int sx = k - (sy * 16);
	SDL_Rect cell_rect = {
		sx * cmd_font.w - 3,
		sy * cmd_font.h - 3,
		cmd_font.w, cmd_font.h
	};
	SDL_RenderCopy(td->renderer, tx_cmd, &cell_rect, dst);
}

static void recountSlotItems(void)
{
	size_t i;
	for (i = 0; i < 10; i++)
	{
		SlotData *slot = &quick_slots[i];
		if (!STRZERO(slot->macro_item))
		{
			slot->item_counter = count_items_by_name(slot->macro_item, TRUE, TRUE, TRUE);
		}
	}
}

static void convertIconToSlot(void *slot_p, void *icon_p, int slot_id)
{
	SlotData *slot = (SlotData*)slot_p;
	IconData *icon = (IconData*)icon_p;

	slot->draw = icon->draw;
	slot->action = icon->action;
	slot->sub_action = icon->sub_action;

	slot->a = 0;
	slot->c = 0;

	slot->macro_act[0] = '\0';
	slot->macro_item[0] = '\0';
	slot->display[0] = '\0';

	/* Copy quick slot */
	if (icon->action == MICON_LOCAL_QUICK_SLOT)
	{
		SlotData *orig = &quick_slots[icon->sub_action];

		slot->action = orig->action;
		slot->sub_action = orig->sub_action;
		my_strcpy(slot->macro_act, orig->macro_act, 1024);
		my_strcpy(slot->macro_item, orig->macro_item, 1024);
		my_strcpy(slot->display, orig->display, 1024);

		slot->draw = orig->draw;
		slot->a = orig->a;
		slot->c = orig->c;
		slot->attr = orig->attr;
		slot->item_counter = orig->item_counter;

		if (orig->action == MICON_LOCAL_EXECUTE_MACRO)
		{
			slot->action = orig->action;
			slot->sub_action = slot_id; /* Important */
		}
	}
	else if (icon->action == MICON_SELECT_SPELL)
	{
		char cmd;
		char buf[1024];
		int idx = icon->sub_action;
		int book = idx / SPELLS_PER_BOOK;
		int sn = idx - (book * SPELLS_PER_BOOK);

		slot->macro_act[0] = '\0';
		buf[0] = '\0';

		cmd = command_by_item(book, TRUE);
		spell_as_keystroke(sn, book, cmd, buf, 1024, (CTXT_WITH_CMD));

		/* Do not project */
		if ((spell_flag[idx] & PY_SPELL_PROJECT))
		{
			my_strcat(buf, "n", 1024);
		}

		/* Add '*t' to the macro */
		if ((spell_flag[idx] & PY_SPELL_AIM))
		{
			my_strcat(buf, "*t", 1024);
		}

		text_to_ascii(slot->macro_act, 1024, buf);

		/* Extract "single name" */
		buf[0] = '\0';
		item_as_keystroke(book, cmd, buf, 1024, 0);
		buf[strlen(buf)-1] = '\0';
		my_strcpy(slot->macro_item, &buf[1], 1024);
		slot->item_counter = inventory[book].number;

		my_strcpy(slot->display, spell_info[book][sn] + 5,
			(icon->pos.w+16+1) / tiny_font.w);
		slot->attr = inventory[book].sval;

		slot->action = MICON_LOCAL_EXECUTE_MACRO;
		slot->sub_action = slot_id;
	}
	else if ((icon->action == MICON_SELECT_INVEN)
	 || (icon->action == MICON_SELECT_EQUIP))
	{
		char cmd;
		char buf[1024];
		custom_command_type *cc_ptr;
		int idx = icon->sub_action;
		slot->a = inventory[idx].ix;
		slot->c = inventory[idx].iy;

		slot->macro_act[0] = '\0';
		buf[0] = '\0';

		cmd = command_by_item(idx, TRUE);
		item_as_keystroke(idx, cmd, buf, 1024, (CTXT_WITH_CMD));

		/* Add '*t' to the macro */
		if ((cc_ptr = match_custom_command(cmd, FALSE)))
		{
			if (cc_ptr->flag & COMMAND_TARGET_ALLOW)
			{
				my_strcat(buf, "*t", 1024);
			}
		}

		text_to_ascii(slot->macro_act, 1024, buf);

		/* Extract "single name" */
		buf[0] = '\0';
		item_as_keystroke(idx, cmd, buf, 1024, 0);
		buf[strlen(buf)-1] = '\0';
		my_strcpy(slot->macro_item, &buf[1], 1024);
		slot->item_counter = inventory[idx].number;

		slot->action = MICON_LOCAL_EXECUTE_MACRO;
		slot->sub_action = slot_id;
	}

	/* Always do it after creating new slot */
	recountSlotItems();
}

static void handleMIcon(int action, int sub_action)
{
	terms[0].win_need_render = TRUE;
		switch (action)
		{
			/* Escape */
			case MICON_ESCAPE:
			{
				Term_keypress(ESCAPE);
				if (section_icky_row || screen_icky) return;
				if (term2_icon_pref != MICONS_SELECTOR_CLOSE)
				{
					if (term2_icon_pref != MICONS_COMMANDS)
						term2_icon_pref = -1;
					//if (term2_icon_pref != MICONS_COMMANDS)
					//	micon_command_filter = MICONS_QUICKEST;
					term2_item_pref = -1;
					term2_spell_pref = -1;
				}
			}
			break;
			/* Toggle onscreen keyboard */
			case MICON_TOGGLE_KEYBOARD:
			{
				if (SDL_IsTextInputActive() || SDL_IsScreenKeyboardShown(terms[0].window))
				{
					SDL_StopTextInput();
				}
				else
				{
					SDL_StartTextInput();
				}
			}
			break;
			/* Toggle icon overlay */
			case MICON_TOGGLE_OVERLAY:
			{
				if (term2_icon_pref != MICONS_SELECTOR_CLOSE)
				{
					term2_icon_pref = MICONS_SELECTOR_CLOSE;
				}
				else
				{
					term2_icon_pref = -1;
					//micon_command_filter = MICONS_QUICKEST;
					term2_item_pref = -1;
					term2_spell_pref = -1;
				}
			}
			break;
			/* Root selector */
			case MICON_TOGGLE_ROOT:
			{
				/* Hack -- waiting for an item && action is toggle */
				if ((
				    (term2_icon_context == MICONS_EQUIP)
				    && (sub_action == MICONS_INVEN)
				    ) || (
				    (term2_icon_context == MICONS_ITEM)
				    && (sub_action == MICONS_EQUIP)
				))
				{
					/* Toggle inven/equip */
					Term_keypress('/');
					break;
				}

				term2_icon_pref = sub_action;
				if (sub_action == MICONS_QUICKEST) micon_command_filter = MICONS_QUICKEST;
				if (sub_action == MICONS_ALTER) micon_command_filter = MICONS_ALTER;
				if (sub_action == MICONS_SYSTEM) micon_command_filter = MICONS_SYSTEM;
			}
			break;
			/* Local command */
			case MICON_LOCAL_COMMAND_HACK:
			{
				int key = sub_action;
				Term_keypress(29);
				Term_keypress('\f');
				Term_keypress(ESCAPE);
				Term_keypress(ESCAPE);
				Term_keypress(ESCAPE);
				Term_keypress('\\');
				Term_keypress(key);
				Term_keypress(30);
			}
			break;
			/* Command! */
			case MICON_RUN_COMMAND:
			{
				custom_command_type *cmd = &custom_command[sub_action];
				/* Hack -- shop commands are handled differently: */
				if (cmd->flag & COMMAND_STORE)
				{
					Term_keypress(cmd->m_catch);
					break;
				}
				Term_keypress(29);
				Term_keypress('\f');
				Term_keypress(ESCAPE);
				Term_keypress(ESCAPE);
				Term_keypress(ESCAPE);
				Term_keypress('\\');
				Term_keypress(cmd->m_catch);
				if ((micon_command_filter == MICONS_SPELLS)
				   && (cmd->flag & COMMAND_NEED_SPELL))
				{
					if (term2_spell_pref > -1)
					{
						int book = term2_spell_pref / SPELLS_PER_BOOK;
						int sn = term2_spell_pref - (book * SPELLS_PER_BOOK);
						Term_keypress('a' + book);
						Term_keypress('a' + sn);
					}
				}
				if ((micon_command_filter == MICONS_EQUIP)
				   && (cmd->flag & COMMAND_ITEM_EQUIP))
				{
					if (term2_item_pref > -1)
					{
						/* Command is equip-only, yet we're in inven. context */
						if ((cmd->flag & COMMAND_ITEM_INVEN)
						&& (term2_icon_context != MICONS_EQUIP))
						{
							Term_keypress('/');
						}
						Term_keypress('a' + term2_item_pref - INVEN_WIELD);
					}
					/* Player is already examining Equip panel */
					else if (term2_icon_pref == MICONS_EQUIP
					|| term2_icon_pref == -1)
					{
						if ((cmd->flag & COMMAND_ITEM_INVEN)
						/*&& (term2_icon_context != MICONS_EQUIP)*/)
						{
							Term_keypress('/');
						}
					}
					else {
					}
				}
				if ((micon_command_filter == MICONS_INVEN)
				   && (cmd->flag & COMMAND_ITEM_INVEN))
				{
					if (term2_item_pref > -1)
					{
						Term_keypress('a' + term2_item_pref);
					}
					if (term2_item_pref <= -11)
					{
						Term_keypress('-');
					}
				}
				Term_keypress(30);
			}
			break;
			/* Direction */
			case MICON_PICK_DIRECTION:
			{
				int key = '0' + sub_action;
				Term_keypress(key);
			}
			break;
			/* Confirmation */
			case MICON_SINGLE_KEY:
			{
				int key = sub_action;
				Term_keypress(key);
			}
			break;
			/* Item selection */
			case MICON_SELECT_INVEN:
			{
				int key = 'a' + sub_action;
				int ind = sub_action;
				/* Waiting for an item */
				if (term2_icon_context == MICONS_ITEM)
				{
					Term_keypress(key);
				}
				else
				{
					term2_item_pref = ind;
				}
			}
			break;
			/* Equip selection */
			case MICON_SELECT_EQUIP:
			{
				int key = 'a' + sub_action - INVEN_WIELD;
				int ind = sub_action;
				/* Waiting for an item */
				if (term2_icon_context == MICONS_EQUIP)
				{
					Term_keypress(key);
				}
				else
				{
					term2_item_pref = ind;
				}
			}
			break;
			/* Floor selection */
			case MICON_SELECT_FLOOR:
			{
				int key = '-';
				/* Waiting for an item */
				if (term2_icon_context == MICONS_ITEM)
				{
					Term_keypress(key);
				}
				else
				{
					term2_icon_pref = MICONS_INVEN;
					term2_item_pref = -11;
				}
			}
			break;
			/* Select spell */
			case MICON_SELECT_SPELL:
			{
				int book = sub_action / SPELLS_PER_BOOK;
				int sn = sub_action - (book*SPELLS_PER_BOOK);
				int sn_key = 'a' + sn;
				int bk_key = 'a' + book;
				//printf("BOOK: %d SPELL: %d\n", book, sn);
				/* Waiting for a spell */
				if (term2_icon_context == MICONS_SPELL)
				{
					Term_keypress(sn_key);
				}
				/* Waiting for an item */
				else if (term2_icon_context == MICONS_ITEM)
				{
					Term_keypress(bk_key);
					Term_keypress(sn_key);
				}
				else
				{
					term2_spell_pref = sub_action;
				}
			}
			break;
			/* Execute quick slot */
			case MICON_LOCAL_QUICK_SLOT:
			{
				SlotData *slot = &quick_slots[sub_action];
				/* Assign to quick slot */
				if (dragging_icon)
				{
					convertIconToSlot(slot, &drag_icon, sub_action);
					break;
				}
				/* If this slot has some kind of action */
				if ((slot->action != MICON_DO_NOTHING)
				 && (slot->action != MICON_LOCAL_QUICK_SLOT))
				{
					/* Recurse */
					handleMIcon(slot->action, slot->sub_action);
				}
			}
			break;
			/* Quickslots -> Execute macro */
			case MICON_LOCAL_EXECUTE_MACRO:
			{
				SlotData *slot = &quick_slots[sub_action];
				char *s;
				s = slot->macro_act;
				Term_keypress(29);
				while (*s) Term_keypress(*s++);
				Term_keypress(30);
			}
			break;
			default:
			break;
		}
}

static int matchIcon(int wx, int wy)
{
	int i;
	for (i = 0; i < num_icons; i++)
	{
		IconData *id = &icons[i];
		if (wx >= id->pos.x && wy >= id->pos.y
		&& wx <= id->pos.x + id->pos.w
		&& wy <= id->pos.y + id->pos.h)
		{
			return i;
		}
	}
	return -i;
}

static void renderMIcon(SDL_Rect *panel, SDL_Rect *pos, int action, int sub_action, int draw, int spacing)
{
	IconData *id = &icons[num_icons];

	id->pos.x = pos->x;
	id->pos.y = pos->y;
	id->pos.w = pos->w;
	id->pos.h = pos->h;

	id->action = action;
	id->sub_action = sub_action;

	id->draw = draw;
	/* Hack -- we have already set "a","c" elsewhere */
	if (draw != -2)
	{
		id->a = 0;
		id->c = 0;
	}
	//id->attr = 0;

	if (draw >= 0) drawUiIcon(pos, draw);

	/* move */
	pos->x += pos->w;
	pos->x += spacing;
	if (pos->x + pos->w >= panel->x + panel->w)
	{
		pos->x = panel->x;
		pos->y += pos->h;
		pos->y += spacing;
	}

	num_icons++;
}

static void drawIconPanel_Commands(SDL_Rect *size, int filter)
{
	int i;
	int spacing = MICON_S;
	SDL_Rect pos = { 0, 0, MICON_W, MICON_H };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;

	/* Hack -- add some commands */
	if (filter == MICONS_QUICKEST)
	{
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(TERM_WHITE));
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, 'R', 'R', spacing);
	}
	if (filter == MICONS_SYSTEM)
	{
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(TERM_WHITE));
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, 'C', 'C', spacing);
	}
	for (i = 0; i < custom_commands; i++)
	{
		custom_command_type *cmd = &custom_command[i];
		byte attr = TERM_WHITE;
		int draw_key = cmd->m_catch;
		if (draw_key < 32) draw_key += 224;
		if (filter == MICONS_QUICKEST)
		{
			if (!(cmd->flag == 0)) continue;
			if (cmd->flag & COMMAND_STORE) continue;
			/* HACK -- simply ignore searching */
			if (cmd->m_catch == 's') continue;
			/* HACK -- simply ignore 'stay' */
			//if (cmd->m_catch == 'g') continue;
		}
		if (filter == MICONS_ALTER)
		{
			if (!(cmd->flag & COMMAND_TARGET_DIR)) continue;
			if (cmd->flag & COMMAND_STORE) continue;
		}
		if (filter == MICONS_SYSTEM)
		{
			if (!(cmd->flag & COMMAND_INTERACTIVE)) continue;
			/* HACK -- simply ignore some commands */
			if (cmd->m_catch == '&') continue;
			if (cmd->m_catch == 'M') continue;
		}
		if (filter == MICONS_INVEN)
		{
			if (!(cmd->flag & COMMAND_ITEM_INVEN)) continue;
			if (cmd->flag & COMMAND_STORE) continue;
			if (term2_item_pref < 0 && floor_item.tval == 0)
			{
				if (cmd->tval) continue;
			}
			else
			{
				if (!command_tester_okay(i, term2_item_pref)) continue;
				if (cmd->tval != 0)
				{
					if (term2_item_pref < 0) attr = floor_item.sval;
					else attr = inventory[term2_item_pref].sval;
				}
			}
		}
		if (filter == MICONS_EQUIP)
		{
			if (!(cmd->flag & COMMAND_ITEM_EQUIP)) continue;
			if (cmd->flag & COMMAND_STORE) continue;
			if (term2_item_pref < 0)
			{
				if (cmd->tval) continue;
			}
			else
			{
				if (!command_tester_okay(i, term2_item_pref)) continue;
				if (cmd->tval != 0)
				{
					attr = inventory[term2_item_pref].sval;
				}
			}
		}
		if (filter == MICONS_STORE)
		{
			//if (!(cmd->flag & COMMAND_ITEM_INVEN)) continue;
			if (!(cmd->flag & COMMAND_STORE)) continue;
			if (cmd->m_catch == 's') draw_key = MICO_SELL;
			if (cmd->m_catch == 'p') draw_key = MICO_PURCHASE;
			if (cmd->m_catch == 'l') draw_key = 'I';
		}
		if (filter == MICONS_SPELLS)
		{
			if (!(cmd->flag & COMMAND_SPELL_BOOK)
			&&  !(cmd->flag & COMMAND_SPELL_RESET)) continue;
			if (cmd->flag & COMMAND_STORE) continue;
		}
		//iconText(pos.x, pos.y, TERM_WHITE, cmd->display, FALSE);
		//
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(attr));
		renderMIcon(size, &pos, MICON_RUN_COMMAND, i, draw_key, spacing);

		if (pos.y >= size->y + size->h) break;
	}
	/* More local commands */
	if (filter == MICONS_QUICKEST)
	{
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(TERM_WHITE));
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, 'M', 'M', spacing);
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, '*', '*', spacing);
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, '(', '(', spacing);
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, 'L', 'L', spacing);
	}
	if (filter == MICONS_SYSTEM)
	{
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(TERM_WHITE));
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, ':', ':', spacing);
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, KTRL('D'), 0xE4, spacing);
		renderMIcon(size, &pos, MICON_LOCAL_COMMAND_HACK, KTRL('P'), 0xF0, spacing);
	}
	if (filter == MICONS_STORE)
	{
		SDL_SetTextureColorMod(tx_cmd, TERM_RGB(TERM_WHITE));
		renderMIcon(size, &pos, MICON_SINGLE_KEY, ' ', '3', spacing);
	}
	
	/* Undo modulation */
	SDL_SetTextureColorMod(tx_cmd, 255, 255, 255);
}
static void drawIconPanel_Direction(SDL_Rect *size, bool allow_target, bool allow_friend)
{
	int i, j;
	int spacing = 16;
	int dirs[10] = { 7, 8, 9, 4, 5, 6, 1, 2, 3 };
	SDL_Rect pos = { 0, 0, 32, 32 };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;
	for (i = 0; i < 9; i++)
	{
		int j = dirs[i];
		int draw = '0' + j;
		renderMIcon(size, &pos, MICON_PICK_DIRECTION, j, draw, spacing);
		if (pos.y >= size->y + size->h) break;
	}
}

static void drawIconPanel_Confirmation(SDL_Rect *size)
{
	int i;
	int spacing = MICON_W * 2;
	int keys[2] = { 'y', 'n' };
	int syms[2] = { MICO_YES, MICO_NO };
	SDL_Rect pos = { 0, 0, MICON_W * 2, MICON_H * 2 };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;
	spacing *= 2;
	for (i = 0; i < 2; i++)
	{
		renderMIcon(size, &pos, MICON_SINGLE_KEY, keys[i], syms[i], spacing);
		if (pos.y >= size->y + size->h) break;
	}
}

static void drawIconPanel_Selector(SDL_Rect *size, int filter)
{
	int imgs[] = { ',', '+', MICO_ONE_ATT, 'i', 'e', MICO_SPELL_BOOK };
	int acts[] = {
		MICONS_QUICKEST,
		MICONS_ALTER,
		MICONS_SYSTEM,
		MICONS_INVEN,
		MICONS_EQUIP,
		MICONS_SPELLS,
	};
	int i;
	int spacing = 8;
	SDL_Rect pos = { 0, 0, 32, 32 };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;
	for (i = 0; i < 6; i++)
	{
		if (acts[i] == micon_command_filter)
		{
			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 210);
			SDL_RenderFillRect(terms[0].renderer, &pos);
		}
		
		renderMIcon(size, &pos, MICON_TOGGLE_ROOT, acts[i], imgs[i], spacing);
		if (pos.y >= size->y + size->h) break;
	}
}

static void drawIconPanel_Inventory(SDL_Rect *size, bool inven, bool equip, bool onfloor)
{
	int i;
	int spacing = 0;
	SDL_Rect pos = { 0, 0, 32, 32 };
	SDL_Rect pre_pos;
	int fw = terms[0].font_data->w;
	int fh = terms[0].font_data->h;
	char numbuf[8] = { 0 };

	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;

	/*
	SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(terms[0].renderer, size);
	*/

	for (i = 0; i < INVEN_WIELD - 1; i++)
	{
		/* Highlight selected item */
		if (term2_item_pref == i)
		{
			SDL_Rect box = { size->x + size->w - 60 * fw, 32+16,
				60 * fw, fh * 2 };
			SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 200);
			SDL_RenderFillRect(terms[0].renderer, &box);
			iconText(box.x+fw, box.y+fh/3, inventory[i].sval, inventory_name[i], FALSE);
			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 210);
		}
		else
		{
			SDL_SetRenderDrawColor(terms[0].renderer, 64, 32, 0, 200);
		}
		SDL_RenderFillRect(terms[0].renderer, &pos);
		
		/* Draw border */
		SDL_SetRenderDrawColor(terms[0].renderer, 127, 64, 0, 255);
		SDL_RenderDrawRect(terms[0].renderer, &pos);
		
		/* Draw inventory item */
		if (inventory[i].number)
		{
			iconPict(&pos, inventory[i].ix, inventory[i].iy, TRUE);
			if (inventory[i].number > 1)
			{
				sprintf(numbuf, "%2d", inventory[i].number);
				iconText(pos.x + pos.w - fw*2, pos.y + pos.h-fh, inventory[i].sval, numbuf, FALSE);
			}
		}
		/* And an actual icon */
		renderMIcon(size, &pos, MICON_SELECT_INVEN, i, -2, spacing);
		if (pos.y >= size->y + size->h) break;
	}

	if ((INVEN_WIELD-1) % 4 != 0)
	{
		pos.y += pos.h;
	}

	/* Add Inven/Equip toggle */
	if (equip || term2_icon_context != MICONS_ITEM)
	{
		pos.x = size->x;
		renderMIcon(size, &pos, MICON_TOGGLE_ROOT, MICONS_EQUIP, 'e', spacing);
	}

	/* Add Floor toggle */
	if (onfloor || term2_icon_context != MICONS_ITEM)
	{
		pos.x = size->x + size->w - pos.w;
		if (term2_item_pref == -11)
		{
			SDL_Rect box = { size->x + size->w - 60 * fw, 32 + 16,
				60 * fw, fh * 2 };
			SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 200);
			SDL_RenderFillRect(terms[0].renderer, &box);
			iconText(box.x+fw, box.y+fh/3, floor_item.sval, floor_name, FALSE);
			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 210);

			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 210);
		}
		else
		{
			SDL_SetRenderDrawColor(terms[0].renderer, 64, 32, 0, 200);
		}
		SDL_RenderFillRect(terms[0].renderer, &pos);
		pre_pos = pos;
		renderMIcon(size, &pos, MICON_SELECT_FLOOR, -11, MICO_FLOOR_ITEM, spacing);
		if (floor_item.tval)
		{
			iconPict(&pre_pos, floor_item.ix, floor_item.iy, TRUE);
		}
		if (floor_item.number > 1)
		{
			sprintf(numbuf, "%2d", floor_item.number);
			iconText(pre_pos.x + pre_pos.w - fw*2, pre_pos.y + pre_pos.h-fh, floor_item.sval, numbuf, FALSE);
		}
	}
}

static void drawIconPanel_Equipment(SDL_Rect *size, bool inven, bool equip, bool onfloor)
{
	IconData *id;
	int i;
	int spacing = 1;
	SDL_Rect pos = { 0, 0, 32, 32 };
	SDL_Rect rsize, xsize;
	int fw = terms[0].font_data->w;
	int fh = terms[0].font_data->h;
	char posbuf[2] = { 0 };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;

	rsize.x = size->x + pos.w/2;
	rsize.y = size->y + pos.h/2;
	rsize.w = size->w - pos.w;
	rsize.h = size->h - pos.h;
	
	SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 200);
	SDL_RenderFillRect(terms[0].renderer, size);
	SDL_RenderDrawRect(terms[0].renderer, size);
	
	SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 200);
	SDL_RenderDrawRect(terms[0].renderer, size);
	SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 100);
	SDL_RenderDrawRect(terms[0].renderer, &rsize);
	
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		float fx = (float)eq_xpos[i] / 256;
		float fy = (float)eq_ypos[i] / 256;
#if 0
		int ix = fx * (rsize.w);
		int iy = fy * (rsize.h);
		pos.x = ix + rsize.x - pos.w/2;
		pos.y = iy + rsize.y - pos.h/2;
#else
		int slotx = fx * 5;
		int sloty = fy * 5;
		pos.x = slotx * pos.w + size->x;
		pos.y = sloty * pos.h + size->y;
#endif

		/* Highlight selected item */
		if (term2_item_pref == i)
		{
			SDL_Rect box = { size->x + size->w - 60 * fw, 32+16,
				60 * fw, fh * 2 };
			SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 200);
			SDL_RenderFillRect(terms[0].renderer, &box);
			iconText(box.x+fw, box.y+fh/3, inventory[i].sval, inventory_name[i], FALSE);

			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 64);
			SDL_RenderFillRect(terms[0].renderer, &pos);
		}
		
		/* Draw inventory item */
		if (inventory[i].number)
		{
			iconPict(&pos, inventory[i].ix, inventory[i].iy, TRUE);
		}
		/* And a "fake" icon */
		renderMIcon(size, &pos, MICON_SELECT_EQUIP, i, -2, spacing);
		//if (pos.y >= size->y + size->h) break;
	}
	
	/* And a giant "fake" icon to prevent click-through */
	xsize = *size;
	renderMIcon(&xsize, &xsize, -1, -1, -1, 0);

	/* Add Inven/Equip toggle */
	pos.x = size->x;
	pos.y += pos.w;
	renderMIcon(size, &pos, MICON_TOGGLE_ROOT, MICONS_INVEN, 'i', spacing);
}

static void drawIconPanel_Spells(SDL_Rect *size, int spell_realm, int spell_book)
{
	int i, sn, idx, icon;
	int spacing = 0;
	SDL_Rect pos = { 0, 0, 32, 32 };
	int fw = terms[0].font_data->w;
	int fh = terms[0].font_data->h;
	char numbuf[8] = { 0 };
	char namebuf[80] = { 0 };

	static char header_sp[80] = "     Name                          Lv Mana Fail";


	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;

	SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(terms[0].renderer, size);

	for (i = 0; i < INVEN_TOTAL; i++)
	{
		if (inventory[i].tval == 0) continue;
		if (spell_book > -1 && spell_book != i) continue;
		for (sn = 0; sn < PY_MAX_SPELLS; sn++)
		{
			SDL_Rect pre_pos;
			if (spell_info[i][sn][0] == '\0') break;

			idx = i * SPELLS_PER_BOOK + sn;

			icon = MICO_SPELL_HAND;
			if (spell_flag[idx] & PY_SPELL_AIM) icon = MICO_SPELL_BEAM;
			if (spell_flag[idx] & PY_SPELL_ITEM) icon = MICO_SPELL_ITEM;
			if (spell_flag[idx] & PY_SPELL_PROJECT) icon = MICO_SPELL_PROJECT;

			if (!(spell_flag[idx] & PY_SPELL_LEARNED)
			 ||  (spell_flag[idx] & PY_SPELL_FORGOTTEN)) icon = MICO_SPELL_UNKNOWN;

			/* Highlight selected spell */
			if (term2_spell_pref == idx)
			{
				SDL_Rect box = { size->x + size->w - 60 * fw, 32,
					60 * fw, fh * 3 };
				SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 200);
				SDL_RenderFillRect(terms[0].renderer, &box);
				iconText(box.x+fw, box.y+fh/3+fh*0, TERM_WHITE, header_sp, FALSE);
				iconText(box.x+fw, box.y+fh/3+fh*1, TERM_WHITE, spell_info[i][sn], FALSE);
				SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 210);			
			}
			else
			{
				SDL_SetRenderDrawColor(terms[0].renderer, 64, 32, 0, 200);
			}
			SDL_RenderFillRect(terms[0].renderer, &pos);

			/* Draw border */
			SDL_SetRenderDrawColor(terms[0].renderer, 127, 64, 0, 255);
			SDL_RenderDrawRect(terms[0].renderer, &pos);

			/* And an actual icon */
			pre_pos = pos;
			renderMIcon(size, &pos, MICON_SELECT_SPELL, idx, icon, spacing);
	
			/* Text on top */
			{
				int max_chars = (pre_pos.w+1) / tiny_font.w;
				my_strcpy(namebuf, spell_info[i][sn] + 5, max_chars);
				iconText(pre_pos.x + 1, pre_pos.y + pos.h - tiny_font.h - 1,
					inventory[i].sval, namebuf, TRUE);
			}
	

			if (pos.y >= size->y + size->h) break;
		}
	}
}

static void drawIconPanel_Slots(SDL_Rect *size)
{
	int i, j;
	int spacing = 16;
	SDL_Rect pos = { 0, 0, 32, 32 };
	int fw = terms[0].font_data->w;
	int fh = terms[0].font_data->h;
	char posbuf[2] = { 0 };
	char numbuf[8] = { 0 };
	pos.x = size->x;
	pos.y = size->y;
	pos.w *= 2;
	pos.h *= 2;
	SDL_SetTextureAlphaMod(terms[0].pict_texture, 255);
	SDL_SetTextureColorMod(terms[0].pict_texture, 255, 255, 255);
	for (i = 0; i < 10; i++)
	{
		SDL_Rect pre_pos;
		SlotData *slot = &quick_slots[i];
		posbuf[0] = '0' + i;
		
		if (dragging_icon)
		{
			if (num_icons == icon_hover)
			{
				SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 190);
			}
		}
		else
		{
			SDL_SetRenderDrawColor(terms[0].renderer, 255, 255, 255, 64);
		}

		SDL_RenderDrawRect(terms[0].renderer, &pos);

		iconText(pos.x+1, pos.y+1, TERM_WHITE, posbuf, FALSE);

		if (slot->a && slot->c) iconPict(&pos, slot->a, slot->c, FALSE);

		pre_pos = pos;
		renderMIcon(size, &pos, MICON_LOCAL_QUICK_SLOT, i, slot->draw, spacing);
		if (slot->draw == MICO_FLOOR_ITEM || slot->draw == 'g')
		{
			if (floor_item.tval)
			{
				iconPict(&pre_pos, floor_item.ix, floor_item.iy, TRUE);
			}
			if (floor_item.number > 1)
			{
				sprintf(numbuf, "%2d", floor_item.number);
				iconText(pre_pos.x + pre_pos.w - fw*2, pre_pos.y + pre_pos.h-fh, floor_item.sval, numbuf, FALSE);
			}
		}
		if (!STRZERO(slot->display))
		{
			iconText(pre_pos.x + 1, pre_pos.y + pos.h - tiny_font.h - 1,
				slot->attr, slot->display, TRUE);
		}
		else if (!STRZERO(slot->macro_item))
		{
			strnfmt(numbuf, 8, "%2d", slot->item_counter);
			iconText(pre_pos.x + pre_pos.w - fw * 2, pre_pos.y + pos.h - fh,
				TERM_WHITE, numbuf, FALSE);
		}

		if (pos.y >= size->y + size->h) break;
	}
}

static void drawIconPanel_PassedMenu(SDL_Rect *size)
{
	int i, j;
	int spacing = MICON_S;
	SDL_Rect pos = { 0, 0, MICON_W, MICON_H };
	int fw = terms[0].font_data->w;
	int fh = terms[0].font_data->h;
#ifdef MOBILE_UI
	if (isHighDPI())
	{
		fw *= 2;
		fh *= 2;
	}
#endif
	pos.x = size->x;
	pos.y = size->y;
	pos.w = 0;
	pos.h = fh + fh / 2;

	SDL_SetRenderDrawColor(terms[0].renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(terms[0].renderer, size);

	for (i = 0; i < micon_passed_menu->count; i++)
	{
		char tmp[8];
		char buf[80];
		char sel = micon_passed_menu->selections[i];
		micon_passed_menu_resolver(buf, 80, micon_passed_menu, i);
		strnfmt(tmp, 4, "%c) ", sel);
		if (STRZERO(buf)) tmp[0] = '\0';
		iconText(pos.x, pos.y + pos.h / 2 - fh / 2, TERM_WHITE, tmp, -2);
		iconText(pos.x + fw * 3, pos.y + pos.h / 2 - fh / 2, TERM_WHITE, buf, -2);
		pos.w = 13 * fw;
		renderMIcon(size, &pos, MICON_SINGLE_KEY, sel, -1, spacing);
		if (pos.y >= size->y + size->h) break;
	}
}

#endif /* USE_ICON_OVERLAY */



/* ==== Configuration loading/saving functions ==== */
static errr loadConfig()
{
	char section[128];
	cptr value;
	int window_id;

	int f;
	for (f = 0; f < MAX_QUICK_FONTS; f++)
	{
		char fbuf[8];
		strnfmt(fbuf, 8, "%d", f);
		value = conf_get_string("SDL-Fonts", fbuf, NULL);
		if (value) {
			strcpy(quick_font_names[f], value);
		} else {
			break;
		}
	}

	/* Read font directory */
	if (f == 0)
	{
		sdl_font_read_dir(NULL, quick_font_names, MAX_QUICK_FONTS);
	}

	/* Assume quickfont-0 is our default font. (for now) */
	if (f > 0)
	{
		strcpy(default_font, quick_font_names[0]);
		strcpy(default_font_small, quick_font_names[0]);
	} else {
		/* User has ZERO quick fonts, revert to hardcoded defaults */
		strcpy(default_font, "nethack10x19-10.hex");
		strcpy(default_font_small, "misc6x13.hex");
	}
	/* Assume quickfont-1 is our default large font. (for now) */
	if (f > 1)
	{
		strcpy(default_font, quick_font_names[1]);
	}

	value = conf_get_string("SDL2", "FontFile", default_font);
	if (strcmp(default_font, value) != 0) {
		strcpy(default_font, value);
	}

	default_font_size = conf_get_int("SDL2", "FontSize", default_font_size);

	use_sound = (bool)conf_get_int("SDL2", "Sound", 1);

	sdl_combine_arrowkeys = (bool)conf_get_int("SDL2", "CombineArrows", 1);
	sdl_combiner_delay = conf_get_int("SDL2", "CombineArrowsDelay", 50);
	sdl_game_mouse = (bool)conf_get_int("SDL2", "GameMouse", 1);

	for (window_id = 0; window_id < TERM_MAX; window_id++)
	{
		graphics_mode* gm = get_graphics_mode((byte)use_graphics);
		strnfmt(section, 128, "SDL2-Window-%d", window_id);
		strncpy(terms[window_id].title, conf_get_string(section, "Title", ""), 128);
		my_strcpy(terms[window_id].pict_file, gm->file, 128);
		my_strcpy(terms[window_id].mask_file, gm->mask, 128);
		strncpy(terms[window_id].font_file, conf_get_string(section, "FontFile", ""), 128);
		terms[window_id].font_size = conf_get_int(section, "FontSize", 0);

		value = conf_get_string(section, "PictMode", "static");
		if (strcmp(value, "static") == 0)
		{
			terms[window_id].pict_mode = TERM_PICT_STATIC;
		}
		else if (strcmp(value, "stretch") == 0)
		{
			terms[window_id].pict_mode = TERM_PICT_STRETCH;
		}
		else if (strcmp(value, "scale") == 0)
		{
			terms[window_id].pict_mode = TERM_PICT_SCALE;
		}

		value = conf_get_string(section, "FontSmoothing", "true");
		if (strcmp(value, "true") == 0)
		{
			terms[window_id].config |= TERM_FONT_SMOOTH;
		}

		value = conf_get_string(section, "CharMode", "static");
		if (strcmp(value, "static") == 0)
		{
			terms[window_id].char_mode = TERM_CHAR_STATIC;
		}
		else if (strcmp(value, "stretch") == 0)
		{
			terms[window_id].char_mode = TERM_CHAR_STRETCH;
		}
		else if (strcmp(value, "scale") == 0)
		{
			terms[window_id].char_mode = TERM_CHAR_SCALE;
		}

		terms[window_id].orig_w = conf_get_int(section, "CellWidth", 0);
		terms[window_id].orig_w = conf_get_int(section, "CellHeight", 0);

		value = conf_get_string(section, "CellMode", "font");
		if (strcmp(value, "pict") == 0)
		{
			terms[window_id].cell_mode = TERM_CELL_PICT;
		}
		else if (strcmp(value, "font") == 0)
		{
			terms[window_id].cell_mode = TERM_CELL_FONT;
		}
		else if (strcmp(value, "custom") == 0)
		{
			terms[window_id].cell_mode = TERM_CELL_CUST;
		}

		if (!conf_get_int(section, "Visible", window_id < 5 ? 1 : 0) && window_id)
		{
			terms[window_id].config |= TERM_IS_HIDDEN;
		}

#ifdef MOBILE_UI
		/* Hack -- hide all other windows on mobile */
		if (window_id > 0)
		{
			terms[window_id].config |= TERM_IS_HIDDEN;
			terms[window_id].config |= TERM_IS_VIRTUAL;
		}
#endif

		terms[window_id].zoom = conf_get_int(section, "Zoom", 100);
		if (terms[window_id].zoom < 1) terms[window_id].zoom = 1;
		if (terms[window_id].zoom > 1000) terms[window_id].zoom = 1000;

		if (conf_get_int(section, "AltDungeon", 1))
		{
			terms[window_id].config |= TERM_DO_SCALE;
		}
		else
		{
			terms[window_id].zoom = 100;
		}

		if (conf_get_int(section, "Virtual", 1))
		{
			terms[window_id].ren_rect.x = conf_get_int(section, "X", -1);
			terms[window_id].ren_rect.y = conf_get_int(section, "Y", -1);
			terms[window_id].ren_rect.w = conf_get_int(section, "Width", 0);
			terms[window_id].ren_rect.h = conf_get_int(section, "Height", 0);

			terms[window_id].config |= TERM_IS_VIRTUAL;
		}
		else
		{
			terms[window_id].x = conf_get_int(section, "X", -1);
			terms[window_id].y = conf_get_int(section, "Y", -1);
			terms[window_id].width = conf_get_int(section, "Width", 0);
			terms[window_id].height = conf_get_int(section, "Height", 0);
			terms[window_id].ren_rect.x = 0;
			terms[window_id].ren_rect.y = 0;
			terms[window_id].ren_rect.w = terms[window_id].width;
			terms[window_id].ren_rect.h = terms[window_id].height;
		}

		if (window_id == TERM_MAIN)
		{
			graphics_mode* gm;

			terms[window_id].x = conf_get_int("SDL2", "X", -1);
			terms[window_id].y = conf_get_int("SDL2", "Y", -1);
			terms[window_id].width = conf_get_int("SDL2", "Width", 0);
			terms[window_id].height = conf_get_int("SDL2", "Height", 0);

			terms[window_id].config |= TERM_IS_VIRTUAL;

#ifdef MOBILE_UI
			strcpy(default_font, "misc6x13.hex");
			use_graphics = 3;
			gm = get_graphics_mode((byte)use_graphics);
			strcpy(terms[window_id].font_file, default_font);
			strcpy(terms[window_id].pict_file, gm->file);
			terms[window_id].x = 0;
			terms[window_id].y = 0;
			terms[window_id].width = 0;
			terms[window_id].height = 0;
			terms[window_id].cols = 0;
			terms[window_id].rows = 0;
			if (use_graphics) terms[window_id].config |= TERM_DO_SCALE;
			else terms[window_id].config &= ~TERM_DO_SCALE;
			terms[window_id].zoom = 100;
#endif
		}
	}

	return 0;
}
static errr saveConfig()
{
	char section[128];
	int window_id;

	conf_set_string("SDL2", "FontFile", default_font);
	conf_set_int("SDL2", "FontSize", default_font_size);

	conf_set_int("SDL2", "Graphics", use_graphics);
	conf_set_int("SDL2", "Sound", use_sound);

	conf_set_int("SDL2", "CombineArrows", sdl_combine_arrowkeys);
	conf_set_int("SDL2", "CombineArrowsDelay", sdl_combiner_delay);
	conf_set_int("SDL2", "GameMouse", sdl_game_mouse);

	for (window_id = 0; window_id < TERM_MAX; window_id++)
	{
		strnfmt(section, 128, "SDL2-Window-%d", window_id);
		conf_set_string(section, "Title", terms[window_id].title);

		conf_set_string(section, "FontFile", terms[window_id].font_file);
		conf_set_int(section, "FontSize", terms[window_id].font_size);

		conf_set_int(section, "Zoom", terms[window_id].zoom);
		if (!window_id)
			conf_set_int(section, "AltDungeon", (terms[window_id].config & TERM_DO_SCALE));

		if (terms[window_id].pict_mode == TERM_PICT_STATIC)
		{
			conf_set_string(section, "PictMode", "static");
		}
		else if (terms[window_id].pict_mode == TERM_PICT_STRETCH)
		{
			conf_set_string(section, "PictMode", "stretch");
		}
		else if (terms[window_id].pict_mode == TERM_PICT_SCALE)
		{
			conf_set_string(section, "PictMode", "scale");
		}

		conf_set_string(section, "FontSmoothing", (terms[window_id].config & TERM_FONT_SMOOTH) ? "true" : "false");

		if (terms[window_id].char_mode == TERM_CHAR_STATIC)
		{
			conf_set_string(section, "CharMode", "static");
		}
		else if (terms[window_id].char_mode == TERM_CHAR_STRETCH)
		{
			conf_set_string(section, "CharMode", "stretch");
		}
		else if (terms[window_id].char_mode == TERM_CHAR_SCALE)
		{
			conf_set_string(section, "CharMode", "scale");
		}

		conf_set_int(section, "CellWidth", terms[window_id].orig_w);
		conf_set_int(section, "CellHeight", terms[window_id].orig_w);

		if (terms[window_id].cell_mode == TERM_CELL_PICT)
		{
			conf_set_string(section, "CellMode", "pict");
		}
		else if (terms[window_id].cell_mode == TERM_CELL_FONT)
		{
			conf_set_string(section, "CellMode", "font");
		}
		else if (terms[window_id].cell_mode == TERM_CELL_CUST)
		{
			conf_set_string(section, "CellMode", "custom");
		}

		conf_set_int(section, "Visible", terms[window_id].config & TERM_IS_HIDDEN ? 0 : 1);
		//conf_set_int(section, "Visible", (terms[window_id].config & TERM_IS_ONLINE) ? 0 : 1);

		if (terms[window_id].config & TERM_IS_VIRTUAL || window_id == TERM_MAIN)
		{
			conf_set_int(section, "Virtual", 1);

			conf_set_int(section, "X", terms[window_id].ren_rect.x);
			conf_set_int(section, "Y", terms[window_id].ren_rect.y);
			conf_set_int(section, "Width", terms[window_id].ren_rect.w);
			conf_set_int(section, "Height", terms[window_id].ren_rect.h);

		}
		else
		{
			conf_set_int(section, "Virtual", 0);

			conf_set_int(section, "X", terms[window_id].x);
			conf_set_int(section, "Y", terms[window_id].y);
			conf_set_int(section, "Width", terms[window_id].width);
			conf_set_int(section, "Height", terms[window_id].height);
		}
	}

	conf_set_int("SDL2", "X", terms[TERM_MAIN].x);
	conf_set_int("SDL2", "Y", terms[TERM_MAIN].y);
	conf_set_int("SDL2", "Width", terms[0].width);
	conf_set_int("SDL2", "Height", terms[0].height);

	return 0;
}

/* ==== Font-related functions ==== */
/* cleanFontData
This function takes the FontData, destroys its SDL_Texture (if it exists),
nulls it, then sets its entire struct to 0.
*/
static errr cleanFontData(FontData *fd) {
	if (fd->surface) SDL_FreeSurface(fd->surface);
	fd->surface = NULL;
	if (fd->filename) string_free(fd->filename);
	fd->filename = NULL;

	memset(fd, 0, sizeof(FontData));
	return 0;
}
static errr fileToFont(FontData *fd, cptr filename, int fontsize, int smoothing)
{
	SDL_Rect info;
	SDL_Surface *surface;

	if (fd->w || fd->h || fd->surface) return 1; // Return if FontData is not clean

	surface = sdl_font_load(filename, &info, fontsize, smoothing);

	if (surface == NULL) return 1;

	fd->filename = string_make(filename);

	fd->w = info.w;
	fd->h = info.h;

	fd->scalable = 0;
	if (isuffix(filename, ".ttf") || isuffix(filename, ".otf"))
	{
		fd->scalable = 1;
	}

	/* Convert to 32bpp surface */
	fd->surface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0);
	SDL_BlitSurface(surface, NULL, fd->surface, NULL);
	//SDL_SaveBMP(surface, "old_font.bmp");//debug
	//SDL_SaveBMP(surface, "new_font.bmp");//debug
	SDL_FreeSurface(surface);

	return 0;
}

/* ==== Pict-related functions ==== */
static errr cleanPictData(PictData *pd)
{
	if (pd->surface) SDL_FreeSurface(pd->surface);
	pd->surface = NULL;
	if (pd->filename) string_free(pd->filename);
	pd->filename = NULL;
	memset(pd, 0, sizeof(PictData));
	return 0;
}

static errr imgToPict(PictData *pd, cptr filename, cptr maskname)
{
	SDL_Rect glyph_info;
	Uint32 width, height;
	char buf[1036];
	char *image_error;
	if (pd->w || pd->h || pd->surface) return 1; // return if PictData is unclean

	// Get and open our image from the xtra dir
	//path_build(buf, 1024, ANGBAND_DIR_XTRA, filename);

	// Load 'er up
	sdl_graf_prefer_rgba = TRUE;
	pd->surface = sdl_graf_load(filename, &glyph_info, maskname);
	if (pd->surface == NULL)
	{
		//plog_fmt("imgToPict: %s", "can't load font");
		return 1;
	}
	// Cool, get our dimensions
	width = glyph_info.w;
	height = glyph_info.h;
	if (width == 0 || height == 0)
	{
		plog_fmt("imgToPict: %s", "width or height of 0 is not allowed!");
		SDL_FreeSurface(pd->surface);
		pd->surface = NULL;
		return 3;
	}

	// don't forget our colorkey
	//TODO: grafmode.transperancy is a better check
	if (maskname)
	{
		SDL_SetColorKey(pd->surface, SDL_TRUE, 0);
	}

	// set up our PictData
	pd->w = width; pd->h = height;

	// save filename
	pd->filename = string_make(filename);

	return 0;
}

#endif /* USE_SDL2 */
