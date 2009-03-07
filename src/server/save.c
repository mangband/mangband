/* File: save.c */

/* Purpose: interact with savefiles */

#define SERVER

#include "angband.h"
#include "../common/md5.h"

/*
 * Some "local" parameters, used to help write savefiles
 */

static FILE	*file_handle;		/* Current save "file" */

static u32b	v_stamp = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_stamp = 0L;	/* A simple "checksum" on the encoded bytes */

static int xml_indent = 0L;
static char xml_buf[32];
static char *xml_prefix = xml_buf;

/* Start a section */
static void start_section(char* name)
{
	int i;
	if(xml_indent == 0) xml_prefix[0] = '\0';
	fprintf(file_handle,"%s<%s>\n",xml_prefix,name);
	xml_indent += 2;
	for(i = 0;i<xml_indent;i++) xml_buf[i] = ' ';
	xml_buf[xml_indent] = '\0';
}

/* End a section */
static void end_section(char* name)
{
	int i;
	xml_indent -= 2;
	for(i = 0;i<xml_indent;i++) xml_buf[i] = ' ';
	xml_buf[xml_indent] = '\0';
	fprintf(file_handle,"%s</%s>\n",xml_prefix,name);
}

/* Write an integer */
static void write_int(char* name, int value)
{
	fprintf(file_handle,"%s%s = %i\n",xml_prefix,name,value);
}

/* Write an unsigned integer value */
static void write_uint(char* name, unsigned int value)
{
	fprintf(file_handle,"%s%s = %u\n",xml_prefix,name,value);
}

/* Write an signed long value */
static void write_huge(char* name, huge value)
{
	fprintf(file_handle,"%s%s = %llu\n",xml_prefix,name,value);
}

/* Write a string */
static void write_str(char* name, char* value)
{
	fprintf(file_handle,"%s%s = %s\n",xml_prefix,name,value);
}

static void write_float(char* name, float value)
{
	fprintf(file_handle,"%s%s = %f\n",xml_prefix,name,value);
}

/* Write binary data */
static void write_binary(char* name, char* data)
{
	int i,len;
	byte b;
	fprintf(file_handle,"%s%s = ",xml_prefix,name);
	len = MAX_WID;
	for(i=0;i<len;i++)
	{
		b = data[i];
		fprintf(file_handle,"%2x",b);
	}
	fprintf(file_handle,"\n",name);
}



/*
 * These functions write info in larger logical records
 */


/*
 * Write an "item" record
 */
static void wr_item(object_type *o_ptr)
{
	char obj_name[80];
	int tmp = 0;
	
	start_section("item");

	object_desc(0, obj_name, o_ptr, TRUE, 3);
	write_str("name",obj_name);

	write_int("k_idx",o_ptr->k_idx);

	write_int("iy",o_ptr->iy);
	write_int("ix",o_ptr->ix);
	write_int("dun_depth",o_ptr->dun_depth);

	write_int("tval",o_ptr->tval);
	write_int("sval",o_ptr->sval);
	write_int("bpval",o_ptr->bpval);
	write_int("pval",o_ptr->pval);

	write_int("discount",o_ptr->discount);
	write_int("number",o_ptr->number);
	write_int("weight",o_ptr->weight);

	write_int("name1",o_ptr->name1);
	write_int("name2",o_ptr->name2);
	write_int("name3",o_ptr->name3);
	write_int("timeout",o_ptr->timeout);

	write_int("to_h",o_ptr->to_h);
	write_int("to_d",o_ptr->to_d);
	write_int("to_a",o_ptr->to_a);
	write_int("ac",o_ptr->ac);
	write_int("dd",o_ptr->dd);
	write_int("ds",o_ptr->ds);

	write_int("ident",o_ptr->ident);

	write_int("xtra1",o_ptr->xtra1);
	write_int("xtra2",o_ptr->xtra2);

	/* Save the inscription (if any) */
	if (o_ptr->note)
	{
		write_str("inscription",quark_str(o_ptr->note));
	}
	else
	{
		write_str("inscription","");
	}
	
	/* Held by monster index */ 
	/* FIXME: we disable this here because it's broken and can cause the server save
	 * file to fail to load due to invalid monster indexes */
	/* write_int("held_m_idx", o_ptr->held_m_idx); */
	write_int("held_m_idx", tmp);
	
	end_section("item");
}


/*
 * Write a "monster" record
 */
