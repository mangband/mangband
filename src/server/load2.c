/* File: load2.c */

/* Purpose: support for loading savefiles -BEN- */

#include "mangband.h"
#include "../common/md5.h"


/*
 * This file is responsible for loading all "2.7.X" savefiles
 *
 * Note that 2.7.0 - 2.7.2 savefiles are obsolete and will not work.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that Angband 2.7.9 encodes "terrain features" in the savefile
 * using the old 2.7.8 method.  Angband 2.8.0 will use the same method
 * to read pre-2.8.0 savefiles, but will use a new method to save them,
 * which will only affect "save.c".
 *
 * Note that Angband 2.8.0 will use a VERY different savefile method,
 * which will use "blocks" of information which can be ignored or parsed,
 * and which will not use a silly "protection" scheme on the savefiles,
 * but which may still use some form of "checksums" to prevent the use
 * of "corrupt" savefiles, which might cause nasty weirdness.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 *
 * We should also make the "cheating" options official flags, and
 * move the "byte" options to a different part of the code, perhaps
 * with a few more (for variety).
 *
 * Implement simple "savefile extenders" using some form of "sized"
 * chunks of bytes, with a {size,type,data} format, so everyone can
 * know the size, interested people can know the type, and the actual
 * data is available to the parsing routines that acknowledge the type.
 *
 * Consider changing the "globe of invulnerability" code so that it
 * takes some form of "maximum damage to protect from" in addition to
 * the existing "number of turns to protect for", and where each hit
 * by a monster will reduce the shield by that amount.
 *
 * XXX XXX XXX
 */





/*
 * Local "savefile" pointer
 */
static FILE *file_handle; 
/* Line counter */
static int line_counter;


/*
 * Hack -- simple "checksum" on the actual values
 */
static u32b	v_check = 0L;

/*
 * Hack -- simple "checksum" on the encoded bytes
 */
static u32b	x_check = 0L;

/*
 * Hack -- buffer for reading text save files
 */
static char file_buf[1024];

/*
 * Functions to read data from the textual format save file
 */

/* Start a section */
void start_section_read(char* name)
{
	char seek_section[80];
	char got_section[80];
	bool matched = FALSE;
	
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		sprintf(seek_section,"<%s>",name);
		if(sscanf(file_buf,"%s",got_section) == 1)
		{
			matched = !strcmp(got_section,seek_section);
		}
	}
	if(!matched)
	{
		plog(format("Missing section.  Expected '%s', found '%s' at line %i",seek_section,got_section,line_counter));
		exit(1);
	}
}

/* End a section */
void end_section_read(char* name)
{
	char seek_section[80];
	char got_section[80];
	bool matched = FALSE;
		
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		sprintf(seek_section,"</%s>",name);
		if(sscanf(file_buf,"%s",got_section) == 1)
		{
			matched = !strcmp(got_section,seek_section);
		}
	}
	if(!matched)
	{
		plog(format("Missing end section.  Expected '%s', found '%s' at line %i",seek_section,got_section,line_counter));
		exit(1);
	}
}

