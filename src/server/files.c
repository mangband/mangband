/* File: files.c */

/* Purpose: code dealing with files (and death) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"

#ifdef HANDLE_SIGNALS
#include <signal.h>

volatile sig_atomic_t signalbusy = 0;

#endif



/*
 * You may or may not want to use the following "#undef".
 */
/* #undef _POSIX_SAVED_IDS */


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
			if (*t == '\'') /* ' */
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
				if (*t != '\'') *t = '\'';
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


bool skip_next_line = FALSE; 
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
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a normal macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a command macro, given an encoded macro trigger
 *   C:<str>
 *
 * Create a keyset mapping
 *   S:<key>:<key>:<dir>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 */
errr process_pref_file_aux(char *buf)
{
	int i, j, k, n1, n2;

	char *zz[16];

	/* Skip "empty" && "blank" lines */
	if (!buf[0] || isspace(buf[0])) 
	{
			skip_next_line = FALSE;	
			return (0);
	}

	/* Skip comments */
	if (buf[0] == '#') return (0);


	/* Require "?:*" format */
	if (buf[1] != ':') return (1);

	/* Hack - Do not load any Evaluated Expressions */
	if (skip_next_line)		return(0);

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
			monster_race *r_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= z_info->r_max) return (1);
			/*r_ptr = &r_info[i];
			if (n1) r_ptr->x_attr = n1;
			if (n2) r_ptr->x_char = n2;*/
			if (n1) r_attr_s[i] = n1;
			if (n2) r_char_s[i] = n2;
			return (0);
		}
	}


	/* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
	else if (buf[0] == 'K')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			object_kind *k_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= z_info->k_max) return (1);
			/*k_ptr = &k_info[i];
			if (n1) k_ptr->x_attr = n1;
			if (n2) k_ptr->x_char = n2;*/
			if (n1) k_attr_s[i] = n1;
			if (n2) k_char_s[i] = n1;
			return (0);
		}
	}


	/* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
	else if (buf[0] == 'F')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			feature_type *f_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= z_info->f_max) return (1);
			/*f_ptr = &f_info[i];
			if (n1) f_ptr->x_attr = n1;
			if (n2) f_ptr->x_char = n2;*/
			if (n1) f_attr_s[i] = n1;
			if (n2) f_char_s[i] = n2;
			return (0);
		}
	}


	/* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
	else if (buf[0] == 'U')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			j = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			for (i = 1; i < z_info->k_max; i++)
			{
				object_kind *k_ptr = &k_info[i];
				if (k_ptr->tval == j)
				{
					if (n1) k_ptr->d_attr = n1;
					if (n2) k_ptr->d_char = n2;
				}
			}
			return (0);
		}
	}


	/* Process "E:<tv>:<a>/<c>" -- attr/char for equippy chars */
	else if (buf[0] == 'E')
	{
		if (tokenize(buf+2, 2, zz) == 2)
		{
			j = (byte)strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			if (n1) tval_to_attr[j] = n1;
			return (0);
		}
		if (tokenize(buf+2, 3, zz) == 3)
		{
			j = (byte)strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (n1) tval_to_attr[j] = n1;
			if (n2) tval_to_char[j] = n2;
			return (0);
		}
	}


	/* Process "A:<str>" -- save an "action" for later */
	else if (buf[0] == 'A')
	{
		text_to_ascii(macro__buf, buf+2);
		return (0);
	}

	/* Process "P:<str>" -- create normal macro */
	else if (buf[0] == 'P')
	{
		char tmp[1024];
		text_to_ascii(tmp, buf+2);
		macro_add(tmp, macro__buf, FALSE);
		return (0);
	}

	/* Process "C:<str>" -- create command macro */
	else if (buf[0] == 'C')
	{
		char tmp[1024];
		text_to_ascii(tmp, buf+2);
		macro_add(tmp, macro__buf, TRUE);
		return (0);
	}


	/* Process "S:<num>:<a>/<c>" -- attr/char for special things */
	else if (buf[0] == 'S')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= (long)N_ELEMENTS(misc_to_attr))) return (1);
			misc_to_attr[i] = (byte)n1;
			misc_to_char[i] = (char)n2;
			return (0);
		}

	}
	/* Process "S:<key>:<key>:<dir>" -- keymap */		
	else if (buf[0] == 'S')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			i = strtol(zz[0], NULL, 0) & 0x7F;
			j = strtol(zz[0], NULL, 0) & 0x7F;
			k = strtol(zz[0], NULL, 0) & 0x7F;
			if ((k > 9) || (k == 5)) k = 0;
			keymap_cmds[i] = j;
			keymap_dirs[i] = k;
			return (0);
		}
	}


	/* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
	else if (buf[0] == 'V')
	{
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
	else if (buf[0] == 'X')
	{
		return (0);
	}

	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'Y')
	{
		return (0);
	}

	/* Process "W:<num>:<use> -- set window use */
	else if (buf[0] == 'W')
	{
		return (0);
	}
	
	
	/* Process "?: -- expression */
	else if (buf[0] == '?')
	{
		skip_next_line = TRUE;
		return (0);
	}

	/* Failure */
	return (1);
}


/*
 * Process the "user pref file" with the given name
 *
 * See the function above for a list of legal "commands".
 */
errr process_pref_file(cptr name)
{
	ang_file* fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_PREF, name);

	/* Open the file */
	fp = file_open(buf, MODE_READ, -1);

	/* Catch errors */
	if (!fp) return (-1);

	/* Process the file */
	while (file_getl(fp, buf, 1024))
	{
		/* Process the line */
		if (process_pref_file_aux(buf))
		{
			/* Useful error message */
			plog(format("Error in '%s' parsing '%s'.", buf, name));
		}
	}

	/* Close the file */
	file_close(fp);

	/* Success */
	return (0);
}







#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] =
{
	"SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
	"MON:XXXXXXXX.........XXXXXXX",
	"TUE:XXXXXXXX.........XXXXXXX",
	"WED:XXXXXXXX.........XXXXXXX",
	"THU:XXXXXXXX.........XXXXXXX",
	"FRI:XXXXXXXX.........XXXXXXX",
	"SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

	time_t              c;
	struct tm		*tp;

	/* No restrictions */
	if (!check_time_flag) return (0);

	/* Check for time violation */
	c = time((time_t *)0);
	tp = localtime(&c);

	/* Violation */
	if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);

#endif

	/* Success */
	return (0);
}



/*
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

	ang_file* fp;

	char	buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "time.txt");

	/* Open the file */
	fp = file_open(buf, MODE_READ, -1);

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Assume restrictions */
	check_time_flag = TRUE;

	/* Parse the file */
	while (file_getl(fp, buf, 80))
	{
		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Chop the buffer */
		buf[29] = '\0';

		/* Extract the info */
		if (prefix(buf, "SUN:")) strcpy(days[0], buf);
		if (prefix(buf, "MON:")) strcpy(days[1], buf);
		if (prefix(buf, "TUE:")) strcpy(days[2], buf);
		if (prefix(buf, "WED:")) strcpy(days[3], buf);
		if (prefix(buf, "THU:")) strcpy(days[4], buf);
		if (prefix(buf, "FRI:")) strcpy(days[5], buf);
		if (prefix(buf, "SAT:")) strcpy(days[6], buf);
	}

	/* Close it */
	file_close(fp);

#endif

	/* Success */
	return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN  64
#endif

typedef struct statstime statstime;

struct statstime
{
	int                 cp_time[4];
	int                 dk_xfer[4];
	unsigned int        v_pgpgin;
	unsigned int        v_pgpgout;
	unsigned int        v_pswpin;
	unsigned int        v_pswpout;
	unsigned int        v_intr;
	int                 if_ipackets;
	int                 if_ierrors;
	int                 if_opackets;
	int                 if_oerrors;
	int                 if_collisions;
	unsigned int        v_swtch;
	long                avenrun[3];
	struct timeval      boottime;
	struct timeval      curtime;
};

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

	struct statstime    st;

	/* Success if not checking */
	if (!check_load_value) return (0);

	/* Check the load */
	if (0 == rstat("localhost", &st))
	{
		long val1 = (long)(st.avenrun[2]);
		long val2 = (long)(check_load_value) * FSCALE;

		/* Check for violation */
		if (val1 >= val2) return (1);
	}

#endif

	/* Success */
	return (0);
}


/*
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

	ang_file* fp;

	char	buf[1024];

	char	temphost[MAXHOSTNAMELEN+1];
	char	thishost[MAXHOSTNAMELEN+1];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_TEXT, "load.txt");

	/* Open the "load" file */
	fp = file_open(buf, MODE_READ, -1);

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Default load */
	check_load_value = 100;

	/* Get the host name */
	(void)gethostname(thishost, (sizeof thishost) - 1);

	/* Parse it */
	while (file_getl(fp, buf, 1024))
	{
		int value;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Parse, or ignore */
		if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

		/* Skip other hosts */
		if (!streq(temphost, thishost) &&
		    !streq(temphost, "localhost")) continue;

		/* Use that value */
		check_load_value = value;

		/* Done */
		break;
	}

	/* Close the file */
	file_close(fp);

#endif

	/* Success */
	return (0);
}





/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 *
 * This will send the info to the client now --KLJ--
 *
 * Except that this (and display_player) are never called. --KLJ--
 */
