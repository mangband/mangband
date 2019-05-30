/* File: cmd2.c */

/* Purpose: Movement commands (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



/*
 * Go up one level					-RAK-
 */
void do_cmd_go_up(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type *c_ptr;

	/* Make sure he hasn't just changed depth */
	if (p_ptr->new_level_flag)
		return;

	/* Check preventive inscription '^<' */	
	__trap(Ind, CPI(p_ptr, '<'));

	/* Player grid */
	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];

	/* Verify stairs if not a ghost, or admin wizard */
	if (!p_ptr->ghost && c_ptr->feat != FEAT_LESS)
	{
		msg_print(Ind, "I see no up staircase here.");
		return;
	}
	else
	{	
		if (p_ptr->dun_depth <= 0)
		{
			msg_print(Ind, "There is nothing above you.");
			return;
		}
	}

	if (cfg_ironman)
	{
		/*
		 * Ironmen don't go up
		 */
		if(!is_dm_p(p_ptr))
		{
			msg_print(Ind, "Morgoth awaits you in the darkness below.");
			return;
		}
	}

	/* Remove the player from the old location */
	c_ptr->m_idx = 0;

	/* Show everyone that's he left */
	everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);

	/* Tell everyone to re-calculate visiblity for this player */
	update_player(Ind);

	/* Forget his lite and viewing area */
	forget_lite(Ind);
	forget_view(Ind);

	/* Hack -- take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Success */
	if (c_ptr->feat == FEAT_LESS)
	{
		msg_print(Ind, "You enter a maze of up staircases.");
		sound(Ind, MSG_STAIRS_UP);
		p_ptr->new_level_method = LEVEL_UP;
	}
	else
	{
		msg_print(Ind, "You float upwards.");
		p_ptr->new_level_method = LEVEL_GHOST;
	}

	/* A player has left this depth */
	players_on_depth[p_ptr->dun_depth]--;

	/* Go up the stairs */
	p_ptr->dun_depth--;

	/* And another player has entered this depth */
	players_on_depth[p_ptr->dun_depth]++;

	p_ptr->new_level_flag = TRUE;

	/* Create a way back */
	create_down_stair = TRUE;
}


/*
 * Go down one level
 */
void do_cmd_go_down(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type *c_ptr;

	/* Make sure he hasn't just changed depth */
	if (p_ptr->new_level_flag)
		return;

	/* Check preventive inscription '^>' */
	__trap(Ind, CPI(p_ptr, '>'));

	/* Player grid */
	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];

	/* Verify stairs */
	if (!p_ptr->ghost && c_ptr->feat != FEAT_MORE)
	{
		msg_print(Ind, "I see no down staircase here.");
		return;
	}
	else
	{
		/* No ghost diving unless DM or allowed in config */
		if (p_ptr->ghost && !cfg_ghost_diving && !is_dm_p(p_ptr)) {
			msg_print(Ind, "You seem unable to go down.  Try going up.");
			return;
		};
		
		/* Can't go down on a "quest" level */
		if (is_quest_level(Ind, p_ptr->dun_depth))
		{
			/* Inform */
			msg_print(Ind, "An unvanquished adversary pulls you back, you can descend no further.");
			return; 
		}
		
		/* Can't go down in the wilderness */
		if (p_ptr->dun_depth < 0)
		{
			msg_print(Ind, "There is nothing below you.");
			return;
		}
	
		/* Verify maximum depth */
		if (p_ptr->dun_depth >= 127)
		{
			msg_print(Ind, "You are at the bottom of the dungeon.");
			return;
		}
	}

	/* Remove the player from the old location */
	c_ptr->m_idx = 0;

	/* Show everyone that's he left */
	everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);

	/* Tell everyone to re-calculate visiblity for this player */
	update_player(Ind);

	/* Forget his lite and viewing area */
	forget_lite(Ind);
	forget_view(Ind);

	/* Hack -- take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Success */
	if (c_ptr->feat == FEAT_MORE)
	{
		msg_print(Ind, "You enter a maze of down staircases.");
		sound(Ind, MSG_STAIRS_DOWN);
		p_ptr->new_level_method = LEVEL_DOWN;
	}
	else
	{
		msg_print(Ind, "You float downwards.");
		p_ptr->new_level_method = LEVEL_GHOST;
	}

	/* A player has left this depth */
	players_on_depth[p_ptr->dun_depth]--;

	/* Go down */
	p_ptr->dun_depth++;

	/* Another player has entered this depth */
	players_on_depth[p_ptr->dun_depth]++;

	p_ptr->new_level_flag = TRUE;

	/* Create a way back */
	create_up_stair = TRUE;
}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		/*command_rep = command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Search */
	search(Ind);
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Stop searching */
	if (p_ptr->searching)
	{
		/* Clear the searching flag */
		p_ptr->searching = FALSE;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Start searching */
	else
	{
		/* Set the searching flag */
		p_ptr->searching = TRUE;

		/* Update stuff */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_SPEED);
	}
}



/*
 * Allocates objects upon opening a chest    -BEN-
 *
 * Disperse treasures from the chest "o_ptr", centered at (x,y).
 *
 * Small chests often contain "gold", while Large chests always contain
 * items.  Wooden chests contain 2 items, Iron chests contain 4 items,
 * and Steel chests contain 6 items.  The "value" of the items in a
 * chest is based on the "power" of the chest, which is in turn based
 * on the level on which the chest is generated.
 */
static void chest_death(int Ind, int y, int x, object_type *o_ptr)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	int		i, d, ny, nx;
	int		number, small_;


	/* Must be a chest */
	if (o_ptr->tval != TV_CHEST) return;

	/* Small chests often hold "gold" */
	small_ = (o_ptr->sval < SV_CHEST_MIN_LARGE);

	/* Determine how much to drop (see above) */
	number = (o_ptr->sval % SV_CHEST_MIN_LARGE) * 2;

	/* Generate some treasure */
	if (o_ptr->pval && (number > 0))
	{
		/* Drop some objects (non-chests) */
		for (; number > 0; --number)
		{
			/* Try 20 times per item */
			for (i = 0; i < 20; ++i)
			{
				/* Pick a distance */
				d = ((i + 15) / 15);

				/* Pick a location */
				scatter(Depth, &ny, &nx, y, x, d, 0);

				/* Must be a clean floor grid */
				if (!cave_clean_bold(Depth, ny, nx)) continue;

				/* Opening a chest */
				opening_chest = TRUE;

				/* Determine the "value" of the items */
				object_level = ABS(o_ptr->pval) + 10;

				/* Small chests often drop gold */
				if (small_ && (rand_int(100) < 75))
				{
					place_gold(Depth, ny, nx);
				}

				/* Otherwise drop an item */
				else
				{
					place_object(Depth, ny, nx, FALSE, FALSE, 0);
				}

				/* Reset the object level */
				object_level = Depth;

				/* No longer opening a chest */
				opening_chest = FALSE;

				/* Notice it */
				note_spot(Ind, ny, nx);

				/* Display it */
				everyone_lite_spot(Depth, ny, nx);

				/* Under the player */
				if (cave[Depth][y][x].m_idx < 0) 
				{
					msg_print(0 - cave[Depth][y][x].m_idx, "You feel something roll beneath your feet.");
					floor_item_notify(0 - cave[Depth][y][x].m_idx, cave[Depth][y][x].o_idx, TRUE);
				}

				/* Successful placement */
				break;
			}
		}
	}

	/* Empty */
	o_ptr->pval = 0;

	/* Known */
	object_known(o_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int Ind, int y, int x, object_type *o_ptr)
{
	player_type *p_ptr = Players[Ind];

	int  i, trap;


	/* Only analyze chests */
	if (o_ptr->tval != TV_CHEST) return;

	/* Ignore disarmed chests */
	if (o_ptr->pval <= 0) return;

	/* Obtain the traps */
	trap = chest_traps[o_ptr->pval];

	/* Lose strength */
	if (trap & CHEST_LOSE_STR)
	{
		msg_print(Ind, "A small needle has pricked you!");
		take_hit(Ind, damroll(1, 4), "a poison needle");
		(void)do_dec_stat(Ind, A_STR);
	}

	/* Lose constitution */
	if (trap & CHEST_LOSE_CON)
	{
		msg_print(Ind, "A small needle has pricked you!");
		take_hit(Ind, damroll(1, 4), "a poison needle");
		(void)do_dec_stat(Ind, A_CON);
	}

	/* Poison */
	if (trap & CHEST_POISON)
	{
		msg_print(Ind, "A puff of green gas surrounds you!");
		if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
		{
			(void)set_poisoned(Ind, p_ptr->poisoned + 10 + randint(20));
		}
	}

	/* Paralyze */
	if (trap & CHEST_PARALYZE)
	{
		msg_print(Ind, "A puff of yellow gas surrounds you!");
		if (!p_ptr->free_act)
		{
			(void)set_paralyzed(Ind, p_ptr->paralyzed + 10 + randint(20));
		}
	}

	/* Summon monsters */
	if (trap & CHEST_SUMMON)
	{
		int num = 2 + randint(3);
		msg_print(Ind, "You are enveloped in a cloud of smoke!");
		sound(Ind, MSG_SUM_MONSTER);
		for (i = 0; i < num; i++)
		{
			(void)summon_specific(p_ptr->dun_depth, y, x, p_ptr->dun_depth, 0);
		}
	}

	/* Explode */
	if (trap & CHEST_EXPLODE)
	{
		msg_print(Ind, "There is a sudden explosion!");
		msg_print(Ind, "Everything inside the chest is destroyed!");
		o_ptr->pval = 0;
		take_hit(Ind, damroll(5, 8), "an exploding chest");
	}
}


/*
 * Return the index of a house given an coordinate pair
 */
int pick_house(int Depth, int y, int x)
{
	int i;

	/* Check each house */
	for (i = 0; i < num_houses; i++)
	{
		/* Check this one */
		if (houses[i].door_x == x && houses[i].door_y == y && houses[i].depth == Depth)
		{
			/* Return */
			return i;
		}
	}

	/* Failure */
	return -1;
}

/*
 * Determine if the player is inside the house
 */
