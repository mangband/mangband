/* Handle the printing of info to the screen */



#include "angband.h"

/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(cptr info, int row, int col)
{
	/* Dump 13 spaces to clear */
	c_put_str(TERM_WHITE, "             ", row, col);

	/* Dump the info itself */
	c_put_str(TERM_L_BLUE, info, row, col);
}


/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val)
{
	if (val > 18)
	{
		int bonus = (val - 18);

		if (bonus >= 220)
		{
			sprintf(out_val, "18/%3s", "***");
		}
		else if (bonus >= 100)
		{
			sprintf(out_val, "18/%03d", bonus);
		}
		else
		{
			sprintf(out_val, " 18/%02d", bonus);
		}
	}
	else
	{
		sprintf(out_val, "    %2d", val);
	}
}


/*
 * Print character stat in given row, column
 */
void prt_stat(int stat, int max, int cur, bool maxed)
{
	char tmp[32];

	if (cur < max)
	{
		Term_putstr(0, ROW_STAT + stat, -1, TERM_WHITE, stat_names_reduced[stat]);
		cnv_stat(cur, tmp);
		Term_putstr(COL_STAT + 6, ROW_STAT + stat, -1, TERM_YELLOW, tmp);
	}

	else
	{
		Term_putstr(0, ROW_STAT + stat, -1, TERM_WHITE, stat_names[stat]);
		cnv_stat(cur, tmp);
	if (maxed)
		Term_putstr(COL_STAT + 6, ROW_STAT + stat, -1, TERM_L_UMBER, tmp);
	else
		Term_putstr(COL_STAT + 6, ROW_STAT + stat, -1, TERM_L_GREEN, tmp);
	}
}

/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
void prt_title(cptr title)
{
	prt_field(title, ROW_TITLE, COL_TITLE);
}


/*
 * Prints level and experience
 */
void prt_level(int level, int max, int cur, int adv)
{
	char tmp[32];

	sprintf(tmp, "%6d", level);

	Term_putstr(0, ROW_LEVEL, -1, TERM_WHITE, "LEVEL ");
	Term_putstr(COL_LEVEL + 6, ROW_LEVEL, -1, TERM_L_GREEN, tmp);

	sprintf(tmp, "%8ld", (long)cur);

	if (cur >= max)
	{
		Term_putstr(0, ROW_EXP, -1, TERM_WHITE, "EXP ");
		Term_putstr(COL_EXP + 4, ROW_EXP, -1, TERM_L_GREEN, tmp);
	}
	else
	{
		Term_putstr(0, ROW_EXP, -1, TERM_WHITE, "Exp ");
		Term_putstr(COL_EXP + 4, ROW_EXP, -1, TERM_YELLOW, tmp);
	}
}

/*
 * Prints current gold
 */
void prt_gold(int gold)
{
	char tmp[32];

	put_str("AU ", ROW_GOLD, COL_GOLD);
	sprintf(tmp, "%9ld", (long)gold);
	c_put_str(TERM_L_GREEN, tmp, ROW_GOLD, COL_GOLD + 3);
}

/*
 * Prints current AC
 */
void prt_ac(int ac)
{
	char tmp[32];

	put_str("Cur AC ", ROW_AC, COL_AC);
	sprintf(tmp, "%5d", ac);
	c_put_str(TERM_L_GREEN, tmp, ROW_AC, COL_AC + 7);
}

/*
 * Prints Max/Cur hit points
 */
void prt_hp(int max, int cur)
{
	char tmp[32];
	byte color;

	put_str("Max HP ", ROW_MAXHP, COL_MAXHP);

	sprintf(tmp, "%5d", max);
	color = TERM_L_GREEN;

	c_put_str(color, tmp, ROW_MAXHP, COL_MAXHP + 7);


	put_str("Cur HP ", ROW_CURHP, COL_CURHP);

	sprintf(tmp, "%5d", cur);

	if (cur >= max)
	{
		color = TERM_L_GREEN;
	}
	else if (cur > max / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}

	c_put_str(color, tmp, ROW_CURHP, COL_CURHP + 7);
}

/*
 * Prints Max/Cur spell points
 */
