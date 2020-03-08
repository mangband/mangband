/* File: birth.c */

/* Purpose: create a player character */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


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
static s16b		stat_use[A_MAX];



/*
 * Returns adjusted stat -JK-
 * Algorithm by -JWT-
 *
 * auto_roll is boolean and states maximum changes should be used rather
 * than random ones to allow specification of higher values to wait for
 *
 * The "p_ptr->maximize" code is important	-BEN-
 */
static int adjust_stat(player_type *p_ptr, int value, s16b amount, int auto_roll)
{
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
				value += ((auto_roll ? 15 : randint1(15)) + 5);
			}
			else if (value < 18+90)
			{
				value += ((auto_roll ? 6 : randint1(6)) + 2);
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
static void get_stats(player_type *p_ptr)
{
	int		i, j;
	int		bonus;
	int		dice[18];
	int		stats[A_MAX];
	int         n17, n16, n15;

	/* Clear "stats" array */
	for (i = 0; i < A_MAX; i++)
		stats[i] = 0;

	/* Check over the given stat order, to prevent cheating */
	for (i = 0; i < A_MAX; i++)
	{
		/* Check range */
		if (p_ptr->stat_order[i] < 0 || p_ptr->stat_order[i] > (A_MAX-1))
		{
			p_ptr->stat_order[i] = 1;
		}

		/* Check for duplicated entries */
		if (stats[p_ptr->stat_order[i]] == 1)
		{
			/* Find a stat that hasn't been specified yet */
			for (j = 0; j < A_MAX; j++)
			{
				if (stats[j])
					continue;

				p_ptr->stat_order[i] = j;
			}
		}

		/* Set flag */
		stats[p_ptr->stat_order[i]] = 1;
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
			dice[i] = randint1(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 42) && (j < 54)) break;
	}

	/* Acquire the stats */
	for (i = 0; i < A_MAX; i++)
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
	for (i = 0; i < A_MAX; i++)
	{
		for (j = 0; j < (A_MAX - 1); j++)
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
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_max[p_ptr->stat_order[i]] = stats[i];
	}

	/* Adjust the stats */
	for (i = 0; i < A_MAX; i++)
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
			stat_use[i] = adjust_stat(p_ptr, p_ptr->stat_max[i], bonus, FALSE);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}
	}
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra(player_type *p_ptr)
{
	int		i, j, min_value, max_value;


	/* Level one (never zero!) */
	p_ptr->lev = 1;

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
			j = randint1(p_ptr->hitdie);
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
static void get_history(player_type *p_ptr)
{
	int		i, n, chart, roll, social_class;

	char	*s, *t, *p;

	char	buf[240];



	/* Clear the previous history strings */
	for (i = 0; i < 4; i++) p_ptr->history[i][0] = '\0';


	/* Clear the history text */
	buf[0] = '\0';

	/* Set pointers */
	t = &buf[0];
	p = &p_ptr->descrip[0];

	/* Initial social class */
	social_class = randint1(4);

	/* Starting place */
	chart = p_info[p_ptr->prace].hist;

	/* Process the history */
	while (chart)
	{
		/* Start over */
		i = 0;

		/* Roll for nobility */
		roll = randint1(100);

		/* Access the proper entry in the table */
		while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

		/* Acquire the textual history */
		for (s = h_text + h_info[i].text; *s; s++)
		{
#define g_strcat(P, T) n = strlen((T)); strncpy((P), (T), n); (P) += n 
#define P_CASE(C, F, T) case (C): g_strcat(t, (F)); g_strcat(p, (T)); break
#define G_CASE(C, F, TM, TF) case (C): g_strcat(t, (F)); g_strcat(p, (p_ptr->male ? (TM) : (TF)) ); break
			switch (*s)
			{
			case '$':
			case '~':
				s++;
				switch (*s)
				{
					G_CASE('u', "You", "He", "She");
					G_CASE('r', "Your", "His", "Her");
					P_CASE('a', "are", "is");
					P_CASE('h', "have", "has");
					default: continue;
				}
			break;
			default:
				*t++ = *p++ = *s;
			}
		}
		*t = *p = '\0';

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
static void get_ahw(player_type *p_ptr)
{
	/* Calculate the age */
	p_ptr->age = p_ptr->rp_ptr->b_age + randint1(p_ptr->rp_ptr->m_age);

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
static void get_money(player_type *p_ptr)
{
	int        i, gold;

	/* Social Class determines starting gold */
	gold = (p_ptr->sc * 6) + randint1(100) + 300;

	/* Process the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Mega-Hack -- reduce gold for high stats */
		if (p_ptr->stat_cur[i] >= 18+50) gold -= 300;
		else if (p_ptr->stat_cur[i] >= 18+20) gold -= 200;
		else if (p_ptr->stat_cur[i] > 18) gold -= 150;
		else gold -= (p_ptr->stat_cur[i] - 8) * 10;
	}

	/* Minimum 100 gold */
	if (gold < 100) gold = 100;

	/* She charmed the banker into it! -CJS- */
	/* She slept with the banker.. :) -GDH-  */
	if (!p_ptr->male) gold += 50;

	/* Save the gold */
	p_ptr->au = gold;
}



/*
 * Clear all the global "character" data
 */
void player_wipe(player_type *p_ptr)
{
	object_type *old_inven;
	monster_lore *old_lore;
	monster_lore *l_ptr;
	//byte *old_channels;
	byte *old_arts;
	bool *old_kind_aware;
	bool *old_kind_tried;
	s16b *old_r_killed;
	byte *f_attr, *k_attr, *d_attr, *r_attr, *pr_attr;
	char *f_char, *k_char, *d_char, *r_char, *pr_char;
	char *c_buf;
	int i;


	/* Hack -- save pointers */
	old_inven = p_ptr->inventory;
	old_lore = p_ptr->l_list;
	//old_channels = p_ptr->on_channel;
	old_arts = p_ptr->a_info;
	old_kind_aware = p_ptr->kind_aware;
	old_kind_tried = p_ptr->kind_tried;
	old_r_killed = p_ptr->r_killed;
	f_attr = p_ptr->f_attr; f_char = p_ptr->f_char;
	r_attr = p_ptr->r_attr; r_char = p_ptr->r_char;
	k_attr = p_ptr->k_attr; k_char = p_ptr->k_char;
	d_attr = p_ptr->d_attr; d_char = p_ptr->d_char;
	pr_attr = p_ptr->pr_attr; pr_char = p_ptr->pr_char;
	c_buf = p_ptr->cbuf.buf;

	/* Clear character history ! */
	history_wipe(p_ptr->charhist);

	/* Hack -- zero the struct */
	WIPE(p_ptr, player_type);

	/* Hack -- restore pointers */
	p_ptr->inventory = old_inven;
	p_ptr->l_list = old_lore;
	//p_ptr->on_channel = old_channels;
	p_ptr->a_info = old_arts;
	p_ptr->kind_aware = old_kind_aware;
	p_ptr->kind_tried = old_kind_tried;
	p_ptr->r_killed = old_r_killed;
	p_ptr->f_attr = f_attr; p_ptr->f_char = f_char;
	p_ptr->r_attr = r_attr; p_ptr->r_char = r_char;
	p_ptr->k_attr = k_attr; p_ptr->k_char = k_char;
	p_ptr->d_attr = d_attr; p_ptr->d_char = d_char;
	p_ptr->pr_attr = pr_attr; p_ptr->pr_char = pr_char;
	p_ptr->cbuf.buf = c_buf;

	/* Wipe grafmode offsets */
	for (i = 0; i < LIGHTING_MAX; i++)
	{
		p_ptr->graf_lit_offset[i][0] = 0;
		p_ptr->graf_lit_offset[i][1] = 0;
	}

	/* Set default options */
	for (i = 0; i < OPT_MAX; i++)
	{
		const option_type *opt_ptr = &option_info[i];
		/* Option is locked */
		if (opt_ptr->o_bit)
		{
			p_ptr->options[opt_ptr->o_uid] = opt_ptr->o_norm;
		}
	}

	/* Default support flags */
	p_ptr->supports_slash_fx = FALSE;

	/* Wipe the birth history */
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

	/* Reset the "monsters" */
	for (i = 1; i < z_info->r_max; i++)
	{
		l_ptr = p_ptr->l_list + i;

		/* Clear player kills */
		l_ptr->pkills = 0;
		p_ptr->r_killed[i] = 0;
	}
	/* Clean "old_lore" */
	l_ptr = &p_ptr->old_l;
	for (i = 0; i < MONSTER_BLOW_MAX; i++) l_ptr->blows[i] = 0;	
	l_ptr->flags1 = l_ptr->flags2 = l_ptr->flags3 = l_ptr->flags4 = l_ptr->flags5 = l_ptr->flags6 = 0L;   
	l_ptr->cast_innate = l_ptr->cast_spell = 0; 
	p_ptr->old_monster_race_idx = 0;

	/* Wipe item knowledge */
	for (i = 0; i < z_info->k_max; i++)
	{
		p_ptr->kind_aware[i] = FALSE;
		p_ptr->kind_tried[i] = FALSE;
	}

	/* Clear "artifacts found" list */
	for (i = 0; i < z_info->a_max; i++)
	{
		p_ptr->a_info[i] = ARTS_NOT_FOUND;
	}

	/* Start with no quests */
	for (i = 1; i < MAX_Q_IDX; i++)
	{
		p_ptr->q_list[i].level = 0;
	}
	
	/* Hack -- Add Sauron & Morgoth quests */
	p_ptr->q_list[0].level = 99;
	p_ptr->q_list[1].level = 100;

	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;


	/* Wipe the spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		p_ptr->spell_flags[i] = 00;
		p_ptr->spell_order[i] = 99;
	}

	/* Assume no winning game */
	p_ptr->total_winner = FALSE;

	/* Assume no cheating */
	p_ptr->noscore = 0;

	/* Not running */
	p_ptr->running = FALSE;
	p_ptr->run_request = 0;
	p_ptr->ran_tiles = 0;

	/* Feelings don't carry-on between saves (sorry) */
	p_ptr->feeling = 0;

	/* clear the wilderness map */
	for (i = 0; i < MAX_WILD/8; i++) p_ptr->wild_map[i] = 0;

	/* clear the wilderness map */
	for (i = 0; i < MAX_WILD/8; i++) p_ptr->wild_map[i] = 0;

	/* Hack -- assume the player has an initial knowledge of the area close to town */
	for (i = 0; i < 13; i++)  p_ptr->wild_map[i/8] |= 1<<(i%8);
	
	/* Setup stream pointers */
	for (i = 0; i < MAX_STREAMS; i++)
	{
		if (streams[i].addr == NTERM_WIN_OVERHEAD
		 || streams[i].addr == NTERM_WIN_MAP)
		{
			p_ptr->stream_cave[i] = &p_ptr->scr_info[0][0];
		}
		/*else if (i == STREAM_FILE_TEXT)
		{
			p_ptr->stream_cave[i] = &p_ptr->file[0][0];
		}*/
		else
		{
			p_ptr->stream_cave[i] = &p_ptr->info[0][0];
		}
	}

	/* Clear old channels */
	for (i = 0; i < MAX_CHANNELS; i++) p_ptr->on_channel[i] = 0;

	/* Listen on the default chat channel */
	p_ptr->main_channel = 0;
	strcpy(p_ptr->second_channel, "");
	p_ptr->on_channel[0] |= UCM_EAR;
	
	/* Output to default terminal */
	p_ptr->remote_term = NTERM_WIN_OVERHEAD;
}

/* XXX XXX XXX HACK -- Wipe player but keep some stuff... */
void player_net_wipe(player_type *p_ptr, int reach)
{
	player_type p_tmp = { 0 };
	int i;

	p_tmp.conn = p_ptr->conn;
	strcpy(p_tmp.name, p_ptr->name);
	strcpy(p_tmp.pass, p_ptr->pass);
	strcpy(p_tmp.basename, p_ptr->basename);
	strcpy(p_tmp.realname, p_ptr->realname);
	strcpy(p_tmp.hostname, p_ptr->hostname);
	strcpy(p_tmp.addr, p_ptr->addr);
	p_tmp.version = p_ptr->version;
	p_tmp.state = p_ptr->state;
	p_tmp.id = p_ptr->id;
	strcpy(p_tmp.savefile, p_ptr->savefile);

	p_tmp.cbuf.max = p_ptr->cbuf.max;

	p_tmp.lives = p_ptr->lives;

	for (i = 0; i < 6; i++)
	{
		p_tmp.infodata_sent[i] = p_ptr->infodata_sent[i];
	}

	for (i = 0; i < A_MAX; i++)
	{
		p_tmp.stat_order[i] = p_ptr->stat_order[i];
	}

	p_tmp.prace = p_ptr->prace;
	p_tmp.pclass = p_ptr->pclass;
	p_tmp.male = p_ptr->male;

	player_wipe(p_ptr);

	p_ptr->conn = p_tmp.conn;
	strcpy(p_ptr->name, p_tmp.name);
	strcpy(p_ptr->pass, p_tmp.pass);
	strcpy(p_ptr->basename, p_tmp.basename);
	strcpy(p_ptr->realname, p_tmp.realname);
	strcpy(p_ptr->hostname, p_tmp.hostname);
	strcpy(p_ptr->addr, p_tmp.addr);
	p_ptr->version = p_tmp.version;
	p_ptr->state = p_tmp.state;
	p_ptr->id = p_tmp.id;
	strcpy(p_ptr->savefile, p_tmp.savefile);

	p_ptr->cbuf.max = p_tmp.cbuf.max;

	p_ptr->lives = p_tmp.lives;

	/* TODO: also copy p_ptr->options? MAYBE */

	for (i = 0; i < 6; i++)
	{
		p_ptr->infodata_sent[i] = p_tmp.infodata_sent[i];
	}

	if (reach)
	{
		for (i = 0; i < A_MAX; i++)
		{
			p_ptr->stat_order[i] = p_tmp.stat_order[i];
		}

		p_ptr->prace = p_tmp.prace;
		p_ptr->pclass = p_tmp.pclass;
		p_ptr->male = p_tmp.male;
	}
}

/* 
 * Verify / Overwrite visual data with server defaults
 */
void player_verify_visual(player_type *p_ptr)
{
	int i;

	for (i = 0; i < MIN(MAX_FLVR_IDX, z_info->flavor_max); i++) 
	{
		if (!p_ptr->flvr_attr[i]) p_ptr->flvr_attr[i] = flavor_info[i].d_attr;
		if (!p_ptr->flvr_char[i]) p_ptr->flvr_char[i] = flavor_info[i].d_char;
	}

	for (i = 0; i < z_info->f_max; i++)
	{
		/* Overwrite mimics */
		if (f_info[i].mimic != i)
		{ 
			p_ptr->f_attr[i] = p_ptr->f_attr[f_info[i].mimic];
			p_ptr->f_char[i] = p_ptr->f_char[f_info[i].mimic];
		}
		if (!p_ptr->f_attr[i]) p_ptr->f_attr[i] = f_info[i].d_attr;
		if (!p_ptr->f_char[i]) p_ptr->f_char[i] = f_info[i].d_char;
	}

	for (i = 0; i < z_info->k_max; i++)
	{
		if (!p_ptr->k_attr[i]) p_ptr->k_attr[i] = (k_info[i].flavor ? p_ptr->flvr_attr[k_info[i].flavor]: k_info[i].d_attr);
		if (!p_ptr->k_char[i]) p_ptr->k_char[i] = (k_info[i].flavor ? p_ptr->flvr_char[k_info[i].flavor]: k_info[i].d_char);

		if (!p_ptr->d_attr[i]) p_ptr->d_attr[i] = (k_info[i].flavor ? p_ptr->flvr_attr[k_info[i].flavor]: k_info[i].d_attr);
		if (!p_ptr->d_char[i]) p_ptr->d_char[i] = (k_info[i].flavor ? p_ptr->flvr_char[k_info[i].flavor]: k_info[i].d_char);
	}

	for (i = 0; i < z_info->r_max; i++)
	{
		if (!p_ptr->r_attr[i]) p_ptr->r_attr[i] = r_info[i].d_attr;
		if (!p_ptr->r_char[i]) p_ptr->r_char[i] = r_info[i].d_char;
	}

	for (i = 0; i < 128; i++) 
	{
		if (!p_ptr->tval_attr[i]) p_ptr->tval_attr[i] = tval_to_attr[i]; 
		if (!p_ptr->tval_char[i]) p_ptr->tval_char[i] = tval_to_char[i];
	}

	for (i = 0; i < (z_info->c_max+1)*z_info->p_max; i++)
	{
		if (!p_ptr->pr_attr[i]) p_ptr->pr_attr[i] = p_ptr->r_attr[0];
		if (!p_ptr->pr_char[i]) p_ptr->pr_char[i] = p_ptr->r_char[0];
	}
}

/*
 * Hard-coded items to give DM at his birth.
 */
static byte dm_player_outfit[][4] =
{
	{ TV_POTION, SV_POTION_AUGMENTATION,	20, 0 },	
	{ TV_POTION, SV_POTION_EXPERIENCE,  	30, 0 },
	{ TV_POTION, SV_POTION_HEALING,     	15, 0 },

	{ TV_SCROLL, SV_SCROLL_STAR_IDENTIFY,	25, 0 },
	{ TV_SCROLL, SV_SCROLL_TELEPORT,    	30, 0 },
	{ TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT,20, 0 },

	{ TV_RING,   SV_RING_SPEED,	1, 30 },
	{ TV_AMULET, SV_AMULET_ESP,	1, 10 },

	{ 0, 0, 0, 0 }
};

/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(player_type *p_ptr)
{
	player_class *cp_ptr = &c_info[p_ptr->pclass];
	int i;
	const start_item *e_ptr;
	object_type *i_ptr;
	object_type object_type_body;

#define player_outfit_i(P,K,N,PV) \
	i_ptr = &object_type_body; \
	object_prep(i_ptr, (K)); \
	i_ptr->number = (N); \
	i_ptr->origin = ORIGIN_BIRTH; \
	if ( (PV) ) i_ptr->pval = (PV); \
	object_aware((P), i_ptr); \
	object_known(i_ptr); \
	(void)inven_carry((P), i_ptr)

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

			/* Give item */
			player_outfit_i(p_ptr, k_idx, (byte)rand_range(e_ptr->min, e_ptr->max), 0); 
		}
	}

	/* Give food */
	player_outfit_i(p_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION), rand_range(3, 7) * (cfg_ironman+1), 0);

	/* Give lite */
	if (!cfg_ironman)
	{
		/* Torches */
		player_outfit_i(p_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH), rand_range(3, 7), rand_range(3, 7) * 500);
	}
	else
	{
		/* Lantern + Oil */
		player_outfit_i(p_ptr, lookup_kind(TV_LITE, SV_LITE_LANTERN), 1, 0);
		player_outfit_i(p_ptr, lookup_kind(TV_FLASK, 0), rand_range(6, 14), 0);
	}

	/* Give a free WoR */
	if (!cfg_ironman)
	{
		player_outfit_i(p_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL), 1, 0);
	}

	/*
	 * Give the DM (or all players, if this is dev. mode) some interesting stuff
	 */
