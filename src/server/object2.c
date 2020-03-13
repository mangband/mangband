/* File: object.c */ 

/* Purpose: misc code for objects */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"




void delete_object_ptr(object_type * o_ptr)
{
	/* special function to deal with inventory items being destoryed, instead of just wiping them */
/*
	int i,j;

	int y = o_ptr->iy;
	int x = o_ptr->ix;
	int Depth = o_ptr->dun_depth;
*/
	/* Wipe the object */
	WIPE(o_ptr, object_type);

}

/* 
 * Excise a dungeon object from any stacks 
 */ 
static void excise_object_idx(int o_idx)
{
	object_type *j_ptr;
	s16b this_o_idx, next_o_idx = 0;
	s16b prev_o_idx = 0;

	/* Object */
	j_ptr = &o_list[o_idx];

	/* Monster */
	if (j_ptr->held_m_idx)
	{
		monster_type *m_ptr;

		/* Monster */
		m_ptr = &m_list[j_ptr->held_m_idx];

		/* Scan all objects in the grid */
		for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Get the object */
			o_ptr = &o_list[this_o_idx];

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Done */
			if (this_o_idx == o_idx)
			{
				/* No previous */
				if (prev_o_idx == 0)
				{
					/* Remove from list */
					m_ptr->hold_o_idx = next_o_idx;
				}

				/* Real previous */
				else
				{
					object_type *i_ptr;

					/* Previous object */
					i_ptr = &o_list[prev_o_idx];

					/* Remove from list */
					i_ptr->next_o_idx = next_o_idx;
				}

				/* Forget next pointer */
				o_ptr->next_o_idx = 0;
				
				/* Done */
				break;
			}

			/* Save prev_o_idx */
			prev_o_idx = this_o_idx;
		}
	}

	/* Dungeon */
	else
	{
		int y = j_ptr->iy;
		int x = j_ptr->ix;
		int Depth = j_ptr->dun_depth;
		cave_type *c_ptr = &cave[Depth][y][x];
		int i;

		/* Object is gone */
		c_ptr->o_idx = 0;

		/* No one can see it anymore */
		for (i = 1; i <= NumPlayers; i++)
		{
			if (Players[i]->obj_vis[o_idx]) Players[i]->window |= (PW_ITEMLIST);
			Players[i]->obj_vis[o_idx] = FALSE;
		}
	}
}


static void object_wipe(object_type* o_ptr) 
{ 
	/* Wipe the object */
	WIPE(o_ptr, object_type);
}

/*
 * Prepare an object based on an object kind.
 */
void object_prep(object_type *o_ptr, int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

	/* Clear the record */
	(void)WIPE(o_ptr, object_type);

	/* Save the kind index */
	o_ptr->k_idx = k_idx;

	/* Efficiency -- tval/sval */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;

	/* Default "pval" */
	o_ptr->pval = k_ptr->pval;

	/* Default number */
	o_ptr->number = 1;

	/* Default weight */
	o_ptr->weight = k_ptr->weight;

	/* Default magic */
	o_ptr->to_h = k_ptr->to_h;
	o_ptr->to_d = k_ptr->to_d;
	o_ptr->to_a = k_ptr->to_a;

	/* Default power */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Default owner (none) */
	o_ptr->owner_id = 0;
	o_ptr->owner_name = 0;

	/* Hack -- worthless items are always "broken" */
	if (k_ptr->cost <= 0) o_ptr->ident |= (ID_BROKEN);

	/* Hack -- cursed items are always "cursed" */
	if (k_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (ID_CURSED);
}




/* 
 * Delete a dungeon object 
 */ 
void delete_object_idx(int o_idx)
{ 
	object_type *j_ptr;

	/* Excise */
	excise_object_idx(o_idx);
	
	/* Object */
	j_ptr = &o_list[o_idx];

	/* Dungeon floor */
	if (!j_ptr->held_m_idx)
	{
		int y, x, Depth;

		/* Location */
		y = j_ptr->iy;
		x = j_ptr->ix;
		Depth = j_ptr->dun_depth;

		/* Visual update */
		everyone_lite_spot(Depth, y, x);
	}

	/* Wipe the object */
	object_wipe(j_ptr);
}


/* 
 * Deletes object from given location 
 */ 
void delete_object(int Depth, int y, int x) 
{ 
	cave_type *c_ptr;

	/* Paranoia */ 
	if (!in_bounds(Depth, y, x)) return;

	/* Paranoia -- make sure the level has been allocated */ 
	if (!cave[Depth])
	{
		plog(format("Error : tried to delete object on unallocated level %d",Depth));
		return;
	}

	/* Find where it was */
	c_ptr = &cave[Depth][y][x];

	/* Delete the object */
	if (c_ptr->o_idx) delete_object_idx(c_ptr->o_idx);
} 


/* 
 * Move an object from index i1 to index i2 in the object list 
 */ 
static void compact_objects_aux(int i1, int i2) 
{ 
	int i, Ind;
	object_type *o_ptr;

	/* Do nothing */
	if (i1 == i2) return;

	/* Repair objects */
	for (i = 1; i < o_max; i++)
	{
		/* Get the object */
		o_ptr = &o_list[i];

		/* Skip "dead" objects */
		if (!o_ptr->k_idx) continue;

		/* Repair "next" pointers */
		if (o_ptr->next_o_idx == i1)
		{
			/* Repair */
			o_ptr->next_o_idx = i2;
		}
	}

	/* Get the object */
	o_ptr = &o_list[i1];
 
	/* Monster */
	if (o_ptr->held_m_idx)
	{
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = &m_list[o_ptr->held_m_idx];

		/* Repair monster */
		if (m_ptr->hold_o_idx == i1)
		{
			/* Repair */
			m_ptr->hold_o_idx = i2;
		}
	}

	/* Dungeon */
	else
	{
		int y, x, Depth;

		/* Get location */
		y = o_ptr->iy;
		x = o_ptr->ix;
		Depth = o_ptr->dun_depth;

		/* Repair grid */
		if (cave[Depth] && (cave[Depth][y][x].o_idx == i1))
		{
			/* Repair */
			cave[Depth][y][x].o_idx = i2;
		}
	}

	/* Copy the visibility flags for each player */
	for (Ind = 1; Ind <= NumPlayers; Ind++)
		Players[Ind]->obj_vis[i2] = Players[Ind]->obj_vis[i1];

	/* Hack -- move object */
	COPY(&o_list[i2], &o_list[i1], object_type);

	/* Hack -- wipe hole */
	object_wipe(o_ptr);
} 


/* 
 * Compact and reorder the object list 
 * 
 * This function can be very dangerous, use with caution! 
 * 
 * When compacting objects, we first destroy crops, junk and skeletons, on the basis that
 * those are the least important objects with almost no gameplay value (MAngband-specific).
 * 
 * When compacting other objects, we base the saving throw on a combination of 
 * object level, distance from player, and current "desperation". 
 * 
 * After compacting, we "reorder" the objects into a more compact order, and we 
 * reset the allocation info, and the "live" array. 
 */ 
void compact_objects(int size) 
{ 
	int i, y, x, cnt;
	int cur_lev, cur_val, chance;

	/* Reorder objects when not passed a size */
	if (!size)
	{
		/* Excise dead objects (backwards!) */
		for (i = o_max - 1; i >= 1; i--)
		{
			object_type *o_ptr = &o_list[i];

			/* Skip real objects */
			if (o_ptr->k_idx) continue;

			/* Move last object into open hole */
			compact_objects_aux(o_max - 1, i);

			/* Compress "o_max" */
			o_max--;
		}

		/* Reset "o_nxt" */
		o_nxt = o_max;

		/* Reset "o_top" */
		o_top = 0;

		/* Collect "live" objects */
		for (i = 1; i < o_max; i++)
		{
			/* Collect indexes */
			o_fast[o_top++] = i;
		}

		return;
	}

	/* Message */
	plog("Compacting objects...");

	/*** Try destroying objects ***/

	/* First do crops, junk and skeletons */
	for (i = 1; (i < o_max) && (size); i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Nuke crops */
		if (((o_ptr->tval == TV_FOOD) && (o_ptr->sval >= SV_FOOD_POTATO) && (o_ptr->sval < SV_FOOD_BISCUIT)) ||
		/* Nuke junk items and skeletons */
		((o_ptr->tval == TV_SKELETON) || (o_ptr->tval == TV_JUNK)))
		{
			delete_object_idx(i);
			size--;
		}
	}

	/* Compact at least 'size' objects */ 
	for (cnt = 1; size; cnt++) 
	{ 
		/* Get more vicious each iteration */ 
		cur_lev = 5 * cnt; 

		/* Destroy more valuable items each iteration */ 
		cur_val = 500 * (cnt - 1); 

		/* Examine the objects */ 
		for (i = 1; (i < o_max) && (size); i++) 
		{ 
			object_type *o_ptr = &o_list[i]; 
			object_kind *k_ptr = &k_info[o_ptr->k_idx]; 

			/* Skip dead objects */ 
			if (!o_ptr->k_idx) continue; 

			/* Hack -- High level objects start out "immune" */ 
			if (k_ptr->level > cur_lev) continue; 

			/* Valuable objects start out "immune" */ 
			if (object_value(NULL, o_ptr) > cur_val) continue; 

			/* Saving throw */ 
			chance = 90; 

			/* Monster */ 
			if (o_ptr->held_m_idx) 
			{ 
				monster_type *m_ptr; 

				/* Get the monster */ 
				m_ptr = &m_list[o_ptr->held_m_idx]; 

				/* Monsters protect their objects */ 
				//if (magik(90)) 
				continue; 
			}

			/* Dungeon */ 
			else
			{
				/* Get the location */ 
				y = o_ptr->iy; 
				x = o_ptr->ix; 

				/* Hack -- only compact items in houses in emergencies */ 
				/* Make sure that houses in the wilderness are dealt with too! */
				if ((o_ptr->dun_depth <= 0) && (cave[0][y][x].info & CAVE_ICKY)) 
				{ 
					/* Grant immunity except in emergencies */ 
					if (cnt < 1000) chance = 100; 
				} 
			}

			/* Hack -- only compact artifacts in emergencies */ 
			if (true_artifact_p(o_ptr) && (cnt < 1000)) chance = 100;
		
			/* Apply the saving throw */ 
			if (magik(chance)) continue; 

			/* Delete the object */ 
			delete_object_idx(i); 
			size--; 
		}
	} 

	/* Reorder objects */ 
	compact_objects(0); 
}



/*
 * Delete all the items when player leaves the level
 *
 * Note -- we do NOT visually reflect these (irrelevant) changes
 */
 

void wipe_o_list(int Depth)
{
	int i;//, house_depth;

	/* Delete the existing objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip objects not on this depth */
		if (o_ptr->dun_depth != Depth)
			continue;

		/* Mega-Hack -- preserve artifacts */
		/* Hack -- Preserve unknown artifacts */
		/* We now preserve ALL artifacts, known or not */
		if (true_artifact_p(o_ptr)/* && !object_known_p(o_ptr)*/)
		{
			/* Info */
			/* s_printf("Preserving artifact %d.\n", o_ptr->name1); */

			/* Mega-Hack -- Preserve the artifact */
			a_info[o_ptr->name1].cur_num = 0;

			/* Ultra-Hack -- If this artifact belongs to player, set abandoned */
			if (o_ptr->owner_id)
			{
				int j;
				for (j = 1; j <= NumPlayers; j++)
				{
					/* Only works when player is ingame */
					if ((Players[j]->id == o_ptr->owner_id) && object_known_p(Players[j], o_ptr))
					{
						set_artifact_p(Players[j], o_ptr->name1, ARTS_ABANDONED);
						break;
					}
				}
			}
		}

		/* Monster */
		if (o_ptr->held_m_idx)
		{
			monster_type *m_ptr;
			m_ptr = &m_list[o_ptr->held_m_idx];
			m_ptr->hold_o_idx = 0;			
		}

		/* Wipe the object */
		WIPE(o_ptr, object_type);
	}

	/* Compact the object list */
	compact_objects(0);
}


/*
 * Acquires and returns the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 *
 * Note that this function must maintain the special "o_fast"
 * array of pointers to "live" objects.
 */
s16b o_pop(void)
{
	int i, n, k;


	/* Initial allocation */
	if (o_max < MAX_O_IDX)
	{
		/* Get next space */
		i = o_max;

		/* Expand object array */
		o_max++;

		/* Update "o_fast" */
		o_fast[o_top++] = i;

		/* Use this object */
		return (i);
	}


	/* Check for some space */
	for (n = 1; n < MAX_O_IDX; n++)
	{
		/* Get next space */
		i = o_nxt;

		/* Advance (and wrap) the "next" pointer */
		if (++o_nxt >= MAX_O_IDX) o_nxt = 1;

		/* Skip objects in use */
		if (o_list[i].k_idx) continue;

		/* Verify space XXX XXX */
		if (o_top >= MAX_O_IDX) continue;

		/* Verify not allocated */
		for (k = 0; k < o_top; k++)
		{
			/* Hack -- Prevent errors */
			if (o_fast[k] == i) i = 0;
		}

		/* Oops XXX XXX */
		if (!i) continue;

		/* Update "o_fast" */
		o_fast[o_top++] = i;

		/* Use this object */
		return (i);
	}


	/* Warn the player */
	if (server_dungeon) plog("Too many objects!");

	/* Oops */
	return (0);
}



/*
 * Apply a "object restriction function" to the "object allocation table"
 */
errr get_obj_num_prep(void)
{
	int i;

	/* Get the entry */
	alloc_entry *table = alloc_kind_table;

	/* Scan the allocation table */
	for (i = 0; i < alloc_kind_size; i++)
	{
		/* Accept objects which pass the restriction, if any */
		if (!get_obj_num_hook || (*get_obj_num_hook)(table[i].index))
		{
			/* Accept this object */
			table[i].prob2 = table[i].prob1;
		}

		/* Do not use this object */
		else
		{
			/* Decline this object */
			table[i].prob2 = 0;
		}
	}

	/* Success */
	return (0);
}