void prt_sp(int max, int cur)
{
	char tmp[32];
	byte color;

	put_str("Max SP ", ROW_MAXSP, COL_MAXSP);

	sprintf(tmp, "%5d", max);
	color = TERM_L_GREEN;

	c_put_str(color, tmp, ROW_MAXSP, COL_MAXSP + 7);


	put_str("Cur SP ", ROW_CURSP, COL_CURSP);

	sprintf(tmp, "%5d", cur);

	if (cur >= max)
	{
		color = TERM_L_GREEN;
	}
	else if (cur > max / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}

	c_put_str(color, tmp, ROW_CURSP, COL_CURSP + 7);
}

/*
 * Prints depth into the dungeon
 */
void prt_depth(int depth)
{
	char depths[32];

	if (!depth)
	{
		(void)strcpy(depths, "Town");
	}
	else if (depth_in_feet)
	{
		(void)sprintf(depths, "%d ft", depth * 50);
	}
	else
	{
		(void)sprintf(depths, "Lev %d", depth);
	}

	/* Right-Adjust the "depth" and clear old values */
	prt(format("%7s", depths), 23, COL_DEPTH);
}

/*
 * Prints hunger information
 */
void prt_hunger(int food)
{
	if (food < PY_FOOD_FAINT)
	{
		c_put_str(TERM_RED, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
	}

	else if (food < PY_FOOD_WEAK)
	{
		c_put_str(TERM_ORANGE, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
	}

	else if (food < PY_FOOD_ALERT)
	{
		c_put_str(TERM_YELLOW, "Hungry", ROW_HUNGRY, COL_HUNGRY);
	}

	else if (food < PY_FOOD_FULL)
	{
		c_put_str(TERM_L_GREEN, "      ", ROW_HUNGRY, COL_HUNGRY);
	}

	else if (food < PY_FOOD_MAX)
	{
		c_put_str(TERM_L_GREEN, "Full  ", ROW_HUNGRY, COL_HUNGRY);
	}

	else
	{
		c_put_str(TERM_GREEN, "Gorged", ROW_HUNGRY, COL_HUNGRY);
	}
}

/*
 * Prints blindness status
 */
void prt_blind(bool blind)
{
	if (blind)
	{
		c_put_str(TERM_ORANGE, "Blind", ROW_BLIND, COL_BLIND);
	}
	else
	{
		put_str("     ", ROW_BLIND, COL_BLIND);
	}
}

/*
 * Prints confused status
 */
void prt_confused(bool confused)
{
	if (confused)
	{
		c_put_str(TERM_ORANGE, "Confused", ROW_CONFUSED, COL_CONFUSED);
	}
	else
	{
		put_str("        ", ROW_CONFUSED, COL_CONFUSED);
	}
}

/*
 * Prints fear status
 */
void prt_afraid(bool fear)
{
	if (fear)
	{
		c_put_str(TERM_ORANGE, "Afraid", ROW_AFRAID, COL_AFRAID);
	}
	else
	{
		put_str("      ", ROW_AFRAID, COL_AFRAID);
	}
}

/*
 * Prints poisoned status
 */
void prt_poisoned(bool poisoned)
{
	if (poisoned)
	{
		c_put_str(TERM_ORANGE, "Poisoned", ROW_POISONED, COL_POISONED);
	}
	else
	{
		put_str("        ", ROW_POISONED, COL_POISONED);
	}
}

/*
 * Prints paralyzed/searching status
 */
void prt_state(bool paralyzed, bool searching, bool resting)
{
	byte attr = TERM_WHITE;

	char text[16];

	if (paralyzed)
	{
		attr = TERM_RED;

		strcpy(text, "Paralyzed!");
	}

	else if (searching)
	{
		if (player.pclass != CLASS_ROGUE)
		{
			strcpy(text, "Searching ");			
		}
		
		else
		{
			attr = TERM_L_DARK;
			strcpy(text,"Stlth Mode");
		}
		
	}

	else if (resting)
	{
		strcpy(text, "Resting   ");
	}
	else
	{
		strcpy(text, "          ");
	}

	c_put_str(attr, text, ROW_STATE, COL_STATE);
}

/*
 * Prints speed
 */
void prt_speed(int speed)
{
	int attr = TERM_WHITE;
	char buf[32] = "";

	if (speed > 0)
	{
		attr = TERM_L_GREEN;
		sprintf(buf, "Fast (+%d)", speed);
	}

	else if (speed < 0)
	{
		attr = TERM_L_UMBER;
		sprintf(buf, "Slow (%d)", speed);
	}

	/* Display the speed */
	c_put_str(attr, format("%-14s", buf), ROW_SPEED, COL_SPEED);
}

/*
 * Prints ability to gain spells
 */
void prt_study(bool study)
{
	if (study)
	{
		put_str("Study", ROW_STUDY, 64);
	}
	else
	{
		put_str("     ", ROW_STUDY, COL_STUDY);
	}
}

/*
 * Prints cut status
 */
void prt_cut(int cut)
{
	if (cut > 1000)
	{
		c_put_str(TERM_L_RED,  "Mortal wound", ROW_CUT, COL_CUT);
	}
	else if (cut > 200)
	{
		c_put_str(TERM_RED,    "Deep gash   ", ROW_CUT, COL_CUT);
	}
	else if (cut > 100)
	{
		c_put_str(TERM_RED,    "Severe cut  ", ROW_CUT, COL_CUT);
	}
	else if (cut > 50)
	{
		c_put_str(TERM_ORANGE, "Nasty cut   ", ROW_CUT, COL_CUT);
	}
	else if (cut > 25)
	{
		c_put_str(TERM_ORANGE, "Bad cut     ", ROW_CUT, COL_CUT);
	}
	else if (cut > 10)
	{
		c_put_str(TERM_YELLOW, "Light cut   ", ROW_CUT, COL_CUT);
	}
	else if (cut)
	{
		c_put_str(TERM_YELLOW, "Graze       ", ROW_CUT, COL_CUT);
	}
	else
	{
			       put_str("            ", ROW_CUT, COL_CUT);
	}
}

/*
 * Prints stun status
 */
void prt_stun(int stun)
{
	if (stun > 100)
	{
		c_put_str(TERM_RED, "Knocked out ", ROW_STUN, COL_STUN);
	}
	else if (stun > 50)
	{
		c_put_str(TERM_ORANGE, "Heavy stun  ", ROW_STUN, COL_STUN);
	}
	else if (stun)
	{
		c_put_str(TERM_ORANGE, "Stun        ", ROW_STUN, COL_STUN);
	}
	else
	{
		put_str("            ", ROW_STUN, COL_STUN);
	}
}

/*
 * Prints race/class info
 */
void prt_basic(void)
{
	cptr r, c;

	switch(race)
	{
        case RACE_HUMAN: r = "Human"; break;
        case RACE_HALF_ELF: r = "Half-elf"; break;
        case RACE_ELF: r = "Elf"; break;
        case RACE_HOBBIT: r = "Hobbit"; break;
        case RACE_GNOME: r = "Gnome"; break;
        case RACE_DWARF: r = "Dwarf"; break;
        case RACE_HALF_ORC: r = "Half-orc"; break;
        case RACE_HALF_TROLL: r = "Half-troll"; break;
        case RACE_DUNADAN: r = "Dunadan"; break;
        case RACE_HIGH_ELF: r = "High-elf"; break;
        case RACE_KOBOLD: r = "Kobold"; break;
	}

	switch(class)
	{
        case CLASS_WARRIOR: c = "Warrior"; break;
        case CLASS_MAGE: c = "Mage"; break;
        case CLASS_PRIEST: c = "Priest"; break;
        case CLASS_ROGUE: c = "Rogue"; break;
        case CLASS_RANGER: c = "Ranger"; break;
        case CLASS_PALADIN: c = "Paladin"; break;
	}

	prt_field(r, ROW_RACE, COL_RACE);
	prt_field(c, ROW_CLASS, COL_CLASS);
}

/*
 * Redraw the lag bar
 */

void prt_lag(u32b mark, u32b num)
{
	int attr=0;
	static int flipper =0;
	static u32b last=0;

	/* Default to "unknown" */


	Term_erase(COL_LAG, ROW_LAG, 12);
	/* Term_putstr(COL_LAG, ROW_LAG, 12, (flipper%2)?TERM_WHITE:TERM_L_DARK, "LAG: [-----]"); */

	Term_putstr(COL_LAG, ROW_LAG, 12, TERM_L_DARK, "LAG:[------]"); 

	if( num == 10000 ) {
		c_msg_print(format("Time out Mark: %lu, Last: %lu", mark,last));	
		if( num < last ) {
			num=last;
		} else {
			last=num;
		};
		attr=TERM_VIOLET;
	} 

	// if(!(++flipper%1)) c_msg_print(format("Ticks %lu Mark %lu, Last: %lu, New: %lu, Latency %lu.", ticks,mark,last,num,(num+last)/2L));

	num=last=(num+last)/2L;
		
	num /= (4000/6);
	if(!attr) {
		attr=TERM_L_GREEN;
		if(num > 3L) attr=TERM_YELLOW;
		if(num > 5L) attr=TERM_RED;
	};
	Term_putstr(COL_LAG + 5, ROW_LAG, num, attr, "******");
}

/*
 * Redraw the monster health bar
 */

void health_redraw(int num, byte attr)
{
	/* Not tracking */
	if (!attr)
	{
		/* Erase the health bar */
		Term_erase(COL_INFO, ROW_INFO, 12);
	}

	/* Tracking a monster */
	else
	{
		/* Default to "unknown" */
		Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		Term_putstr(COL_INFO + 1, ROW_INFO, num, attr, "**********");
	}
}

/*
 * Choice window "shadow" of the "show_inven()" function. 
 */
static void display_inven(void)
{
	int	i, n, z = 0;
	int	wgt;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Track non-empty slots */
		if (o_ptr->tval) z = i + 1;
	}

	/* Display the inventory */
	for (i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item acceptable? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i, 3, TERM_WHITE, tmp_val);
		
		/* Describe the object */
		strcpy(o_name, inventory_name[i]);

		/* Obtain length of description */
		n = strlen(o_name);

		/* Clear the line with the (possibly indented) index */
		Term_putstr(3, i, n, o_ptr->sval, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			wgt = o_ptr->weight;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = z; i < Term->hgt; i++)
	{
		/* Erase the line */
		Term_erase(0, i, 255);
	}
}


/*
 * Choice window "shadow" of the "show_equip()" function. 
 */
static void display_equip(void)
{
	int	i, n;
	int	wgt;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	/* Find the "final" slot */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item acceptable? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = index_to_label(i);

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);
		
		/* Describe the object */
		strcpy(o_name, inventory_name[i]);

		/* Obtain length of the description */
		n = strlen(o_name);

		/* Clear the line with the (possibly indented) index */
		Term_putstr(3, i - INVEN_WIELD, n, o_ptr->sval, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i - INVEN_WIELD, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			wgt = o_ptr->weight;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(71, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = INVEN_TOTAL - INVEN_WIELD; i < Term->hgt; i++)
	{
		/* Erase the line */
		Term_erase(0, i, 255);
	}
}


/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(void)
{
	int	i, j, k, l, z = 0;
	int	col, len, lim, wgt;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	int	out_index[23];
	byte	out_color[23];
	char	out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Default "max-length" */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	lim -= 9;


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Track non-empty slots */
		if (o_ptr->tval) z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		strcpy(o_name, inventory_name[i]);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the object index, color, and descrtiption */
		out_index[k] = i;
		out_color[k] = o_ptr->sval;
		(void)strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare and index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			wgt = o_ptr->weight;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;
}


/*
 * Display the equipment.
 */
void show_equip(void)
{
	int	i, j, k, l;
	int	col, len, lim, wgt;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	int	out_index[23];
	byte	out_color[23];
	char	out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Default "max-length" */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	lim -= 9;


	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		strcpy(o_name, inventory_name[i]);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the object index, color, and descrtiption */
		out_index[k] = i;
		out_color[k] = o_ptr->sval;
		(void)strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare and index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			wgt = o_ptr->weight * o_ptr->number;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;
}


/*
 * Display inventory in sub-windows
 */
void fix_inven(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_INVEN)) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display inventory */
		display_inven();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Display equipment in sub-windows
 */
