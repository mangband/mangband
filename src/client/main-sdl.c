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


#include "angband.h"





/* 
 *
 * Pre-processor configuration, data structure definitions, global variables,
 * and #includes.
 *
 * The #defines could be moved into config.h if this module was a standard
 * feature of Angband, if people find that helpful.
 *
 * If certain parts of this file are moved to a separate file, the data
 * structures would possibly need to be moved to a header file. That's probably
 * not a good idea since no other display module works that way.
 *
 */


#ifdef USE_SDL 

#include <SDL/SDL.h>
#include <string.h>


/* this stuff was moved to sdl-maim.c */
extern errr SDL_GetPixel (SDL_Surface *f, Uint32 x, Uint32 y, Uint8 *r, Uint8 *g, Uint8 *b);
extern errr SDL_PutPixel (SDL_Surface *f, Uint32 x, Uint32 y, Uint8 r, Uint8 g, Uint8 b);
extern errr SDL_ScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr);
extern errr SDL_FastScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr);
extern SDL_Surface *SDL_ScaleTiledBitmap (SDL_Surface *src, Uint32 t_oldw, Uint32 t_oldh, Uint32 t_neww, Uint32 t_newh, int dealloc_src);

extern errr strtoii(const char *str, Uint32 *w, Uint32 *h);
extern char *formatsdlflags(Uint32 flags);

extern void Multikeypress(char *k);
extern int IsMovement(SDLKey k);
extern char *SDL_keysymtostr(SDL_keysym *ks); /* this is the important one. */

extern errr SDL_init_screen_cursor(Uint32 w, Uint32 h);
//extern errr SDL_DrawCursor(SDL_Surface *dst, SDL_Rect *dr);

extern SDL_Surface *sdl_screen_cursor ;
extern SDL_Rect sdl_screen_cursor_sr;

inline static errr Term_char_sdl (int x, int y, byte a, unsigned char c);
static errr Term_curs_sdl(int x, int y);

/* perhaps this should be in config.h:  (not if it's distributed on its own) */
/*#define USE_HEX_FONTS
#define SDL_MAGIC_KEYS
#define SDL_ALLOW_FLOAT
#define SDL_ALLOW_MAGIC_GOATS
#define SDL_DRINK_TOO_MUCH_COFFEE
#define SDL_BE_EXTRA_STUPID


These #defines were really mostly pointless. All the code they optionally
left out should compile everywhere where the SDL exists, I think.
They just made the source harder to read.
*/

/* You need to define this if compiling for Angband 2.7.9 throug 2.8.x
 * It's probably only relevant for MAngband, though. XXX */

/* META_ADDRESS is probably only defined in MAngband, right? XXX XXX XXX */
#if defined(MANGBAND) || defined(META_ADDRESS) 
#define OLD_TERM_28X
#endif

/*#ifdef USE_GRAPHICS
#define USE_TILE_GRAPHICS
#endif*/


#define CURS_MAG_X 0
#define CURS_MAG_Y 21

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
	/*
	 * to find a character:
	 * x = character * w
	 */
	SDL_Surface *face;
	/* 
	 * font metrics.
	 * Obviously, the font system is very minimalist.
	 */
	Uint8 w;
	Uint8 h;

	Uint8 dw; /* width and height of font on destination surface */
	Uint8 dh;

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

	bool 		online;

	Uint32 width, height; 
	/* XXX width, height, bpp, and flags are only used to provide hints to 
	 * Term_init_sdl(). Consider them write-only values!
	 * If you need  to read the actual window dimensions, use
	 * face->w, face->h, and face->format
	 */

	font_data *fd;

	graf_tiles *gt;

	Uint8 w, h; /* width and height of an individual 2D element */

	Sint32 cx, cy; /* last known cursor coordinates */

	Uint32 xoff,yoff;

	bool prefer_fresh ; /* in case we don't implement FROSH in a graphics engine */
	bool cursor_on;

	bool cursor_magic; /* experimental cursor effects */

	void (*graf_link)(term_data *data, int i); /* if this is set, our term_data_link() will defer to it... */

	/* hooks from main-sdl.c since the other graphics engines will set different
	 * ones in 't'.
	 */
	void (*init_hook)(term *t);
	void (*nuke_hook)(term *t);

	errr (*user_hook)(int n); /* perhaps this useless function will be used */
	errr (*xtra_hook)(int n, int v);
	errr (*curs_hook)(int x, int y);
	errr (*wipe_hook)(int x, int y, int n);
	errr (*text_hook)(int x, int y, int n, byte a, cptr s);
#ifdef USE_TRANSPARENCY
	errr (*pict_hook)(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);
#else /* USE_TRANSPARENCY */
	errr (*pict_hook)(int x, int y, int n, const byte *ap, const char *cp);
#endif /* USE_TRANSPARENCY */

	void *graf; /* extra data for other graphics engines */
};

#ifndef SDL_HEADER

#ifndef ANGBAND_TERM_MAX
#define ANGBAND_TERM_MAX 4
/* this hack is to allow the rest of our init code behave the same in older
 * and newer versions of Angband. */
/* let's declare this static just for the heck of it. */

term *angband_term[ANGBAND_TERM_MAX];
#endif


//#define MAX_TERM_DATA ANGBAND_TERM_MAX
#define MAX_TERM_DATA 8
// ANGBAND_TERM_MAX
static term_data data[MAX_TERM_DATA];

/*
static term_data screen;
static term_data mirror;
static term_data recall;
static term_data choice;
*/


/* main sdl surface + window size and params */

SDL_Surface *bigface;

	Uint32 width, height, bpp, flags; 

	/* XXX width, height, bpp, and flags are only used to provide hints to 
	 * Term_init_sdl(). Consider them write-only values!
	 * If you need  to read the actual window dimensions, use
	 * face->w, face->h, and face->format
	 */


