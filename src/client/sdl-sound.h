/* File: sdl-sound.h */
/* Purpose: SDL_Audio routines for loading and playing WAVs */
#ifndef INCLUDED_SDL_SOUND_H
#define INCLUDED_SDL_SOUND_H

#if defined(USE_SDL) || defined(USE_SDL2)

extern void sdl_init_sound(void);
extern void sdl_cleanup_sound(void);
extern void sdl_load_sound(int v, int s);
extern void sdl_play_sound(int v, int s);
extern void sdl_play_sound_end(bool wait);

#endif /* USE_SDL || USE_SDL2 */

extern void sdl_bell(void);

#endif /* INCLUDED_SDL_SOUND_H */
