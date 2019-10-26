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
#define snprintf _snprintf
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


#define MAX_QUICK_FONTS 16
static char quick_font_names[MAX_QUICK_FONTS][128] = {
	"nethack10x19-10.hex",
	"misc6x13.hex",
	""
};

#define MAX_FONT_SIZES 24
const int ttf_font_sizes[MAX_FONT_SIZES] = {
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 18, 20, 24, 30, 32, 36, 48, 0
};


static cptr GFXBMP[] = { "8x8.png", "8x8.png", "16x16.png", "32x32.png", 0 };
static cptr GFXMASK[] = { 0, 0, "mask.bmp", "mask32.bmp", 0 };
static cptr GFXNAME[] = { 0, "old", "new", "david", 0 };
#define MAX_QUICK_TILESETS 3

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
static void mobileAutoLayout(void)
{
	TermData *td = &terms[0];
	int maxw, maxh;
	int w = td->font_data->w;
	int h = td->font_data->h;

	getRendererSize(td, &maxw, &maxh);

	maxh -= 32;
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
	use_graphics = conf_get_int("SDL2", "Graphics", 0);
	if (use_graphics)
	{
		ANGBAND_GRAF = GFXNAME[use_graphics];
	}
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

	// **** Activate Main z-term and gooo ****
	Term_activate(&(terms[TERM_MAIN].t)); // set active Term to terms[TERM_MAIN]
	term_screen = Term;                   // set term_screen to terms[TERM_MAIN]

	/** Activate Term-2 hooks **/
	cave_char_aux = Term2_cave_char;
	query_size_aux = Term2_query_area_size;
	refresh_char_aux = Term2_refresh_char;
	screen_keyboard_aux = Term2_screen_keyboard;

	return 0;
}

