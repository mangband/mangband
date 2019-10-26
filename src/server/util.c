/* File: util.c */

/* Purpose: Angband utilities -BEN- */


#include "mangband.h"







#ifdef SET_UID



/*
 * Hack -- External functions
 */
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();


#endif



/*
 * The concept of the "file" routines below (and elsewhere) is that all
 * file handling should be done using as few routines as possible, since
 * every machine is slightly different, but these routines always have the
 * same semantics.
 *
 * In fact, perhaps we should use the "path_parse()" routine below to convert
 * from "canonical" filenames (optional leading tilde's, internal wildcards,
 * slash as the path seperator, etc) to "system" filenames (no special symbols,
 * system-specific path seperator, etc).  This would allow the program itself
 * to assume that all filenames are "Unix" filenames, and explicitly "extract"
 * such filenames if needed (by "path_parse()", or perhaps "path_canon()").
 *
 * Note that "path_temp" should probably return a "canonical" filename.
 *
 * Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
 * and "my_move()" and "my_copy()" should all take "canonical" filenames.
 *
 * Note that "canonical" filenames use a leading "slash" to indicate an absolute
 * path, and a leading "tilde" to indicate a special directory, and default to a
 * relative path, but MSDOS uses a leading "drivename plus colon" to indicate the
 * use of a "special drive", and then the rest of the path is parsed "normally",
 * and MACINTOSH uses a leading colon to indicate a relative path, and an embedded
 * colon to indicate a "drive plus absolute path", and finally defaults to a file
 * in the current working directory, which may or may not be defined.
 *
 * We should probably parse a leading "~~/" as referring to "ANGBAND_DIR". (?)
 */


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 */


#else /* ACORN */



#endif

/*
 * Acquire a "temporary" file name if possible
 *
 * This filename is always in "system-specific" form.
 */
errr path_temp(char *buf, int max)
{
#ifdef WIN32
	char prefix[] = "mng";
	if(!GetTempPath(max,buf)) return(-1);
	if(!GetTempFileName(buf,prefix,0,buf)) return(-1);
#else
	int p;
	strcpy(buf,"/tmp/mangXXXXXX");
	if((p = mkstemp(buf)) < 0)
	{
		return(-1);
	}
	fclose(fdopen(p, "r"));
#endif

	/* Success */
	return (0);
}



/*
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "TERM_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, TERM_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * The following info is from "Torbjorn Lindgren" (see "main-xaw.c").
 *
 * These values are NOT gamma-corrected.  On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7.  The converted value for each of the five
 * different "quarter" values is given below:
 *
 *  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
 *  -----       ----            ----            ----          ---
 *   0/4        0.00            0.00            0.00          #00
 *   1/4        0.25            0.27            0.28          #47
 *   2/4        0.50            0.55            0.56          #8f
 *   3/4        0.75            0.82            0.84          #d7
 *   4/4        1.00            1.00            1.00          #ff
 *
 * Note that some machines (i.e. most IBM machines) are limited to a
 * hard-coded set of colors, and so the information above is useless.
 *
 * Also, some machines are limited to a pre-determined set of colors,
 * for example, the IBM can only display 16 colors, and only 14 of
 * those colors resemble colors used by Angband, and then only when
 * you ignore the fact that "Slate" and "cyan" are not really matches,
 * so on the IBM, we use "orange" for both "Umber", and "Light Umber"
 * in addition to the obvious "Orange", since by combining all of the
 * "indeterminate" colors into a single color, the rest of the colors
 * are left with "meaningful" values.
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

/*
 * Convert a color to it's opposite 
 */
int color_opposite(int color) {
	switch (color) {
		case TERM_L_BLUE: 	return TERM_BLUE;
		case TERM_L_GREEN: 	return TERM_GREEN;
		case TERM_L_RED: 		return TERM_RED;
		case TERM_L_WHITE: 	return TERM_SLATE;
		case TERM_UMBER: 		return TERM_L_UMBER;
		case TERM_ORANGE: 	return TERM_YELLOW;
		case TERM_DARK: 		return TERM_L_DARK;
		case TERM_VIOLET:		return ( randint1(100) < 50 ? TERM_BLUE : TERM_RED );
		/* and vice versa */
		case TERM_BLUE: 		return TERM_L_BLUE;
		case TERM_GREEN: 		return TERM_L_GREEN;
		case TERM_RED: 		return TERM_L_RED;
		case TERM_SLATE: 		return TERM_L_WHITE;
		case TERM_L_UMBER: 	return TERM_UMBER;
		case TERM_YELLOW: 	return TERM_ORANGE;
		case TERM_L_DARK: 	return TERM_DARK;
	}
	return color;
}


/*
 * Move the cursor
 */
#if 0
void move_cursor(int row, int col)
{
	Term_gotoxy(col, row);
}
#endif



/*
 * Convert a decimal to a single digit octal number
 */
static char octify(uint i)
{
	return (hexsym[i%8]);
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(uint i)
{
	return (hexsym[i%16]);
}


/*
 * Convert a octal-digit into a decimal
 */
static int deoct(char c)
{
	if (isdigit(c)) return (D2I(c));
	return (0);
}

/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
	if (isdigit(c)) return (D2I(c));
	if (islower(c)) return (A2I(c) + 10);
	if (isupper(c)) return (A2I(tolower(c)) + 10);
	return (0);
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * I have no clue if this function correctly handles, for example,
 * parsing "\xFF" into a (signed) char.  Whoever thought of making
 * the "sign" of a "char" undefined is a complete moron.  Oh well.
 */
void text_to_ascii(char *buf, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		/* Backslash codes */
		if (*str == '\\')
		{
			/* Skip the backslash */
			str++;

			/* Hex-mode XXX */
			if (*str == 'x')
			{
				*s = 16 * dehex(*++str);
				*s++ += dehex(*++str);
			}

			/* Hack -- simple way to specify "backslash" */
			else if (*str == '\\')
			{
				*s++ = '\\';
			}

			/* Hack -- simple way to specify "caret" */
			else if (*str == '^')
			{
				*s++ = '^';
			}

			/* Hack -- simple way to specify "space" */
			else if (*str == 's')
			{
				*s++ = ' ';
			}

			/* Hack -- simple way to specify Escape */
			else if (*str == 'e')
			{
				*s++ = ESCAPE;
			}

			/* Backspace */
			else if (*str == 'b')
			{
				*s++ = '\b';
			}

			/* Newline */
			else if (*str == 'n')
			{
				*s++ = '\n';
			}

			/* Return */
			else if (*str == 'r')
			{
				*s++ = '\r';
			}

			/* Tab */
			else if (*str == 't')
			{
				*s++ = '\t';
			}

			/* Octal-mode */
			else if (*str == '0')
			{
				*s = 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '1')
			{
				*s = 64 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '2')
			{
				*s = 64 * 2 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '3')
			{
				*s = 64 * 3 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;
			*s++ = (*str++ & 037);
		}

		/* Normal chars */
		else
		{
			*s++ = *str++;
		}
	}

	/* Terminate */
	*s = '\0';
}


/*
 * Hack -- convert a string into a printable form
 */
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


#if 0
/*
 * Variable used by the functions below
 */
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

		/* Jam a door (Spike) */
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
	switch (command)
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

	/* Notice changes in the "rogue_like_commands" flag */
	static char old_rogue_like = -1;

	/* Hack -- notice changes in "rogue_like_commands" */
	if (old_rogue_like == rogue_like_commands) return;

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

	/* Save the "rogue_like_commands" setting */
	old_rogue_like = rogue_like_commands;
}
#endif








