/* File: main-sdl.c */

/* Purpose: SDL visual module for Angband 2.7.9 through 2.9.2 */

/* Most of the code in this file (all not appearing in main-xxx.c) is
 * Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
 * Updated to handle multiple "terminals" via same window by flambard 
 * You may use it under the terms of the standard Angband license (below).
 *
 * The Angband license states:
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
*/



/*
 * This file originally written by "Ben Harrison (benh@voicenet.com)".
 *
 */
#ifdef USE_SDL
#ifndef SDL_HEADER

/* Options */
#undef SINGLE_SURFACE /* Eat more CPU, but less RAM (only signifcant when drawing GUI) */
#define SHADE_FONTS /* Allow fonts with multiple colors */

#include "c-angband.h"

bool need_render = FALSE;	/* very important -- triggers frame redrawing */

bool mouse_mode = FALSE;	/* pass mouse events to Angband !!! */

bool quartz_hack = FALSE; /* Enable special mode on OSX */

#include <SDL/SDL.h>
#include <string.h>

#include "sdl-font.h"
#include "sdl-sound.h"

/* this stuff was moved to sdl-maim.c */
extern SDL_Surface *SDL_ScaleTiledBitmap (SDL_Surface *src, Uint32 t_oldw, Uint32 t_oldh, Uint32 t_neww, Uint32 t_newh, int dealloc_src);
extern SDL_Surface* SurfaceTo8BIT(SDL_Surface *face, int free_src);
extern char *formatsdlflags(Uint32 flags);
extern void Multikeypress(char *k);
extern char *SDL_keysymtostr(SDL_keysym *ks); /* this is the important one. */

/* this is for arrow combiner: */
extern int sdl_combine_arrowkeys;
extern int sdl_combiner_delay;
extern Uint32 event_timestamp;
extern void Flushdelayedkey(bool execute, bool force_flush, SDLKey key, Uint32 new_timestamp);



/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 *
 * A font_data struct keeps the SDL_Surface and other info for tile graphics
 * which include fonts.
 */

typedef struct font_data font_data; /* must be here to avoid fwd. ref. */

struct font_data
{
	SDL_Surface *face;
	cptr name;
	/*
	 * Font metrics.
	 * Obviously, the font system is very minimalist.
	 */
	Uint8 w;
	Uint8 h;

#ifdef SHADE_FONTS
	/* Our color ramp */
	SDL_Color ramp[255];
#endif

	/* TODO: actual precolorization?! */
	Uint8 precolorized;
};


/* tile graphics are very very similar to bitmap fonts. */
typedef struct font_data graf_tiles;


typedef struct term_data term_data;

struct term_data
{
	term		t;

	cptr		name;

	SDL_Surface *face;

	bool		online;

	int		rows;
	int		cols;

	Uint32 width, height;

	font_data	*fd;
	font_data	sfd;

	graf_tiles	*gt;
	graf_tiles	sgt;

	Uint8 w, h; /* width and height of an individual 2D element */

	Sint32 cx, cy; /* last known cursor coordinates */

	Uint32 xoff, yoff; /* position on parent window */

	bool prefer_fresh ; /* in case we don't implement FROSH in a graphics engine */
	bool cursor_on;

	/* hooks */
	void (*init_hook)(term *t);
	void (*nuke_hook)(term *t);

	errr (*user_hook)(int n); /* perhaps this useless function will be used */
	errr (*xtra_hook)(int n, int v);
	errr (*curs_hook)(int x, int y);
	errr (*wipe_hook)(int x, int y, int n);
	errr (*text_hook)(int x, int y, int n, byte a, cptr s);
	errr (*pict_hook)(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);
};

static term_data tdata[ANGBAND_TERM_MAX];


/* XXX XXX SDL surface + window size and params */
SDL_Surface *bigface;
Uint32 width, height, bpp, flags;
bool fullscreen;

/* Cursor surface */
SDL_Surface *sdl_screen_cursor = NULL;
SDL_Rect sdl_screen_cursor_sr;

/*
 * Mini-Gui variables 
 */
term_data *sel_term;		/* pointer to Selected term */
int m_term = -1;			/* index of Selected term */
int m_subterm = -1;		/* index of last Selected term */
int mx, my;					/* mouse X, mouse Y */
int tmx = 0, tmy = 0; /* snapping tresholders */
bool m_shift = FALSE; /* is SHIFT button pressed? */
bool m_control = FALSE; /* is CTRL butto pressed? */
bool m_rescaled = FALSE; /* was RESCALING performed? */
bool m_resized = FALSE; /* was RESIZING performed? */
bool m_moved = FALSE; /* was MOVEMENT performed? */
/* Gui colors */
Uint32 gui_color_back_ground;
Uint32 gui_color_back_active;
Uint32 gui_color_term_border;
Uint32 gui_color_term_header;
Uint32 gui_color_term_title;

/* Prototypes */
bool term_set_font(int i, cptr fontname);
bool init_one_term(int i, bool force);
void term_rescale(int i, bool create, bool redraw);
bool term_spawn();
void term_open(int i);
void term_close(int i);
void term_redraw(int i);
void term_display_all();
void term_cursor(int i);
void term_unload_ptr(term_data *td);

void save_sdl_prefs();


/* color data copied straight from main-xxx.c */
static SDL_Color color_data_sdl[16] =
{
	{0, 0, 0, 0},
	{4, 4, 4, 0},
	{2, 2, 2, 0},
	{4, 2, 0, 0},
	{3, 0, 0, 0},
	{0, 2, 1, 0},
	{0, 0, 4, 0},
	{2, 1, 0, 0},
	{1, 1, 1, 0},
	{3, 3, 3, 0},
	{4, 0, 4, 0},
	{4, 4, 0, 0},
	{4, 0, 0, 0},
	{0, 4, 0, 0},
	{0, 4, 4, 0},
	{3, 2, 1, 0}
};

/*
 * On functions in this (and maim-sdl.c) file:
 * SDL_XXX			-- perform actual blitting, scaling, colorizing, etc (eg: SDL_BlitChar draws A/C)
 *	Term_sdl_XXX 	-- ZTerm wrappers for the above (eg: Term_char_sdl calls SDL_BlitChar)
 * term_XXX			-- perform interal operations on terms, such as showing, hiding, restacking, rescaling, (un)loading fonts
 * ang_mouse_XXX		-- transform raw SDL events into Angband mouse events
 * gui_term_XXX	-- hooks for WYSIWYG term editor
 * init_XXX			-- standart init fare, similar to other "main-xxx" files
 */


/*
 * XXX XXX XXX
 *           Helper functions!
 *
 *	Despite their name, they take a large bulk of the file and do lots of important operations,
 * espesially the load_HEX_font and load_BMP_graf routines.
 *
 */

/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
int color_char_to_attr(char c)
{
	switch (c)
	{
		case 'd': return (TERM_DARK);
		case 'w': return (TERM_WHITE);
		case 's': return (TERM_SLATE);
		case 'o': return (TERM_ORANGE);
		case 'r': return (TERM_RED);
		case 'g': return (TERM_GREEN);
		case 'b': return (TERM_BLUE);
		case 'u': return (TERM_UMBER);

		case 'D': return (TERM_L_DARK);
		case 'W': return (TERM_L_WHITE);
		case 'v': return (TERM_VIOLET);
		case 'y': return (TERM_YELLOW);
		case 'R': return (TERM_L_RED);
		case 'G': return (TERM_L_GREEN);
		case 'B': return (TERM_L_BLUE);
		case 'U': return (TERM_L_UMBER);
	}
	return (-1);
}
/* Return an SDL-ready color from R G B */
Uint32 sdl_shift_color(Uint32 R, Uint32 G, Uint32 B)
{
	return (R << 16) | (G << 8) | B;
}
/* Return an SDL-ready int color from SDL_Color */
Uint32 sdl_shift_color2(SDL_Color *c)
{
	return sdl_shift_color(c->r, c->g, c->b);
}
/* Return an SDL-ready color from string
 * Accepted formats are "16711833", "0xFF0099", "#FF0099"
 *  for 16711833
 */
Uint32 sdl_string_color(cptr str)
{
	char *pt;
	SDL_Color col;
	Uint32 tmp;

	/* Hex (HTML style) */
	if (str[0] == '#')
	{
		tmp = strtol(str+1, &pt, 16);
	}
	/* Hex (C style) */
	else if (strlen(str) > 1 && str[0] == '0' && str[1] == 'x')
	{
		tmp = strtol(str+2, &pt, 16);
	}
	/* MAngband ('w') */
	else if (strlen(str) == 1 && isalpha(str[0]))
	{
		tmp = sdl_shift_color2(&(color_data_sdl[(byte)color_char_to_attr(str[0]) & 0xf]));
	}
	/* Int */
	else
	{
		tmp = strtol(str, &pt, 10);
	}

	col.r = (tmp >> 16) & 0xFF;
	col.g = (tmp >> 8 ) & 0xFF;
	col.b = (tmp >> 0 ) & 0xFF;

	return SDL_MapRGB(bigface->format, col.r, col.g, col.b);
}
/*
void sdl_copy_rect(SDL_Rect *sr, SDL_Rect *dr) {
	dr->x = sr->x;
	dr->y = sr->y;
	dr->w = sr->w;
	dr->h = sr->h;
}*/
/* Do not use more then 1 quick_rect in one statement! */
SDL_Rect *sdl_quick_rect(int x, int y, int w, int h) {
	static SDL_Rect lr;
	lr.x = x;
	lr.y = y;
	lr.w = w;
	lr.h = h;
	return &(lr);
}

