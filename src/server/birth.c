/* File: birth.c */

/* Purpose: create a player character */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"


/*
 * Forward declare
 */
typedef struct birther birther;

/*
 * A structure to hold "rolled" information
 */
struct birther
{
	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[6];

	char history[4][60];
};


/*
 * Current stats
 */
static s16b		stat_use[6];



/*
 * Returns adjusted stat -JK-
 * Algorithm by -JWT-
 *
 * auto_roll is boolean and states maximum changes should be used rather
 * than random ones to allow specification of higher values to wait for
 *
 * The "p_ptr->maximize" code is important	-BEN-
 */
static int adjust_stat(int Ind, int value, s16b amount, int auto_roll)
{
	player_type *p_ptr = Players[Ind];
	int i;

	/* Negative amounts */
	if (amount < 0)
	{
		/* Apply penalty */
		for (i = 0; i < (0 - amount); i++)
		{
			if (value >= 18+10)
			{
				value -= 10;
			}
			else if (value > 18)
			{
				value = 18;
			}
			else if (value > 3)
			{
				value--;
			}
		}
	}

	/* Positive amounts */
	else if (amount > 0)
	{
		/* Apply reward */
		for (i = 0; i < amount; i++)
		{
			if (value < 18)
			{
				value++;
			}
			else if (p_ptr->maximize)
			{
				value += 10;
			}
			else if (value < 18+70)
			{
				value += ((auto_roll ? 15 : randint(15)) + 5);
			}
			else if (value < 18+90)
			{
				value += ((auto_roll ? 6 : randint(6)) + 2);
			}
			else if (value < 18+100)
			{
				value++;
			}
		}
	}

	/* Return the result */
	return (value);
}




