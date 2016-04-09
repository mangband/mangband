/* File: melee2.c */

/* Purpose: Monster spells and movement */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



#ifdef DRS_SMART_OPTIONS


/*
 * And now for Intelligent monster attacks (including spells).
 *
 * Original idea and code by "DRS" (David Reeves Sward).
 * Major modifications by "BEN" (Ben Harrison).
 *
 * Give monsters more intelligent attack/spell selection based on
 * observations of previous attacks on the player, and/or by allowing
 * the monster to "cheat" and know the player status.
 *
 * Maintain an idea of the player status, and use that information
 * to occasionally eliminate "ineffective" spell attacks.  We could
 * also eliminate ineffective normal attacks, but there is no reason
 * for the monster to do this, since he gains no benefit.
 * Note that MINDLESS monsters are not allowed to use this code.
 * And non-INTELLIGENT monsters only use it partially effectively.
 *
 * Actually learn what the player resists, and use that information
 * to remove attacks or spells before using them.  This will require
 * much less space, if I am not mistaken.  Thus, each monster gets a
 * set of 32 bit flags, "smart", build from the various "SM_*" flags.
 *
 * This has the added advantage that attacks and spells are related.
 * The "smart_learn" option means that the monster "learns" the flags
 * that should be set, and "smart_cheat" means that he "knows" them.
 * So "smart_cheat" means that the "smart" field is always up to date,
 * while "smart_learn" means that the "smart" field is slowly learned.
 * Both of them have the same effect on the "choose spell" routine.
 */




/*
 * Internal probablility routine
 */
static bool int_outof(monster_race *r_ptr, int prob)
{
	/* Non-Smart monsters are half as "smart" */
	if (!(r_ptr->flags2 & RF2_SMART)) prob = prob / 2;

	/* Roll the dice */
	return (rand_int(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, u32b *f4p, u32b *f5p, u32b *f6p)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u32b f4 = (*f4p);
	u32b f5 = (*f5p);
	u32b f6 = (*f6p);

	u32b smart = 0L;


	/* Too stupid to know anything */
	if (r_ptr->flags2 & RF2_STUPID) return;


	/* Must be cheating or learning */
	if (!smart_cheat && !smart_learn) return;


	/* Update acquired knowledge */
	if (smart_learn)
	{
		/* Hack -- Occasionally forget player status */
		if (m_ptr->smart && (rand_int(100) < 1)) m_ptr->smart = 0L;

		/* Use the memorized flags */
		smart = m_ptr->smart;
	}


	/* Cheat if requested */
	if (smart_cheat)
	{
		/* Know basic info */
		if (p_ptr->resist_acid) smart |= SM_RES_ACID;
		if (p_ptr->oppose_acid) smart |= SM_OPP_ACID;
		if (p_ptr->immune_acid) smart |= SM_IMM_ACID;
		if (p_ptr->resist_elec) smart |= SM_RES_ELEC;
		if (p_ptr->oppose_elec) smart |= SM_OPP_ELEC;
		if (p_ptr->immune_elec) smart |= SM_IMM_ELEC;
		if (p_ptr->resist_fire) smart |= SM_RES_FIRE;
		if (p_ptr->oppose_fire) smart |= SM_OPP_FIRE;
		if (p_ptr->immune_fire) smart |= SM_IMM_FIRE;
		if (p_ptr->resist_cold) smart |= SM_RES_COLD;
		if (p_ptr->oppose_cold) smart |= SM_OPP_COLD;
		if (p_ptr->immune_cold) smart |= SM_IMM_COLD;

		/* Know poison info */
		if (p_ptr->resist_pois) smart |= SM_RES_POIS;
		if (p_ptr->oppose_pois) smart |= SM_OPP_POIS;

		/* Know special resistances */
		if (p_ptr->resist_neth) smart |= SM_RES_NETH;
		if (p_ptr->resist_lite) smart |= SM_RES_LITE;
		if (p_ptr->resist_dark) smart |= SM_RES_DARK;
		if (p_ptr->resist_fear) smart |= SM_RES_FEAR;
		if (p_ptr->resist_conf) smart |= SM_RES_CONF;
		if (p_ptr->resist_chaos) smart |= SM_RES_CHAOS;
		if (p_ptr->resist_disen) smart |= SM_RES_DISEN;
		if (p_ptr->resist_blind) smart |= SM_RES_BLIND;
		if (p_ptr->resist_nexus) smart |= SM_RES_NEXUS;
		if (p_ptr->resist_sound) smart |= SM_RES_SOUND;
		if (p_ptr->resist_shard) smart |= SM_RES_SHARD;

		/* Know bizarre "resistances" */
		if (p_ptr->free_act) smart |= SM_IMM_FREE;
		if (!p_ptr->msp) smart |= SM_IMM_MANA;
	}


	/* Nothing known */
	if (!smart) return;


	if (smart & SM_IMM_ACID)
	{
		if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_ACID;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_ACID;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ACID;
	}
	else if ((smart & SM_OPP_ACID) && (smart & SM_RES_ACID))
	{
		if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_ACID;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_ACID;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ACID;
	}
	else if ((smart & SM_OPP_ACID) || (smart & SM_RES_ACID))
	{
		if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_ACID;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_ACID;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ACID;
	}


	if (smart & SM_IMM_ELEC)
	{
		if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_ELEC;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_ELEC;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ELEC;
	}
	else if ((smart & SM_OPP_ELEC) && (smart & SM_RES_ELEC))
	{
		if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_ELEC;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_ELEC;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ELEC;
	}
	else if ((smart & SM_OPP_ELEC) || (smart & SM_RES_ELEC))
	{
		if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_ELEC;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_ELEC;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ELEC;
	}


	if (smart & SM_IMM_FIRE)
	{
		if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_FIRE;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_FIRE;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_FIRE;
	}
	else if ((smart & SM_OPP_FIRE) && (smart & SM_RES_FIRE))
	{
		if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_FIRE;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_FIRE;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_FIRE;
	}
	else if ((smart & SM_OPP_FIRE) || (smart & SM_RES_FIRE))
	{
		if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_FIRE;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_FIRE;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_FIRE;
	}


	if (smart & SM_IMM_COLD)
	{
		if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_COLD;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_COLD;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_COLD;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ICEE;
	}
	else if ((smart & SM_OPP_COLD) && (smart & SM_RES_COLD))
	{
		if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_COLD;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_COLD;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_COLD;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ICEE;
	}
	else if ((smart & SM_OPP_COLD) || (smart & SM_RES_COLD))
	{
		if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_COLD;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_COLD;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_COLD;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ICEE;
	}


	if ((smart & SM_OPP_POIS) && (smart & SM_RES_POIS))
	{
		if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_POIS;
		if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_POIS;
	}
	else if ((smart & SM_OPP_POIS) || (smart & SM_RES_POIS))
	{
		if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_POIS;
		if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_POIS;
	}


	if (smart & SM_RES_NETH)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_NETH;
		if (int_outof(r_ptr, 50)) f5 &= ~RF5_BA_NETH;
		if (int_outof(r_ptr, 50)) f5 &= ~RF5_BO_NETH;
	}

	if (smart & SM_RES_LITE)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_LITE;
	}

	if (smart & SM_RES_DARK)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_DARK;
		if (int_outof(r_ptr, 50)) f5 &= ~RF5_BA_DARK;
	}

	if (smart & SM_RES_FEAR)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_SCARE;
	}

	if (smart & SM_RES_CONF)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_CONF;
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CONF;
	}

	if (smart & SM_RES_CHAOS)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_CONF;
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CONF;
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CHAO;
	}

	if (smart & SM_RES_DISEN)
	{
		if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_DISE;
	}

	if (smart & SM_RES_BLIND)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_BLIND;
	}

	if (smart & SM_RES_NEXUS)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_NEXU;
		if (int_outof(r_ptr, 50)) f6 &= ~RF6_TELE_LEVEL;
	}

	if (smart & SM_RES_SOUND)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_SOUN;
	}

	if (smart & SM_RES_SHARD)
	{
		if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_SHAR;
	}


	if (smart & SM_IMM_FREE)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_HOLD;
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_SLOW;
	}

	if (smart & SM_IMM_MANA)
	{
		if (int_outof(r_ptr, 100)) f5 &= ~RF5_DRAIN_MANA;
	}


	/* XXX XXX XXX No spells left? */
	/* if (!f4 && !f5 && !f6) ... */


	(*f4p) = f4;
	(*f5p) = f5;
	(*f6p) = f6;
}


#endif


/*
 * Cast a bolt at the player
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void bolt(int Ind, int m_idx, int typ, int dam_hp)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;

	/* Target the player with a bolt attack */
	(void)project(m_idx, 0, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam_hp, typ, flg);
}


/*
 * Cast a breath (or ball) attack at the player
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and the player
 */
static void breath(int Ind, int m_idx, int typ, int dam_hp)
{
	player_type *p_ptr = Players[Ind];

	int rad;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Determine the radius of the blast */
	rad = (r_ptr->flags2 & RF2_POWERFUL) ? 3 : 2;

	/* Target the player with a ball attack */
	(void)project(m_idx, rad, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam_hp, typ, flg);
}