bool house_inside(int Ind, int house)
{
	player_type *p_ptr = Players[Ind];

	if (house >= 0 && house < num_houses)
	{
		if (houses[house].depth == p_ptr->dun_depth
			&& p_ptr->px >= houses[house].x_1 && p_ptr->px <= houses[house].x_2 
			&& p_ptr->py >= houses[house].y_1 && p_ptr->py <= houses[house].y_2)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Determine if the given house is owned
 */
bool house_owned(int house)
{
	if (house >= 0 && house < num_houses)
	{
		if (houses[house].owned[0])
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Determine if the given player owns the given house
 */
bool house_owned_by(int Ind, int house)
{
	player_type *p_ptr = Players[Ind];

	/* If not owned at all, obviously not owned by this player */
	if (!house_owned(house)) return FALSE;
	
	/* It's owned, is it by this player */
	if (!strcmp(p_ptr->name,houses[house].owned))
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Return the number of houses owned by the player
 */
int houses_owned(int Ind)
{
	int i;
	int owned = 0;

	/* Check each house */
	for (i = 0; i < num_houses; i++)
	{
		if(house_owned_by(Ind,i))
		{
			owned++;
		}
	}

	return owned;
}

/*
 * Given coordinates return a house to which they belong.
 * Houses can be overlapping, so a single coordinate pair may match several
 * houses.  The offset parameter allows searching for the next match.
 */
int find_house(int Ind, int x, int y, int offset)
{
	player_type *p_ptr = Players[Ind];
	int i;
	
	for (i = offset; i < num_houses; i++)
	{
		/* Check the house position *including* the walls */
		if (houses[i].depth == p_ptr->dun_depth
			&& x >= houses[i].x_1-1 && x <= houses[i].x_2+1
			&& y >= houses[i].y_1-1 && y <= houses[i].y_2+1)
		{
			/* We found the house this section of wall belongs to */
			return i;
		}
	}
	return -1;
}

/*
 * Determine if the given location is ok to use as part of the foundation
 * of a house.
 */
bool is_valid_foundation(int Ind, int x, int y)
{
	int house;
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;
	object_type	*o_ptr;

	/* Foundation stones are always valid */
	o_ptr = &o_list[cave[p_ptr->dun_depth][y][x].o_idx];
	if (o_ptr->tval == TV_JUNK && o_ptr->sval == SV_HOUSE_FOUNDATION)
	{
		return TRUE;
	}

	/* Perma walls and doors are valid if they are part of a house owned 
	 * by this player */
	c_ptr = &cave[p_ptr->dun_depth][y][x];
	if( (c_ptr->feat == FEAT_PERM_EXTRA)
		|| (c_ptr->feat >= FEAT_HOME_HEAD && c_ptr->feat <= FEAT_HOME_TAIL))
	{
		/* Looks like part of a house, which house? */
		house = find_house(Ind, x, y, 0);
		if(house >= 0)
		{
			/* Do we own this house? */
			if(house_owned_by(Ind,house))
			{
				/* Valid, a wall or door in our own house. */
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * Create a new house door at the given location.
 * Due to the fact that houses can overlap (i.e. share a common wall) it
 * may not be possible to identify the house to which the door should belong.
 * 
 * For example, on the left below, we have two houses overlapping, neither
 * have doors. On the right the player creates a door, but to which house does
 * it belong?
 * 
 *   ####                        ####    
 *   #  #@                       #  #@  
 *   #  ###                      #  +##
 *   #### #                      #### #
 *      # #                         # #
 *      ###                         ###
 * 
 * It is therefore possible to create a complex of houses such that the player
 * owned shop mechanism becomes confused.  When a player bumps one door they
 * see the contents of a different room listed.
 * 
 * FIXME Therefore the player owned shop mechanism should treat overlapping 
 * player created houses as a *single* house and present all goods in all
 * attached houses.
 */
bool create_house_door(int Ind, int x, int y)
{
	int house, i, lastmatch;
	cave_type		*c_ptr;
	player_type *p_ptr = Players[Ind];

	/* Which house is the given location part of? */
	lastmatch = 0;
	while( (house = find_house(Ind,x,y,lastmatch)) > -1 )
	{
		lastmatch = house+1;

		/* Do we own this house? */
		if(!house_owned_by(Ind,house))
		{
			/* If we don't own this one, we can't own any overlapping ones */
			msg_print(Ind, "You do not own this house");
			return FALSE;
		}

		/* Does it already have a door? */
		if(houses[house].door_y == 0 && houses[house].door_x == 0)
		{
			/* No door, so create one! */
			houses[house].door_y = y;
			houses[house].door_x = x;
			c_ptr = &cave[p_ptr->dun_depth][y][x];
			c_ptr->feat = FEAT_HOME_HEAD;
			everyone_lite_spot(p_ptr->dun_depth, y, x);
			msg_print(Ind, "You create a door for your house!");
			return TRUE;
		}
	}
	/* We searched all matching houses and none needed a door */
	return FALSE;
}

/*
 * Determine the area for a house foundation.
 * 
 * Although an individual house must be rectangular, a foundation
 * can be non-rectangular.  This is because we allow existing walls to 
 * form part of our foundation, and therefore allow complex shaped houses
 * to be consructed.
 *                                              ~~~
 * For example this is a legal foundation:   ~~~~~~
 *                                           ~~~~~~
 * In this sitation:
 * 
 *   #####                               #####   
 *   #   #                               #   #
 *   #   #      Forming a final shape:   #   #
 *   #####~~~                            ###+####
 *     ~~~~~~                              #    #
 *     ~~~~~~                              ######
 * 
 * This function is also responsible for rejecting illegal shapes and sizes.
 * 
 * We start from the player location (who must be stood on a foundation stone)
 * and work our way outwards to find the bounding rectange of the foundation.
 * Conceptually imagine a box radiating out from the player, we keep extending
 * the box in each dimension for as long as all points on the perimeter are
 * either foundation stones or walls of houses the player owns.
 * 
 */
bool get_house_foundation(int Ind, int *px1, int *py1, int *px2, int *py2)
{
	player_type *p_ptr = Players[Ind];
	int x, y, x1, y1, x2, y2;
	bool done;
	bool n,s,e,w,ne,nw,se,sw;
	object_type	*o_ptr;

	plog(format("Player is at x,y %d, %d",p_ptr->px, p_ptr->py));

	/* We must be stood on a house foundation */
	o_ptr = &o_list[cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx];
	if (o_ptr->tval != TV_JUNK || o_ptr->sval != SV_HOUSE_FOUNDATION)
	{
		msg_print(Ind, "There is no house foundation here.");
		return FALSE;
	}
	
	/* Start from the players position */
	x1 = p_ptr->px;
	x2 = p_ptr->px;
	y1 = p_ptr->py;
	y2 = p_ptr->py;
	
	done = FALSE;
	while(!done)
	{
		n = s = e = w = ne = nw = se = sw = FALSE;

		/* Could we expand north? */
		n = TRUE;
		for(x = x1; x <= x2; x++)
		{
			/* Is this a valid location for part of our house? */
			if(!is_valid_foundation(Ind, x, y1-1))
			{
				/* Not a valid perimeter */
				n = FALSE; 
				break;
			}
		}

		/* Could we expand east? */
		e = TRUE;
		for(y = y1; y <= y2; y++)
		{
			/* Is this a valid location for part of our house? */
			if(!is_valid_foundation(Ind, x2+1, y))
			{
				/* Not a valid perimeter */
				e = FALSE;
				break;
			}
		}

		/* Could we expend south? */
		s = TRUE;
		for(x = x1; x <= x2; x++)
		{
			/* Is this a valid location for part of our house? */
			if(!is_valid_foundation(Ind, x, y2+1))
			{
				/* Not a valid perimeter */
				s = FALSE;
				break;
			}
		}

		/* Could we expand west? */
		w = TRUE;
		for(y = y1; y <= y2; y++)
		{
			/* Is this a valid location for part of our house? */
			if(!is_valid_foundation(Ind, x1-1, y))
			{
				/* Not a valid perimeter */
				w = FALSE;
				break;
			}
		}

		/* Could we expand the corners? */
		ne = is_valid_foundation(Ind, x2+1, y1-1);
		nw = is_valid_foundation(Ind, x1-1, y1-1);
		se = is_valid_foundation(Ind, x2+1, y2+1);
		sw = is_valid_foundation(Ind, x1-1, y2+1);

		/* Only permit expansion in a way that maintains a rectangle, we don't
		 * want to create fancy polygons. */
		if( n ) n = (!e && !w) || ( e && ne ) || ( w && nw );
		if( e ) e = (!n && !s) || ( n && ne ) || ( s && se );
		if( s ) s = (!e && !w) || ( e && se ) || ( w && sw );
		if( w ) w = (!n && !s) || ( n && nw ) || ( s && sw );

		/* Actually expand the boundary */
		if( n ) y1--;
		if( s ) y2++;
		if( w ) x1--;
		if( e ) x2++;

		/* Stop if we couldn't expand */
		done = !(n || s || w || e);

	}

	plog(format("Proposed house is at x1,y1,x2,y2 %d,%d,%d,%d",x1,y1,x2,y2));
	
	/* Is the bounding rectangle we found big enough? */
	if(x2-x1 < 2 || y2-y1 < 2)
	{
		msg_print(Ind, "The foundation is too small.");
		return FALSE;
	}

	/* Return the area */
	*px1 = x1;
	*px2 = x2;
	*py1 = y1;
	*py2 = y2;
	return TRUE;
}

/*
 * Create a new house.
 * The creating player owns the house.
 */
bool create_house(int Ind)
{
	int x1, x2, y1, y2, x, y;
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;

	/* Not in dungeon, not in town */
	if (p_ptr->dun_depth >= 0 || check_special_level(p_ptr->dun_depth))
	{
		msg_print(Ind, "The surrounding magic is too strong for House Creation.");
		return FALSE;
	}

	/* Determine the area of the house foundation */
	if(!get_house_foundation(Ind,&x1,&y1,&x2,&y2))
	{
		return FALSE;
	}

	/* Is the location allowed? */
	/* XXX We should check if too near other houses, roads, level edges, etc */

	/* Add a house to our houses list */
	houses[num_houses].price = 0;	/* XXX */
	houses[num_houses].x_1 = x1+1;
	houses[num_houses].y_1 = y1+1;
	houses[num_houses].x_2 = x2-1;
	houses[num_houses].y_2 = y2-1;
	houses[num_houses].depth = p_ptr->dun_depth;
	houses[num_houses].door_y = 0;
	houses[num_houses].door_x = 0;
	set_house_owner(Ind, num_houses);
	num_houses++;

	/* Render into the terrain */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Get the grid */
			c_ptr = &cave[p_ptr->dun_depth][y][x];

			/* Delete any object */
			delete_object(p_ptr->dun_depth, y, x);

			/* Build a wall, but don't destroy any existing door */
			if( c_ptr->feat < FEAT_HOME_HEAD || c_ptr->feat > FEAT_HOME_TAIL)
			{
				c_ptr->feat = FEAT_PERM_EXTRA;
			}
			
			/* Update the spot */
			everyone_lite_spot(p_ptr->dun_depth, y, x);	
		}
	}		
	for (y = y1 + 1; y < y2; y++)
	{
		for (x = x1 + 1; x < x2; x++)
		{
			/* Get the grid */
			c_ptr = &cave[p_ptr->dun_depth][y][x];

			/* Delete any object */
			delete_object(p_ptr->dun_depth, y, x);

			/* Fill with floor */
			c_ptr->feat = FEAT_FLOOR;

			/* Make it "icky" */
			c_ptr->info |= CAVE_ICKY;

			/* Update the spot */
			everyone_lite_spot(p_ptr->dun_depth, y, x);
		}
	}
	return TRUE;
}

/*
 * Set the owner of the given house
 */
bool set_house_owner(int Ind, int house)
{
	player_type *p_ptr = Players[Ind];

	/* Not if it's already owned */
	if (house_owned(house)) return FALSE;

	/* Set the player as the owner */
	my_strcpy(houses[house].owned, p_ptr->name, MAX_NAME_LEN+1);

	return TRUE;
}

/*
 * Set the given house as unowned
 */
void disown_house(int house)
{
	cave_type *c_ptr;
	int i,j, Depth;

	if (house >= 0 && house < num_houses)
	{
		Depth = houses[house].depth;
		houses[house].owned[0] = '\0';
		houses[house].strength = 0;
		/* Remove all players from the house */
		for (i = 1; i < NumPlayers + 1; i++)
		{
			if (house_inside(i, house))
			{
				msg_print(i, "You have been expelled from the house.");
				teleport_player(i, 5);
			}
		}
		/* Clear any items from the house */
		for (i = houses[house].y_1; i <= houses[house].y_2; i++)
		{
			for (j = houses[house].x_1; j <= houses[house].x_2; j++)
			{
				delete_object(houses[house].depth,i,j);
			}
		}

		/* Paranoia! */
		if (!cave[Depth]) return;

		/* Get requested grid */
		c_ptr = &cave[Depth][houses[house].door_y][houses[house].door_x];

		/* Close the door */
		c_ptr->feat = FEAT_HOME_HEAD + houses[house].strength;

		/* Reshow */
		everyone_lite_spot(Depth, houses[house].door_y, houses[house].door_x);

	}
}

/*
 * Attempt to open the given chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_open_chest(int Ind, int y, int x, s16b o_idx)
{
	player_type *p_ptr = Players[Ind];

	int i, j;

	bool flag = TRUE;

	bool more = FALSE;

	object_type *o_ptr = &o_list[o_idx];


	/* Attempt to unlock it */
	if (o_ptr->pval > 0)
	{
		/* Assume locked, and thus not open */
		flag = FALSE;

		/* Get the "disarm" factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite(Ind)) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the difficulty */
		j = i - o_ptr->pval;

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success -- May still have traps */
		if (rand_int(100) < j)
		{
			msg_print_aux(Ind, "You have picked the lock.", MSG_LOCKPICK);
			sound(Ind, MSG_LOCKPICK);
			gain_exp(Ind, 1);
			flag = TRUE;
		}

		/* Failure -- Keep trying */
		else
		{
			/* We may continue repeating */
			more = TRUE;
			/*if (flush_failure) flush();*/
			msg_print_aux(Ind, "You failed to pick the lock.", MSG_LOCKPICK_FAIL);
			sound(Ind, MSG_LOCKPICK_FAIL);
		}
	}

	/* Allowed to open */
	if (flag)
	{
		/* Apply chest traps, if any */
		chest_trap(Ind, y, x, o_ptr);

		/* Let the Chest drop items */
		chest_death(Ind, y, x, o_ptr);
	}

	/* Result */
	return (more);
}

/*
 * Attempt to disarm the chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_disarm_chest(int Ind, int y, int x, s16b o_idx)
{
	player_type *p_ptr = Players[Ind];
	int i, j;

	bool more = FALSE;

	object_type *o_ptr = &o_list[o_idx];


	/* Get the "disarm" factor */
	i = p_ptr->skill_dis;

	/* Penalize some conditions */
	if (p_ptr->blind || no_lite(Ind)) i = i / 10;
	if (p_ptr->confused || p_ptr->image) i = i / 10;

	/* Extract the difficulty */
	j = i - o_ptr->pval;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Must find the trap first. */
	if (!object_known_p(p_ptr, o_ptr))
	{
		msg_print(Ind, "I don't see any traps.");
	}

	/* Already disarmed/unlocked */
	else if (o_ptr->pval <= 0)
	{
		msg_print(Ind, "The chest is not trapped.");
	}

	/* No traps to find. */
	else if (!chest_traps[o_ptr->pval])
	{
		msg_print(Ind, "The chest is not trapped.");
	}

	/* Success (get a lot of experience) */
	else if (rand_int(100) < j)
	{
		msg_print_aux(Ind, "You have disarmed the chest.", MSG_DISARM);
		sound(Ind, MSG_DISARM);
		gain_exp(Ind, o_ptr->pval);
		o_ptr->pval = (0 - o_ptr->pval);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint(i) > 5))
	{
		/* We may keep trying */
		more = TRUE;
		/*if (flush_failure) flush();*/
		msg_print(Ind, "You failed to disarm the chest.");
	}

	/* Failure -- Set off the trap */
	else
	{
		msg_print(Ind, "You set off a trap!");
		chest_trap(Ind, y, x, o_ptr);
	}

	/* Result */
	return (more);
}




/*
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * This will, however, produce grids which are NOT illuminated
 * (or darkened) along with the rest of the room.
 */
static bool twall(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type	*c_ptr = &cave[Depth][y][x];
	byte		*w_ptr = &p_ptr->cave_flag[y][x];

	/* Paranoia -- Require a wall or door or some such */
	if (cave_floor_bold(Depth, y, x)) return (FALSE);

	/* Sound */
	sound(Ind, MSG_DIG);

	/* Remove the feature */
	if (Depth > 0)
		c_ptr->feat = FEAT_FLOOR;
	else
		c_ptr->feat = FEAT_DIRT;

	/* Forget the "field mark" */
	*w_ptr &= ~CAVE_MARK;

	/* Notice */
	note_spot_depth(Depth, y, x);

	/* Redisplay the grid */
	everyone_lite_spot(Depth, y, x);

	/* Update some things */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

	/* Result */
	return (TRUE);
}

/*
 * Determine if a given grid may be "opened"
 */
static bool do_cmd_open_test(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	/* Ghosts cannot open */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot open things!");

		return (FALSE);
	}

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Must have knowledge */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Must be a closed door */
	if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
	      (c_ptr->feat <= FEAT_DOOR_TAIL)) &&
	      /* OR MAngband-specific: Player house doors */
	    !((c_ptr->feat >= FEAT_HOME_HEAD) &&
	      (c_ptr->feat <= FEAT_HOME_TAIL)) &&
	      /* OR MAngband-specific: House Creation */
	     !(c_ptr->feat == FEAT_PERM_EXTRA))
	{
		/* Message */
		msg_print_aux(Ind, "You see nothing there to open.", MSG_NOTHING_TO_OPEN);
		sound(Ind, MSG_NOTHING_TO_OPEN);

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "open" command on doors
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_open_aux(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	int i, j, k;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_open_test(Ind, y, x)) return (FALSE);

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];


	/* We put MAngband-specific tests on top, as new FEATs are larger */
	/* Player Houses */
	if (c_ptr->feat >= FEAT_HOME_HEAD && c_ptr->feat <= FEAT_HOME_TAIL)
	{
		i = pick_house(Depth, y, x);
		
		if (i == -1)
		{
			debug(format("No house found at Depth %d, X=%d, Y=%d !", Depth, y, x));
			return (FALSE);
		}

		/* Do we own this house? */
		if (house_owned_by(Ind,i) || (p_ptr->dm_flags & DM_HOUSE_CONTROL) )
		{

			/* If someone is in our store, we eject them (anti-exploit) */
			for (k = 1; k <= NumPlayers; k++ )
			{
				/* We don't block if the owner is getting out! */
				player_type *q_ptr = Players[k];
				if(q_ptr && Ind != k)
				{
					/* We do block if the owner is getting in */
					if(q_ptr->player_store_num == i && q_ptr->store_num == 8)
					{
						q_ptr->store_num = -1;
						send_store_leave(k);
						msg_print(k, "The shopkeeper locks the doors.");
					}
				}
			}

			/* Open the door */
			c_ptr->feat = FEAT_HOME_OPEN;

			/* Notice */
			note_spot_depth(Depth, y, x);

			/* Redraw */
			everyone_lite_spot(Depth, y, x);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
		}

		/* He's not the owner, check if owned */
		else if (house_owned(i))
		{
			/* Player owned store! */

			/* Disturb */
			disturb(Ind, 0, 0);

			/* Hack -- Enter store */
			command_new = '_';
			do_cmd_store(Ind,i);
		}
		else
		{
			int price, factor;

			/* Take CHR into account */
			factor = adj_chr_gold[p_ptr->stat_ind[A_CHR]];
            price = (unsigned long) houses[i].price * factor / 100;
			if(Depth==0) {
				price = (unsigned long)price *5L;
			};

			/* Tell him the price */
			msg_format(Ind, "This house costs %ld gold.", price);
		}
	}
		
	/* Open a perma wall */
	else if (c_ptr->feat == FEAT_PERM_EXTRA)
	{
		/* Opening a wall?  Either the player has lost his mind or he 
		 * is trying to create a door! */
		create_house_door(Ind, x, y);
	}
			
	/* Jammed door */
	else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x08)
	{
		/* Stuck */
		msg_print(Ind, "The door appears to be stuck.");
	}

	/* Locked door */
	else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x01)
	{
		/* Disarm factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite(Ind)) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the lock power */
		j = c_ptr->feat - FEAT_DOOR_HEAD;

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (rand_int(100) < j)
		{
			/* Message */
			msg_print_aux(Ind, "You have picked the lock.", MSG_LOCKPICK);
			sound(Ind, MSG_LOCKPICK);

			/* Open the door */
			c_ptr->feat = FEAT_OPEN;

			/* Notice */
			note_spot_depth(Depth, y, x);

			/* Redraw */
			everyone_lite_spot(Depth, y, x);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

			/* Experience */
			gain_exp(Ind, 1);
		}

		/* Failure */
		else
		{
			/* Failure */
			/*if (flush_failure) flush();*/

			/* Message */
			msg_print_aux(Ind, "You failed to pick the lock.", MSG_LOCKPICK_FAIL);
			sound(Ind, MSG_LOCKPICK_FAIL);

			/* We may keep trying */
			more = TRUE;
		}
	}

	/* Closed door */
	else
	{
		/* Open the door */
		/*cave_set_feat(y, x, FEAT_OPEN);*/
		c_ptr->feat = FEAT_OPEN;

		/* Notice */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

		/* Sound */
		sound(Ind, MSG_OPENDOOR);
	}

	/* Result */
	return (more);
}



