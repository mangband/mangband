/* File: spells2.c */

/* Purpose: Spell code (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


/* Wipe everything */
void wipe_spell(int Depth, int cy, int cx, int r)
{
	int		yy, xx, dy, dx;

	cave_type	*c_ptr;



	/* Don't hurt town or surrounding areas */
	if (Depth <= 0 ? wild_info[Depth].radius <= 2 : 0)
		return;

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Lose room and vault */
			if (Depth > 0)
            		{
                		c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
            		}
			
			/* Delete monsters */
			delete_monster(Depth, yy, xx);

			/* Destroy "valid" grids */
			if ((cave_valid_bold(Depth, yy, xx)) && !(c_ptr->info & CAVE_ICKY))
			{
				/* Turn into basic floor */
				c_ptr->feat = FEAT_FLOOR;
			
				/* Delete objects */
				delete_object(Depth, yy, xx);
			}

			everyone_lite_spot(Depth, yy, xx);
		}
}


/*
 * Increase players hit points, notice effects, and tell the player about it.
 */
bool hp_player(int Ind, int num)
{
	player_type *p_ptr = Players[Ind];

	// The "number" that the character is displayed as before healing
	int old_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
	int new_num; 


	if (p_ptr->chp < p_ptr->mhp)
	{
		p_ptr->chp += num;

		if (p_ptr->chp > p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Update health bars */
		update_health(0 - Ind);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Figure out of if the player's "number" has changed */
		new_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
		if (new_num >= 7) new_num = 10;

		/* If so then refresh everyone's view of this player */
		if (new_num != old_num)
			everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		num = num / 5;
		if (num < 3)
		{
			if (num == 0)
			{
				msg_print(Ind, "You feel a little better.");
			}
			else
			{
				msg_print(Ind, "You feel better.");
			}
		}
		else
		{
			if (num < 7)
			{
				msg_print(Ind, "You feel much better.");
			}
			else
			{
				msg_print(Ind, "You feel very good.");
			}
		}

		return (TRUE);
	}

	return (FALSE);
}

/*
 * Increase players hit points, notice effects, and don't tell the player it.
 */
bool hp_player_quiet(int Ind, int num)
{
	player_type *p_ptr = Players[Ind];

	// The "number" that the character is displayed as before healing
	int old_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
	int new_num; 


	if (p_ptr->chp < p_ptr->mhp)
	{
		p_ptr->chp += num;

		if (p_ptr->chp > p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Update health bars */
		update_health(0 - Ind);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Figure out of if the player's "number" has changed */
		new_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
		if (new_num >= 7) new_num = 10;

		/* If so then refresh everyone's view of this player */
		if (new_num != old_num)
			everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		return (TRUE);
	}

	return (FALSE);
}




/*
 * Leave a "glyph of warding" which prevents monster movement
 */
bool warding_glyph(int Ind)
{
	player_type *p_ptr = Players[Ind];

	cave_type *c_ptr;

	/* Can't create in town */
	if (level_is_town(p_ptr->dun_depth))
	{
		msg_print(Ind, "The very soil of the Town prevents you.");
		return FALSE;
	}


	/* Require clean space */
	if (!cave_clean_bold(p_ptr->dun_depth, p_ptr->py, p_ptr->px))
	{
		msg_print(Ind, "The object resists the spell.");
		return FALSE;
	}

	/* Access the player grid */
	c_ptr = &cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px];

	/* Don't allow glyphs inside houses */
	if (p_ptr->dun_depth < 0 && c_ptr->info & CAVE_ICKY)
	{
		msg_print(Ind, "The floor of the house resists your spell.");
		return FALSE;
	}

	/* Create a glyph of warding */
	c_ptr->feat = FEAT_GLYPH;

	return TRUE;
}




/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
	"strong",
	"smart",
	"wise",
	"dextrous",
	"healthy",
	"cute"
};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};


/*
 * Lose a "point"
 */
