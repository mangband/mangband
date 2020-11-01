/* File: sdl-font.h */
/* Purpose: SDL-font header file for mangband */
#ifndef SDL_FONT_H
#define SDL_FONT_H

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>

extern errr sdl_font_init(void);
extern errr sdl_font_quit(void);

extern bool sdl_graf_prefer_rgba;

extern SDL_Surface* sdl_font_load(cptr filename, SDL_Rect* info, int fontsize, int smoothing);
extern SDL_Surface* sdl_graf_load(cptr filename, SDL_Rect* info, cptr maskname);

/* ================== ================= */
extern int sdl_font_read_dir(cptr path, char files[][1024], size_t size);
extern SDL_Surface* SDL_CreateCircleSurface32(int w, int h, int r, SDL_Color *c);

#endif

#endif