static void wr_monster(monster_type *m_ptr)
{
	char mon_name[80];

	start_section("monster");
	monster_desc(0,mon_name,m_ptr->r_idx,0x88);
	write_str("name",mon_name);
	write_int("r_idx",m_ptr->r_idx);
	write_int("fy",m_ptr->fy);
	write_int("fx",m_ptr->fx);
	write_int("dun_depth",m_ptr->dun_depth);
	write_int("hp",m_ptr->hp);
	write_int("maxhp",m_ptr->maxhp);
	write_int("csleep",m_ptr->csleep);
	write_int("mspeed",m_ptr->mspeed);
	write_int("energy",m_ptr->energy);
	write_int("stunned",m_ptr->stunned);
	write_int("confused",m_ptr->confused);
	write_int("afraid",m_ptr->monfear);
	end_section("monster");
}


/*
 * Write a "lore" record
 */
static void wr_lore(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	start_section("lore");

	/* Write monster name in XML */
	write_str("name",r_name + r_ptr->name);

	/* Count sights/deaths/kills */
	write_int("sights",r_ptr->r_sights);
	write_int("deaths",r_ptr->r_deaths);
	write_int("pkills",r_ptr->r_pkills);
	write_int("tkills",r_ptr->r_tkills);

	/* Count wakes and ignores */
	write_int("wake",r_ptr->r_wake);
	write_int("ignore",r_ptr->r_ignore);

	/* Save the amount of time until the unique respawns */
	write_uint("respawn_timer",r_ptr->respawn_timer);

	/* Count drops */
	write_int("drop_gold",r_ptr->r_drop_gold);
	write_int("drop_item",r_ptr->r_drop_item);

	/* Count spells */
	write_int("cast_innate",r_ptr->r_cast_inate);
	write_int("cast_spell",r_ptr->r_cast_spell);

	/* Count blows of each type */
	start_section("blows");
	write_int("blow",r_ptr->r_blows[0]);
	write_int("blow",r_ptr->r_blows[1]);
	write_int("blow",r_ptr->r_blows[2]);
	write_int("blow",r_ptr->r_blows[3]);
	end_section("blows");

	/* Memorize flags */
	start_section("flags");
	write_uint("flag",r_ptr->r_flags1);
	write_uint("flag",r_ptr->r_flags2);
	write_uint("flag",r_ptr->r_flags3);
	write_uint("flag",r_ptr->r_flags4);
	write_uint("flag",r_ptr->r_flags5);
	write_uint("flag",r_ptr->r_flags6);
	end_section("flags");

	/* Monster limit per level */
	write_int("max_num",r_ptr->max_num);
	
	/* Killer */
	write_uint("killer",r_ptr->killer);
	
	end_section("lore");
}


/*
 * Write an "xtra" record
 */
static void wr_xtra(int Ind, int k_idx)
{
	player_type *p_ptr = Players[Ind];
	byte tmp8u = 0;

	if (p_ptr->obj_aware[k_idx]) tmp8u |= 0x01;
	if (p_ptr->obj_tried[k_idx]) tmp8u |= 0x02;

	write_int("flags",tmp8u);
}


/*
 * Write a "store" record
 */
static void wr_store(store_type *st_ptr)
{
	int j;

	start_section("store");

	/* Save the "open" counter */
	write_huge("store_open",st_ptr->store_open);

	/* Save the "insults" */
	write_uint("insult_cur",st_ptr->insult_cur);

	/* Save the current owner */
	write_uint("owner",st_ptr->owner);

	/* Save the stock size */
	write_uint("stock_num",st_ptr->stock_num);

	/* Save the "haggle" info */
	write_int("good_buy",st_ptr->good_buy);
	write_int("bad_buy",st_ptr->bad_buy);

	/* Save the stock */
	start_section("stock");
	for (j = 0; j < st_ptr->stock_num; j++)
	{
		/* Save each item in stock */
		wr_item(&st_ptr->stock[j]);
	}
	end_section("stock");
	end_section("store");

}

static void wr_party(party_type *party_ptr)
{
	start_section("party");
	/* Save the party name */
	write_str("name",party_ptr->name);

	/* Save the owner's name */
	write_str("owner",party_ptr->owner);

	/* Save the number of people and creation time */
	write_int("num",party_ptr->num);
	write_huge("created",party_ptr->created);
	
	end_section("party");
}

static void wr_wild(wilderness_type *w_ptr)
{
	/* level flags */
	write_uint("flags",w_ptr->flags);
}


/*
 * Write the information about a house
 */
static void wr_house(house_type *house)
{
	start_section("house");
	write_int("x1",house->x_1);
	write_int("y1",house->y_1);
	write_int("x2",house->x_2);
	write_int("y2",house->y_2);
	
	write_int("door_y",house->door_y);
	write_int("door_x",house->door_x);
	write_int("strength",house->strength);
	write_str("owned",house->owned);

	write_int("depth",house->depth);
	write_int("price",house->price);
	end_section("house");
}


