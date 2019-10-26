/* File: cmd5.c */

/* Purpose: Spell/Prayer commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



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
static s16b spell_chance(player_type *p_ptr, int spell)
{
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

	/* Non mage/priest characters never get better than 5 percent */
	if (!(p_ptr->cp_ptr->flags & CF_ZERO_FAIL))
	{
		if (minfail < 5) minfail = 5;
	}

	/* Priest prayer penalty for "edged" weapons (before minfail) */
	if (p_ptr->icky_wield) chance += 25;

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
static bool spell_okay(player_type *p_ptr, int j, bool known)
{
	magic_type *s_ptr;

	/* Access the spell */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* Spell is illegal */
	if (s_ptr->slevel > p_ptr->lev) return (FALSE);

	/* Spell is forgotten */
	if (p_ptr->spell_flags[j] & PY_SPELL_FORGOTTEN)
	{
		/* Never okay */
		return (FALSE);
	}

	/* Spell is learned */
	if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
	{
		/* Okay to cast, not to study */
		return (known);
	}

	/* Okay to study, not to cast */
	return (!known);
}




/*
 * Print a list of spells (for browsing or casting)
 */
static void print_spells(player_type *p_ptr, int book, byte *spell, int num)
{
	int			i, j;

	magic_type		*s_ptr;

	cptr		comment;

	char		out_val[160];

	byte flag, item_tester;

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
			send_spell_info(p_ptr, book, i, 0, 0, out_val);
			continue;
		}

		/* XXX XXX Could label spells above the players level */

		/* Get extra info */
		comment = get_spell_info(p_ptr, j);

		/* Analyze the spell */
		if (p_ptr->spell_flags[j] & PY_SPELL_FORGOTTEN)
		{
			comment = " forgotten";
		}
		else if (!(p_ptr->spell_flags[j] & PY_SPELL_LEARNED))
		{
			comment = " unknown";
		}
		else if (!(p_ptr->spell_flags[j] & PY_SPELL_WORKED))
		{
			comment = " untried";
		}

		/* Get flag and item tester */
		flag = get_spell_flag(p_ptr->cp_ptr->spell_book, j, p_ptr->spell_flags[j], &item_tester);

		/* Dump the spell --(-- */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
		        I2A(i), get_spell_name(p_ptr->cp_ptr->spell_book, j),
		        s_ptr->slevel, s_ptr->smana, spell_chance(p_ptr, j), comment);
		send_spell_info(p_ptr, book, i, flag, item_tester, out_val);
	}
}



/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 */
void do_cmd_browse(player_type *p_ptr, int book)
{
	int			i, item;
	int spell;
	int num = 0;

	byte		spells[PY_MAX_SPELLS];


	object_type		*o_ptr;

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !is_dm_p(p_ptr) )
	{
		msg_print(p_ptr, "You cannot read books!");
		return;
	}

	/* Warriors are illiterate */
	if (!p_ptr->cp_ptr->spell_book)
	{
		msg_print(p_ptr, "You cannot read books!");
		return;
	}

#if 0
	/* No lite */
	if (p_ptr->blind || no_lite(p_ptr))
	{
		msg_print(p_ptr, "You cannot see!");
		return;
	}

	/* Confused */
	if (p_ptr->confused)
	{
		msg_print(p_ptr, "You are too confused!");
		return;
	}
#endif


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
			msg_print(p_ptr, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - item];
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Tried browsing a bad book */
		return;
	}



	/* Extract spells */
	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		spell = get_spell_index(p_ptr, o_ptr, i);

		/* Collect this spell */
		if (spell != -1) spells[num++] = spell;

		/* Hack? -- stop looking after first "-1" spell */
		else break;
	}

	/* Display the spells */
	print_spells(p_ptr, book, spells, num);
}




/*
 * Study a book to gain a new spell/prayer
 */
