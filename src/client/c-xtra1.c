/* Handle the printing of info to the screen */



#include "c-angband.h"

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
static void prt_stat(int stat, int row, int col)
{
	char tmp[32];

	/* Display "injured" stat */
	if (p_ptr->stat_use[stat] < p_ptr->stat_top[stat])
	{
		put_str(stat_names_reduced[stat], row, col);
		cnv_stat(p_ptr->stat_use[stat], tmp);
		c_put_str(TERM_YELLOW, tmp, row, col + 6);
	}

	/* Display "healthy" stat */
	else
	{
		put_str(stat_names[stat], row, col);
		cnv_stat(p_ptr->stat_use[stat], tmp);
		c_put_str(TERM_L_GREEN, tmp, row, col + 6);
	}

	/* Indicate natural maximum */
	if (p_ptr->stat_max[stat] == 18+100)
	{
		put_str("!", row, col + 3);
	}
}


/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title(int row, int col)
{
	prt_field(ptitle, ROW_TITLE, COL_TITLE);
}


/*
 * Prints level
 */
static void prt_level(int row, int col)
{
	char tmp[32];

	sprintf(tmp, "%6d", p_ptr->lev);

	if (p_ptr->lev >= p_ptr->lev) // :( used to be max_lev
	{
		put_str("LEVEL ", row, col);
		c_put_str(TERM_L_GREEN, tmp, row, col + 6);
	}
	else
	{
		put_str("Level ", row, col);
		c_put_str(TERM_YELLOW, tmp, row, col + 6);
	}
}


/*
 * Display the experience
 */
static void prt_exp(int row, int col)
{
	char out_val[32];

	sprintf(out_val, "%8ld", (long)p_ptr->exp);

	if (p_ptr->exp >= p_ptr->max_exp)
	{
		put_str("EXP ", row, col);
		c_put_str(TERM_L_GREEN, out_val, row, col + 4);
	}
	else
	{
		put_str("Exp ", row, col);
		c_put_str(TERM_YELLOW, out_val, row, col + 4);
	}
}


/*
 * Prints current gold
 */
static void prt_gold(int row, int col)
{
	char tmp[32];

	put_str("AU ", row, col);
	sprintf(tmp, "%9ld", (long)p_ptr->au);
	c_put_str(TERM_L_GREEN, tmp, row, col + 3);
}

/*
 * Equippy chars
 */
static void display_player_equippy(int y, int x); // HACK -- prototype
static void prt_equippy(int row, int col)
{
	int i;
	/* Clear */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		Term_putch(col + i - INVEN_WIELD, row, TERM_WHITE, ' ');
	/* Dump */
	display_player_equippy(row, col);
}


/*
 * Prints current AC
 */
static void prt_ac(int row, int col)
{
	char tmp[32];

	put_str("Cur AC ", row, col);
	sprintf(tmp, "%5d", p_ptr->dis_ac + p_ptr->dis_to_a);
	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}


/*
 * Prints Cur hit points
 */
static void prt_cur_hp(int row, int col)
{
	char tmp[32];

	byte color;

	put_str("Cur HP ", row, col);

	sprintf(tmp, "%5d", p_ptr->chp);

	if (p_ptr->chp >= p_ptr->mhp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->chp > (p_ptr->mhp * p_ptr->hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}

	c_put_str(color, tmp, row, col + 7);
}


/*
 * Prints Max hit points
 */
static void prt_max_hp(int row, int col)
{
	char tmp[32];

	put_str("Max HP ", row, col);

	sprintf(tmp, "%5d", p_ptr->mhp);

	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}


/*
 * Prints players max/cur spell points
 */
static void prt_cur_sp(int row, int col)
{
	char tmp[32];
	byte color;


	/* Do not show mana unless it matters */
	if (!c_info[class].spell_book) return;

	put_str("Cur SP ", row, col);

	sprintf(tmp, "%5d", p_ptr->csp);

	if (p_ptr->csp >= p_ptr->msp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csp > p_ptr->msp / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}

	/* Show mana */
	c_put_str(color, tmp, row, col + 7);
}


/*
 * Prints players max/cur spell points
 */
static void prt_max_sp(int row, int col)
{
	char tmp[32];

	/* Do not show mana unless it matters */
	if (!c_info[class].spell_book) return;

	put_str("Max SP ", row, col);

	sprintf(tmp, "%5d", p_ptr->msp);

	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}


/*
 * Prints depth into the dungeon
 */
static void prt_depth(int row, int col)
{
	char depths[32];

	if (!p_ptr->dun_depth)
	{
		(void)strcpy(depths, "Town");
	}
	else if (depth_in_feet)
	{
		(void)sprintf(depths, "%d ft", p_ptr->dun_depth * 50);
	}
	else
	{
		(void)sprintf(depths, "Lev %d", p_ptr->dun_depth);
	}

	/* Right-Adjust the "depth" and clear old values */
	put_str(format("%7s", depths), row, col);
}

/*
 * Prints status of hunger
 */
static void prt_hunger(int row, int col)
{
	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		c_put_str(TERM_RED, "Weak  ", row, col);
	}
 
	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		c_put_str(TERM_ORANGE, "Weak  ", row, col);
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		c_put_str(TERM_YELLOW, "Hungry", row, col);
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		c_put_str(TERM_L_GREEN, "      ", row, col);
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		c_put_str(TERM_L_GREEN, "Full  ", row, col);
	}

	/* Gorged */
	else
	{
		c_put_str(TERM_GREEN, "Gorged", row, col);
	}
}