/*
 * Write some "extra" info
 */
static void wr_extra(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int i;

	start_section("player");

	write_str("playername",p_ptr->name);

	write_str("pass",p_ptr->pass);

	write_str("died_from",p_ptr->died_from);
	write_str("died_from_list",p_ptr->died_from_list);
	write_int("died_from_depth",p_ptr->died_from_depth);

	start_section("history");
	for (i = 0; i < 4; i++)
	{
		write_str("history",p_ptr->history[i]);
	}
	end_section("history");

	/* Race/Class/Gender/Party */
	write_int("prace",p_ptr->prace);
	write_int("pclass",p_ptr->pclass);
	write_int("male",p_ptr->male);
	write_int("party",p_ptr->party);

	write_int("hitdie",p_ptr->hitdie);
	write_int("expfact",p_ptr->expfact);

	write_int("age",p_ptr->age);
	write_int("ht",p_ptr->ht);
	write_int("wt",p_ptr->wt);

	/* Dump the stats (maximum and current) */
	start_section("stats");
	for (i = 0; i < 6; ++i) write_int("stat_max",p_ptr->stat_max[i]);
	for (i = 0; i < 6; ++i) write_int("stat_cur",p_ptr->stat_cur[i]);
	end_section("stats");

	write_int("id",p_ptr->id);

	write_int("au",p_ptr->au);

	write_uint("max_exp",p_ptr->max_exp);
	write_uint("exp",p_ptr->exp);
	write_int("exp_frac",p_ptr->exp_frac);
	write_int("lev",p_ptr->lev);

	write_int("mhp",p_ptr->mhp);
	write_int("chp",p_ptr->chp);
	write_int("chp_frac",p_ptr->chp_frac);

	write_int("msp",p_ptr->msp);
	write_int("csp",p_ptr->csp);
	write_int("csp_frac",p_ptr->csp_frac);
	
	write_int("no_ghost",p_ptr->no_ghost);

	/* Max Player and Dungeon Levels */
	write_int("max_plv",p_ptr->max_plv);
	write_int("max_dlv",p_ptr->max_dlv);

	/* Player location */
	write_int("py",p_ptr->py);
	write_int("px",p_ptr->px);
	write_int("dun_depth",p_ptr->dun_depth);
	write_int("world_x",p_ptr->world_x);
	write_int("world_y",p_ptr->world_y);


	/* More info */
	write_int("ghost",p_ptr->ghost);
	write_int("sc",p_ptr->sc);
	write_int("fruit_bat",p_ptr->fruit_bat);

	write_int("lives",p_ptr->lives);
	write_int("blind",p_ptr->blind);
	write_int("paralyzed",p_ptr->paralyzed);
	write_int("confused",p_ptr->confused);
	write_int("food",p_ptr->food);
	write_int("energy",p_ptr->energy);
	write_int("fast",p_ptr->fast);
	write_int("slow",p_ptr->slow);
	write_int("afraid",p_ptr->afraid);
	write_int("cut",p_ptr->cut);
	write_int("stun",p_ptr->stun);
	write_int("poisoned",p_ptr->poisoned);
	write_int("image",p_ptr->image);
	write_int("protevil",p_ptr->protevil);
	write_int("invuln",p_ptr->invuln);
	write_int("hero",p_ptr->hero);
	write_int("shero",p_ptr->shero);
	write_int("shield",p_ptr->shield);
	write_int("blessed",p_ptr->blessed);
	write_int("tim_invis",p_ptr->tim_invis);
	write_int("word_recall",p_ptr->word_recall);
	write_int("see_infra",p_ptr->see_infra);
	write_int("tim_infra",p_ptr->tim_infra);

	write_int("oppose_fire",p_ptr->oppose_fire);
	write_int("oppose_cold",p_ptr->oppose_cold);
	write_int("oppose_acid",p_ptr->oppose_acid);
	write_int("oppose_elec",p_ptr->oppose_elec);
	write_int("oppose_pois",p_ptr->oppose_pois);

	write_int("confusing",p_ptr->confusing);
	write_int("searching",p_ptr->searching);
	write_int("maximize",p_ptr->maximize);
	write_int("preserve",p_ptr->preserve);

	/* Dump the monster lore */
	start_section("uniques");
	for (i = 0; i < MAX_R_IDX; i++) write_int("unique",p_ptr->r_killed[i]);
	end_section("uniques");

	/* Special stuff */
	write_int("panic_save",panic_save);
	write_int("total_winner",p_ptr->total_winner);
	write_int("retire_timer",p_ptr->retire_timer);
	write_int("noscore",p_ptr->noscore);

	/* Write death */
	write_int("death",p_ptr->death);

	end_section("player");
}