/*
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(int Ind, int stat_order[6])
{
	player_type *p_ptr = Players[Ind];
	int		i, j;
	int		bonus;
	int		dice[18];
	int		stats[6];
    int         n17, n16, n15;

	/* Clear "stats" array */
	for (i = 0; i < 6; i++)
		stats[i] = 0;

	/* Check over the given stat order, to prevent cheating */
	for (i = 0; i < 6; i++)
	{
		/* Check range */
		if (stat_order[i] < 0 || stat_order[i] > 5)
		{
			stat_order[i] = 1;
		}

		/* Check for duplicated entries */
		if (stats[stat_order[i]] == 1)
		{
			/* Find a stat that hasn't been specified yet */
			for (j = 0; j < 6; j++)
			{
				if (stats[j])
					continue;

				stat_order[i] = j;
			}
		}

		/* Set flag */
		stats[stat_order[i]] = 1;
	}

    /* Ensure that the primary stat is 17, secondary stat >= 16 and
	third stat >= 15 --> no more endless suicides */
    do
    {
	n17 = 0;
	n16 = 0;
	n15 = 0;
	/* Roll and verify some stats */
	while (TRUE)
	{
		/* Roll some dice */
		for (j = i = 0; i < 18; i++)
		{
			/* Roll the dice */
			dice[i] = randint(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 42) && (j < 54)) break;
	}

	/* Acquire the stats */
	for (i = 0; i < 6; i++)
	{
		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];
          if (j == 17) n17++;
	  if (j >= 16) n16++;
	  if (j >= 15) n15++;

		/* Save that value */
		stats[i] = j;
	}
    }
    while ((n17 < 1) || (n16 < 2) || (n15 < 3));

	/* Now sort the stats */
	/* I use a bubble sort because I'm lazy at the moment */
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 5; j++)
		{
			if (stats[j] < stats[j + 1])
			{
				int t;

				t = stats[j];
				stats[j] = stats[j + 1];
				stats[j + 1] = t;
			}
		}
	}

	/* Now, put them in the correct order */
	for (i = 0; i < 6; i++)
	{
		p_ptr->stat_max[stat_order[i]] = stats[i];
	}

	/* Adjust the stats */
	for (i = 0; i < 6; i++)
	{
		/* Obtain a "bonus" for "race" and "class" */
		bonus = p_ptr->rp_ptr->r_adj[i] + p_ptr->cp_ptr->c_adj[i];

		/* Variable stat maxes */
		if (p_ptr->maximize)
		{
			/* Start fully healed */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i];

			/* Efficiency -- Apply the racial/class bonuses */
			stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
		}

		/* Fixed stat maxes */
		else
		{
			/* Apply the bonus to the stat (somewhat randomly) */
			stat_use[i] = adjust_stat(Ind, p_ptr->stat_max[i], bonus, FALSE);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}
	}
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int		i, j, min_value, max_value;


	/* Level one (never zero!) */
	p_ptr->max_plv = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = p_ptr->rp_ptr->r_exp + p_ptr->cp_ptr->c_exp;

	/* Hitdice */
	p_ptr->hitdie = p_ptr->rp_ptr->r_mhp + p_ptr->cp_ptr->c_mhp;

	/* Assume base hitpoints (fully healed) */
	p_ptr->chp = p_ptr->mhp = p_ptr->hitdie;


	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			j = randint(p_ptr->hitdie);
			p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] < min_value) continue;
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] > max_value) continue;

		/* Acceptable */
		break;
	}
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int		i, n, chart, roll, social_class;

	char	*s, *t;

	char	buf[240];



	/* Clear the previous history strings */
	for (i = 0; i < 4; i++) p_ptr->history[i][0] = '\0';


	/* Clear the history text */
	buf[0] = '\0';

	/* Initial social class */
	social_class = randint(4);

	/* Starting place */
	chart = p_info[p_ptr->prace].hist;

	/* Process the history */
	while (chart)
	{
		/* Start over */
		i = 0;

		/* Roll for nobility */
		roll = randint(100);

		/* Access the proper entry in the table */
		while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

		/* Acquire the textual history */
		(void)strcat(buf, h_text + h_info[i].text);

		/* Add in the social class */
		social_class += (int)(h_info[i].bonus) - 50;

		/* Enter the next chart */
		chart = h_info[i].next;
	}



	/* Verify social class */
	if (social_class > 100) social_class = 100;
	else if (social_class < 1) social_class = 1;

	/* Save the social class */
	p_ptr->sc = social_class;


	/* Skip leading spaces */
	for (s = buf; *s == ' '; s++) /* loop */;

	/* Get apparent length */
	n = strlen(s);

	/* Kill trailing spaces */
	while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';


	/* Start at first line */
	i = 0;

	/* Collect the history */
	while (TRUE)
	{
		/* Extract remaining length */
		n = strlen(s);

		/* All done */
		if (n < 60)
		{
			/* Save one line of history */
			strcpy(p_ptr->history[i++], s);

			/* All done */
			break;
		}

		/* Find a reasonable break-point */
		for (n = 60; ((n > 0) && (s[n-1] != ' ')); n--) /* loop */;

		/* Save next location */
		t = s + n;

		/* Wipe trailing spaces */
		while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';

		/* Save one line of history */
		strcpy(p_ptr->history[i++], s);

		/* Start next line */
		for (s = t; *s == ' '; s++) /* loop */;
	}
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Calculate the age */
	p_ptr->age = p_ptr->rp_ptr->b_age + randint(p_ptr->rp_ptr->m_age);

	/* Calculate the height/weight for males */
	if (p_ptr->male)
	{
		p_ptr->ht = randnor(p_ptr->rp_ptr->m_b_ht, p_ptr->rp_ptr->m_m_ht);
		p_ptr->wt = randnor(p_ptr->rp_ptr->m_b_wt, p_ptr->rp_ptr->m_m_wt);
	}

	/* Calculate the height/weight for females */
	else
	{
		p_ptr->ht = randnor(p_ptr->rp_ptr->f_b_ht, p_ptr->rp_ptr->f_m_ht);
		p_ptr->wt = randnor(p_ptr->rp_ptr->f_b_wt, p_ptr->rp_ptr->f_m_wt);
	}
}