#ifndef DEBUG
	if (!is_dm_p(p_ptr)) return;
#endif
	/* Max recall depth */
	p_ptr->max_dlv = (MAX_DEPTH - 1);
	/* Lots of gold */
	p_ptr->au = 10000000;
	/* High Spell books */
	for (i = 4; (cp_ptr->spell_book != 0 && i < 8); i++)
	{
		player_outfit_i(p_ptr, lookup_kind(cp_ptr->spell_book, i), 1, 0);
	}
	/* Other items */
	for (i = 0; dm_player_outfit[i][0]; i++)
	{
		player_outfit_i(p_ptr, lookup_kind(dm_player_outfit[i][0], dm_player_outfit[i][1]), 
			dm_player_outfit[i][2], dm_player_outfit[i][3]);
	}
}

static void player_admin(player_type *p_ptr);
void player_setup(player_type *p_ptr)
{
	player_type *q_ptr;
	int y, x, i, d, k, count = 0, Depth = p_ptr->dun_depth;
	cave_type *c_ptr;
	bool reposition;

	bool dawn = ((turn.turn % (10L * TOWN_DAWN)) < (10L * TOWN_DAWN / 2)), require_los = 1; 

	/* DM powers? */
	player_admin(p_ptr);

	/* Count players on this depth */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Skip this player */
		if (same_player(p_ptr, Players[i])) continue;

		/* Count */
		if (Players[i]->dun_depth == Depth)
			count++;
	}

	/* Make sure he's supposed to be here -- if not, then the level has
	 * been unstaticed and so he should forget his memory of the old level.
	 */
	if ((count >= players_on_depth[Depth]) || ((!cave[Depth]) && (Depth > 0))
	    || (ht_passed(&turn_cavegen[Depth], &p_ptr->last_turn, 0)) )
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

		/* Hack -- ironmen get 2 extra turns of invulnerability, see #1014 */
		if (p_ptr->dun_depth > 0 && cfg_ironman)
			if (p_ptr->invuln == 0) set_invuln(p_ptr, p_ptr->invuln + 2);
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
			generate_cave(p_ptr, Depth, option_p(p_ptr,AUTO_SCUM));
		}
		else
		/* rebuild the wilderness level */
		{
			alloc_dungeon_level(Depth);
			/* NB: Wilderness levels do not currently honor auto_scum */
			generate_cave(p_ptr, Depth, 0);
			/* hack -- this is important */
			if (!players_on_depth[Depth]) players_on_depth[Depth] = 1;
			
			/* paranoia, update the players wilderness map. */
			p_ptr->wild_map[(-p_ptr->dun_depth)/8] |= (1<<((-p_ptr->dun_depth)%8));
		}
	}

	/* Hack -- grid might already be occupied by us, clear it */
	if (cave[Depth][p_ptr->py][p_ptr->px].m_idx == 0 - p_ptr->Ind)
		cave[Depth][p_ptr->py][p_ptr->px].m_idx = 0;

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
		if (house_inside(p_ptr, i))
		{
			/* If we don't own it, get out of it */
			if( !house_owned_by(p_ptr, i) )
			{
				reposition = TRUE;
				break;
			}
			/* Is anyone shopping in it? */
			for (k = 1; k <= NumPlayers; k++ )
			{
				q_ptr = Players[k];
				if (!same_player(q_ptr, p_ptr))
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

	/* Don't allow placement inside an arena */
	if (pick_arena(Depth, p_ptr->py, p_ptr->px) != -1)
	{
		reposition = TRUE;
	}
	/* Reset */
	p_ptr->arena_num = -1;

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
			// Hack -- invert require_los since scatter actually takes
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
	cave[Depth][y][x].m_idx = 0 - p_ptr->Ind;

	/* Show him to everybody */
	everyone_lite_spot(Depth, y, x);

	/* Add him to the player name database, if he is not already there */
	if (!lookup_player_name(p_ptr->id))
	{
		/* Add */
		add_player_name(p_ptr->name, p_ptr->id);
	printf("Player Name is [%s], id is %d\n", p_ptr->name, (int)p_ptr->id);
	}

	/* Set his "current activities" variables */
	p_ptr->command_arg = -2; 
	p_ptr->command_dir = 0;
	p_ptr->current_spell = p_ptr->current_object = -1;
	p_ptr->current_house = p_ptr->current_selling = p_ptr->store_num = -1;
	p_ptr->panel_row_old = p_ptr->panel_col_old = -1;

	/* Make sure his party still exists */
	if (p_ptr->party && (
		parties[p_ptr->party].num == 0
		|| ht_passed(&parties[p_ptr->party].created, &p_ptr->last_turn, 0)
	))
	{
		/* Reset to neutral */
		p_ptr->party = 0;
	}

	/* Tell the server to redraw the player's display */
	p_ptr->redraw |= PR_MAP | PR_EXTRA | PR_BASIC | PR_VARIOUS | PR_OFLAGS;
	p_ptr->redraw |= PR_PLUSSES | PR_FLOOR;

	/* Update his view, light, bonuses, and torch radius */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_BONUS | PU_TORCH | PU_DISTANCE);

	/* Update his inventory, equipment, and spell info */
	p_ptr->redraw_inven |= (0xFFFFFFFFFFFFFFFFLL);
	p_ptr->window |= (PW_SPELL);
	p_ptr->window |= (PW_ITEMLIST);

	/* This guy is alive now */
	p_ptr->alive = TRUE;

	/* Set turn player entered level */
	p_ptr->old_turn = turn;
}