void fix_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_EQUIP)) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display inventory */
		display_equip();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Display character sheet in sub-windows
 */
void fix_player(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_PLAYER)) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display inventory */
		display_player();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}

 
/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */

// [grk] Gross hack to display messages in seperate term

#define PMSG_TERM 4

void fix_message(void)
{
        int j, c, i,ii, pmdone, mdone;
        int w, h, pmw, pmh;
        int x, y;
        term *oldt;
	char from_us[30];

	/* Determine what messages from us are prefixed with */
	sprintf(from_us,"[%s]",nick);

        /* Scan windows */
        for (j = 0; j < 8; j++)
        {
                term *old = Term;

                /* No window */
                if (!ang_term[j]) continue;

                /* No relevant flags */
                if (!(window_flag[j] & PW_MESSAGE)) continue;

                /* Activate */
                Term_activate(ang_term[j]);

                /* Get size */
                Term_get_size(&w, &h);

                /* Dump messages */
                i=0; c=0;
                while(i<h)
                {
			byte a;
			cptr msg;

			msg = message_str(c++);

			if (ang_term[PMSG_TERM]) {

				if (msg[0] == '[') continue;
		else if(strstr(msg, "begins a new game")!=NULL) continue;
				else if(strstr(msg, "has entered the game")!=NULL) continue;
				else if(strstr(msg, "has left the game")!=NULL) continue;
				else if(strstr(msg, "committed suicide.")!=NULL) continue;
				else if(strstr(msg, "was killed by")!=NULL) continue;
				else if(strstr(msg, "was slain by")!=NULL) continue;
				else if(strstr(msg, "rises from the dead")!=NULL) continue;
				else if(strstr(msg, "ghost was destroyed by")!=NULL) continue;
		else if(strstr(msg, "has attained level")!=NULL) continue;
		else if(strstr(msg, "Welcome to level")!=NULL) continue;

			} 

			a = TERM_WHITE;

			if(msg[0] == '['){
				a = TERM_L_BLUE;
				if( (strstr(msg, nick)!=NULL) && (strstr(msg, from_us)==NULL) ) a = TERM_L_GREEN;
			} else {
				a=TERM_WHITE;

                if( (strstr(msg, "begins a new game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "has entered the game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "has left the game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "committed suicide")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "ghost was destroyed by")!=NULL)) a=TERM_RED; 
                if( (strstr(msg, "was slain by")!=NULL))
		{
			if (strstr(msg, "Morgoth") != NULL)
				a = TERM_VIOLET;
			else
				a = TERM_YELLOW;
		}
				if( (strstr(msg, "rises from the dead")!=NULL)) a=TERM_ORANGE;
				if( (strstr(msg, "was killed by")!=NULL)) a=TERM_RED;
                if(strstr(msg, "has attained level")!=NULL) a = TERM_L_GREEN;
		if(strstr(msg, "Welcome to level")!=NULL) a = TERM_L_GREEN;
			};

			/* Dump the message on the appropriate line */
			Term_putstr(0, (h - 1) - i, -1, a, msg);

			/* Cursor */
			Term_locate(&x, &y);

			/* Clear to end of line */
			Term_erase(x, y, 255);
		
			i++;
                }

                /* Fresh */
                Term_fresh();

                /* Restore */
                Term_activate(old);
        }

        // Display player messages in term 4
                
	oldt = Term;

	/* No window */
	if (ang_term[PMSG_TERM]) {

		/* Activate */
		Term_activate(ang_term[PMSG_TERM]);

		/* Get size */
		Term_get_size(&w, &h);

		/* Dump messages */
		i=0; c=0;
		while(i<h) {
			byte a;
			cptr msg;

			msg = message_str(c++);

			if(msg[0] == 0) { i++; continue; };

			if(msg[0] == '['){
				a = TERM_L_BLUE;
				if( (strstr(msg, nick)!=NULL) && (strstr(msg, from_us)==NULL) ) a = TERM_L_GREEN;
			} else {
				a=TERM_WHITE;

                if( (strstr(msg, "begins a new game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "has entered the game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "has left the game")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "committed suicide")!=NULL)) a=TERM_L_DARK;
				if( (strstr(msg, "ghost was destroyed by")!=NULL)) a=TERM_RED; 
                if( (strstr(msg, "was slain by")!=NULL))
		{
			if (strstr(msg, "Morgoth") != NULL)
				a = TERM_VIOLET;
			else
				a = TERM_YELLOW;
		}
				if( (strstr(msg, "rises from the dead")!=NULL)) a=TERM_ORANGE;
				if( (strstr(msg, "was killed by")!=NULL)) a=TERM_RED;
		if(strstr(msg, "has attained level")!=NULL) a = TERM_L_GREEN;
		if(strstr(msg, "Welcome to level")!=NULL) a = TERM_L_GREEN;
			};

			if(a != TERM_WHITE) {

				/* Dump the message on the appropriate line */
				Term_putstr(0, (h - 1) - i, -1, a, msg);

				/* Cursor */
				Term_locate(&x, &y);

				/* Clear to end of line */
				Term_erase(x, y, 255);
			
				i++;
			};
		}

                /* Fresh */
                Term_fresh();

		/* Restore */
		Term_activate(oldt);
	}

}

/*
 * Hack -- pass color info around this file
 */
static byte likert_color = TERM_WHITE;


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
		likert_color = TERM_RED;
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
		case 0:
		case 1:
		{
			likert_color = TERM_RED;
			return ("Bad");
		}
		case 2:
		{
			likert_color = TERM_RED;
			return ("Poor");
		}
		case 3:
		case 4:
		{
			likert_color = TERM_YELLOW;
			return ("Fair");
		}
		case 5:
		{
			likert_color = TERM_YELLOW;
			return ("Good");
		}
		case 6:
		{
			likert_color = TERM_YELLOW;
			return ("Very Good");
		}
		case 7:
		case 8:
		{
			likert_color = TERM_L_GREEN;
			return ("Excellent");
		}
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			likert_color = TERM_L_GREEN;
			return ("Superb");
		}
		case 14:
		case 15:
		case 16:
		case 17:
		{
			likert_color = TERM_L_GREEN;
			return ("Heroic");
		}
		default:
		{
			likert_color = TERM_L_GREEN;
			return ("Legendary");
		}
	}
}