/*
 * Write the list of players a player is hostile toward
 */
static void wr_hostilities(int Ind)
{
	player_type *p_ptr = Players[Ind];
	hostile_type *h_ptr;
	int i;
	u16b tmp16u = 0;

	start_section("hostilities");

	/* Count hostilities */
	for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next)
	{
		/* One more */
		tmp16u++;
	}

	/* Save number */
	write_int("num",tmp16u);

	/* Start at beginning */
	h_ptr = p_ptr->hostile;

	/* Save each record */
	for (i = 0; i < tmp16u; i++)
	{
		/* Write ID */
		write_uint("id",h_ptr->id);

		/* Advance pointer */
		h_ptr = h_ptr->next;
	}
	end_section("hostilities");
}


/*
 * Write the player name hash table.
 */
static void wr_player_names(void)
{
	int i, num, *id_list;

	/* Get the list of player ID's */
	num = player_id_list(&id_list);

	/* Store the number of entries */
	write_int("num_players",num);

	/* Store each entry */
	for (i = 0; i < num; i++)
	{
		start_section("player");
		/* Store the ID */
		write_uint("id",id_list[i]);

		/* Store the player name */
		write_str("name",lookup_player_name(id_list[i]));
		
		end_section("player");
	}

	/* Free the memory in the list */
	C_KILL(id_list, num, int);
}


/*
 * Write a specified depth 
 *
 * Each row is broken down into a series of run-length encoded runs.
 * Each run has a constant feature type, and flags.
 *
 * Note that a cave_type's monster index and object indecies are not stored
 * here.  They should be assigned automatically when the objects
 * and monsters are loaded later.
 *
 * This could probably be made more efficient by allowing runs to encompass
 * more than one row.
 *
 * We could also probably get a large efficiency increase by splitting the features
 * and flags into two seperate run-length encoded blocks.
 *
 * -APD
 */

static void wr_dungeon(int Depth)
{
	int y, x;
	byte prev_feature, prev_info;
	unsigned char runlength;
	char cave_row[MAX_WID+1];

	cave_type *c_ptr;
	start_section("dungeon_level");

	/* Depth */
	write_int("depth",Depth);

	/* Dungeon size */
	write_int("max_height",MAX_HGT);
	write_int("max_width",MAX_WID);

	/* How many players are on this depth */
	write_int("players_on_depth",players_on_depth[Depth]);

	/* The staircase locations on this depth */
	/* Hack -- this information is currently not present for the wilderness
	 * levels.
	 */

	if (Depth >= 0)
	{
		write_int("level_up_y",level_up_y[Depth]);
		write_int("level_up_x",level_up_x[Depth]);
		write_int("level_down_y",level_down_y[Depth]);
		write_int("level_down_x",level_down_x[Depth]);
		write_int("level_rand_y",level_rand_y[Depth]);
		write_int("level_rand_x",level_rand_x[Depth]);
	}

	/*** Textual encoding of cave ***/
	start_section("features");
	for (y = 0; y < MAX_HGT; y++)
	{
		/* break the row down into runs */
		for (x = 0; x < MAX_WID; x++)
		{
			c_ptr = &cave[Depth][y][x];
			cave_row[x] = c_ptr->feat;
		}
		cave_row[MAX_WID] = '\0';
		write_binary("row",cave_row); 
	}	
	end_section("features");

	start_section("info");
	for (y = 0; y < MAX_HGT; y++)
	{
		/* break the row down into runs */
		for (x = 0; x < MAX_WID; x++)
		{
			c_ptr = &cave[Depth][y][x];
			cave_row[x] = c_ptr->info;
		}
		cave_row[MAX_WID] = '\0';
		write_binary("row",cave_row); 
	}	
	end_section("info");

	end_section("dungeon_level");
}

/* Write a players memory of a cave, simmilar to the above function. */
void wr_cave_memory(Ind)
{
	player_type *p_ptr = Players[Ind];
	int y,x;
	char prev_flag;
	unsigned char runlength = 0;
	char cave_row[MAX_WID+1];

	start_section("cave_memory");
	/* write the number of flags */
	write_int("max_height",MAX_HGT);
	write_int("max_width",MAX_WID);

	/* break the cave down into rows */
	for (y = 0; y < MAX_HGT; y++)
	{
		/* write each row */
		for (x = 0; x < MAX_WID; x++)
		{
			cave_row[x] = p_ptr->cave_flag[y][x];
		}
		cave_row[MAX_WID] = '\0';
		write_binary("row",cave_row); 
	}	

	end_section("cave_memory");
}



/*
 * Actually write a save-file
 */