/*
 * Prints Blind status
 */
static void prt_blind(int row, int col)
{
	if (p_ptr->blind)
	{
		c_put_str(TERM_ORANGE, "Blind", row, col);
	}
	else
	{
		put_str("     ", row, col);
	}
}

/*
 * Prints Confusion status
 */
void prt_confused(int row, int col)
{
	if (p_ptr->confused)
	{
		c_put_str(TERM_ORANGE, "Confused", row, col);
	}
	else
	{
		put_str("        ", row, col);
	}
}

/*
 * Prints fear status
 */
static void prt_afraid(int row, int col)
{
	if (p_ptr->afraid)
	{
		c_put_str(TERM_ORANGE, "Afraid", row, col);
	}
	else
	{
		put_str("      ", row, col);
	}
}

/*
 * Prints poisoned status
 */
static void prt_poisoned(int row, int col)
{
	if (p_ptr->poisoned)
	{
		c_put_str(TERM_ORANGE, "Poisoned", row, col);
	}
	else
	{
		put_str("        ", row, col);
	}
}

/*
 * Prints paralyzed/searching status
 */
static void prt_state(int row, int col)
{
	byte attr = TERM_WHITE;

	char text[16];

	if (p_ptr->paralyzed)
	{
		attr = TERM_RED;

		strcpy(text, "Paralyzed!");
	}

	else if (p_ptr->searching)
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

	else if (p_ptr->resting)
	{
		strcpy(text, "Resting   ");
	}
	else
	{
		strcpy(text, "          ");
	}

	c_put_str(attr, text, row, col);
}

/*
 * Prints speed
 */
static void prt_speed(int row, int col)
{
	int attr = TERM_WHITE;
	char buf[32] = "";

	if (p_ptr->pspeed > 0)
	{
		attr = TERM_L_GREEN;
		sprintf(buf, "Fast (+%d)", p_ptr->pspeed);
	}

	else if (p_ptr->pspeed < 0)
	{
		attr = TERM_L_UMBER;
		sprintf(buf, "Slow (%d)", p_ptr->pspeed);
	}

	/* Display the speed */
	c_put_str(attr, format("%-14s", buf), row, col);
}

/*
 * Prints ability to gain spells
 */
static void prt_study(int row, int col)
{
	if (p_ptr->new_spells)
	{
		put_str("Study", row, col);
	}
	else
	{
		put_str("     ", row, col);
	}
}

/*
 * Prints cut status
 */
static void prt_cut(int row, int col)
{
	int c = p_ptr->cut;

	if (c > 1000)
	{
		c_put_str(TERM_L_RED, "Mortal wound", row, col);
	}
	else if (c > 200)
	{
		c_put_str(TERM_RED, "Deep gash   ", row, col);
	}
	else if (c > 100)
	{
		c_put_str(TERM_RED, "Severe cut  ", row, col);
	}
	else if (c > 50)
	{
		c_put_str(TERM_ORANGE, "Nasty cut   ", row, col);
	}
	else if (c > 25)
	{
		c_put_str(TERM_ORANGE, "Bad cut     ", row, col);
	}
	else if (c > 10)
	{
		c_put_str(TERM_YELLOW, "Light cut   ", row, col);
	}
	else if (c)
	{
		c_put_str(TERM_YELLOW, "Graze       ", row, col);
	}
	else
	{
		put_str("            ", row, col);
	}
}

/*
 * Prints stun status
 */
static void prt_stun(int row, int col)
{
	int s = p_ptr->stun;

	if (s > 100)
	{
		c_put_str(TERM_RED, "Knocked out ", row, col);
	}
	else if (s > 50)
	{
		c_put_str(TERM_ORANGE, "Heavy stun  ", row, col);
	}
	else if (s)
	{
		c_put_str(TERM_ORANGE, "Stun        ", row, col);
	}
	else
	{
		put_str("            ", row, col);
	}
}

/*
 * Display temp. resists
 */
