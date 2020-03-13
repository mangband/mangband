/* File: misc.c */

/* Purpose: misc code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"




/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val)
{
	/* Above 18 */
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

	/* From 3 to 18 */
	else
	{
		sprintf(out_val, "    %2d", val);
	}
}



/*
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
s16b modify_stat_value(int value, int amount)
{
	int    i;

	/* Reward */
	if (amount > 0)
	{
		/* Apply each point */
		for (i = 0; i < amount; i++)
		{
			/* One point at a time */
			if (value < 18) value++;

			/* Ten "points" at a time */
			else value += 10;
		}
	}

	/* Penalty */
	else if (amount < 0)
	{
		/* Apply each point */
		for (i = 0; i < (0 - amount); i++)
		{
			/* Ten points at a time */
			if (value >= 18+10) value -= 10;

			/* Hack -- prevent weirdness */
			else if (value > 18) value = 18;

			/* One point at a time */
			else if (value > 3) value--;
		}
	}

	/* Return new value */
	return (value);
}





/*
 * Print character stat in given row, column
 */
static void prt_stat(player_type *p_ptr, int stat)
{
	int stat_max;

	/* Stat is maxed */
	if (p_ptr->stat_max[stat] == 18 + 100)
	{
		/* Assume top value *is* the max value */
		stat_max = p_ptr->stat_top[stat];
	}
	else
	{
		/* Set max to an impossibly large value: */
		stat_max = p_ptr->stat_top[stat] + 1;
	}

	send_indication(p_ptr, IN_STAT0 + stat,
		p_ptr->stat_use[stat], p_ptr->stat_top[stat], stat_max);

}




/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title(player_type *p_ptr)
{
	cptr p = "";

	/* Winner */
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = (p_ptr->male ? "**KING**" : "**QUEEN**");
	}

	/* Normal */
	else
	{
		p = c_text + p_ptr->cp_ptr->title[(p_ptr->lev-1)/5];
	}

	/* Ghost */
	if (p_ptr->ghost)
		p = "Ghost";
	/* Fruit bat */
	if (p_ptr->fruit_bat)
		p = "Fruitbat";

	send_indication(p_ptr, IN_TITLE, p);
	send_ghost(p_ptr);
}


/*
 * Prints level
 */
static void prt_level(player_type *p_ptr)
{
	send_indication(p_ptr, IN_LEVEL, MAX(p_ptr->max_plv, p_ptr->lev), p_ptr->lev);
}


/*
 * Display the experience
 */
static void prt_exp(player_type *p_ptr)
{
	int adv_exp;

	if (p_ptr->lev >= PY_MAX_LEVEL)
		adv_exp = 0;
	else adv_exp = (s32b)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L);

	send_indication(p_ptr, IN_EXP, p_ptr->max_exp, p_ptr->exp, adv_exp);
}


/*
 * Prints current gold
 */
static void prt_gold(player_type *p_ptr)
{
	send_indication(p_ptr, IN_GOLD, p_ptr->au);
}



/*
 * Prints current AC
 */
static void prt_ac(player_type *p_ptr)
{
	send_indication(p_ptr, IN_ARMOR, (p_ptr->dis_ac+p_ptr->dis_to_a), p_ptr->dis_ac, p_ptr->dis_to_a );
}


/*
 * Prints Cur/Max hit points
 */
static void prt_hp(player_type *p_ptr)
{
	send_indication(p_ptr, IN_HP, p_ptr->chp, p_ptr->mhp );
}

/*
 * Prints players max/cur spell points
 */
static void prt_sp(player_type *p_ptr)
{
	/* Do not show mana unless it matters */
	if (p_ptr->cp_ptr->spell_book)
	{
		send_indication(p_ptr, IN_SP, p_ptr->csp, p_ptr->msp);
	}
	else
	{
		send_indication(p_ptr, IN_SP, 0, 0);
	}
}


/*
 * Prints depth in stat area
 */
static void prt_depth(player_type *p_ptr)
{
	send_indication(p_ptr, IN_DEPTH, p_ptr->dun_depth);
}


/*
 * Prints status of hunger
 */
static void prt_hunger(player_type *p_ptr)
{
	byte f = 0;

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		f = 0;
	}
 
	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		f = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		f = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		f = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		f = 4;
	}

	/* Gorged */
	else
	{
		f = 5;
	}

	send_indication(p_ptr, IN_FOOD, f);
}


/*
 * Prints Blind status
 */
static void prt_blind(player_type *p_ptr)
{
	if (p_ptr->blind)
	{
		send_indication(p_ptr, IN_BLIND, TRUE);
	}
	else
	{
		send_indication(p_ptr, IN_BLIND, FALSE);
	}
}


/*
 * Prints Confusion status
 */
static void prt_confused(player_type *p_ptr)
{
	if (p_ptr->confused)
	{
		send_indication(p_ptr, IN_CONFUSED, TRUE);
	}
	else
	{
		send_indication(p_ptr, IN_CONFUSED, FALSE);
	}
}


/*
 * Prints Fear status
 */
static void prt_afraid(player_type *p_ptr)
{
	if (p_ptr->afraid)
	{
		send_indication(p_ptr, IN_AFRAID, TRUE);
	}
	else
	{
		send_indication(p_ptr, IN_AFRAID, FALSE);
	}
}


/*
 * Prints Poisoned status
 */
static void prt_poisoned(player_type *p_ptr)
{
	if (p_ptr->poisoned)
	{
		send_indication(p_ptr, IN_POISONED, TRUE);
	}
	else
	{
		send_indication(p_ptr, IN_POISONED, FALSE);
	}
}

/*
 * Prints Opposed Elements
 */
static void prt_oppose_elements(player_type *p_ptr)
{
	send_indication(p_ptr, IN_OPPOSE, p_ptr->oppose_acid, p_ptr->oppose_elec, p_ptr->oppose_fire, p_ptr->oppose_cold, p_ptr->oppose_pois);
}

/*
 * Prints Searching, Resting, Paralysis, or 'count' status
 * Display is always exactly 10 characters wide (see below)
 *
 * This function was a major bottleneck when resting, so a lot of
 * the text formatting code was optimized in place below.
 */
static void prt_state(player_type *p_ptr)
{
	bool p, s, r;

	/* Paralysis */
	if (p_ptr->paralyzed)
	{
		p = TRUE;
	}
	else
	{
		p = FALSE;
	}

	/* Searching */
	if (p_ptr->searching)
	{
		s = TRUE;
		/* Hack -- stealth mode */
		if (p_ptr->cp_ptr->flags & CF_STEALTH_MODE) s = 2;
	}
	else
	{
		s = FALSE;
	}

	/* Resting */
	if (p_ptr->resting)
	{
		r = TRUE;
	}
	else
	{
		r = FALSE;
	}

	send_indication(p_ptr, IN_STATE, p, s, r);
}


/*
 * Prints the speed of a character.			-CJS-
 */
static void prt_speed(player_type *p_ptr)
{
	int i = p_ptr->pspeed;

	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->searching) i += 10;

	send_indication(p_ptr, IN_SPEED, i - 110 );
}

static void prt_study(player_type *p_ptr)
{
	if (p_ptr->new_spells)
	{
		send_indication(p_ptr, IN_STUDY, TRUE);
	}
	else
	{
		send_indication(p_ptr, IN_STUDY, FALSE);
	}
}


static void prt_cut(player_type *p_ptr)
{
	int c = p_ptr->cut;
	int s = 0;
	if (c > 1000)
	{
		s = 7;
	}
	else if (c > 200)
	{
		s = 6;
	}
	else if (c > 100)
	{
		s = 5;
	}
	else if (c > 50)
	{
		s = 4;
	}
	else if (c > 25)
	{
		s = 3;
	}
	else if (c > 10)
	{
		s = 2;
	}
	else if (c)
	{
		s = 1;
	}
	else
	{
		s = 0;
	}

	send_indication(p_ptr, IN_CUT, s);
}



static void prt_stun(player_type *p_ptr)
{
	int s = p_ptr->stun;
	int r;

	if (s > 100)
	{
		r = 3;
	}
	else if (s > 50)
	{
		r = 2;
	}
	else if (s)
	{
		r = 1;
	}
	else
	{
		r = 0;
	}

	send_indication(p_ptr, IN_STUN, r);
}

/*
 * Hack - Display the status line
 */
int cv_put_str(cave_view_type* dest, byte attr, cptr str, int col, int max_col)
{
	int i;
	for (i = 0; i < max_col; i++)
	{
		dest[i+col].a = attr;
		dest[i+col].c = str[i];
	}
	return 1;
}
void c_prt_status_line(player_type *p_ptr, cave_view_type *dest, int len)
{
	char buf[32];
	int col = 0;
	int i, a;
	
	/* Clear */
	for (i = 0; i < len; i++)
	{
		dest[i].a = TERM_WHITE;
		dest[i].c = ' ';
	}

	/* Hungry */
	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
		cv_put_str(dest, TERM_RED, "Weak  ", COL_HUNGRY, 6);
	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
		cv_put_str(dest, TERM_ORANGE, "Weak  ", COL_HUNGRY, 6);
	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
		cv_put_str(dest, TERM_YELLOW, "Hungry", COL_HUNGRY, 6);
	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
		cv_put_str(dest, TERM_L_GREEN, "      ", COL_HUNGRY, 6);
	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
		cv_put_str(dest, TERM_L_GREEN, "Full  ", COL_HUNGRY, 6);
	/* Gorged */
	else
		cv_put_str(dest, TERM_GREEN, "Gorged", COL_HUNGRY, 6);

	/* Blind */
	if (p_ptr->blind)
		cv_put_str(dest, TERM_ORANGE, "Blind", COL_BLIND, 5);

	/* Confused */
	if (p_ptr->confused)
		cv_put_str(dest, TERM_ORANGE, "Confused", COL_CONFUSED, 8);

	/* Afraid */
	if (p_ptr->poisoned)
		cv_put_str(dest, TERM_ORANGE, "Afraid", COL_AFRAID, 6);

	/* Poisoned */
	if (p_ptr->poisoned)
		cv_put_str(dest, TERM_ORANGE, "Poisoned", COL_POISONED, 8);

	/* State */
	a = TERM_WHITE;
	if (p_ptr->paralyzed)
	{
		a = TERM_RED;
		strcpy(buf, "Paralyzed!");
	}
	else if (p_ptr->searching)
	{
		if (!(p_ptr->cp_ptr->flags & CF_STEALTH_MODE))
		{
			strcpy(buf, "Searching ");			
		}
		else
		{
			a = TERM_L_DARK;
			strcpy(buf,"Stlth Mode");
		}
	}
	else if (p_ptr->resting)
	{
		strcpy(buf, "Resting   ");
	}
	else
	{
		strcpy(buf, "          ");
	}
	cv_put_str(dest, a, buf, COL_STATE, 9);

	/* Speed */
	a = TERM_WHITE;
	i = p_ptr->pspeed - 110;
	buf[0] = '\0';
	if (p_ptr->searching) i += 10;
	if (i > 0)
	{
		a = TERM_L_GREEN;
		sprintf(buf, "Fast (+%d)", i);
	}
	else if (i < 0)
	{
		a = TERM_L_UMBER;
		sprintf(buf, "Slow (%d)", i);
	}
	if (!STRZERO(buf))
		cv_put_str(dest, a, format("%-14s", buf), COL_SPEED, 14);

	/* Study */
	if (p_ptr->new_spells)
		cv_put_str(dest, TERM_WHITE, "Study", COL_STUDY, 5);

	/* Depth */
	buf[0] = '\0';	
	if (!p_ptr->dun_depth)
		strcpy(buf, "Town");
	else if (option_p(p_ptr,DEPTH_IN_FEET))
		sprintf(buf, "%d ft", p_ptr->dun_depth * 50);
	else
		sprintf(buf, "Lev %d", p_ptr->dun_depth);
	cv_put_str(dest, TERM_WHITE, format("%7s", buf), COL_DEPTH, 7);
	
	/* Temp. resists */
	col = COL_OPPOSE_ELEMENTS;
	i = MIN((len - COL_OPPOSE_ELEMENTS) / 5, 5);
	if (i > 0)
	{
		if (p_ptr->oppose_acid)
			cv_put_str(dest, TERM_SLATE, "Acid ", col, i);
		col += i;

		if (p_ptr->oppose_elec)
			cv_put_str(dest, TERM_BLUE, "Elec ", col, i);
		col += i;

		if (p_ptr->oppose_fire)
			cv_put_str(dest, TERM_RED, "Fire ", col, i);
		col += i;

		if (p_ptr->oppose_cold)
			cv_put_str(dest, TERM_WHITE, "Cold ", col, i);
		col += i;

		if (p_ptr->oppose_pois)
			cv_put_str(dest, TERM_GREEN, "Pois ", col, i);
		col += i; /* Unused */
	}
}

