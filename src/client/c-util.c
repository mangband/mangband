#include "c-angband.h"
#include "z-term.h"

#define MACRO_USE_CMD	0x01
#define MACRO_USE_STD	0x02


static bool after_macro = FALSE;
static bool parse_macro = FALSE;
static bool parse_under = FALSE;
static bool parse_slash = FALSE;
static bool strip_chars = FALSE;

static bool flush_later = FALSE;

static byte macro__use[256];

static char octify(uint i)
{
	return (hexsym[i%8]);
}

static char hexify(uint i)
{
	return (hexsym[i%16]);
}

void move_cursor(int row, int col)
{
	Term_gotoxy(col, row);
}

void flush(void)
{
	flush_later = TRUE;
}

void flush_now(void)
{
	/* Clear various flags */
	flush_later = FALSE;

	/* Cancel "macro" info */
	parse_macro = after_macro = FALSE;

	/* Cancel "sequence" info */
	parse_under = parse_slash = FALSE;

	/* Cancel "strip" mode */
	strip_chars = FALSE;

	/* Forgot old keypresses */
	Term_flush();
}

/*
 * Free the macro trigger package
 */
errr macro_trigger_free(void)
{
	int i;
	int num;

	if (macro_template != NULL)
	{
		/* Free the template */
		string_free(macro_template);
		macro_template = NULL;

		/* Free the trigger names and keycodes */
		for (i = 0; i < max_macrotrigger; i++)
		{
			string_free(macro_trigger_name[i]);

			string_free(macro_trigger_keycode[0][i]);
			string_free(macro_trigger_keycode[1][i]);
		}

		/* No more macro triggers */
		max_macrotrigger = 0;

		/* Count modifier-characters */
		num = strlen(macro_modifier_chr);

		/* Free modifier names */
		for (i = 0; i < num; i++)
		{
			string_free(macro_modifier_name[i]);
		}

		/* Free modifier chars */
		string_free(macro_modifier_chr);
		macro_modifier_chr = NULL;
	}

	/* Success */
	return (0);
}


/*
 * Check for possibly pending macros
 */
static int macro_maybe(cptr buf, int n)
{
	int i;

	/* Scan the macros */
	for (i = n; i < macro__num; i++)
	{
		/* Skip inactive macros */
		if (macro__cmd[i] && !inkey_flag) continue;

		/* Check for "prefix" */
		if (prefix(macro__pat[i], buf))
		{
			/* Ignore complete macros */
			if (!streq(macro__pat[i], buf)) return (i);
		}
	}

	/* No matches */
	return (-1);
}


/*
 * Find the longest completed macro
 */
static int macro_ready(cptr buf)
{
	int i, t, n = -1, s = -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; i++)
	{
		/* Skip inactive macros */
		if (macro__cmd[i] && !inkey_flag) continue;

		/* Check for "prefix" */
		if (!prefix(buf, macro__pat[i])) continue;

		/* Check the length of this entry */
		t = strlen(macro__pat[i]);

		/* Find the "longest" entry */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Return the result */
	return (n);
}

/*
 * Hack -- set graphics mode 
 */
void set_graphics(int mode) {
	use_graphics = mode;
	if (mode == 0) ANGBAND_GRAF = "none";
	if (mode == 1) ANGBAND_GRAF = "old";
	if (mode == 2) ANGBAND_GRAF = "new";
	if (mode == 3) ANGBAND_GRAF = "david";
}

/*
 * Hack -- add a macro definition (or redefinition).
 *
 * If "cmd_flag" is set then this macro is only active when
 * the user is being asked for a command (see below).
 */
void macro_add(cptr pat, cptr act, bool cmd_flag)
{
        int n;


        /* Paranoia -- require data */
        if (!pat || !act) return;


        /* Look for a re-usable slot */
        for (n = 0; n < macro__num; n++)
        {
                /* Notice macro redefinition */
                if (streq(macro__pat[n], pat))
                {
                        /* Free the old macro action */
                        string_free(macro__act[n]);

                        /* Save the macro action */
                        macro__act[n] = string_make(act);

                        /* Save the "cmd_flag" */
                        macro__cmd[n] = cmd_flag;

                        /* All done */
                        return;
                }
        }


        /* Save the pattern */
        macro__pat[macro__num] = string_make(pat);

        /* Save the macro action */
        macro__act[macro__num] = string_make(act);

        /* Save the "cmd_flag" */
        macro__cmd[macro__num] = cmd_flag;

        /* One more macro */
        macro__num++;


        /* Hack -- Note the "trigger" char */
        macro__use[(byte)(pat[0])] |= MACRO_USE_STD;

        /* Hack -- Note the "trigger" char of command macros */
        if (cmd_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_CMD;
}


/*
 * Helper function called only from "inkey()"
 *
 * This function does most of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros,
 * as well as "extra" keys read in while choosing a macro, and the
 * actual action for the macro.
 *
 * Embedded macros are illegal, although "clever" use of special
 * control chars may bypass this restriction.  Be very careful.
 *
 * The user only gets 500 (1+2+...+29+30) milliseconds for the macro.
 *
 * Note the annoying special processing to "correctly" handle the
 * special "control-backslash" codes following a "control-underscore"
 * macro sequence.  See "main-x11.c" and "main-xaw.c" for details.
 *
 *
 * Note that we NEVER wait for a keypress without also checking the network
 * for incoming packets.  This stops annoying "timeouts" and also lets
 * the screen get redrawn if need be while we are waiting for a key. --KLJ--
 */
static char inkey_aux(void)
{
	int	k = 0, n, p = 0, w = 0;

	char	ch = 0;
	event_type chkey;

	cptr	pat, act;

	char	buf[1024];
	

	/* Fetch keypress */
	/* Unset hack before we start */
	inkey_exit = FALSE;

	/* Wait for keypress, while also checking for net input */
	do
	{
		/* Look for a keypress */
		(void)(Term_inkey(&chkey, FALSE, TRUE));
		ch = chkey.key;

		/* If we got a key, break */
		if (ch) break;

		/* Do networking */
		network_loop();

		/* Update screen */
		flush_updates();

		/* Hack: perform emergency network-ordered escape */
		if (inkey_exit)
		{
			inkey_exit = FALSE;
			return (ESCAPE);
		}

		/* Return as soon as possible */
		if (inkey_nonblock)
		{
			return (0);
		}
	} while (!ch);

	
#if 0
	int net_fd;

	/* Acquire and save maximum file descriptor */
	net_fd = Net_fd();

	/* If no network yet, just wait for a keypress */
	if (net_fd == -1)
	{
		/* Look for a keypress */
		Term_inkey(&chkey, TRUE, TRUE);
		ch = chkey.key;
	}
	else
	{
		/* Hack -- initially process any data that is stil in rbuf, so it won't be
		 * wiped in Net_read if more data arrives.  Data still might be in rbuf
		 * since we may have been called from a Receive handler such as
		 * Receive_pause.
		 */
		Sockbuf_advance(&rbuf, rbuf.ptr - rbuf.buf);
		if (Sockbuf_write(&cbuf, rbuf.ptr, rbuf.len) != rbuf.len)
		{
			plog("Can't copy reliable data to buffer in Net_input");
			return -1;
		}
		Net_packet();
		/* Redraw stuff if necessary */
		if (p_ptr->redraw)
		{
			redraw_stuff();
		}
		/* Update the screen */
		Term_fresh();
		/* Redraw windows if necessary */
		if (p_ptr->window)
		{
			window_stuff();
		}


		/* Unset hack before we start */
		inkey_exit = FALSE;

		/* Wait for keypress, while also checking for net input */
		do
		{
			int result;

			/* Look for a keypress */
			Term_inkey(&chkey, FALSE, TRUE);
			ch = chkey.key;

			/* If we got a key, break */
			if (ch) break;

			/* Update our timer and if neccecary send a keepalive packet
			 */
			update_ticks();
			do_keepalive();

			/* Flush the network output buffer */
			Net_flush();

			/* Wait for .001 sec, or until there is net input */
			SetTimeout(0, 1000);

			/* Parse net input if we got any */
			if (SocketReadable(net_fd))
			{
				if ((result = Net_input()) == -1)
				{
					quit(NULL);
				}

				/* Redraw stuff if necessary */
				if (p_ptr->redraw)
				{
					redraw_stuff();
				}

				/* Update the screen */
				Term_fresh();

				/* Redraw windows if necessary */
				if (p_ptr->window)
				{
					window_stuff();
				}
			}

			/* Hack: perform emergency network-ordered escape */
			if (inkey_exit)
			{
				inkey_exit = FALSE;
				return (ESCAPE);
			}
		} while (!ch);
	}
#endif
	/* ARCANE MAGIC STARTS BELOW: */


	/* End of internal macro */
	if (ch == 29) parse_macro = FALSE;


	/* Do not check "ascii 28" */
	if (ch == 28) return (ch);

	
	/* Do not check "ascii 29" */
	if (ch == 29) return (ch);


	/* Do not check macro actions */
	if (parse_macro) return (ch);

	/* Do not check "control-underscore" sequences */
	if (parse_under) return (ch);

	/* Do not check "control-backslash" sequences */
	if (parse_slash) return (ch);


	/* Efficiency -- Ignore impossible macros */
	if (!macro__use[(byte)(ch)]) return (ch);

	/* Efficiency -- Ignore inactive macros */
	if (!inkey_flag && (macro__use[(byte)(ch)] == MACRO_USE_CMD)) return (ch);

#ifdef USE_GCU
	/* NCurses client actually uses escape as "begin macro" sequence */
	if (escape_in_macro_triggers)
	{
		/* So we allow it */
	} else
#endif
	/* Efficiency/Hack -- Ignore escape key for macros */
	if (ch == ESCAPE) { first_escape = TRUE; return (ch); }

	/* Save the first key, advance */
	buf[p++] = ch;
	buf[p] = '\0';


	/* Wait for a macro, or a timeout */
	while (TRUE)
	{
		/* Check for possible macros */
		k = macro_maybe(buf, k);

		/* Nothing matches */
		if (k < 0) break;

		/* Check for (and remove) a pending key */
		if (0 == Term_inkey(&chkey, FALSE, TRUE))
		{
			ch = chkey.key;

			/* Append the key */
			buf[p++] = ch;
			buf[p] = '\0';

			/* Restart wait */
			w = 0;
		}

		/* No key ready */
		else
		{
			/* Increase "wait" */
			w += 10;

			/* Excessive delay */
			if (w >= 100) break;

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, w);
		}
	}


	/* Check for a successful macro */
	k = macro_ready(buf);

	/* No macro available */
	if (k < 0)
	{
		/* Push all the keys back on the queue */
		while (p > 0)
		{
			/* Push the key, notice over-flow */
			if (Term_key_push(buf[--p])) return (0);
		}

		/* Wait for (and remove) a pending key */
		(void)Term_inkey(&chkey, TRUE, TRUE);
		ch = chkey.key;

		/* Return the key */
		return (ch);
	}


	/* Access the macro pattern */
	pat = macro__pat[k];

	/* Get the length of the pattern */
	n = strlen(pat);


	/* Push the "extra" keys back on the queue */
	while (p > n)
	{
		if (Term_key_push(buf[--p])) return (0);
	}

	/* We are now inside a macro */
	parse_macro = TRUE;
	first_escape = TRUE;

	/* Push the "macro complete" key */
	if (Term_key_push(29)) return (0);

	
	/* Access the macro action */
	act = macro__act[k];

	/* Get the length of the action */
	n = strlen(act);

	/* Push the macro "action" onto the key queue */
	while (n > 0)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(act[--n])) return (0);
	}

	/* Force "inkey()" to call us again */
	return (0);
}