/* Three hackety-hack functions to hack SDL renderer */

void SDL_FillRectXX(term_data *td, Uint32 color) {
	SDL_Rect nr;
	nr.x = td->xoff;
	nr.y = td->yoff;
	nr.w = td->width;//face->w;
	nr.h = td->height;//face->h;

	SDL_FillRect(bigface, &nr, color);
	return;
}

void SDL_FillRectX(SDL_Surface *f, SDL_Rect *r, Uint32 color, int xoff, int yoff) {
	SDL_FillRect(f,r,color);
	if (r == NULL) {
		return;
	} else {
		r->x += xoff;
		r->y += yoff;
//		r->w += xoff;
//		r->h += yoff;
//		SDL_FillRect(bigface, r, color);
	}
	
}

void SDL_UpdateRectX(int x, int y, int w, int h, int xoff, int yoff) {
//	plog(format("[???] updateRectXX: %d,%d -- %d,%d", x+xoff,y+yoff,w+xoff,h+yoff));
//	SDL_UpdateRect(f, x, y, w, h);
	if (x+xoff+w > width) w = width-xoff-x;
	if (y+yoff+h > height) h = height-yoff-y;
	SDL_UpdateRect(bigface, x+xoff, y+yoff, w, h);
}





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
 * Text drawing code. 
 *
 * This could also be moved into another file.
 *
 */



font_data screen_font;
font_data mirror_font;
font_data chat_font;
graf_tiles screen_tiles;



/*
 * Load a HEX font.
 * See http://czyborra.com/unifont/
 *
 * XXX Note. Your file should not be all full-width glyphs. At least one
 * half-width glyph must be present for my lame algorithm to work.
 * It is OK to have all half-width glyphs.
 * 
 * This routine will try to use strtoii() to figure out the font's bounding
 * box from the filename. This seems to be an acceptable thing to try, 
 * as seen in main-win.c
 *
 * FIXME
 * BUGS: There is no attempt made at figuring out a righteous bounding box.
 *	      Certain HEX fonts can be *wider* than 16 pixels. They may break.
 *
 *	What we need is a BDF loader. It's not a high priority though.
 *
 */

#define highhextoi(x) (strchr("ABCDEF", (x))? 0xA + ((x)-'A'):0)
#define hexchartoi(x) (strchr("0123456789", (x))? (x)-'0' : highhextoi((x)))

#ifndef MAX_HEX_FONT_LINE
#define MAX_HEX_FONT_LINE 1024
#endif

