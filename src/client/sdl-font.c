#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#include "c-angband.h"

#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
#include <SDL_image.h>
#else
#include "lupng/lupng.h"
#endif
#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
#include <SDL_ttf.h>
#endif

/*
 * This file contains various font loading routines, extracted from 
 * both SDL1 and SDL2 clients, to be then used by both.
 */

static cptr ANGBAND_DIR_XTRA_FONT;

#define USE_BITMASK /* Load "mask files" and use them as colorkeys when doing graphics. Slower, but neatier */

/* Global config option: */
bool sdl_graf_prefer_rgba = FALSE;

/* Useful constants */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif
#define RGBAMASK RMASK, GMASK, BMASK, AMASK

/* The following function will extract height and width info from a filename
 * such as 16x16.xyz or 8X13.bar or even argle8ook16.foo
 * I realize now that it's also useful for reading integers out of an argument
 * such as --fooscale1=2
 - Taken from "maim_sdl.c", Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
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


/* Generate a surface with a filled circle on a transparent background */
SDL_Surface* SDL_CreateCircleSurface32(int w, int h, int r, SDL_Color *c)
{
	int x, y, cx, cy;
	SDL_Surface *face;
	face = SDL_CreateRGBSurface(0, w, h, 32, RGBAMASK);
	if (!face) return NULL;
	
	/* Center pixel */	
	cx = w / 2;
	cy = h / 2;

	/* Iterate over each pixel... */
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			Uint32 col; Uint32 *px;
			if (IHYPOT(cx - x, cy - y) > r)
			{
				col = SDL_MapRGBA(face->format, 0, 0, 0, 0);
			}
			else
			{
				col = SDL_MapRGBA(face->format, c->r, c->g, c->b, 255);
			}
			/* Get pointer and set color */
			px = (Uint32*)((Uint8*)face->pixels
				+ (y * face->pitch + x * face->format->BytesPerPixel));
			*px = col;
		}
	}
	return face;
}

/* Create a software, 8-bpp SDL Surface */
SDL_Surface* SDL_Create8BITSurface(int w, int h)
{
	SDL_Color pal[2];
	SDL_Surface *face;
	face = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	if (!face) return NULL;

	/* Set monochrome palette */
	pal[0].r = pal[0].g = pal[0].b = 0;
	pal[1].r = pal[1].g = pal[1].b = 255;
#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(face, pal, 0, 2);
#else
	pal[0].a = 0;
	pal[1].a = 255;
	SDL_SetPaletteColors(face->format->palette, pal, 0, 2);
#endif
	return face;
}

/*
 * bmpToFont
 * This assumes the file is already in the correct
 * 16x16 glyph table format, and is a pallettized one.
 */
SDL_Surface *bmpToFont(SDL_Rect *fd, cptr filename) {
	SDL_Surface *font;
	Uint32 width, height;
	char buf[1036];
#if 0
	SDL_Surface *surface;
	SDL_Rect full_rect;
#endif

	// Get and open our BMP font from the xtra dir
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);
	font = SDL_LoadBMP(buf);
	if (!font) {
		return NULL;
	}
	if (font->format->BitsPerPixel != 8) {
		SDL_SetError("bmpToFont: was expecting bitmap with a palette");
		return NULL;
	}
	// Poorly get our font metrics and maximum cell size in pixels
	width = 0;
	height = 0;
	if (strtoii(filename, &width, &height) != 0) {
		width = font->w / 16;
		height = font->h / 16;
	}
	fd->w = width;
	fd->h = height;

	return font;
#if 0
	//SDL_SaveBMP(font, "original_font.bmp");//debug

	// Create our glyph surface that will store 256 characters in a 16x16 matrix
	surface = SDL_Create8BITSurface(width*16, height*16);

	full_rect.x = 0;
	full_rect.y = 0;
	full_rect.w = font->w;
	full_rect.h = font->h;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(surface, font->format->palette->colors, 0, font->format->palette->ncolors);