static void prt_oppose_elements(int row, int col, int wid)
{
	/* Number of resists to display */
	int count = 5;

	/* Print up to 5 letters of the resist */
	int n = MIN(wid / count, 5);

	/* Check space */
	if (n <= 0) return;


	if (p_ptr->oppose_acid)
		Term_putstr(col, row, n, TERM_SLATE, "Acid ");
	else
		Term_putstr(col, row, n, TERM_SLATE, "     ");

	col += n;

	if (p_ptr->oppose_elec)
		Term_putstr(col, row, n, TERM_BLUE, "Elec ");
	else
		Term_putstr(col, row, n, TERM_BLUE, "     ");

	col += n;

	if (p_ptr->oppose_fire)
		Term_putstr(col, row, n, TERM_RED, "Fire ");
	else
		Term_putstr(col, row, n, TERM_RED, "     ");

	col += n;

	if (p_ptr->oppose_cold)
		Term_putstr(col, row, n, TERM_WHITE, "Cold ");
	else
		Term_putstr(col, row, n, TERM_WHITE, "     ");

	col += n;

	if (p_ptr->oppose_pois)
		Term_putstr(col, row, n, TERM_GREEN, "Pois ");
	else
		Term_putstr(col, row, n, TERM_GREEN, "     ");

	col += n; /* Unused */
}

/*
 * Print dungeon map. Called prt_map_easy so it wouldn't be
 * called by (& PR_MAP) check accidently.
 */
void prt_map_easy()
{
	int y;
	for (y = 0; y < Client_setup.settings[2]; y++)
	{
		Term_erase(y, 0, Client_setup.settings[1]);
		caveprt(p_ptr->scr_info[y], Client_setup.settings[1], DUNGEON_OFFSET_X, y);
	} 
}
/* Very Dirty Hack -- Force Redraw of PRT_FRAME_COMPACT on main screen */
void prt_player_hack(bool force)
{
	int n;
	if (window_flag[0] & PW_PLAYER_2)
	{
		for (n = 0; n < 22; n++)
			Term_erase(0, n, 13);
		p_ptr->redraw |= (PR_COMPACT | PR_LAG_METER);
		if (force)
			redraw_stuff();
	}
}

/*
 * Redraw the lag bar
 */

static void prt_lag(int row, int col)
{
	int attr=0;
	//static int flipper =0;
	static u32b last=0;

	u32b num = lag_mark;
#if 0
	/* Output lag in miliseconds instead of stars */
	char buf[10];
	buf[0] = '\0';
	float ping = num / 10;
	sprintf(buf, "%6.1fms", ping);
#endif
	/* Default to "unknown" */

	//num/=10;
	//if(num>30000) return; // sometimes we catch a negative flip, just ignore it.


	Term_erase(col, row, 12);
	/* Term_putstr(COL_LAG, ROW_LAG, 12, (flipper%2)?TERM_WHITE:TERM_L_DARK, "LAG: [-----]"); */

	Term_putstr(col, row, 12, TERM_L_DARK, "LAG:[------]"); 

	if( lag_mark == 10000 ) {
		c_msg_print_aux("Time Out", MSG_LOCAL);
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
	Term_putstr(col + 5, row, num, attr, "******");
#if 0
	Term_putstr(col + 4, row, 8, TERM_L_DARK, buf);
#endif
}

/*
 * Redraw the monster health bar
 */

void health_redraw(int row, int col)
{
	int num;
	byte attr;
	
	num = (int)health_track_num;
	attr = health_track_attr; 	
	
	/* Not tracking */
	if (!attr)
	{
		/* Erase the health bar */
		Term_erase(col, row, 12);
	}

	/* Tracking a monster */
	else
	{
		/* Default to "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		Term_putstr(col + 1, row, num, attr, "**********");
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

/* Uses Equip Window ! */
void display_floor_item(void)
{
	byte i = INVEN_TOTAL - INVEN_WIELD;
	/* Floor item! */
	Term_erase(0, i, 255);
	if (floor_item.tval) 
	{
		if (item_tester_okay(&floor_item))
			Term_putstr(0, i, 4, TERM_WHITE, "-)");
		Term_putstr(3, i, strlen(floor_name), floor_item.sval, floor_name);
	}
}

/*
 * Choice window "shadow" of the "show_equip()" function. 
 */
static void display_equip(void)
{
	int	i, n;
	int	wgt;
	int	col;

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

		/* Display the slot description (if needed) */
		if (show_labels)
		{
			Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
			Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, eq_name + eq_names[i]);
		}

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			wgt = o_ptr->weight;
			col = (show_labels ? 52 : 71);
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
		}
	}

	display_floor_item();	
	
	/* Erase the rest of the window */
	for (i = INVEN_TOTAL - INVEN_WIELD + 1; i < Term->hgt; i++)
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
 * Display floor item in equipment sub-window
 */
void fix_floor(void)
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
		display_floor_item();

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
 * Hack -- display player in sub-windows (mode 0)
 */
static void fix_player_0(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER_0))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display player */
		display_player(0);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display player in sub-windows (mode 1)
 */