errr load_HEX_font_sdl(font_data *fd, cptr filename, bool justmetrics)
{
	FILE *f;

	char buf[1036]; /* 12 (or 11? ;->)extra bytes for good luck. */

	char gs[MAX_HEX_FONT_LINE]; /* glyph string */

	Uint32 i,j;

	errr fail = 0; /* did we fail? */

	Uint32 x; /* current x in fd->face */
	Uint32 y; /* current y in fd->face */

	Uint32 gn; /* current glyph n */

	Uint32 n; /* current nibble or byte or whatever data from file */

	Uint32 pos; /* position in the nasty string */

	Uint32 bytesdone; /* bytes processed */

	Uint32 mw, mh; /* for strtoii() */



	/* check font_data */
	if (fd->w || fd->h || fd->face) return 1; /* dealloc it first, dummy. */

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_XTRA, filename);

	f = fopen(buf, "r");

	if (!f) 
	{
		plog(format("Couldn't open: %s", buf));
		return -1;
	}

	

	/* try hard to figure out the font metrics */

	while (fgets(gs, MAX_HEX_FONT_LINE, f) != NULL)
	{
		i = strlen(gs);

		if (gs[i-1] == '\n') i--;
		if (gs[i-1] == '\r') i--;
		
		i -= 5; /* each line begins with 1234: */

		/* now i is the number of nibbles in the line */

		if (i & 1)
		{
			plog("Error in HEX line measurment. Report to hmaon@bumba.net.");
			fclose(f);
			fail = -1;
			break;
		}

		i >>= 1; /* i is number of bytes */

		if (!fd->h)
		{
			fd->dw = fd->w = 8; /* a nasty guess. */
			fd->dh = fd->h = i;
			/*if (i & 1) break;*/ /* odd number of bytes. this is the height. */
		} else 
		{
			if (i > fd->h) {
				fd->dw = fd->w = 16; /* an even nastier guess (full-width glyphs here) */
				if(fd -> h / 2 == i / 3)
				{
					/* this sucks. */
					fd->dh = fd->h = i / 3;
					fd->dw = fd->w = 24;
				} else
				if(i != (fd->h)*2) /* check sanity and file integrity */
				{
					plog("Error 2 in HEX measurement.");
					/*fail = -1;*/
				}
				break; /* this is a full-width glyph. We have the height. */
			} else
			if (i < fd->h) {
				if (i*2 != fd->h)
				{
					plog("Error 3 in HEX measurement.");
					/*fail = -1;*/
				}
				fd->dw = fd->w = 16; /* the same nastier guess. */
				fd->dh = fd->h = i; /* Ah, so this is the height */
			}
			/* they're equal. we can say nothing about the glyph height */
		}
	}

	/* analyze the file name */
	if(!strtoii(filename, &mw, &mh))
	{
		/* success! */
		fd->dw = mw;
		fd->dh = mh;
	} else
	{
		plog("You may wish to incude the dimensions of a font in its file name. ie \"vga8x16.hex\"");
	}

	if (justmetrics) 
	{
		fclose(f);
		return fail;
	}

	/* Might as well allocate the bitmap here. */
	/* XXX I'm not sure what pixel format to use but the text blitter is 
	 * probably the wrong thing to start optimizing.
	 */
	fd->face = SDL_CreateRGBSurface(SDL_SWSURFACE, fd->w, 256*fd->h,8,0,0,0,0); 
	if(!(fd->face)) return -1;
	SDL_SetAlpha(fd->face, SDL_RLEACCEL, SDL_ALPHA_OPAQUE); /* use RLE */

	rewind(f);

	while (fgets(gs, MAX_HEX_FONT_LINE, f) != NULL)
	{
#ifdef FONT_LOAD_DEBUGGING
		puts("");
		puts(gs);
#endif
		/* figure out character code (aka index). xxxx:... */
		if (sscanf(gs, "%4x", &gn) != 1)
		{
			plog("Broken HEX file.");
			fail = -1;
			break;
		}

#ifdef FONT_LOAD_DEBUGGING
		printf("%4x:\n", gn);
#endif
		if (gn > 255) {
			gn = 255;
		}

		x = 0; 
		y = fd->h * gn;
		
		i = strlen(gs);

		if (gs[i-1] == '\n') {
			i--;
			gs[i] = '\0';
		}
		if (gs[i-1] == '\r') 
		{
			i--;
			gs[i] = '\0';
		}
		
		i -= 5; /* each line begins with 1234: */
		/* now i is the number of nibbles represented in the line */
		i >>= 1; /* now bytes. */

		pos = 5;
		bytesdone = 0; 

		while (gs[pos] != '\0' && pos < strlen(gs)) 
		{
			n  = (hexchartoi(gs[pos])) << 4; pos++; 
			n += (hexchartoi(gs[pos])); pos++;
			/* they're macros. do NOT pass a "pos++" to them! :) :) :) */

			for(j = 0; j < 8; ++j, ++x, n <<= 1)
			{
				if (n & 0x80) 
				{
#ifdef FONT_LOAD_DEBUGGING
					printf("#");
#endif
					((Uint8 *)fd->face->pixels)[x + y*fd->face->pitch] = 0xff;
				} else
				{
#ifdef FONT_LOAD_DEBUGGING
					printf("-");
#endif
					((Uint8 *)fd->face->pixels)[x + y*fd->face->pitch] = 0x00;
				}
			}
			++bytesdone;

			/* processing half-width glyph or just finished even byte */
			if (i == fd->h || ((i == 2*fd->h) && !(bytesdone & 1))) 
			{
				x = 0;
				++y;
#ifdef FONT_LOAD_DEBUGGING
				printf("\n");
#endif
			} else if (i == 2*fd->h)
			{
				/* XXX do nothing? */
			} else 
			{
				/* XXX XXX XXX softer errors since HEX seems to actually permit
				 * this situation
				 */
				/*plog("HEX measurement error, fd->h is not a multiple of i.");*/
				/*fail = -1;*/
			}
		} /* while (gs[pos... */
	} /* while (fgets... */

	return fail;
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
static void Term_init_sdl(term *t)
{
	
/*
	term_data *td = (term_data*)(t->data);

	td->face = SDL_SetVideoMode(td->width, td->height, td->bpp, td->flags);

	if (td->width && td->height) {
		td->face = SDL_CreateRGBSurface(SDL_SWSURFACE, td->width, td->height, 8,0,0,0,0); 
	} 
*/


}



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

	if (td->online) 
		td->online = FALSE;

	if (td->face) 
	{
		SDL_FreeSurface(td->face); /* what happen! someone set up us the bomb! */
		td->face = NULL;
	}

	if (bigface) 
	{
		SDL_FreeSurface(bigface); /* --all-- your base */
		bigface = NULL;
	}

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
	/*term_data *td = (term_data*)(Term->data);*/

	/* XXX XXX XXX Handle the request */

	/* TODO What? Huh? */

	/* Unknown */
	return (1);
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


	/* Analyze */
	switch (n)
	{
		case TERM_XTRA_EVENT:
		if (1 == 1) {
		do {
			if (v) 
			{
				if (!SDL_WaitEvent(&event)) return(0); /* TODO handle errors */
				v = 0;
			} else 
			{
				if(!SDL_PollEvent(&event)) return(0);
			}
			if (event.type == SDL_QUIT) 
			{
				quit("Goodbye.");
			} else
			if (event.type == SDL_KEYDOWN) 
			{
				if (event.key.state == SDL_PRESSED) 
				{
					/* Various frivolous hacks. */
					switch(event.key.keysym.sym)
					{
						char buf[1024];
						FILE *tmp;
						int i;
#ifndef OLD_TERM_28X /* I'll deal with MAngband later. XXX */
					/* Try to toggle graphics. */
					case SDLK_SCROLLOCK:
						use_graphics = !use_graphics;
						data[0].t.higher_pict = !data[0].t.higher_pict;
#ifndef SDL_OLD_RESET_VISUALS
						reset_visuals(TRUE);
#else
						reset_visuals();
#endif

						/*sprintf(buf, "%s-%s.prf", (use_graphics ? "graf" : "font"), ANGBAND_SYS);*/
#endif

					/* Try to save a screenshot. */
					case SDLK_F12:
						if (event.key.keysym.mod & KMOD_ALT) {
							if (SDL_SaveBMP(bigface, "newshot.bmp")) 
							{
								plog("You fail to get the screenshot off!");
								break;
							}
							for (i = 0; i < 999; ++i) {
								snprintf(buf, 1024, "%03d.bmp", i);
								if ((tmp = fopen(buf, "rb")) != NULL)
								{
									fclose(tmp);
									continue;
								}
								rename("newshot.bmp", buf);
							}
							plog("*click*");
						}
						break;
					default:
						break;
					} /* switch */
				Multikeypress(SDL_keysymtostr(&event.key.keysym));

				} /* SDL_PRESSED */ 
			} 
		} while (SDL_PollEvent(NULL));
		} //before do

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

		/* a NULL dstrect fills the entire window */
		/* XXX the color 0 will not necessarily work for 8-bit modes */

//		SDL_FillRectX(td->face, NULL, 0, td->xoff, td->yoff);
		SDL_FillRectXX(td, 0);
//		SDL_UpdateRectX(td->face, 0, 0, td->width, td->height, td->xoff, td->yoff);

		return (0);

		case TERM_XTRA_SHAPE:

		/* XXX XXX XXX Set the cursor visibility (optional) */
		/* This action should change the visibility of the cursor, */
		/* if possible, to the requested value (0=off, 1=on) */
		/* This action is optional, but can improve both the */
		/* efficiency (and attractiveness) of the program. */
		

		td->cursor_on = n ? TRUE : FALSE;

		return (0);

		case TERM_XTRA_FROSH:

		/* XXX XXX XXX Flush a row of output (optional) */
		/* This action should make sure that row "v" of the "output" */
		/* to the window will actually appear on the window. */
		/* This action is optional on most systems. */

		if (!td->online) return -1;
		if (td->prefer_fresh) return 0;
//		SDL_UpdateRect(bigface, 0, v*td->h, td->face->w, td->h);
		SDL_UpdateRectX( 0, v*td->h, td->width, td->h, td->xoff, td->yoff);


		return (0);

		case TERM_XTRA_FRESH:

		/* XXX XXX XXX Flush output (optional) */
		/* This action should make sure that all "output" to the */
		/* window will actually appear on the window. */
		/* This action is optional if all "output" will eventually */
		/* show up on its own, or when actually requested. */



		if (!bigface) return -1;
		if ((bigface->flags & SDL_HWSURFACE && bigface->flags & SDL_DOUBLEBUF)
		    || td->prefer_fresh) 
		{
			SDL_Flip(bigface);
		}

		return (0);

		case TERM_XTRA_NOISE:

		/* XXX XXX XXX Make a noise (optional) */
		/* This action should produce a "beep" noise. */
		/* This action is optional, but nice. */

		return (0);

		case TERM_XTRA_SOUND:

		/* XXX XXX XXX Make a sound (optional) */
		/* This action should produce sound number "v", where */
		/* the "name" of that sound is "sound_names[v]". */
		/* This action is optional, and not important. */

#ifdef USE_SDL_MIXER
		/* TODO make some noise. */
#else
		/* TODO We can actually make noise without the mixer too... */
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

		SDL_Flip(bigface); /* I guess... XXX XXX XXX */ 

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
static errr Term_wipe_sdl(int x, int y, int n)
{
	SDL_Rect dr;
	term_data *td = (term_data*)(Term->data);

	/*printf("%d, %d, %d\n", x, y, n);*/

	if (!td->gt || !td->fd) return 0;


	if (use_graphics)
	{
		dr.w = n * td->w;
		dr.h = td->h;
		dr.x = td->w * x;
		dr.y = td->h * y;
/*		SDL_FillRectX(td->face, &dr, 0, td->xoff, td->yoff); ??????? */
		if (td->cx == x && td->cy == y)
		{
			SDL_UpdateRectX( dr.x, dr.y, dr.w, dr.h, td->xoff, td->yoff);
			if (td->cursor_magic && td->t.higher_pict)
			{
				dr.x = CURS_MAG_X*td->w;
				dr.y = CURS_MAG_Y*td->h;
				dr.w = 2*td->w;
				dr.h = 2*td->h;
/*				SDL_FillRectX(td->face, &dr, 0, td->xoff, td->yoff); ???? */
				SDL_UpdateRectX( dr.x, dr.y, dr.w, dr.h, td->xoff, td->yoff);
			}
		}
	}


	/* Success */
	return (0);
}


/*
void RedrawChar(int x, int y)
{
	Uint8 a, c;

	a = Term->scr->a[y][x];	c = Term->scr->c[y][x];
	
	//Term_wipe_sdl(x, y, 1);
	Term_char_sdl(x, y, a, c);
}
*/
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
	SDL_Rect dr, mr, gr; /* cursor destination, magic, and graphic tile loc. */
	Uint8 a, c;
	
	//if (td->cx != -1 && td->cy != -1) { 
	//		RedrawChar(td->cx, td->cy); 
	//	}
	if (td->cx == x && td->cy == y) return;
	//if (x == -1 && y == -1) return;
	if (td->cursor_on) 
	{
		//Term_wipe_sdl(	td->cx,	td->cy, 1);
				
		td->cx = x, td->cy = y;
		dr.x = x * td->w;
		dr.y = y * td->h;
		dr.w = td->w;
		dr.h = td->h;
		if(td->cursor_magic && td->t.higher_pict && td->gt)
		{
			mr.x = CURS_MAG_X*td->w;
			mr.y = CURS_MAG_Y*td->h;
			mr.w = td->w*2;
			mr.h = td->h*2;
#if 1 /* the following code assumes knowledge of the term struct */
			/* it also allows one to view the magnified tile with all detail
			 * even if tiles in the display are scaled down, thus losing detail */
			a = Term->scr->a[y][x];
			c = Term->scr->c[y][x];
			if (a & 0x80 && c & 0x80) /* don't bother magnifying letters */
			{
				a &= 0x7F;
				c &= 0x7F;
				gr.x = c * td->gt->w;
				gr.y = a * td->gt->h;
				gr.w = td->gt->w;
				gr.h = td->gt->h;
/*				SDL_FastScaleBlit(td->gt->face, &gr, td->face, &mr); ??? */
				SDL_UpdateRectX( mr.x, mr.y, mr.w, mr.h, td->xoff, td->yoff);
			}
#else
//			SDL_FastScaleBlit(td->face, &dr, td->face, &mr);
			SDL_UpdateRectX( mr.x, mr.y, mr.w, mr.h, td->xoff, td->yoff);
#endif
		}
		//draw char beneath it		
		//Term_wipe_sdl(x,y,1);
		//RedrawChar(x, y);
		
		SDL_DrawCursorX(&dr);
		SDL_UpdateRectX(dr.x, dr.y, dr.w, dr.h, td->xoff, td->yoff);
	}


	/* Success */
	return (0);
}