/*
 * Hack -- see below
 */
static cptr display_player_flag_names[4][8] =
{
	{
		" Acid:",	/* TR2_RES_ACID */
		" Elec:",	/* TR2_RES_ELEC */
		" Fire:",	/* TR2_RES_FIRE */
		" Cold:",	/* TR2_RES_COLD */
		" Pois:",	/* TR2_RES_POIS */
		" Fear:",	/* TR2_RES_FEAR */
		" Lite:",	/* TR2_RES_LITE */
		" Dark:"	/* TR2_RES_DARK */
	},

	{
		"Blind:",	/* TR2_RES_BLIND */
		"Confu:",	/* TR2_RES_CONFU */
		"Sound:",	/* TR2_RES_SOUND */
		"Shard:",	/* TR2_RES_SHARD */
		"Nexus:",	/* TR2_RES_NEXUS */
		"Nethr:",	/* TR2_RES_NETHR */
		"Chaos:",	/* TR2_RES_CHAOS */
		"Disen:"	/* TR2_RES_DISEN */
	},

	{
		"S.Dig:",	/* TR3_SLOW_DIGEST */
		"Feath:",	/* TR3_FEATHER */
		"PLite:",	/* TR3_LITE */
		"Regen:",	/* TR3_REGEN */
		"Telep:",	/* TR3_TELEPATHY */
		"Invis:",	/* TR3_SEE_INVIS */
		"FrAct:",	/* TR3_FREE_ACT */
		"HLife:"	/* TR3_HOLD_LIFE */
	},

	{
		"Stea.:",	/* TR1_STEALTH */
		"Sear.:",	/* TR1_SEARCH */
		"Infra:",	/* TR1_INFRA */
		"Tunn.:",	/* TR1_TUNNEL */
		"Speed:",	/* TR1_SPEED */
		"Blows:",	/* TR1_BLOWS */
		"Shots:",	/* TR1_SHOTS */
		"Might:"	/* TR1_MIGHT */
	}
};