static void fix_player_1(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER_1))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display flags */
		display_player(2);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack - Display the left-hand-side of the main term in a separate window
 */
static void prt_frame_compact(void)
{
	int row = 0;
	int col = 0;
	int i;
	bool use_lagmeter = TRUE; /* TODO: make it a real option */
	
	/* Name */
	prt_field(nick, row++, col);	
	
	/* Race and Class */
	prt_field(p_name + race_info[race].name, row++, col);
	prt_field(c_name +c_info[class].name, row++, col);

	/* Title */
	prt_field(ptitle, row++, col);

	/* Level/Experience */
	prt_level(row++, col);
	prt_exp(row++, col);

	/* Gold */
	prt_gold(row++, col);

	/* Lag Meter OR Equippy */
	if (use_lagmeter)
		/* Lag Meter (should I add MAngband-specific?) */
		prt_lag(row++, col);
	else
		/* Equippy chars */
		prt_equippy(row++, col);

	/* All Stats */
	for (i = 0; i < A_MAX; i++) prt_stat(i, row++, col);


	/* Equippy or empty row */
	if (use_lagmeter)
		/* Equippy chars */
		prt_equippy(row++, col);
	else
		/* Empty row */
		row++;

	/* Armor */
	prt_ac(row++, col);

	/* Hitpoints */
	prt_max_hp(row++, col);
	prt_cur_hp(row++, col);

	/* Spellpoints */
	prt_max_sp(row++, col);
	prt_cur_sp(row++, col);

	/* Special */
	health_redraw(row++, col);

	/* Cut */
	prt_cut(row++, col);

	/* Stun */
	prt_stun(row++, col);
}


/*
 * Hack -- display player in sub-windows (compact)
 */
static void fix_player_compact(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER_2))) continue;

		/* Hack -- ignore main term */
		if (j == 0) continue;		
		
		/* Activate */
		Term_activate(ang_term[j]);

		/* Display player */
		prt_frame_compact();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack - Display the status line in a separate window
 */
static void prt_status_line(void)
{
	int row = 0;

	/* Hungry */
	prt_hunger(row, COL_HUNGRY);

	/* Blind */
	prt_blind(row, COL_BLIND);

	/* Confused */
	prt_confused(row, COL_CONFUSED);

	/* Afraid */
	prt_afraid(row, COL_AFRAID);

	/* Poisoned */
	prt_poisoned(row, COL_POISONED);

	/* State */
	prt_state(row, COL_STATE);

	/* Speed */
	prt_speed(row, COL_SPEED);

	/* Study */
	prt_study(row, COL_STUDY);

	/* Depth */
	prt_depth(row, COL_DEPTH);

	/* Temp. resists */
	prt_oppose_elements(row, COL_OPPOSE_ELEMENTS,
	                    Term->wid - COL_OPPOSE_ELEMENTS);
}


/*
 * Hack -- display status in sub-windows
 */
static void fix_status(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_STATUS))) continue;

		/* Hack -- ignore main term (it has it's own methods) */
		if (j == 0) continue;		
		
		/* Activate */
		Term_activate(ang_term[j]);

		/* Display status line */
		prt_status_line();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display dungeon map view in sub-windows.
 */
static void fix_map(void)
{
	int j, y;
	int w, h;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MAP))) continue;

		/* Activate */
		Term_activate(ang_term[j]);
		
		/* Get size */
		Term_get_size(&w, &h);

		/* Print map */
		for (y = 0; y < last_remote_line[NTERM_WIN_MAP]+1; y++)
		{
			caveprt(remote_info[NTERM_WIN_MAP][y], w, 0, y);
		}

		/* Erase rest */
		for (y = y-1; y < h; y++)
		{
			Term_erase(0, 1+y, 255);
		}
			
		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display spell list in sub-windows.
 */