/*
 * Choose an object kind that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "object allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" object, in
 * a relatively efficient manner.
 *
 * It is (slightly) more likely to acquire an object of the given level
 * than one of a lower level.  This is done by choosing several objects
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that if no objects are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
s16b get_obj_num(int level)
{
	int			i, j, p;

	int			k_idx;

	long		value, total;

	object_kind		*k_ptr;

	alloc_entry		*table = alloc_kind_table;


	/* Boost level */
	if (level > 0)
	{
		/* Occasional "boost" */
		if (randint0(GREAT_OBJ) == 0)
		{
			/* What a bizarre calculation */
			level = 1 + (level * MAX_DEPTH / randint1(MAX_DEPTH));
		}
	}


	/* Reset total */
	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc_kind_size; i++)
	{
		/* Objects are sorted by depth */
		if (table[i].level > level) break;

		/* Default */
		table[i].prob3 = 0;

		/* Access the index */
		k_idx = table[i].index;

		/* Access the actual kind */
		k_ptr = &k_info[k_idx];

		/* Hack -- prevent embedded chests */
		if (opening_chest && (k_ptr->tval == TV_CHEST)) continue;

		/* Accept */
		table[i].prob3 = table[i].prob2;

		/* Total */
		total += table[i].prob3;
	}

	/* No legal objects */
	if (total <= 0) return (0);


	/* Pick an object */
	value = randint0(total);

	/* Find the object */
	for (i = 0; i < alloc_kind_size; i++)
	{
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value = value - table[i].prob3;
	}


	/* Power boost */
	p = randint0(100);

	/* Try for a "better" object once (50%) or twice (10%) */
	if (p < 60)
	{
		/* Save old */
		j = i;

		/* Pick a object */
		value = randint0(total);

		/* Find the monster */
		for (i = 0; i < alloc_kind_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}

	/* Try for a "better" object twice (10%) */
	if (p < 10)
	{
		/* Save old */
		j = i;

		/* Pick a object */
		value = randint0(total);

		/* Find the object */
		for (i = 0; i < alloc_kind_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}


	/* Result */
	return (table[i].index);
}

#define ITEM_ACT_CASE_AIM \
	case ACT_FIRE3: \
	case ACT_FROST5: \
	case ACT_ELEC2: \
	case ACT_BIZZARE: \
	case ACT_MISSILE: \
	case ACT_FIRE1: \
	case ACT_FROST1: \
	case ACT_LIGHTNING_BOLT: \
	case ACT_ACID1: \
	case ACT_ARROW: \
	case ACT_STINKING_CLOUD: \
	case ACT_FROST2: \
	case ACT_FROST4: \
	case ACT_FROST3: \
	case ACT_FIRE2: \
	case ACT_DRAIN_LIFE2: \
	case ACT_STONE_TO_MUD: \
	case ACT_TELE_AWAY: \
	case ACT_CONFUSE: \
	case ACT_DRAIN_LIFE1: \
	case ACT_MANA_BOLT:

/* Returns TRUE if object *could be used* as fuel.
 * "fits" is set to TRUE, if it is to be used with
 * currently wielded light source. */
bool object_is_fuel(player_type *p_ptr, object_type *o_ptr, bool *fits)
{
	object_type *lamp_o_ptr;
	bool is_fuel = TRUE;
	bool has_torch = FALSE;
	bool has_lantern = FALSE;

	*fits = FALSE;

	/* See what a player is wielding. */
	lamp_o_ptr = &(p_ptr->inventory[INVEN_LITE]);
	if (lamp_o_ptr->tval == TV_LITE)
	{
		/* It's a lamp */
		if (lamp_o_ptr->sval == SV_LITE_LANTERN)
		{
			has_lantern = TRUE;
		}
		/* It's a torch */
		if (lamp_o_ptr->sval == SV_LITE_TORCH)
		{
			has_torch = TRUE;
		}
	}

	/* item_tester_refill_lantern: */
	if (!o_ptr->name3 &&
	     (o_ptr->tval == TV_FLASK ||
	     (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_LANTERN)))
	{
		is_fuel = TRUE;
		if (has_lantern) *fits = TRUE;
	}
	/* item_tester_refill_torch: */
	else if (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_TORCH)
	{
		is_fuel = TRUE;
		if (has_torch) *fits = TRUE;
	}
	return is_fuel;
}

/*
 * This function determines primary item tester and use flag for a given object.
 * It uses hardcoded TVALs/SVALs, similarly to how all our code does.
 * Note: some items have a *secondary* tester, which will also be determined
 * and stored in the "secondary_tester" pointer.
 */
byte object_tester_flag(player_type *p_ptr, object_type *o_ptr, byte *secondary_tester)
{
	byte flag = 0;
	u32b f1, f2, f3;
	bool relevant_fuel = FALSE;
	
	bool aware = object_aware_p(p_ptr, o_ptr);
	bool tried = aware || object_tried_p(p_ptr, o_ptr);
	*secondary_tester = 0;
	
	/* item_tester_hook_wear: */
	if (wield_slot(p_ptr, o_ptr) >= INVEN_WIELD)
	{
		flag |= ITF_WEAR;
	}
	
	/* item_tester_hook_activate: */
	if (object_known_p(p_ptr, o_ptr))
	{
		object_flags(o_ptr, &f1, &f2, &f3);
		if (f3 & TR3_ACTIVATE)
		{
			flag |= ITF_ACT;

			/* Hack: ask for direction ? (FOR ACTIVATION) */
			/* Artifacts (use-obj.c) */
			if (true_artifact_p(o_ptr))
				switch(a_info[o_ptr->name1].activation)
				{
					ITEM_ACT_CASE_AIM flag |= ITEM_ASK_AIM; break;
					default: break;
				}
			/* Hack -- Dragon Scale Mail can be activated as well (use-obj.c) */
			if (o_ptr->tval == TV_DRAG_ARMOR) flag |= ITEM_ASK_AIM;
			/* Hack -- some Rings can be activated for double resist and element ball (use-obj.c) */
			if (o_ptr->tval == TV_RING) flag |= ITEM_ASK_AIM;
		}
	}
	
	/* ask for direction ? (FOR RODS) */
	if (o_ptr->tval == TV_ROD)
	{
		/* Hack -- KNOWN Rod of Perception asks for item */
		if ((o_ptr->sval == SV_ROD_IDENTIFY) && aware)
		{
			flag |= ITEM_ASK_ITEM;
		}
		else
		/* Get a direction (unless KNOWN not to need it) */
		if ((o_ptr->sval >= SV_ROD_MIN_DIRECTION) || !tried)
		{
			flag |= ITEM_ASK_AIM;
		}
	}

	/* ask for another item? (FOR STAFFS) */
	if (o_ptr->tval == TV_STAFF)
	{
		/* Get an item (unless KNOWN not to need it) */
		if ((o_ptr->sval >= SV_STAFF_IDENTIFY && o_ptr->sval <= SV_STAFF_REMOVE_CURSE)
		|| !tried)
		{
			flag |= ITEM_ASK_ITEM;
		}
		/* Specify secondary tester (if player KNOWS what that spell is) */
		if ((flag & ITEM_ASK_ITEM) && aware)
		{
			switch (o_ptr->sval)
			{
				case SV_STAFF_REMOVE_CURSE:
					*secondary_tester = item_test(WEAR);
				default: break;
			}
		}
	}

	/* ask for another item? (Id, Enchant, Curse, ...) */
	if (o_ptr->tval == TV_SCROLL)
	{
		/* Get a second item (unless KNOWN not to need it) */
		if (o_ptr->sval == SV_SCROLL_CREATE_ARTIFACT
		|| (o_ptr->sval >= SV_SCROLL_IDENTIFY && o_ptr->sval <= SV_SCROLL_RECHARGING)
		|| !tried)
		{
			flag |= ITEM_ASK_ITEM;
		}
		/* Specify secondary tester (if player KNOWS what that spell is) */
		if ((flag & ITEM_ASK_ITEM) && aware)
		{
			switch (o_ptr->sval)
			{
				case SV_SCROLL_CURSE_WEAPON:
				case SV_SCROLL_STAR_ENCHANT_WEAPON:
				case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
				case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
					*secondary_tester = item_test(WEAPON);
				break;
				case SV_SCROLL_CURSE_ARMOR:
				case SV_SCROLL_ENCHANT_ARMOR:
				case SV_SCROLL_STAR_ENCHANT_ARMOR:
					*secondary_tester = item_test(ARMOR);
				break;
				case SV_SCROLL_RECHARGING:
					*secondary_tester = item_test(RECHARGE);
				break;
				default: break;
			}
		}
	}
	
	/* refill light ? */
	if (object_is_fuel(p_ptr, o_ptr, &relevant_fuel))
	{
		/* torch matching torch or flask matching lantern */
		if (relevant_fuel)
		{
			flag |= ITF_FUEL;
		}
	}
	
	/* Return Flag */
	return flag;
}




/*
 * Known is true when the "attributes" of an object are "known".
 * These include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more.  In fact, the player is always "aware" of any
 * item of which he has full "knowledge".
 *
 * But having full knowledge of, say, one "wand of wonder", does not, by
 * itself, give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as fully "know" it.
 *
 * This routine also removes any inscriptions generated by "feelings".
 */
void object_known(object_type *o_ptr)
{
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

	/* Clear the "Felt" info */
	o_ptr->ident &= ~ID_SENSE;

	/* Clear the "Empty" info */
	o_ptr->ident &= ~ID_EMPTY;

	/* Now we know about the item */
	o_ptr->ident |= ID_KNOWN;
}





/*
 * The player is now aware of the effects of the given object.
 */
void object_aware(player_type *p_ptr, object_type *o_ptr)
{
	int item;
	int _tmp;
	object_type *j_ptr;
	
	/* Fully aware of the effects */
	p_ptr->kind_aware[o_ptr->k_idx] = TRUE;
	
	/* Update resistant flags */
	p_ptr->redraw |= PR_OFLAGS;
	
	/* Update all similar objects */
	for (item = 0; item < INVEN_TOTAL; item++)
	{
		object_type *j_ptr = &p_ptr->inventory[item];
		if (o_ptr->k_idx == j_ptr->k_idx)
		{
			player_redraw_item(p_ptr, item);
		}
	}
	/* And the floor object */
	if ((j_ptr = player_get_floor_item(p_ptr, &_tmp)))
	{
		if (o_ptr->k_idx == j_ptr->k_idx)
		{
			player_redraw_item(p_ptr, FLOOR_INDEX);
		}
	}
}



/*
 * Something has been "sampled"
 */
void object_tried(player_type *p_ptr, object_type *o_ptr)
{
	/* Mark it as tried (even if "aware") */
	p_ptr->kind_tried[o_ptr->k_idx] = TRUE;
}



/*
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b object_value_base(player_type *p_ptr, object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Aware item -- use template cost */
	if (p_ptr == NULL || object_aware_p(p_ptr, o_ptr)) return (k_ptr->cost);

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Un-aware Food */
		case TV_FOOD: return (5L);

		/* Un-aware Potions */
		case TV_POTION: return (20L);

		/* Un-aware Scrolls */
		case TV_SCROLL: return (20L);

		/* Un-aware Staffs */
		case TV_STAFF: return (70L);

		/* Un-aware Wands */
		case TV_WAND: return (50L);

		/* Un-aware Rods */
		case TV_ROD: return (90L);

		/* Un-aware Rings */
		case TV_RING: return (45L);

		/* Un-aware Amulets */
		case TV_AMULET: return (45L);
	}

	/* Paranoia -- Oops */
	return (0L);
}


/*
 * Return the "real" price of a "known" item, not including discounts
 *
 * Wand and staffs get cost for each charge
 *
 * Armor is worth an extra 100 gold per bonus point to armor class.
 *
 * Weapons are worth an extra 100 gold per bonus point (AC,TH,TD).
 *
 * Missiles are only worth 5 gold per bonus point, since they
 * usually appear in groups of 20, and we want the player to get
 * the same amount of cash for any "equivalent" item.  Note that
 * missiles never have any of the "pval" flags, and in fact, they
 * only have a few of the available flags, primarily of the "slay"
 * and "brand" and "ignore" variety.
 *
 * Armor with a negative armor bonus is worthless.
 * Weapons with negative hit+damage bonuses are worthless.
 *
 * Every wearable item with a "pval" bonus is worth extra (see below).
 */
static s32b object_value_real(object_type *o_ptr)
{
	s32b value;

        u32b f1, f2, f3;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];


	/* Hack -- "worthless" items */
	if (!k_ptr->cost) return (0L);

	/* Base cost */
	value = k_ptr->cost;


	/* Extract some flags */
    object_flags(o_ptr, &f1, &f2, &f3);


	/* Artifact */
    if (artifact_p(o_ptr))
    {
	artifact_type *a_ptr = artifact_ptr(o_ptr);

		/* Hack -- "worthless" artifacts */
		if (!a_ptr->cost) return (0L);

		/* Hack -- Use the artifact cost instead */
		value = a_ptr->cost;
	}

	/* Ego-Item */
	else if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		/* Hack -- "worthless" ego-items */
		if (!e_ptr->cost) return (0L);

		/* Hack -- Reward the ego-item with a bonus */
		value += e_ptr->cost;
	}


	/* Analyze pval bonus */
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
		case TV_LITE:
		case TV_AMULET:
		case TV_RING:
		{
			/* Hack -- Negative "pval" is always bad */
			if (o_ptr->pval < 0) return (0L);

			/* No pval */
			if (!o_ptr->pval) break;

			/* Give credit for stat bonuses */
			if (f1 & TR1_STR) value += (o_ptr->pval * 200L);
			if (f1 & TR1_INT) value += (o_ptr->pval * 200L);
			if (f1 & TR1_WIS) value += (o_ptr->pval * 200L);
			if (f1 & TR1_DEX) value += (o_ptr->pval * 200L);
			if (f1 & TR1_CON) value += (o_ptr->pval * 200L);
			if (f1 & TR1_CHR) value += (o_ptr->pval * 200L);

			/* Give credit for stealth and searching */
			if (f1 & TR1_STEALTH) value += (o_ptr->pval * 100L);
			if (f1 & TR1_SEARCH) value += (o_ptr->pval * 100L);

			/* Give credit for infra-vision and tunneling */
			if (f1 & TR1_INFRA) value += (o_ptr->pval * 50L);
			if (f1 & TR1_TUNNEL) value += (o_ptr->pval * 50L);

			/* Give credit for extra attacks */
			if (f1 & TR1_BLOWS) value += (o_ptr->pval * 2000L);

			/* Hack -- amulets of speed and rings of speed are
             * cheaper than other items of speed.
			 */
			if (o_ptr->tval == TV_AMULET)
			{
				/* Give credit for speed bonus */
				if (f1 & TR1_SPEED) value += (o_ptr->pval * 25000L);
			}
			else if (o_ptr->tval == TV_RING)
			{
				/* Give credit for speed bonus */
				if (f1 & TR1_SPEED) value += (o_ptr->pval * 50000L);
			}
			else if (f1 & TR1_SPEED) value += (o_ptr->pval * 100000L);
			break;
		}
	}


	/* Analyze the item */
	switch (o_ptr->tval)
	{
		/* Wands/Staffs */
		case TV_WAND:
		case TV_STAFF:
		{
			/* Pay extra for charges */
			value += ((value / 20) * (o_ptr->pval / o_ptr->number));

			/* Done */
			break;
		}

		/* Rings/Amulets */
		case TV_RING:
		case TV_AMULET:
		{
			/* Hack -- negative bonuses are bad */
			if (o_ptr->to_a < 0) return (0L);
			if (o_ptr->to_h < 0) return (0L);
			if (o_ptr->to_d < 0) return (0L);

			/* Give credit for bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Done */
			break;
		}

		/* Armor */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			/* Hack -- negative armor bonus */
			if (o_ptr->to_a < 0) return (0L);

			/* Give credit for bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Done */
			break;
		}

		/* Bows/Weapons */
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_SWORD:
		case TV_POLEARM:
		{
			/* Hack -- negative hit/damage bonuses */
			if (o_ptr->to_h + o_ptr->to_d < 0) return (0L);

			/* Factor in the bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Hack -- Factor in extra damage dice */
			if ((o_ptr->dd > k_ptr->dd) && (o_ptr->ds == k_ptr->ds))
			{
				value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 100L;
			}

			/* Done */
			break;
		}

		/* Ammo */
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			/* Hack -- negative hit/damage bonuses */
			if (o_ptr->to_h + o_ptr->to_d < 0) return (0L);

			/* Factor in the bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d) * 5L);

			/* Hack -- Factor in extra damage dice */
			if ((o_ptr->dd > k_ptr->dd) && (o_ptr->ds == k_ptr->ds))
			{
				value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 5L;
			}

			/* Done */
			break;
		}
	}


	/* Return the value */
	return (value);
}


/*
 * Return the price of an item including plusses (and charges)
 *
 * This function returns the "value" of the given item (qty one)
 *
 * Never notice "unknown" bonuses or properties, including "curses",
 * since that would give the player information he did not have.
 *
 * Note that discounted items stay discounted forever, even if
 * the discount is "forgotten" by the player via memory loss.
 */
s32b object_value(player_type *p_ptr, object_type *o_ptr)
{
	s32b value;


	/* Unknown items -- acquire a base value */
	if (p_ptr == NULL || object_known_p(p_ptr, o_ptr))
	{
		/* Broken items -- worthless */
		if (broken_p(o_ptr)) return (0L);

		/* Cursed items -- worthless */
		if (cursed_p(o_ptr)) return (0L);

		/* Real value (see above) */
		value = object_value_real(o_ptr);
	}

	/* Known items -- acquire the actual value */
	else
	{
		/* Hack -- Felt broken items */
		if ((o_ptr->ident & ID_SENSE) && broken_p(o_ptr)) return (0L);

		/* Hack -- Felt cursed items */
		if ((o_ptr->ident & ID_SENSE) && cursed_p(o_ptr)) return (0L);

		/* Base value (see above) */
		value = object_value_base(p_ptr, o_ptr);
	}


	/* Apply discount (if any) */
	if (o_ptr->discount) value -= (value * o_ptr->discount / 100L);


	/* Return the final value */
	return (value);
}


/*
 * Determine if an item can "absorb" a second item on the floor -- MAngband-specific Hack
 *
 * It is very similar from the object_similar function, minus the player specific-checks.
 * It denies most attempts, but is allows discounts & inscription stack.
 * 
 */
bool object_similar_floor(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;


	/* Require identical object types */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);


	/* Analyze the items */
	switch (o_ptr->tval)
	{
		/* Chests */
		case TV_CHEST:
		{
			/* Never okay */
			return (0);
		}

		/* Junk */
		case TV_JUNK:
		{
			/* Assume okay */
			break;
		}

		/* Food and Potions and Scrolls */
		case TV_FOOD:
		case TV_POTION:
		case TV_SCROLL:
		{
			/* Assume okay */
			break;
		}

		/* Staffs and Wands */
		case TV_STAFF:
		case TV_WAND:
		{
			/* Require both to be empty or identified */
			if (!(o_ptr->ident & (ID_EMPTY)) &&
			     !(o_ptr->ident & (ID_KNOWN)) ||
			   (!(j_ptr->ident & (ID_EMPTY)) &&
			    !(o_ptr->ident & (ID_KNOWN)))) return(0);

			/* Assume ok */
			break;
		}

		/* Rods */
		case TV_ROD:
		{
			/* Assume ok */
			break;
		}

		/* Weapons and Armor */
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
			/* hack -- MAngband-specific bonus */
			if (o_ptr->bpval != j_ptr->bpval) return (0);

			/* Fallthrough */
		}

		/* Rings, Amulets, Lites */
		case TV_RING:
		case TV_AMULET:
		case TV_LITE:
		{
			/* Fallthrough */
		}

		/* Missiles */
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			/* Require identical knowledge of both items */
			if ((o_ptr->ident & ID_KNOWN) != (j_ptr->ident & ID_KNOWN)) return (0);

			/* Require identical "bonuses" */
			if (o_ptr->to_h != j_ptr->to_h) return (FALSE);
			if (o_ptr->to_d != j_ptr->to_d) return (FALSE);
			if (o_ptr->to_a != j_ptr->to_a) return (FALSE);

			/* Require identical "pval" code */
			if (o_ptr->pval != j_ptr->pval) return (FALSE);

			/* Require identical "artifact" names */
			if (o_ptr->name1 != j_ptr->name1) return (FALSE);

			/* Require identical "ego-item" names */
			if (o_ptr->name2 != j_ptr->name2) return (FALSE);

		        /* Require identical "random artifact" names */
	                if (o_ptr->name3 != j_ptr->name3) return (FALSE);

			/* Hack -- Never stack "powerful" items */
			if (o_ptr->xtra1 || j_ptr->xtra1) return (FALSE);

			/* Hack -- Never stack recharging items */
			if ((o_ptr->timeout || j_ptr->timeout) && o_ptr->tval != TV_LITE)
			    return (FALSE);

			/* Lites must have same amount of fuel */
			else if(o_ptr->timeout != j_ptr->timeout && o_ptr->tval == TV_LITE)
				return FALSE;

			/* Require identical "values" */
			if (o_ptr->ac != j_ptr->ac) return (FALSE);
			if (o_ptr->dd != j_ptr->dd) return (FALSE);
			if (o_ptr->ds != j_ptr->ds) return (FALSE);

			/* Probably okay */
			break;
		}

		/* Magic books */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			/* Assume okay */
			break;
		}

		/* Various */
		default:
		{
			/* Never okay -- why?*/
			return(0);
		}
	}


	/* Hack -- Require identical "cursed" status */
	if ((o_ptr->ident & ID_CURSED) != (j_ptr->ident & ID_CURSED)) return (0);

	/* Hack -- Require identical "broken" status */
	if ((o_ptr->ident & ID_BROKEN) != (j_ptr->ident & ID_BROKEN)) return (0);

	/* Hack -- require semi-matching "inscriptions" */
	if (o_ptr->note && j_ptr->note && (o_ptr->note != j_ptr->note)) return (0);

	/* Hack -- normally require matching "inscriptions"
	if ((o_ptr->note != j_ptr->note)) return (0);*/

	/* Hack -- normally require matching "discounts"
	if ((o_ptr->discount != j_ptr->discount)) return (0);*/

	/* Maximal "stacking" limit */
	if (total >= MAX_STACK_SIZE) return (0);

	/* They match, so they must be similar */
	return (TRUE);
}



