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
		put_str(stat_names/*_reduced*/[stat], row, col);
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
 * Prints "class"
 */
static void prt_pclass(int row, int col)
{
	prt_field(c_name + c_info[pclass].name, row, col);
}

/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_prace(int row, int col)
{
	prt_field(p_name + race_info[race].name, row, col);
}

/*
 * Prints level
 */
static void prt_level(int row, int col)
{
	char tmp[32];

	sprintf(tmp, "%6d", p_ptr->lev);

	if (p_ptr->lev >= p_ptr->max_plv)
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
	if (!c_info[pclass].spell_book) return;

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
	if (!c_info[pclass].spell_book) return;

	put_str("Max SP ", row, col);

	sprintf(tmp, "%5d", p_ptr->msp);

	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}


/*
 * Prints depth into the dungeon
 */
static void prt_depth(int row, int col, int id)
{
	char depths[32];

	/* Hack -- if indicator index is passed, use value from coffers */
	if (id != -1)
	{
		p_ptr->dun_depth = (s16b)coffers[coffer_refs[id]];
	}

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

	/* Memorize if on main screen */
	if (Term == term_screen) mem_line(row, col, 7);
}

#if 0
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
#endif

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

	else if (p_ptr->searching == 2)
	{	
		attr = TERM_L_DARK;

		strcpy(text, "Stlth Mode");
	}

	else if (p_ptr->searching)
	{
		strcpy(text, "Searching ");
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
static void prt_oppose_elements(int row, int col)
{
	/* Number of resists to display */
	int count = 5;

	/* Print up to 5 letters of the resist */
	int n = MIN((Term->wid - col) / count, 5);

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
	for (y = 0; y < Term->hgt; y++)
	{
		Term_erase(y, 0, Term->wid);
		caveprt(stream_cave(0, y), p_ptr->stream_wid[0], DUNGEON_OFFSET_X, y);
	} 
}
/* Very Dirty Hack -- Force Redraw of PRT_FRAME_COMPACT on main screen */
/* Note: this is currently not used by anything, but could prove useful */
void prt_player_hack(void)
{
	int n;
	if (window_flag[0] & PW_PLAYER_2)
	{
		for (n = 1; n < 22; n++)
			Term_erase(0, n, 13);
		//p_ptr->redraw |= (PR_COMPACT | PR_LAG_METER);
		schedule_redraw(PW_PLAYER_2);
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
		/*c_msg_print_aux("Time Out", MSG_LOCAL);*/
		do_handle_message("Time Out", MSG_LOCAL);
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
		my_strcpy(o_name, inventory_name[i], sizeof(o_name));

		/* Obtain length of description */
		n = strlen(o_name);

		/* Clear the line with the (possibly indented) index */
		Term_putstr(3, i, n, o_ptr->sval, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight && Term->wid >= 18)
		{
			Term_erase(Term->wid - 9, i, 9);
			wgt = o_ptr->weight;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(Term->wid - 9, i, -1, TERM_WHITE, tmp_val);
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
		my_strcpy(o_name, inventory_name[i], sizeof(o_name));

		/* Obtain length of the description */
		n = strlen(o_name);

		/* Clear the line with the (possibly indented) index */
		Term_putstr(3, i - INVEN_WIELD, n, o_ptr->sval, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i - INVEN_WIELD, 255);

		/* Display the slot description (if needed) */
		if (show_labels && Term->wid >= 24)
		{
			col = Term->wid - 19;
			Term_erase(col, i - INVEN_WIELD, 255);
			Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
			Term_putstr(col+4, i - INVEN_WIELD, -1, TERM_WHITE, eq_name + eq_names[i]);
		}

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight && Term->wid >= 36)
		{
			wgt = o_ptr->weight;
			col = (show_labels ? Term->wid - 28 : Term->wid - 9);
			Term_erase(col, i - INVEN_WIELD, 9);
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
	size_t	truncate;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	int	out_index[23];
	byte	out_color[23];
	char	out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Default "max-length" */
	len = Term->wid - 1 - col;

	/* Maximum space allowed for descriptions */
	lim = Term->wid - 2;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Hack -- ensure we never try to truncate out of array bounds */
	truncate = lim;
	if (lim < 0) truncate = 0;
	if (truncate > sizeof(o_name) - 1) truncate = sizeof(o_name) - 1;

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
		my_strcpy(o_name, inventory_name[i], sizeof(o_name));

		/* Hack -- enforce max length */
		o_name[truncate] = '\0';

		/* Save the object index, color, and description */
		out_index[k] = i;
		out_color[k] = o_ptr->sval;
		my_strcpy(out_desc[k], o_name, 80);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > Term->wid - 4) ? 0 : (Term->wid - 1 - len);

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
			put_str(tmp_val, j + 1, lim + 1);
		}

		/* Save the index (for other functions to use!) */
		inven_out_index[j] = out_index[j];
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;

	/* Make screen icky */
	if (!screen_icky)
	{
		section_icky_row = j + 2;
		section_icky_col = 0 - (Term->wid - col) - 2;
	}

	/* Hack -- cripple the rest of inven_out_index */
	for ( ; j < 256; j++) inven_out_index[j] = -2;
}


/*
 * Display the equipment.
 */
void show_equip(void)
{
	int	i, j, k, l;
	int	col, len, lim, wgt;
	size_t	truncate;

	object_type *o_ptr;

	char	o_name[80];

	char	tmp_val[80];

	int	out_index[23];
	byte	out_color[23];
	char	out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Default "max-length" */
	len = Term->wid - 1 - col;

	/* Maximum space allowed for descriptions */
	lim = Term->wid - 2;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Hack -- ensure we never try to truncate out of array bounds */
	truncate = lim;
	if (lim < 0) truncate = 0;
	if (truncate > sizeof(o_name) - 1) truncate = sizeof(o_name) - 1;

	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		my_strcpy(o_name, inventory_name[i], sizeof(o_name));

		/* Hack -- enforce max length */
		o_name[truncate] = '\0';

		/* Save the object index, color, and descrtiption */
		out_index[k] = i;
		out_color[k] = o_ptr->sval;
		my_strcpy(out_desc[k], o_name, 80);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > Term->wid - 4) ? 0 : (Term->wid - 1 - len);

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
		if (show_weights && o_ptr->weight && Term->wid >= 18)
		{
			wgt = o_ptr->weight * o_ptr->number;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, lim + 1);
		}

		/* Save the index (for other functions to use!) */
		inven_out_index[j] = out_index[j];
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;

	/* Make screen icky */
	if (!screen_icky)
	{
		section_icky_row = j + 2;
		section_icky_col = 0 - (Term->wid - col) - 2;
	}

	/* Hack -- cripple the rest of inven_out_index */
	for ( ; j < 256; j++) inven_out_index[j] = -2;
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
		if (!flip_inven) display_inven();
		else display_equip();

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
		if (!flip_inven) display_equip();
		else display_inven();

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
		display_player(flip_charsheet);

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
 * Hack -- display player in sub-windows (mode 2)
 */
