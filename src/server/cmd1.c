/* File: cmd1.c */

/* Purpose: Movement commands (part 1) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = randint0(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Never hit */
	if (chance <= 0) return (FALSE);

	/* Invisible monsters are harder to hit */
	if (!vis) chance = (chance + 1) / 2;

	/* Power competes against armor */
	if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

	/* Assume hit */
	return (TRUE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = randint0(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Wimpy attack never hits */
	if (chance <= 0) return (FALSE);

	/* Penalize invisible targets */
	if (!vis) chance = (chance + 1) / 2;

	/* Power must defeat armor */
	if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

	/* Assume hit */
	return (TRUE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
s16b critical_shot(player_type *p_ptr, int weight, int plus, int dam)
{
	int i, k;

	/* Extract "shot" power */
	i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));

	/* Critical hit */
	if (randint1(5000) <= i)
	{
		k = weight + randint1(500);

		if (k < 500)
		{
			msg_print(p_ptr, "It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 1000)
		{
			msg_print(p_ptr, "It was a great hit!");
			dam = 2 * dam + 10;
		}
		else
		{
			msg_print(p_ptr, "It was a superb hit!");
			dam = 3 * dam + 15;
		}
	}

	return (dam);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
s16b critical_norm(player_type *p_ptr, int weight, int plus, int dam)
{
	int i, k;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));

	/* Chance */
	if (randint1(5000) <= i)
	{
		k = weight + randint1(650);

		if (k < 400)
		{
			msg_print(p_ptr, "It was a good hit!");
			sound(p_ptr, MSG_HIT_GOOD);
			dam = 2 * dam + 5;
		}
		else if (k < 700)
		{
			msg_print(p_ptr, "It was a great hit!");
			sound(p_ptr, MSG_HIT_GREAT);
			dam = 2 * dam + 10;
		}
		else if (k < 900)
		{
			msg_print(p_ptr, "It was a superb hit!");
			sound(p_ptr, MSG_HIT_SUPERB);
			dam = 3 * dam + 15;
		}
		else if (k < 1300)
		{
			msg_print(p_ptr, "It was a *GREAT* hit!");
			sound(p_ptr, MSG_HIT_HI_GREAT);
			dam = 3 * dam + 20;
		}
		else
		{
			msg_print(p_ptr, "It was a *SUPERB* hit!");
			sound(p_ptr, MSG_HIT_HI_SUPERB);
			dam = ((7 * dam) / 2) + 25;
		}
	}

	sound(p_ptr, MSG_HIT);
	return (dam);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 * 
 * Note that this function also allows player to learn more about monster,
 * so a player index is passed to populate the lore struct. (ml--mon_vis)
 */
s16b tot_dam_aux(player_type *p_ptr, object_type *o_ptr, int tdam, monster_type *m_ptr, bool ml)
{
	int mult = 1;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = p_ptr->l_list + m_ptr->r_idx;
	
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Some "weapons" and "ammo" do extra damage */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Slay Animal */
			if ((f1 & TR1_SLAY_ANIMAL) &&
			    (r_ptr->flags3 & RF3_ANIMAL))
			{
				if (ml) l_ptr->flags3 |= RF3_ANIMAL;

				if (mult < 2) mult = 2;
			}

			/* Slay Evil */
			if ((f1 & TR1_SLAY_EVIL) &&
			    (r_ptr->flags3 & RF3_EVIL))
			{
				if (ml) l_ptr->flags3 |= RF3_EVIL;

				if (mult < 2) mult = 2;
			}

			/* Slay Undead */
			if ((f1 & TR1_SLAY_UNDEAD) &&
			    (r_ptr->flags3 & RF3_UNDEAD))
			{
				if (ml) l_ptr->flags3 |= RF3_UNDEAD;

				if (mult < 3) mult = 3;
			}

			/* Slay Demon */
			if ((f1 & TR1_SLAY_DEMON) &&
			    (r_ptr->flags3 & RF3_DEMON))
			{
				if (ml) l_ptr->flags3 |= RF3_DEMON;

				if (mult < 3) mult = 3;
			}

			/* Slay Orc */
			if ((f1 & TR1_SLAY_ORC) &&
			    (r_ptr->flags3 & RF3_ORC))
			{
				if (ml) l_ptr->flags3 |= RF3_ORC;

				if (mult < 3) mult = 3;
			}

			/* Slay Troll */
			if ((f1 & TR1_SLAY_TROLL) &&
			    (r_ptr->flags3 & RF3_TROLL))
			{
				if (ml) l_ptr->flags3 |= RF3_TROLL;

				if (mult < 3) mult = 3;
			}

			/* Slay Giant */
			if ((f1 & TR1_SLAY_GIANT) &&
			    (r_ptr->flags3 & RF3_GIANT))
			{
				if (ml) l_ptr->flags3 |= RF3_GIANT;

				if (mult < 3) mult = 3;
			}

			/* Slay Dragon  */
			if ((f1 & TR1_SLAY_DRAGON) &&
			    (r_ptr->flags3 & RF3_DRAGON))
			{
				if (ml) l_ptr->flags3 |= RF3_DRAGON;

				if (mult < 3) mult = 3;
			}

			/* Execute Dragon */
			if ((f1 & TR1_KILL_DRAGON) &&
			    (r_ptr->flags3 & RF3_DRAGON))
			{
				if (ml) l_ptr->flags3 |= RF3_DRAGON;

				if (mult < 5) mult = 5;
			}

			/* Execute Demon */
			if ((f1 & TR1_KILL_DEMON) &&
			    (r_ptr->flags3 & RF3_DEMON))
			{
				if (ml) l_ptr->flags3 |= RF3_DEMON;

				if (mult < 5) mult = 5;
			}

			/* Execute Undead */
			if ((f1 & TR1_KILL_UNDEAD) &&
			    (r_ptr->flags3 & RF3_UNDEAD))
			{
				if (ml) l_ptr->flags3 |= RF3_UNDEAD;

				if (mult < 5) mult = 5;
			}


			/* Brand (Acid) */
			if (f1 & TR1_BRAND_ACID)
			{
				/* Notice immunity */
				if (r_ptr->flags3 & RF3_IM_ACID)
				{
					if (ml) l_ptr->flags3 |= RF3_IM_ACID;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Elec) */
			if (f1 & TR1_BRAND_ELEC)
			{
				/* Notice immunity */
				if (r_ptr->flags3 & RF3_IM_ELEC)
				{
					if (ml) l_ptr->flags3 |= RF3_IM_ELEC;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Fire) */
			if (f1 & TR1_BRAND_FIRE)
			{
				/* Notice immunity */
				if (r_ptr->flags3 & RF3_IM_FIRE)
				{
					if (ml) l_ptr->flags3 |= RF3_IM_FIRE;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Cold) */
			if (f1 & TR1_BRAND_COLD)
			{
				/* Notice immunity */
				if (r_ptr->flags3 & RF3_IM_COLD)
				{
					if (ml) l_ptr->flags3 |= RF3_IM_COLD;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Poison) */
			if (f1 & TR1_BRAND_POIS)
			{
				/* Notice immunity */
				if (r_ptr->flags3 & RF3_IM_POIS)
				{
					if (ml) l_ptr->flags3 |= RF3_IM_POIS;
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			break;
		}
	}


	/* Return the total damage */
	return (tdam * mult);
}

/*
 * Extract the "total damage" from a given object hitting a given player.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 */
s16b tot_dam_aux_player(object_type *o_ptr, int tdam, player_type *p_ptr)
{
	int mult = 1;

	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Some "weapons" and "ammo" do extra damage */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			/* Brand (Acid) */
			if (f1 & TR1_BRAND_ACID)
			{
				/* Notice immunity */
				if (p_ptr->immune_acid)
				{
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Elec) */
			if (f1 & TR1_BRAND_ELEC)
			{
				/* Notice immunity */
				if (p_ptr->immune_elec)
				{
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Fire) */
			if (f1 & TR1_BRAND_FIRE)
			{
				/* Notice immunity */
				if (p_ptr->immune_fire)
				{
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Cold) */
			if (f1 & TR1_BRAND_COLD)
			{
				/* Notice immunity */
				if (p_ptr->immune_cold)
				{
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Poison) */
			if (f1 & TR1_BRAND_POIS)
			{
				/* Notice resistance */
				if (p_ptr->resist_pois)
				{
				}

				/* Otherwise, take the damage */
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			break;
		}
	}


	/* Return the total damage */
	return (tdam * mult);
}

/*
 * Searches for hidden things.			-RAK-
 */

void search(player_type *p_ptr)
{
	int Depth = p_ptr->dun_depth;

	int           y, x, chance;

	cave_type    *c_ptr;
	object_type  *o_ptr;


	/* Start with base search ability */
	chance = p_ptr->skill_srh;

	/* Penalize various conditions */
	if (p_ptr->blind || no_lite(p_ptr)) chance = chance / 10;
	if (p_ptr->confused || p_ptr->image) chance = chance / 10;

	/* Search the nearby grids, which are always in bounds */
	
	for (y = (p_ptr->py - 1); y <= (p_ptr->py + 1); y++)
	{
		for (x = (p_ptr->px - 1); x <= (p_ptr->px + 1); x++)
		{
			/* Sometimes, notice things */
			if (randint0(100) < chance)
			{
				/* Access the grid */
				c_ptr = &cave[Depth][y][x];

				/* Access the object */
				o_ptr = &o_list[c_ptr->o_idx];

				/* Invisible trap */
				if (c_ptr->feat == FEAT_INVIS)
				{
					/* Pick a trap */
					pick_trap(Depth, y, x);

					/* Message */
					msg_print(p_ptr, "You have found a trap.");

					/* Disturb */
					disturb(p_ptr, 0, 0);
				}

				/* Secret door */
				else if (c_ptr->feat == FEAT_SECRET)
				{
					/* Message */
					msg_print(p_ptr, "You have found a secret door.");

					/* Pick a door XXX XXX XXX */
					place_closed_door(Depth, y, x);

					/* Disturb */
					disturb(p_ptr, 0, 0);

					/* Give the player an EXP bump */
					gain_exp(p_ptr, 1);
				}

				/* Search chests */
				else if (o_ptr->tval == TV_CHEST)
				{
					/* Examine chests for traps */
					if (!object_known_p(p_ptr, o_ptr) && (chest_traps[o_ptr->pval]))
					{
						/* Message */
						msg_print(p_ptr, "You have discovered a trap on the chest!");

						/* Know the trap */
						object_known(o_ptr);

						/* Notice it */
						disturb(p_ptr, 0, 0);
					}
				}
			}
		}
	}
}


/*
 * Determine if the object can be picked up, and has "=g" in its inscription.
 *
 * This function wall also check if "pickup_inven" option is applicable.
 */
static bool auto_pickup_okay(const player_type *p_ptr, const object_type *o_ptr)
{
	int i;
	cptr s;

	/* It can't be carried */
	//if (!inven_carry_okay(o_ptr)) return (FALSE);

	/* No inscription */
	//if (!o_ptr->note) return (FALSE);

	/* Find a '=' */
	s = o_ptr->note ? strchr(quark_str(o_ptr->note), '=') : NULL;

	/* Process inscription */
	while (s)
	{
		/* Auto-pickup on "=g" */
		if (s[1] == 'g') return (TRUE);

		/* Find another '=' */
		s = strchr(s + 1, '=');
	}

	/* Hack -- also handle "pickup_inven" option */
	if (option_p(p_ptr,PICKUP_INVEN))
	{
		/* Compare each inventory item :( */
		for (i = 0; i < INVEN_PACK; i++)
		{
			/* Skip non-items */
			if (!p_ptr->inventory[i].k_idx) continue;

			/* Stackable? */
			if (object_similar(p_ptr, &p_ptr->inventory[i], o_ptr))
			{
				return (TRUE);
			}
		}
	}

	/* Don't auto pickup */
	return (FALSE);
}



/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(player_type *p_ptr, int pickup, int confirm)
{
	int Depth = p_ptr->dun_depth;

	cave_type  *c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];

	object_type *o_ptr;

	char	o_name[120];
	int 	sound_msg;

	/* Hack -- nothing here to pick up */
	if (!(c_ptr->o_idx)) return;

	if (!(p_ptr->dm_flags & DM_GHOST_BODY)) {
		/* Normal Ghosts cannot pick things up */
		if ((p_ptr->ghost) || (p_ptr->fruit_bat) ) return;
	};

	/* Paralyzed players can't pick things up */
	if (p_ptr->paralyzed) return;

	/* Get the object */
	o_ptr = &o_list[c_ptr->o_idx];

	/* Describe the object */
	object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 3);
	
	/* Check for auto-pickup */
	if (auto_pickup_okay(p_ptr, o_ptr)) pickup = 1;

	/* Pick up gold */
	if (o_ptr->tval == TV_GOLD)
	{
		/* Hack -- ghosts don't autopick gold even if they could */
		if (p_ptr->ghost && !pickup) return;
	
		/* Disturb */
		disturb(p_ptr, 0, 0);

		/* MEGA HACK -- Change "owner" of this pile */
		object_own(p_ptr, o_ptr);

		/* Determine which sound to play */
		if (o_ptr->pval < 200) sound_msg = MSG_MONEY1;
		else if (o_ptr->pval < 600) sound_msg = MSG_MONEY2;
		else sound_msg = MSG_MONEY3;
		sound(p_ptr, sound_msg);

		/* Message */
		msg_format(p_ptr, "You have found %ld gold piece%s worth of %s.",
		           (long)o_ptr->pval,
		           (o_ptr->pval == 1 ? "" : "s"),
		           o_name);

		/* Collect the gold */
		p_ptr->au += o_ptr->pval;

		/* Redraw gold */
		p_ptr->redraw |= (PR_GOLD);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Delete gold */
		delete_object(Depth, p_ptr->py, p_ptr->px);

		/* Tell the client */
		floor_item_notify(p_ptr, 0, FALSE);
	}

	/* Pick it up */
	else
	{
		/* Hack -- disturb */
		if (!p_ptr->ghost)
			disturb(p_ptr, 0, 0);

		/* Refresh floor */
		floor_item_notify(p_ptr, c_ptr->o_idx, FALSE);
		
		/* Describe the object */
		if (!pickup)
		{
			if (!p_ptr->ghost)
				msg_format(p_ptr, "You see %s.", o_name);
		}

		/* Note that the pack is too full */
		else if (!inven_carry_okay(p_ptr, o_ptr))
		{
			msg_format(p_ptr, "You have no room for %s.", o_name);
		}

		/* Pick up the item (if requested and allowed) */
		else
		{
			int okay = TRUE;

			if (protected_p(p_ptr, o_ptr, 'g')) 
			{
				/* Major Hack -- allow purchase from floor */
				s32b price;
				okay = FALSE;
				if ((price = player_price_item(p_ptr, o_ptr)) >= 0)
				{
					if (!confirm)
					{
						char out_val[160];
						object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 2); /* shorten name */
						sprintf(out_val, "Purchase %s for %ld gold? ", o_name, (long)price);
						send_pickup_check(p_ptr, out_val);
						return;
					} 
					else 
					{
						/* Attempt to buy this object */
						player_type *q_ptr;
						int i;
						/* Seller must be online */
						for (i = 1; i <= NumPlayers; i++)
						{
							q_ptr = Players[i];
							if (obj_own_p(q_ptr,o_ptr))
							{
								okay = p_ptr->play_los[i];
								break;
							}
						}
						if (!okay)
						{
							msg_print(p_ptr, "Item owner must be nearby!");
							return;
						}
						else if (p_ptr->au < price)
						{
							/* Simple message (no insult) */
							msg_print(p_ptr, "You do not have enough gold.");
							return;
						}
						else
						{
							char msg[80];
							/* Perfrom the transaction */
							p_ptr->au -= price;
							q_ptr->au += price;
							p_ptr->redraw |= PR_GOLD;
							q_ptr->redraw |= PR_GOLD;
							/* ALLOW PICKUP! */
							o_ptr->note = 0;
							/* Message */
							object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 2);/* short name */
							msg_format(p_ptr, "You bought %s for %ld gold.", o_name, (long)price);
							msg_format(q_ptr, "You sold %s for %ld gold.", o_name, (long)price);
							/* Mark artifact as sold */
							if (true_artifact_p(o_ptr)) set_artifact_p(q_ptr, o_ptr->name1, ARTS_SOLD);
							/* Audit */
							sprintf(msg, "PS %s-%d | %s-%d $ %ld",
									p_ptr->name, (int)p_ptr->id,
									q_ptr->name, (int)q_ptr->id,
									(long)price);
							audit(msg);
							audit("PS+gold");
							okay = TRUE;
						}
					}
				}
				/* Finalize hack */
				__trap(p_ptr, !okay);
			}

			/* Hack -- query every item */
			if (option_p(p_ptr,CARRY_QUERY_FLAG) && !confirm)
			{
				char out_val[160];
				sprintf(out_val, "Pick up %s? ", o_name);
				send_pickup_check(p_ptr, out_val);
				return;
			}

			/* Attempt to pick up an object. */
			if (okay)
			{
				int slot;

				/* Carry the item */
				slot = inven_carry(p_ptr, o_ptr);

				/* Get the item again */
				o_ptr = &(p_ptr->inventory[slot]);

				/* Describe the object */
				object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 3);

				/* Message */
				msg_format(p_ptr, "You have %s (%c).", o_name, index_to_label(slot));

				/* Delete original */
				delete_object(Depth, p_ptr->py, p_ptr->px);

				/* Tell the client */
				floor_item_notify(p_ptr, 0, FALSE);
			}
		}
	}
}





/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int check_hit(player_type *p_ptr, int power)
{
	int k, ac;

	/* Percentile dice */
	k = randint0(100);

	/* Hack -- 5% hit, 5% miss */
	if (k < 10) return (k < 5);

	/* Paranoia -- No power */
	if (power <= 0) return (FALSE);

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Power competes against Armor */
	if (randint1(power) > ((ac * 3) / 4)) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Handle player hitting a real trap
 */
static void hit_trap(player_type *p_ptr)
{
	int Depth = p_ptr->dun_depth;

	int			i, num, dam;

	cave_type		*c_ptr;
	byte			*w_ptr;

	cptr		name = "a trap";


	/* Ghosts ignore traps */
	if ((p_ptr->ghost) || (p_ptr->fruit_bat)) return;

	/* Disturb the player */
	disturb(p_ptr, 0, 0);

	/* Get the cave grid */
	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];
	w_ptr = &p_ptr->cave_flag[p_ptr->py][p_ptr->px];

	/* Analyze XXX XXX XXX */
	switch (c_ptr->feat)
	{
		case FEAT_TRAP_HEAD + 0x00:
		{
			/* MEGAHACK: Ignore Wilderness (or Questlevels) trap doors. */
			if( p_ptr->dun_depth<0 || is_quest_level(p_ptr, Depth)) {
				msg_print(p_ptr, "You feel quite certain something really awfull just happened..");
				break;
			}

			msg_print(p_ptr, "You fell through a trap door!");
			if (p_ptr->feather_fall)
			{
				msg_print(p_ptr, "You float gently down to the next level.");
			}
			else
			{
				dam = damroll(2, 8);
				take_hit(p_ptr, dam, name);
			}
			p_ptr->new_level_flag = TRUE;
			p_ptr->new_level_method = LEVEL_RAND;
			
			/* The player is gone */
			cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0;

			/* Erase his light */
			forget_lite(p_ptr);

			/* Show everyone that he's left */
			everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

			/* Tell everyone to re-calculate visiblity for this player */
			update_player(p_ptr);

			/* Reduce the number of players on this depth */
			players_on_depth[p_ptr->dun_depth]--;

			p_ptr->dun_depth++;

			/* Increase the number of players on this next depth */
			players_on_depth[p_ptr->dun_depth]++;

			break;
		}

		case FEAT_TRAP_HEAD + 0x01:
		{
			msg_print(p_ptr, "You fell into a pit!");
			if (p_ptr->feather_fall)
			{
				msg_print(p_ptr, "You float gently to the bottom of the pit.");
			}
			else
			{
				dam = damroll(2, 6);
				take_hit(p_ptr, dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x02:
		{
			msg_print(p_ptr, "You fall into a spiked pit!");

			if (p_ptr->feather_fall)
			{
				msg_print(p_ptr, "You float gently to the floor of the pit.");
				msg_print(p_ptr, "You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (randint0(100) < 50)
				{
					msg_print(p_ptr, "You are impaled!");

					dam = dam * 2;
					(void)set_cut(p_ptr, p_ptr->cut + randint1(dam));
				}

				/* Take the damage */
				take_hit(p_ptr, dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x03:
		{
			msg_print(p_ptr, "You fall into a spiked pit!");

			if (p_ptr->feather_fall)
			{
				msg_print(p_ptr, "You float gently to the floor of the pit.");
				msg_print(p_ptr, "You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (randint0(100) < 50)
				{
					msg_print(p_ptr, "You are impaled on poisonous spikes!");

					dam = dam * 2;
					(void)set_cut(p_ptr, p_ptr->cut + randint1(dam));

					if (p_ptr->resist_pois || p_ptr->oppose_pois)
					{
						msg_print(p_ptr, "The poison does not affect you!");
					}

					else
					{
						dam = dam * 2;
						(void)set_poisoned(p_ptr, p_ptr->poisoned + randint1(dam));
					}
				}

				/* Take the damage */
				take_hit(p_ptr, dam, name);
			}

			break;
		}

		case FEAT_TRAP_HEAD + 0x04:
		{
			msg_print(p_ptr, "You are enveloped in a cloud of smoke!");
			sound(p_ptr, MSG_SUM_MONSTER);
			c_ptr->feat = FEAT_FLOOR;
			*w_ptr &= ~CAVE_MARK;
			note_spot_depth(Depth, p_ptr->py, p_ptr->px);
			everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);
			num = 2 + randint1(3);
			for (i = 0; i < num; i++)
			{
				(void)summon_specific(Depth, p_ptr->py, p_ptr->px, Depth, 0);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x05:
		{
			msg_print(p_ptr, "You hit a teleport trap!");
			teleport_player(p_ptr, 100);
			break;
		}

		case FEAT_TRAP_HEAD + 0x06:
		{
			msg_print(p_ptr, "You are enveloped in flames!");
			dam = damroll(4, 6);
			fire_dam(p_ptr, dam, "a fire trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x07:
		{
			msg_print(p_ptr, "You are splashed with acid!");
			dam = damroll(4, 6);
			acid_dam(p_ptr, dam, "an acid trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x08:
		{
			if (check_hit(p_ptr, 125))
			{
				msg_print(p_ptr, "A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(p_ptr, dam, name);
				(void)set_slow(p_ptr, p_ptr->slow + randint0(20) + 20);
			}
			else
			{
				msg_print(p_ptr, "A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x09:
		{
			if (check_hit(p_ptr, 125))
			{
				msg_print(p_ptr, "A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(p_ptr, dam, name);
				(void)do_dec_stat(p_ptr, A_STR);
			}
			else
			{
				msg_print(p_ptr, "A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0A:
		{
			if (check_hit(p_ptr, 125))
			{
				msg_print(p_ptr, "A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(p_ptr, dam, name);
				(void)do_dec_stat(p_ptr, A_DEX);
			}
			else
			{
				msg_print(p_ptr, "A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0B:
		{
			if (check_hit(p_ptr, 125))
			{
				msg_print(p_ptr, "A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(p_ptr, dam, name);
				(void)do_dec_stat(p_ptr, A_CON);
			}
			else
			{
				msg_print(p_ptr, "A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0C:
		{
			msg_print(p_ptr, "A black gas surrounds you!");
			if (!p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr, p_ptr->blind + randint0(50) + 25);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0D:
		{
			msg_print(p_ptr, "A gas of scintillating colors surrounds you!");
			if (!p_ptr->resist_conf)
			{
				(void)set_confused(p_ptr, p_ptr->confused + randint0(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0E:
		{
			msg_print(p_ptr, "A pungent green gas surrounds you!");
			if (!p_ptr->resist_pois && !p_ptr->oppose_pois)
			{
				(void)set_poisoned(p_ptr, p_ptr->poisoned + randint0(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0F:
		{
			msg_print(p_ptr, "A strange white mist surrounds you!");
			if (!p_ptr->free_act)
			{
				(void)set_paralyzed(p_ptr, p_ptr->paralyzed + randint0(10) + 5);
			}
			break;
		}
	}
}



/*
 * Player attacks another player!
 *
 * If no "weapon" is available, then "punch" the player one time.
 */
void py_attack_player(player_type *p_ptr, int y, int x)
{
	int Depth = p_ptr->dun_depth;

	int num = 0, k, bonus, chance;
	u32b blow_energy;

	cave_type *c_ptr = &cave[Depth][y][x];

	player_type *q_ptr = Players[0 - c_ptr->m_idx];

	object_type *o_ptr;

	char pvp_name[80];

	bool do_quake = FALSE;

	/* Hack -- reset counter */
	p_ptr->dealt_blows = 0;

	/* Disturb both players */
	disturb(p_ptr, 0, 0);
	disturb(q_ptr, 0, 0);

	/* Extract name */
	my_strcpy(pvp_name, q_ptr->name, 80);

	/* Track player health */
	if (p_ptr->play_vis[0 - c_ptr->m_idx]) health_track(p_ptr, c_ptr->m_idx);

	/* Handle attacker fear */
	if (p_ptr->afraid)
	{
		/* Message */
		msg_format(p_ptr, "You are too afraid to attack %s!", pvp_name);

		/* Done */
		return;
	}

	/* Access the weapon */
	o_ptr = &(p_ptr->inventory[INVEN_WIELD]);

	/* Calculate the "attack quality" */
	bonus = p_ptr->to_h + o_ptr->to_h;
	chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

	/* Energy cost for one blow */
	blow_energy = level_speed(p_ptr->dun_depth) / p_ptr->num_blow;

	/* Attack once */
	while (p_ptr->energy >= blow_energy)
	{
		/* Spend energy */
		p_ptr->energy -= blow_energy;

		/* Hack -- increase counter (for later) */
		p_ptr->dealt_blows++;

		/* Test for hit */
		if (test_hit_norm(chance, q_ptr->ac + q_ptr->to_a, 1))
		{
			/* Messages */
			msg_format(p_ptr, "You hit %s.", pvp_name);
			msg_format(q_ptr, "%s hits you.", p_ptr->name);

			/* Hack -- bare hands do one damage */
			k = 1;

			/* Ghosts do damages relative to level */
			if (p_ptr->ghost)
				k = p_ptr->lev;
			if (p_ptr->fruit_bat)
				k = (p_ptr->lev / 5) + 1;

			/* Handle normal weapon */
			if (o_ptr->k_idx)
			{
				k = damroll(o_ptr->dd, o_ptr->ds);
				k = tot_dam_aux_player(o_ptr, k, q_ptr);
				if (p_ptr->impact && (k > 50)) do_quake = TRUE;
				k = critical_norm(p_ptr, o_ptr->weight, o_ptr->to_h, k);
				k += o_ptr->to_d;
			}

			/* Apply the player damage bonuses */
			k += p_ptr->to_d;

			/* No negative damage */
			if (k < 0) k = 0;

			/* Damage */
			take_hit(q_ptr, k, p_ptr->name);

			/* Check for death */
			if (q_ptr->death) break;

			/* Confusion attack */
			if (p_ptr->confusing)
			{
				/* Cancel glowing hands */
				p_ptr->confusing = FALSE;

				/* Message */
				msg_print(p_ptr, "Your hands stop glowing.");

				/* Confuse the monster */
				if (q_ptr->resist_conf)
				{
					msg_format(p_ptr, "%^s is unaffected.", pvp_name);
				}
				else if (randint0(100) < q_ptr->lev)
				{
					msg_format(p_ptr, "%^s is unaffected.", pvp_name);
				}
				else
				{
					msg_format(p_ptr, "%^s appears confused.", pvp_name);
					set_confused(q_ptr, q_ptr->confused + 10 + randint0(p_ptr->lev) / 5);
				}
			}

			/* Ghosts get fear attacks */
			if (p_ptr->ghost)
			{
				int fear_chance = 50 + (p_ptr->lev - q_ptr->lev) * 5;

				if (randint0(100) < fear_chance)
				{
					msg_format(p_ptr, "%^s appears afraid.", pvp_name);
					set_afraid(q_ptr, q_ptr->afraid + 4 + randint0(p_ptr->lev) / 5);
				}
			}
		}

		/* Player misses */
		else
		{
			/* Sound */
			sound(p_ptr, MSG_MISS);

			/* Messages */
			msg_format(p_ptr, "You miss %s.", pvp_name);
			msg_format(q_ptr, "%s misses you.", p_ptr->name);
		}
	}

	/* Mega-Hack -- apply earthquake brand */
	if (do_quake) earthquake(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 10);
}



/*
 * Player attacks a (poor, defenseless) creature	-RAK-
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack_mon(player_type *p_ptr, int y, int x)
{
	int Depth = p_ptr->dun_depth;

	int			num = 0, k, bonus, chance;
	u32b blow_energy;

	cave_type		*c_ptr = &cave[Depth][y][x];

	monster_type	*m_ptr = &m_list[c_ptr->m_idx];
	monster_race	*r_ptr = &r_info[m_ptr->r_idx];
	monster_lore	*l_ptr = p_ptr->l_list + m_ptr->r_idx;

	object_type		*o_ptr;

	char		m_name[80];

	bool		fear = FALSE;

	bool		do_quake = FALSE;

	bool		backstab = FALSE, stab_fleeing = FALSE;

	/* Hack -- reset counter */
	p_ptr->dealt_blows = 0;

	/* Disturb the player */
	disturb(p_ptr, 0, 0);


	/* Extract monster name (or "it") */
	monster_desc(p_ptr, m_name, c_ptr->m_idx, 0);


	/* Auto-Recall if possible and visible */
	if (p_ptr->mon_vis[c_ptr->m_idx]) monster_race_track(p_ptr, m_ptr->r_idx);

	/* Track a new monster */
	if (p_ptr->mon_vis[c_ptr->m_idx]) health_track(p_ptr, c_ptr->m_idx);


	/* Handle player fear */
	if (p_ptr->afraid)
	{
		/* Message */
		msg_format(p_ptr, "You are too afraid to attack %s!", m_name);

		/* Done */
		return;
	}

	if (p_ptr->cp_ptr->flags & CF_BACK_STAB)
	{
		if (p_ptr->mon_vis[c_ptr->m_idx])
		{
			if (m_ptr->csleep) backstab = TRUE;
			else if (m_ptr->monfear) stab_fleeing = TRUE;
		}
	}
		
	
	/* Disturb the monster */
	m_ptr->csleep = 0;


	/* Access the weapon */
	o_ptr = &(p_ptr->inventory[INVEN_WIELD]);

	/* Calculate the "attack quality" */
	bonus = p_ptr->to_h + o_ptr->to_h;
	chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

	blow_energy = level_speed(p_ptr->dun_depth) / p_ptr->num_blow;

	/* Attack while we have energy */
	while (p_ptr->energy >= blow_energy)
	{
		/* Remove energy */
		p_ptr->energy -= blow_energy;

		/* Hack -- increase counter (for later) */
		p_ptr->dealt_blows++;

		/* Test for hit */
		if (test_hit_norm(chance, r_ptr->ac, p_ptr->mon_vis[c_ptr->m_idx]))
		{
			/* Message */
			if ((!backstab) && (!stab_fleeing))
				msg_format(p_ptr, "You hit %s.", m_name);
			else if(backstab)
				msg_format(p_ptr, "You cruelly stab the helpless, sleeping %s!", (r_name + r_info[m_ptr->r_idx].name));
			else
				msg_format(p_ptr, "You backstab the fleeing %s!", (r_name + r_info[m_ptr->r_idx].name));

			/* Hack -- bare hands do one damage */
			k = 1;

			/* Ghosts get damage relative to level */
			if (p_ptr->ghost)
				k = p_ptr->lev;
				
			if (p_ptr->fruit_bat)
				k = (p_ptr->lev / 5) + 1;

			/* Handle normal weapon */
			if (o_ptr->k_idx)
			{
				k = damroll(o_ptr->dd, o_ptr->ds);
				k = tot_dam_aux(p_ptr, o_ptr, k, m_ptr, p_ptr->mon_vis[c_ptr->m_idx]);
				if (backstab)
				{
					backstab = FALSE;
					k *= (3 + (p_ptr->lev / 40));
				}
				else if (stab_fleeing)
				{
					k = ((3 * k) / 2);
				}
				if (p_ptr->impact && (k > 50)) do_quake = TRUE;
				k = critical_norm(p_ptr, o_ptr->weight, o_ptr->to_h, k);
				k += o_ptr->to_d;
			}

			/* Apply the player damage bonuses */
			k += p_ptr->to_d;

			/* No negative damage */
			if (k < 0) k = 0;

			/* Complex message */
			if (is_dm_p(p_ptr))
			{
				msg_format(p_ptr, "You do %d (out of %d) damage.", k, m_ptr->hp);
			}

			/* Damage, check for fear and death */
			if (mon_take_hit(p_ptr, c_ptr->m_idx, k, &fear, NULL)) break;

			/* Confusion attack */
			if (p_ptr->confusing)
			{
				/* Cancel glowing hands */
				p_ptr->confusing = FALSE;

				/* Message */
				msg_print(p_ptr, "Your hands stop glowing.");

				/* Confuse the monster */
				if (r_ptr->flags3 & RF3_NO_CONF)
				{
					if (p_ptr->mon_vis[c_ptr->m_idx]) l_ptr->flags3 |= RF3_NO_CONF;
					msg_format(p_ptr, "%^s is unaffected.", m_name);
				}
				else if (randint0(100) < r_ptr->level)
				{
					msg_format(p_ptr, "%^s is unaffected.", m_name);
				}
				else
				{
					msg_format(p_ptr, "%^s appears confused.", m_name);
					m_ptr->confused += 10 + randint0(p_ptr->lev) / 5;
				}
			}

			/* Ghosts get fear attacks */
			if (p_ptr->ghost)
			{
				int fear_chance = 50 + (p_ptr->lev - r_ptr->level) * 5;

				if (!(r_ptr->flags3 & RF3_NO_FEAR) && randint0(100) < fear_chance)
				{
					msg_format(p_ptr, "%^s appears afraid.", m_name);
					m_ptr->monfear = m_ptr->monfear + 4 + randint0(p_ptr->lev) / 5;
				}
			}

		}

		/* Player misses */
		else
		{
			/* Sound */
			sound(p_ptr, MSG_MISS);

			backstab = FALSE;

			/* Message */
			msg_format(p_ptr, "You miss %s.", m_name);
		}
	}


	/* Hack -- delay fear messages */
	if (fear && p_ptr->mon_vis[c_ptr->m_idx] && !(r_ptr->flags2 & RF2_WANDERER))
	{
		/* Sound */
		sound(p_ptr, MSG_FLEE);

		/* Message */
		msg_format(p_ptr, "%^s flees in terror!", m_name);
	}


	/* Mega-Hack -- apply earthquake brand */
	if (do_quake) earthquake(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 10);
}


/*
 * Attacking something, figure out what and spawn appropriately.
 */
void py_attack(player_type *p_ptr, int y, int x)
{
	int Depth = p_ptr->dun_depth;
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Check for monster */
	if (c_ptr->m_idx > 0)
		py_attack_mon(p_ptr, y, x);

	/* Check for player */
	if (c_ptr->m_idx < 0)
		py_attack_player(p_ptr, y, x);
}


/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
 
 /* Bounds checking is used in dungeon levels <= 0, which is used
    to move between wilderness levels. 
    
    The wilderness levels are stored in rings radiating from the town,
    see calculate_world_index for more information.

    Diagonals aren't handled properly, but I don't feel that is important.
        
    -APD- 
 */
 
void move_player(player_type *p_ptr, int dir, int do_pickup)
{
	int Depth = p_ptr->dun_depth;

	int			y, x, old_world_x, old_world_y, oldx, oldy;

	cave_type		*c_ptr;
	object_type		*o_ptr;
	monster_type		*m_ptr;
	player_type		*q_ptr;
	byte			*w_ptr;

	/* Ensure "dir" is in ddx/ddy array bounds */
	if (!VALID_DIR(dir)) return;

	/* Hack -- reset temp. flag */
	p_ptr->dealt_blows = 0;

	/* Find the result of moving */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Update wilderness positions */
	if (p_ptr->dun_depth <= 0)
	{
		/* Make sure he hasn't just changed depth */
		if (p_ptr->new_level_flag) return;
		
		/* save his old location */
		old_world_x = p_ptr->world_x; old_world_y = p_ptr->world_y;
		oldx = p_ptr->px; oldy = p_ptr->py;
		
		/* we have gone off the map */
		if (!in_bounds(Depth, y, x))
		{
			/* Leaving Town -- DENY! */
			if (!Depth && (cfg_town_wall || cfg_ironman))
			{
				if (!cfg_town_wall)
					switch(randint0(5))
					{
						case 0: msg_print(p_ptr, "You don't feel like going to pick flowers right now."); break;
						case 1: msg_print(p_ptr, "Where do you think you are going?"); break; /* [Warrior] */
						case 2: msg_print(p_ptr, "Morgoth the potato farmer? - get real!"); break; /* [Warrior] */
						case 3: msg_print(p_ptr, "Morgoth awaits you in the depths not in the fields."); break;
						case 4: msg_print(p_ptr, "Something draws your attention back to the stairs."); break;
					}
				else 
					msg_print(p_ptr, "There is a wall blocking your way.");
				disturb(p_ptr, 1, 0);
				return;
			}
			/* Leaving Town -- ALLOW, BUT WARN */
			else if (!Depth && p_ptr->lev == 1)
				msg_print(p_ptr, "Really enter the wilderness? The dungeon entrance is in the town!");

			/* find his new location */
			if (y <= 0)
			{	
				/* new player location */
				p_ptr->world_y++;
				p_ptr->py = MAX_HGT-2;
			}
			if (y >= 65)
			{
				/* new player location */
				p_ptr->world_y--;
				p_ptr->py = 1;
			}
			if (x <= 0)
			{
				/* new player location */
				p_ptr->world_x--;
				p_ptr->px = MAX_WID-2;
			}
			if (x >= 197)
			{
				/* new player location */
				p_ptr->world_x++;
				p_ptr->px = 1;
			}

			/* check to make sure he hasnt hit the edge of the world */
			if (world_index(p_ptr->world_x, p_ptr->world_y) <= -MAX_WILD) 
			{
				switch(randint1(2))
				{
					case 0: msg_print(p_ptr, "You have reached the Walls of the World. You can not pass."); break;
					case 1: msg_print(p_ptr, "You cannot go beyond the Walls of the World."); break;
				}
				/* TODO: if wearing Crown of Morgoth, suicide! */
				p_ptr->world_x = old_world_x;
				p_ptr->world_y = old_world_y;
				p_ptr->px = oldx;
				p_ptr->py = oldy;
				disturb(p_ptr, 1, 0);
				return;
			}
			
			/* Remove the player from the old location */
			cave[Depth][oldy][oldx].m_idx = 0;

			/* Show everyone that's he left */
			everyone_lite_spot(Depth, oldy, oldx);

			/* Tell everyone to re-calculate visiblity for this player */
			update_player(p_ptr);

			/* forget his light and viewing area */
			forget_lite(p_ptr);
			forget_view(p_ptr);

			/* A player has left this depth */
			players_on_depth[p_ptr->dun_depth]--;
			
			/* (required) paranoia, allows integration of old wilderness
			   saves onto new severs.
			*/
			if (players_on_depth[p_ptr->dun_depth] < 0)
				players_on_depth[p_ptr->dun_depth] = 0;
			
			/* Calculate the new level index */
			p_ptr->dun_depth = world_index(p_ptr->world_x, p_ptr->world_y);

			/* update the wilderness map */
			p_ptr->wild_map[(-p_ptr->dun_depth)/8] |= (1<<((-p_ptr->dun_depth)%8));

			/* disturb if necessary */
			if (option_p(p_ptr,DISTURB_PANEL)) disturb(p_ptr, 0, 0);

			players_on_depth[p_ptr->dun_depth]++;
			p_ptr->new_level_flag = TRUE;
			p_ptr->new_level_method = LEVEL_OUTSIDE;

			return;
		}
	}


	/* Examine the destination */
	c_ptr = &cave[Depth][y][x];
	w_ptr = &p_ptr->cave_flag[y][x];

	/* Get the object */
	o_ptr = &o_list[c_ptr->o_idx];

	/* Get the monster */
	m_ptr = &m_list[c_ptr->m_idx];

	/* Save "last direction moved" */
	p_ptr->last_dir = dir;

	/* Bump into other players */
	if ((q_ptr = player_on_cave(Depth, y, x)))
	{
		/* Don't bump into self! */
		if (!same_player(q_ptr, p_ptr))
		{
			/* Check for an attack */
			if (pvp_okay(p_ptr, q_ptr, 1))
				py_attack(p_ptr, y, x);

			/* If both want to switch, do it */
			else if ((!p_ptr->ghost && !q_ptr->ghost &&
					 (ddy[q_ptr->last_dir] == -(ddy[dir])) &&
					 (ddx[q_ptr->last_dir] == (-ddx[dir]))) ||
				(is_dm_p(q_ptr)))
			{
				/* Swap m_idx */
				cave[Depth][q_ptr->py][q_ptr->px].m_idx = 0 - p_ptr->Ind;
				cave[Depth][p_ptr->py][p_ptr->px].m_idx = 0 - q_ptr->Ind;

				q_ptr->py = p_ptr->py;
				q_ptr->px = p_ptr->px;

				p_ptr->py = y;
				p_ptr->px = x;

				/* Tell both of them */
				/* Don't tell people they bumped into the Dungeon Master */
				if (!is_dm_p(q_ptr))
				{
					msg_format(p_ptr, "You switch places with %s.", q_ptr->name);
					msg_format(q_ptr, "You switch places with %s.", p_ptr->name);
				}

				/* Disturb both of them */
				disturb(p_ptr, 1, 0);
				disturb(q_ptr, 1, 0);

				/* Unhack both of them */
				q_ptr->last_dir = p_ptr->last_dir = 5;

				/* Re-show both grids */
				everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);
				everyone_lite_spot(Depth, q_ptr->py, q_ptr->px);

				/* Tell everyone to re-calculate visiblity for those players */
				update_player(p_ptr);
				update_player(q_ptr);

				/* Update stuff for both */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
				q_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

				/* Update the monsters */
				p_ptr->update |= (PU_DISTANCE);
				q_ptr->update |= (PU_DISTANCE);

				/* Refresh floor item for both */
				floor_item_notify(p_ptr, cave[Depth][p_ptr->py][p_ptr->px].o_idx, FALSE);
				floor_item_notify(q_ptr, cave[Depth][q_ptr->py][q_ptr->px].o_idx, FALSE);
			}

			/* Hack -- the Dungeon Master cannot bump people */
			else if (!is_dm_p(p_ptr))
			{
				/* Tell both about it */
				msg_format(p_ptr, "You bump into %s.", q_ptr->name);
				msg_format(q_ptr, "%s bumps into you.", p_ptr->name);

				/* Disturb both parties */
				disturb(p_ptr, 1, 0);
				disturb(q_ptr, 1, 0);
			}
		}
	}

	/* Hack -- attack monsters */
	else if (c_ptr->m_idx > 0)
	{
		/* Hack -- the dungeon master switches places with his monsters */
		if (p_ptr->dm_flags & DM_MONSTER_FRIEND)
		{
			/* save old player location */
			oldx = p_ptr->px;
			oldy = p_ptr->py;
			/* update player location */
			p_ptr->px = m_list[c_ptr->m_idx].fx;
			p_ptr->py = m_list[c_ptr->m_idx].fy;
			/* update monster location */
			m_list[c_ptr->m_idx].fx = oldx;
			m_list[c_ptr->m_idx].fy = oldy;
			/* update cave monster indexes */
			cave[Depth][oldy][oldx].m_idx = c_ptr->m_idx;
			c_ptr->m_idx = (0 - p_ptr->Ind);

			/* Re-show both grids */
			everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);
			everyone_lite_spot(Depth, oldy, oldx);
		}
		/* Attack */
		else py_attack(p_ptr, y, x);
	}
	/* Arena */
	else if (c_ptr->feat == FEAT_PVP_ARENA)
	{
		/* Hack -- ghosts can't access arena */
		if (p_ptr->ghost)
		{
			msg_print(p_ptr, "The wall blocks your movement.");
			disturb(p_ptr, 0, 0);
		} else
		access_arena(p_ptr, y, x);
	}

	/* Player can not walk through "walls", but ghosts can */
	else if ((!p_ptr->ghost) && (!cave_floor_bold(Depth, y, x)))
	{
		/* Disturb the player */
		disturb(p_ptr, 0, 0);

		/* Notice things in the dark */
		if (!(*w_ptr & CAVE_MARK) &&
		    (p_ptr->blind || !(*w_ptr & CAVE_LITE)))
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
				msg_print(p_ptr, "You feel some rubble blocking your way.");
				sound(p_ptr, MSG_HITWALL);
				*w_ptr |= CAVE_MARK;
				everyone_lite_spot(Depth, y, x);
			}

			/* Closed door */
			else if ((c_ptr->feat < FEAT_SECRET && c_ptr->feat >= FEAT_DOOR_HEAD) ||
			         (c_ptr->feat >= FEAT_HOME_HEAD && c_ptr->feat <= FEAT_HOME_TAIL))
			{
				msg_print(p_ptr, "You feel a closed door blocking your way.");
				sound(p_ptr, MSG_HITWALL);
				*w_ptr |= CAVE_MARK;
				everyone_lite_spot(Depth, y, x);
			}

			/* Tree */
			else if (c_ptr->feat == FEAT_TREE)
			{
				msg_print(p_ptr, "You feel a tree blocking your way.");
				sound(p_ptr, MSG_HITWALL);
				*w_ptr |= CAVE_MARK;
				everyone_lite_spot(Depth, y, x);
			}

			/* Wall (or secret door) */
			else
			{
				msg_print(p_ptr, "You feel a wall blocking your way.");
				sound(p_ptr, MSG_HITWALL);
				*w_ptr |= CAVE_MARK;
				everyone_lite_spot(Depth, y, x);
			}
		}

		/* Notice things */
		else
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
				msg_print(p_ptr, "There is rubble blocking your way.");
				sound(p_ptr, MSG_HITWALL);
			}

			/* Closed doors */
			else if ((c_ptr->feat < FEAT_SECRET && c_ptr->feat >= FEAT_DOOR_HEAD) || 
			         (c_ptr->feat >= FEAT_HOME_HEAD && c_ptr->feat <= FEAT_HOME_TAIL))
			{
				msg_print(p_ptr, "There is a closed door blocking your way.");
				sound(p_ptr, MSG_HITWALL);
			}

			/* Tree */
			else if (c_ptr->feat == FEAT_TREE)
			{
				msg_print(p_ptr, "There is a tree blocking your way.");
				sound(p_ptr, MSG_HITWALL);
			}

			/* Wall (or secret door) */
			else
			{
				msg_print(p_ptr, "There is a wall blocking your way.");
				sound(p_ptr, MSG_HITWALL);
			}
		}
	}

	/* Ghost trying to walk into a permanent wall */
	else if (p_ptr->ghost && c_ptr->feat == FEAT_PERM_SOLID)
	{
		/* Message */
		msg_print(p_ptr, "The wall blocks your movement.");

		disturb(p_ptr, 0, 0);
	}

	/* Player bumping shop door */
	else if (!p_ptr->ghost && c_ptr->feat >= FEAT_SHOP_HEAD && c_ptr->feat <= FEAT_SHOP_TAIL - 1)
	{
		/* Disturb */
		disturb(p_ptr, 0, 0);
	
		/* Note "FEAT_SHOP_TAIL - 1" above, means we exclude tavern */
		do_cmd_store(p_ptr, -2 -(c_ptr->feat - FEAT_SHOP_HEAD));
	}
	
	/* Normal movement */
	else
	{
		int oy, ox;

		/* Save old location */
		oy = p_ptr->py;
		ox = p_ptr->px;

		/* Move the player */
		p_ptr->py = y;
		p_ptr->px = x;

		/* Update the player indices */
		cave[Depth][oy][ox].m_idx = 0;
		cave[Depth][y][x].m_idx = 0 - p_ptr->Ind;



		/* Redraw new spot */
		everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);

		/* Redraw old spot */
		everyone_lite_spot(Depth, oy, ox);

		/* Check for new panel (redraw map) */
		verify_panel(p_ptr);

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Hack -- quickly update the view, to reduce perceived lag */
		redraw_stuff(p_ptr);
		window_stuff(p_ptr);

		/* Spontaneous Searching */
		if ((p_ptr->skill_fos >= 50) ||
		    (0 == randint0(50 - p_ptr->skill_fos)))
		{
			search(p_ptr);
		}

		/* Continuous Searching */
		if (p_ptr->searching)
		{
			search(p_ptr);
		}

		/* Handle "objects" */
		if (c_ptr->o_idx) carry(p_ptr, do_pickup, 0);
		else  floor_item_notify(p_ptr, 0, FALSE);

		/* Handle "store doors" */
		if ((!p_ptr->ghost) &&
		    (c_ptr->feat >= FEAT_SHOP_HEAD) &&
		    (c_ptr->feat <= FEAT_SHOP_TAIL))
		{
			/* Disturb */
			disturb(p_ptr, 0, 0);

#if 0
			/* Hack -- Enter store */
			if (c_ptr->feat != FEAT_SHOP_HEAD+7)
			{
				/*command_new = '_';*/
				do_cmd_store(p_ptr, -1);
			}
#endif
		}

		/* Handle resurrection */
		else if (p_ptr->ghost && c_ptr->feat == FEAT_SHOP_HEAD + 3)
		{
			/* Resurrect him */
			resurrect_player(p_ptr);
		}

		/* Discover invisible traps */
		else if (c_ptr->feat == FEAT_INVIS)
		{
			/* Disturb */
			disturb(p_ptr, 0, 0);

			/* Message */
			msg_print(p_ptr, "You found a trap!");

			/* Pick a trap */
			pick_trap(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

			/* Hit the trap */
			hit_trap(p_ptr);
		}

		/* Set off an visible trap */
		else if ((c_ptr->feat >= FEAT_TRAP_HEAD) &&
		         (c_ptr->feat <= FEAT_TRAP_TAIL))
		{
			/* Disturb */
			disturb(p_ptr, 0, 0);

			/* Hit the trap */
			hit_trap(p_ptr);
		}

		/* Mega-hack -- if we are the dungeon master, and our movement hook
		 * is set, call it.  This is used to make things like building walls
		 * and summoning monster armies easier.
		 */
		if (p_ptr->master_hook[0])
			master_new_hook(p_ptr, 'x', oy, ox);
	}
}


/*
 * Hack -- Check for a "motion blocker" (see below)
 */
int see_wall(player_type *p_ptr, int dir, int y, int x)
{
	int Depth = p_ptr->dun_depth;

	/* Ensure "dir" is in ddx/ddy array bounds */
	if (!VALID_DIR(dir)) return FALSE;

	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Ghosts run right through everything */
	if (p_ptr->ghost) return (FALSE);

	/* Do wilderness hack, keep running from one outside level to another */
	if ( (!in_bounds(Depth, y, x)) && (Depth <= 0) ) return FALSE;

	/* Illegal grids are blank */
	if (!in_bounds2(Depth, y, x)) return (FALSE);

	/* Must actually block motion */
	if (cave_floor_bold(Depth, y, x)) return (FALSE);

	/* Must be known to the player */
	if (!(p_ptr->cave_flag[y][x] & CAVE_MARK)) return (FALSE);

	/* Default */
	return (TRUE);
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(player_type *p_ptr, int dir, int y, int x)
{
	int Depth = p_ptr->dun_depth;

	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are unknown */
	if (!in_bounds2(Depth, y, x)) return (TRUE);

	/* Memorized grids are known */
	if (p_ptr->cave_flag[y][x] & CAVE_MARK) return (FALSE);

	/* Non-floor grids are unknown */
	if (!cave_floor_bold(Depth, y, x)) return (TRUE);

	/* Viewable grids are known */
	if (player_can_see_bold(p_ptr, y, x)) return (FALSE);

	/* Default */
	return (TRUE);
}





/*
 * The running algorithm:			-CJS-
 *
 * In the diagrams below, the player has just arrived in the
 * grid marked as '@', and he has just come from a grid marked
 * as 'o', and he is about to enter the grid marked as 'x'.
 *
 * Of course, if the "requested" move was impossible, then you
 * will of course be blocked, and will stop.
 *
 * Overview: You keep moving until something interesting happens.
 * If you are in an enclosed space, you follow corners. This is
 * the usual corridor scheme. If you are in an open space, you go
 * straight, but stop before entering enclosed space. This is
 * analogous to reaching doorways. If you have enclosed space on
 * one side only (that is, running along side a wall) stop if
 * your wall opens out, or your open space closes in. Either case
 * corresponds to a doorway.
 *
 * What happens depends on what you can really SEE. (i.e. if you
 * have no light, then running along a dark corridor is JUST like
 * running in a dark room.) The algorithm works equally well in
 * corridors, rooms, mine tailings, earthquake rubble, etc, etc.
 *
 * These conditions are kept in static memory:
 * find_openarea	 You are in the open on at least one
 * side.
 * find_breakleft	 You have a wall on the left, and will
 * stop if it opens
 * find_breakright	 You have a wall on the right, and will
 * stop if it opens
 *
 * To initialize these conditions, we examine the grids adjacent
 * to the grid marked 'x', two on each side (marked 'L' and 'R').
 * If either one of the two grids on a given side is seen to be
 * closed, then that side is considered to be closed. If both
 * sides are closed, then it is an enclosed (corridor) run.
 *
 * LL		L
 * @x	       LxR
 * RR	       @R
 *
 * Looking at more than just the immediate squares is
 * significant. Consider the following case. A run along the
 * corridor will stop just before entering the center point,
 * because a choice is clearly established. Running in any of
 * three available directions will be defined as a corridor run.
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * #.#
 * ##.##
 * .@x..
 * ##.##
 * #.#
 *
 * Likewise, a run along a wall, and then into a doorway (two
 * runs) will work correctly. A single run rightwards from @ will
 * stop at 1. Another run right and down will enter the corridor
 * and make the corner, stopping at the 2.
 *
 * #@x	  1
 * ########### ######
 * 2	    #
 * #############
 * #
 *
 * After any move, the function area_affect is called to
 * determine the new surroundings, and the direction of
 * subsequent moves. It examines the current player location
 * (at which the runner has just arrived) and the previous
 * direction (from which the runner is considered to have come).
 *
 * Moving one square in some direction places you adjacent to
 * three or five new squares (for straight and diagonal moves
 * respectively) to which you were not previously adjacent,
 * marked as '!' in the diagrams below.
 *
 * ...!	  ...
 * .o@!	  .o.!
 * ...!	  ..@!
 * !!!
 *
 * You STOP if any of the new squares are interesting in any way:
 * for example, if they contain visible monsters or treasure.
 *
 * You STOP if any of the newly adjacent squares seem to be open,
 * and you are also looking for a break on that side. (that is,
 * find_openarea AND find_break).
 *
 * You STOP if any of the newly adjacent squares do NOT seem to be
 * open and you are in an open area, and that side was previously
 * entirely open.
 *
 * Corners: If you are not in the open (i.e. you are in a corridor)
 * and there is only one way to go in the new squares, then turn in
 * that direction. If there are more than two new ways to go, STOP.
 * If there are two ways to go, and those ways are separated by a
 * square which does not seem to be open, then STOP.
 *
 * Otherwise, we have a potential corner. There are two new open
 * squares, which are also adjacent. One of the new squares is
 * diagonally located, the other is straight on (as in the diagram).
 * We consider two more squares further out (marked below as ?).
 *
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid, and "check_dir" to the grid marked 's'.
 *
 * .s
 * @x?
 * #?
 *
 * If they are both seen to be closed, then it is seen that no
 * benefit is gained from moving straight. It is a known corner.
 * To cut the corner, go diagonally, otherwise go straight, but
 * pretend you stepped diagonally into that next location for a
 * full view next time. Conversely, if one of the ? squares is
 * not seen to be closed, then there is a potential choice. We check
 * to see whether it is a potential corner or an intersection/room entrance.
 * If the square two spaces straight ahead, and the space marked with 's'
 * are both blank, then it is a potential corner and enter if find_examine
 * is set, otherwise must stop because it is not a corner.
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
/*static byte find_current;*/

/*
 * The direction we came from
 */
/*static byte find_prevdir;*/

/*
 * We are looking for open area
 */
/*static bool find_openarea;*/

/*
 * We are looking for a break
 */
/*static bool find_breakright;
static bool find_breakleft;*/



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(player_type *p_ptr, int dir)
{
	int		row, col, deepleft, deepright;
	int		i, shortleft, shortright;

	/* Ensure "dir" is in ddx/ddy array bounds */
	if (!VALID_DIR(dir)) return;

	/* Reset counter */
	p_ptr->ran_tiles = 0;

	/* Save the direction */
	p_ptr->find_current = dir;

	/* Assume running straight */
	p_ptr->find_prevdir = dir;

	/* Assume looking for open area */
	p_ptr->find_openarea = TRUE;

	/* Assume not looking for breaks */
	p_ptr->find_breakright = p_ptr->find_breakleft = FALSE;

	/* Assume no nearby walls */
	deepleft = deepright = FALSE;
	shortright = shortleft = FALSE;

	/* Find the destination grid */
	row = p_ptr->py + ddy[dir];
	col = p_ptr->px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for walls */
	/* When in the town/wilderness, don't break left/right. -APD- */
	if (see_wall(p_ptr, cycle[i+1], p_ptr->py, p_ptr->px))
	{
		/* if in the dungeon */
		if (p_ptr->dun_depth > 0)
		{
			p_ptr->find_breakleft = TRUE;
			shortleft = TRUE;
		}
	}
	else if (see_wall(p_ptr, cycle[i+1], row, col))
	{
		/* if in the dungeon */
		if (p_ptr->dun_depth > 0)
		{
			p_ptr->find_breakleft = TRUE;
			deepleft = TRUE;
		}
	}

	/* Check for walls */
	if (see_wall(p_ptr, cycle[i-1], p_ptr->py, p_ptr->px))
	{
		/* if in the dungeon */
		if (p_ptr->dun_depth > 0)
		{
			p_ptr->find_breakright = TRUE;
			shortright = TRUE;
		}
	}
	else if (see_wall(p_ptr, cycle[i-1], row, col))
	{
		/* if in the dungeon */
		if (p_ptr->dun_depth > 0)
		{
			p_ptr->find_breakright = TRUE;
			deepright = TRUE;
		}
	}

	if (p_ptr->find_breakleft && p_ptr->find_breakright)
	{
		/* Not looking for open area */
		/* In the town/wilderness, always in an open area */
		if (p_ptr->dun_depth > 0) 
			p_ptr->find_openarea = FALSE;	

		/* Hack -- allow angled corridor entry */
		if (dir & 0x01)
		{
			if (deepleft && !deepright)
			{
				p_ptr->find_prevdir = cycle[i - 1];
			}
			else if (deepright && !deepleft)
			{
				p_ptr->find_prevdir = cycle[i + 1];
			}
		}

		/* Hack -- allow blunt corridor entry */
		else if (see_wall(p_ptr, cycle[i], row, col))
		{
			if (shortleft && !shortright)
			{
				p_ptr->find_prevdir = cycle[i - 2];
			}
			else if (shortright && !shortleft)
			{
				p_ptr->find_prevdir = cycle[i + 2];
			}
		}
	}
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(player_type *p_ptr)
{
	int Depth = p_ptr->dun_depth;

	int			prev_dir, new_dir, check_dir = 0;

	int			row, col;
	int			i, max, inv;
	int			option, option2;

	cave_type		*c_ptr;
	byte			*w_ptr;


	/* XXX -- Ghosts never stop running */
	if (p_ptr->ghost) return (FALSE);

	/* No options yet */
	option = 0;
	option2 = 0;

	/* Where we came from */
	prev_dir = p_ptr->find_prevdir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++)
	{
		new_dir = cycle[chome[prev_dir] + i];

		row = p_ptr->py + ddy[new_dir];
		col = p_ptr->px + ddx[new_dir];

		c_ptr = &cave[Depth][row][col];
		w_ptr = &p_ptr->cave_flag[row][col];


		/* Visible monsters abort running */
		if (c_ptr->m_idx > 0)
		{
			/* Visible monster */
			if (p_ptr->mon_vis[c_ptr->m_idx]) return (TRUE);
		}

		/* Visible objects abort running */
		if (c_ptr->o_idx)
		{
			/* Visible object */
			if (p_ptr->obj_vis[c_ptr->o_idx]) return (TRUE);
		}

		/* Hack -- always stop in water */
		if (c_ptr->feat == FEAT_WATER) return TRUE;

		/* Assume unknown */
		inv = TRUE;

		/* Check memorized grids */
		if (*w_ptr & CAVE_MARK)
		{
			bool notice = TRUE;

			/* Examine the terrain */
			switch (c_ptr->feat)
			{
				/* Floors */
			        case FEAT_FLOOR:

				/* Invis traps */
				case FEAT_INVIS:

				/* Secret doors */
				case FEAT_SECRET:

				/* Normal veins */
				case FEAT_MAGMA:
				case FEAT_QUARTZ:

				/* Hidden treasure */
				case FEAT_MAGMA_H:
				case FEAT_QUARTZ_H:

				/* Grass, trees, and dirt */
				case FEAT_GRASS:
				case FEAT_TREE:
				case FEAT_DIRT:

				/* Walls */
				case FEAT_WALL_EXTRA:
				case FEAT_WALL_INNER:
				case FEAT_WALL_OUTER:
				case FEAT_WALL_SOLID:
				case FEAT_PERM_EXTRA:
				case FEAT_PERM_INNER:
				case FEAT_PERM_OUTER:
				case FEAT_PERM_SOLID:
				case FEAT_PERM_CLEAR:
				{
					/* Ignore */
					notice = FALSE;

					/* Done */
					break;
				}

				/* Open doors */
				case FEAT_OPEN:
				case FEAT_BROKEN:
				{
					/* Option -- ignore */
					if (option_p(p_ptr,FIND_IGNORE_DOORS)) notice = FALSE;

					/* Done */
					break;
				}

				/* Stairs */
				case FEAT_LESS:
				case FEAT_MORE:
				{
					/* Option -- ignore */
					if (option_p(p_ptr,FIND_IGNORE_STAIRS)) notice = FALSE;

					/* Done */
					break;
				}
			}

			/* Interesting feature */
			if (notice) return (TRUE);

			/* The grid is "visible" */
			inv = FALSE;
		}

		/* Analyze unknown grids and floors */
		/* wilderness hack to run from one level to the next */
		if (inv || cave_floor_bold(Depth, row, col) || ((!in_bounds(Depth, row, col)) && (Depth <= 0)) )
		{
			/* Looking for open area */
			if (p_ptr->find_openarea)
			{
				/* Nothing */
			}

			/* The first new direction. */
			else if (!option)
			{
				option = new_dir;
			}

			/* Three new directions. Stop running. */
			else if (option2)
			{
				return (TRUE);
			}

			/* Two non-adjacent new directions.  Stop running. */
			else if (option != cycle[chome[prev_dir] + i - 1])
			{
				return (TRUE);
			}

			/* Two new (adjacent) directions (case 1) */
			else if (new_dir & 0x01)
			{
				check_dir = cycle[chome[prev_dir] + i - 2];
				option2 = new_dir;
			}

			/* Two new (adjacent) directions (case 2) */
			else
			{
				check_dir = cycle[chome[prev_dir] + i + 1];
				option2 = option;
				option = new_dir;
			}
		}

		/* Obstacle, while looking for open area */
		/* When in the town/wilderness, don't break left/right. */
		else
		{
			if (p_ptr->find_openarea)
			{
				if (i < 0)
				{
					/* Break to the right */
					if (p_ptr->dun_depth > 0)
						p_ptr->find_breakright = (TRUE);
				}

				else if (i > 0)
				{
					/* Break to the left */
					if (p_ptr->dun_depth > 0)
						p_ptr->find_breakleft = (TRUE);
				}
			}
		}
	}


	/* Looking for open area */
	if (p_ptr->find_openarea)
	{
		/* Hack -- look again */
		for (i = -max; i < 0; i++)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = p_ptr->py + ddy[new_dir];
			col = p_ptr->px + ddx[new_dir];

			/* Unknown grid or floor */
			if (!(p_ptr->cave_flag[row][col] & CAVE_MARK) || cave_floor_bold(Depth, row, col))
			{
				/* Looking to break right */
				if (p_ptr->find_breakright)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break left */
				if (p_ptr->find_breakleft)
				{
					return (TRUE);
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = p_ptr->py + ddy[new_dir];
			col = p_ptr->px + ddx[new_dir];

			/* Unknown grid or floor */
			if (!(p_ptr->cave_flag[row][col] & CAVE_MARK) || cave_floor_bold(Depth, row, col))
			{
				/* Looking to break left */
				if (p_ptr->find_breakleft)
				{
					return (TRUE);
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break right */
				if (p_ptr->find_breakright)
				{
					return (TRUE);
				}
			}
		}
	}


	/* Not looking for open area */
	else
	{
		/* No options */
		if (!option)
		{
			return (TRUE);
		}

		/* One option */
		else if (!option2)
		{
			/* Primary option */
			p_ptr->find_current = option;

			/* No other options */
			p_ptr->find_prevdir = option;
		}

		/* Two options, examining corners */
		else if (option_p(p_ptr,FIND_EXAMINE) && !option_p(p_ptr,FIND_CUT))
		{
			/* Primary option */
			p_ptr->find_current = option;

			/* Hack -- allow curving */
			p_ptr->find_prevdir = option2;
		}

		/* Two options, pick one */
		else
		{
			/* Get next location */
			row = p_ptr->py + ddy[option];
			col = p_ptr->px + ddx[option];

			/* Don't see that it is closed off. */
			/* This could be a potential corner or an intersection. */
			if (!see_wall(p_ptr, option, row, col) ||
			    !see_wall(p_ptr, check_dir, row, col))
			{
				/* Can not see anything ahead and in the direction we */
				/* are turning, assume that it is a potential corner. */
				if (option_p(p_ptr,FIND_EXAMINE) &&
				    see_nothing(p_ptr, option,  row, col) &&
				    see_nothing(p_ptr, option2, row, col))
				{
					p_ptr->find_current = option;
					p_ptr->find_prevdir = option2;
				}

				/* STOP: we are next to an intersection or a room */
				else
				{
					return (TRUE);
				}
			}

			/* This corner is seen to be enclosed; we cut the corner. */
			else if (option_p(p_ptr,FIND_CUT))
			{
				p_ptr->find_current = option2;
				p_ptr->find_prevdir = option2;
			}

			/* This corner is seen to be enclosed, and we */
			/* deliberately go the long way. */
			else
			{
				p_ptr->find_current = option;
				p_ptr->find_prevdir = option2;
			}
		}
	}


	/* About to hit a known wall, stop */
	if (see_wall(p_ptr, p_ptr->find_current, p_ptr->py, p_ptr->px))
	{
		return (TRUE);
	}


	/* Failure */
	return (FALSE);
}

/*
 * Take one step along the pathfinder.
 * Returns TRUE if a valid next step was picked.
 * Returns FALSE if pathfinding was aborted.
 */
bool run_nextstep(player_type *p_ptr)
{
	int x, y;
	int Depth = p_ptr->dun_depth;

	/* Abort if we have finished */
	if (p_ptr->pf_result_index < 0) return FALSE;

	/* Abort if we would hit a wall */
	else if (p_ptr->pf_result_index == 0)
	{
		/* Get next step */
		y = p_ptr->py + ddy[p_ptr->pf_result[p_ptr->pf_result_index] - '0'];
		x = p_ptr->px + ddx[p_ptr->pf_result[p_ptr->pf_result_index] - '0'];

		/* Known wall */
		if ((p_ptr->cave_flag[y][x] & (CAVE_MARK)) && !cave_floor_bold(Depth, y, x))
		{
			return FALSE;
		}
	}
	/* Hack -- walking stick lookahead.
	 *
	 * If the player has computed a path that is going to end up in a wall,
	 * we notice this and convert to a normal run. This allows us to click
	 * on unknown areas to explore the map.
	 *
	 * We have to look ahead two, otherwise we don't know which is the last
	 * direction moved and don't initialise the run properly.
	 */
	else if (p_ptr->pf_result_index > 0)
	{
		/* Get next step */
		y = p_ptr->py + ddy[p_ptr->pf_result[p_ptr->pf_result_index] - '0'];
		x = p_ptr->px + ddx[p_ptr->pf_result[p_ptr->pf_result_index] - '0'];

		/* Known wall */
		if ((p_ptr->cave_flag[y][x] & (CAVE_MARK)) && !cave_floor_bold(Depth, y, x))
		{
			return FALSE;
		}

		/* Get step after */
		y = y + ddy[p_ptr->pf_result[p_ptr->pf_result_index-1] - '0'];
		x = x + ddx[p_ptr->pf_result[p_ptr->pf_result_index-1] - '0'];

		/* Known wall */
		if ((p_ptr->cave_flag[y][x] & (CAVE_MARK)) && !cave_floor_bold(Depth, y, x))
		{
			p_ptr->running_withpathfind = FALSE;
			//run_init(p_ptr, p_ptr->pf_result[p_ptr->pf_result_index] - '0');
			p_ptr->run_request = p_ptr->pf_result[p_ptr->pf_result_index] - '0';

			return FALSE;
		}
	}

	/* Use basic running code */
	p_ptr->find_current = p_ptr->pf_result[p_ptr->pf_result_index--] - '0';
	return TRUE;
}

/*
 * Take one step along the current "run" path
 */
void run_step(player_type *p_ptr, int dir)
{
	/* Check for just changed level */
	if (p_ptr->new_level_flag) return;
#if 1
	/* Run along pathfinder */
	if (p_ptr->running_withpathfind)
	{
		/* Off */
		p_ptr->run_request = 0;
		/* Get next step (or abort) */
		if (!run_nextstep(p_ptr))
		{
			/* Disturb */
			disturb(p_ptr, 0, 0);

			/* Done */
			return;
		}
		/* On */
		p_ptr->running = TRUE;
	} else
#endif

	/* Start running */
	if (p_ptr->run_request)
	{
		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);

		/* Initialize */
		run_init(p_ptr, p_ptr->run_request);
		
		/* We are running */
		p_ptr->run_request = 0;
		p_ptr->running = TRUE;
	}

	/* Keep running */
	else
	{
		/* Update run */
		if (run_test(p_ptr))
		{
			/* Disturb */
			disturb(p_ptr, 0, 0);

			/* Done */
			return;
		}
	}

	/* Increase counter */
	p_ptr->ran_tiles += 1;


	/* Make noise */
	set_noise(p_ptr, p_ptr->noise + (30 - p_ptr->skill_stl));

	/* Move the player, using the "pickup" flag */
	move_player(p_ptr, p_ptr->find_current, option_p(p_ptr,ALWAYS_PICKUP));

	/* Hack -- don't spend energy if player attacked someone */
	/* Because we have already spent an appropriate amount elsewhere */
	if (!p_ptr->dealt_blows)
	{

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	}/* End Hack */

	/* Classic MAnghack #5. Reset built-up energy. */
	p_ptr->energy_buildup = 0;
}
