/*
 * File: pathfind.c
 * Purpose: Pathfinding algorithm
 *
 * Copyright (c) 2004 Christophe Cavalaria, Leon Marrick
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "mangband.h"


/*** Constants ***/
#define DUNGEON_WID MAX_WID
#define DUNGEON_HGT MAX_HGT



/*** Globals ***/

static int terrain[MAX_PF_RADIUS][MAX_PF_RADIUS];
/*char pf_result[MAX_PF_LENGTH];
int pf_result_index;*/

static int ox, oy, ex, ey;
static int dir_search[8] = {2,4,6,8,1,3,7,9};


/*** Pathfinding code ***/

static bool is_valid_pf(player_type *p_ptr, int y, int x)
{
	int Ind = Get_Ind[p_ptr->conn];
	int Depth = p_ptr->dun_depth;
	/* Unvisited means allowed */
	if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK))) return (TRUE);

	/* Require open space */
	return (cave_floor_bold(Depth, y, x));
}

static void fill_terrain_info(player_type *p_ptr)
{
	int i, j;

	ox = MAX(p_ptr->px - MAX_PF_RADIUS / 2, 0);
	oy = MAX(p_ptr->py - MAX_PF_RADIUS / 2, 0);

	ex = MIN(p_ptr->px + MAX_PF_RADIUS / 2 - 1, DUNGEON_WID);
	ey = MIN(p_ptr->py + MAX_PF_RADIUS / 2 - 1, DUNGEON_HGT);

	for (j = 0; j < MAX_PF_RADIUS; j++)
	for (i = 0; i < MAX_PF_RADIUS; i++)
		terrain[j][i] = -1;

	for (j = oy; j < ey; j++)
		for (i = ox; i < ex; i++)
			if (is_valid_pf(p_ptr, j, i))
				terrain[j - oy][i - ox] = MAX_PF_LENGTH;

	terrain[p_ptr->py - oy][p_ptr->px - ox] = 1;
}

#define MARK_DISTANCE(c,d) if ((c <= MAX_PF_LENGTH) && (c > d)) { c = d; try_again = (TRUE); }

bool findpath(player_type *p_ptr, int y, int x)
{
	int i, j, k;
	int dir = 10;
	bool try_again;
	int cur_distance;

	fill_terrain_info(p_ptr);

	terrain[p_ptr->py - oy][p_ptr->px - ox] = 1;

	if ((x >= ox) && (x < ex) && (y >= oy) && (y < ey))
	{
		if ((cave[p_ptr->dun_depth][y][x].m_idx > 0) && (p_ptr->mon_los[cave[p_ptr->dun_depth][y][x].m_idx ]))
		{
			terrain[y - oy][x - ox] = MAX_PF_LENGTH;
		}

#if 0
		else if (terrain[y-oy][x-ox] != MAX_PF_LENGTH)
		{
		   bell();
		   return (FALSE);
		}
#endif

		terrain[y - oy][x - ox] = MAX_PF_LENGTH;
	}
	else
	{
		bell();
		return (FALSE);
	}

	if (terrain[y - oy][x - ox] == -1)
	{
		bell();
		return (FALSE);
	}


	/* 
	 * And now starts the very naive and very 
	 * inefficient pathfinding algorithm
	 */
	do
	{
		try_again = FALSE;

		for (j = oy + 1; j < ey - 1; j++)
		{
			for (i = ox + 1; i < ex - 1; i++)
			{
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH))
				{
					for (dir = 1; dir < 10; dir++)
					{
						if (dir == 5)
							dir++;

						MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
					}
				}
			}
		}

		if (terrain[y - oy][x - ox] < MAX_PF_LENGTH)
			try_again = (FALSE);

	}
	while (try_again);

	/* Failure */
	if (terrain[y - oy][x - ox] == MAX_PF_LENGTH)
	{
		bell();
		return (FALSE);
	}

	/* Success */
	i = x;
	j = y;

	p_ptr->pf_result_index = 0;

	while ((i != p_ptr->px) || (j != p_ptr->py))
	{
		cur_distance = terrain[j - oy][i - ox] - 1;
		for (k = 0; k < 8; k++)
		{
			dir = dir_search[k];
			if (terrain[j - oy + ddy[dir]][i - ox + ddx[dir]] == cur_distance)
				break;
		}

		/* Should never happend */
		if (dir == 10)
		{
			bell();
			return (FALSE);
		}

		else if (dir == 5)
		{
			bell();
			return (FALSE);
		}

		p_ptr->pf_result[p_ptr->pf_result_index++] = '0' + (char)(10 - dir);
		i += ddx[dir];
		j += ddy[dir];
	}

	p_ptr->pf_result_index--;

	return (TRUE);
}