/*
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "TRUE" if a spell (or whatever) was (successfully) cast.
 *
 * XXX XXX XXX This function could use some work, but remember to
 * keep it as optimized as possible, while retaining generic code.
 *
 * Verify the various "blind-ness" checks in the code.
 *
 * XXX XXX XXX Note that several effects should really not be "seen"
 * if the player is blind.  See also "effects.c" for other "mistakes".
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 *
 * Perhaps smart monsters should decline to use "bolt" spells if
 * there is a monster in the way, unless they wish to kill it.
 *
 * Note that, to allow the use of the "track_target" option at some
 * later time, certain non-optimal things are done in the code below,
 * including explicit checks against the "direct" variable, which is
 * currently always true by the time it is checked, but which should
 * really be set according to an explicit "projectable()" test, and
 * the use of generic "x,y" locations instead of the player location,
 * with those values being initialized with the player location.
 *
 * It will not be possible to "correctly" handle the case in which a
 * monster attempts to attack a location which is thought to contain
 * the player, but which in fact is nowhere near the player, since this
 * might induce all sorts of messages about the attack itself, and about
 * the effects of the attack, which the player might or might not be in
 * a position to observe.  Thus, for simplicity, it is probably best to
 * only allow "faulty" attacks by a monster if one of the important grids
 * (probably the initial or final grid) is in fact in view of the player.
 * It may be necessary to actually prevent spell attacks except when the
 * monster actually has line of sight to the player.  Note that a monster
 * could be left in a bizarre situation after the player ducked behind a
 * pillar and then teleported away, for example.
 *
 * Note that certain spell attacks do not use the "project()" function
 * but "simulate" it via the "direct" variable, which is always at least
 * as restrictive as the "project()" function.  This is necessary to
 * prevent "blindness" attacks and such from bending around walls, etc,
 * and to allow the use of the "track_target" option in the future.
 *
 * Note that this function attempts to optimize the use of spells for the
 * cases in which the monster has no spells, or has spells but cannot use
 * them, or has spells but they will have no "useful" effect.  Note that
 * this function has been an efficiency bottleneck in the past.
 */