/* Config system extension: find next font listed in file, 
	using terminals's loaded font name as refrence point */
cptr conf_find_font(term_data *td) {
	static char fontname[100];
	char buf[10];
	int i = 0, j = -1;
	cptr ret = fontname; 
	buf[0] = fontname[0] = '\0';
	
	if (!conf_section_exists("SDL-Fonts")) {
		char fonts[24][1024];
		int n, k;
		/* Read fonts from dir */
		n = sdl_font_read_dir(NULL, fonts, 24);
		for (k = 0; k < n; k++) {
			sprintf(buf, "%d", k);
			conf_set_string("SDL-Fonts", buf, fonts[k]);
		}
		if (n == 0) {
			/* Final fallback */
			conf_set_string("SDL-Fonts", "0", "misc6x13.hex");
			conf_set_string("SDL-Fonts", "1", "nethack10x19-10.hex");
		}
	}

	do
	{
		sprintf(buf, "%d", i);	
		/* attempt to read next in conf */
		strcpy(fontname, conf_get_string("SDL-Fonts", buf, "\0"));

		/* it's the same as current */
		if (!my_stricmp(fontname, td->fd->name))
		{
			j = i + 1;
			break;
		}
		i++;
	} while (fontname[0] != '\0');

	fontname[0] = '\0';
	if (j != -1)
	{
		/* attempt to read font j */
		sprintf(buf, "%d", j);
		strcpy(fontname, conf_get_string("SDL-Fonts", buf, "\0"));
	}
	/* fail? try font 0 */
	if (fontname[0] == '\0') {
		sprintf(buf, "%d", 0);
		strcpy(fontname, conf_get_string("SDL-Fonts", buf, "\0"));
		if (fontname[0] == '\0') {
			/* full failure */
			return "nethack10x19-10.hex";
		}
	}
	return ret;
}
/* Find a terminal at X Y mouse position */
int pick_term(int x, int y)
{
	int r = -1;
	int i = 0;
	for (i = ANGBAND_TERM_MAX - 1; i > -1; i--)
	{
		if (!tdata[i].online) continue;

		if (x >= tdata[i].xoff && x <= tdata[i].xoff + tdata[i].width &&
			 y >= tdata[i].yoff && y <= tdata[i].yoff + tdata[i].height)
		{
			r = i;
			break;
		}
	}
	return r;
}



/*
 * Load a tileset.
 *
 * If maskname != NULL, a second file with mask will be loaded,
 * and the tileset will be recolored to use it as the colorkey.
 *
 */
errr load_ANY_graf_sdl(font_data *fd, cptr filename, cptr maskname)
{
	SDL_Rect glyph_info;

	if ((fd->face = sdl_graf_load(filename, &glyph_info, maskname)) != NULL)
	{
		/* Attempt to get dimensions from filename */
		fd->w = glyph_info.w;
		fd->h = glyph_info.h;
	}
	else
	{
		return 1;
	}

	return 0;
}

/*
 * Load SDL font.
 * "load_HEX_font_sdl" was moved to sdl-font.c
 */
errr load_ANY_font_sdl(font_data *fd, cptr filename)
{
#ifdef SHADE_FONTS
	int i;
#endif
	SDL_Rect info;
	SDL_Surface *face;

	/* check font_data */
	if (fd->w || fd->h || fd->face) return -1; /* dealloc it first, dummy. */

	face = sdl_font_load(filename, &info, 16, 0);

	if (!face)
	{
		return -1;
	}

#ifdef SHADE_FONTS
	if (!face->format->palette)
	{
		face = SurfaceTo8BIT(face, 1);
	}
	for (i = 0; i < face->format->palette->ncolors; i++)
	{
		fd->ramp[i] = face->format->palette->colors[i];
	}
#endif

	fd->face = face;
	fd->w = info.w;
	fd->h = info.h;
	return 0;
}

/* Helper functions end here. Let's do some GUI functions now */


/* Grab term (+LMB) */
bool gui_term_grab(int i) {
	if (i == -1) return FALSE;
	m_term = i;
	m_subterm = -1;
	sel_term = &tdata[i];
	need_render = TRUE; /* highlight our selection */
	return TRUE;
}
/* Drag term (MM) */
bool gui_term_drag(int nmx, int nmy) {
	int mx = 0, my = 0, hold_x = 0, hold_y = 0;

	if (m_term == -1) return FALSE;

	/* Thresholder */
	hold_x = sel_term->w,
	hold_y = sel_term->h;

	if (m_control && !m_shift) {
		if (sel_term->cols == 80) hold_x = 150;
		if (sel_term->rows == 24 || sel_term->rows == 13) hold_y = 150;

		tmx += nmx;
		tmy += nmy;

		if (tmx >  hold_x) { mx = 1; tmx -= hold_x; }
		if (tmx < -hold_x) { mx =-1; tmx += hold_x; }
		if (tmy >  hold_y) { my = 1; tmy -= hold_y; }
		if (tmy < -hold_y) { my =-1; tmy += hold_y; }
		
		/* Nothing happend! */
		if (!mx && !my) return FALSE;

		sel_term->cols += mx;
		sel_term->rows += my;

		if (sel_term->cols < 3) { sel_term->cols = 3; }
		if (sel_term->rows < 1) { sel_term->rows = 1; }

		{
			byte rows = sel_term->rows;
			byte cols = sel_term->cols;
			net_term_clamp(m_term, &rows, &cols);
			sel_term->rows = rows;
			sel_term->cols = cols;
		}

		/* Nothing happend! */
		if (!mx && !my) return FALSE;

		sel_term->width  = sel_term->w * sel_term->cols;
		sel_term->height = sel_term->h * sel_term->rows;

		m_resized = TRUE;
		need_render = TRUE;
	}
	else if (m_shift) {
		/*if (sel_term->w == 8 || sel_term->w == 16 || sel_term->w == 32) hold_x = 150;
		if (sel_term->h == 8 || sel_term->h == 16 || sel_term->h == 32) hold_y = 150;*/

		tmx += nmx;
		tmy += nmy;

		if (tmx >  hold_x) { mx = 1; tmx = 0; }
		if (tmx < -hold_x) { mx =-1; tmx = 0; }
		if (tmy >  hold_y) { my = 1; tmy = 0; }
		if (tmy < -hold_y) { my =-1; tmy = 0; }

		/* Nothing happend! */
		if (!mx && !my) return FALSE;

		sel_term->w += mx;
		sel_term->h += my;

		if (sel_term->w < sel_term->fd->w) { sel_term->w = sel_term->fd->w; mx = 0; }
		if (sel_term->h < sel_term->fd->h) { sel_term->h = sel_term->fd->h; my = 0; }

		/* Nothing happend! */
		if (!mx && !my) return FALSE;

		sel_term->width  = sel_term->w * sel_term->cols;
		sel_term->height = sel_term->h * sel_term->rows;

		/*if (!m_rescaled) term_rescale(m_term, FALSE, FALSE);
		 term_close(m_term);
		 term_open(m_term);*/
		#ifndef SINGLE_SURFACE
		term_redraw(m_term);
		#endif
		m_rescaled = TRUE;
		need_render = TRUE;
	} else if (nmx || nmy) {
		mx = sel_term->xoff;
		my = sel_term->yoff;

		sel_term->xoff += nmx;
		sel_term->yoff += nmy;

		if (sel_term->xoff + sel_term->width  > width ) sel_term->xoff = width  - sel_term->width;
		if (sel_term->yoff + sel_term->height > height) sel_term->yoff = height - sel_term->height;
		if ((int)sel_term->xoff < 0) sel_term->xoff = 0;
		if ((int)sel_term->yoff < 0) sel_term->yoff = 0;

		if (mx == sel_term->xoff && my == sel_term->yoff) return FALSE;

		m_moved = TRUE;
		need_render = TRUE;
	}
	return TRUE;
}
/* Drop term (-LMB) */
bool gui_term_drop() {
	if (m_term != -1) {
		if (m_resized && !m_control)
			m_resized = FALSE;
		if (m_rescaled && !m_shift)
			m_rescaled = FALSE;
		if ((m_rescaled && m_shift) || (m_resized && m_control))
			m_subterm = m_term;
		m_term = -1;
		m_moved = FALSE;
		need_render = TRUE; /* highlight lack of selection */
	}
	else return FALSE;
	return TRUE;
}
/* Slap term (RMB) */
bool gui_term_slap(int i) {
	if (i > 0)
	{
		term_close(i);
		return TRUE;
	}
	else
	{
		return term_spawn();
	}
}

/* Flip term (TAB) */
bool gui_term_next() {
	char fontname[100];
	if (m_term != -1)
	{
		term_close(m_term);
		strcpy(fontname, conf_find_font(sel_term));
		term_set_font(m_term, fontname);
		term_open(m_term);

		/* Do not pass this keypress to Angband */
		return TRUE;
	}
	return FALSE;
}

/* Hold shift on term (+SHIFT) */
bool gui_term_shift() {
	m_shift = TRUE;
	tmx = 0;
	tmy = 0;
	return TRUE;
}
/* Release shift on term (-SHIFT) */
bool gui_term_unshift() {
	int i = m_term;

	m_shift = FALSE;

	if (i == -1) i = m_subterm;
	if (i == -1) return FALSE;

	if (!m_rescaled) return FALSE;

	term_close(i);
	term_rescale(i, TRUE, TRUE);
	term_open(i);
	m_rescaled = FALSE;
	m_subterm = m_term = -1;
	need_render = TRUE;

	return TRUE;
}