/*
 * Get the player's starting money
 */
static void get_money(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int        i, gold;

	/* Social Class determines starting gold */
	gold = (p_ptr->sc * 6) + randint(100) + 300;

	/* Process the stats */
	for (i = 0; i < 6; i++)
	{
		/* Mega-Hack -- reduce gold for high stats */
		if (stat_use[i] >= 18+50) gold -= 300;
		else if (stat_use[i] >= 18+20) gold -= 200;
		else if (stat_use[i] > 18) gold -= 150;
		else gold -= (stat_use[i] - 8) * 10;
	}

	/* Minimum 100 gold */
	if (gold < 100) gold = 100;

	/* She charmed the banker into it! -CJS- */
	/* She slept with the banker.. :) -GDH-  */
	if (!p_ptr->male) gold += 50;

	/* Save the gold */
	p_ptr->au = gold;
	
	if (!strcmp(p_ptr->name,cfg_dungeon_master))
	{
		p_ptr->au = 50000000;
		p_ptr->lev = 50;
		p_ptr->exp = 15000000;
		p_ptr->invuln = -1;
		p_ptr->ghost = 1;
		p_ptr->noscore = 1;
	}
	
}



/*
 * Clear all the global "character" data
 */
static void player_wipe(int Ind)
{
	player_type *p_ptr = Players[Ind];
	object_type *old_inven;
	int i;


	/* Hack -- save the inventory pointer */
	old_inven = p_ptr->inventory;

	/* Hack -- zero the struct */
	WIPE(p_ptr, player_type);

	/* Hack -- reset the inventory pointer */
	p_ptr->inventory = old_inven;

	/* Wipe the history */
	for (i = 0; i < 4; i++)
	{
		strcpy(p_ptr->history[i], "");
	}

	/* No weight */
	p_ptr->total_weight = 0;

	/* No items */
	p_ptr->inven_cnt = 0;
	p_ptr->equip_cnt = 0;

	/* Clear the inventory */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		invwipe(&p_ptr->inventory[i]);
	}


	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;


	/* Wipe the spells */
	p_ptr->spell_learned1 = p_ptr->spell_learned2 = 0L;
	p_ptr->spell_worked1 = p_ptr->spell_worked2 = 0L;
	p_ptr->spell_forgotten1 = p_ptr->spell_forgotten2 = 0L;
	for (i = 0; i < 64; i++) p_ptr->spell_order[i] = 99;


	/* Clear "cheat" options */
	cheat_peek = FALSE;
	cheat_hear = FALSE;
	cheat_room = FALSE;
	cheat_xtra = FALSE;
	cheat_know = FALSE;
	cheat_live = FALSE;

	/* Assume no winning game */
	p_ptr->total_winner = FALSE;

	/* Assume no panic save */
	panic_save = 0;

	/* Assume no cheating */
	p_ptr->noscore = 0;
	
	/* clear the wilderness map */
	for (i = 0; i < MAX_WILD/8; i++) p_ptr->wild_map[i] = 0;

	/* clear the wilderness map */
	for (i = 0; i < MAX_WILD/8; i++) p_ptr->wild_map[i] = 0;

	/* Hack -- assume the player has an initial knowledge of the area close to town */
	for (i = 0; i < 13; i++)  p_ptr->wild_map[i/8] |= 1<<(i%8);
	
	/* Listen on the default chat channel */
	strncpy(p_ptr->main_channel,DEFAULT_CHANNEL,MAX_CHAN_LEN);

}




/*
 * Each player starts out with a few items, given as tval/sval pairs.
 * In addition, he always has some food and a few torches.
 */