static void display_player_middle(player_type *p_ptr)
{
	int show_tohit = p_ptr->dis_to_h;
	int show_todam = p_ptr->dis_to_d;

	s32b adv_exp;

	object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Hack -- add in weapon info if known */
	if (object_known_p(p_ptr, o_ptr)) show_tohit += o_ptr->to_h;
	if (object_known_p(p_ptr, o_ptr)) show_todam += o_ptr->to_d;

	/* Dump the bonuses to hit/dam */
	//Send_plusses(Ind, show_tohit, show_todam);

	/* Dump the armor class bonus */
	//Send_ac(Ind, p_ptr->dis_ac, p_ptr->dis_to_a);

	if (p_ptr->lev >= PY_MAX_LEVEL)
		adv_exp = 0;
	else adv_exp = (s32b)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L);

	//Send_experience(Ind, p_ptr->lev, p_ptr->max_exp, p_ptr->exp, adv_exp);

	//Send_gold(Ind, p_ptr->au);

	//Send_hp(Ind, p_ptr->mhp, p_ptr->chp);

	//Send_sp(Ind, p_ptr->msp, p_ptr->csp);
}



/*
 * Display the character on the screen (with optional history)
 *
 * The top two and bottom two lines are left blank.
 */
void display_player(player_type *p_ptr)
{
	int i;


	/* Send basic information */
	//Send_char_info(Ind, p_ptr->prace, p_ptr->pclass, p_ptr->male);

	/* Age, Height, Weight, Social */
	//Send_various(Ind, p_ptr->ht, p_ptr->wt, p_ptr->age, p_ptr->sc);

	/* Send all the stats */
	for (i = 0; i < A_MAX; i++)
	{
		//Send_stat(Ind, i, p_ptr->stat_top[i], p_ptr->stat_use[i]);
		//Send_maxstat(Ind, i, p_ptr->stat_max[i]);
	}

	/* Extra info */
	display_player_middle(p_ptr);

	/* Display "history" info */
	//Send_history(Ind, i, p_ptr->history[i]);
}


void c_put_str_b(char buffer[100][82], byte attr, cptr str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	char* s;
	int i;
	s = buffer[row-1];
	i = col;
	while(*str)
	{
		s[i++] = (*str);
		str++;
	}
}
void put_str_b(char buffer[100][82], cptr str, int row, int col)
{
	c_put_str_b(buffer,0,str,row,col);
}
void prt_num_b(char buffer[100][82], cptr header, int num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str_b(buffer,header, row, col);
	put_str_b(buffer,"   ", row, col + len);
	(void)sprintf(out_val, "%6ld", (long)num);
	c_put_str_b(buffer,color, out_val, row, col + len + 3);
}

void prt_lnum_b(char buffer[100][82], cptr header, s32b num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str_b(buffer,header, row, col);
	(void)sprintf(out_val, "%9ld", (long)num);
	c_put_str_b(buffer,color, out_val, row, col + len);
}

/*
 * Returns a "rating" of x depending on y
 */
static cptr likert(int x, int y)
{
	/* Paranoia */
	if (y <= 0) y = 1;

	/* Negative values */
	if (x < 0)
	{
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
		case 0:
		case 1:
		{
			return ("Bad");
		}
		case 2:
		{
			return ("Poor");
		}
		case 3:
		case 4:
		{
			return ("Fair");
		}
		case 5:
		{
			return ("Good");
		}
		case 6:
		{
			return ("Very Good");
		}
		case 7:
		case 8:
		{
			return ("Excellent");
		}
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			return ("Superb");
		}
		case 14:
		case 15:
		case 16:
		case 17:
		{
			return ("Heroic");
		}
		default:
		{
			return ("Legendary");
		}
	}
}


/*
 * Similar to the function in c-xtra but modified to work server-side.
 * We print text into a buffer rather than to a term.
 * This is used for serverside character dumps.
 */