/* Hold ctrl on term (+CTRL) */
bool gui_term_ctrl() {
	m_control = TRUE;
	tmx = 0;
	tmy = 0;
	return TRUE;
}
/* Release ctrl on term (-CTRL) */
bool gui_term_unctrl() {
	int i = m_term;

	m_control = FALSE;

	if (i == -1) i = m_subterm;
	if (i == -1) return FALSE;

	if (!m_resized) return FALSE;

	if (i) Term_activate(&sel_term->t);
	Term_resize(sel_term->cols, sel_term->rows);
	if (i) Term_activate(term_screen);
	term_close(i);
	term_open(i);
	m_resized = FALSE;
	m_subterm = m_term = -1;
	need_render = TRUE;

	return TRUE;
}
/* Take a screenshot of the whole screen and dump it to a .bmp file */
void gui_take_snapshot() {
	char buf[1024];
	int i;

	term_data *td = (term_data*)(Term->data);

	if (SDL_SaveBMP(td->face, "newshot.bmp")) 
	{
		plog("You fail to get the screenshot off!");
		return;
	}
	for (i = 0; i < 999; ++i) 
	{
		strnfmt(buf, sizeof(buf), "%03d.bmp", i);
		if (file_exists(buf))
		{
			continue;
		}
		file_move("newshot.bmp", buf);
		break;
	}
	plog("*click*");
}
/* Handle SDL_event/gui */
bool gui_term_event(SDL_Event* event) {
	bool taken = FALSE;
	switch (event->type)
	{
		case SDL_QUIT:
		quit("Goodbye.");
		break;

		case SDL_MOUSEMOTION:
		taken = gui_term_drag(event->motion.xrel, event->motion.yrel);
		break;

		case SDL_MOUSEBUTTONUP:
		if( event->button.button == SDL_BUTTON_LEFT )
		{
			taken = gui_term_drop();
		}
		if( event->button.button == SDL_BUTTON_RIGHT )
		{
			taken = gui_term_slap(pick_term(event->button.x, event->button.y));
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		if( event->button.button == SDL_BUTTON_LEFT )
		{
			taken = gui_term_grab(pick_term(event->button.x, event->button.y));
		}
		break;

		case SDL_KEYUP:
		if (event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
		{
			taken = gui_term_unshift();
		}
		if (event->key.keysym.sym == SDLK_RCTRL || event->key.keysym.sym == SDLK_LCTRL)
		{
			taken = gui_term_unctrl();
		}
		break;

		case SDL_KEYDOWN: 
		if (event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
		{
			gui_term_shift();
			taken = FALSE;
		}
		if (event->key.keysym.sym == SDLK_RCTRL || event->key.keysym.sym == SDLK_LCTRL)
		{
			gui_term_ctrl();
			taken = FALSE;
		}
		/* Various frivolous hacks. */
		if (event->key.keysym.sym == SDLK_TAB)
		{
			taken = gui_term_next();
			break;
		}
		if (event->key.keysym.sym == SDLK_F12 && (event->key.keysym.mod & KMOD_ALT))
		{
			gui_take_snapshot();
			taken = TRUE;
		}
		break;

		default:	break;
	}
	return taken;
}

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

/* Handle SDL_event/angband */
void ang_mouse_move(SDL_Event* event) {
	static int last_mouse_x[ANGBAND_TERM_MAX] = { 0 };
	static int last_mouse_y[ANGBAND_TERM_MAX] = { 0 };
	term_data *td;// = (term_data*)(Term->data);

	int x = event->motion.x;
	int y = event->motion.y;
	int i = pick_term(x, y);

	if (i == -1) return;

	/* Ignore non-main windows (for now) */
	if (i != 0) return;

	td = &tdata[i];

	x = (x - td->xoff) / td->w;
	y = (y - td->yoff) / td->h;

	if (x == last_mouse_x[i] && y == last_mouse_y[i])
	{
		return;
	}
	last_mouse_x[i] = x;
	last_mouse_y[i] = y;

	Term_mousepress(x, y, 0);
}
void ang_mouse_press(SDL_Event* event) {
	term_data *td;// = (term_data*)(Term->data);

	int x = event->button.x;
	int y = event->button.y;
	int button = event->button.button;
	int i = pick_term(x, y);

	if (i == -1) return;

	td = &tdata[i];

	x = (x - td->xoff) / td->w;
	y = (y - td->yoff) / td->h;

	/* Convention picked from main-win.c of V341 */
	if (Noticemodkeypress(SDLK_LCTRL, -1))  button |= 16;
	if (Noticemodkeypress(SDLK_LSHIFT, -1)) button |= 32;
	if (Noticemodkeypress(SDLK_LALT, -1))   button |= 64;

	/* Hack -- sub-window click */
	if (i != 0)
	{
		cmd_term_mousepress(i, x, y, button);
		return;
	}

	Term_mousepress(x, y, button);
}
bool ang_mouse_event(SDL_Event* event) {
	bool taken = FALSE;
	switch (event->type)
	{
		/* Assist mouse with modifier keys: */
		case SDL_KEYUP:
		{
			Noticemodkeypress(event->key.keysym.sym, 0);
		}
		break;
		case SDL_KEYDOWN:
		{
			Noticemodkeypress(event->key.keysym.sym, 1);
		}
		break;

		case SDL_MOUSEMOTION:
		{
			ang_mouse_move(event);
			taken = TRUE;
		}
		break;
		case SDL_MOUSEBUTTONUP:
		if ( (event->button.button == SDL_BUTTON_LEFT)
		  || (event->button.button == SDL_BUTTON_RIGHT) )
		{
			ang_mouse_press(event);
			taken = TRUE;
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		if ( (event->button.button == SDL_BUTTON_LEFT)
		  || (event->button.button == SDL_BUTTON_RIGHT) )
		{
			taken = TRUE;
		}
		break;

		default:	break;
	}
	return taken;
}


/*#define SCALETOCOLOR(x) (x=((x)*63+((x)-1)))*/
#define ScaleToColor(x) ((x)=((x)*60)+15)
/*#define ScaleToColor(x) ((x)=((x)*63))*/
void init_color_data_sdl() {
	Uint8 i;

	for (i = 0; i < 16; ++i) {
		color_data_sdl[i].unused = 255; /* no reason. */
		if(!color_data_sdl[i].r && !color_data_sdl[i].g && !color_data_sdl[i].b)
			continue;
		ScaleToColor(color_data_sdl[i].r);
		ScaleToColor(color_data_sdl[i].g);
		ScaleToColor(color_data_sdl[i].b);
	}
}






/*
 *
 * SDL drawing functions!.
 *
 * This could also be moved into another file.
 *
 */



/* Use this function to make SDL_Rect safe brefore SDL_Update */
void SDL_SafeRect(SDL_Rect *ndr)
{
	if (ndr->x + ndr->w > width)  ndr->w = width  - ndr->x;
	if (ndr->y + ndr->h > height) ndr->h = height - ndr->y;
	if (ndr->x < 0) ndr->x = 0;
	if (ndr->y < 0) ndr->y = 0;
}
/* This function brings data from 'term' to 'screen'
 * --- it actually shows some stuff on the screen! Finally!
 *
 * in SINGLE_SURFCE mode, it just updates the main surface
 * which is allready blitted upon
 * 
 * in MULTI_SUFRACE mode, it firstly blits from 'term' surface
 * to 'screen' sufrace, then updates the screen.
 * if 'rendering' is queued anyway, it does nothing
 *
 */
void SDL_FrontRect(term_data *td, int x, int y, int w, int h, bool render, bool update) {
	/* Create a source and a destination rectangle */
	SDL_Rect sr, dr;

	/* Size */
	sr.w = dr.w = w;
	sr.h = dr.h = h;

	/* Origin */
	dr.x = sr.x = x;
	dr.y = sr.y = y;

	/* Position */
	dr.x = ( td ? x + td->xoff : x );
	dr.y = ( td ? y + td->yoff : y );
	/* DO IT! */
#ifndef SINGLE_SURFACE
	if (!render) return;
	if (td) SDL_BlitSurface(td->face, &sr, bigface, &dr);
#endif
	if (quartz_hack)
	{
		return;
	}
	if (update)
	{
		dr.w = w; dr.h = h;
		SDL_SafeRect(&dr);
		SDL_UpdateRect(bigface, dr.x, dr.y, dr.w, dr.h);
	}
}
/* Bring whole terminal to the screen */ 
void SDL_FrontTerm(term_data *td, bool update) {
#ifdef SINGLE_SURFACE
	SDL_FrontRect(td, 0, 0, td->width, td->height, TRUE, update);
#else
	SDL_FrontRect(td, 0, 0, td->face->w, td->face->h, TRUE, update);
#endif
}
/* Bring single grid to screen */
void SDL_FrontChar(term_data* td, int x, int y) {
	SDL_FrontRect(td, td->w * x, td->h * y, td->w, td->h, !need_render, TRUE);
}
/* Create a 'rectangle' from a 'term' pointing to screen pixels 
 * Only used in single-surface mode */
void SDL_GetScreenRect(term_data *td, SDL_Rect *r) {
	r->x = td->xoff;
	r->y = td->yoff;
	r->w = td->width;
	r->h = td->height;
}
/* Find screen pixels given terminal character occupies */
void SDL_GetCharRect(term_data *td, int x, int y, SDL_Rect *r) {
	r->x = td->xoff + x * td->w;
	r->y = td->yoff + y * td->h;
	r->w = td->w;
	r->h = td->h;
}
/* Fill whole screen with color */
void SDL_FillAll(Uint32 color) {
	SDL_Rect nr;
	nr.x = 0;
	nr.y = 0;
	nr.w = width;
	nr.h = height;

	SDL_FillRect(bigface, &nr, color);
	return;
}
/* Fill 'term' with some color */
void SDL_FillTerm(term_data *td, Uint32 color) {
#ifdef SINGLE_SURFACE
	SDL_Rect dr;
	SDL_GetScreenRect(td, &dr);
	SDL_FillRect(bigface, &dr, color);
#else
	SDL_FillRect(td->face, NULL, color);
#endif
}
/* Prepare cursor surface */
errr SDL_PrepareCursor(Uint32 w, Uint32 h)
{
	Uint32 i;

	if (sdl_screen_cursor)
	{
		if (sdl_screen_cursor_sr.w == w && sdl_screen_cursor_sr.h == h) return 0;
		SDL_FreeSurface(sdl_screen_cursor); 
	}

	sdl_screen_cursor_sr.x = sdl_screen_cursor_sr.y = 0;
	sdl_screen_cursor_sr.w = w;
	sdl_screen_cursor_sr.h = h;

	sdl_screen_cursor = NULL;
	sdl_screen_cursor = SDL_CreateRGBSurface(SDL_SRCALPHA, w, h, 32,
	                                         0xff000000,
														  0x00ff0000,
														  0x0000ff00,
														  0x00000000);


	if (!sdl_screen_cursor) return -1;

	SDL_SetAlpha(sdl_screen_cursor, SDL_SRCALPHA | SDL_RLEACCEL, 0x80);
	for (i = 0; i < w*h*4; ++i)
	{
		((Uint8 *)(sdl_screen_cursor->pixels))[i] = !(i & 2)?0x80 : 0xFF;
	}

	return 0;
}
/* Blit cursor to screen at given pixel location */
errr SDL_DrawCursor(SDL_Rect *dr)
{
	if (!dr || !sdl_screen_cursor) return -1;
	sdl_screen_cursor_sr.w = dr->w;
	sdl_screen_cursor_sr.h = dr->h;
	if (SDL_BlitSurface(sdl_screen_cursor, &sdl_screen_cursor_sr, bigface, dr)) return -1;
	SDL_FrontRect(NULL, dr->x, dr->y, dr->w, dr->h, TRUE, TRUE);
	return 0;
}
/* Print char to screen (no buffering). This is NOT used by ZTerm, and is for interal GUI drawing ONLY !!
 *	For regular ZTerm operations, see "SDL_BlitChar()" and the plethora of "Term_XXX_sdl()" functions.
 */
void SDL_PrintChar(term_data* td, int x, int y, Uint32 a, unsigned char c) {
	SDL_Rect sr, dr;
	SDL_Color dc;

	dr.w = sr.w = td->fd->w;
	dr.h = sr.h = td->fd->h;

	sr.x = (c % 16) * td->fd->w;
	sr.y = (c / 16) * td->fd->h;

	dr.x = x * td->fd->w + td->xoff;
	dr.y = y * td->fd->h + td->yoff;
	if (y < 0) dr.y = td->height - (-y * td->fd->h) + td->yoff;

	dc.r = a >> 16;
	dc.g = a >> 8;
	dc.b = a;

	if (td->fd->face->format->BitsPerPixel == 8)
	SDL_SetColors(td->fd->face, &dc, 0x01, 1);
	SDL_SetColorKey(td->fd->face, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
	SDL_BlitSurface(td->fd->face, &sr, bigface, &dr);
	SDL_SetColorKey(td->fd->face, SDL_RLEACCEL, 0);
}
/* Call SDL_PrintChar "n" times for each letter in "text" */
void SDL_PrintText(term_data* td, int x, int y, Uint32 color, cptr text) {
	int n = strlen(text);
	while(n > 0) {
		SDL_PrintChar(td, x, y, color, *text);
		++x; --n; ++text;
	}
}
/*
 * Main function for drawing characters to screen.
 *
 * Note: this function will not handle graphic tiles correctly, see "Term_tile_sdl()"
 */
void SDL_BlitChar_AUX(term_data* td, font_data *fd, int x, int y, byte a, unsigned char c) {
	SDL_Rect sr, dr;

	dr.x = x * td->w;
	dr.y = y * td->h;

	sr.w = dr.w = fd->w;
	sr.h = dr.h = fd->h;

	sr.x = (c % 16) * fd->w;
	sr.y = (c / 16) * fd->h;

	/* Tweaking palette with SDL_SetColors is not optimal AT ALL */
	if (td->fd->precolorized)
		sr.x += a * fd->face->w;
	else
	{
#ifdef SHADE_FONTS
		int i;
		for (i = 1; i < fd->face->format->palette->ncolors; i++)
		{
			SDL_Color dc;
			dc.r = color_data_sdl[a&0xf].r * fd->ramp[i].r / 255;
			dc.g = color_data_sdl[a&0xf].g * fd->ramp[i].g / 255;
			dc.b = color_data_sdl[a&0xf].b * fd->ramp[i].b / 255;
			SDL_SetColors(fd->face, &dc, i, 1);
		}
#else
		SDL_SetColors(fd->face, &(color_data_sdl[a&0xf]), 0x01, 1);
#endif
	}

#ifdef SINGLE_SURFACE
	dr.x += td->xoff;
	dr.y += td->yoff;
	SDL_BlitSurface(fd->face, &sr, bigface, &dr);
#else
	SDL_BlitSurface(fd->face, &sr, td->face, &dr);
#endif
}
/* Proxy function which determines if 'rescaled' font is to be used */
void SDL_BlitChar(term_data* td, int x, int y, byte a, unsigned char c) {
	if (td->sfd.face)
	{
		SDL_BlitChar_AUX(td, &(td->sfd), x, y, a, c);
	}
	else
	{
		SDL_BlitChar_AUX(td, td->fd, x, y, a, c);
	}
}
/* Call BlitChar "n" times for "text" */
void SDL_BlitText(term_data* td, int x, int y, byte a, cptr text) {
	int n = strlen(text);
	while(n > 0) {
		SDL_BlitChar(td, x, y, a, *text);
		++x; --n; ++text;
	}
}






/*
 *
 * Function hooks needed by "Term"
 *
 * This is really the meat of the display module.
 * If you implement a new graphics engine, you implement several of these
 * hooks (at least pict_hook, aka Term_pict_???()), and replace mine with
 * them in term_data_link(). Ask me if you have any questions.
 * Perhaps by the time you read this, a short hacker's guide is already
 * available.
 *
 * I think that it is important to make sure that new modules using main-sdl.c 
 * remain compatible with future revisions of main-sdl.c. I don't want other
 * modules falling behind when I make important changes. I also think that it
 * is an easy thing to avoid. -- Greg V.
 *
 */





/*
 * XXX XXX XXX Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */

/*
static void Term_init_sdl(term *t)
{
	term_data *td = (term_data*)(t->data);

	td->face = SDL_SetVideoMode(td->width, td->height, td->bpp, td->flags);

	if (td->width && td->height) {
		td->face = SDL_CreateRGBSurface(SDL_SWSURFACE, td->width, td->height, 8,0,0,0,0);
	}
}
*/


/*
 * XXX XXX XXX Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_sdl(term *t)
{
	term_data *td = (term_data*)(t->data);

	if (bigface)
	{
		save_sdl_prefs();
		SDL_FreeSurface(bigface); /* --all-- your base */
		bigface = NULL;
	}

	term_unload_ptr(td); /* what happen! someone set up us the bomb! */
}



/*
 * XXX XXX XXX Do a "user action" on the current "term"
 *
 * This function allows the visual module to do things.
 *
 * This function is currently unused, but has access to the "info"
 * field of the "term" to hold an extra argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_user_sdl(int n)
{
	term_data *td = (term_data*)(Term->data);

	mouse_mode = !mouse_mode;
	c_msg_print_aux(format("*** Mouse mode set: %s ***", mouse_mode ? "Game control" : "Term control"), MSG_LOCAL);

	/* OK */
	return (0);
}

/*
 * XXX XXX XXX Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "term.c" package.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_sdl(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	SDL_Event event; /* just a temporary place to hold an event */

	/* Used by hacks */
	int i;

	/* Analyze */
	switch (n)
	{
		case TERM_XTRA_EVENT:

		Flushdelayedkey(TRUE, FALSE, 0, SDL_GetTicks());

		while (1) {
			if (v)
			{
				if (!SDL_WaitEvent(&event)) return(0); /* TODO handle errors */
				v = 0;
			} else
			{
				if (!SDL_PollEvent(&event)) return(0);
			}

			event_timestamp = SDL_GetTicks();/*event.key.timestamp;*/

			if (mouse_mode && ang_mouse_event(&event)) continue;

			if (gui_term_event(&event)) continue;

			if (event.type == SDL_KEYUP)
			{
				Flushdelayedkey(TRUE, FALSE, event.key.keysym.sym, 0);
			}

			if (event.type == SDL_KEYDOWN)
			{
				Flushdelayedkey(TRUE, FALSE, event.key.keysym.sym, event_timestamp);

				/* PASS Keypress to Angband Terminals finally */
				Multikeypress(SDL_keysymtostr(&event.key.keysym));
			}
		}
		return (0);

		case TERM_XTRA_FLUSH:

		/* XXX XXX XXX Flush all pending events */
		/* This action should handle all events waiting on the */
		/* queue, optionally discarding all "keypress" events, */
		/* since they will be discarded anyway in "term.c". */
		/* This action is required, but is often not "essential". */

		while (SDL_PollEvent(&event)) /* do nothing */ ;

		return (0);

		case TERM_XTRA_CLEAR:

		/* XXX XXX XXX Clear the entire window */
		/* This action should clear the entire window, and redraw */
		/* any "borders" or other "graphic" aspects of the window. */
		/* This action is required. */

		if (!td->online) return(1);

		/* Fill it with black */
		SDL_FillTerm(td, 0);

		return (0);

		case TERM_XTRA_SHAPE:

		/* XXX XXX XXX Set the cursor visibility (optional) */
		/* This action should change the visibility of the cursor, */
		/* if possible, to the requested value (0=off, 1=on) */
		/* This action is optional, but can improve both the */
		/* efficiency (and attractiveness) of the program. */

		td->cursor_on = v ? TRUE : FALSE;

		return (0);

		case TERM_XTRA_FROSH:

		/* XXX XXX XXX Flush a row of output (optional) */
		/* This action should make sure that row "v" of the "output" */
		/* to the window will actually appear on the window. */
		/* This action is optional on most systems. */

		if (!td->online) return -1;
		if (td->prefer_fresh) return 0;

		SDL_FrontRect(td, 0, v*td->h, td->width, td->h, !need_render, TRUE);

#ifndef SINGLE_SURFACE
		if (td->cursor_on && v == td->cy) term_cursor(0);
#endif
		return (0);

		case TERM_XTRA_FRESH:
#ifdef USE_SOUND
		/* HACK !!! */
		/* Terminate current sound if necessary */
		if (use_sound) sdl_play_sound_end(TRUE);
#endif
		/* XXX XXX XXX Flush output (optional) */
		/* This action should make sure that all "output" to the */
		/* window will actually appear on the window. */
		/* This action is optional if all "output" will eventually */
		/* show up on its own, or when actually requested. */

		if (!bigface) return -1;
		if ((bigface->flags & SDL_HWSURFACE && bigface->flags & SDL_DOUBLEBUF)
		    || td->prefer_fresh || need_render) 
		{
			term_display_all();
			SDL_Flip(bigface);
		}
		else if (quartz_hack)
		{
			SDL_Flip(bigface);
		}
		return (0);

		case TERM_XTRA_NOISE:

		/* XXX XXX XXX Make a noise (optional) */
		/* This action should produce a "beep" noise. */
		/* This action is optional, but nice. */
		sdl_bell();

		return (0);

		case TERM_XTRA_SOUND:
#ifdef USE_SOUND
		/* Make a sound */
		i = sound_count(v);
		if (i) sdl_play_sound (v, randint0(i));
#endif
		return (0);

		case TERM_XTRA_BORED:

		/* XXX XXX XXX Handle random events when bored (optional) */
		/* This action is optional, and not important */

		/* TODO add nifty effects here, I guess? */

		return (0);

		case TERM_XTRA_REACT:

		/* XXX XXX XXX React to global changes (optional) */
		/* For example, this action can be used to react to */
		/* changes in the global "color_table[256][4]" array. */
		/* This action is optional, but can be very useful */
#if 0
		term_display_all();
		SDL_Flip(bigface); /* I guess... XXX XXX XXX */ 
#endif
		return (0);

		case TERM_XTRA_ALIVE:

		/* XXX XXX XXX Change the "hard" level (optional) */
		/* This action is used if the program changes "aliveness" */
		/* by being either "suspended" (v=0) or "resumed" (v=1) */
		/* This action is optional, unless the computer uses the */
		/* same "physical screen" for multiple programs, in which */
		/* case this action should clean up to let other programs */
		/* use the screen, or resume from such a cleaned up state. */
		/* This action is currently only used on UNIX machines */

		/* TODO this should probably do something... no, nevermind. */

		return (0);

		case TERM_XTRA_LEVEL:

		/* XXX XXX XXX Change the "soft" level (optional) */
		/* This action is used when the term window changes "activation" */
		/* either by becoming "inactive" (v=0) or "active" (v=1) */
		/* This action is optional but can be used to do things like */
		/* activate the proper font / drawing mode for the newly active */
		/* term window.  This action should NOT change which window has */
		/* the "focus", which window is "raised", or anything like that. */
		/* This action is optional if all the other things which depend */
		/* on what term is active handle activation themself. */

		return (0);

		case TERM_XTRA_DELAY:

		/* XXX XXX XXX Delay for some milliseconds (optional) */
		/* This action is important for certain visual effects, and also */
		/* for the parsing of macro sequences on certain machines. */

		SDL_Delay(v);

		return (0);
	}

	/* Unknown or Unhandled action */
	return (1);
}


/*
 * XXX XXX XXX Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr  Term_wipe_sdl(int x, int y, int n)
{
	SDL_Rect dr;
	term_data *td = (term_data*)(Term->data);

	if (!td->face || !td->fd) { return 0; }

	dr.x = x * td->w;
	dr.y = y * td->h;
	dr.w = n * td->w;
	dr.h = td->h;
#ifdef SINGLE_SURFACE
	dr.x += td->xoff;
	dr.y += td->yoff;
	SDL_FillRect(bigface, &dr, 0);
	dr.x -= td->xoff;
	dr.y -= td->yoff;
#else
	SDL_FillRect(td->face, &dr, 0);
#endif
	SDL_FrontRect(td, dr.x, dr.y, dr.w, dr.h, FALSE, TRUE);
	
	/* Erase cursor */
/*
	if (td->cx >= x && td->cx <= x + n && td->cy == y)
	{
		td->cx = td->cy = -1;
	}
*/
	/* Success */
	return (0);
}