/*
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow wands/staffs (if they are known to have equal
 * charges) and rods (if fully charged) to combine.
 *
 * Note that rods/staffs/wands are then unstacked when they are used.
 *
 * If permitted, we allow weapons/armor to stack, if they both known.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests never stack (for various reasons).
 *
 * We do NOT allow activatable items (artifacts or dragon scale mail)
 * to stack, to keep the "activation" code clean.  Artifacts may stack,
 * but only with another identical artifact (which does not exist).
 *
 * Ego items may stack as long as they have the same ego-item type.
 * This is primarily to allow ego-missiles to stack.
 */
bool object_similar(const player_type *p_ptr, const object_type *o_ptr, const object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;


	/* Require identical object types */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);


	/* Analyze the items */
	switch (o_ptr->tval)
	{
		/* Chests */
		case TV_CHEST:
		{
			/* Never okay */
			return (0);
		}

		/* Food and Potions and Scrolls */
		case TV_FOOD:
		case TV_POTION:
		case TV_SCROLL:
		{
			/* Assume okay */
			break;
		}

		/* Staffs and Wands */
		case TV_STAFF:
		case TV_WAND:
		{
			/* Require permission */
			if (!option_p(p_ptr,STACK_ALLOW_WANDS)) return (0);

			/* Require either knowledge or known empty for both wands/staves */
			if ((!(o_ptr->ident & (ID_EMPTY)) &&
				!object_known_p(p_ptr, o_ptr)) ||
				(!(j_ptr->ident & (ID_EMPTY)) &&
				!object_known_p(p_ptr, j_ptr))) return(0);

			/* Assume okay */
			break;
		}

		/* Staffs and Wands and Rods */
		case TV_ROD:
		{
			/* Require permission */
			if (!option_p(p_ptr,STACK_ALLOW_WANDS)) return (0);
#if 0
			/* Require identical charges */
			if (o_ptr->pval != j_ptr->pval)
			{
				/* Check to make sure they aren't stacked 
				   in the wrong order */
				p_ptr->notice |= (PN_REORDER);
				return (0);
			}
			/* Fall throu */
#endif
			/* Probably okay */
			break;
		}

		/* Weapons and Armor */
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
			/* Require permission */
			if (!option_p(p_ptr,STACK_ALLOW_ITEMS)) return (0);

			/* XXX XXX XXX Require identical "sense" status */
			/* if ((o_ptr->ident & ID_SENSE) != */
			/*     (j_ptr->ident & ID_SENSE)) return (0); */

			/* require identical bonuses */
			/* hack -- MAngband-specific bonus */
			if (o_ptr->bpval != j_ptr->bpval) return (0);

			/* Fall through */
		}

		/* Rings, Amulets, Lites */
		case TV_RING:
		case TV_AMULET:
		case TV_LITE:
		{
			/* Require full knowledge of both items */
            if (!object_known_p(p_ptr, o_ptr) || !object_known_p(p_ptr, j_ptr) || (o_ptr->name3)) return (0);

			/* Fall through */
		}

		/* Missiles */
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			/* Require identical knowledge of both items */
			if (object_known_p(p_ptr,o_ptr) != object_known_p(p_ptr,j_ptr)) return (FALSE);
		
			/* Require identical "bonuses" */
			if (o_ptr->to_h != j_ptr->to_h) return (FALSE);
			if (o_ptr->to_d != j_ptr->to_d) return (FALSE);
			if (o_ptr->to_a != j_ptr->to_a) return (FALSE);

			/* Require identical "pval" code */
			if (o_ptr->pval != j_ptr->pval) return (FALSE);

			/* Require identical "artifact" names */
			if (o_ptr->name1 != j_ptr->name1) return (FALSE);

			/* Require identical "ego-item" names */
			if (o_ptr->name2 != j_ptr->name2) return (FALSE);

		        /* Require identical "random artifact" names */
	                if (o_ptr->name3 != j_ptr->name3) return (FALSE);

			/* Hack -- Never stack "powerful" items */
			if (o_ptr->xtra1 || j_ptr->xtra1) return (FALSE);

			/* Hack -- Never stack recharging items */
			if (o_ptr->timeout || j_ptr->timeout) return (FALSE);

			/* Require identical "values" */
			if (o_ptr->ac != j_ptr->ac) return (FALSE);
			if (o_ptr->dd != j_ptr->dd) return (FALSE);
			if (o_ptr->ds != j_ptr->ds) return (FALSE);

			/* Probably okay */
			break;
		}

		/* Various */
		default:
		{
			/* Require knowledge */
			if (!object_known_p(p_ptr, o_ptr) || !object_known_p(p_ptr, j_ptr)) return (0);

			/* Probably okay */
			break;
		}
	}


	/* Hack -- Require identical "cursed" status */
	if ((o_ptr->ident & ID_CURSED) != (j_ptr->ident & ID_CURSED)) return (0);

	/* Hack -- Require identical "broken" status */
	if ((o_ptr->ident & ID_BROKEN) != (j_ptr->ident & ID_BROKEN)) return (0);


	/* Hack -- require semi-matching "inscriptions" */
	if (o_ptr->note && j_ptr->note && (o_ptr->note != j_ptr->note)) return (0);

	/* Hack -- normally require matching "inscriptions" */
	if (!option_p(p_ptr,STACK_FORCE_NOTES) && (o_ptr->note != j_ptr->note)) return (0);

	/* Hack -- normally require matching "discounts" */
	if (!option_p(p_ptr,STACK_FORCE_COSTS) && (o_ptr->discount != j_ptr->discount)) return (0);


	/* Maximal "stacking" limit */
	if (total >= MAX_STACK_SIZE) return (0);


	/* They match, so they must be similar */
	return (TRUE);
}


/*
 * Allow one item to "absorb" another, assuming they are similar
 *
 * Player pointer "p_ptr" is allowed to be NULL, when it is not
 * NULL, "object known" statuses are also blended. 
 */
void object_absorb(player_type *p_ptr, object_type *o_ptr, object_type *j_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = ((total < MAX_STACK_SIZE) ? total : (MAX_STACK_SIZE - 1));

	/* Hack -- blend "known" status */
	if (p_ptr && object_known_p(p_ptr, j_ptr)) object_known(o_ptr);

	/* Hack -- blend "rumour" status */
	if (j_ptr->ident & ID_RUMOUR) o_ptr->ident |= ID_RUMOUR;

	/* Hack -- blend "mental" status */
	if (j_ptr->ident & ID_MENTAL) o_ptr->ident |= ID_MENTAL;

	/* Hack -- blend "inscriptions" */
	if (j_ptr->note) o_ptr->note = j_ptr->note;

	/* Hack -- could average discounts XXX XXX XXX */
	/* Hack -- save largest discount XXX XXX XXX */
	if (o_ptr->discount < j_ptr->discount) o_ptr->discount = j_ptr->discount;
	
	/*
	 * Hack -- if rods are stacking, re-calculate the
	 * pvals (maximum timeouts) and current timeouts together
	 */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->pval = total * k_ptr->pval;
		o_ptr->timeout += j_ptr->timeout;
	}

	/* Hack -- if wands or staves are stacking, combine the charges */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
	{
		o_ptr->pval += j_ptr->pval;
	}

	/* Forget original owner */
	if (o_ptr->origin_player != j_ptr->origin_player)
	{
		o_ptr->origin_player = 0;
	}

	/* Merge origins */
	if ((o_ptr->origin != j_ptr->origin) ||
	    (o_ptr->origin_depth != j_ptr->origin_depth) ||
	    (o_ptr->origin_xtra != j_ptr->origin_xtra))
	{
		int act = 2;

		if ((o_ptr->origin == ORIGIN_DROP) && (o_ptr->origin == j_ptr->origin))
		{
			monster_race *r_ptr = &r_info[o_ptr->origin_xtra];
			monster_race *s_ptr = &r_info[j_ptr->origin_xtra];

			bool r_uniq = r_ptr->flags1 & RF1_UNIQUE ? TRUE : FALSE;
			bool s_uniq = s_ptr->flags1 & RF1_UNIQUE ? TRUE : FALSE;

			if (r_uniq && !s_uniq) act = 0;
			else if (s_uniq && !r_uniq) act = 1;
			else act = 2;
		}

		switch (act)
		{
			/* Overwrite with j_ptr */
			case 1:
			{
				o_ptr->origin = j_ptr->origin;
				o_ptr->origin_depth = j_ptr->origin_depth;
				o_ptr->origin_xtra = j_ptr->origin_xtra;
			}

			/* Set as "mixed" */
			case 2:
			{
				o_ptr->origin = ORIGIN_MIXED;
			}
		}
	}
}

/*
 * Get a random object kind, by tval.
 * Returns 0 if nothing was found.
 */
u16b rand_tval_kind(int tval)
{
	int k_idx, kinds = 0;
	int tbl[MAX_FLVR_IDX];

	for (k_idx = 0; k_idx < z_info->k_max; k_idx++)
	{
		object_kind *k_ptr = &k_info[k_idx];
		if (k_ptr->tval != tval) continue;
		tbl[kinds++] = k_idx;
		if (kinds >= MAX_FLVR_IDX) break;
	}
	/* Nothing found, give up */
	if (!kinds) return 0;

	/* Randomly */
	return tbl[randint0(kinds)];
}

/*
 * Find the index of the object_kind with the given tval and sval
 */
s16b lookup_kind(int tval, int sval)
{
	int k;

	/* Look for it */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Found a match */
		if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
	}
#ifdef DEBUG
	/* Oops */
	plog(format("No object (%d,%d)", tval, sval));
#endif
	/* Oops */
	return (0);
}


/*
 * Clear an item
 */
void invwipe(object_type *o_ptr)
{
	/* Clear the record */
	WIPE(o_ptr, object_type);
}


/*
 * Make "o_ptr" a "clean" copy of the given "kind" of object
 */