void display_player_server(player_type *p_ptr, char buffer[100][82])
{
	int i;
	char buf[80];
	cptr desc;
/*	bool hist = FALSE;*/

	int show_tohit = p_ptr->dis_to_h;
	int show_todam = p_ptr->dis_to_d;
	object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Name, Sex, Race, Class */
	put_str_b(buffer,"Name        :", 2, 1);
	put_str_b(buffer,"Sex         :", 3, 1);
	put_str_b(buffer,"Race        :", 4, 1);
	put_str_b(buffer,"Class       :", 5, 1);

	c_put_str_b(buffer,TERM_L_BLUE, p_ptr->name, 2, 15);
	c_put_str_b(buffer,TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
	c_put_str_b(buffer,TERM_L_BLUE, p_name + p_info[p_ptr->prace].name, 4, 15);
	c_put_str_b(buffer,TERM_L_BLUE, c_name + c_info[p_ptr->pclass].name, 5, 15);

	/* Age, Height, Weight, Social */
	prt_num_b(buffer,"Age          ", (int)p_ptr->age, 2, 32, TERM_L_BLUE);
	prt_num_b(buffer,"Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE);
	prt_num_b(buffer,"Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE);
	prt_num_b(buffer,"Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE);

	/* Display the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Special treatment of "injured" stats */
		if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
		{
			int value;

			/* Use lowercase stat name */
			put_str_b(buffer,stat_names_reduced[i], 2 + i, 61);

			/* Get the current stat */
			value = p_ptr->stat_use[i];

			/* Obtain the current stat (modified) */
			cnv_stat(value, buf);

			/* Display the current stat (modified) */
			c_put_str_b(buffer,TERM_YELLOW, buf, 2 + i, 66);

			/* Acquire the max stat */
			value = p_ptr->stat_top[i];

			/* Obtain the maximum stat (modified) */
			cnv_stat(value, buf);

			/* Display the maximum stat (modified) */
			if (p_ptr->stat_max[i] == 18+100)
				c_put_str_b(buffer,TERM_L_UMBER, buf, 2 + i, 73);
			else
				c_put_str_b(buffer,TERM_L_GREEN, buf, 2 + i, 73);
		}

		/* Normal treatment of "normal" stats */
		else
		{
			/* Assume uppercase stat name */
			put_str_b(buffer,stat_names[i], 2 + i, 61);

			/* Obtain the current stat (modified) */
			cnv_stat(p_ptr->stat_use[i], buf);

			/* Display the current stat (modified) */
			if (p_ptr->stat_max[i] == 18+100)
				c_put_str_b(buffer,TERM_L_UMBER, buf, 2 + i, 66);
			else
				c_put_str_b(buffer,TERM_L_GREEN, buf, 2 + i, 66);
		}
	}

	put_str_b(buffer,"(Miscellaneous Abilities)", 15, 25);

	/* Display "skills" */
	put_str_b(buffer,"Fighting    :", 16, 1);
	desc = likert(p_ptr->skill_thn, 12);
	c_put_str_b(buffer,0, desc, 16, 15);

	put_str_b(buffer,"Bows/Throw  :", 17, 1);
	desc = likert(p_ptr->skill_thb, 12);
	c_put_str_b(buffer,0, desc, 17, 15);

	put_str_b(buffer,"Saving Throw:", 18, 1);
	desc = likert(p_ptr->skill_sav, 6);
	c_put_str_b(buffer,0, desc, 18, 15);

	put_str_b(buffer,"Stealth     :", 19, 1);
	desc = likert(p_ptr->skill_stl, 1);
	c_put_str_b(buffer,0, desc, 19, 15);


	put_str_b(buffer,"Perception  :", 16, 28);
	desc = likert(p_ptr->skill_fos, 6);
	c_put_str_b(buffer,0, desc, 16, 42);

	put_str_b(buffer,"Searching   :", 17, 28);
	desc = likert(p_ptr->skill_srh, 6);
	c_put_str_b(buffer,0, desc, 17, 42);

	put_str_b(buffer,"Disarming   :", 18, 28);
	desc = likert(p_ptr->skill_dis, 8);
	c_put_str_b(buffer,0, desc, 18, 42);

	put_str_b(buffer,"Magic Device:", 19, 28);
	desc = likert(p_ptr->skill_dev, 6);
	c_put_str_b(buffer,0, desc, 19, 42);


	put_str_b(buffer,"Blows/Round:", 16, 55);
	put_str_b(buffer,format("%d", p_ptr->num_blow), 16, 69);

	put_str_b(buffer,"Shots/Round:", 17, 55);
	put_str_b(buffer,format("%d", p_ptr->num_fire), 17, 69);

	put_str_b(buffer,"Infra-Vision:", 19, 55);
	put_str_b(buffer,format("%d feet", p_ptr->see_infra * 10), 19, 69);

	/* Dump the bonuses to hit/dam */
	if (o_ptr->k_idx)
	{
		show_tohit += o_ptr->to_h;
		show_todam += o_ptr->to_d;
	}
	prt_num_b(buffer,"+ To Hit    ", show_tohit, 9, 1, TERM_L_BLUE);
	prt_num_b(buffer,"+ To Damage ", show_todam, 10, 1, TERM_L_BLUE);

	/* Dump the armor class bonus */
	prt_num_b(buffer,"+ To AC     ", p_ptr->dis_to_a, 11, 1, TERM_L_BLUE);

	/* Dump the total armor class */
	prt_num_b(buffer,"  Base AC   ", p_ptr->dis_ac, 12, 1, TERM_L_BLUE);

	prt_num_b(buffer,"Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN);

	if (p_ptr->exp >= p_ptr->max_exp)
	{
		prt_lnum_b(buffer,"Experience ", p_ptr->exp, 10, 28, TERM_L_GREEN);
	}
	else
	{
		prt_lnum_b(buffer,"Experience ", p_ptr->exp, 10, 28, TERM_YELLOW);
	}

	prt_lnum_b(buffer,"Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);

	if (p_ptr->lev >= PY_MAX_LEVEL)
	{
		put_str_b(buffer,"Exp to Adv.", 12, 28);
		c_put_str_b(buffer,TERM_L_GREEN, "    *****", 12, 28+11);
	}
	else
	{
		/* prt_lnum_b(buffer,"Exp to Adv.", p_ptr->exp_adv, 12, 28, TERM_L_GREEN); */
	}

	prt_lnum_b(buffer,"Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);

	prt_num_b(buffer,"Max Hit Points ", p_ptr->mhp, 9, 52, TERM_L_GREEN);

	if (p_ptr->chp >= p_ptr->mhp)
	{
		prt_num_b(buffer,"Cur Hit Points ", p_ptr->chp, 10, 52, TERM_L_GREEN);
	}
	else if (p_ptr->chp > (p_ptr->mhp) / 10)
	{
		prt_num_b(buffer,"Cur Hit Points ", p_ptr->chp, 10, 52, TERM_YELLOW);
	}
	else
	{
		prt_num_b(buffer,"Cur Hit Points ", p_ptr->chp, 10, 52, TERM_RED);
	}

	prt_num_b(buffer,"Max SP (Mana)  ", p_ptr->msp, 11, 52, TERM_L_GREEN);

	if (p_ptr->csp >= p_ptr->msp)
	{
		prt_num_b(buffer,"Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_L_GREEN);
	}
	else if (p_ptr->csp > (p_ptr->msp) / 10)
	{
		prt_num_b(buffer,"Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_YELLOW);
	}
	else
	{
		prt_num_b(buffer,"Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_RED);
	}

	/* Check for history */
	put_str_b(buffer, "(Character Background)", 21, 25);

	for (i = 0; i < 4; i++)
	{
		put_str_b(buffer,p_ptr->history[i], i + 22, 10);
	}
}


/*
 * Hack -- Dump a character description file
 * This is for server-side character dumps
 */
errr file_character_server(player_type *p_ptr, cptr name)
{
	int		i, j, x, y, x1, x2, y1, y2;
	byte		a;
	char		c, attr;
	cptr		paren = ")";
	ang_file*	fff = NULL;
	char		o_name[80];
	char		today[10];
	char		buf[1024];
	cave_view_type status[80];
	char		buffer[100][82];
	time_t ct = time((time_t*)0);

	// Init buffer...
	for(i=0;i<100;i++)
	{
		for(x=0;x<80;x++)
			buffer[i][x] = ' ';
		buffer[i][80] = '\0';
	}

	/* Drop priv's 
	safe_setuid_drop(); */

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_BONE, name);

	/* Grab priv's 
	safe_setuid_grab(); */
	
	/* Open the non-existing file */
	if (!file_exists(buf))
	{
		fff = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	}

	/* Invalid file */
	if (!fff)
	{
		/* Error */
		return (-1);
	}

	/* Add ladder information, this line is used by the online ladder and 
	 * not displayed when viewing a character dump online.
	 */
	strftime(today, 9, "%m/%d/%y", localtime(&ct));
	file_putf(fff, "# %lu|%lu|%-.8s|%-.25s|%c|%2d|%2d|%3d|%3d|%3d|%3d|%-.31s|%d.%d.%d\n",
		(long)total_points(p_ptr),
		(long)p_ptr->au,
		today,
		p_ptr->name,
		p_ptr->male ? 'm' : 'f',
		p_ptr->prace,
		p_ptr->pclass,
		p_ptr->lev,
		p_ptr->died_from_depth,
		p_ptr->max_plv,
		p_ptr->max_dlv,
		p_ptr->died_from_list,
		SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);

#ifndef DEBUG
	/* Leave it at that for characters lower than level 20 */
	if( p_ptr->lev < 20 )
	{
		/* Close dump file */
		file_close(fff);

		/* Success */
		return (0);	
	}
#endif

	/* Begin dump */
	if (cfg_ironman)
		file_putf(fff, "  [Ironman Mangband %d.%d.%d Character Dump]\n\n",
		        SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);
	else
		file_putf(fff, "  [Mangband %d.%d.%d Character Dump]\n\n",
		        SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);

	/* Display the player info */
	display_player_server(p_ptr, buffer);

	/* Dump the buffer */
	for(i=0;i<26;i++)
	{
		file_putf(fff,"%s\n",buffer[i]);
	}

	/* Dump the equipment */
	file_putf(fff, "%s", "  [Character Equipment]\n\n");
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_desc(0, o_name, sizeof(o_name), &p_ptr->inventory[i], TRUE, 3);
		file_putf(fff, "%c%s %s\n",
		        index_to_label(i), paren, o_name);
	}
	file_putf(fff, "%s", "\n\n");

	/* Dump the inventory */
	file_putf(fff, "%s", "  [Character Inventory]\n\n");
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_desc(0, o_name, sizeof(o_name), &p_ptr->inventory[i], TRUE, 3);
		file_putf(fff, "%c%s %s\n",
		        index_to_label(i), paren, o_name);
	}
	file_putf(fff, "%s", "\n\n");

	/* Dump house inventory */
	file_putf(fff, "%s", "  [Home Inventory]\n");
	for (i = 0; i < num_houses; i++)
	{
		if (house_owned_by(p_ptr, i))
		{
			int Depth = houses[i].depth;
			cave_type *c_ptr;

			file_putf(fff, "%s", "\n"); j = 0;
			for(y=houses[i].y_1; y<=houses[i].y_2;y++)
			{
				for(x=houses[i].x_1; x<=houses[i].x_2;x++)
				{
					/* Paranoia -- unallocated Depth */
					if (!cave[Depth]) continue;
					c_ptr = &cave[Depth][y][x];
					if (c_ptr->o_idx)
					{
						if (j > 12) { file_putf(fff, "%s", "\n"); j = 0; }
						object_desc(0, o_name, sizeof(o_name), &o_list[c_ptr->o_idx], TRUE, 3);
						file_putf(fff, "%c%s %s\n",
			        		index_to_label(j), paren, o_name);
						j++;
					}
				}
			}
		}
	}
	file_putf(fff, "%s", "\n\n");

	/* Dump character history */
	if(p_ptr->birth_turn.turn || p_ptr->birth_turn.era)
	{
		history_event *evt;
		file_putf(fff, "%s", "  [Character History]\n\n");
		file_putf(fff, "%s", "Time       Dungeon Char  Event\n");
		file_putf(fff, "%s", "           Level   Level\n\n");
		for(evt = p_ptr->charhist; evt; evt = evt->next)
		{
			file_putf(fff, "%s\n", format_history_event(evt));
		}
		file_putf(fff, "%s", "\n\n");
	}

	/* Dump last messages */
	file_putf(fff, "%s", "  [Last Messages]\n\n");
	i = p_ptr->msg_hist_ptr;
	for(j=0;j<MAX_MSG_HIST;j++)
	{
		if(i >= MAX_MSG_HIST) i = 0;
		if(!STRZERO(p_ptr->msg_log[i]))
			file_putf(fff, "%s\n",p_ptr->msg_log[i]);
		i++;
	}
	file_putf(fff, "%s", "\n\n");

	/* Dump the scene of death */
	file_putf(fff, "%s", "  [Scene of Death]\n\n");
	/* Get an in bounds area */
	x1 = p_ptr->px - 39;
	x2 = p_ptr->px + 39;
	y1 = p_ptr->py - 10;
	y2 = p_ptr->py + 10;
	if (y1 < 0)
	{
		y2 = y2-y1;
		y1 = 0;
	}
	if (x1 < 0)
	{
		x2 = x2-x1;
		x1 = 0;
	}
	if (y2 > MAX_HGT-1)
	{
		y1 = y1 - (y2-(MAX_HGT-1));
		y2 = MAX_HGT-1;
	}
	if (x2 > MAX_WID-1)
	{
		x1 = x1 - (x2-(MAX_WID-1));
		x2 = MAX_WID-1;
	}
	/* Prepare status line */
	c_prt_status_line(p_ptr, &status[0], 80);

	/* Describe each row */
	for(y=y1;y<=y2+1;y++)
	{
		for(x=x1;x<=x2;x++)
		{
			/* Get the features */
			if (y > y2)
			{
				/* Hack -- read from status line */
				if (x-x1 < 80) {
					a = status[x-x1].a;
					c = status[x-x1].c;
				} else {
					a = TERM_WHITE; c = ' ';
				}
			}
			else
				map_info(p_ptr, y, x, &a, &c, &a, &c, TRUE);
			/* Hack for the player who is already dead and gone */
			if( (x == p_ptr->px) && (y == p_ptr->py))
			{
				c = '@';
				a = 'W';
			}
			/* translate the attr */
			attr = 'w';
			switch (a)
			{
				case TERM_DARK: attr = 'd'; break;
				case TERM_WHITE: attr = 'w'; break;
				case TERM_SLATE: attr = 's'; break;
				case TERM_ORANGE: attr = 'o'; break;
				case TERM_RED: attr = 'r'; break;
				case TERM_GREEN: attr = 'g'; break;
				case TERM_BLUE: attr = 'b'; break;
				case TERM_UMBER: attr = 'u'; break;
				case TERM_L_DARK: attr = 'D'; break;
				case TERM_L_WHITE: attr = 'W'; break;
				case TERM_VIOLET: attr = 'v'; break;
				case TERM_YELLOW: attr = 'y'; break;
				case TERM_L_RED: attr = 'R'; break;
				case TERM_L_GREEN: attr = 'G'; break;
				case TERM_L_BLUE: attr = 'B'; break;
				case TERM_L_UMBER: attr = 'U'; break;
			}
			/* Config file controls if we output with color codes */
			if(cfg_chardump_color)
			{
				/* Output with attr colour code */
				file_putf(fff,"%c%c",attr,c);
			}
			else
			{
				/* Output plain ASCII */
				file_putf(fff,"%c",c);
			}
		}
		file_putf(fff, "%s", "\n");
	}
	file_putf(fff, "%s", "\n\n");


	/* Close it */
	file_close(fff);

	/* Success */
	return (0);
}




int file_peruse_next(player_type *p_ptr, char query, int next)
{
	/* Process query */
	if (query)
	{
		if (query == '1') /* 'End' */
			p_ptr->interactive_line = p_ptr->interactive_size-20;
		else /* Other keys */
			common_peruse(p_ptr, query);

		/* Adjust viewport boundaries */
		if (p_ptr->interactive_line > p_ptr->interactive_size-20)
			p_ptr->interactive_line = p_ptr->interactive_size-20;
		if (p_ptr->interactive_line < 0)
			p_ptr->interactive_line = 0;

		/* Shift window! */
		if ((p_ptr->interactive_line+20 > p_ptr->interactive_next+MAX_TXT_INFO)
			|| (p_ptr->interactive_line < p_ptr->interactive_next))
		{
			next = p_ptr->interactive_line - MAX_TXT_INFO / 2;
		}

		/* Adjust window boundaries */
		if (next > p_ptr->interactive_size - MAX_TXT_INFO)
			next = p_ptr->interactive_size - MAX_TXT_INFO;
		if (next < 0) next = 0;
	}
	return next;
}


/*
 * On-Line help.
 *
 * Process user commands, access sub-menu entries and browse files. 
 * This function manages a virtual 'window' which buffers file
 * contents using "copy_file_info" function.
 */
void common_file_peruse(player_type *p_ptr, char query)
{
	int next = p_ptr->interactive_next;

	/* Enter sub-menu */
	if (isalpha(query))
	{
		/* Extract the requested menu item */
		int k = A2I(query);

		/* Verify the menu item */
		if ((k >= 0) && (k <= 25) && !STRZERO(p_ptr->interactive_hook[k]))
		{
			/* Paranoia -- free string */
			string_free(p_ptr->interactive_file);
			/* Select that file */
			p_ptr->interactive_file = string_make(p_ptr->interactive_hook[k]);
			/* Hack: enforce update */
			p_ptr->interactive_next = -1;
			next = 0;
			/* Query processed */
			query = 0;
		}
	}

	/* Use default file */
	if (!p_ptr->interactive_file)
	{
		/* Paranoia -- free string */
		string_free(p_ptr->interactive_file);
		/* Select default file */
		p_ptr->interactive_file = string_make("help.hlp");
		/* Hack: enforce update */
		p_ptr->interactive_next = -1;
		next = 0;
	}

	/* We're just starting. Reset counter */
	if (!query)
	{
		p_ptr->interactive_line = 0;
	}

	/* We're done. Clear file, exit */
	if (query == ESCAPE)
	{
		if (p_ptr->interactive_file)
		{
			string_free(p_ptr->interactive_file);
			p_ptr->interactive_file = NULL;
		}
		p_ptr->special_file_type = 0;
		return;
	}

	/* Process query */
	if (query)
	{
		next = file_peruse_next(p_ptr, query, next);
	}

	/* Hack -- something overwrote "info" */
	if (!p_ptr->last_info_line)
	{
		p_ptr->interactive_next = -1;
	}

	/* Update file */
	if (next != p_ptr->interactive_next)
	{
		p_ptr->interactive_next = next;
		copy_file_info(p_ptr, p_ptr->interactive_file, next, 0);
	}
}

/*
 * Read a file and copy a portion of it into player's "info[]" array.
 *
 * TODO: Add 'search' from do_cmd_help_aux()
 *
 */
void copy_file_info(player_type *p_ptr, cptr name, int line, int color)
{
	int i = 0, k;

	/* Current help file */
	ang_file* fff = NULL;

	/* Number of "real" lines passed by */
	int		next = 0;

	/* Path buffer */
	char	path[1024];

	/* General buffer */
	char	buf[1024];

	/* Strlen */
	int 	len;

	/* Build the filename */
	path_build(path, 1024, ANGBAND_DIR_HELP, name);

	/* Open the file */
	fff = file_open(path, MODE_READ, -1);

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg_format(p_ptr, "Cannot open '%s'.", name);
		msg_print(p_ptr, NULL);

		/* Oops */
		return;
	}

	/* Wipe the hooks */
	for (k = 0; k < 26; k++) p_ptr->interactive_hook[k][0] = '\0';

	/* Parse the file */
	while (TRUE)
	{
		byte attr = TERM_WHITE;

		/* Read a line or stop */
		if (!file_getl(fff, buf, 1024)) break;

		/* XXX Parse "menu" items */
		if (prefix(buf, "***** "))
		{
			char b1 = '[', b2 = ']';

			/* Notice "menu" requests */
			if ((buf[6] == b1) && isalpha(buf[7]) &&
			    (buf[8] == b2) && (buf[9] == ' '))
			{
				/* Extract the menu item */
				k = A2I(buf[7]);

				/* Store the menu item (if valid) */
				if ((k >= 0) && (k < 26))
					my_strcpy(p_ptr->interactive_hook[k], buf + 10, sizeof(p_ptr->interactive_hook[0]));
			}

			/* Skip this */
			continue;
		}

		/* Get length */
		len = strlen(buf);

		/* Count the "real" lines */
		next++;

		/* Wait for needed one */
		if (next <= line) continue;

		/* Too much */
		if (i >= MAX_TXT_INFO) continue;

		/* Extract color */
		if (color) attr = color_char_to_attr(buf[0]);

		/* Clear rest of line with spaces */
		for (k = len; k < 80 + color; k++)
		{
			buf[k] = ' ';
		}

		/* Dump the line */
		for (k = 0; k < 80; k++)
		{
			p_ptr->info[i][k].a = attr;
			p_ptr->info[i][k].c = buf[k+color];
		}

		/* Count the "info[]" lines */
		i++;
	}

	/* Save last "real" line */
	p_ptr->interactive_size = next;

	/* Save last dumped line */
	p_ptr->last_info_line = i - 1;

	/* Close the file */
	file_close(fff);
}

#if 0
/*
 * Recursive "help file" perusal.  Return FALSE on "ESCAPE".
 *
 * XXX XXX XXX Consider using a temporary file.
 *
 */
static bool do_cmd_help_aux(player_type *p_ptr, cptr name, cptr what, int line, int color)
{
	int		i, k;

	/* Number of "real" lines passed by */
	int		next = 0;

	/* Number of "real" lines in the file */
	int		size = 0;

	/* Backup value for "line" */
	int		back = 0;

	/* This screen has sub-screens */
	bool	menu = FALSE;

	/* Current help file */
	ang_file* fff = NULL;

	/* Find this string (if any) */
	cptr	find = NULL;

	/* Hold a string to find */
	char	finder[128];

	/* Hold a string to show */
	char	shower[128];

	/* Describe this thing */
	char	caption[128];

	/* Path buffer */
	char	path[1024];

	/* General buffer */
	char	buf[1024];

	/* Sub-menu information */
	char	hook[10][32];


	/* Wipe finder */
	strcpy(finder, "");

	/* Wipe shower */
	strcpy(shower, "");

	/* Wipe caption */
	strcpy(caption, "");

	/* Wipe the hooks */
	for (i = 0; i < 10; i++) hook[i][0] = '\0';


	/* Hack XXX XXX XXX */
	if (what)
	{
		/* Caption */
		strcpy(caption, what);

		/* Access the "file" */
		strcpy(path, name);

		/* Open */
		fff = file_open(path, MODE_READ, -1);
	}

	/* Look in "help" */
	if (!fff)
	{
		/* Caption */
		sprintf(caption, "Help file '%s'", name);

		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_HELP, name);

		/* Open the file */
		fff = file_open(path, MODE_READ, -1);
	}

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg_format(p_ptr, "Cannot open '%s'.", name);
		msg_print(Ind, NULL);

		/* Oops */
		return (TRUE);
	}


	/* Pre-Parse the file */
	while (TRUE)
	{
		/* Read a line or stop */
		if (!file_getl(fff, buf, 1024)) break;

		/* XXX Parse "menu" items */
		if (prefix(buf, "***** "))
		{
			char b1 = '[', b2 = ']';

			/* Notice "menu" requests */
			if ((buf[6] == b1) && isdigit(buf[7]) &&
			    (buf[8] == b2) && (buf[9] == ' '))
			{
				/* This is a menu file */
				menu = TRUE;

				/* Extract the menu item */
				k = buf[7] - '0';

				/* Extract the menu item */
				strcpy(hook[k], buf + 10);
			}

			/* Skip this */
			continue;
		}

		/* Count the "real" lines */
		next++;
	}

	/* Save the number of "real" lines */
	size = next;



	/* Display the file */
#if 0
	while (TRUE)
	{
		/* Clear screen */
		Term_clear();

#endif

		/* Restart when necessary */
		if (line >= size) Players[Ind]->interactive_line = line = 0;


		/* Re-open the file if needed */
		if (next > line)
		{
			/* Close it */
			file_close(fff);

			/* Hack -- Re-Open the file */
			fff = file_open(path, MODE_READ, -1);

			/* Oops */
			if (!fff) return (FALSE);

			/* File has been restarted */
			next = 0;
		}

		/* Skip lines if needed */
		for (; next < line; next++)
		{
			/* Skip a line */
			if (!file_getl(fff, buf, 1024)) break;
		}


		/* Dump the next 20 lines of the file */
		for (i = 0; i < 20; )
		{
			byte attr = TERM_WHITE;

			/* Hack -- track the "first" line */
			if (!i) line = next;

			/* Get a line of the file or stop */
			if (!file_getl(fff, buf, 1024)) break;

			/* Hack -- skip "special" lines */
			if (prefix(buf, "***** ")) continue;

			/* Count the "real" lines */
			next++;

			/* Hack -- keep searching */
			if (find && !i && !strstr(buf, find)) continue;

			/* Hack -- stop searching */
			find = NULL;

			/* Extract color */
			if (color) attr = color_char_to_attr(buf[0]);

			/* Hack -- show matches */
			if (shower[0] && strstr(buf, shower)) attr = TERM_YELLOW;

			/* Dump the line */
			Send_special_line(Ind, size, i, attr, &buf[color]);

			/* Count the printed lines */
			i++;
		}

		/* Hack -- failed search */
		if (find)
		{
			bell();
			line = back;
			find = NULL;
			return (TRUE);
		}

		/* Inform about empty file/list */
		if (!i)
			Send_special_line(Ind, 1, 0, TERM_WHITE, "  (nothing)");

#if 0
		/* Show a general "title" */
		prt(format("[MAngband %d.%d.%d, %s, Line %d/%d]",
		           SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH,
		           caption, line, size), 0, 0);


		/* Prompt -- menu screen */
		if (menu)
		{
			/* Wait for it */
			prt("[Press a Number, or ESC to exit.]", 23, 0);
		}

		/* Prompt -- small files */
		else if (size <= 20)
		{
			/* Wait for it */
			prt("[Press ESC to exit.]", 23, 0);
		}

		/* Prompt -- large files */
		else
		{
			/* Wait for it */
			prt("[Press Return, Space, -, =, /, or ESC to exit.]", 23, 0);
		}

		/* Get a keypress */
		k = inkey();

		/* Hack -- return to last screen */
		if (k == '?') break;

		/* Hack -- try showing */
		if (k == '=')
		{
			prt("Show: ", 23, 0);
			(void)askfor_aux(shower, 80);
		}

		/* Hack -- try finding */
		if (k == '/')
		{
			prt("Find: ", 23, 0);
			if (askfor_aux(finder, 80))
			{
				find = finder;
				back = line;
				line = line + 1;
			}
		}

		/* Hack -- go to a specific line */
		if (k == '#')
		{
			char tmp[80];
			prt("Goto Line: ", 23, 0);
			strcpy(tmp, "0");
			if (askfor_aux(tmp, 80))
			{
				line = atoi(tmp);
			}
		}

		/* Hack -- go to a specific file */
		if (k == '%')
		{
			char tmp[80];
			prt("Goto File: ", 23, 0);
			strcpy(tmp, "help.hlp");
			if (askfor_aux(tmp, 80))
			{
				if (!do_cmd_help_aux(tmp, NULL, 0)) k = ESCAPE;
			}
		}

		/* Hack -- Allow backing up */
		if (k == '-')
		{
			line = line - 10;
			if (line < 0) line = 0;
		}

		/* Hack -- Advance a single line */
		if ((k == '\n') || (k == '\r'))
		{
			line = line + 1;
		}

		/* Advance one page */
		if (k == ' ')
		{
			line = line + 20;
		}

		/* Recurse on numbers */
		if (menu && isdigit(k) && hook[k-'0'][0])
		{
			/* Recurse on that file */
			if (!do_cmd_help_aux(hook[k-'0'], NULL, 0)) k = ESCAPE;
		}

		/* Exit on escape */
		if (k == ESCAPE) break;
	}

#endif

	/* Close the file */
	file_close(fff);

	/* Escape */
	if (k == ESCAPE) return (FALSE);

	/* Normal return */
	return (TRUE);
}


/*
 * Peruse the On-Line-Help, starting at the given file.
 *
 * Disabled --KLJ--
 */
void do_cmd_help(player_type *p_ptr, int line)
{
	cptr name = "help.hlp";

	/* Peruse the main help file */
	(void)do_cmd_help_aux(p_ptr, name, NULL, line, FALSE);
}
#endif


/*
 * Hack -- display the contents of a file on the screen
 *
 * XXX XXX XXX Use this function for commands such as the
 * "examine object" command.
 */
errr show_file(player_type *p_ptr, cptr name, cptr what, int line, int color)
{
	/* Prepare */
	clear_from(p_ptr, 0);

	/* Peruse the requested file */
	copy_file_info(p_ptr, name, line, color);

	/* Send header */
	Send_special_other(p_ptr, (char*)what);

	/* Success */
	return (0);
}

/* Given a player name in "nick_name", write out a new version to "wptr",
 * where a) all words are capitalized b) double-spaces are removed.
 * For example, "juG  tHe brAve" should become "Jug The Brave".
 * If "bptr" is not NULL, the "base" version will be written to
 * it, replacing all spaces with underscore (-> "Jug_The_Brave").
 * It is assumed that the only characters that exist in "nick_name" are
 *  letters (a-zA-Z), digits (0-9) and space ( ).
 * Some other function should've taken care of that.
 * It is assumed that "wptr" is at least of MAX_CHARS length.
 * It is assumed that "bptr" is at least of MAX_CHARS length.
 *
 * Note: because we actually want to allow names that do not follow
 * this format (i.e. "ZalPriest", "Master_of_Puppets"), this function
 * SHOULD NOT be called! However, if you're on an machine with
 * case-insensitive filesystem (looking at you, Win32), this function
 * MUST be called, as early as possible, to prevent savefile name collision.
 */
int rewrite_player_name(char *wptr, char *bptr, const char *nick_name)
{
	/* Re-write nick, removing double spaces */
	const char *p;
	bool recap;
	const char *wptr_start;
	const char *bptr_start;

	/* Nothing to do */
	if (!wptr && !bptr) return 0;

	wptr_start = wptr;
	bptr_start = bptr;

	if (wptr) *wptr = '\0';
	if (bptr) *bptr = '\0';
	recap = TRUE;
	for (p = nick_name; *p; p++)
	{
		char c = *p;
		if (c == ' ') /* A space */
		{
			if (recap) continue; /* Skip 2nd, 3rd, Nth space */
			recap = TRUE;
		}
		else /* It's a symbol or digit */
		{
			if (isalpha(c))
			{
				c = recap ? toupper(c) : tolower(c);
			}
			recap = FALSE;
		}
		/* Save one char */
		if (wptr) *wptr++ = c;

		/* Save "base" version */
		if (bptr) *bptr++ = (c == ' ' ? '_': c);
	}
	/* Terminate strings */
	if (wptr) *wptr = '\0';
	if (bptr) *bptr = '\0';

	/* Right-trim spaces */
	if (wptr) for ( ; wptr-- > wptr_start; )
	{
		if (*wptr == ' ') *wptr = '\0';
		else break;
	}
	if (bptr) for ( ; bptr-- > bptr_start; )
	{
		if (*bptr == '_') *bptr = '\0';
		else break;
	}

	if (!wptr) return bptr - bptr_start;
	return wptr - wptr_start;
}

/*
 * XXXXXXXXX
 */
int process_player_name_aux(cptr name, cptr base, bool sf)
{
	int i, k = 0;
	char local_base[MAX_CHARS];
	char *basename = (char*) base;

	
	/* Cannot be too long */
	if (strlen(name) > MAX_NAME_LEN)
	{
		/* Abort */
		return -1;
	}

	/* Cannot contain "icky" characters */
	for (i = 0; name[i]; i++)
	{
		/* No control characters */
		if (iscntrl(name[i]))
		{
			/* Abort */
			return -2;
		}
	}
	
	if (base == NULL) basename = &local_base[0];

#ifdef MACINTOSH

	/* Extract "useful" letters */
	for (i = 0; name[i]; i++)
	{
		char c = name[i];

		/* Convert "dot" to "underscore" */
		if (c == '.') c = '_';

		/* Accept all the letters */
		basename[k++] = c;
	}

#else

	/* Extract "useful" letters */
	for (i = 0; name[i]; i++)
	{
		char c = name[i];

		/* Accept some letters */
		if (isalpha(c) || isdigit(c)) basename[k++] = c;

		/* Convert space, dot, and underscore to underscore */
		else if (strchr(". _", c)) basename[k++] = '_';
	}

#endif

	/* Terminate */
	basename[k] = '\0';

	/* Require a "base" name */
	if (!basename[0]) strcpy(basename, "PLAYER");

	/* Change the savefile name */
	if (sf)
	{
		char temp[128];

		/* Rename the savefile, using the player_base */
		(void)sprintf(temp, "%s", basename);

#ifdef VM
		/* Hack -- support "flat directory" usage on VM/ESA */
		(void)sprintf(temp, "%s.sv", player_base);
#endif /* VM */

		/* Build the filename */
		path_build((char*)name, 1024, ANGBAND_DIR_SAVE, temp);
	}

	/* Success */
	return 0;
}



/*
 * Process the player name.
 * Extract a clean "base name".
 * Build the savefile name if needed.
 */
bool process_player_name(player_type *p_ptr, bool sf)
{
	int ret;
	
#ifdef SAVEFILE_MUTABLE

	/* Accept */
	sf = TRUE;

#endif
	
	ret = process_player_name_aux(p_ptr->name, p_ptr->basename, FALSE);

	/* Name is too long or contained illegal characters */
	if (ret < 0)
	{
		/* Abort */
		return FALSE;
	}

	/* Change the savefile name */
	if (sf)
	{
		char temp[128];

		/* Rename the savefile, using the player_base */
		(void)sprintf(temp, "%s", p_ptr->basename);

#ifdef VM
		/* Hack -- support "flat directory" usage on VM/ESA */
		(void)sprintf(temp, "%s.sv", player_base);
#endif /* VM */

		/* Build the filename */
		path_build(p_ptr->savefile, 1024, ANGBAND_DIR_SAVE, temp);
	}

	/* Success */
	return TRUE;
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Assumes that "display_player()" has just been called
 * XXX Perhaps we should NOT ask for a name (at "birth()") on Unix?
 *
 * The name should be sent to us from the client, so this is unnecessary --KLJ--
 */
void get_name(player_type *p_ptr)
{
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(player_type *p_ptr)
{
	/* Mark as suicide */
	p_ptr->alive = FALSE;

	/* Hack -- set the cause of death */
	if (!p_ptr->ghost) 
	{
		strcpy(p_ptr->died_from_list, "self-inflicted wounds");
		p_ptr->died_from_depth = p_ptr->dun_depth;
	}

	/* Hack -- clear ghost */
	p_ptr->ghost = FALSE;

	if (p_ptr->total_winner) kingly(p_ptr);

	/* Queue "death", to be handled in dungeon() tick, so it happens inside game turns */
	p_ptr->death = TRUE;
}



/*
 * Save a character
 */
void do_cmd_save_game(player_type *p_ptr)
{
	/* Disturb the player */
	disturb(p_ptr, 1, 0);

	/* Clear messages */
	msg_print(p_ptr, NULL);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Message */
	msg_print(p_ptr, "Saving game...");

	/* Refresh */
	/*Term_fresh();*/

	/* The player is not dead */
	(void)strcpy(p_ptr->died_from, "(saved)");

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Save the player */
	if (save_player(p_ptr))
	{
		msg_print(p_ptr, "Saving game... done.");
	}

	/* Save failed (oops) */
	else
	{
		msg_print(p_ptr, "Saving game... failed!");
	}

	/* Allow suspend again */
	signals_handle_tstp();

	/* Refresh */
	/*Term_fresh();*/

	/* Note that the player is not dead */
	(void)strcpy(p_ptr->died_from, "(alive and well)");
}



/*
 * Hack -- Calculates the total number of points earned		-JWT-
 */
long total_points(player_type *p_ptr)
{
	/* Standard scoring */
	long score = (p_ptr->max_exp + (100 * p_ptr->max_dlv));

	/* Remove 10% for energy_buildup */
	if (option_p(p_ptr,ENERGY_BUILDUP))
	{
		score = (long)(score * 0.9);
	}

	/* Add 50% for unburdened monsters */
	if (!option_p(p_ptr,MONSTER_RECOIL))
	{
		score = (long)(score * 1.5);
	}

	/* We award a 50% score bonus for bravery with no_ghost characters */
	if (option_p(p_ptr, NO_GHOST) && !cfg_ironman)
	{
		score = (long)(score * 1.5);
	}

	/* Standard scoring */
	return score;
}




/*
 * Display some character info
 *
 * FIXME -- This is very broken.  There is no home.  There is no way to get
 *   most of this information transferred to the client.  This isn't used all
 *   that often.  I'll worry about it later.  --KLJ--
 */
static void show_info(player_type *p_ptr)
{
#if 0
	int			i, j, k;

	object_type		*o_ptr;

	store_type		*st_ptr = &store[7];


	/* Hack -- Know everything in the inven/equip */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];
		if (o_ptr->k_idx)
		{
			object_aware(p_ptr, o_ptr);
			object_known(o_ptr);
		}
	}

	/* Hack -- Know everything in the home */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		o_ptr = &st_ptr->stock[i];
		if (o_ptr->k_idx)
		{
			object_aware(p_ptr, o_ptr);
			object_known(o_ptr);
		}
	}

	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Flush all input keys */
	flush();

	/* Flush messages */
	msg_print(NULL);


	/* Describe options */
	prt("You may now dump a character record to one or more files.", 21, 0);
	prt("Then, hit RETURN to see the character, or ESC to abort.", 22, 0);

	/* Dump character records as requested */
	while (TRUE)
	{
		char out_val[160];

		/* Prompt */
		put_str("Filename: ", 23, 0);

		/* Default */
		strcpy(out_val, "");

		/* Ask for filename (or abort) */
		if (!askfor_aux(out_val, 60)) return;

		/* Return means "show on screen" */
		if (!out_val[0]) break;

		/* Dump a character file */
		(void)file_character(out_val, FALSE);
	}


	/* Show player on screen */
	display_player(FALSE);

	/* Prompt for inventory */
	prt("Hit any key to see more information (ESC to abort): ", 23, 0);

	/* Allow abort at this point */
	if (inkey() == ESCAPE) return;


	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (equip_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_equip();
		prt("You are using: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven();
		prt("You are carrying: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}



	/* Home -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Display contents of the home */
		for (k = 0, i = 0; i < st_ptr->stock_num; k++)
		{
			/* Clear screen */
			Term_clear();

			/* Show 12 items */
			for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
			{
				char o_name[80];
				char tmp_val[80];

				/* Acquire item */
				o_ptr = &st_ptr->stock[i];

				/* Print header, clear line */
				sprintf(tmp_val, "%c) ", I2A(j));
				prt(tmp_val, j+2, 4);

				/* Display object description */
				object_desc(o_name, o_ptr, TRUE, 3);
				c_put_str(tval_to_attr[o_ptr->tval], o_name, j+2, 7);
			}

			/* Caption */
			prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

			/* Wait for it */
			if (inkey() == ESCAPE) return;
		}
	}
#endif
}





/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score
{
	char what[8];		/* Version info (string) */

	char pts[10];		/* Total Score (number) */

	char gold[10];		/* Total Gold (number) */

	char turns[20];		/* Turns Taken (number) */

	char day[10];		/* Time stamp (string) */

	char who[16];		/* Player Name (string) */

	char uid[8];		/* Player UID (number) */

	char sex[2];		/* Player Sex (string) */
	char p_r[3];		/* Player Race (number) */
	char p_c[3];		/* Player Class (number) */

	char cur_lev[4];		/* Current Player Level (number) */
	char cur_dun[4];		/* Current Dungeon Level (number) */
	char max_lev[4];		/* Max Player Level (number) */
	char max_dun[4];		/* Max Dungeon Level (number) */

	char how[32];		/* Method of death (string) */
};



/*
 * The "highscore" file descriptor, if available.
 */
static ang_file* highscore_fd = NULL;


/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{
	/* Seek for the requested record */
	return (file_seek(highscore_fd, (huge)(i) * sizeof(high_score))) ? 0 : -1;
}


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
	/* Read the record, note failure */
	return (file_read(highscore_fd, (char*)(score), sizeof(high_score))) > 0 ? 0 : -1;
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
	/* Write the record, note failure */
	return (file_write(highscore_fd, (char*)(score), sizeof(high_score))) ? 0 : -1;
}




/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(high_score *score)
{
	int			i;

	high_score		the_score;

	/* Paranoia -- it may not have opened */
	if (highscore_fd == NULL) return (-1);

	/* Go to the start of the highscore file */
	if (highscore_seek(0)) return (-1);

	/* Read until we get to a higher score */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) return (i);
		if (strcmp(the_score.pts, score->pts) < 0) return (i);
	}

	/* The "last" entry is always usable */
	return (MAX_HISCORES - 1);
}


/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
	int			i, slot;
	bool		done = FALSE;

	high_score		the_score, tmpscore;


	/* Paranoia -- it may not have opened */
	if (highscore_fd == NULL) return (-1);

	/* Determine where the score should go */
	slot = highscore_where(score);

	/* Hack -- Not on the list */
	if (slot < 0) return (-1);

	/* Hack -- prepare to dump the new score */
	the_score = (*score);

	/* Slide all the scores down one */
	for (i = slot; !done && (i < MAX_HISCORES); i++)
	{
		/* Read the old guy, note errors */
		if (highscore_seek(i)) return (-1);
		if (highscore_read(&tmpscore)) done = TRUE;

		/* Back up and dump the score we were holding */
		if (highscore_seek(i)) return (-1);
		if (highscore_write(&the_score)) return (-1);

		/* Hack -- Save the old score, for the next pass */
		the_score = tmpscore;
	}

	/* Return location used */
	return (slot);
}