/*
 * Open a closed/locked/jammed door or a closed/locked chest.
 *
 * Unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;
	object_type		*o_ptr;

	int y, x;

	s16b o_idx;

	bool more = FALSE;

#if 0
	/* Easy Open */
	if (p_ptr->easy_open)
	{
		int num_doors, num_chests;

		/* Count closed doors */
		num_doors = count_feats(&y, &x, is_closed, FALSE);

		/* Count chests (locked) */
		num_chests = count_chests(&y, &x, FALSE);

		/* See if only one target */
		if ((num_doors + num_chests) == 1)
		{
			p_ptr->command_dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
		}
	}
#endif
	/* Get a direction (or abort) */
	/*if (!get_rep_dir(&dir)) return;*/
	if (!VALID_DIR(dir)) return;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Access the item */
	o_ptr = &o_list[c_ptr->o_idx];

	/* Check for chests */
	o_idx = (o_ptr->tval == TV_CHEST ? c_ptr->o_idx : 0);


	/* Verify legality */
	if (!o_idx && !do_cmd_open_test(Ind, y, x)) return;


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Apply confusion */
	if (confuse_dir((bool)p_ptr->confused, &dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[Depth][y][x];
	
		/* Access the item */
		o_ptr = &o_list[c_ptr->o_idx];
	
		/* Check for chest */
		o_idx = (o_ptr->tval == TV_CHEST ? c_ptr->o_idx : 0);
	}


	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/*p_ptr->command_rep = p_ptr->command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (c_ptr->m_idx > 0)
	{
		/* Message */
		msg_print(Ind, "There is a monster in the way!");

		/* Attack */
		py_attack(Ind, y, x);
	}

	/* Player in the way */
	else if (c_ptr->m_idx < 0)
	{
		/* Take a turn */
		p_ptr->energy -= level_speed(p_ptr->dun_depth);

		/* Message */
		msg_print(Ind, "There is a player in the way!");

	}

	/* Chest */
	else if (o_idx)
	{
		/* Open the chest */
		more = do_cmd_open_chest(Ind, y, x, o_idx);
	}

	/* Door */
	else
	{
		/* Open the door */
		more = do_cmd_open_aux(Ind, y, x);
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(Ind, 0, 0);
}


/*
 * Determine if a given grid may be "closed"
 */
static bool do_cmd_close_test(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

#if 0
	/* Ghosts cannot close */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot close things!");

		return (FALSE);
	}