/* Read an integer */
int read_int(char* name)
{
	char seek_name[80];
	bool matched = FALSE;
	int value;
		
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if(sscanf(file_buf,"%s = %i",seek_name,&value) == 2)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{
		plog(format("Missing integer.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		exit(1);
	}
	return value;
}

/* Read an unsigned integer */
uint read_uint(const char* name)
{
	char seek_name[80];
	bool matched = FALSE;
	uint value;
		
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if(sscanf(file_buf,"%s = %u",seek_name,&value) == 2)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{		
		plog(format("Missing unsigned integer.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		exit(1);
	}
	return value;
}

/* Read a 'huge' */
huge read_huge(char* name)
{
	char seek_name[80];
	bool matched = FALSE;
	huge value;
		
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if(sscanf(file_buf,"%s = %" SCNu64 ,seek_name,&value) == 2)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{		
		plog(format("Missing signed long.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		exit(1);
	}
	return value;
}

/* Read an hturn */
void read_hturn(char* name, hturn *value)
{
	char seek_name[80];	
	bool matched = FALSE;
	s64b era, turn;

	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if (sscanf(file_buf, "%s = %" SCNu64 " %" SCNu64, seek_name, &era, &turn) == 3)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{		
		plog(format("Missing hturn.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		exit(1);
	}
	
	value->era = era;
	value->turn = turn;

	return;
}

/* Read a string */
/* Returns TRUE if the string could be read */
void read_str(char* name, char* value)
{
	char seek_name[80];
	bool matched = FALSE;
	char *c;
	
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		sscanf(file_buf,"%s = ",seek_name);
		if (!strcmp(seek_name,name))
		{
			matched = TRUE;
		}
	}
	if (!matched)
	{
		plog(format("Missing string data.  Expected '%s' got '%s' at line %i",name,seek_name,line_counter));
		exit(1);
	}

	c = file_buf;	
	while(*c != '=') c++;
	c+=2;

	while( *c >= 31 )
	{
		*value = *c; c++; value++;
	}
	*value = '\0';
}

/* Read a float */
/* Returns TRUE if the float could be read */
float read_float(char* name)
{
	char seek_name[80];
	bool matched = FALSE;
	float value;
	
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if(sscanf(file_buf,"%s = %f",seek_name,&value) == 2)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{
		plog(format("Missing float.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		exit(1);
	}
	return value;
}

/* Read some binary data */
void read_binary(char* name, char* value, int max_len)
{
	char seek_name[80];
	bool matched = FALSE;
	char hex[3];
	char *c;
	char *bin;
	unsigned int abyte;
	hex[2] = '\0';

	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		sscanf(file_buf,"%s = ",seek_name);
		if (!strcmp(seek_name,name))
		{
			matched = TRUE;
		}
	}
	if (!matched)
	{
		plog(format("Missing binary data.  Expected '%s' got '%s' at line %i",name,seek_name,line_counter));
		exit(1);
	}

	c = file_buf;	
	while(*c != '=') c++;
	c+=2;
	
	bin = value;
	while( *c >= 31 )
	{
		hex[0] = *c; c++;
		hex[1] = *c; c++;
		sscanf(hex,"%2x",&abyte);
		*bin = (byte)abyte;
		bin++;
	}
}

/* Skip a named value */
void skip_value(char* name)
{
	char seek_name[80];
	long fpos;
	
	/* Remember where we are incase there is nothing to skip */
	fpos = ftell(file_handle);
	sprintf(seek_name,"%s = ",name);
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		line_counter++;
		if(strstr(file_buf,seek_name) == NULL)
		{
			/* Move back on seek failures */
			fseek(file_handle,fpos,SEEK_SET);
			line_counter--;
			/* plog(format("Missing value.  Expected to skip '%s', found '%s' at line %i\n",name,seek_name,line_counter)); */
			/* exit(1); */
		}
	}
}

/* Check if the given named value is next */
bool value_exists(const char* name)
{
	char seek_name[80];
	bool matched = FALSE;
	long fpos;
	
	/* Remember where we are */
	fpos = ftell(file_handle);
	sprintf(seek_name,"%s = ",name);
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		matched = TRUE;
		if(strstr(file_buf,seek_name) == NULL)
		{
			matched = FALSE;
		}
	}
	/* Move back */
	fseek(file_handle,fpos,SEEK_SET);
	return(matched);
}

/* Check if the given named section is next */
bool section_exists(char* name)
{
	char seek_section[80];
	char got_section[80];
	bool matched = FALSE;
	long fpos;
	
	/* Remember where we are */
	fpos = ftell(file_handle);
	if (fgets(file_buf, sizeof(file_buf)-1, file_handle))
	{
		sprintf(seek_section,"<%s>",name);
		if(sscanf(file_buf,"%s",got_section) == 1)
		{
			matched = !strcmp(got_section,seek_section);
		}
	}
	/* Move back */
	fseek(file_handle,fpos,SEEK_SET);
	return(matched);
}

/*
 * Show information on the screen, one line at a time.
 * Start at line 2, and wrap, if needed, back to line 2.
 */
static void note(cptr msg)
{
#if 0
	static int y = 2;

	/* Draw the message */
	prt(msg, y, 0);

	/* Advance one line (wrap if needed) */
	if (++y >= 24) y = 2;

	/* Flush it */
	Term_fresh();
#endif
}


/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
static bool wearable_p(object_type *o_ptr)
{
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
		case TV_LITE:
		case TV_AMULET:
		case TV_RING:
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}


/*
 * Read an item (2.7.0 or later)
 *
 * Note that savefiles from 2.7.0 and 2.7.1 are obsolete.
 *
 * Note that pre-2.7.9 savefiles (from Angband 2.5.1 onward anyway)
 * can be read using the code above.
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 *
 * Note that Angband 2.7.9 introduced a new method for object "flags"
 * in which the "flags" on an object are actually extracted when they
 * are needed from the object kind, artifact index, ego-item index,
 * and two special "xtra" fields which are used to encode any "extra"
 * power of certain ego-items.  This had the side effect that items
 * imported from pre-2.7.9 savefiles will lose any "extra" powers they
 * may have had, and also, all "uncursed" items will become "cursed"
 * again, including Calris, even if it is being worn at the time.  As
 * a complete hack, items which are inscribed with "uncursed" will be
 * "uncursed" when imported from pre-2.7.9 savefiles.
 */
 /* For wilderness levels, dun_depth has been changed from 1 to 4 bytes. */
static void rd_item(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;

	u32b f1, f2, f3;

	object_kind *k_ptr;

	char note[128];

	start_section_read("item");
	
	/* Hack -- wipe */
	WIPE(o_ptr, object_type);

	/* Skip name */
	skip_value("name");

	/* Kind */
	o_ptr->k_idx = read_int("k_idx");

	/* Location */
	o_ptr->iy = read_int("iy");
	o_ptr->ix = read_int("ix");
	
	o_ptr->dun_depth = read_int("dun_depth");

	/* Type/Subtype */
	o_ptr->tval = read_int("tval");
	o_ptr->sval = read_int("sval");

	/* Base pval */
	o_ptr->bpval = read_int("bpval");

	/* Special pval */
	o_ptr->pval = read_int("pval");


	o_ptr->discount = read_int("discount");
	o_ptr->number = read_int("number");
	o_ptr->weight = read_int("weight");

	o_ptr->name1 = read_int("name1");
	o_ptr->name2 = read_int("name2");
	o_ptr->name3 = read_int("name3");
	o_ptr->timeout = read_int("timeout");

	o_ptr->to_h = read_int("to_h");
	o_ptr->to_d = read_int("to_d");
	o_ptr->to_a = read_int("to_a");

	o_ptr->ac = read_int("ac");

	old_dd = read_int("dd");
	old_ds = read_int("ds");

	o_ptr->ident = read_int("ident");

	/* Special powers */
	o_ptr->xtra1 = read_int("xtra1");
	o_ptr->xtra2 = read_int("xtra2");

	/* Inscription */
	read_str("inscription",note); 
 
	/* Save the inscription */
	if (note[0]) o_ptr->note = quark_add(note);

	/* Owner information */
	if (value_exists("owner_name"))
	{
		/* Name */
		read_str("owner_name",note);
		/* Save */
		if (!STRZERO(note)) o_ptr->owner_name = quark_add(note); 
		/* Id */
		o_ptr->owner_id = read_int("owner_id");
	}

	/* Monster holding object */ 
   o_ptr->held_m_idx = read_int("held_m_idx");

	end_section_read("item");

	/* Mega-Hack -- handle "dungeon objects" later */
	if ((o_ptr->k_idx >= 445) && (o_ptr->k_idx <= 479)) return;


	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Obtain tval/sval from k_info */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;


	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= ID_BROKEN;


	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Acquire correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Acquire correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* Acquire correct weight */
		o_ptr->weight = k_ptr->weight;

		/* Paranoia */
		o_ptr->name1 = o_ptr->name2 = 0;

		/* All done */
		return;
	}


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);


	/* Paranoia */
	if (true_artifact_p(o_ptr))
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Verify that artifact */
		if (!a_ptr->name) o_ptr->name1 = 0;
	}

	/* Paranoia */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Acquire standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Acquire standard weight */
	o_ptr->weight = k_ptr->weight;

	/* Hack -- extract the "broken" flag */
	if (o_ptr->pval < 0) o_ptr->ident |= ID_BROKEN;


	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
#if defined(RANDART)
		if (o_ptr->name1 == ART_RANDART)
		{
			a_ptr = randart_make(o_ptr);
		}
		else
		{
#endif
		a_ptr = &a_info[o_ptr->name1];
#if defined(RANDART)
		}
#endif
		/* Acquire new artifact "pval" */
		o_ptr->pval = a_ptr->pval;

		/* Acquire new artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Acquire new artifact weight */
		o_ptr->weight = a_ptr->weight;

		/* Hack -- extract the "broken" flag */
		if (!a_ptr->cost) o_ptr->ident |= ID_BROKEN;
	}

	/* Ego items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Hack -- keep some old fields */
		if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
		{
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;
		}

		/* Hack -- extract the "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= ID_BROKEN;

		/* Mega-Hack - Enforce the special broken items */
		if ((o_ptr->name2 == EGO_BLASTED) ||
			(o_ptr->name2 == EGO_SHATTERED))
		{
			/* These were set to k_info values by preceding code */
			o_ptr->ac = 0;
			o_ptr->dd = 0;
			o_ptr->ds = 0;
		}
	}
}


/*
 * Read a monster
 */

static void rd_monster(monster_type *m_ptr)
{
	start_section_read("monster");

	/* Hack -- wipe */
	WIPE(m_ptr, monster_type);

	skip_value("name");

	/* Read the monster race */
	m_ptr->r_idx = read_int("r_idx");

	/* Read the other information */
	m_ptr->fy = read_int("fy");
	m_ptr->fx = read_int("fx");
	m_ptr->dun_depth = read_int("dun_depth");
	m_ptr->hp = read_int("hp");
	m_ptr->maxhp = read_int("maxhp");
	m_ptr->csleep = read_int("csleep");
	m_ptr->mspeed = read_int("mspeed");
	m_ptr->energy = read_uint("energy");
	m_ptr->stunned = read_int("stunned");
	m_ptr->confused = read_int("confused");
	m_ptr->monfear = read_int("afraid");

	end_section_read("monster");
}





/*
 * Read the monster lore
 */
static void rd_lore(player_type *p_ptr, int r_idx)
{
	int i;
	
	monster_lore *l_ptr = p_ptr->l_list + r_idx;

	start_section_read("lore");

	/* Count sights/deaths/kills */
	l_ptr->sights = read_int("sights");
	l_ptr->deaths = read_int("deaths");
	l_ptr->pkills = read_int("pkills");
	l_ptr->tkills = read_int("tkills");

	/* Count wakes and ignores */
	l_ptr->wake = read_int("wake");
	l_ptr->ignore = read_int("ignore");

	/* Count drops */
	l_ptr->drop_gold = read_int("drop_gold");
	l_ptr->drop_item = read_int("drop_item");

	/* Count spells */
	l_ptr->cast_innate = read_int("cast_innate");
	l_ptr->cast_spell = read_int("cast_spell");

	/* Count blows of each type */
	start_section_read("blows");
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
		l_ptr->blows[i] = read_int("blow");
	end_section_read("blows");


	/* Memorize flags */
	start_section_read("flags");
	l_ptr->flags1 = read_int("flag");
	l_ptr->flags2 = read_int("flag");
	l_ptr->flags3 = read_int("flag");
	l_ptr->flags4 = read_int("flag");
	l_ptr->flags5 = read_int("flag");
	l_ptr->flags6 = read_int("flag");
	end_section_read("flags");

	/* Repair the lore flags */
	/* No need to repair AFAIU
	l_ptr->flags1 &= r_ptr->flags1;
	l_ptr->flags2 &= r_ptr->flags2;
	l_ptr->flags3 &= r_ptr->flags3;
	l_ptr->flags4 &= r_ptr->flags4;
	l_ptr->flags5 &= r_ptr->flags5;
	l_ptr->flags6 &= r_ptr->flags6;
	*/
	end_section_read("lore");
}

/*
 * Read the uniques lore
 */
static void rd_u_lore(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	start_section_read("lore");

	skip_value("name");

	/* Count sights/deaths/kills */
	r_ptr->r_sights = read_int("sights");
	if (value_exists("deaths")) skip_value("deaths");
	if (value_exists("pkills")) skip_value("pkills");
	r_ptr->r_tkills = read_int("tkills");
	
	if (value_exists("wake")) skip_value("wake");
	if (value_exists("ignore")) skip_value("ignore");
	if (value_exists("respawn_timer")) skip_value("respawn_timer");
	if (value_exists("drop_gold")) skip_value("drop_gold");
	if (value_exists("drop_item")) skip_value("drop_item");
	if (value_exists("cast_innate")) skip_value("cast_innate");
	if (value_exists("cast_spell")) skip_value("cast_spell");
	
	if (section_exists("blows")) 
	{
		start_section_read("blows");
		while(value_exists("blow")) skip_value("blow");
		end_section_read("blows");
	}
	if (section_exists("flags")) 
	{
		start_section_read("flags");
		while(value_exists("flag")) skip_value("flag");
		end_section_read("flags");
	}
	
	/* Read the "Racial" monster limit per level */
	r_ptr->max_num = read_int("max_num");

	if (value_exists("killer")) skip_value("killer");
	
	end_section_read("lore");

}




/*
 * Read a store
 */
static errr rd_store(int n)
{
	store_type *st_ptr = &store[n];

	int j;

	byte own, num;

	start_section_read("store");

	/* Read the basic info */
	read_hturn("store_open", &st_ptr->store_open);
	st_ptr->insult_cur = read_int("insult_cur");
	own = read_int("owner");
	num = read_int("stock_num");
	st_ptr->good_buy = read_int("good_buy");
	st_ptr->bad_buy = read_int("bad_buy");

	/* Extract the owner (see above) */
	st_ptr->owner = own;

	start_section_read("stock");
	/* Read the items */
	for (j = 0; j < num; j++)
	{
		object_type forge;

		/* Read the item */
		rd_item(&forge);

		/* Acquire valid items */
		if (st_ptr->stock_num < STORE_INVEN_MAX)
		{
			/* Acquire the item */
			st_ptr->stock[st_ptr->stock_num++] = forge;
		}
	}
	end_section_read("stock");
	end_section_read("store");

	/* Success */
	return (0);
}


/*
 * Read some party info
 
 FOUND THE BUIG!!!!!! the disapearing party bug. no more.
 I hope......
-APD-
 */
static void rd_party(int n)
{
	party_type *party_ptr = &parties[n];

	start_section_read("party");

	/* Party name */
	read_str("name",party_ptr->name);

	/* Party owner's name */
	read_str("owner",party_ptr->owner);

	/* Number of people and creation time */
	party_ptr->num = read_int("num");
	read_hturn("created", &party_ptr->created);

	end_section_read("party");
}

/*
 * Read some house info
 */
static void rd_house(int n)
{
	house_type *house_ptr = &houses[n];

	start_section_read("house");

	/* coordinates of corners of house */
	house_ptr->x_1 = read_int("x1");
	house_ptr->y_1 = read_int("y1");
	house_ptr->x_2 = read_int("x2");
	house_ptr->y_2 = read_int("y2");
	
	/* coordinates of the house door */
	house_ptr->door_y = read_int("door_y");
	house_ptr->door_x = read_int("door_x");

	/* Door Strength */
	house_ptr->strength = read_int("strength");

	/* Owned or not */
	read_str("owned",house_ptr->owned); 

	house_ptr->depth = read_int("depth");
	house_ptr->price = read_int("price");

	end_section_read("house");

}

/*
 * Read some arena info
 */
static void rd_arena(int n)
{
	arena_type *arena_ptr = &arenas[n];

	start_section_read("arena");

	/* coordinates of corners of house */
	arena_ptr->x_1 = read_int("x1");
	arena_ptr->y_1 = read_int("y1");
	arena_ptr->x_2 = read_int("x2");
	arena_ptr->y_2 = read_int("y2");
	
	arena_ptr->depth = read_int("depth");

	end_section_read("arena");
}

static void rd_wild(int n)
{
	wilderness_type *w_ptr = &wild_info[-n];
	
	/* the flags */
	w_ptr->flags = read_uint("flags");
	
}


/*
 * Read/Write the "extra" information
 */

static bool rd_extra(player_type *p_ptr, bool had_header)
{
	int i = 0;

	start_section_read("player");

	if (!had_header)
	{
		read_str("playername",p_ptr->name); /* 32 */
		skip_value("pass");
	}

	read_str("died_from",p_ptr->died_from); /* 80 */

	read_str("died_from_list",p_ptr->died_from_list); /* 80 */
	p_ptr->died_from_depth = read_int("died_from_depth");

	start_section_read("history");
	for (i = 0; i < 4; i++)
	{
		read_str("history",p_ptr->history[i]); /* 60 */
	}
	if (value_exists("descrip"))
	read_str("descrip",p_ptr->descrip); /* 240?! */
	end_section_read("history");

	/* Class/Race/Gender/Party */
	if (!had_header)
	{
		p_ptr->prace = read_int("prace");
		p_ptr->pclass = read_int("pclass");
		p_ptr->male = read_int("male");
	}
	p_ptr->party = read_int("party");

	/* Special Race/Class info */
	p_ptr->hitdie = read_int("hitdie");
	p_ptr->expfact = read_int("expfact");

	/* Age/Height/Weight */
	p_ptr->age = read_int("age");
	p_ptr->ht = read_int("ht");
	p_ptr->wt = read_int("wt");

	/* Read the stat info */
	start_section_read("stats");
	for (i = 0; i < 6; i++) p_ptr->stat_max[i] = read_int("stat_max");
	for (i = 0; i < 6; i++) p_ptr->stat_cur[i] = read_int("stat_cur");
	end_section_read("stats");

	p_ptr->id = read_int("id");

	/* If he was created in the pre-ID days, give him one */
	if (!p_ptr->id)
		p_ptr->id = player_id++;

	p_ptr->au = read_int("au");

	p_ptr->max_exp = read_int("max_exp");
	p_ptr->exp = read_int("exp");
	p_ptr->exp_frac = read_int("exp_frac");

	p_ptr->lev = read_int("lev");

	p_ptr->mhp = read_int("mhp");
	p_ptr->chp = read_int("chp");
	p_ptr->chp_frac = read_int("chp_frac");

	p_ptr->msp = read_int("msp");
	p_ptr->csp = read_int("csp");
	p_ptr->csp_frac = read_int("csp_frac");

	if(value_exists("no_ghost"))
	{
		(void)read_int("no_ghost");
	}
	
	p_ptr->max_plv = read_int("max_plv");
	p_ptr->max_dlv = read_int("max_dlv");
	
	p_ptr->recall_depth = p_ptr->max_dlv;

	p_ptr->py = read_int("py");
	p_ptr->px = read_int("px");
	p_ptr->dun_depth = read_int("dun_depth");

	p_ptr->world_x = read_int("world_x");
	p_ptr->world_y = read_int("world_y");

	/* More info */
	
	p_ptr->ghost = read_int("ghost");
	p_ptr->sc = read_int("sc");
	p_ptr->fruit_bat = read_int("fruit_bat");

	/* Read the flags */
	p_ptr->lives = read_int("lives");

	/* hack */
	p_ptr->blind = read_int("blind");
	p_ptr->paralyzed = read_int("paralyzed");
	p_ptr->confused = read_int("confused");
	p_ptr->food = read_int("food");
	p_ptr->energy = read_uint("energy");
	p_ptr->fast = read_int("fast");
	p_ptr->slow = read_int("slow");
	p_ptr->afraid = read_int("afraid");
	p_ptr->cut = read_int("cut");
	p_ptr->stun = read_int("stun");
	p_ptr->poisoned = read_int("poisoned");
	p_ptr->image = read_int("image");
	p_ptr->protevil = read_int("protevil");
	p_ptr->invuln = read_int("invuln");
	p_ptr->hero = read_int("hero");
	p_ptr->shero = read_int("shero");
	p_ptr->shield = read_int("shield");
	p_ptr->blessed = read_int("blessed");
	p_ptr->tim_invis = read_int("tim_invis");
	p_ptr->word_recall = read_int("word_recall");
	p_ptr->see_infra = read_int("see_infra");
	p_ptr->tim_infra = read_int("tim_infra");
	
	p_ptr->oppose_fire = read_int("oppose_fire");
	p_ptr->oppose_cold = read_int("oppose_cold");
	p_ptr->oppose_acid = read_int("oppose_acid");
	p_ptr->oppose_elec = read_int("oppose_elec");
	p_ptr->oppose_pois = read_int("oppose_pois");

	p_ptr->confusing = read_int("confusing");
	p_ptr->searching = read_int("searching");
	p_ptr->maximize = read_int("maximize");
	p_ptr->preserve = read_int("preserve");

	/* Read the unique list info */
	start_section_read("uniques");
	for (i = 0; i < z_info->r_max; i++) p_ptr->r_killed[i] = read_int("unique");
	end_section_read("uniques");

	/* Special stuff */
	panic_save = read_int("panic_save");
	p_ptr->total_winner = read_int("total_winner");
	p_ptr->retire_timer = read_int("retire_timer");
	p_ptr->noscore = read_int("noscore");

	/* Read "death" */
	p_ptr->death = read_int("death");

	end_section_read("player");

	/* Success */
	return FALSE;
}




/*
 * Read the player inventory
 *
 * Note that the inventory changed in Angband 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
 *
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
static errr rd_inventory(player_type *p_ptr)
{
	int slot = 0;

	object_type forge;

	start_section_read("inventory");

	/* No weight */
	p_ptr->total_weight = 0;

	/* No items */
	p_ptr->inven_cnt = 0;
	p_ptr->equip_cnt = 0;

	/* Read until done */
	while (1)
	{
		u16b n;

		/* Get the next item index */
		n = read_int("inv_entry");

		/* Nope, we reached the end */
		if (n == 0xFFFF) break;

		/* Read the item */
		rd_item(&forge);

		/* Hack -- verify item */
		if (!forge.k_idx) return (53);

		/* Mega-Hack -- Handle artifacts that aren't yet "created" */
		if (artifact_p(&forge))
		{
			/* If this artifact isn't created, mark it as created */
			/* Only if this isn't a "death" restore */
			if (!a_info[forge.name1].cur_num && !p_ptr->death)
				a_info[forge.name1].cur_num = 1;
		}

		/* Wield equipment */
		if (n >= INVEN_WIELD)
		{
			/* Structure copy */
			p_ptr->inventory[n] = forge;

			/* Add the weight */
			p_ptr->total_weight += (forge.number * forge.weight);

			/* One more item */
			p_ptr->equip_cnt++;
		}

		/* Warning -- backpack is full */
		else if (p_ptr->inven_cnt == INVEN_PACK)
		{
			/* Oops */
			/*note("Too many items in the inventory!");*/

			/* Fail */
			return (54);
		}

		/* Carry inventory */
		else
		{
			/* Get a slot */
			n = slot++;

			/* Structure copy */
			p_ptr->inventory[n] = forge;

			/* Add the weight */
			p_ptr->total_weight += (forge.number * forge.weight);

			/* One more item */
			p_ptr->inven_cnt++;
		}
	}

	end_section_read("inventory");

	/* Success */
	return (0);
}