static bool wr_savefile_new(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int        i;

	u32b              now, tmp32u;

	byte		tmp8u;
	u16b		tmp16u;


	/* Guess at the current time */
	now = time((time_t *)0);


	/* Note the operating system */
	sf_xtra = 0L;

	/* Note when the file was saved */
	sf_when = now;

	/* Note the number of saves */
	sf_saves++;


	/*** Actually write the file ***/

	start_section("mangband_player_save");

	/* Dump the file header */
	start_section("version");
	write_int("major",VERSION_MAJOR);
	write_int("minor",VERSION_MINOR);
	write_int("patch",VERSION_PATCH);
	end_section("version");


	/* Reset the checksum */
	v_stamp = 0L;
	x_stamp = 0L;


	/* Operating system */
	write_uint("sf_xtra",sf_xtra);

	/* Time file last saved */
	write_uint("sf_when",sf_when);

	/* Number of past lives */
	write_int("sf_lives",sf_lives);

	/* Number of times saved */
	write_int("sf_saves",sf_saves);
	
	/* Write the server turn */
	write_huge("turn",turn);

	/* Write the players birth turn */
	write_huge("birth_turn",p_ptr->birth_turn);

	/* Write the players turn */
	write_huge("player_turn",p_ptr->turn);

	/* Dump the object memory */
	start_section("object_memory");
	tmp16u = MAX_K_IDX;
	write_int("max_k_idx",tmp16u);
	for (i = 0; i < tmp16u; i++) wr_xtra(Ind, i);
	end_section("object_memory");

	/* Write the "extra" information */
	wr_extra(Ind);

	/* Dump the "player hp" entries */
	start_section("hp");
	tmp16u = PY_MAX_LEVEL;
	write_int("py_max_level",tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		write_int("hp",p_ptr->player_hp[i]);
	}
	end_section("hp");

	/* Write spell data */
	write_uint("spell_learned1",p_ptr->spell_learned1);
	write_uint("spell_learned2",p_ptr->spell_learned2);
	write_uint("spell_worked1",p_ptr->spell_worked1);
	write_uint("spell_worked2",p_ptr->spell_worked2);
	write_uint("spell_forgotten1",p_ptr->spell_forgotten1);
	write_uint("spell_forgotten2",p_ptr->spell_forgotten2);

	/* Dump the ordered spells */
	start_section("spell_order");
	for (i = 0; i < 64; i++)
	{
		write_int("order",p_ptr->spell_order[i]);
	}
	end_section("spell_order");


	/* Write the inventory */
	start_section("inventory");
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		if (p_ptr->inventory[i].k_idx)
		{
			write_int("inv_entry",i);
			wr_item(&p_ptr->inventory[i]);
		}
	}
	write_int("inv_entry",0xFFFF);
	end_section("inventory");


	/* Write the list of hostilities */
	wr_hostilities(Ind);
	
	/* write the cave flags (our memory of our current level) */
	wr_cave_memory(Ind);

	/* write the wilderness map */
	start_section("wilderness");
	tmp32u = MAX_WILD/8;
	write_uint("max_wild",tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		write_int("wild_map",p_ptr->wild_map[i]);
	}
	end_section("wilderness");
	
	/* write character event history */
	start_section("event_history");
	for(i=0;i<p_ptr->char_hist_ptr;i++)
	{
		write_str("hist",p_ptr->char_hist[i]);
	}
	end_section("event_history");

	end_section("mangband_player_save");

	/* Error in save */
	if (ferror(file_handle) || (fflush(file_handle) == EOF)) return FALSE;

	/* Successful save */
	return TRUE;
}


/*
 * Medium level player saver
 *
 */
static bool save_player_aux(int Ind, char *name)
{
	bool	ok = FALSE;

	int		fd = -1;

	int		mode = 0644;


	/* No file yet */
	file_handle = NULL;

	/* File type is "SAVE" */
	FILE_TYPE(FILE_TYPE_SAVE);


	/* Create the savefile */
	fd = fd_make(name, mode);

	/* File is okay */
	if (fd >= 0)
	{
		/* Close the "fd" */
		(void)fd_close(fd);

		/* Open the savefile */
		file_handle = my_fopen(name, "w");

		/* Successful open */
		if (file_handle)
		{
			/* Write the savefile */
			if (wr_savefile_new(Ind)) ok = TRUE;

			/* Attempt to close it */
			if (my_fclose(file_handle)) ok = FALSE;
		}

		/* Remove "broken" files */
		if (!ok) (void)fd_kill(name);
	}


	/* Failure */
	if (!ok) return (FALSE);

	/* Successful save */
	/*server_saved = TRUE;*/

	/* Success */
	return (TRUE);
}



/*
 * Attempt to save the player in a savefile
 */