#endif
	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Must have knowledge */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there.");

		/* Nope */
		return (FALSE);
	}

 	/* Require open/broken door */
	if ((c_ptr->feat != FEAT_OPEN) &&
	    (c_ptr->feat != FEAT_BROKEN) &&
	    (c_ptr->feat != FEAT_HOME_OPEN))
	{
		/* Message */
		msg_print(Ind, "You see nothing there to close.");

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "close" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_close_aux(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;
	
	int 	i;

	bool more = FALSE;

	/* Verify legality */
	if (!do_cmd_close_test(Ind, y, x)) return (FALSE);


	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];


	/* Broken door */
	if (c_ptr->feat == FEAT_BROKEN)
	{
		/* Message */
		msg_print(Ind, "The door appears to be broken.");
	}

	/* (MAngband-specific) House door, close it */
	else if (c_ptr->feat == FEAT_HOME_OPEN)
	{
		/* Find this house */
		i = pick_house(Depth, y, x);

		/* Close the door */
		c_ptr->feat = FEAT_HOME_HEAD + houses[i].strength;

		/* Notice */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
	}

	/* Open door */
	else
	{
		/* Close the door */
		c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

		/* Notice */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
		
		/* Sound */
		sound(Ind, MSG_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	int y, x;

	bool more = FALSE;

	/* Check preventive inscription '^c' */
	__trap(Ind, CPI(p_ptr, 'c'));

#if 0
	/* Easy Close */
	if (p_ptr->easy_open)
	{
		/* Count open doors */
		if (count_feats(&y, &x, is_open, FALSE) == 1)
		{
			p_ptr->command_dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
		}
	}
#endif
	/* Get a direction (or abort) */
	/*if (!get_rep_dir(&dir)) return;*/
	if (!VALID_DIR(dir)) return;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_close_test(Ind, y, x)) return;

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Apply confusion */
	if (confuse_dir((bool)p_ptr->confused, &dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
	}

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];
	
	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/*p_ptr->command_rep = p_ptr->command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (c_ptr->m_idx > 0)
	{
		/* Message */
		msg_print(Ind, "There is a monster in the way!");

		/* Attack */
		py_attack(Ind, y, x);
	}

	/* Player in the way */
	else if (c_ptr->m_idx < 0)
	{
		/* Message */
		msg_print(Ind, "There is a player in the way!");
	}

	/* Door */
	else
	{
		/* Close door */
		more = do_cmd_close_aux(Ind, y, x);
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(Ind, 0, 0);
}


/*
 * Determine if a given grid may be "tunneled"
 */
static bool do_cmd_tunnel_test(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;


	/* Ghosts cannot tunnel */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot tunnel!");

		return (FALSE);
	}

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Must have knowledge */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Must be a wall/door/etc */
	if (cave_floor_bold(Depth, y, x))
	{
		/* Message */
		msg_print(Ind, "You see nothing there to tunnel.");

		/* Nope */
		return (FALSE);
	}

	/* No tunnelling through house doors */
	if (c_ptr->feat == FEAT_HOME_OPEN || 
		(c_ptr->feat >= FEAT_HOME_HEAD && c_ptr->feat <= FEAT_HOME_TAIL))
	{
		/* Message */
		msg_print(Ind, "You cannot tunnel through house doors.");
		
		/* Nope */
		return (FALSE);
	}
#if 0
	/* No tunnelling through emptiness */
	if ( (cave_floor_bold(Depth, y, x)) || (c_ptr->feat == FEAT_PERM_CLEAR) )
	{
		/* Message */
		msg_print(Ind, "You see nothing there to tunnel through.");
	}
#endif 
	/* Okay */
	return (TRUE);
}

/* Return "cutting bonus" (0-4) based on weapon effeciency against trees 
 * This macro depends on TV_SWORD SVALues being ordered from less
 * powerfull to more powerfull. */
#define wielding_cut(TV,SV) \
	((((TV) == TV_SWORD) ? (((SV) >= SV_LONG_SWORD) ? 2 : \
		(((SV) >= SV_SMALL_SWORD) ? 1 : 0)) : \
	(((TV) == TV_POLEARM) ? (((SV) == SV_SCYTHE_OF_SLICING) ? 4 : \
		(((SV) >= SV_SCYTHE) ? 3 : (((SV) >= SV_BEAKED_AXE) ? 2 : 0))) : 0 )))

#define wielding_cut_o(O) \
	(wielding_cut((O).tval,(O).sval))
#define wielding_cut_p(P) \
	(wielding_cut_o((P)->inventory[INVEN_WIELD]))