/*
 * Read the birth options
 */
static errr rd_birthoptions(player_type *p_ptr)
{
	s32b i, id;
	u16b tmp16u, ind;

	if (!section_exists("options"))
	{
		/* Fine, no options */
		return (0);
	}

	/* Begin */
	start_section_read("options");

	/* Read number */
	tmp16u = read_int("num");

	/* Read each record */
	id = 0;
	for (i = 0; i < OPT_MAX; i++)
	{
		const option_type *opt_ptr = &option_info[i];

		/* Real index is in the o_uid! */
		ind = option_info[i].o_uid;

		if (opt_ptr->o_page != 1) continue;

		/* Next entry is what we expect */
		if (value_exists(opt_ptr->o_text))
		{
			/* Read it */
			u32b val = read_uint(opt_ptr->o_text);

			/* Set it */
			p_ptr->options[ind] = val ? TRUE : FALSE;
		}
		else
		{
			end_section_read("options");

			/* Unexpected option */
			return (29);
		}

		id++;
		/* Don't read anymore */
		if (id >= tmp16u) break;
	}

	/* Done */
	end_section_read("options");

	/* Success */
	return (0);
}


/*
 * Read hostility information
 *
 * Note that this function is responsible for deleting stale entries
 */
static errr rd_hostilities(player_type *p_ptr)
{
	hostile_type *h_ptr;
	int i;
	s32b id;
	u16b tmp16u;
  
  start_section_read("hostilities");
  
	/* Read number */
	tmp16u = read_int("num");

	/* Read available ID's */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read next ID */
		id = read_int("id");

		/* Check for stale player */
		if (id > 0 && !lookup_player_name(id)) continue;

		/* Check for stale party */
		if (id < 0 && !parties[0 - id].num) continue;

		/* Create node */
		MAKE(h_ptr, hostile_type);
		h_ptr->id = id;

		/* Add to chain */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;
	}

  end_section_read("hostilities");

	/* Success */
	return (0);
}
	