/*
 * Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()".  This key sequence will not
 * trigger any macros, and cannot be bypassed by the Borg.  It is used
 * in Angband to handle "keymaps".
 */
static cptr inkey_next = NULL;

/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is TRUE, then all pending keypresses will be flushed,
 * and any macro processing in progress will be aborted.  This flag is
 * set by the "flush()" function, which does not actually flush anything
 * itself, but rather, triggers delayed input flushing via "inkey_xtra".
 *
 * If "inkey_scan" is TRUE, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_base" is TRUE, then all macro processing will be bypassed.
 * If "inkey_base" and "inkey_scan" are both TRUE, then this function will
 * not return immediately, but will wait for a keypress for as long as the
 * normal macro matching code would, allowing the direct entry of macro
 * triggers.  The "inkey_base" flag is extremely dangerous!
 *
 * If "inkey_flag" is TRUE, then we will assume that we are waiting for a
 * normal command, and we will only show the cursor if "hilite_player" is
 * TRUE (or if the player is in a store), instead of always showing the
 * cursor.  The various "main-xxx.c" files should avoid saving the game
 * in response to a "menu item" request unless "inkey_flag" is TRUE, to
 * prevent savefile corruption.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of "ascii 30" (ctrl-caret, aka ctrl-shift-six)
 * and "ascii 31" (ctrl-underscore, aka ctrl-shift-minus), which are used to
 * provide support for simple keyboard "macros".  These keys are so strange
 * that their loss as normal keys will probably be noticed by nobody.  The
 * "ascii 30" key is used to indicate the "end" of a macro action, which
 * allows recursive macros to be avoided.  The "ascii 31" key is used by
 * some of the "main-xxx.c" files to introduce macro trigger sequences.
 *
 * Hack -- we use "ascii 29" (ctrl-right-bracket) as a special "magic" key,
 * which can be used to give a variety of "sub-commands" which can be used
 * any time.  These sub-commands could include commands to take a picture of
 * the current screen, to start/stop recording a macro action, etc.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
event_type inkey_ex(void)
{
	bool cursor_state;
	event_type kk;
	event_type ke;
	
	bool done = FALSE;
	
	term *old = Term;
	
	
	/* Initialise keypress */
	ke.key = 0;
	ke.type = EVT_KBRD;
	
	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next && !inkey_xtra)
	{
		/* Get next character, and advance */
		ke.key = *inkey_next++;
		
		/* Cancel the various "global parameters" */
		inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
		
		/* Accept result */
		return (ke);
	}
	
	/* Forget pointer */
	inkey_next = NULL;
	
	
#ifdef ALLOW_BORG
	
	/* Mega-Hack -- Use the special hook */
	if (inkey_hack && ((ch = (*inkey_hack)(inkey_xtra)) != 0))
	{
		/* Cancel the various "global parameters" */
		inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
		ke.type = EVT_KBRD;
		
		/* Accept result */
		return (ke);
	}
	
#endif /* ALLOW_BORG */
	
	
	/* Hack -- handle delayed "flush()" */
	if (inkey_xtra)
	{
		/* End "macro action" */
		parse_macro = FALSE;
		ke.type = EVT_KBRD;
		
		/* End "macro trigger" */
		parse_under = FALSE;
		
		/* Forget old keypresses */
		Term_flush();
	}
	
	
	/* Get the cursor state */
	(void)Term_get_cursor(&cursor_state);
	
	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag /*|| hilite_player || character_icky*/))
	{
		/* Show the cursor */
		(void)Term_set_cursor(TRUE);
	}
	
	
	/* Hack -- Activate main screen */
	Term_activate(term_screen);
	
	
	/* Get a key */
	while (!ke.key)
	{
		/* Hack -- Handle "inkey_scan" */
		if (!inkey_base && inkey_scan &&
		   (0 != Term_inkey(&kk, FALSE, FALSE)))
		{
			break;
		}
		

		/* Hack -- Flush output once when no key ready */
		if (!done && (0 != Term_inkey(&kk, FALSE, FALSE)))
		{

			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate main screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			/*character_saved = FALSE;*/
		
			/* Mega-Hack -- reset signal counter */
			/*signal_count = 0;*/
		
			/* Only once */
			done = TRUE;
		}
		
		
		/* Hack -- Handle "inkey_base" */
		if (inkey_base)
		{
			int w = 0;

			/* Wait forever */
			if (!inkey_scan)
			{
				/* Wait for (and remove) a pending key */
				if (0 == Term_inkey(&ke, TRUE, TRUE))
				{
					/* Done */
					break;
				}

				/* Oops */
				break;
			}

			/* Wait only as long as macro activation would wait*/
			while (TRUE)
			{
				/* Check for (and remove) a pending key */
				if (0 == Term_inkey(&ke, FALSE, TRUE))
				{
					/* Done */
					break;
				}

				/* No key ready */
				else
				{
					/* Increase "wait" */
					w += 10;

					/* Excessive delay */
					if (w >= 100) break;

					/* Delay */
					Term_xtra(TERM_XTRA_DELAY, w);

				}
			}

			/* Done */
			ke.type = EVT_KBRD;
			break;
		}
		
		
			/* Get a key (see above) */
			ke.key = inkey_aux();
		
		
		/* Handle "control-right-bracket" */
		if (ke.key == 29)
		{
			/* Strip this key */
			ke.key = 0;
		
			/* Continue */
			continue;
		}
		
		
		/* Treat back-quote as escape */
		if (ke.key == '`') ke.key = ESCAPE;
		
		
		/* End "macro trigger" */
		if (parse_under && (ke.key <= 32))
		{
			/* Strip this key */
			ke.key = 0;
		
			/* End "macro trigger" */
			parse_under = FALSE;
		}

		/* Handle "control-caret" */
		if (ke.key == 30)
		{
			/* Strip this key */
			ke.key = 0;
		}

		/* Handle "control-underscore" */
		else if (ke.key == 31)
		{
			/* Strip this key */
			ke.key = 0;

			/* Begin "macro trigger" */
			parse_under = TRUE;
		}

		/* Inside "macro trigger" */
    	else if (parse_under)
		{
			/* Strip this key */
			ke.key = 0;
		}
	}
	
	
	/* Hack -- restore the term */
	Term_activate(old);
	
	
	/* Restore the cursor */
	Term_set_cursor(cursor_state);
	
	
	/* Cancel the various "global parameters" */
	inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
	
	
	/* Return the keypress */
	return (ke);
}


/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * Normally, this function will process "macros", but if "inkey_base" is
 * TRUE, then we will bypass all "macro" processing.  This allows direct
 * usage of the "Term_inkey()" function.
 *
 * Normally, this function will do something, but if "inkey_xtra" is TRUE,
 * then something else will happen.
 *
 * Normally, this function will wait until a "real" key is ready, but if
 * "inkey_scan" is TRUE, then we will return zero if no keys are ready.
 *
 * Normally, this function will show the cursor, and will process all normal
 * macros, but if "inkey_flag" is TRUE, then we will only show the cursor if
 * "hilite_player" is TRUE, and also, we will only process "command" macros.
 *
 * Note that the "flush()" function does not actually flush the input queue,
 * but waits until "inkey()" is called to perform the "flush".
 *
 * Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
 *
 * Refresh the screen if waiting for a keypress and no key is ready.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of a few "special" control-keys, which
 * are reserved to simplify the use of various "main-xxx.c" files,
 * or used by the "macro" code above.
 *
 * Ascii 27 is "control left bracket" -- normal "Escape" key
 * Ascii 28 is "control backslash" -- special macro delimiter
 * Ascii 29 is "control right bracket" -- end of macro action
 * Ascii 30 is "control caret" -- indicates "keypad" key
 * Ascii 31 is "control underscore" -- begin macro-trigger
 *
 * Hack -- Make sure to allow calls to "inkey()" even if "term_screen"
 * is not the active Term, this allows the various "main-xxx.c" files
 * to only handle input when "term_screen" is "active".
 *
 * Note the nasty code used to process the "inkey_base" flag, which allows
 * various "macro triggers" to be entered as normal key-sequences, with the
 * appropriate timing constraints, but without actually matching against any
 * macro sequences.  Most of the nastiness is to handle "ascii 28" (see below).
 *
 * The "ascii 28" code is a complete hack, used to allow "default actions"
 * to be associated with a given keypress, and used only by the X11 module,
 * it may or may not actually work.  The theory is that a keypress can send
 * a special sequence, consisting of a "macro trigger" plus a "default action",
 * with the "default action" surrounded by "ascii 28" symbols.  Then, when that
 * key is pressed, if the trigger matches any macro, the correct action will be
 * executed, and the "strip default action" code will remove the "default action"
 * from the keypress queue, while if it does not match, the trigger itself will
 * be stripped, and then the "ascii 28" symbols will be stripped as well, leaving
 * the "default action" keys in the "key queue".  Again, this may not work.
 */