/*
 * Legal bit-flags for macro__use[X]
 */
#define MACRO_USE_CMD	0x01	/* X triggers a command macro */
#define MACRO_USE_STD	0x02	/* X triggers a standard macro */

/*
 * Fast check for trigger of any macros
 */
static byte macro__use[256];



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
 * Check for possibly pending macros
 */
#if 0
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
#endif


/*
 * Find the longest completed macro
 */
#if 0
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
#endif



/*
 * Local "need flush" variable
 */
static bool flush_later = FALSE;


/*
 * Local variable -- we just finished a macro action
 */
/*static bool after_macro = FALSE;*/

/*
 * Local variable -- we are inside a macro action
 */
/*static bool parse_macro = FALSE;*/

/*
 * Local variable -- we are inside a "control-underscore" sequence
 */
/*static bool parse_under = FALSE;*/

/*
 * Local variable -- we are inside a "control-backslash" sequence
 */
/*static bool parse_slash = FALSE;*/

/*
 * Local variable -- we are stripping symbols for a while
 */
/*static bool strip_chars = FALSE;*/



/*
 * Flush all input chars.  Actually, remember the flush,
 * and do a "special flush" before the next "inkey()".
 *
 * This is not only more efficient, but also necessary to make sure
 * that various "inkey()" codes are not "lost" along the way.
 */
void flush(void)
{
	/* Do it later */
	flush_later = TRUE;
}


/*
 * Flush the screen, make a noise
 */
void bell(void)
{
#if 0
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
#endif
}


/*
 * Mega-Hack -- Make a (relevant?) sound
 */
void sound(player_type *p_ptr, int val)
{
	/* Make a sound */
	send_sound(p_ptr, val);
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
 */
#if 0
static char inkey_aux(void)
{
	int		k = 0, n, p = 0, w = 0;

	char	ch;

	cptr	pat, act;

	char	buf[1024];


	/* Wait for keypress */
	(void)(Term_inkey(&ch, TRUE, TRUE));


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
		/* Push the key, notice over-flow */
		if (Term_key_push(buf[--p])) return (0);
	}


	/* We are now inside a macro */
	parse_macro = TRUE;

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
#endif




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
#if 0
char inkey(int Ind)
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
	if (!inkey_scan && (!inkey_flag || hilite_player))
	{
		/* Show the cursor */
		(void)Term_set_cursor(1);
	}


	/* Hack -- Activate the screen */
	Term_activate(term_screen);


	/* Get a (non-zero) keypress */
	for (ch = 0; !ch; )
	{
		/* Nothing ready, not waiting, and not doing "inkey_base" */
		if (!inkey_base && inkey_scan && (0 != Term_inkey(&ch, FALSE, FALSE))) break;


		/* Hack -- flush output once when no key ready */
		if (!done && (0 != Term_inkey(&ch, FALSE, FALSE)))
		{
			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate the screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			character_saved = FALSE;

			/* Mega-Hack -- reset signal counter */
			signal_count = 0;

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
	inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;


	/* Return the keypress */
	return (ch);
}
#endif





/*
 * We use a global array for all inscriptions to reduce the memory
 * spent maintaining inscriptions.  Of course, it is still possible
 * to run out of inscription memory, especially if too many different
 * inscriptions are used, but hopefully this will be rare.
 *
 * We use dynamic string allocation because otherwise it is necessary
 * to pre-guess the amount of quark activity.  We limit the total
 * number of quarks, but this is much easier to "expand" as needed.
 *
 * Any two items with the same inscription will have the same "quark"
 * index, which should greatly reduce the need for inscription space.
 *
 * Note that "quark zero" is NULL and should not be "dereferenced".
 */

/*
 * Add a new "quark" to the set of quarks.
 */
s16b quark_add(cptr str)
{
	int i;

	/* Look for an existing quark */
	for (i = 1; i < quark__num; i++)
	{
		/* Check for equality */
		if (streq(quark__str[i], str)) return (i);
	}

	/* Paranoia -- Require room */
	if (quark__num == QUARK_MAX) return (0);

	/* New maximal quark */
	quark__num = i + 1;

	/* Add a new quark */
	quark__str[i] = string_make(str);

	/* Return the index */
	return (i);
}


/*
 * This function looks up a quark
 */
cptr quark_str(s16b i)
{
	cptr q;

	/* Verify */
	if ((i < 0) || (i >= quark__num)) i = 0;

	/* Access the quark */
	q = quark__str[i];

	/* Return the quark */
	return (q);
}


#define end_of_segment(A) ((A) == ' ' || (A) == '!' || (A) == '@' || (A) == '^')
/*
 * Parse item's inscriptons, extract "^abc" and "^a ^b ^c"
 * cases and cache them. (adapted from check_guard_inscription)
 */
void fill_prevent_inscription(bool *arr, s16b quark)
{
	const char *ax;

	/* Init quark */
	ax = quark_str(quark);
	if (ax == NULL) return;

	/* Find start of segment */
	while((ax = strchr(ax, '^')) != NULL) 
	{
		/* Parse segment */
		while(ax++ != NULL) 
		{
			/* Reached end of quark, stop */
			if (*ax == 0) break;

			/* Reached end of segment, stop */
			if (end_of_segment(*ax)) break;

			/* Found a "Preventing Inscription" */
			arr[MIN(127,(byte)(*ax))] = TRUE;
	    }
	}
}
/*
 * Refresh combined list of player's preventive inscriptons
 * after an update to his equipment was made. 
 */
void update_prevent_inscriptions(player_type *p_ptr)
{
	object_type *o_ptr;
	int i;

	/* Clear flags */
	for (i = 0; i < 128; i++)
	{
		p_ptr->prevents[i] = FALSE;
	}

	/* Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Item exists and has inscription */
		if (o_ptr->tval && o_ptr->note)
		{
			/* Fill */
			fill_prevent_inscription(p_ptr->prevents, o_ptr->note);
		}
	}
}

/*
 * Check to make sure they haven't inscribed an item against what
 * they are trying to do -Crimson
 * look for "!*Erm" type, and "!* !A !f" type.
 */

bool check_guard_inscription( s16b quark, char what ) {
    const char  *   ax;     
    ax=quark_str(quark);
    if( ax == NULL ) { return FALSE; };
    while( (ax=strchr(ax,'!')) != NULL ) {
	while( ax++ != NULL ) {
            if (*ax==0)  {
		 return FALSE; /* end of quark, stop */
	    }
            if (*ax==' ') {
		 break;	/* end of segment, stop */
	    }
            if (*ax==what) {
		return TRUE; /* exact match, stop */
	    }
	    if(*ax =='*') {
		switch( what ) { /* check for paraniod tags */
		    case '{': /* no inscribe */
		    case '}': /* no unscribe */
		    case 'g': case ',': /* no pickup! */
			/* ^ Owner must override those */
			/* Protect against loss: */
		    case 'd': /* no drop */
		    case 'k': /* no destroy */
#if 0
		    case 's': /* no sell */
#endif
		    case 'v': /* no thowing */
		    case 'f': /* no firing */
			/* Protect against consumption: */
		    case 'q': /* no quaff */
		    case 'E': /* no eat */
		    case 'r': /* no read */
		    case 'a': case 'z': case 'u': /* no magic devices */
		      return TRUE;
		};
            };  
        };  
    };  
    return FALSE;
};  




/*
 * Second try for the "message" handling routines.
 *
 * Each call to "message_add(s)" will add a new "most recent" message
 * to the "message recall list", using the contents of the string "s".
 *
 * The messages will be stored in such a way as to maximize "efficiency",
 * that is, we attempt to maximize the number of sequential messages that
 * can be retrieved, given a limited amount of storage space.
 *
 * We keep a buffer of chars to hold the "text" of the messages, not
 * necessarily in "order", and an array of offsets into that buffer,
 * representing the actual messages.  This is made more complicated
 * by the fact that both the array of indexes, and the buffer itself,
 * are both treated as "circular arrays" for efficiency purposes, but
 * the strings may not be "broken" across the ends of the array.
 *
 * The "message_add()" function is rather "complex", because it must be
 * extremely efficient, both in space and time, for use with the Borg.
 */



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
 * Add a new message, with great efficiency
 */
void message_add(cptr str)
{
#if 0
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
#endif
}



/*
 * Hack -- flush
 */
#if 0
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* Hack -- fake monochrome */
	if (!use_color) a = TERM_WHITE;

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
void msg_print(player_type *p_ptr, cptr msg)
{
	msg_print_aux(p_ptr, msg, MSG_GENERIC);
}
void msg_print_aux(player_type *p_ptr, cptr msg, u16b type)
{
	bool log = TRUE;
	bool add = FALSE;
	bool dup = FALSE;
	char multiplier[12];
	s16b ptr;
	
	/* We don't need to log *everything* */
	if(type > MSG_CHAT || (msg && strchr("[",*msg)))
	{
		log = FALSE;
	}

	/* Log messages for each player, so we can dump last messages
	 * in server-side character dumps */
	if(msg && p_ptr && log)
	{
		add = TRUE;
		/* Ensure we know where the last message is */
		ptr = p_ptr->msg_hist_ptr - 1;
		if(ptr < 0) ptr = MAX_MSG_HIST-1;
		/* If this message is already in the buffer, count it as a dupe */
		if(!strcmp(p_ptr->msg_log[ptr],msg))
		{
			p_ptr->msg_hist_dupe++;
			/* And don't add another copy to the buffer */
			add = FALSE;
			dup = TRUE;
		}
		/* This message is the end of a series of dupes */
		else if(p_ptr->msg_hist_dupe > 0)
		{
			/* Add the dupe counter to the end of the last message */
			sprintf(multiplier," (x%d)",p_ptr->msg_hist_dupe+1);
			strcat(p_ptr->msg_log[ptr],multiplier);
			p_ptr->msg_hist_dupe = 0;
		}
		if(add)
		{
			/* Standard, unique (for the moment) message */
			strncpy(p_ptr->msg_log[p_ptr->msg_hist_ptr], msg, 78);
			p_ptr->msg_log[p_ptr->msg_hist_ptr++][78] = '\0';
		}
		/* Maintain a circular buffer */
		if(p_ptr->msg_hist_ptr == MAX_MSG_HIST)
			p_ptr->msg_hist_ptr = 0;
		plog_fmt("%s: %s", p_ptr->name, msg);
	}
	else if(msg && log)
	{
		plog_fmt("%d: %s", 0, msg);
	}

	/* Hack -- repeated message of the same type */
	if (dup && type == p_ptr->msg_last_type)
	{
		send_message_repeat(p_ptr, type);
		return;
	}
	
	/* Remember last type sent */
	p_ptr->msg_last_type = type;
	
	/* Ahh, the beautiful simplicity of it.... --KLJ-- */
	send_message(p_ptr, msg, type);
}

void msg_broadcast(player_type *p_ptr, cptr msg)
{
	int i;

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Skip the specified player */
		if (same_player(Players[i], p_ptr)) continue;
		printf("Broadcasting: %s\n", msg);
		/* Tell this one */
		msg_print_aux(Players[i], msg, MSG_CHAT);
	}

	/* Send to console */
	console_print((char*)msg, 0);
}

