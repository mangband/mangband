/* Purpose: Angband game engine */
/* Purpose: Angband game engine */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


bool is_boring(byte feat)
{
	return (feat <= FEAT_INVIS || (feat >= 0x40 && feat <= 0x4F));
}


int find_player(s32b id)
{
	int i;

	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];
		
		if (p_ptr->id == id) return i;
	}

	/* assume none */
	return 0;
}


int find_player_name(char *name)
{
	int i;

	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];
		
		if (!strcmp(p_ptr->name, name)) return i;
	}

	/* assume none */
	return 0;
}

int count_players(int Depth)
{
	int i, count = 0;

	/* Count players on this depth */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Count */
		if (Players[i]->dun_depth == Depth) count++;
	}

	return count;
}

/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
static cptr value_check_aux1(object_type *o_ptr)
{
	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "terrible";

		/* Normal */
		return "special";
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "worthless";

		/* Normal */
		return "excellent";
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items */
	if (broken_p(o_ptr)) return "broken";

	/* Good "armor" bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/* Default to "average" */
	return "average";
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static cptr value_check_aux2(object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return "broken";

	/* Artifacts -- except cursed/broken ones */
	if (artifact_p(o_ptr)) return "good";

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return "good";

	/* Good armor bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/* No feeling */
	return (NULL);
}




/*
 * Sense the inventory
 *
 *   Class 0 = Warrior --> fast and heavy
 *   Class 1 = Mage    --> slow and light
 *   Class 2 = Priest  --> fast but light
 *   Class 3 = Rogue   --> okay and heavy
 *   Class 4 = Ranger  --> okay and heavy
 *   Class 5 = Paladin --> slow but heavy
 */
static void sense_inventory(player_type *p_ptr)
{
	int		i;

	int		plev = p_ptr->lev;

	bool	heavy = ((p_ptr->cp_ptr->flags & CF_PSEUDO_ID_HEAVY) ? TRUE : FALSE);

	cptr	feel;

	object_type *o_ptr;

	char o_name[80];
	char o_inscribe[80];


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (p_ptr->confused) return;

	if (p_ptr->cp_ptr->flags & CF_PSEUDO_ID_IMPROV)
	{
		if (0 != randint0(p_ptr->cp_ptr->sense_base / (plev * plev + p_ptr->cp_ptr->sense_div)))
			return;
	}
	else
	{
		if (0 != randint0(p_ptr->cp_ptr->sense_base / (plev + p_ptr->cp_ptr->sense_div)))
			return;
	}


	/*** Sense everything ***/

	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool okay = FALSE;

		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			{
				okay = TRUE;
				break;
			}
		}

		/* Hack non-sense machines */
		if (!okay) continue;

		/* We know about it already, do not tell us again */
		if (o_ptr->ident & ID_SENSE) continue;

		/* It is fully known, no information needed */
		if (object_known_p(p_ptr, o_ptr)) continue;

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && (0 != randint0(5))) continue;

		/* Check for a feeling */
		feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

		/* Skip non-feelings */
		if (!feel) continue;

		/* Stop everything */
		if (option_p(p_ptr,DISTURB_MINOR)) disturb(p_ptr, 0, 0);

		/* Get an object description */
		object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, FALSE, 0);

		sound(p_ptr, MSG_PSEUDOID);

		/* Message (equipment) */
		if (i >= INVEN_WIELD)
		{
			msg_format(p_ptr, "You feel the %s (%c) you are %s %s %s...",
			           o_name, index_to_label(i), describe_use(p_ptr, i),
			           ((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* Message (inventory) */
		else
		{
			msg_format(p_ptr, "You feel the %s (%c) in your pack %s %s...",
			           o_name, index_to_label(i),
			           ((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* We have "felt" it */
		o_ptr->ident |= (ID_SENSE);

		/* Inscribe it textually */
		if (o_ptr->note)
		{
			if (strstr((const char*)quark_str(o_ptr->note), (const char*)feel) == NULL)
			{
				my_strcpy(o_inscribe, (const char *)feel, sizeof(o_inscribe));
				my_strcat(o_inscribe, " - ", sizeof(o_inscribe));
				my_strcat(o_inscribe, quark_str(o_ptr->note), sizeof(o_inscribe));
				o_ptr->note = quark_add(o_inscribe);
			}
		}
		else
		{
			o_ptr->note = quark_add(feel);
		}

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Redraw slot */
		p_ptr->redraw_inven |= (1LL << i);
	}
}



/*
 * Regenerate hit points				-RAK-
 */
static void regenhp(player_type *p_ptr, int percent)
{
	s32b        new_chp, new_chp_frac;
	int                   old_chp;

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
	/* Apply the healing */
	hp_player_quiet(p_ptr, new_chp >> 16);
	//p_ptr->chp += new_chp >> 16;   /* div 65536 */

	/* check for overflow -- this is probably unneccecary */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;

	/* handle fractional hit point healing */
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		hp_player_quiet(p_ptr, 1);
		p_ptr->chp_frac = new_chp_frac - 0x10000L;
	}
	else
	{
		p_ptr->chp_frac = new_chp_frac;
	}
}


/*
 * Regenerate mana points				-RAK-
 */
static void regenmana(player_type *p_ptr, int percent)
{
	s32b        new_mana, new_mana_frac;
	int                   old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += new_mana >> 16;	/* div 65536 */
	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = new_mana_frac - 0x10000L;
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}






/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
/* Note that since this is done in real time, monsters will regenerate
 * faster in game time the deeper they are in the dungeon.
 */
static void regen_monsters(void)
{
	int i, frac;
	int time, timefactor;

	/* Regenerate everyone */
	for (i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Check if it's time to regenerate */

		/* Determine basic frequency of regen in game turns */
		time = 100; /* Default is every 100 turns (level_speed(m_ptr->dun_depth)/1000) */;
		
		/* Scale frequency by players local time bubble */
		if (m_ptr->closest_player > 0 && m_ptr->closest_player <= NumPlayers)
		{
			timefactor = base_time_factor(Players[m_ptr->closest_player], 0);
			time = time / ((float)timefactor / 100);
		}

		/* Not yet */
		if ((turn.turn % time)) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags2 & RF2_REGENERATE) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Update health bars */
			update_health(i);
		}
		/* HACK !!! Act like nobody ever hurt this monster */
		else
		{
			for (frac = 1; frac <= NumPlayers; frac++)
				Players[frac]->mon_hrt[i] = FALSE;
		}
	}
}


static void play_ambient_sound(player_type *p_ptr)
{
	int Depth = p_ptr->dun_depth;

	if (Depth < 0) return;

	/* Town sound */
	if (Depth == 0)
	{
		/* Hack - is it daytime or nighttime? */
		if (IS_DAY)
		{
			/* It's day. */
			sound(p_ptr, MSG_AMBIENT_DAY);
		}
		else
		{
			/* It's night. */
			sound(p_ptr, MSG_AMBIENT_NITE);
		}
	}

	/* Dungeon level 1-20 */
	else if (Depth <= 20)
		sound(p_ptr, MSG_AMBIENT_DNG1);

	/* Dungeon level 21-40 */
	else if (Depth <= 40)
		sound(p_ptr, MSG_AMBIENT_DNG2);

	/* Dungeon level 41-60 */
	else if (Depth <= 60)
		sound(p_ptr, MSG_AMBIENT_DNG3);

	/* Dungeon level 61-80 */
	else if (Depth <= 80)
		sound(p_ptr, MSG_AMBIENT_DNG4);

	/* Dungeon level 80- */
	else
		sound(p_ptr, MSG_AMBIENT_DNG5);
}


/*
 * Handle certain things once every 50 game turns
 */

static void process_world(player_type *p_ptr)
{
	int		x, y;

	cave_type		*c_ptr;
	byte			*w_ptr;

	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* Every 50 game turns */
	if (turn.turn % 50) return;


	/*** Check the Time and Load ***/
	/* The server will never quit --KLJ-- */

	/* Play an ambient sound at regular intervals. */
	if (!(turn.turn % ((10L * TOWN_DAWN) / 4))) play_ambient_sound(p_ptr);

	/*** Handle the "town" (stores and sunshine) ***/

	/* While in town or wilderness */
	if (p_ptr->dun_depth <= 0)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if (!(turn.turn % ((10L * TOWN_DAWN) / 2)))
		{
			int Depth = p_ptr->dun_depth;
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn.turn % (10L * TOWN_DAWN)));

			/* Day breaks */
			if (dawn)
			{
				/* Message */
				msg_print(p_ptr, "The sun has risen.");
	
				/* Make sure we're not in a store */
				if ((Depth == 0) && (p_ptr->store_num == -1))
				{
					/* Hack -- Scan the level */
					for (y = 0; y < MAX_HGT; y++)
					{
						for (x = 0; x < MAX_WID; x++)
						{
							/* Get the cave grid */
							c_ptr = &cave[Depth][y][x];
							w_ptr = &p_ptr->cave_flag[y][x];

							/* Assume lit */
							c_ptr->info |= CAVE_GLOW;

							/* Hack -- Memorize lit grids if allowed */
							if ((!Depth) && option_p(p_ptr,VIEW_PERMA_GRIDS)) *w_ptr |= CAVE_MARK;

							/* Hack -- Notice spot */
							note_spot(p_ptr, y, x);
						}
					}
				}
			}

			/* Night falls */
			else
			{
				/* Message  */
				msg_print(p_ptr, "The sun has fallen.");

				/* Make sure we're not in a store */
				if ((Depth == 0) && (p_ptr->store_num == -1))
				{
					/* Hack -- Scan the level */
					for (y = 0; y < MAX_HGT; y++)
					{
						for (x = 0; x < MAX_WID; x++)
						{
							/*  Get the cave grid */
							c_ptr = &cave[Depth][y][x];
							w_ptr = &p_ptr->cave_flag[y][x];

							/*  Darken "boring" features */
							if (is_boring(c_ptr->feat) && !(c_ptr->info & CAVE_ROOM))
							{
								/* Forget the grid */
								c_ptr->info &= ~CAVE_GLOW;
								*w_ptr &= ~CAVE_MARK;

								/* Hack -- Notice spot */
								note_spot(p_ptr, y, x);
							}
						}
					}
				}
			}

			/* Update the monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
		}
	}

	/* While in the dungeon */
	else
	{
		/*** Update the Stores ***/
		/*  Don't do this for each player.  In fact, this might be */
		/*  taken out entirely for now --KLJ-- */
	}


	/*** Process the monsters ***/
	/* Note : since monsters are added at a constant rate in real time,
	 * this corresponds in game time to them appearing at faster rates
	 * deeper in the dungeon.
	 */

	/* Check for creature generation */
	if (randint0(MAX_M_ALLOC_CHANCE) == 0)
	{
		/* Set the monster generation depth */
		if (p_ptr->dun_depth >= 0)
			monster_level = p_ptr->dun_depth;

		else monster_level = 2 + (wild_info[p_ptr->dun_depth].radius / 2);

		/* Make a new monster */
		if (p_ptr->dun_depth >= 0)
			(void)alloc_monster(p_ptr->dun_depth, MAX_SIGHT + 5, FALSE);
		else wild_add_monster(p_ptr->dun_depth);
	}
}



/*
 * Verify use of "wizard" mode
 */
#if 0
static bool enter_wizard_mode(void)
{
#ifdef ALLOW_WIZARD
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0002))
	{
		/* Mention effects */
		msg_print("Wizard mode is for debugging and experimenting.");
		msg_print("The game will not be scored if you enter wizard mode.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0002;
	}

	/* Success */
	return (TRUE);
#endif /* ALLOW_WIZARD */

	/* XXX XXX XXX Return FALSE if wizard mode is compiled out --KLJ-- */
	return (FALSE);
}
#endif


#ifdef ALLOW_WIZARD

/*
 * Verify use of "debug" commands
 */
static bool enter_debug_mode(void)
{
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0008))
	{
		/* Mention effects */
		msg_print("The debug commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use debug commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use debug commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0008;
	}

	/* Success */
	return (TRUE);
}