static void player_admin(player_type *p_ptr)
{
	/* Hack -- set Dungeon Master flags */
#ifdef DEBUG
	p_ptr->dm_flags |= (DM___MENU | DM_CAN_MUTATE_SELF);
#endif
	if (cfg_dungeon_master && !strcmp(p_ptr->name, cfg_dungeon_master))
	{
		/* All DM powers ! */
		p_ptr->dm_flags = 0xFFFFFFFF;
		if (!cfg_secret_dungeon_master)	p_ptr->dm_flags ^= DM_SECRET_PRESENCE;
	}
}

/*
 * Allocate player structure
 */
player_type* player_alloc()
{
	player_type *p_ptr;

	/* Allocate memory for him */
	MAKE(p_ptr, player_type);

	/* Allocate memory for his inventory */
	C_MAKE(p_ptr->inventory, INVEN_TOTAL, object_type);

	/* Allocate memory for his lore array */
	C_MAKE(p_ptr->l_list, z_info->r_max, monster_lore);

	/* Allocate memory for his artifact array */
	C_MAKE(p_ptr->a_info, z_info->a_max, byte);

	/* Allocate memory for his dungeon flags array */
	C_MAKE(p_ptr->kind_aware, z_info->k_max, bool);
	C_MAKE(p_ptr->kind_tried, z_info->k_max, bool);
	C_MAKE(p_ptr->r_killed,  z_info->r_max, s16b);

	/* Allocate memory for his visuals */
	C_MAKE(p_ptr->f_attr, z_info->f_max, byte);
	C_MAKE(p_ptr->f_char, z_info->f_max, char);
	C_MAKE(p_ptr->k_attr, z_info->k_max, byte);
	C_MAKE(p_ptr->k_char, z_info->k_max, char);
	C_MAKE(p_ptr->d_attr, z_info->k_max, byte);
	C_MAKE(p_ptr->d_char, z_info->k_max, char);	
	C_MAKE(p_ptr->r_attr, z_info->r_max, byte);
	C_MAKE(p_ptr->r_char, z_info->r_max, char);
	C_MAKE(p_ptr->pr_attr, (z_info->c_max+1)*z_info->p_max, byte);
	C_MAKE(p_ptr->pr_char, (z_info->c_max+1)*z_info->p_max, char);

	/* Hack -- initialize history */
	p_ptr->charhist = NULL;

	/* Set pointer */
	return p_ptr;
}
/*
 * Free player structure
 */