/*
 * Equippy chars
 */
static void display_player_equippy(int y, int x)
{
	int i;
	byte a;
	char c;
	
	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; ++i)
	{
		/* Get attr/char for display */
		a = p_ptr->hist_flags[0][i-INVEN_WIELD].a;
		c = p_ptr->hist_flags[0][i-INVEN_WIELD].c;

		/* Dump */
		Term_putch(x+i-INVEN_WIELD, y, a, c);
	}
}

static void display_player_sust_info(void)
{
	int i, row, col, stat;
	byte a;
	char c;

	/* Row */
	row = 2;
	/* Column */
	col = 46;

	/* Header */
	c_put_str(TERM_WHITE, "abcdefghijkl@", row-1, col);

	for (stat = 0; stat < 6; ++stat)
	{
		for (i = 0; i < 13; ++i)
		{
			a = p_ptr->hist_flags[stat+1][i].a;
			c = p_ptr->hist_flags[stat+1][i].c;
			/* Dump proper character */
			Term_putch(col+i, row+stat, a, c);
			//c_put_str(a, c, col+i, row+stat);
		}
	
	}

	col = 46;

	/* Footer */
	c_put_str(TERM_WHITE, "abcdefghijkl@", row+6, col);

	/* Equippy */
	display_player_equippy(row+7, col);
}

