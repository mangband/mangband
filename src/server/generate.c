/* File: generate.c */

/* Purpose: Dungeon generation */

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
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines...  Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "v_info.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors.  Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type.  The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).  This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the old 33x11 blocks)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  It may be annoying enough to go back to 33x11
 * blocks to prevent this visual situation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits".  The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon).  Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
 */


/*
 * Dungeon generation values
 */
#define DUN_ROOMS	50	/* Number of rooms to attempt */
#define DUN_UNUSUAL	200	/* Level/chance of unusual room */
#define DUN_DEST	15	/* 1/chance of having a destroyed level */

/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND	10	/* Chance of random direction */
#define DUN_TUN_CHG	30	/* Chance of changing direction */
#define DUN_TUN_CON	15	/* Chance of extra tunneling */
#define DUN_TUN_PEN	25	/* Chance of doors at room entrances */
#define DUN_TUN_JCT	90	/* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_DEN	5	/* Density of streamers */
#define DUN_STR_RNG	2	/* Width of streamers */
#define DUN_STR_MAG	3	/* Number of magma streamers */
#define DUN_STR_MC	90	/* 1/chance of treasure per magma */
#define DUN_STR_QUA	2	/* Number of quartz streamers */
#define DUN_STR_QC	40	/* 1/chance of treasure per quartz */

/*
 * Dungeon treausre allocation values
 */
#define DUN_AMT_ROOM	9	/* Amount of objects for rooms */
#define DUN_AMT_ITEM	3	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD	3	/* Amount of treasure for rooms/corridors */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR		1	/* Hallway */
#define ALLOC_SET_ROOM		2	/* Room */
#define ALLOC_SET_BOTH		3	/* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1	/* Rubble */
#define ALLOC_TYP_TRAP		3	/* Trap */
#define ALLOC_TYP_GOLD		4	/* Gold */
#define ALLOC_TYP_OBJECT	5	/* Object */



/*
 * The "size" of a "generation block" in grids
 */
#define BLOCK_HGT	11
#define BLOCK_WID	11

/*
 * Maximum numbers of rooms along each axis (currently 6x6)
 */
#define MAX_ROOMS_ROW	(MAX_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(MAX_WID / BLOCK_WID)


/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	100
#define DOOR_MAX	400
#define WALL_MAX	1000
#define TUNN_MAX	1800


/*
 * Maximal number of room types
 */
#define ROOM_MAX	9



/*
 * Simple structure to hold a map location
 */

typedef struct coord coord;

struct coord
{
	byte y;
	byte x;
};


/*
 * Room type information
 */

typedef struct room_data room_data;

struct room_data
{
	/* Required size in blocks */
	s16b dy1, dy2, dx1, dx2;

	/* Hack -- minimum level */
	s16b level;
};


/*
 * Structure to hold all "dungeon generation" data
 */

typedef struct dun_data dun_data;

struct dun_data
{
	/* Array of centers of rooms */
	int cent_n;
	coord cent[CENT_MAX];

	/* Array of possible door locations */
	int door_n;
	coord door[DOOR_MAX];

	/* Array of wall piercing locations */
	int wall_n;
	coord wall[WALL_MAX];

	/* Array of tunnel grids */
	int tunn_n;
	coord tunn[TUNN_MAX];

	/* Number of blocks along each axis */
	int row_rooms;
	int col_rooms;

	/* Array of which blocks are used */
	bool room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	/* Hack -- there is a pit/nest on this level */
	bool crowded;
};


/*
 * Dungeon generation data -- see "cave_gen()"
 */
static dun_data *dun;


/*
 * Array of room types (assumes 11x11 blocks)
 */
static room_data room[ROOM_MAX] =
{
	{ 0, 0, 0, 0, 0 },		/* 0 = Nothing */
	{ 0, 0, -1, 1, 1 },		/* 1 = Simple (33x11) */
	{ 0, 0, -1, 1, 1 },		/* 2 = Overlapping (33x11) */
	{ 0, 0, -1, 1, 3 },		/* 3 = Crossed (33x11) */
	{ 0, 0, -1, 1, 3 },		/* 4 = Large (33x11) */
	{ 0, 0, -1, 1, 5 },		/* 5 = Monster nest (33x11) */
	{ 0, 0, -1, 1, 5 },		/* 6 = Monster pit (33x11) */
	{ 0, 1, -1, 1, 5 },		/* 7 = Lesser vault (33x22) */
	{ -1, 2, -2, 3, 10 }	/* 8 = Greater vault (66x44) */
};



/*
 * Always picks a correct direction
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
	/* Extract vertical and horizontal directions */
	*rdir = (y1 == y2) ? 0 : (y1 < y2) ? 1 : -1;
	*cdir = (x1 == x2) ? 0 : (x1 < x2) ? 1 : -1;

	/* Never move diagonally */
	if (*rdir && *cdir)
	{
		if (rand_int(100) < 50)
		{
			*rdir = 0;
		}
		else
		{
			*cdir = 0;
		}
	}
}


/*
 * Pick a random direction
 */
static void rand_dir(int *rdir, int *cdir)
{
	/* Pick a random direction */
	int i = rand_int(4);

	/* Extract the dy/dx components */
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


/*
 * Returns random co-ordinates for player starts
 */
static void new_player_spot(int Depth)
{
	int        y, x;

	/* Place the player */
	while (1)
	{
		/* Pick a legal spot */
		y = rand_range(1, MAX_HGT - 2);
		x = rand_range(1, MAX_WID - 2);

		/* Must be a "naked" floor grid */
		if (!cave_naked_bold(Depth, y, x)) continue;

		/* Refuse to start on anti-teleport grids */
		if (cave[Depth][y][x].info & CAVE_ICKY) continue;

		/* Done */
		break;
	}

	/* Save the new grid */
	level_rand_y[Depth] = y;
	level_rand_x[Depth] = x;
}



/*
 * Count the number of walls adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y, x)"
 *
 * We count only granite walls and permanent walls.
 */
static int next_to_walls(int Depth, int y, int x)
{
	int        k = 0;

	if (cave[Depth][y+1][x].feat >= FEAT_WALL_EXTRA) k++;
	if (cave[Depth][y-1][x].feat >= FEAT_WALL_EXTRA) k++;
	if (cave[Depth][y][x+1].feat >= FEAT_WALL_EXTRA) k++;
	if (cave[Depth][y][x-1].feat >= FEAT_WALL_EXTRA) k++;

	return (k);
}



/*
 * Convert existing terrain type to rubble
 */
static void place_rubble_aux(int Depth, int y, int x)
{
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Create rubble */
	c_ptr->feat = FEAT_RUBBLE;
}

/*
 * Find secondary spot for rubble
 *
 * Note: unlike doors, first element might generate anywhere,
 * leaving the possibility of useless rubble. That is intenteded
 */
static void place_rubble(int Depth, int y, int x)
{
	int i,j;
	cave_type *c_ptr = &cave[Depth][y][x];

	place_rubble_aux(Depth, y, x);

	for (j = -1; j < 2; j++) {
		for (i = -1; i < 2; i++) {
			/* Skip corners */
			if (abs(i+j) != 1) continue; 
			
			/* Check Bounds */
			if (!in_bounds(Depth, y+j, x+i)) continue;

			/* Totally useless AKA Require a certain number of adjacent walls */
			if (next_to_walls(Depth, y+j, x+i) < 2) continue;			
			
			/* Require wall grid */
			if (cave_naked_bold(Depth, y+j, x+i)) continue;

			/* Require usefullness -- Not nessecary, since first element sucks anyway */
			/* if (next_to_walls(Depth, y+j*-1, x+i*-1) < 1) continue; */

			/* Require an empty grid on the opposite side */
			if (!cave_naked_bold(Depth, y+j*-1, x+i*-1)) continue;
		
			/* Place on the opposite side */
			place_rubble_aux(Depth, y+j*-1, x+i*-1);
			
			/* Done */
			return;		
		}
	}
	/* None */
}

/*
 * Convert existing terrain type to "up stairs"
 */
static void place_up_stairs(int Depth, int y, int x)
{
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Create up stairs */
	c_ptr->feat = FEAT_LESS;
}


/*
 * Convert existing terrain type to "down stairs"
 */
static void place_down_stairs(int Depth, int y, int x)
{
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Create down stairs */
	c_ptr->feat = FEAT_MORE;
}





/*
 * Place an up/down staircase at given location
 */
static void place_random_stairs(int Depth, int y, int x)
{
	/* Paranoia */
	if (!cave_clean_bold(Depth, y, x)) return;

	/* Choose a staircase */
	if (!Depth)
	{
		place_down_stairs(Depth, y, x);
	}
	else if (is_quest(Depth) || (Depth >= MAX_DEPTH-1))
	{
		place_up_stairs(Depth, y, x);
	}
	else if (rand_int(100) < 50)
	{
		place_down_stairs(Depth, y, x);
	}
	else
	{
		place_up_stairs(Depth, y, x);
	}
}


/*
 * Place a locked door at the given location
 */
static void place_locked_door(int Depth, int y, int x)
{
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Create locked door */
	c_ptr->feat = FEAT_DOOR_HEAD + (byte_hack)randint(7);
}


/*
 * Place a secret door at the given location
 */
static void place_secret_door(int Depth, int y, int x)
{
	cave_type *c_ptr = &cave[Depth][y][x];

	/* Create secret door */
	c_ptr->feat = FEAT_SECRET;
}


/*
 * Place a random type of door at the given location
 */
static void place_random_door(int Depth, int y, int x)
{
	int tmp;

	cave_type *c_ptr = &cave[Depth][y][x];

	/* Choose an object */
	tmp = rand_int(1000);

	/* Open doors (300/1000) */
	if (tmp < 300)
	{
		/* Create open door */
		c_ptr->feat = FEAT_OPEN;
	}

	/* Broken doors (100/1000) */
	else if (tmp < 400)
	{
		/* Create broken door */
		c_ptr->feat = FEAT_BROKEN;
	}

	/* Secret doors (200/1000) */
	else if (tmp < 600)
	{
		/* Create secret door */
		c_ptr->feat = FEAT_SECRET;
	}

	/* Closed doors (300/1000) */
	else if (tmp < 900)
	{
		/* Create closed door */
		c_ptr->feat = FEAT_DOOR_HEAD + 0x00;
	}

	/* Locked doors (99/1000) */
	else if (tmp < 999)
	{
		/* Create locked door */
		c_ptr->feat = FEAT_DOOR_HEAD + (byte_hack)randint(7);
	}

	/* Stuck doors (1/1000) */
	else
	{
		/* Create jammed door */
		c_ptr->feat = FEAT_DOOR_HEAD + 0x08 + (byte_hack)rand_int(8);
	}
}



/*
 * Places some staircases near walls
 */
static void alloc_stairs(int Depth, int feat, int num, int walls)
{
	int                 y, x, i, j, flag;

	cave_type		*c_ptr;


	/* Place "num" stairs */
	for (i = 0; i < num; i++)
	{
		/* Place some stairs */
		for (flag = FALSE; !flag; )
		{
			/* Try several times, then decrease "walls" */
			for (j = 0; !flag && j <= 3000; j++)
			{
				/* Pick a random grid */
				y = rand_int(Depth ? MAX_HGT : MAX_HGT);
				x = rand_int(Depth ? MAX_WID : MAX_WID);

				/* Require "naked" floor grid */
				if (!cave_naked_bold(Depth, y, x)) continue;

				/* Require a certain number of adjacent walls */
				if (next_to_walls(Depth, y, x) < walls) continue;

				/* Access the grid */
				c_ptr = &cave[Depth][y][x];

				/* Town -- must go down */
				if (!Depth)
				{
					/* Clear previous contents, add down stairs */
					c_ptr->feat = FEAT_MORE;
				}

				/* Quest -- must go up */
				else if (is_quest(Depth) || (Depth >= MAX_DEPTH-1))
				{
					/* Clear previous contents, add up stairs */
					c_ptr->feat = FEAT_LESS;

					/* Set this to be the starting location for people going down */
					level_down_y[Depth] = y;
					level_down_x[Depth] = x;
				}

				/* Requested type */
				else
				{
					/* Clear previous contents, add stairs */
					c_ptr->feat = feat;

					if (feat == FEAT_LESS)
					{
						/* Set this to be the starting location for people going down */
						level_down_y[Depth] = y;
						level_down_x[Depth] = x;
					}
					if (feat == FEAT_MORE)
					{
						/* Set this to be the starting location for people going up */
						level_up_y[Depth] = y;
						level_up_x[Depth] = x;
					}
				}

				/* All done */
				flag = TRUE;
			}

			/* Require fewer walls */
			if (walls) walls--;
		}
	}
}




/*
 * Allocates some objects (using "place" and "type")
 */
static void alloc_object(int Depth, int set, int typ, int num)
{
	int y, x, k;

	/* Place some objects */
	for (k = 0; k < num; k++)
	{
		/* Pick a "legal" spot */
		while (TRUE)
		{
			bool room;

			/* Location */
			y = rand_int(Depth ? MAX_HGT : MAX_HGT);
			x = rand_int(Depth ? MAX_WID : MAX_WID);

			/* Require "naked" floor grid */
			if (!cave_naked_bold(Depth, y, x)) continue;

			/* Check for "room" */
			room = (cave[Depth][y][x].info & CAVE_ROOM) ? TRUE : FALSE;

			/* Require corridor? */
			if ((set == ALLOC_SET_CORR) && room) continue;

			/* Require room? */
			if ((set == ALLOC_SET_ROOM) && !room) continue;

			/* Accept it */
			break;
		}

		/* Place something */
		switch (typ)
		{
			case ALLOC_TYP_RUBBLE:
			{
				place_rubble(Depth, y, x);
				break;
			}

			case ALLOC_TYP_TRAP:
			{
				place_trap(Depth, y, x);
				break;
			}

			case ALLOC_TYP_GOLD:
			{
				place_gold(Depth, y, x);
				break;
			}

			case ALLOC_TYP_OBJECT:
			{
                place_object(Depth, y, x, FALSE, FALSE, 0);
				break;
			}
		}
	}
}



/*
 * Places "streamers" of rock through dungeon
 *
 * Note that their are actually six different terrain features used
 * to represent streamers.  Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold.  The
 * hidden gold types are currently unused.
 */
static void build_streamer(int Depth, int feat, int chance)
{
	int		i, tx, ty;
	int		y, x, dir;

	cave_type *c_ptr;

	/* Hack -- Choose starting point */
	y = rand_spread((Depth ? MAX_HGT : MAX_HGT) / 2, 10);
	x = rand_spread((Depth ? MAX_WID : MAX_WID) / 2, 15);

	/* Choose a random compass direction */
	dir = ddd[rand_int(8)];

	/* Place streamer into dungeon */
	while (TRUE)
	{
		/* One grid per density */
		for (i = 0; i < DUN_STR_DEN; i++)
		{
			int d = DUN_STR_RNG;

			/* Pick a nearby grid */
			while (1)
			{
				ty = rand_spread(y, d);
				tx = rand_spread(x, d);
				if (!in_bounds2(Depth, ty, tx)) continue;
				break;
			}

			/* Access the grid */
			c_ptr = &cave[Depth][ty][tx];

			/* Only convert "granite" walls */
			if (c_ptr->feat < FEAT_WALL_EXTRA) continue;
			if (c_ptr->feat > FEAT_WALL_SOLID) continue;

			/* Clear previous contents, add proper vein type */
			c_ptr->feat = feat;

			/* Hack -- Add some (known) treasure */
			if (rand_int(chance) == 0) c_ptr->feat += 0x04;
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Quit before leaving the dungeon */
		if (!in_bounds(Depth, y, x)) break;
	}
}


/*
 * Build a destroyed level
 */
static void destroy_level(int Depth)
{
	int y1, x1, y, x, k, t, n;

	cave_type *c_ptr;

	/* Note destroyed levels */
	/*if (cheat_room) msg_print("Destroyed Level");*/

	/* Drop a few epi-centers (usually about two) */
	for (n = 0; n < randint(5); n++)
	{
		/* Pick an epi-center */
		y1 = rand_range(5, (Depth ? MAX_HGT : MAX_HGT) - 6);
		x1 = rand_range(5, (Depth ? MAX_WID : MAX_WID) - 6);

		/* Big area of affect */
		for (y = (y1 - 15); y <= (y1 + 15); y++)
		{
			for (x = (x1 - 15); x <= (x1 + 15); x++)
			{
				/* Skip illegal grids */
				if (!in_bounds(Depth, y, x)) continue;

				/* Extract the distance */
				k = distance(y1, x1, y, x);

				/* Stay in the circle of death */
				if (k >= 16) continue;

				/* Delete the monster (if any) */
				delete_monster(Depth, y, x);

				/* Destroy valid grids */
				if (cave_valid_bold(Depth, y, x))
				{
					/* Delete the object (if any) */
					delete_object(Depth, y, x);

					/* Access the grid */
					c_ptr = &cave[Depth][y][x];

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

					/* No longer part of a room or vault */
					c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

					/* No longer illuminated or known */
					c_ptr->info &= ~CAVE_GLOW;
				}
			}
		}
	}
}



/*
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
static void vault_objects(int Depth, int y, int x, int num)
{
	int        i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (1)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				if (!in_bounds(Depth, j, k)) continue;
				break;
			}

			/* Require "clean" floor space */
			if (!cave_clean_bold(Depth, j, k)) continue;

			/* Place an item */
			if (rand_int(100) < 75)
			{
                place_object(Depth, j, k, FALSE, FALSE, 0);
			}

			/* Place gold */
			else
			{
				place_gold(Depth, j, k);
			}

			/* Placement accomplished */
			break;
		}
	}
}