errr SDL_DrawCursorX(SDL_Rect *dr)
{
	//SDL_Surface* sdl_screen_cursor = SDL_return_screen_surface();
	if (!dr || !sdl_screen_cursor) return -1;
	if (SDL_BlitSurface(sdl_screen_cursor, &sdl_screen_cursor_sr, bigface, dr)) return -1;
	//SDL_UpdateRectX( dr.x, dr.y, dr.w, dr.h, dr->x, dr->y, dr->w, dr->h);
	return 0;
}

/* x and y are in !!PIXELS!! This is because we don't know tile dimensions
 * for the destination bitmap, by design. XXX XXX XXX
 * It allows us to draw on arbitrary SDL_Surfaces
 */
inline static errr SDL_DrawChar ( Uint32 x, Uint32 y, font_data *fd, Uint8 a, Uint8 c, int xoff, int yoff)
{
	SDL_Rect sr, dr, ndr;

//	if (!f) return -1;

	dr.w = sr.w = fd->dw;
	dr.h = sr.h = fd->dh;

	sr.x = 0;
	sr.y = c * fd->h;

	dr.x = x;
	dr.y = y;

	if (fd->precolorized)
	{
		sr.x = a * fd->w;
	} else
	{
		/* XXX Force SDL, or whatever it wraps, to make the text the color we want
		 * by tweaking the palette. This really is slower than blits between
		 * surfaces with identical color formats but it's so easy and convenient!
		 * Anyway, it seems to be fast enough on my machine. 
		 * Alternately, if fd->precolorized is set, assume that the font has been
		 * drawn in all possible colors in the source bitmap already. I only
		 * use this for scaled fonts since they need blurring.
		 */
		SDL_SetColors(fd->face, &(color_data_sdl[a&0xf]), 0xff, 1);
	}

	/*if(SDL_MUSTLOCK(f)) SDL_LockSurface(f);*/
	ndr = dr;
//no need actually!
//	SDL_BlitSurface(fd->face, &sr, f, &dr);

	ndr.x = ndr.x + xoff;
	ndr.y = ndr.y + yoff;
//	ndr.w = ndr.w + xoff;
//	ndr.h = ndr.h + yoff;

//TJH!
	SDL_BlitSurface(fd->face, &sr, bigface, &ndr);
//	SDL_UpdateRect(bigface, dr.x,dr.y, dr.w, dr.h);



//	SDL_ScaleBlit(f, &sr, bigface, &dr);

	/*if(SDL_MUSTLOCK(f)) SDL_UnlockSurface(f);*/

	return 0; /* check for failure, perhs? */
}


