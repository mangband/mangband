/* File: cmd6.c */

/* Purpose: Object commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



/*
 * This file includes code for eating food, drinking potions,
 * reading scrolls, aiming wands, using staffs, zapping rods,
 * and activating artifacts.
 *
 * In all cases, if the player becomes "aware" of the item's use
 * by testing it, mark it as "aware" and reward some experience
 * based on the object's level, always rounding up.  If the player
 * remains "unaware", mark that object "kind" as "tried".
 *
 * This code now correctly handles the unstacking of wands, staffs,
 * and rods.  Note the overly paranoid warning about potential pack
 * overflow, which allows the player to use and drop a stacked item.
 *
 * In all "unstacking" scenarios, the "used" object is "carried" as if
 * the player had just picked it up.  In particular, this means that if
 * the use of an item induces pack overflow, that item will be dropped.
 *
 * For simplicity, these routines induce a full "pack reorganization"
 * which not only combines similar items, but also reorganizes various
 * items to obey the current "sorting" method.  This may require about
 * 400 item comparisons, but only occasionally.
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "o_ptr" to
 * no longer point at the correct item, with horrifying results.
 *
 * Note that food/potions/scrolls no longer use bit-flags for effects,
 * but instead use the "sval" (which is also used to sort the objects).
 */