void player_free(player_type *p_ptr)
{
	if (!p_ptr) return;

	if (p_ptr->inventory)
		KILL(p_ptr->inventory);

	if (p_ptr->l_list)
		KILL(p_ptr->l_list);

	if (p_ptr->a_info)
		KILL(p_ptr->a_info);

	if (p_ptr->kind_aware)	KILL(p_ptr->kind_aware);
	if (p_ptr->kind_tried)	KILL(p_ptr->kind_tried);
	if (p_ptr->r_killed)	KILL(p_ptr->r_killed);

	if (p_ptr->f_attr)		KILL(p_ptr->f_attr);
	if (p_ptr->f_char)		KILL(p_ptr->f_char);
	if (p_ptr->k_attr)		KILL(p_ptr->k_attr);
	if (p_ptr->k_char)		KILL(p_ptr->k_char);
	if (p_ptr->d_attr)		KILL(p_ptr->d_attr);
	if (p_ptr->d_char)		KILL(p_ptr->d_char);
	if (p_ptr->r_attr)		KILL(p_ptr->r_attr);
	if (p_ptr->r_char)		KILL(p_ptr->r_char);
	if (p_ptr->pr_attr)		KILL(p_ptr->pr_attr);
	if (p_ptr->pr_char)		KILL(p_ptr->pr_char);

	history_wipe(p_ptr->charhist);

	KILL(p_ptr);
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
bool player_birth(int ind, int race, int pclass, int sex, int stat_order[])
{
	player_type *p_ptr = ConnPlayers(ind);
	int i;

	/* Do some consistency checks */
	if (race < 0 || race >= z_info->p_max) race = 0;
	if (pclass < 0 || pclass >= z_info->c_max) pclass = 0;
	if (sex < 0 || sex > 1) sex = 0;

	/* Mark new game */
	p_ptr->new_game = TRUE;

	/* Reprocess his name */
	//if (!process_player_name(Ind, TRUE)) return FALSE;

	/* DM powers? */
	player_admin(p_ptr);

	/* Set info */
	p_ptr->prace = race;
	p_ptr->pclass = pclass;
	p_ptr->male = sex;

	/* Set pointers */
	p_ptr->rp_ptr = &p_info[p_ptr->prace];
	p_ptr->cp_ptr = &c_info[pclass];
	p_ptr->mp_ptr = &c_info[pclass].spells;

	/* Set his ID */
	p_ptr->id = player_id++;

	/* Actually Generate */

	/* This enables maximize mode for new characters. --RLS */

	p_ptr->maximize=1;

	/* Remember when this player was created in terms of the server turn counter */
	p_ptr->birth_turn = turn;

	/* No autoroller */
	get_stats(p_ptr);

	/* Roll for base hitpoints */
	get_extra(p_ptr);

	/* Roll for age/height/weight */
	get_ahw(p_ptr);

	/* Roll for social class */
	get_history(p_ptr);

	/* Roll for gold */
	get_money(p_ptr);

	/* Hack -- grant some Dungeon Master powers */
	if (is_dm_p(p_ptr))
	{
		p_ptr->au = 50000000;
		p_ptr->lev = 50;
		p_ptr->exp = 15000000;
		p_ptr->ghost = 1;
		p_ptr->noscore = 1;
		
		if (p_ptr->dm_flags & DM_INVULNERABLE)
			p_ptr->invuln = -1;
	}

	/* Hack -- outfit the player */
	player_outfit(p_ptr);
	
	/* Hack -- Give him "awareness" of certain objects */
	for (i = 0; i < z_info->k_max; i++) 
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) p_ptr->kind_aware[i] = TRUE;
	}

	/* Set his location, panel, etc. */
	//player_setup(p_ptr);

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
	for (i = 0; i < z_info->r_max; i++)
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
