/* File: cmd5.c */

/* Purpose: Spell/Prayer commands */

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
 * XXX XXX XXX XXX
 */
extern void display_spell_list(void);

/*
 * Hack -- Display all known spells in a window
 *
 * XXX XXX XXX Need to analyze size of the window.
 *
 * XXX XXX XXX Need more color coding.
 *
 * I dunno about this -- I've never found it very useful, so it'll be
 * gone from MAngband (for a while, at least).  I'll eventually just transmit
 * all of this information to client and let it handle it if the user wants
 * to have this in a window --KLJ--
 */
void display_spell_list(void)
{
}



/*
 * Returns spell chance of failure for spell		-RAK-
 */
static s16b spell_chance(int Ind, int spell)
{
	player_type *p_ptr = Players[Ind];

	int		chance, minfail;

	magic_type	*s_ptr;


	/* Paranoia -- must be literate */
	if (!p_ptr->cp_ptr->spell_book) return (100);

	/* Access the spell */
	s_ptr = &p_ptr->mp_ptr->info[spell];

	/* Extract the base spell failure rate */
	chance = s_ptr->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (p_ptr->lev - s_ptr->slevel);

	/* Reduce failure rate by INT/WIS adjustment */
    //chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[p_ptr->cp_ptr->spell_stat]] - 1);
    chance -= adj_mag_stat[p_ptr->stat_ind[p_ptr->cp_ptr->spell_stat]];

	/* Not enough mana to cast */
	if (s_ptr->smana > p_ptr->csp)
	{
		/* Hack -- Since at the moment casting spells without enough mana*/
		/* is impossible, I'm taking this out, as it confuses people. */
		/* chance += 5 * (s_ptr->smana - p_ptr->csp); */
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[p_ptr->cp_ptr->spell_stat]];

    /* Non mage/priest characters never get too good */
    if ((p_ptr->pclass != CLASS_MAGE) && (p_ptr->pclass != CLASS_PRIEST) )
	{
		if (minfail < 5) minfail = 5;
	}

	/* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
    if ((p_ptr->pclass == CLASS_PRIEST) && (p_ptr->icky_wield)) chance += 25;

	/* Minimum failure rate */
	if (chance < minfail) chance = minfail;

	/* Stunning makes spells harder */
	if (p_ptr->stun > 50) chance += 25;
	else if (p_ptr->stun) chance += 15;

	/* Always a 5 percent chance of working */
	if (chance > 95) chance = 95;

	/* Return the chance */
	return (chance);
}



/*
 * Determine if a spell is "okay" for the player to cast or study
 * The spell must be legible, not forgotten, and also, to cast,
 * it must be known, and to study, it must not be known.
 */
static bool spell_okay(int Ind, int j, bool known)
{
	player_type *p_ptr = Players[Ind];

	magic_type *s_ptr;

	/* Access the spell */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* Spell is illegal */
	if (s_ptr->slevel > p_ptr->lev) return (FALSE);

	/* Spell is forgotten */
	if ((j < 32) ?
	    (p_ptr->spell_forgotten1 & (1L << j)) :
	    (p_ptr->spell_forgotten2 & (1L << (j - 32))))
	{
		/* Never okay */
		return (FALSE);
	}

	/* Spell is learned */
	if ((j < 32) ?
	    (p_ptr->spell_learned1 & (1L << j)) :
	    (p_ptr->spell_learned2 & (1L << (j - 32))))
	{
		/* Okay to cast, not to study */
		return (known);
	}

	/* Okay to study, not to cast */
	return (!known);
}



/*
 * Extra information on a spell		-DRS-
 *
 * We can use up to 14 characters of the buffer 'p'
 *
 * The strings in this function were extracted from the code in the
 * functions "do_cmd_cast()" and "do_cmd_pray()" and may be dated.
 */
static void do_spell_info(int Ind, char *p, int j)
{
	player_type *p_ptr = Players[Ind];

	/* Default */
	strcpy(p, "");

#ifdef DRS_SHOW_SPELL_INFO

	/* Mage spells */
	if (p_ptr->cp_ptr->spell_book == TV_MAGIC_BOOK)
	{
		int plev = p_ptr->lev;

		/* Analyze the spell */
		switch (j)
		{
            case MSPELL_MAGIC_MISSILE: sprintf(p, " dam %dd4", 3+((plev-1)/5)); break;
            case MSPELL_PHASE_DOOR: strcpy(p, " range 10"); break;
            case MSPELL_CURE_LIGHT_WOUNDS: strcpy(p, " heal 2d8"); break;
            case MSPELL_STINKING_CLOUD: sprintf(p, " dam %d", 10 + (plev / 2)); break;
            case MSPELL_LIGHTNING_BOLT: sprintf(p, " dam %dd6", (3+((plev-5)/6))); break;
            case MSPELL_TELEPORT_SELF: sprintf(p, " range %d", plev * 5); break;
         	case MSPELL_SHOCK_WAVE:	sprintf(p, " dam %d", 10 + plev); break;
            case MSPELL_SPEAR_OF_LIGHT: strcpy(p, " dam 6d8"); break;
            case MSPELL_FROST_BOLT: sprintf(p, " dam %dd8", (5+((plev-5)/4))); break;
            case MSPELL_FIRE_BOLT: sprintf(p, " dam %dd8", (6+((plev-5)/4))); break;
            case MSPELL_FROST_BALL: sprintf(p, " dam %d", 30 + plev); break;
	    		case MSPELL_FIRE_BALL: sprintf(p, " dam %d", 55 + plev); break;
            case MSPELL_HASTE_SELF: sprintf(p, " dur %d+d20", plev); break;
	    		case MSPELL_ACID_BALL: sprintf(p, " dam %d", 40 + plev*2); break;            
            case MSPELL_ACID_BOLT: sprintf(p, " dam %dd8", (8 + ((plev-5)/4))); break;
            case MSPELL_CLOUD_KILL: sprintf(p, " dam %d", 40 + plev/2); break;
/*            case MSPELL_ACID_BALL: sprintf(p, " dam %d", 40 + plev); break; */
            case MSPELL_ICE_STORM: sprintf(p, " dam %d", 50 + plev*2); break;
            case MSPELL_METEOR_SWARM: sprintf(p, " dam %d", 65 + plev); break;
            case MSPELL_MANA_STORM: sprintf(p, " dam %d", 300 + plev*2); break;
            case MSPELL_RESIST_FIRE: strcpy(p, " dur 20+d20"); break;
            case MSPELL_RESIST_COLD: strcpy(p, " dur 20+d20"); break;
/*            case MSPELL_RESIST_ACID: strcpy(p, " dur 20+d20"); break; */
            case MSPELL_RESIST_POISON: strcpy(p, " dur 20+d20"); break;
            case MSPELL_RESISTANCE: strcpy(p, " dur 20+d20"); break;
            case MSPELL_HEROISM: strcpy(p, " dur 25+d25"); break;
            case MSPELL_SHIELD: strcpy(p, " dur 30+d20"); break;
            case MSPELL_BERSERKER: strcpy(p, " dur 25+d25"); break;
/*            case MSPELL_HASTE_SELF: sprintf(p, " dur %d+d30", 30+plev); break; */
/*            case MSPELL_SHIELD_GOI: strcpy(p, " dur 10+d10"); break; */
		}
	}

	/* Priest spells */
	if (p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK)
	{
		int plev = p_ptr->lev;

		/* See below */
        int orb = (plev / ((p_ptr->pclass == CLASS_PRIEST) ? 2 : 4));

        /* Analyze the spell */
        switch (j)
        {
            case PSPELL_CURE_LIGHT: strcpy(p, " heal 2d10"); break;
            case PSPELL_HERO_BLESS: strcpy(p, " dur 12+d12"); break;
            case PSPELL_TELEPORT_PORTAL: sprintf(p, " range %d", 3*plev); break;
            case PSPELL_CURE_SERIOUS: strcpy(p, " heal 4d10"); break;
            case PSPELL_HERO_CHANT: strcpy(p, " dur 24+d24"); break;
            case PSPELL_RESIST_LOWS: strcpy(p, " dur 10+d10"); break;
            case PSPELL_ORB_DRAINING: sprintf(p, " %d+3d6", plev + orb); break;
            case PSPELL_CURE_CRITICAL: strcpy(p, " heal 6d10"); break;
            case PSPELL_SENSE_INVISIBLE: strcpy(p, " dur 24+d24"); break;
            case PSPELL_PROTECTION_EVIL: sprintf(p, " dur %d+d25", 3*plev); break;
            case PSPELL_CURE_MORTAL: strcpy(p, " heal 8d10"); break;
            case PSPELL_HERO_PRAYER: strcpy(p, " dur 48+d48"); break;
            case PSPELL_DISPEL_UNDEAD: sprintf(p, " dam d%d", 3*plev); break;
            case PSPELL_CURE_HEAL: strcpy(p, " heal 300"); break;
            case PSPELL_DISPEL_EVIL: sprintf(p, " dam d%d", 3*plev); break;
            case PSPELL_HOLY_WORD: strcpy(p, " heal 1000"); break;
            case PSPELL_CURE_SERIOUS_WOUNDS2: strcpy(p, " heal 4d10"); break;
            case PSPELL_CURE_MORTAL_WOUNDS2: strcpy(p, " heal 8d10"); break;
            case PSPELL_HEALING: strcpy(p, " heal 2000"); break;
            case PSPELL_DISPEL_UNDEAD2: sprintf(p, " dam d%d", 4*plev); break;
            case PSPELL_DISPEL_EVIL2: sprintf(p, " dam d%d", 4*plev); break;
            case PSPELL_ANNIHILAT_BOLT: strcpy(p, " dam 200"); break;
            case PSPELL_TELEPORT_BLINK: strcpy(p, " range 10"); break;
            case PSPELL_TELEPORT_SELF: sprintf(p, " range %d", 8*plev); break;
        }
    }
#endif

}