void invcopy(object_type *o_ptr, int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

	/* Clear the record */
	WIPE(o_ptr, object_type);

	/* Save the kind index */
	o_ptr->k_idx = k_idx;

	/* Efficiency -- tval/sval */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;

	/* Default "pval" */
	o_ptr->pval = k_ptr->pval;

	/* Default number */
	o_ptr->number = 1;

	/* Default weight */
	o_ptr->weight = k_ptr->weight;

	/* Default magic */
	o_ptr->to_h = k_ptr->to_h;
	o_ptr->to_d = k_ptr->to_d;
	o_ptr->to_a = k_ptr->to_a;

	/* Default power */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Hack -- worthless items are always "broken" */
	if (k_ptr->cost <= 0) o_ptr->ident |= ID_BROKEN;

	/* Hack -- cursed items are always "cursed" */ 
	if ((k_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(k_ptr->flags3 & TR3_HEAVY_CURSE) ||
 		 		(k_ptr->flags3 & TR3_PERMA_CURSE))
					o_ptr->ident |= ID_CURSED;
}





/*
 * Help determine an "enchantment bonus" for an object.
 *
 * To avoid floating point but still provide a smooth distribution of bonuses,
 * we simply round the results of division in such a way as to "average" the
 * correct floating point value.
 *
 * This function has been changed.  It uses "randnor()" to choose values from
 * a normal distribution, whose mean moves from zero towards the max as the
 * level increases, and whose standard deviation is equal to 1/4 of the max,
 * and whose values are forced to lie between zero and the max, inclusive.
 *
 * Since the "level" rarely passes 100 before Morgoth is dead, it is very
 * rare to get the "full" enchantment on an object, even a deep levels.
 *
 * It is always possible (albeit unlikely) to get the "full" enchantment.
 *
 * A sample distribution of values from "m_bonus(10, N)" is shown below:
 *
 *   N       0     1     2     3     4     5     6     7     8     9    10
 * ---    ----  ----  ----  ----  ----  ----  ----  ----  ----  ----  ----
 *   0   66.37 13.01  9.73  5.47  2.89  1.31  0.72  0.26  0.12  0.09  0.03
 *   8   46.85 24.66 12.13  8.13  4.20  2.30  1.05  0.36  0.19  0.08  0.05
 *  16   30.12 27.62 18.52 10.52  6.34  3.52  1.95  0.90  0.31  0.15  0.05
 *  24   22.44 15.62 30.14 12.92  8.55  5.30  2.39  1.63  0.62  0.28  0.11
 *  32   16.23 11.43 23.01 22.31 11.19  7.18  4.46  2.13  1.20  0.45  0.41
 *  40   10.76  8.91 12.80 29.51 16.00  9.69  5.90  3.43  1.47  0.88  0.65
 *  48    7.28  6.81 10.51 18.27 27.57 11.76  7.85  4.99  2.80  1.22  0.94
 *  56    4.41  4.73  8.52 11.96 24.94 19.78 11.06  7.18  3.68  1.96  1.78
 *  64    2.81  3.07  5.65  9.17 13.01 31.57 13.70  9.30  6.04  3.04  2.64
 *  72    1.87  1.99  3.68  7.15 10.56 20.24 25.78 12.17  7.52  4.42  4.62
 *  80    1.02  1.23  2.78  4.75  8.37 12.04 27.61 18.07 10.28  6.52  7.33
 *  88    0.70  0.57  1.56  3.12  6.34 10.06 15.76 30.46 12.58  8.47 10.38
 *  96    0.27  0.60  1.25  2.28  4.30  7.60 10.77 22.52 22.51 11.37 16.53
 * 104    0.22  0.42  0.77  1.36  2.62  5.33  8.93 13.05 29.54 15.23 22.53
 * 112    0.15  0.20  0.56  0.87  2.00  3.83  6.86 10.06 17.89 27.31 30.27
 * 120    0.03  0.11  0.31  0.46  1.31  2.48  4.60  7.78 11.67 25.53 45.72
 * 128    0.02  0.01  0.13  0.33  0.83  1.41  3.24  6.17  9.57 14.22 64.07
 */
static s16b m_bonus(int max, int level)
{
	int bonus, stand, extra, value;


	/* Paranoia -- enforce maximal "level" */
	if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;


	/* The "bonus" moves towards the max */
	bonus = ((max * level) / MAX_DEPTH);

	/* Hack -- determine fraction of error */
	extra = ((max * level) % MAX_DEPTH);

	/* Hack -- simulate floating point computations */
	if (randint0(MAX_DEPTH) < extra) bonus++;


	/* The "stand" is equal to one quarter of the max */
	stand = (max / 4);

	/* Hack -- determine fraction of error */
	extra = (max % 4);

	/* Hack -- simulate floating point computations */
	if (randint0(4) < extra) stand++;


	/* Choose an "interesting" value */
	value = randnor(bonus, stand);

	/* Enforce the minimum value */
	if (value < 0) return (0);

	/* Enforce the maximum value */
	if (value > max) return (max);

	/* Result */
	return (value);
}



#if 0

/*
 * Cheat -- describe a created object for the user
 */
static void object_mention(object_type *o_ptr)
{
	char o_name[80];

	/* Describe */
		object_desc_store(o_name, o_ptr, FALSE, 0);

	/* Artifact */
	if (true_artifact_p(o_ptr))
	{
		/* Silly message */
		msg_format("Artifact (%s)", o_name);
	}

	/* Randart */
	else if (artifact_p(o_ptr))
	{
		/* Silly message */
		msg_format("Randart (%s)", o_name);
	}

	/* Ego-item */
	else if (ego_item_p(o_ptr))
	{
		/* Silly message */
		msg_format("Ego-item (%s)", o_name);
	}

	/* Normal item */
	else
	{
		/* Silly message */
		msg_format("Object (%s)", o_name);
	}
}

#endif



/*
 * Mega-Hack -- Attempt to create one of the "Special Objects"
 *
 * We are only called from "place_object()", and we assume that
 * "apply_magic()" is called immediately after we return.
 *
 * Note -- see "make_artifact()" and "apply_magic()"
 */
static bool make_artifact_special(int Depth, object_type *o_ptr)
{
	int			i;
	int			k_idx = 0;
	char o_name[80];

	/* No artifacts in the town */
	if (!Depth) return (FALSE);

	/* Check the artifact list (just the "specials") */
	for (i = 0; i < ART_MIN_NORMAL; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > Depth)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (a_ptr->level - Depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* XXX XXX Enforce maximum "depth" (loosely) */
		if (Depth > (a_ptr->level +20))
		{
			/* Acquire the "out-of-depth factor" */
			int d = (a_ptr->level - Depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Artifact "rarity roll" */
	if (randint0(a_ptr->rarity) != 0) continue;

		/* Find the base object */
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* XXX XXX Enforce minimum "object" level (loosely) */
		if (k_info[k_idx].level > object_level)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (k_info[k_idx].level - object_level) * 5;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Assign the template */
		invcopy(o_ptr, k_idx);

		/* Mega-Hack -- mark the item as an artifact */
		o_ptr->name1 = i;
        object_desc(NULL, o_name, sizeof(o_name), o_ptr, TRUE, 3);
        plog(format("Special artifact %s created", o_name));


		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
}


/*
 * Attempt to change an object into an artifact
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(int Depth, object_type *o_ptr)
{
	int i, j;
    char o_name[80];

	/* No artifacts in the town */
	if (!Depth) return (FALSE);

	/* Paranoia -- no "plural" artifacts */
	if (o_ptr->number != 1) return (FALSE);

	/* Check the artifact list (skip the "specials") */
	for (i = ART_MIN_NORMAL; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		bool okay = TRUE;

		/* Skip "empty" items */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;
		if (a_ptr->sval != o_ptr->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > Depth)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (a_ptr->level - Depth) * 2;
printf("Out of depth?\n");
			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}
printf("Art rarity roll...\n");
		/* We must make the "rarity roll" */
		if (randint0(a_ptr->rarity) != 0) continue;
printf("Passed!\n");
		/* MEGA-HACK! GAMEPLAY BREAKER! */
		for (j = 1; j <= NumPlayers; j++)
		{
			player_type *p_ptr = Players[j];
			/* There's a player on a level who already found this artifact once
			 * -- this causes ALL other players on level to suffer */
			if ((p_ptr->dun_depth == Depth) && (p_ptr->a_info[i] > cfg_preserve_artifacts))
			{
				/* Artifact WON'T be generated! */
				okay = FALSE;
				break;
			}
		}
		if (!okay) continue;

		/* Hack -- mark the item as an artifact */
		o_ptr->name1 = i;
        object_desc(NULL, o_name, sizeof(o_name), o_ptr, TRUE, 3);
        plog(format("Artifact %s created", o_name));

		/* Success */
		return (TRUE);
	}

#if defined(RANDART)
    /* An extra chance at being a randart */
    if (cfg_random_artifacts && !randint0(RANDART_RARITY))
    {
	o_ptr->name1 = ART_RANDART;

	/* Piece together a 32-bit random seed */
	o_ptr->name3 = randint0(0xFFFF) << 16;
	o_ptr->name3 += randint0(0xFFFF);
	/* Check the tval is allowed */
	if (randart_make(o_ptr) == NULL)
	{
		/* If not, wipe seed. No randart today */
		o_ptr->name1 = 0;
		o_ptr->name3 = 0L;

		return (FALSE);
	}
		
	return (TRUE);
    }
#endif

	/* Failure */
	return (FALSE);
}


/*
 * Charge a new wand.
 */
static void charge_wand(object_type *o_ptr)
{
	switch (o_ptr->sval)
	{
		case SV_WAND_HEAL_MONSTER:		o_ptr->pval = randint1(20) + 8; break;
		case SV_WAND_HASTE_MONSTER:		o_ptr->pval = randint1(20) + 8; break;
		case SV_WAND_CLONE_MONSTER:		o_ptr->pval = randint1(5)  + 3; break;
		case SV_WAND_TELEPORT_AWAY:		o_ptr->pval = randint1(5)  + 6; break;
		case SV_WAND_DISARMING:			o_ptr->pval = randint1(5)  + 4; break;
		case SV_WAND_TRAP_DOOR_DEST:	o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_STONE_TO_MUD:		o_ptr->pval = randint1(4)  + 3; break;
		case SV_WAND_LITE:				o_ptr->pval = randint1(10) + 6; break;
		case SV_WAND_SLEEP_MONSTER:		o_ptr->pval = randint1(15) + 8; break;
		case SV_WAND_SLOW_MONSTER:		o_ptr->pval = randint1(10) + 6; break;
		case SV_WAND_CONFUSE_MONSTER:	o_ptr->pval = randint1(12) + 6; break;
		case SV_WAND_FEAR_MONSTER:		o_ptr->pval = randint1(5)  + 3; break;
		case SV_WAND_DRAIN_LIFE:		o_ptr->pval = randint1(3)  + 3; break;
		case SV_WAND_POLYMORPH:			o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_STINKING_CLOUD:	o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_MAGIC_MISSILE:		o_ptr->pval = randint1(10) + 6; break;
		case SV_WAND_ACID_BOLT:			o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_ELEC_BOLT:			o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_FIRE_BOLT:			o_ptr->pval = randint1(8)  + 6; break;
		case SV_WAND_COLD_BOLT:			o_ptr->pval = randint1(5)  + 6; break;
		case SV_WAND_ACID_BALL:			o_ptr->pval = randint1(5)  + 2; break;
		case SV_WAND_ELEC_BALL:			o_ptr->pval = randint1(8)  + 4; break;
		case SV_WAND_FIRE_BALL:			o_ptr->pval = randint1(4)  + 2; break;
		case SV_WAND_COLD_BALL:			o_ptr->pval = randint1(6)  + 2; break;
		case SV_WAND_WONDER:			o_ptr->pval = randint1(15) + 8; break;
		case SV_WAND_ANNIHILATION:		o_ptr->pval = randint1(2)  + 1; break;
		case SV_WAND_DRAGON_FIRE:		o_ptr->pval = randint1(3)  + 1; break;
		case SV_WAND_DRAGON_COLD:		o_ptr->pval = randint1(3)  + 1; break;
		case SV_WAND_DRAGON_BREATH:		o_ptr->pval = randint1(3)  + 1; break;
	}
}



/*
 * Charge a new staff.
 */
static void charge_staff(object_type *o_ptr)
{
	switch (o_ptr->sval)
	{
		case SV_STAFF_DARKNESS:			o_ptr->pval = randint1(8)  + 8; break;
		case SV_STAFF_SLOWNESS:			o_ptr->pval = randint1(8)  + 8; break;
		case SV_STAFF_HASTE_MONSTERS:	o_ptr->pval = randint1(8)  + 8; break;
		case SV_STAFF_SUMMONING:		o_ptr->pval = randint1(3)  + 1; break;
		case SV_STAFF_TELEPORTATION:	o_ptr->pval = randint1(4)  + 5; break;
		case SV_STAFF_IDENTIFY:			o_ptr->pval = randint1(15) + 5; break;
		case SV_STAFF_REMOVE_CURSE:		o_ptr->pval = randint1(3)  + 4; break;
		case SV_STAFF_STARLITE:			o_ptr->pval = randint1(5)  + 6; break;
		case SV_STAFF_LITE:				o_ptr->pval = randint1(20) + 8; break;
		case SV_STAFF_MAPPING:			o_ptr->pval = randint1(5)  + 5; break;
		case SV_STAFF_DETECT_GOLD:		o_ptr->pval = randint1(20) + 8; break;
		case SV_STAFF_DETECT_ITEM:		o_ptr->pval = randint1(15) + 6; break;
		case SV_STAFF_DETECT_TRAP:		o_ptr->pval = randint1(5)  + 6; break;
		case SV_STAFF_DETECT_DOOR:		o_ptr->pval = randint1(8)  + 6; break;
		case SV_STAFF_DETECT_INVIS:		o_ptr->pval = randint1(15) + 8; break;
		case SV_STAFF_DETECT_EVIL:		o_ptr->pval = randint1(15) + 8; break;
		case SV_STAFF_CURE_LIGHT:		o_ptr->pval = randint1(5)  + 6; break;
		case SV_STAFF_CURING:			o_ptr->pval = randint1(3)  + 4; break;
		case SV_STAFF_HEALING:			o_ptr->pval = randint1(2)  + 1; break;
		case SV_STAFF_THE_MAGI:			o_ptr->pval = randint1(2)  + 2; break;
		case SV_STAFF_SLEEP_MONSTERS:	o_ptr->pval = randint1(5)  + 6; break;
		case SV_STAFF_SLOW_MONSTERS:	o_ptr->pval = randint1(5)  + 6; break;
		case SV_STAFF_SPEED:			o_ptr->pval = randint1(3)  + 4; break;
		case SV_STAFF_PROBING:			o_ptr->pval = randint1(6)  + 2; break;
		case SV_STAFF_DISPEL_EVIL:		o_ptr->pval = randint1(3)  + 4; break;
		case SV_STAFF_POWER:			o_ptr->pval = randint1(3)  + 1; break;
		case SV_STAFF_HOLINESS:			o_ptr->pval = randint1(2)  + 2; break;
		case SV_STAFF_BANISHMENT:			o_ptr->pval = randint1(2)  + 1; break;
		case SV_STAFF_EARTHQUAKES:		o_ptr->pval = randint1(5)  + 3; break;
		case SV_STAFF_DESTRUCTION:		o_ptr->pval = randint1(3)  + 1; break;
	}
}



/* get an ego item from e_info */
 bool check_ego(object_type *o_ptr, int level, int power, int idx)
{
	int idxtval;
	ego_item_type *e_ptr = &e_info[idx];
	
	/* check possible ego tval values */
	if (o_ptr->tval == e_ptr->tval[0])
		idxtval = 0;
	else if (o_ptr->tval == e_ptr->tval[1])
		idxtval = 1;
	else if (o_ptr->tval == e_ptr->tval[2])
		idxtval = 2;
	else
		return FALSE;
		
	/* check sval range */
	if ((o_ptr->sval < e_ptr->min_sval[idxtval]) ||
			(o_ptr->sval > e_ptr->max_sval[idxtval]))
		return FALSE;

	/* check min depth */
	if (level < e_ptr->level)
		return FALSE;
		
	/* check rarity */
	if (randint0(e_ptr->rarity) != 0)
		return FALSE;
		
	/* check cursed egos */
	if ((power > 1) && ( (e_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(e_ptr->flags3 & TR3_HEAVY_CURSE) ||
		 		(e_ptr->flags3 & TR3_PERMA_CURSE) ) )
		return FALSE;
		
	/* check cursed objects */
	if ( (power < -1) && !( (e_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(e_ptr->flags3 & TR3_HEAVY_CURSE) ||
		 		(e_ptr->flags3 & TR3_PERMA_CURSE) ) )
		return FALSE;
	
	/* we have a winner */
	return TRUE;
}

/*
 * Apply magic to an item known to be a "weapon"
 * Uses check_ego to generate a random ego item
 */
static void a_m_aux_1(object_type *o_ptr, int level, int power)
{
	int tohit1 = randint1(5) + m_bonus(5, level);
	int todam1 = randint1(5) + m_bonus(5, level);

	int tohit2 = m_bonus(10, level);
	int todam2 = m_bonus(10, level);

    int idx, nbtries = 1000;


	/* Good */
	if (power > 0)
	{
		/* Enchant */
		o_ptr->to_h += tohit1;
		o_ptr->to_d += todam1;

		/* Very good */
		if (power > 1)
		{
			/* Enchant again */
			o_ptr->to_h += tohit2;
			o_ptr->to_d += todam2;
		}
	}

    /* Cursed */
    else if (power < 0)
    {
        /* Penalize */
        o_ptr->to_h -= tohit1;
        o_ptr->to_d -= todam1;

			/* Very cursed */
        if (power < -1)
			{
            /* Penalize again */
            o_ptr->to_h -= tohit2;
            o_ptr->to_d -= todam2;
        }

        /* Cursed (if "bad") */
        if (o_ptr->to_h + o_ptr->to_d < 0) o_ptr->ident |= ID_CURSED;
    }

	/* Analyze type */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		{
			/* Very bad */
			if (power < -1)
			{
				/* Hack -- Horrible digging bonus */
				o_ptr->pval = 0 - (5 + randint1(5));
			}

			/* Bad */
			else if (power < 0)
			{
				/* Hack -- Reverse digging bonus */
				o_ptr->pval = 0 - (o_ptr->pval);
			}

			break;
		}


		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* Very Good */
			if (power > 1)
			{
				/* Hack -- Super-charge the damage dice */
				while ((o_ptr->dd * o_ptr->ds > 0) &&
				       (randint0(10L * o_ptr->dd * o_ptr->ds) == 0))
				{
					o_ptr->dd++;
				}

				/* Hack -- Lower the damage dice */
				if (o_ptr->dd > 9) o_ptr->dd = 9;
			}

			break;
		}


		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			/* Very good */
			if (power > 1)
			{
				/* Hack -- super-charge the damage dice */
				while ((o_ptr->dd * o_ptr->ds > 0) &&
				       (randint0(10L * o_ptr->dd * o_ptr->ds) == 0))
				{
					o_ptr->dd++;
				}

				/* Hack -- restrict the damage dice */
				if (o_ptr->dd > 9) o_ptr->dd = 9;
			}

			break;
		}
	}
    
    /* are we done? */
    if ((power >= -1) && (power <= 1)) return;
    
    /* Retrieve an ego item randomly */
    while (--nbtries)
    {
    	idx = randint0(z_info->e_max);
    	
    	/* check ego */
    	if (check_ego(o_ptr, level, power, idx))
		{
    		o_ptr->name2 = idx; /* EGO_XXX */
			break;
		}
	}
}


/*
 * Apply magic to an item known to be "armor"
 * Uses check_ego to generate a random ego item
 */
static void a_m_aux_2(object_type *o_ptr, int level, int power)
{
    int toac1 = randint1(5) + m_bonus(5, level);
    int toac2 = m_bonus(10, level);
    int idx, nbtries = 1000;

    /* Good */
    if (power > 0)
                        {
        /* Enchant */
        o_ptr->to_a += toac1;

			/* Very good */
			if (power > 1)
			{
            /* Enchant again */
            o_ptr->to_a += toac2;
        }
    }

    /* Cursed */
    else if (power < 0)
				{
        /* Penalize */
        o_ptr->to_a -= toac1;

        /* Very cursed */
        if (power < -1)
        {
            /* Penalize again */
            o_ptr->to_a -= toac2;
        }

        /* Cursed (if "bad") */
        if (o_ptr->to_a < 0) o_ptr->ident |= ID_CURSED;
    }

    /* DSM */
    if (o_ptr->tval == TV_DRAG_ARMOR)
    {
    	/* Rating boost */
    	rating += 30;

    	/* Mention the item */
    	/*if (cheat_peek) object_mention(o_ptr);*/

    	return;
			}

    /* Set the orcish shield's STR and CON bonus */
    if ((o_ptr->tval == TV_SHIELD) && (o_ptr->sval == SV_ORCISH_SHIELD))
	 {
    	o_ptr->bpval = randint1(2);
    	
    	/* Cursed orcish shield */
    	if (power < 0) 
    	{
    		o_ptr->bpval = -o_ptr->bpval;
    		o_ptr->ident |= ID_CURSED;
    	}
    }
    
    /* Set the Witan Boots stealth penalty */
    if ((o_ptr->tval == TV_BOOTS) && (o_ptr->sval == SV_PAIR_OF_WITAN_BOOTS))
				{
    	o_ptr->bpval = -2;
				}
    
	/* Set the Kolla cloak's base bonuses */
	if ((o_ptr->tval == TV_CLOAK) && (o_ptr->sval == SV_KOLLA))
	{
		o_ptr->bpval = randint1(2);

		/* Cursed kolla */
		if (o_ptr->to_a < 0) 
		{
			o_ptr->bpval = -o_ptr->bpval;
			o_ptr->ident |= ID_CURSED;
		}
		else
		{
			rating += 20;
		}
	}

	/* are we done? */
	if ((power >= -1) && (power <= 1)) return;
    
    /* Retrieve an ego item randomly */
    while (--nbtries)
    {
    	idx = randint0(z_info->e_max);
    	
    	/* check ego */
    	if (check_ego(o_ptr, level, power, idx))
    	{
    		o_ptr->name2 = idx; /* EGO_XXX */
			break;
		}
	}
}



/*
 * Apply magic to an item known to be a "ring" or "amulet"
 *
 * Hack -- note special rating boost for ring of speed
 * Hack -- note special "pval boost" code for ring of speed
 * Hack -- note that some items must be cursed (or blessed)
 */
static void a_m_aux_3(object_type *o_ptr, int level, int power)
{
	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_RING:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Strength, Constitution, Dexterity, Intelligence */
				case SV_RING_STR:
				case SV_RING_CON:
				case SV_RING_DEX:
				case SV_RING_INT:
				{
					/* Stat bonus */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Ring of Speed! */
				case SV_RING_SPEED:
				{
					/* Base speed (1 to 10) */
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					/* Super-charge the ring */
					while (randint0(100) < 50) o_ptr->pval++;

					/* Cursed Ring */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);

						break;
					}

					/* Rating boost */
					rating += 25;

					break;
				}

				/* Searching */
				case SV_RING_SEARCHING:
				{
					/* Bonus to searching */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

                /* Flames, Acid, Ice, Lightning */
				case SV_RING_FLAMES:
				case SV_RING_ACID:
				case SV_RING_ICE:
                case SV_RING_LIGHTNING:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
					break;
				}

				/* Weakness, Stupidity */
				case SV_RING_WEAKNESS:
				case SV_RING_STUPIDITY:
				{
					/* Broken */
					o_ptr->ident |= ID_BROKEN;

					/* Cursed */
					o_ptr->ident |= ID_CURSED;

					/* Penalize */
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* WOE, Stupidity */
				case SV_RING_WOE:
				{
					/* Broken */
					o_ptr->ident |= ID_BROKEN;

					/* Cursed */
					o_ptr->ident |= ID_CURSED;

					/* Penalize */
					o_ptr->to_a = 0 - (5 + m_bonus(10, level));
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* Ring of damage */
				case SV_RING_DAMAGE:
				{
					/* Bonus to damage */
					o_ptr->to_d = 5 + randint1(5) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse bonus */
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

					break;
				}

				/* Ring of Accuracy */
				case SV_RING_ACCURACY:
				{
					/* Bonus to hit */
					o_ptr->to_h = 5 + randint1(5) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse tohit */
						o_ptr->to_h = 0 - (o_ptr->to_h);
					}

					break;
				}

				/* Ring of Protection */
				case SV_RING_PROTECTION:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse toac */
						o_ptr->to_a = 0 - (o_ptr->to_a);
					}

					break;
				}

				/* Ring of Slaying */
				case SV_RING_SLAYING:
				{
					/* Bonus to damage and to hit */
					o_ptr->to_d = randint1(5) + m_bonus(5, level);
					o_ptr->to_h = randint1(5) + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse bonuses */
						o_ptr->to_h = 0 - (o_ptr->to_h);
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

					break;
				}
			}

			break;
		}

		case TV_AMULET:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Amulet of wisdom/charisma */
				case SV_AMULET_WISDOM:
				case SV_AMULET_CHARISMA:
				{
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Amulet of searching */
				case SV_AMULET_SEARCHING:
				{
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

                /* Amulet of ESP -- never cursed */
                case SV_AMULET_ESP:
                {
					o_ptr->pval = randint1(5) + m_bonus(5, level);

                    break;
                }

				/* Amulet of the Magi -- never cursed */
				case SV_AMULET_THE_MAGI:
				{
                    o_ptr->pval = 1 + m_bonus(2, level);
                    o_ptr->to_a = randint1(5) + m_bonus(5, level);
                    
                    /* mangband-specific -- TURN IT ON IF YOU NEED
                    o_ptr->xtra1 = OBJECT_XTRA_TYPE_RESIST;
                    o_ptr->xtra2 = (byte)randint1(OBJECT_XTRA_SIZE_RESIST);
					*/

                    break;
                }

				/* Amulet of speed */
				case SV_AMULET_SPEED:
				{
					// Amulets of speed can't give very
					// much, and are rarely +3.
					o_ptr->pval = randint1(randint1(3)); 

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= ID_BROKEN;

						/* Cursed */
						o_ptr->ident |= ID_CURSED;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

         			/* Amulet of Terken -- never cursed */
				case SV_AMULET_TERKEN:
				{
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					o_ptr->xtra1 = OBJECT_XTRA_TYPE_POWER;
					o_ptr->xtra2 = (byte)randint0(OBJECT_XTRA_SIZE_POWER);
					break;
				}

         			/* Amulet of the Moon -- never cursed */
				case SV_AMULET_THE_MOON:
				{
					o_ptr->pval = randint1(5) + m_bonus(5, level);
                    o_ptr->to_h = randint1(5);
                    o_ptr->to_d = randint1(5);

					break;
				}

                /* Amulet of Devotion -- never cursed */
                case SV_AMULET_DEVOTION:
                {
					o_ptr->pval = 1 + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

                    break;
                }

                /* Amulet of Weaponmastery -- never cursed */
                case SV_AMULET_WEPMASTERY:
                {
					o_ptr->to_h = 1 + m_bonus(4, level);
					o_ptr->to_d = 1 + m_bonus(4, level);
					o_ptr->pval = 1 + m_bonus(2, level);

					/* Boost the rating */
					rating += 25;

                    break;
                }

                /* Amulet of Trickery -- never cursed */
                case SV_AMULET_TRICKERY:
                {
					o_ptr->pval = randint1(1) + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

                    break;
                }

                /* Amulet of infravision */
                case SV_AMULET_INFRA:
                {
                    o_ptr->pval = randint1(5) + m_bonus(5, level);

                    /* Cursed */
                    if (power < 0)
                    {
                        /* Broken */
                        o_ptr->ident |= ID_BROKEN;

                        /* Cursed */
                        o_ptr->ident |= ID_CURSED;

                        /* Reverse bonuses */
                        o_ptr->pval = 0 - (o_ptr->pval);
                    }

					break;
				}

				/* Amulet of Doom -- always cursed */
				case SV_AMULET_DOOM:
				{
					/* Broken */
					o_ptr->ident |= ID_BROKEN;

					/* Cursed */
					o_ptr->ident |= ID_CURSED;

					/* Penalize */
					o_ptr->pval = 0 - (randint1(5) + m_bonus(5, level));
					o_ptr->to_a = 0 - (randint1(5) + m_bonus(5, level));

					break;
				}
			}

			break;
		}
	}
}


