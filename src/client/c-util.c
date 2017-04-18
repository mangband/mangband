#include "angband.h"

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

	cptr	pat, act;

	char	buf[1024];

	int net_fd;

	/* Acquire and save maximum file descriptor */
	net_fd = Net_fd();

	/* If no network yet, just wait for a keypress */
	if (net_fd == -1)
	{
		/* Look for a keypress */
		(void)(Term_inkey(&ch, TRUE, TRUE));
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
		/* Update the screen */
		Term_fresh();
		/* Redraw windows if necessary */
		if (p_ptr->window)
		{
			window_stuff();
		}


		/* Wait for keypress, while also checking for net input */
		do
		{
			int result;

			/* Look for a keypress */
			(void)(Term_inkey(&ch, FALSE, TRUE));

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

				/* Update the screen */
				Term_fresh();

				/* Redraw windows if necessary */
				if (p_ptr->window)
				{
					window_stuff();
				}
			}
		} while (!ch);
	}


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
		if (0 == Term_inkey(&ch, FALSE, TRUE))
		{
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
		(void)Term_inkey(&ch, TRUE, TRUE);

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
	int v;

	char kk, ch;

	bool done = FALSE;

	term *old = Term;

	int w = 0;

	int skipping = FALSE;


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
	(void)Term_get_cursor(&v);

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
		if (!inkey_base && inkey_scan && (0 != Term_inkey(&ch, FALSE, FALSE))) break;


		/* Hack -- flush the output once no key is ready */
		if (!done && (0 != Term_inkey(&ch, FALSE, FALSE)))
		{
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
			char xh;

			/* Check for keypress, optional wait */
			(void)Term_inkey(&xh, !inkey_scan, TRUE);

			/* Key ready */
			if (xh)
			{
				/* Reset delay */
				w = 0;

				/* Mega-Hack */
				if (xh == 28)
				{
					/* Toggle "skipping" */
					skipping = !skipping;
				}

				/* Use normal keys */
				else if (!skipping)
				{
					/* Use it */
					ch = xh;
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
	Term_set_cursor(v);


	/* Cancel the various "global parameters" */
	inkey_base = inkey_scan = inkey_flag = FALSE;


	/* Return the keypress */
	return (ch);
}

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



/*
 * React to new value of "rogue_like_commands".
 *
 * Initialize the "keymap" arrays based on the current value of
 * "rogue_like_commands".  Note that all "undefined" keypresses
 * by default map to themselves with no direction.  This allows
 * "standard" commands to use the same keys in both keysets.
 *
 * To reset the keymap, simply set "rogue_like_commands" to -1,
 * call this function, restore its value, call this function.
 *
 * The keymap arrays map keys to "command_cmd" and "command_dir".
 *
 * It is illegal for keymap_cmds[N] to be zero, except for
 * keymaps_cmds[0], which is unused.
 *
 * You can map a key to "tab" to make it "non-functional".
 */
void keymap_init(void)
{
	int i, k;

	/* Initialize every entry */
	for (i = 0; i < 128; i++)
	{
		/* Default to "no direction" */
		hack_dir = 0;

		/* Attempt to translate */
		if (rogue_like_commands)
		{
			k = roguelike_commands(i);
		}
		else
		{
			k = original_commands(i);
		}

		/* Save the keypress */
		keymap_cmds[i] = k;

		/* Save the direction */
		keymap_dirs[i] = hack_dir;
	}
}


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

void prt_format(int row, int col, cptr format, ...)
{
	char buffer[160];
	va_list ap;

	va_start(ap, format);

	vstrnfmt(buffer, sizeof buffer, format, ap);

	va_end(ap);

	prt(buffer, row, col);
}


/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 */

/* APD -- added private so passwords will not be displayed. */
bool askfor_aux(char *buf, int len, char private)
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
	Term_putstr(x, y, -1, TERM_YELLOW, buf);


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
				Term_putch(x+k-1, y, TERM_WHITE, private ? 'x' : buf[k-1]);
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

		/* Access the array info */
		command_cmd = keymap_cmds[cmd & 0x7F];
		command_dir = keymap_dirs[cmd & 0x7F];
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

	else dir = keymap_dirs[command & 0x7F];

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

void c_put_fmt(byte attr, int row, int col, cptr fmt, ...)
{
	char buffer[160];
	va_list ap;

	va_start(ap, fmt);

	vstrnfmt(buffer, sizeof buffer, fmt, ap);

	va_end(ap);

	c_put_str(attr, buffer, row, col);
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
		if (quick_messages) break;
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
 * Recall the "text" of a saved message
 */
cptr message_str(s16b age)
{
        s16b x;
        s16b o;
        cptr s;

        /* Forgotten messages have no text */
        if ((age < 0) || (age >= message_num())) return ("");

        /* Acquire the "logical" index */
        x = (message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

        /* Get the "offset" for the message */
        o = message__ptr[x];

        /* Access the message text */
        s = &message__buf[o];

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
 * Add a new message, with great efficiency
 */
void c_message_add(cptr str)
{
        int i, k, x, n;


        /*** Step 1 -- Analyze the message ***/

        /* Hack -- Ignore "non-messages" */
        if (!str) return;

        /* Message length */
        n = strlen(str);

        /* Important Hack -- Ignore "long" messages */
        if (n >= MESSAGE_BUF / 4) return;


        /*** Step 2 -- Attempt to optimize ***/

        /* Limit number of messages to check */
        k = message_num() / 4;

        /* Limit number of messages to check */
        if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

        /* Check the last few messages (if any to count) */
        for (i = message__next; k; k--)
        {
                u16b q;

                cptr old;

                /* Back up and wrap if needed */
                if (i-- == 0) i = MESSAGE_MAX - 1;

                /* Stop before oldest message */
                if (i == message__last) break;

                /* Extract "distance" from "head" */
                q = (message__head + MESSAGE_BUF - message__ptr[i]) % MESSAGE_BUF;

                /* Do not optimize over large distance */
                if (q > MESSAGE_BUF / 2) continue;

                /* Access the old string */
                old = &message__buf[message__ptr[i]];

                /* Compare */
                if (!streq(old, str)) continue;

                /* Get the next message index, advance */
                x = message__next++;

                /* Handle wrap */
                if (message__next == MESSAGE_MAX) message__next = 0;

                /* Kill last message if needed */
                if (message__next == message__last) message__last++;

                /* Handle wrap */
                if (message__last == MESSAGE_MAX) message__last = 0;

                /* Assign the starting address */
                message__ptr[x] = message__ptr[i];

		/* Redraw */
		p_ptr->window |= (PW_MESSAGE);

                /* Success */
                return;
        }


        /*** Step 3 -- Ensure space before end of buffer ***/

        /* Kill messages and Wrap if needed */
        if (message__head + n + 1 >= MESSAGE_BUF)
        {
                /* Kill all "dead" messages */
                for (i = message__last; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next) break;

                        /* Kill "dead" messages */
                        if (message__ptr[i] >= message__head)
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


        /*** Step 4 -- Ensure space before next message ***/

        /* Kill messages if needed */
        if (message__head + n + 1 > message__tail)
        {
                /* Grab new "tail" */
                message__tail = message__head + n + 1;

                /* Advance tail while possible past first "nul" */
                while (message__buf[message__tail-1]) message__tail++;

                /* Kill all "dead" messages */
                for (i = message__last; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next) break;

                        /* Kill "dead" messages */
                        if ((message__ptr[i] >= message__head) &&
                            (message__ptr[i] < message__tail))
                        {
                                /* Track oldest message */
                                message__last = i + 1;
                        }
                }
        }


        /*** Step 5 -- Grab a new message index ***/

        /* Get the next message index, advance */
        x = message__next++;

        /* Handle wrap */
        if (message__next == MESSAGE_MAX) message__next = 0;

        /* Kill last message if needed */
        if (message__next == message__last) message__last++;

        /* Handle wrap */
        if (message__last == MESSAGE_MAX) message__last = 0;



        /*** Step 6 -- Insert the message text ***/

        /* Assign the starting address */
        message__ptr[x] = message__head;

        /* Append the new part of the message */
        for (i = 0; i < n; i++)
        {
                /* Copy the message */
                message__buf[message__head + i] = str[i];
        }

        /* Terminate */
        message__buf[message__head + i] = '\0';

        /* Advance the "head" pointer */
        message__head += n + 1;

	/* Window stuff */
	p_ptr->window |= PW_MESSAGE;
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
	static int p = 0;

	int n;

	char *t;

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
	c_message_add(msg);


	/* Copy it */
	strcpy(buf, msg);

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
		sprintf(tmp, "Quantity (1-%d): ", max);

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


void ascii_to_text(char *buf, cptr str)
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
		ascii_to_text(buf, macro__act[i]);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the action */
		ascii_to_text(buf, macro__pat[i]);

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

static void get_macro_trigger(char *buf)
{
	int i, n = 0;

	char tmp[1024];

	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	i = inkey();

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
	ascii_to_text(tmp, buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);
}

void interact_macros(void)
{
	int i;

	char tmp[160], buf[1024];
	char* str;

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


		/* Describe that action */
		Term_putstr(0, 19, -1, TERM_WHITE, "Current action (if any) shown below:");

		/* Analyze the current action */
		ascii_to_text(buf, macro__buf);

		/* Display the current action */
		Term_putstr(0, 21, -1, TERM_WHITE, buf);


		/* Selections */
		Term_putstr(5,  4, -1, TERM_WHITE, "(1) Load macros");
		Term_putstr(5,  5, -1, TERM_WHITE, "(2) Save macros");
		Term_putstr(5,  6, -1, TERM_WHITE, "(3) Enter a new action");
		Term_putstr(5,  7, -1, TERM_WHITE, "(4) Create a command macro");
		Term_putstr(5,  8, -1, TERM_WHITE, "(5) Create a normal macro");
		Term_putstr(5,  9, -1, TERM_WHITE, "(6) Create an identity macro");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Create an empty macro");
#if 0
		Term_putstr(5, 12, -1, TERM_WHITE, "(X) Turn off an option (by name)");
		Term_putstr(5, 13, -1, TERM_WHITE, "(Y) Turn on an option (by name)");
#endif

		/* Prompt */
		Term_putstr(0, 15, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

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
			tmp[80] = '\0';

			/* Get an encoded action */
			if (!askfor_aux(buf, 80, 0)) continue;

			/* Extract an action */
			text_to_ascii(macro__buf, buf);
		}

		/* Create a command macro */
		else if (i == '4')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create a command macro");

			/* Prompt */
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, macro__buf, TRUE);

			/* Message */
			c_msg_print("Created a new command macro.");
		}

		/* Create a normal macro */
		else if (i == '5')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create a normal macro");

			/* Prompt */
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, macro__buf, FALSE);

			/* Message */
			c_msg_print("Created a new normal macro.");
		}

		/* Create an identity macro */
		else if (i == '6')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create an identity macro");

			/* Prompt */
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, buf, FALSE);

			/* Message */
			c_msg_print("Created a new identity macro.");
		}

		/* Create an empty macro */
		else if (i == '7')
		{
			/* Prompt */
			Term_putstr(0, 15, -1, TERM_WHITE, "Command: Create an empty macro");

			/* Prompt */
			Term_putstr(0, 17, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, "", FALSE);

			/* Message */
			c_msg_print("Created a new empty macro.");
		}

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
static void do_cmd_options_aux(int page, cptr info)
{
	char	ch;

	int		i, k = 0, n = 0;

	int		opt[24];

	char	buf[80];


	/* Lookup the options */
	for (i = 0; i < 24; i++) opt[i] = 0;

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		/* Notice options on this "page" */
		if (option_info[i].o_page == page) opt[n++] = i;
	}


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		/* Prompt XXX XXX XXX */
		sprintf(buf, "%s (RET to advance, y/n to set, ESC to accept) ", info);
		prt(buf, 0, 0);
	
		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			sprintf(buf, "%-48s: %s  (%s)",
			        option_info[opt[i]].o_desc,
			        (*option_info[opt[i]].o_var ? "yes" : "no "),
			        option_info[opt[i]].o_text);
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
				(*option_info[opt[k]].o_var) = TRUE;
				Client_setup.options[opt[k]] = TRUE;
				k = (k + 1) % n;
				break;
			}

			case 'n':
			case 'N':
			case '4':
			{
				(*option_info[opt[k]].o_var) = FALSE;
				Client_setup.options[opt[k]] = FALSE;
				k = (k + 1) % n;
				break;
			}

			default:
			{
				bell();
				break;
			}
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
				/* Clear windows */
				for (j = 0; j < 8; j++)
				{
					window_flag[j] &= ~(1L << y);
				}

				/* Clear flags */
				for (i = 0; i < 16; i++)
				{
					window_flag[x] &= ~(1L << i);
				}

				/* Fall through */
			}

			case 'y':
			case 'Y':
			{
				/* Ignore screen */
				if (x == 0) break;

				/* Set flag */
				window_flag[x] |= (1L << y);
				break;
			}

			case 'n':
			case 'N':
			{
				/* Clear flag */
				window_flag[x] &= ~(1L << y);
				break;
			}

			default:
			{
				d = keymap_dirs[ch & 0x7F];

				x = (x + ddx[d] + 8) % 8;
				y = (y + ddy[d] + 16) % 16;

				if (!d) bell();
			}
		}
	}

	/* Notice changes */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!ang_term[j]) continue;

		/* Ignore non-changes */
		if (window_flag[j] == old_flag[j]) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Erase */
		Term_clear();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}

	/* Update windows */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_MESSAGE | PW_PLAYER);

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

		/* Give some choices */
		prt("(1) User Interface Options", 4, 5);
		prt("(2) Disturbance Options", 5, 5);
		prt("(3) Game-Play Options", 6, 5);
		prt("(4) Efficiency Options", 7, 5);

		/* Window flags */
		prt("(W) Window flags", 9, 5);

		/* Prompt */
		prt("Command: ", 11, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE) break;

		/* General Options */
		if (k == '1')
		{
			/* Process the general options */
			do_cmd_options_aux(1, "User Interface Options");
		}

		/* Disturbance Options */
		else if (k == '2')
		{
			/* Process the running options */
			do_cmd_options_aux(2, "Disturbance Options");
		}

		/* Inventory Options */
		else if (k == '3')
		{
			/* Process the running options */
			do_cmd_options_aux(3, "Game-Play Options");
		}

		/* Efficiency Options */
		else if (k == '4')
		{
			/* Process the efficiency options */
			do_cmd_options_aux(4, "Efficiency Options");
		}

		/* Window flags */
		else if (k == 'W')
		{
			/* Spawn */
			do_cmd_options_win();
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


	/* Verify the keymap */
	keymap_init();

	/* Resend options to server */
	Send_options();

	/* Resend options to server */
	Save_options();

	/* Send a redraw request */
	Send_redraw();
}




#ifdef SET_UID

# ifndef HAS_USLEEP

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

# endif /* HAS_USLEEP */

#endif /* SET_UID */

#ifdef WIN32
int usleep(long microSeconds)
{
	Sleep(microSeconds/10); /* meassured in milliseconds not microseconds*/
	return 0;
}
#endif /* WIN32 */