static void fix_player_2(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER_3))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display flags */
		display_player(1);

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
	prt_field(c_name +c_info[pclass].name, row++, col);

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
		row++;
	else
		prt_equippy(row++, col);


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
	bool use_lagmeter = TRUE; /* TODO: make it a real option. */
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
		/* prt_frame_compact(); */
		/* Equippy or empty row */
		if (use_lagmeter)
		{
			prt_lag(ROW_LAG, COL_LAG);
			prt_equippy(ROW_AC-1, COL_AC);
		}
		else
			prt_equippy(ROW_LAG, COL_LAG);
		
		/* Display relevant indicators */
		redraw_indicators(PW_PLAYER_2);

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
#if 0
	/* Hungry */
	prt_hunger(row, COL_HUNGRY);
#endif
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
	prt_depth(row, COL_DEPTH, -1);

	/* Temp. resists */
	prt_oppose_elements(row, COL_OPPOSE_ELEMENTS);
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
		/*
		prt_status_line();
		*/

		/* Display relevant indicators */
		redraw_indicators(PW_STATUS);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display some remote view in sub-windows.
 */
static void fix_stream(int k)
{
	int j, y;
	int sw, sh;
	int w, h;
	byte win = streams[k].addr;

	/* Get stream bounds */	
	sw = p_ptr->stream_wid[k];
	sh = MIN(p_ptr->stream_hgt[k], last_remote_line[win] + 1);

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* Hack -- there are many ways we can handle sub-window
		 * streams on window 0, but instead of doing any of that,
		 * we just bail out. Something else will take care of the
		 * main window... */
		if (j == 0) continue;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (streams[k].window_flag))) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Get term bounds */
		Term_get_size(&w, &h);

		/* Adjust bounds (clamp stream size by window size) */
		if (sw > w) sw = w;
		if (sh > h) sh = h;

		/* Print it */
		for (y = 0; y < sh; y++)
		{
			Term_erase(0, y, sw);
			caveprt(stream_cave(k,y), sw, 0, y);
		}

		/* Erase rest */
		clear_from(y);

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
			caveprt(stream_cave(window_to_stream[NTERM_WIN_MAP], y), w, 0, y);
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
		item_tester_tval = c_info[pclass].spell_book;

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
			caveprt(stream_cave(window_to_stream[rterm], y), w, 0, y);
		}
		/* Erase rest */
		for (y = y-1; y < h; y++)
		{
			Term_erase(0, 1+y, w);
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
		my_strcpy(buf, msg + 1, sizeof(buf));
		buf[offset - msg - 1] = '\0';
		/* Short text */
		pmsg = msg + (offset - msg) + strlen(from_us) + 1;
		sprintf(text, "[%s] %s", nick, pmsg);
	}
	/* Message To Us */
	else if (strstr(msg, to_us) != NULL)
	{
		/* From who */
		my_strcpy(buf, msg + strlen(to_us), sizeof(buf));
		offset = strstr(msg, "]");
		buf[offset - msg - strlen(to_us)] = '\0';
		/* Short text */
		sprintf(text, "[%s] %s", buf, offset + 2);
	}
	/* Some other kind of message (probably to Your Party) */
	else if ((offset = strstr(msg, ":")))
	{
		/* Destination */
		my_strcpy(buf, msg + 1, sizeof(buf));
		buf[offset - msg - 1] = '\0';
		/* Sender */
		my_strcpy(from_us, offset + 1, sizeof(from_us));
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
		my_strcpy(buf, msg, sizeof(buf));
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

void fix_special_message_aux(byte win)
{
	int j, c, i;
	int w, h, t;
	int x, y, tab;
	cptr msg;
	byte a;
	char text[MSG_LEN];
	
	term *old = Term;

	/* No window */
	if (!win || !ang_term[win]) return;

	/* Activate */
	Term_activate(ang_term[win]);

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
		if ((int)strlen(channels[j].name) + c + 1 >= w)
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
		else if (message_type(c-1) == MSG_YELL)
		{
			/* hack -- "&yell" */
			tab = find_whisper_tab("&yell", text);
			if ( !tab || tab != view_channel ) continue;
		}
		else continue;
		
		i++;
		
		message_color(msg, &a);

		/* Dump the message on the appropriate line(s) */
		j += prt_multi(0, (h - 1) - j, -1, -(h - 1 - (t + 1) - j), a, msg);
#if 0
		/* Cursor */
		Term_locate(&x, &y);

		/* Clear to end of line */
		Term_erase(x, y, 255);
		
		j++;
#endif
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

/* New, proper "fix_special_message" to deal with all relevant terms */
void fix_special_message(void)
{
	int j;
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_MESSAGE_CHAT)) continue;

		/* Call origin function */
		fix_special_message_aux(j);
	}
}