/*
 * Perform the basic "tunnel" command
 *
 * Assumes that no monster is blocking the destination
 *
 * Uses "twall" (above) to do all "terrain feature changing".
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_tunnel_aux(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_tunnel_test(Ind, y, x)) return (FALSE);

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Sound XXX XXX XXX */
	sound(Ind, MSG_DIG);

	/* Hack -- We put MAngband-specific terrain features on top, as they are higher */

	/* Vegetation */
	if (c_ptr->feat == FEAT_TREE)
	{
		/* Mow down the vegetation */
		if ((p_ptr->skill_dig + wielding_cut_p(p_ptr) * 10 > rand_int(400)) && twall(Ind, y, x))
		{
			if (Depth == 0) trees_in_town--;
		
			/* Message */
			msg_print(Ind, "You hack your way through the vegetation.");
		}
		else
		{
			/* Message, keep digging */
			msg_print(Ind, "You attempt to clear a path.");
			more = TRUE;
		}
	}

	else if (c_ptr->feat == FEAT_EVIL_TREE)
	{
		/* Mow down the vegetation */
		if ((p_ptr->skill_dig + wielding_cut_p(p_ptr) * 10 > rand_int(600)) && twall(Ind, y, x))
		{
			/* Message */
			msg_print(Ind, "You hack your way through the vegetation.");
			
			/* Notice */
			note_spot_depth(Depth, y, x);

			/* Display */
			everyone_lite_spot(Depth, y, x);
		}
		else
		{
			/* Message, keep digging */
			msg_print(Ind, "You attempt to clear a path.");
			more = TRUE;
		}
	}

	/* Titanium */
	else if (c_ptr->feat >= FEAT_PERM_EXTRA)
	{
		msg_print(Ind, "This seems to be permanent rock.");
	}

	/* Granite */
	else if (c_ptr->feat >= FEAT_WALL_EXTRA)
	{
		/* Tunnel */
		if ((p_ptr->skill_dig > 40 + rand_int(1600)) && twall(Ind, y, x))
		{
			msg_print(Ind, "You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg_print(Ind, "You tunnel into the granite wall.");
			more = TRUE;
		}
	}

	/* Quartz / Magma */
	else if (c_ptr->feat >= FEAT_MAGMA)
	{
		bool okay = FALSE;
		bool gold = FALSE;
		bool hard = FALSE;

		/* Found gold */
		if (c_ptr->feat >= FEAT_MAGMA_H)
		{
			gold = TRUE;
		}

		/* Extract "quartz" flag XXX XXX XXX */
		if ((c_ptr->feat - FEAT_MAGMA) & 0x01)
		{
			hard = TRUE;
		}

		/* Quartz */
		if (hard)
		{
			okay = (p_ptr->skill_dig > 20 + rand_int(800));
		}

		/* Magma */
		else
		{
			okay = (p_ptr->skill_dig > 10 + rand_int(400));
		}

		/* Success */
		if (okay && twall(Ind, y, x))
		{
			/* Found treasure */
			if (gold)
			{
				/* Place some gold */
				place_gold(Depth, y, x);

				/* Message */
				msg_print(Ind, "You have found something!");
			}

			/* Found nothing */
			else
			{
				/* Message */
				msg_print(Ind, "You have finished the tunnel.");
			}
		}

		/* Failure (quartz) */
		else if (hard)
		{
			/* Message, continue digging */
			msg_print(Ind, "You tunnel into the quartz vein.");
			more = TRUE;
		}

		/* Failure (magma) */
		else
		{
			/* Message, continue digging */
			msg_print(Ind, "You tunnel into the magma vein.");
			more = TRUE;
		}
	}

	/* Rubble */
	else if (c_ptr->feat == FEAT_RUBBLE)
	{
		/* Remove the rubble */
		if ((p_ptr->skill_dig > rand_int(200)) && twall(Ind, y, x))
		{
			/* Message */
			msg_print(Ind, "You have removed the rubble.");

			/* Hack -- place an object */
			if (rand_int(100) < 10)
			{
				/* Create a simple object */
				place_object(Depth, y, x, FALSE, FALSE, 0);

				/* Observe new object */
				if (player_can_see_bold(Ind, y, x))
				{
					msg_print(Ind, "You have found something!");
				}
			}
		}

		else
		{
			/* Message, keep digging */
			msg_print(Ind, "You dig in the rubble.");
			more = TRUE;
		}
	}

	/* Secret doors */
	else if (c_ptr->feat >= FEAT_SECRET)
	{
		/* Tunnel */
		if ((p_ptr->skill_dig > 30 + rand_int(1200)) && twall(Ind, y, x))
		{
			msg_print(Ind, "You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg_print(Ind, "You tunnel into the granite wall.");
			more = TRUE;

			/* Occasional Search XXX XXX */
			if (rand_int(100) < 25) search(Ind);
		}
	}
	
	/* Doors */
	else
	{
		/* Tunnel */
		if ((p_ptr->skill_dig > 30 + rand_int(1200)) && twall(Ind, y, x))
		{
			msg_print(Ind, "You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg_print(Ind, "You tunnel into the door.");
			more = TRUE;
		}
	}

	/* Result */
	return (more);
}


/*
 * Tunnel through "walls" (including rubble and secret doors)
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	int y, x;

	bool more = FALSE;

	/* Check preventive inscription '^T' */
	__trap(Ind, CPI(p_ptr, 'T'));

	/* Get a direction (or abort) */
	/*if (!get_rep_dir(&dir)) return;*/
	if (!VALID_DIR(dir)) return;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Oops */
	if (!do_cmd_tunnel_test(Ind, y, x)) return;

	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Apply confusion */
	if (confuse_dir((bool)p_ptr->confused, &dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
	}		

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/* p_ptr->command_rep = p_ptr->command_arg - 1; */

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (c_ptr->m_idx > 0)
	{
		/* Message */
		msg_print(Ind, "There is a monster in the way!");

		/* Attack */
		py_attack(Ind, y, x);
	}
	
	/* Player in the way */
	else if (c_ptr->m_idx < 0)
	{
		/* Message */
		msg_print(Ind, "There is a player in the way!");
	}

	/* Walls */
	else
	{
		/* Tunnel through walls */
		more = do_cmd_tunnel_aux(Ind, y, x);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(Ind, 0, 0);
}


/*
 * Determine if a given grid may be "disarmed"
 */
static bool do_cmd_disarm_test(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type		*c_ptr;
	object_type		*o_ptr;


	/* Ghosts cannot disarm */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot disarm things!");

		return (FALSE);
	}

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Access the item */
	o_ptr = &o_list[c_ptr->o_idx];

	/* Must have knowledge */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Require an actual trap */
	if (!((c_ptr->feat >= FEAT_TRAP_HEAD) &&
	      (c_ptr->feat <= FEAT_TRAP_TAIL)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there to disarm.");

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "disarm" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_disarm_aux(int Ind, int y, int x, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type		*c_ptr;

	int i, j, power;

	cptr name;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_disarm_test(Ind, y, x)) return (FALSE);

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Get the trap name */
	name = (f_name + f_info[c_ptr->feat].name);

	/* Get the "disarm" factor */
	i = p_ptr->skill_dis;

	/* Penalize some conditions */
	if (p_ptr->blind || no_lite(Ind)) i = i / 10;
	if (p_ptr->confused || p_ptr->image) i = i / 10;

	/* XXX XXX XXX Variable power? */

	/* Extract trap "power" */
	power = 5;

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (rand_int(100) < j)
	{
		/* Message */
		msg_format_type(Ind, MSG_DISARM, "You have disarmed the %s.", name);
		sound(Ind, MSG_DISARM);

		/* Reward */
		gain_exp(Ind, power);

		/* Forget the trap */
		everyone_forget_spot(Depth, y, x);

		/* Remove the trap */
		/* cave_set_feat(Depth, y, x, FEAT_FLOOR); */
		c_ptr->feat = FEAT_FLOOR;

		/* Notice */
		note_spot_depth(Depth, y, x);

		/* Redisplay the grid */
		everyone_lite_spot(Depth, y, x);
#if 0
		/* move the player onto the trap grid */
		move_player(Ind, dir, FALSE);
#endif
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint(i) > 5))
	{
		/* Failure */
		/*if (flush_failure) flush();*/

		/* Message */
		msg_format(Ind, "You failed to disarm the %s.", name);

		/* We may keep trying */
		more = TRUE;
	}

	/* Failure -- Set off the trap */
	else
	{
		/* Message */
		msg_format(Ind, "You set off the %s!", name);

		/* Hit the trap */
		/*hit_trap(y, x); in MAngband we move.. some hack..? */
		/* Move the player onto the trap */
		move_player(Ind, dir, FALSE);
	}

	/* Result */
	return (more);
}


/*
 * Disarms a trap, or a chest
 */
void do_cmd_disarm(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type		*c_ptr;
	object_type		*o_ptr;

	int y, x;

	s16b o_idx;

	bool more = FALSE;

	/* Check preventive inscription '^D' */
	__trap(Ind, CPI(p_ptr, 'D'));

#if 0
	/* Easy Disarm */
	if (p_ptr->easy_open)
	{
		int num_traps, num_chests;

		/* Count visible traps */
		num_traps = count_feats(&y, &x, is_trap, TRUE);

		/* Count chests (trapped) */
		num_chests = count_chests(&y, &x, TRUE);

		/* See if only one target */
		if (num_traps || num_chests)
		{
			if (num_traps + num_chests <= 1)
				p_ptr->command_dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
		}
	}

	/* Get a direction (or abort) */
	if (!get_rep_dir(&dir)) return;
#endif
	if (!VALID_DIR(dir)) return;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Access the item */
	o_ptr = &o_list[c_ptr->o_idx];

	/* Check for chests */
	o_idx = (o_ptr->tval == TV_CHEST ? c_ptr->o_idx : 0);


	/* Verify legality */
	if (!o_idx && !do_cmd_disarm_test(Ind, y, x)) return;


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Apply confusion */
	if (confuse_dir((bool)p_ptr->confused, &dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[Depth][y][x];
	
		/* Access the item */
		o_ptr = &o_list[c_ptr->o_idx];
	
		/* Check for chests */
		o_idx = (o_ptr->tval == TV_CHEST ? c_ptr->o_idx : 0);
	}


	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/* p_ptr->command_rep = p_ptr->command_arg - 1; */

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (c_ptr->m_idx > 0)
	{
		/* Message */
		msg_print(Ind, "There is a monster in the way!");

		/* Attack */
		py_attack(Ind, y, x);
	}

	/* Player in the way */
	else if (c_ptr->m_idx < 0)
	{
		/* Message */
		msg_print(Ind, "There is a player in the way!");
	}

	/* Chest */
	else if (o_idx)
	{
		/* Disarm the chest */
		more = do_cmd_disarm_chest(Ind, y, x, o_idx);
	}

	/* Disarm trap */
	else
	{
		/* Disarm the trap */
		more = do_cmd_disarm_aux(Ind, y, x, dir);
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(Ind, 0, 0);
}

/*
 * Determine if a given grid may be "bashed"
 */
static bool do_cmd_bash_test(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	cave_type		*c_ptr;


	/* Ghosts cannot bash */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot bash things!");

		return (FALSE);
	}

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Must have knowledge */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg_print(Ind, "You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Require a door */
	if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
	      (c_ptr->feat <= FEAT_DOOR_TAIL)))
	{	
		/* Message */
		msg_print(Ind, "You see nothing there to bash.");

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "bash" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_bash_aux(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	
	cave_type		*c_ptr;

	int bash, temp;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_bash_test(Ind, y, x)) return (FALSE);

	/* Get grid and contents */
	c_ptr = &cave[Depth][y][x];

	/* Message */
	msg_print(Ind, "You smash into the door!");

	/* Hack -- Bash power based on strength */
	/* (Ranges from 3 to 20 to 100 to 200) */
	bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

	/* Extract door power */
	temp = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

	/* Compare bash power to door power XXX XXX XXX */
	temp = (bash - (temp * 10));

	/* Hack -- always have a chance */
	if (temp < 1) temp = 1;

	/* Hack -- attempt to bash down the door */
	if (rand_int(100) < temp)
	{
		/* Message */
		msg_print_aux(Ind, "The door crashes open!", MSG_OPENDOOR);
		sound(Ind, MSG_OPENDOOR);

		/* Break down the door */
		if (rand_int(100) < 50)
		{
			c_ptr->feat = FEAT_BROKEN;
		}

		/* Open the door */
		else
		{
			c_ptr->feat = FEAT_OPEN;
		}

		/* Notice */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);
#if 0
		/* Hack -- Fall through the door */
		move_player(Ind, dir, FALSE);
#endif
		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_LITE);
		p_ptr->update |= (PU_DISTANCE);
	}

	/* Saving throw against stun */
	else if (rand_int(100) < adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
	         p_ptr->lev)
	{
		/* Message */
		msg_print(Ind, "The door holds firm.");

		/* Allow repeated bashing */
		more = TRUE;
	}

	/* High dexterity yields coolness */
	else
	{
		/* Message */
		msg_print(Ind, "You are off-balance.");

		/* Hack -- Lose balance ala paralysis */
		(void)set_paralyzed(Ind, p_ptr->paralyzed + 2 + rand_int(2));
	}

	/* Result */
	return (more);
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 */
void do_cmd_bash(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	
	int Depth = p_ptr->dun_depth;

	cave_type		*c_ptr;
	
	int y, x;

	/* Check preventive inscription '^B' */
	__trap(Ind, CPI(p_ptr, 'B'));

	/* Get a direction (or abort) */
	/* if (!get_rep_dir(&dir)) return; */
	if (!VALID_DIR(dir)) return;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_bash_test(Ind, y, x)) return;


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Apply confusion */
	if (confuse_dir((bool)p_ptr->confused, &dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
	}

	/* Get grid */
	c_ptr = &cave[Depth][y][x];

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/*p_ptr->command_rep = p_ptr->command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Monster */
	if (c_ptr->m_idx > 0)
	{
		/* Message */
		msg_print(Ind, "There is a monster in the way!");

		/* Attack */
		py_attack(Ind, y, x);
	}
	
	/* Player in the way */
	else if (c_ptr->m_idx < 0)
	{
		/* Message */
		msg_print(Ind, "There is a player in the way!");
	}

	/* Door */
	else
	{
		/* Bash the door */
		if (!do_cmd_bash_aux(Ind, y, x))
		{
			/* Cancel repeat */
			disturb(Ind, 0, 0);
		}
	}
}






/*
 * Manipulate an adjacent grid in some way
 *
 * Attack monsters, tunnel through walls, disarm traps, open doors.
 *
 * <s>This command must always take energy, to prevent free detection
 * of invisible monsters.</s>
 * REVISED FOR MAngband-specific reasons: we don't care if someone
 * detects a monster by tunneling into it, and treat "tunnel air" as an
 * error, which DOES NOT spend player's energy. This is a classic MAnghack,
 * updated to newer V306 realities.
 *
 * The "semantics" of this command must be chosen before the player
 * is confused, and it must be verified against the new grid.
 */
void do_cmd_alter(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	int y, x;

	int feat;

	bool more = FALSE;
	bool spend = TRUE;

	cave_type		*c_ptr;

	/* Check preventive inscription '^+' */
	__trap(Ind, CPI(p_ptr, '+'));

	/* Get a direction */
	if (!VALID_DIR(dir)) return;

	/* Apply confusion */
	confuse_dir((bool)p_ptr->confused, &dir);

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* MEGA-HACK! DM HOOK! */
	if (p_ptr->master_hook[1])
		master_new_hook(Ind, 'a', y, x);

	/* Get grid */
	c_ptr = &cave[Depth][y][x];

	/* Original feature */
	feat = c_ptr->feat;

	/* Must have knowledge to know feature XXX XXX */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK))) feat = FEAT_NONE;

	/* Allow repeated command */
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		/*p_ptr->command_rep = p_ptr->command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}

	/* Attack monsters */
	if (c_ptr->m_idx > 0)
	{
		/* Attack */
		py_attack(Ind, y, x);
	}

	/* MAngband-specific: Open closed House doors */
	else if (feat >= FEAT_HOME_HEAD && feat <= FEAT_HOME_TAIL)
	{
		/* Open */
		more = do_cmd_open_aux(Ind, y, x);
	}

	/* MAngband-specific: Open walls (House Creation) */
	else if (feat == FEAT_PERM_EXTRA)
	{
		/* Open */
		more = do_cmd_open_aux(Ind, y, x);
	}

	/* Tunnel through walls */
	else if (feat >= FEAT_SECRET)
	{
		/* Tunnel */
		more = do_cmd_tunnel_aux(Ind, y, x);
	}
#if 0
	/* Bash jammed doors */
	else if (feat >= FEAT_DOOR_HEAD + 0x08)
	{
		/* Bash */
		more = do_cmd_bash_aux(Ind, y, x);
	}
#endif /* 0 */
	/* Open closed doors */
	else if (feat >= FEAT_DOOR_HEAD)
	{
		/* Open */
		more = do_cmd_open_aux(Ind, y, x);
	}

	/* Disarm traps */
	else if (feat >= FEAT_TRAP_HEAD)
	{
		/* Disarm */
		more = do_cmd_disarm_aux(Ind, y, x, dir);
	}
#if 0
	/* Close open doors */
	else if (feat == FEAT_OPEN)
	{
		/* Close */
		more = do_cmd_close_aux(y, x);
	}
#endif

	/* Oops */
	else
	{
		/* Oops */
		msg_print(Ind, "You spin around.");
		/* Do not spend energy. */
		spend = FALSE;
	}

	if (spend)
	{
		/* Take a turn */
		p_ptr->energy -= level_speed(p_ptr->dun_depth);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(Ind, 0, 0);
}



/*
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Let user choose a pile of spikes, perhaps?
 */
static bool get_spike(int Ind, int *ip)
{
	player_type *p_ptr = Players[Ind];

	int i;

	/* Check every item in the pack */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &(p_ptr->inventory[i]);

		/* Check the "tval" code */
		if (o_ptr->tval == TV_SPIKE)
		{
			/* Save the spike index */
			(*ip) = i;

			/* Success */
			return (TRUE);
		}
	}

	/* Oops */
	return (FALSE);
}


