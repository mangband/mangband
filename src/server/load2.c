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
static ang_file* file_handle; 
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
bool start_section_read(char* name)
{
	char seek_section[80];
	char got_section[80];
	bool matched = FALSE;
	
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	return (TRUE);
}

/* End a section */
bool end_section_read(char* name)
{
	char seek_section[80];
	char got_section[80];
	bool matched = FALSE;
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	return (TRUE);
}

/* Read a puny byte */
bool read_byte(char* name, byte *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	byte value;
#ifndef SCNu8
	u16b larger_value;
#endif
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
	{
		line_counter++;
#ifndef SCNu8
		if(sscanf(file_buf,"%s = %" SCNu16, seek_name,&larger_value) == 2)
#else
		if(sscanf(file_buf,"%s = %" SCNu8, seek_name,&value) == 2)
#endif
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{
		plog(format("Missing integer.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		return (FALSE);
	}
#ifndef SCNu8
	if (larger_value > 255)
	{
		plog(format("Integer overflow.  Expected '%s' <= 255, found '%u' at line %i",name,larger_value,line_counter));
		return (FALSE);
	}
	*dst = (byte)larger_value;
#else
	*dst = value;
#endif
	return (TRUE);
}


/* Read a short integer */
bool read_short(char* name, s16b *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	s16b value;
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
	{
		line_counter++;
		if(sscanf(file_buf,"%s = %" SCNu16, seek_name,&value) == 2)
		{
			matched = !strcmp(seek_name,name);
		}
	}
	if(!matched)
	{
		plog(format("Missing integer.  Expected '%s', found '%s' at line %i",name,file_buf,line_counter));
		return (FALSE);
	}
	*dst = value;
	return (TRUE);
}

/* Read an integer */
bool read_int(char* name, int *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	int value;
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	*dst = value;
	return (TRUE);
}

/* Read an unsigned integer */
bool read_uint(const char* name, uint *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	uint value;
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	*dst = value;
	return (TRUE);
}

/* Read a 'huge' */
bool read_huge(char* name, huge *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	huge value;
		
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	*dst = value;
	return (TRUE);
}

/* Read an hturn */
bool read_hturn(char* name, hturn *value)
{
	char seek_name[80];	
	bool matched = FALSE;
	s64b era, turn;

	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	
	value->era = era;
	value->turn = turn;
	return (TRUE);
}

/* Read a string */
/* Returns TRUE if the string could be read */
bool read_str(char* name, char* value)
{
	char seek_name[80];
	bool matched = FALSE;
	char *c;
	
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return FALSE;
	}

	c = file_buf;	
	while(*c != '=') c++;
	c+=2;

	while( *c >= 31 )
	{
		*value = *c; c++; value++;
	}
	*value = '\0';
	return (TRUE);
}

/* Read a quark */
/* Tiny wrapper around read_str */
bool read_quark(char* name, u16b *value)
{
	char note[80];
	/* Read string into temp buffer. */
	if (!read_str(name, note))
	{
		return (FALSE);
	}
	if (STRZERO(note))
	{
		*value = 0;
		return (TRUE);
	}
	/* And create a quark from it */
	*value = quark_add(note);
	return (TRUE);
}

/* Read a float */
/* Returns TRUE if the float could be read */
bool read_float(char* name, float *dst)
{
	char seek_name[80];
	bool matched = FALSE;
	float value;
	
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
	}
	*dst = value;
	return (TRUE);
}

/* Read some binary data */
bool read_binary(char* name, char* value, int max_len)
{
	char seek_name[80];
	bool matched = FALSE;
	char hex[3];
	char *c;
	char *bin;
	unsigned int abyte;
	hex[2] = '\0';

	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
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
		return (FALSE);
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
	return (TRUE);
}

/* Skip a named value */
void skip_value(char* name)
{
	char seek_name[80];
	long fpos;
	
	/* Remember where we are incase there is nothing to skip */
	fpos = file_tell(file_handle);
	sprintf(seek_name,"%s = ",name);
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
	{
		line_counter++;
		if(strstr(file_buf,seek_name) == NULL)
		{
			/* Move back on seek failures */
			file_seek(file_handle, fpos);
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
	fpos = file_tell(file_handle);
	sprintf(seek_name,"%s = ",name);
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
	{
		matched = TRUE;
		if(strstr(file_buf,seek_name) == NULL)
		{
			matched = FALSE;
		}
	}
	/* Move back */
	file_seek(file_handle, fpos);
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
	fpos = file_tell(file_handle);
	if (file_getl(file_handle, file_buf, sizeof(file_buf)-1))
	{
		sprintf(seek_section,"<%s>",name);
		if(sscanf(file_buf,"%s",got_section) == 1)
		{
			matched = !strcmp(got_section,seek_section);
		}
	}
	/* Move back */
	file_seek(file_handle, fpos);
	return(matched);
}

/*
 * Show information on the screen, one line at a time.
 * Start at line 2, and wrap, if needed, back to line 2.
 */
static void note(cptr msg)
{
	plog(msg);
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
static bool rd_item(object_type *o_ptr)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }
	byte old_dd;
	byte old_ds;

	u32b f1, f2, f3;

	object_kind *k_ptr;

	char note[128];

	__try( start_section_read("item") );
	
	/* Hack -- wipe */
	WIPE(o_ptr, object_type);

	/* Skip name */
	skip_value("name");

	/* Kind */
	__try( read_short("k_idx", &o_ptr->k_idx) );

	/* Location */
	__try( read_byte("iy", &o_ptr->iy) );
	__try( read_byte("ix", &o_ptr->ix) );
	
	__try( read_short("dun_depth", &o_ptr->dun_depth) );

	/* Type/Subtype */
	__try( read_byte("tval", &o_ptr->tval) );
	__try( read_byte("sval", &o_ptr->sval) );

	/* Base pval */
	__try( read_int("bpval", &o_ptr->bpval) );

	/* Special pval */
	__try( read_int("pval", &o_ptr->pval) );


	__try( read_byte("discount", &o_ptr->discount) );
	__try( read_byte("number", &o_ptr->number) );
	__try( read_short("weight", &o_ptr->weight) );

	__try( read_byte("name1", &o_ptr->name1) );
	__try( read_byte("name2", &o_ptr->name2) );
	__try( read_int("name3", &o_ptr->name3) );
	__try( read_short("timeout", &o_ptr->timeout) );

	__try( read_short("to_h", &o_ptr->to_h) );
	__try( read_short("to_d", &o_ptr->to_d) );
	__try( read_short("to_a", &o_ptr->to_a) );

	__try( read_short("ac", &o_ptr->ac) );

	__try( read_byte("dd", &old_dd) );
	__try( read_byte("ds", &old_ds) );

	__try( read_byte("ident", &o_ptr->ident) );

	/* Special powers */
	__try( read_byte("xtra1", &o_ptr->xtra1) );
	__try( read_byte("xtra2", &o_ptr->xtra2) );

	/* Inscription */
	__try( read_quark("inscription", &o_ptr->note) );
 
	/* Owner information */
	if (value_exists("owner_name"))
	{
		__try( read_quark("owner_name", &o_ptr->owner_name) );
		__try( read_int("owner_id", &o_ptr->owner_id) );
	}

	/* Monster holding object */ 
	__try( read_short("held_m_idx", &o_ptr->held_m_idx) );

	/* Origin */
	if (value_exists("origin"))
	{
		__try( read_byte("origin", &o_ptr->origin) );
		__try( read_byte("origin_depth", &o_ptr->origin_depth) );
		__try( read_short("origin_xtra", &o_ptr->origin_xtra) );
		__try( read_quark("origin_player", &o_ptr->origin_player) );
		if (o_ptr->origin_xtra >= z_info->r_max) { o_ptr->origin_xtra = 0; o_ptr->origin = ORIGIN_NONE; }
		if (o_ptr->origin >= ORIGIN_MAX) o_ptr->origin = ORIGIN_NONE;
	}

	__try( end_section_read("item") );

	/* Mega-Hack -- handle "dungeon objects" later */
	if ((o_ptr->k_idx >= 445) && (o_ptr->k_idx <= 479)) return (TRUE);


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
		return (TRUE);
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
		artifact_type *a_ptr = artifact_ptr(o_ptr);

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
	return (TRUE);
}


/*
 * Read a monster
 */

static bool rd_monster(monster_type *m_ptr)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	__try( start_section_read("monster") );

	/* Hack -- wipe */
	WIPE(m_ptr, monster_type);

	skip_value("name");

	/* Read the monster race */
	__try( read_short("r_idx", &m_ptr->r_idx) );

	/* Read the other information */
	__try( read_byte("fy", &m_ptr->fy) );
	__try( read_byte("fx", &m_ptr->fx) );
	__try( read_short("dun_depth", &m_ptr->dun_depth) );
	__try( read_short("hp", &m_ptr->hp) );
	__try( read_short("maxhp", &m_ptr->maxhp) );
	__try( read_short("csleep", &m_ptr->csleep) );
	__try( read_byte("mspeed", &m_ptr->mspeed) );
	__try( read_huge("energy", &m_ptr->energy) );
	__try( read_byte("stunned", &m_ptr->stunned) );
	__try( read_byte("confused", &m_ptr->confused) );
	__try( read_byte("afraid", &m_ptr->monfear) );

	__try( end_section_read("monster") );

	return (TRUE);
}





/*
 * Read the monster lore
 */
static bool rd_lore(player_type *p_ptr, int r_idx)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	int i;
	
	monster_lore *l_ptr = p_ptr->l_list + r_idx;

	__try( start_section_read("lore") );

	/* Count sights/deaths/kills */
	__try( read_short("sights", &l_ptr->sights) );
	__try( read_short("deaths", &l_ptr->deaths) );
	__try( read_short("pkills", &l_ptr->pkills) );
	__try( read_short("tkills", &l_ptr->tkills) );

	/* Count wakes and ignores */
	__try( read_byte("wake", &l_ptr->wake) );
	__try( read_byte("ignore", &l_ptr->ignore) );

	/* Count drops */
	__try( read_byte("drop_gold", &l_ptr->drop_gold) );
	__try( read_byte("drop_item", &l_ptr->drop_item) );

	/* Count spells */
	__try( read_byte("cast_innate", &l_ptr->cast_innate) );
	__try( read_byte("cast_spell", &l_ptr->cast_spell) );

	/* Count blows of each type */
	__try( start_section_read("blows") );
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
	{
		__try( read_byte("blow", &l_ptr->blows[i]) );
	}
	__try( end_section_read("blows") );


	/* Memorize flags */
	__try( start_section_read("flags") );
	__try( read_int("flag", &l_ptr->flags1) );
	__try( read_int("flag", &l_ptr->flags2) );
	__try( read_int("flag", &l_ptr->flags3) );
	__try( read_int("flag", &l_ptr->flags4) );
	__try( read_int("flag", &l_ptr->flags6) );
	__try( read_int("flag", &l_ptr->flags6) );
	__try( end_section_read("flags") );

	/* Repair the lore flags */
	/* No need to repair AFAIU
	l_ptr->flags1 &= r_ptr->flags1;
	l_ptr->flags2 &= r_ptr->flags2;
	l_ptr->flags3 &= r_ptr->flags3;
	l_ptr->flags4 &= r_ptr->flags4;
	l_ptr->flags5 &= r_ptr->flags5;
	l_ptr->flags6 &= r_ptr->flags6;
	*/
	__try( end_section_read("lore") );

	return (TRUE);
}