/*
 * XXX XXX XXX Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  Note the "soft_cursor"
 * flag which tells "term.c" to treat the "cursor" as a "visual"
 * thing and not as a "hardware" cursor.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_grab(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 */
static errr Term_curs_sdl(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	SDL_Rect dr;

	/* Erase old */
	if (td->cx != x || td->cy != y)
	{
		if (td->cx != -1 && td->cy != -1)
		{
			SDL_FrontChar(td, td->cx, td->cy);
		}
	}

	if (!td->cursor_on) return (0);

	/* No change */
	if (td->cx == x && td->cy == y) return (0);

	/* Asign & Redraw */
	if (td->cursor_on)
	{
		td->cx = x;
		td->cy = y;

		/* if (x == -1 || y == -1) return; */

		SDL_GetCharRect(td, x, y, &dr);
		SDL_DrawCursor(&dr);
	}


	/* Success */
	return (0);
}
/* This one draws a graphical tile. */
static errr Term_tile_sdl(int x, int y, Uint8 a, Uint8 c, Uint8 ta, Uint8 tc){
	SDL_Rect sr, dr, nsr;
	/*int n;*/
	term_data *td = (term_data*)(Term->data);
	SDL_Surface *dst = td->face;
	graf_tiles *gt_ptr;

	if(!td->online) return -1;


	if (td->sgt.face)
	{
		gt_ptr = &(td->sgt);
	}
	else if (td->gt)
	{
		gt_ptr = td->gt;
	}
	else return -1;

	Term_wipe_sdl(x, y, 1);

	nsr.w = sr.w = gt_ptr->w;
	nsr.h = sr.h = gt_ptr->h;

	dr.w = td->w;
	dr.h = td->h;

	nsr.x = sr.x = (c & 0x7F) * gt_ptr->w;
	nsr.y = sr.y = (a & 0x7F) * gt_ptr->h;

	dr.x = x * td->w;
	dr.y = y * td->h;

#ifdef SINGLE_SURFACE
	dr.x += td->xoff;
	dr.y += td->yoff;
	dst = bigface;
#endif

	if (use_graphics >= GRAPHICS_TRANSPARENT)
	{
			nsr.x = (tc & 0x7F) * gt_ptr->w;
			nsr.y = (ta & 0x7F) * gt_ptr->h;
			SDL_BlitSurface(gt_ptr->face, &nsr, dst, &dr);

			/* Only draw if terrain and overlay are different */
			if ((sr.x != nsr.y) || (sr.y != nsr.y))
			{
				SDL_SetColorKey(gt_ptr->face, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
				SDL_BlitSurface(gt_ptr->face, &sr, dst, &dr);
				SDL_SetColorKey(gt_ptr->face, SDL_RLEACCEL, 0);
			}
	}
	else
		SDL_BlitSurface(gt_ptr->face, &sr, dst, &dr);

	/* Erase cursor */
/*
	if (td->cursor_on && td->cx == x && td->cy == y)
	{
		SDL_FrontChar(td, x, y);
		td->cx = td->cy = -1;
	}
*/
	/* Success */
	return (0);
}



/*
 * XXX XXX XXX Display a character text on the screen
 *
 */
static errr Term_char_sdl (int x, int y, byte a, unsigned char c) {
	term_data *td = (term_data*)(Term->data);

	if (!td->online) return -1;

	/* Pass it */
	SDL_BlitChar(td, x, y, a, c);

	/* Erase cursor */
/*
	if (td->cursor_on && td->cx == x && td->cy == y)
	{
		SDL_FrontChar(td, x, y);
		td->cx = td->cy = -1;
	}
*/
	/* Success */
	return (0);
}

/*
 * XXX XXX XXX Draw a "picture" on the screen
 *
 */

static errr Term_pict_sdl(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);

	if (!td->gt || !td->gt->face)
	{
		Term_char_sdl(x, y, *ap, *cp);
	} else
	while(n--)
	{
		if (td->gt && td->gt->face) /* it never hurts (much) to check */
		{
			Term_tile_sdl(x, y, *ap, *cp, *tap, *tcp); /* draw a graphical tile */
		}
		/* Ugh... */
		ap--; cp--; tap--; tcp--;
		x++;
	}

	/* Success */
	return (0);
}