/*
 * XXX XXX Obtain the "flags" for the player as if he was an item
 */
void player_flags(player_type *p_ptr, u32b *f1, u32b * f2, u32b *f3)
{
	u32b cf = c_info[p_ptr->pclass].flags;
	/* Clear */
	(*f1) = (*f2) = (*f3) = 0L;

	/*
			Welcome to the bright future!
	*/		
	(*f1) |= p_info[p_ptr->prace].flags1;
	(*f2) |= p_info[p_ptr->prace].flags2;
	(*f3) |= p_info[p_ptr->prace].flags3;

	if (cf & CF_BRAVERY_30)
	{
		if (p_ptr->lev >= 30) (*f2) |= (TR2_RES_FEAR);
	}

	/* MAngband-specific: Rogues & Fruit Bats */
	if ( ((cf & CF_SPEED_BONUS) && !option_p(p_ptr,UNSETH_BONUS)) ||
		 p_ptr->fruit_bat)
			*f1 |= TR1_SPEED;

	/* MAngband-specific: Ghost */
	if (p_ptr->ghost) {
		*f3 |= TR3_SEE_INVIS;
		*f2 |= TR2_RES_NETHR;
		*f3 |= TR3_HOLD_LIFE;
		*f2 |= TR2_RES_FEAR;
		*f3 |= TR3_FREE_ACT;
		*f1 |= TR1_INFRA;
	}
}

static void prt_floor_item(player_type *p_ptr)
{
	int Depth = p_ptr->dun_depth;
	cave_type	*c_ptr;
	if (cave[Depth]) {
		c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];
		floor_item_notify(p_ptr, c_ptr->o_idx, TRUE);
	}
}

/*
 * Hack -- see below
 */
static const byte display_player_flag_set[4] =
{
	2,
	2,
	3,
	1
};

/*
 * Hack -- see below
 */
static const u32b display_player_flag_head[4] =
{
	TR2_RES_ACID,
	TR2_RES_BLIND,
	TR3_SLOW_DIGEST,
	TR1_STEALTH
};


static void prt_player_equippy(player_type *p_ptr)
{
	int i;

	byte a;
	char c;

	object_type *o_ptr;

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; ++i)
	{
		/* Object */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty objects */
		if (!o_ptr->k_idx) {
			a = c = 0;
		} else {
			/* Get attr/char for display */
			/*
			a = k_info[o_ptr->k_idx].d_attr;
			c = k_info[o_ptr->k_idx].d_char;
			*/
			a = object_attr_p(p_ptr, o_ptr);
			c = object_char_p(p_ptr, o_ptr);
		}

		/* Dump proper character */
		p_ptr->hist_flags[i-INVEN_WIELD][0].a = a;
		p_ptr->hist_flags[i-INVEN_WIELD][0].c = c;

	}
}


static void prt_player_sust_info(player_type *p_ptr)
{
	int i, row, col, stat, boost;
	object_type *o_ptr;
	object_kind *k_ptr;
	ego_item_type *e_ptr;
	u32b f1, f2, f3, f1_hack = 0;
	u32b ignore_f2, ignore_f3;
	byte a;
	char c;

	ignore_f2 = ignore_f3 = 0L;

	/* Row */
	row = 3;
	/* Column */
	col = 26;
	/* Header */
	//c_put_str(TERM_WHITE, "abcdefghijkl@", row-1, col);
	/* Process equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; ++i)
	{
		/* Get the object */
		o_ptr = &p_ptr->inventory[i];
		/* And it's base/ego */
		k_ptr = &k_info[o_ptr->k_idx];
		e_ptr = &e_info[o_ptr->name2];

		/* Clear flags */
		f1 = f2 = f3 = 0L;
		/* Get the "known" flags */
		object_flags_known(p_ptr, o_ptr, &f1, &f2, &f3);
		/* Hack -- assume stat modifiers are known .. because they can be calculated */
		object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3);

		/* Hack -- make a second set of flags for "bpval" items */
		if (k_ptr->flags1 & TR1_PVAL_MASK) 
			f1_hack = k_ptr->flags1;

		/* Hack -- clear out any pval bonuses that are in the base item */
		if (o_ptr->name2)
			f1 &= ~(k_ptr->flags1 & TR1_PVAL_MASK & ~e_ptr->flags1);

		/* Hack -- same for bpval/randarts */
		if (randart_p(o_ptr))
			f1_hack &= ~(k_ptr->flags1 & TR1_PVAL_MASK);

		/* Initialize color based of sign of pval. 6 -- total num of stats*/
		for (stat = 0; stat < A_MAX; stat++)
		{
			/* Default */
			a = TERM_SLATE;
			c = '.';
			boost = 0;
			
			/* Hack -- precalculate boost */
			if (f1 & (1<<stat))				boost += o_ptr->pval;
			if (f1_hack & (1<<stat))		boost += o_ptr->bpval;
				
			/* Boost */
			if (boost)
			{
				/* Default */
				c = '*';
				/* Good */
				if (boost > 0)
				{
					/* Good */
					a = TERM_L_GREEN;
					/* Label boost */
					if (boost < 10) c = I2D(boost);
				}
				/* Bad */
				if (boost < 0)
				{
					/* Bad */
					a = TERM_RED;
					/* Label boost */
					if (boost > -10) c = I2D(-(boost));
				}
			}
			/* Sustain */
			if (f2 & (1<<stat))
			{
				/* Dark green */
				a = TERM_GREEN;
				/* Convert '.' to 's' */
				if (c == '.') c = 's';
			}
			/* Dump proper character */
			p_ptr->hist_flags[i-INVEN_WIELD][1+stat].a = a;
			p_ptr->hist_flags[i-INVEN_WIELD][1+stat].c = c;
			//Term_putch(col, row+stat, a, c);
		}
		/* Advance */
		col++;
	}
	/* Player flags */
	player_flags(p_ptr, &f1, &f2, &f3);
	/* Check stats */
	for (stat = 0; stat < A_MAX; ++stat)
	{
		/* Default */
		a = TERM_SLATE;
		c = '.';
		/* Sustain */
		if (f2 & (1<<stat))
		{
			/* Dark green "s" */
			a = TERM_GREEN;
			c = 's';
		}
		/* Dump */
		p_ptr->hist_flags[12][1+stat].a = a;
		p_ptr->hist_flags[12][1+stat].c = c;
		//Term_putch(col, row+stat, a, c);
	}
	/* Column */
	col = 26;
	/* Footer */
	//c_put_str(TERM_WHITE, "abcdefghijkl@", row+6, col);
	/* Equippy */
	// display_player_equippy(row+7, col);
}

static void prt_player_flag_info(player_type *p_ptr)
{
	int x, y, i, n;

	int row, col;
	
	int realX, realY;

	int set;
	u32b head;
	u32b flag;
	//cptr name;


	u32b f[4];

	byte attr = TERM_SLATE;
	char c = '.';

	object_type *o_ptr;

	realX = 0;
	realY = 7;

	/* Four columns */
	for (x = 0; x < 4; x++)
	{
		/* Reset */
		row = 11;
		col = 20 * x;

		/* Extract set */
		set = display_player_flag_set[x];

		/* Extract head */
		head = display_player_flag_head[x];

		/* Header */
		//c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);
		
		/* Eight rows */
		for (y = 0; y < 8; y++)
		{
			/* Extract flag */
			flag = (head << y);


			/* Extract name */
			//name = display_player_flag_names[x][y];

			/* Header */
			//c_put_str(TERM_WHITE, name, row, col);

			/* Check equipment */
			for (n = 6, i = INVEN_WIELD; i < INVEN_TOTAL; ++i, ++n)
			{

				/* Object */
				o_ptr = &p_ptr->inventory[i];
				
				/* Default Value */
				attr = TERM_SLATE;
				c = '.';

				/* Clear flags */
				f[1] = f[2] = f[3] = 0L;

				/* Fill in Known flags */
				if (o_ptr->k_idx) /* don't waste time */
				{
					object_flags_known(p_ptr, o_ptr, &f[1], &f[2], &f[3]);

					/* Hack -- additional lite flag */				
					if (i == INVEN_LITE)
						if (artifact_p(o_ptr) || k_info[o_ptr->k_idx].sval == SV_LITE_DWARVEN || k_info[o_ptr->k_idx].sval == SV_LITE_FEANOR)
							f[3] |= TR3_LITE;
				}

				/* Color columns by parity */
				if (n % 2) attr = TERM_L_WHITE;

				/* Non-existant objects */
				if (!o_ptr->k_idx) attr = TERM_L_DARK; 
				
				/* Hack -- Check immunities */
				if ((x == 0) && (y < 4) &&
				    (f[set] & ((TR2_IM_ACID) << y)))
				{
					attr = TERM_WHITE;
					c = '*';
					//c_put_str(TERM_WHITE, "*", row, col+n);
					//p_ptr->hist_flags[realX][realY].a = TERM_WHITE;
					//p_ptr->hist_flags[realX][realY].c = '*';
				}

				/* Check flags */
				else if (f[set] & flag)
				{
					//c_put_str(TERM_WHITE, "+", row, col+n);
					attr = TERM_WHITE;
					c = '+';
					//p_ptr->hist_flags[realX][realY].a = TERM_WHITE;
					//p_ptr->hist_flags[realX][realY].c = '+';
				}

				/* Default */
				else
				{
					//c_put_str(attr, ".", row, col+n);
					//p_ptr->hist_flags[realX][realY].a = attr;
					//p_ptr->hist_flags[realX][realY].c = c;
				}

				p_ptr->hist_flags[realX][realY].a = attr;
				p_ptr->hist_flags[realX][realY].c = c;
				
				realX++;
			}

			/* Clear */
			f[1] = f[2] = f[3] = 0L;

			/* Player flags */
			player_flags(p_ptr, &f[1], &f[2], &f[3]);

			/* Default */
			//c_put_str(TERM_SLATE, ".", row, col+n);
			p_ptr->hist_flags[realX][realY].a = TERM_SLATE;
			p_ptr->hist_flags[realX][realY].c = '.';

			/* Hack -- Check immunities */
			if ((x == 0) && (y < 4) &&
			    (f[set] & ((TR2_IM_ACID) << y)))
			{
				//c_put_str(TERM_WHITE, "*", row, col+n);
				p_ptr->hist_flags[realX][realY].a = TERM_WHITE;
				p_ptr->hist_flags[realX][realY].c = '*';
			}

			/* Check flags */
			else if (f[set] & flag) {
				//c_put_str(TERM_WHITE, "+", row, col+n);
				p_ptr->hist_flags[realX][realY].a = TERM_WHITE;
				p_ptr->hist_flags[realX][realY].c = '+';
			}

			/* Advance */
			row++;

			realY++; realX = 0;
		}

		/* Footer */
		//c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);

		/* Equippy */
		//display_player_equippy(row++, col+6);
	}
}


static void prt_flags(player_type *p_ptr)
{
	/* player_type *p_ptr = Players[Ind]; */
	int i;

	prt_player_equippy(p_ptr);
	prt_player_sust_info(p_ptr);
	prt_player_flag_info(p_ptr);

	for (i = 0; i < MAX_OBJFLAGS_ROWS; i++)
	{
		send_objflags(p_ptr, i);
	}
}