/*
 * Read the uniques lore
 */
static bool rd_u_lore(int r_idx)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	monster_race *r_ptr = &r_info[r_idx];

	__try( start_section_read("lore") );

	skip_value("name");

	/* Count sights/deaths/kills */
	__try( read_short("sights", &r_ptr->r_sights) );
	if (value_exists("deaths")) skip_value("deaths");
	if (value_exists("pkills")) skip_value("pkills");
	__try( read_short("tkills", &r_ptr->r_tkills) );
	
	if (value_exists("wake")) skip_value("wake");
	if (value_exists("ignore")) skip_value("ignore");
	if (value_exists("respawn_timer")) skip_value("respawn_timer");
	if (value_exists("drop_gold")) skip_value("drop_gold");
	if (value_exists("drop_item")) skip_value("drop_item");
	if (value_exists("cast_innate")) skip_value("cast_innate");
	if (value_exists("cast_spell")) skip_value("cast_spell");

	if (section_exists("blows"))
	{
		__try( start_section_read("blows") );
		while(value_exists("blow")) skip_value("blow");
		__try( end_section_read("blows") );
	}
	if (section_exists("flags"))
	{
		__try( start_section_read("flags") );
		while(value_exists("flag")) skip_value("flag");
		__try( end_section_read("flags") );
	}
	
	/* Read the "Racial" monster limit per level */
	__try( read_byte("max_num", &r_ptr->max_num) );

	if (value_exists("killer")) skip_value("killer");
	
	__try( end_section_read("lore") );

	return (TRUE);
}