/* We need a different version of Term_pict_sdl() for older versions
 * of the Angband term code. This is mostly only necessary for MAngband.
 */
static errr Term_pict_sdl_28x(int x, int y, byte a, char c)
{
	return Term_pict_sdl(x, y, 1, &a, &c, &a, &c);
}

/*
 * XXX XXX XXX Display some text on the screen
 *
 * This function should actually display a string of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which is terminated
 * with a nul character and which has exactly "n" characters.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * You may ignore the "color" parameter if you are only supporting
 * a monochrome environment, unless you have set the "draw_blanks"
 * flag, since this routine is normally never called to display
 * "black" (invisible) text, and all other colors should be drawn
 * in the "normal" color in a monochrome environment.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_sdl(int x, int y, int n, byte a, cptr s)
{
	/*term_data *td = (term_data*)(Term->data);*/

	while(n > 0) {
		Term_char_sdl(x, y, a, *s);
		++x; --n; ++s;
	}

	/* Success, we hope. */
	return (0);
}










/* Here go internal terminal function */





/*
 * XXX XXX XXX Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(int i)
{
	term_data *td = &(tdata[i]);
	term *t = &(td->t);

	int j = 1;
	if (i == 0) j = 256;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, j);

	/* We'll handle our curosr */
	t->soft_cursor = TRUE;

	/* Support graphics */
	t->higher_pict = TRUE;

	/* We do not handle bored yet */
	t->never_bored = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';


	/* Prepare the init/nuke hooks */
	td->nuke_hook = t->nuke_hook = Term_nuke_sdl;

	/* Prepare the template hooks */
	td->user_hook = t->user_hook = Term_user_sdl;
	td->wipe_hook = t->wipe_hook = Term_wipe_sdl;
	td->curs_hook = t->curs_hook = Term_curs_sdl;
	td->text_hook = t->text_hook = Term_text_sdl;
	td->xtra_hook = t->xtra_hook = Term_xtra_sdl;

	td->pict_hook = t->pict_hook = Term_pict_sdl;

	/* Remember where we came from */
	t->data = (vptr)(td);

	/* Save pointer */
	ang_term[i] = t;

	/* Special pointer for Main Window */
	if (!i) term_screen = t;

	/* Activate it */
	/* Term_activate(t); */
}