/* The following draws one character to the Term, using font_data. */
inline static errr Term_char_sdl (int x, int y, byte a, unsigned char c){
	/*SDL_Rect sr, dr;*/
	term_data *td = (term_data*)(Term->data);
	int xadj, yadj;

	if (!td->online) return -1;
	
	Term_wipe_sdl(x, y, 1);

	xadj = td->w > td->fd->w ? td->w - td->fd->w : 0;
	yadj = td->h > td->fd->h ? td->h - td->fd->h : 0;

	if (SDL_DrawChar( x*td->w + xadj, y*td->h + yadj, td->fd, a, c, td->xoff, td->yoff)) 
	{
		plog(">:(");
		return -1;
	}
	
	if (td->cursor_on && td->cx == x && td->cy == y)
	{
		//SDL_UpdateRect(td->face, x*td->w, y*td->h, td->w, td->h);
		SDL_UpdateRectX( x*td->w, y*td->h, td->w, td->h, td->xoff, td->yoff);
	
		td->cx = td->cy = -1;
	}


	/* Success */
	return (0);
}



/* This one draws a graphical tile. */
inline static errr Term_tile_sdl (int x, int y, Uint8 a, Uint8 c){
	SDL_Rect sr, dr;
	/*int n;*/
	term_data *td = (term_data*)(Term->data);

	if(!td->online) return -1;

	Term_wipe_sdl(x, y, 1);

	sr.w = td->gt->w;
	sr.h = td->gt->h;

	dr.w = td->w;
	dr.h = td->h;

	sr.x = (c & 0x7F) * td->gt->w;
	sr.y = (a & 0x7F) * td->gt->h;


	dr.x = x * td->w;
	dr.y = y * td->h;

	if (sr.x > td->gt->face->w)
	{
/*
		SDL_FillRectX(td->face, &dr, SDL_MapRGB(td->face->format, 255, 64, 64), td->xoff, td->yoff);
		plog(format("OOBound (%d, %d) (%d, %d bitmap)", c&0x7f, a&0x7f, sr.x, sr.y)); 
?????
*/
	} else
	{
/*
		SDL_BlitSurface(td->gt->face, &sr, td->face, &dr);
?????
*/
	}
	if (td->cx == x && td->cy == y)
	{
		SDL_UpdateRectX( dr.x, dr.y, dr.w, dr.h, td->xoff, td->yoff);
	}

	/* Success */
	return (0);
}


/*
 * XXX XXX XXX Draw a "picture" on the screen
 *
 * This routine should display the given attr/char pair at the
 * given location (x,y).  This function is only used if one of
 * the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pair, when displayed, will
 * erase anything (including any visual cursor) that used to be
 * at the given location.  On many machines this is automatic, on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * "(a&0x7F)" as a "row" and "(c&0x7F)" as a "column" to index into
 * a special auxiliary pixmap of special pictures.
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn one at a time, instead of trying
 * to "collect" the attr/char pairs into "strips" with similar "attr"
 * codes, which would be sent to "Term_text_xxx()".
 *
 * This is the implementation of the text and 2D tile graphics display.
 * TODO Implement and wrap other graphics engines. They should replace
 * Term_pict_sdl as the hook in term, probably.
 *
 */