/*
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(int Depth, int y, int x, int yd, int xd)
{
	int		count, y1, x1;

	/* Place traps */
	for (count = 0; count <= 5; count++)
	{
		/* Get a location */
		while (1)
		{
			y1 = rand_spread(y, yd);
			x1 = rand_spread(x, xd);
			if (!in_bounds(Depth, y1, x1)) continue;
			break;
		}

		/* Require "naked" floor grids */
		if (!cave_naked_bold(Depth, y1, x1)) continue;

		/* Place the trap */
		place_trap(Depth, y1, x1);

		/* Done */
		break;
	}
}


/*
 * Place some traps with a given displacement of given location
 */
static void vault_traps(int Depth, int y, int x, int yd, int xd, int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		vault_trap_aux(Depth, y, x, yd, xd);
	}
}


/*
 * Hack -- Place some sleeping monsters near the given location
 */
static void vault_monsters(int Depth, int y1, int x1, int num)
{
	int          k, i, y, x;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++)
	{
		/* Try nine locations */
		for (i = 0; i < 9; i++)
		{
			int d = 1;

			/* Pick a nearby location */
			scatter(Depth, &y, &x, y1, x1, d, 0);

			/* Require "empty" floor grids */
			if (!cave_empty_bold(Depth, y, x)) continue;

			/* Place the monster (allow groups) */
			monster_level = Depth + 2;
			(void)place_monster(Depth, y, x, TRUE, TRUE);
			monster_level = Depth;
		}
	}
}




/*
 * Room building routines.
 *
 * Six basic room types:
 *   1 -- normal
 *   2 -- overlapping
 *   3 -- cross shaped
 *   4 -- large room with features
 *   5 -- monster nests
 *   6 -- monster pits
 *   7 -- simple vaults
 *   8 -- greater vaults
 */


/*
 * Type 1 -- normal rectangular rooms
 */
static void build_type1(int Depth, int yval, int xval)
{
	int			y, x, y2, x2;
	int                 y1, x1;

	bool		light;

	cave_type *c_ptr;


	/* Choose lite or dark */
	light = (Depth <= randint(25));


	/* Pick a room size */
	y1 = yval - randint(4);
	y2 = yval + randint(3);
	x1 = xval - randint(11);
	x2 = xval + randint(11);


	/* Place a full floor under the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}

	/* Walls around the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}


	/* Hack -- Occasional pillar room */
	if (rand_int(20) == 0)
	{
		for (y = y1; y <= y2; y += 2)
		{
			for (x = x1; x <= x2; x += 2)
			{
				c_ptr = &cave[Depth][y][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}
		}
	}

	/* Hack -- Occasional ragged-edge room */
	else if (rand_int(50) == 0)
	{
		for (y = y1 + 2; y <= y2 - 2; y += 2)
		{
			c_ptr = &cave[Depth][y][x1];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y][x2];
			c_ptr->feat = FEAT_WALL_INNER;
		}
		for (x = x1 + 2; x <= x2 - 2; x += 2)
		{
			c_ptr = &cave[Depth][y1][x];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y2][x];
			c_ptr->feat = FEAT_WALL_INNER;
		}
	}
}


/*
 * Type 2 -- Overlapping rectangular rooms
 */