/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
static void display_scores_aux(player_type *p_ptr, int line, int note, high_score *score)
{
	int		i, j, from, to, attr, place;

	high_score	the_score;

	char	out_val[256];

	ang_file* fff;
	char file_name[1024];

	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return;

	/* Seek to the beginning (or abort) */
	if (highscore_seek(0)) return;
	
	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open the temp file */
	fff = file_open(file_name, MODE_WRITE, FTYPE_TEXT);

	/* Paranoia */
	if (!fff) 
	{
		plog(format("ERROR! %s (writing %s)", strerror(errno), file_name));
		return;
	}

	/* Assume we will show the first 20 */
	from = 0;
	to = 20;
	if (to > MAX_HISCORES) to = MAX_HISCORES;


	/* Hack -- Count the high scores */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) break;
	}

	/* Hack -- allow "fake" entry to be last */
	if ((note == i) && score) i++;

	/* Forget about the last entries */
	if (i > to) i = to;


	/* Show 5 per page, until "done" */
	for (j = from, place = j+1; j < i; j++, place++)
	{
		int pr, pc, clev, mlev, cdun, mdun;

		cptr user, gold, when, aged;


		/* Hack -- indicate death in yellow */
		attr = (j == note) ? TERM_YELLOW : TERM_WHITE;


		/* Mega-Hack -- insert a "fake" record */
		if ((note == j) && score)
		{
			the_score = (*score);
			attr = TERM_L_GREEN;
			score = NULL;
			note = -1;
			j--;
		}

		/* Read a normal record */
		else
		{
			/* Read the proper record */
			if (highscore_seek(j)) break;
			if (highscore_read(&the_score)) break;
		}

		/* Extract the race/class */
		pr = atoi(the_score.p_r);
		pc = atoi(the_score.p_c);

		/* Extract the level info */
		clev = atoi(the_score.cur_lev);
		mlev = atoi(the_score.max_lev);
		cdun = atoi(the_score.cur_dun);
		mdun = atoi(the_score.max_dun);

		/* Hack -- extract the gold and such */
		for (user = the_score.uid; isspace(*user); user++) /* loop */;
		for (when = the_score.day; isspace(*when); when++) /* loop */;
		for (gold = the_score.gold; isspace(*gold); gold++) /* loop */;
		for (aged = the_score.turns; isspace(*aged); aged++) /* loop */;

		/* Dump some info */
		sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
			place, the_score.pts, the_score.who,
			p_name + p_info[pr].name, c_name + c_info[pc].name,
			clev);

		/* Append a "maximum level" */
		if (mlev > clev) my_strcat(out_val, format(" (Max %d)", mlev), sizeof(out_val));

		/* Dump the first line */
		file_putf(fff, "%s\n", out_val);

		/* Another line of info */
		if (strcmp(the_score.how, "winner"))
			sprintf(out_val, "               Killed by %s on %s %d",
			the_score.how, "Dungeon Level", cdun);
		else
			sprintf(out_val, "               Retired after a legendary career");

		/* Hack -- some people die in the town */
		if ((!cdun) && (strcmp(the_score.how, "winner")))
		{
			sprintf(out_val, "               Killed by %s in the Town",
				the_score.how);
		}

		/* Append a "maximum level" */
		if (mdun > cdun) strcat(out_val, format(" (Max %d)", mdun));

		/* Dump the info */
		file_putf(fff, "%s\n", out_val);

		/* And still another line of info */
		sprintf(out_val,
			"               (User %s, Date %s, Gold %s, Turn %s).",
			user, when, gold, aged);
		file_putf(fff, "%s\n", out_val);

		/* Print newline if this isn't the last one */
		if (j < i - 1)
			file_putf(fff, "%s", "\n");
	}

	/* Close the file */
	file_close(fff);

	/* Display the file contents */
	show_file(p_ptr, file_name, "High Scores", line, 0);

	/* Remove the file */
	file_delete(file_name);
}