/* Our de-initializer */
void quit_sdl2(cptr s)
{
	/* save all values */
	saveConfig();

	/* Call regular quit hook */
	quit_hook(s);

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
			net_term_update(TRUE);
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
	net_term_update(TRUE);

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
	plog_fmt("loadFont(): %s", font_error, NULL);
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
	}

	/* Render all terms */
	for (i = 0; i < TERM_MAX; i++)
	{
		TermData *td = &terms[i]; /* or TERM_MAX - 1 - i to invert */
		if (!(td->config & TERM_IS_ONLINE)) continue;
		if (td->window_id != mtd->window_id) continue;

		renderTerm(td);
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
		if (section_icky_row) return 0;
		if (section_icky_col) return 0;
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

		if (section_icky_row > 0 && y < section_icky_row)
			continue;

		for (x = sx; x < td->cols; x++)
		{
			if (section_icky_col > 0 && x > section_icky_col)
				continue;

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
			terms[i].need_cutout = 0;
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
			for (j = 0; 0 < MAX_QUICK_TILESETS; j++) {
				char buf[32], on;
				if (j >= 4) break;
				if (j && GFXNAME[j] == 0) break;
				on = 0;
				if (j && !strcmp(GFXBMP[j], td->font_file)) on = 1;
				if (!j && use_graphics == 0) on = 1;
				if (j == use_graphics) on = 1;
				strnfmt(buf, 32, " [%c] %-12s ", (on ? 'X' : '.'),
				j ? GFXNAME[j] : "Off");
				renderTButton(td, sx, sy + j, buf, MENU_QUICK_GRAF0 + j);
			}
			td->menu_rect.h += j * grid_h;
			sub_w = 18;
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
	if (i == TERM_MAIN)
	{
		int button = 0;
		int cx = (wx - terms[i].ren_rect.x) / terms[i].cell_w;
		int cy = (wy - terms[i].ren_rect.y) / terms[i].cell_h;
		if (terms[i].config & TERM_DO_SCALE)
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

	if (menu_mode)
	{
		bool handled = handleMouseEvent_Menu(ev);
		if (handled) return;	
	}

	eventMouseCoordinates(ev, &wx, &wy);

	if (ev->type == SDL_MOUSEBUTTONDOWN)
	{
		
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

/* Options: */
static bool ignore_keyboard_layout = FALSE;
static bool collapse_numpad_keys = FALSE;
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
}

static void handleKeyboardEvent(SDL_Event *ev)
{
	/* Handle text input */
	if (ev->type == SDL_TEXTINPUT)
	{
		/* This is a utf-8 string */
		char *c = ev->text.text;
		/* Hack -- if we ignore_keyboard_layout, don't use textinput */	
		if (ignore_keyboard_layout) c = NULL;
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
		Term_keypress(ESCAPE);
		return;
	}

	/* Key released */
	if (ev->type == SDL_KEYUP && ev->key.state == SDL_RELEASED)
	{
		int key = ev->key.keysym.sym;
		/* Track modifier keypresses , for mouse sake */
		if (key >= SDLK_LCTRL && key <= SDLK_RGUI)
		{
			Noticemodkeypress(key, 0);
			return;
		}
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
		}

		/* ASCII */
		if (key <= 127)
		{
			/* Space is already handled by text input, ignore */
			if (!ignore_keyboard_layout && key == ' ' && mod == 0) return;

			/* Ignore modifier keypresses */
			if (key >= SDLK_LCTRL && key <= SDLK_RGUI) return;

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
				    "",
				    ev->key.keysym.mod & KMOD_ALT   ? "M" : "",
				    (unsigned long) key, 13);
#ifdef DEBUG
			printf("Macro: ^_%s%s%s%s_%lX\\r%s\n",
				    ev->key.keysym.mod & KMOD_CTRL  ? "N" : "",
				    ev->key.keysym.mod & KMOD_SHIFT ? "S" : "",
				    "",
				    ev->key.keysym.mod & KMOD_ALT   ? "M" : "",
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
		//return (Term_xtra_win_noise());
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
	SDL_SetRenderDrawColor(td->renderer, 128, 255, 64, 255);
	SDL_RenderDrawRect(td->renderer, &cell_rect);
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
		td->need_cutout = TRUE;
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

	SDL_SetTextureColorMod(td->font_texture, color_table[attr][1], color_table[attr][2], color_table[attr][3]);

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
	int i, alt_dungeon = 0;
	TermData *td = (TermData*)(Term->data);
	struct FontData *fd = td->font_data;
	SDL_SetTextureColorMod(td->font_texture, color_table[attr][1], color_table[attr][2], color_table[attr][3]);

	SDL_SetRenderTarget(td->renderer, td->framebuffer);
	SDL_SetRenderDrawColor(td->renderer, 255, 255, 255, 255);
	SDL_SetRenderDrawBlendMode(td->renderer, SDL_BLENDMODE_NONE);

	for (i = 0; i < n; i++)
	{
		wipeTermCell_UI(x + i, y, 0);
		textTermCell_Char(x + i, y, attr, s[i]);
	}

	if (td->config & TERM_DO_SCALE)
	{
		td->need_cutout = TRUE;
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
	}

	if (td->config & TERM_DO_SCALE)
	{
		td->need_cutout = TRUE;
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
		use_graphics = f;
//		unloadFont(&terms[i]);
		unloadPict(&terms[i]);
		strcpy(td->pict_file, GFXBMP[f]);
		strcpy(td->mask_file, GFXMASK[f] ? GFXMASK[f] : "");
		applyTermConf(&terms[i]);
		ANGBAND_GRAF = GFXNAME[f];
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
	}
	return 0;
}


static errr sysText(TermData *td, int x, int y, int n, byte attr, cptr s)
{
	int i;
	struct FontData *fd = td->font_data;
	SDL_SetTextureColorMod(td->font_texture, color_table[attr][1], color_table[attr][2], color_table[attr][3]);

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

	for (window_id = 0; window_id < TERM_MAX; window_id++)
	{
		strnfmt(section, 128, "SDL2-Window-%d", window_id);
		strncpy(terms[window_id].title, conf_get_string(section, "Title", ""), 128);
		my_strcpy(terms[window_id].pict_file, GFXBMP[use_graphics], 128);
		my_strcpy(terms[window_id].mask_file, GFXMASK[use_graphics] ? GFXMASK[use_graphics] : "", 128);
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

			terms[window_id].x = conf_get_int("SDL2", "X", -1);
			terms[window_id].y = conf_get_int("SDL2", "Y", -1);
			terms[window_id].width = conf_get_int("SDL2", "Width", 0);
			terms[window_id].height = conf_get_int("SDL2", "Height", 0);

			terms[window_id].config |= TERM_IS_VIRTUAL;

#ifdef MOBILE_UI
			strcpy(default_font, "misc6x13.hex");
			use_graphics = 3;
			strcpy(terms[window_id].font_file, default_font);
			strcpy(terms[window_id].pict_file, GFXBMP[use_graphics]);
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

	for (window_id = 0; window_id < TERM_MAX; window_id++)
	{
		strnfmt(section, 128, "SDL2-Window-%d", window_id);
		conf_set_string(section, "Title", terms[window_id].title);

		conf_set_string(section, "FontFile", terms[window_id].font_file);
		conf_set_int(section, "FontSize", terms[window_id].font_size);

		conf_set_int(section, "Zoom", terms[window_id].zoom);
		if (window_id)
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