/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	int                  y, x, item;

	cave_type		*c_ptr;

	/* Check preventive inscription '^j' */
	__trap(Ind, CPI(p_ptr, 'j'));

	/* Ghosts cannot spike */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		/* Message */
		msg_print(Ind, "You cannot spike doors!");

		return;
	}

	/* Get a "repeated" direction */
	if (VALID_DIR(dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[Depth][y][x];

		/* Require closed door */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		      (c_ptr->feat <= FEAT_DOOR_TAIL)))
		{
			/* Message */
			msg_print(Ind, "You see nothing there to spike.");
		}

		/* Get a spike */
		else if (!get_spike(Ind, &item))
		{
			/* Message */
			msg_print(Ind, "You have no spikes!");
		}

		/* Is a monster in the way? */
		else if (c_ptr->m_idx > 0)
		{
			/* Take a turn */
			p_ptr->energy -= level_speed(p_ptr->dun_depth);

			/* Message */
			msg_print(Ind, "There is a monster in the way!");

			/* Attack */
			py_attack(Ind, y, x);
		}

		/* Go for it */
		else
		{
			/* Take a turn */
			p_ptr->energy -= level_speed(p_ptr->dun_depth);

			/* Successful jamming */
			msg_print(Ind, "You jam the door with a spike.");

			/* Convert "locked" to "stuck" XXX XXX XXX */
			if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08) c_ptr->feat += 0x08;

			/* Add one spike to the door */
			if (c_ptr->feat < FEAT_DOOR_TAIL) c_ptr->feat++;

			/* Use up, and describe, a single spike, from the bottom */
			inven_item_increase(Ind, item, -1);
			inven_item_describe(Ind, item);
			inven_item_optimize(Ind, item);
		}
	}
}

/*
 * Clicked somewhere on the dungeon.
 *
 * "mod" can contain any MCURSOR_XXX flag, except for
 *  MCURSOR_EMB and MCURSOR_META (so buttons 1, 2, 3 and
 *  modifiers CTRL, ALT, SHIFT).
 */
void do_cmd_mouseclick(player_type *p_ptr, int mod, int y, int x)
{
	/* Right now, we only support 1 mouse button */
	if (!(mod & MCURSOR_LMB)) return;

	y = y + p_ptr->panel_row_min;
	x = x + p_ptr->panel_col_min;

	if (x < p_ptr->panel_col_min) x = p_ptr->panel_col_min;
	if (y < p_ptr->panel_row_min) y = p_ptr->panel_row_min;
	if (x > p_ptr->panel_col_max) x = p_ptr->panel_col_max;
	if (y > p_ptr->panel_row_max) y = p_ptr->panel_row_max;

	/* Hack -- execute '_' ? */
	if ((mod & MCURSOR_LMB) && (mod & MCURSOR_SHFT))
	{
		/* Grid offset is 0 (standing on) */
		if (p_ptr->px == x && p_ptr->py == y)
		{
			do_cmd_enterfeat(Get_Ind[p_ptr->conn]);
		}
		return;
	}

	/* Hack -- execute alter? */
	if ((mod & MCURSOR_LMB) && (mod & MCURSOR_KTRL))
	{
		/* Grid is nearby */
		if (ABS(p_ptr->px - x) <= 1 && ABS(p_ptr->py - y) <= 1)
		{
			int dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
			do_cmd_alter(Get_Ind[p_ptr->conn], dir);
		}
		return;
	}

	do_cmd_pathfind(Get_Ind[p_ptr->conn], y, x);
}


/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int Ind, int dir, int pickup)
{
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;

	bool more = FALSE;

	/* Check preventive inscription '^;' */
	__trap(Ind, CPI(p_ptr, ';'));

	/* Make sure he hasn't just switched levels */
	if (p_ptr->new_level_flag) return;

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		/*command_rep = command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (VALID_DIR(dir))
	{
		/* Hack -- handle confusion */
		if (p_ptr->confused)
		{
			dir = 5;

			/* Prevent walking nowhere */
			while (dir == 5)
				dir = ddd[rand_int(8)];
		}

		/* Handle the "easy_alter" option */
		if (option_p(p_ptr,EASY_ALTER))
		{
			/* Get requested grid */
			c_ptr = &cave[p_ptr->dun_depth][p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]];

			if ((p_ptr->cave_flag[p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]] & (CAVE_MARK)) && 
				(((c_ptr->feat >= FEAT_TRAP_HEAD) && 
			      (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			    ((c_ptr->feat >= FEAT_HOME_HEAD) &&
			      (c_ptr->feat <= FEAT_HOME_TAIL))))
			{
				do_cmd_alter(Ind, dir);
				return;
			}
		}

		/* Actually move the character */
		move_player(Ind, dir, pickup);

		/* Take a turn */
		p_ptr->energy -= level_speed(p_ptr->dun_depth);

		/* Allow more walking */
		more = TRUE;
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(Ind, 0, 0);
}



/*
 * Start running.
 */
int do_cmd_run(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;

	/* Check preventive inscription '^.' */
	if (CPI(p_ptr, '.')) { msg_print(Ind, "The item's inscription prevents it."); return 0; }

	/* Classic MAnghack #3. */
	/* Treat this as a walk request 
	 * -- instead of disallowing running when confused */
	if (p_ptr->confused)
	{
		/* BUT NOT IN TOWN ... */
		if (p_ptr->dun_depth)
		{
			do_cmd_walk(Ind, dir, option_p(p_ptr, ALWAYS_PICKUP));
			return 1;
		}
	}

	if (p_ptr->confused)
	{
		msg_print(Ind, "You are too confused!");
		return 0;
	}

	/* Ignore if we are already running in this direction */
	if (p_ptr->running && (dir == p_ptr->find_current) ) return 1;

	/* Get a "repeated" direction */
	if (VALID_DIR(dir))
	{
		/* Make sure we have an empty space to run into */
		if (see_wall(Ind, dir, p_ptr->py, p_ptr->px) && p_ptr->energy >= level_speed(p_ptr->dun_depth))
		{
			/* Handle the "easy_alter" option */
			if (option_p(p_ptr,EASY_ALTER))
			{
				/* Get requested grid */
				c_ptr = &cave[p_ptr->dun_depth][p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]];

				if ((p_ptr->cave_flag[p_ptr->py+ddy[dir]][p_ptr->px+ddx[dir]] & (CAVE_MARK)) && 
					(((c_ptr->feat >= FEAT_TRAP_HEAD) && 
				      (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
				    ((c_ptr->feat >= FEAT_HOME_HEAD) &&
				      (c_ptr->feat <= FEAT_HOME_TAIL)))) 
				{
					/* Check if we have enough energy to alter grid */
					if (p_ptr->energy >= level_speed(p_ptr->dun_depth))
					{
						/* If so, do it. */
						do_cmd_alter(Ind, dir);
					}
					return 1;
				}
			}

			/* Message */
			msg_print(Ind, "You cannot run in that direction.");

			/* Disturb */
			disturb(Ind, 0, 0);

			return 1;
		}

		/* Initialise running */
		p_ptr->run_request = dir;
		p_ptr->running = FALSE;
		p_ptr->ran_tiles = 0;
	}
	return 1;
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_hold_or_stay(int Ind, int pickup, int take_stairs)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	cave_type *c_ptr;
	
	if (p_ptr->new_level_flag) return;

	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		/*command_rep = command_arg - 1;*/

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}


/* We don't want any of this */
#if 0
	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);


	/* Spontaneous Searching */
	if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos)))
	{
		search(Ind);
	}

	/* Continuous Searching */
	if (p_ptr->searching)
	{
		search(Ind);
	}
#endif


	/* Hack -- enter a store if we are on one */
	if ((c_ptr->feat >= FEAT_SHOP_HEAD) &&
	    (c_ptr->feat <= FEAT_SHOP_TAIL))
	{
		/* Disturb */
		disturb(Ind, 0, 0);

		/* Hack -- enter store */
		command_new = '_';
	}


	/* Try to Pick up anything under us */
	carry(Ind, pickup, 0);

	/* Hack -- enter stairs if we are on one */
	if (take_stairs)
	{
		if (c_ptr->feat == FEAT_MORE)
		{
			do_cmd_go_down(Ind);
		}
		if (c_ptr->feat == FEAT_LESS)
		{
			do_cmd_go_up(Ind);
		}
	}
}

/*
 * Hold still (always pickup and enter stairs)
 */
void do_cmd_enterfeat(int Ind)
{
	/* Hold still (always pickup, enter stairs) */
	do_cmd_hold_or_stay(Ind, TRUE, TRUE);
}

/*
 * Hold still (always pickup)
 */
void do_cmd_hold(int Ind)
{
	/* Hold still (always pickup) */
	do_cmd_hold_or_stay(Ind, TRUE, FALSE);
}

/*
 * Stay still (usually do not pickup)
 */
void do_cmd_stay(int Ind)
{
	/* Stay still (usually do not pickup) */
	do_cmd_hold_or_stay(Ind, !option_p(Players[Ind],ALWAYS_PICKUP), FALSE);
}



/*
 * Toggle rest mode.
 */
void do_cmd_toggle_rest(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Set flag */
	p_ptr->resting = TRUE;

	/* Make sure we aren't running */
	p_ptr->running = FALSE;

	/* Take a lot of energy to enter "rest mode" */
	p_ptr->energy -= (level_speed(p_ptr->dun_depth));

	/* Redraw */
	p_ptr->redraw |= (PR_STATE);
}

/*
 * Resting allows a player to safely restore his hp	-RAK-
 */
#if 0
void do_cmd_rest(void)
{
	/* Prompt for time if needed */
	if (command_arg <= 0)
	{
		cptr p = "Rest (0-9999, '*' for HP/SP, '&' as needed): ";

		char out_val[80];

		/* Default */
		strcpy(out_val, "&");

		/* Ask for duration */
		if (!get_string(p, out_val, 4)) return;

		/* Rest until done */
		if (out_val[0] == '&')
		{
			command_arg = (-2);
		}

		/* Rest a lot */
		else if (out_val[0] == '*')
		{
			command_arg = (-1);
		}

		/* Rest some */
		else
		{
			command_arg = atoi(out_val);
			if (command_arg <= 0) return;
		}
	}


	/* Paranoia */
	if (command_arg > 9999) command_arg = 9999;


	/* Take a turn XXX XXX XXX (?) */
	energy -= level_speed(p_ptr->dun_depth);

	/* Save the rest code */
	resting = command_arg;

	/* Cancel searching */
	p_ptr->searching = FALSE;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff();

	/* Refresh */
	Term_fresh();
}
#endif


/*
 * Start running with pathfinder.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_pathfind(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];

	/* Hack -- translate nearby grid into walk request */
	if (ABS(p_ptr->px - x) <= 1 && ABS(p_ptr->py - y) <= 1)
	{
		int dir = motion_dir(p_ptr->py, p_ptr->px, y, x);
		do_cmd_walk(Ind, dir, option_p(p_ptr, ALWAYS_PICKUP));
		return;
	}

	/* Hack XXX XXX XXX */
	if (p_ptr->confused)
	{
		/* TODO: Maybe convert to walk request? */
		msg_print(Ind, "You are too confused!");
		return;
	}

	if (findpath(p_ptr, y, x))
	{
		p_ptr->running_withpathfind = TRUE;
		p_ptr->run_request = -1;
#if 0 /* In MAngband, we just schedule for later */
//		/* Calculate torch radius */
//		p_ptr->update |= (PU_TORCH);
//		run_step(Ind, 0);
#endif
	}
}




/*
 * Determines the odds of an object breaking when thrown at a monster
 *
 * Note that artifacts never break, see the "drop_near()" function.
 */
static int breakage_chance(object_type *o_ptr)
{
	/* Examine the item type */
	switch (o_ptr->tval)
	{
		/* Always break */
		case TV_FLASK:
		case TV_POTION:
		case TV_BOTTLE:
		case TV_FOOD:
		case TV_JUNK:
		{
			return (100);
		}

		/* Often break */
		case TV_LITE:
		case TV_SCROLL:
		case TV_ARROW:
		case TV_SKELETON:
		{
			return (50);
		}

		/* Sometimes break */
		case TV_WAND:
		case TV_SHOT:
		case TV_BOLT:
		case TV_SPIKE:
		{
			return (25);
		}
	}

	/* Rarely break */
	return (10);
}