#else
	SDL_SetPaletteColors(surface->format->palette, font->format->palette->colors, 0, font->format->palette->ncolors);
#endif
	surface->format->palette->ncolors = font->format->palette->ncolors;
	SDL_BlitSurface(font, &full_rect, surface, &full_rect);

	//plog_fmt("Loaded font: %s (%d x %d)", filename, width, height);

	SDL_FreeSurface(font);

	return surface;
#endif
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
  /* Hack -- square'ize */
  if (smoothing == 2)
  {
    if (width > height) height = width;
    else if (height > width) width = height;
  }
  // For .fon files, try the NxN-in-filename approach
  if (isuffix(filename, ".fon")) {
    Uint32 _width = 0;
    Uint32 _height = 0;
    if (!strtoii(filename, &_width, &_height)) {
      width = _width;
      height = _height;
    }
  }

  fd->w = width;
  fd->h = height;

  // Create our glyph surface that will store 256 characters in a 16x16 matrix
  surface = SDL_CreateRGBSurface(0, width*16, height*16, 32, RGBAMASK);
  if (surface == NULL) {
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
	ang_file *f;

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

	f = file_open(buf, MODE_READ, FTYPE_TEXT);

	if (!f) 
	{
		plog(format("Couldn't open: %s", buf));
		return NULL;
	}

	/* try hard to figure out the font metrics */

	while (file_getl(f, gs, MAX_HEX_FONT_LINE))
	{
		i = strlen(gs);

		if (gs[i-1] == '\n') i--;
		if (gs[i-1] == '\r') i--;

		i -= 5; /* each line begins with 1234: */

		/* now i is the number of nibbles in the line */

		if (i & 1)
		{
			plog("Error in HEX line measurment. Report to hmaon@bumba.net.");
			file_close(f);
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
		file_close(f);
		return NULL;
	}

	/* Allocate the bitmap here. */
	face = SDL_Create8BITSurface(iw, 256 * ih);

	if (!face) 
	{
		file_close(f);
		return NULL;
	}
	//SDL_SetAlpha(face, SDL_RLEACCEL, SDL_ALPHA_OPAQUE); /* use RLE */

	file_seek(f, 0);

	while (file_getl(f, gs, MAX_HEX_FONT_LINE))
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

	file_close(f);

	if (fail == -1) 
	{
		SDL_FreeSurface(face);
		face = NULL;
	}

	/* Hack -- limit palette */
	face->format->palette->ncolors = 2;

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

	// Create our glyph surface that will store 256 characters in a 16x16 matrix
	surface = SDL_Create8BITSurface(fd->w*16, fd->h*16);

	sr.w = dr.w = fd->w;
	sr.h = dr.h = fd->h;

#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(surface, font->format->palette->colors, 0, 2);
	surface->format->palette->ncolors = 2;
#else
	SDL_SetPaletteColors(surface->format->palette, font->format->palette->colors, 0, 2);
	surface->format->palette->ncolors = 2;
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

/*
 * BDF font loader.
 *
 * NOTE: This function will crash hard on any kind of error.
 * The air around it is charged with crash-inducing electricity.
 */
SDL_Surface* load_BDF_font(SDL_Rect *fd, cptr filename)
{
	ang_file *f;
	SDL_Surface *face;
	char buf[1024];
	char line[1024];

	int n, bitmap_mode = 0, bitmap_line = 0;
	bool fony_error = FALSE;

	int iw = 16;
	int ih = 24;

	int glyph_w, glyph_h;
	int glyph_x, glyph_y;
	int font_ox, font_oy;
	int num_chars = 256;
	int cols = 16;
	int rows = 16;
	int cx, cy;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, filename);

	f = file_open(buf, MODE_READ, FTYPE_TEXT);

	if (!f)
	{
		plog(format("Couldn't open: %s", buf));
		return NULL;
	}

	while (file_getl(f, line, 1024))
	{
		/* Chomp */
		n = strlen(line);
		if (n && line[n-1] == '\n') n--;
		if (n && line[n-1] == '\r') n--;
		line[n] = '\0';

		/* "Parse" */
		if (!my_strnicmp(line, "ENDCHAR", 7)) {
			bitmap_mode = 0;
			continue;
		}
		else if (bitmap_mode) {
			/* We get a string with hex, e.g. FF0C */
			u32b b;
			char hbuf[3] = { 0, 0, 0 };
			int i, j, l = strlen(line) / 2; /* num bytes */
			if (!face) {
				file_close(f);
				return NULL;
			}
			for (j = 0; j < l; j++) { /* for each byte */
				/* Read hex string, 2 chars at a time */
				hbuf[0] = line[(j*2) + 0];
				hbuf[1] = line[(j*2) + 1];
				b = (u32b)strtol(hbuf, NULL, 16);
				for (i = 0; i < 8; i++) { /* for each bit */
					int mask =  (1 << i);
					int test = (b & mask);

					int x = cx * iw + j*8 + (8-i) + glyph_x + font_ox;
					int y = cy * ih + bitmap_line + (ih-glyph_h) - glyph_y + font_oy;

					/* ignore padding bits */
					if ( j*8 + (8-i) > glyph_w ) continue;

					((Uint8 *)face->pixels)[
					  x + y * face->pitch] = test ? 1 : 0;

					//printf("%c", mask ? 'X' : '.');
				}
			}
			bitmap_line++;
			//printf("\n");
		}
		else if (!my_strnicmp(line, "STARTCHAR", 9)) {
			int i = atoi(&line[10]);
			cx = i % cols;
			cy = i / cols;
			glyph_w = 0;
			glyph_h = 0;
			glyph_x = 0;
			glyph_y = 0;
		}
		else if (!my_strnicmp(line, "BBX", 3)) {
			char *sp;
			char *t = strtok(line, " ");
			char *_w = strtok(NULL, " ");
			char *_h = strtok(NULL, " ");
			char *_x = strtok(NULL, " ");
			char *_y = strtok(NULL, " ");
			glyph_w = atoi(_w);
			glyph_h = atoi(_h);
			glyph_x = atoi(_x);
			glyph_y = atoi(_y);
		}
		else if (!my_strnicmp(line, "BITMAP", 6)) {
			bitmap_mode = 1;
			bitmap_line = 0;
		}
		else if (!my_strnicmp(line, "SIZE", 4)) {
			ih = atoi(&line[5]);
		}
		else if (!my_strnicmp(line, "FONTBOUNDINGBOX", 15)) {
			char *sp;
			char *t = strtok(line, " ");
			char *_w = strtok(NULL, " ");
			char *_h = strtok(NULL, " ");
			char *_x = strtok(NULL, " ");
			char *_y = strtok(NULL, " ");
			int font_w = atoi(_w);
			int font_h = atoi(_h);
			font_ox = atoi(_x);
			font_oy = atoi(_y);
			iw = font_w;
			font_oy = ih - font_h + font_oy;
			/*font_ox = iw - font_w + font_ox;*/
			if (fony_error) {
				font_ox -= 1;
				font_oy -= 1;
			}
		}
		else if (!my_strnicmp(line, "COMMENT Exported by Fony v1.4.", 30)) {
			fony_error = TRUE;
		}
		else if (!my_strnicmp(line, "CHARS", 5)) {
			num_chars = atoi(&line[6]);
			rows = num_chars / cols;
		}
		else if (!my_strnicmp(line, "ENDPROPERTIES", 13)) {
			/* By this time, we should know our bounding box */
			if (iw <= 0 || ih <= 0)
			{
				file_close(f);
				return NULL;
			}
			face = SDL_Create8BITSurface(cols * iw, rows * ih);
			if (!face)
			{
				file_close(f);
				return NULL;
			}
		}
	}

	file_close(f);

	face->format->palette->ncolors = 2;

	fd->w = iw;
	fd->h = ih;

	return face;
}

/* Read a PNG file using LuPng, convert it to SDL_Surface. */
#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
#else
size_t RWopsLuRead(void *outPtr, size_t size, size_t count, void *userPtr)
{
	return SDL_RWread((SDL_RWops*)userPtr, outPtr, size, count);
}
size_t RWopsLuWrite(const void *inPtr, size_t size, size_t count, void *userPtr)
{
	return SDL_RWwrite((SDL_RWops*)userPtr, inPtr, size, count);
}

SDL_Surface* SDLU_LoadPNG_RW(SDL_RWops *rw, int freesrc)
{
	SDL_Surface *face;
	int x, y, i;
	int npal = 0;
	SDL_Color pal[256];
	LuUserContext userCtx;
	LuImage *img;

	luUserContextInitDefault(&userCtx);
	userCtx.readProc = RWopsLuRead;
	userCtx.readProcUserPtr = (void*)rw;

	img = luPngReadUC(&userCtx);

	if (freesrc) SDL_RWclose(rw);

	if (!img)
	{
		SDL_SetError("Can't read png file");
		return NULL;
	}

	/* Make 32-bit image */
	if (sdl_graf_prefer_rgba)
	{
		face = SDL_CreateRGBSurface(0, img->width, img->height, 32, RGBAMASK);
		if (!face)
		{
			luImageRelease(img, NULL);
			return NULL;
		}
		for (y = 0; y < img->height; y++) {
		for (x = 0; x < img->width ; x++) {
			Uint8 r = img->data[y * img->width * img->channels + x * img->channels + 0];
			Uint8 g = img->data[y * img->width * img->channels + x * img->channels + 1];
			Uint8 b = img->data[y * img->width * img->channels + x * img->channels + 2];
			Uint8 a = 255;
			Uint32 col; Uint32 *px;
			if (img->channels == 4) {
				a = img->data[y * img->width * img->channels + x * img->channels + 3];
			}
			col = SDL_MapRGBA(face->format, r, g, b, a);
			px = (Uint32*)((Uint8*)face->pixels + (y * face->pitch + x * face->format->BytesPerPixel));
			*px = col;
		} }
		luImageRelease(img, NULL);
		return face;
	}

	luImageDarkenAlpha(img);

	/* Make 8-bit image */
	face = SDL_CreateRGBSurface(SDL_SWSURFACE, img->width, img->height, 8, 0,0,0,0);
	if (!face)
	{
		luImageRelease(img, NULL);
		return NULL;
	}
#if SDL_MAJOR_VERSION < 2
	SDL_SetAlpha(face, SDL_RLEACCEL, SDL_ALPHA_OPAQUE); /* use RLE */
#endif

	pal[0].r = pal[0].g = pal[0].b = 0; /* chroma black */
	pal[1].r = pal[1].g = pal[1].b = 1; /* subtitution black */
#if SDL_MAJOR_VERSION >= 2
	pal[0].a = 0; pal[1].a = 255;
#endif
	npal = 2;

	for (y = 0; y < img->height; y++) {
	for (x = 0; x < img->width ; x++) {
		
		Uint8 r = img->data[y * img->width * img->channels + x * img->channels + 0];
		Uint8 g = img->data[y * img->width * img->channels + x * img->channels + 1];
		Uint8 b = img->data[y * img->width * img->channels + x * img->channels + 2];
		Uint8 col = 255;
		for (i = 0; i < npal; i++) {
			if (pal[i].r == r && pal[i].g == g && pal[i].b == b) {
				col = i;
				break;
			}
		}
		if (col == 255 && npal < 255) {
			i = npal;
			npal++;
			pal[i].r = r;
			pal[i].g = g;
			pal[i].b = b;
#if SDL_MAJOR_VERSION >= 2
			pal[i].a = 255;
#endif
			col = i;
		}
		if (col == 0 && img->channels == 4) {
			Uint8 a = img->data[y * img->width * img->channels + x * img->channels + 3];
			if (a <= 32) col = 0;
			else col = 1;
		}
		((Uint8*)face->pixels)[y * face->pitch + x * face->format->BytesPerPixel] = col;
	} }
#if SDL_MAJOR_VERSION < 2
	SDL_SetColors(face, &pal[0], 0, npal);
#else
	SDL_SetPaletteColors(face->format->palette, pal, 0, npal);
#endif

	luImageRelease(img, NULL);

	return face;
}
SDL_Surface* SDLU_LoadPNG(const char *path)
{
	return SDLU_LoadPNG_RW(SDL_RWFromFile(path, "rb"), 1);
}
#endif

errr sdl_font_init(void)
{
	char path[1024];
	
	/* Font */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");
	ANGBAND_DIR_XTRA_FONT = string_make(path);

#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
	if (TTF_Init() == -1) {
		plog_fmt("TTF_Init(): %s", TTF_GetError());
		return -1;
	}
#endif

#if defined(USD_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
	if (IMG_Init(IMG_INIT_PNG) == -1) {
		plog_fmt("IMG_Init(): %s", IMG_GetError());
		return -2;
	}
#endif

	return 0;
}
errr sdl_font_quit()
{
	string_free(ANGBAND_DIR_XTRA_FONT);

#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
	TTF_Quit();
#endif

#if defined(USD_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
	IMG_Quit();
#endif
	return 0;
}

enum fonttype {
	FONT_UNKNOWN,
	FONT_BMP,
	FONT_HEX,
	FONT_BDF,
	FONT_TTF,
	FONT_OTF,
	FONT_FON,
	FONT_FNT,
	FONT_PCF,
	FONT__MAX,
};
static const char* typenames[] = {
	"UNKNOWN",
	".bmp",
	".hex",
	".bdf",
	".ttf",
	".otf",
	".fon",
	".fnt",
	".pcf",
};

enum fonttype match_fonttype(cptr filename)
{
	int fonttype;
	char *ext;

	/* Extract file extension */
	ext = strrchr(filename, '.');

	/* Failed to get extension? */
	if (ext == NULL)
	{
		fonttype = FONT_UNKNOWN;
	}
	else
	{
		/* Compare each string from "typenames" */
		int i;
		fonttype = FONT_UNKNOWN;
		for (i = 0; i < FONT__MAX; i++)
		{
			if (!my_stricmp(ext, typenames[i]))
			{
				fonttype = i;
				break;
			}
		}
	}
	return fonttype;
}

int sdl_font_read_dir(cptr path, char files[][1024], size_t size)
{
	char buf[1024];
	int n = 0;
	ang_dir *d;
	if (path == NULL)
	{
		path = ANGBAND_DIR_XTRA_FONT;
	}
	d = my_dopen(path);
	if (d == NULL)
	{
		return 0;
	}
	while (my_dread(d, buf, sizeof(buf)))
	{
		int fonttype = match_fonttype(buf);
		/* Hack -- Skip ui-cmd font */
		if (!my_stricmp(buf, "ui-cmd.ttf")) continue;
		#if !(defined(USE_SDL2_TTF) || defined(USE_SDL_TTF))
			/* Skip unsupported fonts */
			if ((fonttype == FONT_BMP)
				|| (fonttype == FONT_HEX)
				|| (fonttype == FONT_BDF))
					continue;
		#endif
		if (fonttype != FONT_UNKNOWN)
		{
			my_strcpy(files[n], buf, 1024);
			n++;
		}
	}
	my_dclose(d);
	return n;
}

SDL_Surface* sdl_font_load(cptr filename, SDL_Rect* info, int fontsize, int smoothing)
{
	SDL_Rect glyph_info;
	SDL_Surface *surface;

	int fonttype;

	fonttype = match_fonttype(filename);

	if (fonttype == FONT_UNKNOWN)
	{
		/* Pick a default */
#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
		fonttype = FONT_TTF;
#else
		fonttype = FONT_BMP;
#endif
	}

	switch (fonttype)
	{
		case FONT_HEX:
			surface = load_HEX_font_sdl_(info, filename);
		break;
		case FONT_BDF:
		/*#if !(defined(USE_SDL2_TTF) || defined(USE_SDL_TTF))*/
		/* XXX Do not use SDL_ttf, opt for our loader. */
			surface = load_BDF_font(info, filename);
		break;
		/*#endif*/
		case FONT_PCF:
		case FONT_FON:
		case FONT_FNT:
		/*
		* if we ever implement our own .FON loading, it will go here.
		* for now, drop to SDL_ttf, which can actually load them
		*
		* //surface = fonToFont(info, filename, :raw_fnt => fonttype == FONT_FNT ? 1 : 0)
		* //break;
		*/
		case FONT_TTF:
		case FONT_OTF:
#if defined(USE_SDL2_TTF) || defined(USE_SDL_TTF)
			surface = ttfToFont(info, filename, fontsize, smoothing);
#else
			surface = NULL;
			SDL_SetError("compiled without ttf support");
#endif
		break;
		default:
		case FONT_BMP:
			surface = bmpToFont(info, filename);
		break;
	}
	
	if (surface == NULL)
	{
		plog_fmt("%s: %s: failed to load %s - %s", ANGBAND_SYS, typenames[fonttype], filename, SDL_GetError());
	}

	return surface;
}

/* Graphics loading API. */


/*
This function was known as "load_BMP_graf_sdl"
Taken from "main-sdl.c", Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
Updated to support SDL_Image.
Updated to support LuPNG.
*/
SDL_Surface* sdl_graf_load(cptr filename, SDL_Rect *info, cptr maskname)
{
#ifdef USE_BITMASK
	int x, y, mask_offset, tile_offset;
	Uint8 *mask_pixels, *tile_pixels;
	Uint8 sub_black; /* substitution color for black */
	SDL_Surface *mask;
#endif
	SDL_Surface *face;
	char path[1024];
	Uint32 mw, mh;

	info->w = info->h = 0;

	path_build(path, 1024, ANGBAND_DIR_XTRA_GRAF, filename);
#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
	face = IMG_Load(path);
#else
	if (isuffix(filename, ".png"))
	{
		face = SDLU_LoadPNG(path);
		maskname = NULL;
	}
	else
	{
		face = SDL_LoadBMP(path);
	}
#endif
	if (face != NULL)
	{
		/* Attempt to get dimensions from filename */
		if (!strtoii(filename, &mw, &mh))
		{
			info->w = mw;
			info->h = mh;
		}

		/* Convert mask to color-key */
#ifdef USE_BITMASK
		if (!maskname) return face; /* No mask, we're done */

		path_build(path, 1024, ANGBAND_DIR_XTRA_GRAF, maskname);

#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
		if ((mask = IMG_Load(path)) != NULL)
#else
		if ((mask = SDL_LoadBMP(path)) != NULL)
#endif
		{
			sub_black = SDL_MapRGB(face->format, 1, 1, 1);

			mask_pixels = (Uint8 *)mask->pixels;
			tile_pixels = (Uint8 *)face->pixels;

			for (y = 0; y < mask->h; y++) {
			for (x = 0; x < mask->w; x++) {

				mask_offset = (mask->pitch/2 * y + x);
				tile_offset = (face->pitch/2 * y + x);

				if (!tile_pixels[tile_offset])
					tile_pixels[tile_offset] = ( mask_pixels[mask_offset] ? 0 : sub_black );

			}
			}

			SDL_FreeSurface(mask);
			mask = NULL;
		}
#endif
	}
	else
	{
#if defined(USE_SDL2_IMAGE) || defined(USE_SDL_IMAGE)
		plog_fmt("%s %s: %s", ANGBAND_SYS, filename, IMG_GetError());
#else
		plog_fmt("%s %s: %s", ANGBAND_SYS, filename, SDL_GetError());
#endif
		return NULL;
	}

	return face;
}
#endif
