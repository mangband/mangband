/* File: main-xxx.c */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


#include "angband.h"
#include "main-sdl.h"

#ifdef USE_XXX

typedef struct graf_data graf_data;

struct graf_data
{
	int happy; /* are we having fun yet? */

	/* Your special state information goes here.
	 * ie:
	SDL_Surface *tiles;

	   or perhaps, if your tiles are simple bitmaps, you can take advantage of
	
	font_data *tiles;
	 */
};


static graf_data graphics; /* change this line. */


static void Term_init_xxx(term *t)
{
	term_data *td = (term_data*)(t->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* your code here. $5 */

	td->init_hook(t);

}



static void Term_nuke_xxx(term *t)
{
	term_data *td = (term_data*)(t->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* Nuke your stuff first. */

	td->nuke_hook(t);

}

static errr Term_user_xxx(int n)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* If you think of a use for this, good for you. */


	return td->user_hook(n);
}


static errr Term_xtra_xxx(int n, int v)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* Analyze */
	switch (n)
	{
		/* You probably don't need to process events. (Unless you want the mouse?)
		case TERM_XTRA_EVENT:
		{
			return (0);
		}

		case TERM_XTRA_FLUSH:
		{

			return (0);
		}
		*/

		/* Unless you need to do something special, and you may, let main-sdl.c do it. */
		case TERM_XTRA_CLEAR:
		{
			break;
		}
		

		/* I suggest you implement this one: */
		case TERM_XTRA_SHAPE:
		{
			/*
			 * Set the cursor visibility XXX XXX XXX
			 *
			 * This action should change the visibility of the cursor,
			 * if possible, to the requested value (0=off, 1=on)
			 *
			 * This action is optional, but can improve both the
			 * efficiency (and attractiveness) of the program.
			 */

			/* Handle this yourself if you implement a cursor (which you should) */

			return (0);


		}

		/* This event probably doesn't fit into non-top-down graphics models
		case TERM_XTRA_FROSH:
		{
			return (0);
		}
		*/

		/* You probably do not need to do anything for FRESH, actually.
		 * If you cannot use FROSH, set td->prefer_fresh to ask for
		 * full-time SDL_Flip() on TERM_XTRA_FRESH in main-sdl.c
		 */
		case TERM_XTRA_FRESH:
		{

			break;
		}

		/*
		case TERM_XTRA_NOISE:
		{
			return (0);
		}


		case TERM_XTRA_SOUND:
		{

			return (0);
		}*/

		case TERM_XTRA_BORED:
		{
			/* Please, feel free to do something here... */

			break;
		}

		/*  Hopefully, you don't need these either.
		case TERM_XTRA_REACT:
		{
			return (0);
		}
		

		case TERM_XTRA_ALIVE:
		{

			return (0);
		}

		case TERM_XTRA_LEVEL:
		{

			return (0);
		}

		case TERM_XTRA_DELAY:
		{

			return (0);
		}
		*/

		default:
			break;
	}

	/* Unknown or Unhandled action */
	return td->xtra_hook(n, v);
}


static errr Term_curs_xxx(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}


static errr Term_wipe_xxx(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* XXX XXX XXX */

	return td->wipe_hook(x, y, n);
}


/* Probably you should leave the text drawing to main-sdl.c
static errr Term_text_xxx(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	return (0);
}*/


/*
 * Draw some attr/char pairs on the screen
 *
 * This routine should display the given "n" attr/char pairs at
 * the given location (x,y).  This function is only used if one
 * of the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pairs, when displayed, will
 * erase anything (including any visual cursor) that used to be at
 * the given location.  On many machines this is automatic, but on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * the attr/char pair as an encoded index into a pixmap of special
 * "pictures".
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn by this function, which can be
 * very useful if this file can optimize its own display calls.
 *
 * This function is often associated with the "arg_graphics" flag.
 *
 * This function is only used if one of the "higher_pict" and/or
 * "always_pict" flags are set.
 */