void prt_history(player_type *p_ptr)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		send_indication(p_ptr, IN_HISTORY0 + i, p_ptr->history[i]);
	}

	send_indication(p_ptr, IN_NAME, p_ptr->name);
	send_indication(p_ptr, IN_GENDER, p_ptr->male ? "Male" : "Female");
	send_indication(p_ptr, IN_RACE, p_name + p_info[p_ptr->prace].name);
	send_indication(p_ptr, IN_CLASS, c_name + c_info[p_ptr->pclass].name);
}
void prt_misc(player_type *p_ptr)
{
	send_indication(p_ptr, IN_NAME, p_ptr->name);
	send_indication(p_ptr, IN_RACE, p_name + p_info[p_ptr->prace].name);
	send_indication(p_ptr, IN_CLASS, c_name + c_info[p_ptr->pclass].name);
}

static void prt_various(player_type *p_ptr)
{
	send_indication(p_ptr, IN_VARIOUS, p_ptr->age, p_ptr->ht, p_ptr->wt, p_ptr->sc);
}

static void prt_plusses(player_type *p_ptr)
{
	int show_tohit = p_ptr->dis_to_h;
	int show_todam = p_ptr->dis_to_d;

	object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];

	if (object_known_p(p_ptr, o_ptr)) show_tohit += o_ptr->to_h;
	if (object_known_p(p_ptr, o_ptr)) show_todam += o_ptr->to_d;

	send_indication(p_ptr, IN_PLUSSES, show_tohit, show_todam);
}

static void prt_skills(player_type *p_ptr)
{
	s16b skills[11];
	s16b factors[11];
	int tmp;
	object_type *o_ptr;

	/* Fighting skill */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];
	tmp = p_ptr->to_h + o_ptr->to_h;
	skills[0] = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);
	factors[0] = 12;

	/* Shooting skill */
	o_ptr = &p_ptr->inventory[INVEN_BOW];
	tmp = p_ptr->to_h + o_ptr->to_h;
	skills[1] = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);
	factors[1] = 12;

	/* Basic abilities */
	skills[2] = p_ptr->skill_sav;
	factors[2] = 6;
	skills[3] = p_ptr->skill_stl;
	factors[3] = 1;
	skills[4] = p_ptr->skill_fos;
	factors[4] = 6;
	skills[5] = p_ptr->skill_srh;
	factors[5] = 6;
	skills[6] = p_ptr->skill_dis;
	factors[6] = 8;
	skills[7] = p_ptr->skill_dev;
	factors[7] = 6;

	/* Number of blows */
	skills[8] = p_ptr->num_blow;
	skills[9] = p_ptr->num_fire;

	/* Infravision */
	skills[10] = p_ptr->see_infra;

	send_indication(p_ptr, IN_SKILLS,
		skills[0], factors[0],
		skills[4], factors[4],
		skills[1], factors[1],
		skills[5], factors[5],
		skills[2], factors[2],
		skills[6], factors[6],
		skills[3], factors[3],
		skills[7], factors[7]);

	send_indication(p_ptr, IN_SKILLS2,	skills[8], skills[9], skills[10]);
}

/*
 * Redraw the cursor
 *
 * This function must simply calculate correct offset for each player
 * and update his cursor location
 */
static void cursor_redraw(player_type *p_ptr)
{
	int vis, x, y = 0;

	/* Not tracking */
	if (p_ptr->cursor_who == 0)
	{
		/* Reset the cursor */
		vis = 0;
	}

	/* Tracking a hallucinatory monster */
	/* commented: this can't happen as 'looking' doesn't work while hallucinating */
	/* else if (p_ptr->image)
	{
		
	}
	*/
	/* Tracking a player */
	else if (p_ptr->cursor_who < 0)
	{
		player_type *q_ptr;
		/* Make sure we have a valid index */
		if (0 - p_ptr->cursor_who > NumPlayers)
		{
			/* Invalid index -- reset the cursor */
			send_cursor(p_ptr, 0,0,0);
			/* Reset the index */
			p_ptr->cursor_who = 0;
			return;
		}
		
		q_ptr = Players[0 - p_ptr->cursor_who];

		/* Tracking a bad player (?) */
		if (!q_ptr)
		{
			/* Reset the cursor */
			vis = 0;
		}

		/* Tracking an unseen player */
		else if (!p_ptr->play_vis[0 - p_ptr->cursor_who])
		{
			/* Should not be possible */
			vis = 0;
		}

		/* Tracking a visible player */
		else
		{
			vis = 1;
			x = q_ptr->px - p_ptr->panel_col_prt;
			y = q_ptr->py - p_ptr->panel_row_prt;
		}
	}

	/* Tracking a bad monster (?) */
	else if (!m_list[p_ptr->cursor_who].r_idx)
	{
		/* Reset the cursor */
		vis = 0;
	}

	/* Tracking an unseen monster */
	else if (!p_ptr->mon_vis[p_ptr->cursor_who])
	{
		/* Reset cursor */
		vis = 0;
	}

	/* Tracking a dead monster (???) */
	else if (m_list[p_ptr->cursor_who].hp < 0)
	{
		/* Reset cursor */
		vis = 0;
	}

	/* Tracking a visible monster */
	else
	{
		monster_type *m_ptr = &m_list[p_ptr->cursor_who];
		
		vis = 1;
		x = m_ptr->fx - p_ptr->panel_col_prt;
		y = m_ptr->fy - p_ptr->panel_row_prt;
	}
	
	if (vis == 1)
	{
		send_cursor(p_ptr, vis, x, y);
	}
	else
	{
		send_cursor(p_ptr, 0, 0, 0);

		/* Cancel tracking */
		p_ptr->cursor_who = 0;
	}

}



/*
 * Redraw the "monster health bar"	-DRS-
 * Rather extensive modifications by	-BEN-
 *
 * The "monster health bar" provides visual feedback on the "health"
 * of the monster currently being "tracked".  There are several ways
 * to "track" a monster, including targetting it, attacking it, and
 * affecting it (and nobody else) with a ranged attack.
 *
 * Display the monster health bar (affectionately known as the
 * "health-o-meter").  Clear health bar if nothing is being tracked.
 * Auto-track current target monster when bored.  Note that the
 * health-bar stops tracking any monster that "disappears".
 */


static void health_redraw(player_type *p_ptr)
{
#ifdef DRS_SHOW_HEALTH_BAR
	byte attr = 0;
	int len = 0;

	/* Not tracking */
	if (p_ptr->health_who == 0)
	{
		/* Erase the health bar */
	}

	/* Tracking a hallucinatory monster */
	else if (p_ptr->image)
	{
		/* Indicate that the monster health is "unknown" */
		attr = TERM_WHITE;
	}

	/* Tracking a player */
	else if (p_ptr->health_who < 0)
	{
		player_type *q_ptr;
		/* Make sure we have a valid index */
		if (0 - p_ptr->health_who > NumPlayers)
		{
			/* Invalid index -- erase the health bar */
			/* Reset the index */
			p_ptr->health_who = 0;
		}
		
		q_ptr = Players[0 - p_ptr->health_who];

		/* Tracking a bad player (?) */
		if (!q_ptr)
		{
			/* Erase the health bar */
		}

		/* Tracking an unseen player */
		else if (!p_ptr->play_vis[0 - p_ptr->health_who])
		{
			/* Indicate that the player health is "unknown" */
			attr = TERM_WHITE;
		}

		/* Tracking a visible player */
		else
		{
			int pct;

			/* Default to almost dead */
			attr = TERM_RED;

			/* Extract the "percent" of health */
			pct = 100L * q_ptr->chp / q_ptr->mhp;

			/* Badly wounded */
			if (pct >= 10) attr = TERM_L_RED;

			/* Wounded */
			if (pct >= 25) attr = TERM_ORANGE;

			/* Somewhat Wounded */
			if (pct >= 60) attr = TERM_YELLOW;

			/* Healthy */
			if (pct >= 100) attr = TERM_L_GREEN;

			/* Afraid */
			if (q_ptr->afraid) attr = TERM_VIOLET;

			/* Asleep (?) */
			if (q_ptr->paralyzed) attr = TERM_BLUE;

			/* Convert percent into "health" */
			len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;
		}
	}

	/* Tracking a bad monster (?) */
	else if (!m_list[p_ptr->health_who].r_idx)
	{
		/* Erase the health bar */
	}

	/* Tracking an unseen monster */
	else if (!p_ptr->mon_vis[p_ptr->health_who])
	{
		/* Indicate that the monster health is "unknown" */
		attr = TERM_WHITE;
	}

	/* Tracking a dead monster (???) */
	else if (m_list[p_ptr->health_who].hp < 0)
	{
		/* Indicate that the monster health is "unknown" */
		attr = TERM_WHITE;
	}

	/* Tracking a visible monster */
	else
	{
		int pct;

		monster_type *m_ptr = &m_list[p_ptr->health_who];

		/* Default to almost dead */
		attr = TERM_RED;

		/* Extract the "percent" of health */
		pct = 100L * m_ptr->hp / m_ptr->maxhp;

		/* Badly wounded */
		if (pct >= 10) attr = TERM_L_RED;

		/* Wounded */
		if (pct >= 25) attr = TERM_ORANGE;

		/* Somewhat Wounded */
		if (pct >= 60) attr = TERM_YELLOW;

		/* Healthy */
		if (pct >= 100) attr = TERM_L_GREEN;

		/* Afraid */
		if (m_ptr->monfear) attr = TERM_VIOLET;

		/* Asleep */
		if (m_ptr->csleep) attr = TERM_BLUE;

		/* Convert percent into "health" */
		len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;
	}

	send_indication(p_ptr, IN_MON_HEALTH, (s16b)attr, (s16b)len);
#endif
}



/*
 * Hack -- display monsters in sub-windows
 */
static void fix_monlist(player_type *p_ptr)
{
	/* Hack -- do nothing if player is not subscribed */
	if (!p_ptr->stream_hgt[STREAM_MONLIST_TEXT]) return;

	/* HACK -- Save other player info */
	text_out_save(p_ptr);

	/* Prepare 'visible monsters' list */
	display_monlist(p_ptr);

	/* Send it */
	send_prepared_info(p_ptr, NTERM_WIN_MONLIST, STREAM_MONLIST_TEXT, 0);

	/* HACK -- Load other player info */
	text_out_load(p_ptr);

	return;
}


/*
 * Hack -- display dungeon items in sub-windows
 */
static void fix_itemlist(player_type *p_ptr)
{
	/* Hack -- do nothing if player is not subscribed */
	if (!p_ptr->stream_hgt[STREAM_ITEMLIST_TEXT]) return;

	/* HACK -- Save other player info */
	text_out_save(p_ptr);

	/* Prepare 'visible monsters' list */
	display_itemlist(p_ptr);

	/* Send it */
	send_prepared_info(p_ptr, NTERM_WIN_ITEMLIST, STREAM_ITEMLIST_TEXT, 0);

	/* HACK -- Load other player info */
	text_out_load(p_ptr);

	return;
}


/*
 * Hack -- display inventory in sub-windows
 */
static void fix_inven(player_type *p_ptr)
{
	/* Resend the inventory */
	display_inven(p_ptr);
}



/*
 * Hack -- display equipment in sub-windows
 */
static void fix_equip(player_type *p_ptr)
{
	/* Resend the equipment */
	display_equip(p_ptr);
}


/*
 * XXX XXX XXX XXX
 */
extern void display_spell_list(void);

/*
 * Hack -- display equipment in sub-windows
 */
static void fix_spell(player_type *p_ptr)
{
	int i;

	/* Ghosts get a different set */
	if (p_ptr->ghost)
	{
		show_ghost_spells(p_ptr);
		return;
	}

	/* Warriors don't need this */
	if (!p_ptr->cp_ptr->spell_book)
		return;

#if 0
	/* Check for blindness and no lite and confusion */
	if (p_ptr->blind || no_lite(p_ptr) || p_ptr->confused)
	{
		return;
	}
#endif

	/* Scan for appropriate books */
	for (i = 0; i < INVEN_WIELD; i++)
	{
		if (p_ptr->inventory[i].tval == p_ptr->cp_ptr->spell_book)
		{
			do_cmd_browse(p_ptr, i);
		}
	}


#if 0
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!ang_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & PW_SPELL)) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Display spell list */
		display_spell_list();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