char inkey(void)
{
	char kk, ch;
	event_type chkey;

	bool done = FALSE;

	term *old = Term;

	int w = 0;

	int skipping = FALSE;

	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next && !flush_later)
	{
	
		/* Get next character, and advance */
		ch = *inkey_next++;

		/* Cancel the various "global parameters" */
		inkey_base = flush_later = inkey_flag = inkey_scan = FALSE;

		command_cmd = ch;
		command_dir = 0;

		/* Accept result */
		return (ch);
	}

	/* Forget pointer */
	inkey_next = NULL;
	
        /* Hack -- handle delayed "flush()" */
        if (flush_later)
        {
                /* Done */
                flush_later = FALSE;

                /* Cancel "macro" info */
                parse_macro = after_macro = FALSE;

                /* Cancel "sequence" info */
                parse_under = parse_slash = FALSE;

                /* Cancel "strip" mode */
                strip_chars = FALSE;

                /* Forget old keypresses */
                Term_flush();
        }


	/* Access cursor state */
	/*(void)Term_get_cursor(&v);*/

	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag))
	{
		/* Show the cursor */	
		/*(void)Term_set_cursor(1);*/
	}


	/* Hack -- Activate the screen */
	Term_activate(term_screen);


	/* Get a (non-zero) keypress */
	for (ch = 0; !ch; )
	{
		/* Nothing ready, not waiting, and not doing "inkey_base" */
		if (!inkey_base && inkey_scan && (0 != Term_inkey(&chkey, FALSE, FALSE))) break;


		/* Hack -- flush the output once no key is ready */
		if (!done && (0 != Term_inkey(&chkey, FALSE, FALSE)))
		{
			ch = chkey.key;

			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate the screen */
			Term_activate(term_screen);

			/* Only once */
			done = TRUE;
		}

		/* Hack */
		if (inkey_base)
		{
			event_type xh;

			/* Check for keypress, optional wait */
			(void)Term_inkey(&xh, !inkey_scan, TRUE);

			/* Key ready */
			if (xh.key)
			{
				/* Reset delay */
				w = 0;

				/* Mega-Hack */
				if (xh.key == 28)
				{
					/* Toggle "skipping" */
					skipping = !skipping;
				}

				/* Use normal keys */
				else if (!skipping)
				{
					/* Use it */
					ch = xh.key;
				}
			}

			/* No key ready */
			else
			{
				/* Increase "wait" */
				w += 10;

				/* Excessive delay */
				if (w >= 100) break;

				/* Delay */
				Term_xtra(TERM_XTRA_DELAY, w);
			}

			/* Continue */
			continue;
		}


		/* Get a key (see above) */
		kk = ch = inkey_aux();

		/* Hack -- special nonblock mode */
		if (!ch && inkey_nonblock) return (0);

		/* Finished a "control-underscore" sequence */
		if (parse_under && (ch <= 32))
		{
			/* Found the edge */
			parse_under = FALSE;

			/* Stop stripping */
			strip_chars = FALSE;

			/* Strip this key */
			ch = 0;
		}


		/* Finished a "control-backslash" sequence */
		if (parse_slash && (ch == 28))
		{
			/* Found the edge */
			parse_slash = FALSE;

			/* Stop stripping */
			strip_chars = FALSE;

			/* Strip this key */
			ch = 0;
		}


		/* Handle some special keys */
		switch (ch)
		{
			/* Hack -- convert back-quote into escape */
			case '`':

			/* Convert to "Escape" */
			ch = ESCAPE;

			/* Done */
			break;

			/* Hack -- strip "control-right-bracket" end-of-macro-action */
			case 29:

			/* Strip this key */
			ch = 0;

			/* Done */
			break;

			/* Hack -- strip "control-caret" special-keypad-indicator */
			case 30:

			/* Strip this key */
			ch = 0;

			/* Done */
			break;

			/* Hack -- strip "control-underscore" special-macro-triggers */
			case 31:

			/* Strip this key */
			ch = 0;

			/* Inside a "underscore" sequence */
			parse_under = TRUE;

			/* Strip chars (always) */
			strip_chars = TRUE;

			/* Done */
			break;

			/* Hack -- strip "control-backslash" special-fallback-strings */
			case 28:

			/* Strip this key */
			ch = 0;

			/* Inside a "control-backslash" sequence */
			parse_slash = TRUE;

			/* Strip chars (sometimes) */
			strip_chars = after_macro;

			/* Done */
			break;
		}


		/* Hack -- Set "after_macro" code */
		after_macro = ((kk == 29) ? TRUE : FALSE);


		/* Hack -- strip chars */
		if (strip_chars) ch = 0;
	}


	/* Hack -- restore the term */
	Term_activate(old);


	/* Restore the cursor */
	/*Term_set_cursor(v);*/


	/* Cancel the various "global parameters" */
	inkey_base = inkey_scan = inkey_flag = FALSE;


	/* Return the keypress */
	return (ch);
}

#if 0
static int hack_dir = 0;
/*
 * Convert a "Rogue" keypress into an "Angband" keypress
 * Pass extra information as needed via "hack_dir"
 *
 * Note that many "Rogue" keypresses encode a direction.
 */
static char roguelike_commands(char command)
{
        /* Process the command */
        switch (command)
        {
                /* Movement (rogue keys) */
                case 'b': hack_dir = 1; return (';');
                case 'j': hack_dir = 2; return (';');
                case 'n': hack_dir = 3; return (';');
                case 'h': hack_dir = 4; return (';');
                case 'l': hack_dir = 6; return (';');
                case 'y': hack_dir = 7; return (';');
                case 'k': hack_dir = 8; return (';');
                case 'u': hack_dir = 9; return (';');

                /* Running (shift + rogue keys) */
                case 'B': hack_dir = 1; return ('.');
                case 'J': hack_dir = 2; return ('.');
                case 'N': hack_dir = 3; return ('.');
                case 'H': hack_dir = 4; return ('.');
                case 'L': hack_dir = 6; return ('.');
                case 'Y': hack_dir = 7; return ('.');
                case 'K': hack_dir = 8; return ('.');
                case 'U': hack_dir = 9; return ('.');

                /* Tunnelling (control + rogue keys) */
                case KTRL('B'): hack_dir = 1; return ('+');
                case KTRL('J'): hack_dir = 2; return ('+');
                case KTRL('N'): hack_dir = 3; return ('+');
                case KTRL('H'): hack_dir = 4; return ('+');
                case KTRL('L'): hack_dir = 6; return ('+');
                case KTRL('Y'): hack_dir = 7; return ('+');
                case KTRL('K'): hack_dir = 8; return ('+');
                case KTRL('U'): hack_dir = 9; return ('+');

                /* Hack -- White-space */
                case KTRL('M'): return ('\r');

                /* Allow use of the "destroy" command */
                case KTRL('D'): return ('k');

                /* Hack -- Commit suicide */
                case KTRL('C'): return ('Q');

                /* Locate player on map */
                case 'W': return ('L');

                /* Browse a book (Peruse) */
                case 'P': return ('b');

                /* Steal */
                case 'S': return ('j');

                /* Toggle search mode */
                case '#': return ('S');

                /* Use a staff (Zap) */
                case 'Z': return ('u');

                /* Take off equipment */
                case 'T': return ('t');

                /* Fire an item */
                case 't': return ('f');

                /* Bash a door (Force) */
                case 'f': return ('B');

                /* Look around (examine) */
                case 'x': return ('l');

                /* Aim a wand (Zap) */
                case 'z': return ('a');

                /* Zap a rod (Activate) */
                case 'a': return ('z');

		/* Party mode */
		case 'O': return ('P');

                /* Run */
                case ',': return ('.');

                /* Stay still (fake direction) */
                case '.': hack_dir = 5; return (',');

                /* Stay still (fake direction) */
                case '5': hack_dir = 5; return (',');

                /* Standard walking */
                case '1': hack_dir = 1; return (';');
                case '2': hack_dir = 2; return (';');
                case '3': hack_dir = 3; return (';');
                case '4': hack_dir = 4; return (';');
                case '6': hack_dir = 6; return (';');
                case '7': hack_dir = 7; return (';');
                case '8': hack_dir = 8; return (';');
                case '9': hack_dir = 9; return (';');
        }

        /* Default */
        return (command);
}



/*
 * Convert an "Original" keypress into an "Angband" keypress
 * Pass direction information back via "hack_dir".
 *
 * Note that "Original" and "Angband" are very similar.
 */
static char original_commands(char command)
{
	/* Process the command */
	switch(command)
	{
		/* Hack -- White space */
		case KTRL('J'): return ('\r');
		case KTRL('M'): return ('\r');

		/* Tunnel */
		case 'T': return ('+');

		/* Run */
		case '.': return ('.');

		/* Stay still (fake direction) */
		case ',': hack_dir = 5; return (',');

		/* Stay still (fake direction) */
		case '5': hack_dir = 5; return (',');

		/* Standard walking */
		case '1': hack_dir = 1; return (';');
		case '2': hack_dir = 2; return (';');
		case '3': hack_dir = 3; return (';');
		case '4': hack_dir = 4; return (';');
		case '6': hack_dir = 6; return (';');
		case '7': hack_dir = 7; return (';');
		case '8': hack_dir = 8; return (';');
		case '9': hack_dir = 9; return (';');

		/* Hack -- Commit suicide */
		case KTRL('K'): return ('Q');
		case KTRL('C'): return ('Q');
	}

	/* Default */
	return (command);
}
#endif


/*
 * Flush the screen, make a noise
 */
void bell(void)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}

/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
	/* Hack -- fake monochrome */
	/* if (!use_color) attr = TERM_WHITE; */

	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}

/*
 * As above, but in "white"
 */
void prt(cptr str, int row, int col)
{
	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}


/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note: "len" MUST BE LESS than total "buf" size.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 */

/* APD -- added private so passwords will not be displayed. */
bool askfor_aux(char *buf, int len, char m_private)
{
	int y, x;

	int i = 0;

	int k = 0;

	bool done = FALSE;


	/* Locate the cursor */
	Term_locate(&x, &y);

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Paranoia -- check len */
	if (len < 1) len = 1;

	/* Paranoia -- check column */
	if ((x < 0) || (x >= 80)) x = 0;

	/* Restrict the length */
	if (x + len > 80) len = 80 - x;


	/* Paranoia -- Clip the default entry */
	buf[len] = '\0';


	/* Display the default answer */
	Term_erase(x, y, len);
	Term_putstr(x, y, -1, TERM_YELLOW, m_private ? "xxxxxx" : buf);


	/* Process input */
	while (!done)
	{
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		i = inkey();

		/* Analyze the key */
		switch (i)
		{
			case ESCAPE:
			k = 0;
			done = TRUE;
			break;

			case '\n':
			case '\r':
			k = strlen(buf);
			done = TRUE;
			break;

			case 0x7F:
			case '\010':
			if (k > 0) k--;
			break;

			default:
			if ((k < len) && (isprint(i)))
			{
				buf[k++] = i;

				/* Update the entry */
				Term_putch(x+k-1, y, TERM_WHITE, m_private ? 'x' : buf[k-1]);
			}
			else
			{
				bell();
			}
			break;
		}

		Term_erase(x+k, y, len - k);

		/* Terminate */
		buf[k] = '\0';

	}

	/* The top line is OK now */
	topline_icky = FALSE;
	Flush_queue();

	/* Aborted */
	if (i == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Get a string from the user
 *
 * The "prompt" should take the form "Prompt: "
 *
 * Note: "len" MUST BE LESS than total "buf" size.
 *
 * Note that the initial contents of the string is used as
 * the default response, so be sure to "clear" it if needed.
 *
 * We clear the input, and return FALSE, on "ESCAPE".
 */
bool get_string(cptr prompt, char *buf, int len)
{
	bool res;

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len, 0);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}

/* Same as get_string, but with ** .. stupid code duplication :( */
bool get_string_masked(cptr prompt, char *buf, int len)
{
	bool res;

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len, 1);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}
/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	*command = inkey();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Fix the top line */
	topline_icky = FALSE;

	/* Flush any events */
	Flush_queue();

	/* Handle "cancel" */
	if (*command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];