bool do_dec_stat(int Ind, int stat)
{
	player_type *p_ptr = Players[Ind];

	bool sust = FALSE;

	/* Access the "sustain" */
	switch (stat)
	{
		case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
		case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
		case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
		case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
		case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
		case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
	}

	/* Sustain */
	if (sust)
	{
		/* Message */
		msg_format(Ind, "You feel %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (dec_stat(Ind, stat, 10, FALSE))
	{
		/* Message */
		msg_format(Ind, "You feel very %s.", desc_stat_neg[stat]);
		sound(Ind, MSG_DRAIN_STAT);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int Ind, int stat)
{
	/* Attempt to increase */
	if (res_stat(Ind, stat))
	{
		/* Message */
		msg_format(Ind, "You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int Ind, int stat)
{
	bool res;

	/* Restore strength */
	res = res_stat(Ind, stat);

	/* Attempt to increase */
	if (inc_stat(Ind, stat))
	{
		/* Message */
		msg_format(Ind, "Wow!  You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format(Ind, "You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}



/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int                 i;
	object_type        *o_ptr;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (o_ptr->k_idx)
		{
			object_aware(p_ptr, o_ptr);
			object_known(o_ptr);
		}
	}
}






/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] =
{
	0, 10,  50, 100, 200,
	300, 400, 500, 700, 950,
	990, 992, 995, 997, 999,
	1000
};


/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int Ind, int all)
{
	player_type *p_ptr = Players[Ind];

	int		i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
	        u32b f1, f2, f3;

		object_type *o_ptr = &p_ptr->inventory[i];

		/* Uncursed already */
		if (!cursed_p(o_ptr)) continue;

		/* Extract the flags */
    		object_flags(o_ptr, &f1, &f2, &f3);

		/* Heavily Cursed Items need a special spell */
		if (!all && (f3 & TR3_HEAVY_CURSE)) continue;

		/* Perma-Cursed Items can NEVER be uncursed */
		if (f3 & TR3_PERMA_CURSE) continue;

		/* Uncurse it */
		o_ptr->ident &= ~ID_CURSED;

		/* Hack -- Assume felt */
		o_ptr->ident |= ID_SENSE;

		/* Take note */
		o_ptr->note = quark_add("uncursed");

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(int Ind)
{
	return (remove_curse_aux(Ind, FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(int Ind)
{
	return (remove_curse_aux(Ind, TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Restore experience */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Message */
		msg_print(Ind, "You feel your life energies returning.");

		/* Restore the experience */
		p_ptr->exp = p_ptr->max_exp;

		/* Check the experience */
		check_experience(Ind);

		/* Did something */
		return (TRUE);
	}

	/* No effect */
	return (FALSE);
}

/*
 * Obtain the "flags" for the player as if he was an item.
 *
 * Include all spoilery bits.
 *
 * This function works alot like "calc_bonuses()", but in reverse.
 */
void player_flags_spoil(int Ind, u32b *f1, u32b * f2, u32b *f3)
{
	player_type *p_ptr = Players[Ind];

	/* Non-spoiler flags */
	player_flags(Ind, f1, f2, f3);

	/* Spoilers: */
	if (p_ptr->aggravate)	 *f3 |= TR3_AGGRAVATE; 
	if (p_ptr->see_infra)	 *f1 |= TR1_INFRA;
	if (p_ptr->see_inv)	     *f3 |= TR3_SEE_INVIS;
	if (p_ptr->feather_fall) *f3 |= TR3_FEATHER;
	if (p_ptr->free_act)	 *f3 |= TR3_FREE_ACT;
	if (p_ptr->regenerate)	 *f3 |= TR3_REGEN;
	if (p_ptr->slow_digest)	 *f3 |= TR3_SLOW_DIGEST;
	if (p_ptr->telepathy == TR3_TELEPATHY)	 *f3 |= TR3_TELEPATHY;
	if (p_ptr->hold_life)	 *f3 |= TR3_HOLD_LIFE;
	if (p_ptr->lite)	     *f3 |= TR3_LITE;

	if (p_ptr->immune_acid) *f2 |= TR2_IM_ACID;
	if (p_ptr->resist_acid) *f2 |= TR2_RES_ACID;

	if (p_ptr->immune_elec) *f2 |= TR2_IM_ELEC;
	if (p_ptr->resist_elec) *f2 |= TR2_RES_ELEC;

	if (p_ptr->immune_fire) *f2 |= TR2_IM_FIRE;
	if (p_ptr->resist_fire) *f2 |= TR2_RES_FIRE;

	if (p_ptr->immune_cold) *f2 |= TR2_IM_COLD;
	if (p_ptr->resist_cold) *f2 |= TR2_RES_COLD;

	if (p_ptr->resist_pois) *f2 |= TR2_RES_POIS;

	if (p_ptr->resist_lite)	 *f2 |= TR2_RES_LITE;
	if (p_ptr->resist_dark)	 *f2 |= TR2_RES_DARK;
	if (p_ptr->resist_conf)	 *f2 |= TR2_RES_CONFU;
	if (p_ptr->resist_sound) *f2 |= TR2_RES_SOUND;
	if (p_ptr->resist_disen) *f2 |= TR2_RES_DISEN;
	if (p_ptr->resist_chaos) *f2 |= TR2_RES_CHAOS;
	if (p_ptr->resist_shard) *f2 |= TR2_RES_SHARD;
	if (p_ptr->resist_nexus) *f2 |= TR2_RES_NEXUS;
	if (p_ptr->resist_neth)	 *f2 |= TR2_RES_NETHR;
	if (p_ptr->resist_fear)	 *f2 |= TR2_RES_FEAR;
	if (p_ptr->resist_blind) *f2 |= TR2_RES_BLIND;

	if (p_ptr->sustain_str) *f2 |= TR2_SUST_STR;
	if (p_ptr->sustain_int) *f2 |= TR2_SUST_INT;
	if (p_ptr->sustain_wis) *f2 |= TR2_SUST_WIS;
	if (p_ptr->sustain_dex) *f2 |= TR2_SUST_DEX;
	if (p_ptr->sustain_con) *f2 |= TR2_SUST_CON;
	if (p_ptr->sustain_chr) *f2 |= TR2_SUST_CHR;
}
 
/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge(int Ind, bool spoil)
{
	player_type *p_ptr = Players[Ind];

	int		i = 0, k;

	u32b	f1 = 0L, f2 = 0L, f3 = 0L;
	u32b	o1 = 0L, o2 = 0L, o3 = 0L;
	u32b t1, t2, t3;
 
	object_type	*o_ptr;

	int j,e; /* copy hack */
	static cptr s_info[MAX_TXT_INFO];		/* Temp storage of *ID* and Self Knowledge info */
	cptr	*info = s_info;
	


	/* Clear the info area first. */
	memset(s_info,0,sizeof(s_info));

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	/* Acquire item flags from equipment */
	for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
	{
		t1 = t2 = t3 = 0L;
		o_ptr = &p_ptr->inventory[k];

		/* Skip empty items */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
		if (spoil)
			object_flags(o_ptr, &t1, &t2, &t3);
		else
			object_flags_known(Ind, o_ptr, &t1, &t2, &t3);

		/* Extract flags */
		f1 |= t1;
		f2 |= t2;
		f3 |= t3;
	}

	/* Save object-only flags */
	o1 = f1; o2 = f2; o3 = f3;

	/* Acquire player flags */
	t1 = t2 = t3 = 0L;	
	if (!spoil)
		player_flags(Ind, &t1, &t2, &t3);
	else
		player_flags_spoil(Ind, &t1, &t2, &t3);
	/* Combine flags */
	f1 |= t1;
	f2 |= t2;
	f3 |= t3;


	/* Birth options */
	if (option_p(p_ptr, NO_GHOST) && !cfg_ironman)
	{
		info[i++] = "You never compromise.";
	}

	/* Status */
	if (p_ptr->blind)
	{
		info[i++] = "You cannot see.";
	}
	if (p_ptr->confused)
	{
		info[i++] = "You are confused.";
	}
	if (p_ptr->afraid)
	{
		info[i++] = "You are terrified.";
	}
	if (p_ptr->cut)
	{
		info[i++] = "You are bleeding.";
	}
	if (p_ptr->stun)
	{
		info[i++] = "You are stunned.";
	}
	if (p_ptr->poisoned)
	{
		info[i++] = "You are poisoned.";
	}
	if (p_ptr->image)
	{
		info[i++] = "You are hallucinating.";
	}
	if (p_ptr->teleport)
	{
        info[i++] = "You are teleporting.";
	}

	/* Spell effects */
	if (p_ptr->blessed)
	{
        info[i++] = "You feel righteous.";
	}
	if (p_ptr->hero)
	{
        info[i++] = "You feel like a hero.";
	}
	if (p_ptr->shero)
	{
		info[i++] = "You are in a battle rage.";
	}
	if (p_ptr->protevil)
	{
		info[i++] = "You are protected from evil.";
	}
	if (p_ptr->shield)
	{
		info[i++] = "You are protected by a mystic shield.";
	}
	if (p_ptr->invuln)
	{
		info[i++] = "You are temporarily invulnerable.";
	}
	if (p_ptr->confusing)
	{
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->searching)
	{
		info[i++] = "You are looking around very carefully.";
	}
	if (p_ptr->new_spells)
	{
		info[i++] = "You can learn some more spells.";
	}
	if (p_ptr->word_recall)
	{
		info[i++] = "You will soon be recalled.";
	}

	/* Equipment-related */
	if (f3 & TR3_AGGRAVATE)
	{
		info[i++] = "You aggravate monsters.";
	}
	if (f1 & TR1_INFRA)
	{
		info[i++] = "Your eyes are sensitive to infrared light.";
	}
	if (f3 & TR3_SEE_INVIS)
	{
		info[i++] = "You can see invisible creatures.";
	}
	if (f3 & TR3_FEATHER)
	{
		info[i++] = "You land gently.";
	}
	if (f3 & TR3_FREE_ACT)
	{
		info[i++] = "You have free action.";
	}
	if (f3 & TR3_REGEN)
	{
		info[i++] = "You regenerate quickly.";
	}
	if (f3 & TR3_SLOW_DIGEST)
	{
		info[i++] = "Your appetite is small.";
	}
    if (f3 & TR3_TELEPATHY)
	{
		info[i++] = "You have ESP.";
	}
	if (f3 & TR3_HOLD_LIFE)
	{
		info[i++] = "You have a firm hold on your life force.";
	}
	if (f3 & TR3_LITE)
	{
		info[i++] = "You are carrying a permanent light.";
	}
	if (f2 & TR2_IM_ACID)
	{
		info[i++] = "You are completely immune to acid.";
	}
	else if ((f2 & TR2_RES_ACID) && (p_ptr->oppose_acid))
	{
		info[i++] = "You resist acid exceptionally well.";
	}
	else if ((f2 & TR2_RES_ACID) || (p_ptr->oppose_acid))
	{
		info[i++] = "You are resistant to acid.";
	}

	if (f2 & TR2_IM_ELEC)
	{
		info[i++] = "You are completely immune to lightning.";
	}
	else if ((f2 & TR2_RES_ELEC) && (p_ptr->oppose_elec))
	{
		info[i++] = "You resist lightning exceptionally well.";
	}
	else if ((f2 & TR2_RES_ELEC) || (p_ptr->oppose_elec))
	{
		info[i++] = "You are resistant to lightning.";
	}

	if (f2 & TR2_IM_FIRE)
	{
		info[i++] = "You are completely immune to fire.";
	}
	else if ((f2 & TR2_RES_FIRE) && (p_ptr->oppose_fire))
	{
		info[i++] = "You resist fire exceptionally well.";
	}
	else if ((f2 & TR2_RES_FIRE) || (p_ptr->oppose_fire))
	{
		info[i++] = "You are resistant to fire.";
	}

	if (f2 & TR2_IM_COLD)
	{
		info[i++] = "You are completely immune to cold.";
	}
	else if ((f2 & TR2_RES_COLD) && (p_ptr->oppose_cold))
	{
		info[i++] = "You resist cold exceptionally well.";
	}
	else if ((f2 & TR2_RES_COLD) || (p_ptr->oppose_cold))
	{
		info[i++] = "You are resistant to cold.";
	}

	if ((f2 & TR2_RES_POIS) && (p_ptr->oppose_pois))
	{
		info[i++] = "You resist poison exceptionally well.";
	}
	else if ((f2 & TR2_RES_POIS) || (p_ptr->oppose_pois))
	{
		info[i++] = "You are resistant to poison.";
	}

	if (f2 & TR2_RES_LITE)
	{
		info[i++] = "You are resistant to bright light.";
	}
	if (f2 & TR2_RES_DARK)
	{
		info[i++] = "You are resistant to darkness.";
	}
	if (f2 & TR2_RES_CONFU)
	{
		info[i++] = "You are resistant to confusion.";
	}
	if (f2 & TR2_RES_SOUND)
	{
		info[i++] = "You are resistant to sonic attacks.";
	}
	if (f2 & TR2_RES_DISEN)
	{
		info[i++] = "You are resistant to disenchantment.";
	}
	if (f2 & TR2_RES_CHAOS)
	{
		info[i++] = "You are resistant to chaos.";
	}
	if (f2 & TR2_RES_SHARD)
	{
		info[i++] = "You are resistant to blasts of shards.";
	}
	if (f2 & TR2_RES_NEXUS)
	{
		info[i++] = "You are resistant to nexus attacks.";
	}
	if (f2 & TR2_RES_NETHR)
	{
		info[i++] = "You are resistant to nether forces.";
	}
	if (f2 & TR2_RES_FEAR)
	{
		info[i++] = "You are completely fearless.";
	}
	if (f2 & TR2_RES_BLIND)
	{
		info[i++] = "Your eyes are resistant to blindness.";
	}

	if (f2 & TR2_SUST_STR)
	{
		info[i++] = "Your strength is sustained.";
	}
	if (f2 & TR2_SUST_INT)
	{
		info[i++] = "Your intelligence is sustained.";
	}
	if (f2 & TR2_SUST_WIS)
	{
		info[i++] = "Your wisdom is sustained.";
	}
	if (f2 & TR2_SUST_CON)
	{
		info[i++] = "Your constitution is sustained.";
	}
	if (f2 & TR2_SUST_DEX)
	{
		info[i++] = "Your dexterity is sustained.";
	}
	if (f2 & TR2_SUST_CHR)
	{
		info[i++] = "Your charisma is sustained.";
	}

	/* Remove player flags */
	f1 = o1;
	f2 = o2;
	f3 = o3;
	
	/* Equipment itself */
	if (f1 & TR1_STR)
	{
		info[i++] = "Your strength is affected by your equipment.";
	}
	if (f1 & TR1_INT)
	{
		info[i++] = "Your intelligence is affected by your equipment.";
	}
	if (f1 & TR1_WIS)
	{
		info[i++] = "Your wisdom is affected by your equipment.";
	}
	if (f1 & TR1_DEX)
	{
		info[i++] = "Your dexterity is affected by your equipment.";
	}
	if (f1 & TR1_CON)
	{
		info[i++] = "Your constitution is affected by your equipment.";
	}
	if (f1 & TR1_CHR)
	{
		info[i++] = "Your charisma is affected by your equipment.";
	}

	if (f1 & TR1_STEALTH)
	{
		info[i++] = "Your stealth is affected by your equipment.";
	}
	if (f1 & TR1_SEARCH)
	{
		info[i++] = "Your searching ability is affected by your equipment.";
	}
	if (f1 & TR1_INFRA)
	{
		info[i++] = "Your infravision is affected by your equipment.";
	}
	if (f1 & TR1_TUNNEL)
	{
		info[i++] = "Your digging ability is affected by your equipment.";
	}
	if (f1 & TR1_SPEED)
	{
		info[i++] = "Your speed is affected by your equipment.";
	}
	if (f1 & TR1_BLOWS)
	{
		info[i++] = "Your attack speed is affected by your equipment.";
	}


	/* Access the current weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Analyze the weapon */
	if (o_ptr->k_idx)
	{
		/* Indicate Blessing */
		if (f3 & TR3_BLESSED)
		{
			info[i++] = "Your weapon has been blessed by the gods.";
		}

		/* Hack */
		if (f3 & TR3_IMPACT)
		{
			info[i++] = "The impact of your weapon can cause earthquakes.";
		}

		/* Special "Attack Bonuses" */
		if (f1 & TR1_BRAND_ACID)
		{
			info[i++] = "Your weapon melts your foes.";
		}
		if (f1 & TR1_BRAND_ELEC)
		{
			info[i++] = "Your weapon shocks your foes.";
		}
		if (f1 & TR1_BRAND_FIRE)
		{
			info[i++] = "Your weapon burns your foes.";
		}
		if (f1 & TR1_BRAND_COLD)
		{
			info[i++] = "Your weapon freezes your foes.";
		}
        if (f1 & TR1_BRAND_POIS)
        {
            info[i++] = "Your weapon poisons your foes.";
        }

		/* Special "slay" flags */
		if (f1 & TR1_SLAY_ANIMAL)
		{
			info[i++] = "Your weapon strikes at animals with extra force.";
		}
		if (f1 & TR1_SLAY_EVIL)
		{
			info[i++] = "Your weapon strikes at evil with extra force.";
		}
		if (f1 & TR1_SLAY_UNDEAD)
		{
			info[i++] = "Your weapon strikes at undead with holy wrath.";
		}
		if (f1 & TR1_SLAY_DEMON)
		{
			info[i++] = "Your weapon strikes at demons with holy wrath.";
		}
		if (f1 & TR1_SLAY_ORC)
		{
			info[i++] = "Your weapon is especially deadly against orcs.";
		}
		if (f1 & TR1_SLAY_TROLL)
		{
			info[i++] = "Your weapon is especially deadly against trolls.";
		}
		if (f1 & TR1_SLAY_GIANT)
		{
			info[i++] = "Your weapon is especially deadly against giants.";
		}
		if (f1 & TR1_SLAY_DRAGON)
		{
			info[i++] = "Your weapon is especially deadly against dragons.";
		}

		/* Special "kill" flags */
		if (f1 & TR1_KILL_DRAGON)
		{
			info[i++] = "Your weapon is a great bane of dragons.";
		}
        if (f1 & TR1_KILL_DEMON)
        {
            info[i++] = "Your weapon is a great bane of demons.";
        }
        if (f1 & TR1_KILL_UNDEAD)
        {
            info[i++] = "Your weapon is a great bane of undead.";
        }
	}

	/* HACK !!! --- Copy to rem_info */
	for (k = 0; k < i; k++)
	{
		e = strlen(info[k]);
		for (j = 0; j < e; j++)
		{
			p_ptr->info[k][j].c = info[k][j];
			p_ptr->info[k][j].a = TERM_WHITE;
		}
		for (j = e; j < 80; j++)
		{
			p_ptr->info[k][j].c = ' ';
			p_ptr->info[k][j].a = TERM_WHITE;
		}
	}
	/* Last line */
	p_ptr->last_info_line = i - 1;

	/* Let the client see it */
	send_prepared_popup(Ind, "Self-Knowledge");
}






/*
 * Forget everything
 */
bool lose_all_info(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int                 i;

	/* Forget info about objects */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-items */
		if (!o_ptr->k_idx) continue;

		/* Allow "protection" by the MENTAL flag */
		if (o_ptr->ident & ID_MENTAL) continue;

		/* Remove "default inscriptions" */
		if (o_ptr->note && (o_ptr->ident & ID_SENSE))
		{
			/* Access the inscription */
			cptr q = quark_str(o_ptr->note);

			/* Hack -- Remove auto-inscriptions */
			if ((streq(q, "cursed")) ||
			    (streq(q, "broken")) ||
			    (streq(q, "good")) ||
			    (streq(q, "average")) ||
			    (streq(q, "excellent")) ||
			    (streq(q, "worthless")) ||
			    (streq(q, "special")) ||
			    (streq(q, "terrible")))
			{
				/* Forget the inscription */
				o_ptr->note = 0;
			}
		}

		/* Hack -- Clear the "empty" flag */
		o_ptr->ident &= ~ID_EMPTY;

		/* Hack -- Clear the "known" flag */
		o_ptr->ident &= ~ID_KNOWN;

		/* Hack -- Clear the "felt" flag */
		o_ptr->ident &= ~ID_SENSE;
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Mega-Hack -- Forget the map */
	wiz_dark(Ind);

	/* It worked */
	return (TRUE);
}

/*
 * Set word of recall as appropriate
 */
void set_recall(int Ind, object_type * o_ptr)
{
	int recall_depth = 0;
	player_type * p_ptr = Players[Ind];
	char * inscription = (char *) quark_str(o_ptr->note);

	/* Ironmen don't recall unless they've won */
	if (cfg_ironman && !p_ptr->total_winner)
	{
		msg_print(Ind,"Nothing happens.");
		return;
	}
	
	/* Activate recall */
	if (!p_ptr->word_recall)
	{
		/* default to the players maximum depth */
		p_ptr->recall_depth = p_ptr->max_dlv;
	
		/* scan the inscription for @R */
		if(inscription)
		{
			while (*inscription != '\0')
			{
				if (*inscription == '@')
				{
					inscription++;
					if (*inscription == 'R')
					{			
						/* a valid @R has been located */
						inscription++;
						/* convert the inscription into a level index */
						recall_depth = atoi(inscription);
						if(!(recall_depth % 50)) 
						{
							recall_depth/=50;
						}
					}
				}
				inscription++;
			}
		}
	
		/* do some bounds checking / sanity checks */
		if((recall_depth > p_ptr->max_dlv) || (!recall_depth)) recall_depth = p_ptr->max_dlv;
	
		/* if a wilderness level, verify that the player has visited here before */
		if (recall_depth < 0)
		{
			/* if the player has not visited here, set the recall depth to the town */
			if (!is_dm_p(p_ptr))
				if (!(p_ptr->wild_map[-recall_depth/8] & (1 << -recall_depth%8)))
					recall_depth = 1;

		}
	
		p_ptr->recall_depth = recall_depth;
		p_ptr->word_recall = (s16b)rand_int(20) + 15;
		msg_print(Ind, "The air about you becomes charged...");
		msg_format_complex_near(Ind, Ind, MSG_PY_MISC, "The air about %s becomes charged...", p_ptr->name); 
	}
	else
	{
		p_ptr->word_recall = 0;
		msg_print(Ind, "A tension leaves the air around you...");
		msg_format_complex_near(Ind, Ind, MSG_PY_MISC, "A tension leaves the air around %s...", p_ptr->name);
	}	

}



/*** Detection spells ***/

/*
 * Useful constants for the area around the player to detect.
 * This is instead of using circular detection spells.
 */
#define DETECT_DIST_X	52	/* Detect 52 grids to the left & right */
#define DETECT_DIST_Y	23	/* Detect 23 grids to the top & bottom */



/*
 * Map an area around the player.
 *
 * We must never attempt to map the outer dungeon walls, or we
 * might induce illegal cave grid references.
 */
void map_area(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	int		i, x, y, y1, y2, x1, x2;

	cave_type	*c_ptr;
	byte		*w_ptr;


	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	/* Speed -- shrink to fit legal bounds */
	if (y1 < 1) y1 = 1;
	if (y2 > p_ptr->cur_hgt-2) y2 = p_ptr->cur_hgt-2;
	if (x1 < 1) x1 = 1;
	if (x2 > p_ptr->cur_wid-2) x2 = p_ptr->cur_wid-2;

	/* Scan that area */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			c_ptr = &cave[Depth][y][x];
			w_ptr = &p_ptr->cave_flag[y][x];

			/* All non-walls are "checked", including MAngband specifics */
			if ((c_ptr->feat < FEAT_SECRET) || 
				((c_ptr->feat >= FEAT_DIRT) && (c_ptr->feat < FEAT_DRAWBRIDGE)))
			{
				/* Memorize normal features */
				if (!is_boring(c_ptr->feat))
				{
					/* Memorize the object */
					*w_ptr |= CAVE_MARK;
				}

				/* Memorize known walls */
				for (i = 0; i < 8; i++)
				{
					c_ptr = &cave[Depth][y+ddy_ddd[i]][x+ddx_ddd[i]];
					w_ptr = &p_ptr->cave_flag[y+ddy_ddd[i]][x+ddx_ddd[i]];

					/* Memorize walls (etc) */
					if ((c_ptr->feat >= FEAT_SECRET) && 
						((c_ptr->feat < FEAT_DIRT) || (c_ptr->feat >= FEAT_DRAWBRIDGE)))
					{
						/* Memorize the walls */
						*w_ptr |= CAVE_MARK;
					}
				}
			}
		}
	}

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * Detect any treasure on the current panel		-RAK-
 *
 * We do not yet create any "hidden gold" features XXX XXX XXX
 */
bool detect_treasure(int Ind)
{
	player_type *p_ptr = Players[Ind];
	
	int Depth = p_ptr->dun_depth;
	int x1, x2, y1, y2;
	
	int		y, x;
	bool	detect = FALSE;

	cave_type	*c_ptr;
	byte		*w_ptr;

	object_type	*o_ptr;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	

	
	/* Scan the current panel */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			c_ptr = &cave[Depth][y][x];
			w_ptr = &p_ptr->cave_flag[y][x];

			o_ptr = &o_list[c_ptr->o_idx];

			/* Magma/Quartz + Known Gold */
			if ((c_ptr->feat == FEAT_MAGMA_K) ||
			    (c_ptr->feat == FEAT_QUARTZ_K))
			{
				/* Notice detected gold */
				if (!(*w_ptr & CAVE_MARK))
				{
					/* Detect */
					detect = TRUE;

					/* Hack -- memorize the feature */
					*w_ptr |= CAVE_MARK;

					/* Redraw */
					lite_spot(Ind, y, x);
				}
			}

			/* Notice embedded gold */
			if ((c_ptr->feat == FEAT_MAGMA_H) ||
			    (c_ptr->feat == FEAT_QUARTZ_H))
			{
				/* Expose the gold */
				c_ptr->feat += 0x02;

				/* Detect */
				detect = TRUE;

				/* Hack -- memorize the item */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, y, x);
			}
#if 0
			/* Notice gold */
			if (o_ptr->tval == TV_GOLD)
			{
				/* Notice new items */
				if (!(p_ptr->obj_vis[c_ptr->o_idx]))
				{
					/* Detect */
					detect = TRUE;

					/* Hack -- memorize the item */
					p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

					/* Redraw */
					lite_spot(Ind, y, x);
				}
			}
#endif
		}
	}
	
	/* Describe */
	if (detect)
	{
		party_msg_format_near(Ind, MSG_PY_MISC, "%s senses the presence of buried treasure!", p_ptr->name);
		msg_print(Ind, "You sense the presence of buried treasure!");
	}

	return (detect);
}

/*
 * Detect magic items.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 */
bool detect_objects_magic(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;
	int x1, x2, y1, y2;

	int		i, j, tv;
	bool	detect = FALSE;

	cave_type	*c_ptr;
	object_type	*o_ptr;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	
	/* Scan the current panel */
	for (i = y1; i <= y2; i++)
	{
		for (j = x1; j <= x2; j++)
		{
			/* Access the grid and object */
			c_ptr = &cave[Depth][i][j];
			o_ptr = &o_list[c_ptr->o_idx];

			/* Nothing there */
			if (!(c_ptr->o_idx)) continue;

			/* Examine the tval */
			tv = o_ptr->tval;

			/* Artifacts, misc magic items, or enchanted wearables */
			if (artifact_p(o_ptr) || ego_item_p(o_ptr) ||
			    (tv == TV_AMULET) || (tv == TV_RING) ||
			    (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
			    (tv == TV_SCROLL) || (tv == TV_POTION) ||
			    (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
			    ((o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0)))
			{
				/* Note new items */
				if (!(p_ptr->obj_vis[c_ptr->o_idx]))
				{
					/* Detect */
					detect = TRUE;

					/* Memorize the item */
					p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

					/* Redraw */
					lite_spot(Ind, i, j);
				}
			}
		}
	}

	/* Return result */
	return (detect);
}



/*
 * Increment magical detection counter for a monster/player
 */
void give_detect(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];
	int power, i;

	/* The detection counter is expressed in "effect turns"
	 * (each tick gives 1 poision damage) */
	/* Note, that we should probably use caster level and not
	 * victim level, but, oh well. */
	power = 2 + ((p_ptr->lev + 2) / 5);

	/* Also, let's scale down when spamming */
	i = (m_idx < 0 ? p_ptr->play_det[0 - m_idx] : p_ptr->mon_det[m_idx]);
	power = i ? 1 : power;

	/* Players */
	if (m_idx < 0)
	{
		m_idx = 0 - m_idx;
		p_ptr->play_det[m_idx] = MIN(
		    (int)p_ptr->play_det[m_idx] + power, 255);
	}
	/* Monsters */
	else
	{
		p_ptr->mon_det[m_idx] = MIN(
		    (int)p_ptr->mon_det[m_idx] + power, 255);
	}
}

/*
 * Locates and displays all invisible creatures on current panel -RAK-
 */
bool detect_invisible(int Ind, bool pause)
{
	player_type *p_ptr = Players[Ind];
	int x1, x2, y1, y2;

	int		i;
	bool	flag = FALSE;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;
	
	
	/* Detect all invisible monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];
		monster_lore *l_ptr = p_ptr->l_list + m_ptr->r_idx;

		int fy = m_ptr->fy;
		int fx = m_ptr->fx;
		
		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Only detect nearby monsters */
		if (fx < x1 || fy < y1 || fx > x2 || fy > y2) continue;

		/* Detect all invisible monsters */
		if (r_ptr->flags2 & (RF2_INVISIBLE))
		{
			/* Increment detection counter */
			give_detect(Ind, i);

			/* Skip visible monsters */
			if (p_ptr->mon_vis[i]) continue;

			/* Take note that they are invisible */
			l_ptr->flags2 |= RF2_INVISIBLE;
			
			/* Track this fact */
			if (p_ptr->monster_race_idx == m_ptr->r_idx) 
				p_ptr->window |= PW_MONSTER;

			flag = TRUE;
		}
	}

	/* Detect all invisible players */
	for (i = 1; i < NumPlayers + 1; i++)
	{
		player_type *q_ptr = Players[i];

		int py = q_ptr->py;
		int px = q_ptr->px;

		/* Skip ourself */
		if (i == Ind) continue;

		/* Skip players not on this depth */
		if (p_ptr->dun_depth != q_ptr->dun_depth) continue;

		/* Only detect nearby players */
		if (px < x1 || py < y1 || px > x2 || py > y2) continue;

		/* Skip the dungeon master */
		if (q_ptr->dm_flags & DM_SECRET_PRESENCE) continue;

		/* Detect all invisible players (except dungeon master) */
		if (q_ptr->ghost)
		{
			/* Increment detection counter */
			give_detect(Ind, 0 - i);

			/* Skip visible players */
			if (p_ptr->play_vis[0 - i]) continue;

			/* Trigger detect effects */
			flag = TRUE;
		}
	}

	/* Describe result, and clean up */
	if (flag && pause)
	{
		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
		/* Handle Window stuff */
		handle_stuff(Ind);
	
		/* Describe, and wait for acknowledgement */
		party_msg_format_near(Ind, MSG_PY_MISC, "%s senses the presence of invisible creatures!", p_ptr->name);
		msg_print(Ind, "You sense the presence of invisible creatures!");
		msg_print(Ind, NULL);

		/* Hack -- Pause */
		if (option_p(p_ptr, PAUSE_AFTER_DETECT))
			Send_pause(p_ptr);
	}

	/* Result */
	return (flag);
}



/*
 * Display evil creatures on current panel		-RAK-
 */
bool detect_evil(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int	x1, x2, y1, y2;

	int		i;
	bool	flag = FALSE;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	
	/* Display all the evil monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];
		monster_lore *l_ptr = p_ptr->l_list + m_ptr->r_idx;
		
		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Only detect nearby monsters */
		if (fx < x1 || fy < y1 || fx > x2 || fy > y2) continue;
		
		/* Detect evil monsters */
		if (r_ptr->flags3 & (RF3_EVIL))
		{
			/* Take note that they are evil */
			l_ptr->flags3 |= RF3_EVIL;

			/* Track this fact */
			if (p_ptr->monster_race_idx == m_ptr->r_idx) 
				p_ptr->window |= PW_MONSTER;

			/* Increment detection counter */
			give_detect(Ind, i);

			/* Trigger detect effects */
			flag = TRUE;
		}
	}

	/* Note effects and clean up */
	if (flag)
	{
		/* Mega-Hack -- Fix the monsters */
		update_monsters(FALSE);
		/* Handle Window stuff */
		handle_stuff(Ind);

		/* Describe, and wait for acknowledgement */
		party_msg_format_near(Ind, MSG_PY_MISC, "%s senses the presence of evil creatures!", p_ptr->name);
		msg_print(Ind, "You sense the presence of evil creatures!");
		msg_print(Ind, NULL);

		/* Hack -- Pause */
		if (option_p(p_ptr, PAUSE_AFTER_DETECT))
			Send_pause(p_ptr);
	}

	/* Result */
	return (flag);
}



/*
 * Display all non-invisible monsters/players on the current panel
 */
bool detect_creatures(int Ind, bool pause)
{
	player_type *p_ptr = Players[Ind];
	int	x1, x2, y1, y2;

	int		i;
	bool	flag = FALSE;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	
	/* Detect non-invisible monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Only detect nearby monsters */
		if (fx < x1 || fy < y1 || fx > x2 || fy > y2) continue;
		
		/* Detect all non-invisible monsters */
		if (!(r_ptr->flags2 & (RF2_INVISIBLE)))
		{
			/* Increment detection counter */
			give_detect(Ind, i);

			/* Skip visible monsters */
			if (p_ptr->mon_vis[i]) continue;

			flag = TRUE;
		}
	}

	/* Detect non-invisible players */
	for (i = 1; i < NumPlayers + 1; i++)
	{
		player_type *q_ptr = Players[i];

		int py = q_ptr->py;
		int px = q_ptr->px;

		/* Skip ourself */
		if (i == Ind) continue;

		/* Skip players not on this depth */
		if (p_ptr->dun_depth != q_ptr->dun_depth) continue;

		/* Only detect nearby players */
		if (px < x1 || py < y1 || px > x2 || py > y2) continue;
		
		/* Detect all non-invisible players */
		if (!q_ptr->ghost)
		{
			/* Increment detection counter */
			give_detect(Ind, 0 - i);

			/* Skip visible players */
			if (p_ptr->play_vis[i]) continue;

			/* Trigger detect effects */
			flag = TRUE;
		}
	}

	/* Describe and clean up */
	if (flag && pause)
	{
		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
		/* Handle Window stuff */
		handle_stuff(Ind);

		/* Describe, and wait for acknowledgement */
		party_msg_format_near(Ind, MSG_PY_MISC, "%s senses the presence of creatures!", p_ptr->name);
		msg_print(Ind, "You sense the presence of creatures!");
		msg_print(Ind, NULL);

		/* Hack -- Pause */
		if (option_p(p_ptr, PAUSE_AFTER_DETECT))
			Send_pause(p_ptr);
	}

	/* Result */
	return (flag);
}


/*
 * Detect everything
 */
bool detection(int Ind)
{
	bool	detect = FALSE;
	bool	detected_invis, detected_creatures = FALSE;

	/* Detect the easy things */
	if (detect_treasure(Ind)) detect = TRUE;
	if (detect_objects_normal(Ind)) detect = TRUE;
	if (detect_trap(Ind)) detect = TRUE;
	if (detect_sdoor(Ind)) detect = TRUE;
	detected_creatures = detect_creatures(Ind, FALSE);
	detected_invis = detect_invisible(Ind, FALSE);
		
	/* Describe result, and clean up */
	if (detected_creatures || detected_invis)
	{
		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
		/* Handle Window stuff */
		handle_stuff(Ind);

		detect = TRUE;
		/* Describe, and wait for acknowledgement */
		party_msg_format_near(Ind, MSG_PY_MISC, "%s senses the presence of creatures!", Players[Ind]->name);
		msg_print(Ind, "You sense the presence of creatures!");
		msg_print(Ind, NULL);

		/* Hack -- Pause */
		if (option_p(Players[Ind], PAUSE_AFTER_DETECT))
			Send_pause(Players[Ind]);
	}

	/* Result */
	return (detect);
}


/*
 * Detect all objects on the current panel		-RAK-
 */
bool detect_objects_normal(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;
	int	x1, x2, y1, y2;
	
	int		i, j;
	bool	detect = FALSE;

	cave_type	*c_ptr;

	object_type	*o_ptr;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	
	/* Scan the current panel */
	for (i = y1; i <= y2; i++)
	{
		for (j = x1; j <= x2; j++)
		{
			c_ptr = &cave[Depth][i][j];

			o_ptr = &o_list[c_ptr->o_idx];

			/* Nothing here */
			if (!(c_ptr->o_idx)) continue;

			/* Do not detect "gold" */
			if (o_ptr->tval == TV_GOLD) continue;

			/* Note new objects */
			if (!(p_ptr->obj_vis[c_ptr->o_idx]))
			{
				/* Detect */
				detect = TRUE;

				/* Hack -- memorize it */
				p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

				/* Redraw */
				lite_spot(Ind, i, j);
			}
		}
	}

	if (detect) p_ptr->window |= (PW_ITEMLIST);

	return (detect);
}


/*
 * Locates and displays traps on current panel
 */
bool detect_trap(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;
	int x1, x2, y1, y2;
	
	int		i, j;
	bool	detect = FALSE;

	cave_type  *c_ptr;
	byte *w_ptr;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	/* Scan the current panel */
	for (i = y1; i <= y2; i++)
	{
		for (j = x1; j <= x2; j++)
		{
			/* Access the grid */
			c_ptr = &cave[Depth][i][j];
			w_ptr = &p_ptr->cave_flag[i][j];

			/* Detect invisible traps */
			if (c_ptr->feat == FEAT_INVIS)
			{
				/* Pick a trap */
				pick_trap(Depth, i, j);

				/* Hack -- memorize it */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}

			/* Already seen traps */
			else if (c_ptr->feat >= FEAT_TRAP_HEAD && c_ptr->feat <= FEAT_TRAP_TAIL)
			{
				/* Memorize it */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	return (detect);
}



/*
 * Locates and displays all stairs and secret doors on current panel -RAK-
 */
bool detect_sdoor(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;
	int x1, x2, y1, y2;

	int		i, j;
	bool	detect = FALSE;

	cave_type *c_ptr;
	byte *w_ptr;

	/* Pick an area to map */
	y1 = p_ptr->py - DETECT_DIST_Y;
	y2 = p_ptr->py + DETECT_DIST_Y;
	x1 = p_ptr->px - DETECT_DIST_X;
	x2 = p_ptr->px + DETECT_DIST_X;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > p_ptr->cur_hgt-1) y2 = p_ptr->cur_hgt-1;
	if (x2 > p_ptr->cur_wid-1) x2 = p_ptr->cur_wid-1;	
	
	
	/* Scan the panel */
	for (i = y1; i <= y2; i++)
	{
		for (j = x1; j <= x2; j++)
		{
			/* Access the grid and object */
			c_ptr = &cave[Depth][i][j];
			w_ptr = &p_ptr->cave_flag[i][j];

			/* Hack -- detect secret doors */
			if (c_ptr->feat == FEAT_SECRET)
			{
				/* Find the door XXX XXX XXX */
				c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

				/* Memorize the door */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
			else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			     (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			    ((c_ptr->feat == FEAT_OPEN) ||
			     (c_ptr->feat == FEAT_BROKEN)))
			{ /* Deal with non-hidden doors as well */
				/* Memorize the door */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}

			/* Ignore known grids */
			if (*w_ptr & CAVE_MARK) continue;

			/* Hack -- detect stairs */
			if ((c_ptr->feat == FEAT_LESS) ||
			    (c_ptr->feat == FEAT_MORE))
			{
				/* Memorize the stairs */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	return (detect);
}


/*
 * Create stairs at the player location
 */
void stair_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	/* Access the grid */
	cave_type *c_ptr;

	if(Depth <= 0 ) { return;};

	/* Access the player grid */
	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];

	/* forbid perma-grids
	 * forbid grids containing artifacts
	 * forbid house doors
	 */
	if (!cave_valid_bold(Depth, p_ptr->py, p_ptr->px))
	{
		msg_print(Ind, "The object resists the spell.");
		return;
	}

	/* Hack -- Delete old contents */
	delete_object(Depth, p_ptr->py, p_ptr->px);

	/* Create a staircase */
	if (!Depth) /* Should never happen, would be in town */
	{
		c_ptr->feat = FEAT_MORE;
	}
	else if (is_quest(Depth) || (Depth >= MAX_DEPTH-1))
	{
		c_ptr->feat = FEAT_LESS;
	}
	else if (rand_int(100) < 50)
	{
		c_ptr->feat = FEAT_MORE;
	}
	else
	{
		c_ptr->feat = FEAT_LESS;
	}

	/* Notice */
	note_spot(Ind, p_ptr->py, p_ptr->px);

	/* Redraw */
	everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);
}




/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Curse the players armor
 */
bool curse_armor(int Ind)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;

	char o_name[80];


	/* Curse the body armor */
	o_ptr = &p_ptr->inventory[INVEN_BODY];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Attempt a saving throw for artifacts */
	if (artifact_p(o_ptr) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format(Ind, "A %s tries to %s, but your %s resists the effects!",
		           "terrible black aura", "surround your armor", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format(Ind, "A terrible black aura blasts your %s!", o_name);

		/* Hack -- preserve artifact */
		if (artifact_p(o_ptr))	a_info[o_ptr->name1].cur_num = 0;

		/* Blast the armor */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_BLASTED;
	o_ptr->name3 = 0;
		o_ptr->to_a = 0 - randint(5) - randint(5);
		o_ptr->to_h = 0;
		o_ptr->to_d = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;

		/* Curse it */
		o_ptr->ident |= ID_CURSED;

		/* Break it */
		o_ptr->ident |= ID_BROKEN;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	return (TRUE);
}


/*
 * Curse the players weapon
 */
bool curse_weapon(int Ind)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;

	char o_name[80];


	/* Curse the weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Attempt a saving throw */
	if (artifact_p(o_ptr) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format(Ind, "A %s tries to %s, but your %s resists the effects!",
		           "terrible black aura", "surround your weapon", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format(Ind, "A terrible black aura blasts your %s!", o_name);

		/* Hack -- preserve artifact */
		if (artifact_p(o_ptr))	a_info[o_ptr->name1].cur_num = 0;

		/* Shatter the weapon */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_SHATTERED;
	o_ptr->name3 = 0;
		o_ptr->to_h = 0 - randint(5) - randint(5);
		o_ptr->to_d = 0 - randint(5) - randint(5);
		o_ptr->to_a = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;

		/* Curse it */
		o_ptr->ident |= ID_CURSED;

		/* Break it */
		o_ptr->ident |= ID_BROKEN;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	/* Notice */
	return (TRUE);
}


/*
 * Brand weapons (or ammo)
 *
 * Turns the (non-magical) object into an ego-item of 'brand_type'.
 */
void brand_object(int Ind, object_type *o_ptr, byte brand_type)
{
	player_type *p_ptr = Players[Ind];
	/* you can never modify artifacts / ego-items */
	/* you can never modify broken / cursed items */
	if ((o_ptr->k_idx) &&
	    (!artifact_p(o_ptr)) && (!ego_item_p(o_ptr)) &&
	    (!broken_p(o_ptr)) && (!cursed_p(o_ptr)))
	{
		cptr act = "magical";
		char o_name[80];

		switch (brand_type)
		{
			case EGO_BRAND_FIRE:
			case EGO_FLAME:
				act = "fiery";
				break;
			case EGO_BRAND_COLD:
			case EGO_FROST:
				act = "frosty";
				break;
			case EGO_BRAND_POIS:
			case EGO_AMMO_VENOM:
				act = "sickly";
				break;
		}

		 object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 0);

		/* Describe */
		msg_format(Ind, "A %s aura surrounds the %s.", act, o_name);

		/* Brand the object */
		o_ptr->name2 = brand_type;

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Enchant */
		enchant(Ind, o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);

	}
	else
	{
		/*if (flush_failure) flush();*/
		msg_print(Ind, "The Branding failed.");
	}
}

/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(int Ind, object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * Brand chosen ammo
 */
void brand_ammo(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];
	object_type *o_ptr;
	int r;
	byte brand_type;

	/* Only accept ammo */
	/* item_tester_hook = item_tester_hook_ammo; */

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - item];
		p_ptr->redraw |= (PR_FLOOR);
	}

	if (!item_tester_hook_ammo(Ind, o_ptr)) {
			msg_print(Ind, "You cannot brand that!");
			return;
	}

	r = rand_int(100);

	/* Select the brand */
	if (r < 33)
		brand_type = EGO_FLAME;
	else if (r < 67)
		brand_type = EGO_FROST;
	else
		brand_type = EGO_AMMO_VENOM;

	/* Brand the ammo */
	brand_object(Ind, o_ptr, brand_type);

	/* Done */
	return;
}

/*
 * Brand the current weapon
 */
void brand_weapon(int Ind)
{
    player_type *p_ptr = Players[Ind];

    object_type *o_ptr;

    byte brand_type;
    
    o_ptr = &p_ptr->inventory[INVEN_WIELD];
    
    /* Select a brand */
	 if (rand_int(100) < 25)
		brand_type = EGO_BRAND_FIRE;
	 else
		brand_type = EGO_BRAND_COLD;

	 brand_object(Ind, o_ptr, brand_type);
}





/*
 * Enchants a plus onto an item.                        -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(int Ind, object_type *o_ptr, int n, int eflag)
{
	player_type *p_ptr = Players[Ind];

	int i, chance, prob;

	bool res = FALSE;

	bool a = artifact_p(o_ptr);

    u32b f1, f2, f3;

    /* Magic ammo are always +0 +0 */
    if (((o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
	(o_ptr->tval == TV_BOLT)) && (o_ptr->sval == SV_AMMO_MAGIC))
	return FALSE;

    /* Artifact ammo cannot be enchanted */
    if (((o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
	(o_ptr->tval == TV_BOLT)) && a)
	return FALSE;

	/* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3);


	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	    (o_ptr->tval == TV_ARROW) ||
	    (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i=0; i<n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if (rand_int(prob) >= 100) continue;

		/* Enchant to hit */
		if (eflag & ENCH_TOHIT)
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_h++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}

		/* Enchant to damage */
		if (eflag & ENCH_TODAM)
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_d++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}

		/* Enchant to armor class */
		if (eflag & ENCH_TOAC)
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_a++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Resend plusses */
	p_ptr->redraw |= (PR_PLUSSES);

	/* Success */
	return (TRUE);
}

void spell_clear(int Ind)
{
  player_type *p_ptr = Players[Ind];

  /* Hack */  
  p_ptr->current_spell = -1;
  p_ptr->current_object = -1;
}

bool create_artifact(int Ind)
{
  int item;

  if (!get_item(Ind, &item, ITEM_ANY)) return FALSE;
  
  create_artifact_aux(Ind, item);  
  
  return TRUE;
}


bool create_artifact_aux(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
#if defined(RANDART)
	char o_name[80]; /* Only used by randart() */
#endif
	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
			if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
		p_ptr->redraw |= (PR_FLOOR);
	}
#if !defined(RANDART)
	/* Cheap hack: maximum depth , playerlevel, etc */ 
	apply_magic(127, o_ptr, 75, TRUE, TRUE, TRUE);
#else
	if (o_ptr->number > 1) return FALSE;
	if (artifact_p(o_ptr)) return FALSE;
	
	/* Description */
	object_desc(Ind, o_name, o_ptr, FALSE, 0);

	/* Describe */
	msg_format(Ind, "%s %s glow%s brightly!",
		((item >= 0) ? "Your" : "The"), o_name,
		((o_ptr->number > 1) ? "" : "s"));

	o_ptr->name1 = ART_RANDART;

	/* Piece together a 32-bit random seed */
	o_ptr->name3 = rand_int(0xFFFF) << 16;
	o_ptr->name3 += rand_int(0xFFFF);
	/* Check the tval is allowed */
	if (randart_make(o_ptr) == NULL)
	{
		/* If not, wipe seed. No randart today */
		o_ptr->name1 = 0;
		o_ptr->name3 = 0L;

		return FALSE;
	}

	apply_magic(p_ptr->dun_depth, o_ptr, p_ptr->lev, FALSE, FALSE, FALSE);

	/* Remove all inscriptions */
        if (o_ptr->note)
        {
		/* Forget the inscription */
                o_ptr->note = 0;
        }
#endif
	/* Clear flags */
	o_ptr->ident &= ~ID_KNOWN;
	o_ptr->ident &= ~ID_SENSE;
	o_ptr->ident &= ~ID_MENTAL;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
/*
	p_ptr->current_artifact = FALSE;
*/
	return TRUE;
}


bool enchant_spell(int Ind, int num_hit, int num_dam, int num_ac)
{
	int item;

	if (!get_item(Ind, &item, (num_ac ? item_test(ARMOR) : item_test(WEAPON)))) return (FALSE);

	enchant_spell_aux(Ind, item, num_hit, num_dam, num_ac);

	return (TRUE);
}
	
/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell_aux(int Ind, int item, int num_hit, int num_dam, int num_ac)
{
	player_type *p_ptr = Players[Ind];

	bool		okay = FALSE;

	object_type		*o_ptr;

	char		o_name[80];


	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Enchant armor if requested */
	if (num_ac) item_tester_hook = item_tester_hook_armour;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
		p_ptr->redraw |= (PR_FLOOR);
	}


	if (!item_tester_hook(o_ptr))
	{
		msg_print(Ind, "Sorry, you cannot enchant that item.");
		return (FALSE);
	}

	/* Description */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 0);

	/* Describe */
	msg_format(Ind, "%s %s glow%s brightly!",
	           ((item >= 0) ? "Your" : "The"), o_name,
	           ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(Ind, o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(Ind, o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(Ind, o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		/*if (flush_failure) flush();*/

		/* Message */
		msg_print(Ind, "The enchantment failed.");
	}

	/* Something happened */
	return (TRUE);
}



bool ident_spell(int Ind)
{
	int item;

	if (!get_item(Ind, &item, ITEM_ANY)) return FALSE;
	
	ident_spell_aux(Ind, item);

	return TRUE;
}

/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_aux(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type		*o_ptr;

	char		o_name[80];


	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
		p_ptr->redraw |= (PR_FLOOR);
	}


	/* Identify it fully */
	object_aware(p_ptr, o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Description */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Possibly play a sound depending on object quality. */
	if (cursed_p(o_ptr) || broken_p(o_ptr))
	{
		/* This is a bad item. */
		sound(Ind, MSG_IDENT_BAD);
	}
	else if (artifact_p(o_ptr))
	{
		/* We have a good artifact. */
		sound(Ind, MSG_IDENT_ART);
	}
	else if (ego_item_p(o_ptr))
	{
		/* We have a good ego item. */
		sound(Ind, MSG_IDENT_EGO);
	}

	/* Notice artifacts */
	if (artifact_p(o_ptr))
	{
		artifact_notify(p_ptr, o_ptr);
	}

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format(Ind, "%^s: %s (%c).",
		           describe_use(Ind, item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format(Ind, "In your pack: %s (%c).",
		           o_name, index_to_label(item));
	}
	else
	{
		msg_format(Ind, "On the ground: %s.",
		           o_name);
	}

	/* Something happened */
	return (TRUE);
}


bool identify_fully(int Ind)
{
	int item;

	if (!get_item(Ind, &item, ITEM_ANY)) return FALSE;

	identify_fully_item(Ind, item);	

	return TRUE;
}


/*
 * Fully "identify" an object in the inventory	-BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully_item(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type		*o_ptr;

	char		o_name[80];


	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
		p_ptr->redraw |= (PR_FLOOR);
	}


	/* Identify it fully */
	object_aware(p_ptr, o_ptr);
	object_known(o_ptr);

	/* Mark the item as fully known */
	o_ptr->ident |= (ID_MENTAL);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Handle artifact knowledge */
	if (artifact_p(o_ptr))
	{
		artifact_notify(p_ptr, o_ptr);
	}

	/* Handle stuff */
	handle_stuff(Ind);

	/* Description */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format(Ind, "%^s: %s (%c).",
		           describe_use(Ind, item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format(Ind, "In your pack: %s (%c).",
		           o_name, index_to_label(item));
	}
	else
	{
		msg_format(Ind, "On the ground: %s.",
		           o_name);
	}

	/* Describe it fully */
	identify_fully_aux(Ind, o_ptr);
	send_prepared_popup(Ind, o_name);

	/* Success */
	return (TRUE);
}




/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(object_type *o_ptr)
{
	/* Recharge staffs */
	if (o_ptr->tval == TV_STAFF) return (TRUE);

	/* Recharge wands */
	if (o_ptr->tval == TV_WAND) return (TRUE);

	/* Nope */
	return (FALSE);
}


bool recharge(int Ind, int spell_strength)
{
	int item;

	if (!get_item(Ind, &item, item_test(RECHARGE))) return FALSE;
	
	recharge_aux(Ind, item, spell_strength);
	
	return TRUE;
}


/*
 * Recharge a wand or staff from the pack or on the floor.
 *
 * It is harder to recharge high level, and highly charged wands.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 *
 * Should probably not "destroy" over-charged items, unless we
 * "replace" them by, say, a broken stick or some such.  The only
 * reason this is okay is because "scrolls of recharging" appear
 * BEFORE all staffs/wands/rods in the inventory.  Note that the
 * new "auto_sort_pack" option would correctly handle replacing
 * the "broken" wand with any other item (i.e. a broken stick).
 *
 * XXX XXX XXX Perhaps we should auto-unstack recharging stacks.
 */
bool recharge_aux(int Ind, int item, int spell_strength)
{
	player_type *p_ptr = Players[Ind];

	int i, t, lev, _idx;

	object_type *o_ptr;


	/* Only accept legal items */
	item_tester_full = FALSE;
	item_tester_tval = 0;
	item_tester_hook = NULL; /*item_tester_hook_recharge;*/

	/* Get the item */
	if ( !(o_ptr = player_get_item(p_ptr, item, &_idx)) )
	{
		return FALSE;
	}

	item_tester_hook = item_tester_hook_recharge;
	if (!item_tester_hook(o_ptr))
	{
		msg_print(Ind, "You cannot recharge that item.");
		return (FALSE);
	}

	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;

	/*
	 * Chance of failure = 1 time in
	 * [Spell_strength + 100 - item_level - 10 * charge_per_item]/15
	 */
	i = (spell_strength + 100 - lev - (10 * (o_ptr->pval / o_ptr->number))) / 15;

	/* Back-fire */
	if ((i <= 1) || one_in_(i))
	{
		msg_print(Ind, "The recharge backfires!");

		reduce_charges(o_ptr, 1);

		/* Destroy the item */
		if (!cfg_safe_recharge)
		{
			msg_print(Ind, "There is a bright flash of light.");

			/* Reduce and describe inventory */
			if (item >= 0)
			{
				inven_item_increase(Ind, item, -1);
				inven_item_describe(Ind, item);
				inven_item_optimize(Ind, item);
			}
			/* Reduce and describe floor item */
			else
			{
				floor_item_increase(0 - item, -1);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
				floor_item_notify(Ind, 0 - item, TRUE);
			}
		}
		/* MAngband-specific: remove even MORE charges */
		else
		{
			reduce_charges(o_ptr, 1);

			/* *Identified* items keep the knowledge about the charges */
			if (!(o_ptr->ident & ID_MENTAL))
			{
				/* We no longer "know" the item */
				o_ptr->ident &= ~(ID_KNOWN);
			}
		}
	}

	/* Recharge */
	else
	{
		/* Extract a "power" */
		t = (spell_strength / (lev + 2)) + 1;

		/* Recharge based on the power */
		if (t > 0) o_ptr->pval += 2 + randint(t);

		/* We no longer think the item is empty */
		o_ptr->ident &= ~ID_EMPTY;
	}

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Redraw floor */
	if (item < 0) p_ptr->redraw |= (PR_FLOOR);

	/* Something was done */
	return (TRUE);
}








/*
 * Apply a "project()" directly to all viewable monsters
 */
static bool project_hack(int Ind, int typ, int dam)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, x, y;

	int		flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	bool	obvious = FALSE;


	/* Affect all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Skip monsters not on this depth */
		if (Depth != m_ptr->dun_depth) continue;

		/* Require line of sight */
		if (!player_has_los_bold(Ind, y, x)) continue;

		/* Jump directly to the target monster */
		if (project(0 - Ind, 0, Depth, y, x, dam, typ, flg)) obvious = TRUE;
	}

	/* Result */
	return (obvious);
}


/*
 * Speed monsters
 */
bool speed_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SPEED, p_ptr->lev));
}