/*
 * Read a store
 */
static bool rd_store(int n)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	store_type *st_ptr = &store[n];

	int j;

	byte own, num;

	__try( start_section_read("store") );

	/* Read the basic info */
	__try( read_hturn("store_open", &st_ptr->store_open) );
	__try( read_short("insult_cur", &st_ptr->insult_cur) );
	__try( read_byte("owner", &own) );
	__try( read_byte("stock_num", &num) );
	__try( read_short("good_buy", &st_ptr->good_buy) );
	__try( read_short("bad_buy", &st_ptr->bad_buy) );

	/* Extract the owner (see above) */
	st_ptr->owner = own;

	__try( start_section_read("stock") );
	/* Read the items */
	for (j = 0; j < num; j++)
	{
		object_type forge;

		/* Read the item */
		__try( rd_item(&forge) );

		/* Acquire valid items */
		if (st_ptr->stock_num < STORE_INVEN_MAX)
		{
			/* Acquire the item */
			st_ptr->stock[st_ptr->stock_num++] = forge;
		}
	}
	__try( end_section_read("stock") );
	__try( end_section_read("store") );

	/* Success */
	return (TRUE);
}


/*
 * Read some party info
 
 FOUND THE BUIG!!!!!! the disapearing party bug. no more.
 I hope......
-APD-
 */
static bool rd_party(int n)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	party_type *party_ptr = &parties[n];

	__try( start_section_read("party") );

	/* Party name */
	__try( read_str("name",party_ptr->name) );

	/* Party owner's name */
	__try( read_str("owner",party_ptr->owner) );

	/* Number of people and creation time */
	__try( read_int("num", &party_ptr->num) );
	__try( read_hturn("created", &party_ptr->created) );

	__try( end_section_read("party") );

	return (TRUE);
}

/*
 * Read some house info
 */
static bool rd_house(int n)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	house_type *house_ptr = &houses[n];

	__try( start_section_read("house") );

	/* coordinates of corners of house */
	__try( read_byte("x1", &house_ptr->x_1) );
	__try( read_byte("y1", &house_ptr->y_1) );
	__try( read_byte("x2", &house_ptr->x_2) );
	__try( read_byte("y2", &house_ptr->y_2) );
	
	/* coordinates of the house door */
	__try( read_byte("door_y", &house_ptr->door_y) );
	__try( read_byte("door_x", &house_ptr->door_x) );

	/* Door Strength */
	__try( read_byte("strength", &house_ptr->strength) );

	/* Owned or not */
	__try( read_str("owned", house_ptr->owned) );

	__try( read_int("depth", &house_ptr->depth) );
	__try( read_int("price", &house_ptr->price) );

	__try( end_section_read("house") );

	return (TRUE);
}

/*
 * Read some arena info
 */
static bool rd_arena(int n)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	arena_type *arena_ptr = &arenas[n];

	__try( start_section_read("arena") );

	/* coordinates of corners of house */
	__try( read_byte("x1", &arena_ptr->x_1) );
	__try( read_byte("y1", &arena_ptr->y_1) );
	__try( read_byte("x2", &arena_ptr->x_2) );
	__try( read_byte("y2", &arena_ptr->y_2) );
	
	__try( read_int("depth", &arena_ptr->depth) );

	__try( end_section_read("arena") );

	return (TRUE);
}

static bool rd_wild(int n)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	wilderness_type *w_ptr = &wild_info[-n];
	
	/* the flags */
	__try( read_short("flags", &w_ptr->flags) );

	return (TRUE);
}


/*
 * Read/Write the "extra" information
 */

