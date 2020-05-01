/* File: sdl-sound.c */
/* Purpose: SDL_Audio routines for loading and playing WAVs */
#if defined(USE_SDL) || defined(USE_SDL2)

#include "c-angband.h"
#include <SDL.h>

#ifdef USE_SOUND

typedef struct sound_wave
{
	Uint8* buffer;
	Uint32 length;
	Uint32 pos;
	bool playing;
} sound_wave;
sound_wave sound_data[MSG_MAX][SAMPLE_MAX];
sound_wave *now_playing = NULL;

SDL_AudioSpec wav_obtained;

static void wav_play(void *userdata, Uint8 *stream, int len)
{
	/* Grab */
	sound_wave * wav = now_playing;

	/* Paranoia */
	Uint32 tocopy = ((wav->length - wav->pos > len) ? len: wav->length - wav->pos);

	/* Samples to copy for silence */
	Uint32 topad = len - tocopy;

	/* Copy data to audio buffer */
	memcpy(stream, wav->buffer + wav->pos, tocopy);

	/* Copy silence to audio buffer */
	/* TODO: for >=16bit unsigned formats, "0" is wrong, but I don't want
	 * to do interleaved memset here :( Ideally, we should have a static
	 * buffer pre-filled with silence and copy from there... */
	memset(stream + tocopy, 0, topad);

	/* Advance */
	wav->pos += tocopy;
}

void sdl_play_sound_end(bool wait)
{
	bool end_sound;
	sound_wave * wav = now_playing;

	if (wav == NULL) return;

	end_sound = (wait? (wav->pos == wav->length): TRUE);

	/* Wait for end of audio thread */
	if ((wav->length != 0) && end_sound && wav->playing)
	{
		/* Stop playing */
		SDL_PauseAudio(1);
		wav->playing = FALSE;
	}
}

/*
 * Init sound
 */
void sdl_init_sound()
{
	SDL_AudioSpec wav_desired;
	int i, j;

	/* Desired audio parameters */
	wav_desired.freq = 44100;
	wav_desired.format = 0; /* let OS pick //AUDIO_U16SYS;*/
	wav_desired.channels = 2;
	wav_desired.samples = 512;
	wav_desired.callback = &wav_play;
	wav_desired.userdata = NULL;

	/* Open the audio device */
	if (SDL_OpenAudio(&wav_desired, &wav_obtained) != 0)
	{
		plog_fmt("Could not open audio: %s", SDL_GetError());
		use_sound = FALSE;
		return;
	}

	/* Clear sound_data */
	for (j = 0; j < MSG_MAX; j++)	for (i = 0; i < SAMPLE_MAX; i++)
	{
		(void)WIPE(&sound_data[j][i], sound_wave);
	}
}

/*
 * Close sound
 */
void sdl_cleanup_sound()
{
	int j, i;

	/* Close the audio device */
	SDL_CloseAudio();

	/* Free loaded wavs */
	for (j = 0; j < MSG_MAX; j++)	for (i = 0; i < SAMPLE_MAX; i++)
	{
		if (sound_data[j][i].buffer != NULL)
		{
			free(sound_data[j][i].buffer);
		}
	}
}
/*
 * Load a sound
 */
void sdl_load_sound(int v, int s)
{
	SDL_AudioSpec wav_spec;
	SDL_AudioCVT wav_cvt;
	char buf[MSG_LEN];

	sound_wave * wav = &sound_data[v][s];

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_SOUND, sound_file[v][s]);

	/* Load the WAV */
	if (SDL_LoadWAV(buf, &wav_spec, &wav->buffer, &wav->length) == NULL)
	{
		plog_fmt("Could not open %s: %s", buf, SDL_GetError());
		return;
	}

	/* Build the audio converter */
	if (SDL_BuildAudioCVT(&wav_cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
		wav_obtained.format, wav_obtained.channels, wav_obtained.freq) < 0)
	{
		plog_fmt("Could not build audio converter: %s", SDL_GetError());
		return;
	}

	/* Allocate a buffer for the audio converter */
	wav_cvt.buf = malloc(wav->length * wav_cvt.len_mult);
	wav_cvt.len = wav->length;
	memcpy(wav_cvt.buf, wav->buffer, wav->length);

	/* Convert audio data to correct format */
	if (SDL_ConvertAudio(&wav_cvt) != 0)
	{
		plog_fmt("Could not convert audio file: %s", SDL_GetError());
		return;
	}

	/* Free the WAV */
	SDL_FreeWAV(wav->buffer);

	/* Allocate a buffer for the audio data */
	wav->buffer = malloc(wav_cvt.len_cvt);
	memcpy(wav->buffer, wav_cvt.buf, wav_cvt.len_cvt);
	free(wav_cvt.buf);
	wav->length = wav_cvt.len_cvt;
}
/*
 * Make a sound
 */
void sdl_play_sound(int v, int s)
{
	sound_wave * wav = &sound_data[v][s];

	/* If another sound is currently playing, stop it */
	sdl_play_sound_end(FALSE);

	/* If sound isn't loaded, load it */
	if (wav->buffer == NULL) sdl_load_sound(v,s);

	/* Start playing */
	wav->pos = 0;
	wav->playing = TRUE;
	SDL_PauseAudio(0);

	now_playing = wav;
}

/*
 * BONUS API - Play a system beep sound
 */
#ifdef ON_OSX
#include "CoreFoundation/CoreFoundation.h"
#endif
#ifdef WINDOWS
/* <windows.h> should already be included by "c-angband.h" */
#endif
void sdl_bell(void)
{
#ifdef ON_OSX
	NSBeep();
#elif defined(WINDOWS)
	MessageBeep(MB_ICONASTERISK);
#elif defined(SET_UID) && !defined(MOBILE_UI)
	/* Hacky fallback for "UNIX" */
	fprintf(stdout, "\a");
#endif
}

#endif
#endif