/*
 * Apply magic to an item known to be "boring"
 *
 * Hack -- note the special code for various items
 */
static void a_m_aux_4(object_type *o_ptr, int level, int power)
{
	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_LITE:

		/* Hack -- Torches -- random fuel */
		if (o_ptr->sval == SV_LITE_TORCH)
		{
			if (o_ptr->pval) o_ptr->pval = randint1(o_ptr->pval);
		}

		/* Hack -- Lanterns -- random fuel */
		if (o_ptr->sval == SV_LITE_LANTERN)
		{
			if (o_ptr->pval) o_ptr->pval = randint1(o_ptr->pval);
		}

		break;


		case TV_WAND:

		/* Hack -- charge wands */
		charge_wand(o_ptr);

		break;


		case TV_STAFF:

		/* Hack -- charge staffs */
		charge_staff(o_ptr);

		break;
		
		case TV_ROD:

		/* Transfer the pval. */
		o_ptr->pval = k_info[o_ptr->k_idx].pval;
		break;

		case TV_CHEST:

		/* Hack -- skip ruined chests */
		if (k_info[o_ptr->k_idx].level <= 0) break;

		/* Hack -- pick a "difficulty" */
		o_ptr->pval = randint1(k_info[o_ptr->k_idx].level);

		/* Never exceed "difficulty" of 55 to 59 */
		if (o_ptr->pval > 55) o_ptr->pval = 55 + randint0(5);

		break;
	}
}



/*
 * Complete the "creation" of an object by applying "magic" to the item
 *
 * This includes not only rolling for random bonuses, but also putting the
 * finishing touches on ego-items and artifacts, giving charges to wands and
 * staffs, giving fuel to lites, and placing traps on chests.
 *
 * In particular, note that "Instant Artifacts", if "created" by an external
 * routine, must pass through this function to complete the actual creation.
 *
 * The base "chance" of the item being "good" increases with the "level"
 * parameter, which is usually derived from the dungeon level, being equal
 * to the level plus 10, up to a maximum of 75.  If "good" is true, then
 * the object is guaranteed to be "good".  If an object is "good", then
 * the chance that the object will be "great" (ego-item or artifact), also
 * increases with the "level", being equal to half the level, plus 5, up to
 * a maximum of 20.  If "great" is true, then the object is guaranteed to be
 * "great".  At dungeon level 65 and below, 15/100 objects are "great".
 *
 * If the object is not "good", there is a chance it will be "cursed", and
 * if it is "cursed", there is a chance it will be "broken".  These chances
 * are related to the "good" / "great" chances above.
 *
 * Otherwise "normal" rings and amulets will be "good" half the time and
 * "cursed" half the time, unless the ring/amulet is always good or cursed.
 *
 * If "okay" is true, and the object is going to be "great", then there is
 * a chance that an artifact will be created.  This is true even if both the
 * "good" and "great" arguments are false.  As a total hack, if "great" is
 * true, then the item gets 3 extra "attempts" to become an artifact.
 */
void apply_magic(int Depth, object_type *o_ptr, int lev, bool okay, bool good, bool great)
{
	int i, rolls, f1, f2, power;


	/* Magic ammo are always +0 +0 */
	if (((o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
	     (o_ptr->tval == TV_BOLT)) && (o_ptr->sval == SV_AMMO_MAGIC))
		return;

	/* Maximum "level" for various things */
	if (lev > MAX_DEPTH - 1) lev = MAX_DEPTH - 1;


	/* Base chance of being "good" */
	f1 = lev + 10;

	/* Maximal chance of being "good" */
	if (f1 > 75) f1 = 75;

	/* Base chance of being "great" */
	f2 = f1 / 2;

	/* Maximal chance of being "great" */
	if (f2 > 20) f2 = 20;


	/* Assume normal */
	power = 0;

	/* Roll for "good" */
	if (good || magik(f1))
	{
		/* Assume "good" */
		power = 1;

		/* Roll for "great" */
		if (great || magik(f2)) power = 2;
	}

	/* Roll for "cursed" */
	else if (magik(f1))
	{
		/* Assume "cursed" */
		power = -1;

		/* Roll for "broken" */
		if (magik(f2)) power = -2;
	}


	/* Assume no rolls */
	rolls = 0;

	/* Get one roll if excellent */
	if (power >= 2) rolls = 1;

	/* Hack -- Get four rolls if forced great */
	if (great) rolls = 4;

	/* Hack -- Get no rolls if not allowed */
    if (!okay || artifact_p(o_ptr)) rolls = 0;

	/* Roll for artifacts if allowed */
	for (i = 0; i < rolls; i++)
	{
		/* Roll for an artifact */
		if (make_artifact(Depth, o_ptr)) break;
	}


	/* Hack -- analyze artifacts */
    if (artifact_p(o_ptr))
    {
	artifact_type *a_ptr = artifact_ptr(o_ptr);

	if (true_artifact_p(o_ptr))
	{
		/* Hack -- Mark the artifact as "created" */
		a_ptr->cur_num = 1;

		/* Hack -- reset ownership (just in case) */
		a_ptr->owner_id = 0;
		a_ptr->owner_name = 0;
	}

		/* Info */
		/* s_printf("Created artifact %d.\n", o_ptr->name1); */

		/* Extract the other fields */
		o_ptr->pval = a_ptr->pval;
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;
		o_ptr->to_a = a_ptr->to_a;
		o_ptr->to_h = a_ptr->to_h;
		o_ptr->to_d = a_ptr->to_d;
		o_ptr->weight = a_ptr->weight;

		/* Hack -- extract the "broken" flag */
		if (!a_ptr->cost) o_ptr->ident |= ID_BROKEN;

		/* Hack -- extract the "cursed" flag */
		if ((a_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(a_ptr->flags3 & TR3_HEAVY_CURSE) ||
		 		(a_ptr->flags3 & TR3_PERMA_CURSE))
					o_ptr->ident |= ID_CURSED;		
		

		/* Mega-Hack -- increase the rating */
		rating += 10;

		/* Mega-Hack -- increase the rating again */
		if (a_ptr->cost > 50000L) rating += 10;

		/* Set the good item flag */
		good_item_flag = TRUE;

		/* Cheat -- peek at the item */
		/* if (cheat_peek) object_mention(o_ptr); */

		/* Done */
		return;
	}


	/* Apply magic */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOW:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (power) a_m_aux_1(o_ptr, lev, power);
			break;
		}

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_HELM:
		case TV_CROWN:
		case TV_CLOAK:
		case TV_GLOVES:
		case TV_BOOTS:
		{
			// Power is no longer required since things such as
			// Kollas need magic applied to finish their normal
			// generation.
			//if (power) a_m_aux_2(o_ptr, lev, power);
			a_m_aux_2(o_ptr, lev, power);
			break;
		}

		case TV_RING:
		case TV_AMULET:
		{
			if (!power && (randint0(100) < 50)) power = -1;
			a_m_aux_3(o_ptr, lev, power);
			break;
		}

		default:
		{
			a_m_aux_4(o_ptr, lev, power);
			break;
		}
	}


	/* Hack -- analyze ego-items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		/* Extra powers */
		if (e_ptr->xtra)
		{
			o_ptr->xtra1 = e_ptr->xtra;
			switch (o_ptr->xtra1)
			{
				case OBJECT_XTRA_TYPE_SUSTAIN:
				{
					o_ptr->xtra2 = (byte)randint0(OBJECT_XTRA_SIZE_SUSTAIN);
					break;
				}

				case OBJECT_XTRA_TYPE_RESIST:
				{
					o_ptr->xtra2 = (byte)randint0(OBJECT_XTRA_SIZE_RESIST);
					break;
				}

				case OBJECT_XTRA_TYPE_POWER:
				{
					o_ptr->xtra2 = (byte)randint0(OBJECT_XTRA_SIZE_POWER);
					break;
				}
			}
		}

		/* Hack -- acquire "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= ID_BROKEN;

		/* Hack -- acquire "cursed" flag */
		if ((e_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(e_ptr->flags3 & TR3_HEAVY_CURSE) ||
		 		(e_ptr->flags3 & TR3_PERMA_CURSE))
					o_ptr->ident |= ID_CURSED;		
		

		/* Hack -- apply extra penalties if needed */
		if (cursed_p(o_ptr) || broken_p(o_ptr))
		{
			/* Hack -- obtain bonuses */
			if (e_ptr->max_to_h) o_ptr->to_h -= randint1(e_ptr->max_to_h);
			if (e_ptr->max_to_d) o_ptr->to_d -= randint1(e_ptr->max_to_d);
			if (e_ptr->max_to_a) o_ptr->to_a -= randint1(e_ptr->max_to_a);

			/* Hack -- obtain pval */
			if (e_ptr->max_pval) o_ptr->pval -= randint1(e_ptr->max_pval);
		}

		/* Hack -- apply extra bonuses if needed */
		else
		{
			/* Hack -- obtain bonuses */
	   	/* Don't forget minuses!! */
            if (e_ptr->max_to_h > 0) o_ptr->to_h += randint1(e_ptr->max_to_h);
	    else if (e_ptr->max_to_h < 0) o_ptr->to_h -= randint1(-e_ptr->max_to_h);
	    		if (e_ptr->max_to_d > 0) o_ptr->to_d += randint1(e_ptr->max_to_d);
	    else if (e_ptr->max_to_d < 0) o_ptr->to_d -= randint1(-e_ptr->max_to_d);
	    		if (e_ptr->max_to_a > 0) o_ptr->to_a += randint1(e_ptr->max_to_a);
	    else if (e_ptr->max_to_a < 0) o_ptr->to_a -= randint1(-e_ptr->max_to_a);

			/* Hack -- obtain pval */
	   	/* Don't forget minuses!! */
            if (e_ptr->max_pval > 0) o_ptr->pval += randint1(e_ptr->max_pval);
	    else if (e_ptr->max_pval < 0) o_ptr->pval -= randint1(-e_ptr->max_pval);
		}

		/* Hack -- apply rating bonus */
		rating += e_ptr->rating;

		/* Cheat -- describe the item */
		/*if (cheat_peek) object_mention(o_ptr);*/

		/* Done */
		return;
	}


	/* Examine real objects */
	if (o_ptr->k_idx)
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		/* Hack -- acquire "broken" flag */
		if (!k_ptr->cost) o_ptr->ident |= ID_BROKEN;

		/* Hack -- acquire "cursed" flag */
		if ((k_ptr->flags3 & TR3_LIGHT_CURSE) ||
				(k_ptr->flags3 & TR3_HEAVY_CURSE) ||
 		 		(k_ptr->flags3 & TR3_PERMA_CURSE))
		 			o_ptr->ident |= ID_CURSED;
	}
}



/*
 * Hack -- determine if a template is "good"
 */