bool make_attack_spell(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int			k, chance, thrown_spell, rlev;

	byte		spell[96], num = 0;

	u32b		f4, f5, f6;

	monster_type	*m_ptr = &m_list[m_idx];
	monster_race	*r_ptr = &r_info[m_ptr->r_idx];
	monster_lore	*l_ptr = p_ptr->l_list + m_ptr->r_idx;

	char		m_name[80];
	char		m_poss[80];

	char		ddesc[80];


	/* Target location */
	int x = p_ptr->px;
	int y = p_ptr->py;

	/* Summon count */
	int count = 0;


	/* Extract the blind-ness */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	bool seen = (!blind && p_ptr->mon_vis[m_idx]);


	/* Assume "normal" target */
	bool normal = TRUE;

	/* Assume "projectable" */
	bool direct = TRUE;


	/* Hack -- Extract the spell probability */
	chance = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;

	/* Not allowed to cast spells */
	if (!chance) return (FALSE);

	/* Cannot cast spells when confused */
	if (m_ptr->confused) return (FALSE);

	/* Only do spells occasionally */
	if (rand_int(100) >= chance) return (FALSE);


	/* XXX XXX XXX Handle "track_target" option (?) */


	/* Hack -- require projectable player */
	if (normal)
	{
		/* Check range */
		if (m_ptr->cdis > MAX_RANGE) return (FALSE);

		/* Check path (destination could be standing on a wall) */
		if (!projectable_wall(p_ptr->dun_depth, m_ptr->fy, m_ptr->fx, p_ptr->py, p_ptr->px))
		    return (FALSE);
	}


	/* Extract the monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Extract the racial spell flags */
	f4 = r_ptr->flags4;
	f5 = r_ptr->flags5;
	f6 = r_ptr->flags6;


	/* Hack -- allow "desperate" spells */
	if ((r_ptr->flags2 & RF2_SMART) &&
	    (m_ptr->hp < m_ptr->maxhp / 10) &&
	    (rand_int(100) < 50))
	{
		/* Require intelligent spells */
		f4 &= RF4_INT_MASK;
		f5 &= RF5_INT_MASK;
		f6 &= RF6_INT_MASK;

		/* No spells left */
		if (!f4 && !f5 && !f6) return (FALSE);
	}


#ifdef DRS_SMART_OPTIONS

	/* Remove the "ineffective" spells */
	remove_bad_spells(m_idx, &f4, &f5, &f6);

	/* No spells left */
	if (!f4 && !f5 && !f6) return (FALSE);

#endif


	/* Extract the "inate" spells */
	for (k = 0; k < 32; k++)
	{
		if (f4 & (1L << k)) spell[num++] = k + 32 * 3;
	}

	/* Extract the "normal" spells */
	for (k = 0; k < 32; k++)
	{
		if (f5 & (1L << k)) spell[num++] = k + 32 * 4;
	}

	/* Extract the "bizarre" spells */
	for (k = 0; k < 32; k++)
	{
		if (f6 & (1L << k)) spell[num++] = k + 32 * 5;
	}

	/* No spells left */
	if (!num) return (FALSE);


	/* Stop if player is dead or gone */
	if (!p_ptr->alive || p_ptr->death || p_ptr->new_level_flag) return (FALSE);


	/* Get the monster name (or "it") */
	monster_desc(Ind, m_name, m_idx, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(Ind, m_poss, m_idx, 0x22);

	/* Hack -- Get the "died from" name */
	monster_desc(Ind, ddesc, m_idx, 0x88);


	/* Choose a spell to cast */
	thrown_spell = spell[rand_int(num)];


	/* Cast the spell. */
	switch (thrown_spell)
	{
		/* RF4_SHRIEK */
		case 96+0:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s makes a high pitched shriek.", m_name);
			sound(Ind, MSG_SHRIEK);
			aggravate_monsters(Ind, m_idx);
			break;
		}

		/* RF4_XXX2X4 */
		case 96+1:
		{
			break;
		}

		/* RF4_XXX3X4 */
		case 96+2:
		{
			break;
		}

		/* RF4_XXX4X4 */
		case 96+3:
		{
			break;
		}

		/* RF4_ARROW_1 */
		case 96+4:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s makes a strange noise.", m_name);
			else msg_format(Ind, "%^s fires an arrow.", m_name);
			bolt(Ind, m_idx, GF_ARROW, damroll(1, 6));
			break;
		}

		/* RF4_ARROW_2 */
		case 96+5:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s makes a strange noise.", m_name);
			else msg_format(Ind, "%^s fires an arrow!", m_name);
			bolt(Ind, m_idx, GF_ARROW, damroll(3, 6));
			break;
		}

		/* RF4_ARROW_3 */
		case 96+6:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s makes a strange noise.", m_name);
			else msg_format(Ind, "%^s fires a missile.", m_name);
			bolt(Ind, m_idx, GF_ARROW, damroll(5, 6));
			break;
		}

		/* RF4_ARROW_4 */
		case 96+7:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s makes a strange noise.", m_name);
			else msg_format(Ind, "%^s fires a missile!", m_name);
			bolt(Ind, m_idx, GF_ARROW, damroll(7, 6));
			break;
		}

		/* RF4_BR_ACID */
		case 96+8:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_ACID);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes acid.", m_name);
			breath(Ind, m_idx, GF_ACID,
			       ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
			update_smart_learn(m_idx, DRS_ACID);
			break;
		}

		/* RF4_BR_ELEC */
		case 96+9:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_ELEC);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes lightning.", m_name);
			breath(Ind, m_idx, GF_ELEC,
			       ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
			update_smart_learn(m_idx, DRS_ELEC);
			break;
		}

		/* RF4_BR_FIRE */
		case 96+10:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_FIRE);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes fire.", m_name);
			breath(Ind, m_idx, GF_FIRE,
			       ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
			update_smart_learn(m_idx, DRS_FIRE);
			break;
		}

		/* RF4_BR_COLD */
		case 96+11:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_FROST);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes frost.", m_name);
			breath(Ind, m_idx, GF_COLD,
			       ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
			update_smart_learn(m_idx, DRS_COLD);
			break;
		}

		/* RF4_BR_POIS */
		case 96+12:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_GAS);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes gas.", m_name);
			breath(Ind, m_idx, GF_POIS,
			       ((m_ptr->hp / 3) > 800 ? 800 : (m_ptr->hp / 3)));
			update_smart_learn(m_idx, DRS_POIS);
			break;
		}

		/* RF4_BR_NETH */
		case 96+13:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_NETHER);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes nether.", m_name);
			breath(Ind, m_idx, GF_NETHER,
			       ((m_ptr->hp / 6) > 550 ? 550 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_NETH);
			break;
		}

		/* RF4_BR_LITE */
		case 96+14:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_LIGHT);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes light.", m_name);
			breath(Ind, m_idx, GF_LITE,
			       ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_LITE);
			break;
		}

		/* RF4_BR_DARK */
		case 96+15:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_DARK);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes darkness.", m_name);
			breath(Ind, m_idx, GF_DARK,
			       ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_DARK);
			break;
		}

		/* RF4_BR_CONF */
		case 96+16:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_CONF);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes confusion.", m_name);
			breath(Ind, m_idx, GF_CONFUSION,
			       ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_CONF);
			break;
		}

		/* RF4_BR_SOUN */
		case 96+17:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_SOUND);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes sound.", m_name);
			breath(Ind, m_idx, GF_SOUND,
			       ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_SOUND);
			break;
		}

		/* RF4_BR_CHAO */
		case 96+18:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_CHAOS);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes chaos.", m_name);
			breath(Ind, m_idx, GF_CHAOS,
			       ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_CHAOS);
			break;
		}

		/* RF4_BR_DISE */
		case 96+19:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_DISENCHANT);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes disenchantment.", m_name);
			breath(Ind, m_idx, GF_DISENCHANT,
			       ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_DISEN);
			break;
		}

		/* RF4_BR_NEXU */
		case 96+20:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_NEXUS);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes nexus.", m_name);
			breath(Ind, m_idx, GF_NEXUS,
			       ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_NEXUS);
			break;
		}

		/* RF4_BR_TIME */
		case 96+21:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_TIME);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes time.", m_name);
			breath(Ind, m_idx, GF_TIME,
			       ((m_ptr->hp / 3) > 150 ? 150 : (m_ptr->hp / 3)));
			break;
		}

		/* RF4_BR_INER */
		case 96+22:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_INERTIA);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes inertia.", m_name);
			breath(Ind, m_idx, GF_INERT,
			       ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
			break;
		}

		/* RF4_BR_GRAV */
		case 96+23:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_GRAVITY);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes gravity.", m_name);
			breath(Ind, m_idx, GF_GRAVITY,
			       ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)));
			break;
		}

		/* RF4_BR_SHAR */
		case 96+24:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_SHARDS);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes shards.", m_name);
			breath(Ind, m_idx, GF_SHARDS,
			       ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
			update_smart_learn(m_idx, DRS_SHARD);
			break;
		}

		/* RF4_BR_PLAS */
		case 96+25:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_PLASMA);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes plasma.", m_name);
			breath(Ind, m_idx, GF_PLASMA,
			       ((m_ptr->hp / 6) > 150 ? 150 : (m_ptr->hp / 6)));
			break;
		}

		/* RF4_BR_WALL */
		case 96+26:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_BR_FORCE);
			if (blind) msg_format(Ind, "%^s breathes.", m_name);
			else msg_format(Ind, "%^s breathes force.", m_name);
			breath(Ind, m_idx, GF_FORCE,
			       ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
			break;
		}

		/* RF4_BR_MANA */
		case 96+27:
		{
			/* XXX XXX XXX */
			break;
		}

		/* RF4_XXX5X4 */
		case 96+28:
		{
			break;
		}

		/* RF4_XXX6X4 */
		case 96+29:
		{
			break;
		}

		/* RF4_XXX7X4 */
		case 96+30:
		{
			break;
		}

		/* RF4_BOULDER */
		case 96+31:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "You hear something grunt with exertion.", m_name);
			else msg_format(Ind, "%^s hurls a boulder at you!", m_name);
			bolt(Ind, m_idx, GF_ARROW, damroll(1 + r_ptr->level / 7, 12));
			break;
		}



		/* RF5_BA_ACID */
		case 128+0:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts an acid ball.", m_name);
			breath(Ind, m_idx, GF_ACID,
			       randint(rlev * 3) + 15);
			update_smart_learn(m_idx, DRS_ACID);
			break;
		}

		/* RF5_BA_ELEC */
		case 128+1:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a lightning ball.", m_name);
			breath(Ind, m_idx, GF_ELEC,
			       randint(rlev * 3 / 2) + 8);
			update_smart_learn(m_idx, DRS_ELEC);
			break;
		}

		/* RF5_BA_FIRE */
		case 128+2:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a fire ball.", m_name);
			breath(Ind, m_idx, GF_FIRE,
			       randint(rlev * 7 / 2) + 10);
			update_smart_learn(m_idx, DRS_FIRE);
			break;
		}

		/* RF5_BA_COLD */
		case 128+3:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a frost ball.", m_name);
			breath(Ind, m_idx, GF_COLD,
			       randint(rlev * 3 / 2) + 10);
			update_smart_learn(m_idx, DRS_COLD);
			break;
		}

		/* RF5_BA_POIS */
		case 128+4:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a stinking cloud.", m_name);
			breath(Ind, m_idx, GF_POIS,
			       damroll(12, 2));
			update_smart_learn(m_idx, DRS_POIS);
			break;
		}

		/* RF5_BA_NETH */
		case 128+5:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a nether ball.", m_name);
			breath(Ind, m_idx, GF_NETHER,
			       (50 + damroll(10, 10) + rlev));
			update_smart_learn(m_idx, DRS_NETH);
			break;
		}

		/* RF5_BA_WATE */
		case 128+6:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s gestures fluidly.", m_name);
			msg_print(Ind, "You are engulfed in a whirlpool.");
			breath(Ind, m_idx, GF_WATER,
			       randint(rlev * 5 / 2) + 50);
			break;
		}

		/* RF5_BA_MANA */
		case 128+7:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles powerfully.", m_name);
			else msg_format(Ind, "%^s invokes a mana storm.", m_name);
			breath(Ind, m_idx, GF_MANA,
			       (rlev * 5) + damroll(10, 10));
			break;
		}

		/* RF5_BA_DARK */
		case 128+8:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles powerfully.", m_name);
			else msg_format(Ind, "%^s invokes a darkness storm.", m_name);
			breath(Ind, m_idx, GF_DARK,
			       (rlev * 5) + damroll(10, 10));
			update_smart_learn(m_idx, DRS_DARK);
			break;
		}

		/* RF5_DRAIN_MANA */
		case 128+9:
		{
			if (!direct) break;
			if (p_ptr->csp)
			{
				int r1;

				/* Disturb if legal */
				disturb(Ind, 1, 0);

				/* Basic message */
				msg_format(Ind, "%^s draws psychic energy from you!", m_name);

				/* Attack power */
				r1 = (randint(rlev) / 2) + 1;

				/* Full drain */
				if (r1 >= p_ptr->csp)
				{
					r1 = p_ptr->csp;
					p_ptr->csp = 0;
					p_ptr->csp_frac = 0;
				}

				/* Partial drain */
				else
				{
					p_ptr->csp -= r1;
				}

				/* Redraw mana */
				p_ptr->redraw |= (PR_MANA);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER);

				/* Heal the monster */
				if (m_ptr->hp < m_ptr->maxhp)
				{
					/* Heal */
					m_ptr->hp += (6 * r1);
					if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

					/* Redraw (later) if needed */
					update_health(m_idx);

					/* Special message */
					if (seen)
					{
						msg_format(Ind, "%^s appears healthier.", m_name);
					}
				}
			}
			update_smart_learn(m_idx, DRS_MANA);
			break;
		}

		/* RF5_MIND_BLAST */
		case 128+10:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (!seen)
			{
				msg_print(Ind, "You feel something focusing on your mind.");
			}
			else
			{
				msg_format(Ind, "%^s gazes deep into your eyes.", m_name);
			}

			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				msg_print(Ind, "Your mind is blasted by psionic energy.");
				if (!p_ptr->resist_conf)
				{
					(void)set_confused(Ind, p_ptr->confused + rand_int(4) + 4);
				}
				take_hit(Ind, damroll(8, 8), ddesc);
			}
			break;
		}

		/* RF5_BRAIN_SMASH */
		case 128+11:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (!seen)
			{
				msg_print(Ind, "You feel something focusing on your mind.");
			}
			else
			{
				msg_format(Ind, "%^s looks deep into your eyes.", m_name);
			}
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				msg_print(Ind, "Your mind is blasted by psionic energy.");
				take_hit(Ind, damroll(12, 15), ddesc);
				if (!p_ptr->resist_blind)
				{
					(void)set_blind(Ind, p_ptr->blind + 8 + rand_int(8));
				}
				if (!p_ptr->resist_conf)
				{
					(void)set_confused(Ind, p_ptr->confused + rand_int(4) + 4);
				}
				if (!p_ptr->free_act)
				{
					(void)set_paralyzed(Ind, p_ptr->paralyzed + rand_int(4) + 4);
				}
				(void)set_slow(Ind, p_ptr->slow + rand_int(4) + 4);
			}
			break;
		}

		/* RF5_CAUSE_1 */
		case 128+12:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s points at you and curses.", m_name);
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				take_hit(Ind, damroll(3, 8), ddesc);
			}
			break;
		}

		/* RF5_CAUSE_2 */
		case 128+13:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s points at you and curses horribly.", m_name);
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				take_hit(Ind, damroll(8, 8), ddesc);
			}
			break;
		}

		/* RF5_CAUSE_3 */
		case 128+14:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles loudly.", m_name);
			else msg_format(Ind, "%^s points at you, incanting terribly!", m_name);
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				take_hit(Ind, damroll(10, 15), ddesc);
			}
			break;
		}

		/* RF5_CAUSE_4 */
		case 128+15:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s screams the word 'DIE!'", m_name);
			else msg_format(Ind, "%^s points at you, screaming the word DIE!", m_name);
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				take_hit(Ind, damroll(15, 15), ddesc);
				(void)set_cut(Ind, p_ptr->cut + damroll(10, 10));
			}
			break;
		}

		/* RF5_BO_ACID */
		case 128+16:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a acid bolt.", m_name);
			bolt(Ind, m_idx, GF_ACID,
			     damroll(7, 8) + (rlev / 3));
			update_smart_learn(m_idx, DRS_ACID);
			break;
		}

		/* RF5_BO_ELEC */
		case 128+17:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a lightning bolt.", m_name);
			bolt(Ind, m_idx, GF_ELEC,
			     damroll(4, 8) + (rlev / 3));
			update_smart_learn(m_idx, DRS_ELEC);
			break;
		}

		/* RF5_BO_FIRE */
		case 128+18:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a fire bolt.", m_name);
			bolt(Ind, m_idx, GF_FIRE,
			     damroll(9, 8) + (rlev / 3));
			update_smart_learn(m_idx, DRS_FIRE);
			break;
		}

		/* RF5_BO_COLD */
		case 128+19:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a frost bolt.", m_name);
			bolt(Ind, m_idx, GF_COLD,
			     damroll(6, 8) + (rlev / 3));
			update_smart_learn(m_idx, DRS_COLD);
			break;
		}

		/* RF5_BO_POIS */
		case 128+20:
		{
			/* XXX XXX XXX */
			break;
		}

		/* RF5_BO_NETH */
		case 128+21:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a nether bolt.", m_name);
			bolt(Ind, m_idx, GF_NETHER,
			     30 + damroll(5, 5) + (rlev * 3) / 2);
			update_smart_learn(m_idx, DRS_NETH);
			break;
		}

		/* RF5_BO_WATE */
		case 128+22:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a water bolt.", m_name);
			bolt(Ind, m_idx, GF_WATER,
			     damroll(10, 10) + (rlev));
			break;
		}

		/* RF5_BO_MANA */
		case 128+23:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a mana bolt.", m_name);
			bolt(Ind, m_idx, GF_MANA,
			     randint(rlev * 7 / 2) + 50);
			break;
		}

		/* RF5_BO_PLAS */
		case 128+24:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a plasma bolt.", m_name);
			bolt(Ind, m_idx, GF_PLASMA,
			     10 + damroll(8, 7) + (rlev));
			break;
		}

		/* RF5_BO_ICEE */
		case 128+25:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts an ice bolt.", m_name);
			bolt(Ind, m_idx, GF_ICE,
			     damroll(6, 6) + (rlev));
			update_smart_learn(m_idx, DRS_COLD);
			break;
		}

		/* RF5_MISSILE */
		case 128+26:
		{
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a magic missile.", m_name);
			bolt(Ind, m_idx, GF_MISSILE,
			     damroll(2, 6) + (rlev / 3));
			break;
		}

		/* RF5_SCARE */
		case 128+27:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			sound(Ind, MSG_CAST_FEAR);
			if (blind) msg_format(Ind, "%^s mumbles, and you hear scary noises.", m_name);
			else msg_format(Ind, "%^s casts a fearful illusion.", m_name);
			if (p_ptr->resist_fear)
			{
				msg_print(Ind, "You refuse to be frightened.");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You refuse to be frightened.");
			}
			else
			{
				(void)set_afraid(Ind, p_ptr->afraid + rand_int(4) + 4);
			}
			update_smart_learn(m_idx, DRS_FEAR);
			break;
		}

		/* RF5_BLIND */
		case 128+28:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s casts a spell, burning your eyes!", m_name);
			if (p_ptr->resist_blind)
			{
				msg_print(Ind, "You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				(void)set_blind(Ind, 12 + rand_int(4));
			}
			update_smart_learn(m_idx, DRS_BLIND);
			break;
		}

		/* RF5_CONF */
		case 128+29:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles, and you hear puzzling noises.", m_name);
			else msg_format(Ind, "%^s creates a mesmerising illusion.", m_name);
			if (p_ptr->resist_conf)
			{
				msg_print(Ind, "You disbelieve the feeble spell.");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You disbelieve the feeble spell.");
			}
			else
			{
				(void)set_confused(Ind, p_ptr->confused + rand_int(4) + 4);
			}
			update_smart_learn(m_idx, DRS_CONF);
			break;
		}

		/* RF5_SLOW */
		case 128+30:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s drains power from your muscles!", m_name);
			if (p_ptr->free_act)
			{
				msg_print(Ind, "You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				(void)set_slow(Ind, p_ptr->slow + rand_int(4) + 4);
			}
			update_smart_learn(m_idx, DRS_FREE);
			break;
		}

		/* RF5_HOLD */
		case 128+31:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s stares deep into your eyes!", m_name);
			if (p_ptr->free_act)
			{
				msg_print(Ind, "You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_format(Ind, "You resist the effects!");
			}
			else
			{
				(void)set_paralyzed(Ind, p_ptr->paralyzed + rand_int(4) + 4);
			}
			update_smart_learn(m_idx, DRS_FREE);
			break;
		}



		/* RF6_HASTE */
		case 160+0:
		{
			disturb(Ind, 1, 0);
			if (blind)
			{
				msg_format(Ind, "%^s mumbles.", m_name);
			}
			else
			{
				msg_format(Ind, "%^s concentrates on %s body.", m_name, m_poss);
			}

			/* Allow quick speed increases to base+10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				msg_format(Ind, "%^s starts moving faster.", m_name);
				m_ptr->mspeed += 10;
			}

			/* Allow small speed increases to base+20 */
			else if (m_ptr->mspeed < r_ptr->speed + 20)
			{
				msg_format(Ind, "%^s starts moving faster.", m_name);
				m_ptr->mspeed += 2;
			}

			break;
		}

		/* RF6_XXX1X6 */
		case 160+1:
		{
			break;
		}

		/* RF6_HEAL */
		case 160+2:
		{
			/* No stupid message when at full health */ 
			if (m_ptr->hp == m_ptr->maxhp) break;
			
			disturb(Ind, 1, 0);

			/* Message */
			if (blind)
			{
				msg_format(Ind, "%^s mumbles.", m_name);
			}
			else
			{
				msg_format(Ind, "%^s concentrates on %s wounds.", m_name, m_poss);
			}

			/* Heal some */
			m_ptr->hp += (rlev * 6);

			/* Fully healed */
			if (m_ptr->hp >= m_ptr->maxhp)
			{
				/* Fully healed */
				m_ptr->hp = m_ptr->maxhp;

				/* Message */
				if (seen)
				{
					msg_format(Ind, "%^s looks REALLY healthy!", m_name);
				}
				else
				{
					msg_format(Ind, "%^s sounds REALLY healthy!", m_name);
				}
			}

			/* Partially healed */
			else
			{
				/* Message */
				if (seen)
				{
					msg_format(Ind, "%^s looks healthier.", m_name);
				}
				else
				{
					msg_format(Ind, "%^s sounds healthier.", m_name);
				}
			}

			/* Redraw (later) if needed */
			update_health(m_idx);

			/* Cancel fear */
			if (m_ptr->monfear)
			{
				/* Cancel fear */
				m_ptr->monfear = 0;

				/* Message */
				msg_format(Ind, "%^s recovers %s courage.", m_name, m_poss);
			}

			break;
		}

		/* RF6_XXX2X6 */
		case 160+3:
		{
			break;
		}

		/* RF6_BLINK */
		case 160+4:
		{
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s blinks away.", m_name);
			teleport_away(m_idx, 10);
			break;
		}

		/* RF6_TPORT */
		case 160+5:
		{
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s teleports away.", m_name);
			teleport_away(m_idx, MAX_SIGHT * 2 + 5);
			break;
		}

		/* RF6_XXX3X6 */
		case 160+6:
		{
			break;
		}

		/* RF6_XXX4X6 */
		case 160+7:
		{
			break;
		}

		/* RF6_TELE_TO */
		case 160+8:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s commands you to return.", m_name);
			teleport_player_to(Ind, m_ptr->fy, m_ptr->fx);
			break;
		}

		/* RF6_TELE_AWAY */
		case 160+9:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s teleports you away.", m_name);
			teleport_player(Ind, 100);
			break;
		}

		/* RF6_TELE_LEVEL */
		case 160+10:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles strangely.", m_name);
			else msg_format(Ind, "%^s gestures at your feet.", m_name);
			if (p_ptr->resist_nexus)
			{
				msg_print(Ind, "You are unaffected!");
			}
			else if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else
			{
				teleport_player_level(Ind);
			}
			update_smart_learn(m_idx, DRS_NEXUS);
			break;
		}

		/* RF6_XXX5 */
		case 160+11:
		{
			break;
		}

		/* RF6_DARKNESS */
		case 160+12:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s gestures in shadow.", m_name);
			(void)unlite_area(Ind, 0, 3);
			break;
		}

		/* RF6_TRAPS */
		case 160+13:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			sound(Ind, MSG_CREATE_TRAP);
			if (blind) msg_format(Ind, "%^s mumbles, and then cackles evilly.", m_name);
			else msg_format(Ind, "%^s casts a spell and cackles evilly.", m_name);
			(void)trap_creation(Ind);
			break;
		}

		/* RF6_FORGET */
		case 160+14:
		{
			if (!direct) break;
			disturb(Ind, 1, 0);
			msg_format(Ind, "%^s tries to blank your mind.", m_name);

			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print(Ind, "You resist the effects!");
			}
			else if (lose_all_info(Ind))
			{
				msg_print(Ind, "Your memories fade away.");
			}
			break;
		}

		/* RF6_XXX6X6 */
		case 160+15:
		{
			break;
		}

		/* RF6_S_KIN */
		case 160+16:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_MONSTER);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons %s %s.", m_name, m_poss,
			                ((r_ptr->flags1) & RF1_UNIQUE ?
			                 "minions" : "kin"));

			/* Hack -- Set the letter of the monsters to summon */
			summon_kin_type = r_ptr->d_char;
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_KIN);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many things appear nearby.");
			}
			break;
		}

		/* RF6_S_HI_DEMON */
		case 160+17:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_HI_DEMON);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons greater demons!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HI_DEMON);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many evil things appear nearby.");
			}
			break;
		}

		/* RF6_S_MONSTER */
		case 160+18:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_MONSTER);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons help!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(Depth, y, x, rlev, 0);
			}
			if (blind && count) msg_print(Ind, "You hear something appear nearby.");
			break;
		}

		/* RF6_S_MONSTERS */
		case 160+19:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_MONSTER);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons monsters!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, 0);
			}
			if (blind && count) msg_print(Ind, "You hear many things appear nearby.");
			break;
		}

		/* RF6_S_ANIMAL */
		case 160+20:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_ANIMAL);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons animals.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_ANIMAL);
			}
			if (blind && count) msg_print(Ind, "You hear many things appear nearby.");
			break;
		}

		/* RF6_S_SPIDER */
		case 160+21:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_SPIDER);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons spiders.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_SPIDER);
			}
			if (blind && count) msg_print(Ind, "You hear many things appear nearby.");
			break;
		}

		/* RF6_S_HOUND */
		case 160+22:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_HOUND);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons hounds.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HOUND);
			}
			if (blind && count) msg_print(Ind, "You hear many things appear nearby.");
			break;
		}

		/* RF6_S_HYDRA */
		case 160+23:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_HYDRA);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons hydras.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HYDRA);
			}
			if (blind && count) msg_print(Ind, "You hear many things appear nearby.");
			break;
		}

		/* RF6_S_ANGEL */
		case 160+24:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_ANGEL);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons an angel!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_ANGEL);
			}
			if (blind && count) msg_print(Ind, "You hear something appear nearby.");
			break;
		}

		/* RF6_S_DEMON */
		case 160+25:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_DEMON);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons a hellish adversary!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_DEMON);
			}
			if (blind && count) msg_print(Ind, "You hear something appear nearby.");
			break;
		}

		/* RF6_S_UNDEAD */
		case 160+26:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_UNDEAD);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons an undead adversary!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_UNDEAD);
			}
			if (blind && count) msg_print(Ind, "You hear something appear nearby.");
			break;
		}

		/* RF6_S_DRAGON */
		case 160+27:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_DRAGON);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons a dragon!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_DRAGON);
			}
			if (blind && count) msg_print(Ind, "You hear something appear nearby.");
			break;
		}

		/* RF6_S_HI_UNDEAD */
		case 160+28:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_HI_UNDEAD);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons greater undead!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many creepy things appear nearby.");
			}
			break;
		}

		/* RF6_S_HI_DRAGON */
		case 160+29:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_HI_DRAGON);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons ancient dragons!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HI_DRAGON);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many powerful things appear nearby.");
			}
			break;
		}

		/* RF6_S_WRAITH */
		case 160+30:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_WRAITH);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons mighty undead opponents!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_WRAITH);
			}
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many creepy things appear nearby.");
			}
			break;
		}

		/* RF6_S_UNIQUE */
		case 160+31:
		{
			disturb(Ind, 1, 0);
			sound(Ind, MSG_SUM_UNIQUE);
			if (blind) msg_format(Ind, "%^s mumbles.", m_name);
			else msg_format(Ind, "%^s magically summons special opponents!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_UNIQUE);
			}
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(Depth, y, x, rlev, SUMMON_HI_UNDEAD);
			}
			if (blind && count)
			{
				msg_print(Ind, "You hear many powerful things appear nearby.");
			}
			break;
		}
	}


	/* Remember what the monster did to us */
	if (seen)
	{
		/* Inate spell */
		if (thrown_spell < 32*4)
		{
			l_ptr->flags4 |= (1L << (thrown_spell - 32*3));
			if (l_ptr->cast_innate < MAX_UCHAR) l_ptr->cast_innate++;
		}

		/* Bolt or Ball */
		else if (thrown_spell < 32*5)
		{
			l_ptr->flags5 |= (1L << (thrown_spell - 32*4));
			if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}

		/* Special spell */
		else if (thrown_spell < 32*6)
		{
			l_ptr->flags6 |= (1L << (thrown_spell - 32*5));
			if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}
	}


	/* Always take note of monsters that kill you */
	if (p_ptr->death && (l_ptr->deaths < MAX_SHORT)) l_ptr->deaths++;


	/* A spell was cast */
	return (TRUE);
}