/*
 * Print a list of spells (for browsing or casting)
 */
static void print_spells(int Ind, int book, byte *spell, int num)
{
	player_type *p_ptr = Players[Ind];

	int			i, j;

	magic_type		*s_ptr;

	cptr		comment;

	char		info[80];

	char		out_val[160];

    int p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0);

	/* Dump the spells */
	for (i = 0; i < num; i++)
	{
		/* Access the spell */
		j = spell[i];

		/* Access the spell */
		s_ptr = &p_ptr->mp_ptr->info[j];

		/* Skip illegible spells */
		if (s_ptr->slevel >= 99)
		{
			sprintf(out_val, "  %c) %-30s", I2A(i), "(illegible)");
			Send_spell_info(Ind, book, i, out_val);
			continue;
		}

		/* XXX XXX Could label spells above the players level */

		/* Get extra info */
		do_spell_info(Ind, info, j);

		/* Use that info */
		comment = info;

		/* Analyze the spell */
		if ((j < 32) ?
		    ((p_ptr->spell_forgotten1 & (1L << j))) :
		    ((p_ptr->spell_forgotten2 & (1L << (j - 32)))))
		{
			comment = " forgotten";
		}
		else if (!((j < 32) ?
		           (p_ptr->spell_learned1 & (1L << j)) :
		           (p_ptr->spell_learned2 & (1L << (j - 32)))))
		{
			comment = " unknown";
		}
		else if (!((j < 32) ?
		           (p_ptr->spell_worked1 & (1L << j)) :
		           (p_ptr->spell_worked2 & (1L << (j - 32)))))
		{
			comment = " untried";
		}

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
		        I2A(i), spell_names[p][j],
		        s_ptr->slevel, s_ptr->smana, spell_chance(Ind, j), comment);
		Send_spell_info(Ind, book, i, out_val);
	}
}



/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 */
void do_cmd_browse(int Ind, int book)
{
	player_type *p_ptr = Players[Ind];

	int			i, j, item, sval;

	byte		spell[64], num = 0;

	object_type		*o_ptr;

    int p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0);


	/* Warriors are illiterate */
	if (!p_ptr->cp_ptr->spell_book)
	{
		msg_print(Ind, "You cannot read books!");
		return;
	}

	/* No lite */
	if (p_ptr->blind || no_lite(Ind))
	{
		msg_print(Ind, "You cannot see!");
		return;
	}

	/* Confused */
	if (p_ptr->confused)
	{
		msg_print(Ind, "You are too confused!");
		return;
	}


	/* Restrict choices to "useful" books */
	item_tester_tval = p_ptr->cp_ptr->spell_book;

	item = book;

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
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Tried browsing a bad book */
		return;
	}


	/* Access the item's sval */
	sval = o_ptr->sval;

	/* Extract spells */
	for (i = 0; i < 64; i++)
	{
		/* Check for this spell */
		if ((i < 32) ?
		    (spell_flags[p][sval][0] & (1L << i)) :
		    (spell_flags[p][sval][1] & (1L << (i - 32))))
		{
			/* Collect this spell */
			spell[num++] = i;
		}
	}

	/* Display the spells */
	print_spells(Ind, book, spell, num);
}




/*
 * Study a book to gain a new spell/prayer
 */