static void fix_spells(void)
{
	int b, i, j, y = 0;
	int w, h;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;
		byte old_tester;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_SPELL))) continue;

		/* Activate */
		Term_activate(ang_term[j]);
		
		/* Get size */
		Term_get_size(&w, &h);

		/* Header */
		prt("", y, 1);
		put_str("Name", y, 6);
		put_str("Lv Mana Fail", y, 36);
		y++;
		
		/* Hack: Hijack item tester */
		old_tester = item_tester_tval;
		item_tester_tval = c_info[class].spell_book;

		/* For each book */
		for (b = 0; b < INVEN_PACK - 1; b++)
		{
			if (item_tester_okay(&inventory[b]))
			{
				/* Dump the spells */
				for (i = 0; i < SPELLS_PER_BOOK; i++)
				{
					/* End of terminal */
					if (y >= h) break;

					/* Check for end of the book */
					if (spell_info[b][i][0] == '\0')
					break;
					
					/* skip Illegible */
					if (strstr(spell_info[b][i], "(illegible)"))
					continue;
					
					/* skip uncastable */
					if (!(spell_flag[b*SPELLS_PER_BOOK+i] & PY_SPELL_LEARNED))
					continue;
					
					/* skip forgotten */
					if (spell_flag[b*SPELLS_PER_BOOK+i] & PY_SPELL_FORGOTTEN)
					continue;

					/* Dump the info */
					prt(format("%c", index_to_label(b)), y, 0);
					prt(spell_info[b][i], y, 1);
					
					y++;
					
				}
			}
		}

		/* Restore old tester */
		item_tester_tval = old_tester;

		/* Erase rest */
		for (; y < h; y++)
		{
			Term_erase(0, y, 255);
		}
			
		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display some recall in some sub-windows
 */
static void fix_remote_term(byte rterm, u32b windows)
{
	int y, j;
	int w, h;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (windows))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Get size */
		Term_get_size(&w, &h);		
		
		/* Print data */
		for (y = 0; y < last_remote_line[rterm]+1; y++)
		{
			caveprt(remote_info[rterm][y], w, 0, y);
		}
		/* Erase rest */
		for (y = y-1; y < h; y++)
		{
			Term_erase(0, 1+y, 255);
		}

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}
 
/* Determine message color based on string templates */ 
bool message_color(cptr msg, byte *ap)
{
	char from_us[30];
	byte a;
	
	/* Determine what messages from us are prefixed with */
	sprintf(from_us,"[%s]",nick);

	if(msg[0] == '[') {
				a = TERM_L_BLUE;
				if( (strstr(msg, nick)!=NULL) && (strstr(msg, from_us)==NULL) ) a = TERM_L_GREEN;
	} else {
				a = TERM_WHITE;

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
				if( strstr(msg, "has attained level")!=NULL) a = TERM_L_GREEN;
				if( strstr(msg, "has dropped to level")!=NULL) a = TERM_L_GREEN;
				if( strstr(msg, "Welcome to level")!=NULL) a = TERM_L_GREEN;
	}
	*ap = a;
	return (a != TERM_WHITE); 
}
/*
 * When we got a private message in format "[Recepient:Sender] Message"
 * this function could be used to determine if it relates to any of the 
 * chat tabs opened 
 */
int find_whisper_tab(cptr msg, char *text)
{
	char from_us[30], to_us[30], buf[80];
	int i, tab = 0;
	cptr offset, pmsg;
	
	buf[0] = '\0';
	sprintf(from_us,":%s]",nick);
	sprintf(to_us,"[%s:",nick);

	/* Message From Us */
	if ((offset = strstr(msg, from_us)) != NULL)
	{
		/* To who */
		strcpy(buf, msg + 1);
		buf[offset - msg - 1] = '\0';
		/* Short text */
		pmsg = msg + (offset - msg) + strlen(from_us) + 1;
		sprintf(text, "[%s] %s", nick, pmsg);
	}
	/* Message To Us */
	else if (strstr(msg, to_us) != NULL)
	{
		/* From who */
		strcpy(buf, msg + strlen(to_us));
		offset = strstr(msg, "]");
		buf[offset - msg - strlen(to_us)] = '\0';
		/* Short text */
		sprintf(text, "[%s] %s", buf, offset + 2);
	}
	/* Some other kind of message (probably to Your Party) */
	else if ((offset = strstr(msg, ":")))
	{
		/* Destination */
		strcpy(buf, msg + 1);
		buf[offset - msg - 1] = '\0';
		/* Sender */
		strcpy(from_us, offset + 1);
		pmsg = strstr(offset, "]");
		from_us[pmsg - offset - 1] = '\0';
		/* Short text */
		pmsg = msg + (pmsg - msg) + 2;
		sprintf(text, "[%s] %s", from_us, pmsg);
	}
	/* HACK -- "&xxx" */
	else if (msg[0] == '&')
	{
		/* Dest. */
		strcpy(buf, msg);
		buf[strlen(msg)] = '\0';
	}
	
	if (STRZERO(buf)) return 0;
	
	/* Find related tab */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (STRZERO(channels[i].name)) continue; //is empty
		if (channels[i].id != MAX_CHANNELS) continue; //is channel
		if (strcmp(channels[i].name, buf)) continue; //name mismatch
		
		tab = i;
		break;
	}

	return tab;
}