void msg_channel(int chan, cptr msg)
{
	int i;
	/* Log to file */
	if (channels[chan].mode & CM_PLOG)
	{
		plog(msg);
	}
	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++)
	{
		if (Players[i]->on_channel[chan] & UCM_EAR)
			msg_print_aux(Players[i], msg, MSG_CHAT + chan);
	}
	/* And every console */
	console_print((char*)msg, chan);
}


/*
 * Display a formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(player_type *p_ptr, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print(p_ptr, buf);
}
/* Dirty hack */
void msg_format_type(player_type *p_ptr, u16b type, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_aux(p_ptr, buf, type);
}


/*
 * Display a message originated by a monster "m_idx".
 * Everyone nearby should see it.
 * If "q_ptr" is not NULL, this player will be skipped.
 *
 * Provide 2 messages, "fmt_vis" and "fmt_inv", one for players who can
 * see this monster, and one for players who can't (but are still nearby).
 * For example, "%%s picks %s's pocket!" and "%%s makes some grunting noises."
 * -> "a kobold picks Player's pocket", "something makes some grunting noises."
 *
 * "fmt_inv" can be NULL, in which case no message will be displayed for
 * players who don't see this monster (even though they are very close).
 *
 * Note, that the fomrat strings are expected to have "%%s" in them, which
 * will be substitued with monster's name (or "it" or "something").
 */
void msg_format_monster(int m_idx, player_type *q_ptr, u16b type, cptr fmt_vis, cptr fmt_inv, ...)
{
	va_list vp;

	int Depth, y, x, i;

	char m_name_vis[80];
	char m_name_invis[80];

	char buf[1024];
	char buf_vis[1024];
	char buf_invis[1024];

	monster_type *m_ptr = &m_list[m_idx];

	/* Obtain monster name(s) */
	monster_desc(NULL, m_name_vis,  m_idx, 0x80);  /* mode 0x80 - Forced visible */
	monster_desc(NULL, m_name_invis, m_idx, 0x40); /* mode 0x40 - Forced hidden */

	/* Prepare the "normal" message" (Varargs Stuff) */
	va_start(vp, fmt_inv);
	(void)vstrnfmt(buf, 1024, fmt_vis, vp);
	(void)strnfmt(buf_vis, 1024, buf, m_name_vis);
	va_end(vp);

	/* Is there a secondary message? */
	if (fmt_inv)
	{
		/* Prepare the "invisible" message" (Varargs Stuff) */
		va_start(vp, fmt_inv);
		(void)vstrnfmt(buf, 1024, fmt_inv, vp);
		(void)strnfmt(buf_invis, 1024, buf, m_name_invis, buf);
		va_end(vp);
	}

	/* Extract monster's location */
	Depth = m_ptr->dun_depth;
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this player */
		player_type *qq_ptr = Players[i];

		/* Don't send the message to the ignoree */
		if (same_player(qq_ptr, q_ptr)) continue;

		/* Make sure this player is at this depth */
		if (qq_ptr->dun_depth != Depth) continue;

		/* Is the player near? (we also check if monster considers him near)*/
		if (!player_has_los_bold(qq_ptr, y, x) &&
		    !(m_ptr->closest_player == qq_ptr->Ind)) continue;

		/* Can he see this monster? */
		if (qq_ptr->mon_vis[m_idx])
		{
			/* Send "normal" message */
			msg_print_aux(qq_ptr, buf_vis, type);
		}
		else if (fmt_inv)
		{
			/* Send "invisible" message */
			msg_print_aux(qq_ptr, buf_invis, type);
		}
	}
}