/*
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * You must use slings + pebbles/shots, bows + arrows, xbows + bolts.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
void do_cmd_fire(int Ind, int item, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	int Depth = p_ptr->dun_depth;

	int			i, j, y, x, ny, nx, ty, tx;
	int			tdam, tdis, thits, tmul;
	int			bonus, chance;
	int			cur_dis, visible;

	object_type         throw_obj;
	object_type		*o_ptr;

	object_type		*j_ptr;

	bool		hit_body = FALSE;

	int			missile_attr;
	int			missile_char;

	char		o_name[80];
	bool		magic = FALSE;

	/* Check preventive inscription '^f' */	
	__trap(Ind, CPI(p_ptr, 'f'));

	/* Restrict ghosts */
	if ( (p_ptr->ghost || p_ptr->fruit_bat) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(Ind, "You cannot shoot!");
		return;
	}

	/* Get the "bow" (if any) */
	j_ptr = &(p_ptr->inventory[INVEN_BOW]);

	/* Require a launcher */
	if (!j_ptr->tval)
	{
		msg_print(Ind, "You have nothing to fire with.");
		return;
	}


	/* Require proper missile */
	item_tester_tval = p_ptr->tval_ammo;

	if (!item_tester_tval)
	{
		msg_print(Ind, "You have nothing to fire.");
		return;
	}

	/* Access the item (if in the pack) */
	if (item >= 0)
	{
		o_ptr = &(p_ptr->inventory[item]);
	}
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		o_ptr = &o_list[0 - item];
	}

	/* Check guard inscription '!f' */
	__trap(Ind, CGI(o_ptr, 'f'));

	if (o_ptr->tval != p_ptr->tval_ammo)
	{
		msg_print(Ind, "You cannot fire that!");
		return;
	}

	if (!o_ptr->tval)
	{
		msg_print(Ind, "You cannot fire that!");
		return;
	}

	/* Magic ammo */
	if ((o_ptr->sval == SV_AMMO_MAGIC) || artifact_p(o_ptr))
		magic = TRUE;

	/* Get a direction (or cancel) */
	p_ptr->command_dir = dir;
	if (!get_aim_dir(Ind, &dir)) return;

	/* Create a "local missile object" */
	throw_obj = *o_ptr;
	throw_obj.number = 1;

	if (!magic)
	{
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
			floor_item_optimize(0 - item);
			floor_item_notify(Ind, 0 - item, TRUE);
		}
	}

	/* Use the missile object */
	o_ptr = &throw_obj;

	/* Sound */
	sound(Ind, MSG_SHOOT);

	/* Describe the object */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(o_ptr);
	missile_char = object_char(o_ptr);


	/* Use the proper number of shots */
	thits = p_ptr->num_fire;

	/* Use a base distance */
	tdis = 10;

	/* Base damage from thrown object plus launcher bonus */
	tdam = damroll(o_ptr->dd, o_ptr->ds) + o_ptr->to_d + j_ptr->to_d;

	/* Actually "fire" the object */
	bonus = (p_ptr->to_h + o_ptr->to_h + j_ptr->to_h);
	chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

	/* Assume a base multiplier */
	tmul = 1;

	/* Analyze the launcher */
	switch (j_ptr->sval)
	{
		/* Sling and ammo */
		case SV_SLING:
		{
			tmul = 2;
			break;
		}

		/* Short Bow and Arrow */
		case SV_SHORT_BOW:
		{
			tmul = 2;
			break;
		}

		/* Long Bow and Arrow */
		case SV_LONG_BOW:
		{
			tmul = 3;
			break;
		}

		/* Light Crossbow and Bolt */
		case SV_LIGHT_XBOW:
		{
			tmul = 3;
			break;
		}

		/* Heavy Crossbow and Bolt */
		case SV_HEAVY_XBOW:
		{
			tmul = 4;
			break;
		}
	}

	/* Get extra "power" from "extra might" */
	if (p_ptr->xtra_might) tmul++;

	/* Boost the damage */
	tdam *= tmul;

	/* Base range */
	tdis = 10 + 5 * tmul;


	/* Take a (partial) turn */
	p_ptr->energy -= (level_speed(p_ptr->dun_depth) / thits);


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay(Ind))
	{
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff(Ind);


	/* Travel until stopped */
	for (cur_dis = 0; cur_dis <= tdis; )
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);

		/* Stopped by walls/doors */
		if (!cave_floor_bold(Depth, ny, nx)) break;

		/* Advance the distance */
		cur_dis++;

		/* Save the new location */
		x = nx;
		y = ny;

		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		/* Display it for each player */
		for (i = 1; i < NumPlayers + 1; i++)
		{
			int dispx, dispy;

			/* Use this player */
			p_ptr = Players[i];

			/* If he's not here, skip him */
			if (p_ptr->dun_depth != Depth)
				continue;

			/* The player can see the (on screen) missile */
			if (panel_contains(y, x) && player_can_see_bold(i, y, x))
			{
				/* Draw, Hilite, Fresh, Pause, Erase */
				dispy = y - p_ptr->panel_row_prt;
				dispx = x - p_ptr->panel_col_prt;

				/* Remember the projectile */
				//p_ptr->scr_info[dispy][dispx].c = missile_char;
				//p_ptr->scr_info[dispy][dispx].a = missile_attr;

				/* Tell the client */
				//Stream_tile(i, p_ptr, dispy, dispx);

				/* Tell the client */
				(void)send_air_char(i, dispy, dispx, missile_attr, missile_char, cur_dis, 1);

				/* Flush and wait */
				//if (cur_dis % tmul) Send_flush(i);

				/* Restore */
				//lite_spot(i, y, x);
			}

			/* The player cannot see the missile */
			else
			{
				/* Pause anyway, for consistancy */
				/*Term_xtra(TERM_XTRA_DELAY, msec);*/
			}
		}

		/* Restore the player pointer */
		p_ptr = q_ptr;

		/* Player here, hit him */
		if (cave[Depth][y][x].m_idx < 0)
		{
			cave_type *c_ptr = &cave[Depth][y][x];

			q_ptr = Players[0 - c_ptr->m_idx];

			/* AD hack cntd -- "pass over" if players aren't hostile */
			if (!pvp_okay(Ind, 0 - c_ptr->m_idx, (p_ptr->target_who == c_ptr->m_idx ? 2 : 3) ))
			{
				continue;
			}

			/* Check the visibility */
			visible = p_ptr->play_vis[0 - c_ptr->m_idx];

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, q_ptr->ac + q_ptr->to_a, visible))
			{
				char pvp_name[80];

				/* Get the name */
				my_strcpy(pvp_name, q_ptr->name, 80);

				/* Handle unseen player */
				if (!visible)
				{
					/* Invisible player */
					msg_format(Ind, "The %s finds a mark.", o_name);
					sound(Ind, MSG_SHOOT_HIT);
					msg_format(0 - c_ptr->m_idx, "You are hit by a %s!", o_name);
				}

				/* Handle visible player */
				else
				{
					/* Messages */
					msg_format(Ind, "The %s hits %s.", o_name, pvp_name);
					sound(Ind, MSG_SHOOT_HIT);
					msg_format(0 - c_ptr->m_idx, "%^s hits you with a %s.", p_ptr->name, o_name);

					/* Track this player's health */
					health_track(Ind, c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux_player(o_ptr, tdam, q_ptr);
				tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* XXX Reduce damage by 1/3 */
				tdam = (tdam + 2) / 3;

				/* Take damage */
				take_hit(0 - c_ptr->m_idx, tdam, p_ptr->name);

				/* Stop looking */
				break;
			}
		}

		/* Monster here, Try to hit it */
		if (cave[Depth][y][x].m_idx > 0)
		{
			cave_type *c_ptr = &cave[Depth][y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Check the visibility */
			visible = p_ptr->mon_vis[c_ptr->m_idx];

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, r_ptr->ac, visible))
			{
				bool fear = FALSE;

				/* Assume a default death */
				cptr note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags3 & RF3_DEMON) ||
				    (r_ptr->flags3 & RF3_UNDEAD) ||
				    (r_ptr->flags2 & RF2_STUPID) ||
				    (strchr("Evg", r_ptr->d_char)))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}


				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msg_format(Ind, "The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(Ind, m_name, c_ptr->m_idx, 0);

					/* Message */
					msg_format(Ind, "The %s hits %s.", o_name, m_name);

					/* Hack -- Track this monster race */
					if (visible) monster_race_track(Ind, m_ptr->r_idx);

					/* Hack -- Track this monster */
					if (visible) health_track(Ind, c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, p_ptr->mon_vis[c_ptr->m_idx]);
				tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* Complex message */
				if (wizard)
				{
					msg_format(Ind, "You do %d (out of %d) damage.",
					           tdam, m_ptr->hp);
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(Ind, c_ptr->m_idx, tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(Ind, c_ptr->m_idx, tdam);

					/* Take note */
					if (fear && visible && !(r_ptr->flags2 & RF2_WANDERER))
					{
						char m_name[80];

						/* Sound */
						sound(Ind, MSG_FLEE);

						/* Get the monster name (or "it") */
						monster_desc(Ind, m_name, c_ptr->m_idx, 0);

						/* Message */
						msg_format(Ind, "%^s flees in terror!", m_name);
					}
				}

				/* Stop looking */
				break;
			}
		}
	}

	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(o_ptr) : 0);

	/* Drop (or break) near that location */
	if (!magic) drop_near(o_ptr, j, Depth, y, x);
}