/*
 * Request a command from the user.
 *
 * Sets command_cmd, command_dir, command_rep, command_arg.
 *
 * Note that "caret" ("^") is treated special, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 */
void request_command(bool shopping)
{
	char cmd;
	
	int mode;
	
	cptr act;
	
	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}

	/* Flush the input */
	/* flush(); */

	/* Get a keypress in "command" mode */

	/* Activate "command mode" */
	inkey_flag = TRUE;

	/* Activate "scan mode" */
	inkey_scan = TRUE;

	/* Get a command */
	cmd = inkey();

	/* Return if no key was pressed */
	if (!cmd) return;

	msg_flag = FALSE;
	c_msg_print(NULL);

	/* Clear top line */
	prt("", 0, 0);

	/* Bypass "keymap" */
	if (cmd == '\\')
	{
		/* Get a char to use without casting */
		(void)(get_com("Command: ", &cmd));

		/* Hack -- bypass keymaps */
		if (!inkey_next) inkey_next = "";
		
		/* Hack -- allow "control chars" to be entered */
		if (cmd == '^')
		{
			/* Get a char to "cast" into a control char */
			(void)(get_com("Command: Control: ", &cmd));

			/* Convert */
			cmd = KTRL(cmd);
		}

		/* Use the key directly */
		command_cmd = cmd;
	}

	else
	{
		/* Hack -- allow "control chars" to be entered */
		if (cmd == '^')
		{
			/* Get a char to "cast" into a control char */
			(void)(get_com("Control: ", &cmd));

			/* Convert */
			cmd = KTRL(cmd);
		}
		
		/* Look up applicable keymap */
		act = keymap_act[mode][(byte)(cmd)];

		/* Apply keymap if not inside a keymap already */
		if (act && !inkey_next)
		{
			/* Install the keymap */
			my_strcpy(request_command_buffer, act,
			          sizeof(request_command_buffer));

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			return;
		}
		
		/* Accept command */
		command_cmd = cmd;
	}

	/* Paranoia */
	if (!command_cmd) command_cmd = ESCAPE;

	/* Shopping */
	if (shopping)
	{
		/* Convert */
		switch (command_cmd)
		{
			/* Command "p" -> "purchase" (get) */
			case 'p': command_cmd = 'g'; break;

			/* Command "m" -> "purchase" (get) */
			case 'm': command_cmd = 'g'; break;

			/* Command "s" -> "sell" (drop) */
			case 's': command_cmd = 'd'; break;
			
			/* Command "I" -> "look" (Inspect) */
			case 'I': command_cmd = 'l'; break;
			
			/* Command "x" -> "look" (examine) */
			case 'x': command_cmd = 'l'; break;
		}
	}

	/* Hack -- erase the message line. */
	prt("", 0, 0);
}

bool c_get_dir(char *dp, cptr prompt, bool allow_target, bool allow_friend)
{
	int	dir = 0;

	char	command;

	cptr	p;

	char buf[80];
	buf[0] = '\0';
	my_strcpy(buf, prompt, 80);
	
	if (allow_target)
		p = "Direction ('*' to choose target, non-direction cancels) ?";
	else
		p = "Direction (non-direction cancels)?";
	strcat(buf, p);

	get_com(buf, &command);

	/* Handle target request */
	if (command == '*' && allow_target)
	{
		if (cmd_target())
			dir = 5;
	}

	else if (command == '(' && allow_friend)
	{
		if (cmd_target_friendly())
			dir = 5;
	}

	else dir = target_dir(command);

	*dp = (byte)dir;

	if (!dir) return (FALSE);

	return (TRUE);
}

/*
 * Extract a direction (or zero) from a character
 */
int target_dir(char ch)
{
	int d = 0;

	int mode;

	cptr act;

	cptr s;


	/* Already a direction? */
	if (isdigit((unsigned char)ch))
	{
		d = D2I(ch);
	}
	else
	{
		/* Roguelike */
		if (rogue_like_commands)
		{
			mode = KEYMAP_MODE_ROGUE;
		}

		/* Original */
		else
		{
			mode = KEYMAP_MODE_ORIG;
		}

		/* Extract the action (if any) */
		act = keymap_act[mode][(byte)(ch)];

		/* Analyze */
		if (act)
		{
			/* Convert to a direction */
			for (s = act; *s; ++s)
			{
				/* Use any digits in keymap */
				if (isdigit((unsigned char)*s)) d = D2I(*s);
			}
		}
	}

	/* Paranoia */
	if (d == 5) d = 0;

	/* Return direction */
	return (d);
}

bool get_dir(int *dp)
{
	int	dir = 0;

	char	command;

	cptr	p;

	p = "Direction ('*' to choose a target, non-direction cancels) ? ";

	get_com(p, &command);

	/* Handle target request */
	if (command == '*')
	{
		if (cmd_target())
			dir = 5;
	}

	else dir = target_dir(command);

	*dp = dir;

	if (!dir) return (FALSE);

	return (TRUE);
}


/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/*
 * As above, but in "white"
 */
void put_str(cptr str, int row, int col)
{
	/* Spawn */
	Term_putstr(col, row, -1, TERM_WHITE, str);
}

/*
 * Memorize an already drawn line.
 */
void mem_line(int row, int col, int n)
{
	int i;
	/* No ickyness */
	if (!Term->mem) return;

	/* Check row bounds */
	if (row < 0) return;
	if (row >= Term->hgt) return;

	for (i = 0; i < n; i++)
	{
		if (col + i < 0) continue;
		if (col + i >= Term->wid) break;
		Term_mem_ch(col + i, row,
			Term->scr->a[row][col+i],
			Term->scr->c[row][col+i],
			Term->scr->ta[row][col+i],
			Term->scr->tc[row][col+i]);
	}
}

/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(cptr prompt)
{
	int i;

	char buf[80];
	
	/* Option -- "auto_accept" */
	if (auto_accept) return (TRUE);

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		i = inkey();
		if (i == ESCAPE) break;
		if (strchr("YyNn", i)) break;
		bell();
	}

	/* Erase the prompt */
	prt("", 0, 0);

	/* The top line is OK again */
	topline_icky = FALSE;

	/* Flush any events that came in while we were icky */
	Flush_queue();

	/* Normal negation */
	if ((i != 'Y') && (i != 'y')) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Calculate the index of a message
 */
static s16b message_age2idx(int age)
{
	return ((message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX);
}

/*
 * Recall the "text" of a saved message
 */
cptr message_str(s16b age)
{
	static char buf[1024];
	s16b x;
	u16b o;
	cptr s;

	/* Forgotten messages have no text */
	if ((age < 0) || (age >= message_num())) return ("");

	/* Get the "logical" index */
	x = message_age2idx(age);

	/* Get the "offset" for the message */
	o = message__ptr[x];

	/* Get the message text */
	s = &message__buf[o];

	/* HACK - Handle repeated messages */
	if (message__count[x] > 1)
	{
		strnfmt(buf, sizeof(buf), "%s <%dx>", s, message__count[x]);
		s = buf;
	}

	/* Return the message text */
	return (s);
}

/*
 * Recall the "text" of a last saved message, which is NON-LOCAL
 */
cptr message_last()
{
    s16b x;
    u16b o;
    cptr s = "";

    /* Get the "logical" last index */
    x = message_age2idx(0);

    /* Loop */
    while (x != message__last)
    {
        /* Get the "offset" for the message */
        o = message__ptr[x];

        /* Get the message text */
        s = &message__buf[o];

        /* Make sure it's not "local" */
        if (message__type[x] != MSG_LOCAL) break;

        /* Advance x, wrap if needed */
        if (x == 0) x = MESSAGE_MAX - 1;
        else x--;
    }

    /* Return the message text */
    return (s);
}

/*
 * How many messages are "available"?
 */
s16b message_num(void)
{
        int last, next, n;

        /* Extract the indexes */
        last = message__last;
        next = message__next;

        /* Handle "wrap" */
        if (next < last) next += MESSAGE_MAX;

        /* Extract the space */
        n = (next - last);

        /* Return the result */
        return (n);
}

/*
 * Recall the "type" of a saved message
 */
u16b message_type(s16b age)
{
	s16b x;

	/* Paranoia */
	if (!message__type) return (MSG_GENERIC);

	/* Forgotten messages are generic */
	if ((age < 0) || (age >= message_num())) return (MSG_GENERIC);

	/* Get the "logical" index */
	x = message_age2idx(age);

	/* Return the message type */
	return (message__type[x]);
}

/*
 * Add a new message, with great efficiency
 */
void c_message_add(cptr str, u16b type)
{
	int k, i, x, o;
	size_t n;

	cptr s;

	cptr u;
	char *v;

	/*** Step 1 -- Analyze the message ***/

	/* Hack -- Ignore "non-messages" */
	if (!str) return;

	/* Message length */
	n = strlen(str);

	/* Hack -- Ignore "long" messages */
	if (n >= MESSAGE_BUF / 4) return;


	/*** Step 2 -- Attempt to optimize ***/

	/* Get the "logical" last index */
	x = message_age2idx(0);

	/* Get the "offset" for the last message */
	o = message__ptr[x];

	/* Get the message text */
	s = &message__buf[o];

	/* Last message repeated? */
	if (streq(str, s))
	{
		/* Increase the message count */
		message__count[x]++;

		/* Redraw */
		p_ptr->window |= (PW_MESSAGE | PW_MESSAGE_CHAT);

		/* Success */
		return;
	}

	/*** Step 3 -- Attempt to optimize ***/

	/* Limit number of messages to check */
	k = message_num() / 4;

	/* Limit number of messages to check */
	if (k > 32) k = 32;

	/* Start just after the most recent message */
	i = message__next;

	/* Check the last few messages for duplication */
	for ( ; k; k--)
	{
		u16b q;

		cptr old;

		/* Back up, wrap if needed */
		if (i-- == 0) i = MESSAGE_MAX - 1;

		/* Stop before oldest message */
		if (i == message__last) break;

		/* Index */
		o = message__ptr[i];

		/* Extract "distance" from "head" */
		q = (message__head + MESSAGE_BUF - o) % MESSAGE_BUF;

		/* Do not optimize over large distances */
		if (q >= MESSAGE_BUF / 4) continue;

		/* Get the old string */
		old = &message__buf[o];

		/* Continue if not equal */
		if (!streq(str, old)) continue;

		/* Get the next available message index */
		x = message__next;

		/* Advance 'message__next', wrap if needed */
		if (++message__next == MESSAGE_MAX) message__next = 0;

		/* Kill last message if needed */
		if (message__next == message__last)
		{
			/* Advance 'message__last', wrap if needed */
			if (++message__last == MESSAGE_MAX) message__last = 0;
		}

		/* Assign the starting address */
		message__ptr[x] = message__ptr[i];

		/* Store the message type */
		message__type[x] = type;

		/* Store the message count */
		message__count[x] = 1;

		/* Redraw */
		p_ptr->window |= (PW_MESSAGE | PW_MESSAGE_CHAT);

		/* Success */
		return;
	}

	/*** Step 4 -- Ensure space before end of buffer ***/

	/* Kill messages, and wrap, if needed */
	if (message__head + (n + 1) >= MESSAGE_BUF)
	{
		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Get offset */
			o = message__ptr[i];

			/* Kill "dead" messages */
			if (o >= message__head)
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}

		/* Wrap "tail" if needed */
		if (message__tail >= message__head) message__tail = 0;

		/* Start over */
		message__head = 0;
	}


	/*** Step 5 -- Ensure space for actual characters ***/

	/* Kill messages, if needed */
	if (message__head + (n + 1) > message__tail)
	{
		/* Advance to new "tail" location */
		message__tail += (MESSAGE_BUF / 4);

		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Get offset */
			o = message__ptr[i];

			/* Kill "dead" messages */
			if ((o >= message__head) && (o < message__tail))
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}
	}


	/*** Step 6 -- Grab a new message index ***/

	/* Get the next available message index */
	x = message__next;

	/* Advance 'message__next', wrap if needed */
	if (++message__next == MESSAGE_MAX) message__next = 0;

	/* Kill last message if needed */
	if (message__next == message__last)
	{
		/* Advance 'message__last', wrap if needed */
		if (++message__last == MESSAGE_MAX) message__last = 0;
	}


	/*** Step 7 -- Insert the message text ***/

	/* Assign the starting address */
	message__ptr[x] = message__head;

	/* Inline 'strcpy(message__buf + message__head, str)' */
	v = message__buf + message__head;
	for (u = str; *u; ) *v++ = *u++;
	*v = '\0';

	/* Advance the "head" pointer */
	message__head += ((u16b)n + 1); /* should work, n (strlen) is unlikely to be larger than u16b */

	/* Store the message type */
	message__type[x] = type;

	/* Store the message count */
	message__count[x] = 1;
	

	/* Window stuff */
	p_ptr->window |= PW_MESSAGE | PW_MESSAGE_CHAT;
}