static bool kind_is_good(int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

	/* Analyze the item type */
	switch (k_ptr->tval)
	{
		/* Armor -- Good unless damaged */
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		{
			if (k_ptr->to_a < 0) return (FALSE);
			return (TRUE);
		}

		/* Weapons -- Good unless damaged */
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			if (k_ptr->to_h < 0) return (FALSE);
			if (k_ptr->to_d < 0) return (FALSE);
			return (TRUE);
		}

		/* Ammo -- Arrows/Bolts are good */
	/* Shots should be good too, why not? */
		case TV_BOLT:
		case TV_ARROW:
        case TV_SHOT:
		{
	    /* Magic ammo never get good */
            return (k_ptr->sval != SV_AMMO_MAGIC);
		}

		/* Books -- High level books are good */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			if (k_ptr->sval >= SV_BOOK_MIN_GOOD) return (TRUE);
			return (FALSE);
		}

		/* Rings -- Rings of Speed are good */
		case TV_RING:
		{
			if (k_ptr->sval == SV_RING_SPEED) return (TRUE);
			return (FALSE);
		}
	}

	/* Assume not good */
	return (FALSE);
}


		
bool place_specific_object(int Depth, int y1, int x1, object_type *forge, int lev, int num)
{
	int o_idx, i, d, x, y, xtra_hack;
	
	/* Scatter some objects */
	for (; num > 0; --num)
	{
		/* Check near the player for space */
		for (i = 0; i < 25; ++i)
		{
			/* Increasing Distance */
			d = (i + 4) / 5;

			/* Pick a location */
			scatter(Depth, &y, &x, y1, x1, d, 0);

			/* Must have a clean grid */
			if (!cave_clean_bold(Depth, y, x)) continue;

			/* Place a great object */
			
			/* Make an object */
			o_idx = o_pop();
		
			/* Success */
			if (o_idx)
			{
				cave_type		*c_ptr;
				object_type		*o_ptr;
				int			i;
		
				o_ptr = &o_list[o_idx];
		
				(*o_ptr) = (*forge);
		
				o_ptr->ident = 0;		
		
				o_ptr->iy = y;
				o_ptr->ix = x;
				o_ptr->dun_depth = Depth;
		
				c_ptr = &cave[Depth][y][x];
				c_ptr->o_idx = o_idx;
				
				/* Make sure no one sees it at first */
				for (i = 1; i <= NumPlayers; i++)
				{
					/* He can't see it */
					Players[i]->obj_vis[o_idx] = FALSE;
				}
			
				
				xtra_hack = forge->xtra2;
				apply_magic(Depth, o_ptr, lev, TRUE, FALSE, FALSE);
				o_ptr->xtra2 = xtra_hack;
				
				o_ptr->to_h = forge->to_h;
				o_ptr->to_d = forge->to_d;
				o_ptr->to_a = forge->to_a;
				o_ptr->pval = forge->pval;

				/* Identify fully? */
				if ((forge->ident & ID_KNOWN))
				{
					object_known(o_ptr);
				}

				/* Notice */
				note_spot_depth(Depth, y, x);

				/* Redraw */
				everyone_lite_spot(Depth, y, x);
					
			}

			/* Placement accomplished */
			break;
		}
	}	

	return !num;
}


/*
 * Attempt to place an object (normal or good/great) at the given location.
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * This routine requires a clean floor grid destination.
 */
object_type* place_object(int Depth, int y, int x, bool good, bool great, byte origin)
{
	int			o_idx, prob, base;

	int			old = rating;

	object_type		forge;


	/* Paranoia -- check bounds */
	if (!in_bounds(Depth, y, x)) return (NULL);

	/* Require clean floor space */
	if (!cave_clean_bold(Depth, y, x)) return (NULL);

	/* Chance of "special object" */
	prob = (good ? 10 : 1000);

	/* Base level for the object */
	base = (good ? (object_level + 10) : object_level);


	/* Hack -- clear out the forgery */
	invwipe(&forge);

	/* Generate a special object, or a normal object */
	if ((randint0(prob) != 0) || !make_artifact_special(Depth, &forge))
	{
		int k_idx;

		/* Good objects */
		if (good)
		{
			/* Activate restriction */
			get_obj_num_hook = kind_is_good;

			/* Prepare allocation table */
			get_obj_num_prep();
		}

		/* Pick a random object */
		k_idx = get_obj_num(base);

		/* Good objects */
		if (good)
		{
			/* Clear restriction */
			get_obj_num_hook = NULL;

			/* Prepare allocation table */
			get_obj_num_prep();
		}

		/* Handle failure */
		if (!k_idx) return (NULL);

		/* Prepare the object */
		invcopy(&forge, k_idx);
	}

	/* Apply magic (allow artifacts) */
	apply_magic(Depth, &forge, object_level, TRUE, good, great);

    /* Hack -- generate multiple spikes/missiles (except artifacts!) */
	switch (forge.tval)
	{
		case TV_SPIKE:
		case TV_SHOT:
		case TV_ARROW:
        case TV_BOLT: if (!artifact_p(&forge)) forge.number = damroll(6, 7);
	}


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		cave_type		*c_ptr;
		object_type		*o_ptr;
		int			i;

		o_ptr = &o_list[o_idx];

		(*o_ptr) = (forge);

		o_ptr->iy = y;
		o_ptr->ix = x;
		o_ptr->dun_depth = Depth;

		c_ptr = &cave[Depth][y][x];
		c_ptr->o_idx = o_idx;

		/* Notice "okay" out-of-depth objects (unless already noticed) */
		if (!cursed_p(o_ptr) && !broken_p(o_ptr) &&
		    (rating == old) && (k_info[o_ptr->k_idx].level > Depth))
		{
			/* Rating increase */
			rating += (k_info[o_ptr->k_idx].level - Depth);

			/* Cheat -- peek at items */
			/*if (cheat_peek) object_mention(o_ptr);*/
		}

		/* Make sure no one sees it at first */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* He can't see it */
			Players[i]->obj_vis[o_idx] = FALSE;
		}

		/* Add origin */
		o_ptr->origin = origin;
		o_ptr->origin_depth = Depth;

#ifdef DEBUG
	  {
		char tmp[120];
		object_desc(NULL, tmp, sizeof(tmp), o_ptr, FALSE, 1);
		/* Audit item allocation */
		cheat(format("%s %s", artifact_p(o_ptr) ? "+a": "+o", tmp));
	  }
#endif
	}

	/* Observe placement. (dungeon level has already been generated) */
	if (server_dungeon && o_idx)
	{
		player_type *q_ptr;

		/* Notice it */
		note_spot_depth(Depth, y, x);

		/* Display it */
		everyone_lite_spot(Depth, y, x);

		/* Under some player */
		if ((q_ptr = player_on_cave(Depth,y,x)))
		{
			msg_print(q_ptr, "You feel something roll beneath your feet.");
			floor_item_notify(q_ptr, o_idx, TRUE);
		}
	}

	/* Success */
	if (o_idx) return &o_list[o_idx];

	return FALSE;
}



/*
 * Scatter some "great" objects near the player
 *
 * TODO: port version from 312 (!) (or later?)
 * This version ignores the "great" bool
 */
void acquirement(int Depth, int y1, int x1, int num, bool great)
{
	int        y, x, i, d;
    int oblev;

	/* Scatter some objects */
	for (; num > 0; --num)
	{
		/* Check near the player for space */
		for (i = 0; i < 25; ++i)
		{
			/* Increasing Distance */
			d = (i + 4) / 5;

			/* Pick a location */
			scatter(Depth, &y, &x, y1, x1, d, 0);

			/* Must have a clean grid */
			if (!cave_clean_bold(Depth, y, x)) continue;

			/* Place a great object */
			oblev = object_level;
			object_level = Depth;
			place_object(Depth, y, x, TRUE, TRUE, ORIGIN_ACQUIRE);
			object_level = oblev;

			/* Placement accomplished */
			break;
		}
	}
}





/*
 * Places a random trap at the given location.
 *
 * The location must be a valid, empty, clean, floor grid.
 *
 * Note that all traps start out as "invisible" and "untyped", and then
 * when they are "discovered" (by detecting them or setting them off),
 * the trap is "instantiated" as a visible, "typed", trap.
 */
void place_trap(int Depth, int y, int x)
{
	cave_type *c_ptr;

	/* Paranoia -- verify location */
	if (!in_bounds(Depth, y, x)) return;

	/* Require empty, clean, floor grid */
	if (!cave_naked_bold(Depth, y, x)) return;

	/* Access the grid */
	c_ptr = &cave[Depth][y][x];

	/* Place an invisible trap */
	c_ptr->feat = FEAT_INVIS;

	/* Note: we don't use cave_set_feat here, so that
	 * the clients DON'T get any floor updates... */
}


/*
 * XXX XXX XXX Do not use these hard-coded values.
 */
#define OBJ_GOLD_LIST	480	/* First "gold" entry */
#define MAX_GOLD	18	/* Number of "gold" entries */

/*
 * Places a treasure (Gold or Gems) at given location
 * The location must be a valid, empty, floor grid.
 */
void place_gold(int Depth, int y, int x)
{
	int		i, j, o_idx;

	s32b	base;

	cave_type	*c_ptr;
	object_type	*o_ptr;


	/* Paranoia -- check bounds */
	if (!in_bounds(Depth, y, x)) return;

	/* Require clean floor grid */
	if (!cave_clean_bold(Depth, y, x)) return;


	/* Hack -- Pick a Treasure variety */
	i = ((randint1(object_level + 2) + 2) / 2) - 1;

	/* Apply "extra" magic */
	if (randint0(GREAT_OBJ) == 0)
	{
		i += randint1(object_level + 1);
	}

	/* Hack -- Creeping Coins only generate "themselves" */
	if (coin_type) i = coin_type;

	/* Do not create "illegal" Treasure Types */
	if (i >= MAX_GOLD) i = MAX_GOLD - 1;


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		o_ptr = &o_list[o_idx];

		invcopy(o_ptr, OBJ_GOLD_LIST + i);

		o_ptr->iy = y;
		o_ptr->ix = x;
		o_ptr->dun_depth = Depth;

		c_ptr = &cave[Depth][y][x];
		c_ptr->o_idx = o_idx;

		/* Hack -- Base coin cost */
		base = k_info[OBJ_GOLD_LIST+i].cost;

		/* Determine how much the treasure is "worth" */
		o_ptr->pval = (base + (8L * randint1(base)) + randint1(8));

		/* No one can see it */
		for (j = 1; j <= NumPlayers; j++)
		{
			/* This player can't see it */
			Players[j]->obj_vis[o_idx] = FALSE;
		}
	}

	/* Observe placement. (dungeon level has already been generated) */
	if (server_dungeon && o_idx)
	{
		player_type *q_ptr;

		/* Notice it */
		note_spot_depth(Depth, y, x);

		/* Display it */
		everyone_lite_spot(Depth, y, x);

		/* Under some player */
		if ((q_ptr = player_on_cave(Depth,y,x)))
		{
			msg_print(q_ptr, "You feel something roll beneath your feet.");
			floor_item_notify(q_ptr, o_idx, TRUE);
		}
	}

}



/*
 * Let an item 'o_ptr' fall to the ground at or near (y,x).
 * The initial location is assumed to be "in_bounds()".
 *
 * This function takes a parameter "chance".  This is the percentage
 * chance that the item will "disappear" instead of drop.  If the object
 * has been thrown, then this is the chance of disappearance on contact.
 *
 * Hack -- this function uses "chance" to determine if it should produce
 * some form of "description" of the drop event (under the player).
 *
 * This function should probably be broken up into a function to determine
 * a "drop location", and several functions to actually "drop" an object.
 *
 * XXX XXX XXX Consider allowing objects to combine on the ground.
 */
void drop_near(object_type *o_ptr, int chance, int Depth, int y, int x)
{
	int		k, d, ny, nx, y1, x1, o_idx;

	cave_type	*c_ptr;
	object_type *j_ptr;

	bool flag = FALSE;
	bool comb = FALSE;

	if(o_ptr == NULL) return;


	/* Start at the drop point */
	ny = y1 = y;  nx = x1 = x;

	/* See if the object "survives" the fall */
	if (artifact_p(o_ptr) || (randint0(100) >= chance))
	{
		/* Start at the drop point */
		ny = y1 = y; nx = x1 = x;

		/* Try (20 times) to find an adjacent usable location */
		for (k = 0; !flag && (k < 50); ++k)
		{
			/* Distance distribution */
			d = ((k + 14) / 15);

			/* Pick a "nearby" location */
			scatter(Depth, &ny, &nx, y1, x1, d, 0);

			/* Get the cave grid */
			c_ptr = &cave[Depth][ny][nx];

			if (c_ptr->o_idx) {
				j_ptr = &o_list[c_ptr->o_idx];

				/* Check for combination */
				if (object_similar_floor(o_ptr, j_ptr)) 
				{
					flag = TRUE;
					comb = TRUE;
				}
			}
			
			/* Require clean floor space */
			if (!cave_clean_bold(Depth, ny, nx)) continue;

			/* Here looks good */
			flag = TRUE;
		}
	}

	/* Try really hard to place an artifact */
	if (!flag && artifact_p(o_ptr))
	{
		/* Start at the drop point */
		ny = y1 = y;  nx = x1 = x;

		/* Try really hard to drop it */
		for (k = 0; !flag && (k < 1000); k++)
		{
			d = 1;

			/* Pick a location */
			scatter(Depth, &ny, &nx, y1, x1, d, 0);

			/* Do not move through walls */
			if (!cave_floor_bold(Depth, ny, nx)) continue;

			/* Hack -- "bounce" to that location */
			y1 = ny; x1 = nx;

			/* Get the cave grid */
			c_ptr = &cave[Depth][ny][nx];

			/* XXX XXX XXX */

			/* Nothing here?  Use it */
			if (!(c_ptr->o_idx)) flag = TRUE;

			/* After trying 99 places, crush any (normal) object */
			else if ((k>99) && cave_valid_bold(Depth, ny, nx)) flag = TRUE;
		}

		/* Hack -- Artifacts will destroy ANYTHING to stay alive */
		if (!flag)
		{
			/* Location */
			ny = y;
			nx = x;

			/* Always okay */
			flag = TRUE;

			/* Description */
			/*object_desc(o_name, o_ptr, FALSE, 0);*/

			/* Message */
			/*msg_format("The %s crashes to the floor.", o_name);*/
		}
	}


	/* Successful drop */
	if (flag)
	{
		/* Assume fails */
		flag = FALSE;

		/* Crush contents or combine two objects? */
		if (!comb) {
			/* Crush anything under us (for artifacts) */
			delete_object(Depth, ny, nx);
	
			/* Make a new object */
			o_idx = o_pop();
	
			/* Success */
			if (o_idx)
			{
				/* Structure copy */
				o_list[o_idx] = *o_ptr;
	
				/* Access */
				o_ptr = &o_list[o_idx];
	
				/* Locate */
				o_ptr->iy = ny;
				o_ptr->ix = nx;
				o_ptr->dun_depth = Depth;
	
				/* Place */
				c_ptr = &cave[Depth][ny][nx];
				c_ptr->o_idx = o_idx;
				
				/* Notify ! */
				flag = TRUE;
			}
		}
		else
		{
			/* Use old object */
			o_idx = c_ptr->o_idx;			
			
			/* Absorb */
			object_absorb(NULL, j_ptr, o_ptr);
			
			/* Notify ! */
			flag = TRUE;
		}
	}
	if (flag) {
			player_type *q_ptr;

			/* Clear visibility flags */
			for (k = 1; k <= NumPlayers; k++)
			{
				/* This player cannot see it */
				Players[k]->obj_vis[o_idx] = FALSE;
			}

			/* Note the spot */
			note_spot_depth(Depth, ny, nx);

			/* Draw the spot */
			everyone_lite_spot(Depth, ny, nx);


			/* Mega-Hack -- no message if "dropped" by player */
			/* Message when an object falls under the player */
			if ((q_ptr = player_on_cave_p(c_ptr)))
			{
				if (chance) msg_print(q_ptr, "You feel something roll beneath your feet.");
				floor_item_notify(q_ptr, o_idx, TRUE);
				sound(q_ptr, MSG_DROP);
			}
	}


	/* Poor little object */
	if (!flag)
	{
		/* Describe */

		/* Message */
		/*
		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("The %s disappear%s.",
		           o_name, ((o_ptr->number == 1) ? "s" : ""));
		*/

		delete_object_ptr(o_ptr);
	}
}




/*
 * Hack -- instantiate a trap
 *
 * XXX XXX XXX This routine should be redone to reflect trap "level".
 * That is, it does not make sense to have spiked pits at 50 feet.
 * Actually, it is not this routine, but the "trap instantiation"
 * code, which should also check for "trap doors" on quest levels.
 */
void pick_trap(int Depth, int y, int x)
{
	int feat;

	cave_type *c_ptr = &cave[Depth][y][x];

	/* Paranoia */
	if (c_ptr->feat != FEAT_INVIS) return;

	/* Pick a trap */
	while (1)
	{
		/* Hack -- pick a trap */
		feat = FEAT_TRAP_HEAD + randint0(16);

		/* Hack -- no trap doors on quest levels */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && is_quest(Depth)) continue;

		/* Hack -- no trap doors on the deepest level */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && (Depth >= MAX_DEPTH-1)) continue;

		/* Done */
		break;
	}

	/* Activate the trap */
	c_ptr->feat = feat;

	/* Notice */
	note_spot_depth(Depth, y, x);

	/* Redraw */
	everyone_lite_spot(Depth, y, x);
}




/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(player_type *p_ptr, int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	if (!object_known_p(p_ptr, o_ptr)) return;

	/* Multiple charges */
	if (o_ptr->pval != 1)
	{
		/* Print a message */
		msg_format(p_ptr, "You have %d charges remaining.", o_ptr->pval);
	}

	/* Single charge */
	else
	{
		/* Print a message */
		msg_format(p_ptr, "You have %d charge remaining.", o_ptr->pval);
	}
}


/*
 * Describe an item in the inventory.
 */
void inven_item_describe(player_type *p_ptr, int item)
{
	object_type	*o_ptr = &p_ptr->inventory[item];

	char	o_name[80];

	/* Get a description */
	object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, 3);

	/* Print a message */
	msg_format(p_ptr, "You have %s.", o_name);
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(player_type *p_ptr, int item, int num)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number and weight */
	if (num)
	{
		/* Add the number */
		o_ptr->number += num;

		/* Add the weight */
		p_ptr->total_weight += (num * o_ptr->weight);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana XXX */
		p_ptr->update |= (PU_MANA);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Redraw this slot */
		p_ptr->redraw_inven |= (1LL << item);
	}

	/* Mega-hack! If an artifact was "decreased", forget ownership */
	if (true_artifact_p(o_ptr) && (num < 0))
	{
		a_info[o_ptr->name1].owner_name = 0;
		a_info[o_ptr->name1].owner_id = 0;
	}

	/* Hack -- redraw fuel items */
	if ((item == INVEN_LITE) && (num < 0))
	{
		player_redraw_fuel_items(p_ptr);
	}
}