/*
 * Read the run-length encoded dungeon
 *
 * Note that this only reads the dungeon features and flags, 
 * the objects and monsters will be read later.
 *
 */

static errr rd_dungeon(bool ext, int Depth)
{
	s32b depth;
	u16b max_y, max_x;

	int y, x;
	cave_type *c_ptr;
	char cave_row[MAX_WID+1];

	start_section_read("dungeon_level");

	/*** Depth info ***/

	/* Level info */
	depth = read_int("depth");
	max_y = read_int("max_height");
	max_x = read_int("max_width");
	if (ext) depth = Depth;

	/* Turn this level was generated */
	if (value_exists("gen_turn"))
		read_hturn("gen_turn", &turn_cavegen[depth]);

	/* players on this depth */
	players_on_depth[depth] = read_int("players_on_depth");

	/* Hack -- only read in staircase information for non-wilderness
	 * levels
	 */

	if (depth >= 0)
	{

		level_up_y[depth] = read_int("level_up_y");
		level_up_x[depth] = read_int("level_up_x");
		level_down_y[depth] = read_int("level_down_y");
		level_down_x[depth] = read_int("level_down_x");
		level_rand_y[depth] = read_int("level_rand_y");
		level_rand_x[depth] = read_int("level_rand_x");
		
	}
	/* HACK */
	else if (value_exists("level_up_y"))
	{
		skip_value("level_up_y");
		skip_value("level_up_x");
		skip_value("level_down_y");
		skip_value("level_down_x");
		skip_value("level_rand_y");
		skip_value("level_rand_x");	
	}

	/* allocate the memory for the dungoen if it has not already */
	/* been allocated - which it might have been if we are loading */
	/* a special static level file */
	if(!cave[depth])
		alloc_dungeon_level(depth);

	/* Load features */
	start_section_read("features");

		for (y = 0; y < max_y; y++)
		{
			read_binary("row",cave_row,MAX_WID);
			for(x = 0; x < max_x; x++)
			{
				/* Access the cave */
				c_ptr = &cave[depth][y][x];

				/* set the feature */
				c_ptr->feat = cave_row[x];
			}			
		}

	end_section_read("features");

	/* Load info */
	start_section_read("info");

		for (y = 0; y < max_y; y++)
		{
			read_binary("row",cave_row,MAX_WID);
			for(x = 0; x < max_x; x++)
			{
				/* Access the cave */
				c_ptr = &cave[depth][y][x];

				/* set the feature */
				c_ptr->info = cave_row[x];
			}			
		}

	end_section_read("info");

	end_section_read("dungeon_level");
	/* Success */
	return (0);
}