#endif
}


/*
 * Hack -- display character in sub-windows
 */
static void fix_player(player_type *p_ptr)
{
}



/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */
static void fix_message(player_type *p_ptr)
{
}


/*
 * Hack -- display overhead view in sub-windows
 *
 * Note that the "player" symbol does NOT appear on the map.
 */
static void fix_overhead(player_type *p_ptr)
{
}


/*
 * Hack -- display mini-map view in sub-windows
 *
 * Note that the "player" symbol does NOT appear on the map.
 */
static void fix_map(player_type *p_ptr)
{
	display_map(p_ptr, TRUE);
}


/*
 * Hack -- display monster recall in sub-windows
 */
static void fix_monster(player_type *p_ptr)
{
	/* HACK -- Save other player info */
	text_out_save(p_ptr);

	do_cmd_monster_desc_aux(p_ptr, p_ptr->monster_race_idx, TRUE);

	/* HACK -- Load other player info */
	text_out_load(p_ptr);
}


/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(player_type *p_ptr)
{
	int			i, j, k, levels;
	int			num_allowed, num_known;
	int percent_spells;

	magic_type		*s_ptr;

	cptr p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? "prayer" : "spell");

	int mtype = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0);

	/* Hack -- must be literate */
	if (!p_ptr->cp_ptr->spell_book) return;


	/* Determine the number of spells allowed */
	levels = p_ptr->lev - p_ptr->cp_ptr->spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Number of 1/100 spells per level */
	percent_spells = adj_mag_study[p_ptr->stat_ind[p_ptr->cp_ptr->spell_stat]];

	/* Extract total allowed spells (rounded up) */
	num_allowed = (((percent_spells * levels) + 50) / 100);

	/* Assume none known */
	num_known = 0;

	/* Count the number of spells we know */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Count known spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			num_known++;
		}
	}

	/* See how many spells we must forget or may learn */
	p_ptr->new_spells = num_allowed - num_known;



	/* Forget spells which are too hard */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Access the spell */
		j = p_ptr->spell_order[i];

		/* Skip non-spells */
		if (j >= 99) continue;

		/* Get the spell */
		s_ptr = &p_ptr->mp_ptr->info[j];

		/* Skip spells we are allowed to know */
		if (s_ptr->slevel <= p_ptr->lev) continue;

		/* Is it known? */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg_format(p_ptr, "You have forgotten the %s of %s.", p,
			           get_spell_name(p_ptr->cp_ptr->spell_book,j));


			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}


	/* Forget spells if we know too many spells */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Stop when possible */
		if (p_ptr->new_spells >= 0) break;

		/* Get the (i+1)th spell learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) continue;

		/* Forget it (if learned) */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg_format(p_ptr, "You have forgotten the %s of %s.", p,
			           spell_names[mtype][j]);


			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}


	/* Check for spells to remember */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* None left to remember */
		if (p_ptr->new_spells <= 0) break;

		/* Get the next spell we learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) break;

		/* Access the spell */
		s_ptr = &p_ptr->mp_ptr->info[j];

		/* Skip spells we cannot remember */
		if (s_ptr->slevel > p_ptr->lev) continue;

		/* First set of spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_FORGOTTEN)
		{
			/* No longer forgotten */
			p_ptr->spell_flags[j] &= ~PY_SPELL_FORGOTTEN;

			/* Known once more */
			p_ptr->spell_flags[j] |= PY_SPELL_LEARNED;

			/* Message */
			msg_format(p_ptr, "You have remembered the %s of %s.",
			           p, spell_names[mtype][j]);


			/* One less can be learned */
			p_ptr->new_spells--;
		}
	}


	/* Assume no spells available */
	k = 0;

	/* Count spells that can be learned */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Access the spell */
		s_ptr = &p_ptr->mp_ptr->info[j];

		/* Skip spells we cannot remember */
		if (s_ptr->slevel > p_ptr->lev) continue;

		/* Skip spells we already know */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			continue;
		}

		/* Count it */
		k++;
	}

	/* Cannot learn more spells than exist */
	if (p_ptr->new_spells > k) p_ptr->new_spells = k;

	/* Learn new spells */
	if (p_ptr->new_spells && !p_ptr->old_spells)
	{
		/* Message */
		msg_format(p_ptr, "You can learn some new %ss now.", p);

		/* Display "study state" later */
		p_ptr->redraw |= (PR_STUDY);
	}

	/* No more spells */
	else if (!p_ptr->new_spells && p_ptr->old_spells)
	{
		/* Display "study state" later */
		p_ptr->redraw |= (PR_STUDY);
	}

	/* Save the new_spells value */
	p_ptr->old_spells = p_ptr->new_spells;
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(player_type *p_ptr)
{
	int		new_mana, levels, cur_wgt, max_wgt;

	object_type	*o_ptr;
	u32b f1, f2, f3;


	/* Hack -- Must be literate */
	if (!p_ptr->cp_ptr->spell_book) return;


	/* Extract "effective" player level */
	levels = (p_ptr->lev - p_ptr->cp_ptr->spell_first) + 1;

	/* Hack -- no negative mana */
	if (levels < 0) levels = 0;

	/* Extract total mana */
	new_mana = adj_mag_mana[p_ptr->stat_ind[p_ptr->cp_ptr->spell_stat]] * levels / 100;

	/* Hack -- usually add one mana */
	if (new_mana) new_mana++;

    /* Get the gloves */
    o_ptr = &p_ptr->inventory[INVEN_HANDS];

    /* Examine the gloves */
    object_flags(o_ptr, &f1, &f2, &f3);

	/* Only mages are affected */
	if (p_ptr->cp_ptr->spell_book == TV_MAGIC_BOOK)
	{
		/* Assume player is not encumbered by gloves */
		p_ptr->cumber_glove = FALSE;

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->k_idx &&
		    !(f3 & TR3_FREE_ACT) &&
		    !((f1 & TR1_DEX) && (o_ptr->pval > 0)))
		{
			/* Encumbered */
			p_ptr->cumber_glove = TRUE;

			/* Reduce mana */
			new_mana = (3 * new_mana) / 4;
		}
	}

	/* Assume player not encumbered by armor */
	p_ptr->cumber_armor = FALSE;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += p_ptr->inventory[INVEN_BODY].weight;
	cur_wgt += p_ptr->inventory[INVEN_HEAD].weight;
	cur_wgt += p_ptr->inventory[INVEN_ARM].weight;
	cur_wgt += p_ptr->inventory[INVEN_OUTER].weight;
	cur_wgt += p_ptr->inventory[INVEN_HANDS].weight;
	cur_wgt += p_ptr->inventory[INVEN_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = p_ptr->cp_ptr->spell_weight;

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		p_ptr->cumber_armor = TRUE;

		/* Reduce mana */
		new_mana -= ((cur_wgt - max_wgt) / 10);
	}


	/* Mana can never be negative */
	if (new_mana < 0) new_mana = 0;


	/* Maximum mana has changed */
	if (p_ptr->msp != new_mana)
	{
		/* Player has no mana now */
		if (!new_mana)
		{
			/* No mana left */
			p_ptr->csp = 0;
			p_ptr->csp_frac = 0;
		}

		/* Player had no mana, has some now */
		else if (!p_ptr->msp)
		{
			/* Reset mana */
			/* disabled because horribly exploitable!!! */
			// p_ptr->csp = new_mana;
			p_ptr->csp_frac = 0;
		}

		/* Player had some mana, adjust current mana */
		else
		{
			s32b value;

			/* change current mana proportionately to change of max mana, */
			/* divide first to avoid overflow, little loss of accuracy */
			value = ((((long)p_ptr->csp << 16) + p_ptr->csp_frac) /
			         p_ptr->msp * new_mana);

			/* Extract mana components */
			p_ptr->csp = (value >> 16);
			p_ptr->csp_frac = (value & 0xFFFF);
		}

		/* Save new mana */
		p_ptr->msp = new_mana;

		/* Display mana later */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}


	/* Take note when "glove state" changes */
	if (p_ptr->old_cumber_glove != p_ptr->cumber_glove)
	{
		/* Message */
		if (p_ptr->cumber_glove)
		{
			msg_print(p_ptr, "Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msg_print(p_ptr, "Your hands feel more suitable for spellcasting.");
		}

		/* Save it */
		p_ptr->old_cumber_glove = p_ptr->cumber_glove;
	}


	/* Take note when "armor state" changes */
	if (p_ptr->old_cumber_armor != p_ptr->cumber_armor)
	{
		/* Message */
		if (p_ptr->cumber_armor)
		{
			msg_print(p_ptr, "The weight of your armor encumbers your movement.");
		}
		else
		{
			msg_print(p_ptr, "You feel able to move more freely.");
		}

		/* Save it */
		p_ptr->old_cumber_armor = p_ptr->cumber_armor;
	}
}



/*
 * Calculate the players (maximal) hit points
 * Adjust current hitpoints if necessary
 */
 
/* An option of giving mages an extra hit point per level has been added,
 * to hopefully facilitate them making it down to 1600ish and finding  
 * Constitution potions.  This should probably be changed to stop after level
 * 30.
 */

