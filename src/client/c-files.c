/*
 * Handle client-side files, such as the .mangrc configuration
 * file, and some various "pref files".
 */

#include "c-angband.h"

#if defined(ON_IOS) || (defined(ON_OSX) && !defined(HAVE_CONFIG_H))
#include "appl-dir.h"
#endif

/*
 * MAngband-specific R:0 loader hack.
 * An image for a Race/Class combo is stored in client pref files. Traditionally (as in V),
 * [ EQU $RACE ] [ EQU $CLASS ] are used to load those into the r_info[0] slot accordingly
 * to character's race and class. In MAngband, however, players can encounter other
 * characters and thus need the infromation for ALL the Race/Class combos there are.
 * To load this information, a 'virtual mode' hack is used, which stores the last encountered
 * [ EQU $RACE ] [ EQU $CLASS ] check as 'fake_race'/'fake_class' global variables.
 * 'Virtual mode' only works with 'R:0:...' lines, doesn't allow includes and cancels itself
 * after each read 'R:0:...' line.
 *
 * Note: player's own race and class image will be loaded into r_info[0] slot as usual.
 *
 * Note: we support a fake class called 'Special' (hopefully no variants ever need this 
 * as valid class name), to load several MAngband-specific images. Index 0 is 'Ghost'
 * and Index 1 is 'Fruitbat'.
 *
 * Note: for this hack to work, we also support digits as race 'names', i.e.
 * [ EQU $RACE 0 ] *will* work with our parser. 
 *
 */
int fake_race = -1;
int fake_class = -1;
static int find_race(cptr name)
{
	int i;

	/* Hack: allow digit as race name, directly converting it */
	if (isdigit(name[0]))
	{
		i = D2I(name[0]);
		if (i < 0 || i >= z_info.p_max) return -1;
		return i;
	}

	/* Find race by name */
	for (i = 0; i < z_info.p_max; i++)
	{
		const char* v = p_name + race_info[i].name;
		if (!my_strnicmp(v, name, strlen(v)))
			return i;
	}
	return -1;
}
static int find_class(cptr name)
{
	int i;

	/* Hack: allow "Special" as class name, return last entry */
	if (!my_strnicmp("Special", name, 7)) return z_info.c_max;

	/* Find class by name */
	for (i = 0; i < z_info.c_max; i++)
	{
		const char* v = c_name + c_info[i].name;
		if (!my_strnicmp(v, name, strlen(v)))
			return i;
	}
	return -1;
}