/* Hack: Instead of deleting the message, it's being converted to GENERIC
 * TODO: Remove it completly */
void c_message_del(s16b age)
{
	/* static char buf[1024]; */
	s16b x;
	u16b o;

	/* Forgotten messages have no text */
	if ((age < 0) || (age >= message_num())) return;

	/* Get the "logical" index */
	x = message_age2idx(age);

	/* Get the "offset" for the message */
	o = message__ptr[x];

	/* Hide */
	message__type[x] = MSG_GENERIC;

	/* Return */
	return;
}


/*
 * Hack -- flush
 *
 * Not needed --KLJ--
 */
#if 0
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	/* Get an acceptable keypress */
	while (1)
	{
		int cmd = inkey();
		if (quick_messages) break;
		if ((cmd == ESCAPE) || (cmd == ' ')) break;
		if ((cmd == '\n') || (cmd == '\r')) break;
		bell();
	}

	/* Clear the line */
	Term_erase(0, 0, 255);

	/* The top line is OK */
	topline_icky = FALSE;

	Flush_queue();
}
#endif


/*
 * Output a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do "Term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to
 * "erase" any "pending" messages still on the screen.
 *
 * XXX XXX XXX Note that we must be very careful about using the
 * "msg_print()" functions without explicitly calling the special
 * "msg_print(NULL)" function, since this may result in the loss
 * of information if the screen is cleared, or if anything is
 * displayed on the top line.
 *
 * XXX XXX XXX Note that "msg_print(NULL)" will clear the top line
 * even if no messages are pending.  This is probably a hack.
 */
void c_msg_print(cptr msg)
{
	c_msg_print_aux(msg, MSG_LOCAL);
}
void c_msg_print_aux(cptr msg, u16b type)
{
	static int p = 0;

	int n;
#if 0
	char *t;
#endif
	char buf[1024];


	/* Hack -- Reset */
	if (!msg_flag) p = 0;

	/* Keldon-Hack -- Always reset */
	p = 0;
	prt("", 0, 0);

	/* Message length */
	n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (p && (!msg || ((p + n) > 72)))
	{
		/* Flush */
		/*msg_flush(p);*/

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		p = 0;
	}


	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;


	/* Memorize the message */
	c_message_add(msg, type);


	/* Copy it */
	my_strcpy(buf, msg, sizeof(buf));
	
	/* Strip it */
	buf[80] = '\0';
	
	/* Display it */
	Term_putstr(0, 0, 80, TERM_WHITE, buf);
#if 0
	/* Analyze the buffer */
	t = buf;

	/* Split message */
	while (n > 72)
	{
		char oops;

		int check, split;

		/* Default split */
		split = 72;

		/* Find the "best" split point */
		for (check = 40; check < 72; check++)
		{
			/* Found a valid split point */
			if (t[check] == ' ') split = check;
		}

		/* Save the split character */
		oops = t[split];

		/* Split the message */
		t[split] = '\0';

		/* Display part of the message */
		Term_putstr(0, 0, split, TERM_WHITE, t);

		/* Flush it */
		/*msg_flush(split + 1);*/

		/* Restore the split character */
		t[split] = oops;

		/* Insert a space */
		t[--split] = ' ';

		/* Prepare to recurse on the rest of "buf" */
		t += split; n -= split;
	}


	/* Display the tail of the message */
	Term_putstr(p, 0, n, TERM_WHITE, t);
#endif
	/* Remember the message */
	msg_flag = TRUE;

	/* Remember the position */
	p += n + 1;
}


/*
 * Request a "quantity" from the user
 *
 * Hack -- allow "command_arg" to specify a quantity
 */
s32b c_get_quantity(cptr prompt, s32b max)
{
	int amt;

	char tmp[80];

	char buf[80];


	/* Build a prompt if needed */
	if (!prompt)
	{
		/* Build a prompt */
		sprintf(tmp, "Quantity (1-%" PRId32 "): ", max);

		/* Use that prompt */
		prompt = tmp;
	}


	/* Default to one */
	amt = 1;

	/* Build the default */
	sprintf(buf, "%d", amt);

	/* Ask for a quantity */
	if (!get_string(prompt, buf, 8)) return (0);

	/* Extract a number */
	amt = atoi(buf);

	/* A letter means "all" */
	if (isalpha(buf[0])) amt = max;

	/* Enforce the maximum, if maximum is defined */
	if ((max >= 0) && (amt > max)) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	/* Return the result */
	return (amt);
}


/*
 * Create a new path by appending a file (or directory) to a path
 *
 * This requires no special processing on simple machines, except
 * for verifying the size of the filename, but note the ability to
 * bypass the given "path" with certain special file-names.
 *
 * Note that the "file" may actually be a "sub-path", including
 * a path and a file.
 *
 * Note that this function yields a path which must be "parsed"
 * using the "parse" function above.
 */
errr path_build(char *buf, int max, cptr path, cptr file)
{
        /* Special file */
        if (file[0] == '~')
        {
                /* Use the file itself */
                strnfmt(buf, max, "%s", file);
        }

        /* Absolute file, on "normal" systems */
        else if (prefix(file, PATH_SEP) && !streq(PATH_SEP, ""))
        {
                /* Use the file itself */
                strnfmt(buf, max, "%s", file);
        }

        /* No path given */
        else if (!path[0])
        {
                /* Use the file itself */
                strnfmt(buf, max, "%s", file);
        }

        /* Path and File */
        else
        {
                /* Build the new path */
                strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
        }

        /* Success */
        return (0);
}

void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}

int caveclr(cave_view_type* dest, int len)
{
	int i;
	/* Erase a character n times */
	for (i = 0; i < len; i++)
	{
		dest[i].a = 0x80;
		dest[i].c = 0x80;
	}
	return 1;
}

int cavecpy(cave_view_type* dest, cave_view_type *src, int len)
{
	int i;
	/* Copy a line with length "len" */
	for (i = 0; i < len; i++)
	{
		dest[i].a = src[i].a;
		dest[i].c = src[i].c;
	}
	return 1;
}

int cavemem(cave_view_type* src, int len, s16b x, s16b y)
{
	int i;
	s16b dx = x + DUNGEON_OFFSET_X;
	s16b dy = y + DUNGEON_OFFSET_Y;
	
	/* Draw a character n times */
	for (i = 0; i < len; i++)
	{
		byte ta = p_ptr->trn_info[y][x + i].a;
		char tc = p_ptr->trn_info[y][x + i].c;
		Term_mem_ch(i + dx, dy, src[i].a, src[i].c, ta, tc);
	}
	return 1;
}

int cavedraw(cave_view_type* src, int len, s16b x, s16b y)
{
	int i;
	s16b dx = x + DUNGEON_OFFSET_X;
	s16b dy = y + DUNGEON_OFFSET_Y;

	/* Paranoia - bounds */
	if (dx < 0) return -1;
	if (dy < 0 || dy >= Term->hgt) return -1;

	/* Draw a character n times */
	for (i = 0; i < len; i++)
	{
		/* Paranoia - bounds */
		if (dx + i >= Term->wid) return -1;

		/* Don't draw on screen if character is 0 */
		if (src[i].c)
		{
			byte ta = p_ptr->trn_info[y][x + i].a;
			char tc = p_ptr->trn_info[y][x + i].c;
			Term_queue_char(Term, i + dx, dy, src[i].a, src[i].c, ta, tc);
		}
	}
	return 1;
}

int caveprt(cave_view_type* src, int len, s16b x, s16b y)
{
	int i;

	/* Paranoia - bounds */
	if (x < 0 || x >= Term->wid) return -1;
	if (y < 0 || y >= Term->hgt) return -1;

	/* Draw a character n times */
	for (i = 0; i < len; i++)
	{
		/* Don't draw on screen if character is 0 */
		if (src[i].c)
		{
			Term_draw(i + x, y, src[i].a, src[i].c);
		}
	}
	return 1;
}

int cavestr(cave_view_type* dest, cptr str, byte attr, int max_col)
{
	int i, e;
	e = strlen(str);
	for (i = 0; i < e; i++)
	{
		dest[i].a = attr;
		dest[i].c = str[i];
	}
	for (i = e; i < max_col; i++)
	{
		dest[i].a = TERM_WHITE;
		dest[i].c = ' ';
	}
	return 1;
}