/* Create new scaled surfaces if 'term' sizes are weird
 * If both 'create' and 'redraw' are false, this will only clean them.
*/
void term_rescale(int i, bool create, bool redraw) {
	term_data *td = &(tdata[i]);

	if (td->sfd.face) {
		SDL_FreeSurface(td->sfd.face);
		td->sfd.face = NULL;
		td->sfd.w = td->sfd.h = 0;
		redraw = TRUE;
	}
	if (td->sgt.face) {
		SDL_FreeSurface(td->sgt.face);
		td->sgt.face = NULL;
		td->sgt.w = td->sgt.h = 0;
		redraw = TRUE;
	}

	if (!create) return;

	/* Rescale font */
	if (td->fd->face && (td->fd->w != td->w || td->fd->h != td->h)) {
		SDL_SetColors(td->fd->face, &(color_data_sdl[TERM_WHITE&0xf]), 0xff, 1);
		td->sfd.face = SDL_ScaleTiledBitmap(td->fd->face, td->fd->w, td->fd->h, td->w, td->h, 0);
#ifdef SHADE_FONTS
		memcpy(td->sfd.ramp, td->fd->ramp, sizeof(SDL_Color) * 255);
#endif
		td->sfd.w = td->w;
		td->sfd.h = td->h;

		if (td->cursor_on)
			SDL_PrepareCursor(td->w, td->h);
	}

	/* Rescale tileset */
	if (td->gt && td->gt->face && (td->gt->w != td->w || td->gt->h != td->h)) {
		bool reused = FALSE;
		int j;

		/* Can we steal rescaled bitmap from another term? */
		for (j = 0; j < ANGBAND_TERM_MAX; j++) {
			if (i == j || tdata[j].sgt.face == NULL) continue;
			/* Size matches */
			if (tdata[j].sgt.w == td->w && tdata[j].sgt.h == td->h) {
				td->sgt.face = tdata[j].sgt.face;
				tdata[j].sgt.face->refcount++;
				reused = TRUE;
				break;
			}
		}

		if (!reused) {

		/* Print message */
		SDL_FillRect(bigface, sdl_quick_rect(td->xoff, td->yoff, td->fd->w * 35, td->fd->h-1), 0);
		SDL_PrintText(td, 0, 0, gui_color_term_title, " Re-scaling tiles. Please wait... ");
#ifdef SINGLE_SURFACE
		SDL_FrontRect(td, 0, 0, td->fd->w * 35, td->fd->h, FALSE, TRUE);
#else
		SDL_Flip(bigface);
#endif

		/* Do the rescaling */
		td->sgt.face = SDL_ScaleTiledBitmap(td->gt->face, td->gt->w, td->gt->h, td->w, td->h, 0);

		/* Wipe message */
		SDL_FillRect(bigface, sdl_quick_rect(td->xoff, td->yoff, td->fd->w * 35, td->fd->h), 0);
#ifdef SINGLE_SURFACE
		SDL_FrontRect(td, 0, 0, td->fd->w * 35, td->fd->h, FALSE, TRUE);
#else
		SDL_Flip(bigface);
#endif
		}
		/* Keep new size */
		td->sgt.w = td->w;
		td->sgt.h = td->h;
	}

	if (redraw)
	{
		/* term_redraw(m_term); */
	}
}
/*
 * This function finds whether 2 'terms' overlap
 */
bool terms_collide(int i, int j) {
    int left1, left2;
    int right1, right2;
    int top1, top2;
    int bottom1, bottom2;

    left1 = tdata[i].xoff;
    left2 = tdata[j].xoff;
    right1 = tdata[i].xoff + tdata[i].width;
    right2 = tdata[j].xoff + tdata[j].width;
    top1 = tdata[i].yoff;
    top2 = tdata[j].yoff;
    bottom1 = tdata[i].yoff + tdata[i].height;
    bottom2 = tdata[j].yoff + tdata[j].height;

    if (bottom1 < top2) return(FALSE);
    if (top1 >= bottom2) return(FALSE);

    if (right1 < left2) return(FALSE);
    if (left1 >= right2) return(FALSE);

    return(TRUE);
}

void term_stack(int i) {
	term_data *td = &tdata[i];
	int j;
	bool found = FALSE;

	td->xoff = td->yoff = 0;

	if (i == 0) found = TRUE;

	while (!found) {
		/* Iterate throu all previous terminals to find non-occupied space */
		for (j = 0; j < 8; j++) {
			found = TRUE;

			if (i == j) continue; /* Skip self */

			if (tdata[j].online == FALSE) continue; /* Skip offline terminals */

			/* Collision! */
			if (terms_collide(i, j)) {
				/* Move right if there is still space */
				if (td->xoff + td->width + tdata[j].width < width) {
					td->xoff = td->xoff + tdata[j].width;
					found = FALSE;
					break;
				}
				/* Perform a carriage return */
				if (td->yoff + 1 + tdata[i].height < height) {
					td->yoff = td->yoff + 1;
					td->xoff = 0;
					found = FALSE;
					break;
				}

				/* Turn off!
				plog("turning off");
				tdata[i].online = FALSE;
				found = TRUE;
				break; */
			}
		}
	}
}

void term_close(int i)
{
	term_data *td = &tdata[i];

	td->online = FALSE;
	need_render = TRUE;

#ifndef SINGLE_SURFACE
	SDL_FreeSurface(td->face);
	td->face = NULL;
#endif

	/* Unlink */
	ang_term[i] = NULL;
}