void do_cmd_study(int Ind, int book, int spell)
{
	player_type *p_ptr = Players[Ind];

	int			i, sval;

	int			j = -1;

    cptr p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? "prayer" : "spell");

	object_type		*o_ptr;

	byte spells[64], num = 0;

	if (!p_ptr->cp_ptr->spell_book)
	{
		msg_print(Ind, "You cannot read books!");
		return;
	}

	if (p_ptr->blind || no_lite(Ind))
	{
		msg_print(Ind, "You cannot see!");
		return;
	}

	if (p_ptr->confused)
	{
		msg_print(Ind, "You are too confused!");
		return;
	}

	if (!(p_ptr->new_spells))
	{
		msg_format(Ind, "You cannot learn any new %ss!", p);
		return;
	}


	/* Restrict choices to "useful" books */
	item_tester_tval = p_ptr->cp_ptr->spell_book;

	/* Get the item (in the pack) */
	if (book >= 0)
	{
		o_ptr = &p_ptr->inventory[book];
	}

	/* Get the item (on the floor) */
	else
	{
		book = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (book == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - book];
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Trying to gain a spell from a bad book */
		return;
	}

	/* Access the item's sval */
	sval = o_ptr->sval;

    /* Spellcaster -- Learn a selected spell */
    if (p == "spell")
	{
		for (i = 0; i < 64; i++)
		{
			/* Check for this spell */
			if ((i < 32) ?
				(spell_flags[0][sval][0] & (1L << i)) :
				(spell_flags[0][sval][1] & (1L << (i - 32))))
			{
				/* Collect this spell */
				spells[num++] = i;
			}
		}

		/* Set the spell number */
		j = spells[spell];

		if (!spell_okay(Ind, j, FALSE))
		{
			msg_print(Ind, "You cannot gain that spell!");
			return;
		}
	}

    /* Cleric -- Learn a random prayer */
	if (p == "prayer")
	{
		int k = 0;

		/* Extract spells */
		for (i = 0; i < 64; i++)
		{
			/* Check spells in the book */
			if ((i < 32) ?
			    (spell_flags[1][sval][0] & (1L << i)) :
			    (spell_flags[1][sval][1] & (1L << (i - 32))))
			{
				/* Skip non "okay" prayers */
				if (!spell_okay(Ind, i, FALSE)) continue;

				/* Hack -- Prepare the randomizer */
				k++;

				/* Hack -- Apply the randomizer */
				if (rand_int(k) == 0) j = i;
			}
		}
	}

	/* Nothing to study */
	if (j < 0)
	{
		/* Message */
		msg_format(Ind, "You cannot learn any %ss in that book.", p);

		/* Abort */
		return;
	}


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Learn the spell */
	if (j < 32)
	{
		p_ptr->spell_learned1 |= (1L << j);
	}
	else
	{
		p_ptr->spell_learned2 |= (1L << (j - 32));
	}

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < 64; i++)
	{
		/* Stop at the first empty space */
		if (p_ptr->spell_order[i] == 99) break;
	}

	/* Add the spell to the known list */
	p_ptr->spell_order[i++] = j;

	/* Mention the result */
	msg_format(Ind, "You have learned the %s of %s.",
	           p, spell_names[((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0)][j]);

#if 0
	/* Mention the result */
	msg_format(Ind, "You have learned the %s of %s.",
	           p, spell_names[p_ptr->mp_ptr->spell_type][j]);
#endif

	/* One less spell available */
	p_ptr->new_spells--;

	/* Report on remaining prayers */
	if (p_ptr->new_spells)
	{
		msg_format(Ind, "You can learn %d more %ss.", p_ptr->new_spells, p);
	}

	/* Save the new_spells value */
	p_ptr->old_spells = p_ptr->new_spells;

	/* Redraw Study Status */
	p_ptr->redraw |= (PR_STUDY);

	/* Update the spell info */
	p_ptr->window |= (PW_SPELL);
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

      object_desc(Ind, o_name, o_ptr, FALSE, 0);

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
		if (flush_failure) flush();
		msg_print(Ind, "The Branding failed.");
	}
}
/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(const object_type *o_ptr)
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
	cptr q, s;
	int r;
	byte brand_type;

	/* Only accept ammo */
	item_tester_hook = item_tester_hook_ammo;

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
	}

	if (!item_tester_hook(o_ptr)) {
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
 * Cast a spell
 *
 * Many of the spells have a strange get_aim_dir(), and then they return.
 * What this does is it sends a PKT_GET_DIRECTION to the client, which then
 * (when the player hits a direction key), send a response to the server.  The
 * server then calls do_cmd_cast_aux() with the direction.  This is a crappy
 * way of doing things, but it should work.  Without this, the server is hung
 * until the player hits a direction key, and we try very hard not to have
 * any undue slowness in the server. --KLJ--
 */
void do_cmd_cast(int Ind, int book, int spell)
{
	player_type *p_ptr = Players[Ind];

	int			i, j, sval;
	int			chance, beam;
	int			plev = p_ptr->lev;

	object_type		*o_ptr;

	magic_type		*s_ptr;

	byte spells[64], num = 0;

    int p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0);

	/* Require spell ability */
	if (p_ptr->cp_ptr->spell_book != TV_MAGIC_BOOK)
	{
		msg_print(Ind, "You cannot cast spells!");
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite(Ind))
	{
		msg_print(Ind, "You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print(Ind, "You are too confused!");
		return;
	}


	/* Restrict choices to spell books */
	item_tester_tval = p_ptr->cp_ptr->spell_book;

	/* Get the item (in the pack) */
	if (book >= 0)
	{
		o_ptr = &p_ptr->inventory[book];
	}

	/* Get the item (on the floor) */
	else
	{
		book = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (book == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - book];
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Tried to cast spell from bad book */
		return;
	}

        if( check_guard_inscription( o_ptr->note, 'm' )) {
                msg_print(Ind, "The item's inscription prevents it");
                return;
        };

	/* Access the item's sval */
	sval = o_ptr->sval;

	for (i = 0; i < 64; i++)
	{
		/* Check for this spell */
		if ((i < 32) ?
			(spell_flags[p][sval][0] & (1L << i)) :
			(spell_flags[p][sval][1] & (1L << (i - 32))))
		{
			/* Collect this spell */
			spells[num++] = i;
		}
	}

	/* Set the spell number */
	j = spells[spell];

	if (!spell_okay(Ind, j, 1))
	{
		msg_print(Ind, "You cannot cast that spell.");
		return;
	}

	/* Access the spell */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* Check mana */
	if (s_ptr->smana > p_ptr->csp)
	{
		msg_print(Ind, "You do not have enough mana.");
		return;
	}

	/* Spell failure chance */
	chance = spell_chance(Ind, j);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		if (flush_failure) flush();
		msg_print(Ind, "You failed to get the spell off!");
	}

	/* Process spell */
	else
	{
		/* Hack -- chance of "beam" instead of "bolt" */
        beam = ((p_ptr->pclass == CLASS_MAGE) ? plev : (plev / 2));

		/* Spells.  */
		switch (j)
		{
            case MSPELL_MAGIC_MISSILE:
			{
                p_ptr->current_spell = MSPELL_MAGIC_MISSILE;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_DETECT_MONSTERS:
			{
				(void)detect_creatures(Ind, TRUE);
				break;
			}

            case MSPELL_PHASE_DOOR:
			{
				msg_format_near(Ind, "%s blinks away!", p_ptr->name);
				teleport_player(Ind, 10);
				break;
			}

            case MSPELL_LIGHT_AREA:
			{
				(void)lite_area(Ind, damroll(2, (plev / 2)), (plev / 10) + 1);
				break;
			}

            case MSPELL_TREASURE_DETECTION:
			{
				(void)detect_treasure(Ind);
				break;
			}

            case MSPELL_CURE_LIGHT_WOUNDS:
			{
				(void)hp_player(Ind, damroll(2, 8));
				(void)set_cut(Ind, p_ptr->cut - 15);
				break;
			}

            case MSPELL_OBJECT_DETECTION:
			{
				(void)detect_objects_normal(Ind);
				break;
			}

            case MSPELL_FIND_TRAPS_DOORS:
			{
				(void)detect_sdoor(Ind);
				(void)detect_trap(Ind);
				break;
			}

            case MSPELL_STINKING_CLOUD:
			{
                p_ptr->current_spell = MSPELL_STINKING_CLOUD;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_CONFUSE_MONSTER:
			{
                p_ptr->current_spell = MSPELL_CONFUSE_MONSTER;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_LIGHTNING_BOLT:
			{
                p_ptr->current_spell = MSPELL_LIGHTNING_BOLT;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_TRAP_DOOR_DESTRUCTION:
			{
				(void)destroy_doors_touch(Ind);
				break;
			}

            case MSPELL_SLEEP_MONSTER:
			{
                p_ptr->current_spell = MSPELL_SLEEP_MONSTER;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_CURE_POISON:
			{
				(void)set_poisoned(Ind, 0);
				break;
			}

            case MSPELL_TELEPORT_SELF:
			{
				msg_format_near(Ind, "%s teleports away!", p_ptr->name);
				teleport_player(Ind, plev * 5);
				break;
			}

            case MSPELL_SPEAR_OF_LIGHT:
			{
                p_ptr->current_spell = MSPELL_SPEAR_OF_LIGHT;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_FROST_BOLT:
			{
                p_ptr->current_spell = MSPELL_FROST_BOLT;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_TURN_STONE_TO_MUD:
			{
                p_ptr->current_spell = MSPELL_TURN_STONE_TO_MUD;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_SATISFY_HUNGER:
			{
				(void)set_food(Ind, PY_FOOD_MAX - 1);
				break;
			}

            case MSPELL_RECHARGE_ITEM_I:
			{
				(void)recharge(Ind, 2 + plev / 5);
				break;
			}

	    case MSPELL_FIRE_BOLT:
			{
                p_ptr->current_spell = MSPELL_FIRE_BOLT;
                get_aim_dir(Ind);
                return;
			}

            case MSPELL_POLYMORPH_OTHER:
			{
                p_ptr->current_spell = MSPELL_POLYMORPH_OTHER;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_IDENTIFY:
			{
				(void)ident_spell(Ind);
				break;
			}

/*            case MSPELL_SLEEP_MONSTER:
			{
				(void)sleep_monsters(Ind);
				break;
			}
*/
            case MSPELL_FROST_BALL:
			{
                p_ptr->current_spell = MSPELL_FROST_BALL;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_SLOW_MONSTER:
			{
                p_ptr->current_spell = MSPELL_SLOW_MONSTER;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_FIRE_BALL:
			{
                p_ptr->current_spell = MSPELL_FIRE_BALL;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_RECHARGE_ITEM_II:
			{
				(void)recharge(Ind, 50 + plev);
				break;
			}

            case MSPELL_TELEPORT_OTHER:
			{
                p_ptr->current_spell = MSPELL_TELEPORT_OTHER;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_HASTE_SELF:
			{
				msg_format_near(Ind, "%s starts moving faster.", p_ptr->name);
				if (!p_ptr->fast)
				{
					(void)set_fast(Ind, randint(20) + plev);
				}
				else
				{
					(void)set_fast(Ind, p_ptr->fast + randint(5));
				}
				break;
			}

            case MSPELL_ACID_BALL:
			{
                p_ptr->current_spell = MSPELL_ACID_BALL;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_WORD_OF_DESTRUCTION:
			{
				msg_format_near(Ind, "%s unleashes great power!", p_ptr->name);
				destroy_area(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 15, TRUE);
				break;
			}

            case MSPELL_BANISHMENT:
			{
				(void)banishment(Ind);
				break;
			}

            case MSPELL_DOOR_CREATION:
			{
				(void)door_creation(Ind);
				break;
			}

            case MSPELL_STAIR_CREATION:
			{
				(void)stair_creation(Ind);
				break;
			}

            case MSPELL_TELEPORT_LEVEL:
			{
				(void)teleport_player_level(Ind);
				break;
			}

            case MSPELL_EARTHQUAKE:
			{
				msg_format_near(Ind, "%s casts a spell, and the ground shakes!", p_ptr->name);
				earthquake(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 10);
				break;
			}

            case MSPELL_WORD_OF_RECALL:
			{
				set_recall(Ind, o_ptr);
				break;
			}

            case MSPELL_ACID_BOLT:
			{
                p_ptr->current_spell = MSPELL_ACID_BOLT;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_CLOUD_KILL:
			{
                p_ptr->current_spell = MSPELL_CLOUD_KILL;
				get_aim_dir(Ind);
				return;
			}

/*            case MSPELL_ACID_BALL:
			{
                p_ptr->current_spell = MSPELL_ACID_BALL;
				get_aim_dir(Ind);
				return;
			}
*/
            case MSPELL_ICE_STORM:
			{
                p_ptr->current_spell = MSPELL_ICE_STORM;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_METEOR_SWARM:
			{
                p_ptr->current_spell = MSPELL_METEOR_SWARM;
				get_aim_dir(Ind);
				return;
			}

            case MSPELL_MANA_STORM:
			{
                p_ptr->current_spell = MSPELL_MANA_STORM;
				get_aim_dir(Ind);
				return;
			}

#if 0
			case MSPELL_DETECT_EVIL:
			{
				(void)detect_evil(Ind);
				break;
			}
#endif
            case MSPELL_DETECT_ENCHANTMENT:
			{
				(void)detect_objects_magic(Ind);
				break;
			}

#if 0
			case MSPELL_RECHARGE_ITEM3:
			{
				recharge(Ind, 100);
				break;
			}
#endif
/*            case MSPELL_BANISHMENT:
			{
				(void)genocide(Ind);
				break;
			}
*/
            case MSPELL_MASS_BANISHMENT:
			{
				(void)mass_banishment(Ind);
				break;
			}

            case MSPELL_RESIST_FIRE:
			{
				(void)set_oppose_fire(Ind, p_ptr->oppose_fire + randint(20) + 20);
				break;
			}

            case MSPELL_RESIST_COLD:
			{
				(void)set_oppose_cold(Ind, p_ptr->oppose_cold + randint(20) + 20);
				break;
			}

#if 0
			case MSPELL_RESIST_ACID:
			{
				(void)set_oppose_acid(Ind, p_ptr->oppose_acid + randint(20) + 20);
				break;
			}
#endif
            case MSPELL_RESIST_POISON:
			{
				(void)set_oppose_pois(Ind, p_ptr->oppose_pois + randint(20) + 20);
				break;
			}

            case MSPELL_RESISTANCE:
			{
				(void)set_oppose_acid(Ind, p_ptr->oppose_acid + randint(20) + 20);
				(void)set_oppose_elec(Ind, p_ptr->oppose_elec + randint(20) + 20);
				(void)set_oppose_fire(Ind, p_ptr->oppose_fire + randint(20) + 20);
				(void)set_oppose_cold(Ind, p_ptr->oppose_cold + randint(20) + 20);
				(void)set_oppose_pois(Ind, p_ptr->oppose_pois + randint(20) + 20);
				break;
			}

            case MSPELL_HEROISM:
			{
				(void)hp_player(Ind, 10);
				(void)set_hero(Ind, p_ptr->hero + randint(25) + 25);
				(void)set_afraid(Ind, 0);
				break;
			}

            case MSPELL_SHIELD:
			{
				msg_format_near(Ind, "%s forms a mystic shield.", p_ptr->name);
				(void)set_shield(Ind, p_ptr->shield + randint(20) + 30);
				break;
			}

            case MSPELL_BERSERKER:
			{
				msg_format_near(Ind, "%s enters a battle rage!", p_ptr->name);
				(void)hp_player(Ind, 30);
				(void)set_shero(Ind, p_ptr->shero + randint(25) + 25);
				(void)set_afraid(Ind, 0);
				break;
			}

            case MSPELL_ENCHANT_WEAPON:
			{
				(void)enchant_spell(Ind, rand_int(4) + 1, rand_int(4) + 1, 0);
				break;
			}

            case MSPELL_ENCHANT_ARMOR:
			{
				(void)enchant_spell(Ind, 0, 0, rand_int(3) + 2);
				break;
			}

            case MSPELL_ELEMENTAL_BRAND:
			{
		                p_ptr->current_spell = MSPELL_ELEMENTAL_BRAND;
				get_item(Ind);
				return;
				//brand_ammo(Ind);
				//break;
			}

			case MSPELL_DETECT_INVISIBLE:
			{
				(void)detect_invisible(Ind, TRUE);
				break;
			}

			case MSPELL_WONDER:
			{
                p_ptr->current_spell = MSPELL_WONDER;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_SHOCK_WAVE:
			{
                p_ptr->current_spell = MSPELL_SHOCK_WAVE;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_EXPLOSION:
			{
                p_ptr->current_spell = MSPELL_EXPLOSION;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_MASS_SLEEP:
			{
				(void)sleep_monsters(Ind);
				break;
			}

			case MSPELL_BEDLAM:
			{
                p_ptr->current_spell = MSPELL_BEDLAM;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_REND_SOUL:
			{
                p_ptr->current_spell = MSPELL_REND_SOUL;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_CHAOS_STRIKE:
			{
                p_ptr->current_spell = MSPELL_CHAOS_STRIKE;
				get_aim_dir(Ind);
				return;
			}

			case MSPELL_RUNE_OF_PROTECTION:
			{
                msg_format_near(Ind, "%s lays down a rune of protection.", p_ptr->name);
				(void)warding_glyph(Ind);
				break;
			}

			case MSPELL_RIFT:
			{
                p_ptr->current_spell = MSPELL_RIFT;
				get_aim_dir(Ind);
				return;
			}

#if 0
			case MSPELL_HASTE_SELF:
			{
				if (!p_ptr->fast)
				{
					(void)set_fast(Ind, randint(30) + 30 + plev);
				}
				else
				{
					(void)set_fast(Ind, p_ptr->fast + randint(10));
				}
				break;
			}
#endif
#if 0
			case MSPELL_SHIELD_GOI:
			{
				//(void)set_invuln(Ind, p_ptr->invuln + randint(8) + 8);
				// GOI no longer stacks, as when it did it was vastly overpowered. 
				if (!p_ptr->invuln)
					set_invuln(Ind, 10 + randint(10));
				break;
			}
#endif
		}

		/* A spell was cast */
		if (!((j < 32) ?
		      (p_ptr->spell_worked1 & (1L << j)) :
		      (p_ptr->spell_worked2 & (1L << (j - 32)))))
		{
			int e = s_ptr->sexp;

			/* The spell worked */
			if (j < 32)
			{
				p_ptr->spell_worked1 |= (1L << j);
			}
			else
			{
				p_ptr->spell_worked2 |= (1L << (j - 32));
			}

			/* Gain experience */
			gain_exp(Ind, e * s_ptr->slevel);

			/* Fix the spell info */
			p_ptr->window |= PW_SPELL;
		}
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Sufficient mana */
	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print(Ind, "You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(Ind, p_ptr->paralyzed + randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool perm = (rand_int(100) < 25);

			/* Message */
			msg_print(Ind, "You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(Ind, A_CON, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


static int beam_chance(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int plev = p_ptr->lev;
	return ((p_ptr->cp_ptr->flags & CF_BEAM) ? plev : (plev / 2));
}


static void spell_wonder(int Ind, int dir)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */

	player_type *p_ptr = Players[Ind];

	int py = p_ptr->py;
	int px = p_ptr->px;
	int plev = p_ptr->lev;
	int die = randint(100) + plev / 5;
	int beam = beam_chance(Ind);

	if (die > 100)
		msg_print(Ind, "You feel a surge of power!");
	if (die < 8) clone_monster(Ind, dir);
	else if (die < 14) speed_monster(Ind, dir);
	else if (die < 26) heal_monster(Ind, dir);
	else if (die < 31) poly_monster(Ind, dir);
	else if (die < 36)
		fire_bolt_or_beam(Ind, beam - 10, GF_MISSILE, dir,
		                  damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) confuse_monster(Ind, dir, plev);
	else if (die < 46) fire_ball(Ind, GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) lite_line(Ind, dir);
	else if (die < 56)
		fire_beam(Ind, GF_ELEC, dir, damroll(3+((plev-5)/6), 6));
	else if (die < 61)
		fire_bolt_or_beam(Ind, beam-10, GF_COLD, dir,
		                  damroll(5+((plev-5)/4), 8));
	else if (die < 66)
		fire_bolt_or_beam(Ind, beam, GF_ACID, dir,
		                  damroll(6+((plev-5)/4), 8));
	else if (die < 71)
		fire_bolt_or_beam(Ind, beam, GF_FIRE, dir,
		                  damroll(8+((plev-5)/4), 8));
	else if (die < 76) drain_life(Ind, dir, 75);
	else if (die < 81) fire_ball(Ind, GF_ELEC, dir, 30 + plev / 2, 2);
	else if (die < 86) fire_ball(Ind, GF_ACID, dir, 40 + plev, 2);
	else if (die < 91) fire_ball(Ind, GF_ICE, dir, 70 + plev, 3);
	else if (die < 96) fire_ball(Ind, GF_FIRE, dir, 80 + plev, 3);
	else if (die < 101) drain_life(Ind, dir, 100 + plev);
	else if (die < 104) earthquake(p_ptr->dun_depth, py, px, 12);
	else if (die < 106) destroy_area(p_ptr->dun_depth, py, px, 15, TRUE);
	else if (die < 108) banishment(Ind);
	else if (die < 110) dispel_monsters(Ind, 120);
	else /* RARE */
	{
		dispel_monsters(Ind, 150);
		slow_monsters(Ind);
		sleep_monsters(Ind);
		hp_player(Ind, 300);
	}
}


/*
 * Finish casting a spell that required a direction --KLJ--
 */
void do_cmd_cast_aux(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int plev = p_ptr->lev;
    int beam = ((p_ptr->pclass == CLASS_MAGE) ? plev : (plev / 2));

	magic_type *s_ptr = &p_ptr->mp_ptr->info[p_ptr->current_spell];

	/* Only fire in direction 5 if we have a target */
	if ((dir == 5) && !target_okay(Ind))
	{
		/* Reset current spell */
		p_ptr->current_spell = -1;

		/* Done */
		return;
	}

	/* We assume that the spell can be cast, and so forth */
	switch(p_ptr->current_spell)
	{
        case MSPELL_MAGIC_MISSILE:
		{
			msg_format_near(Ind, "%s fires a magic missile.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam-10, GF_MISSILE, dir,
						damroll(3 + ((plev - 1) / 5), 4));
			break;
		}

        case MSPELL_STINKING_CLOUD:
		{
			msg_format_near(Ind, "%s casts a stinking cloud.", p_ptr->name);
			fire_ball(Ind, GF_POIS, dir,
				10 + (plev / 2), 2);
			break;
		}

        case MSPELL_CONFUSE_MONSTER:
		{
			confuse_monster(Ind, dir, plev);
			break;
		}

        case MSPELL_LIGHTNING_BOLT:
		{
			msg_format_near(Ind, "%s casts a lightning bolt.", p_ptr->name);
			fire_beam(Ind, GF_ELEC, dir, 
			          damroll(3+((plev-5)/6), 6));
			break;
		}

        case MSPELL_SLEEP_MONSTER:
		{
			sleep_monster(Ind, dir);
			break;
		}

        case MSPELL_SPEAR_OF_LIGHT:
		{
			msg_print(Ind, "A line of blue shimmering light appears.");
			lite_line(Ind, dir);
			break;
		}

        case MSPELL_FROST_BOLT:
		{
			msg_format_near(Ind, "%s casts a frost bolt.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam-10, GF_COLD, dir,
				damroll(5+((plev-5)/4), 8));
			break;
		}

        case MSPELL_TURN_STONE_TO_MUD:
		{
			(void)wall_to_mud(Ind, dir);
			break;
		}

        case MSPELL_POLYMORPH_OTHER:
		{
			(void)poly_monster(Ind, dir);
			break;
		}

        case MSPELL_FIRE_BOLT:
		{
			msg_format_near(Ind, "%s casts a fire bolt.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam, GF_FIRE, dir,
				damroll(6+((plev-5)/4), 8));
			break;
		}

        case MSPELL_SLOW_MONSTER:
		{
			(void)slow_monster(Ind, dir);
			break;
		}

        case MSPELL_FROST_BALL:
		{
			msg_format_near(Ind, "%s casts a frost ball.", p_ptr->name);
			fire_ball(Ind, GF_COLD, dir, 30 + (plev), 2);
			break;
		}

        case MSPELL_TELEPORT_OTHER:
		{
			(void)teleport_monster(Ind, dir);
			break;
		}

        case MSPELL_FIRE_BALL:
		{
			msg_format_near(Ind, "%s casts a fire ball.", p_ptr->name);
			fire_ball(Ind, GF_FIRE, dir, 55 + (plev), 2);
			break;
		}

        case MSPELL_ACID_BOLT:
		{
			msg_format_near(Ind, "%s casts an acid bolt.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam, GF_ACID, dir,
				damroll(8+((plev-5)/4), 8));
			break;
		}

        case MSPELL_CLOUD_KILL:
		{
			msg_format_near(Ind, "%s casts a cloud of death.", p_ptr->name);
			fire_ball(Ind, GF_POIS, dir, 40 + (plev / 2), 3);
			break;
		}

        case MSPELL_ACID_BALL:
		{
			msg_format_near(Ind, "%s casts an acid ball.", p_ptr->name);
			fire_ball(Ind, GF_ACID, dir, 40 + (plev), 2);
			break;
		}

        case MSPELL_ICE_STORM:
		{
        		msg_format_near(Ind, "%s invokes an ice storm.", p_ptr->name);
			fire_ball(Ind, GF_ICE, dir, 50 + (plev * 2), 3);
			break;
		}

        case MSPELL_METEOR_SWARM:
		{
			msg_format_near(Ind, "%s casts a meteor shower.", p_ptr->name);
			fire_swarm(Ind, 2 + plev / 20, GF_METEOR, dir, 30 + plev / 2, 1);
			break;
		}

        case MSPELL_MANA_STORM:
		{
			msg_format_near(Ind, "%s casts a mana ball.", p_ptr->name);
			fire_ball(Ind, GF_MANA, dir, 300 + (plev * 2), 3);
			break;
		}
	case MSPELL_ELEMENTAL_BRAND:
		{
			brand_ammo(Ind, dir);
			break;
		}
        case MSPELL_WONDER:
		{
			(void)spell_wonder(Ind, dir);
			break;
		}

		case MSPELL_SHOCK_WAVE:
		{
			msg_format_near(Ind, "%s casts a shock wave.", p_ptr->name);
			fire_ball(Ind, GF_SOUND, dir, 10 + plev, 2);
			break;
		}

		case MSPELL_EXPLOSION:
		{
			msg_format_near(Ind, "%s casts an explosion.", p_ptr->name);
			fire_ball(Ind, GF_SHARDS, dir, 10 + plev, 2);
			break;
		}

		case MSPELL_BEDLAM:
		{
			msg_format_near(Ind, "%s creates confusion.", p_ptr->name);
			fire_ball(Ind, GF_OLD_CONF, dir, plev, 4);
			break;
		}

		case MSPELL_REND_SOUL:
		{
			msg_format_near(Ind, "%s casts a nether ball.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam/4, GF_NETHER, dir,
				damroll(11, plev));
			break;
		}

		case MSPELL_CHAOS_STRIKE:
		{
			msg_format_near(Ind, "%s casts a ball of chaos.", p_ptr->name);
			fire_bolt_or_beam(Ind, beam, GF_CHAOS, dir,
				damroll(13, plev));
			break;
		}

		case MSPELL_RIFT:
		{
			msg_format_near(Ind, "Space warps in a beam from %s.", p_ptr->name);
			fire_beam(Ind, GF_GRAVITY, dir,
				40 + damroll(plev, 7));
			break;
		}

		default:  /* For some reason we got called for a spell that */
		{         /* doesn't require a direction */
			msg_print(Ind, "SERVER ERROR: do_cmd_cast_aux() called for non-directional spell!");
			p_ptr->current_spell = -1;
			return;
		}
	}	

	if (!((p_ptr->current_spell < 32) ?
		(p_ptr->spell_worked1 & (1L << p_ptr->current_spell)) :
		(p_ptr->spell_worked2 & (1L << (p_ptr->current_spell - 32)))))
	{
		int e = s_ptr->sexp;

		if (p_ptr->current_spell < 32)
		{
			p_ptr->spell_worked1 |= (1L << p_ptr->current_spell);
		}
		else
		{
			p_ptr->spell_worked2 |= (1L << (p_ptr->current_spell - 32));
		}

		gain_exp(Ind, e * s_ptr->slevel);

		/* Fix the spell info */
		p_ptr->window |= PW_SPELL;
	}

	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print(Ind, "You faint from the effort!");

		/* Hack -- bypass free action */
		(void)set_paralyzed(Ind, p_ptr->paralyzed + randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool perm = (rand_int(100) < 25);

			/* Message */
			msg_print(Ind, "You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(Ind, A_CON, 15 + randint(10), perm);
		}
	}

	/* Reset current spell */
	p_ptr->current_spell = -1;

	/* Resend mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

/*
 * Pray a prayer
 *
 * See above for an explanation of the weirdness in this function --KLJ--
 */
void do_cmd_pray(int Ind, int book, int spell)
{
    player_type *p_ptr = Players[Ind];

    int item, sval, j, chance, i;
    int plev = p_ptr->lev;

    object_type	*o_ptr;

    magic_type  *s_ptr;

    byte spells[64], num = 0;

    int p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? 1 : 0);

    /* Must use prayer books */
    if (p_ptr->cp_ptr->spell_book != TV_PRAYER_BOOK)
    {
        msg_print(Ind, "Pray hard enough and your prayers may be answered.");
        return;
    }

    /* Must have lite */
    if (p_ptr->blind || no_lite(Ind))
    {
        msg_print(Ind, "You cannot see!");
        return;
    }

    /* Must not be confused */
    if (p_ptr->confused)
    {
        msg_print(Ind, "You are too confused!");
        return;
    }


    /* Restrict choices */
    item_tester_tval = p_ptr->cp_ptr->spell_book;

    item = book;

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
    }

    if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
    {
        /* Tried to pray prayer from bad book */
        return;
    }
        if( check_guard_inscription( o_ptr->note, 'p' )) {
                msg_print(Ind, "The item's inscription prevents it");
                return;
        };

    /* Access the item's sval */
    sval = o_ptr->sval;

	for (i = 0; i < 64; i++)
    {
        /* Check for this spell */
        if ((i < 32) ?
            (spell_flags[p][sval][0] & (1L << i)) :
            (spell_flags[p][sval][1] & (1L << (i - 32))))
        {
            /* Collect this spell */
            spells[num++] = i;
        }
    }
    
    /* OK, this is a unsightly kludge to get some extra (heal other) 
    prayers without modifying the spell_flags... but it should work for 
    now. -AD- */
    
    /* normal prayers */
    if (spell < 64) j = spells[spell];
    
    /* heal other prayers */
    else j = spells[spell-64];

    if (!spell_okay(Ind, j, 1))
    {
        msg_print(Ind, "You cannot pray that prayer.");
        return;
    }
    
    /* get the spell info */
    s_ptr = &p_ptr->mp_ptr->info[j];
    
    /* Check mana */
    if (s_ptr->smana > p_ptr->csp)
    {
        msg_print(Ind, "You do not have enough mana.");
        return;
    }

    /* Spell failure chance */
    chance = spell_chance(Ind, j);

    /* Check for failure */
    if (rand_int(100) < chance)
    {
        if (flush_failure) flush();
        msg_print(Ind, "You failed to concentrate hard enough!");
    }

    /* Success */
    else
    {
        if (spell >= 64) j += 64;
        switch (j)
        {
            case PSPELL_DETECT_EVIL:
            {
                (void)detect_evil(Ind);
                break;
            }

            case PSPELL_CURE_LIGHT:
            {
                (void)hp_player(Ind, damroll(2, 10));
                (void)set_cut(Ind, p_ptr->cut - 10);				
                break;
            }

            /* cure light wounds projectile */
            case PSPELL_CURE_LIGHT+64:
            {				
                p_ptr->current_spell = PSPELL_CURE_LIGHT;
                get_aim_dir(Ind);
                
                return;
            }

            case PSPELL_HERO_BLESS:
            {
                (void)set_blessed(Ind, p_ptr->blessed + randint(12) + 12);
                break;
            }

            case PSPELL_REMOVE_FEAR:
            {
                (void)set_afraid(Ind, 0);
                break;
            }

            case PSPELL_CALL_LIGHT:
            {
                msg_format_near(Ind, "%s calls light.", p_ptr->name);
                (void)lite_area(Ind, damroll(2, (plev / 2)), (plev / 10) + 1);
                break;
            }

            case PSPELL_FIND_TRAPS:
            {
                (void)detect_trap(Ind);
                break;
            }

            case PSPELL_DETECT_FEATURES:
            {
                (void)detect_sdoor(Ind);
                break;
            }

            case PSPELL_SLOW_POISON:
            {
                (void)set_poisoned(Ind, p_ptr->poisoned / 2);
                break;
            }

            case PSPELL_SCARE_MONSTER:
            {
                p_ptr->current_spell = PSPELL_SCARE_MONSTER;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_TELEPORT_PORTAL:
            {
                msg_format_near(Ind, "%s blinks away!", p_ptr->name);
                teleport_player(Ind, plev * 3);
                break;
            }

            case PSPELL_CURE_SERIOUS:
            {
                (void)hp_player(Ind, damroll(4, 10));
                (void)set_cut(Ind, (p_ptr->cut / 2) - 20);
                break;
            }

            /* cure serious wounds projectile */
            case PSPELL_CURE_SERIOUS+64:
            {				
                p_ptr->current_spell = PSPELL_CURE_SERIOUS;
                get_aim_dir(Ind);
                return;
            }


            case PSPELL_HERO_CHANT:
            {
                (void)set_blessed(Ind, p_ptr->blessed + randint(24) + 24);
                break;
            }

            case PSPELL_SLEEP_SANCTUARY:
            {
                (void)sleep_monsters_touch(Ind);
				break;
			}

            case PSPELL_SAT_HUNGER:
            {
                (void)set_food(Ind, PY_FOOD_MAX - 1);
                break;
            }

            case PSPELL_REMOVE_CURSE:
            {
                remove_curse(Ind);
                break;
            }

            case PSPELL_RESIST_LOWS:
            {
                (void)set_oppose_fire(Ind, p_ptr->oppose_fire + randint(10) + 10);
                (void)set_oppose_cold(Ind, p_ptr->oppose_cold + randint(10) + 10);
                break;
            }

            case PSPELL_CURE_POISON:
            {
                (void)set_poisoned(Ind, 0);
                break;
            }

            case PSPELL_ORB_DRAINING:
            {
                p_ptr->current_spell = PSPELL_ORB_DRAINING;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_CURE_CRITICAL:
            {
                (void)hp_player(Ind, damroll(6, 10));
                (void)set_cut(Ind, 0);
                break;
            }

            /* cure critical wounds projectile */
            case PSPELL_CURE_CRITICAL+64:
            {
                p_ptr->current_spell = PSPELL_CURE_CRITICAL;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_SENSE_INVISIBLE:
            {
                (void)set_tim_invis(Ind, p_ptr->tim_invis + randint(24) + 24);
                break;
            }

            case PSPELL_PROTECTION_EVIL:
            {
                (void)set_protevil(Ind, p_ptr->protevil + randint(25) + 3 * p_ptr->lev);
                break;
            }

            case PSPELL_EARTH_QUAKE:
            {
                msg_format_near(Ind, "%s murmurs, and the ground shakes!", p_ptr->name);
                earthquake(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 10);
                break;
            }

            case PSPELL_SENSE_MAP:
            {
                map_area(Ind);
                break;
            }

            case PSPELL_CURE_MORTAL:
            {
                (void)hp_player(Ind, damroll(8, 10));
                (void)set_stun(Ind, 0);
                (void)set_cut(Ind, 0);
                break;
            }
	    /* Cure mortal wounds projectile */
            case PSPELL_CURE_MORTAL+64:
            {
                p_ptr->current_spell = PSPELL_CURE_MORTAL;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_TURN_UNDEAD:
            {
                (void)turn_undead(Ind);
                break;
            }

            case PSPELL_HERO_PRAYER:
            {
                (void)set_blessed(Ind, p_ptr->blessed + randint(48) + 48);
                break;
            }

            case PSPELL_DISPEL_UNDEAD:
            {
                (void)dispel_undead(Ind, randint(plev * 3));
                break;
            }

            case PSPELL_CURE_HEAL:
            {
                (void)hp_player(Ind, 300);
                (void)set_stun(Ind, 0);
                (void)set_cut(Ind, 0);
                break;
            }
            /* Heal other projectile */
            case PSPELL_CURE_HEAL+64:
            {
                p_ptr->current_spell = PSPELL_CURE_HEAL;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_DISPEL_EVIL:
            {
                (void)dispel_evil(Ind, randint(plev * 3));
                break;
            }

            case PSPELL_GLYPH_WARDING:
            {
                msg_format_near(Ind, "%s lays down a glyph of warding.", p_ptr->name);
                warding_glyph(Ind);
                break;
            }

            case PSPELL_HOLY_WORD:
			{
				(void)dispel_evil(Ind, randint(plev * 4));
				(void)hp_player(Ind, 1000);
				(void)set_afraid(Ind, 0);
				(void)set_poisoned(Ind, 0);
				(void)set_stun(Ind, 0);
				(void)set_cut(Ind, 0);
				break;
			}

            case PSPELL_DETECT_MONSTERS:
			{
				(void)detect_creatures(Ind, TRUE);
				break;
			}

            case PSPELL_DETECT_ALL:
			{
				(void)detection(Ind);
				break;
			}

            case PSPELL_IDENTIFY_ITEM:
			{
				(void)ident_spell(Ind);
				break;
			}

            case PSPELL_PROBE_MONSTER:
			{
				(void)probing(Ind);
				break;
			}

            case PSPELL_ENLIGHT_LEVEL:
			{
				wiz_lite(Ind);
				break;
			}

            case PSPELL_CURE_SERIOUS_WOUNDS2:
			{
                (void)hp_player(Ind, damroll(4, 10));
				(void)set_cut(Ind, 0);
				break;
			}
			/* cure serious wounds2 projectile */
            case PSPELL_CURE_SERIOUS_WOUNDS2+64:
            {
                p_ptr->current_spell = PSPELL_CURE_SERIOUS_WOUNDS2;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_CURE_MORTAL_WOUNDS2:
			{
                (void)hp_player(Ind, damroll(8, 10));
				(void)set_stun(Ind, 0);
				(void)set_cut(Ind, 0);
				break;
			}
			/* Healing projectile */
            case PSPELL_CURE_MORTAL_WOUNDS2+64:
            {
                p_ptr->current_spell = PSPELL_CURE_MORTAL_WOUNDS2;
                get_aim_dir(Ind);
                return;
            }

            case PSPELL_HEALING:
			{
				(void)hp_player(Ind, 2000);
				(void)set_stun(Ind, 0);
				(void)set_cut(Ind, 0);
				break;
			}

			/* heal other spell */
            case PSPELL_HEALING+64:
			{
                p_ptr->current_spell = PSPELL_HEALING;
				get_aim_dir(Ind);
				return;
			}

			/* should ressurect ghosts now */
            case PSPELL_RESTORE_STATS:
			{
				/* if we didnt ressurect anybody */
				if (!do_scroll_life(Ind))
				{
					(void)do_res_stat(Ind, A_STR);
					(void)do_res_stat(Ind, A_INT);
					(void)do_res_stat(Ind, A_WIS);
					(void)do_res_stat(Ind, A_DEX);
					(void)do_res_stat(Ind, A_CON);
					(void)do_res_stat(Ind, A_CHR);
				}
				break;
			}

            case PSPELL_RESTORE_XP:
			{
				if (!do_restoreXP_other(Ind))
				{
					(void)restore_level(Ind);
				}
				break;
			}

            case PSPELL_DISPEL_UNDEAD2:
			{
				
				(void)dispel_undead(Ind, randint(plev * 4));
				break;
			}

            case PSPELL_DISPEL_EVIL2:
			{
				(void)dispel_evil(Ind, randint(plev * 4));
				break;
			}

            case PSPELL_BANISH_EVIL:
			{
				if (banish_evil(Ind, 100))
				{
					msg_print(Ind, "The power of your god banishes evil!");
				}
				break;
			}

            case PSPELL_WORD_DESTRUCTION:
			{
				msg_format_near(Ind, "%s unleashs a spell of great power!", p_ptr->name);
				destroy_area(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 15, TRUE);
				break;
			}

            case PSPELL_ANNIHILAT_BOLT:
			{
                p_ptr->current_spell = PSPELL_ANNIHILAT_BOLT;
				get_aim_dir(Ind);
				return;
			}

            case PSPELL_DESTROY_WAYS:
			{
				(void)destroy_doors_touch(Ind);
				break;
			}

            case PSPELL_RECHARGE_ITEM:
			{
				(void)recharge(Ind, 15);
				break;
			}

            case PSPELL_DISPEL_CURSE:
			{
				(void)remove_all_curse(Ind);
				break;
			}

            case PSPELL_ENCHANT_WEAPON:
			{
				(void)enchant_spell(Ind, rand_int(4) + 1, rand_int(4) + 1, 0);
				break;
			}

            case PSPELL_ENCHANT_ARMOR:
			{
				(void)enchant_spell(Ind, 0, 0, rand_int(3) + 2);
				break;
			}

            case PSPELL_ELEM_BRAND:
			{
				brand_weapon(Ind);
				break;
			}

            case PSPELL_TELEPORT_BLINK:
			{
				msg_format_near(Ind, "%s blinks away!", p_ptr->name);
				teleport_player(Ind, 10);
				break;
			}

            case PSPELL_TELEPORT_SELF:
			{
				msg_format_near(Ind, "%s teleports away!", p_ptr->name);
				teleport_player(Ind, plev * 8);
				break;
			}

            case PSPELL_TELEPORT_OTHER:
			{
				p_ptr->current_spell = 54;
				get_aim_dir(Ind);
				return;
			}

            case PSPELL_TELEPORT_LEVEL:
			{
				(void)teleport_player_level(Ind);
				break;
			}

            case PSPELL_WORD_RECALL:
			{
				set_recall(Ind, o_ptr);
				break;
			}

            case PSPELL_ALTER_REALITY:
			{
				/* Alter Reality */
				
				/* Which dungeon level are we changing? */
				int Depth = p_ptr->dun_depth;
        
				/* Don't allow this in towns or the wilderness */
				if( (Depth <= 0) || (check_special_level(Depth)) )
					break;
					
				/* Search for players on this depth */
				for (i = 1; i < NumPlayers + 1; i++)
				{
					player_type *p_ptr = Players[i];
					
					/* Only players on this depth */
					if(p_ptr->dun_depth == Depth)
					{
						/* Tell the player about it */
						msg_print(i, "The world changes!");
						p_ptr->new_level_flag = TRUE;
						p_ptr->new_level_method = LEVEL_RAND;
					}
				}

				/* Deallocate the level */
				dealloc_dungeon_level(Depth);
				cave[Depth] = 0;
				
				break;
			}
		}

    if(spell >= 64) j -= 64; 

		/* A prayer was prayed */
		if (!((j < 32) ?
		      (p_ptr->spell_worked1 & (1L << j)) :
		      (p_ptr->spell_worked2 & (1L << (j - 32)))))
		{
			int e = s_ptr->sexp;

			/* The spell worked */
			if (j < 32)
			{
				p_ptr->spell_worked1 |= (1L << j);
			}
			else
			{
				p_ptr->spell_worked2 |= (1L << (j - 32));
			}

			/* Gain experience */
			gain_exp(Ind, e * s_ptr->slevel);

			/* Fix the spell info */
			p_ptr->window |= PW_SPELL;
		}
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Sufficient mana */
	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print(Ind, "You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(Ind, p_ptr->paralyzed + randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool perm = (rand_int(100) < 25);

			/* Message */
			msg_print(Ind, "You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(Ind, A_CON, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

void do_cmd_pray_aux(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int plev = p_ptr->lev;
	
	magic_type *s_ptr = &p_ptr->mp_ptr->info[p_ptr->current_spell];

	/* Only fire in direction 5 if we have a target */
	if ((dir == 5) && !target_okay(Ind))
	{
		/* Reset current spell */
		p_ptr->current_spell = -1;

		/* Done */
		return;
	}

	/* We assume that the spell can be cast, and so forth */
	switch(p_ptr->current_spell)
	{
		
        case PSPELL_CURE_LIGHT:
		{
			(void)cure_light_wounds_proj(Ind, dir);
			break;
		}
	
        case PSPELL_SCARE_MONSTER:
		{
			(void)fear_monster(Ind, dir, plev);
			break;
		}

        case PSPELL_CURE_SERIOUS:
        case PSPELL_CURE_SERIOUS_WOUNDS2:
		{
			(void)cure_serious_wounds_proj(Ind, dir);
			break;
		}

        case PSPELL_ORB_DRAINING:
		{
			msg_format_near(Ind, "%s fires a holy orb!", p_ptr->name);
			fire_ball(Ind, GF_HOLY_ORB, dir,
				(damroll(3, 6) + plev +
                 (plev / ((p_ptr->pclass == CLASS_PRIEST) ? 2 : 4))),
				 ((plev < 30) ? 2 : 3));
			break;
		}

		case PSPELL_CURE_CRITICAL: 
		{
			(void)cure_critical_wounds_proj(Ind, dir);
			break;	
		}

		case PSPELL_CURE_MORTAL: 
        {
            (void)cure_mortal_wounds_proj(Ind, dir);
            break;	
        }

        case PSPELL_CURE_HEAL:
		{
			heal_other_proj(Ind,dir);
			break;
		}

		case PSPELL_CURE_MORTAL_WOUNDS2:
		{
			cure_mortal_wounds_proj(Ind,dir);
			break;
		}

        case PSPELL_HEALING:
		{
			heal_other_heavy_proj(Ind,dir);
			break;
		}

        case PSPELL_ANNIHILAT_BOLT:
		{
			drain_life(Ind, dir, 200);
			break;
		}

        case PSPELL_TELEPORT_OTHER:
		{
			(void)teleport_monster(Ind, dir);
			break;
		}

		default:   /* Something went wrong */
		{
			msg_print(Ind, "SERVER ERROR: do_cmd_prayer_aux() called for non-directional prayer!");
			p_ptr->current_spell = -1;
		}
	}

	if (!((p_ptr->current_spell < 32) ?
		(p_ptr->spell_worked1 & (1L << p_ptr->current_spell)) :
		(p_ptr->spell_worked2 & (1L << (p_ptr->current_spell - 32)))))
	{
		int e = s_ptr->sexp;

		if (p_ptr->current_spell < 32)
		{
			p_ptr->spell_worked1 |= (1L << p_ptr->current_spell);
		}
		else
		{
			p_ptr->spell_worked2 |= (1L << (p_ptr->current_spell - 32));
		}

		gain_exp(Ind, e * s_ptr->slevel);

		/* Fix the spell info */
		p_ptr->window |= PW_SPELL;
	}

	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print(Ind, "You faint from the effort!");

		/* Hack -- bypass free action */
		(void)set_paralyzed(Ind, p_ptr->paralyzed + randint(5 * oops - 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool perm = (rand_int(100) < 25);

			/* Message */
			msg_print(Ind, "You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(Ind, A_CON, 15 + randint(10), perm);
		}
	}

	/* Reset current spell */
	p_ptr->current_spell = -1;

	/* Resend mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


/*
 * Send the ghost spell info to the client.
 */
void show_ghost_spells(int Ind)
{
	magic_type *s_ptr;
	int i, j = 0;
	char out_val[80];
	cptr comment = "";

	/* Check each spell */
	for (i = 0; i < 64; i++)
	{
		s_ptr = &ghost_spells[i];

		/* Check for existance */
		if (s_ptr->slevel >= 99) continue;

		/* Format information */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
                I2A(j), spell_names[GHOST_SPELLS][i], s_ptr->slevel, s_ptr->smana, 0, comment);

		/* Send it */
		Send_spell_info(Ind, 0, j, out_val);

		/* Next spell */
		j++;
	}
}

/*
 * Use a ghostly ability. --KLJ--
 */
void do_cmd_ghost_power(int Ind, int ability)
{
	player_type *p_ptr = Players[Ind];
	magic_type *s_ptr = &ghost_spells[ability];
	int plev = p_ptr->lev;
	int i, j = 0;

	/* Check for ghost-ness */
	if (!p_ptr->ghost) return;

	/* Must not be confused */
	if (p_ptr->confused)
	{
		/* Message */
		msg_print(Ind, "You are too confused!");
		return;
	}

	/* Check spells */
	for (i = 0; i < 64; i++)
	{
		s_ptr = &ghost_spells[i];

		/* Check for existance */
		if (s_ptr->slevel >= 99) continue;

		/* Next spell */
		if (j++ == ability) break;
	}

	/* Check for level */
	if (s_ptr->slevel > plev)
	{
		/* Message */
		msg_print(Ind, "You aren't powerful enough to use that ability.");
		return;
	}

	/* Spell effects */
	switch(i)
	{
		case 0:
		{
			teleport_player(Ind, 10);
			break;
		}
		case 1:
		{
			get_aim_dir(Ind);
			p_ptr->current_spell = 1;
			return;
		}
		case 2:
		{
			get_aim_dir(Ind);
			p_ptr->current_spell = 2;
			return;
		}
		case 3:
		{
			teleport_player(Ind, plev * 8);
			break;
		}
		case 4:
		{
			get_aim_dir(Ind);
			p_ptr->current_spell = 4;
			return;
		}
		case 5:
		{
			get_aim_dir(Ind);
			p_ptr->current_spell = 5;
			return;
		}
		case 6:
		{
			get_aim_dir(Ind);
			p_ptr->current_spell = 6;
			return;
		}
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Take some experience */
	p_ptr->max_exp -= s_ptr->slevel * s_ptr->smana;
	p_ptr->exp -= s_ptr->slevel * s_ptr->smana;

	/* Too much can kill you */
	if (p_ptr->exp < 0) take_hit(Ind, 5000, "the strain of ghostly powers");

	/* Check experience levels */
	check_experience(Ind);

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


/*
 * Directional ghost ability
 */
void do_cmd_ghost_power_aux(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int plev = p_ptr->lev;
	magic_type *s_ptr;
	
	/* Verify spell number */
	if (p_ptr->current_spell < 0)
		return;

	/* Acquire spell pointer */
	s_ptr = &ghost_spells[p_ptr->current_spell];

	/* We assume everything is still OK to cast */
	switch (p_ptr->current_spell)
	{
		case 1:
		{
			(void)fear_monster(Ind, dir, plev);
			break;
		}
		case 2:
		{
			confuse_monster(Ind, dir, plev);
			break;
		}
		case 4:
		{
			fire_bolt_or_beam(Ind, plev * 2, GF_NETHER, dir, 50 + damroll(5, 5) + plev);
			break;
		}
		case 5:
		{
			fire_ball(Ind, GF_NETHER, dir, 100 + 2 * plev, 2);
			break;
		}
		case 6:
		{
			fire_ball(Ind, GF_DARK, dir, plev * 5 + damroll(10, 10), 3);
			break;
		}
	}

	/* No more spell */
	p_ptr->current_spell = -1;

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Take some experience */
	p_ptr->max_exp -= s_ptr->slevel * s_ptr->smana;
	p_ptr->exp -= s_ptr->slevel * s_ptr->smana;

	/* Too much can kill you */
	if (p_ptr->exp < 0) take_hit(Ind, 5000, "the strain of ghostly powers");

	/* Check experience levels */
	check_experience(Ind);

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}
