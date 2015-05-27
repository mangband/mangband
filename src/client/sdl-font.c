#include <SDL.h>
#include "c-angband.h"

#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
#include <SDL_image.h>
#endif
#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
#include <SDL_ttf.h>
#endif

/*
 * This file contains various font loading routines, extracted from 
 * both SDL1 and SDL2 clients, to be then used by both.
 */

static cptr ANGBAND_DIR_XTRA_FONT;
static cptr ANGBAND_DIR_XTRA_GRAF;

/* strtoii
Function that extracts the numerics from a string such as "sprites8x16.bmp"
Taken from "maim_sdl.c", Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
*/
static errr strtoii(const char *str, Uint32 *w, Uint32 *h) {
	char buf[1024];
	char *s = buf;
	char *tok;
	char *numeric = "0123456789";

	size_t l; /* length of numeric string */

	if (!str || !w || !h) return -1;

	if (strlen(str) < 3) return -1; /* must have room for at least "1x1" */

	strncpy(buf, str, 1023);
	buf[1023] = '\0';

	tok = strpbrk(buf, numeric);
	if (!tok) return -1;

	l = strspn(tok, numeric);
	if (!l) return -1;

	tok[l] = '\0';

	s = tok + l + 1;

	if(!sscanf(tok, "%d", w)) return -1;

	/* next token */
	tok = strpbrk(s, numeric);
	if (!tok) return -1;

	l = strspn(tok, numeric);
	if (!l) return -1;

	tok[l] = '\0';
	/* no need to set s since this is the last token */

	if(!sscanf(tok, "%d", h)) return -1;

	return 0;
}

/*
 * bmpToFont
 * This assumes the file is already in the correct
 * 16x16 glyph table format, and is a pallettized one.
 */
SDL_Surface *bmpToFont(SDL_Rect *fd, cptr filename) {
	SDL_Color pal[2];
	SDL_Surface *font, *surface;
	SDL_Rect full_rect;
	int width, height;
	int i;
	char buf[1036];

	// Get and open our BMP font from the xtra dir	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);
	font = SDL_LoadBMP(buf);
	if (!font) {
		plog_fmt("bmpToFont: %s", SDL_GetError());
		return NULL;
	}
	// Poorly get our font metrics and maximum cell size in pixels
	width = 0;
	height = 0;
	if (strtoii(filename, &width, &height) != 0) {

	}
	fd->w = width;
	fd->h = height;

	pal[0].r = pal[0].g = pal[0].b = 0;
	pal[1].r = pal[1].g = pal[1].b = 255;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(font, pal, 0, 2);
#else
	pal[0].a = 0;
	pal[1].a = 255;
	SDL_SetPaletteColors(font->format->palette, pal, 0, 2);
#endif

	SDL_SaveBMP(font, "oldest.bmp");

	// Create our glyph surface that will store 256 characters in a 16x16 matrix
	surface = SDL_CreateRGBSurface(0, width*16, height*16, font->format->BitsPerPixel, 0, 0, 0, 0);

	full_rect.x = 0;
	full_rect.y = 0;
	full_rect.w = font->w;
	full_rect.h = font->h;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(surface, font->format->palette->colors, 0, font->format->palette->ncolors);
#else
	SDL_SetPaletteColors(surface->format->palette, font->format->palette->colors, 0, font->format->palette->ncolors);
#endif
	SDL_BlitSurface(font, &full_rect, surface, &full_rect);

	//plog_fmt("Loaded font: %s (%d x %d)", filename, width, height);

	SDL_FreeSurface(font);

	return surface;
}