/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static errr top_twenty(player_type *p_ptr)
{
	int          j;

	high_score   the_score;

	time_t ct = time((time_t*)0);


	/* Clear screen */
	/*Term_clear();*/

	/* No score file */
	if (highscore_fd == NULL)
	{
		plog("Score file unavailable.");
		return (0);
	}

#ifndef SCORE_WIZARDS
	/* Wizard-mode pre-empts scoring */
	if (p_ptr->noscore & 0x000F)
	{
		msg_print(p_ptr, "Score not registered for wizards.");
		/*display_scores_aux(0, 10, -1, NULL);*/
		return (0);
	}
#endif

#ifndef SCORE_BORGS
	/* Borg-mode pre-empts scoring */
	if (p_ptr->noscore & 0x00F0)
	{
		msg_print(p_ptr, "Score not registered for borgs.");
		/*display_scores_aux(0, 10, -1, NULL);*/
		return (0);
	}
#endif

#ifndef SCORE_CHEATERS
	/* Cheaters are not scored */
	if (p_ptr->noscore & 0xFF00)
	{
		msg_print(p_ptr, "Score not registered for cheaters.");
		/*display_scores_aux(0, 10, -1, NULL);*/
		return (0);
	}
#endif

	/* Interupted */
	if (!p_ptr->total_winner && streq(p_ptr->died_from, "Interrupting"))
	{
		msg_print(p_ptr, "Score not registered due to interruption.");
		/* display_scores_aux(0, 10, -1, NULL); */
		return (0);
	}

	/* Quitter */
	if (!p_ptr->total_winner && streq(p_ptr->died_from, "Quitting"))
	{
		msg_print(p_ptr, "Score not registered due to quitting.");
		/* display_scores_aux(0, 10, -1, NULL); */
		return (0);
	}


	/* Clear the record */
	WIPE(&the_score, high_score);

	/* Save the version */
	sprintf(the_score.what, "%u.%u.%u",
	        SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points(p_ptr));
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	my_strcpy(the_score.turns, ht_show(&turn,0), sizeof(the_score.turns));

#ifdef HIGHSCORE_DATE_HACK
	/* Save the date in a hacked up form (9 chars) */
	sprintf(the_score.day, "%-.6s %-.2s", ctime(&ct) + 4, ctime(&ct) + 22);
#else
	/* Save the date in standard form (8 chars) */
	strftime(the_score.day, 9, "%m/%d/%y", localtime(&ct));
#endif

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", p_ptr->name);

	/* Save the player info */
	sprintf(the_score.uid, "%7u", 0 /*player_uid*/);
	sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", p_ptr->died_from_depth);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

	/* Save the cause of death (31 chars) */
	/* HACKED to take the saved cause of death of the character, not the ghost */
	sprintf(the_score.how, "%-.31s", p_ptr->died_from_list);


	/* Lock (for writing) the highscore file, or fail */
	if (!file_lock(highscore_fd)) return (1);

	/* Add a new entry to the score list, see where it went */
	j = highscore_add(&the_score);

	/* Unlock the highscore file, or fail */
	if (!file_unlock(highscore_fd)) return (1);


#if 0
	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(0, 15, j, NULL);
	}

	/* Display the scores surrounding the player */
	else
	{
		display_scores_aux(0, 5, j, NULL);
		display_scores_aux(j - 2, j + 7, j, NULL);
	}