static bool rd_extra(player_type *p_ptr, bool had_header)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	int i = 0;

	__try( start_section_read("player") );

	if (!had_header)
	{
		__try( read_str("playername",p_ptr->name) ); /* 32 */
		skip_value("pass");
	}

	__try( read_str("died_from",p_ptr->died_from) ); /* 80 */

	__try( read_str("died_from_list",p_ptr->died_from_list) ); /* 80 */
	__try( read_short("died_from_depth", &p_ptr->died_from_depth) );

	__try( start_section_read("history") );
	for (i = 0; i < 4; i++)
	{
		__try( read_str("history",p_ptr->history[i]) ); /* 60 */
	}
	if (value_exists("descrip"))
	{
		__try( read_str("descrip",p_ptr->descrip) ); /* 240?! */
	}
	__try( end_section_read("history") );

	/* Class/Race/Gender/Party */
	if (!had_header)
	{
		__try( read_byte("prace", &p_ptr->prace) );
		__try( read_byte("pclass", &p_ptr->pclass) );
		__try( read_byte("male", &p_ptr->male) );
	}
	__try( read_byte("party", &p_ptr->party) );

	/* Special Race/Class info */
	__try( read_byte("hitdie", &p_ptr->hitdie) );
	__try( read_short("expfact", &p_ptr->expfact) );

	/* Age/Height/Weight */
	__try( read_short("age", &p_ptr->age) );
	__try( read_short("ht", &p_ptr->ht) );
	__try( read_short("wt", &p_ptr->wt) );

	/* Read the stat info */
	__try( start_section_read("stats") );
	for (i = 0; i < A_MAX; i++)
	{
		__try( read_short("stat_max", &p_ptr->stat_max[i]) );
	}
	for (i = 0; i < A_MAX; i++)
	{
		__try( read_short("stat_cur", &p_ptr->stat_cur[i]) );
	}
	__try( end_section_read("stats") );

	__try( read_int("id", &p_ptr->id) );

	/* If he was created in the pre-ID days, give him one */
	if (!p_ptr->id)
		p_ptr->id = player_id++;

	__try( read_int("au", &p_ptr->au) );

	__try( read_int("max_exp", &p_ptr->max_exp) );
	__try( read_int("exp", &p_ptr->exp) );
	__try( read_short("exp_frac", &p_ptr->exp_frac) );

	__try( read_short("lev", &p_ptr->lev) );

	__try( read_short("mhp", &p_ptr->mhp) );
	__try( read_short("chp", &p_ptr->chp) );
	__try( read_short("chp_frac", &p_ptr->chp_frac) );

	__try( read_short("msp", &p_ptr->msp) );
	__try( read_short("csp", &p_ptr->csp) );
	__try( read_short("csp_frac", &p_ptr->csp_frac) );

	if(value_exists("no_ghost"))
	{
		u32b tmp32u;
		__try( read_int("no_ghost", &tmp32u) );
	}
	
	__try( read_short("max_plv", &p_ptr->max_plv) );
	__try( read_short("max_dlv", &p_ptr->max_dlv) );
	
	p_ptr->recall_depth = p_ptr->max_dlv;

	__try( read_short("py", &p_ptr->py) );
	__try( read_short("px", &p_ptr->px) );
	__try( read_short("dun_depth", &p_ptr->dun_depth) );

	__try( read_short("world_x", &p_ptr->world_x) );
	__try( read_short("world_y", &p_ptr->world_y) );

	/* More info */
	__try( read_short("ghost", &p_ptr->ghost) );
	__try( read_short("sc", &p_ptr->sc) );
	__try( read_short("fruit_bat", &p_ptr->fruit_bat) );

	/* Read the flags */
	__try( read_byte("lives", &p_ptr->lives) );

	/* hack */
	__try( read_short("blind", &p_ptr->blind) );
	__try( read_short("paralyzed", &p_ptr->paralyzed) );
	__try( read_short("confused", &p_ptr->confused) );
	__try( read_short("food", &p_ptr->food) );
	__try( read_uint("energy", &p_ptr->energy) );
	__try( read_short("fast", &p_ptr->fast) );
	__try( read_short("slow", &p_ptr->slow) );
	__try( read_short("afraid", &p_ptr->afraid) );
	__try( read_short("cut", &p_ptr->cut) );
	__try( read_short("stun", &p_ptr->stun) );
	__try( read_short("poisoned", &p_ptr->poisoned) );
	__try( read_short("image", &p_ptr->image) );
	__try( read_short("protevil", &p_ptr->protevil) );
	__try( read_short("invuln", &p_ptr->invuln) );
	__try( read_short("hero", &p_ptr->hero) );
	__try( read_short("shero", &p_ptr->shero) );
	__try( read_short("shield", &p_ptr->shield) );
	__try( read_short("blessed", &p_ptr->blessed) );
	__try( read_short("tim_invis", &p_ptr->tim_invis) );
	__try( read_short("word_recall", &p_ptr->word_recall) );
	__try( read_short("see_infra", &p_ptr->see_infra) );
	__try( read_short("tim_infra", &p_ptr->tim_infra) );

	__try( read_short("oppose_fire", &p_ptr->oppose_fire) );
	__try( read_short("oppose_cold", &p_ptr->oppose_cold) );
	__try( read_short("oppose_acid", &p_ptr->oppose_acid) );
	__try( read_short("oppose_elec", &p_ptr->oppose_elec) );
	__try( read_short("oppose_pois", &p_ptr->oppose_pois) );

	__try( read_byte("confusing", &p_ptr->confusing) );
	__try( read_byte("searching", &p_ptr->searching) );
	__try( read_byte("maximize", &p_ptr->maximize) );
	__try( read_byte("preserve", &p_ptr->preserve) );

	/* Read the unique list info */
	__try( start_section_read("uniques") );
	for (i = 0; i < z_info->r_max; i++)
	{
		__try( read_short("unique", &p_ptr->r_killed[i]) );
	}
	__try( end_section_read("uniques") );

	/* Special stuff */
	__try( read_short("panic_save", &panic_save) );
	__try( read_short("total_winner", &p_ptr->total_winner) );
	__try( read_short("retire_timer", &p_ptr->retire_timer) );
	__try( read_short("noscore", &p_ptr->noscore) );

	/* Read "death" */
	__try( read_byte("death", &p_ptr->death) );

	__try( end_section_read("player") );

	/* Success */
	return (TRUE);
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
#undef __try
#define __try(X) if (!(X)) { return (-1); }

	int slot = 0;

	object_type forge;

	__try( start_section_read("inventory") );

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
		__try( read_short("inv_entry", &n) );

		/* Nope, we reached the end */
		if (n == 0xFFFF) break;

		/* Read the item */
		__try( rd_item(&forge) );

		/* Hack -- verify item */
		if (!forge.k_idx) return (53);

		/* Mega-Hack -- Handle artifacts that aren't yet "created" */
		if (true_artifact_p(&forge))
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

	__try( end_section_read("inventory") );

	/* Success */
	return (0);
}