/* Draw (or don't) a char depending on screen ickyness */
void show_char(s16b y, s16b x, byte a, char c, byte ta, char tc, bool mem)
{
	bool draw = TRUE;

	/* Manipulate offset: */
	x += DUNGEON_OFFSET_X;	
	y += DUNGEON_OFFSET_Y;

	/* Test ickyness */
	if (screen_icky || section_icky_row || shopping) draw = FALSE;
	if (section_icky_row)
	{
		if (y >= section_icky_row) draw = TRUE;
		else if (section_icky_col > 0 && x >= section_icky_col) draw = TRUE;
		else if (section_icky_col < 0 && x < Term->wid + section_icky_col) draw = TRUE;
	}

	/* Test terminal size */
	if (x >= Term->wid || y >= Term->hgt) mem = draw = FALSE;

	/* TODO: also test for ->mem stack */
	if (mem && Term->mem)
		Term_mem_ch(x, y, a, c, ta, tc);

	if (draw)
	{
		/* Update secondary layer */
		if (p_ptr->trn_info[y][x].a != ta || p_ptr->trn_info[y][x].c != tc) 
		{
			/* Hack -- force refresh of that grid no matter what */
			Term->scr->a[y][x] = 0;
			Term->scr->c[y][x] = 0;
			Term->old->a[y][x] = 0;
			Term->old->c[y][x] = 0;
		}

		Term_queue_char(Term, x, y, a, c, ta, tc);
	}

}

/* Show (or don't) a line depending on screen ickyness */
void show_line(int sy, s16b cols, bool mem, int st)
{
	s16b xoff, coff;
	bool draw;
	int y;

	draw = mem ? !screen_icky : interactive_mode;
	xoff = coff = 0;
	y = sy;

	/* Ugly Hack - Shopping */
	if (shopping) draw = FALSE;

	/* Hang on! Icky section! */
	if (section_icky_row && y < section_icky_row)
	{
		if (section_icky_col > 0) xoff = section_icky_col - DUNGEON_OFFSET_X;
		if (section_icky_col < 0) coff = section_icky_col;
		if (xoff >= cols || cols-coff <= 0) draw = FALSE;
	}

	/* Another possible issue - terminal is too small */
	if (cols+coff >= Term->wid) coff -= (Term->wid - (cols+coff));
	if (y >= Term->hgt || cols+coff <= 0) mem = draw = FALSE;

	/* Check the max line count */
	if (last_line_info < y)
		last_line_info = y;

	/* Remember screen */
	if (mem && Term->mem)
		cavemem(stream_cave(st, sy), cols, 0, sy);

	/* Put data to screen */
	if (draw)
		cavedraw(stream_cave(st, sy)+xoff, cols+coff, xoff, sy);
}

/*
 * Handle the air layer
 */
/*
 * Each "air tile" has a delay and a fadeout value, stored in 
 *  air_delay[][] and air_fade[][] arrays. The unit is milliseconds.
 *
 * Both "fade" and "delay" values constantly go down. As soon as "delay"
 * reaches zero, the tile appears on-screen, and should stay there,
 * until the "fade" value also reaches zero.  
 *
 * It's up for the client to select the fadeout threshold. 
 *  A large value (i.e. 100) would draw air tiles with large tracers,
 *  as tiles will "hang" in the air for quite some time. A small
 *  value (i.e. 1 or 2) will make them much more flickerish.
 *
 * Feel free to play around with this (AIR_FADE_THRESHOLD define, see also
 * "recv_air()" in net-client.c)
 *
 * Note: when "air tiles" are used to display projectiles which actually land
 * and then appear on the ground (i.e. arrows, thrown items), the final
 * tile is drawn BEFORE any of the air tiles have a chance to appear/fade.
 *
 * Note: *In theory* main-xxx ports might feel that they can handle the air
 * layer by themselves and more gracefully (e.g. with alpha blendend fades).
 * If you're working on this, and the direct Term_draw writes are too
 * intrusive for you, set global variable "air_refresh" to FALSE.
 * To disable this code completely, set "air_updates" to FALSE.
 *
 * Note: this function uses up the static_timer(2). This means you can't use
 * static_timer(2) anywhere else from now on.
 *
 * Note: this function probably breaks graphics mode in some way. Untested.
 */
void update_air()
{
	micro passed = static_timer(2);
	u16b milli = (u16b)(passed / 1000);
	int j, i;
	for (j = 0; j < MAX_HGT; j++)
	{
		for (i = 0; i < MAX_WID; i++)
		{
			if (air_delay[j][i] > 0)
			{
				air_delay[j][i] -= milli;
				/* Delay timeout */
				if (air_delay[j][i] <= 0)
				{
					air_delay[j][i] = 0;
					if (air_refresh)
					{
						/* Draw air tile */
						show_char(j, i,
							air_info[j][i].a, air_info[j][i].c,
							p_ptr->trn_info[j][i].a,
							p_ptr->trn_info[j][i].c, FALSE);
					}
				}
			}
			if (air_fade[j][i] > 0)
			{
				air_fade[j][i] -= milli;
				/* Fade timeout */
				if (air_fade[j][i] <= 0)
				{
					air_fade[j][i] = 0;
					if (air_refresh)
					{
						/* Erase air tile */
						cave_view_type *scr_info = stream_cave(0, j);
						show_char(j, i,
							scr_info[i].a, scr_info[i].c,
							p_ptr->trn_info[j][i].a,
							p_ptr->trn_info[j][i].c, FALSE);
					}
				}
			}
		}
	}
}

void prt_num(cptr header, int num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str(header, row, col);
	put_str("   ", row, col + len);
	(void)sprintf(out_val, "%6ld", (long)num);
	c_put_str(color, out_val, row, col + len + 3);
}

void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str(header, row, col);
	(void)sprintf(out_val, "%9ld", (long)num);
	c_put_str(color, out_val, row, col + len);
}

/*
 * Transform macro trigger key code ('^_O_64\r' or etc..) 
 * into macro trigger name ('\[alt-D]' etc..)
 */
static size_t trigger_ascii_to_text(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	char key_code[100];
	int i;
	cptr tmp;
	size_t current_len = strlen(buf);
	

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Trigger name will be written as '\[name]' */
	strnfcat(buf, max, &current_len, "\\[");

	/* Use template to read key-code style trigger */
	for (i = 0; macro_template[i]; i++)
	{
		int j;
		char ch = macro_template[i];

		switch(ch)
		{
		case '&':
			/* Read modifier */
			while ((tmp = strchr(macro_modifier_chr, *str)))
			{
				j = (int)(tmp - macro_modifier_chr);
				strnfcat(buf, max, &current_len, "%s", macro_modifier_name[j]);
				str++;
			}
			break;
		case '#':
			/* Read key code */
			for (j = 0; *str && (*str != '\r') && (j < sizeof(key_code) - 1); j++)
				key_code[j] = *str++;
			key_code[j] = '\0';
			break;
		default:
			/* Skip fixed strings */
			if (ch != *str) return 0;
			str++;
		}
	}

	/* Key code style triggers always end with '\r' */
	if (*str++ != '\r') return 0;

	/* Look for trigger name with given keycode (normal or shifted keycode) */
	for (i = 0; i < max_macrotrigger; i++)
	{
		if (!my_stricmp(key_code, macro_trigger_keycode[0][i]) ||
		    !my_stricmp(key_code, macro_trigger_keycode[1][i]))
			break;
	}

	/* Not found? */
	if (i == max_macrotrigger) return 0;

	/* Write trigger name + "]" */
	strnfcat(buf, max, &current_len, "%s]", macro_trigger_name[i]);

	/* Succeed */
	*strptr = str;
	return current_len;
}