/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 */
void do_cmd_throw(int Ind, int item, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	int Depth = p_ptr->dun_depth;

	int			i, j, y, x, ny, nx, ty, tx;
	int			chance, tdam, tdis;
	int			mul, div;
	int			cur_dis, visible;

	object_type         throw_obj;
	object_type		*o_ptr;

	bool		hit_body = FALSE;

	int			missile_attr;
	int			missile_char;

	char		o_name[80];

	/* Check preventive inscription '^v' */
	__trap(Ind, CPI(p_ptr, 'v'));

	/*int			msec = delay_factor * delay_factor * delay_factor;*/

	/* Restrict ghosts */
	if ( ((p_ptr->ghost) || (p_ptr->fruit_bat && item >= 0)) && !(p_ptr->dm_flags & DM_GHOST_HANDS) )
	{
		msg_print(Ind, "You cannot throw things!");
		return;
	}

	/* Access the item (if in the pack) */
	if (item >= 0)
	{
		o_ptr = &(p_ptr->inventory[item]);
	}
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		o_ptr = &o_list[0 - item];
	}
	if(!o_ptr->tval) {
      		msg_print(Ind, "There is nothing there to throw");
                return;
	};

	/* Check guard inscription '!v' */ 
	__trap(Ind, CGI(o_ptr, 'v'));

	/* Never throw artifacts */
	if (artifact_p(o_ptr))
	{
		msg_print(Ind, "You can not throw this!");
		return;	
	}	

	/* Get a direction (or cancel) */
	p_ptr->command_dir = dir;
	if (!get_aim_dir(Ind, &dir)) return;

	/* Create a "local missile object" */
	throw_obj = *o_ptr;
	throw_obj.number = 1;

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(o_ptr, &throw_obj, 1);

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
		floor_item_optimize(0 - item);
		floor_item_notify(Ind, 0 - item, TRUE);
	}

	/* Use the local object */
	o_ptr = &throw_obj;

	/* Description */
	object_desc(Ind, o_name, sizeof(o_name), o_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(o_ptr);
	missile_char = object_char(o_ptr);


	/* Extract a "distance multiplier" */
	mul = 10;

	/* Enforce a minimum "weight" of one pound */
	div = ((o_ptr->weight > 10) ? o_ptr->weight : 10);

	/* Hack -- Distance -- Reward strength, penalize weight */
	tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

	/* Max distance of 10 */
	if (tdis > 10) tdis = 10;

	/* Hack -- Base damage from thrown object */
	tdam = damroll(o_ptr->dd, o_ptr->ds) + o_ptr->to_d;

	/* Chance of hitting */
	chance = (p_ptr->skill_tht + (p_ptr->to_h * BTH_PLUS_ADJ));


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay(Ind))
	{
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff(Ind);


	/* Travel until stopped */
	for (cur_dis = 0; cur_dis <= tdis; )
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);

		/* Stopped by walls/doors */
		if (!cave_floor_bold(Depth, ny, nx)) 
		{
			/* Special case: potion VS house door */
			if (o_ptr->tval == TV_POTION &&
				 cave[Depth][ny][nx].feat >= FEAT_HOME_HEAD && 
				 cave[Depth][ny][nx].feat <= FEAT_HOME_TAIL) 
			{
				/* Break it */
				hit_body = TRUE;
				
				/* Find suitable color */
				for (i = FEAT_HOME_HEAD; i < FEAT_HOME_TAIL + 1; i++)
				{
					if (f_info[i].x_attr == missile_attr || f_info[i].x_attr == color_opposite(missile_attr)) 
					{
						/* Pick a house */
						if ((j = pick_house(Depth, ny, nx)) == -1) break;

						/* Must own the house */
						if (!house_owned_by(Ind,j)) break;
						
						/* Chance to fail */
						if (randint(100) > p_ptr->sc) break;

						/* Perform colorization */
						houses[j].strength = i - FEAT_HOME_HEAD;
						cave[Depth][ny][nx].feat = i;
						everyone_lite_spot(Depth, ny, nx);
						
						/* Done */
						break;
					}
				}		
			}
			break;
		}

		/* Advance the distance */
		cur_dis++;

		/* Save the new location */
		x = nx;
		y = ny;

		/* Save the old "player pointer" */
		q_ptr = p_ptr;

		/* Display it for each player */
		for (i = 1; i < NumPlayers + 1; i++)
		{
			int dispx, dispy;

			/* Use this player */
			p_ptr = Players[i];

			/* If he's not here, skip him */
			if (p_ptr->dun_depth != Depth)
				continue;

			/* The player can see the (on screen) missile */
			if (panel_contains(y, x) && player_can_see_bold(i, y, x))
			{
				/* Draw, Hilite, Fresh, Pause, Erase */
				dispy = y - p_ptr->panel_row_prt;
				dispx = x - p_ptr->panel_col_prt;

				/* Remember the projectile */
				//p_ptr->scr_info[dispy][dispx].c = missile_char;
				//p_ptr->scr_info[dispy][dispx].a = missile_attr;

				/* Tell the client */
				//Stream_tile(i, p_ptr, dispy, dispx);

				/* Tell the client */
				(void)send_air_char(i, dispy, dispx, missile_attr, missile_char, cur_dis, 1);

				/* Flush and wait */
				//if (cur_dis % 2) Send_flush(i);

				/* Restore */
				//lite_spot(i, y, x);
			}

			/* The player cannot see the missile */
			else
			{
				/* Pause anyway, for consistancy */
				/*Term_xtra(TERM_XTRA_DELAY, msec);*/
			}
		}

		/* Restore the player pointer */
		p_ptr = q_ptr;


		/* Player here, try to hit him */
		if (cave[Depth][y][x].m_idx < 0)
		{
			cave_type *c_ptr = &cave[Depth][y][x];

			q_ptr = Players[0 - c_ptr->m_idx];

			/* Check the visibility */
			visible = p_ptr->play_vis[0 - c_ptr->m_idx];

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit him (penalize range) */
			if (test_hit_fire(chance - cur_dis, q_ptr->ac + q_ptr->to_a, visible))
			{
				/* Handle unseen player */
				if (!visible)
				{
					/* Messages */
					msg_format(Ind, "The %s finds a mark!", o_name);
					msg_format(0 - c_ptr->m_idx, "You are hit by a %s!", o_name);
				}
				
				/* Don't do damage if players aren't hostile */
				if (!pvp_okay(Ind, 0 - c_ptr->m_idx, 0))
				{
					hit_body = FALSE;

					/* Messages */
					if (visible)
					{
						msg_format(Ind, "%s shrugs off the %s.", q_ptr->name, o_name);
						msg_format(0 - c_ptr->m_idx, "%s throws you %s %s!", p_ptr->name, 
											(is_a_vowel(o_name[0]) ? "an" : "a"), o_name);
					}
					/* Stop */
					break;
				}
				
				/* Handle visible player */
				if (visible)
				{
					/* Messages */
					msg_format(Ind, "The %s hits %s.", o_name, q_ptr->name);
					msg_format(0 - c_ptr->m_idx, "%s hits you with a %s!", p_ptr->name, o_name);

					/* Track player's health */
					health_track(Ind, c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux_player(o_ptr, tdam, q_ptr);
				tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* XXX Reduce damage by 1/3 */
				tdam = (tdam + 2) / 3;

				/* Take damage */
				take_hit(0 - c_ptr->m_idx, tdam, p_ptr->name);

				/* Stop looking */
				break;
			}
		}

		/* Monster here, Try to hit it */
		if (cave[Depth][y][x].m_idx > 0)
		{
			cave_type *c_ptr = &cave[Depth][y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Check the visibility */
			visible = p_ptr->mon_vis[c_ptr->m_idx];

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, r_ptr->ac, visible))
			{
				bool fear = FALSE;

				/* Assume a default death */
				cptr note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags3 & RF3_DEMON) ||
				    (r_ptr->flags3 & RF3_UNDEAD) ||
				    (r_ptr->flags2 & RF2_STUPID) ||
				    (strchr("Evg", r_ptr->d_char)))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}


				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msg_format(Ind, "The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(Ind, m_name, c_ptr->m_idx, 0);

					/* Message */
					msg_format(Ind, "The %s hits %s.", o_name, m_name);

					/* Hack -- Track this monster race */
					if (visible) monster_race_track(Ind, m_ptr->r_idx);

					/* Hack -- Track this monster */
					if (visible) health_track(Ind, c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux(Ind, o_ptr, tdam, m_ptr, p_ptr->mon_vis[c_ptr->m_idx]);
				tdam = critical_shot(Ind, o_ptr->weight, o_ptr->to_h, tdam);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* Complex message */
				if (wizard)
				{
					msg_format(Ind, "You do %d (out of %d) damage.",
					           tdam, m_ptr->hp);
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(Ind, c_ptr->m_idx, tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(Ind, c_ptr->m_idx, tdam);

					/* Take note */
					if (fear && visible && !(r_ptr->flags2 & RF2_WANDERER))
					{
						char m_name[80];

						/* Sound */
						sound(Ind, MSG_FLEE);

						/* Get the monster name (or "it") */
						monster_desc(Ind, m_name, c_ptr->m_idx, 0);

						/* Message */
						msg_format(Ind, "%^s flees in terror!", m_name);
					}
				}

				/* Stop looking */
				break;
			}
		}
	}

	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(o_ptr) : 0);

	/* Hack -- newbies_cannot_drop -- break item when throwing */
	if ((p_ptr->lev == 1) && (cfg_newbies_cannot_drop))
		j = 100;

	/* Drop (or break) near that location */
	drop_near(o_ptr, j, Depth, y, x);
}


/*
 * Buy a house.  It is assumed that the player already knows the
 * price.
 
 Hacked to sell houses for half price. -APD-
 
 */
void do_cmd_purchase_house(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;

	int y, x, i, factor, price;
	cave_type *c_ptr;

	/* Check preventive inscription '^h' */
	__trap(Ind, CPI(p_ptr, 'h'));

	/* Ghosts cannot buy houses */
	if(!(p_ptr->dm_flags & DM_HOUSE_CONTROL))  {
		if ( (p_ptr->ghost) || (p_ptr->fruit_bat) )
		{
			/* Message */
			msg_print(Ind, "You cannot buy a house.");

			return;
		}
	}

	/* Check for no-direction -- confirmation (when selling house) */
	if (!VALID_DIR(dir))
	{
			i = p_ptr->current_house;
			p_ptr->current_house = -1;

			if (i == -1)
			{
				/* No house, message */
				msg_print(Ind, "You see nothing to sell there.");
				return;
			}

			/* Get requested grid */
			c_ptr = &cave[Depth][houses[i].door_y][houses[i].door_x];

			/* Take player's CHR into account */
			factor = adj_chr_gold[p_ptr->stat_ind[A_CHR]];
			price = (unsigned long) houses[i].price * factor / 100;

			if (house_owned(i))
			{
				/* Is it owned by this player? */
				if (house_owned_by(Ind,i))
				{
					/* house is no longer owned */
					disown_house(i);

					msg_format(Ind, "You sell your house for %ld gold.", price/2);

					 /* Get the money */
					p_ptr->au += price / 2;

					/* Window */
					p_ptr->window |= (PW_INVEN);

					/* Redraw */
					p_ptr->redraw |= (PR_GOLD);

					/* Done */
					return;
					}
			}

			/* No house, message */
			msg_print(Ind, "You don't own this house.");
			return;
	}

	/* Be sure we have a direction */
	if (VALID_DIR(dir))
	{
		/* Get requested direction */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get requested grid */
		c_ptr = &cave[Depth][y][x];

		/* Check for a house */
		if ((i = pick_house(Depth, y, x)) == -1)
		{
			/* No house, message */
			msg_print(Ind, "You see nothing to buy there.");
			return;
		}

		/* Take player's CHR into account */
		factor = adj_chr_gold[p_ptr->stat_ind[A_CHR]];
		price = (unsigned long) houses[i].price * factor / 100;


		/* Check for already-owned house */
		if (house_owned(i))
		{
			
			/* Is it owned by this player? */
			if (house_owned_by(Ind,i))
			{
				if (house_inside(Ind, i)) 
				{
					/* Hack -- Enter own store */
					command_new = '_';
					do_cmd_store(Ind,i);
				}
				else
				{
					/* Delay house transaction */
					p_ptr->current_house = i;
					/* Tell the client about the price */
					send_store_sell(Ind, price/2);
				}
				return;
			}

			if (p_ptr->dm_flags & DM_HOUSE_CONTROL)
			{
				disown_house(i);

				msg_format(Ind, "The house has been reset.");

				return;
			}

			/* Message */
			msg_print(Ind, "That house is already owned.");
			
			/* No sale */
			return;
		}

		if (Depth == 0)
		{
			/* houses in town are *ASTRONOMICAL* in price due to location, location, location. */
			price =(unsigned long)price *5L; 
		}

		/* Check for enough funds */
		if (price > p_ptr->au)
		{
			/* Not enough money, message */
			msg_print(Ind, "You do not have enough money.");
			return;
		}

		/* Check if we have too many houses already */
		if( cfg_max_houses && houses_owned(Ind) >= cfg_max_houses )
		{
			/* Too many houses owned already */
			msg_print(Ind, "You own too many houses already.");
			return;
		}

		/* Open the door */
		c_ptr->feat = FEAT_HOME_OPEN;

		/* Reshow */
		everyone_lite_spot(Depth, y, x);

		/* Take some of the player's money */
		p_ptr->au -= price;

		/* The house is now owned */
		set_house_owner(Ind,i);

		/* Redraw */
		p_ptr->redraw |= (PR_GOLD);
	}
}