/*
 * Hack -- Declare the Wizard Routines
 */
extern int do_cmd_wizard(void);

#endif


#ifdef ALLOW_BORG

/*
 * Verify use of "borg" commands
 */
static bool enter_borg_mode(void)
{
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0010))
	{
		/* Mention effects */
		msg_print("The borg commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use borg commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use borg commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0010;
	}

	/* Success */
	return (TRUE);
}

/*
 * Hack -- Declare the Ben Borg
 */
extern void do_cmd_borg(void);

#endif



/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 *
 * This all happens "automagically" by the Input() function in netserver.c
 */
#if 0
static void process_command(void)
{
}
#endif

/*
 * Check for nearby players or monsters and attack.
 *
 * This function helps out the player by continuing to melee any monster
 * he attacks. If there are nearby monsters and the player has chosen no
 * action, a target will be selected at random and attacked until the 
 * player chooses some other action or the target dies.
 *
 * This function returns FALSE if no attack has been performed, TRUE if an attack
 * has been performed.
 */
static int auto_retaliate(player_type *p_ptr)
{
	player_type *q_ptr;
	int i, targets, ax, ay, tx, ty, target;
	cave_type	*c_ptr;
	int targetlist[8];
	bool istarget = FALSE;
	monster_type *m_ptr;

	/* The dungeon master does not fight his or her offspring */
	if (p_ptr->dm_flags & DM_MONSTER_FRIEND) return FALSE;

	/* Check preventive inscription '^O' */ 
	if (CPI(p_ptr, 'O')) return FALSE;

	/* Check melee weapon inscription '!O' */
	if (CGI(&(p_ptr->inventory[INVEN_WIELD]), 'O')) return FALSE;

	/* How many possible targets around us? */
	targets = 0;
	for(i = 0; i < 8; i++)
	{
		ax = p_ptr->px + ddx_ddd[i];
		ay = p_ptr->py + ddy_ddd[i];
		if(in_bounds(p_ptr->dun_depth,ay,ax))
		{
			/* Anything here? */
			c_ptr = &cave[p_ptr->dun_depth][ay][ax];
			/* Another player perhaps? */
			if (c_ptr->m_idx < 0)
			{
				/* Skip players we cannot see */
				if (!p_ptr->play_vis[0 - c_ptr->m_idx]) continue;

				/* If they are hostile, they are a fair target */
				if (pvp_okay(p_ptr, Players[0 - c_ptr->m_idx], 1))
				{
					targetlist[targets++] = i;
					if(p_ptr->health_who == c_ptr->m_idx)
					{
						q_ptr = Players[0-c_ptr->m_idx];
						istarget = TRUE;
						tx = q_ptr->px;
						ty = q_ptr->py;
					}
				}
			}
			/* Or perhaps a monster */
			else if(c_ptr->m_idx)
			{
				/* Make sure that the player can see this monster */
				if (!p_ptr->mon_vis[c_ptr->m_idx]) continue;
				
				targetlist[targets++] = i;
				if(p_ptr->health_who == c_ptr->m_idx)
				{
					m_ptr = &m_list[c_ptr->m_idx];
					istarget = TRUE;
					tx = m_ptr->fx;
					ty = m_ptr->fy;
				}
			}
		}
	}

	/* If no available targets bail out */
	if(!targets) return FALSE;

	/* If our current target is available, use that */
	if(istarget)
	{
		ax = tx;
		ay = ty;
	}
	else
	{
		/* Pick one of the available targets */
		target = randint0(targets);
		ax = p_ptr->px + ddx_ddd[targetlist[target]];
		ay = p_ptr->py + ddy_ddd[targetlist[target]];
	}
	
	/* Attack it! */
	py_attack(p_ptr, ay, ax);

	return TRUE;
}