static errr Term_pict_sdl(int x, int y, int n,  const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	if (!td->gt || !td->gt->face)
	{
//		errr static Term_text_sdl(int x, int y, int n, const byte *ap, const char *cp);
//		Term_text_sdl(x, y, n, ap, cp);
	} else
	while(n--)
	{

		if (td->gt && td->gt->face) /* it never hurts (much) to check */
		{
			Term_tile_sdl(x, y, *ap, *cp); /* draw a graphical tile */
		} 
	}

	/* Success */
	return (0);
}

/* We need a different version of Term_pict_sdl() for older versions
 * of the Angband term code. This is mostly only necessary for MAngband.
 */
#ifdef OLD_TERM_28X
static errr Term_pict_sdl_28x(int x, int y, byte a, char c)
{
	return Term_pict_sdl(x, y, 1, &a, &c);
}
#endif

/* We also need a different version in case transparency is enabled!
 * Of course, that's not implemented yet but what the heck...
 */

#ifdef USE_TRANSPARENCY
static errr Term_pict_sdl_trans(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);
	/* TODO load the "mask" into the Alpha channel of the graphics tiles
	 * at load-time.
	 */

	td->pict_hook(x, y, n, tap, tcp);
	td->pict_hook(x, y, n, ap, cp);
}
#endif

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

	term_data *td = &(data[i]);
	term *t = &(td->t);



/*	plog(format("setting up data link for term %d (%s) add %d",i,td->name,t)); */

	int j = 0;
	if (i == 0) j = 256;

	/* Initialize the term */
	if (!td->graf_link) 
	term_init(t, 80, 24, j);


	/* XXX XXX XXX Choose "soft" or "hard" cursor */
	/* A "soft" cursor must be explicitly "drawn" by the program */
	/* while a "hard" cursor has some "physical" existance and is */
	/* moved whenever text is drawn on the screen.  See "term.c". */

	t->soft_cursor = TRUE;

	/* XXX XXX XXX Avoid the "corner" of the window */
	/* t->icky_corner = TRUE; */

	/* XXX XXX XXX Use "Term_pict()" for all data */
	/* See the "Term_pict_xxx()" function above. */
	/* t->always_pict = TRUE; */

	/* XXX XXX XXX Use "Term_pict()" for "special" data */
	/* See the "Term_pict_xxx()" function above. */
	/* t->higher_pict = TRUE;*/ /* XXX Should this be set or not? */

	/* XXX XXX XXX Use "Term_text()" for all data */
	/* See the "Term_text_xxx()" function above. */
	/* t->always_text = TRUE; */

	/* XXX XXX XXX Ignore the "TERM_XTRA_BORED" action */
	/* This may make things slightly more efficient. */

	t->never_bored = TRUE;

	/* XXX XXX XXX Ignore the "TERM_XTRA_FROSH" action */
	/* This may make things slightly more efficient. */
	/* t->never_frosh = TRUE; */

	/* Erase with "white space" */

	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';



	/* Prepare the init/nuke hooks */
//if (i == 0) {
//	td->init_hook = t->init_hook = Term_init_sdl;
	td->nuke_hook = t->nuke_hook = Term_nuke_sdl;
//}

	/* Prepare the template hooks */
	td->user_hook = t->user_hook = Term_user_sdl;
	td->wipe_hook = t->wipe_hook = Term_wipe_sdl;
	td->curs_hook = t->curs_hook = Term_curs_sdl;
	td->text_hook = t->text_hook = Term_text_sdl;
	td->xtra_hook = t->xtra_hook = Term_xtra_sdl;





#ifdef OLD_TERM_28X /* These #ifdefs are essential. Sorry. */
	td->pict_hook = t->pict_hook = Term_pict_sdl_28x;
#else
# ifdef USE_TRANSPARENCY
	td->pict_hook = t->pict_hook = Term_pict_sdl_trans;
# else
	td->pict_hook = t->pict_hook = Term_pict_sdl;
# endif
#endif

	/* Remember where we came from */
//	t->data = td;//(vptr)(td);
	t->data = (vptr)(td);


//	if (i == 1) return;

	if (td->graf_link)
	{
		td->graf_link(data, i);
	} else
	{
		/* Activate it */
		Term_activate(t);
	}
}




/*
 * A "normal" system uses "main.c" for the "main()" function, and
 * simply adds a call to "init_xxx()" to that function, conditional
 * on some form of "USE_XXX" define.
 */

/* two gross functions to load fonts */

void dirtyHEX(font_data *fd, cptr str) {
	fd->w = fd->h = 0; fd->face = NULL;
	load_HEX_font_sdl(fd, str, TRUE);

}

errr cleanHEX(font_data *fd, cptr str) {
	memset(fd, 0, sizeof(font_data));
	fd->face = NULL; /* to be pedantic */
	if(load_HEX_font_sdl(fd, str, 0)) 
	{
		plog("load_HEX_font_sdl() failed...");
		return -1;
	}
	return 1;
}


/*
 * XXX XXX XXX Initialization function
 */