/*
 * Read special static pre-designed dungeon levels
 * 
 * Special pre-designed levels are stored in separate
 * files with the filename "server-level-<num>" where num is
 * the level number.  Level files are searched for at runtime
 * and loaded if present.
 */
static errr rd_dungeon_special()
{
	char filename[1024];
	char levelname[32];
	FILE *fhandle;
	FILE *server_handle;
	int i,num_levels,j=0,k=0;
	
	/* Clear all the special levels */
	for(i=0;i<MAX_SPECIAL_LEVELS;i++)
	{
		special_levels[i] = -999;
	}
	
	/* Vanilla Mangand doesn't have special static pre-designed levels */
	if ((!cfg_ironman) || (!cfg_more_towns)) return 0;
	
	num_levels = 0;
	/* k = E/W, J = N/S for wilderness towns */
/*	for(k=0;k<MAX_DEPTH;k++)
	{
	for(j=0;j<MAX_DEPTH;j++)
	{*/
	for(i=0;i<MAX_DEPTH;i++)
	{
		/* build a file name */
		sprintf(levelname,"server.level.%i.%i.%i",k,j,i);
		path_build(filename, 1024, ANGBAND_DIR_SAVE, levelname);
		/* open the file if it exists */
		fhandle = my_fopen(filename, "r");
		if(fhandle)
		{
			/* swap out the main file pointer for our level file */
			server_handle = file_handle;
			file_handle = fhandle;
			/* load the level */
			rd_dungeon(FALSE, 0);
			/* swap the file pointers back */
			file_handle = server_handle;
			/* close the level file */
			my_fclose(fhandle);
			/* we have an arbitrary max number of levels */
			if(num_levels + 1 > MAX_SPECIAL_LEVELS)
			{
				note("Too many special pre-designed level files!");
				break;
			}
			/* add this depth to the special level list */
			special_levels[num_levels++] = i;
		}
	}
/*	}
	}*/
	return 0;
}