/*
 * Hack -- helper function for "process_player()"
 *
 * Check for changes in the "monster memory"
 */
static void player_track_monster(player_type *p_ptr)
{
	int i;
	bool changed = FALSE;

	/* Tracking a monster */
	if (p_ptr->monster_race_idx)
	{
		/* Get the monster lore pointers */
		monster_lore *l_ptr = p_ptr->l_list + p_ptr->monster_race_idx;
		monster_lore *old_l = &p_ptr->old_l;

		for (i = 0; i < MONSTER_BLOW_MAX; i++)
		{
			if (old_l->blows[i] != l_ptr->blows[i])
			{
				changed = TRUE;
				break;
			}
		}

		/* Check for change of any kind */
		if (changed ||
		    (p_ptr->old_monster_race_idx != p_ptr->monster_race_idx) ||
		    (old_l->flags1 != l_ptr->flags1) ||
		    (old_l->flags2 != l_ptr->flags2) ||
		    (old_l->flags3 != l_ptr->flags3) ||
		    (old_l->flags4 != l_ptr->flags4) ||
		    (old_l->flags5 != l_ptr->flags5) ||
		    (old_l->flags6 != l_ptr->flags6) ||
		    (old_l->cast_innate != l_ptr->cast_innate) ||
		    (old_l->cast_spell != l_ptr->cast_spell))
		{
			/* Memorize old race */
			p_ptr->old_monster_race_idx = p_ptr->monster_race_idx;

			/* Memorize flags */
			old_l->flags1 = l_ptr->flags1;
			old_l->flags2 = l_ptr->flags2;
			old_l->flags3 = l_ptr->flags3;
			old_l->flags4 = l_ptr->flags4;
			old_l->flags5 = l_ptr->flags5;
			old_l->flags6 = l_ptr->flags6;

			/* Memorize blows */
			for (i = 0; i < MONSTER_BLOW_MAX; i++)
				old_l->blows[i] = l_ptr->blows[i];

			/* Memorize castings */
			old_l->cast_innate = l_ptr->cast_innate;
			old_l->cast_spell = l_ptr->cast_spell;

			/* Window stuff */
			p_ptr->window |= (PW_MONSTER);
		}
	}
}


/*
 * Player processing that occurs at the beginning of a new turn
 */
static void process_player_begin(player_type *p_ptr)
{
	int energy;

	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* Increment turn count */
	ht_add(&p_ptr->turn,1);

	/* How much energy should we get? */
	energy = extract_energy[p_ptr->pspeed];

	/* Scale depending upon our time bubble */
	p_ptr->bubble_speed = time_factor(p_ptr);
	energy = energy * ((float)p_ptr->bubble_speed / 100);

	/* In town, give everyone a RoS when they are running */
	if ((!p_ptr->dun_depth) && (p_ptr->running))
	{
		energy = energy * ((float)RUNNING_FACTOR / 100);
	}

	/* Give the player some energy */
	p_ptr->energy += energy;

	/* Classic MAnghack #5.
	 * Allow players to "buildup" twice as much energy. */
	if (option_p(p_ptr,ENERGY_BUILDUP))
	{
		/* Fill accumulated buildup into energy (from before) */
		p_ptr->energy += p_ptr->energy_buildup;
		p_ptr->energy_buildup = 0;
		/* Store new buildup (for later) */
		if (p_ptr->energy > (level_speed(p_ptr->dun_depth)))
			p_ptr->energy_buildup = p_ptr->energy - (level_speed(p_ptr->dun_depth));
		/* Don't allow to buildup too much, either */
		if (p_ptr->energy_buildup > (level_speed(p_ptr->dun_depth)))
			p_ptr->energy_buildup = (level_speed(p_ptr->dun_depth));
		/* The rest will be capped off below, as usual */
	}

	/* Make sure they don't have too much */
	if (p_ptr->energy > (level_speed(p_ptr->dun_depth)))
		p_ptr->energy = (level_speed(p_ptr->dun_depth));

	/* Check "resting" status */
	if (p_ptr->resting)
	{
		/* No energy availiable while resting */
		// This prevents us from instantly waking up.
		p_ptr->energy = 0;
	}

	/* Handle paralysis here */
	if (p_ptr->paralyzed || p_ptr->stun >= 100)
		p_ptr->energy = 0;

	/* Mega-Hack -- Random teleportation XXX XXX XXX */
	if ((p_ptr->teleport) && (randint0(100) < 1))
	{
		/* Teleport player */
		teleport_player(p_ptr, 40);
	}

}


/*
 * Player processing that occurs at the end of a turn
 */