/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens)
{
	int i = 0;

	char *s = buf;


	/* Process */
	while (i < num - 1)
	{
		char *t;

		/* Scan the string */
		for (t = s; *t; t++)
		{
			/* Found a delimiter */
			if ((*t == ':') || (*t == '/')) break;

			/* Handle single quotes */
			if (*t == '\'') //'
			{
				/* Advance */
				t++;

				/* Handle backslash */
				if (*t == '\\') t++;

				/* Require a character */
				if (!*t) break;

				/* Advance */
				t++;

				/* Hack -- Require a close quote */
				if (*t != '\'') *t = '\''; //'
			}

			/* Handle back-slash */
			if (*t == '\\') t++;
		}

		/* Nothing left */
		if (!*t) break;

		/* Nuke and advance */
		*t++ = '\0';

		/* Save the token */
		tokens[i++] = s;

		/* Advance */
		s = t;
	}

	/* Save the token */
	tokens[i++] = s;

	/* Number found */
	return (i);
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
 * Transform macro trigger name ('\[alt-D]' etc..)
 * into macro trigger key code ('^_O_64\r' or etc..)
 */
static size_t trigger_text_to_ascii(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	bool mod_status[MAX_MACRO_MOD];

	int i, len = 0;
	int shiftstatus = 0;
	cptr key_code;
	
	size_t current_len = strlen(buf);

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Initialize modifier key status */	
	for (i = 0; macro_modifier_chr[i]; i++)
		mod_status[i] = FALSE;

	str++;

	/* Examine modifier keys */
	while (1)
	{
		/* Look for modifier key name */
		for (i = 0; macro_modifier_chr[i]; i++)
		{
			len = strlen(macro_modifier_name[i]);

			if (!my_strnicmp(str, macro_modifier_name[i], len))
				break;
		}

		/* None found? */
		if (!macro_modifier_chr[i]) break;

		/* Proceed */
		str += len;

		/* This modifier key is pressed */
		mod_status[i] = TRUE;

		/* Shift key might be going to change keycode */
		if ('S' == macro_modifier_chr[i])
			shiftstatus = 1;
	}

	/* Look for trigger name */
	for (i = 0; i < max_macrotrigger; i++)
	{
		len = strlen(macro_trigger_name[i]);

		/* Found it and it is ending with ']' */
		if (!my_strnicmp(str, macro_trigger_name[i], len) && (']' == str[len]))
			break;
	}

	/* Invalid trigger name? */
	if (i == max_macrotrigger)
	{
		/*
		 * If this invalid trigger name is ending with ']',
		 * skip whole of it to avoid defining strange macro trigger
		 */
		str = strchr(str, ']');

		if (str)
		{
			strnfcat(buf, max, &current_len, "\x1F\r");

			*strptr = str; /* where **strptr == ']' */
		}

		return current_len;
	}

	/* Get keycode for this trigger name */
	key_code = macro_trigger_keycode[shiftstatus][i];

	/* Proceed */
	str += len;

	/* Begin with '^_' */
	strnfcat(buf, max, &current_len, "\x1F");

	/* Write key code style trigger using template */
	for (i = 0; macro_template[i]; i++)
	{
		char ch = macro_template[i];
		int j;

		switch(ch)
		{
		case '&':
			/* Modifier key character */
			for (j = 0; macro_modifier_chr[j]; j++)
			{
				if (mod_status[j])
					strnfcat(buf, max, &current_len, "%c", macro_modifier_chr[j]);
			}
			break;
		case '#':
			/* Key code */
			strnfcat(buf, max, &current_len, "%s", key_code);
			break;
		default:
			/* Fixed string */
			strnfcat(buf, max, &current_len, "%c", ch);
			break;
		}
	}

	/* End with '\r' */
	strnfcat(buf, max, &current_len, "\r");

	/* Succeed */
	*strptr = str; /* where **strptr == ']' */

	return current_len;
}



/*
 * Hack -- convert a printable string into real ascii
 *
 * I have no clue if this function correctly handles, for example,
 * parsing "\xFF" into a (signed) char.  Whoever thought of making
 * the "sign" of a "char" undefined is a complete moron.  Oh well.
 */
void text_to_ascii(char *buf, size_t max, cptr str)
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

			/* Macro Trigger */
			if (*str == '[')
			{
				/* Terminate before appending the trigger */
				*s = '\0';

				s += trigger_text_to_ascii(buf, max, &str);
			}

			/* Hex-mode XXX */
			else if (*str == 'x')
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

			/* MAngband-specific: feed queue */
			else if (*str == 'f') *s++ = '\f';

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
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variable.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Mega-Hack -- support fat raw files under NEXTSTEP, using special
 * "suffixed" directories for the "ANGBAND_DIR_DATA" directory, but
 * requiring the directories to be created by hand by the user.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".
 */
void free_file_paths(void)
{
	/* Free the main path */
	string_ifree(ANGBAND_DIR);

	/* Free the sub-paths */
	string_ifree(ANGBAND_DIR_APEX);
	string_ifree(ANGBAND_DIR_BONE);
	string_ifree(ANGBAND_DIR_DATA);
	string_ifree(ANGBAND_DIR_EDIT);
	string_ifree(ANGBAND_DIR_FILE);
	string_ifree(ANGBAND_DIR_HELP);
	string_ifree(ANGBAND_DIR_INFO);
	string_ifree(ANGBAND_DIR_SAVE);
	string_ifree(ANGBAND_DIR_PREF);
	string_ifree(ANGBAND_DIR_USER);
	string_ifree(ANGBAND_DIR_XTRA);

	/* Free extra paths */
	string_ifree(ANGBAND_DIR_XTRA_SOUND);
	string_ifree(ANGBAND_DIR_XTRA_GRAF);
}

void init_file_paths(char *path)
{
        char *tail;


        /*** Free everything ***/
	free_file_paths();

        /*** Prepare the "path" ***/

        /* Hack -- save the main directory */
        ANGBAND_DIR = string_make(path);

        /* Prepare to append to the Base Path */
        tail = path + strlen(path);


#ifdef VM


        /*** Use "flat" paths with VM/ESA ***/

        /* Use "blank" path names */
        ANGBAND_DIR_APEX = string_make("");
        ANGBAND_DIR_BONE = string_make("");
        ANGBAND_DIR_DATA = string_make("");
        ANGBAND_DIR_EDIT = string_make("");
        ANGBAND_DIR_FILE = string_make("");
        ANGBAND_DIR_HELP = string_make("");
        ANGBAND_DIR_INFO = string_make("");
        ANGBAND_DIR_SAVE = string_make("");
        ANGBAND_DIR_PREF = string_make("");
        ANGBAND_DIR_USER = string_make("");
        ANGBAND_DIR_XTRA = string_make("");
        ANGBAND_DIR_XTRA_SOUND = string_make("");
        ANGBAND_DIR_XTRA_GRAF = string_make("");


#else /* VM */


        /*** Build the sub-directory names ***/

        /* Build a path name */
        strcpy(tail, "apex");
        ANGBAND_DIR_APEX = string_make(path);

        /* Build a path name */
        strcpy(tail, "bone");
        ANGBAND_DIR_BONE = string_make(path);

        /* Build a path name */
        strcpy(tail, "data");
        ANGBAND_DIR_DATA = string_make(path);

        /* Build a path name */
        strcpy(tail, "edit");
        ANGBAND_DIR_EDIT = string_make(path);

        /* Build a path name */
        strcpy(tail, "file");
        ANGBAND_DIR_FILE = string_make(path);

        /* Build a path name */
        strcpy(tail, "help");
        ANGBAND_DIR_HELP = string_make(path);

        /* Build a path name */
        strcpy(tail, "info");
        ANGBAND_DIR_INFO = string_make(path);

        /* Build a path name */
        strcpy(tail, "save");
        ANGBAND_DIR_SAVE = string_make(path);

        /* Build a path name */
        strcpy(tail, "pref");
        ANGBAND_DIR_PREF = string_make(path);

        /* Build a path name */
        strcpy(tail, "user");
        ANGBAND_DIR_USER = string_make(path);

        /* Build a path name */
        strcpy(tail, "xtra");
        ANGBAND_DIR_XTRA = string_make(path);

        /* Build a path name */
        strcpy(tail, "xtra/sound");
        ANGBAND_DIR_XTRA_SOUND = string_make(path);

        /* Build a path name */
        strcpy(tail, "xtra/graf");
        ANGBAND_DIR_XTRA_GRAF = string_make(path);

#endif /* VM */


#ifdef NeXT

        /* Allow "fat binary" usage with NeXT */
        if (TRUE)
        {
                cptr next = NULL;

# if defined(m68k)
                next = "m68k";
# endif

# if defined(i386)
                next = "i386";
# endif

# if defined(sparc)
                next = "sparc";
# endif

# if defined(hppa)
                next = "hppa";
# endif

                /* Use special directory */
                if (next)
                {
                        /* Forget the old path name */
                        string_free(ANGBAND_DIR_DATA);

                        /* Build a new path name */
                        sprintf(tail, "data-%s", next);
                        ANGBAND_DIR_DATA = string_make(path);
                }
        }

#endif /* NeXT */

}





/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Parse another file recursively, see below for details
 *   %:<filename>
 *
 * Specify the attr/char values for "monsters" by race index
 *   R:<num>:<a>:<c>
 *
 * Specify the attr/char values for "objects" by kind index
 *   K:<num>:<a>:<c>
 *
 * Specify the attr/char values for "features" by feature index
 *   F:<num>:<a>:<c>
 *
 * Specify the attr/char values for unaware "objects" by kind tval
 *   U:<tv>:<a>:<c>
 *
 * Specify the attr/char values for inventory "objects" by kind tval
 *   E:<tv>:<a>:<c>
 *
 * Execute macro action right now!
 *   \:<str>
 *
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a normal macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a command macro, given an encoded macro trigger
 *   G:<str>
 *
 * Create a keymap, given an encoded macro trigger
 *   C:<num>:<str>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify a use for a subwindow
 *   W:<num>:<use>
 */

errr process_pref_file_command(char *buf)
{
	int i, j;
	int n1, n2;

	char *zz[16];
	bool virt = ((fake_class == -1 || fake_race == -1) ? FALSE : TRUE);

	/* Skip "empty" lines */
	if (!buf[0]) return (0);

	/* Skip "blank" lines */
	if (isspace(buf[0])) return (0);

	/* Skip comments */
	if (buf[0] == '#') return (0);


	/* Require "?:*" format */
	if (buf[1] != ':') return (1);


	/* Process "%:<fname>" */
	if (buf[0] == '%')
	{
		/* Attempt to Process the given file */
		return (process_pref_file(buf + 2));
	}


	/* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
	if (buf[0] == 'R')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			if (i >= z_info.r_max) return (1);
			/* MAngband-specific hack: fill the 'pr' array */
			if ((virt == TRUE) && (i == 0))
			{
				if (n1) p_ptr->pr_attr[fake_class * z_info.p_max + fake_race] = n1;
				if (n2) p_ptr->pr_char[fake_class * z_info.p_max + fake_race] = n2;
				/* And cancel virtual mode */
				fake_race = fake_class = -1;
				return (0);
			}
			if (n1) Client_setup.r_attr[i] = n1;
			if (n2) Client_setup.r_char[i] = n2;
			return (0);
                }
	}


	/* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
	else if (buf[0] == 'K')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= z_info.k_max) return (1);
			if (n1) Client_setup.k_attr[i] = n1;
			if (n2) Client_setup.k_char[i] = n2;
			return (0);
		}
	}


	/* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
	else if (buf[0] == 'F')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= z_info.f_max) return (1);
			if (n1) Client_setup.f_attr[i] = n1;
			if (n2) Client_setup.f_char[i] = n2;
			return (0);
		}
	}

	/* Process "L:<num>:<a>/<c>" -- attr/char for flavors */
	else if (buf[0] == 'L')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= (long)N_ELEMENTS(Client_setup.flvr_x_attr))) return (1);
			if (n1) Client_setup.flvr_x_attr[i] = (byte)n1;
			if (n2) Client_setup.flvr_x_char[i] = (char)n2;
			return (0);
		}
	}