/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */

#define PMSG_TERM 4
void fix_special_message(void)
{
	int j, c, i;
	int w, h, t;
	int x, y, tab;
	cptr msg;
	byte a;
	char text[80];
		
	term *old = Term;

	/* No window */
	if (!ang_term[PMSG_TERM]) return;

	/* Activate */
	Term_activate(ang_term[PMSG_TERM]);

	/* Get size */
	Term_get_size(&w, &h);

	/* Dump header */
	x = y = 0; /* Temp. values */
	c = t = 0; /* Hor. & Vert. Offsets */
	for (j = 0; j < MAX_CHANNELS; j++)
	{
		/* Skip empty */
		if (STRZERO(channels[j].name)) continue;

		/* Color */
		a = TERM_L_DARK;
		if (p_ptr->on_channel[j] == TRUE) a = TERM_WHITE;
		if (view_channel == j) a = TERM_L_BLUE;

		/* Carriage return */		
		if (strlen(channels[j].name) + c + 1 >= w)
		{
			/* Clear to end of line */
			Term_erase(x, y, 255);
			
			c = 0;
			t++;
		}

		/* Dump the message on the appropriate line */		
		Term_putstr(0 + c, 0 + t, -1, a, channels[j].name);
		
		/* Whitespace */
		Term_locate(&x, &y);
		Term_putstr(x, y, -1, TERM_WHITE, " ");
							
		Term_locate(&x, &y);
		c = x;
	}
	
	/* Clear to end of line */
	Term_erase(x, y, 255);

	/* Dump messages */
	i = j = c = 0; /* Counters */
	while(i < h - (t + 1)) {
		msg = message_str(c++);

		/* Filters */
		if(msg[0] == 0) {i++; continue;}
		if (message_type(c-1) == MSG_WHISPER)
		{
			tab = find_whisper_tab(msg, text);
			if ( tab && tab != view_channel ) continue;
			if ( tab ) msg = text;
		}
		else if (message_type(c-1) >= MSG_CHAT) 
		{
			if ((message_type(c-1) - MSG_CHAT) != channels[view_channel].id) continue;
		}
		else if (message_type(c-1) == MSG_TALK)
		{
		 	/* hack -- "&say" */
			tab = find_whisper_tab("&say", text);
			if ( !tab || tab != view_channel ) continue;
		}
		else continue;
		
		i++;
		
		message_color(msg, &a);

		/* Dump the message on the appropriate line */
		Term_putstr(0, (h - 1) - j, -1, a, msg);

		/* Cursor */
		Term_locate(&x, &y);

		/* Clear to end of line */
		Term_erase(x, y, 255);
		
		j++;
	}

	/* Erase rest */
	while (j < h - (t + 1)) 
	{
		/* Clear line */
		Term_erase(0, (h - 1) - j, 255);
		j++;
	}

	/* Fresh */
	Term_fresh();

	/* Restore */
	Term_activate(old);
}

void fix_message(void)
{
        int j, c, i;
        int w, h;
        int x, y;

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
							if(message_type(c-1) >= MSG_WHISPER) continue;
						} 
			
						a = TERM_WHITE;
						message_color(msg, &a);
			
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
}

/*
 * Hack -- display special recall in sub-windows
 */
static void fix_special_info(void)
{
	int y, j;
	int w, h;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_SPECIAL_INFO))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Get size */
		Term_get_size(&w, &h);		
		
		/* Display special title */
		Term_erase(0, 0, 255);
		Term_putstr(0, 0, 80, TERM_YELLOW, special_line_header);
		
		/* Print map */
		for (y = 0; y < last_remote_line[0]+1; y++)
		{
			caveprt(remote_info[0][y], w, 0, y);
		}

		/* Erase rest */
		for (y = y-1; y < h; y++)
		{
			Term_erase(0, 1+y, 255);
		}
#if 0
		/* Dump text */
		for (i = 0; p_ptr->info[i]; i++)
		{
			if (!p_ptr->info[i]) break;
			Term_erase(0, 1+i, 255);
			Term_putstr(0, 1+i, 80, TERM_WHITE, p_ptr->info[i]);
		}
		
		/* Erase rest */
		for (; i < h; i++)
		{
			Term_erase(0, 1+i, 255);
		}
#endif
		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
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