void ascii_to_text(char *buf, size_t len, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		/* Macro Trigger */
		else if (i == 31)
		{
			size_t offset;

			/* Terminate before appending the trigger */
			*s = '\0';

			offset = trigger_ascii_to_text(buf, len, &str);
			
			if (offset == 0)
			{
				/* No trigger found */
				*s++ = '^';
				*s++ = '_';
			}
			else
				s += offset;
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = i + 64;
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else if (i < 64)
		{
			*s++ = '\\';
			*s++ = '0';
			*s++ = octify(i / 8);
			*s++ = octify(i % 8);
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify(i / 16);
			*s++ = hexify(i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}

static errr macro_dump(cptr fname)
{
	int i;

	FILE *fff;

	char buf[1024];

	
	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* Write to the file */
	fff = my_fopen(buf, "w");

	/* Failure */
	if (!fff) return (-1);


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic macro dump\n\n");

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

		/* Extract the action */
		ascii_to_text(buf, sizeof(buf), macro__act[i]);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the action */
		ascii_to_text(buf, sizeof(buf), macro__pat[i]);

		/* Dump command macro */
		if (macro__cmd[i]) fprintf(fff, "C:%s\n", buf);

		/* Dump normal macros */
		else fprintf(fff, "P:%s\n", buf);

		/* End the macro */
		fprintf(fff, "\n\n");
	}

	/* Finish dumping */
	fprintf(fff, "\n\n\n\n");

	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}

static bool get_macro_trigger(char *buf)
{
	int i, n = 0;

	char tmp[1024];

	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	i = inkey();
	
#ifdef USE_GCU
	/* If we allow escape as macro trigger (ncurses) */
	if (escape_in_macro_triggers)
	{
		/* Then, backtick acts as actual escape */
		if (i == '`') return FALSE;
	} else
#endif
	/* Escape on Escape */
	if (i == ESCAPE) return FALSE;

	/* Read the pattern */
	while (i)
	{
		/* Save the key */
		buf[n++] = i;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		i = inkey();
	}

	/* Terminate */
	buf[n] = '\0';

	/* Flush */
	flush();


	/* Convert the trigger */
	ascii_to_text(tmp, sizeof(tmp), buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);	
	
	return TRUE;
}

/*
 * Find the macro (if any) which exactly matches the given pattern
 */
int macro_find_exact(cptr pat)
{
	int i;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not match the pattern */
		if (!streq(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* No matches */
	return (-1);
}

/* Display macros as a list and allow user to navigate through it 
 * Logic in this function is somewhat broken.
 */
void browse_macros(void)
{
	int i;
	int total;
	int hgt = Term->hgt - 4;
	int j = 0;	
	int o = 0;
	int sel = -1;
	char tmp_buf[120];
	char buf[120];
	char act[120];
	char a = TERM_WHITE;

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 0, -1, TERM_WHITE, "Browse Macros     (D delete, A/T to set, ESC to accept)");

		/* Dump them */
		for (i = 0, total = 0; i < macro__num; i++)
		{
			int k = total; 

			/* Skip command macro */
			if (macro__cmd[i]) continue;
		
			/* Extract the action */
			ascii_to_text(act, sizeof(act), macro__act[i]);

			/* Most likely a system action */			
			if (strlen(act) == 1) continue;

			/* Extract the trigger */
			ascii_to_text(buf, sizeof(buf), macro__pat[i]);

			/* Deleted macro */
			if (!strcmp(buf, act)) continue;

			/* It's ok */
			total++;

			/* Too early */
			if (k < o) continue;

			/* Too late */			
			if (k - o >= hgt-2) continue;

			/* Selected */
			a = TERM_WHITE;
			if (j == k) 
			{
				a = TERM_L_BLUE;
				sel = i;
			}

			/* Dump the trigger */
			Term_putstr(00, 2+k-o, -1, a, buf);

			/* Dump the action */
			Term_putstr(30, 2+k-o, -1, a, act);
		}

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		else if (i == 'D') /* Delete */
		{
			/* Keep atleast 1 */
			if (total == 1) continue;		

			/* Get a macro trigger */
			my_strcpy(buf, macro__pat[sel], sizeof(buf));

			/* (un)Link the macro */
			macro_add(buf, buf, FALSE);

			/* Change offsets */
			if (j >= total-1) j--;
			if (j < 0) j = 0;
			else if (o && j - o < hgt/2) o--;
		}

		else if (i == 'T') /* Change trigger */
		{
			/* Get current action */
			my_strcpy(act, macro__act[sel], sizeof(act));

			/* Prompt */	
			clear_from(hgt);
			Term_putstr(0, hgt+1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);
			text_to_ascii(tmp_buf, buf);

			/* Same */
			if (!strcmp(macro__pat[sel], tmp_buf)) continue;

			/* (re)Link the macro */
			macro_add(tmp_buf, act, FALSE);
		}

		else if (i == 'A') /* Change action */
		{
			/* Prompt */	
			clear_from(hgt);
			Term_putstr(0, hgt+1, -1, TERM_WHITE, "Action: ");

			/* Copy 'current action' */
			ascii_to_text(act, sizeof(act), macro__act[sel]);			

			/* Get an encoded action */
			if (!askfor_aux(act, 80, 0)) continue;

			/* Convert to ascii */
			text_to_ascii(tmp_buf, act);
			tmp_buf[strlen(act)] = '\0';

			/* Do not allow empty OR short */
			if (strlen(tmp_buf) <= 1) continue;

			/* (re)Link the macro */
			macro_add(macro__pat[sel], tmp_buf, FALSE);
		}

		else if (i == '2') /* Down */
		{
			j++;
			if (j > total-1) j = total-1;
			else if (j - o > hgt/2 && j < total) o++;
		}
		else if (i == '7') /* Home */
		{
			o = j = 0;
		}
		else if (i == '9') /* Page up */
		{
			j -= hgt;
			if (j < 0) j = 0;
			o = j;
		}
		else if (i == '3') /* Page down */
		{
			j += Term->hgt;
			if (j > total-1) j = total-1;
			o = j - hgt/2;
		}
		else if (i == '1') /* End */ 
		{
			j = total - 1;
			o = j - hgt/2;
		}
		else if (i == '8') /* Up */
		{
			j--;
			if (j < 0) j = 0;
			else if (o && j - o < hgt/2) o--;
		}
	}
}
void interact_macros(void)
{
	int i;

	char tmp[160], buf[1024], tmp_buf[160];
	char* str;
	tmp_buf[0] = '\0';

	/* Screen is icky */
	screen_icky = TRUE;

	/* Save screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Interact with Macros");


		/* Describe the trigger */
		if (!STRZERO(tmp_buf))
		{
			Term_putstr(0, 17, -1, TERM_WHITE, "Current trigger: ");
			Term_addstr(-1, TERM_WHITE, tmp_buf);
		}

		/* Describe that action */
		Term_putstr(0, 19, -1, TERM_WHITE, "Current action (if any) shown below:");

		/* Analyze the current action */
		ascii_to_text(buf, sizeof(buf), macro__buf);

		/* Display the current action */
		Term_putstr(0, 21, -1, TERM_WHITE, buf);


		/* Selections */
		Term_putstr(5,  4, -1, TERM_WHITE, "(1) Load macros");
		Term_putstr(5,  5, -1, TERM_WHITE, "(2) Save macros");
		Term_putstr(5,  6, -1, TERM_WHITE, "(3) Enter a new action");
		Term_putstr(5,  7, -1, TERM_WHITE, "(4) Query key for macro");
		Term_putstr(5,  8, -1, TERM_WHITE, "(5) Create a normal macro");
		Term_putstr(5,  9, -1, TERM_WHITE, "(6) Remove a macro");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Browse macros");
#if 0
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Create an empty macro");
		Term_putstr(5, 10, -1, TERM_WHITE, "(8) Create a command macro");
		Term_putstr(5, 12, -1, TERM_WHITE, "(X) Turn off an option (by name)");
		Term_putstr(5, 13, -1, TERM_WHITE, "(Y) Turn on an option (by name)");
#endif

		/* Prompt */
		Term_putstr(0, 15, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Browse */
		else if (i == '7') browse_macros();

		/* Load a pref file */
		else if (i == '1')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Load a user pref file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 17, -1, TERM_WHITE, "File: ");

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);
		}

		/* Save a 'macro' file */
		else if (i == '2')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Save a macro file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 17, -1, TERM_WHITE, "File: ");

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;
			
			/* Lowercase the filename */
			for(str=tmp;*str;str++) *str=tolower(*str);

			/* Dump the macros */
			(void)macro_dump(tmp);
		}

		/* Enter a new action */
		else if (i == '3')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Enter a new action");

			/* Go to the correct location */
			Term_gotoxy(0, 21);

			/* Hack -- limit the value */
			tmp[MAX_COLS] = '\0';

			/* Get an encoded action */
			if (!askfor_aux(buf, MAX_COLS, 0)) continue;

			/* Extract an action */
			text_to_ascii(macro__buf, buf);
		}

		/* Query key */
		else if (i == '4')
		{
			int k;
			
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Query key for macro");

			/* Prompt */
			Term_erase(0, 17, 255);			
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");
			

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Get the action */
			k = macro_find_exact(buf);
			
			
			/* Nothing found */
			if (k < 0)
			{
				/* Prompt */
				c_msg_print("Found no macro.");
			}

			/* It's an identity macro (empty) */
			else if (streq(buf, macro__act[k]))
			{
				/* Prompt */
				c_msg_print("Found no macro.");
			}
			
			/* Found one */
			else 
			{
				/* Obtain the action */
				my_strcpy(macro__buf, macro__act[k], strlen(macro__buf)+1);

				/* Analyze the current action */
				ascii_to_text(tmp, sizeof(tmp), macro__buf);

				/* Display the current action */
				prt(tmp, 21, 0);

				/* Prompt */
				c_msg_print("Found a macro.");
				
				/* Save key for delayed prompt */
				ascii_to_text(tmp_buf, sizeof(tmp), buf);
			}
		}

		/* Create a normal macro */
		else if (i == '5' || i == '%')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create a normal macro");

			/* Prompt */
			Term_erase(0, 17, 255);
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			if (!get_macro_trigger(buf)) continue;

			/* Interactive mode */
			if (i == '%')
			{
				/* Clear */
				clear_from(20);
	
				/* Prompt */
				Term_putstr(0, 15, -1, TERM_WHITE, "Command: Enter a new action   ");
	
				/* Go to the correct location */
				Term_gotoxy(0, 21);
	
				/* Copy 'current action' */
				ascii_to_text(tmp, sizeof(tmp), macro__buf);			
	
				/* Get an encoded action */
				if (!askfor_aux(tmp, MAX_COLS, 0)) continue;
	
				/* Convert to ascii */
				text_to_ascii(macro__buf, tmp);
			}

			/* Save key for later */
			ascii_to_text(tmp_buf, sizeof(tmp), buf);

			/* Link the macro */
			macro_add(buf, macro__buf, FALSE);

			/* Message */
			c_msg_print("Created a new normal macro.");
		}

		/* Remove a macro */
		else if (i == '6')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Remove a macro");

			/* Prompt */
			Term_erase(0, 17, 255);
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, buf, FALSE);

			/* Message */
			c_msg_print("Removed a macro.");
		}
#if 0
		/* Create an empty macro */
		else if (i == '7')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create an empty macro");

			/* Prompt */
			Term_erase(0, 17, 255);
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, "", FALSE);

			/* Message */
			c_msg_print("Created a new empty macro.");
		}

		/* Create a command macro */
		else if (i == '8')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create a command macro");

			/* Prompt */
			Term_erase(0, 17, 255);
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, macro__buf, TRUE);

			/* Message */
			c_msg_print("Created a new command macro.");
		}
#endif

		/* Oops */
		else
		{
			/* Oops */
			bell();
		}
	}

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	/* Flush the queue */
	Flush_queue();
}