/*
 * Slow monsters
 */
bool slow_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SLOW, p_ptr->lev));
}

/*
 * Sleep monsters
 */
bool sleep_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SLEEP, p_ptr->lev));
}


/*
 * Banish evil monsters
 */
bool banish_evil(int Ind, int dist)
{
	return (project_hack(Ind, GF_AWAY_EVIL, dist));
}


/*
 * Turn undead
 */
bool turn_undead(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_TURN_UNDEAD, p_ptr->lev));
}


/*
 * Dispel undead monsters
 */
bool dispel_undead(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_UNDEAD, dam));
}

/*
 * Dispel evil monsters
 */
bool dispel_evil(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_EVIL, dam));
}

/*
 * Dispel all monsters
 */
bool dispel_monsters(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_ALL, dam));
}




/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int Ind, int who)
{
	player_type *p_ptr = Players[Ind];

	int i, d;

	bool sleep = FALSE;
	bool speed = FALSE;

	/* Aggravate everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Skip aggravating monster (or player) */
		if (i == who) continue;

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Wake up nearby sleeping monsters */
		if (d < MAX_SIGHT * 2)
		{
			/* Wake up */
			if (m_ptr->csleep)
			{
				/* Wake up */
				m_ptr->csleep = 0;
				sleep = TRUE;
			}
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(Ind, m_ptr->fy, m_ptr->fx))
		{
			/* Speed up (instantly) to racial base + 10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				/* Speed up */
				m_ptr->mspeed = r_ptr->speed + 10;
				speed = TRUE;
			}
		}
	}

	/* Messages */
	if (speed) msg_print(Ind, "You feel a sudden stirring nearby!");
	else if (sleep) msg_print(Ind, "You hear a sudden stirring in the distance!");
}