void term_open(int j)
{
	if (j == -1) return;

	/* Init, if it's dead */
	if (!tdata[j].name)
	{
		init_one_term(j, TRUE);
		term_data_link(j);
	}

	/* Link (doesn't matter if "term_data_link" was called) */
	ang_term[j] = &tdata[j].t;

#ifndef SINGLE_SURFACE
	if (!tdata[j].face)
		tdata[j].face = SDL_CreateRGBSurface(SDL_SWSURFACE, tdata[j].width, tdata[j].height, 32,0,0,0,0);
#endif
	need_render = TRUE;
	tdata[j].online = TRUE;

	term_redraw(j);
	p_ptr->window |= window_flag[j];
	if (window_flag[j] & PW_MESSAGE_CHAT) p_ptr->window |= PW_MESSAGE; 	/* XXX Evil Chat hack */
	window_stuff();
}
bool term_spawn()
{
	int j;
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (!tdata[j].online)
		{
			term_open(j);
			term_stack(j);
			need_render = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}
/* Draw a 'window decoration' aka 'border' around terminal */
void term_draw_border(term_data *td) {
		char title[MSG_LEN];
		SDL_Rect nr;

		/* 'Header' */
		nr.x = td->xoff - 1;
		nr.y = td->yoff - 1;
		nr.w = td->width + 2;
		nr.h = td->fd->h;
		SDL_FillRect(bigface, &nr, gui_color_term_header);

		/* Header text */
		sprintf(title, "* %s [%dx%d] (%dx%d)", td->name, td->cols, td->rows, td->w, td->h);	
		SDL_PrintText(td, 1, 0, gui_color_term_title, title);

		/* Top line */
		nr.h = 1;
		SDL_FillRect(bigface, &nr, gui_color_term_border);

		/* Header underline */
		nr.y += td->fd->h;
		SDL_FillRect(bigface, &nr, gui_color_term_border);

		/* Bottom line */
		nr.y = td->yoff + td->height;
		SDL_FillRect(bigface, &nr, gui_color_term_border);

		/* Left line */
		nr.w = 1;
		nr.y = td->yoff;
		nr.h = td->height;
		SDL_FillRect(bigface, &nr, gui_color_term_border);

		/* Right line */
		nr.x += td->width + 1;
		SDL_FillRect(bigface, &nr, gui_color_term_border);
}

/*
 *	MEAT function, the center of the drawing loop, the renderer, etc.
 * it makes sure all the 'terms' are on screen all the time, handles
 * "need_render" and applies term decorations when needed.
 */
void term_display_all() {
	int color;
	int i = 0;

	/* Do not waste precious CPU */
	if (!need_render) return;
	need_render = FALSE;

	/* BG Color */
	if (m_term == -1) 
		color = gui_color_back_ground;
	else
		color = gui_color_back_active;

	/* Clear everything */
	SDL_FillRect(bigface, NULL, color);

	/* Bring every online terminal to front */
	for (i = 0; i < 8; i++) {
		if (!tdata[i].online) continue;
#ifdef SINGLE_SURFACE
		term_redraw(i);
#endif
		SDL_FrontTerm(&tdata[i], FALSE);
#ifndef SINGLE_SURFACE
		if (tdata[i].cursor_on)
			term_cursor(i);
#endif
		if (m_term == i || m_subterm == i || tdata[i].rows == 1)
		{
			term_draw_border(&tdata[i]);
		}
	}
	/* SDL_Flip(bigface); */
}

/*
 * Hack -- redraw a term_data
 */
void term_redraw(int i) {
#ifdef SINGLE_SURFACE
	/* Use customized redrawing function to avoid flickering caused by Term_clear() */ 
	term_data *td = &(tdata[i]);
	int y, x, ty, tx;
	Uint8 a, c, ta, tc;

	if (!td->t.scr) return;

	ty = td->rows;
	tx = td->cols;

	for (y = 0; y < td->rows; y++)
	{
		for (x = 0; x < td->cols; x++)
		{
			if (x >= td->t.wid || y >= td->t.hgt)
			{
				a = td->t.attr_blank;
				c = td->t.char_blank;
				ta = a;
				tc = c;
			}
			else
			{
				a = td->t.scr->a[y][x];
				c = td->t.scr->c[y][x];
				ta = td->t.scr->ta[y][x];
				tc = td->t.scr->tc[y][x];
			}
			/* Use "Term_pict" for "special" data */
			if ((a & 0x80) && (c & 0x80))
			{
				Term_tile_sdl(x, y, a, c, ta, tc);
			} else
			SDL_BlitChar(td, x, y, a, c);
		}
	}
#else
	/* Use standart Angband facilities */
	term_data *td = &tdata[i];

	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Restore the term */
	Term_activate(term_screen);
#endif
}
/* SDL_DrawCursor wrapper */
void term_cursor(int i)
{
	term_data *td = &(tdata[i]);
	SDL_Rect dr;
	if (td->cx == -1 || td->cy == -1) return;
	SDL_GetCharRect(td, td->cx, td->cy, &dr);
	SDL_DrawCursor(&dr);
}

void term_unload_font(int i)
{
	term_data *td = &(tdata[i]);
	bool in_use = FALSE;
	int j;

	if (!td->fd) return;

	/* See if it's in use */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (i == j || !tdata[j].fd) continue;
		if (!strcmp(tdata[j].fd->name, tdata[i].fd->name))
		{
			in_use = TRUE;
			break;
		}
	}

	/* Free to unload */
	if (!in_use)
	{
		if (td->fd->face)
		{
			SDL_FreeSurface(td->fd->face);
			td->fd->face = 0;
		}
		if (td->fd->name)
		{
			string_free(td->fd->name);
			td->fd->name = 0;
		}
		FREE(td->fd);
	}
	td->fd = NULL;
	term_rescale(i, FALSE, FALSE);
}

void term_unload_graf(int i)
{
	term_data *td = &(tdata[i]);
	bool in_use = FALSE;
	int j;

	if (!td->gt) return;

	/* See if it's in use */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (i == j || !tdata[j].gt) continue;
		if (!strcmp(tdata[j].gt->name, tdata[i].gt->name))
		{
			in_use = TRUE;
			break;
		}
	}

	/* Free to unload */
	if (!in_use)
	{
		if (td->gt->face)
		{
			SDL_FreeSurface(td->gt->face);
			td->gt->face = 0;
		}
		if (td->gt->name)
		{
			string_free(td->gt->name);
			td->gt->name = 0;
		}
		FREE(td->gt);
	}
	td->gt = NULL;
}
/* Attempt to load a graphical tileset */
bool term_load_graf(int i, cptr filename, cptr maskname)
{
	int j;
	graf_tiles *load_tiles;
	term_data *td = &(tdata[i]);

	/* Some tileset allready loaded */
	if (td->gt)
	{
		if (!strcmp(td->gt->name, filename))
			return TRUE;
		else
			return FALSE;
	}

	/* Can we steal the tileset from another term? */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (i == j || !tdata[j].gt) continue;
		if (streq(tdata[j].gt->name, filename))
		{
			/* Just use whole graf_data */
			td->gt = tdata[j].gt;

			/* Yes! */
			return TRUE;
		}
	}

	/* Load graf */
	if (!td->gt)
	{
		MAKE(load_tiles, graf_tiles);
		memset(load_tiles, 0, sizeof(graf_tiles));
		load_tiles->face = NULL;

		if (!load_ANY_graf_sdl(load_tiles, filename, maskname))
		{
			load_tiles->name = string_make(filename);
			td->gt = load_tiles;
		}
	}
	if (!td->gt)
	{
		return FALSE;
	}
	return TRUE;
}
/* Attempt to load font from file and bind it to 'term' "i" */
bool term_set_font(int i, cptr fontname)
{
	font_data *load_font;
	term_data *td = &(tdata[i]);
	int j;

	/* Some font allready loaded */
	if (td->fd)
	{
		if (!strcmp(td->fd->name, fontname))
			return TRUE;
		else
			term_unload_font(i);
	}

	/* Attempt to use pre-loaded font */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (i == j || !tdata[j].fd) continue;
		if (!strcmp(tdata[j].fd->name, fontname))
		{
			td->fd = tdata[j].fd;
			break;
		}
	}

	/* Load font */
	if (!td->fd)
	{
		MAKE(load_font, font_data);
		memset(load_font, 0, sizeof(font_data));
		load_font->face = NULL;

		if (!load_ANY_font_sdl(load_font, fontname))
		{
			load_font->name = string_make(fontname);
			td->fd = load_font;
			if (i == 0)
			{
				char *ext = strrchr(fontname, '.');
				ANGBAND_FON = ext ? string_make(ext+1) : ANGBAND_FON;
				ANGBAND_FONTNAME = string_make(fontname);
			}
		}
	}
	if (!td->fd)
	{
		return FALSE;
	}

	/* Metrics */
	td->width = td->fd->w * td->cols;
	td->height = td->fd->h * td->rows;

	td->w = td->fd->w;
	td->h = td->fd->h;

	return TRUE;
}
/* JIC */
void term_unload(int i)
{
	term_data *td = &(tdata[i]);
	bool need_render_hack;

	/* We do not want unloading mess with display */
	need_render_hack = need_render;

	/* Take care of ->fd */
	term_unload_font(i);

	/* Take care of ->gt */
	term_unload_graf(i);

	/* Take care of ->face and ->online */
	term_close(i);

	/* Take care of ->sfd and ->sgt */
	term_rescale(i, FALSE, FALSE);

	/* Mark as dead! This is unique: */
	if (td->name)
	{
		string_free(td->name);
		td->name = 0;
	}

	/* If you want to redraw screen, set need_render after calling term_unload */
	need_render = need_render_hack;
}
void term_unload_ptr(term_data *td) 
{
	int i;
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		if (&(tdata[i]) == td) {
			term_unload(i);
			break;
		}
	}
}




/*
 * XXX XXX XXX Initialization functions
 */
void init_all_terms()
{
	term_data *td;
	int i;
	/* Clear */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		td = &(tdata[i]);
		(void)WIPE(td, term_data);
		td->name = 0;
	}

	/* Init */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		if (init_one_term(i, FALSE))
		{
			term_data_link(i);
		}
	}
}

/* Running this function against 'term' more than once is *strongly* discourged
 * If you need to unload/load 'term's, use term_close() and term_open() functions */