#endif


	/* Success */
	return (0);
}


/*
 * Predict the players location, and display it.
 */
static errr predict_score(player_type *p_ptr, int line)
{
	int          j;

	high_score   the_score;


	/* No score file */
	if (highscore_fd < 0)
	{
		plog("Score file unavailable.");
		return (0);
	}


	/* Save the version */
	sprintf(the_score.what, "%u.%u.%u",
	        SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points(p_ptr));

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);

	/* Save the current turn */
	my_strcpy(the_score.turns, ht_show(&turn,0), sizeof(the_score.turns));

	/* Hack -- no time needed */
	strcpy(the_score.day, "TODAY");

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", p_ptr->name);

	/* Save the player info */
	sprintf(the_score.uid, "%7u", 0 /*player_uid*/);
	sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", p_ptr->dun_depth);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

	/* Hack -- no cause of death */
	strcpy(the_score.how, "nobody (yet!)");


	/* See where the entry would be placed */
	j = highscore_where(&the_score);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(p_ptr, line, j, &the_score);
	}

	/* Display some "useful" scores */
	else
	{
		display_scores_aux(p_ptr, line, -1, NULL);
	}


	/* Success */
	return (0);
}




/*
 * Change a player into a King!			-RAK-
 */
void kingly(player_type *p_ptr)
{
	/* Fake death */
	//(void)strcpy(p_ptr->died_from_list, "Ripe Old Age");
	(void)strcpy(p_ptr->died_from_list, "winner");

	/* Restore the experience */
	p_ptr->exp = p_ptr->max_exp;

	/* Restore the level */
	p_ptr->lev = p_ptr->max_plv;

	/* Hack -- Player gets an XP bonus for beating the game */
	p_ptr->exp = p_ptr->max_exp += 10000000L;

	/* Hack -- Ensure we are retired */
	p_ptr->retire_timer = 0;
}