static void process_player_end(player_type *p_ptr)
{
	int	i, j, new_depth, new_world_x, new_world_y, time, timefactor;
	int	regen_amount;
	char	attackstatus;
	u32b blow_energy;
	int minus;
	int fatal_err;

	object_type		*o_ptr;
	object_kind		*k_ptr;

	/* Remember last server turn */
	p_ptr->last_turn = turn;

	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* Try to execute any commands on the command queue. */
	fatal_err = process_player_commands(p_ptr);

	/* Paranoia -- buffered commands shouldn't even cause fatal errors */
	if (fatal_err == -1) return;

	/* Paranoia -- process_player_commands could've caused player
	 * to change level, so, once agan: */
	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* How much energy does it cost do deal 1 blow? */
	blow_energy = level_speed(p_ptr->dun_depth) / p_ptr->num_blow;


	/* Check for auto-retaliate */
	if ((p_ptr->energy >= blow_energy)                  /* - have spare energy */
	   && !p_ptr->confused && !p_ptr->afraid            /* - not confused or afraid */
	   && !p_ptr->run_request && !cq_len(&p_ptr->cbuf)) /* - no commands queued */
	{
		/* Check for nearby monsters and try to kill them */
		/* If auto_retaliate returns nonzero than we attacked
		 * something and so should use energy.
		 */
		if ((attackstatus = auto_retaliate(p_ptr)))
		{
			/* Use energy */ /* XXX spent in attack code */
			/*p_ptr->energy -= blow_energy;*/
		}
	}

	/* If we are are in a slow time condition, give visual warning */
	timefactor = base_time_factor(p_ptr, 0);
	if(timefactor < NORMAL_TIME)
	{
		/* Paranoia: cave pointer not set */
		if (p_ptr->new_level_flag == FALSE)
		lite_spot(p_ptr, p_ptr->py, p_ptr->px);
	}

	/* Handle running */
	if ((p_ptr->energy >= level_speed(p_ptr->dun_depth))
	&& (p_ptr->running || p_ptr->run_request))
	{
		run_step(p_ptr, 0);
	}

	/* Notice stuff */
	if (p_ptr->notice) notice_stuff(p_ptr);

	/* XXX XXX XXX Pack Overflow */
	if (p_ptr->inventory[INVEN_PACK].k_idx)
	{
		int		amt;

		char	o_name[80];


		/* Choose an item to spill */
		i = INVEN_PACK;

		/* Access the slot to be dropped */
		o_ptr = &p_ptr->inventory[i];

		/* Drop all of that item */
		amt = o_ptr->number;

		/* Disturbing */
		disturb(p_ptr, 0, 0);

		/* Warning */
		msg_print(p_ptr, "Your pack overflows!");

		/* Describe */
		object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 3);

		/* Message */
		msg_format(p_ptr, "You drop %s.", o_name);

		/* Drop it (carefully) near the player */
		drop_near(o_ptr, 0, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Decrease the item, optimize. */
		inven_item_increase(p_ptr, i, -amt);
		inven_item_optimize(p_ptr, i);
	}


	/* Process things such as regeneration, poison, cuts, etc. */
	
	/* Determine basic frequency of regen in game turns */
	time = level_speed(p_ptr->dun_depth)/1000;
	
	/* Scale frequency by players local time bubble */
	time = time / ((float)timefactor / 100);

	/* Use food, 10 times slower than other regen effects */
	if ( !(turn.turn % (time*10)) )
	{
		bool do_digest = TRUE;
		/* Ghosts don't need food */
		if (p_ptr->ghost) do_digest = FALSE;
		/* Noone uses food in town */
		if ((p_ptr->dun_depth == 0 || check_special_level(p_ptr->dun_depth))
			/* ...unless gorged */
			&& (p_ptr->food < PY_FOOD_MAX))
				do_digest = FALSE;
		/* Proceed? */
		if (do_digest)
		{
			/* Digest normally */
			if (p_ptr->food < PY_FOOD_MAX)
			{
				/* Basic digestion rate based on speed */
				i = (extract_energy[p_ptr->pspeed]/100) * 2;

				/* Regeneration takes more food */
				if (p_ptr->regenerate) i += 30;

				/* Slow digestion takes less food */
				if (p_ptr->slow_digest) i -= 10;

				/* Digest some food */
				(void)set_food(p_ptr, p_ptr->food - i);
			}

			/* Digest quickly when gorged */
			else
			{
				/* Digest a lot of food */
				(void)set_food(p_ptr, p_ptr->food - 100);
			}

			/* Starve to death (slowly) */
			if (p_ptr->food < PY_FOOD_STARVE)
			{
				/* Hack -- if the player is idle for >= 3 minutes,
				 * destroy his conneciton */
				#ifdef DISCONNECT_STARVING
				if (p_ptr->afk_seconds >= DISCONNECT_STARVING)
				{
					player_disconnect(p_ptr, "Starving to death!");
					return;
				}
				#endif

				/* Calculate damage */
				i = (PY_FOOD_STARVE - p_ptr->food) / 10;

				/* Take damage */
				take_hit(p_ptr, i, "starvation");
			}
		}

		/* Semi-constant hallucination (but not in stores) */
		if (p_ptr->image && p_ptr->store_num == -1)
		{
			p_ptr->image_seed++;
			p_ptr->redraw |= (PR_MAP);
		}
	}

	if ( !(turn.turn % time) )
	{
		/* Hack -- Fade monster Detect over time */
		for (i = 0; i < m_max; i++)
		{
			if (p_ptr->mon_det[i])
			{
				if (--p_ptr->mon_det[i] == 0) 
				{
					update_mon(i, FALSE);
				}
			}
		}
		/* Hack -- Fade player Detect over time */
		for (i = 1; i <= NumPlayers; i++)
		{
			if (p_ptr->play_det[i])
			{
				if (--p_ptr->play_det[i] == 0)
				{
					update_player(Players[i]);
				}
			}
		}

		/*** Damage over Time ***/

		/* Take damage from poison */
		if (p_ptr->poisoned)
		{
			/* Take damage */
			take_hit(p_ptr, 1, "poison");
		}

		/* Take damage from cuts */
		if (p_ptr->cut)
		{
			/* Mortal wound or Deep Gash */
			if (p_ptr->cut > 200)
			{
				i = 3;
			}

			/* Severe cut */
			else if (p_ptr->cut > 100)
			{
				i = 2;
			}

			/* Other cuts */
			else
			{
				i = 1;
			}

			/* Take damage */
			take_hit(p_ptr, i, "a fatal wound");
		}

		/*** Check the Food, and Regenerate ***/

		/* Default regeneration */
		regen_amount = PY_REGEN_NORMAL;

		/* Getting Weak */
		if (p_ptr->food < PY_FOOD_WEAK)
		{
			/* Lower regeneration */
			if (p_ptr->food < PY_FOOD_STARVE)
			{
				regen_amount = 0;
			}
			else if (p_ptr->food < PY_FOOD_FAINT)
			{
				regen_amount = PY_REGEN_FAINT;
			}
			else
			{
				regen_amount = PY_REGEN_WEAK;
			}

			/* Getting Faint */
			if (!p_ptr->ghost && p_ptr->food < PY_FOOD_FAINT)
			{
				/* Faint occasionally */
				if (!p_ptr->paralyzed && (randint0(100) < 10))
				{
					/* Message */
					msg_print(p_ptr, "You faint from the lack of food.");
					disturb(p_ptr, 1, 0);

					/* Hack -- faint (bypass free action) */
					(void)set_paralyzed(p_ptr, p_ptr->paralyzed + 1 + randint0(5));
				}
			}
		}

		/* Regeneration ability */
		if (p_ptr->regenerate)
		{
			regen_amount = regen_amount * 2;
		}

		/* Resting */
		if (p_ptr->resting || p_ptr->searching)
		{
			regen_amount = regen_amount * 2;
		}

		/* Regenerate the mana */
		if (p_ptr->csp < p_ptr->msp)
		{
			regenmana(p_ptr, regen_amount);
		}

		/* Poisoned or cut yields no healing */
		if (p_ptr->poisoned) regen_amount = 0;
		if (p_ptr->cut) regen_amount = 0;

		/* Regenerate Hit Points if needed */
		if (p_ptr->chp < p_ptr->mhp)
		{
			regenhp(p_ptr, regen_amount);
		}

		/* Disturb if we are done resting */
		if ((p_ptr->resting) && (p_ptr->chp == p_ptr->mhp) && (p_ptr->csp == p_ptr->msp))
		{
			disturb(p_ptr, 0, 0);
		}

		minus = 1;

		/* Finally, at the end of our turn, update certain counters. */
		/*** Timeout Various Things ***/

		/* Reduce noise levels */
		if (p_ptr->noise)
		{
			(void)set_noise(p_ptr, p_ptr->noise - (p_ptr->skill_stl + 1));
		}

		/* Hack -- Hallucinating */
		if (p_ptr->image)
		{
			(void)set_image(p_ptr, p_ptr->image - 1);
		}

		/* Blindness */
		if (p_ptr->blind)
		{
			(void)set_blind(p_ptr, p_ptr->blind - 1);
		}

		/* Times see-invisible */
		if (p_ptr->tim_invis)
		{
			(void)set_tim_invis(p_ptr, p_ptr->tim_invis - 1);
		}

		/* Timed infra-vision */
		if (p_ptr->tim_infra)
		{
			(void)set_tim_infra(p_ptr, p_ptr->tim_infra - 1);
		}

		/* Paralysis */
		if (p_ptr->paralyzed)
		{
			(void)set_paralyzed(p_ptr, p_ptr->paralyzed - 1);
		}

		/* Confusion */
		if (p_ptr->confused)
		{
			(void)set_confused(p_ptr, p_ptr->confused - 1);
		}

		/* Afraid */
		if (p_ptr->afraid)
		{
			(void)set_afraid(p_ptr, p_ptr->afraid - 1);
		}

		/* Fast */
		if (p_ptr->fast)
		{
			(void)set_fast(p_ptr, p_ptr->fast - 1);
		}

		/* Slow */
		if (p_ptr->slow)
		{
			(void)set_slow(p_ptr, p_ptr->slow - 1);
		}

		/* Protection from evil */
		if (p_ptr->protevil)
		{
			(void)set_protevil(p_ptr, p_ptr->protevil - 1);
		}

		/* Invulnerability */
		/* Hack -- make -1 permanent invulnerability */
		if (p_ptr->invuln)
		{
			if (p_ptr->invuln > 0)
				(void)set_invuln(p_ptr, p_ptr->invuln - 1);
		}

		/* Heroism */
		if (p_ptr->hero)
		{
			(void)set_hero(p_ptr, p_ptr->hero - 1);
		}

		/* Super Heroism */
		if (p_ptr->shero)
		{
			(void)set_shero(p_ptr, p_ptr->shero - 1);
		}

		/* Blessed */
		if (p_ptr->blessed)
		{
			(void)set_blessed(p_ptr, p_ptr->blessed - 1);
		}

		/* Shield */
		if (p_ptr->shield)
		{
			(void)set_shield(p_ptr, p_ptr->shield - 1);
		}

		/* Oppose Acid */
		if (p_ptr->oppose_acid)
		{
			(void)set_oppose_acid(p_ptr, p_ptr->oppose_acid - 1);
		}

		/* Oppose Lightning */
		if (p_ptr->oppose_elec)
		{
			(void)set_oppose_elec(p_ptr, p_ptr->oppose_elec - 1);
		}

		/* Oppose Fire */
		if (p_ptr->oppose_fire)
		{
			(void)set_oppose_fire(p_ptr, p_ptr->oppose_fire - 1);
		}

		/* Oppose Cold */
		if (p_ptr->oppose_cold)
		{
			(void)set_oppose_cold(p_ptr, p_ptr->oppose_cold - 1);
		}

		/* Oppose Poison */
		if (p_ptr->oppose_pois)
		{
			(void)set_oppose_pois(p_ptr, p_ptr->oppose_pois - 1);
		}

		/*** Poison and Stun and Cut ***/

		/* Poison */
		if (p_ptr->poisoned)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Apply some healing */
			(void)set_poisoned(p_ptr, p_ptr->poisoned - adjust);
		}

		/* Stun */
		if (p_ptr->stun)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Apply some healing */
			(void)set_stun(p_ptr, p_ptr->stun - adjust);
		}

		/* Cut */
		if (p_ptr->cut)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Hack -- Truly "mortal" wound */
			if (p_ptr->cut > 1000) adjust = 0;

			/* Apply some healing */
			(void)set_cut(p_ptr, p_ptr->cut - adjust);
		}

		/*** Process Light ***/

		/* Check for light being wielded */
		o_ptr = &p_ptr->inventory[INVEN_LITE];

		/* Burn some fuel in the current lite */
		if (o_ptr->tval == TV_LITE)
		{
			/* Hack -- Use some fuel (sometimes) */
			if (!artifact_p(o_ptr) && !(o_ptr->sval == SV_LITE_DWARVEN)
                && !(o_ptr->sval == SV_LITE_FEANOR) && (o_ptr->pval > 0) && (!o_ptr->name3))
			{
				/* Decrease life-span */
				o_ptr->pval--;

				/* Hack -- notice interesting fuel steps */
				if ((o_ptr->pval < 100) || (!(o_ptr->pval % 100)))
				{
					/* Redraw this slot */
					p_ptr->redraw_inven |= (1LL << INVEN_LITE);
				}

				/* Hack -- Special treatment when blind */
				if (p_ptr->blind)
				{
					/* Hack -- save some light for later */
					if (o_ptr->pval == 0) o_ptr->pval++;
				}

				/* The light is now out */
				else if (o_ptr->pval == 0)
				{
					disturb(p_ptr, 0, 0);
					msg_print(p_ptr, "Your light has gone out!");
				}

				/* The light is getting dim */
				else if ((o_ptr->pval < 100) && (!(o_ptr->pval % 10)))
				{
					if (option_p(p_ptr,DISTURB_MINOR)) disturb(p_ptr, 0, 0);
					msg_print(p_ptr, "Your light is growing faint.");
				}
			}
		}

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);

		/*** Process Inventory ***/

		/* Handle experience draining */
		if (p_ptr->exp_drain)
		{
			if ((randint0(100) < 10) && (p_ptr->exp > 0))
			{
				p_ptr->exp--;
				p_ptr->max_exp--;
				check_experience(p_ptr);
			}
		}

		/* Note changes */
		j = 0;

		/* Process equipment */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			/* Get the object */
			o_ptr = &p_ptr->inventory[i];

			/* Skip non-objects */
			if (!o_ptr->k_idx) continue;

			/* Recharge activatable objects */
			if (o_ptr->timeout > 0)
			{
				/* Recharge */
				o_ptr->timeout--;

				/* Notice changes */
				if (!(o_ptr->timeout))
				{
					j++;
					/* Redraw slot */
					p_ptr->redraw_inven |= (1LL << i);
				}
			}
		}

		/* Notice changes (equipment) */
		if (j)
		{
		}

		j = 0;

		/* Don't recharge rods in shops (fixes stacking exploits) */
		if (p_ptr->store_num < 0)
		{
			/* Recharge rods */
			for (i = 0; i < INVEN_PACK; i++)
			{
				o_ptr = &p_ptr->inventory[i];
				k_ptr = &k_info[o_ptr->k_idx];

				/* Skip non-objects */
				if (!o_ptr->k_idx) continue;

				/* Examine all charging rods */
				if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
				{
					/* Determine how many rods are charging */
					int temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

					if (temp > o_ptr->number) temp = o_ptr->number;

					/* Decrease timeout by that number */
					o_ptr->timeout -= temp;

					/* Boundary control */
					if (o_ptr->timeout < 0) o_ptr->timeout = 0;

					/* Update if any rods are recharged */
					if (temp > (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval)
					{
						/* Notice changes */
						j++;
						/* Redraw slot */
						p_ptr->redraw_inven |= (1LL << i);
					}
				}
			}
		}

		/* Notice changes */
		if (j)
		{
			/* Combine pack */
			p_ptr->notice |= (PN_COMBINE);
		}

		/* Feel the inventory */
		sense_inventory(p_ptr);

		/*** Involuntary Movement ***/

		/* Delayed Word-of-Recall */
		if (p_ptr->word_recall)
		{
			/* Count down towards recall */
			p_ptr->word_recall--;


			/* MEGA HACK: no recall if in shop */
			if(!p_ptr->word_recall && p_ptr->store_num != -1)
			{
				/* Delay for 1 turn */
			    p_ptr->word_recall++;
			}

			/* Activate the recall */
			if (!p_ptr->word_recall)
			{
				/* Disturbing! */
				disturb(p_ptr, 0, 0);

				/* Sound */
				sound(p_ptr, MSG_TPLEVEL);

				/* Determine the level */
				if (p_ptr->dun_depth > 0)
				{
					/* Messages */
					msg_print(p_ptr, "You feel yourself yanked upwards!");
					msg_format_near(p_ptr, "%s is yanked upwards!", p_ptr->name);
					
					/* New location */
					new_depth = 0;
					new_world_x = p_ptr->world_x;
					new_world_y = p_ptr->world_y;
					
					p_ptr->new_level_method = LEVEL_RAND;
				}
				else if ((p_ptr->dun_depth < 0) || (p_ptr->recall_depth < 0))
				{
					/* Messages */
					msg_print(p_ptr, "You feel yourself yanked sideways!");
					msg_format_near(p_ptr, "%s is yanked sideways!", p_ptr->name);
					
					/* New location */
					if (p_ptr->dun_depth < 0) 
					{
						new_depth = 0;
						new_world_x = 0;
						new_world_y = 0;										
					}
					else 
					{ 
						new_depth = p_ptr->recall_depth;
						new_world_x = wild_info[new_depth].world_x;
						new_world_y = wild_info[new_depth].world_y;
					}
					p_ptr->new_level_method = LEVEL_OUTSIDE_RAND;												
				}
				else
				{
					/* Messages */
					msg_print(p_ptr, "You feel yourself yanked downwards!");
					msg_format_near(p_ptr, "%s is yanked downwards!", p_ptr->name);
					new_depth = p_ptr->recall_depth;
					new_world_x = p_ptr->world_x;
					new_world_y = p_ptr->world_y;
					p_ptr->new_level_method = LEVEL_RAND;
				}

				/* One less person here */
				players_on_depth[p_ptr->dun_depth]--;

				/* paranoia, required for adding old wilderness saves to new servers */
				if (players_on_depth[p_ptr->dun_depth] < 0) players_on_depth[p_ptr->dun_depth] = 0;

				/* Remove the player */
				cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0;

				/* Show everyone that he's left */
				everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

				/* Tell everyone to re-calculate visiblity for this player */
				update_player(p_ptr);

				/* Forget his lite and view */
				forget_lite(p_ptr);
				forget_view(p_ptr);

				p_ptr->dun_depth = new_depth;
				p_ptr->world_x = new_world_x;
				p_ptr->world_y = new_world_y;
				/* XXX Hack -- arena paranoia */
				p_ptr->arena_num = -1;

				/* One more person here */
				players_on_depth[p_ptr->dun_depth]++;

				p_ptr->new_level_flag = TRUE;
			}
		}
	}

	/* Track monster */
	player_track_monster(p_ptr);

	/* HACK -- redraw stuff a lot, this should reduce perceived latency. */
	/* This might not do anything, I may have been silly when I added this. -APD */
	/* Notice stuff (if needed) */
	//handle_stuff(p_ptr);
}