static errr Term_pict_xxx(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);

	/* THIS IS POSSIBLY THE MOST IMPORTANT FUNCTION TO BE IMPLEMENTED HERE.
	 *
	 * DON'T SCREW IT UP. :)
	 *
	 * XXX XXX XXX
	 */


	return td->pict_hook(x, y, n, ap, cp); /* for testing (or if you wish to allow
															toggling of graphics types?) */
	/* Success */
	return (0);
}


#ifdef USE_TRANSPARENCY
static errr Term_pict_xxx_trans(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tac)
{
	term_data *td = (term_data*)(Term->data);
	graf_data *gd = (graf_data*)(td->graf);
	/* Perhaps use tap, tac to draw backgrounds/floors here. XXX XXX XXX */

	Term_pict_xxx(x, y, n, ap, ac);
}
#endif

/*** Internal Functions ***/


/*
 * Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.  One way to allow some
 * variation is to add fields to the "term_data" structure listing
 * parameters for that window, initialize them in the "init_xxx()"
 * function, and then use them in the code below.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 *
 * This function is called at the end of main-sdl.c's term_data_link
 * before "activation." You should use it to overwrite main-sdl.c's default
 * values. If, and only if, this function gets called, the user wants to 
 * use this graphics engine. 
 *
 * The most impotant thing here is hook initialization. It is appropriate
 * to call term_init() here since it will not be called from main-sdl.c's
 * term_data_link() if an alternate graphics engine is requested.
 *
 * Likely default flags have been uncommented. Fix them if they're not right
 * for you.
 */
static void term_data_link(term_data *data, int i)
{
	term_data *td = &data[i];
	term *t = &td->t;

	graf_data *gd = td->graf = &graphics;

	/* Initialize the term */
	term_init(t, 80, 24, 256);

	/* Choose "soft" or "hard" cursor XXX XXX XXX */
	/* A "soft" cursor must be explicitly "drawn" by the program */
	/* while a "hard" cursor has some "physical" existance and is */
	/* moved whenever text is drawn on the screen.  See "z-term.c". */
	t->soft_cursor = TRUE;

	/* Avoid the "corner" of the window XXX XXX XXX */
	/* t->icky_corner = TRUE; */

	/* Use "Term_pict()" for all attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* t->always_pict = TRUE; */

	/* Use "Term_pict()" for some attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	t->higher_pict = TRUE;

	/* Use "Term_text()" even for "black" text XXX XXX XXX */
	/* See the "Term_text_xxx()" function above. */
	/* t->always_text = TRUE; */

	/* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	t->never_bored = TRUE;

	/* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	/*t->never_frosh = TRUE; */

	/* Erase with "white space" XXX XXX XXX */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_xxx;
	t->nuke_hook = Term_nuke_xxx;

	/* Prepare the template hooks */
	t->user_hook = Term_user_xxx;
	t->xtra_hook = Term_xtra_xxx;
	t->curs_hook = Term_curs_xxx;
	t->wipe_hook = Term_wipe_xxx;
	
#ifdef USE_TRANSPARENCY
	t->pict_hook = Term_pict_xxx_trans;
#else
	t->pict_hook = Term_pict_xxx;
#endif

	/* let main-sdl.c handle text, probably... */
	t->text_hook = td->text_hook;


	/* Remember where we came from */
	t->data = (vptr)(td);

	/* If you expect special screen dimensions, speak now 
	 * or forever hold your peace. XXX */
#if 0
	td->width = 1024;
	td->height = 768;
#endif

	/* Please don't mess with td->bpp unless you have a very
	 * good reason to, though. */


	/* Activate it */
	Term_activate(t);

	/* Global pointer */
	//angband_term[i] = t;
}



/*
 * Initialization function
 * This is called from init_sdl() if this graphics engine is to be used.
 *
 * You may want command line parameters. 
 */
typedef void (*link_callback)(term_data *, int);

link_callback init_xxx(int argc, char **argv)
{


	/* Do you need to initalize something really special? Do you need to load
	 * your graphics? Do you need to convert the default Angband tiles to your
	 * own format? (Oh yes you do!) This is a good place to do it.
	 */

	/* Success */
	return term_data_link; /* On error, please return NULL */
}




#endif /* USE_XXX */