#if 0
	/* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
	else if (buf[0] == 'U')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			j = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (j > 100) return 0;
			if (n1) Client_setup.u_attr[j] = n1;
			if (n2) Client_setup.u_char[j] = n2;
			return (0);
		}
	}
#endif
	/* Process "S:<num>:<a>/<c>" -- attr/char for special things */
	else if (buf[0] == 'S')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= (long)N_ELEMENTS(Client_setup.misc_attr))) return (1);
			Client_setup.misc_attr[i] = (byte)n1;
			Client_setup.misc_char[i] = (char)n2;
			return (0);
		}
	}

	/* Process "E:<tv>:<a>" -- attribute for inventory objects  */
	else if (buf[0] == 'E')
	{
		/*  "E:<tv>:<a>" -- attribute for inventory objects  */
		if (tokenize(buf+2, 2, zz) == 2)
		{
			i = strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			if ((i < 0) || (i >= (long)N_ELEMENTS(Client_setup.tval_attr))) return (1);
			if (n1) Client_setup.tval_attr[i] = (byte)n1;
			return (0);
		}
		/*  "E:<tv>:<a>/<c>" -- attr/char for equippy chars */
		if (tokenize(buf+2, 3, zz) == 3)
		{
			j = (byte)strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (n1) Client_setup.tval_attr[j] = n1;
			if (n2) Client_setup.tval_char[j] = n2;
			return (0);
		}
	}

	/* Process "M" -- create mousemap */
	else if (buf[0] == 'M')
	{
		byte mb_from, mb_to;

		if (tokenize(buf+2, 2, zz) != 2) return (1);

		mb_from = (byte)strtol(zz[0], NULL, 16);
		mb_to = (byte)strtol(zz[1], NULL, 16);

		mousemap[mb_from] = mb_to;

		return (0);
	}

	/* Process "{" -- single auto-inscribe instruction */
	else if (buf[0] == '{')
	{
		char tmpbuf[1024];
		char *item_query;
		char *item_inscription;
		my_strcpy(tmpbuf, buf + 2, sizeof(tmpbuf));
		item_query = strtok(tmpbuf, ":");
		item_inscription = strtok(NULL, ":");
		if (!item_query || !item_inscription) return 1; /* Fail */
		do_cmd_inscribe_auto(item_query, item_inscription);
		/* Done */
		return (0);
	}

	/* Process "A:<str>" -- save an "action" for later */
	else if (buf[0] == 'A')
	{
		text_to_ascii(macro__buf, 1024, buf+2);
		return (0);
	}

	/* Process "\:" -- execute "action" right away */
	else if (buf[0] == '\\')
	{
		int i, n;
		char raw_cmd[1024];
		text_to_ascii(raw_cmd, sizeof(raw_cmd), buf+2);
		n = strlen(raw_cmd);
		for (i = 0; i < n; i++) Term_keypress(raw_cmd[i]);
		return (0);
	}

	/* Process "P:<str>" -- create normal macro */
	else if (buf[0] == 'P')
	{
		char tmp[1024];
		text_to_ascii(tmp, sizeof(tmp), buf+2);
		macro_add(tmp, macro__buf, FALSE);
		return (0);
	}
	/* Process "G:<str>" -- create command macro */
	else if (buf[0] == 'G')
	{
		char tmp[1024];
		text_to_ascii(tmp, sizeof(tmp), buf+2);
		macro_add(tmp, macro__buf, TRUE);
		return (0);
	}
	/* Process "C:<num>:<str>" -- create keymap */
	else if (buf[0] == 'C')
	{
		long mode;

		char tmp[1024];

		if (tokenize(buf+2, 2, zz) != 2) return (1);

		mode = strtol(zz[0], NULL, 0);
		if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);

		text_to_ascii(tmp, sizeof(tmp), zz[1]);
		if (!tmp[0] || tmp[1]) return (1);
		i = (long)tmp[0];

		string_free(keymap_act[mode][i]);
		keymap_act[mode][i] = string_make(macro__buf);

		return (0);
	}

	/* set macro trigger names and a template */
	/* Process "T:<trigger>:<keycode>:<shift-keycode>" */
	/* Process "T:<template>:<modifier chr>:<modifier name>:..." */
	else if (buf[0] == 'T')
	{
		int tok;

		tok = tokenize(buf + 2, MAX_MACRO_MOD + 2, zz);

		/* Trigger template */
		if (tok >= 4)
		{
			int i;
			int num;

			/* Free existing macro triggers and trigger template */
			macro_trigger_free();

			/* Clear template done */
			if (*zz[0] == '\0') return 0;

			/* Count modifier-characters */
			num = strlen(zz[1]);

			/* One modifier-character per modifier */
			if (num + 2 != tok) return 1;

			/* Macro template */
			macro_template = string_make(zz[0]);

			/* Modifier chars */
			macro_modifier_chr = string_make(zz[1]);

			/* Modifier names */
			for (i = 0; i < num; i++)
			{
				macro_modifier_name[i] = string_make(zz[2+i]);
			}
		}
		/* Macro trigger */
		else if (tok >= 2)
		{
			char *buf;
			cptr s;
			char *t;

			if (max_macrotrigger >= MAX_MACRO_TRIGGER)
			{
				c_msg_print("Too many macro triggers!");
				return 1;
			}

			/* Buffer for the trigger name */
			C_MAKE(buf, strlen(zz[0]) + 1, char);

			/* Simulate strcpy() and skip the '\' escape character */
			s = zz[0];
			t = buf;

			while (*s)
			{
				if ('\\' == *s) s++;
				*t++ = *s++;
			}

			/* Terminate the trigger name */
			*t = '\0';

			/* Store the trigger name */
			macro_trigger_name[max_macrotrigger] = string_make(buf);

			/* Free the buffer */
			FREE(buf);

			/* Normal keycode */
			macro_trigger_keycode[0][max_macrotrigger] = string_make(zz[1]);

			/* Special shifted keycode */
			if (tok == 3)
			{
				macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[2]);
			}
			/* Shifted keycode is the same as the normal keycode */
			else
			{
				macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[1]);
			}

			/* Count triggers */
			max_macrotrigger++;
		}

		return 0;
	}

	/* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
	else if (buf[0] == 'V')
	{
		/* Do nothing */
		// return (0);  // @@@ Why was this here? -kts

		if (tokenize(buf+2, 5, zz) == 5)
		{
			i = (byte)strtol(zz[0], NULL, 0);
			color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
			color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
			color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
			color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
			return (0);
		}
	}


	/* Process "X:<str>" -- turn option off */
	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'X' || buf[0] == 'Y')
	{
		bool opt_value = TRUE;
		if (buf[0] == 'X') opt_value = FALSE;
			for (i = 0; local_option_info[i].o_desc; i++)
			{
				if (local_option_info[i].o_var &&
					local_option_info[i].o_text &&
					streq(local_option_info[i].o_text, buf + 2))
				{
					if (local_option_info[i].o_set)
					{
						p_ptr->options[ local_option_info[i].o_set ] = opt_value;
					}
					(*local_option_info[i].o_var) = opt_value;
				}
			}
			for (i = 0; i < options_max; i++)
			{
				if (option_info[i].o_page == 1 && ignore_birth_options == TRUE) continue;
				if (option_info[i].o_page &&
					option_info[i].o_text &&
					streq(option_info[i].o_text, buf + 2))
				{
					//printf("[%02d] %s = %s\n", option_info[i].o_page, option_info[i].o_text, opt_value ? "TRUE" : "FALSE");
					if (option_info[i].o_set)
					{
						(*local_option_info[ option_info[i].o_set ].o_var) = opt_value;
					}
					p_ptr->options[i] = opt_value;
				}
		}

		/* Success, even if option name was unrecognized */
		return (0);
	}

	/* MAngband-specific hack: read hitpoint warning */
	else if (buf[0] == 'H')
	{
		if (tokenize(buf + 2, 1, zz) == 1)
		{
			i = strtol(zz[0], NULL, 0);

			/* Bounds */
			if (i < 0) i = 0;
			if (i > 9) i = 9;

			p_ptr->hitpoint_warn = i;
		}
		/* Whatever the value is, we accept it */
		return (0);
	}

	/* Process "W:<num>:<use>" -- specify window action */
	else if (buf[0] == 'W')
	{
		long win, flag;

		if (tokenize(buf + 2, 2, zz) == 2)
		{
			win = strtol(zz[0], NULL, 0);
			flag = strtol(zz[1], NULL, 0);

			/* Ignore illegal windows */
			if ((win >= ANGBAND_TERM_MAX)) return (0);
			/* Hack -- Ignore the main window (but let STATUS and COMPACT be) */
			if ((win <= 0) && ((1L << flag) != PW_STATUS) && ((1L << flag) != PW_PLAYER_2)) return (0);
#ifdef PMSG_TERM
			/* Hack -- Ignore Term-4/PW_MESSGAE_CHAT settings */
			if (flag == PW_MESSAGE_CHAT && win != PMSG_TERM) flag = 0;
			if (win == PMSG_TERM) flag = PW_MESSAGE_CHAT;
#endif

			/* Ignore illegal flags */
			if ((flag < 0) || (flag >= 32)) return (0);

			/* Require a real flag */
			/* No need to be so strict in MAngband, might be server-defined window
			//if (window_flag_desc[flag]) */
			{
				/* Turn flag on */
				window_flag[win] |= (1L << flag);
				window_flag_o[win] |= (1L << flag);
			}

			/* Success */
			return (0);
		}
	}

	/* Failure */
	return (1);
}