/*
 * Erase an inventory slot if it has no more items
 */
void inven_item_optimize(player_type *p_ptr, int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Only optimize real items */
	if (!o_ptr->k_idx) return;

	/* Only optimize empty items */
	if (o_ptr->number) return;

	/* The item is in the pack */
	if (item < INVEN_WIELD)
	{
		int i;

		/* One less item */
		p_ptr->inven_cnt--;

		/* Slide everything down */
		for (i = item; i < INVEN_PACK; i++)
		{
			/* Structure copy */
			p_ptr->inventory[i] = p_ptr->inventory[i+1];

			/* Slot changed */
			p_ptr->redraw_inven |= (1LL << i);
		}

		/* Erase the "final" slot */
		invwipe(&p_ptr->inventory[i]);

		/* Slot changed */
		p_ptr->redraw_inven |= (1LL << i);
	}

	/* The item is being wielded */
	else
	{
		/* One less item */
		p_ptr->equip_cnt--;

		/* Erase the empty slot */
		invwipe(&p_ptr->inventory[item]);

		/* Redraw slot */
		p_ptr->redraw_inven |= (1LL << item);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate torch */
		p_ptr->update |= (PU_TORCH);

		/* Recalculate mana XXX */
		p_ptr->update |= (PU_MANA);
	}

	/* Window stuff */
	p_ptr->window |= (PW_SPELL | PW_PLAYER);
}


/*
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
	object_type *o_ptr = &o_list[item];

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	/*if (!object_known_p(o_ptr)) return;*/

	/* Multiple charges */
	if (o_ptr->pval != 1)
	{
		/* Print a message */
		/*msg_format("There are %d charges remaining.", o_ptr->pval);*/
	}

	/* Single charge */
	else
	{
		/* Print a message */
		/*msg_format("There is %d charge remaining.", o_ptr->pval);*/
	}
}



/*
 * Describe an item in the inventory.
 */
void floor_item_describe(int item)
{
	/* Get a description */
	/*object_desc(o_name, o_ptr, TRUE, 3);*/

	/* Print a message */
	/*msg_format("You see %s.", o_name);*/
}

/*
 * Inform player about item he is standing on
 */
void floor_item_notify(player_type *p_ptr, s16b o_idx, bool force)
{
	object_type *o_ptr;
	char	o_name[80];
	char	o_name_one[80];
	byte ga; char gc;
	byte    attr;
	byte    flag, secondary_tester;
	int i;

	/* XXX HACK XXX -- find OTHER players who also see this item */
	if (o_idx) for (i = 1; i <= NumPlayers; i++)
	{
		if (Players[i]->dun_depth != p_ptr->dun_depth) continue;
		if (Players[i]->obj_vis[o_idx]) Players[i]->window |= (PW_ITEMLIST);
	}

	if (!force && p_ptr->delta_floor_item == o_idx) return;
	p_ptr->delta_floor_item = o_idx;
	
	if (o_idx)
	{
		o_ptr = &o_list[o_idx];

		/* Pick color */
		attr = p_ptr->tval_attr[o_ptr->tval % 128];
		if (!option_p(p_ptr, USE_COLOR)) attr = TERM_WHITE;

		/* Get a/c symbols */
		gc = object_char_p(p_ptr, o_ptr);
		ga = object_attr_p(p_ptr, o_ptr);

		/* Describe the object */
		object_desc(p_ptr, o_name, sizeof(o_name) - 1, o_ptr, TRUE, 3);
		object_desc_one(p_ptr, o_name_one, sizeof(o_name) - 1, o_ptr, FALSE, 0);
		flag = object_tester_flag(p_ptr, o_ptr, &secondary_tester);
		send_floor(p_ptr, ga, gc, attr, o_ptr->number, o_ptr->tval, flag, secondary_tester, o_name, o_name_one);
	}
	else
	{
		send_floor(p_ptr, 0, 0, 0, 0, 0, 0, 0, "", "");
	}
}

/*
 * Increase the "number" of an item on the floor
 */
void floor_item_increase(int item, int num)
{
	object_type *o_ptr = &o_list[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number */
	o_ptr->number += num;
}


/*
 * Optimize an item on the floor (destroy "empty" items)
 */
void floor_item_optimize(int item)
{
	object_type *o_ptr = &o_list[item];

	/* Paranoia -- be sure it exists */
	if (!o_ptr->k_idx) return;

	/* Only optimize empty items */
	if (o_ptr->number) return;

	/* Delete it */
	delete_object_idx(item);
}


/*
 * Check if we're allowed to get rid of an item easily.
 */
bool inven_drop_okay(player_type *p_ptr, object_type *o_ptr)
{
	/* Never drop artifacts above their base depth */
	if (true_artifact_p(o_ptr) && (p_ptr->dun_depth < a_info[o_ptr->name1].level))
	{
		/* Do not apply this rule to ironman and DMs */
		if (!cfg_ironman && !dm_flag_p(p_ptr,ARTIFACT_CONTROL))
		{
			return (FALSE);
		}
	}
	/* If you add more rules here, make sure dm_flag_p(p_ptr,OBJECT_CONTROL)
	 * overrides them ... */
	return (TRUE);
}


/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(player_type *p_ptr, object_type *o_ptr)
{
	int i;

	/* Empty slot? */
	if (p_ptr->inven_cnt < INVEN_PACK) return (TRUE);

	/* Similar slot? */
	for (i = 0; i < INVEN_PACK; i++)
	{
		/* Get that item */
		object_type *j_ptr = &p_ptr->inventory[i];

		/* Check if the two items can be combined */
		if (object_similar(p_ptr, j_ptr, o_ptr)) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}


/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using "object_similar()" and "object_absorb()", otherwise,
 * the item will be placed into the "proper" location in the inventory.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 */
s16b inven_carry(player_type *p_ptr, object_type *o_ptr)
{
	int         i, j, k;
	int		n = -1;

	object_type	*j_ptr;

	/* Hack -- change ownee! */
	object_own(p_ptr, o_ptr);

	/* Check for combining */
	for (j = 0; j < INVEN_PACK; j++)
	{
		j_ptr = &p_ptr->inventory[j];

		/* Skip empty items */
		if (!j_ptr->k_idx) continue;

		/* Hack -- track last item */
		n = j;

		/* Check if the two items can be combined */
		if (object_similar(p_ptr, j_ptr, o_ptr))
		{
			/* Combine the items */
			object_absorb(p_ptr, j_ptr, o_ptr);

			/* Increase the weight */
			p_ptr->total_weight += (o_ptr->number * o_ptr->weight);

			/* Recalculate bonuses */
			p_ptr->update |= (PU_BONUS);

			/* Redraw slot */
			p_ptr->redraw_inven |= (1LL << j);

			/* Window stuff */
			p_ptr->window |= (PW_SPELL | PW_PLAYER);

			/* Success */
			return (j);
		}
	}


	/* Paranoia */
	if (p_ptr->inven_cnt > INVEN_PACK) return (-1);


	/* Find an empty slot */
	for (j = 0; j <= INVEN_PACK; j++)
	{
		j_ptr = &p_ptr->inventory[j];

		/* Use it if found */
		if (!j_ptr->k_idx) break;
	}

	/* Use that slot */
	i = j;


	/* Hack -- pre-reorder the pack */
	if (i < INVEN_PACK)
	{
		s32b		o_value, j_value;

		/* Get the "value" of the item */
		o_value = object_value(p_ptr, o_ptr);

		/* Scan every occupied slot */
		for (j = 0; j < INVEN_PACK; j++)
		{
			j_ptr = &p_ptr->inventory[j];

			/* Use empty slots */
			if (!j_ptr->k_idx) break;

			/* Hack -- readable books always come first */
			if ((o_ptr->tval == p_ptr->cp_ptr->spell_book) &&
			    (j_ptr->tval != p_ptr->cp_ptr->spell_book)) break;
			if ((j_ptr->tval == p_ptr->cp_ptr->spell_book) &&
			    (o_ptr->tval != p_ptr->cp_ptr->spell_book)) continue;

			/* Objects sort by decreasing type */
			if (o_ptr->tval > j_ptr->tval) break;
			if (o_ptr->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_aware_p(p_ptr, o_ptr)) continue;
			if (!object_aware_p(p_ptr, j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o_ptr->sval < j_ptr->sval) break;
			if (o_ptr->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_known_p(p_ptr, o_ptr)) continue;
			if (!object_known_p(p_ptr, j_ptr)) break;

			/* Determine the "value" of the pack item */
			j_value = object_value(p_ptr, j_ptr);

			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Use that slot */
		i = j;

		/* Structure slide (make room) */
		for (k = n; k >= i; k--)
		{
			/* Hack -- Slide the item */
			p_ptr->inventory[k+1] = p_ptr->inventory[k];

			/* Redraw this slot */
			p_ptr->redraw_inven |= (1LL << (k+1));
		}

		/* Paranoia -- Wipe the new slot */
		invwipe(&p_ptr->inventory[i]);

		/* Redraw this slot */
		p_ptr->redraw_inven |= (1LL << i);
	}


	/* Structure copy to insert the new item */
	p_ptr->inventory[i] = (*o_ptr);

	/* Forget monster */ 
	p_ptr->inventory[i].held_m_idx = 0; 

	/* Forget the old location */
	p_ptr->inventory[i].iy = p_ptr->inventory[i].ix = p_ptr->inventory[i].dun_depth = 0;

	/* Increase the weight, prepare to redraw */
	p_ptr->total_weight += (o_ptr->number * o_ptr->weight);

	/* Count the items */
	p_ptr->inven_cnt++;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Reorder pack */
	p_ptr->notice |= (PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_SPELL | PW_PLAYER);

	/* Redraw slot */
	p_ptr->redraw_inven |= (1LL << i);

	/* Return the slot */
	return (i);
}




/*
 * Combine items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void combine_pack(player_type *p_ptr)
{
	int		i, j, k;

	object_type	*o_ptr;
	object_type	*j_ptr;

	bool	flag = FALSE;


	/* Combine the pack (backwards) */
	for (i = INVEN_PACK; i > 0; i--)
	{
		/* Get the item */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty items */
		if (!o_ptr->k_idx) continue;

		/* Scan the items above that item */
		for (j = 0; j < i; j++)
		{
			/* Get the item */
			j_ptr = &p_ptr->inventory[j];

			/* Skip empty items */
			if (!j_ptr->k_idx) continue;

			/* Can we drop "o_ptr" onto "j_ptr"? */
			if (object_similar(p_ptr, j_ptr, o_ptr))
			{
				/* Take note */
				flag = TRUE;

				/* Add together the item counts */
				object_absorb(p_ptr, j_ptr, o_ptr);

				/* One object is gone */
				p_ptr->inven_cnt--;

				/* Slide everything down */
				for (k = i; k < INVEN_PACK; k++)
				{
					/* Structure copy */
					p_ptr->inventory[k] = p_ptr->inventory[k+1];

					/* Redraw slot */
					p_ptr->redraw_inven |= (1LL << k);
				}

				/* Erase the "final" slot */
				invwipe(&p_ptr->inventory[k]);

				/* Redraw slot */
				p_ptr->redraw_inven |= (1LL << k);

				/* Window stuff */
				p_ptr->window |= (PW_SPELL);

				/* Done */
				break;
			}
		}
	}

	/* Message */
	if (flag) msg_print(p_ptr, "You combine some items in your pack.");
}


/*
 * Reorder items in the pack
 *
 * Note special handling of the "overflow" slot
 *
 * Note special handling of empty slots  XXX XXX XXX XXX
 */
void reorder_pack(player_type *p_ptr)
{
	int		i, j, k;

	s32b	o_value;
	s32b	j_value;

	object_type *o_ptr;
	object_type *j_ptr;

	object_type	temp;

	bool	flag = FALSE;


	/* Re-order the pack (forwards) */
	for (i = 0; i < INVEN_PACK; i++)
	{
		/* Mega-Hack -- allow "proper" over-flow */
		if ((i == INVEN_PACK) && (p_ptr->inven_cnt == INVEN_PACK)) break;

		/* Get the item */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Get the "value" of the item */
		o_value = object_value(p_ptr, o_ptr);

		/* Scan every occupied slot */
		for (j = 0; j < INVEN_PACK; j++)
		{
			/* Get the item already there */
			j_ptr = &p_ptr->inventory[j];

			/* Use empty slots */
			if (!j_ptr->k_idx) break;

			/* Hack -- readable books always come first */
			if ((o_ptr->tval == p_ptr->cp_ptr->spell_book) &&
			    (j_ptr->tval != p_ptr->cp_ptr->spell_book)) break;
			if ((j_ptr->tval == p_ptr->cp_ptr->spell_book) &&
			    (o_ptr->tval != p_ptr->cp_ptr->spell_book)) continue;

			/* Objects sort by decreasing type */
			if (o_ptr->tval > j_ptr->tval) break;
			if (o_ptr->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_aware_p(p_ptr, o_ptr)) continue;
			if (!object_aware_p(p_ptr, j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o_ptr->sval < j_ptr->sval) break;
			if (o_ptr->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_known_p(p_ptr, o_ptr)) continue;
			if (!object_known_p(p_ptr, j_ptr)) break;

			/* Determine the "value" of the pack item */
			j_value = object_value(p_ptr, j_ptr);

			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Never move down */
		if (j >= i) continue;

		/* Take note */
		flag = TRUE;

		/* Save the moving item */
		temp = p_ptr->inventory[i];

		/* Structure slide (make room) */
		for (k = i; k > j; k--)
		{
			/* Slide the item */
			p_ptr->inventory[k] = p_ptr->inventory[k-1];

			/* Redraw slot */
			p_ptr->redraw_inven |= (1LL << k);
		}

		/* Insert the moved item */
		p_ptr->inventory[j] = temp;

		/* Redraw slot */
		p_ptr->redraw_inven |= (1LL << j);
	}

	/* Message */
	if (flag) msg_print(p_ptr, "You reorder some items in your pack.");
}


/*
 * Hack -- find out if a player is standing on an object "o_idx", and redraw
 * the floor info. We also update PW_ITEMLIST for each player who is tracking
 * this object.
 */
static void notify_player_standing_on(int o_idx)
{
	player_type *p_ptr;
	object_type *o_ptr;
	int Depth, y, x;
	int Ind;

	/* Get the object */
	o_ptr = &o_list[o_idx];

	/* Get location */
	Depth = o_ptr->dun_depth;
	y = o_ptr->iy;
	x = o_ptr->ix;

	/* Unallocated cave? */
	if (!cave[Depth]) return;

	/* Get the player standing here */
	p_ptr = player_on_cave(Depth, y, x);
	if (p_ptr)
	{
		/* Let's schedule floor redraw */
		p_ptr->redraw |= (PR_FLOOR);
	}

	/* Hack -- also update itemlist for ALL players */
	for (Ind = 1; Ind <= NumPlayers; Ind++)
	{
		p_ptr = Players[Ind];
		if (p_ptr->obj_vis[o_idx]) p_ptr->window |= (PW_ITEMLIST);
	}
}

/*
 * Hack -- process the objects
 */
void process_objects(void)
{
	int i, k;

	object_type *o_ptr;


	/* Hack -- only every ten game turns */
	if ((turn.turn % 10) != 5) return;


	/* Process objects */
	for (k = o_top - 1; k >= 0; k--)
	{
		/* Access index */
		i = o_fast[k];

		/* Access object */
		o_ptr = &o_list[i];

		/* Excise dead objects */
		if (!o_ptr->k_idx)
		{
			/* Excise it */
			o_fast[k] = o_fast[--o_top];

			/* Skip */
			continue;
		}

		/* Recharge rods on the ground */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
		{
			/* Charge it (charge ALL rods) */
			/* Note: this is the behavior in vanilla Angband 3.0.9 */
			o_ptr->timeout -= o_ptr->number;

			/* Alternate code */
			/* Only charging rods should recharge (same as charging from the inventory) */
			/*object_kind* k_ptr = &k_info[o_ptr->k_idx];
			int temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;
			if (temp > o_ptr->number) temp = o_ptr->number;
			o_ptr->timeout -= temp;*/

			/* Boundary control */
			if (o_ptr->timeout < 0) o_ptr->timeout = 0;

			/* Notify player standing on top of that item */
			notify_player_standing_on(i);
		}
	}


#ifdef SHIMMER_OBJECTS

#if 0
	/* Optimize */
	if (!avoid_other && scan_objects)
	{
		/* Process the objects */
		for (i = 1; i < o_max; i++)
		{
			o_ptr = &o_list[i];

			/* Skip dead objects */
			if (!o_ptr->k_idx) continue;

			/* XXX XXX XXX Skip unseen objects */

			/* Shimmer Multi-Hued Objects XXX XXX XXX */
			lite_spot(o_ptr->iy, o_ptr->ix);
		}

		/* Clear the flag */
		scan_objects = FALSE;
	}
#endif

#endif

}

/*
 * Set the "o_idx" fields in the cave array to correspond
 * to the objects in the "o_list".
 */
void setup_objects(void)
{
	int i;

	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip objects on depths that aren't allocated */
		if (!cave[o_ptr->dun_depth]) continue;

		/* Set the o_idx correctly */
		cave[o_ptr->dun_depth][o_ptr->iy][o_ptr->ix].o_idx = i;
	}
}



/* Takes a (partial) item_kind name and returns an index, or 0 if no match
 * was found.
 */
int item_kind_index_fuzzy(char * name)
{
	char match[MAX_CHARS];
	char* str;
	char* dst;
	int i;

	/* Lowercase our search string */
	for(str=name;*str;str++) *str=tolower(*str);

	/* for each item kind race */
	for (i = 1; i <= z_info->k_max; i++)
	{
		/* Clean up it's name */
		dst = match;
		for(str=(k_name + k_info[i].name);*str;str++)
		{
			if (isalpha(*str) || *str==32) *dst++ = tolower(*str);
		}
		*dst++ = '\0';
		/* If cleaned name matches our search string, return it */
		if (strstr(match,name)) { return i; }
	}
	return 0;
}


/* Takes a (partial) item ego name and returns an index, or 0 if no match
 * was found.
 */
int ego_kind_index_fuzzy(char * name)
{
	char match[MAX_CHARS];
	char* str;
	char* dst;
	int i;

	/* Lowercase our search string */
	for(str=name;*str;str++) *str=tolower(*str);

	/* for each ego kind race */
	for (i = 1; i <= z_info->e_max; i++)
	{
		/* Clean up it's name */
		dst = match;
		for(str=(e_name + e_info[i].name);*str;str++)
		{
			if (isalpha(*str) || *str==32) *dst++ = tolower(*str);
		}
		*dst++ = '\0';
		/* If cleaned name matches our search string, return it */
		if (strstr(match,name)) return i;
	}
	return 0;
}

/*
 * Distribute charges of rods, staves, or wands.
 *
 * o_ptr = source item
 * q_ptr = target item, must be of the same type as o_ptr
 * amt   = number of items that are transfered
 */
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt)
{
	/*
	 * Hack -- If rods, staves, or wands are dropped, the total maximum
	 * timeout or charges need to be allocated between the two stacks.
	 * If all the items are being dropped, it makes for a neater message
	 * to leave the original stack's pval alone. -LM-
	 */
	if ((o_ptr->tval == TV_WAND) ||
	    (o_ptr->tval == TV_STAFF) ||
	    (o_ptr->tval == TV_ROD))
	{
		q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

		if (amt < o_ptr->number) o_ptr->pval -= q_ptr->pval;

		/*
		 * Hack -- Rods also need to have their timeouts distributed.
		 *
		 * The dropped stack will accept all time remaining to charge up to
		 * its maximum.
		 */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
		{
			if (q_ptr->pval > o_ptr->timeout)
				q_ptr->timeout = o_ptr->timeout;
			else
				q_ptr->timeout = q_ptr->pval;

			if (amt < o_ptr->number)
				o_ptr->timeout -= q_ptr->timeout;
		}
	}
}


void reduce_charges(object_type *o_ptr, int amt)
{
	/*
	 * Hack -- If rods or wand are destroyed, the total maximum timeout or
	 * charges of the stack needs to be reduced, unless all the items are
	 * being destroyed. -LM-
	 */
	if (((o_ptr->tval == TV_WAND) ||
	     (o_ptr->tval == TV_STAFF) ||
	     (o_ptr->tval == TV_ROD)) &&
	    (amt < o_ptr->number))
	{
		o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;
	}
}

void artifact_notify(player_type *p_ptr, object_type *o_ptr)
{
	/* If this is the first time, log it */
	if (p_ptr->a_info[o_ptr->name1] == ARTS_NOT_FOUND)
	{
		char o_name[80];
		char buf[80];
		object_desc(NULL, o_name, sizeof(o_name), o_ptr, FALSE, 0);
		sprintf(buf, "Found The %s", o_name);
		log_history_event(p_ptr, buf, TRUE);

		/* Mark artifact as found */
		set_artifact_p(p_ptr, o_ptr->name1, ARTS_FOUND);
	}
}

void object_audit(player_type *p_ptr, object_type *o_ptr, int number)
{
	if (o_ptr->owner_id && o_ptr->owner_id != p_ptr->id)
	{
		char o_name[80];
		char buf[512];
		/* Create the object copy (structure copy)
		object_type temp_obj;
		if (o_ptr->number != number)
		{
			object_copy(&temp_obj, o_ptr);
			o_ptr = &temp_obj;
			o_ptr->number = number;
		}
		*/ /* ^ - no need (yet), we cam just temp-adjust number */
		int actual_number = o_ptr->number;
		o_ptr->number = number;
		
		/* Log transaction */
		sprintf(buf,"TR %s-%ld | %s-%ld $ %ld",
			quark_str(o_ptr->owner_name), (long)o_ptr->owner_id,
			p_ptr->name, (long)p_ptr->id, (long)object_value(p_ptr, o_ptr) * number);
		audit(buf);
		/* Object name */
		object_desc(NULL, o_name, sizeof(o_name), o_ptr, TRUE, 3);
		sprintf(buf,"TR+%s", o_name);
		audit(buf);
		
		/* Restore real number */
		o_ptr->number = actual_number;
	}
}

void object_own(player_type *p_ptr, object_type *o_ptr)
{
	/* Log ownership change */
	object_audit(p_ptr, o_ptr, o_ptr->number);

	/* Handle artifacts */
	if (true_artifact_p(o_ptr) && object_known_p(p_ptr, o_ptr))
	{
		artifact_notify(p_ptr, o_ptr);
	}

	/* Store artifact owner */
	if (true_artifact_p(o_ptr))
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];
		a_ptr->owner_id = p_ptr->id;
		a_ptr->owner_name = quark_add(p_ptr->name);
	}

	/* Set original owner ONCE */
	if (o_ptr->origin_player == 0)
	{
		o_ptr->origin_player = quark_add(p_ptr->name);
	}

	/* Set new owner */
	o_ptr->owner_id = p_ptr->id;
	o_ptr->owner_name = quark_add(p_ptr->name);
}

/* Return pointer to a floor object, if any, or NULL */
object_type* player_get_floor_item(player_type *p_ptr, int *idx)
{
	object_type *o_ptr = NULL;
	int o_idx;

	o_idx = cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
	if (o_idx > 0)
	{
		*idx = 0 - o_idx;
		o_ptr = &o_list[o_idx];
	}

	return o_ptr;
}

/*
 * Select an item from an index provided by Player.
 *
 * Note: this function DOES test with item_tester_*
 * framework.
 *
 * Note: if a floor item has been selected, "idx" is changed
 * to reflect it's "o_list" index. For inven/equip items, "idx"
 * is undefined.
 *
 */
object_type* player_get_item(player_type *p_ptr, int item, int *idx)
{
	object_type *o_ptr = NULL;
	int o_idx;

	/* Get the item (in the pack) */
	if (item >= 0 && item < INVEN_TOTAL)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_idx = cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (o_idx == 0)
		{
			msg_print(p_ptr, "There is nothing on the floor.");
			return NULL;
		}
		o_ptr = &o_list[o_idx];

		/* HACK -- invert value */
		*idx = 0 - o_idx;
	}

	/* Perform item test */
	if (!item_tester_okay(o_ptr))
	{
		return NULL;
	}

	return o_ptr;
}