/*
 * Delete all non-unique monsters of a given "type" from the level
 *
 * This is different from normal Angband now -- the closest non-unique
 * monster is chosen as the designed character to genocide.
 */
bool banishment(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i;

	char	typ;

	bool	result = FALSE;

	/* int		msec = delay_factor * delay_factor * delay_factor; */

	int d = 999, tmp;

	/* Hack -- disable in dwarven halls / custom towns */
	if (check_special_level(p_ptr->dun_depth)) return TRUE;

	/* Search all monsters and find the closest */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Check distance */
		if ((tmp = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx)) < d)
		{
			/* Set closest distance */
			d = tmp;

			/* Set char */
			typ = r_ptr->d_char;
		}
	}

	/* Check to make sure we found a monster */
	if (d == 999)
	{
		return FALSE;
	}

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take damage */
		take_hit(Ind, randint(4), "the strain of casting Banishment");

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		/* p_ptr->window |= (PW_PLAYER); */

		/* Handle */
		handle_stuff(Ind);

		/* Fresh */
		/* Term_fresh(); */

		/* Delay */
		Send_flush(Ind);

		/* Take note */
		result = TRUE;
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff(Ind);

	return (result);
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_banishment(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i, d;

	bool	result = FALSE;

	/*int		msec = delay_factor * delay_factor * delay_factor;*/

	/* Hack -- disable in dwarven halls / custom towns */
	if (check_special_level(p_ptr->dun_depth)) return TRUE;

	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Skip distant monsters */
		if (d > MAX_SIGHT) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Hack -- visual feedback */
		/* does not effect the dungeon master, because it disturbs his movement
		 */
		if (!(p_ptr->dm_flags & DM_INVULNERABLE))
			take_hit(Ind, randint(3), "the strain of casting Mass Banishment");

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		/* p_ptr->window |= (PW_PLAYER); */

		/* Handle */
		handle_stuff(Ind);

		/* Fresh */
		/*Term_fresh();*/

		/* Delay */
		Send_flush(Ind);

		/* Note effect */
		result = TRUE;
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff(Ind);

	return (result);
}