/*
 * Display a message to everyone who is on the same dungeon level.
 *
 * This serves two functions: a dungeon level-wide chat, and a way
 * to attract attention of other nearby players.
 */
void msg_format_complex_far(player_type *p_ptr, player_type *q_ptr, u16b type, cptr fmt, cptr sender, ...)
{
	va_list vp;

	int Depth, y, x, i;

	char buf[1024];
	char buf_vis[1024];
	char buf_invis[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, sender);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);
	(void)strnfmt(buf_vis, 1024, "%s %s", sender, buf);
	(void)strnfmt(buf_invis, 1024, "%s %s", "Someone", buf);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Extract player's location */
	Depth = p_ptr->dun_depth;
	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this player */
		player_type *qq_ptr = Players[i];

		/* Don't send the message to the player who caused it */
		if (same_player(qq_ptr, p_ptr)) continue;

		/* Don't send the message to the second ignoree */
		if (same_player(qq_ptr, q_ptr)) continue;

		/* Make sure this player is at this depth */
		if (qq_ptr->dun_depth != Depth) continue;

		/* Can he see this player? */
		if (qq_ptr->cave_flag[y][x] & CAVE_VIEW)
		{
			/* Send the message */
			msg_print_aux(qq_ptr, buf_vis, type);
			/* Disturb player */
			disturb(qq_ptr, 0, 0);
		}
		else
		{
			/* Send "invisible" message (e.g. "Someone yells") */
			msg_print_aux(qq_ptr, buf_invis, type);
		}
	}
}


/*
 * Display a message to everyone who is in sight of another player.
 *
 * This is mainly used to keep other players advised of actions done
 * by a player.  The message is not sent to the player who performed
 * the action.
 */
void msg_print_complex_near(player_type *p_ptr, player_type *q_ptr, u16b type, cptr msg)
{
	int Depth, y, x, i;

	/* Extract player's location */
	Depth = p_ptr->dun_depth;
	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this player */
		player_type *qq_ptr = Players[i];

		/* Don't send the message to the player who caused it */
		if (same_player(qq_ptr, p_ptr)) continue;

		/* Don't send the message to the second ignoree */
		if (same_player(qq_ptr, q_ptr)) continue;
		
		/* Make sure this player is at this depth */
		if (qq_ptr->dun_depth != Depth) continue;

		/* Can he see this player? */
		if (qq_ptr->cave_flag[y][x] & CAVE_VIEW)
		{
			/* Send the message */
			msg_print_aux(qq_ptr, msg, type);
		}
	}
}
void msg_print_near(player_type *p_ptr, cptr msg)
{
	msg_print_complex_near(p_ptr, p_ptr, MSG_GENERIC, msg);
}


/*
 * Same as above, except send a formatted message.
 */
void msg_format_complex_near(player_type *p_ptr, player_type *q_ptr, u16b type, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_complex_near(p_ptr, q_ptr, type, buf);
}
void msg_format_near(player_type *p_ptr, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_near(p_ptr, buf);
}


/* Analyze the 'search' string and determine if it has any special
 *  target.
 * Returns  0 - on error, and an error string is put into 'error' 
 * > 0 - player index
 * < 0 - party index
 */

#define VIRTUAL_CHANNELS 8
cptr virt_channels[VIRTUAL_CHANNELS] = { NULL, "&say", "&yell", NULL };
int find_chat_target(cptr search, char *error)
{
	int i, j, len, target = 0;
	cptr problem = "";
	player_type *q_ptr;
	bool party_trap = FALSE;
	bool channel_trap = FALSE;

	/* Acquire length of search string */
	len = strlen(search);
	
	/* Virtual channels ? */
	if (len && search[0] == '&')
	{
		channel_trap = TRUE;
		
		/* Find one */
		for (i = 1; i < VIRTUAL_CHANNELS; i++)
		{
			/* Done */
			if (!virt_channels[i]) break;
		
			/* Compare names */
			if (!my_strnicmp(virt_channels[i], search, len))
			{
					/* Set target if not set already or an exact match */
					if ((!target) || (len == strlen(virt_channels[i])))
					{
						target = i;
						problem = "";
					}
					else
					{
						/* Matching too many */
						/* Make sure we don't already have an exact match */
						if (len != strlen(parties[0 - target].name))
							problem = "channels";
					}
					break;
			}
		}
	}

	/* Look for a recipient who matches the search string */
	if (len && !channel_trap)
	{
		/* Check for party hinter */
		if (search[0] == '^')
		{
			party_trap = TRUE;
			search = search + 1;
		}	
	
		/* First check parties */
		for (i = 1; i < MAX_PARTIES; i++)
		{
			/* Skip if empty */
			if (!parties[i].num) continue;

			/* Check name */
			if (!my_strnicmp(parties[i].name, search, len))
			{
				/* Make sure one of the party members is actually
				 * logged on. */
				for (j = 1; j <= NumPlayers; j++)
				{
					if (Players[j]->conn == NOT_CONNECTED)
						continue;

					/* Check this guy */
					if (player_in_party(i, Players[j]))
					{
						/* Set target if not set already or an exact match */
						if ((!target) || (len == strlen(parties[i].name)))
						{
							target = 0 - i;
							problem = "";
						}
						else
						{
							/* Matching too many parties */
							/* Make sure we don't already have an exact match */
							if (len != strlen(parties[0 - target].name))
								problem = "parties";
						}
						break;
					}
				}
			}
		}

		/* Was hinting at party, Ignore players */
		if (!party_trap)		{
		/* Then check players */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Check this one */
			q_ptr = Players[i];
			
			/* Skip DM */
			if (q_ptr->dm_flags & DM_SECRET_PRESENCE) continue;

			/* Check name */
			if (!my_strnicmp(q_ptr->name, search, len))
			{
				/* Set target if not set already or an exact match */
				if ((!target) || (len == strlen(q_ptr->name)))
				{
					target = i;
					problem = "";
				}
				/* Matching too many people */
				else if (target > 0)
				{
					/* Make sure we don't already have an exact match */
					if (len != strlen(Players[target]->name))
						problem = "players";
				}
				else	problem = "players or parties";
			}
		}
		/* End party hinter */	}
	}

	/* Check for recipient set but no match found */
	if (len && !target)
	{
		/* Prepare an error message */
		sprintf(error, "Could not match name '%s'.", search); 

		/* Give up */
		return 0;
	}

	/* Check for multiple recipients found */
	if (!STRZERO(problem))
	{
		/* Send an error message */
		sprintf(error, "'%s' matches too many %s.", search, problem);
	
		/* Give up */
		return 0;
	}

	/* Hack -- pack player targets and virtual channels together */
	if (target > 0 && !channel_trap) target += VIRTUAL_CHANNELS;

	return target;
}