/*
 * This function handles "global" things such as the stores,
 * day/night in the town, etc.
 */
 
/* Added the japanese unique respawn patch -APD- 
   It appears that each moment of time is equal to 10 minutes?
*/
static void process_various(void)
{
	int i, j, y, x, num_on_depth;
	cave_type *c_ptr;
	player_type *p_ptr;

	//char buf[1024];

	/* Save the server state occasionally */
	if (!(turn.turn % (cfg_fps * 60 * SERVER_SAVE)))
	{
		save_server_info();

		/* Save each player */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Save this player */
			save_player(Players[i]);
		}
	}

	/* Handle certain things once a minute */
	if (!(turn.turn % (cfg_fps * 60)))
	{
		/* Update the player retirement timers */
		for (i = 1; i <= NumPlayers; i++)
		{
			p_ptr = Players[i];

			// If our retirement timer is set
			if (p_ptr->retire_timer > 0)
			{
				// Decrement our retire timer
				p_ptr->retire_timer--;

				// If the timer runs out, forcibly retire
				// this character.
				if (!p_ptr->retire_timer)
				{
					do_cmd_suicide(p_ptr);
				}
			}

		}

		/* Update the unique respawn timers */
		for (i = 1; i < z_info->r_max; i++)
		{
			monster_race *r_ptr = &r_info[i];

			/* Make sure we are looking at a dead unique */
			if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
			if (r_ptr->max_num > 0) continue;

			/* Never respawn Morgoth */
			if (r_ptr->flags1 & RF1_DROP_CHOSEN) continue;

			/* Hack -- Initially set a newly killed uniques respawn timer */
			/* -1 denotes that the respawn timer is unset */
   			if (r_ptr->respawn_timer < 0) 
   			{
				r_ptr->respawn_timer = cfg_unique_respawn_time * (r_ptr->level + 1);
				if (r_ptr->respawn_timer > cfg_unique_max_respawn_time)
  					r_ptr->respawn_timer = cfg_unique_max_respawn_time;
  			}
			// Decrament the counter
			else r_ptr->respawn_timer--;
			// Once the timer hits 0, ressurect the unique.
#if 0
			if (!r_ptr->respawn_timer)
			{
				/* "Ressurect" the unique */
				r_ptr->max_num = 1;
				r_ptr->respawn_timer = -1;

				/* don't announce */
				/*
				sprintf(buf,"%s rises from the dead!",(r_name + r_ptr->name));
				msg_broadcast(0,buf);
				*/

			}
#endif
		}

		// If the level unstaticer is not disabled
		if (cfg_level_unstatic_chance > 0)
		{
			// For each dungeon level
			for (i = 1; i < MAX_DEPTH; i++)
			{
				// If this depth is static
				if (players_on_depth[i])
				{
					num_on_depth = 0;
					// Count the number of players actually in game on this depth
					for (j = 1; j <= NumPlayers; j++)
					{
						p_ptr = Players[j];
						if (p_ptr->dun_depth == i) num_on_depth++;
					}
					// If this level is static and no one is actually on it
					if (!num_on_depth)
					{
						// random chance of the level unstaticing
						// the chance is one in (base_chance * depth)/250 feet.
						if (!randint0(((cfg_level_unstatic_chance * (i+5))/5)-1))
						{
							// unstatic the level
							players_on_depth[i] = 0;
						}
					}
				}
			}
		}
	}

	/* Grow crops very occasionally */
	if (!(turn.turn % (10L * GROW_CROPS)))
	{
		/* For each wilderness level */
		for (i = 1; i < MAX_DEPTH; i++)
		{
			wild_grow_crops(-i);
		}
	}

	/* Grow trees very occasionally */
	if (!(turn.turn % (10L * GROW_TREE)) && (trees_in_town < cfg_max_trees || cfg_max_trees == -1))
	{
		/* Find a suitable location */
		for (i = 1; i < 1000; i++)
		{
			cave_type *c_ptr;

			/* Pick a location */
			y = rand_range(1, MAX_HGT - 1);
			x = rand_range(1, MAX_WID - 1);

			/* Acquire pointer */
			c_ptr = &cave[0][y][x];

			/* Only allow "dirt" */
			if (c_ptr->feat != FEAT_DIRT) continue;

			/* Never grow on top of objects or monsters */
			if (c_ptr->m_idx) continue;
			if (c_ptr->o_idx) continue;

			/* Grow a tree here */
			c_ptr->feat = FEAT_TREE;
			trees_in_town++;

			/* Show it */
			everyone_lite_spot(0, y, x);

			/* Done */
			break;
		}
	}

	/* Update the stores */
	if (!(turn.turn % (10L * STORE_TURNS)))
	{
		int n;

		/* Maintain each shop */
		for (n = 0; n < MAX_STORES; n++)
		{
			/* Maintain */
			store_maint(n);
		}

		/* Sometimes, shuffle the shopkeepers */
		if (randint0(STORE_SHUFFLE) == 0)
		{
			/* Shuffle a random shop (except home and higher) */
			store_shuffle(randint0(MAX_STORES - 2));
		}
	}

	/* Hack -- Daybreak/Nightfall outside the dungeon */
	if (!(turn.turn % ((10L * TOWN_DAWN) / 2)))
	{
		bool dawn;

		/* Check for dawn */
		dawn = (!(turn.turn % (10L * TOWN_DAWN)));
		/* Day breaks */
		if (dawn)
		{
			/* Mega-Hack -- erase all wilderness monsters.
			 * This should prevent wilderness monster "buildup",
			 * massive worm infestations, and uniques getting
			 * lost out there.
			 */
			for (i = 1; i < MAX_WILD; i++) 
				/* if no one is here the monsters 'migrate'.*/
				if (!players_on_depth[-i]) wipe_m_list(-i);
			/* another day, more stuff to kill... */
			for (i = 1; i < MAX_WILD; i++) wild_info[-i].flags &= ~(WILD_F_INHABITED);

			/* Hack -- Scan the town */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					/* Get the cave grid */
					c_ptr = &cave[0][y][x];

					/* Assume lit */
					c_ptr->info |= CAVE_GLOW;

					/* Hack -- Notice spot */
					note_spot_depth(0, y, x);
				}
			} 
		}	
		else
		{
			/* Hack -- Scan the town */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					 /* Get the cave grid */
					c_ptr = &cave[0][y][x];

					 /* Darken "boring" features */
					if (is_boring(c_ptr->feat) && !(c_ptr->info & CAVE_ROOM))
					{
						 /* Darken the grid */
						c_ptr->info &= ~CAVE_GLOW;
					}
				}
			}
			/* hack -- make fruit bat wear off */
			for (x = 1; x <= NumPlayers; x++)
			{
				p_ptr = Players[x];
				if (p_ptr->fruit_bat > 0)
				{
					p_ptr->fruit_bat--;
					if (!p_ptr->fruit_bat)
					{
						p_ptr->update |= (PU_BONUS | PU_HP);
						msg_print(p_ptr, "Your form feels much more familliar.");
					}
				}
			}
		}
	}
}