/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency.  Like not using "mod" or "div" when possible.  And
 * attempting to check the conditions in an optimal order.  Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];

	monster_type *m_ptr = &m_list[m_idx];

#ifdef ALLOW_TERROR

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

#endif

	/* Keep monsters from running too far away */
	if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

	/* All "afraid" monsters will run away */
	if (m_ptr->monfear) return (TRUE);

#ifdef ALLOW_TERROR

	/* Nearby monsters will not become terrified */
	if (m_ptr->cdis <= 5) return (FALSE);

	/* Examine player power (level) */
	p_lev = p_ptr->lev;

	/* Examine monster power (level plus morale) */
	m_lev = r_ptr->level + (m_idx & 0x08) + 25;

	/* Optimize extreme cases below */
	if (m_lev > p_lev + 4) return (FALSE);
	if (m_lev + 4 <= p_lev) return (TRUE);

	/* Examine player health */
	p_chp = p_ptr->chp;
	p_mhp = p_ptr->mhp;

	/* Examine monster health */
	m_chp = m_ptr->hp;
	m_mhp = m_ptr->maxhp;

	/* Prepare to optimize the calculation */
	p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
	m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

	/* Strong players scare strong monsters */
	if (p_val * m_mhp > m_val * p_mhp) return (TRUE);