/* Uber hack + Code duplication */
errr Save_windows(void)
{
	int i;
	byte j;

	ang_file* fp;

	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "window.prf");

	/* Open the file */
	fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Catch errors */
	if (!fp) return (-1);

	/* Skip space */
	file_putf(fp, "# Window.prf:  Set the 'usage' on the various windows\n");
	file_putf(fp, "\n\n");
	file_putf(fp, "# Usage: W:<window number>:<usage number>\n");
	file_putf(fp, "# \n");
	file_putf(fp, "# Valid usage numbers:\n");
	/* Describe */
	for (j = 0; j < 32; j++)
	{
		if (window_flag_desc[j])
		file_putf(fp, "# 	%d - %s\n", j, window_flag_desc[j]);
	}
	file_putf(fp, "\n\n");

	/* Dump */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		if (window_flag[i])
		{
			for (j = 0; j < 32; j++)
			{
				if (window_flag[i] & (1L << j))
					file_putf(fp, "W:%d:%d\n", i, j);
			}
		}
	}

	/* Close the file */
	file_close(fp);

	return 0;
}

errr Save_options(void)
{
	int i;
	errr windows;
	ang_file* fp;

	char buf[1024];
	byte last_page;

	windows = Save_windows();


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "options.prf");

	/* Open the file */
	fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Catch errors */
	if (!fp) return (-1);

	/* Skip space */
	file_putf(fp, "# This file can be used to set or clear all of the options.\n");
	file_putf(fp, "# Note that all of the options are given.\n\n");
	file_putf(fp, "# Remember that \"X\" turns an option OFF, while \"Y\" turns an option ON.\n");
	file_putf(fp, "# Also remember that not all options are used.\n\n");

	/* Dump local options with "X:<str>" and "Y:<str>" */
	last_page = 0;
	for (i = 0; local_option_info[i].o_desc; i++)
	{
		if (local_option_info[i].o_text)
		{
			file_putf(fp, "%c:%s\n", (((*local_option_info[i].o_var) == TRUE) ? 'Y' : 'X'), local_option_info[i].o_text);
		}
		if (last_page != local_option_info[i].o_page)
		{
			file_putf(fp, "\n");
		}
		last_page = local_option_info[i].o_page;
	}
	/* Dump remote options, skipping linked ones */
	last_page = 0;
	for (i = 0; i < options_max; i++)
	{
		if (!option_info[i].o_set && option_info[i].o_text)
		{
			file_putf(fp, "%c:%s\n", (p_ptr->options[i] == TRUE ? 'Y' : 'X'), option_info[i].o_text);
		}
		if (last_page != option_info[i].o_page)
		{
			file_putf(fp, "\n");
		}
		last_page = option_info[i].o_page;
	}

	/* MAngband-specific Hack: hitpoint warning (in V, it is stored in savefile) */
	file_putf(fp, "\n# Hitpoint warning\nH:%d\n", MAX(p_ptr->hitpoint_warn, hitpoint_warn_toggle));

	/* Close the file */
	file_close(fp);

	return 0;
}

/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static cptr process_pref_file_expr(char **sp, char *fp)
{
	cptr v;

	char *b;
	char *s;

	char b1 = '[';
	char b2 = ']';

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace((unsigned char)*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == b1)
	{
		const char *p;
		const char *t;

		/* Skip b1 */
		s++;

		/* First */
		t = process_pref_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) >= 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) <= 0)) v = "0";
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != b2) v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';
	}

	/* Other */
	else
	{
		/* Accept all printables except spaces and brackets */
		while (isprint((unsigned char)*s) && !strchr(" []", *s)) ++s;

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			/* System */
			if (streq(b+1, "SYS"))
			{
				v = ANGBAND_SYS;
			}

			/* Graphics */
			else if (streq(b+1, "GRAF"))
			{
				v = ANGBAND_GRAF;
			}

			/* Specific tileset */
			if (streq(b+1, "GRAFNAME"))
			{
				v = ANGBAND_GRAFNAME;
			}

			/* Font-system */
			if (streq(b+1, "FON"))
			{
				v = ANGBAND_FON;
			}

			/* Specific font */
			if (streq(b+1, "FONTNAME"))
			{
				v = ANGBAND_FONTNAME;
			}

			/* Race */
			else if (streq(b+1, "RACE"))
			{
				v = p_name + race_info[p_ptr->prace].name;
				/* MAngband-specific hack: enter virtual mode */
				if (s) fake_race = find_race(s);
			}

			/* Class */
			else if (streq(b+1, "CLASS"))
			{
				v = c_name + c_info[p_ptr->pclass].name;
				/* MAngband-specific hack: enter virtual mode */
				if (s) fake_class = find_class(s);
			}

			/* Player */
			else if (streq(b+1, "PLAYER"))
			{
				v = nick;//op_ptr->base_name;
			}

			/* Game version */
			else if (streq(b+1, "VERSION"))
			{
				v = format("%d.%d.%d",CLIENT_VERSION_MAJOR,CLIENT_VERSION_MINOR,CLIENT_VERSION_PATCH);
			}
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;

	/* Save */
	(*sp) = s;

	/* Result */
	return (v);
}

/*
 * Open the "user pref file" and parse it.
 */
static errr process_pref_file_aux(cptr name)
{
	ang_file* fp;

	char buf[1024];

	char old[1024];

	int line = -1;

	errr err = 0;

	bool bypass = FALSE;


	/* Open the file */
	fp = file_open(name, MODE_READ, -1);

	/* No such file */
	if (!fp) return (-1);


	/* Process the file */
	while (file_getl(fp, buf, sizeof(buf)))
	{
		/* Count lines */
		line++;


		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace((unsigned char)buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;


		/* Save a copy */
		my_strcpy(old, buf, sizeof(old));


		/* Process "?:<expr>" */
		if ((buf[0] == '?') && (buf[1] == ':'))
		{
			char f;
			cptr v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_pref_file_expr(&s, &f);

			/* Set flag */
			bypass = (streq(v, "0") ? TRUE : FALSE);

			/* Continue */
			continue;
		}

		/* Apply conditionals (unless it's an 'R' entry) */
		if (bypass && buf[0] != 'R') continue;

		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			(void)process_pref_file(buf + 2);

			/* Continue */
			continue;
		}


		/* Process the line */
		err = process_pref_file_command(buf);

		/* Oops */
		if (err) break;
	}


	/* Error */
	if (err)
	{
		/* Print error message */
		/* ToDo: Add better error messages */
		printf("Error %d in line %d of file '%s'.\n", err, line, name);
		printf("Parsing '%s'\n", old);
	}

	/* Close the file */
	file_close(fp);

	/* Result */
	return (err);
}