/*
 * Main loop --KLJ--
 *
 * This is main loop; it is called every 1/FPS seconds.  Usually FPS is about
 * 50, so that a normal unhasted unburdened character gets 1 player turn per
 * second.  Note that we process every player and the monsters, then quit.
 * The "scheduling" code (see sched.c) is the REAL main loop, which handles
 * various inputs and timings.
 */

void dungeon(void)
{
	int i, d, j;
	byte *w_ptr;
	cave_type *c_ptr;
	int dy, dx;

	/* Return if no one is playing */
	/* if (!NumPlayers) return; */

	/* Check for death.  Go backwards (very important!) */
	for (i = NumPlayers; i > 0; i--)
	{
		/* Check for death */
		if (Players[i]->death)
		{
			/* Kill him */
			player_death(Players[i]);
		}
	}


	/* Deallocate any unused levels */
	for (j = -MAX_WILD+1; j < MAX_DEPTH; j++)
	{
		/* Everybody has left a level that is still generated */
		if (players_on_depth[j] == 0 && cave[j])
		{
			/* Destroy the level */
			/* Hack -- don't dealloc the town */
			/* Hack -- don't dealloc special levels */
			if( (j) && (!check_special_level(j)) )
				dealloc_dungeon_level(j);
		}
	}

	/* Check player's depth info */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];
		int Depth = p_ptr->dun_depth;
		int x, y, startx, starty;

		if (players_on_depth[Depth] == 0) continue;

		if (!p_ptr->new_level_flag)
			continue;

		/* Check "maximum depth" to make sure it's still correct */
		if (Depth > p_ptr->max_dlv)
			p_ptr->max_dlv = Depth;

		/* Make sure the server doesn't think the player is in a store */
		p_ptr->store_num = -1;

		/* Somebody has entered an ungenerated level */
		if (players_on_depth[Depth] && !cave[Depth])
		{
			/* Allocate space for it */
			alloc_dungeon_level(Depth);

			/* Generate a dungeon level there */
			generate_cave(p_ptr, Depth, option_p(p_ptr,AUTO_SCUM));
			
			/* Give a level feeling to this player */
			p_ptr->feeling = feeling;
			/* No feeling outside the dungeon */
			if (Depth > 0) do_cmd_feeling(p_ptr);
		}

		/* Clear the "marked" and "lit" flags for each cave grid */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				w_ptr = &p_ptr->cave_flag[y][x];

				*w_ptr = 0;
			}
		}

		/* hack -- update night/day in wilderness levels */
		if ((Depth < 0) && (IS_DAY)) wild_apply_day(Depth); 
		if ((Depth < 0) && (IS_NIGHT)) wild_apply_night(Depth);

		/* Memorize the town and all wilderness levels close to town */
		if (Depth <= 0 ? (wild_info[Depth].radius <= 2) : 0)
		{
			bool dawn = ((turn.turn % (10L * TOWN_DAWN)) < (10L * TOWN_DAWN / 2)); 

			setup_panel(p_ptr, FALSE);

			/* Memorize the town for this player (if daytime) */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					w_ptr = &p_ptr->cave_flag[y][x];
					c_ptr = &cave[Depth][y][x];

					/* Memorize if daytime or "interesting" */
					if (dawn || (!is_boring(c_ptr->feat)) || c_ptr->info & CAVE_ROOM)
						*w_ptr |= CAVE_MARK;
				}
			}
		}
		else
		{
			setup_panel(p_ptr, FALSE);
		}

		/* Determine starting location */
		switch (p_ptr->new_level_method)
		{
			/* Climbed down */
			case LEVEL_DOWN:  starty = level_down_y[Depth];
					  startx = level_down_x[Depth];
					  break;

			/* Climbed up */
			case LEVEL_UP:    starty = level_up_y[Depth];
					  startx = level_up_x[Depth];
					  break;
			
			/* Teleported level */
			case LEVEL_RAND:  starty = level_rand_y[Depth];
					  startx = level_rand_x[Depth];
					  break;
			
			/* Used ghostly travel */
			case LEVEL_GHOST: starty = p_ptr->py;
					  startx = p_ptr->px;
					  break;
					  
			/* Over the river and through the woods */
			case LEVEL_OUTSIDE: starty = p_ptr->py;
				            startx = p_ptr->px;
				            break;
			/* this is used instead of extending the level_rand_y/x
			   into the negative direction to prevent us from
			   alocing so many starting locations.  Although this does
			   not make players teleport to simmilar locations, this
			   could be achieved by seeding the RNG with the depth.
			*/
			case LEVEL_OUTSIDE_RAND: 
			
				/* make sure we aren't in an "icky" location */
				do
				{
					starty = randint0(MAX_HGT-3)+1;
					startx = randint0(MAX_WID-3)+1;
				}
				while (  (cave[Depth][starty][startx].info & CAVE_ICKY)
				      || (!cave_floor_bold(Depth, starty, startx)) );
				break;
		}

		/* Place the player in an empty space */
		for (j = 0; j < 1500; ++j)
		{
			/* Increasing distance */
			d = (j + 149) / 150;

			/* Pick a location */
			scatter(Depth, &y, &x, starty, startx, d, 1);

			/* Must have an "empty" grid */
			if(cave[Depth]) if (!cave_empty_bold(Depth, y, x)) continue;

			/* Not allowed to go onto a icky location (house) if Depth <= 0 */
			if ((Depth <= 0) && (cave[Depth][y][x].info & CAVE_ICKY))
				continue;

			break;
		}