/*
 * Probe nearby monsters
 */
bool probing(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	int            i,d;
	bool	probe = FALSE;

	char m_name[80];

	/* Probe all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (Depth != m_ptr->dun_depth) continue;

		/* MAngband specific spell change - don't require LoS, instead limit
		 * to radius around the player */

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Require line of sight */
		if (d > MAX_SIGHT) continue;

		/* Start the message */
		if (!probe) msg_print(Ind, "Probing...");

		/* Get "the monster" or "something" */
		monster_desc(Ind, m_name, i, 0x04);

		/* Describe the monster */
		msg_format(Ind, "%^s has %d hit points.", m_name, m_ptr->hp);

		/* Learn all of the non-spell, non-treasure flags */
		lore_do_probe(Ind, i);

		/* Probe worked */
		probe = TRUE;
	}

	/* Done */
	if (probe)
	{
		msg_print(Ind, "That's all.");
	}

	/* Result */
	return (probe);
}



/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int Depth, int y1, int x1, int r, bool full)
{
	int y, x, k, t, Ind;

	player_type *p_ptr;

	cave_type *c_ptr;

	/*bool flag = FALSE;*/


	/* XXX XXX */
	full = full ? full : 0;

	/* Don't hurt the main town or surrounding areas */
	if (Depth <= 0 ? (wild_info[Depth].radius <= 2) : 0)
		return;

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds(Depth, y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][y][x];

			/* Lose room and vault */
			/* Hack -- don't do this to houses/rooms outside the dungeon,
			 * this will protect hosues outside town.
			 */
			if (Depth > 0)
			{
				c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
			}

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW);
			everyone_forget_spot(Depth, y, x);

			/* Hack -- Notice player affect */
			if (c_ptr->m_idx < 0)
			{
				Ind = 0 - c_ptr->m_idx;
				p_ptr = Players[Ind];

				/* Message */
				msg_print(Ind, "There is a searing blast of light!");
	
				/* Blind the player */
				if (!p_ptr->resist_blind && !p_ptr->resist_lite)
				{
					/* Become blind */
					(void)set_blind(Ind, p_ptr->blind + 10 + randint(10));
				}

				/* Mega-Hack -- Forget the view and lite */
				p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

				/* Update stuff */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

				/* Update the monsters */
				p_ptr->update |= (PU_MONSTERS);
		
				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD);

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			delete_monster(Depth, y, x);

			/* Destroy "valid" grids */
			if ((cave_valid_bold(Depth, y, x)) && !(c_ptr->info&CAVE_ICKY))
			{
				/* Delete the object (if any) */
				delete_object(Depth, y, x);

				/* Wall (or floor) type */
				t = rand_int(200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}
}