static void display_player_flag_info(void)
{
	int i, row, col, y, x, off;
	cptr name;

	byte a;
	char c;

	/* Four columns */
	for (x = 0; x < 4; x++)
	{
		/* Reset */
		row = 10;
		col = 20 * x;
		
		/* Header */
		c_put_str(TERM_WHITE, "abcdefghijkl@", row, col+6);

		/* Eight Lines */	
		for (y = 0; y < 8; ++y)
		{

			/* Extract name */
			name = display_player_flag_names[x][y];
	
			/* Name */
			c_put_str(TERM_WHITE, name, row+1, col);

			off = 7 + (x * 8 + y);			
			
			/* Draw dots */
			for (i = 0; i < 13; i++) 
			{
				a = p_ptr->hist_flags[off][i].a;
				c = p_ptr->hist_flags[off][i].c;
				/* Dump proper character */
				Term_putch(col+6+i, row+1, a, c);
			}			
			
			row++;
		}
	
		/* Footer */
		c_put_str(TERM_WHITE, "abcdefghijkl@", row+1, col+6);
		/* Equippy */
	 	display_player_equippy(row+2, col+6);
	}	
	
}

void display_player_stats_info()
{
	int i;
	int row = 2;
	char buf[80];
        /* Display the stats */
        for (i = 0; i < 6; i++)
        {
                /* Special treatment of "injured" stats */
                if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
                {
                        int value;

                        /* Use lowercase stat name */
                        put_str(stat_names_reduced[i], row + i, 61);

                        /* Get the current stat */
                        value = p_ptr->stat_use[i];

                        /* Obtain the current stat (modified) */
                        cnv_stat(value, buf);

                        /* Display the current stat (modified) */
                        c_put_str(TERM_YELLOW, buf, row + i, 66);

                        /* Acquire the max stat */
                        value = p_ptr->stat_top[i];

                        /* Obtain the maximum stat (modified) */
                        cnv_stat(value, buf);

                        /* Display the maximum stat (modified) */
								if (p_ptr->stat_max[i] == 18+100)
									c_put_str(TERM_L_UMBER, buf, row + i, 73);
								else
                        	c_put_str(TERM_L_GREEN, buf, row + i, 73);
                			}

                /* Normal treatment of "normal" stats */
                else
                {
                        /* Assume uppercase stat name */
                        put_str(stat_names[i], row + i, 61);

                        /* Obtain the current stat (modified) */
                        cnv_stat(p_ptr->stat_use[i], buf);

                        /* Display the current stat (modified) */
								if (p_ptr->stat_max[i] == 18+100)
									c_put_str(TERM_L_UMBER, buf, row + i, 66);
								else
	                        c_put_str(TERM_L_GREEN, buf, row + i, 66);
   			   }
	        }
}