bool init_one_term(int i, bool force)
{
	char term_name[30];
	char sec_name[30];
	char fontname[100];
	term_data *td;
	td = &(tdata[i]);
	term_name[0] = '\0';

	switch (i)
	{
		case 0: 	strcpy(term_name, "Angband");	break;
		case 1: 	strcpy(term_name, "Mirror");	break;
		case 2: 	strcpy(term_name, "Recall");	break;
		case 3: 	strcpy(term_name, "Choice");	break;
		default:	sprintf(term_name, "%d", i);
	}

	sprintf(sec_name, "SDL-Term-%s", term_name);

	/* Is it visible ? */
	td->online = (bool)conf_get_int(sec_name, "Visible", 0);
	if (!i) td->online = TRUE;	/* Hack: Main Window always visible */

	/* Hack: if it's not visible, STOP RIGHT NOW */
	if (!td->online && !force) return FALSE;

	/* Load font */
	td->fd = NULL;
	strcpy(fontname, conf_get_string(sec_name, "Font", "nethack10x19-10.hex"));
	if (!term_set_font(i, fontname))
	{
		/* Failed to load font, rest is useless */
		return FALSE;
	}

	/* Load graphics */
	td->gt = NULL;
	if (use_graphics)
	{
		graphics_mode* gm = get_graphics_mode((byte)use_graphics);
		if (gm && term_load_graf(i, gm->file, gm->mask))
		{
			/* Tileset loaded */
			ANGBAND_GRAF = gm->pref;
		}
	}

	/* If we are here,  consider "term" loaded */

	/* Name */
	td->name = string_make(term_name);
	td->face = NULL;

	/* Metrics */
	td->cols = conf_get_int(sec_name, "Cols", 80);
	td->rows = conf_get_int(sec_name, "Rows", 24);
	if (!td->cols) td->cols = 80;
	if (!td->rows) td->rows = 24;
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	td->xoff = conf_get_int(sec_name, "PositionX", 0);
	td->yoff = conf_get_int(sec_name, "PositionY", 0);

	/* Scaling */
	td->w = (byte)conf_get_int(sec_name, "ScaleX", 0);
	td->h = (byte)conf_get_int(sec_name, "ScaleY", 0);

	if(td->w < td->fd->w) td->w = td->fd->w; 
	if(td->h < td->fd->h) td->h = td->fd->h; 

	td->width = td->w * td->cols;
	td->height = td->h * td->rows;

	term_rescale(i, TRUE, FALSE);

#ifndef SINGLE_SURFACE
	td->face = SDL_CreateRGBSurface(SDL_SWSURFACE, td->width, td->height, 32,0,0,0,0);
#endif

	/* Enable cursor for "term-0" */
	td->cursor_on = (!i ? TRUE : FALSE);

	/* Cursor default position: off */
	td->cx = td->cy = -1;

	return TRUE;
}

static void quit_sdl(cptr str)
{
	/* Call regular quit hook */
	quit_hook(str);

	/* Forget grafmodes */
	close_graphics_modes();

	/* Do SDL-specific stuff */
#ifdef USE_SOUND
	sdl_cleanup_sound();
#endif
	sdl_font_quit();
	SDL_Quit();
}

/* "--help" text */
const char help_sdl[] =
"SDL module (single-window client):\n"
"      --width PIXELS        Window width.\n"
"      --height PIXELS       Window height.\n"
"      --bpp BPP             Bits-Per-Pixel, 32 by default.\n"
"      --fullscreen          Run in fullscreen mode.\n"
"      --graphics TILESET    Tileset ID to use, 0 to disable graphics.\n"
"      --sound TOGGLE        0 to disable sound, 1 to enable.\n"
;

/*
 * A "normal" system uses "main.c" for the "main()" function, and
 * simply adds a call to "init_xxx()" to that function, conditional
 * on some form of "USE_XXX" define.
 */
errr init_sdl(void)
{
	
	char buf[1024];
	char *name;
	term_data *td;
	Uint32 initflags = SDL_INIT_VIDEO; /* What's the point, if not video? */

	ANGBAND_SYS = "sdl";

	/* Read config for main window */
	use_graphics = conf_get_int("SDL", "Graphics", 0);
	use_sound = (bool)conf_get_int("SDL", "Sound", 0);
	fullscreen = (bool)conf_get_int("SDL", "Fullscreen", 0);
	width =  conf_get_int("SDL", "Width", 0);
	height = conf_get_int("SDL", "Height", 0);
	bpp = conf_get_int("SDL", "BPP", 32);

	sdl_combine_arrowkeys = conf_get_int("SDL", "CombineArrows", 1);
	sdl_combiner_delay = conf_get_int("SDL", "CombineArrowsDelay", 20);
	mouse_mode = conf_get_int("SDL", "GameMouse", 0);

	/* Read command-line arguments */
	clia_read_int(&width, "width");
	clia_read_int(&height, "height");
	clia_read_bool(&fullscreen, "fullscreen");
	clia_read_int(&bpp, "bpp");
	clia_read_int(&use_graphics, "graphics");
	clia_read_bool(&use_sound, "sound");

#ifdef USE_SOUND
	initflags |= SDL_INIT_AUDIO;
#endif

	/* Force core dumps in Debug mode. Leave friendly stack traces for Normal mode. */
#ifdef DEBUG
	initflags |= SDL_INIT_NOPARACHUTE;
#endif

	/* Init SDL. If this fails, everything else fails */
	if (SDL_Init(initflags) != 0) {
		return -1;
	}

	/*
	 * Here I initialize the screen.
	 */
	flags = 0;
	if (fullscreen) flags |= SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF;

	bigface = SDL_SetVideoMode(width, height, bpp, flags);

#ifdef DEBUG
		if (bigface == NULL) {
			plog("SDL could not initialize video mode."); 
		}
		if (bigface->flags != flags) {
			char asked[200];
			char got[200];

			strncpy(asked, formatsdlflags(flags), 199);
			asked[199] = 0;
			strncpy(got, formatsdlflags(bigface->flags), 199);
			got[199] = 0;
			plog(format("Vid. init.: We  %s and got %s", asked, got));
		}
#endif

	name = SDL_VideoDriverName(buf, 1024);
	if (name && !strcmp(name, "Quartz"))
	{
		quartz_hack = TRUE;
	}

	/* No screen? No game */
	if (bigface == NULL) {
		plog("Shutting down SDL due to error(s).");
		SDL_Quit();
		return -1;
	}

	/* Store (new?) dimensions */
	width = bigface->w;
	height = bigface->h;
	bpp = bigface->format->BitsPerPixel;

	/* Read gui style */
	gui_color_back_ground = sdl_string_color(conf_get_string("SDL-Colors", "BackGround", "0"));
	gui_color_back_active = sdl_string_color(conf_get_string("SDL-Colors", "BackGroundActive", "#222225"));
	gui_color_term_border = sdl_string_color(conf_get_string("SDL-Colors", "TermBorder", "#cccccc"));
	gui_color_term_header = sdl_string_color(conf_get_string("SDL-Colors", "TermTitleBar", "#595961"));
	gui_color_term_title  = sdl_string_color(conf_get_string("SDL-Colors", "TermTitleText", "#ededf9"));

	/* Exit handler */
	atexit(SDL_Quit);

	/* Some SDL settings */
	SDL_WM_SetCaption("MAngband","Ang");
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	/* Use SDL_DISABLE or SDL_ENABLEAd to turn on/off OS Cursor */
	SDL_ShowCursor(SDL_ENABLE);

	/* UNICODE is *required* for weird keyboard layouts */
	SDL_EnableUNICODE(1);

	/*
	 * SDL Initialized just fine!
	 *
	 * Let's do some (M)Angband stuff...
	 *
	 */
	init_color_data_sdl();

	/* Sound */
#ifdef USE_SOUND
	load_sound_prefs();
	sdl_init_sound();
#endif

	/* Load the possible graphics modes */
	init_graphics_modes("graphics.txt");

	/* Init font loading sublibraries */
	sdl_font_init();

	/* Init all 'Terminals' */
	init_all_terms();
	td = &(tdata[0]);

	/* Activate the "Angband" window screen */
	Term_activate(term_screen);

	/* Create cursor surface */
	if (td->cursor_on)
		SDL_PrepareCursor(td->w, td->h);

	/* Activate quit hook */
	quit_aux = quit_sdl;

	/*
	 *
	 * Screen window initialization only ends here!
	 * init_sdl() ought to be split up into smaller functions. XXX
	 * I'll do that later.
	 *
	 */
	return 0;
}
/* Save settings ! */
void save_one_term(int i) {
	term_data *td = &(tdata[i]);
	char sec_name[30];
	char term_name[30];

	if (td->name)
	{
		sprintf(sec_name, "SDL-Term-%s", td->name);
	} else
	{
		switch (i)
		{
			case 0: 	strcpy(term_name, "Angband");	break;
			case 1: 	strcpy(term_name, "Mirror");	break;
			case 2: 	strcpy(term_name, "Recall");	break;
			case 3: 	strcpy(term_name, "Choice");	break;
			default:	sprintf(term_name, "%d", i);
		}
		sprintf(sec_name, "SDL-Term-%s", term_name);
	}

	conf_set_int(sec_name, "Visible", (int)td->online);
	if (td->fd && td->fd->name)
	conf_set_string(sec_name, "Font", td->fd->name);

	/* Bad Hack :( -- since we allow slight overhead, make sure we're in bounds */
	//if (!i && Setup.max_col && !(window_flag[0] & PW_PLAYER_2) && td->cols > Setup.max_col) td->cols = Setup.max_col; /* Compact */
	//if (!i && Setup.max_row && !(window_flag[0] & PW_STATUS)   && td->rows > Setup.max_row) td->rows = Setup.max_row; /* Status line */	

	conf_set_int(sec_name, "Cols", td->cols);
	conf_set_int(sec_name, "Rows", td->rows);
	conf_set_int(sec_name, "PositionX", td->xoff);
	conf_set_int(sec_name, "PositionY", td->yoff);
	conf_set_int(sec_name, "ScaleX", td->sfd.w);
	conf_set_int(sec_name, "ScaleY", td->sfd.h);
}
void save_sdl_prefs() {
	int i;

	/* Root settings */
	conf_set_int("SDL", "Width", width);
	conf_set_int("SDL", "Height", height);
	conf_set_int("SDL", "BPP", bpp);

	conf_set_int("SDL", "Fullscreen", fullscreen);
	conf_set_int("SDL", "Graphics", use_graphics);
	conf_set_int("SDL", "Sound", use_sound);

	conf_set_int("SDL", "CombineArrows", sdl_combine_arrowkeys ? 1 : 0);
	conf_set_int("SDL", "CombineArrowsDelay", sdl_combiner_delay);
	conf_set_int("SDL", "GameMouse", mouse_mode ? 1 : 0);

	/* Terms */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		save_one_term(i);
	}
	conf_save();
}



#endif /* SDL_HEADER */
#endif /* USE_SDL */