static byte player_init[MAX_CLASS][3][2] =
{
	{
		/* Warrior */
		{ TV_POTION, SV_POTION_BESERK_STRENGTH },
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL }
	},

	{
		/* Mage */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_SWORD, SV_DAGGER },
		{ TV_POTION, SV_POTION_CURE_CRITICAL }
	},

	{
		/* Priest */
		{ TV_PRAYER_BOOK, 0 },
		{ TV_HAFTED, SV_MACE },
		{ TV_POTION, SV_POTION_CURE_CRITICAL }
	},

	{
		/* Rogue */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_SWORD, SV_SMALL_SWORD },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR }
	},

	{
		/* Ranger */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_BOW, SV_LONG_BOW }
    }

    ,{
		/* Paladin */
		{ TV_PRAYER_BOOK, 0 },
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL }
	}
};

static byte ironman_player_init[MAX_CLASS][3][2] =
{
	{
		/* Warrior */
		{ TV_LITE, SV_LITE_LANTERN },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
	},

	{
		/* Mage */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 1 },
		{ TV_LITE, SV_LITE_LANTERN },
	},

	{
		/* Priest */
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_LITE, SV_LITE_LANTERN },
	},

	{
		/* Rogue */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 1 },
		{ TV_LITE, SV_LITE_LANTERN },
	},

	{
		/* Ranger */
		{ TV_MAGIC_BOOK, 0 },
		{ TV_LITE, SV_LITE_LANTERN },
		{ TV_BOW, SV_LONG_BOW }
    }
    ,{
		/* Paladin */
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_LITE, SV_LITE_LANTERN },
    },

};

#if 0
// This is a very good function to use. The shortcoming is that it is missing all the usefull
// ironman and debug addons, and for some reason food + lite is not bundled with it.
/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(int Ind)
{
	player_type *p_ptr = Players[Ind];
	player_class *cp_ptr = &c_info[p_ptr->pclass];
	int i;
	const start_item *e_ptr;
	object_type *i_ptr;
	object_type object_type_body;


	/* Hack -- Give the player his equipment */
	for (i = 0; i < MAX_START_ITEMS; i++)
	{
		/* Access the item */
		e_ptr = &(cp_ptr->start_items[i]);

		/* Get local object */
		i_ptr = &object_type_body;

		/* Hack	-- Give the player an object */
		if (e_ptr->tval > 0)
		{
			/* Get the object_kind */
			int k_idx = lookup_kind(e_ptr->tval, e_ptr->sval);

			/* Valid item? */
			if (!k_idx) continue;

			/* Prepare the item */
			object_prep(i_ptr, k_idx);
			i_ptr->number = (byte)rand_range(e_ptr->min, e_ptr->max);

			object_aware(Ind, i_ptr);
			object_known(i_ptr);
			(void)inven_carry(Ind, i_ptr);
		}
	}
}
#endif

/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int		i, tv, sv;

	object_type	forge;
	object_type	*o_ptr = &forge;



	/* Hack -- Give the player some food */
	invcopy(o_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
	o_ptr->number = rand_range(3, 7);
    if (cfg_ironman) o_ptr->number *= 2;
	object_aware(Ind, o_ptr);
	object_known(o_ptr);
	(void)inven_carry(Ind, o_ptr);

	/* Give the player a WoR */
	invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL));
	o_ptr->number = 1;
	object_aware(Ind, o_ptr);
	object_known(o_ptr);
	(void)inven_carry(Ind, o_ptr);

    if (cfg_ironman)
    {
	/* Hack -- Give the player some oil */
	invcopy(o_ptr, lookup_kind(TV_FLASK, 0));
	o_ptr->number = rand_range(6, 14);
	object_known(o_ptr);
	(void)inven_carry(Ind, o_ptr);	
    }
    else
    {
	/* Hack -- Give the player some torches */
	invcopy(o_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
	o_ptr->number = rand_range(3, 7);
	o_ptr->pval = rand_range(3, 7) * 500;
	object_known(o_ptr);
	(void)inven_carry(Ind, o_ptr);
    }

    if (cfg_ironman)
    {
	/* More items for Ironmen */

	/* Scrolls of teleportation */
	invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_TELEPORT));
	o_ptr->number = 5;
	/* Warrior and rogues get twice as many */
	if( (p_ptr->pclass == CLASS_WARRIOR) || (p_ptr->pclass == CLASS_ROGUE) )
		o_ptr->number *= 2;
	o_ptr->discount = 0;
	object_aware(Ind, o_ptr);
	object_known(o_ptr);
	(void)inven_carry(Ind, o_ptr);
	
	/* Warriors get cure serious wounds */
	if( (p_ptr->pclass == CLASS_WARRIOR) )
	{
		invcopy(o_ptr, lookup_kind(TV_POTION, SV_POTION_CURE_SERIOUS));
		o_ptr->number = 5;
		o_ptr->discount = 0;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
	}
	
	/* Mages get third book */
	if( (p_ptr->pclass == CLASS_MAGE) )
	{
		invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 2));
		o_ptr->number = 1;
		o_ptr->discount = 0;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
	}

	/* Priests get third book */
	if( (p_ptr->pclass == CLASS_PRIEST) )
	{
		invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 2));
		o_ptr->number = 1;
		o_ptr->discount = 0;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
	}

    	/* Rangers get second book */
	if( (p_ptr->pclass == CLASS_RANGER) )
	{
		invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 1));
		o_ptr->number = 1;
		o_ptr->discount = 0;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
	}
    }

	/* 
     * Give the DM some interesting stuff or all players if this is dev mode
	 */