/* HACK -- Read from file */
bool rd_dungeon_special_ext(int Depth, cptr levelname)
{
	char filename[1024];
	FILE *fhandle;
	FILE *server_handle;
	
	path_build(filename, 1024, ANGBAND_DIR_SAVE, levelname);

	fhandle = my_fopen(filename, "r");

	if (fhandle)
	{
			/* swap out the main file pointer for our level file */
			server_handle = file_handle;
			file_handle = fhandle;

			/* load the level */
			rd_dungeon(TRUE, Depth);

			/* swap the file pointers back */
			file_handle = server_handle;

			/* close the level file */
			my_fclose(fhandle);

			return TRUE;
	}
	return FALSE;
}


/* Reads in a players memory of the level he is currently on, in run-length encoded
 * format.  Simmilar to the above function.
 */

static errr rd_cave_memory(player_type *p_ptr)
{
	u16b max_y, max_x;
	int y, x;
	char cave_row[MAX_WID+1];

	start_section_read("cave_memory");

	/* Memory dimensions */
	max_y = read_int("max_height");
	max_x = read_int("max_width");

	for (y = 0; y < max_y; y++)
	{
		read_binary("row",cave_row,MAX_WID);
		for(x = 0; x < max_x; x++)
		{
			p_ptr->cave_flag[y][x] = cave_row[x];
		}			
	}

	end_section_read("cave_memory");

	/* Success */
	return (0);
}

/* XXX XXX XXX 
 * This function parses savefile as if it was a text file, searching for
 * "pass =" string. It ignores the 'xml' format for sake
 * of maintance simplicity (i.e. it doesn't care about savefile format
 * changes). It attempts to read out the stored password, and compares it
 * to the password provided in "pass_word". If it matches, the hashed
 * password stored back onto the "pass_word" buff, which is assumed to be
 * of MAX_CHARS length.
 *
 * Returns 0 on match, -1 on parsing error and -2 if password do not
 * match.
 *
 * See "scoop_player" in "save.c" for more info.
 */
errr rd_savefile_new_scoop_aux(char *sfile, char *pass_word)
{
	errr err;

	char pass[80];
	char temp[80];
	char temp2[80];
	
	char *read;

	bool read_pass = FALSE;

	char buf[1024];

	/* The savefile is a text file */
	file_handle = my_fopen(sfile, "r");

	/* Paranoia */
	if (!file_handle) return (-1);

	/* Try to fetch the data */
	while (fgets(buf, 1024, file_handle))
	{
		read = strtok(buf, " \t=");
		if (!strcmp(read, "pass"))
		{
			read = strtok(NULL, " \t\n=");
			my_strcpy(pass, read, 80);
			read_pass = TRUE;
			continue;
		}
		if (read_pass) break;
	}

	/* Paranoia */
	temp[0] = '\0';
	
	/* Here's where we do our password encryption handling */
	my_strcpy(temp, (const char *)pass, 80);
	MD5Password(temp); /* The hashed version of our stored password */
	my_strcpy(temp2, (const char *)pass_word, 80);
	MD5Password(temp2); /* The hashed version of password from client */

	err = 0;

	if (strstr(pass, "$1$"))
	{ /* Most likely an MD5 hashed password saved */
		if (strcmp(pass, pass_word))
		{ /* No match, might be clear text from client */
			if (strcmp(pass, temp2))
			{
				/* No, it's not correct */
				err = -2;
			}
			/* Old style client, but OK otherwise */
		}
	}
	else
	{ /* Most likely clear text password saved */
		if (strstr(pass_word, "$1$"))
		{ /* Most likely hashed password from new client */
			if (strcmp(temp, pass_word))
			{
				/* No, it doesn't match hatched */
				err = -2;
			}
		}
		else
		{ /* Most likely clear text from client as well */
			if (strcmp(pass, pass_word))
			{
				/* No, it's not correct */
				err = -2;
			}
		}
		/* Good match with clear text, save the hashed */
		my_strcpy(pass_word, (const char *)temp, MAX_CHARS);
	}

	/* Check for errors */
	if (ferror(file_handle)) err = -1;

	/* Close the file */
	my_fclose(file_handle);

	/* Result */
	return (err);
}

/*
 * Actually read the savefile
 *
 */