static void display_player_misc_info(void)
{
        /* Dump the bonuses to hit/dam */
        prt_num("+ To Hit    ", p_ptr->dis_to_h, 9, 1, TERM_L_BLUE);
        prt_num("+ To Damage ", p_ptr->dis_to_d, 10, 1, TERM_L_BLUE);

        /* Dump the armor class bonus */
        prt_num("+ To AC     ", p_ptr->dis_to_a, 11, 1, TERM_L_BLUE);

        /* Dump the total armor class */
        prt_num("  Base AC   ", p_ptr->dis_ac, 12, 1, TERM_L_BLUE);

        prt_num("Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN);

        if (p_ptr->exp >= p_ptr->max_exp)
        {
                prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_L_GREEN);
        }
        else
        {
                prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_YELLOW);
        }

        prt_lnum("Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);

        if (p_ptr->lev >= PY_MAX_LEVEL)
        {
                put_str("Exp to Adv.", 12, 28);
                c_put_str(TERM_L_GREEN, "    *****", 12, 28+11);
        }
        else
        {
                prt_lnum("Exp to Adv.", exp_adv, 12, 28, TERM_L_GREEN);
        }

        prt_lnum("Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);

        prt_num("Max Hit Points ", p_ptr->mhp, 9, 52, TERM_L_GREEN);

        if (p_ptr->chp >= p_ptr->mhp)
        {
                prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_L_GREEN);
        }
        else if (p_ptr->chp > (p_ptr->mhp) / 10)
        {
                prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_YELLOW);
        }
        else
        {
                prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_RED);
        }

        prt_num("Max SP (Mana)  ", p_ptr->msp, 11, 52, TERM_L_GREEN);

        if (p_ptr->csp >= p_ptr->msp)
        {
                prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_L_GREEN);
        }
        else if (p_ptr->csp > (p_ptr->msp) / 10)
        {
                prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_YELLOW);
        }
        else
        {
                prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_RED);
	     }
}