#ifndef DEBUG
	if (!strcmp(p_ptr->name,cfg_dungeon_master))
	{
#endif
		p_ptr->au = 10000000;

		/* All deep books */
		if ((p_ptr->pclass == CLASS_MAGE) || (p_ptr->pclass == CLASS_RANGER) ||
			(p_ptr->pclass == CLASS_ROGUE))
		{
			invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 4));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 5));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 6));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 7));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_MAGIC_BOOK, 8));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
		} 
		if ((p_ptr->pclass == CLASS_PRIEST) || (p_ptr->pclass == CLASS_PALADIN))
		{
			invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 4));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 5));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 6));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 7));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
			invcopy(o_ptr, lookup_kind(TV_PRAYER_BOOK, 8));
			o_ptr->number = 1;
			object_known(o_ptr);
			(void)inven_carry(Ind, o_ptr);
		} 

		/* Useful potions */
		invcopy(o_ptr, lookup_kind(TV_POTION, SV_POTION_AUGMENTATION));
		o_ptr->number = 20;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		invcopy(o_ptr, lookup_kind(TV_POTION, SV_POTION_EXPERIENCE));
		o_ptr->number = 30;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		invcopy(o_ptr, lookup_kind(TV_POTION, SV_POTION_HEALING));
		o_ptr->number = 15;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);

		/* Useful scrolls */
		invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_STAR_IDENTIFY));
		o_ptr->number = 25;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_TELEPORT));
		o_ptr->number = 30;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT));
		o_ptr->number = 20;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		
		/* Useful equipment */
		invcopy(o_ptr, lookup_kind(TV_RING, SV_RING_SPEED));
		o_ptr->pval = 30;
		o_ptr->number = 1;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		invcopy(o_ptr, lookup_kind(TV_AMULET, SV_AMULET_ESP));
		o_ptr->pval = 10;
		o_ptr->number = 1;
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
		
#ifndef DEBUG
	}
#endif
	
	/* Hack -- Give the player three useful objects */
	for (i = 0; i < 3; i++)
	{
	if (cfg_ironman)
	{
		tv = ironman_player_init[p_ptr->pclass][i][0];
        	sv = ironman_player_init[p_ptr->pclass][i][1];
	}
	else
	{
		tv = player_init[p_ptr->pclass][i][0];
		sv = player_init[p_ptr->pclass][i][1];
	}
		invcopy(o_ptr, lookup_kind(tv, sv));
		object_aware(Ind, o_ptr);
		object_known(o_ptr);
		(void)inven_carry(Ind, o_ptr);
	}
	
}