/*
 * Swap the players/monsters (if any) at two locations
 */
static void monster_swap(int Depth, int y1, int x1, int y2, int x2)
{
	int m1, m2;
	monster_type *m_ptr;
	cave_type *c_ptr1, *c_ptr2;
	player_type *p_ptr;

	/* Access the grids */
	c_ptr1 = &cave[Depth][y1][x1];
	c_ptr2 = &cave[Depth][y2][x2];

	/* Monsters */
	m1 = c_ptr1->m_idx;
	m2 = c_ptr2->m_idx;

	/* Update grids */
	c_ptr1->m_idx = m2;
	c_ptr2->m_idx = m1;

	/* Monster 1 */
	if (m1 > 0)
	{
		m_ptr = &m_list[m1];

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* Update monster */
		update_mon(m1, TRUE);
	}

	/* Player 1 */
	else if (m1 < 0)
	{
		p_ptr = Players[0 - m1];

		/* Move player */
		p_ptr->py = y2;
		p_ptr->px = x2;

		/* Update the panel */
		verify_panel(0 - m1);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_FLOW);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);
	}

	/* Monster 2 */
	if (m2 > 0)
	{
		m_ptr = &m_list[m2];

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* Update monster */
		update_mon(m2, TRUE);
	}

	/* Player 2 */
	else if (m2 < 0)
	{
		p_ptr = Players[0 - m2];

		/* Move player */
		p_ptr->py = y1;
		p_ptr->px = x1;

		/* Update the panel */
		verify_panel(0 - m2);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_FLOW);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);
	}

	/* Redraw */
	everyone_lite_spot(Depth, y1, x1);
	everyone_lite_spot(Depth, y2, x2);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 */