bool save_player(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		result = FALSE;

	char	safe[1024];


#ifdef SET_UID

# ifdef SECURE

	/* Get "games" permissions */
	beGames();

# endif

#endif


	/* New savefile */
	strcpy(safe, p_ptr->savefile);
	strcat(safe, ".new");

#ifdef VM
	/* Hack -- support "flat directory" usage on VM/ESA */
	strcpy(safe, p_ptr->savefile);
	strcat(safe, "n");
#endif /* VM */

	/* Remove it */
	fd_kill(safe);

	/* Attempt to save the player */
	if (save_player_aux(Ind, safe))
	{
		char temp[1024];

		/* Old savefile */
		strcpy(temp, p_ptr->savefile);
		strcat(temp, ".old");

#ifdef VM
		/* Hack -- support "flat directory" usage on VM/ESA */
		strcpy(temp, p_ptr->savefile);
		strcat(temp, "o");
#endif /* VM */

		/* Remove it */
		fd_kill(temp);

		/* Preserve old savefile */
		fd_move(p_ptr->savefile, temp);

		/* Activate new savefile */
		fd_move(safe, p_ptr->savefile);

		/* Remove preserved savefile */
		fd_kill(temp);

		/* Hack -- Pretend the character was loaded */
		/*character_loaded = TRUE;*/

#ifdef VERIFY_SAVEFILE

		/* Lock on savefile */
		strcpy(temp, savefile);
		strcat(temp, ".lok");

		/* Remove lock file */
		fd_kill(temp);

#endif

		/* Success */
		result = TRUE;
	}


#ifdef SET_UID

# ifdef SECURE

	/* Drop "games" permissions */
	bePlayer();

# endif

#endif


	/* Return the result */
	return (result);
}



/*
 * Attempt to Load a "savefile"
 *
 * Version 2.7.0 introduced a slightly different "savefile" format from
 * older versions, requiring a completely different parsing method.
 *
 * Note that savefiles from 2.7.0 - 2.7.2 are completely obsolete.
 *
 * Pre-2.8.0 savefiles lose some data, see "load2.c" for info.
 *
 * Pre-2.7.0 savefiles lose a lot of things, see "load1.c" for info.
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the global
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool load_player(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		fd = -1;

	errr	err = 0;

	byte	vvv[4];

	cptr	what = "generic";


	/* Paranoia */
	/*turn = 0;*/

	/* Paranoia */
	p_ptr->death = FALSE;


	/* Allow empty savefile name */
	if (!p_ptr->savefile[0]) return (TRUE);


#if !defined(MACINTOSH) && !defined(VM)

	/* XXX XXX XXX Fix this */

	/* Verify the existance of the savefile */
	if (access(p_ptr->savefile, 0) < 0)
	{
		/* Give a message */
		plog(format("Savefile does not exist for player %s", p_ptr->name));

		/* Allow this */
		return (TRUE);
	}

#endif


#ifdef VERIFY_SAVEFILE

	/* Verify savefile usage */
	if (!err)
	{
		FILE *fkk;

		char temp[1024];

		/* Extract name of lock file */
		strcpy(temp, p_ptr->savefile);
		strcat(temp, ".lok");

		/* Check for lock */
		fkk = my_fopen(temp, "r");

		/* Oops, lock exists */
		if (fkk)
		{
			/* Close the file */
			my_fclose(fkk);

			/* Message */
			msg_print(Ind, "Savefile is currently in use.");
			msg_print(Ind, NULL);

			/* Oops */
			return (FALSE);
		}

		/* Create a lock file */
		fkk = my_fopen(temp, "w");

		/* Dump a line of info */
		fprintf(fkk, "Lock file for savefile '%s'\n", p_ptr->savefile);

		/* Close the lock file */
		my_fclose(fkk);
	}

#endif


	/* Okay */
	if (!err)
	{
		/* Open the savefile */
		fd = fd_open(p_ptr->savefile, O_RDONLY);

		/* No file */
		if (fd < 0) err = -1;

		/* Message (below) */
		if (err) what = "Cannot open savefile";
	}

	/* Process file */
	if (!err)
	{

		/* Read the first four bytes */
		if (fd_read(fd, (char*)(vvv), 4)) err = -1;

		/* What */
		if (err) what = "Cannot read savefile";

		/* Close the file */
		(void)fd_close(fd);
	}

	/* Process file */
	if (!err)
	{
		/* Attempt to load */
		err = rd_savefile_new(Ind);

		/* Message (below) */
		if (err) what = "Cannot parse savefile";
		if (err == BAD_PASSWORD) what = "Incorrect password";
	}

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}

	/* Okay */
	if (!err)
	{
		/* Player is dead */
		if (p_ptr->death)
		{
			/* Player is no longer "dead" */
			p_ptr->death = FALSE;

#if 0
			/* Cheat death */
			if (arg_wizard)
			{
				/* A character was loaded */
				character_loaded = TRUE;

				/* Done */
				return (TRUE);
			}
#endif

			/* Count lives */
			sf_lives++;

			/* Forget turns */
			/*turn = old_turn = 0;*/

			/* Done */
			return (TRUE);
		}

		/* A character was loaded */
		character_loaded = TRUE;

		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			(void)strcpy(p_ptr->died_from, "(alive and well)");
		}

		/* Success */
		return (TRUE);
	}