/*
 * Read the birth options
 */
static errr rd_birthoptions(player_type *p_ptr)
{
#undef __try
#define __try(X) if (!(X)) { return (-1); }

	s32b i, id;
	u16b tmp16u, ind;

	if (!section_exists("options"))
	{
		/* Fine, no options */
		return (0);
	}

	/* Begin */
	__try( start_section_read("options") );

	/* Read number */
	__try( read_short("num", &tmp16u) );

	/* HACK -- compat for new birth options */
	if (tmp16u == 2) tmp16u = 4;

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
			u32b val;
			__try( read_uint(opt_ptr->o_text, &val) );

			/* Set it */
			p_ptr->options[ind] = val ? TRUE : FALSE;
		}
		/* Hack -- compat for new birth options *//* REMOVE ME */
		else if (ind == OPT_ENERGY_BUILDUP || ind == OPT_MONSTER_RECOIL)
		{
			p_ptr->options[ind] = FALSE;
		}
		else
		{
			__try( end_section_read("options") );
			/* Unexpected option */
			return (29);
		}

		id++;
		/* Don't read anymore */
		if (id >= tmp16u) break;
	}

	/* Done */
	__try( end_section_read("options") );

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
#undef __try
#define __try(X) if (!(X)) { return (-1); }

	hostile_type *h_ptr;
	int i;
	s32b id;
	u16b tmp16u;

	__try( start_section_read("hostilities") );

	/* Read number */
	__try( read_short("num", &tmp16u) );

	/* Read available ID's */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read next ID */
		__try( read_int("id", &id) );

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

	__try( end_section_read("hostilities") );

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