/*
 * Add a player to the high score list.
 */
void add_high_score(player_type *p_ptr)
{
	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "scores.raw");

	/* Open the high score file, for reading/writing */
	highscore_fd = file_open(buf, MODE_READWRITE, FTYPE_RAW);

	/* Add them */
	top_twenty(p_ptr);

	/* Shut the high score file */
	(void)file_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = NULL;
}


/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 *
 * In here we try to save everybody's game, as well as save the server state.
 */
void close_game(void)
{
	int i;

	/* No suspending now */
	signals_ignore_tstp();

	for (i = 0; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];

		/* Handle stuff */
		handle_stuff(p_ptr);

		/* Flush the messages */
		msg_print(p_ptr, NULL);

		/* Flush the input */
		/*flush();*/


		/* Hack -- Character is now "icky" */
		/*character_icky = TRUE;*/


		/* Build the filename */
		/*path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");*/

		/* Open the high score file, for reading/writing */
		/*highscore_fd = fd_open(buf, O_RDWR);*/


		/* Handle death */
		if (p_ptr->death)
		{
			/* Handle retirement */
			if (p_ptr->total_winner) kingly(p_ptr);

			/* Save memories */
			if (!save_player(p_ptr)) msg_print(p_ptr, "death save failed!");

			/* Dump bones file 
			make_bones(i);
			*/

			/* Show more info */
			show_info(p_ptr);
	
			/* Handle score, show Top scores */
			top_twenty(p_ptr);
		}

		/* Still alive */
		else
		{
			/* Save the game */
			do_cmd_save_game(p_ptr);

			/* Prompt for scores XXX XXX XXX */
			/*prt("Press Return (or Escape).", 0, 40);*/
	
			/* Predict score (or ESCAPE) */
			/*if (inkey() != ESCAPE) predict_score();*/
		}


		/* Shut the high score file */
		/*(void)file_close(highscore_fd);*/

		/* Forget the high score fd */
		/*highscore_fd = NULL;*/
	}

	/* Try to save the server information */
	save_server_info();

	/* Allow suspending now */
	signals_handle_tstp();
}


/*
 * Hack -- Display the scores in a given range and quit.
 *
 * This function is only called from "main.c" when the user asks
 * to see the "high scores".
 */
void display_scores(player_type *p_ptr, int line)
{
	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "scores.raw");

	/* Open the binary high score file, for reading */
	highscore_fd = file_open(buf, MODE_READ, -1);

	/* Paranoia -- No score file */
	if (highscore_fd < 0)
	{
		/* Message to server admin */
		plog("Score file unavailable.");

		/* Quit */
		return;
	}

	/* Clear screen */
	/* Term_clear(); */

	/* Display the scores */
	predict_score(p_ptr, line);

	/* Shut the high score file */
	(void)file_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = NULL;

	/* Quit */
	/* quit(NULL); */
}