#endif

	/* Assume no terror */
	return (FALSE);
}




#ifdef MONSTER_FLOW

/*
 * Choose the "best" direction for "flowing"
 *
 * Note that ghosts and rock-eaters are never allowed to "flow",
 * since they should move directly towards the player.
 *
 * Prefer "non-diagonal" directions, but twiddle them a little
 * to angle slightly towards the player's actual location.
 *
 * Allow very perceptive monsters to track old "spoor" left by
 * previous locations occupied by the player.  This will tend
 * to have monsters end up either near the player or on a grid
 * recently occupied by the player (and left via "teleport").
 *
 * Note that if "smell" is turned on, all monsters get vicious.
 *
 * Also note that teleporting away from a location will cause
 * the monsters who were chasing you to converge on that location
 * as long as you are still near enough to "annoy" them without
 * being close enough to chase directly.  I have no idea what will
 * happen if you combine "smell" with low "aaf" values.
 */
static bool get_moves_aux(int m_idx, int *yp, int *xp)
{
	int i, y, x, y1, x1, when = 0, cost = 999;

	cave_type *c_ptr;

	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Monster flowing disabled */
	if (!flow_by_sound) return (FALSE);

	/* Monster can go through rocks */
	if (r_ptr->flags2 & RF2_PASS_WALL) return (FALSE);
	if (r_ptr->flags2 & RF2_KILL_WALL) return (FALSE);

	/* Monster location */
	y1 = m_ptr->fy;
	x1 = m_ptr->fx;

	/* Monster grid */
	c_ptr = &cave[y1][x1];

	/* The player is not currently near the monster grid */
	if (c_ptr->when < cave[py][px].when)
	{
		/* The player has never been near the monster grid */
		if (!c_ptr->when) return (FALSE);

		/* The monster is not allowed to track the player */
		if (!flow_by_smell) return (FALSE);
	}

	/* Monster is too far away to notice the player */
	if (c_ptr->cost > MONSTER_FLOW_DEPTH) return (FALSE);
	if (c_ptr->cost > r_ptr->aaf) return (FALSE);

	/* Hack -- Player can see us, run towards him */
	if (player_has_los_bold(y1, x1)) return (FALSE);

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		/* Get the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Ignore illegal locations */
		if (!cave[y][x].when) continue;

		/* Ignore ancient locations */
		if (cave[y][x].when < when) continue;

		/* Ignore distant locations */
		if (cave[y][x].cost > cost) continue;

		/* Save the cost and time */
		when = cave[y][x].when;
		cost = cave[y][x].cost;

		/* Hack -- Save the "twiddled" location */
		(*yp) = py + 16 * ddy_ddd[i];
		(*xp) = px + 16 * ddx_ddd[i];
	}

	/* No legal move (?) */
	if (!when) return (FALSE);

	/* Success */
	return (TRUE);
}

#endif


/*
 * Choose "logical" directions for monster movement
 */
static void get_moves(int Ind, int m_idx, int *mm)
{
	player_type *p_ptr = Players[Ind];

	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int y, ay, x, ax;

	int move_val = 0;
	
	int y2 = p_ptr->py;
	int x2 = p_ptr->px;

	/* Wanderers have their own ideas about where they are going unless
	 * the player is aggravating them or they are hurt and standing near
	 * a player who is higher than level 1.
	 */
	if ( (r_ptr->flags2 & RF2_WANDERER) && (!p_ptr->aggravate) 
	   && ( (m_ptr->hp == m_ptr->maxhp) || (p_ptr->lev == 1) ) )
	{
		x2 = y2 = 0;
		
		/* Do we know where we are going? */
		if ( m_ptr->wx || m_ptr->wy )
		{
			/* Yes, set that as our target */
			x2 = m_ptr->wx;
			y2 = m_ptr->wy;
			
			/* If we have nearly arrived, go somewhere else */
			if ( (abs(x2 - m_ptr->fx) < 10) && (abs(y2 - m_ptr->fy) < 10) )
			{
				x2 = y2 = 0;
			}

			/* Occasionally we change our mind about where we want to go */
			if (randint(1000) < 10)
			{
				x2 = y2 = 0;
			}
		}
		if ( !x2 && !y2 )
		{
			/* We don't know where we're going, pick a destination */
			x2 = m_ptr->wx = randint(MAX_WID);
			y2 = m_ptr->wy = randint(MAX_HGT);
		}
	}


#ifdef MONSTER_FLOW
	/* Flow towards the player */
	if (flow_by_sound)
	{
		/* Flow towards the player */
		(void)get_moves_aux(m_idx, &y2, &x2);
	}
#endif

	/* Extract the "pseudo-direction" */
	y = m_ptr->fy - y2;
	x = m_ptr->fx - x2;


	/* Apply fear if possible and necessary (wanderers never run away) */
	if ( mon_will_run(Ind, m_idx) && !(r_ptr->flags2 & RF2_WANDERER) )
	{
		/* XXX XXX Not very "smart" */
		y = (-y), x = (-x);
	}


	/* Extract the "absolute distances" */
	ax = ABS(x);
	ay = ABS(y);

	/* Do something weird */
	if (y < 0) move_val += 8;
	if (x > 0) move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1))
	{
		move_val++;
		move_val++;
	}
	else if (ax > (ay << 1))
	{
		move_val++;
	}

	/* Extract some directions */
	switch (move_val)
	{
		case 0:
		mm[0] = 9;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 6;
			mm[3] = 7;
			mm[4] = 3;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 8;
			mm[3] = 3;
			mm[4] = 7;
		}
		break;
		case 1:
		case 9:
		mm[0] = 6;
		if (y < 0)
		{
			mm[1] = 3;
			mm[2] = 9;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 9;
			mm[2] = 3;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
		case 2:
		case 6:
		mm[0] = 8;
		if (x < 0)
		{
			mm[1] = 9;
			mm[2] = 7;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 9;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
		case 4:
		mm[0] = 7;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 4;
			mm[3] = 9;
			mm[4] = 1;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 8;
			mm[3] = 1;
			mm[4] = 9;
		}
		break;
		case 5:
		case 13:
		mm[0] = 4;
		if (y < 0)
		{
			mm[1] = 1;
			mm[2] = 7;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 1;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
		case 8:
		mm[0] = 3;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 6;
			mm[3] = 1;
			mm[4] = 9;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 2;
			mm[3] = 9;
			mm[4] = 1;
		}
		break;
		case 10:
		case 14:
		mm[0] = 2;
		if (x < 0)
		{
			mm[1] = 3;
			mm[2] = 1;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 1;
			mm[2] = 3;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
		case 12:
		mm[0] = 1;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 4;
			mm[3] = 3;
			mm[4] = 7;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 2;
			mm[3] = 7;
			mm[4] = 3;
		}
		break;
	}
}