static bool rd_dungeon(bool ext, int Depth)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	s32b depth;
	u16b max_y, max_x;

	int y, x;
	cave_type *c_ptr;
	char cave_row[MAX_WID+1];

	__try( start_section_read("dungeon_level") );

	/*** Depth info ***/

	/* Level info */
	__try( read_int("depth", &depth) );
	__try( read_short("max_height", &max_y) );
	__try( read_short("max_width", &max_x) );
	if (ext) depth = Depth;

	/* Turn this level was generated */
	if (value_exists("gen_turn"))
	{
		__try( read_hturn("gen_turn", &turn_cavegen[depth]) );
	}

	/* players on this depth */
	__try( read_short("players_on_depth", &players_on_depth[depth]) );

	/* Hack -- only read in staircase information for non-wilderness
	 * levels
	 */

	if (depth >= 0)
	{
		__try( read_byte("level_up_y", &level_up_y[depth]) );
		__try( read_byte("level_up_x", &level_up_x[depth]) );
		__try( read_byte("level_down_y", &level_down_y[depth]) );
		__try( read_byte("level_down_x", &level_down_x[depth]) );
		__try( read_byte("level_rand_y", &level_rand_y[depth]) );
		__try( read_byte("level_rand_x", &level_rand_x[depth]) );
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
	__try( start_section_read("features") );

		for (y = 0; y < max_y; y++)
		{
			__try( read_binary("row",cave_row,MAX_WID) );
			for(x = 0; x < max_x; x++)
			{
				/* Access the cave */
				c_ptr = &cave[depth][y][x];

				/* set the feature */
				c_ptr->feat = cave_row[x];
			}			
		}

	__try( end_section_read("features") );

	/* Load info */
	start_section_read("info");

		for (y = 0; y < max_y; y++)
		{
			__try( read_binary("row",cave_row,MAX_WID) );
			for(x = 0; x < max_x; x++)
			{
				/* Access the cave */
				c_ptr = &cave[depth][y][x];

				/* set the feature */
				c_ptr->info = cave_row[x];
			}			
		}

	__try( end_section_read("info") );

	__try( end_section_read("dungeon_level") );

	/* Success */
	return (TRUE);
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
#undef __try
#define __try(X) if (!(X)) { return (-1); }

	bool ok = FALSE;

	char filename[1024];
	char levelname[32];
	ang_file* fhandle;
	ang_file* server_handle;
	int i,num_levels,j=0,k=0;
	
	/* Clear all the special levels */
	for(i=0;i<MAX_SPECIAL_LEVELS;i++)
	{
		special_levels[i] = -999;
	}
	
	/* Vanilla Mangand doesn't have special static pre-designed levels */
	if ((!cfg_ironman) && (!cfg_more_towns)) return 0;
	
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
		fhandle = file_open(filename, MODE_READ, -1);
		if(fhandle)
		{
			/* swap out the main file pointer for our level file */
			server_handle = file_handle;
			file_handle = fhandle;
			/* load the level */
			ok = rd_dungeon(FALSE, 0);
			/* swap the file pointers back */
			file_handle = server_handle;
			/* close the level file */
			file_close(fhandle);
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
	return ok ? 0 : -1;
}

/* HACK -- Read from file */
bool rd_dungeon_special_ext(int Depth, cptr levelname)
{
	bool ok = FALSE;
	char filename[1024];
	ang_file* fhandle;
	ang_file* server_handle;
	
	path_build(filename, 1024, ANGBAND_DIR_SAVE, levelname);

	fhandle = file_open(filename, MODE_READ, -1);

	if (fhandle)
	{
			/* swap out the main file pointer for our level file */
			server_handle = file_handle;
			file_handle = fhandle;

			/* load the level */
			ok = rd_dungeon(TRUE, Depth);

			/* swap the file pointers back */
			file_handle = server_handle;

			/* close the level file */
			file_close(fhandle);
	}
	return ok;
}


/* Reads in a players memory of the level he is currently on, in run-length encoded
 * format.  Simmilar to the above function.
 */

static bool rd_cave_memory(player_type *p_ptr)
{
#undef __try
#define __try(X) if (!(X)) { return (FALSE); }

	u16b max_y, max_x;
	int y, x;
	char cave_row[MAX_WID+1];

	__try( start_section_read("cave_memory") );

	/* Memory dimensions */
	__try( read_short("max_height", &max_y) );
	__try( read_short("max_width", &max_x) );

	for (y = 0; y < max_y; y++)
	{
		__try( read_binary("row", cave_row, MAX_WID) );
		for(x = 0; x < max_x; x++)
		{
			p_ptr->cave_flag[y][x] = cave_row[x];
		}
	}

	__try( end_section_read("cave_memory") );

	/* Success */
	return (TRUE);
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
	file_handle = file_open(sfile, MODE_READ, -1);

	/* Paranoia */
	if (!file_handle) return (-1);

	/* Try to fetch the data */
	while (file_getl(file_handle, buf, 1024))
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
	if (file_error(file_handle)) err = -1;

	/* Close the file */
	file_close(file_handle);

	/* Result */
	return (err);
}

/*
 * Actually read the savefile
 *
 */
static errr rd_savefile_new_aux(player_type *p_ptr)
{
#undef __try
#define __try(X) if (!(X)) { return (-1); }
	int i;

	u16b tmp16u;
	u32b tmp32u;
	bool clear = FALSE;
	bool had_header = FALSE;
	char stat_order_hack[6];

	__try( start_section_read("mangband_player_save") );
	__try( start_section_read("version") );
	__try( read_int("major", &tmp32u) );
	__try( read_int("minor", &tmp32u) );
	__try( read_int("patch", &tmp32u) );
	__try( end_section_read("version") );
	
	if (section_exists("header"))
	{
		__try( start_section_read("header") );
		had_header = TRUE;

		__try( read_str("playername",p_ptr->name) ); /* 32 */

		skip_value("pass");

		__try( read_byte("prace", &p_ptr->prace) );
		__try( read_byte("pclass", &p_ptr->pclass) );
		__try( read_byte("male", &p_ptr->male) );

		__try( read_binary("stat_order", stat_order_hack, A_MAX) );
		for (i = 0; i < A_MAX; i++)
			p_ptr->stat_order[i] = stat_order_hack[i];

		__try( end_section_read("header") );
	}

	/* Operating system info */
	__try( read_uint("sf_xtra", &sf_xtra) );

	/* Time of savefile creation */
	__try( read_uint("sf_when", &sf_when) );

	/* Number of resurrections */
	__try( read_short("sf_lives", &sf_lives) );

	/* Number of times played */
	__try( read_short("sf_saves", &sf_saves) );

	/* Skip the turn info - if present */
	__try( read_hturn("turn", &p_ptr->last_turn) );
	
	/* Turn this character was born on */
	if(value_exists("birth_turn"))
	{
		__try( read_hturn("birth_turn", &p_ptr->birth_turn) );
	}
	else
		/* Disable character event logging if no birth turn */
		ht_clr(&p_ptr->birth_turn);

	/* Player turns (actually time spent playing) */
	if(value_exists("player_turn"))
	{
		__try( read_hturn("player_turn", &p_ptr->turn) );
	}
	else
		ht_clr(&p_ptr->turn);

	/* Read birth options */
	if (rd_birthoptions(p_ptr))
	{
		return (28);
	}

	/* Monster Memory */
	if (section_exists("monster_lore")) {
	__try( start_section_read("monster_lore") );
	__try( read_short("max_r_idx", &tmp16u) );

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
		__try( rd_lore(p_ptr, i) );
	}
	__try( end_section_read("monster_lore") );
	}
	
	/* Object Memory */
	__try( start_section_read("object_memory") );
	__try( read_short("max_k_idx", &tmp16u) );

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

		__try( read_byte("flags", &tmp8u) );

		p_ptr->kind_aware[i] = (tmp8u & 0x01) ? TRUE : FALSE;
		p_ptr->kind_tried[i] = (tmp8u & 0x02) ? TRUE : FALSE;
	}
	__try( end_section_read("object_memory") );

	/*if (arg_fiddle) note("Loaded Object Memory");*/

	/* Read the extra stuff */
	__try( rd_extra(p_ptr, had_header) );

	/*if (arg_fiddle) note("Loaded extra information");*/


	/* Read the player_hp array */
	__try( start_section_read("hp") );
	__try( read_short("py_max_level", &tmp16u) );

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
	{
		__try( read_short("hp", &p_ptr->player_hp[i]) );
	}
	__try( end_section_read("hp") );


	/* Important -- Initialize the race/class */
	p_ptr->rp_ptr = &p_info[p_ptr->prace];
	p_ptr->cp_ptr = &c_info[p_ptr->pclass];
	

	/* Important -- Choose the magic info */
	p_ptr->mp_ptr = &c_info[p_ptr->pclass].spells;


	/* Read spell info */
	if (section_exists("spell_flags"))
	{
		__try( start_section_read("spell_flags") );
		for (i = 0; i < PY_MAX_SPELLS; i++)
		{
			__try( read_byte("flag", &p_ptr->spell_flags[i]) );
		}
		__try( end_section_read("spell_flags") );
	}
	else
	{
		/* Port spell flags from old format */
		u32b spell_learned1, spell_learned2;
		u32b spell_worked1, spell_worked2;
		u32b spell_forgotten1, spell_forgotten2;
		__try( read_uint("spell_learned1", &spell_learned1) );
		__try( read_uint("spell_learned2", &spell_learned2) );
		__try( read_uint("spell_worked1", &spell_worked1) );
		__try( read_uint("spell_worked2", &spell_worked2) );
		__try( read_uint("spell_forgotten1", &spell_forgotten1) );
		__try( read_uint("spell_forgotten2", &spell_forgotten2) );
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

	__try( start_section_read("spell_order") );
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		__try( read_byte("order", &p_ptr->spell_order[i]) );
	}
	__try( end_section_read("spell_order") );

	/* Read the inventory */
	if (rd_inventory(p_ptr))
	{
		note("Unable to read inventory");
		return (21);
	}

	/* Read hostility information if new enough */
	if (rd_hostilities(p_ptr))
	{
		return (22);
	}

	__try( rd_cave_memory(p_ptr) );
	
	/* read the wilderness map */
	__try( start_section_read("wilderness") );
	/* get the map size */
	__try( read_int("max_wild", &tmp32u) );
		
	/* if too many map entries */
	if (tmp32u > MAX_WILD)
	{
		return 23;
	}
		
	/* read in the map */
	for (i = 0; i < tmp32u; i++)
	{
		__try( read_byte("wild_map", &p_ptr->wild_map[i]) );
	}
	__try( end_section_read("wilderness") );
	
	/* Read the character event history */
	if(section_exists("event_history"))
	{
		char buf[160];
		cptr msg;
		history_event evt;
		history_event *last = NULL;
		__try( start_section_read("event_history") );
		while(value_exists("hist"))
		{
			int depth, level;
			history_event *n_evt = NULL;
			__try( read_str("hist", buf) );
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
		__try( end_section_read("event_history") );
	}

	/* Read the characters quest list */
	if(section_exists("quests"))
	{
		__try( start_section_read("quests") );
		__try( read_short("max_q_idx", &tmp16u) );
		for(i = 0; i < MAX_Q_IDX; i++)
		{
			__try( read_short("level", &tmp16u) );
			p_ptr->q_list[i].level = tmp16u;
		}
		__try( end_section_read("quests") );
	}

	/* Read the characters sold artifact list */
	if(section_exists("found_artifacts"))
	{
		__try( start_section_read("found_artifacts") );
		__try( read_short("max_a_idx", &tmp16u) );
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
				if(!clear)
				{
					__try( read_short("a_info", &tmp16u) );
				}
			}
			p_ptr->a_info[i] = tmp16u;
		}
		__try( end_section_read("found_artifacts") );
	}

	/* Hack -- no ghosts */
	/* r_info[z_info->r_max - 1].max_num = 0; */

	__try( end_section_read("mangband_player_save") );

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
	file_handle = file_open(p_ptr->savefile, MODE_READ, -1);

	/* Paranoia */
	if (!file_handle) return (-1);

	/* Call the sub-function */
	err = rd_savefile_new_aux(p_ptr);

	/* Check for errors */
	if (file_error(file_handle)) err = -1;

	/* Close the file */
	file_close(file_handle);

	/* Result */
	return (err);
}