/* Instruct client to listen on a specific channel for an incoming message. */ 
void assist_whisper(player_type *p_ptr, cptr search)
{
	int target;
	char error[80];

	target = find_chat_target(search, error);

	/* No match */
	if (!target)
	{
		/* Relay error */
		msg_print(p_ptr, error);

		/* Give up */
		return;
	}

	/* All 'virtual channels' occupy the MAX_CHANNELS slot,
	 * while all real channels are < MAX_CHANNELS. */

	/* Virtual channel -- what he sent */
	else if (target > 0 && target < VIRTUAL_CHANNELS)
	{
		send_channel(p_ptr, CHAN_SELECT, MAX_CHANNELS, virt_channels[target]);
	}
	/* A Player */
	else if (target > 0)
	{
		send_channel(p_ptr, CHAN_SELECT, MAX_CHANNELS, Players[target - VIRTUAL_CHANNELS]->name);
	}
	/* A Party */
	else if (target < 0)
	{
		send_channel(p_ptr, CHAN_SELECT, MAX_CHANNELS, parties[0 - target].name);
	}
}

void channel_join(player_type *p_ptr, cptr channel, bool quiet)
{
	int i, last_free = 0;

	/* Find channel */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (!last_free && STRZERO(channels[i].name)) last_free = i;

		/* Name match */
		if (!strcmp(channels[i].name, channel))
		{
			/* Not present on this channel */
			if (!on_channel(p_ptr, i))
			{
				/* Hack -- can't join due to modes? */
				if (((channels[i].mode & CM_KEYLOCK) && !is_dm_p(p_ptr)) ||
					(p_ptr->on_channel[i] & UCM_BAN) ) 
				{
					/* Hack -- route to "unable to join" message */
					last_free = 0;
					break;
				}
				/* Enter channel */
				channels[i].num++;
				p_ptr->on_channel[i] |= UCM_EAR;
				send_channel(p_ptr, CHAN_JOIN, i, channel);
				if (!quiet) msg_format(p_ptr,"Listening to channel %s",channel);
			}
			/* Select channel */
			else
			{
				p_ptr->main_channel = i;
				send_channel(p_ptr, CHAN_SELECT, i, channel);
				if (!quiet) msg_format(p_ptr,"Channel changed to %s",channel);
			}
			return;
		}
	}

	/* No such channel */

	/* We have free space */
	if (last_free)
	{
		/* Create channel */
		my_strcpy(channels[last_free].name, channel, MAX_CHARS);
		channels[last_free].num = 1;
		p_ptr->on_channel[last_free] |= (UCM_EAR | UCM_OPER);
		send_channel(p_ptr, CHAN_JOIN, last_free, channel);
		if (!quiet) msg_format(p_ptr, "Listening to channel %s", channel);
	}
	/* All channel slots are used up */
	else
	{
		if (!quiet) msg_format(p_ptr, "Unable to join channel %s", channel);
	}
}
/* Actual code for leaving channels */
void channel_leave_id(player_type *p_ptr, int i, bool quiet)
{
	if (!i || !(p_ptr->on_channel[i] & UCM_EAR)) return;
	
	channels[i].num--;
	if (!quiet) msg_format(p_ptr, "Left channel %s", channels[i].name);
	if (channels[i].num <= 0 && !(channels[i].mode & CM_SERVICE))
	{
		channels[i].name[0] = '\0';
		channels[i].id = 0;
	}
	if (p_ptr->main_channel == i)
	{
		p_ptr->main_channel = 0;
	}
	p_ptr->on_channel[i] &= ~(UCM_LEAVE);
	if (!quiet)
		send_channel(p_ptr, CHAN_LEAVE, i, "");
}
/* Find channel by name and leave it */
void channel_leave(player_type *p_ptr, cptr channel)
{
	int i;
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (!strcmp(channels[i].name, channel))
		{
			channel_leave_id(p_ptr, i, FALSE);
			break;
		}
	}	
}
/* Leave all channels */
void channels_leave(player_type *p_ptr)
{
	int i;
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (p_ptr->on_channel[i] & UCM_EAR)
		{
			channel_leave_id(p_ptr, i, TRUE);
		}
	}
}


/*
 * A message prefixed by a player name is sent only to that player.
 * Otherwise, it is sent to everyone.
 */