void earthquake(int Depth, int cy, int cx, int r)
{
	int y, x, yy, xx, dy, dx, i, t, j;
	bool map[32][32];
	cave_type *c_ptr;
	int hurt[MAX_PLAYERS];
	int count = 0;
	int Ind;
	player_type *p_ptr;
	int sn, sy, sx;
	int damage;

	/* Don't hurt town or surrounding areas */
	if (Depth <= 0 ? wild_info[Depth].radius <= 2 : 0)
		return;

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Take note of any player */
			if (c_ptr->m_idx < 0)
			{
				hurt[count] = c_ptr->m_idx;
				count++;
			}

			/* Hack -- ICKY spaces are protected outside of the dungeon */
			if ((Depth < 0) && (c_ptr->info & CAVE_ICKY)) continue;

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW);
			everyone_forget_spot(Depth, y, x);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (rand_int(100) < 85) continue;

			/* Damage this grid */
			map[16+yy-cy][16+xx-cx] = TRUE;

			/* Hack -- Take note of player damage */
			if (c_ptr->m_idx < 0) hurt[count-1] = - hurt[count-1];
		}
	}

	/* First, affect the players (if necessary) */
	for (j = 0; j < count; j++)
	{
		Ind = hurt[j];

		/* Skip undamaged players */
		if (Ind < 0) continue;

		p_ptr = Players[Ind];

		sn = 0; sy = 0; sx = 0; damage = 0;

		/* Check around the player */
		for (i = 0; i < 8; i++)
		{
			/* Get the location */
			y = p_ptr->py + ddy_ddd[i];
			x = p_ptr->px + ddx_ddd[i];

			/* Skip illegal grids */
			if (!in_bounds(Depth, y, x)) continue;

			/* Skip non-empty grids */
			if (!cave_empty_bold(Depth, y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16+y-cy][16+x-cx]) continue;

			/* Count "safe" grids, apply the randomizer */
			if ((++sn > 1) && (rand_int(sn) != 0)) continue;

			/* Save the safe location */
			sy = y; sx = x;
		}

		/* Random message */
		switch (randint(3))
		{
			case 1:
			{
				msg_print(Ind, "The cave ceiling collapses!");
				break;
			}
			case 2:
			{
				msg_print(Ind, "The cave floor twists in an unnatural way!");
				break;
			}
			default:
			{
				msg_print(Ind, "The cave quakes!");
				msg_print(Ind, "You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!sn)
		{
			/* Message and damage */
			msg_print(Ind, "You are severely crushed!");
			damage = 300;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint(3))
			{
				case 1:
				{
					msg_print(Ind, "You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2:
				{
					msg_print(Ind, "You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)set_stun(Ind, p_ptr->stun + randint(50));
					break;
				}
				case 3:
				{
					msg_print(Ind, "You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)set_stun(Ind, p_ptr->stun + randint(50));
					break;
				}
			}

			/* Move player */
			monster_swap(Depth, p_ptr->py, p_ptr->px, sy, sx);
		}

		/* Take some damage */
		if (damage) take_hit(Ind, damage, "an earthquake");
	}

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Process monsters */
			if (c_ptr->m_idx > 0)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				monster_race *r_ptr = &r_info[m_ptr->r_idx];

				/* Most monsters cannot co-exist with rock */
				if (!(r_ptr->flags2 & RF2_KILL_WALL) &&
					!(r_ptr->flags2 & RF2_PASS_WALL))
				{
					char m_name[MAX_CHARS];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Get the grid */
							y = yy + ddy_ddd[i];
							x = xx + ddx_ddd[i];

							/* Skip illegal grids */
							if (!in_bounds(Depth, y, x)) continue;

							/* Skip non-empty grids */
							if (!cave_empty_bold(Depth, y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (cave[Depth][y][x].feat == FEAT_GLYPH) continue;

							/* Important -- Skip "quake" grids */
							if (map[16+y-cy][16+x-cx]) continue;

							/* Count "safe" grids, apply the randomizer */
							if ((++sn > 1) && (rand_int(sn) != 0)) continue;

							/* Save the safe grid */
							sy = y; sx = x;
						}
					}

					/* Give players a message */
					for (j = 0; j < count; j++)
					{
						/* Get player */
						Ind = hurt[j];
						if (Ind < 0) Ind = -Ind;
						p_ptr = Players[Ind];

						/* Describe the monster */
						monster_desc(Ind, m_name, c_ptr->m_idx, 0);

						/* Scream in pain */
						msg_format(Ind, "%^s wails out in pain!", m_name);
					}

					/* Take damage from the quake */
					damage = (sn ? damroll(4, 8) : (m_ptr->hp + 1));

					/* Monster is certainly awake */
					m_ptr->csleep = 0;

					/* Apply damage directly */
					m_ptr->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Give players a message */
						for (j = 0; j < count; j++)
						{
							/* Get player */
							Ind = hurt[j];
							if (Ind < 0) Ind = -Ind;
							p_ptr = Players[Ind];

							/* Describe the monster */
							monster_desc(Ind, m_name, c_ptr->m_idx, 0);

							/* Message */
							msg_format(Ind, "%^s is embedded in the rock!", m_name);
						}

						/* Delete the monster */
						delete_monster(Depth, yy, xx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						/* Move the monster */
						monster_swap(Depth, yy, xx, sy, sx);
					}
				}
			}
		}
	}

	/* Important -- no wall on players */
	for (j = 0; j < count; j++)
	{
		/* Get player */
		Ind = hurt[j];
		if (Ind < 0) Ind = -Ind;
		p_ptr = Players[Ind];

		map[16+p_ptr->py-cy][16+p_ptr->px-cx] = FALSE;
	}
	
	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the cave grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Paranoia -- never affect player */
			if (c_ptr->m_idx < 0) continue;

			/* Destroy location (if valid) */
			if (cave_valid_bold(Depth, yy, xx))
			{
				bool floor = cave_floor_bold(Depth, yy, xx);

				/* Delete any object that is still there */
				delete_object(Depth, yy, xx);

				/* Wall (or floor) type */
				t = (floor ? rand_int(100) : 200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}

	for (j = 0; j < count; j++)
	{
		/* Get player */
		Ind = hurt[j];
		if (Ind < 0) Ind = -Ind;
		p_ptr = Players[Ind];

		/* Fully update the visuals */
		p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE | PU_VIEW | PU_LITE | PU_MONSTERS);

		/* Fully update the flow */
		p_ptr->update |= (PU_FLOW);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Update the health bar */
		p_ptr->redraw |= (PR_HEALTH);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_MONSTER);
	}

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Redraw any effected grids */
			everyone_lite_spot(Depth,yy,xx);
		}
	}
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int i;

	/* Clear them all */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		int y = p_ptr->temp_y[i];
		int x = p_ptr->temp_x[i];

		cave_type *c_ptr = &cave[Depth][y][x];

		/* No longer in the array */
		c_ptr->info &= ~CAVE_TEMP;

		/* Update only non-CAVE_GLOW grids */
		/* if (c_ptr->info & CAVE_GLOW) continue; */

		/* Perma-Lite */
		c_ptr->info |= CAVE_GLOW;

		/* Process affected monsters */
		if (c_ptr->m_idx > 0)
		{
			int chance = 25;

			monster_type	*m_ptr = &m_list[c_ptr->m_idx];

			monster_race	*r_ptr = &r_info[m_ptr->r_idx];

			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags2 & RF2_STUPID) chance = 10;

			/* Smart monsters always wake up */
			if (r_ptr->flags2 & RF2_SMART) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->csleep && (rand_int(100) < chance))
			{
				/* Wake up! */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (p_ptr->mon_vis[c_ptr->m_idx])
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(Ind, m_name, c_ptr->m_idx, 0);

					/* Dump a message */
					msg_format(Ind, "%^s wakes up.", m_name);
				}
			}
		}

		/* Note */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);
	}

	/* None left */
	p_ptr->temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int i;

	/* Clear them all */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		int y = p_ptr->temp_y[i];
		int x = p_ptr->temp_x[i];

		cave_type *c_ptr = &cave[Depth][y][x];

		/* No longer in the array */
		c_ptr->info &= ~CAVE_TEMP;

		/* Darken the grid */
		c_ptr->info &= ~CAVE_GLOW;

		/* Hack -- Forget "boring" grids */
		if (is_boring(c_ptr->feat))
		{
			/* Forget the grid */
			p_ptr->cave_flag[y][x] &= ~CAVE_MARK;

			/* Notice */
			note_spot_depth(Depth, y, x);
		}

		/* Process affected monsters */
		if (c_ptr->m_idx > 0)
		{
			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);
		}

		/* Redraw */
		everyone_lite_spot(Depth, y, x);
	}

	/* None left */
	p_ptr->temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int Ind, int Depth, int y, int x)
{
	player_type *p_ptr = Players[Ind];

	cave_type *c_ptr = &cave[Depth][y][x];

	/* Avoid infinite recursion */
	if (c_ptr->info & CAVE_TEMP) return;

	/* Do not "leave" the current room */
	if (!(c_ptr->info & CAVE_ROOM)) return;

	/* Paranoia -- verify space */
	if (p_ptr->temp_n == TEMP_MAX) return;

	/* Mark the grid as "seen" */
	c_ptr->info |= CAVE_TEMP;

	/* Add it to the "seen" set */
	p_ptr->temp_y[p_ptr->temp_n] = y;
	p_ptr->temp_x[p_ptr->temp_n] = x;
	p_ptr->temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int Ind, int Depth, int y1, int x1)
{
	player_type *p_ptr = Players[Ind];

	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(Ind, Depth, y1, x1);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		x = p_ptr->temp_x[i], y = p_ptr->temp_y[i];

		/* Walls get lit, but stop light */
		if (!cave_floor_bold(Depth, y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(Ind, Depth, y + 1, x);
		cave_temp_room_aux(Ind, Depth, y - 1, x);
		cave_temp_room_aux(Ind, Depth, y, x + 1);
		cave_temp_room_aux(Ind, Depth, y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(Ind, Depth, y + 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x - 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y + 1, x - 1);
	}

	/* Now, lite them all up at once */
	cave_temp_room_lite(Ind);
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int Ind, int Depth, int y1, int x1)
{
	player_type *p_ptr = Players[Ind];

	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(Ind, Depth, y1, x1);

	/* Spread, breadth first */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		x = p_ptr->temp_x[i], y = p_ptr->temp_y[i];

		/* Walls get dark, but stop darkness */
		if (!cave_floor_bold(Depth, y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(Ind, Depth, y + 1, x);
		cave_temp_room_aux(Ind, Depth, y - 1, x);
		cave_temp_room_aux(Ind, Depth, y, x + 1);
		cave_temp_room_aux(Ind, Depth, y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(Ind, Depth, y + 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x - 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y + 1, x - 1);
	}

	/* Now, darken them all at once */
	cave_temp_room_unlite(Ind);
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool lite_area(int Ind, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print(Ind, "You are surrounded by a white light.");
	}

	/* Hook into the "project()" function */
	(void)project(0 - Ind, rad, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam, GF_LITE_WEAK, flg);

	/* Lite up the room */
	lite_room(Ind, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlite_area(int Ind, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print(Ind, "Darkness surrounds you.");
	}

	/* Hook into the "project()" function */
	(void)project(0 - Ind, rad, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam, GF_DARK_WEAK, flg);

	/* Lite up the room */
	unlite_room(Ind, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}



/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int Ind, int typ, int dir, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int tx, ty;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ensure "dir" is in ddx/ddy array bounds */
	if (!VALID_DIR(dir)) dir = 5;

	/* Use the given direction */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay(Ind))
	{
		flg &= ~PROJECT_STOP;
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(0 - Ind, rad, p_ptr->dun_depth, ty, tx, dam, typ, flg));
}

/*
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
bool fire_swarm(int Ind, int num, int typ, int dir, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];
	bool noticed = FALSE;

	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Hack -- ensure "dir" is in ddy/ddx array bounds */
	if (!VALID_DIR(dir)) dir = 5;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" (early detonation) */
	if ((dir == 5) && target_okay(Ind))
	{
		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	while (num--)
	{
		/* Analyze the "dir" and the "target".  Hurt items on floor. */
		if (project(0 - Ind, rad, p_ptr->dun_depth, ty, tx, dam, typ, flg)) noticed = TRUE;
	}

	return noticed;
}



/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
bool project_hook(int Ind, int typ, int dir, int dam, int flg)
{
	player_type *p_ptr = Players[Ind];

	int tx, ty;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Use the given direction */
	tx = p_ptr->px + ddx[dir];
	ty = p_ptr->py + ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay(Ind))
	{
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}

	/* Analyze the "dir" and the "target", do NOT explode */
	return (project(0 - Ind, 0, p_ptr->dun_depth, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_bolt(int Ind, int typ, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool fire_beam(int Ind, int typ, int dir, int dam)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;
	return (project_hook(Ind, typ, dir, dam, flg));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int Ind, int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		return (fire_beam(Ind, typ, dir, dam));
	}
	else
	{
		return (fire_bolt(Ind, typ, dir, dam));
	}
}


/*
 * Some of the old functions
 */

bool lite_line(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(Ind, GF_LITE_WEAK, dir, damroll(6, 8), flg));
}

bool strong_lite_line(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(Ind, GF_LITE, dir, damroll(10, 8), flg));
}

bool drain_life(int Ind, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
	return (project_hook(Ind, GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool destroy_door(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(Ind, GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_trap(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(Ind, GF_KILL_TRAP, dir, 0, flg));
}

bool heal_monster(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SPEED, dir, p_ptr->lev, flg));
}

bool slow_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int Ind, int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_CONF, dir, plev, flg));
}

bool poly_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;

	/* Never in the town */
	if(!p_ptr->dun_depth) return(FALSE);

	return (project_hook(Ind, GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	/* Restricted in MAngband. This behaviour is not in vanilla Angband, in vanilla however,
	 * cloning 100 Great Hell Wyrms effects only the cloner. In MAngband this effects all 
	 * players and the whole game economy as the game is flooded with great items */
	if(p_ptr->lev >= 10)
	{
		/* Standard clone effect, less the actual cloning */
		int flg = PROJECT_STOP | PROJECT_KILL;
		msg_print(Ind, "You hear a loud crackling sound.");
		return (project_hook(Ind, GF_OLD_CLONE, dir, 0, flg));
	}
	else
	{
		/* Standard clone effect */
		int flg = PROJECT_STOP | PROJECT_KILL;
		return (project_hook(Ind, GF_OLD_CLONE, dir, 0, flg));
	}
}

bool fear_monster(int Ind, int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_TURN_ALL, dir, plev, flg));
}

bool teleport_monster(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;

	/* Sound */
	sound(Ind, MSG_TPOTHER);

	return (project_hook(Ind, GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}

bool alter_reality(int Ind, bool power)
{
	player_type *p_ptr = Players[Ind];

	/* Which dungeon level are we changing? */
	int Depth = p_ptr->dun_depth, i;

	/* Don't allow this in towns or the wilderness */
	if( (Depth <= 0) || (check_special_level(Depth) && !power) )
		return (FALSE);

	/* Test players on this depth */
	if (!power)
	{
		for (i = 1; i < NumPlayers + 1; i++)
		{
			player_type *q_ptr = Players[i];

			if ((q_ptr->dun_depth == Depth) && (i != Ind))
			{
				if (p_ptr->party && p_ptr->party == q_ptr->party) continue;

				/* Saving throw: perception (harder if hostile) */
				if (rand_int(127) < q_ptr->skill_fos * (pvp_okay(Ind, i, 0) ? 6 : 4))
				{
					msg_format(Ind, "%s sustains reality.", (p_ptr->play_los[i] ? q_ptr->name : "Someone"));
					msg_format(i, "You resist %s's attempt to alter reality.", (q_ptr->play_los[Ind] ? p_ptr->name : "someone") );
					return (FALSE);
				}
			}
		}
	}

	/* Search for players on this depth */
	for (i = 1; i < NumPlayers + 1; i++)
	{
		player_type *q_ptr = Players[i];

		/* Only players on this depth */ 
		if(q_ptr->dun_depth == Depth)
		{
			/* Tell the player about it */
			msg_print(i, "The world changes!");
			q_ptr->new_level_flag = TRUE;
			q_ptr->new_level_method = LEVEL_RAND;
		}
	}

	/* Deallocate the level */
	dealloc_dungeon_level(Depth);
	cave[Depth] = 0;

	/* Reality altered */	
	return (TRUE);
}

bool heal_player_ball(int Ind, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, dam, flg));
}

bool project_prayer_ball(int Ind, int dir, int spell)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_PROJECT_PRAYER, dir, spell, flg));
}