/* Find any opened chat window, or return 0 */
byte find_chat_window(void)
{
	int j;
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_MESSAGE_CHAT)) continue;

		/* XXX XXX XXX special win32 handler :( */
		if (!win32_window_visible(j)) continue;

		return j;
	}
	return 0;
}

void fix_message(void)
{
        int j, c, i;
        int w, h;
        int x, y;

        byte chat_window = find_chat_window();

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
                i = 0; c = 0;
                while (i < h)
		{
			byte a;
			cptr msg;

			msg = message_str(c++);
			
			if (chat_window) {
				if (message_type(c-1) >= MSG_WHISPER) continue;
			}
			
			a = TERM_WHITE;
			message_color(msg, &a);
			
			/* No wrapping, do it the old way */
			if (!wrap_messages)
			{
				/* Dump the message on the appropriate line */
				Term_putstr(0, (h - 1) - i, -1, a, msg);

				/* Cursor */
				Term_locate(&x, &y);
			
				/* Clear to end of line */
				Term_erase(x, y, 255);

				i++;
			}
			else
			{
				/* Dump the message on the appropriate line(s) */
				i += prt_multi(0, (h - 1) - i, -1, -(h - i), a, msg);
			}
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
			caveprt(stream_cave(window_to_stream[j], y), w, 0, y);
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
		a = p_ptr->hist_flags[i-INVEN_WIELD][0].a;
		c = p_ptr->hist_flags[i-INVEN_WIELD][0].c;

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
			a = p_ptr->hist_flags[i][1+stat].a;
			c = p_ptr->hist_flags[i][1+stat].c;
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
				a = p_ptr->hist_flags[i][off].a;
				c = p_ptr->hist_flags[i][off].c;
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
        for (i = 0; i < A_MAX; i++)
        {
                /* Special treatment of "injured" stats */
                if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
                {
                        int value;

                        /* Use lowercase stat name */
                        put_str(stat_names/*_reduced*/[i], row + i, 61);

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

typedef void (*pfcb) (int, int, int); /* "Print Field Call-Back" */
pfcb prt_functions[MAX_INDICATORS];
struct field
{
	cptr	mark;
	s16b	row;
	s16b	col;

	pfcb	field_cb;
	byte	win;
};

/*
 * Schedule redrawing of some indicators based on their window flag.
 *  This should be used instead of p_ptr->redraw |= PR_FOOBAR,
 *  because from now on, PR_* flags are completely server-defined
 *  and might mean something entirely different from what client thinks.
 */
void schedule_redraw(u32b filter)
{
	int i = 0;

	/* For each indicator */
	for (i = 0; i < known_indicators; i++)
	{
		indicator_type *i_ptr = &indicators[i];

		if (indicator_window[i] & filter)
		{
			p_ptr->redraw |= i_ptr->redraw; 
		}
	}
}

/*
 * Redraw large ammount of indicators, filtered by window.
 */
void redraw_indicators(u32b filter)
{
	s16b row;
	int i = 0; 

	/* For each indicator */
	for (i = 0; i < known_indicators; i++)
	{
		indicator_type *i_ptr = &indicators[i];

		if (indicator_window[i] & filter)
		{
			/* Hack: count rows from bottom? */
			row = (i_ptr->row < 0 ? (Term->hgt + i_ptr->row) : i_ptr->row);
#if 0
			if ((row < section_icky_row) && 
				((section_icky_col >= 0 && i_ptr->col < section_icky_col) ||
				 (section_icky_col < 0 && i_ptr->col < 0-section_icky_col))) continue;
			if (screen_icky && !section_icky_row) continue;
#endif
			/* Display field */
			(prt_functions[i])(row, i_ptr->col, i);
		}
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
	u32b filter; /* Indicator filter */

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
     c_put_str(TERM_L_BLUE, c_name + c_info[pclass].name, 5, 15);

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

	/* Hack? Display relevant indicators */
	if (screen_mode == 0) filter = PW_PLAYER_0;
	if (screen_mode == 1) filter = PW_PLAYER_3;
	if (screen_mode == 2) filter = PW_PLAYER_1;

	redraw_indicators(filter);
}

struct field fields[] = 
{
#if 0
	/* Compact display (win = 0) */
	{ "race_",	ROW_RACE,	COL_RACE,	(pfcb)prt_prace,	0},
	{ "class_",	ROW_CLASS,	COL_CLASS,	(pfcb)prt_pclass,	0},
	{ "title_",	ROW_TITLE,	COL_TITLE,	(pfcb)prt_title,	0},
	{ "level",  ROW_LEVEL,	COL_LEVEL,	(pfcb)prt_level,	0},
	{ "exp", 	ROW_EXP,	COL_EXP,	(pfcb)prt_exp,  	0},
	{ "gold", 	ROW_GOLD,	COL_GOLD,	(pfcb)prt_gold, 	0},	

	/* { (PR_EQUIPPY), ROW_EQUIPPY,COL_EQUIPPY, (pfcb)prt_equippy, }, */
	{"",(PR_LAG_METER),ROW_LAG,	COL_LAG,	(pfcb)prt_lag,  	0},

	{ "stat0",	ROW_STAT+0,	COL_STAT,	0,/*prt_stat*/ 0},
	{ "stat1",	ROW_STAT+1,	COL_STAT,	0,/*prt_stat*/ 0},
	{ "stat2",	ROW_STAT+2,	COL_STAT,	0,/*prt_stat*/ 0},
	{ "stat3",	ROW_STAT+3,	COL_STAT,	0,/*prt_stat*/ 0},
	{ "stat4",	ROW_STAT+4,	COL_STAT,	0,/*prt_stat*/ 0},
	{ "stat5",	ROW_STAT+5,	COL_STAT,	0,/*prt_stat*/ 0},

	{ "ac", 	ROW_AC, 	COL_AC, 	(pfcb)prt_ac, 0},
	{ "hp",  	ROW_CURHP,	COL_CURHP,	(pfcb)prt_cur_hp, 0},
	{ "hp",  	ROW_MAXHP,	COL_MAXHP,	(pfcb)prt_max_hp, 0},
	{ "sp",  	ROW_CURSP,	COL_CURSP,	(pfcb)prt_cur_sp, 0},
	{ "sp",  	ROW_MAXSP,	COL_MAXSP,	(pfcb)prt_max_sp, 0},

	{ "track",	ROW_INFO,	COL_INFO,	(pfcb)health_redraw, 0},
	{ "cut", 	ROW_CUT,	COL_CUT,	(pfcb)prt_cut, 0},

	/* Status line (win = 1) */
	{ "hunger", 	-1,	COL_HUNGRY,	(pfcb)prt_hunger,	1},
	{ "blind",  	-1,	COL_BLIND,	(pfcb)prt_blind,	1},
	{ "stund",  	-1,	COL_STUN,	(pfcb)prt_stun, 	1},
	{ "confused",	-1, COL_CONFUSED,(pfcb)prt_confused,1},
	{ "afraid", 	-1,	COL_AFRAID,	(pfcb)prt_afraid,	1},
	{ "poisoned",	-1,	COL_POISONED,(pfcb)prt_poisoned,1},
	{ "state",  	-1,	COL_STATE,	(pfcb)prt_state,	1},
	{ "speed",  	-1,	COL_SPEED,	(pfcb)prt_speed,	1},
	{ "study",  	-1,	COL_STUDY,	(pfcb)prt_study,	1},
#endif
	{ "depth",  	-1,	COL_DEPTH,	(pfcb)prt_depth,	1},
#if 0	
	{ "oppose", 	-1, COL_OPPOSE_ELEMENTS, (pfcb)prt_oppose_elements,	1},
#endif
	/* END */
	{ NULL, 0, 0, 0, 0 }
};
/* Hack -- return a RED-YELLOW-GREEN color based on 2 values weight against each other */
byte color_spotlight(s16b cur, s16b max, bool warn)
{
	s16b n = 1;
	byte color;
	if (warn)
	{
		n = p_ptr->hitpoint_warn;
	}
	if (cur >= max)
	{
		color = TERM_L_GREEN;
	}
	else if (cur > (max * n) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	return color;
}
/* Hack -- return YELLOW-GREEN or GREEN-UMBER color based on 2 values compared against each other */
byte color_dualstat(s16b cur, s16b max, bool shift)
{
	byte colors[3] = { TERM_YELLOW, TERM_L_GREEN, TERM_L_UMBER };
	byte color;
	if (shift > 1)
	{
		shift = 1;
	}
	if (cur >= max)
	{
		color = colors[1 + shift];
	}
	else
	{
		color = colors[0 + shift];
	}
	return color;
}
/* Hack -- our own version of strlen() which treats any control character as lineend */
static int strend(cptr str)
{
	cptr hold = str;
	while(*str > 31) str++;
	return (str - hold);
}
/*
 * Indicator display code.
 * Very heavy, should be optimized/rewritten.
 *
 * Travels along the indicator's "prompt" field, sending ascii characters to screen.
 * Control characters move the carret and the bell (\a) code is overloaded to change
 * colors. '%' symbol starts a printf-like segment, ending with any control character.
 * For example, "Armor: \ar %d", would first display "Armor", then change color to 'r'ed,
 * and then display indicator's value in '%d' format.
 *
 * Each time a piece of data is about to be sent to screen, a series of STRIDING TESTS
 * might occur. Those are defined by populating indicator's "flags" field with 
 * "IN_STRIDE_XXX" bits. If the test doesn't pass, the data is not displayed.
 * Striding tests do not affect control characters, only the actual visual data.
 *
 * A "Vertical Tab" (\v) control character is wildcard modificator with no default 
 * behavior. It is defined in the indicator's "flags" field via "IN_VT_XXX" bits.
 * For example, by setting "IN_VT_CR" bit, each vertical tab whould be treated as
 * a "Carriage Return" (\r) character. 
 *
 * Several other hacks are ofcourse in place. Most are related to the various
 * "IN_" flags, described in "common/defines.h". See also "server/tables.c" for 
 * the actual indicator declarations.
 */
#define advance_coffer() coff++; if (--amnt <= 0) return; val = coffers[coff] 
void prt_indicator(int first_row, int first_col, int id)
{
	indicator_type *i_ptr = &indicators[id];
	int coff = coffer_refs[id];
	char tmp[32], tmp2[32];

	cptr prompt = i_ptr->prompt;

	u32b flag = i_ptr->flag;
	byte amnt = i_ptr->amnt;

	s16b row = first_row;
	u16b col = first_col;

	byte color = TERM_WHITE;

	s32b val = coffers[coff];

	bool value, warn = FALSE, stride = TRUE;
	int n, cut = 0;

	/* Hack -- count rows from bottom of term */
	if (row < 0) row += Term->hgt;

	/* Hack -- count auto-cut */
	if (flag & IN_AUTO_CUT) cut = MIN((Term->wid - col) / amnt, strend(prompt));

	/* Parse prompt */
	for (; !STRZERO(prompt); prompt++)
	{
		value = FALSE;
		switch(*prompt)
		{
 			/* Horizontal tab (Move right) */
			case '\t':
			{
				col++;
				continue;
			}
			/* Backspace (Move left) */
			case '\b':
			{
				col--;
				continue;
			}
			/* Carriage return */
			case '\r':
			{
				col = first_col;
				continue;
			}
			/* Line feed */
			case '\0':
			case '\n':
			{
				row++;
				continue;
			}
			/* Form feed (Next coffer) */
			case '\f':
			{
				advance_coffer();
				continue;
			}
			/* Vertical Tab (Do something flag told us) */
			case '\v':
			{
				if (flag & IN_VT_CR) col = first_col;
				if (flag & IN_VT_LF) row++;
				if (flag & IN_VT_COLOR_RESET) color = TERM_WHITE;
				if (flag & IN_VT_COFFER_RESET) val = coffers[(coff = coffer_refs[id])], amnt = i_ptr->amnt;
				if (flag & IN_VT_DEC_VALUE) val--;
				if (flag & IN_VT_STRIDE_FLIP) stride = !stride;
				if (flag & IN_VT_FF) { advance_coffer(); }
				if (!(flag & IN_VT_COLOR_SET)) continue;
				/* if IN_VT_COLOR_SET is not set, fallthrough */
			}			
			/* Bell (Change color) */
			case '\a':
			{
				/* Read out the color-code */
				prompt++;
				switch (*prompt) 
				{
					case '@': warn = TRUE; /* fallhrough */
					case '#': color = color_spotlight(val, (amnt > 1 ? coffers[coff + 1] : val), warn);
					break;
					case ';': warn = TRUE; /* fallhrough */
					case ':': color = color_dualstat(val, (amnt > 1 ? coffers[coff + 1] : val), warn);
					break;
					case '!': color = (val ? 1 : 0);
					break;
					case ' ': color = (byte) val;
					break;
					default: color = ascii_to_color[*prompt];
					break;
				}
				warn = FALSE;
				continue;
			}
			/* String Format (enable VALUE mode and fall thru) */
			case '%':
			{
				value = TRUE;
			}
			/* Not a control character! */
			default: if ((n = strend(prompt)))
			{
				cptr out = tmp;

				/* Skip this value */
				if (stride)
				{
					bool test_for = TRUE;
					bool passed = FALSE;

					/* Hack -- quit prematurely */
				   	if ((flag & IN_STOP_EMPTY) && (val == 0)) return;

					/* Hack -- test is inverted */
					if (flag & IN_STRIDE_NOT) test_for = FALSE;

					/* Perfrom striding tests */
				    if ((( (flag & IN_STRIDE_POSITIVE) && (val > 0) == test_for) ||
						 ( (flag & IN_STRIDE_NONZERO) && (val != 0) == test_for)) || 
						    ((amnt > 1) && 
						 	(( (flag & IN_STRIDE_EMPTY) && (coffers[coff] == 0) == test_for) ||
							 ( (flag & IN_STRIDE_LARGER) && (val > coffers[coff + 1]) == test_for) ||
							 ( (flag & IN_STRIDE_LESSER) && (val < coffers[coff + 1]) == test_for)
						)))
					{
						/* For each test, we see if it is enabled (flag & IN_STRIDE_* check),
						 * then perform the test. The _EMPTY, _LARGER and _LESSER tests require
						 * at least one succeeding value step to perfrom. */
						passed = TRUE;
					}

 					/* If any of the tests succeeds, the value is being stepped over. */
					if (passed)
					{
						/* Hack -- a stop is requested */ if (flag & IN_STOP_STRIDE) return;
						continue;
					}
				}

				/* Readout value */
				n = MIN(n, sizeof(tmp) - 1);
				strncpy(tmp, prompt, n);
				tmp[n] = '\0';

				/* Advance prompt */
				prompt = prompt + n - 1; 

				/* Format output */
				if ((value) || (flag & IN_TEXT_LABEL))
				{
					if (i_ptr->type == INDITYPE_STRING)
					{
						out = str_coffers[id];
					}
					else if (flag & IN_TEXT_STAT)
					{
						cnv_stat(val, tmp);
						n = 6;
					}
					else if (flag & IN_TEXT_LIKERT)
					{
						strcpy(tmp, likert(val, coffers[coff + 1]));
						color = likert_color;
						//n = 12;
					}
					else if (flag & IN_TEXT_CUT)
					{
						out++;
						cut = val + 1;
						//n--;
					}
					else if (!(flag & IN_TEXT_LABEL))
					{
						strnfmt(tmp2, sizeof(tmp2), tmp, val);
						my_strcpy(tmp, tmp2, sizeof(tmp));
						n = strlen(tmp);
					}
					value = TRUE;
				}

				/* Cut */
				if (cut < 0) cut = 0;
				if (cut)
				{
					if (cut >= sizeof(tmp)) cut = sizeof(tmp)-1;
					tmp[(n = cut)] = '\0';
				}

				/* Send to terminal */
				c_put_str(color, out, row, col);
				/* Memorize if on main screen */
				if (i_ptr->win & (IPW_1 | IPW_2)) mem_line(row, col, n);
				/* Move "cursor" */
				col = col + n;

				/* Hack -- quit prematurely */
				if ((flag & IN_STOP_ONCE) && (value == TRUE)) return;
			}
			break;
		}
	}
}
/* See if a there's a client overload for a server-defined
 * indicator, and if there is, use that instead */
int register_indicator(int id)
{
	struct field *f;
	indicator_type *i = &indicators[id];
	bool found = FALSE;

	/* For each field */
	for (f = &fields[0]; f->field_cb; f++)
	{
			if (!my_stricmp(f->mark, i->mark))
			{
				prt_functions[id] = f->field_cb;
				found = TRUE;
			}
	}

	/* No overloads, use default */
	if (found == FALSE)
		prt_functions[id] = prt_indicator;

	return 0;
}

/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(void)
{
	s16b row;
	int test_ickyness;
	int i = 0; 
	u64b old_redraw = p_ptr->redraw;

	/* Redraw stuff */
	if (!p_ptr->redraw) return;

	/* For each indicator */
	for (i = 0; i < known_indicators; i++)
	{
		indicator_type *i_ptr = &indicators[i];

		if (old_redraw & i_ptr->redraw)
		{
			/* Hack: count rows from bottom? */
			row = (i_ptr->row < 0 ? (Term->hgt + i_ptr->row) : i_ptr->row);

			/* Hack: determine window */
			switch (i_ptr->win)
			{
				case IPW_1:
				case IPW_2:
					test_ickyness = TRUE;
				break;
				case IPW_3:
				case IPW_4:
				case IPW_5:
				case IPW_6:
					test_ickyness = FALSE;
				break;
				default:
					continue;
				break;
			}

			/* HACK: For some indicators, perform ickyness test */
			if (test_ickyness)
			{
				if ((row < section_icky_row) &&
					((section_icky_col >= 0 && i_ptr->col < section_icky_col) ||
					 (section_icky_col < 0 && i_ptr->col < 0-section_icky_col))) continue;
				if (screen_icky && !section_icky_row) continue;
			}

			/* Player disabled display */
			if (!(window_flag[0] & indicator_window[i])) continue;

			/* Display field */
			(prt_functions[i])(row, i_ptr->col, i);

			/* Remove from next update */
			if (i_ptr->redraw & p_ptr->redraw)
			{
				p_ptr->redraw &= ~(i_ptr->redraw);
			}
		}
	}

	/* MAangband-specific local indicator: Lag meter */
	if ((ROW_LAG < section_icky_row) && 
		((section_icky_col >= 0 && COL_LAG < section_icky_col) ||
		 (section_icky_col < 0 && COL_LAG < 0-section_icky_col))) redraw_lag_meter = FALSE;
	if (screen_icky && !section_icky_row) redraw_lag_meter = FALSE;
	if (!(window_flag[0] & PW_PLAYER_2)) redraw_lag_meter = FALSE;
	if (redraw_lag_meter)
	{
		prt_lag(ROW_LAG, COL_LAG);
		redraw_lag_meter = FALSE;
	}
	
}



/*
 * Redraw any necessary windows
 */
void window_stuff(void)
{
	int i;
	u32b window_flags = p_ptr->window; /* Store current flags */

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

	/* Display player (mode 2) */
	if (p_ptr->window & (PW_PLAYER_3))
	{
		p_ptr->window &= ~(PW_PLAYER_3);
		fix_player_2();
	}

	/* Display player (compact) */
	if (p_ptr->window & (PW_PLAYER_2))
	{
		p_ptr->window &= ~(PW_PLAYER_2);
		fix_player_compact();
	}
#if 0
	/* Display map view */
	if (p_ptr->window & (PW_MAP))
	{
		p_ptr->window &= ~(PW_MAP);
		fix_map();
	}
#endif
	/* Display server-defined stream */
	for (i = 0; i < known_streams; i++)
	{
		/* Use classic code: */
		if (p_ptr->window & (streams[stream_group[i]].window_flag))
		{
			p_ptr->window &= ~(streams[stream_group[i]].window_flag);
			fix_stream(stream_group[i]);
		}
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

	/* Display Item list */
	if (p_ptr->window & (PW_ITEMLIST))
	{
		p_ptr->window &= ~(PW_ITEMLIST);
		fix_remote_term(NTERM_WIN_ITEMLIST, PW_ITEMLIST);
	}

	/* Hack -- trigger Term2 event */
	Term_window_updated(window_flags);
}