void player_talk_aux(player_type *p_ptr, cptr message)
{
	int i, target = 0;
	char search[80], sender[80], error[80], tmp_chan[MAX_CHAN_LEN];
	int dest_chan = 0; //#public
	player_type *q_ptr;
	cptr colon, chan_prefix;
	bool msg_off = FALSE;

	/* Get sender's name */
	if (p_ptr)
	{
		/* Get player name */
		my_strcpy(sender, p_ptr->name, 80);
	}
	else
	{
		/* Default name */
		my_strcpy(sender, "", 80);
	}

	/* Default to no search string */
	strcpy(search, "");

	/* Default to #public channel if not originated by a player */
	dest_chan = 0;

	if (p_ptr)
	{
		/* Default to the senders main channel */
		dest_chan = p_ptr->main_channel;
		/* Set search string from senders secondary channel */
		strcpy(search, p_ptr->second_channel);
	}

	/* Is the message destined for a particular channel? */
	if(strchr("#", *message))
	{
		/* Yes, examine in more detail */
		chan_prefix = strchr(message,' ');
		if(!chan_prefix && strlen(message) < MAX_CHAN_LEN)
		{
			/* Channel name only?  Change the players default channel */
			if (p_ptr)
			{
				strncpy(tmp_chan,message,MAX_CHAN_LEN);
				channel_join(p_ptr, tmp_chan, FALSE);
				return;
			}
		}
		else if(!chan_prefix || chan_prefix-message >= MAX_CHAN_LEN)
		{
			/* Invalid channel prefix?  Forget about the channel. */
		}
		else
		{
			/* Channel name followed by text? Extract the channel name */
			strncpy(tmp_chan, message, chan_prefix - message);
			tmp_chan[chan_prefix - message] = '\0';
			dest_chan = -1;
			for (i = 0; i < MAX_CHANNELS; i++)
			{
				if (!strcmp(channels[i].name, tmp_chan))
				{
					dest_chan = i;
					break;
				}
			}
			message += (chan_prefix - message)+1;
			/* Forget about search string */
			msg_off = FALSE;
			strcpy(search, "");
		}
	}

	/* Look for a player's name followed by a colon */
	colon = strchr(message, ':');

	/* Pretend colon wasn't there */
	if (colon)
	{
		/* messanger is undefined OR colon is last symbol OR colon is part of "smiley" */
		if (!p_ptr || !*(colon + 1) || strchr(")(-|\\/", *(colon + 1))) colon = NULL;
	}

	/* Form a search string if we found a colon */
	if (colon)
	{
		/* Copy everything up to the colon to the search string */
		strncpy(search, message, colon - message);

		/* Add a trailing NULL */
		search[colon - message] = '\0';
		
		/* Move colon pointer forward to next word */
		while (*colon && (isspace(*colon) || *colon == ':')) colon++;
		
		/* Offset message */
		msg_off = TRUE;
	}

	/* Find special target */
	if (strlen(search))
	{
		/* There's nothing else , prepare for whisper */
		if (colon - message == strlen(message))
		{
			assist_whisper(p_ptr, search);
			return;
		}
		/* Hack -- empty 'party hinter' hints to own party */		
		if (search[0] == '^' && p_ptr->party && search[1] == '\0')
		{
			strcpy(search, parties[p_ptr->party].name);
		}
		if (!(target = find_chat_target(search, error)))
		{
			/* Error */
			msg_print(p_ptr, error);
			
			/* Done */
			return;
		}
	}
		
	/* No need to offset message */
	if (!msg_off)
	{
		colon = message;
	}

	
	/* Send to a virtual channel */
	if (target > 0)
	{
		/* Make sure it's a channel, not player */
		if (target < VIRTUAL_CHANNELS)
		{
			cptr verb = "say";
			char punct = '.';
			char msg[60];
			my_strcpy(msg, colon, 60);
			switch (target)
			{
				case 1: /* "&say" */
					for (i = strlen(msg) - 1; i > 0; i--)
					{
						switch (msg[i])
						{
							case ' ':
								continue;
							case '?':
								verb = "ask";
								/* fallthrough */
							case '!':
							case '.':
								punct = msg[i];
								msg[i] = '\0';
							default:
								break;
						}
						break;
					}
					/* Send somewhere */
					msg_format_type(p_ptr, MSG_TALK, "You %s, \"%s\"%c", verb, msg, punct);
					msg_format_complex_near(p_ptr, p_ptr, MSG_TALK, "%s %ss, \"%s\"%c", sender, verb, msg, punct);
				break;
				case 2: /* "&yell" */
					verb = "yell";
					punct = '!';
					for (i = strlen(msg) - 1; i > 0; i--)
					{
						switch (msg[i])
						{
							case ' ':
								continue;
							case '?':
							case '!':
							case '.':
								msg[i] = '\0';
							default:
								break;
						}
						break;
					}
					/* Send somewhere */
					msg_format_type(p_ptr, MSG_YELL, "You %s, \"%s\"%c", verb, msg, punct);
					msg_format_complex_far(p_ptr, p_ptr, MSG_YELL, "%ss, \"%s\"%c", sender, verb, msg, punct);
				break;
			}
			return;
		}
		/* It was a player */
		else target -= VIRTUAL_CHANNELS;
	}

	/* Send to appropriate player */
	if (target > 0)
	{
		/* Set target player */
		q_ptr = Players[target];

		/* Send message to target */
		msg_format_type(q_ptr, MSG_WHISPER, "[%s:%s] %s", q_ptr->name, sender, colon);

		/* Also send back to sender */
		msg_format_type(p_ptr, MSG_WHISPER, "[%s:%s] %s", q_ptr->name, sender, colon);

		/* Done */
		return;
	}

	/* Send to appropriate party */
	if (target < 0)
	{
		/* Send message to target party */
		party_msg_format(0 - target, "[%s:%s] %s",
		                 parties[0 - target].name, sender, colon);

		/* Also send back to sender if the sender is not in
		 * the party being messaged. */
		if (p_ptr->party != 0 - target)
			msg_format(p_ptr, "[%s:%s] %s",
				   parties[0 - target].name, sender, colon);

		/* Done */
		return;
	}

	/* Total failure... */
	if (dest_chan == -1) return;
	else if (p_ptr && !can_talk(p_ptr, dest_chan)) return;

	/* Send to everyone in this channel */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];
		if(q_ptr->on_channel[dest_chan] & UCM_EAR)
		{
			/* Send message */
			if (p_ptr)
			{
				msg_format_type(q_ptr, MSG_CHAT + dest_chan, "[%s] %s", sender, message);
			}
			else
			{
				msg_format_type(q_ptr, MSG_CHAT + dest_chan, "%s", message);
			}
		}
	}

	/* Send to the console too */
	console_print(format("[%s] %s", sender, message), dest_chan);
}


/*
 * A player has sent a message to the rest of the world.
 *
 * Parse it and send to everyone or to only the person(s) he requested.
 *
 * Note that more than one message may get sent at once, seperated by
 * tabs ('\t').  Thus, this function splits them and calls
 * "player_talk_aux" to do the dirty work.
 */
void player_talk(player_type *p_ptr, char *message)
{
	char *cur, *next;

	/* Start at the beginning */
	cur = message;

	/* Process until out of messages */
	while (cur)
	{
		/* Find the next tab */
		next = strchr(cur, '\t');

		/* Stop out the tab */
		if (next)
		{
			/* Replace with \0 */
			*next = '\0';
		}

		/* Process this message */
		player_talk_aux(p_ptr, cur);

		/* Move to the next one */
		if (next)
		{
			/* One step past the \0 */
			cur = next + 1;
		}
		else
		{
			/* No more message */
			cur = NULL;
		}
	}
}
	

/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		return (TRUE);
	}

	return (FALSE);
}

/*
 * Converts a string to a terminal color byte.
 */
int color_text_to_attr(cptr name)
{
	if (my_stricmp(name, "dark")       == 0) return (TERM_DARK);
	if (my_stricmp(name, "white")      == 0) return (TERM_WHITE);
	if (my_stricmp(name, "slate")      == 0) return (TERM_SLATE);
	if (my_stricmp(name, "orange")     == 0) return (TERM_ORANGE);
	if (my_stricmp(name, "red")        == 0) return (TERM_RED);
	if (my_stricmp(name, "green")      == 0) return (TERM_GREEN);
	if (my_stricmp(name, "blue")       == 0) return (TERM_BLUE);
	if (my_stricmp(name, "umber")      == 0) return (TERM_UMBER);
	if (my_stricmp(name, "violet")     == 0) return (TERM_VIOLET);
	if (my_stricmp(name, "yellow")     == 0) return (TERM_YELLOW);
	if (my_stricmp(name, "lightdark")  == 0) return (TERM_L_DARK);
	if (my_stricmp(name, "lightwhite") == 0) return (TERM_L_WHITE);
	if (my_stricmp(name, "lightred")   == 0) return (TERM_L_RED);
	if (my_stricmp(name, "lightgreen") == 0) return (TERM_L_GREEN);
	if (my_stricmp(name, "lightblue")  == 0) return (TERM_L_BLUE);
	if (my_stricmp(name, "lightumber") == 0) return (TERM_L_UMBER);

	/* Oops */
	return (-1);
}


/*
 * Extract a textual representation of an attribute
 */