/*
 * Process the "user pref file" with the given name
 *
 * See the functions above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
	char buf[1024];

	errr err = 0;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_PREF, name);

	/* Process the pref file */
	err = process_pref_file_aux(buf);

	/* Stop at parser errors, but not at non-existing file */
	if (err == -1)
	{
		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

		/* Process the pref file */
		err = process_pref_file_aux(buf);
	}

	/* Result */
	return (err);
}


/*
 * Show the Message of the Day.
 *
 * It is given in the "Setup" info sent by the server.
 */
void show_motd(void)
{
	int i;
	char ch;
	event_type chkey;

	/* Clear the screen */
	Term_clear();

	for (i = 0; i < 23; i++)
	{
		/* Show each line */
		//Term_putstr(0, i, -1, TERM_WHITE, &Setup.motd[i * 80]);
	}

	/* Show it */
	Term_fresh();

	/* Wait for a keypress */
	Term_inkey(&chkey, TRUE, TRUE);
	ch = chkey.key;

	/* Clear the screen again */
	Term_clear();
}

void show_recall(byte win, cptr prompt)
{
	int n;

	byte st = window_to_stream[win];
	stream_type *stream = &streams[st];
	cave_view_type *source;

	byte cols = p_ptr->stream_wid[st];
	byte rows = p_ptr->stream_hgt[st];

	if (looking == FALSE)
	{
		return;
	}

	/* Already in icky mode */
	if (section_icky_row)
	{
		/* Reflush */
		Term_load();
		Term_save();
	}

	/* HACK -- Actually hide recall popup */
	if (win == NTERM_WIN_NONE)
	{
		target_recall = FALSE;
		section_icky_row = 0;
		section_icky_col = 0;
		return;
	}

	for (n = 0; n < last_remote_line[win]+2; n++)
	{
		Term_erase(0, n, 80);
	}

	for (n = 0; n < last_remote_line[win]+1; n++)
	{
		source = stream_cave(st, n);
		caveprt(source, 80, 0, n);
	}

	/* Hack -- prompt is NULL ? */
	if (prompt == NULL) prompt = "";

	/* Hack -- append target prompt after ':' */
	source = stream_cave(st, 0);
	for (n = 0; n < 80-2; n++)
	{
		if (source[n].c == ':')
		{
			prt(prompt, 0, n + 2);
			break;
		}
	}

	target_recall = TRUE;
	section_icky_row = last_remote_line[win] + 2;
	section_icky_col = 80;
}

/* Despite it's name, this function is a sister to "cmd_interactive".
 * Here, we *agree* to server's interactive request, so we don't
 * INITIALLY send anything. "cmd_interactive", on the other hand,
 * enters interactive mode by itself AND informs server that it did. */
void prepare_popup(int line_type, bool use_anykey)
{
	char ch;

	/* Hack -- if the screen is already icky, ignore this command */
	if (screen_icky && !shopping) return;

	/* Agree to SPECIAL stream */
	//special_line_type = line_type;
	special_line_onscreen = TRUE;

	/* Save the screen */
	Term_save();

	/* Wait until we get the whole thing */
	while (TRUE)
	{
		ch = inkey();

		if (!ch) continue;

		if (use_anykey) break;

		send_term_key(ch);

		if (ch == ESCAPE) break;
	}

	/* Remove partial ickyness */
	section_icky_col = section_icky_row = 0;

	/* Stop it with SPECIAL stream */
	//special_line_type = 0;
	special_line_onscreen = FALSE;

	/* Reload the screen */
	Term_load();

	/* Flush any queued events */
	Flush_queue();
}

void show_popup(void)
{
	byte n;
	int cols = 80; //TODO: fixme

	/* Hack -- if the screen is already icky, ignore this command */
	if (screen_icky && !shopping) return;

	/* Not waiting for popup, ignore this command */
	if (special_line_onscreen == FALSE) return;

	/* Draw "shadow" */
	for (n = 0; n < last_remote_line[p_ptr->remote_term] + 6; n++)
	{
		Term_erase(0, n, cols);
	}

	/* Draw text */
	for (n = 0; n < last_remote_line[p_ptr->remote_term] + 1; n++)
	{
		caveprt(stream_cave(window_to_stream[p_ptr->remote_term], n), cols, 0, n + 2);
	}

	/* Show a specific "title" -- header */
	c_put_str(TERM_YELLOW, special_line_header, 0, 0);

	/* Prompt */
	c_put_str(TERM_L_BLUE, "[Press any key to continue]", n + 3, 0);

	/* Ickify section of screen */
	section_icky_col = cols;
	section_icky_row = last_remote_line[p_ptr->remote_term] + 6;
}

void show_remote_peruse(s16b line)
{
	byte n;
	s16b last = last_remote_line[p_ptr->remote_term];
	byte st = window_to_stream[p_ptr->remote_term];
	int cols = p_ptr->stream_wid[st];

	/* This was giving me endless trouble, so I'm just
	 * putting it here --flm */
	Term_clear();

	/* Draw text */
	for (n = 2; n < Term->hgt-2; n++)
	{
		if (n + line - 2 > last || last < 0) break;
		caveprt(stream_cave(st, (n + line - 2)), cols, 0, n);
	}

	/* Erase the rest */
	for (; n < Term->hgt; n++)
	{
		Term_erase(0, n, cols);
	}

	/* Show a general "title" + header */
	special_line_header[60] = '\0';
	prt(format("[Mangband %d.%d.%d] %60s",CLIENT_VERSION_MAJOR,
	CLIENT_VERSION_MINOR, CLIENT_VERSION_PATCH, special_line_header), 0, 0);

	/* Prompt (check if we have extra pages) */
	if (last > Term->hgt - 6)
		prt("[Press Space to advance, or ESC to exit.]", Term->hgt - 1, 0);
	else
		prt("[Press ESC to exit.]", Term->hgt - 1, 0);
}

/* Save remote info into local ->file[][] array, to peruse
 * at own leisure */
void stash_remote_info(void)
{
	int j;
	s16b last_line = last_remote_line[p_ptr->remote_term];
	byte st = window_to_stream[p_ptr->remote_term];
	for (j = 0; j < last_line + 1; j++)
	{
		cavecpy(p_ptr->file[j], stream_cave(st, j), p_ptr->stream_wid[st]);
	}
	p_ptr->last_file_line = last_line;
}

/* Show portion of local ->file data */
void show_file_peruse(s16b line)
{
	byte n;
	s16b last = p_ptr->last_file_line;
	byte st = window_to_stream[NTERM_WIN_SPECIAL]; /* Ugh, what about others? */
	int cols = p_ptr->stream_wid[st]; /* TODO: Untangle from streams? */

	/* If we're not in perusal mode, ignore this */
	if (!special_line_onscreen) return;

	/* Clear screen */
	Term_clear();

	/* Draw text */
	for (n = 2; n < Term->hgt-2; n++)
	{
		if (n + line - 2 > last || last < 0) break;
		caveprt(p_ptr->file[n + line - 2], cols, 0, n);
	}

	/* Erase the rest */
	for (; n < Term->hgt; n++)
	{
		Term_erase(0, n, cols);
	}

	/* Show a general "title" + header */
	special_line_header[60] = '\0';
	prt(format("[Mangband %d.%d.%d] %60s",CLIENT_VERSION_MAJOR,
	CLIENT_VERSION_MINOR, CLIENT_VERSION_PATCH, special_line_header), 0, 0);

	/* Prompt (check if we have extra pages) */
	if (last > Term->hgt - 6)
		prt("[Press Space to advance, or ESC to exit.]", Term->hgt - 1, 0);
	else
		prt("[Press ESC to exit.]", Term->hgt - 1, 0);
}