static void calc_hitpoints(player_type *p_ptr)
{
	int bonus, mhp;

	/* Get "1/100th hitpoint bonus per level" value */
	bonus = adj_con_mhp[p_ptr->stat_ind[A_CON]];

	/* Calculate hitpoints */
	if (p_ptr->fruit_bat) mhp = p_ptr->lev + 2;

	else if (p_ptr->ghost) mhp = p_ptr->lev + 2;
	
	else mhp = p_ptr->player_hp[p_ptr->lev-1] + (bonus * p_ptr->lev / 100);

	/* Always have at least one hitpoint per level */
	if (mhp < p_ptr->lev + 1) mhp = p_ptr->lev + 1;

	/* Option : give (most!) mages a bonus hitpoint / lvl */
	if ((p_ptr->cp_ptr->flags & CF_HP_BONUS) 
		&& !option_p(p_ptr,UNSETH_BONUS))
	mhp += p_ptr->lev;

	/* Factor in the hero / superhero settings */
	if (p_ptr->hero) mhp += 10;
	if (p_ptr->shero) mhp += 30;

	/* New maximum hitpoints */
	if (mhp != p_ptr->mhp)
	{
		s32b value;

		/* change current hit points proportionately to change of mhp */
		/* divide first to avoid overflow, little loss of accuracy */
		value = (((long)p_ptr->chp << 16) + p_ptr->chp_frac) / p_ptr->mhp;
		value = value * mhp;
		p_ptr->chp = (value >> 16);
		p_ptr->chp_frac = (value & 0xFFFF);

		/* Save the new max-hitpoints */
		p_ptr->mhp = mhp;

		/* Check bounds (sometimes chp = mhp + 1) */
		if (p_ptr->chp > p_ptr->mhp) p_ptr->chp = p_ptr->mhp;

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}



/*
 * Extract and set the current "lite radius"
 */
static void calc_torch(player_type *p_ptr)
{
	object_type *o_ptr = &p_ptr->inventory[INVEN_LITE];

	/* Base light radius */
	p_ptr->cur_lite = p_ptr->lite;

	/* Examine actual lites */
	if (o_ptr->tval == TV_LITE)
	{
		/* Torches (with fuel) provide some lite */
		if ((o_ptr->sval == SV_LITE_TORCH) && (o_ptr->pval > 0))
		{
			p_ptr->cur_lite += 1;
		}

		/* Lanterns (with fuel) provide more lite */
		if ((o_ptr->sval == SV_LITE_LANTERN) && (o_ptr->pval > 0))
		{
			p_ptr->cur_lite += 2;
		}

		/* Dwarven lanterns provide permanent radius 2 lite */
		if (o_ptr->sval == SV_LITE_DWARVEN)
		{
			p_ptr->cur_lite += 2;
		}

		/* Feanorian lanterns provide permanent, bright, lite */
		if (o_ptr->sval == SV_LITE_FEANOR)
		{
			p_ptr->cur_lite += 3;
		}

		/* Artifact Lites provide permanent, bright, lite */
		if (artifact_p(o_ptr)) p_ptr->cur_lite += 3;
	}

	/* Reduce lite when running if requested */
	if (p_ptr->running && option_p(p_ptr,VIEW_REDUCE_LITE))
	{
		/* Reduce the lite radius if needed */
		if (p_ptr->cur_lite > 1) p_ptr->cur_lite = 1;
	}

	/* Notice changes in the "lite radius" */
	if (p_ptr->old_lite != p_ptr->cur_lite)
	{
		/* Hack - Dungeon Master keeps his light */
		if (dm_flag_p(p_ptr,KEEP_LITE))
		{
			p_ptr->cur_lite = p_ptr->old_lite;
			return;
		}

		/* Update the lite */
		p_ptr->update |= (PU_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Remember the old lite */
		p_ptr->old_lite = p_ptr->cur_lite;
	}
}



/*
 * Computes current weight limit.
 */
static int weight_limit(player_type *p_ptr)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100;

	/* Return the result */
	return (i);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * This function induces various "status" messages.
 */
static void calc_bonuses(player_type *p_ptr)
{
	int			i, j, hold;

	int			old_speed;

	u32b		old_telepathy;
	int			old_see_inv;

	int			old_dis_ac;
	int			old_dis_to_a;

	int			old_dis_to_h;
	int			old_dis_to_d;

	int			extra_blows;
	int			extra_shots;

	object_type		*o_ptr;
	object_kind		*k_ptr;
	ego_item_type 		*e_ptr;

	u32b		f1, f2, f3;

	/*** Memorize ***/

	/* Save the old speed */
	old_speed = p_ptr->pspeed;

	/* Save the old vision stuff */
	old_telepathy = p_ptr->telepathy;
	old_see_inv = p_ptr->see_inv;

	/* Save the old armor class */
	old_dis_ac = p_ptr->dis_ac;
	old_dis_to_a = p_ptr->dis_to_a;

	/* Save the old hit/damage bonuses */
	old_dis_to_h = p_ptr->dis_to_h;
	old_dis_to_d = p_ptr->dis_to_d;

	/*** Reset ***/

	/* Start with "normal" speed */
	 p_ptr->pspeed = 110;

	/* MAngband-specific: Bats get +10 speed ... they need it! */
	if (p_ptr->fruit_bat) p_ptr->pspeed += 10;

	/* Start with a single blow per turn */
	p_ptr->num_blow = 1;

	/* Start with a single shot per turn */
	p_ptr->num_fire = 1;

	/* Reset the "xtra" tval */
	p_ptr->tval_xtra = 0;

	/* Reset the "ammo" tval */
	p_ptr->tval_ammo = 0;

	/* Clear extra blows/shots */
	extra_blows = extra_shots = 0;

	/* Clear the stat modifiers */
	for (i = 0; i < A_MAX; i++) p_ptr->stat_add[i] = 0;

	/* Clear the Displayed/Real armor class */
	p_ptr->dis_ac = p_ptr->ac = 0;

	/* Clear the Displayed/Real Bonuses */
	p_ptr->dis_to_h = p_ptr->to_h = 0;
	p_ptr->dis_to_d = p_ptr->to_d = 0;
	p_ptr->dis_to_a = p_ptr->to_a = 0;

	/* Clear all the flags */
	p_ptr->aggravate = FALSE;
	p_ptr->teleport = FALSE;
	p_ptr->exp_drain = FALSE;
	p_ptr->bless_blade = FALSE;
	p_ptr->xtra_might = FALSE;
	p_ptr->impact = FALSE;
	p_ptr->see_inv = FALSE;
	p_ptr->free_act = FALSE;
	p_ptr->slow_digest = FALSE;
	p_ptr->regenerate = FALSE;
	p_ptr->feather_fall = FALSE;
	p_ptr->hold_life = FALSE;
	p_ptr->telepathy = 0;
	p_ptr->lite = FALSE;
	p_ptr->sustain_str = FALSE;
	p_ptr->sustain_int = FALSE;
	p_ptr->sustain_wis = FALSE;
	p_ptr->sustain_con = FALSE;
	p_ptr->sustain_dex = FALSE;
	p_ptr->sustain_chr = FALSE;
	p_ptr->resist_acid = FALSE;
	p_ptr->resist_elec = FALSE;
	p_ptr->resist_fire = FALSE;
	p_ptr->resist_cold = FALSE;
	p_ptr->resist_pois = FALSE;
	p_ptr->resist_conf = FALSE;
	p_ptr->resist_sound = FALSE;
	p_ptr->resist_lite = FALSE;
	p_ptr->resist_dark = FALSE;
	p_ptr->resist_chaos = FALSE;
	p_ptr->resist_disen = FALSE;
	p_ptr->resist_shard = FALSE;
	p_ptr->resist_nexus = FALSE;
	p_ptr->resist_blind = FALSE;
	p_ptr->resist_neth = FALSE;
	p_ptr->resist_fear = FALSE;
	p_ptr->immune_acid = FALSE;
	p_ptr->immune_elec = FALSE;
	p_ptr->immune_fire = FALSE;
	p_ptr->immune_cold = FALSE;


	/*** Extract race/class info ***/

	/* Base infravision (purely racial) */
	p_ptr->see_infra = p_ptr->rp_ptr->infra;

	/* Base skill -- disarming */
	p_ptr->skill_dis = p_ptr->rp_ptr->r_dis + p_ptr->cp_ptr->c_dis;

	/* Base skill -- magic devices */
	p_ptr->skill_dev = p_ptr->rp_ptr->r_dev + p_ptr->cp_ptr->c_dev;

	/* Base skill -- saving throw */
	p_ptr->skill_sav = p_ptr->rp_ptr->r_sav + p_ptr->cp_ptr->c_sav;

	/* Base skill -- stealth */
	p_ptr->skill_stl = p_ptr->rp_ptr->r_stl + p_ptr->cp_ptr->c_stl;

	/* Base skill -- searching ability */
	p_ptr->skill_srh = p_ptr->rp_ptr->r_srh + p_ptr->cp_ptr->c_srh;

	/* Base skill -- searching frequency */
	p_ptr->skill_fos = p_ptr->rp_ptr->r_fos + p_ptr->cp_ptr->c_fos;

	/* Base skill -- combat (normal) */
	p_ptr->skill_thn = p_ptr->rp_ptr->r_thn + p_ptr->cp_ptr->c_thn;

	/* Base skill -- combat (shooting) */
	p_ptr->skill_thb = p_ptr->rp_ptr->r_thb + p_ptr->cp_ptr->c_thb;

	/* Base skill -- combat (throwing) */
	p_ptr->skill_tht = p_ptr->rp_ptr->r_thb + p_ptr->cp_ptr->c_thb;

	/* Base skill -- digging */
	p_ptr->skill_dig = 0;


	/*** Analyze player ***/

	/* Extract the player flags */
	player_flags(p_ptr, &f1, &f2, &f3);

	/* Good flags */
	if (f3 & (TR3_SLOW_DIGEST)) p_ptr->slow_digest = TRUE;
	if (f3 & (TR3_FEATHER)) p_ptr->feather_fall = TRUE;
	if (f3 & (TR3_LITE)) p_ptr->lite = TRUE;
	if (f3 & (TR3_REGEN)) p_ptr->regenerate = TRUE;
	if (f3 & (TR3_TELEPATHY)) p_ptr->telepathy = TRUE;
	if (f3 & (TR3_SEE_INVIS)) p_ptr->see_inv = TRUE;
	if (f3 & (TR3_FREE_ACT)) p_ptr->free_act = TRUE;
	if (f3 & (TR3_HOLD_LIFE)) p_ptr->hold_life = TRUE;

	/* Weird flags */
	if (f3 & (TR3_BLESSED)) p_ptr->bless_blade = TRUE;

	/* Bad flags */
	if (f3 & (TR3_IMPACT)) p_ptr->impact = TRUE;
	if (f3 & (TR3_AGGRAVATE)) p_ptr->aggravate = TRUE;
	if (f3 & (TR3_TELEPORT)) p_ptr->teleport = TRUE;
	if (f3 & (TR3_DRAIN_EXP)) p_ptr->exp_drain = TRUE;

	/* Immunity flags */
	if (f2 & (TR2_IM_FIRE)) p_ptr->immune_fire = TRUE;
	if (f2 & (TR2_IM_ACID)) p_ptr->immune_acid = TRUE;
	if (f2 & (TR2_IM_COLD)) p_ptr->immune_cold = TRUE;
	if (f2 & (TR2_IM_ELEC)) p_ptr->immune_elec = TRUE;

	/* Resistance flags */
	if (f2 & (TR2_RES_ACID)) p_ptr->resist_acid = TRUE;
	if (f2 & (TR2_RES_ELEC)) p_ptr->resist_elec = TRUE;
	if (f2 & (TR2_RES_FIRE)) p_ptr->resist_fire = TRUE;
	if (f2 & (TR2_RES_COLD)) p_ptr->resist_cold = TRUE;
	if (f2 & (TR2_RES_POIS)) p_ptr->resist_pois = TRUE;
	if (f2 & (TR2_RES_FEAR)) p_ptr->resist_fear = TRUE;
	if (f2 & (TR2_RES_LITE)) p_ptr->resist_lite = TRUE;
	if (f2 & (TR2_RES_DARK)) p_ptr->resist_dark = TRUE;
	if (f2 & (TR2_RES_BLIND)) p_ptr->resist_blind = TRUE;
	if (f2 & (TR2_RES_CONFU)) p_ptr->resist_conf = TRUE;
	if (f2 & (TR2_RES_SOUND)) p_ptr->resist_sound = TRUE;
	if (f2 & (TR2_RES_SHARD)) p_ptr->resist_shard = TRUE;
	if (f2 & (TR2_RES_NEXUS)) p_ptr->resist_nexus = TRUE;
	if (f2 & (TR2_RES_NETHR)) p_ptr->resist_neth = TRUE;
	if (f2 & (TR2_RES_CHAOS)) p_ptr->resist_chaos = TRUE;
	if (f2 & (TR2_RES_DISEN)) p_ptr->resist_disen = TRUE;

	/* Sustain flags */
	if (f2 & (TR2_SUST_STR)) p_ptr->sustain_str = TRUE;
	if (f2 & (TR2_SUST_INT)) p_ptr->sustain_int = TRUE;
	if (f2 & (TR2_SUST_WIS)) p_ptr->sustain_wis = TRUE;
	if (f2 & (TR2_SUST_DEX)) p_ptr->sustain_dex = TRUE;
	if (f2 & (TR2_SUST_CON)) p_ptr->sustain_con = TRUE;
	if (f2 & (TR2_SUST_CHR)) p_ptr->sustain_chr = TRUE;

	/* Hack -- apply racial/class stat maxes */
	if (p_ptr->maximize)
	{
		/* Apply the racial modifiers */
		for (i = 0; i < A_MAX; i++)
		{
			/* Modify the stats for "race" */
			p_ptr->stat_add[i] += (p_ptr->rp_ptr->r_adj[i] + p_ptr->cp_ptr->c_adj[i]);
		}
	}


	/* Option : give (most!) rogues a bonus speed point on level 5,20,35,50 -APD- */
	if ((p_ptr->cp_ptr->flags & CF_SPEED_BONUS) && !option_p(p_ptr,UNSETH_BONUS)) 
	{
		if (p_ptr->lev >= 5)
		{
			p_ptr->pspeed += (((p_ptr->lev-5)/15)+1);
		}
	}

	/* Hack -- the dungeon master gets +50 speed. */
	if (is_dm_p(p_ptr)) 
	{
		p_ptr->pspeed += 50;
		p_ptr->telepathy = TR3_TELEPATHY;
	}


	/*** Analyze equipment ***/

	/* Scan the usable inventory */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		k_ptr = &k_info[o_ptr->k_idx];
		e_ptr = &e_info[o_ptr->name2];

		/* Skip missing items */
		if (!o_ptr->k_idx) continue;

		/* Extract the item flags */
        object_flags(o_ptr, &f1, &f2, &f3);

		/* Hack -- first add any "base bonuses" of the item.  A new
		 * feature in MAngband 0.7.0 is that the magnitude of the
		 * base bonuses is stored in bpval instead of pval, making the
		 * magnitude of "base bonuses" and "ego bonuses" independent 
		 * from each other.
		 * An example of an item that uses this independency is an
		 * Orcish Shield of the Avari that gives +1 to STR and +3 to
		 * CON. (base bonus from the shield +1 STR,CON, ego bonus from
		 * the Avari +2 CON).  
		 * Of course, the proper fix would be to redesign the object
		 * type so that each of the ego bonuses has its own independent
		 * parameter.
		 */
		/* NOTE: Randarts totally ignore "bpval"! */
		/* If we have any base bonuses to add, add them */
		if ((k_ptr->flags1 & TR1_PVAL_MASK) && !randart_p(o_ptr))
		{
			/* Affect stats */
			if (k_ptr->flags1 & TR1_STR) p_ptr->stat_add[A_STR] += o_ptr->bpval;
			if (k_ptr->flags1 & TR1_INT) p_ptr->stat_add[A_INT] += o_ptr->bpval;
			if (k_ptr->flags1 & TR1_WIS) p_ptr->stat_add[A_WIS] += o_ptr->bpval;
			if (k_ptr->flags1 & TR1_DEX) p_ptr->stat_add[A_DEX] += o_ptr->bpval;
			if (k_ptr->flags1 & TR1_CON) p_ptr->stat_add[A_CON] += o_ptr->bpval;
			if (k_ptr->flags1 & TR1_CHR) p_ptr->stat_add[A_CHR] += o_ptr->bpval;

			/* Affect stealth */
			if (k_ptr->flags1 & TR1_STEALTH) p_ptr->skill_stl += o_ptr->bpval;

			/* Affect searching ability (factor of five) */
			if (k_ptr->flags1 & TR1_SEARCH) p_ptr->skill_srh += (o_ptr->bpval * 5);

			/* Affect searching frequency (factor of five) */
			if (k_ptr->flags1 & TR1_SEARCH) p_ptr->skill_fos += (o_ptr->bpval * 5);

			/* Affect infravision */
			if (k_ptr->flags1 & TR1_INFRA) p_ptr->see_infra += o_ptr->bpval;

			/* Affect digging (factor of 20) */
			if (k_ptr->flags1 & TR1_TUNNEL) p_ptr->skill_dig += (o_ptr->bpval * 20);

			/* Affect speed */
			if (k_ptr->flags1 & TR1_SPEED) p_ptr->pspeed += o_ptr->bpval;

			/* Affect blows */
			if (k_ptr->flags1 & TR1_BLOWS) extra_blows += o_ptr->bpval;
		}

		/* Next, add our ego bonuses */
		/* Hack -- clear out any pval bonuses that are in the base item
		 * bonus but not the ego bonus so we don't add them twice.
		*/
		if (o_ptr->name2)
		{
			f1 &= ~(k_ptr->flags1 & TR1_PVAL_MASK & ~e_ptr->flags1);
		}


		/* Affect stats */
		if (f1 & TR1_STR) p_ptr->stat_add[A_STR] += o_ptr->pval;
		if (f1 & TR1_INT) p_ptr->stat_add[A_INT] += o_ptr->pval;
		if (f1 & TR1_WIS) p_ptr->stat_add[A_WIS] += o_ptr->pval;
		if (f1 & TR1_DEX) p_ptr->stat_add[A_DEX] += o_ptr->pval;
		if (f1 & TR1_CON) p_ptr->stat_add[A_CON] += o_ptr->pval;
		if (f1 & TR1_CHR) p_ptr->stat_add[A_CHR] += o_ptr->pval;

		/* Affect stealth */
		if (f1 & TR1_STEALTH) p_ptr->skill_stl += o_ptr->pval;

		/* Affect searching ability (factor of five) */
		if (f1 & TR1_SEARCH) p_ptr->skill_srh += (o_ptr->pval * 5);

		/* Affect searching frequency (factor of five) */
		if (f1 & TR1_SEARCH) p_ptr->skill_fos += (o_ptr->pval * 5);

		/* Affect infravision */
		if (f1 & TR1_INFRA) p_ptr->see_infra += o_ptr->pval;

		/* Affect digging (factor of 20) */
		if (f1 & TR1_TUNNEL) p_ptr->skill_dig += (o_ptr->pval * 20);

		/* Affect speed */
		if (f1 & TR1_SPEED) p_ptr->pspeed += o_ptr->pval;

		/* Affect blows */
		if (f1 & TR1_BLOWS) extra_blows += o_ptr->pval;

		/* Hack -- cause earthquakes */
		if (f3 & TR3_IMPACT) p_ptr->impact = TRUE;

		/* Boost shots */
		if (f1 & TR1_SHOTS) extra_shots++;

		/* Various flags */
		if (f3 & TR3_AGGRAVATE) p_ptr->aggravate = TRUE;
		if (f3 & TR3_TELEPORT) p_ptr->teleport = TRUE;
		if (f3 & TR3_DRAIN_EXP) p_ptr->exp_drain = TRUE;
		if (f3 & TR3_BLESSED) p_ptr->bless_blade = TRUE;
		if (f1 & TR1_MIGHT) p_ptr->xtra_might = TRUE;
		if (f3 & TR3_SLOW_DIGEST) p_ptr->slow_digest = TRUE;
		if (f3 & TR3_REGEN) p_ptr->regenerate = TRUE;
		if (f3 & TR3_LITE) p_ptr->lite += 1;
		if (f3 & TR3_SEE_INVIS) p_ptr->see_inv = TRUE;
		if (f3 & TR3_FEATHER) p_ptr->feather_fall = TRUE;
		if (f2 & TR2_RES_FEAR) p_ptr->resist_fear = TRUE;
		if (f3 & TR3_FREE_ACT) p_ptr->free_act = TRUE;
		if (f3 & TR3_HOLD_LIFE) p_ptr->hold_life = TRUE;

		/* telepathy */
		if (f3 & TR3_TELEPATHY)
			p_ptr->telepathy = TR3_TELEPATHY;
		else if (p_ptr->telepathy != TR3_TELEPATHY)
			p_ptr->telepathy |= f3;

		/* Immunity flags */
		if (f2 & TR2_IM_FIRE) p_ptr->immune_fire = TRUE;
		if (f2 & TR2_IM_ACID) p_ptr->immune_acid = TRUE;
		if (f2 & TR2_IM_COLD) p_ptr->immune_cold = TRUE;
		if (f2 & TR2_IM_ELEC) p_ptr->immune_elec = TRUE;

		/* Resistance flags */
		if (f2 & TR2_RES_ACID) p_ptr->resist_acid = TRUE;
		if (f2 & TR2_RES_ELEC) p_ptr->resist_elec = TRUE;
		if (f2 & TR2_RES_FIRE) p_ptr->resist_fire = TRUE;
		if (f2 & TR2_RES_COLD) p_ptr->resist_cold = TRUE;
		if (f2 & TR2_RES_POIS) p_ptr->resist_pois = TRUE;
		if (f2 & TR2_RES_CONFU) p_ptr->resist_conf = TRUE;
		if (f2 & TR2_RES_SOUND) p_ptr->resist_sound = TRUE;
		if (f2 & TR2_RES_LITE) p_ptr->resist_lite = TRUE;
		if (f2 & TR2_RES_DARK) p_ptr->resist_dark = TRUE;
		if (f2 & TR2_RES_CHAOS) p_ptr->resist_chaos = TRUE;
		if (f2 & TR2_RES_DISEN) p_ptr->resist_disen = TRUE;
		if (f2 & TR2_RES_SHARD) p_ptr->resist_shard = TRUE;
		if (f2 & TR2_RES_NEXUS) p_ptr->resist_nexus = TRUE;
		if (f2 & TR2_RES_BLIND) p_ptr->resist_blind = TRUE;
		if (f2 & TR2_RES_NETHR) p_ptr->resist_neth = TRUE;

		/* Sustain flags */
		if (f2 & TR2_SUST_STR) p_ptr->sustain_str = TRUE;
		if (f2 & TR2_SUST_INT) p_ptr->sustain_int = TRUE;
		if (f2 & TR2_SUST_WIS) p_ptr->sustain_wis = TRUE;
		if (f2 & TR2_SUST_DEX) p_ptr->sustain_dex = TRUE;
		if (f2 & TR2_SUST_CON) p_ptr->sustain_con = TRUE;
		if (f2 & TR2_SUST_CHR) p_ptr->sustain_chr = TRUE;

		/* Modify the base armor class */
		p_ptr->ac += o_ptr->ac;

		/* The base armor class is always known */
		p_ptr->dis_ac += o_ptr->ac;

		/* Apply the bonuses to armor class */
		p_ptr->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		if (object_known_p(p_ptr, o_ptr)) p_ptr->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (i == INVEN_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (i == INVEN_BOW) continue;

		/* Apply the bonuses to hit/damage */
		p_ptr->to_h += o_ptr->to_h;
		p_ptr->to_d += o_ptr->to_d;

		/* Apply the mental bonuses tp hit/damage, if known */
		if (object_known_p(p_ptr, o_ptr)) p_ptr->dis_to_h += o_ptr->to_h;
		if (object_known_p(p_ptr, o_ptr)) p_ptr->dis_to_d += o_ptr->to_d;
	}


	/*** Handle stats ***/

	/* Calculate stats */
	for (i = 0; i < A_MAX; i++)
	{
		int top, use, ind;


		/* Extract the new "stat_use" value for the stat */
		top = modify_stat_value(p_ptr->stat_max[i], p_ptr->stat_add[i]);

		/* Notice changes */
		if (p_ptr->stat_top[i] != top)
		{
			/* Save the new value */
			p_ptr->stat_top[i] = top;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}


		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(p_ptr->stat_cur[i], p_ptr->stat_add[i]);

		/* Notice changes */
		if (p_ptr->stat_use[i] != use)
		{
			/* Save the new value */
			p_ptr->stat_use[i] = use;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}


		/* Values: 3, 4, ..., 17 */
		if (use <= 18) ind = (use - 3);

		/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
		else if (use <= 18+219) ind = (15 + (use - 18) / 10);

		/* Range: 18/220+ */
		else ind = (37);

		/* Notice changes */
		if (p_ptr->stat_ind[i] != ind)
		{
			/* Save the new index */
			p_ptr->stat_ind[i] = ind;

			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				p_ptr->update |= (PU_HP);
			}

			/* Change in INT may affect Mana/Spells */
			else if (i == A_INT)
			{
				if (p_ptr->cp_ptr->spell_stat == A_INT)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Change in WIS may affect Mana/Spells */
			else if (i == A_WIS)
			{
				if (p_ptr->cp_ptr->spell_stat == A_WIS)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
	}


	/*** Temporary flags ***/

	/* Apply temporary "stun" */
	if (p_ptr->stun > 50)
	{
		p_ptr->to_h -= 20;
		p_ptr->dis_to_h -= 20;
		p_ptr->to_d -= 20;
		p_ptr->dis_to_d -= 20;
	}
	else if (p_ptr->stun)
	{
		p_ptr->to_h -= 5;
		p_ptr->dis_to_h -= 5;
		p_ptr->to_d -= 5;
		p_ptr->dis_to_d -= 5;
	}


	/* Invulnerability */
	if (p_ptr->invuln)
	{
		p_ptr->to_a += 100;
		p_ptr->dis_to_a += 100;
	}

	/* Temporary blessing */
	if (p_ptr->blessed)
	{
		p_ptr->to_a += 5;
		p_ptr->dis_to_a += 5;
		p_ptr->to_h += 10;
		p_ptr->dis_to_h += 10;
	}

	/* Temprory shield */
	if (p_ptr->shield)
	{
		p_ptr->to_a += 50;
		p_ptr->dis_to_a += 50;
	}

	/* Temporary "Hero" */
	if (p_ptr->hero)
	{
		p_ptr->to_h += 12;
		p_ptr->dis_to_h += 12;
	}

	/* Temporary "Beserk" */
	if (p_ptr->shero)
	{
		p_ptr->to_h += 24;
		p_ptr->dis_to_h += 24;
		p_ptr->to_a -= 10;
		p_ptr->dis_to_a -= 10;
	}

	/* Temporary "fast" */
	if (p_ptr->fast)
	{
		p_ptr->pspeed += 10;
	}

	/* Temporary "slow" */
	if (p_ptr->slow)
	{
		p_ptr->pspeed -= 10;
	}

	/* Temporary see invisible */
	if (p_ptr->tim_invis)
	{
		p_ptr->see_inv = TRUE;
	}

	/* Temporary infravision boost */
	if (p_ptr->tim_infra)
	{
		p_ptr->see_infra++;
	}

	/* Hack -- Res Chaos -> Res Conf */
	/* Not in recent Angband!
	if (p_ptr->resist_chaos)
	{
		p_ptr->resist_conf = TRUE;
	}
	*/
	
	/* Hack -- Hero/Shero -> Res fear */
	if (p_ptr->hero || p_ptr->shero)
	{
		p_ptr->resist_fear = TRUE;
	}

	
	/*** Analyze weight ***/

	/* Extract the current weight (in tenth pounds) */
	j = p_ptr->total_weight;
	
	/* Cap the weight */
	if (j > 1<<14) j = 1<<14;

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit(p_ptr);

	/* XXX XXX XXX Apply "encumbrance" from weight */
	if (j > i/2) p_ptr->pspeed -= ((j - (i/2)) / (i / 10));

	/* Bloating slows the player down (a little) */
	if (p_ptr->food >= PY_FOOD_MAX) p_ptr->pspeed -= 10;

	/* Searching slows the player down */
	/* -APD- adding "stealth mode" for rogues... will probably need to tweek this */
	if (p_ptr->searching) 
	{
		if (!(p_ptr->cp_ptr->flags & CF_STEALTH_MODE)) p_ptr->pspeed -= 10;
		else 
		{
			p_ptr->pspeed -= 10;
			p_ptr->skill_stl *= 3;
		}
	}
	
	/*** Apply modifier bonuses ***/
	
	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
	p_ptr->to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->dis_to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->dis_to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
	p_ptr->dis_to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->dis_to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);


	/*** Modify skills ***/

	/* Affect Skill -- stealth (bonus one) */
	p_ptr->skill_stl += 1;

	/* Affect Skill -- disarming (DEX and INT) */
	p_ptr->skill_dis += adj_dex_dis[p_ptr->stat_ind[A_DEX]];
	p_ptr->skill_dis += adj_int_dis[p_ptr->stat_ind[A_INT]];

	/* Affect Skill -- magic devices (INT) */
	p_ptr->skill_dev += adj_int_dev[p_ptr->stat_ind[A_INT]];

	/* Affect Skill -- saving throw (WIS) */
	p_ptr->skill_sav += adj_wis_sav[p_ptr->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	p_ptr->skill_dig += adj_str_dig[p_ptr->stat_ind[A_STR]];

	/* Affect Skill -- disarming (Level, by Class) */
	p_ptr->skill_dis += (p_ptr->cp_ptr->x_dis * p_ptr->lev / 10);

	/* Affect Skill -- magic devices (Level, by Class) */
	p_ptr->skill_dev += (p_ptr->cp_ptr->x_dev * p_ptr->lev / 10);

	/* Affect Skill -- saving throw (Level, by Class) */
	p_ptr->skill_sav += (p_ptr->cp_ptr->x_sav * p_ptr->lev / 10);

	/* Affect Skill -- stealth (Level, by Class) */
	p_ptr->skill_stl += (p_ptr->cp_ptr->x_stl * p_ptr->lev / 10);

	/* Affect Skill -- search ability (Level, by Class) */
	p_ptr->skill_srh += (p_ptr->cp_ptr->x_srh * p_ptr->lev / 10);

	/* Affect Skill -- search frequency (Level, by Class) */
	p_ptr->skill_fos += (p_ptr->cp_ptr->x_fos * p_ptr->lev / 10);

	/* Affect Skill -- combat (normal) (Level, by Class) */
	p_ptr->skill_thn += (p_ptr->cp_ptr->x_thn * p_ptr->lev / 10);

	/* Affect Skill -- combat (shooting) (Level, by Class) */
	p_ptr->skill_thb += (p_ptr->cp_ptr->x_thb * p_ptr->lev / 10);

	/* Affect Skill -- combat (throwing) (Level, by Class) */
	p_ptr->skill_tht += (p_ptr->cp_ptr->x_thb * p_ptr->lev / 10);

	/* Limit Skill -- stealth from 0 to 30 */
	if (p_ptr->skill_stl > 30) p_ptr->skill_stl = 30;
	if (p_ptr->skill_stl < 0) p_ptr->skill_stl = 0;

	/* Limit Skill -- digging from 1 up */
	if (p_ptr->skill_dig < 1) p_ptr->skill_dig = 1;

	/* Limit final infravision to >= 0 */
	if (p_ptr->see_infra < 0) p_ptr->see_infra = 0;

	/* Obtain the "hold" value */
	hold = adj_str_hold[p_ptr->stat_ind[A_STR]];


	/*** Analyze current bow ***/

	/* Examine the "current bow" */
	o_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Assume not heavy */
	p_ptr->heavy_shoot = FALSE;

	/* It is hard to carholdry a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
		p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		p_ptr->heavy_shoot = TRUE;
	}

	/* Compute "extra shots" if needed */
	if (o_ptr->k_idx && !p_ptr->heavy_shoot)
	{
		/* Take note of required "tval" for missiles */
		switch (o_ptr->sval)
		{
			case SV_SLING:
			{
				p_ptr->tval_ammo = TV_SHOT;
				break;
			}

			case SV_SHORT_BOW:
			case SV_LONG_BOW:
			{
				p_ptr->tval_ammo = TV_ARROW;
				break;
			}

			case SV_LIGHT_XBOW:
			case SV_HEAVY_XBOW:
			{
				p_ptr->tval_ammo = TV_BOLT;
				break;
			}
		}

		/* Hack -- Reward High Level Rangers using Bows */
       if ((p_ptr->cp_ptr->flags & CF_EXTRA_SHOT) && (p_ptr->tval_ammo == TV_ARROW))	
		{
			/* Extra shot at level 20 */
			if (p_ptr->lev >= 20) p_ptr->num_fire++;

			/* Extra shot at level 40 */
			if (p_ptr->lev >= 40) p_ptr->num_fire++;
		}

		/* Add in the "bonus shots" */
		p_ptr->num_fire += extra_shots;

		/* Require at least one shot */
		if (p_ptr->num_fire < 1) p_ptr->num_fire = 1;
	}


	/*** Analyze weapon ***/

	/* Examine the "main weapon" */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Assume not heavy */
	p_ptr->heavy_wield = FALSE;

	/* It is hard to hold a heavy weapon */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy weapon */
		p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
		p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy weapon */
		p_ptr->heavy_wield = TRUE;
	}


	/* Normal weapons */
	if (o_ptr->k_idx && !p_ptr->heavy_wield)
	{
		int str_index, dex_index;

		int div = 0;

		/* Enforce a minimum "weight" (tenth pounds) */
		div = ((o_ptr->weight < p_ptr->cp_ptr->min_weight) ? p_ptr->cp_ptr->min_weight : o_ptr->weight);

		/* Access the strength vs weight */
		str_index = (adj_str_blow[p_ptr->stat_ind[A_STR]] * p_ptr->cp_ptr->att_multiply / div);

		/* Maximal value */
		if (str_index > 11) str_index = 11;

		/* Index by dexterity */
		dex_index = (adj_dex_blow[p_ptr->stat_ind[A_DEX]]);

		/* Maximal value */
		if (dex_index > 11) dex_index = 11;

		/* Use the blows table */
		p_ptr->num_blow = blows_table[str_index][dex_index];

		/* Maximal value */
		if (p_ptr->num_blow > p_ptr->cp_ptr->max_attacks) p_ptr->num_blow = p_ptr->cp_ptr->max_attacks;

		/* Add in the "bonus blows" */
		p_ptr->num_blow += extra_blows;

		/* Require at least one blow */
		if (p_ptr->num_blow < 1) p_ptr->num_blow = 1;

		/* Boost digging skill by weapon weight */
		p_ptr->skill_dig += (o_ptr->weight / 10);
	}


	/* Assume okay */
	p_ptr->icky_wield = FALSE;

	/* Priest weapon penalty for non-blessed edged weapons */
    if ((p_ptr->cp_ptr->flags & CF_BLESS_WEAPON) && (!p_ptr->bless_blade) &&
	    ((o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM)))
	{
		/* Reduce the real bonuses */
		p_ptr->to_h -= 2;
		p_ptr->to_d -= 2;

		/* Reduce the mental bonuses */
		p_ptr->dis_to_h -= 2;
		p_ptr->dis_to_d -= 2;

		/* Icky weapon */
		p_ptr->icky_wield = TRUE;
	}

	/* Hack -- ensure speed is in bounds */
	if (p_ptr->pspeed < 0) p_ptr->pspeed = 0;
	if (p_ptr->pspeed > 199) p_ptr->pspeed = 199;


	/*** Notice changes ***/

	/* Note -- unlike angband, stat changes are noticed when updated (see above) */ 

	/* Hack -- Telepathy Change */
	if (p_ptr->telepathy != old_telepathy)
	{
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Hack -- See Invis Change */
	if (p_ptr->see_inv != old_see_inv)
	{
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Display the speed (if needed) */
	if (p_ptr->pspeed != old_speed) p_ptr->redraw |= (PR_SPEED);

	/* Redraw armor (if needed) */
	if ((p_ptr->dis_ac != old_dis_ac) || (p_ptr->dis_to_a != old_dis_to_a))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_ARMOR);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}

	/* Redraw To-Hit and To-Dim (if needed) */
	if ((p_ptr->dis_to_h != old_dis_to_h) || (p_ptr->dis_to_d != old_dis_to_d))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_PLUSSES);
	}

	/* Hack -- handle "xtra" mode */
	/*if (character_xtra) return;*/

	/* Take note when "heavy bow" changes */
	if (p_ptr->old_heavy_shoot != p_ptr->heavy_shoot)
	{
		/* Message */
		if (p_ptr->heavy_shoot)
		{
			msg_print(p_ptr, "You have trouble wielding such a heavy bow.");
		}
		else if (p_ptr->inventory[INVEN_BOW].k_idx)
		{
			msg_print(p_ptr, "You have no trouble wielding your bow.");
		}
		else
		{
			msg_print(p_ptr, "You feel relieved to put down your heavy bow.");
		}

		/* Save it */
		p_ptr->old_heavy_shoot = p_ptr->heavy_shoot;
	}

	/* Take note when "heavy weapon" changes */
	if (p_ptr->old_heavy_wield != p_ptr->heavy_wield)
	{
		/* Message */
		if (p_ptr->heavy_wield)
		{
			msg_print(p_ptr, "You have trouble wielding such a heavy weapon.");
		}
		else if (p_ptr->inventory[INVEN_WIELD].k_idx)
		{
			msg_print(p_ptr, "You have no trouble wielding your weapon.");
		}
		else
		{
			msg_print(p_ptr, "You feel relieved to put down your heavy weapon.");
		}

		/* Save it */
		p_ptr->old_heavy_wield = p_ptr->heavy_wield;
	}

	/* Take note when "illegal weapon" changes */
	if (p_ptr->old_icky_wield != p_ptr->icky_wield)
	{
		/* Message */
		if (p_ptr->icky_wield)
		{
			msg_print(p_ptr, "You do not feel comfortable with your weapon.");
		}
		else if (p_ptr->inventory[INVEN_WIELD].k_idx)
		{
			msg_print(p_ptr, "You feel comfortable with your weapon.");
		}
		else
		{
			msg_print(p_ptr, "You feel more comfortable after removing your weapon.");
		}

		/* Save it */
		p_ptr->old_icky_wield = p_ptr->icky_wield;
	}

	/* XXX - Always resend skills */
	p_ptr->redraw |= (PR_SKILLS);
}



/*
 * Handle "p_ptr->notice"
 */
void notice_stuff(player_type *p_ptr)
{
	/* Notice stuff */
	if (!p_ptr->notice) return;


	/* Combine the pack */
	if (p_ptr->notice & PN_COMBINE)
	{
		p_ptr->notice &= ~(PN_COMBINE);
		combine_pack(p_ptr);
	}

	/* Reorder the pack */
	if (p_ptr->notice & PN_REORDER)
	{
		p_ptr->notice &= ~(PN_REORDER);
		reorder_pack(p_ptr);
	}
}


/*
 * Handle "p_ptr->update"
 */
void update_stuff(player_type *p_ptr)
{
	/* Update stuff */
	if (!p_ptr->update) return;


	if (p_ptr->update & PU_BONUS)
	{
		p_ptr->update &= ~(PU_BONUS);
		calc_bonuses(p_ptr);
	}

	if (p_ptr->update & PU_TORCH)
	{
		p_ptr->update &= ~(PU_TORCH);
		calc_torch(p_ptr);
	}

	if (p_ptr->update & PU_HP)
	{
		p_ptr->update &= ~(PU_HP);
		calc_hitpoints(p_ptr);
	}

	if (p_ptr->update & PU_MANA)
	{
		p_ptr->update &= ~(PU_MANA);
		calc_mana(p_ptr);
	}

	if (p_ptr->update & PU_SPELLS)
	{
		p_ptr->update &= ~(PU_SPELLS);
		calc_spells(p_ptr);
	}


	/* Character is not ready yet, no screen updates */
	/*if (!character_generated) return;*/


	/* Character is in "icky" mode, no screen updates */
	/*if (character_icky) return;*/

	/* Character has changed depth very recently, no screen updates */
	if (p_ptr->new_level_flag) return;

	if (p_ptr->update & PU_UN_LITE)
	{
		p_ptr->update &= ~(PU_UN_LITE);
		forget_lite(p_ptr);
	}

	if (p_ptr->update & PU_UN_VIEW)
	{
		p_ptr->update &= ~(PU_UN_VIEW);
		forget_view(p_ptr);
	}


	if (p_ptr->update & PU_VIEW)
	{
		p_ptr->update &= ~(PU_VIEW);
		update_view(p_ptr);
	}

	if (p_ptr->update & PU_LITE)
	{
		p_ptr->update &= ~(PU_LITE);
		update_lite(p_ptr);
	}


	if (p_ptr->update & PU_FLOW)
	{
		p_ptr->update &= ~(PU_FLOW);
		update_flow();
	}


	if (p_ptr->update & PU_DISTANCE)
	{
		p_ptr->update &= ~(PU_DISTANCE);
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(TRUE);
		update_players();
	}

	if (p_ptr->update & PU_MONSTERS)
	{
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(FALSE);
		update_players();
	}
}


/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(player_type *p_ptr)
{
	/* Redraw stuff */
	if (!p_ptr->redraw) return;


	/* Character is not ready yet, no screen updates */
	/*if (!character_generated) return;*/


	/* Character is in "icky" mode, no screen updates */
	/*if (character_icky) return;*/



	/* Hack -- clear the screen */
	if (p_ptr->redraw & PR_WIPE)
	{
		p_ptr->redraw &= ~PR_WIPE;
		msg_print(p_ptr, NULL);
		/*Term_clear();*/
	}


	if (p_ptr->redraw & PR_MAP)
	{
		p_ptr->redraw &= ~(PR_MAP);
		prt_map(p_ptr);
	}

	if (p_ptr->redraw & PR_MISC)
	{
		p_ptr->redraw &= ~(PR_MISC);
		send_character_info(p_ptr);
		prt_misc(p_ptr);
		/*prt_history(p_ptr);*/
	}

	if (p_ptr->redraw & PR_TITLE)
	{
		p_ptr->redraw &= ~(PR_TITLE);
		prt_title(p_ptr);
	}

	if (p_ptr->redraw & PR_LEV)
	{
		p_ptr->redraw &= ~(PR_LEV);
		prt_level(p_ptr);
	}

	if (p_ptr->redraw & PR_EXP)
	{
		p_ptr->redraw &= ~(PR_EXP);
		prt_exp(p_ptr);
	}

	if (p_ptr->redraw & PR_STATS)
	{
		p_ptr->redraw &= ~(PR_STATS);
		prt_stat(p_ptr, A_STR);
		prt_stat(p_ptr, A_INT);
		prt_stat(p_ptr, A_WIS);
		prt_stat(p_ptr, A_DEX);
		prt_stat(p_ptr, A_CON);
		prt_stat(p_ptr, A_CHR);
	}

	if (p_ptr->redraw & PR_ARMOR)
	{
		p_ptr->redraw &= ~(PR_ARMOR);
		prt_ac(p_ptr);
	}

	if (p_ptr->redraw & PR_HP)
	{
		p_ptr->redraw &= ~(PR_HP);
		prt_hp(p_ptr);
	}

	if (p_ptr->redraw & PR_MANA)
	{
		p_ptr->redraw &= ~(PR_MANA);
		prt_sp(p_ptr);
	}

	if (p_ptr->redraw & PR_GOLD)
	{
		p_ptr->redraw &= ~(PR_GOLD);
		prt_gold(p_ptr);
	}

	if (p_ptr->redraw & PR_DEPTH)
	{
		p_ptr->redraw &= ~(PR_DEPTH);
		prt_depth(p_ptr);
	}

	if (p_ptr->redraw & PR_HEALTH)
	{
		p_ptr->redraw &= ~(PR_HEALTH);
		health_redraw(p_ptr);
	}
	
	if (p_ptr->redraw & PR_CURSOR)
	{
		p_ptr->redraw &= ~(PR_CURSOR);
		cursor_redraw(p_ptr);
	}

	/*if (p_ptr->redraw & PR_HISTORY)
	{
		p_ptr->redraw &= ~(PR_HISTORY);
		prt_history(p_ptr);
	}*/
	
	if (p_ptr->redraw & PR_OFLAGS)
	{
		p_ptr->redraw &= ~(PR_OFLAGS);
		prt_flags(p_ptr);
		/* HACK - since OFLAGS mostly indicates changes in equipment,
		 * we can use it trigger inscription updates. FIXME? */
		update_prevent_inscriptions(p_ptr);
	}

	if (p_ptr->redraw & PR_VARIOUS)
	{
		p_ptr->redraw &= ~(PR_VARIOUS);
		prt_various(p_ptr);
	}

	if (p_ptr->redraw & PR_PLUSSES)
	{
		p_ptr->redraw &= ~(PR_PLUSSES);
		prt_plusses(p_ptr);
	}

	if (p_ptr->redraw & PR_SKILLS)
	{
		p_ptr->redraw &= ~(PR_SKILLS);
		prt_skills(p_ptr);
	}

	if (p_ptr->redraw & PR_CUT)
	{
		p_ptr->redraw &= ~(PR_CUT);
		prt_cut(p_ptr);
	}

	if (p_ptr->redraw & PR_STUN)
	{
		p_ptr->redraw &= ~(PR_STUN);
		prt_stun(p_ptr);
	}

	if (p_ptr->redraw & PR_HUNGER)
	{
		p_ptr->redraw &= ~(PR_HUNGER);
		prt_hunger(p_ptr);
	}

	if (p_ptr->redraw & PR_BLIND)
	{
		p_ptr->redraw &= ~(PR_BLIND);
		prt_blind(p_ptr);
	}

	if (p_ptr->redraw & PR_CONFUSED)
	{
		p_ptr->redraw &= ~(PR_CONFUSED);
		prt_confused(p_ptr);
	}

	if (p_ptr->redraw & PR_AFRAID)
	{
		p_ptr->redraw &= ~(PR_AFRAID);
		prt_afraid(p_ptr);
	}

	if (p_ptr->redraw & PR_POISONED)
	{
		p_ptr->redraw &= ~(PR_POISONED);
		prt_poisoned(p_ptr);
	}

	if (p_ptr->redraw & PR_STATE)
	{
		p_ptr->redraw &= ~(PR_STATE);
		prt_state(p_ptr);
	}

	if (p_ptr->redraw & PR_SPEED)
	{
		p_ptr->redraw &= ~(PR_SPEED);
		prt_speed(p_ptr);
	}

	if (p_ptr->redraw & PR_STUDY)
	{
		p_ptr->redraw &= ~(PR_STUDY);
		prt_study(p_ptr);
	}
	
	if (p_ptr->redraw & PR_FLOOR)
	{
		p_ptr->redraw &= ~(PR_FLOOR);
		prt_floor_item(p_ptr);
	}
	
	if (p_ptr->redraw & PR_OPPOSE_ELEMENTS)
	{
		p_ptr->redraw &= ~(PR_OPPOSE_ELEMENTS);
		prt_oppose_elements(p_ptr);
	}
}


/*
 * Handle "p_ptr->window"
 */
void window_stuff(player_type *p_ptr)
{
	/* Window stuff */
	if (!p_ptr->window) return;

	/* Display inventory */
	if (p_ptr->window & PW_INVEN)
	{
		p_ptr->window &= ~(PW_INVEN);
		/* Note: PW_INVEN has been disabled and does absolutely
		 * nothing! To inform client about inventory changes,
		 * set relevant bits in 'redraw_inven'. */
		/* p_ptr->redraw_inven = 0xFFFFFFFFFFFFFFFFLL;
		fix_inven(p_ptr); */
	}

	/* Display equipment */
	if (p_ptr->window & PW_EQUIP)
	{
		p_ptr->window &= ~(PW_EQUIP);
		/* Note: PW_EQUIP has been disabled and does absolutely
		 * nothing! To inform client about inventory changes,
		 * set relevant bits in 'redraw_inven'. */
		/* fix_equip(p_ptr);
		p_ptr->redraw_inven = 0xFFFFFFFFFFFFFFFFLL; */
	}

	/* Display spell list */
	if (p_ptr->window & PW_SPELL)
	{
		p_ptr->window &= ~(PW_SPELL);
		fix_spell(p_ptr);
	}

	/* Display player */
	if (p_ptr->window & PW_PLAYER)
	{
		p_ptr->window &= ~(PW_PLAYER);
		fix_player(p_ptr);
	}

	/* Display message view */
	if (p_ptr->window & PW_MESSAGE)
	{
		p_ptr->window &= ~(PW_MESSAGE);
		fix_message(p_ptr);
	}

	/* Display overhead view */
	if (p_ptr->window & PW_OVERHEAD)
	{
		p_ptr->window &= ~(PW_OVERHEAD);
		fix_overhead(p_ptr);
	}

	/* Display mini-map view */
	if (p_ptr->window & PW_MAP)
	{
		p_ptr->window &= ~(PW_MAP);
		fix_map(p_ptr);
	}

	/* Display monster recall */
	if (p_ptr->window & PW_MONSTER)
	{
		p_ptr->window &= ~(PW_MONSTER);
		fix_monster(p_ptr);
	}

	/* Display monster list */
	if (p_ptr->window & PW_MONLIST)
	{
		p_ptr->window &= ~(PW_MONLIST);
		fix_monlist(p_ptr);
	}

	/* Display item list */
	if (p_ptr->window & PW_ITEMLIST)
	{
		p_ptr->window &= ~(PW_ITEMLIST);
		fix_itemlist(p_ptr);
	}
}


/*
 * Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window"
 */
void handle_stuff(player_type *p_ptr)
{
	if (p_ptr->conn == -1 || !IS_PLAYING(p_ptr)) return;

	/* Hack -- delay updating */
	if (p_ptr->new_level_flag) return;

	/* Update stuff */
	if (p_ptr->update) update_stuff(p_ptr);

	/* Redraw inventory */
	if (p_ptr->redraw_inven)
	{
		fix_inven(p_ptr);
		fix_equip(p_ptr);
		p_ptr->redraw_inven = 0;
	}

	/* Redraw stuff */
	if (p_ptr->redraw) redraw_stuff(p_ptr);

	/* Window stuff */
	if (p_ptr->window) window_stuff(p_ptr);
}