cptr attr_to_text(byte a)
{
	switch (a)
	{
		case TERM_DARK:    return ("Dark");
		case TERM_WHITE:   return ("White");
		case TERM_SLATE:   return ("Slate");
		case TERM_ORANGE:  return ("Orange");
		case TERM_RED:     return ("Red");
		case TERM_GREEN:   return ("Green");
		case TERM_BLUE:    return ("Blue");
		case TERM_UMBER:   return ("Umber");
		case TERM_L_DARK:  return ("L.Dark");
		case TERM_L_WHITE: return ("L.Slate");
		case TERM_VIOLET:  return ("Violet");
		case TERM_YELLOW:  return ("Yellow");
		case TERM_L_RED:   return ("L.Red");
		case TERM_L_GREEN: return ("L.Green");
		case TERM_L_BLUE:  return ("L.Blue");
		case TERM_L_UMBER: return ("L.Umber");
	}

	/* Oops */
	return ("Icky");
}


/* 
 * Record a message in the character history
 */
void log_history_event(player_type *p_ptr, char *msg, bool unique)
{
	int  days, hours, mins, i;
	huge seconds, turn;

	history_event *evt;
	history_event *last = NULL;
	history_event *evt_forge = NULL;

	u16b note = quark_add(msg);

	/* Walk throu event list */
	for (evt = p_ptr->charhist; evt; evt = evt->next) 
	{
		/* Duplicate entries not allowed */
		if (evt->message == note && unique)
			return;
		/* Find last in chain */
		last = evt;
	}

	/* Convert turn counter to real time */
	seconds = days = hours = mins = turn = 0;
	for (i = 0; i < p_ptr->turn.era+2; i++)
	{
		int plus_days = 0, plus_hours = 0, plus_mins = 0;
		turn = HTURN_ERA_FLIP;
		if (i == p_ptr->turn.era+1) turn = p_ptr->turn.turn; 
		seconds = turn / cfg_fps;
		plus_days = seconds / 86400;
		plus_hours = (seconds / 3600) - (24 * days);
		plus_mins = (seconds / 60) % 60;
		days += plus_days;
		hours += plus_hours;
		mins += plus_mins;
	}
	while (mins >= 60) { mins -= 60; hours += 1; }
	while (hours >= 24) { hours -= 24; days += 1; }

	/* Create new entry */
	MAKE(evt_forge, history_event);
	evt_forge->days = days;
	evt_forge->hours = hours;
	evt_forge->mins = mins;
	evt_forge->depth = p_ptr->dun_depth;
	evt_forge->level = p_ptr->lev;
	evt_forge->message = note;

	/* Add to chain */
	if (!p_ptr->charhist)
		p_ptr->charhist = evt_forge;
	else
		last->next = evt_forge;
}
/*
 * Destroy player's history
 */
void history_wipe(history_event *evt) {
	history_event *next;
	while (evt)
	{
		/* Remember */
		next = NULL; if (evt->next)	next = evt->next;

		/* KILL */
		KILL(evt);

		/* Recall */
		evt = NULL;	if (next) evt = next;
	}
}
/*
 * Format 1 string of event history
 */
cptr format_history_event(history_event *evt)
{
	static char buf[160];
	sprintf(buf, "%02i:%02i:%02i   %4ift   %2i   %s",
			evt->days, evt->hours, evt->mins,
			evt->depth*50, evt->level, quark_str(evt->message));
	return &buf[0];
}

void send_prepared_info(player_type *p_ptr, byte win, byte stream, byte extra_params) {
	byte old_term;
	int i;

	/* Save 'current' terminal */
	old_term = p_ptr->remote_term;

	/* Activte new terminal */
	send_term_info(p_ptr, NTERM_ACTIVATE, win);

	/* Clear, Send, Refresh */
	send_term_info(p_ptr, NTERM_CLEAR, 0);
	for (i = 0; i < p_ptr->last_info_line + 1; i++)
		stream_line_as(p_ptr, stream, i, i);
	send_term_info(p_ptr, NTERM_FRESH | extra_params, 0);
	/* Restore active term */
	send_term_info(p_ptr, NTERM_ACTIVATE, old_term);

	/* Hack -- erase 'prepared info' */
	p_ptr->last_info_line = -1;
}

void send_prepared_popup(player_type *p_ptr, cptr header)
{
	int i;
	byte old_term;

	old_term = p_ptr->remote_term;

	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);
	Send_special_other(p_ptr, header);

	/* Clear, Send, Popup! */
	send_term_info(p_ptr, NTERM_CLEAR, 0);
	for (i = 0; i < p_ptr->last_info_line + 1; i++)
		stream_line_as(p_ptr, STREAM_SPECIAL_TEXT, i, i);
	send_term_info(p_ptr, NTERM_POP, 0);

	send_term_info(p_ptr, NTERM_ACTIVATE, old_term);

	/* HACK -- Assume this was NOT monster recall */
	/* This is implied, because monster recall doesn't use send_prepared_popup() */
	monster_race_track_hack(p_ptr);
}

/* This hacky function resets monster tracking after STREAM_SPECIAL_TEXT
 * was used for anything other than actual monster recall. This way,
 * server will definitely send new monster info, once it's required again. */
void monster_race_track_hack(player_type *p_ptr)
{
	/* Paranoia -- Player is not yet in the game */
	if (p_ptr->conn == -1 || !IS_PLAYING(p_ptr)) return;
	/* Only relevant if Player has no dedicated window for monster text */
	if (!p_ptr->stream_wid[STREAM_MONSTER_TEXT])
	{
		/* Hack -- cancel monster tracking */
		monster_race_track(p_ptr, 0);
	}
}

void text_out_init(player_type *p_ptr)
{
	player_textout = p_ptr;
	p_ptr->cur_wid = 0;
	p_ptr->cur_hgt = 0;

	p_ptr->last_info_line = -1;
}

void text_out_done()
{
	int i;
	player_type	*p_ptr = player_textout;
	player_textout = NULL;

	/* BAD HACK -- notify client about abrupt endings */
	if (p_ptr->cur_hgt >= MAX_TXT_INFO-1)
	{
		char *msg = " {Out of buffer space!!!} ";
		size_t len = strlen(msg);
		for (i = 0; i < len; i++)
		{
			p_ptr->info[p_ptr->cur_hgt][i].c = msg[i];
			p_ptr->info[p_ptr->cur_hgt][i].a = TERM_RED;
		}
		p_ptr->cur_wid = i;
	}

	/* HACK!! Clear rest of the line */
	for (i = p_ptr->cur_wid; i < 80; i++)
	{
		p_ptr->info[p_ptr->cur_hgt][i].c = ' ';
		p_ptr->info[p_ptr->cur_hgt][i].a = TERM_WHITE;
	}

	p_ptr->last_info_line = p_ptr->cur_hgt;

	/* Restore height and width of current dungeon level */
	p_ptr->cur_hgt = MAX_HGT;
	p_ptr->cur_wid = MAX_WID;
}

/* Taking (bad) ques from client code, here we copy one
 * buffer into another, instead of just storing pointer
 * to the correct buffer somewhere... */
/* The reason is all the current functions are hard-wired
 * to use p_ptr->info, so instead of massive overhaul (like making
 * *IT* a pointer), we add a literal workaround. */