/*
 * Peruse a file sent by the server.
 *
 * This is used by the artifact list, the unique list, the player
 * list, *Identify*, and Self Knowledge.
 *
 * It may eventually be used for help file perusal as well.
 */
void peruse_file(void)
{
	char k;

	/* Initialize */
	cur_line = 0;
	max_line = 0;

	/* The screen is icky */
	screen_icky = TRUE;

	/* Save the old screen */
	Term_save();

	/* Browser is on screen */
	special_line_onscreen = TRUE;

	/* Show the stuff */
	while (TRUE)
	{
		show_file_peruse(cur_line);

		/* Get a keypress */
		k = inkey();

		/* Hack -- update max line */
		max_line = p_ptr->last_file_line;

		/* Hack -- go to a specific line */
		if (k == '#')
		{
			char tmp[MAX_CHARS];
			prt("Goto Line: ", 23, 0);
			strcpy(tmp, "0");
			if (askfor_aux(tmp, MAX_COLS, 0))
			{
				cur_line = atoi(tmp);
			}
		}

		/* Back up half page */
		if (k == '_')
		{
			cur_line -= 10;
			if (cur_line < 0) cur_line = 0;
		}

		/* Back up one full page */
		if ((k == '-') || (k == '9'))
		{
			cur_line -= 20;
			if (cur_line < 0) cur_line = 0;
		}

		/* Advance to the bottom */
		if (k == '1' && max_line)
		{
			cur_line = max_line - 20;
			if (cur_line < 0) cur_line = 0;
		}

		/* Back up to the top */
		if (k == '7')
		{
			cur_line = 0;
		}

		/* Advance one line */
		if ((k == '\n') || (k == '\r') || (k == '2'))
		{
			cur_line++;
		}

		/* Back up one line */
		if ((k == '=') || (k == '8'))
		{
			cur_line--;
			if (cur_line < 0) cur_line = 0;
		}

		/* Advance one page */
		if (k == ' ' || k == '3')
		{
			cur_line += 20;
		}

		/* Exit on escape */
		if (k == ESCAPE) break;

		/* Check maximum line */
		if (cur_line > max_line - (Term->hgt - 6))
			cur_line = max_line - (Term->hgt - 6);

		/* Check minimum line */
		if (cur_line < 0)
			cur_line = 0;
	}

	/* No longer using file perusal */
	special_line_onscreen = FALSE;

	/* Reload the old screen */
	Term_load();

	/* The screen isn't icky anymore */
	screen_icky = FALSE;

	/* Flush any events that came in */
	Flush_queue();
}


/*
 * Client config file handler
 */
#ifdef WINDOWS
static char config_name[1024];	/* Config filename */
void conf_init(void* param)
{
	char path[1024];
	HINSTANCE hInstance = param;

	/* Search for file in user directory */
	if (GetEnvironmentVariable("USERPROFILE", path, 512))
	{
		my_strcat(path, "\\mangclient.ini", 1024);

		/* Ok */
		if (file_exists(path))
		{
			my_strcpy(config_name, path, 1024);
			return;
		}
	}

	/* Get full path to executable */
	GetModuleFileName(hInstance, path, 512);
	/* Remove ".exe" */
	path[strlen(path) - 4] = '\0';
	/* Remove ANGBAND_SYS suffix */
	/* if (suffix(path, ANGBAND_SYS)) path[strlen(path) - strlen(ANGBAND_SYS)] = '\0'; */
	if (suffix(path, "-sdl")) path[strlen(path) - 4] = '\0';
	if (suffix(path, "-sdl2")) path[strlen(path) - 5] = '\0';
	/* Append ".ini" */
	my_strcpy(config_name, path, 1024);
	my_strcat(config_name, ".ini", 1024);
}
void conf_save()
{ }
void conf_timer(int ticks)
{ }
void conf_done(void)
{ }
bool conf_section_exists(cptr section)
{
	char sections[1024];
	int n;
	size_t i;
	
	n = GetPrivateProfileSectionNames(sections, 1024, config_name);
	if (n != 1024 - 2)
	{
		for (i = 0; sections[i]; i += (strlen(&sections[i]) + 1))
			if (!my_stricmp(&sections[i], section))
				return TRUE;
	}

	return FALSE;
}
cptr conf_get_string(cptr section, cptr name, cptr default_value)
{
	static char value[100];
	GetPrivateProfileString(section, name, default_value,
	                        value, 100, config_name);
	return &value[0];
}
s32b conf_get_int(cptr section, cptr name, s32b default_value)
{
	return GetPrivateProfileInt(section, name, default_value, config_name);
}
void conf_set_string(cptr section, cptr name, cptr value)
{
	WritePrivateProfileString(section, name, value, config_name);
}
void conf_set_int(cptr section, cptr name, s32b value)
{
	char s_value[100];
	sprintf(s_value, "%" PRId32, value);
	WritePrivateProfileString(section, name, s_value, config_name);
}
/* HACK: Append section from other file */
void conf_append_section(cptr section, cptr filename)
{
	char keys[2024];
	char value[1024];
	int n;
	size_t i;

	/* Get all keys */
	n = GetPrivateProfileString(section, NULL, NULL, keys, 2024, filename);
	if (n != 2024 - 2)
	{
		for (i = 0; keys[i]; i += (strlen(&keys[i]) + 1))
		{
			/* Extract key */
			GetPrivateProfileString("Sound", &keys[i], "", value, sizeof(value), filename);
			/* MEGA-HACK: Append key to original config */
			value[100] = '\0'; /* FIXME: change "strings" len */
			conf_set_string(section, &keys[i], value);
		}
	}
}
#else
typedef struct value_conf_type value_conf_type;
typedef struct section_conf_type section_conf_type;
struct value_conf_type
{
	char name[100];
	char value[100];
	value_conf_type *next;	/* Next value in list */
};
struct section_conf_type
{
	char name[100];
	value_conf_type *first;	/* First value in list */
	section_conf_type *next;	/* Next section in list */
};
static section_conf_type *root_node = NULL;
static bool conf_need_save = FALSE;	/* Scheduled save */
static char config_name[1024];	/* Config filename */