static errr rd_savefile_new_aux(player_type *p_ptr)
{
	int i;

	u16b tmp16u;
	u32b tmp32u;
	bool clear = FALSE;
	bool had_header = FALSE;
	char stat_order_hack[6];

	if (!section_exists("mangband_player_save"))
	{
		return (-1); /* Horrible corruption */
	}

	start_section_read("mangband_player_save");
	if (!section_exists("version"))
	{
		return (-1); /* Horrible corruption */
	}
	start_section_read("version");
	read_int("major"); 
	read_int("minor");
	read_int("patch");
	end_section_read("version");
	
	if (section_exists("header")) 
	{
		start_section_read("header");
		had_header = TRUE;

		read_str("playername",p_ptr->name); /* 32 */

		skip_value("pass");

		p_ptr->prace = read_int("prace");
		p_ptr->pclass = read_int("pclass");
		p_ptr->male = read_int("male");

		read_binary("stat_order", stat_order_hack, 6);
		for (i = 0; i < 6; i++)
			p_ptr->stat_order[i] = stat_order_hack[i];

		end_section_read("header");
	}

	/* Operating system info */
	sf_xtra = read_uint("sf_xtra");

	/* Time of savefile creation */
	sf_when = read_uint("sf_when");

	/* Number of resurrections */
	sf_lives = read_int("sf_lives");

	/* Number of times played */
	sf_saves = read_int("sf_saves");

	/* Skip the turn info - if present */
	read_hturn("turn", &p_ptr->last_turn);
	
	/* Turn this character was born on */
	if(value_exists("birth_turn"))
		read_hturn("birth_turn", &p_ptr->birth_turn);
	else
		/* Disable character event logging if no birth turn */
		ht_clr(&p_ptr->birth_turn);

	/* Player turns (actually time spent playing) */
	if(value_exists("player_turn"))
		read_hturn("player_turn", &p_ptr->turn);
	else
		ht_clr(&p_ptr->turn);

	/* Read birth options */
	if (rd_birthoptions(p_ptr))
	{
		return (28);
	}

	/* Monster Memory */
	if (section_exists("monster_lore")) {
	start_section_read("monster_lore");
	tmp16u = read_int("max_r_idx");

	/* Incompatible save files */
	if (tmp16u > z_info->r_max)
	{
		note(format("Too many (%u) monster races!", tmp16u));
		return (21);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read the lore */
		rd_lore(p_ptr, i);
	}
	end_section_read("monster_lore");
	}
	
	/* Object Memory */
	start_section_read("object_memory");
	tmp16u = read_int("max_k_idx");

	/* Incompatible save files */
	if (tmp16u > z_info->k_max)
	{
		note(format("Too many (%u) object kinds!", tmp16u));
		return (22);
	}

	/* Read the object memory */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;

		tmp8u = read_int("flags");

		p_ptr->obj_aware[i] = (tmp8u & 0x01) ? TRUE : FALSE;
		p_ptr->obj_tried[i] = (tmp8u & 0x02) ? TRUE : FALSE;
	}
	end_section_read("object_memory");

	/*if (arg_fiddle) note("Loaded Object Memory");*/

	/* Read the extra stuff */
	rd_extra(p_ptr, had_header);

	/*if (arg_fiddle) note("Loaded extra information");*/


	/* Read the player_hp array */
	start_section_read("hp");
	tmp16u = read_int("py_max_level");

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
	{
		p_ptr->player_hp[i] = read_int("hp");
	}
	end_section_read("hp");


	/* Important -- Initialize the race/class */
	p_ptr->rp_ptr = &p_info[p_ptr->prace];
	p_ptr->cp_ptr = &c_info[p_ptr->pclass];
	

	/* Important -- Choose the magic info */
	p_ptr->mp_ptr = &c_info[p_ptr->pclass].spells;


	/* Read spell info */
	if (section_exists("spell_flags"))
	{
		start_section_read("spell_flags");
		for (i = 0; i < PY_MAX_SPELLS; i++)
		{
			p_ptr->spell_flags[i] = read_int("flag");
		}
		end_section_read("spell_flags");
	}
	else
	{
		/* Port spell flags from old format */
		u32b spell_learned1, spell_learned2;
		u32b spell_worked1, spell_worked2;
		u32b spell_forgotten1, spell_forgotten2;
		spell_learned1 = read_uint("spell_learned1");
		spell_learned2 = read_uint("spell_learned2");
		spell_worked1 = read_uint("spell_worked1");
		spell_worked2 = read_uint("spell_worked2");
		spell_forgotten1 = read_uint("spell_forgotten1");
		spell_forgotten2 = read_uint("spell_forgotten2");
		for (i = 0; i < PY_MAX_SPELLS; i++)
		{
			if ((i < 32) ?
				(spell_forgotten1 & (1L << i)) :
				(spell_forgotten2 & (1L << (i - 32))))
			{
				p_ptr->spell_flags[i] |= PY_SPELL_FORGOTTEN;
			} 
			if ((i < 32) ?
				(spell_learned1 & (1L << i)) :
				(spell_learned2 & (1L << (i - 32))))
			{
				p_ptr->spell_flags[i] |= PY_SPELL_LEARNED;
			}
			if ((i < 32) ?
				(spell_worked1 & (1L << i)) :
				(spell_worked2 & (1L << (i - 32))))
			{
				p_ptr->spell_flags[i] |= PY_SPELL_WORKED;
			}			
		}
	}

	start_section_read("spell_order");
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		p_ptr->spell_order[i] = read_int("order");
	}
	end_section_read("spell_order");

	/* Read the inventory */
	if (rd_inventory(p_ptr))
	{
		/*note("Unable to read inventory");*/
		return (21);
	}

	/* Read hostility information if new enough */
	if (rd_hostilities(p_ptr))
	{
		return (22);
	}
	rd_cave_memory(p_ptr);
	
	/* read the wilderness map */
	start_section_read("wilderness");
	/* get the map size */
	tmp32u = read_int("max_wild");
		
	/* if too many map entries */
	if (tmp32u > MAX_WILD)
	{
		return 23;
	}
		
	/* read in the map */
	for (i = 0; i < tmp32u; i++)
	{
		p_ptr->wild_map[i] = read_int("wild_map");
	}
	end_section_read("wilderness");
	
	/* Read the character event history */
	if(section_exists("event_history"))
	{
		char buf[160];
		cptr msg;
		history_event evt;
		history_event *last = NULL;
		start_section_read("event_history");
		while(value_exists("hist"))
		{
			int depth, level;
			history_event *n_evt = NULL;
			read_str("hist", buf);
			if (sscanf(buf, "%02i:%02i:%02i   %4ift   %2i   ", &evt.days, &evt.hours, &evt.mins,
				&depth, &level) == 5)
			{
				msg = &buf[25];/* skip 25 characters ^ */
				evt.depth = depth / 50;
				evt.message = quark_add(msg);
			}
			/* Allocate */
			MAKE(n_evt, history_event);
			n_evt->days = evt.days; n_evt->hours = evt.hours; n_evt->mins = evt.mins;
			n_evt->depth = evt.depth; n_evt->level = level;
			n_evt->message = evt.message;
			/* Add to chain */
			if (!last)
			{
				p_ptr->charhist = n_evt;
				last = n_evt;
			}
			else
			{
				last->next = n_evt;
				last = n_evt;
			}
		}
		end_section_read("event_history");
	}

	/* Read the characters quest list */
	if(section_exists("quests"))
	{
		start_section_read("quests");
		tmp16u = read_int("max_q_idx");
		for(i = 0; i < MAX_Q_IDX; i++)
		{
			tmp16u = read_int("level");
			p_ptr->q_list[i].level = tmp16u;
		}
		end_section_read("quests");
	}

	/* Read the characters sold artifact list */
	if(section_exists("found_artifacts"))
	{
		start_section_read("found_artifacts");
		tmp16u = read_int("max_a_idx");
		tmp32u = tmp16u;
		/* If we have an unexpected number of arts, just reset our list
		 * of sold artifacts. It's not so important we want to break
		 * save file compatability for it. */
		if( tmp16u != z_info->a_max )
		{
			clear = TRUE;
			tmp16u = 0;
		}
		for(i = 0; i < z_info->a_max; i++)
		{
			if(i < tmp32u)
			{
				if(!clear) tmp16u = read_int("a_info");
			}
			p_ptr->a_info[i] = tmp16u;
		}
		end_section_read("found_artifacts");
	}

	/* Hack -- no ghosts */
	/* r_info[z_info->r_max - 1].max_num = 0; */

  end_section_read("mangband_player_save");
  
	/* Success */
	return (0);
}


/*
 * Actually read the savefile
 *
 * Angband 2.8.0 will completely replace this code, see "save.c",
 * though this code will be kept to read pre-2.8.0 savefiles.
 */
errr rd_savefile_new(player_type *p_ptr)
{
	errr err;

	/* The savefile is a text file */
	file_handle = my_fopen(p_ptr->savefile, "r");

	/* Paranoia */
	if (!file_handle) return (-1);

	/* Call the sub-function */
	err = rd_savefile_new_aux(p_ptr);

	/* Check for errors */
	if (ferror(file_handle)) err = -1;

	/* Close the file */
	my_fclose(file_handle);

	/* Result */
	return (err);
}