void display_player(int screen_mode)
{
	int i;
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
	if (screen_mode == 2)
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
	
		if (screen_mode) {
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
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(void)
{
	/* Redraw stuff */
	if (!p_ptr->redraw) return;


	/* Character is not ready yet, no screen updates */
	//if (!character_generated) return;

	/* Character is shopping, hold on */
	/*if (shopping) return;*/

	/* Character is in "icky" mode, no screen updates */
	if (screen_icky) return;

	/* HACK - Redraw window "Display player (compact)" if necessary */
	if (p_ptr->redraw & (PR_MISC | PR_TITLE | PR_LEV | PR_EXP |
	                     PR_STATS | PR_ARMOR | PR_HP | PR_MANA |
	                     PR_GOLD | PR_HEALTH | PR_EQUIPPY | PR_CUT |
	                     PR_STUN | PR_LAG_METER))
	{
		p_ptr->window |= PW_PLAYER_2;
		/* HACK - Player disabled compact view on main terminal */
		if (!(window_flag[0] & PW_PLAYER_2)) {
			p_ptr->redraw &= ~(PR_MISC | PR_TITLE | PR_LEV | PR_EXP |
	                     PR_STATS | PR_ARMOR | PR_HP | PR_MANA |
	                     PR_GOLD | PR_HEALTH | PR_EQUIPPY | PR_CUT |
	                     PR_STUN | PR_LAG_METER);
		}
	}

	/* HACK - Redraw window "Display status" if necessary */
	if (p_ptr->redraw & (PR_HUNGER | PR_BLIND | PR_CONFUSED | PR_AFRAID |
	                     PR_POISONED | PR_STATE | PR_SPEED | PR_STUDY |
	                     PR_DEPTH | PR_OPPOSE_ELEMENTS))
	{
		p_ptr->window |= PW_STATUS;
		/* HACK - Player disabled status on main terminal */
		if (!(window_flag[0] & PW_STATUS) || Term->wid < 80) {
			p_ptr->redraw &= ~(PR_HUNGER | PR_BLIND | PR_CONFUSED | PR_AFRAID |
	                     PR_POISONED | PR_STATE | PR_SPEED | PR_STUDY |
	                     PR_DEPTH | PR_OPPOSE_ELEMENTS);
		}
	}
#if 0
	if (p_ptr->redraw & (PR_MAP))
	{
		p_ptr->redraw &= ~(PR_MAP);
		prt_map();
	}
#endif
	if (p_ptr->redraw & (PR_MISC))
	{
		p_ptr->redraw &= ~(PR_MISC);
		prt_field(p_name + race_info[race].name, ROW_RACE, COL_RACE);
		prt_field(c_name + c_info[class].name, ROW_CLASS, COL_CLASS);
	}

	if (p_ptr->redraw & (PR_TITLE))
	{
		p_ptr->redraw &= ~(PR_TITLE);
		prt_title(ROW_TITLE, COL_TITLE);
	}

	if (p_ptr->redraw & (PR_LEV))
	{
		p_ptr->redraw &= ~(PR_LEV);
		prt_level(ROW_LEVEL, COL_LEVEL);
	}

	if (p_ptr->redraw & (PR_EXP))
	{
		p_ptr->redraw &= ~(PR_EXP);
		prt_exp(ROW_EXP, COL_EXP);
	}

	if (p_ptr->redraw & (PR_STATS))
	{
		int i;

		for (i = 0; i < A_MAX; i++)
			prt_stat(i, ROW_STAT + i, COL_STAT);

		p_ptr->redraw &= ~(PR_STATS);
	}

	if (p_ptr->redraw & (PR_ARMOR))
	{
		p_ptr->redraw &= ~(PR_ARMOR);
		prt_ac(ROW_AC, COL_AC);
	}

	if (p_ptr->redraw & (PR_HP))
	{
		p_ptr->redraw &= ~(PR_HP);
		prt_cur_hp(ROW_CURHP, COL_CURHP);
		prt_max_hp(ROW_MAXHP, COL_MAXHP);
	}

	if (p_ptr->redraw & (PR_MANA))
	{
		p_ptr->redraw &= ~(PR_MANA);
		prt_cur_sp(ROW_CURSP, COL_CURSP);
		prt_max_sp(ROW_MAXSP, COL_MAXSP);
	}

	if (p_ptr->redraw & (PR_GOLD))
	{
		p_ptr->redraw &= ~(PR_GOLD);
		prt_gold(ROW_GOLD, COL_GOLD);
	}

	if (p_ptr->redraw & (PR_EQUIPPY))
	{
		p_ptr->redraw &= ~(PR_EQUIPPY);
		//prt_equippy(ROW_EQUIPPY, COL_EQUIPPY);
	}

	if (p_ptr->redraw & (PR_LAG_METER))
	{
		p_ptr->redraw &= ~(PR_LAG_METER);
		prt_lag(ROW_LAG, COL_LAG);
	}
	
	if (p_ptr->redraw & (PR_DEPTH))
	{
		p_ptr->redraw &= ~(PR_DEPTH);
		prt_depth(ROW_DEPTH, COL_DEPTH);
	}

	if (p_ptr->redraw & PR_OPPOSE_ELEMENTS)
	{
		p_ptr->redraw &= ~PR_OPPOSE_ELEMENTS;
		prt_oppose_elements(ROW_OPPOSE_ELEMENTS, COL_OPPOSE_ELEMENTS,
		                    Term->wid - COL_OPPOSE_ELEMENTS);
	}

	if (p_ptr->redraw & (PR_HEALTH))
	{
		p_ptr->redraw &= ~(PR_HEALTH);
		health_redraw(ROW_INFO, COL_INFO);
	}

	if (p_ptr->redraw & (PR_CUT))
	{
		p_ptr->redraw &= ~(PR_CUT);
		prt_cut(ROW_CUT, COL_CUT);
	}

	if (p_ptr->redraw & (PR_STUN))
	{
		p_ptr->redraw &= ~(PR_STUN);
		prt_stun(ROW_STUN, COL_STUN);
	}

	if (p_ptr->redraw & (PR_HUNGER))
	{
		p_ptr->redraw &= ~(PR_HUNGER);
		prt_hunger(ROW_HUNGRY, COL_HUNGRY);
	}

	if (p_ptr->redraw & (PR_BLIND))
	{
		p_ptr->redraw &= ~(PR_BLIND);
		prt_blind(ROW_BLIND, COL_BLIND);
	}

	if (p_ptr->redraw & (PR_CONFUSED))
	{
		p_ptr->redraw &= ~(PR_CONFUSED);
		prt_confused(ROW_CONFUSED, COL_CONFUSED);
	}

	if (p_ptr->redraw & (PR_AFRAID))
	{
		p_ptr->redraw &= ~(PR_AFRAID);
		prt_afraid(ROW_AFRAID, COL_AFRAID);
	}

	if (p_ptr->redraw & (PR_POISONED))
	{
		p_ptr->redraw &= ~(PR_POISONED);
		prt_poisoned(ROW_POISONED, COL_POISONED);
	}

	if (p_ptr->redraw & (PR_STATE))
	{
		p_ptr->redraw &= ~(PR_STATE);
		prt_state(ROW_STATE, COL_STATE);
	}

	if (p_ptr->redraw & (PR_SPEED))
	{
		p_ptr->redraw &= ~(PR_SPEED);
		prt_speed(ROW_SPEED, COL_SPEED);
	}

	if (p_ptr->redraw & (PR_STUDY))
	{
		p_ptr->redraw &= ~(PR_STUDY);
		prt_study(ROW_STUDY, COL_STUDY);
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

	/* Display status */
	if (p_ptr->window & (PW_STATUS))
	{
		p_ptr->window &= ~(PW_STATUS);
		fix_status();
	}

	/* Display player (mode 0) */
	if (p_ptr->window & (PW_PLAYER_0))
	{
		p_ptr->window &= ~(PW_PLAYER_0);
		fix_player_0();
	}

	/* Display player (mode 1) */
	if (p_ptr->window & (PW_PLAYER_1))
	{
		p_ptr->window &= ~(PW_PLAYER_1);
		fix_player_1();
	}

	/* Display player (compact) */
	if (p_ptr->window & (PW_PLAYER_2))
	{
		p_ptr->window &= ~(PW_PLAYER_2);
		fix_player_compact();
	}
	
	/* Display map view */
	if (p_ptr->window & (PW_MAP))
	{
		p_ptr->window &= ~(PW_MAP);
		fix_map();
	}

	/* Display spell list */
	if (p_ptr->window & (PW_SPELL))
	{
		p_ptr->window &= ~(PW_SPELL);
		fix_spells();
	}	
	
	/* Display messages */
	if (p_ptr->window & PW_MESSAGE)
	{
		p_ptr->window &= (~PW_MESSAGE);
		fix_message();
	}

	/* Display MAngband messages */
	if (p_ptr->window & PW_MESSAGE_CHAT)
	{
		p_ptr->window &= (~PW_MESSAGE_CHAT);
		fix_special_message();
	}
	
	/* Display MAngband special recall */
	if (p_ptr->window & (PW_SPECIAL_INFO))
	{
		p_ptr->window &= ~(PW_SPECIAL_INFO);
		fix_special_info();
	}
	
	/* Display Monster recall */
	if (p_ptr->window & (PW_MONSTER))
	{
		p_ptr->window &= ~(PW_MONSTER);
		fix_remote_term(NTERM_WIN_MONSTER, PW_MONSTER);
	}

	/* Display Monster list */
	if (p_ptr->window & (PW_MONLIST))
	{
		p_ptr->window &= ~(PW_MONLIST);
		fix_remote_term(NTERM_WIN_MONLIST, PW_MONLIST);
	}
}