errr rd_server_savefile()
{
#undef __try
#define __try(X) if (!(X)) { exit(1); }
#define __tryN(X) if ((X)) { exit(1); }
#define __tryR(X, RET) if (!(X)) { return (RET); }

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
	file_handle = file_open(savefile, MODE_READ, -1);
	line_counter = 0;


	__try( start_section_read("mangband_server_save") );
	__try( start_section_read("version") );
	__try( read_int("major", &major) );
	__try( read_int("minor", &major) );
	__try( read_int("patch", &major) );
	__try( end_section_read("version") );

	/* Paranoia */
	if (!file_handle) return (-1);

        /* Clear the checksums */
        v_check = 0L;
        x_check = 0L;

        /* Operating system info */
	__try( read_uint("xtra", &sf_xtra) );

        /* Time of savefile creation */
	__try( read_int("timestamp", &sf_when) );

        /* Number of lives */
	__try( read_short("sf_lives", &sf_lives) );

        /* Number of times played */
	__try( read_short("sf_saves", &sf_saves) );

        /* Monster Memory */
	__try( start_section_read("monster_lore") );
	__try( read_short("max_r_idx", &tmp16u) );

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
		 __try( rd_u_lore(i) );

		/* Access the monster race */
		r_ptr = &r_info[i];

        }

	__try( end_section_read("monster_lore") );

        /* Load the Artifacts */
	__try( start_section_read("artifacts") );
	__try( read_short("max_a_idx", &tmp16u) );

        /* Incompatible save files */
        if (tmp16u > z_info->a_max)
        {
                note(format("Too many (%u) artifacts!", tmp16u));
                return (24);
        }

        /* Read the artifact flags */
        for (i = 0; i < tmp16u; i++)
        {
		__try( read_byte("artifact", &tmp8u) );
                a_info[i].cur_num = tmp8u;
		/* Owner information */
		if (value_exists("owner_name"))
		{
			__try( read_quark("owner_name", &a_info[i].owner_name) );
			__try( read_int("owner_id", &a_info[i].owner_id) );
		}
        }
	__try( end_section_read("artifacts") );

	/* Read the stores */
	__try( start_section_read("stores") );
	__try( read_short("max_stores", &tmp16u) );
	for (i = 0; i < tmp16u; i++)
	{
		__tryR( rd_store(i), (22) );
	}
	__try( end_section_read("stores") );

	/* Read party info if savefile is new enough */
	__try( start_section_read("parties") );
	__try( read_short("max_parties", &tmp16u) );
		
		/* Incompatible save files */
		if (tmp16u > MAX_PARTIES)
		{
			note(format("Too many (%u) parties!", tmp16u));
			return (25);
		}

		/* Read the available records */
		for (i = 0; i < tmp16u; i++)
		{
			__try( rd_party(i) );
		}
	__try( end_section_read("parties") );

	/* XXX If new enough, read in the saved levels and monsters. */
	__try( start_section_read("dungeon_levels") );
		/* read the number of levels to be loaded */
		__try( read_uint("num_levels", &tmp32u) );
		/* load the levels */
		for (i = 0; i < tmp32u; i++)
		{
			__try( rd_dungeon(FALSE, 0) );
		}
		/* load any special static levels */
		__tryN( rd_dungeon_special() );
	__try( end_section_read("dungeon_levels") );

	__try( start_section_read("monsters") );
		/* get the number of monsters to be loaded */
		__try( read_int("max_monsters", &tmp32u) );
		if (tmp32u > MAX_M_IDX)
		{
			note(format("Too many (%u) monsters!", tmp16u));
			return (29);
		}
		/* load the monsters */
		for (i = 1; i < tmp32u; i++)
		{
			__try( rd_monster(&m_list[m_pop()]) );
		}
	__try( end_section_read("monsters") );

	/* Read object info */
	__try( start_section_read("objects") );
	__try( read_short("max_objects", &tmp16u) );

		/* Incompatible save files */
		if (tmp16u > MAX_O_IDX)
		{
			note(format("Too many (%u) objects!", tmp16u));
			return (26);
		}

		/* Read the available records */
		for (i = 1; i < tmp16u; i++)
		{
			__try( rd_item(&o_list[i]) );
		}

		/* Set the maximum object number */
		o_max = tmp16u;
	__try( end_section_read("objects") );

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
	__try( start_section_read("houses") );
	__try( read_short("num_houses", &tmp16u) );

		/* Incompatible save files */
		if (tmp16u > MAX_HOUSES)
		{
			note(format("Too many (%u) houses!", tmp16u));
			return (27);
		}

		/* Read the available records */
		for (i = 0; i < tmp16u; i++)
		{
			__try( rd_house(i) );
		}
		num_houses = tmp16u;
		__try( end_section_read("houses") );

		/* Read arenas info */
		if (section_exists("arenas")) 
		{
			__try( start_section_read("arenas") );
			__try( read_short("num_arenas", &tmp16u) );
	
			/* Incompatible save files */
			if (tmp16u > MAX_ARENAS)
			{
				note(format("Too many (%u) arenas!", tmp16u));
				return (27);
			}
	
			/* Read the available records */
			for (i = 0; i < tmp16u; i++)
			{
				__try( rd_arena(i) );
			}
			num_arenas = tmp16u;
			__try( end_section_read("arenas") );
		}

	/* Read wilderness info */
	__try( start_section_read("wilderness") );
	/* read how many wilderness levels */
	__try( read_int("max_wild", &tmp32u) );

		if (tmp32u > MAX_WILD)
		{
			note("Too many wilderness levels");
			return 28;
		}
	
		for (i = 1; i < tmp32u; i++)
		{
			__try( rd_wild(i) );
		}
		__try( end_section_read("wilderness") );

	/* Read the player name database  */
	__try( start_section_read("player_names") );
	__try( read_int("num_players", &tmp32u) );

		/* Read the available records */
		for (i = 0; i < tmp32u; i++)
		{
			__try( start_section_read("player") );

			/* Read the ID */
			__try( read_int("id", &tmp32s) );

			/* Read the player name */
			__try( read_str("name", name) );

			/* Store the player name */
			add_player_name(name, tmp32s);

			__try( end_section_read("player") );
		}
	__try( end_section_read("player_names") );


	__try( read_uint("seed_flavor", &seed_flavor) );
	__try( read_uint("seed_town", &seed_town) );

	__try( read_int("player_id", &player_id) );

	__try( read_hturn("turn", &turn) );

        /* Hack -- no ghosts */
        /*r_info[z_info->r_max - 1].max_num = 0;*/

	__try( end_section_read("mangband_server_save") );

	/* Check for errors */
	if (file_error(file_handle)) err = -1;

	/* Close the file */
	file_close(file_handle);

	/* Result */
	return (err);
}