#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
/* ttfToFont
This function takes the given FontData structure and attempts to make a Glyph Table texture from the filename at the point size fontsize with sharp or smooth rendering via the smoothing boolean.
taken from main-sdl2.c
*/
SDL_Surface* ttfToFont(SDL_Rect *fd, cptr filename, int fontsize, int smoothing) {
  SDL_Surface *surface;
  TTF_Font *font;
  int minx, maxx, miny, maxy, width, height;
  int i;
  char buf[1036];
  //if (fd->w || fd->h) return 1; // Return if FontData is not clean
  
  fd->w = fd->h = 0;
  // Get and open our TTF font from the xtra dir
  path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);
  font = TTF_OpenFont(buf, fontsize);
  if (!font) {
    plog_fmt("ttfToFont: %s", TTF_GetError());
    return NULL;
  }
  // Poorly get our font metrics and maximum cell size in pixels
  width = 0;
  height = 0;
  for (i = 0; i < 255; i++) {
    TTF_GlyphMetrics(font, (char)i, &minx, &maxx, &miny, &maxy, NULL);
    if (minx+maxx > width) width = minx+maxx;
    if (miny+maxy > height) height = miny+maxy;
  }
  fd->w = width;
  fd->h = height;
  // Create our glyph surface that will store 256 characters in a 16x16 matrix
  surface = SDL_CreateRGBSurface(0, width*16, height*16, 32, 0, 0, 0, 0);
  if (surface == NULL) {
    plog_fmt("ttfToFont: %s", SDL_GetError());
    TTF_CloseFont(font);
    return NULL;
  }
  // Painstakingly create each glyph as a surface and blit to our glyphs surface
  {
    int ch = 0;
    for (ch = 0; ch < 256; ch++) {
      SDL_Color color = { 255, 255, 255 };
      SDL_Surface *char_surface = (smoothing ? TTF_RenderGlyph_Blended(font, ch, color) : TTF_RenderGlyph_Solid(font, ch, color));
      int row = ch / 16;
      int col = ch - (row*16);
      SDL_Rect glyph_rect = { col*width, row*height, width, height };
      SDL_BlitSurface(char_surface, NULL, surface, &glyph_rect);
      SDL_FreeSurface(char_surface);
    }
  }
  // We're done with the font
  TTF_CloseFont(font);
  return surface;
}
#endif

/*
Taken from "main-sdl.c", Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
*/
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
 
SDL_Surface* load_HEX_font_sdl(SDL_Rect *fd, cptr filename, bool justmetrics)
{
	SDL_Surface *face;
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

	Uint32 iw, ih; /* internal width and height. sometimes larger then final character size */

	/* check font_data */
	fd->w = fd->h = 0;
	//if (fd->w || fd->h) return NULL; /* dealloc it first, dummy. */

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);

	f = fopen(buf, "r");

	if (!f) 
	{
		plog(format("Couldn't open: %s", buf));
		return NULL;
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
			fd->w = 8; /* a nasty guess. */
			fd->h = i;
			/*if (i & 1) break;*/ /* odd number of bytes. this is the height. */
		} else 
		{
			if (i > fd->h) {
				fd->w = 16; /* an even nastier guess (full-width glyphs here) */
				if(fd -> h / 2 == i / 3)
				{
					/* this sucks. */
					fd->h = i / 3;
					fd->w = 24;
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
				fd->w = 16; /* the same nastier guess. */
				fd->h = i; /* Ah, so this is the height */
			}
			/* they're equal. we can say nothing about the glyph height */
		}
	}

	/* Use those dimensions for reading anyway */
	iw = fd->w;
	ih = fd->h;

	/* analyze the file name */
	if(!strtoii(filename, &mw, &mh))
	{
		/* success! */
		fd->w = mw;
		fd->h = mh;
	} else {
		plog("You may wish to incude the dimensions of a font in its file name. ie \"vga8x16.hex\"");
	}

	if (justmetrics) 
	{
		fclose(f);
		return NULL;
	}

	/* Allocate the bitmap here. */
	face = SDL_CreateRGBSurface(SDL_SWSURFACE, iw, 256 * ih, 8,0,0,0,0); 
	if (!face) 
	{
		fclose(f);
		return NULL;
	}
	//SDL_SetAlpha(face, SDL_RLEACCEL, SDL_ALPHA_OPAQUE); /* use RLE */

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
					((Uint8 *)face->pixels)[x + y*face->pitch] = 0x01;
				} else
				{
#ifdef FONT_LOAD_DEBUGGING
					printf("-");
#endif
					((Uint8 *)face->pixels)[x + y*face->pitch] = 0x00;
				}
			}
			++bytesdone;

			/* processing half-width glyph or just finished even byte */
			if (i == ih || ((i == 2*ih) && !(bytesdone & 1))) 
			{
				x = 0;
				++y;
#ifdef FONT_LOAD_DEBUGGING
				printf("\n");
#endif
			} else if (i == 2*ih)
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

	if (fail == -1) 
	{
		SDL_FreeSurface(face);
		face = NULL;
	}

	return face;
}
SDL_Surface* load_HEX_font_sdl_(SDL_Rect *fd, cptr filename) {
	SDL_Color pal[2];
	SDL_Surface *font, *surface;
	SDL_Rect sr, dr;
	int i;
	char buf[1036];

	// Get and open our BMP font from the xtra dir
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);
	// Call the original load_HEX_font_sdl function,
	// which returns a 1x256 glyph table. Ideally, it should return
	// 16x16 table by itself, but it doesn't, and I don't want to mess
	// with it. So we re-arrange into proper table on a new surface
	// after loading.  
	font = load_HEX_font_sdl(fd, filename, 0);
	
	if (font == NULL)
	{
		return NULL;
	}

	pal[0].r = pal[0].g = pal[0].b = 0;
	pal[1].r = pal[1].g = pal[1].b = 255;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(font, pal, 0, 2);