/*
 * Display the character on the screen (3 different modes)
 *
 * The top two lines, and the bottom line (or two) are left blank.
 *
 * Mode 0 = standard display with skills
 * Mode 1 = standard display with history
 * Mode 2 = special display with equipment flags
 */

void display_player(void)
{
	int i;
	char buf[80];
	cptr desc;

     /* Clear screen */
     Term_clear();

     /* Name, Sex, Race, Class */
     put_str("Name        :", 2, 1);
     put_str("Sex         :", 3, 1);
     put_str("Race        :", 4, 1);
     put_str("Class       :", 5, 1);

     c_put_str(TERM_L_BLUE, nick, 2, 15);
     c_put_str(TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
     c_put_str(TERM_L_BLUE, p_name + race_info[race].name, 4, 15);
     c_put_str(TERM_L_BLUE, c_name + c_info[class].name, 5, 15);

	display_player_stats_info();

	/* Check for history */
	if (char_screen_mode == 2)
	{
		/* Stat/Sustain flags */
		display_player_sust_info();

		/* Other flags */
		display_player_flag_info();
	}
	else
	{
		display_player_misc_info();

     /* Age, Height, Weight, Social */
     prt_num("Age          ", (int)p_ptr->age, 2, 32, TERM_L_BLUE);
     prt_num("Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE);
     prt_num("Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE);
     prt_num("Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE);
	
		if (char_screen_mode) {
			put_str("(Character Background)", 15, 25);
	
			for (i = 0; i < 4; i++)
			{
				put_str(p_ptr->history[i], i + 16, 10);
			}
		
		} 
		else
		{
			put_str("(Miscellaneous Abilities)", 15, 25);
			
			/* Display "skills" */
			put_str("Fighting    :", 16, 1);
			desc = likert(p_ptr->skill_thn, 12);
			c_put_str(likert_color, desc, 16, 15);
	
			put_str("Bows/Throw  :", 17, 1);
			desc = likert(p_ptr->skill_thb, 12);
			c_put_str(likert_color, desc, 17, 15);
	
			put_str("Saving Throw:", 18, 1);
			desc = likert(p_ptr->skill_sav, 6);
			c_put_str(likert_color, desc, 18, 15);
	
			put_str("Stealth     :", 19, 1);
			desc = likert(p_ptr->skill_stl, 1);
			c_put_str(likert_color, desc, 19, 15);
	
	
			put_str("Perception  :", 16, 28);
			desc = likert(p_ptr->skill_fos, 6);
			c_put_str(likert_color, desc, 16, 42);
	
			put_str("Searching   :", 17, 28);
			desc = likert(p_ptr->skill_srh, 6);
			c_put_str(likert_color, desc, 17, 42);
	
			put_str("Disarming   :", 18, 28);
			desc = likert(p_ptr->skill_dis, 8);
			c_put_str(likert_color, desc, 18, 42);
	
			put_str("Magic Device:", 19, 28);
			desc = likert(p_ptr->skill_dev, 6);
			c_put_str(likert_color, desc, 19, 42);
	
	
			put_str("Blows/Round:", 16, 55);
			put_str(format("%d", p_ptr->num_blow), 16, 69);
	
			put_str("Shots/Round:", 17, 55);
			put_str(format("%d", p_ptr->num_fire), 17, 69);
	
			put_str("Infra-Vision:", 19, 55);
			put_str(format("%d feet", p_ptr->see_infra * 10), 19, 69);
		}
	}

}

/*
 * Redraw any necessary windows
 */
void window_stuff(void)
{
	/* Window stuff */
	if (!p_ptr->window) return;

	/* Display inventory */
	if (p_ptr->window & PW_INVEN)
	{
		p_ptr->window &= ~(PW_INVEN);
		fix_inven();
	}

	/* Display equipment */
	if (p_ptr->window & PW_EQUIP)
	{
		p_ptr->window &= (~PW_EQUIP);
		fix_equip();
	}

	/* Display player */
	if (p_ptr->window & PW_PLAYER)
	{
		p_ptr->window &= (~PW_PLAYER);
		fix_player();
	}

	/* Display messages */
	if (p_ptr->window & PW_MESSAGE)
	{
		p_ptr->window &= (~PW_MESSAGE);
		fix_message();
	}
}