/*
 * Hack -- local "player stealth" value (see below)
 */
static u32b noise = 0L;


/*
 * Process a monster
 *
 * The monster is known to be within 100 grids of the player
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level.  This should prevent the level from being "swamped" by
 * reproducing monsters.  It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * XXX XXX XXX In addition, monsters which *cannot* open or bash
 * down a door will still stand there trying to open it...
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * A "direction" of "5" means "pick a random direction".
 *
 * Note that the "Ind" specifies the player that the monster will go after.
 */

static void process_monster(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	monster_type	*m_ptr = &m_list[m_idx];
	monster_race	*r_ptr = &r_info[m_ptr->r_idx];
	monster_lore	*l_ptr = p_ptr->l_list + m_ptr->r_idx;

	int			i, d, oy, ox, ny, nx;

	int			mm[8];

	cave_type    	*c_ptr;
	object_type 	*o_ptr;
	monster_type	*y_ptr;

	bool		do_turn;
	bool		do_move;
	bool		do_view;

	bool		did_open_door;
	bool		did_bash_door;
	bool		did_take_item;
	bool		did_kill_item;
	bool		did_move_body;
	bool		did_kill_body;
	bool		did_pass_wall;
	bool		did_kill_wall;


	/* Hack -- don't process monsters on wilderness levels that have not
	   been regenerated yet.
	*/
	if (cave[Depth] == NULL) return;

	/* Handle "sleep" */
	if (m_ptr->csleep)
	{
		u32b notice = 0;

		/* Hack -- handle non-aggravation */
		if (!p_ptr->aggravate) notice = rand_int(1024);

		/* Hack -- See if monster "notices" player */
		if ((notice * notice * notice) <= noise)
		{
			/* Hack -- amount of "waking" */
			int d = 1;

			/* Hack -- make sure the distance isn't zero */
			if (m_ptr->cdis == 0) m_ptr->cdis = 1;

			/* Wake up faster near the player */
			if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

			/* Hack -- handle aggravation */
			if (p_ptr->aggravate) d = m_ptr->csleep;

			/* Still asleep */
			if (m_ptr->csleep > d)
			{
				/* Monster wakes up "a little bit" */
				m_ptr->csleep -= d;

				/* Notice the "not waking up" */
				if (p_ptr->mon_vis[m_idx])
				{
					/* Hack -- Count the ignores */
					if (l_ptr->ignore < MAX_UCHAR) l_ptr->ignore++;
				}
			}

			/* Just woke up */
			else
			{
				/* Reset sleep counter */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (p_ptr->mon_vis[m_idx])
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(Ind, m_name, m_idx, 0);

					/* Dump a message */
					msg_format(Ind, "%^s wakes up.", m_name);

					/* Hack -- Count the wakings */
					if (l_ptr->wake < MAX_UCHAR) l_ptr->wake++;
				}
			}
		}

		/* Still sleeping */
		if (m_ptr->csleep) 
		{
			return;
		}
	}


	/* Handle "stun" */
	if (m_ptr->stunned)
	{
		int d = 1;

		/* Make a "saving throw" against stun */
		if (rand_int(5000) <= r_ptr->level * r_ptr->level)
		{
			/* Recover fully */
			d = m_ptr->stunned;
		}

		/* Hack -- Recover from stun */
		if (m_ptr->stunned > d)
		{
			/* Recover somewhat */
			m_ptr->stunned -= d;
		}

		/* Fully recover */
		else
		{
			/* Recover fully */
			m_ptr->stunned = 0;

			/* Message if visible */
			if (p_ptr->mon_vis[m_idx])
			{
				char m_name[80];

				/* Acquire the monster name */
				monster_desc(Ind, m_name, m_idx, 0);

				/* Dump a message */
				msg_format(Ind, "%^s is no longer stunned.", m_name);
			}
		}

		/* Still stunned */
		if (m_ptr->stunned) 
		{
			return;
		}
	}


	/* Handle confusion */
	if (m_ptr->confused)
	{
		/* Amount of "boldness" */
		int d = randint(r_ptr->level / 10 + 1);

		/* Still confused */
		if (m_ptr->confused > d)
		{
			/* Reduce the confusion */
			m_ptr->confused -= d;
		}

		/* Recovered */
		else
		{
			/* No longer confused */
			m_ptr->confused = 0;

			/* Message if visible */
			if (p_ptr->mon_vis[m_idx])
			{
				char m_name[80];

				/* Acquire the monster name */
				monster_desc(Ind, m_name, m_idx, 0);

				/* Dump a message */
				msg_format(Ind, "%^s is no longer confused.", m_name);
			}
		}
	}

	/* Handle "fear" */
	if (m_ptr->monfear)
	{
		/* Amount of "boldness" */
		int d = randint(r_ptr->level / 10 + 1);

		/* Still afraid */
		if (m_ptr->monfear > d)
		{
			/* Reduce the fear */
			m_ptr->monfear -= d;
		}

		/* Recover from fear, take note if seen */
		else
		{
			/* No longer afraid */
			m_ptr->monfear = 0;

			/* Visual note */
			if (p_ptr->mon_vis[m_idx])
			{
				char m_name[80];
				char m_poss[80];

				/* Acquire the monster name/poss */
				monster_desc(Ind, m_name, m_idx, 0);
				monster_desc(Ind, m_poss, m_idx, 0x22);

				/* Dump a message */
				msg_format(Ind, "%^s recovers %s courage.", m_name, m_poss);
			}
		}
	}

	/* Get the origin */
	oy = m_ptr->fy;
	ox = m_ptr->fx;


	/* attempt to "mutiply" if able and allowed */

	/* Hack -- No reproduction in the wilderness */
	if (Depth > 0)
		if ((r_ptr->flags2 & RF2_MULTIPLY) && (num_repro < MAX_REPRO))
		{
			int k, y, x;

			/* Count the adjacent monsters */
			for (k = 0, y = oy - 1; y <= oy + 1; y++)
			{
				for (x = ox - 1; x <= ox + 1; x++)
				{
					if (cave[Depth][y][x].m_idx > 0) k++;
				}
			}

			/* Hack -- multiply slower in crowded areas */
			if ((k < 4) && (!k || !rand_int(k * MON_MULT_ADJ)))
			{
				/* Try to multiply */
				if (multiply_monster(m_idx))
				{
					/* Take note if visible */
					if (p_ptr->mon_vis[m_idx])
                    {
                        l_ptr->flags2 |= RF2_MULTIPLY;

                        /* Make a sound */
						sound(Ind, MSG_MULTIPLY);
                    }

					return;
				}
			}
		}
						
	/* Attempt to cast a spell */
	if (make_attack_spell(Ind, m_idx)) 
	{
		return;
	}


	/* Hack -- Assume no movement */
	mm[0] = mm[1] = mm[2] = mm[3] = 0;
	mm[4] = mm[5] = mm[6] = mm[7] = 0;


	/* Confused -- 100% random */
	if (m_ptr->confused)
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 75% random movement */
	else if ((r_ptr->flags1 & RF1_RAND_50) &&
	         (r_ptr->flags1 & RF1_RAND_25) &&
	         (rand_int(100) < 75))
	{
		/* Memorize flags */
		if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_RAND_50;
		if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_RAND_25;

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 50% random movement */
	else if ((r_ptr->flags1 & RF1_RAND_50) &&
	         (rand_int(100) < 50))
	{
		/* Memorize flags */
		if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_RAND_50;

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 25% random movement */
	else if ((r_ptr->flags1 & RF1_RAND_25) &&
	         (rand_int(100) < 25))
	{
		/* Memorize flags */
		if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_RAND_25;

		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* Normal movement */
	else
	{
		/* Logical moves */
		get_moves(Ind, m_idx, mm);
	}


	/* Assume nothing */
	do_turn = FALSE;
	do_move = FALSE;
	do_view = FALSE;

	/* Assume nothing */
	did_open_door = FALSE;
	did_bash_door = FALSE;
	did_take_item = FALSE;
	did_kill_item = FALSE;
	did_move_body = FALSE;
	did_kill_body = FALSE;
	did_pass_wall = FALSE;
	did_kill_wall = FALSE;


	/* Take a zero-terminated array of "directions" */
	for (i = 0; mm[i]; i++)
	{
		/* Get the direction */
		d = mm[i];

		/* Hack -- allow "randomized" motion */
		if (d == 5) d = ddd[rand_int(8)];

		/* Get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];			
		
		/* Access that cave grid */
		c_ptr = &cave[Depth][ny][nx];

		/* Access that cave grid's contents */
		o_ptr = &o_list[c_ptr->o_idx];

		/* Access that cave grid's contents */
		y_ptr = &m_list[c_ptr->m_idx];


		/* Tavern entrance? PVP Arena? */
		if (c_ptr->feat == FEAT_SHOP_HEAD+7 || c_ptr->feat == FEAT_PVP_ARENA)
		{
			/* Nothing */
		}

		/* Floor is open? */
		else if (cave_floor_bold(Depth, ny, nx))
		{
			/* Go ahead and move */
			do_move = TRUE;
		}

		/* Player ghost in wall XXX */
		else if (c_ptr->m_idx < 0)
		{
			/* Move into player */
			do_move = TRUE;
		}

		/* Permanent wall */
		else if ( (c_ptr->feat >= FEAT_PERM_EXTRA) || (c_ptr->feat == FEAT_PERM_CLEAR) )
		{
			/* Nothing */
		}

		/* Monster moves through walls (and doors) */
		else if (r_ptr->flags2 & RF2_PASS_WALL)
		{
			/* Pass through walls/doors/rubble */
			do_move = TRUE;

			/* Monster went through a wall */
			did_pass_wall = TRUE;
		}

		/* Monster destroys walls (and doors) */
		else if (r_ptr->flags2 & RF2_KILL_WALL)
		{
			/* Eat through walls/doors/rubble */
			do_move = TRUE;

			/* Monster destroyed a wall */
			did_kill_wall = TRUE;

			/* Create floor */
			c_ptr->feat = FEAT_FLOOR;

			/* Forget the "field mark", if any */
			everyone_forget_spot(Depth, ny, nx);

			/* Notice */
			note_spot_depth(Depth, ny, nx);

			/* Redraw */
			everyone_lite_spot(Depth, ny, nx);

			/* Note changes to viewable region */
			if (player_has_los_bold(Ind, ny, nx)) do_view = TRUE;
		}

		/* Handle doors and secret doors */
		else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		          (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
		         (c_ptr->feat == FEAT_SECRET))
		{
			bool may_bash = TRUE;

			/* Take a turn */
			do_turn = TRUE;

			/* Creature can open doors. */
			if (r_ptr->flags2 & RF2_OPEN_DOOR)
			{
				/* Closed doors and secret doors */
				if ((c_ptr->feat == FEAT_DOOR_HEAD) ||
				    (c_ptr->feat == FEAT_SECRET))
				{
					/* The door is open */
					did_open_door = TRUE;

					/* Do not bash the door */
					may_bash = FALSE;
				}

				/* Locked doors (not jammed) */
				else if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08)
				{
					int k;

					/* Door power */
					k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

#if 0
					/* XXX XXX XXX XXX Old test (pval 10 to 20) */
					if (randint((m_ptr->hp + 1) * (50 + o_ptr->pval)) <
					    40 * (m_ptr->hp - 10 - o_ptr->pval));
#endif

					/* Try to unlock it XXX XXX XXX */
					if (rand_int(m_ptr->hp / 10) > k)
					{
						/* Unlock the door */
						c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

						/* Do not bash the door */
						may_bash = FALSE;
					}
				}
			}

			/* Stuck doors -- attempt to bash them down if allowed */
			if (may_bash && (r_ptr->flags2 & RF2_BASH_DOOR))
			{
				int k;

				/* Door power */
				k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

#if 0
				/* XXX XXX XXX XXX Old test (pval 10 to 20) */
				if (randint((m_ptr->hp + 1) * (50 + o_ptr->pval)) <
				    40 * (m_ptr->hp - 10 - o_ptr->pval));
#endif

				/* Attempt to Bash XXX XXX XXX */
				if (rand_int(m_ptr->hp / 10) > k)
				{
					/* Message */
					msg_print(Ind, "You hear a door burst open!");

					/* Disturb (sometimes) */
					if (option_p(p_ptr,DISTURB_MINOR)) disturb(Ind, 0, 0);

					/* The door was bashed open */
					did_bash_door = TRUE;

					/* Hack -- fall into doorway */
					do_move = TRUE;
				}
			}


			/* Deal with doors in the way */
			if (did_open_door || did_bash_door)
			{
				/* Break down the door */
				if (did_bash_door && (rand_int(100) < 50))
				{
					c_ptr->feat = FEAT_BROKEN;
				}

				/* Open the door */
				else
				{
					c_ptr->feat = FEAT_OPEN;
				}

				/* Notice */
				note_spot_depth(Depth, ny, nx);

				/* Redraw */
				everyone_lite_spot(Depth, ny, nx);

				/* Handle viewable doors */
				if (player_has_los_bold(Ind, ny, nx)) do_view = TRUE;
			}
		}


		/* Hack -- check for Glyph of Warding */
		if (do_move && (c_ptr->feat == FEAT_GLYPH))
		{
			/* Assume no move allowed */
			do_move = FALSE;

			/* Break the ward */
			if (randint(BREAK_GLYPH) < r_ptr->level)
			{
				/* Describe observable breakage */
				if (Players[Ind]->cave_flag[ny][nx] & CAVE_MARK)
				{
					msg_print(Ind, "The rune of protection is broken!");
				}

				/* Break the rune */
				c_ptr->feat = FEAT_FLOOR;

				/* Allow movement */
				do_move = TRUE;
			}
		}

		/* Some monsters never attack */
		if (do_move && (ny == p_ptr->py) && (nx == p_ptr->px) &&
		    (r_ptr->flags1 & RF1_NEVER_BLOW))
		{
			/* Hack -- memorize lack of attacks */
			if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_NEVER_BLOW;

			/* Do not move */
			do_move = FALSE;
		}


		/* The player is in the way.  Attack him. */
		if (do_move && (c_ptr->m_idx < 0))
		{
			/* Do the attack */
			(void)make_attack_normal(0 - c_ptr->m_idx, m_idx);

			/* Do not move */
			do_move = FALSE;

			/* Took a turn */
			do_turn = TRUE;
		}


		/* Some monsters never move */
		if (do_move && (r_ptr->flags1 & RF1_NEVER_MOVE))
		{
			/* Hack -- memorize lack of attacks */
			if (p_ptr->mon_vis[m_idx]) l_ptr->flags1 |= RF1_NEVER_MOVE;

			/* Do not move */
			do_move = FALSE;
		}


		/* A monster is in the way */
		if (do_move && c_ptr->m_idx > 0)
		{
			monster_race *z_ptr = &r_info[y_ptr->r_idx];

			/* Assume no movement */
			do_move = FALSE;

			/* Kill weaker monsters */
			if ((r_ptr->flags2 & RF2_KILL_BODY) &&
			    (r_ptr->mexp > z_ptr->mexp))
			{
				/* Allow movement */
				do_move = TRUE;

				/* Monster ate another monster */
				did_kill_body = TRUE;

				/* XXX XXX XXX Message */

				/* Kill the monster */
				delete_monster(Depth, ny, nx);

				/* Hack -- get the empty monster */
				y_ptr = &m_list[c_ptr->m_idx];
			}

			/* Push past weaker monsters (unless leaving a wall) */
			if ((r_ptr->flags2 & RF2_MOVE_BODY) &&
			    (r_ptr->mexp > z_ptr->mexp) &&
			    (cave_floor_bold(Depth, m_ptr->fy, m_ptr->fx)))
			{
				/* Allow movement */
				do_move = TRUE;

				/* Monster pushed past another monster */
				did_move_body = TRUE;

				/* XXX XXX XXX Message */
			}
		}


		/* Creature has been allowed move */
		if (do_move)
		{
			/* Take a turn */
			do_turn = TRUE;

			/* Hack -- Update the old location */
			cave[Depth][oy][ox].m_idx = c_ptr->m_idx;

			/* Mega-Hack -- move the old monster, if any */
			if (c_ptr->m_idx > 0)
			{
				/* Move the old monster */
				y_ptr->fy = oy;
				y_ptr->fx = ox;

				/* Update the old monster */
				update_mon(c_ptr->m_idx, TRUE);
			}

			/* Hack -- Update the new location */
			c_ptr->m_idx = m_idx;

			/* Move the monster */
			m_ptr->fy = ny;
			m_ptr->fx = nx;

			/* Update the monster */
			update_mon(m_idx, TRUE);

			/* Redraw the old grid */
			everyone_lite_spot(Depth, oy, ox);

			/* Redraw the new grid */
			everyone_lite_spot(Depth, ny, nx);

			/* Possible disturb */
			if (p_ptr->mon_vis[m_idx] &&
			    (option_p(p_ptr,DISTURB_MOVE) ||
			     (p_ptr->mon_los[m_idx] &&
			      option_p(p_ptr,DISTURB_NEAR))))
			{
				/* Disturb */
				disturb(Ind, 0, 0);
			}


			/* XXX XXX XXX Change for Angband 2.8.0 */

			/* Take or Kill objects (not "gold") on the floor */
			if (o_ptr->k_idx && (o_ptr->tval != TV_GOLD) &&
			    ((r_ptr->flags2 & RF2_TAKE_ITEM) ||
			     (r_ptr->flags2 & RF2_KILL_ITEM)))
			{
				u32b f1, f2, f3;

				u32b flg3 = 0L;

				char m_name[80];
				char o_name[80];

				/* Check the grid */
				o_ptr = &o_list[c_ptr->o_idx];

				/* Extract some flags */
		                object_flags(o_ptr, &f1, &f2, &f3);

				/* Acquire the object name */
				object_desc(Ind, o_name, o_ptr, TRUE, 3);

				/* Acquire the monster name */
				monster_desc(Ind, m_name, m_idx, 0x04);

				/* React to objects that hurt the monster */
				if (f1 & TR1_KILL_DRAGON) flg3 |= RF3_DRAGON;
				if (f1 & TR1_SLAY_DRAGON) flg3 |= RF3_DRAGON;
				if (f1 & TR1_SLAY_TROLL) flg3 |= RF3_TROLL;
				if (f1 & TR1_SLAY_GIANT) flg3 |= RF3_GIANT;
				if (f1 & TR1_SLAY_ORC) flg3 |= RF3_ORC;
                if (f1 & TR1_KILL_DEMON) flg3 |= RF3_DEMON;
				if (f1 & TR1_SLAY_DEMON) flg3 |= RF3_DEMON;
                if (f1 & TR1_KILL_UNDEAD) flg3 |= RF3_UNDEAD;
				if (f1 & TR1_SLAY_UNDEAD) flg3 |= RF3_UNDEAD;
				if (f1 & TR1_SLAY_ANIMAL) flg3 |= RF3_ANIMAL;
				if (f1 & TR1_SLAY_EVIL) flg3 |= RF3_EVIL;

				/* The object cannot be picked up by the monster */
				if (artifact_p(o_ptr) || (r_ptr->flags3 & flg3))
				{
					/* Only give a message for "take_item" */
					if (r_ptr->flags2 & RF2_TAKE_ITEM)
					{
						/* Take note */
						did_take_item = TRUE;

						/* Describe observable situations */
						if (p_ptr->mon_vis[m_idx] && player_has_los_bold(Ind, ny, nx))
						{
							/* Dump a message */
							msg_format(Ind, "%^s tries to pick up %s, but fails.",
							           m_name, o_name);
						}
					}
				}

				/* Pick up the item */
				else if (r_ptr->flags2 & RF2_TAKE_ITEM)
				{
					object_type *i_ptr;
					object_type object_type_body;
				
					/* Take note */
					did_take_item = TRUE;

					/* Describe observable situations */
					if (player_has_los_bold(Ind, ny, nx))
					{
						/* Dump a message */
						msg_format(Ind, "%^s picks up %s.", m_name, o_name);
					}

					if	(monster_can_carry(m_idx))
					{
						/* Prepare local object */
						i_ptr = &object_type_body;
						
						/* Obtain local object */
						COPY(i_ptr, o_ptr, object_type);
	
						/* Carry the object */
						(void)monster_carry(Ind, m_idx, i_ptr);
					}
										
					/* Delete the object */
					delete_object(Depth, ny, nx);
				}

				/* Destroy the item */
				else
				{
					/* Take note */
					did_kill_item = TRUE;

					/* Describe observable situations */
					if (player_has_los_bold(Ind, ny, nx))
					{
						/* Dump a message */
						msg_format(Ind, "%^s crushes %s.", m_name, o_name);
						sound(Ind, MSG_DESTROY);
					}
					
					/* Delete the object */
					delete_object(Depth, ny, nx);
				}
			}
		}

		/* Stop when done */
		if (do_turn) 
		{
			break;
		}
	}

	/* Notice changes in view */
	if (do_view)
	{
		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
	}


	/* Learn things from observable monster */
	if (p_ptr->mon_vis[m_idx])
	{
		/* Monster opened a door */
		if (did_open_door) l_ptr->flags2 |= RF2_OPEN_DOOR;

		/* Monster bashed a door */
		if (did_bash_door) l_ptr->flags2 |= RF2_BASH_DOOR;

		/* Monster tried to pick something up */
		if (did_take_item) l_ptr->flags2 |= RF2_TAKE_ITEM;

		/* Monster tried to crush something */
		if (did_kill_item) l_ptr->flags2 |= RF2_KILL_ITEM;

		/* Monster pushed past another monster */
		if (did_move_body) l_ptr->flags2 |= RF2_MOVE_BODY;

		/* Monster ate another monster */
		if (did_kill_body) l_ptr->flags2 |= RF2_KILL_BODY;

		/* Monster passed through a wall */
		if (did_pass_wall) l_ptr->flags2 |= RF2_PASS_WALL;

		/* Monster destroyed a wall */
		if (did_kill_wall) l_ptr->flags2 |= RF2_KILL_WALL;
	}


	/* Hack -- get "bold" if out of options */
	if (!do_turn && !do_move && m_ptr->monfear)
	{
		/* No longer afraid */
		m_ptr->monfear = 0;

		/* Message if seen */
		if (p_ptr->mon_vis[m_idx])
		{
			char m_name[80];

			/* Acquire the monster name */
			monster_desc(Ind, m_name, m_idx, 0);

			/* Dump a message */
			msg_format(Ind, "%^s turns to fight!", m_name);
		}

		/* XXX XXX XXX Actually do something now (?) */
	}
}




/*
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * Note that monsters can never move in the monster array (except when the
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",
 * especially when the player is running.
 *
 * Note the use of the new special "m_fast" array, which allows us to only
 * process monsters which are alive (or at least recently alive), which may
 * provide some optimization, especially when resting.  Note that monsters
 * which are only recently alive are excised, using a simple "excision"
 * method relying on the fact that the array is processed backwards.
 *
 * Note that "new" monsters are always added by "m_pop()" and they are
 * always added at the end of the "m_fast" array.
 */
 
 
void process_monsters(void)
{
	int			k, i, e, pl, bubble;
	int			fx, fy;

	bool		test;

	monster_type	*m_ptr;
	monster_race	*r_ptr;



	/* Process the monsters */
	for (k = m_top - 1; k >= 0; k--)
	{
		player_type *p_ptr;
		int closest = -1, dis_to_closest = 9999, lowhp = 9999;
		bool closest_in_los = FALSE;

		/* Access the index */
		i = m_fast[k];

		/* Access the monster */
		m_ptr = &m_list[i];


		/* Excise "dead" monsters */
		if (!m_ptr->r_idx)
		{
			/* Excise the monster */
			m_fast[k] = m_fast[--m_top];

			/* Skip */
			continue;
		}


		/* Find the closest player */
		for (pl = 1; pl < NumPlayers + 1; pl++)
		{
			int j;
			bool in_los;

			p_ptr = Players[pl];

			/* Hack -- notice death or departure */
			if (!p_ptr->alive || p_ptr->death || p_ptr->new_level_flag)
				continue;

			/* Make sure he's on the same dungeon level */
			if (p_ptr->dun_depth != m_ptr->dun_depth)
				continue;

			/* Hack -- Skip him if he's shopping */
			if (p_ptr->store_num != -1)
				continue;

			/* Hack -- make the dungeon master invisible to monsters */
			if (p_ptr->dm_flags & DM_MONSTER_FRIEND) continue;

			/* Compute distance */
			j = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

			/* Compute los */
			in_los = player_has_los_bold(pl, m_ptr->fy, m_ptr->fx);

			/* Skip if _not_ in LoS while closest _is_ in */
			if (!in_los && closest_in_los) continue;
			
			/* Only check distance if they share LoS properties */		
			else if (closest_in_los == in_los) 
			{
				/* Skip if further than closest */
				if (j > dis_to_closest) continue;
	
				/* Skip if same distance and stronger */
				if (j == dis_to_closest && p_ptr->chp > lowhp) continue;
			}
			/* Remember this player */
			dis_to_closest = j;
			closest = pl;
			lowhp = p_ptr->chp;
			closest_in_los = in_los;
		}

		/* Obtain the energy boost */
		e = extract_energy[m_ptr->mspeed];
		
		/* If we are within a players time bubble, scale our energy */
		if(closest > -1)
		{
			e = e * ((float)time_factor(closest) / 100);	
		}

		/* Give this monster some energy */
		m_ptr->energy += e;

		/* Make sure we don't store up too much energy */
		if (m_ptr->energy > level_speed(m_ptr->dun_depth))
			m_ptr->energy = level_speed(m_ptr->dun_depth);

		/* Not enough energy to move */
		if (m_ptr->energy < level_speed(m_ptr->dun_depth)) continue;
		
		/* Use some energy */
		m_ptr->energy -= level_speed(m_ptr->dun_depth);

		/* Paranoia -- Make sure we found a closest player */
		if (closest == -1)
			continue;

		p_ptr = Players[closest];

		/* Hack -- calculate the "player noise" */
		noise = (1L << (30 - p_ptr->skill_stl));

		m_ptr->cdis = dis_to_closest;
		m_ptr->closest_player = closest;

		/* Access the race */
		r_ptr = &r_info[m_ptr->r_idx];

		/* Hack -- Require proximity unless this is a wanderer */
		if ( !(r_ptr->flags2 & RF2_WANDERER) )
		{
			if (m_ptr->cdis >= 100) continue;
		}

		/* Access the location */
		fx = m_ptr->fx;
		fy = m_ptr->fy;

		/* Assume no move */
		test = FALSE;

		/* Handle "sensing radius" */
		if (m_ptr->cdis <= r_ptr->aaf)
		{
			/* We can "sense" the player */
			test = TRUE;
		}

		/* Handle "sight" and "aggravation" */
		else if ((m_ptr->cdis <= MAX_SIGHT) &&
		         (closest_in_los || p_ptr->aggravate))
		{
			/* We can "see" or "feel" the player */
			test = TRUE;
		}

#ifdef MONSTER_FLOW
		/* Hack -- Monsters can "smell" the player from far away */
		/* Note that most monsters have "aaf" of "20" or so */
		else if (flow_by_sound &&
		         (cave[py][px].when == cave[fy][fx].when) &&
		         (cave[fy][fx].cost < MONSTER_FLOW_DEPTH) &&
		         (cave[fy][fx].cost < r_ptr->aaf))
		{
			/* We can "smell" the player */
			test = TRUE;
		}
#endif

		/* Do nothing unless a wanderer */
		if (!test && !(r_ptr->flags2 & RF2_WANDERER) ) continue;


		/* Process the monster */
		process_monster(closest, i);
	}

	/* Only when needed, every five game turns */
	if (scan_monsters && (!(turn.turn%5)))
	{
		/* Shimmer multi-hued monsters */
		for (i = 1; i < m_max; i++)
		{
			monster_race *r_ptr;

			m_ptr = &m_list[i];

			/* Skip dead monsters */
			if (!m_ptr->r_idx) continue;

			/* Access the monster race */
			r_ptr = &r_info[m_ptr->r_idx];

			/* Skip non-multi-hued monsters */
			if (!(r_ptr->flags1 & RF1_ATTR_MULTI)) continue;

			/* Shimmer Multi-Hued Monsters */
			everyone_lite_spot(m_ptr->dun_depth, m_ptr->fy, m_ptr->fx);
		}
	}
}