void init_one_term(int i, font_data* fd, int xoff, int yoff, bool scale_fit, bool scale_tiles, Uint32 fw,Uint32 fh,Uint32 ftw,Uint32 fth) 
{
	term_data *td;
	td = &(data[i]);
	(void)WIPE(td, term_data);

if (i == 0)	td->name = "Angband";
if (i == 1)	td->name = "Mirror";
if (i == 2)	td->name = "Recall";
if (i == 3)	td->name = "Choice";
if (i == 4)	td->name = "Term-4";

	td->face = NULL;
	td->online = FALSE;

	td->xoff = xoff; 
	td->yoff = yoff;

	td->fd = fd;//&screen_font;
	td->gt = &screen_tiles;

	int rs = 24;
	if (i == 2) rs = 13; /* only 13 rows in Recall window (equip) */

	td->width = td->fd->dw * 80;
	td->height = td->fd->dh * rs;

/*	td->face = SDL_CreateRGBSurface(SDL_SWSURFACE, td->width, td->height, 8,0,0,0,0);  */
	td->online = TRUE;

	if(td->w < td->fd->dw) td->w = td->fd->dw; 
	if(td->h < td->fd->dh) td->h = td->fd->dh; 

	if (use_graphics)
	{
		if (scale_tiles && ftw && fth)
		{
			if (td->w < ftw) td->w = ftw;
			if (td->h < fth) td->h = fth;
		} else 
		if (!scale_fit)
		{
			if (td->w < screen_tiles.w) td->w = screen_tiles.w;
			if (td->h < screen_tiles.h) td->h = screen_tiles.h;
		}
	}
	


	/* Allow a manual override. It's all about the manual override. */
	if(fw && fh)
	{
		td->w = fw;
		td->h = fh;
	}


	td->cursor_on = TRUE;
	td->cursor_magic = TRUE;

	td->cx = -1;
	td->cy = -1;
	
	

//#ifdef USE_XXX
//	if (use_xxx)
//	{
//		typedef void (*link_callback)(term_data *, int);
//		extern link_callback init_xxx(int, char**);
//		td->graf_link = init_xxx(oargc, oargv);
//	}
//#endif

//#ifdef USE_ISO
//	if (use_iso)
//	{
//		typedef void (*link_callback)(term_data *, int);
//		extern link_callback init_iso(int, char**);
//		td->graf_link = init_iso(oargc, oargv);
//	} 
//#e//ndif

}
errr init_sdl(int oargc, char **oargv)
{
	int argc = oargc;
	char **argv = oargv;

#ifdef WINDOWS
	/* Initialize WinSock */
	WSADATA wsadata;
	WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif

	term_data *td;
	Uint32 initflags = SDL_INIT_VIDEO; /* What's the point, if not video? */
	char path[1024];
	char fontname[64];
	char smallfontname[64];
	char midfontname[64];

	char tilebmpname[64];
	char *a; /* temp. pointer for cmd line arguments */
	Uint32 fh = 0, fw = 0; /* term_data.w and .h overrides */
	Uint32 gh = 0, gw = 0;  /* dimensions of graphical tilse.  */

	Uint32 ftw = 0, fth = 0; /* dimensions to scale tiles to, optionally */

	int fullscreen = 0;

	width = 1280;
	height = 768;

//	int bpp = 0;

	/*int i;*/  /* the ever-handy 'i'. This is not an icky thing. XXX */

	bool scale_tiles = FALSE;
	bool scale_fit = FALSE;

	/*bool scale_fonts = FALSE; TODO */

#ifdef USE_XXX
	bool use_xxx = FALSE;
#endif

#ifdef USE_ISO
	bool use_iso = TRUE;
#endif


#ifdef OLD_TERM_28X
	use_graphics = 0;
//har smallfontname[64];
//	strcpy(midfontname, "font/vga8x16.hex");
	strcpy(smallfontname, "font/misc6x13.hex");
	strcpy(fontname, "font/nethack10x19-10.hex");
	strcpy(tilebmpname, "graf/16x16.bmp");
#else
	use_graphics = (arg_graphics == TRUE);
	ANGBAND_GRAF = "new"; /* not necessarily right.. set again below. XXX */
	/*ANGBAND_SYS = "sdl";*/ 
	strcpy(fontname, "font/vga8x16.hex"); /* slashes are system specific!! FIXME */
	strcpy(tilebmpname, "graf/16x16.bmp");
#endif



	--argc, ++argv; /* skip program name */
#define getarg ((argc-- > 0) ? (*(argv++)) : " ")
#define ungetarg (++argc, --argv)
	while(argc > 0) { 
		a = getarg;

		if (!strcmp(a, "--hexfont"))
		{
			strncpy(fontname, getarg, 63);
			fontname[63] = '\0';
			/*if(strchr(fontname, '/')) 
			{
				plog("Disallowed character(s) in font filename.");
				strcpy(fontname, "vga.hex");
			}*/
		} else

		if (!strcmp(a, "--tiles") || !strcmp(a, "--graf")) 
		{
			strncpy(tilebmpname, getarg, 63);
			tilebmpname[63] = '\0';
		} else

		if (!strcmp(a, "--settilesize") || !strcmp(a,"--fwh")) 
		{
			sscanf(getarg, "%d", &fw);
			sscanf(getarg, "%d", &fh);
		} else

		if (!strcmp(a, "--fullscreen"))
		{
			fullscreen = 1;
		} else

		if (!strcmp(a, "--gfx") || !strcmp(a, "-g")) 
		{
			use_graphics = 1;
		} else

		if (!strcmp(a, "--width"))
		{
			sscanf(getarg, "%d", &width);
		} else

		if (!strcmp(a, "--height"))
		{
			sscanf(getarg, "%d", &height);
		} else

		if (!strcmp(a, "--bpp"))
		{
			sscanf(getarg, "%d", &bpp);
		} else

		if (!strcmp(a, "--scaletiles"))
		{
			scale_tiles = TRUE;
			sscanf(getarg, "%d", &ftw);
			sscanf(getarg, "%d", &fth);
		} else
		if (!strcmp(a, "--scale"))
		{
			scale_fit = TRUE;
			scale_tiles = FALSE; /* we're resizing to fit screen tile size */
		}

#ifdef USE_XXX
		if (!strcmp(a, "--xxx"))
		{
			use_xxx = !use_xxx;
		}
#endif

#ifdef USE_ISO
		if (!strcmp(a, "--iso"))
		{
			use_iso = !use_iso;
		}
#endif

	}
		

	if (!use_graphics) scale_fit = TRUE; 
	/* so they look OK if you turn them on later... XXX */


#ifdef USE_SDL_MIXER
	initflags |= SDL_INIT_AUDIO;
	/* TODO perhaps use SDL_InitSubSystem() instead. */
#endif

	/* I don't think you'd want the following (except for core dump): */
#ifdef SDL_NOPARACHUTE
	initflags |= SDL_INIT_NOPARACHUTE;
#endif

	/* This isn't supposed to do anything on Windows but it may break things!
	 * XXX XXX XXX */
	/*initflags |= SDL_INIT_EVENTTHREAD;*/


	if (SDL_Init(initflags) != 0) {
		return -1;
	}

	atexit(SDL_Quit); 

	init_color_data_sdl();

#ifdef OLD_TERM_28X
//	strcpy(path,"lib/"); /* XXX XXX XXX this is system dependent!!!! FIXME */
//	init_file_paths(path);
	init_stuff();
#endif



	/*
	 *
	 * Here I initialize the screen window.
	 * At the moment, it is the only window. 
	 * Even if additional windows are added, this one will still have special
	 * treatment.
	 */

	/* XXX possibly, these should be calculated in Term_init_sdl.c */

	width = width;
	height = height;
	bpp = bpp; 
		/* 0 is for current display bpp */

	if (fullscreen) flags |= SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF;


			bigface = SDL_SetVideoMode(width, height, bpp, flags);

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
				plog(format("Vid. init.: We asked for %s and got %s", asked, got));
			}


	if (bigface == NULL) {
		plog("Shutting down SDL due to error(s).");
		SDL_Quit();
		return -1;
	}



	/* XXX tile sizes will have to be figured out before any of the windows
	 * above can be initialized so move that code if you use them
	 * for something.
	 */
	/* XXX Hack. get font metrics and tile size to calculate window size. */


	dirtyHEX(&screen_font, fontname);
	dirtyHEX(&mirror_font, smallfontname);