#else
	pal[0].a = 0;
	pal[1].a = 255;
	SDL_SetPaletteColors(font->format->palette, pal, 0, 2);
#endif

	// Create our glyph surface that will store 256 characters in a 16x16 matrix
	surface = SDL_CreateRGBSurface(0, fd->w*16, fd->h*16, 8, 0, 0, 0, 0);

	sr.w = dr.w = fd->w;
	sr.h = dr.h = fd->h;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(surface, font->format->palette->colors, 0, 2);
#else
	SDL_SetPaletteColors(surface->format->palette, font->format->palette->colors, 0, 2);
#endif

	// Copy original 1x256 table into 16x16 table

	for (i = 0; i < 256; i++) 
	{
		sr.x = 0;
		sr.y = i * sr.h;
		
		dr.x = (i % 16) * dr.w;
		dr.y = (i / 16) * dr.h;

		SDL_BlitSurface(font, &sr, surface, &dr);
	}

	SDL_FreeSurface(font);

	return surface;
}


errr sdl_font_init() 
{
	char path[1024];
	
	/* Font */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");
	ANGBAND_DIR_XTRA_FONT = string_make(path);

	/* Graf */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "graf");
	ANGBAND_DIR_XTRA_GRAF = string_make(path);
	
	
#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
	if (TTF_Init() == -1) {
		plog_fmt("TTF_Init(): %s", TTF_GetError());
		return -1;
	}
#endif

	return 0;
}
errr sdl_font_quit() 
{
	free(ANGBAND_DIR_XTRA_FONT);
	free(ANGBAND_DIR_XTRA_GRAF);

#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
    TTF_Quit();
#endif

	return 0;
}

SDL_Surface* sdl_font_load(cptr filename, SDL_Rect* info, int fontsize, int smoothing) 
{
	char *ext;
	enum {
		FONT_BMP,
		FONT_HEX,
		FONT_TTF,
		FONT_FON,
	} fonttype;
	char* typenames[] = {
		".bmp",
		".hex",
		".ttf",
		".fon"
	};

	/* Extract file extension */
	ext = strrchr(filename, '.');

	/* Failed to get extension? Assume .. bmp font */
	if (ext == NULL) 
	{
		fonttype = FONT_BMP;
	}
	else if (!strcasecmp(ext, typenames[FONT_HEX]))
	{
		fonttype = FONT_HEX;
	}
	else if (!strcasecmp(ext, typenames[FONT_TTF]))
	{
		fonttype = FONT_TTF;
	}
	else if (!strcasecmp(ext, typenames[FONT_BMP]))
	{
		fonttype = FONT_BMP;
	}

	SDL_Rect glyph_info;
	SDL_Surface *surface;

	switch (fonttype) 
	{
		case FONT_HEX:
			surface = load_HEX_font_sdl_(info, filename);
		break;
		case FONT_TTF:
		#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF) 
			surface = ttfToFont(info, filename, fontsize, smoothing);
		#else
			plog_fmt("compiled without ttf support, can't load %s\n", filename);
		#endif
		break;
		case FONT_FON:
			plog("can't load FON files :(\n");
		break;
		default:
		case FONT_BMP:
			surface = bmpToFont(info, filename);
		break;
	}
	
	if (surface == NULL)
	{
		plog_fmt("%s: %s: failed to load %s", ANGBAND_SYS, typenames[fonttype], filename);
	}

	return surface;
}