errr rd_server_savefile()
{
        int i;

	errr err = 0;

	char savefile[1024];

	byte tmp8u;
        u16b tmp16u;
        u32b tmp32u;
	s32b tmp32s;
	int major;
	char name[80];

	/* Savefile name */
	path_build(savefile, 1024, ANGBAND_DIR_SAVE, "server");

	/* The server savefile is a binary file */
	file_handle = my_fopen(savefile, "r");
	line_counter = 0;

	start_section_read("mangband_server_save");
	start_section_read("version");
	major = read_int("major"); 
	major = read_int("minor");
	major = read_int("patch");
	end_section_read("version");

	/* Paranoia */
	if (!file_handle) return (-1);

        /* Clear the checksums */
        v_check = 0L;
        x_check = 0L;

        /* Operating system info */
		sf_xtra = read_uint("xtra");

        /* Time of savefile creation */
		sf_when = read_uint("timestamp");

        /* Number of lives */
		sf_lives = read_int("sf_lives");

        /* Number of times played */
		sf_saves = read_int("sf_saves");

        /* Monster Memory */
		start_section_read("monster_lore");

		tmp16u = read_int("max_r_idx");

        /* Incompatible save files */
        if (tmp16u > z_info->r_max)
        {
                note(format("Too many (%u) monster races!", tmp16u));
                return (21);
        }

        /* Read the available records */
        for (i = 0; i < tmp16u; i++)
        {
		monster_race *r_ptr;

                /* Read the lore */
               rd_u_lore(i);

		/* Access the monster race */
		r_ptr = &r_info[i];

        }

		end_section_read("monster_lore");
		
        /* Load the Artifacts */
		start_section_read("artifacts");
		tmp16u = read_int("max_a_idx");

        /* Incompatible save files */
        if (tmp16u > z_info->a_max)
        {
                note(format("Too many (%u) artifacts!", tmp16u));
                return (24);
        }

        /* Read the artifact flags */
        for (i = 0; i < tmp16u; i++)
        {
				tmp8u = read_int("artifact");
                a_info[i].cur_num = tmp8u;
        }
		end_section_read("artifacts");

	/* Read the stores */
	start_section_read("stores");
	tmp16u = read_int("max_stores");
	for (i = 0; i < tmp16u; i++)
	{
		if (rd_store(i)) return (22);
	}
	end_section_read("stores");

	/* Read party info if savefile is new enough */
		start_section_read("parties");
		tmp16u = read_int("max_parties");
		
		/* Incompatible save files */
		if (tmp16u > MAX_PARTIES)
		{
			note(format("Too many (%u) parties!", tmp16u));
			return (25);
		}

		/* Read the available records */
		for (i = 0; i < tmp16u; i++)
		{
			rd_party(i);
		}
		end_section_read("parties");

	/* XXX If new enough, read in the saved levels and monsters. */

		start_section_read("dungeon_levels");
		/* read the number of levels to be loaded */
		tmp32u = read_uint("num_levels");
		/* load the levels */
		for (i = 0; i < tmp32u; i++) rd_dungeon(FALSE, 0);
		/* load any special static levels */
		rd_dungeon_special();
		end_section_read("dungeon_levels");

		start_section_read("monsters");
		/* get the number of monsters to be loaded */
		tmp32u = read_int("max_monsters");
		if (tmp32u > MAX_M_IDX)
		{
			note(format("Too many (%u) monsters!", tmp16u));
			return (29);
		}
		/* load the monsters */
		for (i = 1; i < tmp32u; i++)
		{
			rd_monster(&m_list[m_pop()]);
		}
		end_section_read("monsters");

		/* Read object info */
		start_section_read("objects");
		tmp16u = read_int("max_objects");

		/* Incompatible save files */
		if (tmp16u > MAX_O_IDX)
		{
			note(format("Too many (%u) objects!", tmp16u));
			return (26);
		}

		/* Read the available records */
		for (i = 1; i < tmp16u; i++)
		{		
			rd_item(&o_list[i]);
		}

		/* Set the maximum object number */
		o_max = tmp16u;
		end_section_read("objects");

		/* Read holding info */
		/* Reacquire objects */
		for (i = 1; i < o_max; ++i)
		{
			object_type *o_ptr;
			monster_type *m_ptr;
	
			/* Get the object */
			o_ptr = &o_list[i];
	
			/* Ignore dungeon objects */
			if (!o_ptr->held_m_idx) continue;
	
			/* Verify monster index */
			if (o_ptr->held_m_idx > z_info->m_max)
			{
				note("Invalid monster index");
				return (-1);
			}
	
			/* Get the monster */
			m_ptr = &m_list[o_ptr->held_m_idx];
	
			/* Link the object to the pile */
			o_ptr->next_o_idx = m_ptr->hold_o_idx;
	
			/* Link the monster to the object */
			m_ptr->hold_o_idx = i;
		}
	
		/* Read house info */
		start_section_read("houses");
		tmp16u = read_int("num_houses");

		/* Incompatible save files */
		if (tmp16u > MAX_HOUSES)
		{
			note(format("Too many (%u) houses!", tmp16u));
			return (27);
		}

		/* Read the available records */
		for (i = 0; i < tmp16u; i++)
		{
			rd_house(i);
		}
		num_houses = tmp16u;
		end_section_read("houses");

		/* Read arenas info */
		if (section_exists("arenas")) 
		{
			start_section_read("arenas");
			tmp16u = read_int("num_arenas");
	
			/* Incompatible save files */
			if (tmp16u > MAX_ARENAS)
			{
				note(format("Too many (%u) arenas!", tmp16u));
				return (27);
			}
	
			/* Read the available records */
			for (i = 0; i < tmp16u; i++)
			{
				rd_arena(i);
			}
			num_arenas = tmp16u;
			end_section_read("arenas");
		}

		/* Read wilderness info */
		start_section_read("wilderness");
		/* read how many wilderness levels */
		tmp32u = read_int("max_wild");
				
		if (tmp32u > MAX_WILD)
		{
			note("Too many wilderness levels");
			return 28;
		}
	
		for (i = 1; i < tmp32u; i++)
		{
			rd_wild(i);
		}	
		end_section_read("wilderness");

		/* Read the player name database  */
		start_section_read("player_names");

		tmp32u = read_int("num_players");

		/* Read the available records */
		for (i = 0; i < tmp32u; i++)
		{
			start_section_read("player");
			/* Read the ID */
			tmp32s = read_int("id");

			/* Read the player name */
			read_str("name",name);

			/* Store the player name */
			add_player_name(name, tmp32s);
			end_section_read("player");
		}
		end_section_read("player_names");

	seed_flavor = read_uint("seed_flavor");
	seed_town = read_uint("seed_town");

	player_id = read_int("player_id");

	read_hturn("turn", &turn);

        /* Hack -- no ghosts */
        /*r_info[z_info->r_max - 1].max_num = 0;*/

	end_section_read("mangband_server_save");

	/* Check for errors */
	if (ferror(file_handle)) err = -1;

	/* Close the file */
	my_fclose(file_handle);

	/* Result */
	return (err);
}