/*	dirtyHEX(&chat_font, midfontname); */


	screen_tiles.w = screen_tiles.dw = gw;
	screen_tiles.h = screen_tiles.dh = gh;
	if (strtoii(tilebmpname, &gw, &gh))
	{
		// strtoii() has failed. keep the (likely wrong) default */
		plog(format("strtoii() failed for %s", tilebmpname));
	} else
	{
		/*plog(format("%s yielded %d, %d by strtoii()", tilebmpname, gw, gh));*/
		screen_tiles.w = screen_tiles.dw = gw;
		screen_tiles.h = screen_tiles.dh = gh;
		/* This will enlarge the window tile size to fit all elements.
		 * Of course, this might not be the behaviour you want.
		 */


		/* I can't believe that this is what I'm supposed to do: XXX XXX XXX*/
		if (gw == 8 && gh == 8)
		{
			#ifdef OLD_TERM_28X

			#else
			ANGBAND_GRAF = "old";
			#endif
		}
	}


	/* "Angband" window screen */


	init_one_term(0,&screen_font, 0, 0,scale_fit,scale_tiles,fw,fh,ftw,fth);
	td = &(data[0]);
	term_data_link(0);
	term_screen = &td->t;

	/* Mirror */

	init_one_term(1,&mirror_font, 0, td->height ,scale_fit,scale_tiles,fw,fh,ftw,fth);
	term_data_link(1);
	term_mirror = &(data[1].t);


	/* Recall */

	init_one_term(2,&mirror_font, td->width, 0 ,scale_fit,scale_tiles,fw,fh,ftw,fth);
	//data[2].height = data[2].height / 2;
	term_data_link(2);
	term_recall = &(data[2].t);


	/* Choice */

	init_one_term(3,&mirror_font, td->width, data[2].height ,scale_fit,scale_tiles,fw,fh,ftw,fth);
	term_data_link(3);
	term_choice = &(data[3].t);


	/* Term-4 		used for chat in Mangband	*/

	init_one_term(4,&mirror_font, data[1].width , td->height,scale_fit,scale_tiles,fw,fh,ftw,fth);
	term_data_link(4);
	ang_term[4] = &(data[4].t);


	/* Activate the "Angband" window screen */
	Term_activate(&(td->t));


	SDL_WM_SetCaption("MAngband","Ang");
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	/* Adjust those to turn on/off OS Cursor */
	//SDL_ShowCursor(SDL_DISABLE);
	SDL_ShowCursor(SDL_ENABLE);

	SDL_EnableUNICODE(1);

	if (!cleanHEX(&screen_font, fontname)) return -1;
	if (!cleanHEX(&mirror_font, smallfontname)) return -1;
/*	if (!cleanHEX(&chat_font, midfontname)) return -1; */


	path_build(path, 1023, ANGBAND_DIR_XTRA, tilebmpname);

	if((screen_tiles.face = SDL_LoadBMP(path)) == NULL)
	{
		plog(format("Sorry, could not load %s", path));
	} else
	{
		td->t.higher_pict = use_graphics;
		if (scale_fit || (scale_tiles && ftw && fth))
		{
			if (scale_fit) ftw = td->w, fth = td->h;

			screen_tiles.face = SDL_ScaleTiledBitmap(screen_tiles.face, 
			                                         screen_tiles.w,
								  screen_tiles.h,
								  ftw,
								  fth,
								  TRUE);
			if (!screen_tiles.face)
			{
				td->t.higher_pict = use_graphics = 0;
			} else
			{
				screen_tiles.w = ftw;
				screen_tiles.h = fth;
				/*SDL_SaveBMP (screen_tiles.face, "dump.bmp");*/
			}
		}
	}


	SDL_init_screen_cursor(td->w, td->h);

	/*
	 *
	 * Screen window initialization only ends here!
	 * init_sdl() ought to be split up into smaller functions. XXX
	 * I'll do that later.
	 *
	 */

	return 0;
}





#endif /* SDL_HEADER */
#endif /* USE_SDL */