static void build_type2(int Depth, int yval, int xval)
{
	int			y, x;
	int			y1a, x1a, y2a, x2a;
	int			y1b, x1b, y2b, x2b;

	bool		light;

	cave_type *c_ptr;



	/* Choose lite or dark */
	light = (Depth <= randint(25));


	/* Determine extents of the first room */
	y1a = yval - randint(4);
	y2a = yval + randint(3);
	x1a = xval - randint(11);
	x2a = xval + randint(10);

	/* Determine extents of the second room */
	y1b = yval - randint(3);
	y2b = yval + randint(4);
	x1b = xval - randint(10);
	x2b = xval + randint(11);


	/* Place a full floor for room "a" */
	for (y = y1a - 1; y <= y2a + 1; y++)
	{
		for (x = x1a - 1; x <= x2a + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}

	/* Place a full floor for room "b" */
	for (y = y1b - 1; y <= y2b + 1; y++)
	{
		for (x = x1b - 1; x <= x2b + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}


	/* Place the walls around room "a" */
	for (y = y1a - 1; y <= y2a + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1a-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2a+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1a - 1; x <= x2a + 1; x++)
	{
		c_ptr = &cave[Depth][y1a-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2a+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}

	/* Place the walls around room "b" */
	for (y = y1b - 1; y <= y2b + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1b-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2b+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1b - 1; x <= x2b + 1; x++)
	{
		c_ptr = &cave[Depth][y1b-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2b+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}



	/* Replace the floor for room "a" */
	for (y = y1a; y <= y2a; y++)
	{
		for (x = x1a; x <= x2a; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
		}
	}

	/* Replace the floor for room "b" */
	for (y = y1b; y <= y2b; y++)
	{
		for (x = x1b; x <= x2b; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
		}
	}
}



/*
 * Type 3 -- Cross shaped rooms
 *
 * Builds a room at a row, column coordinate
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So the "central pillar" runs from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work (with "bounds checking") for 5x5, or even
 * for unsymetric values like 4x3 or 5x3 or 3x4 or 3x5, or even larger.
 */
static void build_type3(int Depth, int yval, int xval)
{
	int			y, x, dy, dx, wy, wx;
	int			y1a, x1a, y2a, x2a;
	int			y1b, x1b, y2b, x2b;

	bool		light;

	cave_type *c_ptr;



	/* Choose lite or dark */
	light = (Depth <= randint(25));


	/* For now, always 3x3 */
	wx = wy = 1;

	/* Pick max vertical size (at most 4) */
	dy = rand_range(3, 4);

	/* Pick max horizontal size (at most 15) */
	dx = rand_range(3, 11);


	/* Determine extents of the north/south room */
	y1a = yval - dy;
	y2a = yval + dy;
	x1a = xval - wx;
	x2a = xval + wx;

	/* Determine extents of the east/west room */
	y1b = yval - wy;
	y2b = yval + wy;
	x1b = xval - dx;
	x2b = xval + dx;


	/* Place a full floor for room "a" */
	for (y = y1a - 1; y <= y2a + 1; y++)
	{
		for (x = x1a - 1; x <= x2a + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}

	/* Place a full floor for room "b" */
	for (y = y1b - 1; y <= y2b + 1; y++)
	{
		for (x = x1b - 1; x <= x2b + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}


	/* Place the walls around room "a" */
	for (y = y1a - 1; y <= y2a + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1a-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2a+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1a - 1; x <= x2a + 1; x++)
	{
		c_ptr = &cave[Depth][y1a-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2a+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}

	/* Place the walls around room "b" */
	for (y = y1b - 1; y <= y2b + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1b-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2b+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1b - 1; x <= x2b + 1; x++)
	{
		c_ptr = &cave[Depth][y1b-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2b+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}


	/* Replace the floor for room "a" */
	for (y = y1a; y <= y2a; y++)
	{
		for (x = x1a; x <= x2a; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
		}
	}

	/* Replace the floor for room "b" */
	for (y = y1b; y <= y2b; y++)
	{
		for (x = x1b; x <= x2b; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
		}
	}



	/* Special features (3/4) */
	switch (rand_int(4))
	{
		/* Large solid middle pillar */
		case 1:
		for (y = y1b; y <= y2b; y++)
		{
			for (x = x1a; x <= x2a; x++)
			{
				c_ptr = &cave[Depth][y][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}
		}
		break;

		/* Inner treasure vault */
		case 2:

		/* Build the vault */
		for (y = y1b; y <= y2b; y++)
		{
			c_ptr = &cave[Depth][y][x1a];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y][x2a];
			c_ptr->feat = FEAT_WALL_INNER;
		}
		for (x = x1a; x <= x2a; x++)
		{
			c_ptr = &cave[Depth][y1b][x];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y2b][x];
			c_ptr->feat = FEAT_WALL_INNER;
		}

		/* Place a secret door on the inner room */
		switch (rand_int(4))
		{
			case 0: place_secret_door(Depth, y1b, xval); break;
			case 1: place_secret_door(Depth, y2b, xval); break;
			case 2: place_secret_door(Depth, yval, x1a); break;
			case 3: place_secret_door(Depth, yval, x2a); break;
		}

		/* Place a treasure in the vault */
        place_object(Depth, yval, xval, FALSE, FALSE, 0);

		/* Let's guard the treasure well */
		vault_monsters(Depth, yval, xval, rand_int(2) + 3);

		/* Traps naturally */
		vault_traps(Depth, yval, xval, 4, 4, rand_int(3) + 2);

		break;


		/* Something else */
		case 3:

		/* Occasionally pinch the center shut */
		if (rand_int(3) == 0)
		{
			/* Pinch the east/west sides */
			for (y = y1b; y <= y2b; y++)
			{
				if (y == yval) continue;
				c_ptr = &cave[Depth][y][x1a - 1];
				c_ptr->feat = FEAT_WALL_INNER;
				c_ptr = &cave[Depth][y][x2a + 1];
				c_ptr->feat = FEAT_WALL_INNER;
			}

			/* Pinch the north/south sides */
			for (x = x1a; x <= x2a; x++)
			{
				if (x == xval) continue;
				c_ptr = &cave[Depth][y1b - 1][x];
				c_ptr->feat = FEAT_WALL_INNER;
				c_ptr = &cave[Depth][y2b + 1][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}

			/* Sometimes shut using secret doors */
			if (rand_int(3) == 0)
			{
				place_secret_door(Depth, yval, x1a - 1);
				place_secret_door(Depth, yval, x2a + 1);
				place_secret_door(Depth, y1b - 1, xval);
				place_secret_door(Depth, y2b + 1, xval);
			}
		}

		/* Occasionally put a "plus" in the center */
		else if (rand_int(3) == 0)
		{
			c_ptr = &cave[Depth][yval][xval];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y1b][xval];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][y2b][xval];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][yval][x1a];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][yval][x2a];
			c_ptr->feat = FEAT_WALL_INNER;
		}

		/* Occasionally put a pillar in the center */
		else if (rand_int(3) == 0)
		{
			c_ptr = &cave[Depth][yval][xval];
			c_ptr->feat = FEAT_WALL_INNER;
		}

		break;
	}
}


/*
 * Type 4 -- Large room with inner features
 *
 * Possible sub-types:
 *	1 - Just an inner room with one door
 *	2 - An inner room within an inner room
 *	3 - An inner room with pillar(s)
 *	4 - Inner room has a maze
 *	5 - A set of four inner rooms
 */
static void build_type4(int Depth, int yval, int xval)
{
	int        y, x, y1, x1;
	int                 y2, x2, tmp;

	bool		light;

	cave_type *c_ptr;



	/* Choose lite or dark */
	light = (Depth <= randint(25));


	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;


	/* Place a full floor under the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
			if (light) c_ptr->info |= CAVE_GLOW;
		}
	}

	/* Outer Walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}


	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_INNER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_INNER;
	}


	/* Inner room variations */
	switch (randint(5))
	{
		/* Just an inner room with a monster */
		case 1:

		/* Place a secret door */
		switch (randint(4))
		{
			case 1: place_secret_door(Depth, y1 - 1, xval); break;
			case 2: place_secret_door(Depth, y2 + 1, xval); break;
			case 3: place_secret_door(Depth, yval, x1 - 1); break;
			case 4: place_secret_door(Depth, yval, x2 + 1); break;
		}

		/* Place a monster in the room */
		vault_monsters(Depth, yval, xval, 1);

		break;


		/* Treasure Vault (with a door) */
		case 2:

		/* Place a secret door */
		switch (randint(4))
		{
			case 1: place_secret_door(Depth, y1 - 1, xval); break;
			case 2: place_secret_door(Depth, y2 + 1, xval); break;
			case 3: place_secret_door(Depth, yval, x1 - 1); break;
			case 4: place_secret_door(Depth, yval, x2 + 1); break;
		}

		/* Place another inner room */
		for (y = yval - 1; y <= yval + 1; y++)
		{
			for (x = xval -  1; x <= xval + 1; x++)
			{
				if ((x == xval) && (y == yval)) continue;
				c_ptr = &cave[Depth][y][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}
		}

		/* Place a locked door on the inner room */
		switch (randint(4))
		{
			case 1: place_locked_door(Depth, yval - 1, xval); break;
			case 2: place_locked_door(Depth, yval + 1, xval); break;
			case 3: place_locked_door(Depth, yval, xval - 1); break;
			case 4: place_locked_door(Depth, yval, xval + 1); break;
		}

		/* Monsters to guard the "treasure" */
		vault_monsters(Depth, yval, xval, randint(3) + 2);

		/* Object (80%) */
		if (rand_int(100) < 80)
		{
            place_object(Depth, yval, xval, FALSE, FALSE, 0);
		}

		/* Stairs (20%) */
		else
		{
			place_random_stairs(Depth, yval, xval);
		}

		/* Traps to protect the treasure */
		vault_traps(Depth, yval, xval, 4, 10, 2 + randint(3));

		break;


		/* Inner pillar(s). */
		case 3:

		/* Place a secret door */
		switch (randint(4))
		{
			case 1: place_secret_door(Depth, y1 - 1, xval); break;
			case 2: place_secret_door(Depth, y2 + 1, xval); break;
			case 3: place_secret_door(Depth, yval, x1 - 1); break;
			case 4: place_secret_door(Depth, yval, x2 + 1); break;
		}

		/* Large Inner Pillar */
		for (y = yval - 1; y <= yval + 1; y++)
		{
			for (x = xval - 1; x <= xval + 1; x++)
			{
				c_ptr = &cave[Depth][y][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}
		}

		/* Occasionally, two more Large Inner Pillars */
		if (rand_int(2) == 0)
		{
			tmp = randint(2);
			for (y = yval - 1; y <= yval + 1; y++)
			{
				for (x = xval - 5 - tmp; x <= xval - 3 - tmp; x++)
				{
					c_ptr = &cave[Depth][y][x];
					c_ptr->feat = FEAT_WALL_INNER;
				}
				for (x = xval + 3 + tmp; x <= xval + 5 + tmp; x++)
				{
					c_ptr = &cave[Depth][y][x];
					c_ptr->feat = FEAT_WALL_INNER;
				}
			}
		}

		/* Occasionally, some Inner rooms */
		if (rand_int(3) == 0)
		{
			/* Long horizontal walls */
			for (x = xval - 5; x <= xval + 5; x++)
			{
				c_ptr = &cave[Depth][yval-1][x];
				c_ptr->feat = FEAT_WALL_INNER;
				c_ptr = &cave[Depth][yval+1][x];
				c_ptr->feat = FEAT_WALL_INNER;
			}

			/* Close off the left/right edges */
			c_ptr = &cave[Depth][yval][xval-5];
			c_ptr->feat = FEAT_WALL_INNER;
			c_ptr = &cave[Depth][yval][xval+5];
			c_ptr->feat = FEAT_WALL_INNER;

			/* Secret doors (random top/bottom) */
			place_secret_door(Depth, yval - 3 + (randint(2) * 2), xval - 3);
			place_secret_door(Depth, yval - 3 + (randint(2) * 2), xval + 3);

			/* Monsters */
			vault_monsters(Depth, yval, xval - 2, randint(2));
			vault_monsters(Depth, yval, xval + 2, randint(2));

			/* Objects */
            if (rand_int(3) == 0) place_object(Depth, yval, xval - 2, FALSE, FALSE, 0);
            if (rand_int(3) == 0) place_object(Depth, yval, xval + 2, FALSE, FALSE, 0);
		}

		break;


		/* Maze inside. */
		case 4:

		/* Place a secret door */
		switch (randint(4))
		{
			case 1: place_secret_door(Depth, y1 - 1, xval); break;
			case 2: place_secret_door(Depth, y2 + 1, xval); break;
			case 3: place_secret_door(Depth, yval, x1 - 1); break;
			case 4: place_secret_door(Depth, yval, x2 + 1); break;
		}

		/* Maze (really a checkerboard) */
		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				if (0x1 & (x + y))
				{
					c_ptr = &cave[Depth][y][x];
					c_ptr->feat = FEAT_WALL_INNER;
				}
			}
		}

		/* Monsters just love mazes. */
		vault_monsters(Depth, yval, xval - 5, randint(3));
		vault_monsters(Depth, yval, xval + 5, randint(3));

		/* Traps make them entertaining. */
		vault_traps(Depth, yval, xval - 3, 2, 8, randint(3));
		vault_traps(Depth, yval, xval + 3, 2, 8, randint(3));

		/* Mazes should have some treasure too. */
		vault_objects(Depth, yval, xval, 3);

		break;


		/* Four small rooms. */
		case 5:

		/* Inner "cross" */
		for (y = y1; y <= y2; y++)
		{
			c_ptr = &cave[Depth][y][xval];
			c_ptr->feat = FEAT_WALL_INNER;
		}
		for (x = x1; x <= x2; x++)
		{
			c_ptr = &cave[Depth][yval][x];
			c_ptr->feat = FEAT_WALL_INNER;
		}

		/* Doors into the rooms */
		if (rand_int(100) < 50)
		{
			int i = randint(10);
			place_secret_door(Depth, y1 - 1, xval - i);
			place_secret_door(Depth, y1 - 1, xval + i);
			place_secret_door(Depth, y2 + 1, xval - i);
			place_secret_door(Depth, y2 + 1, xval + i);
		}
		else
		{
			int i = randint(3);
			place_secret_door(Depth, yval + i, x1 - 1);
			place_secret_door(Depth, yval - i, x1 - 1);
			place_secret_door(Depth, yval + i, x2 + 1);
			place_secret_door(Depth, yval - i, x2 + 1);
		}

		/* Treasure, centered at the center of the cross */
		vault_objects(Depth, yval, xval, 2 + randint(2));

		/* Gotta have some monsters. */
		vault_monsters(Depth, yval + 1, xval - 4, randint(4));
		vault_monsters(Depth, yval + 1, xval + 4, randint(4));
		vault_monsters(Depth, yval - 1, xval - 4, randint(4));
		vault_monsters(Depth, yval - 1, xval + 4, randint(4));

		break;
	}
}


/*
 * The following functions are used to determine if the given monster
 * is appropriate for inclusion in a monster nest or monster pit or
 * the given type.
 *
 * None of the pits/nests are allowed to include "unique" monsters,
 * or monsters which can "multiply".
 *
 * Some of the pits/nests are asked to avoid monsters which can blink
 * away or which are invisible.  This is probably a hack.
 *
 * The old method made direct use of monster "names", which is bad.
 *
 * Note the use of Angband 2.7.9 monster race pictures in various places.
 */


/*
 * Helper function for "monster nest (jelly)"
 */
static bool vault_aux_jelly(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Require icky thing, jelly, mold, or mushroom */
	if (!strchr("ijm,", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (animal)"
 */
static bool vault_aux_animal(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Require "animal" flag */
	if (!(r_ptr->flags3 & RF3_ANIMAL)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (undead)"
 */
static bool vault_aux_undead(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Require Undead */
	if (!(r_ptr->flags3 & RF3_UNDEAD)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (orc)"
 */
static bool vault_aux_orc(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Hack -- Require "o" monsters */
	if (!strchr("o", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (troll)"
 */
static bool vault_aux_troll(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Hack -- Require "T" monsters */
	if (!strchr("T", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (giant)"
 */
static bool vault_aux_giant(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Hack -- Require "P" monsters */
	if (!strchr("P", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- breath type for "vault_aux_dragon()"
 */
static u32b vault_aux_dragon_mask4;


/*
 * Helper function for "monster pit (dragon)"
 */
static bool vault_aux_dragon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Hack -- Require "d" or "D" monsters */
	if (!strchr("Dd", r_ptr->d_char)) return (FALSE);

	/* Hack -- Require correct "breath attack" */
	if (r_ptr->flags4 != vault_aux_dragon_mask4) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (demon)"
 */
static bool vault_aux_demon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Hack -- Require "U" monsters */
	if (!strchr("U", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}



/*
 * Type 5 -- Monster nests
 *
 * A monster nest is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type strewn about the room.
 *
 * The monsters are chosen from a set of 64 randomly selected monster
 * races, to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the nest.
 *
 * Currently, a monster nest is one of
 *   a nest of "jelly" monsters   (Dungeon level 5 and deeper)
 *   a nest of "animal" monsters  (Dungeon level 30 and deeper)
 *   a nest of "undead" monsters  (Dungeon level 50 and deeper)
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which
 * case the nest will be empty, and will not affect the level rating.
 *
 * Note that "monster nests" will never contain "unique" monsters.
 */
static void build_type5(int Depth, int yval, int xval)
{
	int			y, x, y1, x1, y2, x2;

	int			tmp, i;

	s16b		what[64];

	cave_type		*c_ptr;

	cptr		name;

	bool		empty = FALSE;


	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;


	/* Place the floor area */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
		}
	}

	/* Place the outer walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_INNER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_INNER;
	}


	/* Place a secret door */
	switch (randint(4))
	{
		case 1: place_secret_door(Depth, y1 - 1, xval); break;
		case 2: place_secret_door(Depth, y2 + 1, xval); break;
		case 3: place_secret_door(Depth, yval, x1 - 1); break;
		case 4: place_secret_door(Depth, yval, x2 + 1); break;
	}


	/* Hack -- Choose a nest type */
	tmp = randint(Depth);

	/* Monster nest (jelly) */
	if (tmp < 30)
	{
		/* Describe */
		name = "jelly";

		/* Restrict to jelly */
		get_mon_num_hook = vault_aux_jelly;
	}

	/* Monster nest (animal) */
	else if (tmp < 50)
	{
		/* Describe */
		name = "animal";

		/* Restrict to animal */
		get_mon_num_hook = vault_aux_animal;
	}

	/* Monster nest (undead) */
	else
	{
		/* Describe */
		name = "undead";

		/* Restrict to undead */
		get_mon_num_hook = vault_aux_undead;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 64; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(Depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}


	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Oops */
	if (empty) return;


	/* Describe */
	if (cheat_room)
	{
		/* Room type */
		/*msg_format("Monster nest (%s)", name);*/
	}


	/* Increase the level rating */
	rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Nests") */
	if ((Depth <= 40) && (randint(Depth*Depth + 1) < 300))
	{
		good_item_flag = TRUE;
	}


	/* Place some monsters */
	for (y = yval - 2; y <= yval + 2; y++)
	{
		for (x = xval - 9; x <= xval + 9; x++)
		{
			int r_idx = what[rand_int(64)];

			/* Place that "random" monster (no groups) */
			(void)place_monster_aux(Depth, y, x, r_idx, FALSE, FALSE);
		}
	}
}



/*
 * Type 6 -- Monster pits
 *
 * A monster pit is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type organized in the room.
 *
 * Monster types in the pit
 *   orc pit	(Dungeon Level 5 and deeper)
 *   troll pit	(Dungeon Level 20 and deeper)
 *   giant pit	(Dungeon Level 40 and deeper)
 *   dragon pit	(Dungeon Level 60 and deeper)
 *   demon pit	(Dungeon Level 80 and deeper)
 *
 * The inside room in a monster pit appears as shown below, where the
 * actual monsters in each location depend on the type of the pit
 *
 *   #####################
 *   #0000000000000000000#
 *   #0112233455543322110#
 *   #0112233467643322110#
 *   #0112233455543322110#
 *   #0000000000000000000#
 *   #####################
 *
 * Note that the monsters in the pit are now chosen by using "get_mon_num()"
 * to request 16 "appropriate" monsters, sorting them by level, and using
 * the "even" entries in this sorted list for the contents of the pit.
 *
 * Hack -- all of the "dragons" in a "dragon" pit must be the same "color",
 * which is handled by requiring a specific "breath" attack for all of the
 * dragons.  This may include "multi-hued" breath.  Note that "wyrms" may
 * be present in many of the dragon pits, if they have the proper breath.
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the pit.
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which case
 * the pit will be empty, and will not effect the level rating.
 *
 * Note that "monster pits" will never contain "unique" monsters.
 */
static void build_type6(int Depth, int yval, int xval)
{
	int			tmp, what[16];

	int			i, j, y, x, y1, x1, y2, x2;

	bool		empty = FALSE;

	cave_type		*c_ptr;

	cptr		name;


	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;


	/* Place the floor area */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[Depth][y][x];
			c_ptr->feat = FEAT_FLOOR;
			c_ptr->info |= CAVE_ROOM;
		}
	}

	/* Place the outer walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_OUTER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_OUTER;
	}


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		c_ptr = &cave[Depth][y][x1-1];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y][x2+1];
		c_ptr->feat = FEAT_WALL_INNER;
	}
	for (x = x1 - 1; x <= x2 + 1; x++)
	{
		c_ptr = &cave[Depth][y1-1][x];
		c_ptr->feat = FEAT_WALL_INNER;
		c_ptr = &cave[Depth][y2+1][x];
		c_ptr->feat = FEAT_WALL_INNER;
	}


	/* Place a secret door */
	switch (randint(4))
	{
		case 1: place_secret_door(Depth, y1 - 1, xval); break;
		case 2: place_secret_door(Depth, y2 + 1, xval); break;
		case 3: place_secret_door(Depth, yval, x1 - 1); break;
		case 4: place_secret_door(Depth, yval, x2 + 1); break;
	}


	/* Choose a pit type */
	tmp = randint(Depth);

	/* Orc pit */
	if (tmp < 20)
	{
		/* Message */
		name = "orc";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_orc;
	}

	/* Troll pit */
	else if (tmp < 40)
	{
		/* Message */
		name = "troll";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_troll;
	}

	/* Giant pit */
	else if (tmp < 60)
	{
		/* Message */
		name = "giant";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_giant;
	}

	/* Dragon pit */
	else if (tmp < 80)
	{
		/* Pick dragon type */
		switch (rand_int(6))
		{
			/* Black */
			case 0:
			{
				/* Message */
				name = "acid dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_ACID;

				/* Done */
				break;
			}

			/* Blue */
			case 1:
			{
				/* Message */
				name = "electric dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_ELEC;

				/* Done */
				break;
			}

			/* Red */
			case 2:
			{
				/* Message */
				name = "fire dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_FIRE;

				/* Done */
				break;
			}

			/* White */
			case 3:
			{
				/* Message */
				name = "cold dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_COLD;

				/* Done */
				break;
			}

			/* Green */
			case 4:
			{
				/* Message */
				name = "poison dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_POIS;

				/* Done */
				break;
			}

			/* Multi-hued */
			default:
			{
				/* Message */
				name = "multi-hued dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BR_ACID | RF4_BR_ELEC |
				                          RF4_BR_FIRE | RF4_BR_COLD |
				                          RF4_BR_POIS);

				/* Done */
				break;
			}

		}

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_dragon;
	}

	/* Demon pit */
	else
	{
		/* Message */
		name = "demon";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_demon;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 16; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(Depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}


	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Oops */
	if (empty) return;


	/* XXX XXX XXX */
	/* Sort the entries */
	for (i = 0; i < 16 - 1; i++)
	{
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++)
		{
			int i1 = j;
			int i2 = j + 1;

			int p1 = r_info[what[i1]].level;
			int p2 = r_info[what[i2]].level;

			/* Bubble */
			if (p1 > p2)
			{
				int tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select the entries */
	for (i = 0; i < 8; i++)
	{
		/* Every other entry */
		what[i] = what[i * 2];
	}


#if 0
	/* Message */
	if (cheat_room)
	{
		/* Room type */
		msg_format("Monster pit (%s)", name);

		/* Contents */
		for (i = 0; i < 8; i++)
		{
			/* Message */
			msg_print(r_name + r_info[what[i]].name);
		}
	}
#endif


	/* Increase the level rating */
	rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
	if ((Depth <= 40) && (randint(Depth*Depth + 1) < 300))
	{
		good_item_flag = TRUE;
	}


	/* Top and bottom rows */
	for (x = xval - 9; x <= xval + 9; x++)
	{
		place_monster_aux(Depth, yval - 2, x, what[0], FALSE, FALSE);
		place_monster_aux(Depth, yval + 2, x, what[0], FALSE, FALSE);
	}

	/* Middle columns */
	for (y = yval - 1; y <= yval + 1; y++)
	{
		place_monster_aux(Depth, y, xval - 9, what[0], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 9, what[0], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 8, what[1], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 8, what[1], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 7, what[1], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 7, what[1], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 6, what[2], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 6, what[2], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 5, what[2], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 5, what[2], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 4, what[3], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 4, what[3], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 3, what[3], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 3, what[3], FALSE, FALSE);

		place_monster_aux(Depth, y, xval - 2, what[4], FALSE, FALSE);
		place_monster_aux(Depth, y, xval + 2, what[4], FALSE, FALSE);
	}

	/* Above/Below the center monster */
	for (x = xval - 1; x <= xval + 1; x++)
	{
		place_monster_aux(Depth, yval + 1, x, what[5], FALSE, FALSE);
		place_monster_aux(Depth, yval - 1, x, what[5], FALSE, FALSE);
	}

	/* Next to the center monster */
	place_monster_aux(Depth, yval, xval + 1, what[6], FALSE, FALSE);
	place_monster_aux(Depth, yval, xval - 1, what[6], FALSE, FALSE);

	/* Center monster */
	place_monster_aux(Depth, yval, xval, what[7], FALSE, FALSE);
}



/*
 * Hack -- fill in "vault" rooms
 */
void build_vault(int Depth, int yval, int xval, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y;

	cptr t;

	cave_type *c_ptr;


	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = xval - (xmax / 2) + dx;
			y = yval - (ymax / 2) + dy;

			/* FIXME - find a better solution */
			if(!in_bounds(Depth,y,x))
				continue;
			
			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Access the grid */
			c_ptr = &cave[Depth][y][x];

			/* Lay down a floor */
			c_ptr->feat = FEAT_FLOOR;

			/* Part of a vault */
			c_ptr->info |= (CAVE_ROOM | CAVE_ICKY);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
				case '%':
				c_ptr->feat = FEAT_WALL_OUTER;
				break;

				/* Granite wall (inner) */
				case '#':
				c_ptr->feat = FEAT_WALL_INNER;
				break;

				/* Permanent wall (inner) */
				case 'X':
				c_ptr->feat = FEAT_PERM_INNER;
				break;

				/* Treasure/trap */
				case '*':
				if (rand_int(100) < 75)
				{
                    place_object(Depth, y, x, FALSE, FALSE, 0);
				}
				else
				{
					place_trap(Depth, y, x);
				}
				break;

				/* Secret doors */
				case '+':
				place_secret_door(Depth, y, x);
				break;

				/* Trap */
				case '^':
				place_trap(Depth, y, x);
				break;
			}
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the grid */
			x = xval - (xmax/2) + dx;
			y = yval - (ymax/2) + dy;

			/* FIXME - find a better solution */
			if(!in_bounds(Depth,y,x))
				continue;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t)
			{
				/* Monster */
				case '&':
				monster_level = Depth + 5;
				place_monster(Depth, y, x, TRUE, TRUE);
				monster_level = Depth;
				break;

				/* Meaner monster */
				case '@':
				monster_level = Depth + 11;
				place_monster(Depth, y, x, TRUE, TRUE);
				monster_level = Depth;
				break;

				/* Meaner monster, plus treasure */
				case '9':
				monster_level = Depth + 9;
				place_monster(Depth, y, x, TRUE, TRUE);
				monster_level = Depth;
				object_level = Depth + 7;
                place_object(Depth, y, x, TRUE, FALSE, 0);
				object_level = Depth;
				break;

				/* Nasty monster and treasure */
				case '8':
				monster_level = Depth + 40;
				place_monster(Depth, y, x, TRUE, TRUE);
				monster_level = Depth;
				object_level = Depth + 20;
                place_object(Depth, y, x, TRUE, TRUE, 0);
				object_level = Depth;
				break;

				/* Monster and/or object */
				case ',':
				if (rand_int(100) < 50)
				{
					monster_level = Depth + 3;
					place_monster(Depth, y, x, TRUE, TRUE);
					monster_level = Depth;
				}
				if (rand_int(100) < 50)
				{
					object_level = Depth + 7;
                    place_object(Depth, y, x, FALSE, FALSE, 0);
					object_level = Depth;
				}
				break;
			}
		}
	}
}



/*
 * Type 7 -- simple vaults (see "v_info.txt")
 */
static void build_type7(int Depth, int yval, int xval)
{
	vault_type	*v_ptr;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Access a random vault record */
		v_ptr = &v_info[rand_int(MAX_V_IDX)];

		/* Accept the first lesser vault */
		if (v_ptr->typ == 7) break;
	}

	/* Message */
	/*if (cheat_room) msg_print("Lesser Vault");*/

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((Depth <= 50) ||
	    (randint((Depth-40) * (Depth-40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(Depth, yval, xval, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}



/*
 * Type 8 -- greater vaults (see "v_info.txt")
 */
static void build_type8(int Depth, int yval, int xval)
{
	vault_type	*v_ptr;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Access a random vault record */
		v_ptr = &v_info[rand_int(MAX_V_IDX)];

		/* Accept the first greater vault */
		if (v_ptr->typ == 8) break;
	}

	/* Message */
	/*if (cheat_room) msg_print("Greater Vault");*/

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((Depth <= 50) ||
	    (randint((Depth-40) * (Depth-40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(Depth, yval, xval, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}



/*
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created,
 * since we use the special "granite wall" sub-types to keep track
 * of legal places for corridors to pierce rooms.
 *
 * We use "door_flag" to prevent excessive construction of doors
 * along overlapping corridors.
 *
 * We queue the tunnel grids to prevent door creation along a corridor
 * which intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We "pierce" grids which are "outer" walls of rooms, and when we
 * do so, we change all adjacent "outer" walls of rooms into "solid"
 * walls so that no two corridors may use adjacent grids for exits.
 *
 * The "solid" wall check prevents corridors from "chopping" the
 * corners of rooms off, as well as "silly" door placement, and
 * "excessively wide" room entrances.
 *
 * Useful "feat" values:
 *   FEAT_WALL_EXTRA -- granite walls
 *   FEAT_WALL_INNER -- inner room walls
 *   FEAT_WALL_OUTER -- outer room walls
 *   FEAT_WALL_SOLID -- solid room walls
 *   FEAT_PERM_EXTRA -- shop walls (perma)
 *   FEAT_PERM_INNER -- inner room walls (perma)
 *   FEAT_PERM_OUTER -- outer room walls (perma)
 *   FEAT_PERM_SOLID -- dungeon border (perma)
 */
static void build_tunnel(int Depth, int row1, int col1, int row2, int col2)
{
	int			i, y, x;
	int			tmp_row, tmp_col;
	int                 row_dir, col_dir;
	int                 start_row, start_col;
	int			main_loop_count = 0;

	bool		door_flag = FALSE;

	cave_type		*c_ptr;


	/* Reset the arrays */
	dun->tunn_n = 0;
	dun->wall_n = 0;

	/* Save the starting location */
	start_row = row1;
	start_col = col1;

	/* Start out in the correct direction */
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

	/* Keep going until done (or bored) */
	while ((row1 != row2) || (col1 != col2))
	{
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (rand_int(100) < DUN_TUN_CHG)
		{
			/* Acquire the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;


		/* Extremely Important -- do not leave the dungeon */
		while (!in_bounds(Depth, tmp_row, tmp_col))
		{
			/* Acquire the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;
		}


		/* Access the location */
		c_ptr = &cave[Depth][tmp_row][tmp_col];


		/* Avoid the edge of the dungeon */
		if (c_ptr->feat == FEAT_PERM_SOLID) continue;

		/* Avoid the edge of vaults */
		if (c_ptr->feat == FEAT_PERM_OUTER) continue;

		/* Avoid "solid" granite walls */
		if (c_ptr->feat == FEAT_WALL_SOLID) continue;

		/* Pierce "outer" walls of rooms */
		if (c_ptr->feat == FEAT_WALL_OUTER)
		{
			/* Acquire the "next" location */
			y = tmp_row + row_dir;
			x = tmp_col + col_dir;

			/* Hack -- Avoid outer/solid permanent walls */
			if (cave[Depth][y][x].feat == FEAT_PERM_SOLID) continue;
			if (cave[Depth][y][x].feat == FEAT_PERM_OUTER) continue;

			/* Hack -- Avoid outer/solid granite walls */
			if (cave[Depth][y][x].feat == FEAT_WALL_OUTER) continue;
			if (cave[Depth][y][x].feat == FEAT_WALL_SOLID) continue;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the wall location */
			if (dun->wall_n < WALL_MAX)
			{
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

#ifdef WIDE_CORRIDORS
			/* Save the wall location */
			if (dun->wall_n < WALL_MAX)
			{
				if (cave[Depth][row1 + col_dir][col1 + row_dir].feat != FEAT_PERM_SOLID &&
				    cave[Depth][row1 + col_dir][col1 + row_dir].feat != FEAT_PERM_SOLID)
				{
					dun->wall[dun->wall_n].y = row1 + col_dir;
					dun->wall[dun->wall_n].x = col1 + row_dir;
					dun->wall_n++;
				}
				else
				{
					dun->wall[dun->wall_n].y = row1;
					dun->wall[dun->wall_n].x = col1;
					dun->wall_n++;
				}
			}
#endif

#ifdef WIDE_CORRIDORS
			/* Forbid re-entry near this piercing */
			for (y = row1 - 2; y <= row1 + 2; y++)
			{
				for (x = col1 - 2; x <= col1 + 2; x++)
				{
					/* Be sure we are "in bounds" */
					if (!in_bounds(Depth, y, x)) continue;

					/* Convert adjacent "outer" walls as "solid" walls */
					if (cave[Depth][y][x].feat == FEAT_WALL_OUTER)
					{
						/* Change the wall to a "solid" wall */
						cave[Depth][y][x].feat = FEAT_WALL_SOLID;
					}
				}
			}
#else
			/* Forbid re-entry near this piercing */
			for (y = row1 - 1; y <= row1 + 1; y++)
			{
				for (x = col1 - 1; x <= col1 + 1; x++)
				{
					/* Convert adjacent "outer" walls as "solid" walls */
					if (cave[Depth][y][x].feat == FEAT_WALL_OUTER)
					{
						/* Change the wall to a "solid" wall */
						cave[Depth][y][x].feat = FEAT_WALL_SOLID;
					}
				}
			}
#endif
		}

		/* Travel quickly through rooms */
		else if (c_ptr->info & CAVE_ROOM)
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;
		}

		/* Tunnel through all other walls */
		else if (c_ptr->feat >= FEAT_WALL_EXTRA)
		{
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

#ifdef WIDE_CORRIDORS
			/* Note that this is incredibly hacked */

			/* Make sure we're in bounds */
			if (in_bounds(Depth, row1 + col_dir, col1 + row_dir))
			{
				/* New spot to be hollowed out */
				c_ptr = &cave[Depth][row1 + col_dir][col1 + row_dir];

				/* Make sure it's a wall we want to tunnel */
				if (c_ptr->feat == FEAT_WALL_EXTRA)
				{
					/* Save the tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						dun->tunn[dun->tunn_n].y = row1 + col_dir;
						dun->tunn[dun->tunn_n].x = col1 + row_dir;
						dun->tunn_n++;
					}
				}
			}
#endif

			/* Allow door in next grid */
			door_flag = FALSE;
		}

		/* Handle corridor intersections or overlaps */
		else
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Collect legal door locations */
			if (!door_flag)
			{
				/* Save the door location */
				if (dun->door_n < DOOR_MAX)
				{
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

#ifdef WIDE_CORRIDORS
				/* Save the next door location */
				if (dun->door_n < DOOR_MAX)
				{
					if (in_bounds(Depth, row1 + col_dir, col1 + row_dir))
					{
						dun->door[dun->door_n].y = row1 + col_dir;
						dun->door[dun->door_n].x = col1 + row_dir;
						dun->door_n++;
					}
					
					/* Hack -- Duplicate the previous door */
					else
					{
						dun->door[dun->door_n].y = row1;
						dun->door[dun->door_n].x = col1;
						dun->door_n++;
					}
				}
#endif

				/* No door in next grid */
				door_flag = TRUE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (rand_int(100) >= DUN_TUN_CON)
			{
				/* Distance between row1 and start_row */
				tmp_row = row1 - start_row;
				if (tmp_row < 0) tmp_row = (-tmp_row);

				/* Distance between col1 and start_col */
				tmp_col = col1 - start_col;
				if (tmp_col < 0) tmp_col = (-tmp_col);

				/* Terminate the tunnel */
				if ((tmp_row > 10) || (tmp_col > 10)) break;
			}
		}
	}


	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++)
	{
		/* Access the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Access the grid */
		c_ptr = &cave[Depth][y][x];

		/* Clear previous contents, add a floor */
		c_ptr->feat = FEAT_FLOOR;
	}


	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++)
	{
		int feat;

		/* Access the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Access the grid */
		c_ptr = &cave[Depth][y][x];

		/* Clear previous contents, add up floor */
		c_ptr->feat = FEAT_FLOOR;

		/* Occasional doorway */
		if (rand_int(100) < DUN_TUN_PEN)
		{
			/* Place a random door */
			place_random_door(Depth, y, x);

			/* Remember type of door */
			feat = cave[Depth][y][x].feat;

#ifdef WIDE_CORRIDORS
			/* Make sure both halves get a door */
			if (i % 2)
			{
				/* Access the grid */
				y = dun->wall[i - 1].y;
				x = dun->wall[i - 1].x;
			}
			else
			{
				/* Access the grid */
				y = dun->wall[i + 1].y;
				x = dun->wall[i + 1].x;

				/* Increment counter */
				i++;
			}

			/* Place the same type of door */
			cave[Depth][y][x].feat = feat;
#endif
		}
	}
}




/*
 * Count the number of "corridor" grids adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y1, x1)"
 *
 * XXX XXX This routine currently only counts actual "empty floor"
 * grids which are not in rooms.  We might want to also count stairs,
 * open doors, closed doors, etc.
 */
static int next_to_corr(int Depth, int y1, int x1)
{
	int i, y, x, k = 0;

	cave_type *c_ptr;

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++)
	{
		/* Extract the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Skip non floors */
		if (!cave_floor_bold(Depth, y, x)) continue;

		/* Access the grid */
		c_ptr = &cave[Depth][y][x];

		/* Skip non "empty floor" grids */
		if (c_ptr->feat != FEAT_FLOOR) continue;

		/* Skip grids inside rooms */
		if (c_ptr->info & CAVE_ROOM) continue;

		/* Count these grids */
		k++;
	}

	/* Return the number of corridors */
	return (k);
}


/*
 * Determine if the given location is "between" two walls,
 * and "next to" two corridor spaces.  XXX XXX XXX
 *
 * Assumes "in_bounds(y,x)"
 */
static bool possible_doorway(int Depth, int y, int x)
{
	/* Count the adjacent corridors */
	if (next_to_corr(Depth, y, x) >= 2)
	{
		/* Check Vertical */
		if ((cave[Depth][y-1][x].feat >= FEAT_MAGMA) &&
		    (cave[Depth][y+1][x].feat >= FEAT_MAGMA))
		{
			return (TRUE);
		}

		/* Check Horizontal */
		if ((cave[Depth][y][x-1].feat >= FEAT_MAGMA) &&
		    (cave[Depth][y][x+1].feat >= FEAT_MAGMA))
		{
			return (TRUE);
		}
	}

	/* No doorway */
	return (FALSE);
}


/*
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(int Depth, int y, int x)
{
	/* Paranoia */
	if (!in_bounds(Depth, y, x)) return;

	/* Ignore walls */
	if (cave[Depth][y][x].feat >= FEAT_MAGMA) return;

	/* Ignore room grids */
	if (cave[Depth][y][x].info & CAVE_ROOM) return;

	/* Occasional door (if allowed) */
	if ((rand_int(100) < DUN_TUN_JCT) && possible_doorway(Depth, y, x))
	{
		/* Place a door */
		place_random_door(Depth, y, x);
	}
}




/*
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of "crowded" rooms to reduce
 * the chance of overflowing the monster list during level creation.
 */
static bool room_build(int Depth, int y0, int x0, int typ)
{
	int y, x, y1, x1, y2, x2;


	/* Restrict level */
	if (Depth < room[typ].level) return (FALSE);

	/* Restrict "crowded" rooms */
	if (dun->crowded && ((typ == 5) || (typ == 6))) return (FALSE);

	/* Extract blocks */
	y1 = y0 + room[typ].dy1;
	y2 = y0 + room[typ].dy2;
	x1 = x0 + room[typ].dx1;
	x2 = x0 + room[typ].dx2;

	/* Never run off the screen */
	if ((y1 < 0) || (y2 >= dun->row_rooms)) return (FALSE);
	if ((x1 < 0) || (x2 >= dun->col_rooms)) return (FALSE);

	/* Verify open space */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (dun->room_map[y][x]) return (FALSE);
		}
	}

	/* XXX XXX XXX It is *extremely* important that the following */
	/* calculation is *exactly* correct to prevent memory errors */

	/* Acquire the location of the room */
	y = ((y1 + y2 + 1) * BLOCK_HGT) / 2;
	x = ((x1 + x2 + 1) * BLOCK_WID) / 2;

	/* Build a room */
	switch (typ)
	{
		/* Build an appropriate room */
		case 8: build_type8(Depth, y, x); break;
		case 7: build_type7(Depth, y, x); break;
		case 6: build_type6(Depth, y, x); break;
		case 5: build_type5(Depth, y, x); break;
		case 4: build_type4(Depth, y, x); break;
		case 3: build_type3(Depth, y, x); break;
		case 2: build_type2(Depth, y, x); break;
		case 1: build_type1(Depth, y, x); break;

		/* Paranoia */
		default: return (FALSE);
	}

	/* Save the room location */
	if (dun->cent_n < CENT_MAX)
	{
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	/* Reserve some blocks */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			dun->room_map[y][x] = TRUE;
		}
	}

	/* Count "crowded" rooms */
	if ((typ == 5) || (typ == 6)) dun->crowded = TRUE;

	/* Success */
	return (TRUE);
}


/*
 * Generate a new dungeon level
 *
 * Note that "dun_body" adds about 4000 bytes of memory to the stack.
 */
static void cave_gen(int Depth)
{
	int i, k, y, x, y1, x1;

	bool destroyed = FALSE;

	dun_data dun_body;


	/* Global data */
	dun = &dun_body;


	/* Hack -- Start with basic granite */
	for (y = 0; y < MAX_HGT; y++)
	{
		for (x = 0; x < MAX_WID; x++)
		{
			cave_type *c_ptr = &cave[Depth][y][x];

			/* Create granite wall */
			c_ptr->feat = FEAT_WALL_EXTRA;
		}
	}


	/* Possible "destroyed" level */
	if ((Depth > 10) && (rand_int(DUN_DEST) == 0)) destroyed = TRUE;

	/* Hack -- No destroyed "quest" levels */
	if (is_quest(Depth)) destroyed = FALSE;


	/* Actual maximum number of rooms on this level */
	dun->row_rooms = MAX_HGT / BLOCK_HGT;
	dun->col_rooms = MAX_WID / BLOCK_WID;

	/* Initialize the room table */
	for (y = 0; y < dun->row_rooms; y++)
	{
		for (x = 0; x < dun->col_rooms; x++)
		{
			dun->room_map[y][x] = FALSE;
		}
	}


	/* No "crowded" rooms yet */
	dun->crowded = FALSE;


	/* No rooms yet */
	dun->cent_n = 0;

	/* Build some rooms */
	for (i = 0; i < DUN_ROOMS; i++)
	{
		/* Pick a block for the room */
		y = rand_int(dun->row_rooms);
		x = rand_int(dun->col_rooms);

		/* Align dungeon rooms */
		if (dungeon_align)
		{
			/* Slide some rooms right */
			if ((x % 3) == 0) x++;

			/* Slide some rooms left */
			if ((x % 3) == 2) x--;
		}

		/* Destroyed levels are boring */
		if (destroyed)
		{
			/* Attempt a "trivial" room */
			if (room_build(Depth, y, x, 1)) continue;

			/* Never mind */
			continue;
		}

		/* Attempt an "unusual" room */
		if (rand_int(DUN_UNUSUAL) < Depth)
		{
			/* Roll for room type */
			k = rand_int(100);

			/* Attempt a very unusual room */
			if (rand_int(DUN_UNUSUAL) < Depth)
			{
				/* Type 8 -- Greater vault (10%) */
				if ((k < 10) && room_build(Depth, y, x, 8)) continue;

				/* Type 7 -- Lesser vault (15%) */
				if ((k < 25) && room_build(Depth, y, x, 7)) continue;

				/* Type 6 -- Monster pit (15%) */
				if ((k < 40) && room_build(Depth, y, x, 6)) continue;

				/* Type 5 -- Monster nest (10%) */
				if ((k < 50) && room_build(Depth, y, x, 5)) continue;
			}

			/* Type 4 -- Large room (25%) */
			if ((k < 25) && room_build(Depth, y, x, 4)) continue;

			/* Type 3 -- Cross room (25%) */
			if ((k < 50) && room_build(Depth, y, x, 3)) continue;

			/* Type 2 -- Overlapping (50%) */
			if ((k < 100) && room_build(Depth, y, x, 2)) continue;
		}

		/* Attempt a trivial room */
		if (room_build(Depth, y, x, 1)) continue;
	}


	/* Special boundary walls -- Top */
	for (x = 0; x < MAX_WID; x++)
	{
		cave_type *c_ptr = &cave[Depth][0][x];

		/* Clear previous contents, add "solid" perma-wall */
		c_ptr->feat = FEAT_PERM_SOLID;
	}

	/* Special boundary walls -- Bottom */
	for (x = 0; x < MAX_WID; x++)
	{
		cave_type *c_ptr = &cave[Depth][MAX_HGT-1][x];

		/* Clear previous contents, add "solid" perma-wall */
		c_ptr->feat = FEAT_PERM_SOLID;
	}

	/* Special boundary walls -- Left */
	for (y = 0; y < MAX_HGT; y++)
	{
		cave_type *c_ptr = &cave[Depth][y][0];

		/* Clear previous contents, add "solid" perma-wall */
		c_ptr->feat = FEAT_PERM_SOLID;
	}

	/* Special boundary walls -- Right */
	for (y = 0; y < MAX_HGT; y++)
	{
		cave_type *c_ptr = &cave[Depth][y][MAX_WID-1];

		/* Clear previous contents, add "solid" perma-wall */
		c_ptr->feat = FEAT_PERM_SOLID;
	}


	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++)
	{
		int pick1 = rand_int(dun->cent_n);
		int pick2 = rand_int(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;
	}

	/* Start with no tunnel doors */
	dun->door_n = 0;

	/* Hack -- connect the first room to the last room */
	y = dun->cent[dun->cent_n-1].y;
	x = dun->cent[dun->cent_n-1].x;

	/* Connect all the rooms together */
	for (i = 0; i < dun->cent_n; i++)
	{
		/* Connect the room to the previous room */
		build_tunnel(Depth, dun->cent[i].y, dun->cent[i].x, y, x);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
	}

	/* Place intersection doors	 */
	for (i = 0; i < dun->door_n; i++)
	{
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_door(Depth, y, x - 1);
		try_door(Depth, y, x + 1);
		try_door(Depth, y - 1, x);
		try_door(Depth, y + 1, x);
	}


	/* Hack -- Add some magma streamers */
	for (i = 0; i < DUN_STR_MAG; i++)
	{
		build_streamer(Depth, FEAT_MAGMA, DUN_STR_MC);
	}

	/* Hack -- Add some quartz streamers */
	for (i = 0; i < DUN_STR_QUA; i++)
	{
		build_streamer(Depth, FEAT_QUARTZ, DUN_STR_QC);
	}


	/* Destroy the level if necessary */
	if (destroyed) destroy_level(Depth);


	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(Depth, FEAT_MORE, rand_range(3, 4), 3);

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(Depth, FEAT_LESS, rand_range(1, 2), 3);


	/* Determine the character location */
	new_player_spot(Depth);


	/* Basic "amount" */
	k = (Depth / 3);
	if (k > 10) k = 10;
	if (k < 2) k = 2;


	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint(8);

	/* Put some monsters in the dungeon */
	for (i = i + k; i > 0; i--)
	{
		(void)alloc_monster(Depth, 0, TRUE);
	}


	/* Place some traps in the dungeon */
	alloc_object(Depth, ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint(k));

	/* Put some rubble in corridors */
	alloc_object(Depth, ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint(k));

	/* Put some objects in rooms */
	alloc_object(Depth, ALLOC_SET_ROOM, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ROOM, 3));

	/* Put some objects/gold in the dungeon */
	alloc_object(Depth, ALLOC_SET_BOTH, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ITEM, 3));
	alloc_object(Depth, ALLOC_SET_BOTH, ALLOC_TYP_GOLD, randnor(DUN_AMT_GOLD, 3));
}



/*
 * Builds a store at a given (row, column)
 *
 * Note that the solid perma-walls are at x=0/65 and y=0/21
 *
 * As of 2.7.4 (?) the stores are placed in a more "user friendly"
 * configuration, such that the four "center" buildings always
 * have at least four grids between them, to allow easy running,
 * and the store doors tend to face the middle of town.
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 */
static void build_store(int n, int yy, int xx)
{
	int                 i, y, x, y0, x0, y1, x1, y2, x2, tmp;

	cave_type		*c_ptr;

	y0 = yy * 11 + 5;
	x0 = xx * 16 + 12;

	/* Determine the store boundaries */
	y1 = y0 - randint((yy == 0) ? 3 : 2);
	y2 = y0 + randint((yy == 1) ? 3 : 2);
	x1 = x0 - randint(5);
	x2 = x0 + randint(5);

    /* Hack -- make building's as large as possible */
	//if (n == 12)
 	//{
        y1 = y0 - randint(3);
        y2 = y0 + randint(3);
        x1 = x0 - randint(5);
        x2 = x0 + randint(5);
  	//}

	/* Build an invulnerable rectangular building */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Get the grid */
			c_ptr = &cave[0][y][x];

			/* Clear previous contents, add "basic" perma-wall */
			c_ptr->feat = FEAT_PERM_EXTRA;

			/* Hack -- The buildings are illuminated and known */
			/* c_ptr->info |= (CAVE_GLOW); */
		}
	}

	/* Hack -- Make store "8" (the old home) empty */
	if (n == 7)
	{
		for (y = y1 + 1; y < y2; y++)
		{
			for (x = x1 + 1; x < x2; x++)
			{
				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Clear contents */
				c_ptr->feat = FEAT_FLOOR;

				/* Declare this to be a room */
				c_ptr->info |= CAVE_ROOM | CAVE_GLOW | CAVE_ICKY;
			}
		}

		/* Hack -- have everyone start in the tavern */
		level_down_y[0] = (y1 + y2) / 2;
		level_down_x[0] = (x1 + x2) / 2;
	}

	/* Make doorways, fill with grass and trees */
	if (n == 9)
	{
		for (y = y1 + 1; y < y2; y++)
		{
			for (x = x1 + 1; x < x2; x++)
			{
				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Clear contents */
				c_ptr->feat = FEAT_GRASS;
			}
		}

		y = (y1 + y2) / 2;
		x = (x1 + x2) / 2;

		cave[0][y1][x].feat = FEAT_GRASS;
		cave[0][y2][x].feat = FEAT_GRASS;
		cave[0][y][x1].feat = FEAT_GRASS;
		cave[0][y][x2].feat = FEAT_GRASS;

		for (i = 0; i < 5; i++)
		{
			if (trees_in_town == cfg_max_trees && cfg_max_trees != -1) break;
			
			y = rand_range(y1 + 1, y2 - 1);
			x = rand_range(x1 + 1, x2 - 1);

			c_ptr = &cave[0][y][x];

			c_ptr->feat = FEAT_TREE;
			trees_in_town++;
		}

		return;
	}
			
	/* Pond */
	if (n == 10)
	{
		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Fill with water */
				c_ptr->feat = FEAT_WATER;
			}
		}

		/* Make the pond not so "square" */
		cave[0][y1][x1].feat = FEAT_DIRT;
		cave[0][y1][x2].feat = FEAT_DIRT;
		cave[0][y2][x1].feat = FEAT_DIRT;
		cave[0][y2][x2].feat = FEAT_DIRT;

		return;
	}

	/* Building with stairs */
	if (n == 11)
	{
		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Empty it */
				c_ptr->feat = FEAT_FLOOR;

				/* Put some grass */
				if (randint(100) < 50)
					c_ptr->feat = FEAT_GRASS;
			}
		}

		x = (x1 + x2) / 2;
		y = (y1 + y2) / 2;

		/* Access the stair grid */
		c_ptr = &cave[0][y][x];

		/* Clear previous contents, add down stairs */
		c_ptr->feat = FEAT_MORE;

		/* Hack -- the players start on the stairs while coming up */
		level_up_y[0] = level_rand_y[0] = y;
		level_up_x[0] = level_rand_x[0] = x;

		return;
	}

	/* Forest */
	if (n == 12)
	{
		int xc, yc, max_dis;
		int size = (y2 - y1 + 1) * (x2 - x1 + 1); 
      bool limit_trees = ((cfg_max_trees > 0) && (size > (cfg_max_trees / 4))); 
      int max_chance = (limit_trees? (100 * (cfg_max_trees / 4)): (100 * size));
        
		/* Find the center of the forested area */
		xc = (x1 + x2) / 2;
		yc = (y1 + y2) / 2;

		/* Find the max distance from center */
		max_dis = distance(y2, x2, yc, xc);

		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				int chance;

				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Empty it */
				c_ptr->feat = FEAT_GRASS;

				/* Calculate chance of a tree */
				chance = 100 * (distance(y, x, yc, xc));
				chance /= max_dis;
				chance = 80 - chance;
				chance *= size;

				/* We want at most (cfg_max_trees / 4) trees */ 
            if (limit_trees && (chance > max_chance)) chance = max_chance; 

				/* Put some trees */
				if (randint(100 * size) < chance && (trees_in_town < cfg_max_trees || cfg_max_trees == -1))
					{
						c_ptr->feat = FEAT_TREE;
						trees_in_town++;
					}
			}
		}

		return;
	}


	/* House */
	if (n == 13)
	{
		int price;

		for (y = y1 + 1; y < y2; y++)
		{
			for (x = x1 + 1; x < x2; x++)
			{
				/* Get the grid */
				c_ptr = &cave[0][y][x];

				/* Fill with floor */
				c_ptr->feat = FEAT_FLOOR;

				/* Make it "icky" */
				c_ptr->info |= CAVE_ICKY;
			}
		}

		/* Setup some "house info" */
		price = (x2 - x1 - 1) * (y2 - y1 - 1);
		/*price *= 20 * price; -APD- price is now proportional to size*/
		price *= 20;
		price *= 80 + randint(40);

		/* Remember price */
		houses[num_houses].price = price;
		houses[num_houses].x_1 = x1+1;
		houses[num_houses].y_1 = y1+1;
		houses[num_houses].x_2 = x2-1;
		houses[num_houses].y_2 = y2-1;
		houses[num_houses].depth = 0;
	}


	/* Pick a door direction (S,N,E,W) */
	tmp = rand_int(4);

	/* Re-roll "annoying" doors */
	while (((tmp == 0) && ((yy % 2) == 1)) ||
	    ((tmp == 1) && ((yy % 2) == 0)) ||
	    ((tmp == 2) && ((xx % 2) == 1)) ||
	    ((tmp == 3) && ((xx % 2) == 0)))
	{
		/* Pick a new direction */
		tmp = rand_int(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
		case 0:
		y = y2;
		x = rand_range(x1, x2);
		break;

		/* Top side */
		case 1:
		y = y1;
		x = rand_range(x1, x2);
		break;

		/* Right side */
		case 2:
		y = rand_range(y1, y2);
		x = x2;
		break;

		/* Left side */
		default:
		y = rand_range(y1, y2);
		x = x1;
		break;
	}

	/* Access the grid */
	c_ptr = &cave[0][y][x];

	/* Some buildings get special doors */
	if (n == 13)
	{
		c_ptr->feat = FEAT_HOME_HEAD + houses[num_houses].strength;

		/* hack -- only create houses that aren't already loaded from disk */
		if ((tmp = pick_house(0, y, x)) == -1)
		{
			/* Store door location information */
			houses[num_houses].door_y = y;
			houses[num_houses].door_x = x;
			houses[num_houses].owned[0] = '\0';

			/* One more house */
			num_houses++;
		}
		else
		{
		    c_ptr->feat = FEAT_HOME_HEAD + houses[tmp].strength;
		}
	}
	else if (n == 14) // auctionhouse
	{
		c_ptr->feat = FEAT_PERM_EXTRA; // wants to work.	
	
	}
	else
	{
		/* Clear previous contents, add a store door */
		c_ptr->feat = FEAT_SHOP_HEAD + n;
		
		/* If this is the Black Market add the "back room" */
		if (n == 6)
		{
			/* At the back :) */
			if (y == y2) y = y1; else if (y == y1) y = y2;
			if (x == x2) x = x1; else if (x == x1) x = x2;
			c_ptr = &cave[0][y][x];
			/* Disabled */
			/* c_ptr->feat = FEAT_SHOP_HEAD + 8; */
					
		}
	}
}

/*
 * Build a road.
 */
static void place_street(int vert, int place)
{
	int y, x, y1, y2, x1, x2;

	/* Vertical streets */
	if (vert)
	{
		x = place * 32 + 20;
		x1 = x - 2;
		x2 = x + 2;

		y1 = 5;
		y2 = MAX_HGT - 5;
	}
	else
	{
		y = place * 22 + 10;
		y1 = y - 2;
		y2 = y + 2;

		x1 = 5;
		x2 = MAX_WID - 5;
	}

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (cave[0][y][x].feat != FEAT_FLOOR)
				cave[0][y][x].feat = FEAT_GRASS;
		}
	}

	if (vert)
	{
		x1++;
		x2--;
	}
	else
	{
		y1++;
		y2--;
	}

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave[0][y][x].feat = FEAT_FLOOR;
		}
	}
}



/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(void)
{
	int			y, x, k, n;

#ifdef	DEVEL_TOWN_COMPATIBILITY
	/* -APD- the location of the auction house */
	int			auction_x, auction_y;
#endif

	int                 rooms[72];
	/* int                 rooms[MAX_STORES]; */

	/* Hack limit trees to max/4 */
	 int size = (MAX_HGT - 2) * (MAX_WID - 2); 
    bool limit_trees = ((cfg_max_trees > 0) && (size > (cfg_max_trees / 4))); 
    int max_chance = (limit_trees? (100 * (cfg_max_trees / 4)): (100 * size)); 
    int chance;
   /* Reset counter */
    trees_in_town = 0;
 	/* Calculate chance of a tree */ 
    chance = 4 * size; 
   /* We want at most (cfg_max_trees / 4) trees */ 
    if (limit_trees && (chance > max_chance)) chance = max_chance;
   
	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant town layout */
	Rand_value = seed_town;

	/* Hack -- Start with basic floors */
	for (y = 1; y < MAX_HGT - 1; y++)
	{
		for (x = 1; x < MAX_WID - 1; x++)
		{
			cave_type *c_ptr = &cave[0][y][x];

			/* Clear all features, set to "empty floor" */
			c_ptr->feat = FEAT_DIRT;

			if (rand_int(100 * size) < chance && (cfg_max_trees == -1 || trees_in_town < cfg_max_trees))
			{
				c_ptr->feat = FEAT_TREE;
				trees_in_town++;
			}

			else if (rand_int(100) < 75)
			{
				c_ptr->feat = FEAT_GRASS;
			}
		}
	}

	/* Place horizontal "streets" */
	for (y = 0; y < 3; y++)
	{
		place_street(0, y);
	}

	/* Place vertical "streets" */
	for (x = 0; x < 6; x++)
	{
		place_street(1, x);
	}

	/* Prepare an Array of "remaining stores", and count them */
	for (n = 0; n < MAX_STORES-1; n++) rooms[n] = n;
	for (n = MAX_STORES-1; n < 16; n++) rooms[n] = 10;
	for (n = 16; n < 68; n++) rooms[n] = 13;
	for (n = 68; n < 71; n++) rooms[n] = 12; /* 3 forests */
	rooms[n++] = 11;

	/* Place stores */
	for (y = 2; y < 4; y++)
	{
		for (x = 4; x < 8; x++)
		{
			/* Pick a random unplaced store */
			k = rand_int(n - 64);

			/* Build that store at the proper location */
			build_store(rooms[k], y, x);

			/* One less store */
			n--;

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[n - 64];
		}
	}

	/* Place two rows of stores */
	for (y = 0; y < 6; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 12; x++)
		{
			/* Make sure we haven't already placed this one */
			if (y >= 2 && y <= 3 && x >= 4 && x <= 7) continue;
			
			/* Pick a random unplaced store */
			k = rand_int(n) + 8;

			build_store(rooms[k], y, x);

			/* One less building */
			n--;

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[n + 8];
		}
	}

	/* Hack -- use the "complex" RNG */
	Rand_quick = FALSE;
}




/*
 * Town logic flow for generation of new town
 *
 * We start with a fully wiped cave of normal floors.
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * We place the player on the stairs at the same time we make them.
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
 
 /*
 Hack -- since boundary walls are a 'good thing' for many of the algorithms 
 used, the feature FEAT_PERM_CLEAR was created.  It is used to create an 
 invisible boundary wall for town and wilderness levels, keeping the
 algorithms happy, and the players fooled.
 
 */
 
static void town_gen(void)
{ 
	int	y, x;
	cave_type *c_ptr;

	byte wall_feat;
	if (!cfg_town_wall)	wall_feat = FEAT_PERM_CLEAR;
	else			wall_feat = FEAT_PERM_SOLID;

	/* Perma-walls -- North/South*/
	for (x = 0; x < MAX_WID; x++)
	{
		/* North wall */
		c_ptr = &cave[0][0][x];

		/* Clear previous contents, add "clear" perma-wall */
		c_ptr->feat = wall_feat;

		/* Illuminate and memorize the walls 
		c_ptr->info |= (CAVE_GLOW | CAVE_MARK);*/

		/* South wall */
		c_ptr = &cave[0][MAX_HGT-1][x];

		/* Clear previous contents, add "clear" perma-wall */
		c_ptr->feat = wall_feat;

		/* Illuminate and memorize the walls 
		c_ptr->info |= (CAVE_GLOW);*/
	}

	/* Perma-walls -- West/East */
	for (y = 0; y < MAX_HGT; y++)
	{
		/* West wall */
		c_ptr = &cave[0][y][0];

		/* Clear previous contents, add "clear" perma-wall */
		c_ptr->feat = wall_feat;

		/* Illuminate and memorize the walls
		c_ptr->info |= (CAVE_GLOW);*/

		/* East wall */
		c_ptr = &cave[0][y][MAX_WID-1];

		/* Clear previous contents, add "clear" perma-wall */
		c_ptr->feat = wall_feat;

		/* Illuminate and memorize the walls 
		c_ptr->info |= (CAVE_GLOW);*/
	}
	

	/* Hack -- Build the buildings/stairs (from memory) */
	town_gen_hack();


	/* Day Light */
	if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))
	{	
		/* Lite up the town */ 
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				c_ptr = &cave[0][y][x];

				 /* Perma-Lite */
				c_ptr->info |= CAVE_GLOW;

				 /* Memorize 
				if (view_perma_grids) c_ptr->info |= CAVE_MARK;
				*/
			}
		}

		/* This has been disabled, since the old monsters will now be saved when
		 * the server goes down. -APD
		 */
		/* Make some day-time residents 
		for (i = 0; i < MIN_M_ALLOC_TD; i++) (void)alloc_monster(0, 3, TRUE);
		*/
	}
	 /* Night Time */
	else
	{
		 /* Make some night-time residents 
		for (i = 0; i < MIN_M_ALLOC_TN; i++) (void)alloc_monster(0, 3, TRUE);
		*/
	}
}





/*
 * Allocate the space needed for a dungeon level
 */
void alloc_dungeon_level(int Depth)
{
	int i;

	/* Allocate the array of rows */
	C_MAKE(cave[Depth], MAX_HGT, cave_type *);

	/* Allocate each row */
	for (i = 0; i < MAX_HGT; i++)
	{
		/* Allocate it */
		C_MAKE(cave[Depth][i], MAX_WID, cave_type);
	}
}

/*
 * Deallocate the space needed for a dungeon level
 */
void dealloc_dungeon_level(int Depth)
{
	int i;

	/* Hack to compensate for the half baked hacks below! */
	/* Don't deallocate levels which contain houses owned by players */
	for (i = 0; i < num_houses; i++)
	{
		/* House on this depth and owned? */
		if (houses[i].depth == Depth && house_owned(i))
		{
			return;
		}
	}
	
	/* Delete any monsters on that level */
	/* Hack -- don't wipe wilderness monsters */
	if (Depth > 0) wipe_m_list(Depth);

	/* Delete any objects on that level */
	/* Hack -- don't wipe wilderness objects */
	if (Depth > 0) wipe_o_list(Depth);

	/* Free up the space taken by each row */
	for (i = 0; i < MAX_HGT; i++)
	{
		/* Dealloc that row */
		C_FREE(cave[Depth][i], MAX_WID, cave_type);
	}

	/* Deallocate the array of rows */
	C_FREE(cave[Depth], MAX_HGT, cave_type *);

	/* Set that level to "ungenerated" */
	cave[Depth] = NULL; 
}




/*
 * Generates a random dungeon level			-RAK-
 *
 * Hack -- regenerate any "overflow" levels
 *
 * Hack -- allow auto-scumming via a gameplay option.
 */
 
 
void generate_cave(int Ind, int Depth, int auto_scum)
{
	int i, num;
	int scum = auto_scum;
	player_type *p_ptr = 0;

	/* No dungeon yet */
	server_dungeon = FALSE;

	/* get the player context, if any */
	if(Ind)
	{
		p_ptr = Players[Ind];
	}

	/* Generate */
	for (num = 0; TRUE; num++)
	{
		bool okay = TRUE;

		cptr why = NULL;


		/* Hack -- Reset heaps */
		/*o_max = 1;
		m_max = 1;*/

		/* Start with a blank cave */
		for (i = 0; i < MAX_HGT; i++)
		{
			/* Wipe a whole row at a time */
			C_WIPE(cave[Depth][i], MAX_WID, cave_type);
		}


		/* Mega-Hack -- no player yet */
		/*px = py = 0;*/


		/* Mega-Hack -- no panel yet */
		/*panel_row_min = 0;
		panel_row_max = 0;
		panel_col_min = 0;
		panel_col_max = 0;*/


		/* Reset the monster generation level */
		monster_level = Depth;

		/* Reset the object generation level */
		object_level = Depth;

		/* Nothing special here yet */
		good_item_flag = FALSE;

		/* Nothing good here yet */
		rating = 0;


		/* Build the town */
		if (!Depth)
		{
			/* Small town */
			/*cur_hgt = SCREEN_HGT;
			cur_wid = SCREEN_WID;*/

			/* Determine number of panels */
			/*max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
			max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;*/

				/* Try again */
			/* Assume illegal panel */
			/*panel_row = max_panel_rows;
			panel_col = max_panel_cols;*/

			/* Make a town */
			town_gen();
		}

		/* Build wilderness */
		else if (Depth < 0)
		{
			wilderness_gen(Depth);		
		}

		/* Build a real level */
		else
		{
			/* Big dungeon */
			/*cur_hgt = MAX_HGT;
			cur_wid = MAX_WID;*/

			/* Determine number of panels */
			/*max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
			max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;*/

			/* Assume illegal panel */
			/*panel_row = max_panel_rows;
			panel_col = max_panel_cols;*/

			/* Make a dungeon */
			cave_gen(Depth);
		}


		/* Extract the feeling */
		if (rating > 100) feeling = 2;
		else if (rating > 80) feeling = 3;
		else if (rating > 60) feeling = 4;
		else if (rating > 40) feeling = 5;
		else if (rating > 30) feeling = 6;
		else if (rating > 20) feeling = 7;
		else if (rating > 10) feeling = 8;
		else if (rating > 0) feeling = 9;
		else feeling = 10;
		/* fprintf(stderr," New Level %d Rating %d\n",Depth*50,rating); */

		/* Hack -- Have a special feeling sometimes */
		if (good_item_flag) feeling = 1;

		if (!cfg_ironman && p_ptr)
		{
			/* It takes 1000 game turns for "feelings" to recharge */
			if ((turn - p_ptr->old_turn) < 1000)
			{
				feeling = 0;
				scum = FALSE;
			}
		}

		/* Hack -- no feeling in the town */
		if (!Depth) feeling = 0;


		/* Prevent object over-flow */
		if (o_max >= MAX_O_IDX)
		{
			/* Message */
			why = "too many objects";

			/* Message */
			okay = FALSE;
		}

		/* Prevent monster over-flow */
		if (m_max >= MAX_M_IDX)
		{
			/* Message */
			why = "too many monsters";

			/* Message */
			okay = FALSE;
		}

		/* Mega-Hack -- "auto-scum" */
		/* Auto-scum only in the dungeon!!! */
        if ((Depth > 0) && scum && (num < 100))
		{
			/* fprintf(stderr,"auto_scum in on for this level\n"); */
			/* Require "goodness" */
			if ((feeling > 9) ||
			    ((Depth >= 5) && (feeling > 8)) ||
			    ((Depth >= 10) && (feeling > 7)) ||
			    ((Depth >= 20) && (feeling > 6)) ||
				/* Try again */
			    ((Depth >= 40) && (feeling > 5)))
			{
				/* Give message to cheaters */
				if (cheat_room || cheat_hear ||
				    cheat_peek || cheat_xtra)
				{
					/* Message */
					why = "boring level";
				}

				/* Try again */
				okay = FALSE;
			}
		}

		/* Accept */
		if (okay) break;


		/* Message */
		if (why) plog_fmt("Generation restarted (%s)", why);

		/* Wipe the objects */
		wipe_o_list(Depth);

		/* Wipe the monsters */
		wipe_m_list(Depth);

		/* Compact some objects, if necessary */
		if (o_max >= MAX_O_IDX * 3 / 4)
			compact_objects(32);

		/* Compact some monsters, if necessary */
		if (m_max >= MAX_M_IDX * 3 / 4)
			compact_monsters(32);
	}

	/* Remember when we had a level feeling if we are a player */
	if(p_ptr)
	{
		p_ptr->old_turn = turn;
	}

	/* Dungeon level ready */
	server_dungeon = TRUE;
}