/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(player_type *p_ptr, int item)
{
	int			lev;
	bool ident;

	object_type		*o_ptr;

	/* Check preventive inscription '^E' */
	__trap(p_ptr, CPI(p_ptr, 'E'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost) && !(p_ptr->dm_flags & DM_GHOST_BODY) )
	{
		msg_print(p_ptr, "You cannot eat food!");
		return;
	}

	/* Restrict choices to food */
	item_tester_tval = TV_FOOD;
	item_tester_full = FALSE;
	item_tester_hook = NULL;

	/* Get the item */
	if ( !(o_ptr = player_get_item(p_ptr, item, &item)) )
	{
		/* Paranoia */
		return;
	}

	/* Check guard inscription '!E' */
	__trap(p_ptr, CGI(o_ptr, 'E'));

	/* Sound */
	sound(p_ptr, MSG_EAT);

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Identity not known yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Eat food */
	use_object(p_ptr, o_ptr, item, &ident);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* We have tried it */
	object_tried(p_ptr, o_ptr);

	/* The player is now aware of the object */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Log ownership change (of any) */
	object_audit(p_ptr, o_ptr, 1);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Destroy a food in the pack */
	if (item >= 0)
	{
		inven_item_increase(p_ptr, item, -1);
		inven_item_describe(p_ptr, item);
		inven_item_optimize(p_ptr, item);
	}

	/* Destroy a food on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
		floor_item_notify(p_ptr, 0 - item, TRUE);
	}
}




/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(player_type *p_ptr, int item)
{
	int		lev;
	bool ident;

	object_type	*o_ptr;

	/* Check preventive inscription '^q' */
	__trap(p_ptr, CPI(p_ptr, 'q'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_BODY) )
	{
		msg_print(p_ptr, "You cannot quaff potions!");
		return;
	}

	/* Restrict choices to potions */
	item_tester_tval = TV_POTION;

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

	/* Check guard inscription '!q' */
	__trap(p_ptr, CGI(o_ptr, 'q'));

	if (o_ptr->tval != TV_POTION)
	{
		/* Tried to quaff non-potion */
		return;
	}

	/* Sound */
	sound(p_ptr, MSG_QUAFF);

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Quaff potion */
	use_object(p_ptr, o_ptr, item, &ident);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item has been tried */
	object_tried(p_ptr, o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Log ownership change (of any) */
	object_audit(p_ptr, o_ptr, 1);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Destroy a potion in the pack */
	if (item >= 0)
	{
		inven_item_increase(p_ptr, item, -1);
		inven_item_describe(p_ptr, item);
		inven_item_optimize(p_ptr, item);
	}

	/* Destroy a potion on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
		floor_item_notify(p_ptr, 0 - item, TRUE);
	}
}


/*
 * Read a scroll (from the pack or floor).
 *
 * Certain scrolls can be "aborted" without losing the scroll.  These
 * include scrolls with no effects but recharge or identify, which are
 * cancelled before use.  XXX Reading them still takes a turn, though.
 */
void do_cmd_read_scroll(player_type *p_ptr, int item)
{
	object_type	*o_ptr;
	bool ident;

	/* Check preventive inscription '^r' */
	__trap(p_ptr, CPI(p_ptr, 'r'));

	/* Check some conditions */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(p_ptr, "You cannot read scrolls!");
		return;
	}
	if (p_ptr->blind)
	{
		msg_print(p_ptr, "You can't see anything.");
		return;
	}
	if (no_lite(p_ptr))
	{
		msg_print(p_ptr, "You have no light to read by.");
		return;
	}
	if (p_ptr->confused)
	{
		msg_print(p_ptr, "You are too confused!");
		return;
	}


	/* Restrict choices to scrolls */
	item_tester_tval = TV_SCROLL;

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

	/* Check guard inscription '!r' */
	__trap(p_ptr, CGI(o_ptr, 'r'));

	if (o_ptr->tval != TV_SCROLL)
	{
		/* Tried to read non-scroll */
		return;
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified yet */
	ident = FALSE;

	/* Read the scroll */
	use_object(p_ptr, o_ptr, item, &ident);
}
void do_cmd_read_scroll_end(player_type *p_ptr, int item, bool ident)
{
	object_type		*o_ptr;
	int lev;
	
	/* Get the scroll (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}
	/* Get the scroll (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	} 
  
	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item was tried */
	object_tried(p_ptr, o_ptr);

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* An identification was made */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Log ownership change (of any) */
	object_audit(p_ptr, o_ptr, 1);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Destroy a scroll in the pack */
	if (item >= 0)
	{
		inven_item_increase(p_ptr, item, -1);
		inven_item_describe(p_ptr, item);
		inven_item_optimize(p_ptr, item);
	}
	/* Destroy a scroll on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
		floor_item_notify(p_ptr, 0 - item, TRUE);
	} 

}
void do_cmd_read_scroll_on(player_type *p_ptr, int item, int item2)
{
	p_ptr->command_arg = item2;

	do_cmd_read_scroll(p_ptr, item);
}





/*
 * Use a staff.			-RAK-
 *
 * One charge of one staff disappears.
 *
 * Hack -- staffs of identify can be "cancelled".
 */
void do_cmd_use_staff_pre(player_type *p_ptr, int item, int item2)
{
	p_ptr->command_arg = item2;

	do_cmd_use_staff(p_ptr, item);
}
void do_cmd_use_staff(player_type *p_ptr, int item)
{
	int			chance, lev;
	bool ident;

	object_type		*o_ptr;

	/* Check preventive inscription '^u' */
	__trap(p_ptr, CPI(p_ptr, 'u'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(p_ptr, "You cannot use staves!");
		return;
	}

	/* Restrict choices to wands */
	item_tester_tval = TV_STAFF;

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

	/* Check guard inscription '!u' */
	__trap(p_ptr, CGI(o_ptr, 'u'));

	if (o_ptr->tval != TV_STAFF)
	{
		/* Tried to use non-staff */
		return;
	}

	/* Verify potential overflow */
	/*if ((inven_cnt >= INVEN_PACK) &&
	    (o_ptr->number > 1))
	{*/
		/* Verify with the player */
		/*if (other_query_flag &&
		    !get_check("Your pack might overflow.  Continue? ")) return;
	}*/

	/* Sound */
	sound(p_ptr, MSG_USE_STAFF);

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified yet */
	ident = FALSE;

	/* Extract the item level */
	lev = k_info[o_ptr->k_idx].level;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* Hight level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (randint0(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint1(chance) < USE_DEVICE))
	{
		/*if (flush_failure) flush();*/
		msg_print(p_ptr, "You failed to use the staff properly.");
		return;
	}

	/* Notice empty staffs */
	if (o_ptr->pval <= 0)
	{
		/*if (flush_failure) flush();*/
		if (o_ptr->number == 1)
			msg_print(p_ptr, "The staff has no charges left.");
		else
			msg_print(p_ptr, "The staffs have no charges left.");
		o_ptr->ident |= ID_EMPTY;
		
		/* Redraw target item */
		player_redraw_item(p_ptr, item);

		return;
	}

	/* Default to unknown */
	ident = FALSE;

	/* Use the staff */
	use_object(p_ptr, o_ptr, item, &ident);
}
void do_cmd_use_staff_discharge(player_type *p_ptr, int item, bool ident)
{
	int lev;

	object_type		*o_ptr;
	
	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}
	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Extract the item level */
	lev = k_info[o_ptr->k_idx].level;

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Tried the item */
	object_tried(p_ptr, o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Use a single charge */
	o_ptr->pval--;

	if (!option_p(p_ptr,STACK_ALLOW_WANDS)) 
	{
		/* XXX Hack -- unstack if necessary */
		if ((item >= 0) && (o_ptr->number > 1))
		{
			/* Make a fake item */
			object_type tmp_obj;
			tmp_obj = *o_ptr;
			tmp_obj.number = 1;
			
			distribute_charges(o_ptr, &tmp_obj, 1);			
			
			/* Unstack the used item */
			o_ptr->number--;
			p_ptr->total_weight -= tmp_obj.weight;
			item = inven_carry(p_ptr, &tmp_obj);
	
			/* Message */
			msg_print(p_ptr, "You unstack your staff.");
		}
	}
	
	/* Describe charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(p_ptr, item);

		/* Redraw slot */
		p_ptr->redraw_inven |= (1LL << item);
	}

	/* Describe charges on the floor */
	else
	{
		floor_item_charges(0 - item);

		/* Redraw floor */
		p_ptr->redraw |= (PR_FLOOR);
	}

}

/*
 * Aim a wand (from the pack or floor).
 *
 * Use a single charge from a single item.
 * Handle "unstacking" in a logical manner.
 *
 * For simplicity, you cannot use a stack of items from the
 * ground.  This would require too much nasty code.
 *
 * There are no wands which can "destroy" themselves, in the inventory
 * or on the ground, so we can ignore this possibility.  Note that this
 * required giving "wand of wonder" the ability to ignore destruction
 * by electric balls.
 *
 * All wands can be "cancelled" at the "Direction?" prompt for free.
 *
 * Note that the basic "bolt" wands do slightly less damage than the
 * basic "bolt" rods, but the basic "ball" wands do the same damage
 * as the basic "ball" rods.
 */
void do_cmd_aim_wand(player_type *p_ptr, int item, int dir)
{
	int			lev;
	bool ident;

	object_type		*o_ptr;

	/* Check preventive inscription '^a' */
	__trap(p_ptr, CPI(p_ptr, 'a'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(p_ptr, "You cannot aim wands!");
		return;
	}

	/* Restrict choices to wands */
	item_tester_tval = TV_WAND;

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

	/* Check guard inscription '!a' */
	__trap(p_ptr, CGI(o_ptr, 'a'));

	if (o_ptr->tval != TV_WAND)
	{
		/* Tried to aim non-wand */
		return;
	}

	/* Hack -- verify potential overflow */
	/*if ((inven_cnt >= INVEN_PACK) &&
	    (o_ptr->number > 1))
	{*/
		/* Verify with the player */
	/*	if (other_query_flag &&
		    !get_check("Your pack might overflow.  Continue? ")) return;
	}*/


	/* Allow direction to be cancelled for free */
	/*if (!get_aim_dir(&dir)) return;*/


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified yet */
	ident = FALSE;

	/* Get the level */
	lev = k_info[o_ptr->k_idx].level;

	/* Aim the wand */
	p_ptr->command_dir = dir; 
	if (!use_object(p_ptr, o_ptr, item, &ident)) return;	


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Mark it as tried */
	object_tried(p_ptr, o_ptr);

	/* Apply identification */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);


	/* Use a single charge */
	o_ptr->pval--;

	if (!option_p(p_ptr,STACK_ALLOW_WANDS)) {
		/* Hack -- unstack if necessary */
		if ((item >= 0) && (o_ptr->number > 1))
		{
			/* Make a fake item */
			object_type tmp_obj;
			tmp_obj = *o_ptr;
			tmp_obj.number = 1;
	
			distribute_charges(o_ptr, &tmp_obj, 1);	
				
			/* Unstack the used item */
			o_ptr->number--;
			p_ptr->total_weight -= tmp_obj.weight;
			item = inven_carry(p_ptr, &tmp_obj);
	
			/* Message */
			msg_print(p_ptr, "You unstack your wand.");
		}
	}
	
	/* Describe the charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(p_ptr, item);

		/* Redraw slot */
		p_ptr->redraw_inven |= (1LL << item);
	}

	/* Describe the charges on the floor */
	else
	{
		floor_item_charges(0 - item);

		/* Redraw floor */
		p_ptr->redraw |= (PR_FLOOR);
	}
}





/*
 * Activate (zap) a Rod
 *
 * Unstack fully charged rods as needed.
 *
 * Hack -- rods of perception/genocide can be "cancelled"
 * All rods can be cancelled at the "Direction?" prompt
 */
void do_cmd_zap_rod_pre(player_type *p_ptr, int item, int dir)
{
	if (dir < 0 && dir > -11)
		p_ptr->command_dir = -dir;
	else
		p_ptr->command_arg = dir;

	do_cmd_zap_rod(p_ptr, item);
}
void do_cmd_zap_rod(player_type *p_ptr, int item)
{
	bool ident;
	object_type		*o_ptr;

	/* Check preventive inscription '^z' */
	__trap(p_ptr, CPI(p_ptr, 'z'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(p_ptr, "You cannot zap rods!");
		return;
	}

	/* Restrict choices to rods */
	item_tester_tval = TV_ROD;

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

	/* Check guard inscription '!z' */
	__trap(p_ptr, CGI(o_ptr, 'z'));

	if (o_ptr->tval != TV_ROD)
	{
		/* Tried to zap non-rod */
		return;
	}

	/* Identity not known yet */
	ident = FALSE;

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Zap the rod */
	use_object(p_ptr, o_ptr, item, &ident);
	
	/* Redraw rod */
	player_redraw_item(p_ptr, item);
}
void do_cmd_zap_rod_discharge(player_type *p_ptr, int item, bool ident)
{
	int                 lev;

	object_type		*o_ptr;
	object_kind		*k_ptr;

	item = p_ptr->current_object;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}
	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Extract the item level */
	k_ptr = &k_info[o_ptr->k_idx];
	lev = k_ptr->level;

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Tried the object */
	object_tried(p_ptr, o_ptr);

	/* Successfully determined the object function */
	if (ident && !object_aware_p(p_ptr, o_ptr))
	{
		object_aware(p_ptr, o_ptr);
		gain_exp(p_ptr, (lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Redraw target item */
	player_redraw_item(p_ptr, item);

	/* Drain the charge */
	/* happens in use-obj.c
	o_ptr->timeout += k_ptr->pval;
	*/
	
	if (!option_p(p_ptr,STACK_ALLOW_WANDS)) {
		/* XXX Hack -- unstack if necessary */
		if ((item >= 0) && (o_ptr->number > 1))
		{
			/* Make a fake item */
			object_type tmp_obj;
			tmp_obj = *o_ptr;
			tmp_obj.number = 1;
			
			distribute_charges(o_ptr, &tmp_obj, 1);

			/* Unstack the used item */
			o_ptr->number--;
			p_ptr->total_weight -= tmp_obj.weight;
			item = inven_carry(p_ptr, &tmp_obj);

			/* Message */
			msg_print(p_ptr, "You unstack your rod.");
		}
	}
}



/*
 * Hook to determine if an object is activatable
 */
static bool item_tester_hook_activate(player_type *p_ptr, object_type *o_ptr)
{
	u32b f1, f2, f3;

	/* Not known */
	if (!object_known_p(p_ptr, o_ptr)) return (FALSE);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Check activation flag */
	if (f3 & TR3_ACTIVATE) return (TRUE);

	/* Assume not */
	return (FALSE);
}



/*
 * Activate a wielded object.  Wielded objects never stack.
 * And even if they did, activatable objects never stack.
 *
 * Currently, only (some) artifacts, and Dragon Scale Mail, can be activated.
 * But one could, for example, easily make an activatable "Ring of Plasma".
 *
 * Note that it always takes a turn to activate an artifact, even if
 * the user hits "escape" at the "direction" prompt.
 */
void do_cmd_activate(player_type *p_ptr, int item)
{
	int lev, chance;
	bool ident;
	object_type *o_ptr;

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_BODY) )
	{
		msg_print(p_ptr, "You cannot activate items!");
		return;
	}

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

	/* Check guard inscription '!A' */
	__trap(p_ptr, CGI(o_ptr, 'A'));

	/* Test the item */
	if (!item_tester_hook_activate(p_ptr, o_ptr))
	{
		msg_print(p_ptr, "You cannot activate that item.");
		return;
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Extract the item level */
	lev = k_info[o_ptr->k_idx].level;

	/* Hack -- use artifact level instead */
	if (artifact_p(o_ptr)) lev = artifact_ptr(o_ptr)->level;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (randint0(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint1(chance) < USE_DEVICE))
	{
		/*if (flush_failure) flush();*/
		msg_print(p_ptr, "You failed to activate it properly.");
		return;
	}

	/* Sound */	
	sound(p_ptr, MSG_ACT_ARTIFACT);

	/* Activate the object */
	(void)use_object(p_ptr, o_ptr, item, &ident);

	/* Redraw this item */
	player_redraw_item(p_ptr, item);
}
void do_cmd_activate_dir(player_type *p_ptr, int item, int dir)
{
	if (dir > 0)
		p_ptr->command_dir = dir;

	do_cmd_activate(p_ptr, item);
}
/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_refill_potion(player_type *p_ptr, int item)
{
	bool ident, good = FALSE;

	object_type	*o_ptr;
	object_type	forge;

	int new_tval, new_sval;

	/* Check preventive inscription '^G' */
	__trap(p_ptr, CPI(p_ptr, 'G'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_BODY) )
	{
		msg_print(p_ptr, "You cannot refill potions!");
		return;
	}

	/* Restrict choices to empty bottles */
	item_tester_tval = TV_BOTTLE;

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

	/* Check guard inscription '!G' */
	__trap(p_ptr, CGI(o_ptr, 'G'));

	if (o_ptr->tval != TV_BOTTLE)
	{
		/* Tried to refill non-bottle */
		return;
	}

	/* Verify our feet are wet (standing in water) */
	if (cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].feat == FEAT_WATER)
	{
		/* Better chance result is water bottle */
		if (randint0(100) < 70)
			good = TRUE;
	}
	else if (cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].feat == FEAT_MUD)
	{
		/* Better chance result is bad */
		if (randint0(100) < 30)
			good = TRUE;
	}
	else /* No water with which to fill bottle */
		return;

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified */
	ident = FALSE;

	/* Object Changes from bottle to potion */
	new_tval = TV_POTION;
	new_sval = SV_POTION_SLIME_MOLD; /* some default */
	if (good)
	{
		if (randint0(100) < 5)
			new_sval = SV_POTION_CURE_CRITICAL;
		else
			new_sval = SV_POTION_WATER;
	}
	else
	{
		if (randint0(100) < 50)
			new_sval = SV_POTION_SLIME_MOLD;
		else if (randint0(100) < 50)
			new_sval = SV_POTION_SALT_WATER;
		else if (randint0(100) < 50)
			new_sval = SV_POTION_SLEEP;
		else if (randint0(100) < 50)
			new_sval = SV_POTION_CONFUSION;
		else if (randint0(100) < 30)
			new_sval = SV_POTION_BLINDNESS;
		else if (randint0(100) < 50)
			new_sval = SV_POTION_POISON;
		else if (randint0(100) < 70)
			new_sval = SV_POTION_LOSE_MEMORIES;
		else
			new_sval = SV_POTION_DEATH;
	}

	msg_print(p_ptr, "The bottle shimmers and changes color.");

	/* Destroy the bottle in the pack */
	if (item >= 0)
	{
		inven_item_increase(p_ptr, item, -1);
		inven_item_describe(p_ptr, item);
		inven_item_optimize(p_ptr, item);
	}
	/* Destroy the bottle on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
		floor_item_notify(p_ptr, 0 - item, TRUE);
	}

	/* Create and add new item */
	/* XXX is calling those enough, or should we also do
	 * something else? */
	invcopy(&forge, lookup_kind(new_tval, new_sval));
	inven_carry(p_ptr, &forge);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}