#ifdef VERIFY_SAVEFILE

	/* Verify savefile usage */
	if (TRUE)
	{
		char temp[1024];

		/* Extract name of lock file */
		strcpy(temp, p_ptr->savefile);
		strcat(temp, ".lok");

		/* Remove lock */
		fd_kill(temp);
	}

#endif


	/* Message */
	Destroy_connection(p_ptr->conn, format("Error (%s) reading savefile.",what));

	/* Oops */
	return (FALSE);
}

static bool wr_server_savefile(void)
{
        int        i;

        u32b              now;

        byte            tmp8u;
        u16b            tmp16u;
		u32b		tmp32u;


        /* Guess at the current time */
        now = time((time_t *)0);


        /* Note the operating system */
        sf_xtra = 0L;

        /* Note when the file was saved */
        sf_when = now;

        /* Note the number of saves */
        sf_saves++;


        /*** Actually write the file ***/
		start_section("mangband_server_save");

		/* Dump the file header */
		start_section("version");
		write_int("major",VERSION_MAJOR);
		write_int("minor",VERSION_MINOR);
		write_int("patch",VERSION_PATCH);
		end_section("version");

        /* Operating system */
		write_uint("xtra",sf_xtra);


        /* Time file last saved */
		write_uint("timestamp",sf_when);

        /* Number of past lives */
		write_uint("sf_lives",sf_lives);

        /* Number of times saved */
		write_uint("sf_saves",sf_saves);

        /* Dump the monster (unique) lore */
		start_section("monster_lore");
        tmp16u = MAX_R_IDX;
		write_uint("max_r_idx",tmp16u);
        for (i = 0; i < tmp16u; i++) wr_lore(i);
		end_section("monster_lore");

        /* Hack -- Dump the artifacts */
		start_section("artifacts");
        tmp16u = MAX_A_IDX;
		write_uint("max_a_idx",tmp16u);
        for (i = 0; i < tmp16u; i++)
        {
                artifact_type *a_ptr = &a_info[i];
                write_uint("artifact",a_ptr->cur_num);
        }
		end_section("artifacts");


	start_section("stores");
	/* Note the stores */
	tmp16u = MAX_STORES;
	write_uint("max_stores",tmp16u);

	/* Dump the stores */
	for (i = 0; i < tmp16u; i++) wr_store(&store[i]);
	end_section("stores");

	start_section("parties");
	/* Note the parties */
	tmp16u = MAX_PARTIES;
	write_int("max_parties",tmp16u);

	/* Dump the parties */
	for (i = 0; i < tmp16u; i++) wr_party(&parties[i]);
	end_section("parties");

	/* Dump the dungeons */
	start_section("dungeon_levels");
	/* get the number of levels to dump */
	tmp32u = 0;
	for (i = -MAX_WILD; i < MAX_DEPTH; i++)
	{
		/* make sure the level has been allocated */
		if ((players_on_depth[i] || check_special_level(i) ) && cave[i]) tmp32u++;
	}
	/* write the number of levels */
	write_int("num_levels",tmp32u);

	/* write the levels players are actually on - and special levels */
	/* note that this saves the players_on_depth information */
	for (i = -MAX_WILD; i < MAX_DEPTH; i++)
	{
		if ((players_on_depth[i] || check_special_level(i) ) && cave[i]) 
		{
			wr_dungeon(i);
		}
	}
	end_section("dungeon_levels");

	start_section("monsters");
	/* Prepare to write the monsters */
	compact_monsters(0);
	/* Note the number of monsters */
	tmp32u = m_max;
	write_int("max_monsters",tmp32u);
	/* Dump the monsters */
	for (i = 0; i < tmp32u; i++) wr_monster(&m_list[i]);
	end_section("monsters");

	start_section("objects");
	/* Prepare to write the objects */
	compact_objects(0);
	/* Note the number of objects */
	tmp16u = o_max;
	write_int("max_objects",tmp16u);
	/* Dump the objects */
	for (i = 0; i < tmp16u; i++) wr_item(&o_list[i]);
	end_section("objects");

	start_section("houses");
	/* Note the number of houses */
	tmp16u = num_houses;
	write_int("num_houses",tmp16u);

	/* Dump the houses */
	for (i = 0; i < tmp16u; i++) wr_house(&houses[i]); 
	end_section("houses");

	start_section("wilderness");
	/* Note the size of the wilderness 
	 change this to num_wild ? */
	tmp32u = MAX_WILD;
	write_int("max_wild",tmp32u);
	 /* Dump the wilderness */
	for (i = 1; i < tmp32u; i++) wr_wild(&wild_info[-i]);
	end_section("wilderness");

	/* Write the player name database */
	start_section("player_names");
	wr_player_names();
	end_section("player_names");

	write_uint("seed_flavor",seed_flavor);
	write_uint("seed_town",seed_town);

	write_uint("player_id",player_id);
	write_huge("turn",turn);

	end_section("mangband_server_save");


        /* Error in save */
        if (ferror(file_handle) || (fflush(file_handle) == EOF)) return FALSE;

        /* Successful save */
        return TRUE;
}