/* Schedule network update for indexed item.
 * If "item" is < 0, we treat it as an o_list[] index.
 * If "item" is >= 0, we treat it as an inventory[] slot index. */
void player_redraw_item(player_type *p_ptr, int item)
{
	/* Inven/equip */
	if (item >= 0) p_ptr->redraw_inven |= (1LL << item);
	/* Floor */
	else p_ptr->redraw |= (PR_FLOOR);
}

/* After changing the lightsource from torch to lantern,
 * update all flags for fuel objects. */
void player_redraw_fuel_items(player_type *p_ptr)
{
	object_type *o_ptr;
	bool fits;
	int item;
	/* Inventory */
	for (item = 0; item < INVEN_WIELD; item++)
	{
		o_ptr = &p_ptr->inventory[item];
		if (object_is_fuel(p_ptr, o_ptr, &fits))
		{
			player_redraw_item(p_ptr, item);
		}
	}
	/* And the floor object */
	if ((o_ptr = player_get_floor_item(p_ptr, &item)))
	{
		if (object_is_fuel(p_ptr, o_ptr, &fits))
		{
			player_redraw_item(p_ptr, FLOOR_INDEX);
		}
	}
}

/*
 * Get the indexes of objects at a given floor location. -TNB-
 *
 * Return the number of object indexes acquired.
 *
 * Valid flags are any combination of the bits:
 *   0x01 -- Verify item tester
 *   0x02 -- Marked/visible items only
 *   0x04 -- Only the top item
 */
int scan_floor(player_type *p_ptr, int *items, int max_size, int y, int x, int mode)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	int Depth = p_ptr->dun_depth;

	/* Sanity */
	if (!in_bounds(p_ptr->dun_depth, y, x)) return 0;


	/* Scan all objects in the grid */
	for (this_o_idx = cave[Depth][y][x].o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* XXX Hack -- Enforce limit */
		if (num >= max_size) break;


		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Item tester */
		if ((mode & 0x01) && !item_tester_okay(o_ptr)) continue;

#if 0 /* We don't support squelch */
		/* Marked */
		if ((mode & 0x02) && (!o_ptr->marked || squelch_hide_item(o_ptr)))
			continue;
#else
		/* MAngband-specific: squelch/mode 0x02 alternative */
		if ((mode & 0x02) && !(p_ptr->obj_vis[this_o_idx])) continue;
#endif

		/* Accept this item */
		items[num++] = this_o_idx;

		/* Only one */
		if (mode & 0x04) break;
	}

	return num;
}



/**
 * Sort comparator for objects using only tval and sval.
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 */
static int compare_types(const object_type *o1, const object_type *o2)
{
	if (o1->tval == o2->tval)
		return CMP(o1->sval, o2->sval);
	else
		return CMP(o1->tval, o2->tval);
}

/* some handy macros for sorting */
#define object_is_known_artifact(p_ptr, o) (artifact_p(o) && object_known_p(p_ptr, o))
#define object_is_worthless(p_ptr, o) (object_value(p_ptr, o/*, 1*/) == 0)

/**
 * Sort comparator for objects
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 *
 * The sort order is designed with the "list items" command in mind.
 */
static int compare_items(player_type *p_ptr, const object_type *o1, const object_type *o2)
{
	/* known artifacts will sort first */
	if (object_is_known_artifact(p_ptr, o1) && object_is_known_artifact(p_ptr, o2))
		return compare_types(o1, o2);
	if (object_is_known_artifact(p_ptr, o1)) return -1;
	if (object_is_known_artifact(p_ptr, o2)) return 1;

	/* unknown objects will sort next */
	if (!object_aware_p(p_ptr, o1) && !object_aware_p(p_ptr, o2))
		return compare_types(o1, o2);
	if (!object_aware_p(p_ptr, o1)) return -1;
	if (!object_aware_p(p_ptr, o2)) return 1;

	/* if only one of them is worthless, the other comes first */
	if (object_is_worthless(p_ptr, (object_type*)o1) && !object_is_worthless(p_ptr, (object_type*)o2)) return 1;
	if (!object_is_worthless(p_ptr, (object_type*)o1) && object_is_worthless(p_ptr, (object_type*)o2)) return -1;

	/* otherwise, just compare tvals and svals */
	/* NOTE: arguably there could be a better order than this */
	return compare_types(o1, o2);
}

/* Prepare some fake objects */
#define MAX_FAKE_OBJECTS 8
int scan_floor_mimics(player_type *p_ptr, int y, int x, int *dst, object_type *o_ptr, int *cnt)
{
	int Depth = p_ptr->dun_depth;
	if (!cave[Depth]) return 0;
	if (cave[Depth][y][x].m_idx > 0)
	{
		monster_type *m_ptr = &m_list[cave[Depth][y][x].m_idx];
		if (m_ptr->mimic_k_idx)
		{
			invwipe(&o_ptr[*cnt]);
			invcopy(&o_ptr[*cnt], m_ptr->mimic_k_idx);
			(*cnt)+=1;
			*dst = -(*cnt);
			return 1;
		}
	}
	return 0;
}

/* Forward-compatibility with V310b */
#define ODESC_FULL 2
#define squelch_item_ok(O_PTR) FALSE

/*
 * Display visible items, similar to display_monlist
 */
void display_itemlist(player_type *p_ptr)
{
	int max;
	int mx, my;
	unsigned num;
	int line = 1, x = 0;
	int cur_x;
	unsigned i;
	unsigned disp_count = 0;
	byte a;
	char c;

	object_type *types[MAX_ITEMLIST];
	int counts[MAX_ITEMLIST];
	unsigned counter = 0;

	int dungeon_hgt = MAX_HGT; /*p_ptr->depth == 0 ? TOWN_HGT : DUNGEON_HGT;*/
	int dungeon_wid = MAX_WID; /*p_ptr->depth == 0 ? TOWN_WID : DUNGEON_WID;*/

	byte attr;
	char buf[80];

	int floor_list[MAX_FLOOR_STACK + 1]; /* +1 for mimics */
	object_type fake_objects[8]; /* Allow up to 8 mimics */
	int mimic_hack = 0;

	/* Begin */
	text_out_init(p_ptr);

	/* Look at each square of the dungeon for items */
	for (my = 0; my < dungeon_hgt; my++)
	{
		for (mx = 0; mx < dungeon_wid; mx++)
		{
			num = scan_floor(p_ptr, floor_list, MAX_FLOOR_STACK, my, mx, 0x02);

			/* Hack -- also add mimics (can't trick the DM, though)*/
			if (mimic_hack < 8 && !(p_ptr->dm_flags & DM_SEE_MONSTERS))
			num += scan_floor_mimics(p_ptr, my, mx, &floor_list[num], fake_objects, &mimic_hack);

			/* Iterate over all the items found on this square */
			for (i = 0; i < num; i++)
			{
				object_type *o_ptr;
				unsigned j;

				if (floor_list[i] < 0)
				{ printf("Use fake object: [%d]-> %d\n",floor_list[i], 0 - floor_list[i] - 1);
					o_ptr = &fake_objects[0 - floor_list[i] - 1];
				}
				else
					o_ptr = &o_list[floor_list[i]];

				/* Skip gold/squelched */
				if (o_ptr->tval == TV_GOLD || squelch_item_ok(o_ptr))
					continue;

				/* See if we've already seen a similar item; if so, just add */
				/* to its count */
				for (j = 0; j < counter; j++)
				{
					if (object_similar(p_ptr, o_ptr, types[j]))
					{
						counts[j] += o_ptr->number;
						break;
					}
				}

				/* We saw a new item. So insert it at the end of the list and */
				/* then sort it forward using compare_items(). The types list */
				/* is always kept sorted. */
				if (j == counter)
				{
					types[counter] = o_ptr;
					counts[counter] = o_ptr->number;

					while (j > 0 && compare_items(p_ptr, types[j - 1], types[j]) > 0)
					{
						object_type *tmp_o = types[j - 1];
						int tmpcount;

						types[j - 1] = types[j];
						types[j] = tmp_o;
						tmpcount = counts[j - 1];
						counts[j - 1] = counts[j];
						counts[j] = tmpcount;
						j--;
					}
					counter++;
					/* Problem: we're out of stack space */
					if (counter >= MAX_ITEMLIST - 1) break;
				}
				/* long breakout */
				if (counter >= MAX_ITEMLIST - 1) break;
			}
			/* long breakout */
			if (counter >= MAX_ITEMLIST - 1) break;
		}
		/* long breakout */
		if (counter >= MAX_ITEMLIST - 1) break;
	}

	/* Note no visible items */
	if (!counter)
	{
		/* Clear display and print note */
		text_out_c(TERM_SLATE, "You see no items.\n");

		/* Done */
		text_out_done();
		return;
	}
	else
	{
		/* Reprint Message */
		text_out(format("You can see %d item%s:",
				   counter, (counter > 1 ? "s" : "")));
		text_out("\n");
	}

	for (i = 0; i < counter; i++)
	{
		/* o_name will hold the object_desc() name for the object. */
		/* o_desc will also need to put a (x4) behind it. */
		/* can there be more than 999 stackable items on a level? */
		char o_name[80];
		char o_desc[86];

		object_type *o_ptr = types[i];

		/* We shouldn't list coins or squelched items */
		if (o_ptr->tval == TV_GOLD || squelch_item_ok(o_ptr))
			continue;

		object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, FALSE, ODESC_FULL);
		if (counts[i] > 1)
			sprintf(o_desc, "%s (x%d)", o_name, counts[i]);
		else
			sprintf(o_desc, "%s", o_name);

		/* Reset position */
		cur_x = x;

		/* Note that the number of items actually displayed */
		disp_count++;

		if (artifact_p(o_ptr) && object_known_p(p_ptr, o_ptr))
			/* known artifact */
			attr = TERM_VIOLET;
		else if (!object_aware_p(p_ptr, o_ptr))
			/* unaware of kind */
			attr = TERM_RED;
		else if (object_value(p_ptr, o_ptr/*, 1*/) == 0)
			/* worthless */
			attr = TERM_SLATE;
		else
			/* default */
			attr = TERM_WHITE;

		a = object_kind_attr_p(p_ptr, o_ptr->k_idx);
		c = object_kind_char_p(p_ptr, o_ptr->k_idx);

		/* Display the pict */
		text_out(" ");
		text_out_c(a, format("%c", c));
		text_out(" ");

		/* Print and bump line counter */
		text_out_c(attr, o_desc);
		if (i == counter - 1) break;
		text_out(" ");
		text_out("\n");
		line++;
	}

	/* Done */
	text_out_done();
}