/* TODO: Kill this. */
void text_out_save(player_type *p_ptr)
{
	int i, j;
	/* memcpy is for cowards */
	for (j = 0; j < MAX_TXT_INFO; j++)
	{
		for (i = 0; i < MAX_WID; i++)
		{
			p_ptr->file[j][i].a = p_ptr->info[j][i].a;
			p_ptr->file[j][i].c = p_ptr->info[j][i].c;
		}
	}
	p_ptr->last_file_line = p_ptr->last_info_line;
}
void text_out_load(player_type *p_ptr)
{
	/* mindless code duplication. */
	int i, j;

	for (j = 0; j < MAX_TXT_INFO; j++)
	{
		for (i = 0; i < MAX_WID; i++)
		{
			p_ptr->info[j][i].a = p_ptr->file[j][i].a;
			p_ptr->info[j][i].c = p_ptr->file[j][i].c;
		}
	}
	p_ptr->last_info_line = p_ptr->last_file_line;
	/* I hope you'll delete those functions ASAP */
	/* WHATEVER HAPPENS, PLEASE DON'T UPGRADE THIS */
	/* TO ALLOW STACKING... */
}

void text_out_c(byte a, cptr buf)
{
	int i, j, shorten, buflen;
	player_type	*p_ptr = player_textout;
	static char line_buf[80] = {'\0'};

	bool simple = FALSE;
	bool warped = FALSE;
	i = j = shorten = 0;
	buflen = strlen(buf);

#if 0
	/* Add "auto-paragraph" spaces */
	if (p_ptr->cur_wid == 0) 
	{
		line_buf[0] = '\0';
		strcat(line_buf, "  ");
		p_ptr->cur_wid += 2;
	}
#endif 
 
	while (TRUE)
	{
		/* Problem -- Out of stack space :( */
		if (p_ptr->cur_hgt >= MAX_TXT_INFO-1) break;

#if 0
		/* Add 1 space between stuff (auto-separate) */
		if (buf[shorten] != ' ' && p_ptr->cur_wid)
		{
			strcat(line_buf, " ");
			p_ptr->cur_wid += 1;
		}
#endif

		/* We can fit the info on the same line */
		if (buflen - shorten < 80 - p_ptr->cur_wid) 
		{
			/* Set to copy whole buffer */
			j = buflen - shorten;
			simple = TRUE;
		}
		/* We can't, let's find a suitable wrap point */
		else
		{
			/* Default to whole line */
			j = 0;
			/* Find some nice space near the end */
			for (i = shorten; i < buflen; i++)
			{
				if (buf[i] == ' ' || buf[i] == '\n')
				{
					if (i - shorten < 80 - p_ptr->cur_wid)
					{
						j = i - shorten;
					}
					else
					{
						break;
					}
				}
			}
			simple = FALSE;
			warped = TRUE;
		}

		/* Copy first part */
		for (i = 0; i < j; i++)
		{
			if (buf[i+shorten] != '\n')
			{
				line_buf[p_ptr->cur_wid + i] = buf[i + shorten];
			}
			else
			{
				/* If we encounter a '\n', and it's our first
				 * line ever, we ignore it... */
				if (p_ptr->cur_hgt == 0 && p_ptr->cur_wid <= 0)
				{
					p_ptr->cur_wid -= 1; //ignore, backup a bit
				}
				else
				{
					j = i + 1;
					simple = warped = FALSE;
					break;
				}
			}
		}

		/* Advance forward */
		p_ptr->cur_wid += i;
		
		/* Fill the rest with spaces */
		for (i = p_ptr->cur_wid; i < 80; i++)
		{
			line_buf[i] = ' ';
		}

		/* Dump it */
		for (i = p_ptr->cur_wid-j; i < 80; i++)
		{
			p_ptr->info[p_ptr->cur_hgt][i].c = line_buf[i];
			p_ptr->info[p_ptr->cur_hgt][i].a = a;
		}

		/* End function for simple cases */
		if (simple) break;

		/* Advance to next line */
		p_ptr->cur_hgt += 1;
		p_ptr->cur_wid = 0;

		/* Shorten the text */
		shorten += j;

		/* Handle spaces */
		if (warped && buf[shorten] == ' ') shorten++; 

		/* Finish when we're done */
		if (shorten >= buflen) break; 
	}
}

void text_out(cptr str)
{
	text_out_c(TERM_WHITE, str);
}

void c_prt(player_type *p_ptr, byte attr, cptr str, int row, int col)
{
	/* Paranoia */
	if (row > MAX_TXT_INFO) return;

	while (*str)
	{
		/* Limit */
		if (col > 80) break;

		p_ptr->info[row][col].c = *str;
		p_ptr->info[row][col].a = attr; 

		col++;
		str++;
	}
}
void prt(player_type *p_ptr, cptr str, int row, int col)
{
	c_prt(p_ptr, TERM_WHITE, str, row, col);
}

void clear_line(player_type *p_ptr, int row)
{
	int i;
	for (i = 0; i < 80; i++)
	{
		p_ptr->info[row][i].c = ' ';
		p_ptr->info[row][i].a = TERM_WHITE; 
	}
}
void clear_from(player_type *p_ptr, int row)
{
	int i;
	while (row < MAX_TXT_INFO)
	{
		for (i = 0; i < 80; i++)
		{
			p_ptr->info[row][i].c = ' ';
			p_ptr->info[row][i].a = TERM_DARK; 
		}
		row++;
	}
}

bool askfor_aux(player_type *p_ptr, char query, char *buf, int row, int col, cptr prompt, cptr default_value, byte prompt_attr, byte input_attr)
{
	char * mark = &(p_ptr->interactive_hook[0][1]);
	char * len = &(p_ptr->interactive_hook[0][2]);
	char * y = &(p_ptr->interactive_hook[0][3]);
	char * x = &(p_ptr->interactive_hook[0][4]);
	char * attr = &(p_ptr->interactive_hook[0][5]);
	char * mlen = &(p_ptr->interactive_hook[0][6]);
	char * str = p_ptr->interactive_hook[1];	

	if (*mark)
	{
		*mark = 0;	
		strncpy(buf, str, *len);
		buf[(byte)*len] = '\0';
		return TRUE;
	}
	else
	{
		*mark = query;
		*attr = input_attr;
		*y = row;
		*x = col;
		*len = 0;
		*mlen =0;

		if (!STRZERO(prompt))
		{
 			(*x) += strlen(prompt);
			clear_line(p_ptr, row);
			c_prt(p_ptr, prompt_attr, prompt, row, col);
			Stream_line_p(p_ptr, STREAM_SPECIAL_TEXT, row);
 		}
 		if (!STRZERO(default_value))
 		{
 			/* Hack: ask for 1 character */
 			if (default_value[0] == '*') *mlen =1;
 			else
 			{
				*len = strlen(default_value);
 				strncpy(str, default_value, *len);
 			}
 		}

		do_cmd_interactive_input(p_ptr, 0); /* ! */

		return FALSE;
	}
}
bool ask_for(player_type *p_ptr, char query, char *buf)
{
	return askfor_aux(p_ptr, query, buf, 0, 0, "", "", TERM_DARK, TERM_WHITE);
}