static void player_setup(int Ind)
{
	player_type *p_ptr = Players[Ind];
	player_type *q_ptr;
	int y, x, i, d, k, count = 0, Depth = p_ptr->dun_depth;
	cave_type *c_ptr;
	bool reposition;

	bool dawn = ((turn % (10L * TOWN_DAWN)) < (10L * TOWN_DAWN / 2)), require_los = 1; 

	/* Count players on this depth */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Skip this player */
		if (i == Ind) continue;

		/* Count */
		if (Players[i]->dun_depth == Depth)
			count++;
	}

	/* Make sure he's supposed to be here -- if not, then the level has
	 * been unstaticed and so he should forget his memory of the old level.
	 */
	if ((count >= players_on_depth[Depth]) || ((!cave[Depth]) && (Depth > 0)))
	{
		/* Clear the "marked" and "lit" flags for each cave grid */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				p_ptr->cave_flag[y][x] = 0;
			}
		}
		/* He is now on the level, so add him to the player_on_depth list 
		 * if neccecary. */
		if (count >= players_on_depth[Depth])
			players_on_depth[Depth]++;
	}

	/* Rebuild the level if neccecary */
	if (!cave[Depth]) 
	{		
		/* If a level is unstaticed and a player is on it, he will now
		 * stay in the dungeon and appear on the new level somewhere.
		 */
		if (p_ptr->dun_depth >= 0)
		{
			/* Build a new level and put him on it */
			alloc_dungeon_level(Depth);
			/* option 29 is auto_scum for the player */
			generate_cave(Ind, Depth, p_ptr->options[29]);
		}
		else
		/* rebuild the wilderness level */
		{
			alloc_dungeon_level(Depth);
			/* NB: Wilderness levels do not currently honor auto_scum */
			generate_cave(Ind, Depth, 0);
			/* hack -- this is important */
			if (!players_on_depth[Depth]) players_on_depth[Depth] = 1;
			
			/* paranoia, update the players wilderness map. */
			p_ptr->wild_map[(-p_ptr->dun_depth)/8] |= (1<<((-p_ptr->dun_depth)%8));
		}
	}

	/* Re-Place the player correctly */
	reposition = FALSE;

	/* Default location if just starting */
	if (!Depth && !p_ptr->py && !p_ptr->px)
	{
		reposition = TRUE;
	}

	/* Don't allow placement inside a shop if someone is shopping or 
	 * if we don't own it (anti-exploit) */
	for (i = 0; i < num_houses; i++)
	{
		/* Are we inside this house? */
		if (house_inside(Ind, i))
		{
			/* If we don't own it, get out of it */
			if( !house_owned_by(Ind, i) )
			{
				reposition = TRUE;
				break;
			}
			/* Is anyone shopping in it? */
			for (k = 1; k <= NumPlayers; k++ )
			{
				q_ptr = Players[k];
				if(q_ptr && Ind != k)
				{
					/* Someone in here? */
					if(q_ptr->player_store_num == i && q_ptr->store_num == 8)
					{
						reposition = TRUE;
						break;
					}
				}				
			}
			break;			
		}
	}

	/* If we need to reposition the player, do it */
	if(reposition)
	{
		/* Moving from wilderness to town */
		if (Depth < 0)
		{
			players_on_depth[Depth]--;
			Depth = p_ptr->dun_depth = 0;
			players_on_depth[Depth]++;
			p_ptr->world_x = 0;
			p_ptr->world_y = 0;
		}
		/* Put us in the tavern */
		p_ptr->py = level_down_y[0];
		p_ptr->px = level_down_x[0];
	}

	/* Memorize town */
	if (!Depth)
	{
		/* Memorize the town if it's daytime */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				byte *w_ptr = &p_ptr->cave_flag[y][x];

				/* Acquire pointer */
				c_ptr = &cave[0][y][x];

				/* If day or interesting, memorize */
				if (dawn || !is_boring(c_ptr->feat) || c_ptr->info & CAVE_ROOM)
					*w_ptr |= CAVE_MARK;
			}
		}
	}
	/* hack -- update night/day in wilderness levels */ 
	if ((Depth < 0) && (IS_DAY)) wild_apply_day(Depth); 
	if ((Depth < 0) && (IS_NIGHT)) wild_apply_night(Depth); 

	// Hack -- don't require line of sight if we are stuck in something 
	// solid, such as rock.  This might happen if the level unstatics
	// and then regenerates when we try to log back on.
	if (cave_empty_bold(Depth, p_ptr->py, p_ptr->px))
		require_los = TRUE;
	else 
		require_los = FALSE;

	for (i = 0; i < 3000; i++)
	{
		d = (i + 4) / 10;

		/* Pick a location */
		/* Hack -- ghosts do not scatter, as they may not be in a line of sight
		   with a valid region */
        if (!p_ptr->ghost)
		{
			// Hack -- invery require_los since scatter actually takes
			// a "don't require line of sight" boolean parameter.
			scatter(Depth, &y, &x, p_ptr->py, p_ptr->px, d, !require_los);

			if (!in_bounds(Depth, y, x) || !cave_empty_bold(Depth, y, x)) continue;
		}
		else
		{
			/* ghosts can be on top of stuff */
			x = p_ptr->px;
			y = p_ptr->py;
		}

		break;
	}

	/* Set the player's location */
	p_ptr->py = y;
	p_ptr->px = x;

	/* Update the location's player index */
	cave[Depth][y][x].m_idx = 0 - Ind;

	/* Show him to everybody */
	everyone_lite_spot(Depth, y, x);

	/* Add him to the player name database, if he is not already there */
	if (!lookup_player_name(p_ptr->id))
	{
		/* Add */
		add_player_name(p_ptr->name, p_ptr->id);
	printf("Player Name is [%s], id is %d\n",p_ptr->name, p_ptr->id);
	}

	/* Set his "current activities" variables */
	p_ptr->current_spell = p_ptr->current_rod = p_ptr->current_activation = -1;
	p_ptr->current_house = p_ptr->current_selling = p_ptr->store_num = -1;

	/* Set the player's "panel" information */
	p_ptr->max_panel_rows = (MAX_HGT / SCREEN_HGT) * 2 - 2;
	p_ptr->max_panel_cols = (MAX_WID / SCREEN_WID) * 2 - 2;

	p_ptr->panel_row = ((p_ptr->py - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
	if (p_ptr->panel_row > p_ptr->max_panel_rows) p_ptr->panel_row = p_ptr->max_panel_rows;
	else if (p_ptr->panel_row < 0) p_ptr->panel_row = 0;

	p_ptr->panel_col = ((p_ptr->px - SCREEN_WID / 4) / (SCREEN_WID / 2));
	if (p_ptr->panel_col > p_ptr->max_panel_cols) p_ptr->panel_col = p_ptr->max_panel_cols;
	else if (p_ptr->panel_col < 0) p_ptr->panel_col = 0;

	p_ptr->cur_hgt = MAX_HGT;
	p_ptr->cur_wid = MAX_WID;

	/* Set the rest of the panel information */
	panel_bounds(Ind);

	/* Make sure his party still exists */
	if (p_ptr->party && parties[p_ptr->party].num == 0)
	{
		/* Reset to neutral */
		p_ptr->party = 0;
	}

	/* Tell the server to redraw the player's display */
	p_ptr->redraw |= PR_MAP | PR_EXTRA | PR_BASIC | PR_HISTORY | PR_VARIOUS | PR_OFLAGS;
	p_ptr->redraw |= PR_PLUSSES;

	/* Update his view, light, bonuses, and torch radius */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_BONUS | PU_TORCH | PU_DISTANCE);

	/* Update his inventory, equipment, and spell info */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL);

	/* This guy is alive now */
	p_ptr->alive = TRUE;

	/* Set turn player entered level */
	p_ptr->old_turn = turn;
}