bool project_spell_ball(int Ind, int dir, int spell)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_PROJECT_SPELL, dir, spell, flg));
}

/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */

bool door_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	if (level_is_town(p_ptr->dun_depth)) { return FALSE; }
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	if (level_is_town(p_ptr->dun_depth)) { return FALSE; }
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_MAKE_TRAP, flg));
}

bool destroy_doors_touch(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_KILL_DOOR, flg));
}

bool sleep_monsters_touch(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_KILL | PROJECT_HIDE;
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, p_ptr->lev, GF_OLD_SLEEP, flg));
}

/*
 * Enchant some bolts
 */
bool brand_bolts(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int i;

	/* Use the first (XXX) acceptable bolts */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-bolts */
		if (o_ptr->tval != TV_BOLT) continue;

		/* Skip artifacts and ego-items */
		if (artifact_p(o_ptr) || ego_item_p(o_ptr)) continue;

		/* Skip cursed/broken items */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) continue;

		/* Randomize */
		if (rand_int(100) < 75) continue;

		/* Message */
		msg_print(Ind, "Your bolts are covered in a fiery aura!");

		/* Ego-item */
		o_ptr->name2 = EGO_FLAME;

		/* Enchant */
		enchant(Ind, o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);

		/* Notice */
		return (TRUE);
	}

	/* Flush */
	/*if (flush_failure) flush();*/

	/* Fail */
	msg_print(Ind, "The fiery enchantment failed.");

	/* Notice */
	return (TRUE);
}


/*
 * Hack -- activate the ring of power
 */
void ring_of_power(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	/* Pick a random effect */
	switch (randint(10))
	{
		case 1:
		case 2:
		{
			/* Message */
			msg_print(Ind, "You are surrounded by a malignant aura.");

			/* Decrease all stats (permanently) */
			(void)dec_stat(Ind, A_STR, 50, TRUE);
			(void)dec_stat(Ind, A_INT, 50, TRUE);
			(void)dec_stat(Ind, A_WIS, 50, TRUE);
			(void)dec_stat(Ind, A_DEX, 50, TRUE);
			(void)dec_stat(Ind, A_CON, 50, TRUE);
			(void)dec_stat(Ind, A_CHR, 50, TRUE);

			/* Lose some experience (permanently) */
			p_ptr->exp -= (p_ptr->exp / 4);
			p_ptr->max_exp -= (p_ptr->exp / 4);
			check_experience(Ind);

			break;
		}

		case 3:
		{
			/* Message */
			msg_print(Ind, "You are surrounded by a powerful aura.");

			/* Dispel monsters */
			dispel_monsters(Ind, 1000);

			break;
		}

		case 4:
		case 5:
		case 6:
		{
			/* Mana Ball */
			fire_ball(Ind, GF_MANA, dir, 300, 3);

			break;
		}

		case 7:
		case 8:
		case 9:
		case 10:
		{
			/* Mana Bolt */
			fire_bolt(Ind, GF_MANA, dir, 250);

			break;
		}
	}
}