/* Find a section by name */
section_conf_type* conf_get_section(cptr section)
{
	section_conf_type *s_ptr;
	for (s_ptr = root_node; s_ptr; s_ptr = s_ptr->next)
	{
		if ( !my_stricmp(section, s_ptr->name) )
		{
			return s_ptr;
		}
	}
	return NULL;
}
bool conf_section_exists(cptr section)
{
	if (conf_get_section(section) == NULL)
		return FALSE;

	return TRUE;
}
/* Add new section if it doesn't exist allready */
section_conf_type* conf_add_section_aux(cptr section)
{
	section_conf_type	*s_ptr;
	section_conf_type	*s_forge = NULL;

	/* Find section */
	s_ptr = conf_get_section(section);

	/* Not found */
	if (!s_ptr)
	{
		/* Forge new section */
		s_forge = 0;
		MAKE(s_forge, section_conf_type);

		/* Fill */
		my_strcpy(s_forge->name, section, 100);
		s_forge->next = NULL;
		s_forge->first = NULL;

		/* Attach */
		for (s_ptr = root_node; s_ptr->next; s_ptr = s_ptr->next) { }
		if (!s_ptr)
			root_node->next = s_forge;
		else
			s_ptr->next = s_forge;
		s_ptr = s_forge;

		conf_need_save = TRUE;
	}

	return s_ptr;
}
void conf_add_section(cptr section)
{
	conf_add_section_aux(section);
}
/* Change a "string" prefrence and schedule save */
void conf_set_string(cptr section, cptr name, cptr value)
{
	section_conf_type	*s_ptr = NULL;
	value_conf_type 	*v_ptr;
	value_conf_type 	*v_forge = NULL;
	bool done = FALSE;

	/* If section doesn't exist, create it */
	s_ptr = conf_get_section(section);
	if (!s_ptr)
		s_ptr = conf_add_section_aux(section);

	/* Find node to change */
	for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr->next)
	{
		if ( !my_stricmp(name, v_ptr->name) )
		{
			my_strcpy(v_ptr->value, value, 100);
			done = TRUE;
			break;
		}
	}

	/* Or create new node */
	if (!done)
	{
		/* Forge */
		v_forge = 0;
		MAKE(v_forge, value_conf_type);

		/* Fill */
		strcpy(v_forge->name, name);
		strcpy(v_forge->value, value);
		v_forge->next = NULL;
		
		/* Attach */
		if (s_ptr->first)
			for (v_ptr = s_ptr->first; v_ptr->next; v_ptr = v_ptr->next) { }
		if (!v_ptr)
			s_ptr->first = v_forge;
		else
			v_ptr->next = v_forge;

		done = TRUE;
	}

	if (done) conf_need_save = TRUE;
}
/* Change an "integer" value. All values are stored as strings. */
void conf_set_int(cptr section, cptr name, s32b value)
{
	char s_value[100];
	sprintf(s_value, "%" PRId32, value);
	conf_set_string(section, name, s_value);
}
/*
 * Return value from section "section" , with name "name"
 * For string values, a "cptr" is returned, for integers "int".
 *
 * Not recommended for external usage, use "conf_get_int" and
 * "conf_get_string" instead.
 */
long conf_get_value(cptr section, cptr name, cptr default_value, bool is_int)
{
	section_conf_type	*s_ptr;
	value_conf_type 	*v_ptr;

	for (s_ptr = root_node; s_ptr; s_ptr = s_ptr->next)
	{
		if ( !my_stricmp(section, s_ptr->name) )
		{
			for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr->next)
			{
				if ( !my_stricmp(name, v_ptr->name) )
				{
					if (is_int)
						return atoi(v_ptr->value);
					return (long)v_ptr->value;
				}
			}
		}
	}
	if (is_int)
		return atoi(default_value);
	return (long)default_value;
}
s32b conf_get_int(cptr section, cptr name, s32b default_value)
{
	static char v_value[100];
	sprintf(v_value, "%" PRId32, default_value);
	return (u32b)conf_get_value(section, name, v_value, TRUE);
}
cptr conf_get_string(cptr section, cptr name, cptr default_value)
{
	return (cptr)conf_get_value(section, name, default_value, FALSE);
}
void conf_read_file(ang_file* config, section_conf_type *s_ptr, value_conf_type *v_ptr)
{
	section_conf_type	*s_forge = NULL;
	value_conf_type 	*v_forge = NULL;

	char buf[1024];
	char s_name[100], *name, *value;
	int n;

	/* File is opened (paranoia) */
	if (config)
	{
		/* Read line (till end of file) */
		while (file_getl(config, buf, 1024))
		{
			/* Skip comments, empty lines */
			if (buf[0] == '\n' || buf[0] == '#' || buf[0] == ';')
				continue;

			/* Probably a section */
			if (buf[0] == '[')
			{
				/* Trim */
				for(n = strlen(buf);
			 	((buf[n] == '\n' || buf[n] == '\r' || buf[n] == ' ' || !buf[n]) && n > 1);
			 	n--)	{ 	}

				/* Syntax is correct */
				if (buf[n] == ']' && n > 1)
				{
					/* Get name */
					buf[n] = '\0';
					strcpy(s_name, buf + 1);
					
					/* New section */
					if (!conf_section_exists(s_name)) 
					{
						/* Forge new section */
						s_forge = 0;
						MAKE(s_forge, section_conf_type);

						/* Fill */
						strcpy(s_forge->name, s_name);
						s_forge->next = NULL;
						s_forge->first = NULL;
						s_ptr->next = s_forge;
						s_ptr = s_forge;

						/* Attach */
						v_ptr = s_ptr->first;

						/* Done */
						continue;
					}
				}
				/* Malformed entry, skip */
				continue;
			}

			/* Attempt to read a value */
			name	= strtok(buf, " =\t\n");
			value	= strtok(NULL, " =\t\n");

			/* Read something */
			if (name && value)
			{
				/* Forge new node */
				v_forge = 0;
				MAKE(v_forge, value_conf_type);

				/* Fill */
				strcpy(v_forge->name, name);
				strcpy(v_forge->value, value);
				v_forge->next = NULL;

				/* Attach */
				if (!v_ptr)
					s_ptr->first = v_forge;
				else
					v_ptr->next = v_forge;

				/* Advance */
				v_ptr = v_forge;
			}
		}
	}
} 
/* Initialize global config tree */
void conf_init(void* param)
{
	section_conf_type	*s_ptr = NULL;
	value_conf_type 	*v_ptr = NULL;

	ang_file* config;
	char buf[1024];

	/*
	 * Prepare root node
	 */

	/* Forge root */
	if (!root_node)
		MAKE(root_node, section_conf_type);

	/* Prepare */
	strcpy(root_node->name, "MAngband");
	root_node->next = NULL;
	root_node->first = NULL;

	/* Attach */
	s_ptr = root_node;
	v_ptr = root_node->first;

	/* We start with closed file */
	config = NULL;

	/* Try to get path to config file from command-line "--config" option */
	if (clia_read_string(config_name, 1024, "config"))
	{
		/* Attempt to open file */
		config = file_open(config_name, MODE_READ, -1);
	}

	/*
	 * Get File name 
	 */

	/* EMX Hack */
#ifdef USE_EMX
	strcpy(buf, "\\mang.rc");
#else
	strcpy(buf, "/.mangrc");
#endif

	/* Hack -- make this file easier to find */
#if defined(__APPLE__) || defined(ON_XDG)
	strcpy(buf, "/mangclient.ini");
#endif

	/* Try to find home directory */
	if (!config && getenv("HOME"))
	{
		/* Use home directory as base */
		my_strcpy(config_name, getenv("HOME"), 1024);

		/* Append filename */
		my_strcat(config_name, buf, 1024);

		/* Attempt to open file */
		config = file_open(config_name, MODE_READ, -1);
	}

	/* Otherwise use current directory */
	if (!config)
	{
		/* Current directory */
		my_strcpy(config_name, ".", 1024);

		/* Append filename */
		my_strcat(config_name, buf, 1024);

		/* Attempt to open file */
		config = file_open(config_name, MODE_READ, -1);
	}

#if defined(ON_IOS) || (defined(ON_OSX) && !defined(HAVE_CONFIG_H))
	if (!config)
	{
		/* Application Support directory */
		appl_get_appsupport_dir(config_name, 1024, TRUE);

		/* Append filename */
		my_strcat(config_name, buf, 1024);

		/* Attempt to open file */
		config = file_open(config_name, MODE_READ, -1);
    }
#endif

	/*
	 * Read data
	 */

	/* File is opened */
	if (config)
	{
		/* Use auxilary function */
		conf_read_file(config, s_ptr, v_ptr);

		/* Done reading */
		file_close(config);
	}
#if 0
	//list all sections
	for (s_ptr = root_node; s_ptr; s_ptr = s_ptr->next)
	{
		printf("[%s]\n", s_ptr->name);
		//list all values
		for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr->next)
		{
			printf("  %s = %s\n", v_ptr->name, v_ptr->value);
		}
	}