#if 0
		while (TRUE)
		{
			y = rand_range(1, ((Depth) ? (MAX_HGT - 2) : (p_ptr->screen_hgt - 2)));
			x = rand_range(1, ((Depth) ? (MAX_WID - 2) : (p_ptr->screen_wid - 2)));

			/* Must be a "naked" floor grid */
			if (!cave_naked_bold(Depth, y, x)) continue;

			/* Refuse to start on anti-teleport grids */
			if (cave[Depth][y][x].info & CAVE_ICKY) continue;

			break;
		}
#endif

		p_ptr->py = y;
		p_ptr->px = x;

		/* Update the player location */
		cave[Depth][y][x].m_idx = 0 - i;
    
		/* Prevent hound insta-death */
		switch (p_ptr->new_level_method)
		{
			/* Only when going *down* stairs (we don't want to make stair scuming safe) */
			/* Also when moving to another level via spell, such as when recalling */
			case LEVEL_DOWN:  
			case LEVEL_RAND:

				/* Remove nearby hounds */
				for (j = 1; j < m_max; j++)
				{
					monster_type	*m_ptr = &m_list[j];
					monster_race	*r_ptr = &r_info[m_ptr->r_idx];
		
					/* Paranoia -- Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Hack -- Skip Unique Monsters */
					if (r_ptr->flags1 & RF1_UNIQUE) continue;

					/* Skip monsters other than hounds */
					if (r_ptr->d_char != 'Z') continue;

					/* Skip monsters not on this depth */
					if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

					/* Approximate distance */
					dy = (p_ptr->py > m_ptr->fy) ? (p_ptr->py - m_ptr->fy) : (m_ptr->fy - p_ptr->py);
					dx = (p_ptr->px > m_ptr->fx) ? (p_ptr->px - m_ptr->fx) : (m_ptr->fx - p_ptr->px);
					d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

					/* Skip distant monsters */
					if (d > MAX_SIGHT) continue;

					/* Delete the monster */
					delete_monster_idx(j);
				}
			break;
		}

		/* Recalculate panel */
		p_ptr->panel_row = ((p_ptr->py - p_ptr->screen_hgt / 4) / (p_ptr->screen_hgt / 2));
		if (p_ptr->panel_row > p_ptr->max_panel_rows) p_ptr->panel_row = p_ptr->max_panel_rows;
		else if (p_ptr->panel_row < 0) p_ptr->panel_row = 0;

		p_ptr->panel_col = ((p_ptr->px - p_ptr->screen_wid / 4) / (p_ptr->screen_wid / 2));
		if (p_ptr->panel_col > p_ptr->max_panel_cols) p_ptr->panel_col = p_ptr->max_panel_cols;
		else if (p_ptr->panel_col < 0) p_ptr->panel_col = 0;

		p_ptr->redraw |= (PR_MAP);
		p_ptr->redraw |= (PR_DEPTH);
		p_ptr->redraw |= (PR_FLOOR);

		panel_bounds(p_ptr);
		forget_view(p_ptr);
		forget_lite(p_ptr);
		update_view(p_ptr);
		update_lite(p_ptr);
		update_monsters(TRUE);
		update_players();

		p_ptr->window |= (PW_ITEMLIST);

		/* Clear the flag */
		p_ptr->new_level_flag = FALSE;
	}

	/* Hack -- Compact the object list occasionally */
	if (o_top + 16 > MAX_O_IDX) compact_objects(32);

	/* Hack -- Compact the monster list occasionally */
	if (m_top + 32 > MAX_M_IDX) compact_monsters(64);


	// Note -- this is the END of the last turn

	/* Do final end of turn processing for each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Actually process that player */
		process_player_end(Players[i]);
	}

	/* Check for death.  Go backwards (very important!) */
	for (i = NumPlayers; i > 0; i--)
	{
		/* Check for death */
		if (Players[i]->death)
		{
			/* Kill him */
			player_death(Players[i]);
		}
	}



	///*** BEGIN NEW TURN ***///
	ht_add(&turn,1);

	/* Do some beginning of turn processing for each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Actually process that player */
		process_player_begin(Players[i]);
	}

	/* Process all of the monsters */
	process_monsters();

	/* Process all of the objects */
	process_objects();

	/* Probess the world */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Process the world of that player */
		process_world(Players[i]);
	}

	/* Process everything else */
	process_various();

	/* Hack -- Regenerate the monsters every hundred game turns */
	regen_monsters();

	/* Refresh everybody's displays */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];

		/* Flush pending updates */
		handle_stuff(p_ptr);
	}
}

		
/*
 * Load the various "user pref files"
 */