static bool save_server_aux(char *name)
{
        bool    ok = FALSE;

        int             fd = -1;

        int             mode = 0644;


        /* No file yet */
		file_handle = NULL;

        /* File type is "SAVE" */
        FILE_TYPE(FILE_TYPE_SAVE);


        /* Create the savefile */
        fd = fd_make(name, mode);

        /* File is okay */
        if (fd >= 0)
        {
                /* Close the "fd" */
                (void)fd_close(fd);

                /* Open the savefile */
                file_handle = my_fopen(name,"w");

                /* Successful open */
                if (file_handle)
                {
                        /* Write the savefile */
                        if (wr_server_savefile()) ok = TRUE;

                        /* Attempt to close it */
                        if (my_fclose(file_handle)) ok = FALSE;
                }

                /* Remove "broken" files */
                if (!ok) (void)fd_kill(name);
        }


        /* Failure */
        if (!ok) return (FALSE);

        /* Successful save */
        /*server_saved = TRUE;*/

        /* Success */
        return (TRUE);
}


/*
 * Load the server info (artifacts created and uniques killed)
 * from a special savefile.
 */
bool load_server_info(void)
{
	int fd = -1;

	byte vvv[4];

	errr err = 0;

	cptr what = "generic";

	char buf[1024];

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "server");

#if !defined(MACINTOSH) && !defined(VM)

	/* XXX XXX XXX Fix this */
	if (access(buf, 0) < 0)
	{
		/* Give message */
		plog("Server savefile does not exist");

		/* Allow this */
		return (TRUE);
	}

#endif

        /* Okay */
        if (!err)
        {
                /* Open the savefile */
                fd = fd_open(buf, O_RDONLY);

                /* No file */
                if (fd < 0) err = -1;

                /* Message (below) */
                if (err) what = "Cannot open savefile";
        }

        /* Can we read from the file? */
        if (!err)
        {
                /* Read the first four bytes */
                if (fd_read(fd, (char*)(vvv), 4)) err = -1;

                /* What */
                if (err) what = "Cannot read savefile";

                /* Close the file */
                (void)fd_close(fd);
        }

        /* Process file */
        if (!err)
        {
                /* Attempt to load */
                err = rd_server_savefile();

                /* Message (below) */
                if (err) what = "Cannot parse savefile";
	}

        /* Okay */
        if (!err)
        {
                /* The server state was loaded */
                server_state_loaded = TRUE;

                /* Success */
                return (TRUE);
        }

	/* Message */
	plog(format("Error (%s,%d) reading server savefile.", what, err));

	return (FALSE);
}


/*
 * Save the server state to a "server" savefile.
 */
bool save_server_info(void)
{
	int result = FALSE;
	char safe[1024];

	/* New savefile */
	path_build(safe, 1024, ANGBAND_DIR_SAVE, "server.new");

	/* Remove it */
	fd_kill(safe);

	/* Attempt to save the server state */
	if (save_server_aux(safe))
	{
		char temp[1024];
		char prev[1024];

		/* Old savefile */
		path_build(temp, 1024, ANGBAND_DIR_SAVE, "server.old");

		/* Remove it */
		fd_kill(temp);

		/* Name of previous savefile */
		path_build(prev, 1024, ANGBAND_DIR_SAVE, "server");

		/* Preserve old savefile */
		fd_move(prev, temp);

		/* Activate new savefile */
		fd_move(safe, prev);

		/* Remove preserved savefile */
		fd_kill(temp);

		/* Success */
		result = TRUE;
	}

	/* Return the result */
	return (result);
}