/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, bool local, cptr info)
{
	char	ch;

	int		i, k = 0, n = 0;

	int		opt[24];

	char	buf[80];


	/* Lookup the options */
	for (i = 0; i < 24; i++) opt[i] = 0;

	/* Scan the options */
	if (local)
	{
		/* Local */
		for (i = 0; local_option_info[i].o_desc; i++)
		{
			/* Notice options on this "page" */
			if (local_option_info[i].o_page == page) opt[n++] = i;
		}
	} 
	else 
	{
		/* Server */
		for (i = 0; i < options_max; i++)
		{
			/* Notice options on this "page" */
			if (option_info[i].o_page == page) opt[n++] = i;
		}
	}

	/* Paranoia - zero options */
	if (!n) return;

	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		bool set_must = FALSE;
		bool set_what;
		int  set_id;

		/* Prompt XXX XXX XXX */
		sprintf(buf, "%-30s (RET to advance, y/n to set, ESC to accept) ", info);
		prt(buf, 0, 0);
	
		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			if (local)
			{
				sprintf(buf, "%-48s: %s  (%s)",
				        local_option_info[opt[i]].o_desc,
				        (*local_option_info[opt[i]].o_var ? "yes" : "no "),
				        local_option_info[opt[i]].o_text);
			}
			else
			{
				sprintf(buf, "%-48s: %s  (%s)",
				        option_info[opt[i]].o_desc,
				        (p_ptr->options[opt[i]] ? "yes" : "no "),
				        option_info[opt[i]].o_text);
			}
			c_prt(a, buf, i + 2, 0);
		}

		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/* Analyze */
		switch (ch)
		{
			case ESCAPE:
			{
				return;
			}

			case '-':
			case '8':
			{
				k = (n + k - 1) % n;
				break;
			}

			case ' ':
			case '\n':
			case '\r':
			case '2':
			{
				k = (k + 1) % n;
				break;
			}

			case 'y':
			case 'Y':
			case '6':
			{
				set_must = TRUE;
				set_what = TRUE;
				set_id = k;
				k = (k + 1) % n;
				break;
			}

			case 'n':
			case 'N':
			case '4':
			{
				set_must = TRUE;
				set_what = FALSE;
				set_id = k;
				k = (k + 1) % n;
				break;
			}

			default:
			{
				bell();
				break;
			}
		}
		/* Set option */
		if (set_must)
		{
			int on_opt = -1;
			int on_var = -1;
			if (local)
			{
				if (local_option_info[opt[set_id]].o_set)
				{
					on_opt = local_option_info[opt[set_id]].o_set;
				}
				on_var = opt[set_id];					
			}
			else
			{
				if (option_info[opt[set_id]].o_set)
				{
					on_var = option_info[opt[set_id]].o_set;
				}
				on_opt = opt[set_id]; 
			}
			if (on_opt != -1)
				p_ptr->options[on_opt] = set_what;
			if (on_var != -1)
				(*local_option_info[on_var].o_var) = set_what;
		}
	}
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
	int i, j, d;

	int y = 0;
	int x = 0;

	char ch;

	bool go = TRUE;

	u32b old_flag[8];


	/* Memorize old flags */
	for (j = 0; j < 8; j++)
	{
		/* Acquire current flags */
		old_flag[j] = window_flag[j];
	}


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (go)
	{
		byte st = 0;

		/* Prompt XXX XXX XXX */
		prt("Window flags (<dir>, t, y, n, ESC) ", 0, 0);

		/* Display the windows */
		for (j = 0; j < 8; j++)
		{
			byte a = TERM_WHITE;

			cptr s = ang_term_name[j];

			/* Use color */
			if (use_color && (j == x)) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < 16; i++)
		{
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (use_color && (i == y)) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < 8; j++)
			{
				byte a = TERM_WHITE;

				char c = '.';

				/* Use color */
				if (use_color && (i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (window_flag[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ch = inkey();

		/* Analyze */
		switch (ch)
		{
			case ESCAPE:
			{
				go = FALSE;
				break;
			}

			case 'T':
			case 't':
			{
#ifdef PMSG_TERM
				/* Ignore any change on PMSG_TERM */
				if (x == PMSG_TERM) break;
#endif
				/* Clear windows */
				for (j = 0; j < 8; j++)
				{
					/* Ignore screen (but not for Status AND Compact)*/
					if ((j == 0) && ((1L << y) != PW_STATUS) && ((1L << y) != PW_PLAYER_2)) break;

					window_flag[j] &= ~(1L << y);
				}

				/* Clear flags */
				for (i = 0; i < 16; i++)
				{
					/* Ignore screen (but not for Status AND Compact)*/
					if ((x == 0) && ((1L << i) != PW_STATUS) && ((1L << i) != PW_PLAYER_2)) break;

					window_flag[x] &= ~(1L << i);
				}

				/* Fall through */
			}

			case 'y':
			case 'Y':
			{
#ifdef PMSG_TERM
				/* Ignore any change on PMSG_TERM */
				if (x == PMSG_TERM) break;
#endif
				/* Ignore screen (but not for Status AND Compact)*/
				if ((x == 0) && ((1L << y) != PW_STATUS) && ((1L << y) != PW_PLAYER_2)) break;

				/* Set flag */
				window_flag[x] |= (1L << y);
				break;
			}

			case 'n':
			case 'N':
			{
#ifdef PMSG_TERM
				/* Ignore any change on PMSG_TERM */
				if (x == PMSG_TERM) break;
#endif
				/* Ignore screen (but not for Status AND Compact)*/
				if ((x == 0) && ((1L << y) != PW_STATUS) && ((1L << y) != PW_PLAYER_2)) break;

				/* Clear flag */
				window_flag[x] &= ~(1L << y);
				break;
			}

			default:
			{
				d = target_dir(ch);

				x = (x + ddx[d] + 8) % 8;
				y = (y + ddy[d] + 16) % 16;

				if (!d) bell();
			}
		}
	}

	/* Hack -- Store user choice */
	for (j = 0; j < 8; j++)
	{
		window_flag_o[j] = window_flag[j];
	}

	/* Notice changes */
	p_ptr->window |= net_term_manage(old_flag, window_flag, TRUE);

	/* Update windows */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_MESSAGE | PW_MESSAGE_CHAT | PW_PLAYER | PW_PLAYER_1 | PW_STATUS);

	/* Update windows */
	window_stuff();
}




/*
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void)
{
	int k;
	int col, i, label;

	int old_hitpoint_warn = Client_setup.settings[3];
	int old_window_flags = Client_setup.settings[4];

	/* Enter "icky" mode */
	screen_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Why are we here */
		prt("MAngband options", 2, 0);

		/* Prepare */
		i = label = 0;
		col = 4;
		
		/* Give some choices */
		for (k = 0; local_option_group[k]; k++)
		{
			prt(format("(%d) %s", ++i, local_option_group[k]), col++, 5);
		}
		label = k+1;
		for (k = 0; k < options_groups_max; k++)
		{
			prt(format("(%d) %s", ++i, option_group[k]), col++, 5);
		}

		/* Window flags */
		col += 1;
		prt("(W) Window flags", col++, 5);

		/* Load and Append
		col += 1;
		prt("(L) Load a user pref file", col++, 5);
		prt("(A) Append options to a file", col++, 5); */

		/* Special choices */
		col += 1;
		/* prt("(D) Base Delay Factor", col++, 5); */
		prt("(H) Hitpoint Warning", col++, 5);



		/* Prompt */
		col += 2;
		prt("Command: ", col, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE) break;

		/* Entered some group */
		if (isdigit(k))
		{
			i = D2I(k);
			/* Unknown digit ? */
			if (i >= options_groups_max + label) 
			{
				/* Oops */
				bell();
				continue;
			}
			/* Local */
			if (i < label)
			{
				do_cmd_options_aux(i, TRUE, local_option_group[i - 1]);
			}
			/* Server */
			else
			{
				do_cmd_options_aux(i - label + 1, FALSE, option_group[i - label]);
			}
		}

		/* Window flags */
		else if (k == 'W')
		{
			/* Spawn */
			do_cmd_options_win();
		}

		/* Hack -- hitpoint warning factor */
		else if ((k == 'H') || (k == 'h'))
		{
			/* Prompt */
			prt("Command: Hitpoint Warning", col, 0);

			/* Get a new value */
			while (1)
			{
				char cx;
				prt(format("Current hitpoint warning: %2d%%",
				           p_ptr->hitpoint_warn * 10), col+2, 0);
				prt("New hitpoint warning (0-9 or ESC to accept): ", col+1, 0);

				cx = inkey();
				if (cx == ESCAPE) break;
				if (isdigit((unsigned char)cx)) p_ptr->hitpoint_warn = D2I(cx);
				else bell();/*"Illegal hitpoint warning!");*/
			}
		}

		/* Unknown option */
		else
		{
			/* Oops */
			bell();
		}
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	screen_icky = FALSE;
	Flush_queue();

	/* HACK -- hitpoint warning changed */
	gather_settings();
	if (old_hitpoint_warn != Client_setup.settings[3] ||
		old_window_flags != Client_setup.settings[4]) 
	send_settings();
	
	/* Resend options to server */
	send_options();

	/* Save options to file */
	Save_options();

	/* Send a redraw request */
	send_redraw();
}
void do_cmd_options_birth()
{
	/* Save the screen */
	Term_save();

	/* Hack -- asume Group 1 (index0) as "Birth Options" */
	do_cmd_options_aux(1, FALSE, option_group[0]);

	/* Hack -- make those ones a priority above ones read from pref file */
	ignore_birth_options = TRUE;
	
	/* Restore the screen */
	Term_load();
}



#ifdef SET_UID

# ifndef HAVE_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
int usleep(huge microSeconds)
{
	struct timeval		Timer;

	int			nfds = 0;

#ifdef FD_SET
	fd_set		*no_fds = NULL;
#else
	int			*no_fds = NULL;
#endif

	/* Was: int readfds, writefds, exceptfds; */
	/* Was: readfds = writefds = exceptfds = 0; */


	/* Paranoia -- No excessive sleeping */
	if (microSeconds > 4000000L) core("Illegal usleep() call");


	/* Wait for it */
	Timer.tv_sec = (microSeconds / 1000000L);
	Timer.tv_usec = (microSeconds % 1000000L);

	/* Wait for it */
	if (select(nfds, no_fds, no_fds, no_fds, &Timer) < 0)
	{
		/* Hack -- ignore interrupts */
		if (errno != EINTR) return -1;
	}

	/* Success */
	return 0;
}

# endif /* HAVE_USLEEP */

#endif /* SET_UID */

#ifdef WIN32
int usleep(huge microSeconds)
{ /* meassured in milliseconds not microseconds*/
	DWORD milliseconds = (DWORD)(microSeconds / 1000);
	Sleep(milliseconds);
	return 0;
}
#endif /* WIN32 */


/* HACK -- Count samples in "val" sound */
int sound_count(int val)
{
	int i;

	/* No sound */
	if (!use_sound) return (0);

	/* Illegal sound */
	if ((val < 0) || (val >= MSG_MAX)) return (0);


	/* Count the samples */
	for (i = 0; i < SAMPLE_MAX; i++)
	{
		if (!sound_file[val][i])
			break;
	}

	/* Return number of samples (might be 0) */
	return (i);
}

#ifdef USE_SOUND

/*
 * XXX XXX XXX - Taken from files.c.
 *
 * Extract "tokens" from a buffer
 *
 * This function uses "whitespace" as delimiters, and treats any amount of
 * whitespace as a single delimiter.  We will never return any empty tokens.
 * When given an empty buffer, or a buffer containing only "whitespace", we
 * will return no tokens.  We will never extract more than "num" tokens.
 *
 * By running a token through the "text_to_ascii()" function, you can allow
 * that token to include (encoded) whitespace, using "\s" to encode spaces.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 */
static s16b tokenize_whitespace(char *buf, s16b num, char **tokens)
{
	int k = 0;
	char *s = buf;

	/* Process */
	while (k < num)
	{
		char *t;

		/* Skip leading whitespace */
		for ( ; *s && isspace((unsigned char)*s); ++s) /* loop */;

		/* All done */
		if (!*s) break;

		/* Find next whitespace, if any */
		for (t = s; *t && !isspace((unsigned char)*t); ++t) /* loop */;

		/* Nuke and advance (if necessary) */
		if (*t) *t++ = '\0';

		/* Save the token */
		tokens[k++] = s;

		/* Advance */
		s = t;
	}

	/* Count */
	return (k);
}



void load_sound_prefs(void)
{
	int i, j, num;
	char tmp[MSG_LEN];
	char ini_path[MSG_LEN];
	char wav_path[MSG_LEN];
	char *zz[SAMPLE_MAX];

	/* Access the sound.cfg */
	path_build(ini_path, sizeof(ini_path), ANGBAND_DIR_XTRA_SOUND, "sound.cfg");

	/* Add it to 'global config' */
	conf_append_section("Sound", ini_path);

	for (i = 0; i < MSG_MAX; i++)
	{
		/* Ignore empty sound strings */
		if (!angband_sound_name[i][0]) continue;

		my_strcpy(tmp, conf_get_string("Sound", angband_sound_name[i], ""), sizeof(tmp));

		num = tokenize_whitespace(tmp, SAMPLE_MAX, zz);

		for (j = 0; j < num; j++)
		{
			/* Access the sound */
			path_build(wav_path, sizeof(wav_path), ANGBAND_DIR_XTRA_SOUND, zz[j]);

			/* Save the sound filename, if it exists */
			if (my_fexists(wav_path))
				sound_file[i][j] = string_make(zz[j]);
		}
	}
}

#endif /* USE_SOUND */