/*
 * Create a character.  Then wait for a moment.
 *
 * The delay may be reduced, but is recommended to keep players
 * from continuously rolling up characters, which can be VERY
 * expensive CPU wise.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
bool player_birth(int Ind, cptr name, cptr pass, int conn, int race, int class, int sex, int stat_order[6])
{
	player_type *p_ptr;
	int i;

	/* Do some consistency checks */
    if (race < 0 || race >= MAX_RACES) race = RACE_HUMAN;
    if (class < 0 || class >= MAX_CLASS) class = CLASS_WARRIOR;
	if (sex < 0 || sex > 1) sex = 0;

	/* Allocate memory for him */
	MAKE(Players[Ind], player_type);

	/* Allocate memory for his inventory */
	C_MAKE(Players[Ind]->inventory, INVEN_TOTAL, object_type);

	/* Set pointer */
	p_ptr = Players[Ind];

	/* Clear old information */
	player_wipe(Ind);

	/* Copy his name and connection info */
	strcpy(p_ptr->name, name);
	strcpy(p_ptr->pass, pass);
	p_ptr->conn = conn;

	/* Verify his name and create a savefile name */
	if (!process_player_name(Ind, TRUE)) return FALSE;

	/* Attempt to load from a savefile */
	character_loaded = FALSE;

	/* Try to load */
	if (!load_player(Ind))
	{
		/* Loading failed badly */
		return FALSE;
	}

	/* Did loading succeed? */
	if (character_loaded)
	{
		/* Loading succeeded */		
		player_setup(Ind);
		return TRUE;		
	}

	/* Else, loading failed, but we just create a new character */

	/* Hack -- rewipe the player info if load failed */
	player_wipe(Ind);

	/* Copy his name and connection info */
	strcpy(p_ptr->name, name);
	strcpy(p_ptr->pass, pass);
	p_ptr->conn = conn;

	/* Reprocess his name */
	if (!process_player_name(Ind, TRUE)) return FALSE;

	/* Set info */
	p_ptr->prace = race;
	p_ptr->pclass = class;
	p_ptr->male = sex;

	/* Set pointers */
	p_ptr->rp_ptr = &p_info[p_ptr->prace];
	p_ptr->cp_ptr = &c_info[class];
	p_ptr->mp_ptr = &c_info[class].spells;

	/* Set his ID */
	p_ptr->id = player_id++;

	/* Actually Generate */

 	/* This enables maximize mode for new characters. --RLS */

	p_ptr->maximize=1;

	/* Remember when this player was created in terms of the server turn counter */
	p_ptr->birth_turn = turn;

	/* No autoroller */
	get_stats(Ind, stat_order);

	/* Roll for base hitpoints */
	get_extra(Ind);

	/* Roll for age/height/weight */
	get_ahw(Ind);

	/* Roll for social class */
	get_history(Ind);

	/* Roll for gold */
	get_money(Ind);

	/* Hack -- outfit the player */
	player_outfit(Ind);
	
	/* Hack -- Give him "awareness" of certain objects */
	for (i = 0; i < z_info->k_max; i++) 
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) p_ptr->obj_aware[i] = TRUE;
	}

	/* Set his location, panel, etc. */
	player_setup(Ind);

	/* Success */
	return TRUE;
}


/*
 * We are starting a "brand new" server.  We need to initialze the unique
 * info, so that they will be created.  This function is only called if the
 * server state savefile could not be loaded.
 */
void server_birth(void)
{
	int i;

	/* Initialize uniques */
	for (i = 0; i < MAX_R_IDX; i++)
	{
		/* Make sure we have a unique */
		if (!(r_info[i].flags1 & RF1_UNIQUE))
			continue;

		/* Set his maximum creation number */
		r_info[i].max_num = 1;
		
		/* Number of minutes until he respawns */
		/* -1 is used to denote an undefined respawn time.  This should
		 * be set when the unique is killed.
		 */
		r_info[i].respawn_timer = -1;
	}

	/* Set party zero's name to "Neutral" */
	strcpy(parties[0].name, "Neutral");

	/* First player's ID should be 1 */
	player_id = 1;
}