#endif
}
/* Destroy */
void conf_done()
{
	section_conf_type	*s_ptr = NULL;
	value_conf_type 	*v_ptr = NULL;

	section_conf_type	*s_ptr_next = NULL;
	value_conf_type 	*v_ptr_next = NULL;

	/* Delete all sections */
	for (s_ptr = root_node; s_ptr; s_ptr = s_ptr_next)
	{
		/* Delete all nodes */
		for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr_next)
		{
			v_ptr_next = v_ptr->next;
			FREE(v_ptr);
		}
		s_ptr_next = s_ptr->next;
		FREE(s_ptr);
	}
}
/* Save config file if it is scheduled */
void conf_save()
{
	section_conf_type *s_ptr;
	value_conf_type 	*v_ptr;
	ang_file* config;

	/* No changes */
	if (!conf_need_save) return;

	/* Write */
	if ((config = file_open(config_name, MODE_WRITE, FTYPE_TEXT)))
	{
		for (s_ptr = root_node; s_ptr; s_ptr = s_ptr->next)
		{
			file_putf(config, "[%s]\n", s_ptr->name);
			for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr->next)
			{
				file_putf(config, "%s %s\n", v_ptr->name, v_ptr->value);
			}
			if (s_ptr->next)
				file_putf(config, "\n");
		}
		/* Done writing */
		file_close(config);
		conf_need_save = FALSE;
	}
}
/* Scheduler */
void conf_timer(int ticks)
{
	static int last_update = 0;
	if ((ticks - last_update) > 600) /* 60 seconds? */
	{
		conf_save();
		last_update = ticks;
	}
}
/* HACK: Append section from other file */
void conf_append_section(cptr section, cptr filename)
{
	ang_file* config;

	section_conf_type *s_ptr;
	value_conf_type 	*v_ptr;

	/* Find pointers */
	s_ptr = conf_add_section_aux(section);
	for (v_ptr = s_ptr->first; v_ptr; v_ptr = v_ptr->next) { }

	/* Try opening this 'other file' */
	config = file_open(filename, MODE_READ, -1);

 	/* File is opened */
	if (config)
	{
		/* Use auxilary function */
		conf_read_file(config, s_ptr, v_ptr);

		/* Done reading */
		file_close(config);
	}
}
#endif

int p_argc = 0;
const char **p_argv = NULL;
void clia_init(int argc, const char **argv)
{
	/* If it's unsafe, we'll just copy */
	p_argc = argc;
	p_argv = argv;
}
int clia_find(const char *key)
{
	int i;
	bool awaiting_argument = FALSE;
	bool key_matched = FALSE;
	bool got_hostname = FALSE;
	for (i = 1; i < p_argc; i++)
	{
		if (prefix(p_argv[i], "--"))
		{
			const char *c = &p_argv[i][2];
			if (awaiting_argument && key_matched)
			{
				/* Hack -- if this is second --longopt in a row, and the
				 * last one was matching our key, assume we're done! */
				return i - 1;
			}
			awaiting_argument = TRUE;
			key_matched = FALSE;
			if (!STRZERO(c) && streq(key, c))
			{
				key_matched = TRUE;
			}
		}
		else
		{
			if (awaiting_argument)
			{
				awaiting_argument = FALSE;
				if (key_matched)
				{
					/* Found */
					return i;
				}
			}
			else
			{
				/* Hack -- Ignore MacOSX `-psn_APPID` handle (see #989) */
				if (prefix(p_argv[i], "-psn_")) continue;

				/* Starts with an "-", could be a short option. */
				if (p_argv[i][0] == '-')
				{
					if (
					    (prefix(p_argv[i], "-duser=") && streq(key, "userdir"))
					 || (prefix(p_argv[i], "-dbone=") && streq(key, "bonedir"))
					) {
						/* "Found" */
						return i;
					}
				}
				/* Could be hostname */
				else if (i == p_argc - 1 || (i == p_argc - 2 && !got_hostname))
				{
					if (!got_hostname)
					{
						got_hostname = TRUE;
						/* Host name */
						if (streq(key, "host"))
						{
							/* Found */
							return i;
						}
					}
					else
					{
						/* Port! */
						/* ... */
						if (streq(key, "port"))
						{
							/* Found */
							return i;
						}
					}
				}
				else
				{
					/* Error */
				}
			}
		}
	}
	/* Hack -- if we're left with a dangling --longopt, and it matched our key,
	 * assume it's a toggle option, and mark it as found */
	if (awaiting_argument && key_matched)
	{
		return i - 1;
	}
	return -1;
}
bool clia_cpy_string(char *dst, int len, int i)
{
	if (i > 0 && i < p_argc)
	{
		my_strcpy(dst, p_argv[i], len);
		return TRUE;
	}
	return FALSE;
}
bool clia_cpy_int(s32b *dst, int i)
{
	if (i > 0 && i < p_argc)
	{
		*dst = atoi(p_argv[i]);
		return TRUE;
	}
	return FALSE;
}
bool clia_read_bool(bool *dst, const char *key)
{
	int i = clia_find(key);
	if (i > 0 && i < p_argc)
	{
		const char *ckey;
		if (strlen(p_argv[i]) > 2)
		{
			/* It was a toggle option */
			ckey = p_argv[i] + 2;
			if (streq(key, ckey))
			{
				*dst = 1;
				return TRUE;
			}
		}
		if (streq(p_argv[i], "true") || streq(p_argv[i], "on"))
		{
			*dst = 1;
			return TRUE;
		}
		if (streq(p_argv[i], "false") || streq(p_argv[i], "off"))
		{
			*dst = 0;
			return TRUE;
		}
	}
	return FALSE;
}
bool clia_read_string(char *dst, int len, const char *key)
{
	int i = clia_find(key);
	return clia_cpy_string(dst, len, i);
}
bool clia_read_int(s32b *dst, const char *key)
{
	int i = clia_find(key);
	return clia_cpy_int(dst, i);
}

/* Copy all *.prf files from the distribution to some
 * user-writable directory. Essential on some platforms.
 * Note, that on macOS/OSX, this is handled by "launch_client.sh",
 * so we shouldn't be calling this. On iOS, though, this is required!
 *
 * TODO: we can probably also use this on Windows and Linux,
 * e.g. copy from /usr/share/games/user/<*>.prf to ~/.mangclient/user
 * or what-have-you.
 *
 * Note: after completing this operation, the ANGBAND_DIR_USER is
 * switched to the new, writable location!
 */
void import_user_pref_files(cptr dest_path)
{
	char tmp_dest_path[PATH_MAX];
	char tmp_src_path[PATH_MAX];
	char tmp_fname[PATH_MAX];

	/* Open source directory */
	ang_dir *src_dir = my_dopen(ANGBAND_DIR_USER);
	if (src_dir == NULL)
	{
		plog_fmt("Could not open directory '%s' for reading.", ANGBAND_DIR_USER);
		return;
	}

	/* Iterate over files */
	while (my_dread(src_dir, tmp_fname, PATH_MAX))
	{
		/* It's a .prf file */
		if (isuffix(tmp_fname, ".prf"))
		{
			path_build(tmp_src_path, PATH_MAX, ANGBAND_DIR_USER, tmp_fname);
			path_build(tmp_dest_path, PATH_MAX, dest_path, tmp_fname);
			if (file_exists(tmp_dest_path)) continue; /* Don't overwrite */
			file_copy(tmp_src_path, tmp_dest_path, FTYPE_TEXT);
		}
	}
	/* Done, close source directory */
	my_dclose(src_dir);

	/* Now, let's switch the path */
	string_free(ANGBAND_DIR_USER);
	ANGBAND_DIR_USER = string_make(dest_path);
}