/*
 * Get a random line from a file
 * Based on the monster speech patch by Matt Graham,
 */
errr get_rnd_line(cptr file_name, int entry, char *output)
{
	ang_file* fp;
	char    buf[1024];
	int     line, counter, test, numentries;
	int     line_num = 0;
	bool    found = FALSE;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, file_name);

	/* Open the file */
	fp = file_open(buf, MODE_READ, -1);

	/* Failed */
	if (!fp) return (-1);

	/* Find the entry of the monster */
	while (TRUE)
	{
		/* Get a line from the file */
		if (file_getl(fp, buf, 1024))
		{
			/* Count the lines */
			line_num++;

			/* Look for lines starting with 'N:' */
			if ((buf[0] == 'N') && (buf[1] == ':'))
			{
				/* Allow default lines */
				if (buf[2] == '*')
				{
					/* Default lines */
					found = TRUE;
					break;
				}
				/* Get the monster number */
				else if (sscanf(&(buf[2]), "%d", &test) != EOF)
				{
					/* Is it the right monster? */
					if (test == entry)
					{
						found = TRUE;
						break;
					}
				}
				else
				{
					file_close(fp);
					return (-1);
				}
			}
		}
		else
		{
			/* Reached end of file */
			file_close(fp);
			return (-1);
		}

	}

	/* Get the number of entries */
	while (TRUE)
	{
		/* Get the line */
		if (file_getl(fp, buf, 1024))
		{
			/* Count the lines */
			line_num++;

			/* Look for the number of entries */
			if (isdigit(buf[0]))
			{
				/* Get the number of entries */
				numentries = atoi(buf);
				break;
			}
		}
		else
		{
			/* Count the lines */
			line_num++;

			file_close(fp);
			return (-1);
		}
	}

	if (numentries > 0)
	{
		/* Grab an appropriate line number */
		line = randint0(numentries);

		/* Get the random line */
		for (counter = 0; counter <= line; counter++)
		{
			/* Count the lines */
			line_num++;

			/* Try to read the line */
			if (file_getl(fp, buf, 1024))
			{
				/* Found the line */
				if (counter == line) break;
			}
			else
			{
				file_close(fp);
				return (-1);
			}
		}

		/* Copy the line */
		strcpy(output, buf);
	}

	/* Close the file */
	file_close(fp);

	/* Success */
	return (0);
}


/*
 * Handle a fatal crash.
 *
 * Here we try to save every player's state, and the state of the server
 * in general.  Note that we must be extremely careful not to have a crash
 * in this function, or some things may not get saved.  Also, this function
 * may get called because some data structures are not in a "correct" state.
 * For this reason many paranoia checks are done to prevent bad pointer
 * dereferences.
 *
 * Note that this function would not be needed at all if there were no bugs.
 */
void exit_game_panic()
{
	int i = 1;

	/* If nothing important has happened, just return */
	if (!server_generated || server_saved) return;

	/* Mega-Hack -- see "msg_print()" */
	msg_flag = FALSE;

	while (NumPlayers > (i - 1))
	{
		player_type *p_ptr = Players[i];

		/* Don't dereference bad pointers */
		if (!p_ptr)
		{
			/* Skip to next player */
			i++;

			continue;
		}

		/* Hack -- turn off some things */
		disturb(p_ptr, 1, 0);

		/* Mega-Hack -- Delay death */
		if (p_ptr->chp < 0) p_ptr->death = FALSE;

		/* Hardcode panic save */
		panic_save = 1;

		/* Forbid suspend */
		signals_ignore_tstp();

		/* Indicate panic save */
		(void)strcpy(p_ptr->died_from, "(panic save)");

		/* Try to save the player, don't worry if this fails because there
		 * is nothing we can do now anyway */
		save_player(p_ptr);
		i++;

	}

	/* Clear objects so that artifacts get saved.
	 * This probably isn't neccecary, as all the objects on each of
	 * these levels should have been cleared by now. However, paranoia
	 * can't hurt all that much... -APD
	 */
	for (i = 1; i < MAX_DEPTH; i++)
	{

		/* Paranoia -- wipe this depth's objects if no players are on it*/
		if (!players_on_depth[i]) wipe_o_list(i);
	}

	if (!save_server_info()) plog("server panic info save failed!");

	/* Successful panic save of server info */
	plog("server panic info save succeeded!");

}

/*
 * Windows specific replacement for signal handling [grk]
 */
#ifdef WINDOWS
#ifndef HANDLE_SIGNALS

LPTOP_LEVEL_EXCEPTION_FILTER old_handler;

/* Callback to be called by Windows when our term closes, the user 
 * logs off, the system is shutdown, etc.
 */
BOOL ctrl_handler( DWORD fdwCtrlType ) 
{
	/* Save everything and quit the game */
	shutdown_server();

	return TRUE;
}

/* Global unhandled exception handler */
/* If the server crashes under Windows, this is where we end up */
LONG WINAPI myUnhandledExceptionFilter(
  struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	/* We don't report to the meta server in this case, the meta
	 * server will detect that we've gone anyway 
	 */

	/* Call the previous exception handler, which we are assuming
	 * is the MinGW exception handler which should have been implicitly
	 * setup when we loaded the exchndl.dll library.
	 */
	if(old_handler != NULL)
	{
	  old_handler(ExceptionInfo);
	}

	/* Save everything and quit the game */
	exit_game_panic();

	/* We don't expect to ever get here... but for what it's worth... */
	return(EXCEPTION_EXECUTE_HANDLER); 
		
}


void setup_exit_handler(void)
{
	/* Trap CTRL+C, Logoff, Shutdown, etc */
	if( SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrl_handler, TRUE ) ) 
	{
		plog("Initialised exit save handler.");
	}else{
		plog("ERROR: Could not set panic save handler!");
	}
	/* Trap unhandled exceptions, i.e. server crashes */
	old_handler = SetUnhandledExceptionFilter( myUnhandledExceptionFilter );
}
#endif
#endif

#ifdef HANDLE_SIGNALS


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 *
 * This will probably inflict much anger upon the suspender, but it is still
 * allowed (for now) --KLJ--
 */
static void handle_signal_suspend(int sig)
{
	/* Disable handler */
	(void)signal(sig, SIG_IGN);

#ifdef SIGSTOP

	/* Flush output */
	/*Term_fresh();*/

	/* Suspend the "Term" */
	/*Term_xtra(TERM_XTRA_ALIVE, 0);*/

	/* Suspend ourself */
	(void)kill(0, SIGSTOP);

	/* Resume the "Term" */
	/*Term_xtra(TERM_XTRA_ALIVE, 1);*/

	/* Redraw the term */
	/*Term_redraw();*/

	/* Flush the term */
	/*Term_fresh();*/

#endif

	/* Restore handler */
	(void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 *
 * This simply calls "exit_game_panic()", which should try to save
 * everyone's character and the server info, which is probably nicer
 * than killing everybody. --KLJ--
 */
static void handle_signal_simple(int sig)
{
	/* Disable handler */
	(void)signal(sig, SIG_IGN);


	/* Nothing to save, just quit */
	if (!server_generated || server_saved) quit(NULL);

	/* Hack -- on SIGTERM, quit right away */
	if (sig == SIGTERM)
	{
		signal_count = 5;
	}

	/* Count the signals */
	signal_count++;


	/* Allow suicide (after 5) */
	if (signal_count >= 5)
	{
		/* Perform a "clean" shutdown */
		shutdown_server();
	}

	/* Give warning (after 4) */
	else if (signal_count >= 4)
	{
		plog("Warning: Next signal kills server!");

		/* Make a noise */
		/*Term_xtra(TERM_XTRA_NOISE, 0);*/

		/* Clear the top line */
		/*Term_erase(0, 0, 255);*/

		/* Display the cause */
		/*Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");*/

		/* Flush */
		/*Term_fresh();*/
	}

	/* Give warning (after 2) */
	else if (signal_count >= 2)
	{
		/* Make a noise */
		/*Term_xtra(TERM_XTRA_NOISE, 0);*/
	}

	/* Restore handler */
	(void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 *
 * This one also calls exit_game_panic() --KLJ--
 */
static void handle_signal_abort(int sig)
{
	/* We are *not* reentrant */
	if (signalbusy) raise(sig);
	signalbusy = 1;

    plog("Unexpected signal, panic saving.");

	/* Nothing to save, just quit */
	if (!server_generated || server_saved) quit(NULL);

	/* Save everybody */
    exit_game_panic();

	/* Enable default handler */
	(void)signal(sig, SIG_DFL);

	/* Reraise */
	raise(sig);
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
	(void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
	(void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init(void)
{

#ifdef SIGHUP
	(void)signal(SIGHUP, SIG_IGN);

#ifdef TARGET_OS_OSX
	/* Closing Terminal.app on OSX sends SIGHUP, let's not ignore it! */
	(void)signal(SIGHUP, handle_signal_abort);
#endif

#endif

#ifdef SIGTSTP
	(void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
	(void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
	(void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
	(void)signal(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
	(void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
	(void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
	(void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
	(void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
	(void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
	(void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
	(void)signal(SIGTERM, handle_signal_simple);
#endif

#ifdef SIGPIPE
	(void)signal(SIGPIPE, SIG_IGN);
#endif

#ifdef SIGEMT
	(void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
	(void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
	(void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
	(void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
	(void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else	/* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}


#endif	/* HANDLE_SIGNALS */