void do_cmd_study(player_type *p_ptr, int book, int spell)
{
	int			i, sval;

	int			j = -1;

	cptr p = ((p_ptr->cp_ptr->spell_book == TV_PRAYER_BOOK) ? "prayer" : "spell");

	object_type		*o_ptr;

	byte spells[PY_MAX_SPELLS], num = 0;
	int index; /* spell index */

	/* Check preventive inscription '^G' */
	__trap(p_ptr, CPI(p_ptr, 'G'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !is_dm_p(p_ptr) )
	{
		msg_print(p_ptr, "You cannot read books!");
		return;
	}

	if (!p_ptr->cp_ptr->spell_book)
	{
		msg_print(p_ptr, "You cannot read books!");
		return;
	}

	if (p_ptr->blind || no_lite(p_ptr))
	{
		msg_print(p_ptr, "You cannot see!");
		return;
	}

	if (p_ptr->confused)
	{
		msg_print(p_ptr, "You are too confused!");
		return;
	}

	if (!(p_ptr->new_spells))
	{
		msg_format(p_ptr, "You cannot learn any new %ss!", p);
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
			msg_print(p_ptr, "There's nothing on the floor.");
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
	if (!strcmp(p, "spell"))
	{
		for (i = 0; i < SPELLS_PER_BOOK; i++)
		{
			index = get_spell_index(p_ptr, o_ptr, i);

			/* Collect this spell */
			if (index != -1) spells[num++] = index;
		}

		/* Set the spell number */
		j = spells[spell];

		if (!spell_okay(p_ptr, j, FALSE))
		{
			msg_print(p_ptr, "You cannot gain that spell!");
			return;
		}
	}

	/* Cleric -- Learn a random prayer */
	if (!strcmp(p,"prayer"))
	{
		int k = 0;

		int gift = -1;

		/* Extract spells */
		for (i = 0; i < SPELLS_PER_BOOK; i++)
		{
			index = get_spell_index(p_ptr, o_ptr, i);

			/* Skip empty */
			if (index == -1) continue;

			/* Skip non "okay" prayers */
			if (!spell_okay(p_ptr, index, FALSE)) continue;

			/* Hack -- Apply the randomizer */
			if ((++k > 1) && (randint0(k) != 0)) continue;

			/* Track it */
			gift = index;
		}
		/* Accept gift */
		j = gift;
	}

	/* Nothing to study */
	if (j < 0)
	{
		/* Message */
		msg_format(p_ptr, "You cannot learn any %ss in that book.", p);

		/* Abort */
		return;
	}


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Learn the spell */
	p_ptr->spell_flags[j] |= PY_SPELL_LEARNED;

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* Stop at the first empty space */
		if (p_ptr->spell_order[i] == 99) break;
	}

	/* Add the spell to the known list */
	p_ptr->spell_order[i++] = j;

	/* Mention the result */
	msg_format(p_ptr, "You have learned the %s of %s.",
	           p, get_spell_name(p_ptr->cp_ptr->spell_book, j));
	sound(p_ptr, MSG_STUDY);

	/* One less spell available */
	p_ptr->new_spells--;

	/* Report on remaining prayers */
	if (p_ptr->new_spells)
	{
		msg_format(p_ptr, "You can learn %d more %ss.", p_ptr->new_spells, p);
	}

	/* Save the new_spells value */
	p_ptr->old_spells = p_ptr->new_spells;

	/* Redraw Study Status */
	p_ptr->redraw |= (PR_STUDY);

	/* Update the spell info */
	p_ptr->window |= (PW_SPELL);
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
void do_cmd_cast_pre(player_type *p_ptr, int book, int dir, int spell)
{
	if (dir < 0 && dir > -11)
		p_ptr->command_dir = -dir;
	else
		p_ptr->command_arg = dir;

	do_cmd_cast(p_ptr, book, spell);
}
void do_cmd_cast(player_type *p_ptr, int book, int spell)
{
	int			i, j, sval;
	int			chance, index;

	object_type		*o_ptr;

	magic_type		*s_ptr;

	byte spells[PY_MAX_SPELLS], num = 0;
	byte item_tester = 0;

	/* Check preventive inscription '^m' */
	__trap(p_ptr, CPI(p_ptr, 'm'));

	/* Require spell ability */
	if (p_ptr->cp_ptr->spell_book != TV_MAGIC_BOOK)
	{
		msg_print(p_ptr, "You cannot cast spells!");
		return;
	}

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !is_dm_p(p_ptr) )
	{
		msg_print(p_ptr, "You cannot cast spells!");
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite(p_ptr))
	{
		msg_print(p_ptr, "You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print(p_ptr, "You are too confused!");
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
			msg_print(p_ptr, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - book];
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Tried to cast spell from bad book */
		return;
	}

	/* Check guard inscription '!m' */
	__trap(p_ptr, CGI(o_ptr, 'm'));

	/* Access the item's sval */
	sval = o_ptr->sval;

	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		index = get_spell_index(p_ptr, o_ptr, i);
		
		/* Collect this spell */
 		if (index != -1) spells[num++] = index;
	}

	/* Set the spell number */
	j = spells[spell];

	/* Projected spells */
	if (spell >= SPELL_PROJECTED)
	{
		j = spells[spell - SPELL_PROJECTED];
		if (!(get_spell_flag(o_ptr->tval, j, PY_SPELL_LEARNED, &item_tester) & PY_SPELL_PROJECT))
		{
			msg_print(p_ptr, "You cannot project that spell.");
			return;
		}
	}

	/* Regular test */
	if (!spell_okay(p_ptr, j, 1))
	{
		msg_print(p_ptr, "You cannot cast that spell.");
		return;
	}

	/* Access the spell */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* Check mana */
	if (s_ptr->smana > p_ptr->csp)
	{
		msg_print(p_ptr, "You do not have enough mana.");
		return;
	}

	/* Spell failure chance */
	chance = spell_chance(p_ptr, j);

	/* Add "projection" offset */
	if (spell >= SPELL_PROJECTED)
	{
		j += SPELL_PROJECTED;
		chance -= chance * PROJECTED_CHANCE_RATIO / 100;
	}

	/* Failed spell */
	if (randint0(100) < chance)
	{
		/*if (flush_failure) flush();*/
		msg_print(p_ptr, "You failed to get the spell off!");
		/* Hack: Spend Mana */
		p_ptr->current_spell = j;
		do_cmd_cast_fin(p_ptr, FALSE);
		return;
	}

	/* Cast spell */
	sound(p_ptr, MSG_SPELL);
	cast_spell(p_ptr, p_ptr->cp_ptr->spell_book, j);
}
void do_cmd_cast_fin(player_type *p_ptr, bool tried)
{
	magic_type *s_ptr;
	int j = p_ptr->current_spell;

	/* Remove "projection" offset */
	if (j >= SPELL_PROJECTED) j -= SPELL_PROJECTED;

	/* Set pointer */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* A spell was tried */
	if (!(p_ptr->spell_flags[j] & PY_SPELL_WORKED) && tried)
	{
		int e = s_ptr->sexp;

		/* The spell worked */
		p_ptr->spell_flags[j] |= PY_SPELL_WORKED;

		/* Gain experience */
		gain_exp(p_ptr, e * s_ptr->slevel);

		/* Fix the spell info */
		p_ptr->window |= PW_SPELL;
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
		msg_print(p_ptr, "You faint from the effort!");

		/* Hack -- bypass free action */
		(void)set_paralyzed(p_ptr, p_ptr->paralyzed + randint1(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (randint0(100) < 50)
		{
			bool perm = (randint0(100) < 25);

			/* Message */
			msg_print(p_ptr, "You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(p_ptr, A_CON, 15 + randint1(10), perm);
		}
	}

	/* Resend mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

/*
 * Pray a prayer
 *
 * See above for an explanation of the weirdness in this function --KLJ--
 *
 * After prayer is cast, do_cmd_cast_fin() will be called !
 * If we need to separate them later, we could...
 */
void do_cmd_pray_pre(player_type *p_ptr, int book, int dir, int spell)
{
	if (dir < 0 && dir > -11)
		p_ptr->command_dir = -dir;
	else
		p_ptr->command_arg = dir;

	do_cmd_pray(p_ptr, book, spell);
}
void do_cmd_pray(player_type *p_ptr, int book, int spell)
{
	int item, sval, j, chance, i;
	int index;

	object_type	*o_ptr;

	magic_type  *s_ptr;

	byte spells[PY_MAX_SPELLS], num = 0;
	byte item_tester = 0;

	/* Check preventive inscription '^p' */
	__trap(p_ptr, CPI(p_ptr, 'p'));

	/* Restrict ghosts */
	if (p_ptr->ghost || p_ptr->fruit_bat)
	{
		msg_print(p_ptr, "Pray hard enough and your prayers may be answered.");
		return;
	}

	/* Must use prayer books */
	if (p_ptr->cp_ptr->spell_book != TV_PRAYER_BOOK)
	{
		msg_print(p_ptr, "Pray hard enough and your prayers may be answered.");
		return;
	}

	/* Must have lite */
	if (p_ptr->blind || no_lite(p_ptr))
	{
		msg_print(p_ptr, "You cannot see!");
		return;
	}

	/* Must not be confused */
	if (p_ptr->confused)
	{
		msg_print(p_ptr, "You are too confused!");
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
		if (item == 0)
		{
			msg_print(p_ptr, "There's nothing on the floor.");
			return;
		}
		o_ptr = &o_list[0 - item];
	}

	if (o_ptr->tval != p_ptr->cp_ptr->spell_book)
	{
		/* Tried to pray prayer from bad book */
		return;
	}

	/* Check guard inscription '!p' */
	__trap(p_ptr, CGI(o_ptr, 'p'));

	/* Access the item's sval */
	sval = o_ptr->sval;

	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		index = get_spell_index(p_ptr, o_ptr, i);

		/* Collect this spell */
		if (index != -1) spells[num++] = index;
	}

	/* Set the prayer number */
	j = spells[spell];

	/* Projected prayers */
	if (spell >= SPELL_PROJECTED)
	{
		j = spells[spell - SPELL_PROJECTED];
		if (!(get_spell_flag(o_ptr->tval, j, PY_SPELL_LEARNED, &item_tester) & PY_SPELL_PROJECT))
		{
			msg_print(p_ptr, "You cannot project that prayer.");
			return;
		}
	}

	/* Regular test */
	if (!spell_okay(p_ptr, j, 1))
	{
		msg_print(p_ptr, "You cannot pray that prayer.");
		return;
	}

	/* get the spell info */
	s_ptr = &p_ptr->mp_ptr->info[j];

	/* Check mana */
	if (s_ptr->smana > p_ptr->csp)
	{
		msg_print(p_ptr, "You do not have enough mana.");
		return;
	}

	/* Spell failure chance */
	chance = spell_chance(p_ptr, j);

	/* Add "projection" offset */
	if (spell >= SPELL_PROJECTED) 
	{
		j += SPELL_PROJECTED;
		chance -= chance * PROJECTED_CHANCE_RATIO / 100;
	}

	/* Check for failure */
	if (randint0(100) < chance)
	{
		/*if (flush_failure) flush();*/
		msg_print(p_ptr, "You failed to concentrate hard enough!");
		/* Hack: Spend Mana */
		p_ptr->current_spell = j;
		do_cmd_cast_fin(p_ptr, FALSE);
		return;
	}

	/* Success */
	sound(p_ptr, MSG_PRAYER);
	if (!cast_spell(p_ptr, p_ptr->cp_ptr->spell_book, j)) return;
}


/*
 * Send the ghost spell info to the client.
 */
void show_ghost_spells(player_type *p_ptr)
{
	magic_type *s_ptr;
	int i;
	char out_val[80];
	byte flag, item_tester;

	/* Check each spell */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		s_ptr = &ghost_spells[i];

		/* Check for existance */
		if (s_ptr->slevel >= 99) break;

		/* Format information */
		sprintf(out_val, "  %c) %-30s%2d %4d %3d",
                I2A(i), spell_names[GHOST_REALM][i], s_ptr->slevel, s_ptr->smana, 0);

		/* Get extra info */
		flag = get_spell_flag(0, i, (PY_SPELL_LEARNED | PY_SPELL_WORKED), &item_tester);

		/* Send it */
		send_spell_info(p_ptr, 10, i, flag, item_tester, out_val);
	}
}

/*
 * Use a ghostly ability. --KLJ--
 */
void do_cmd_ghost_power_pre(player_type *p_ptr, int dir, int ability)
{
	if (dir < 0 && dir > -11)
		p_ptr->command_dir = -dir;
	else
		p_ptr->command_arg = dir;

	do_cmd_ghost_power(p_ptr, ability);
}
void do_cmd_ghost_power(player_type *p_ptr, int ability)
{
	magic_type *s_ptr = &ghost_spells[ability];
	int plev = p_ptr->lev;
	int i, j = 0;

	/* Check for ghost-ness */
	if (!p_ptr->ghost) return;

	/* Must not be confused */
	if (p_ptr->confused)
	{
		/* Message */
		msg_print(p_ptr, "You are too confused!");
		return;
	}

	/* Check spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		s_ptr = &ghost_spells[i];

		/* Check for existance */
		if (s_ptr->slevel >= 99) break;

		/* Next spell */
		if (j++ == ability) break;
	}

	/* Check for level */
	if (s_ptr->slevel > plev)
	{
		/* Message */
		msg_print(p_ptr, "You aren't powerful enough to use that ability.");
		return;
	}

	/* Cast ghost spell */
	cast_spell(p_ptr, -1, i);
}
void do_cmd_ghost_power_fin(player_type *p_ptr)
{
	magic_type *s_ptr;

	/* Verify spell number */
	if (p_ptr->current_spell < 0)
		return;

	/* Acquire spell pointer */
	s_ptr = &ghost_spells[p_ptr->current_spell];

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Take some experience */
	p_ptr->max_exp -= s_ptr->slevel * s_ptr->smana;
	p_ptr->exp -= s_ptr->slevel * s_ptr->smana;

	/* Too much can kill you */
	if (p_ptr->exp < 0) take_hit(p_ptr, 5000, "the strain of ghostly powers");

	/* Check experience levels */
	check_experience(p_ptr);

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}