static void load_all_pref_files(void)
{
	char buf[1024];


	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "user.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "font.prf");

	/* Process that file */
	process_pref_file(buf);


}


/*
 * Actually play a game
 *
 * If the "new_game" parameter is true, then, after loading the
 * server-specific savefiles, we will start anew.
 */
void play_game(bool new_game)
{
	int i, n;

	/* Flash a message */
	plog("Please wait...");

	/* Attempt to load the server state information */
	if (!load_server_info())
	{
		/* Oops */
		quit("broken server savefile(s)");
	}

	/* UltraHack -- clear each wilderness levels inhabited flag, so
	   monsters will respawn.
	   hack -- clear the wild_f_in_memory flag, so house objects are added
	   once and only once.
	
	   I believe this is no longer neccecary.
	for (i = 1; i < MAX_WILD; i++) wild_info[-i].flags &= ~(WILD_F_IN_MEMORY);
	*/

	/* Nothing loaded */
	if (!server_state_loaded)
	{
		/* Make server state info */
		new_game = TRUE;

		/* Create a new dungeon */
		server_dungeon = FALSE;
	}

	/* Process old character */
	if (!new_game)
	{
		/* Process the player name */
		/*process_player_name(FALSE);*/
	}

	/* Init the RNG */
	if (Rand_quick)
	{
		u32b seed;

		/* Basic seed */
		seed = (time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() << 1));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Roll new town */
	if (new_game)
	{
		/* Ignore the dungeon */
		server_dungeon = FALSE;

		/* Start in town */
		/*dun_level = 0;*/

		/* Hack -- seed for flavors */
		seed_flavor = randint0(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = randint0(0x10000000);

		/* Initialize server state information */
		/*player_birth();*/
		server_birth();

		/* Hack -- enter the world */
		ht_clr(&turn);
		ht_add(&turn,1);

		/* Initialize the stores */
		for (n = 0; n < MAX_STORES; n++)
		{
			/* Initialize */
			store_init(n);
	
			/* Maintain the shop */
			for (i = 0; i < 10; i++) store_maint(n);
		}
	}


	/* Prepare the spells */
	spells_init();


	/* Flavor the objects */
	flavor_init();

	/* Reset the visual mappings */
	reset_visuals();

	/* Load requested "pref" file */
	if (cfg_load_pref_file)
	{
		plog(format("Loading pref file: %s", cfg_load_pref_file));
		process_pref_file(cfg_load_pref_file);
	}
#if 0
	/* Load the "pref" files */
	load_all_pref_files();
#endif

	/* Make a town if necessary */
	if (!server_dungeon)
	{
		/* Actually generate the town */
		generate_cave(0, 0, 0);
	}

	/* Finish initializing dungeon monsters */
	setup_monsters();

	/* Finish initializing dungeon objects */
	setup_objects();

	/* Server initialization is now "complete" */
	server_generated = TRUE;
}


void shutdown_server(void)
{
	int i;

	plog("Shutting down.");

	/* Kick every player out and save his game */
	while(NumPlayers > 0)
	{
		/* Note the we always save the first player */
		player_type *p_ptr = Players[1];

		/* Indicate cause */
		strcpy(p_ptr->died_from, "server shutdown");
#if 1
//TODO: re-enable nicer messaging
		player_disconnect(p_ptr, "Server shutdown");
		player_leave(1);
#else
		/* Try to save */
		if (!player_leave(1)) player_disconnect(p_ptr, "Server shutdown (save failed)");

		/* Successful save */
		player_disconnect(p_ptr, "Server shutdown (save succeeded)");
#endif
	}

	/* Now wipe every object, to preserve artifacts on the ground */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Wipe this depth if no player on it */
		if (!players_on_depth[i]) wipe_o_list(i);
	}

	/* Save the server state */
	if (!save_server_info()) quit("Server state save failed!");

	/* Tell the metaserver that we're gone */
	report_to_meta_die();

	quit("Server state saved");
}

/*
 * Check if the given depth is special static level, i.e. a hand designed level.
 */
bool check_special_level(s16b special_depth)
{
	int i;
	for(i=0;i<MAX_SPECIAL_LEVELS;i++)
	{
		if(special_depth == special_levels[i]) return(TRUE);
	}
	return(FALSE);
}
