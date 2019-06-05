/*
 * Support for the "party" system.
 */

#include "mangband.h"

/*
 * Lookup a party number by name.
 */
int party_lookup(cptr name)
{
	int i;

	/* Check each party */
	for (i = 0; i < MAX_PARTIES; i++)
	{
		/* Check name */
		if (streq(parties[i].name, name))
			return i;
	}

	/* No match */
	return -1;
}

/*
 * Check for the existance of a player in a party.
 */
bool player_in_party(int party_id, player_type *p_ptr)
{
	/* Check */
	if (p_ptr->party == party_id)
		return TRUE;

	/* Not in the party */
	return FALSE;
}

/*
 * Create a new party, owned by "p_ptr", and called
 * "name".
 */
int party_create(player_type *p_ptr, cptr name)
{
	int index = 0, i;
	hturn oldest = turn;

	/* Check for already existing party by that name */
	if (party_lookup(name) != -1)
	{
		msg_print(p_ptr, "A party by that name already exists.");
		return FALSE;
	}

	/* Make sure this guy isn't in some other party already */
	if (p_ptr->party != 0)
	{
		msg_print(p_ptr, "You already belong to a party!");
		return FALSE;
	}

	/* Find the "best" party index */
	for (i = 1; i < MAX_PARTIES; i++)
	{
		/* Check deletion time of disbanded parties */
		if (parties[i].num == 0 && ht_passed(&oldest, &parties[i].created, 0))
		{
			/* Track oldest */
			oldest = parties[i].created;
			index = i;
		}
	}

	/* Make sure we found an empty slot */
	if (index == 0 || ht_eq(&oldest, &turn))
	{
		/* Error */
		msg_print(p_ptr, "There aren't enough party slots!");
		return FALSE;
	}

	/* Set party name */
	strcpy(parties[index].name, name);

	/* Set owner name */
	strcpy(parties[index].owner, p_ptr->name);

	/* Add the owner as a member */
	p_ptr->party = index;
	parties[index].num++;

	/* Set the "creation time" */
	parties[index].created = turn;

	/* Resend party info */
	send_party_info(p_ptr);

	/* Success */
	return TRUE;
}

/*
 * Add a player to a party.
 */
int party_add(player_type *q_ptr, cptr name)
{
	player_type *p_ptr = NULL;
	int party_id = q_ptr->party, i;

	/* Find name */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this one */
		if (streq(name, Players[i]->name))
		{
			/* Found him */
			p_ptr = Players[i];
			break;
		}
	}

	/* Check for existance */
	if (p_ptr == NULL)
	{
		/* Oops */
		msg_print(q_ptr, "That player is not currently in the game.");

		return FALSE;
	}

	/* Check for hostility */
	if (check_hostile(p_ptr, q_ptr))
	{
		msg_print(q_ptr, "That player is hostile towards you.");
		return FALSE;
	}
	
	if (check_hostile(q_ptr, p_ptr))
	{
		msg_print(q_ptr, "You are hostile to that player.");
		return FALSE;
	}

	/* Make sure this isn't an imposter */
	if (!streq(parties[party_id].owner, q_ptr->name))
	{
		/* Message */
		msg_print(q_ptr, "You must be the owner to add someone.");

		/* Abort */
		return FALSE;
	}

	/* Make sure this added person is neutral */
	if (p_ptr->party != 0)
	{
		/* Message */
		msg_print(q_ptr, "That player is already in a party.");

		/* Abort */
		return FALSE;
	}

	/* Tell the party about its new member */
	party_msg_format(party_id, "%s has been added to party %s.", p_ptr->name, parties[party_id].name);

	/* One more player in this party */
	parties[party_id].num++;

	/* Tell him about it */
	msg_format(p_ptr, "You've been added to party '%s'.", parties[party_id].name);

	/* Set his party number */
	p_ptr->party = party_id;

	/* Resend info */
	send_party_info(p_ptr);

	/* Success */
	return TRUE;
}

/*
 * Remove a person from a party.
 *
 * Removing the party owner destroys the party.
 */
int party_remove(player_type *q_ptr, cptr name)
{
	player_type *p_ptr = NULL;
	int party_id = q_ptr->party, i;

	/* Find name */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this one */
		if (streq(name, Players[i]->name))
		{
			/* Found him */
			p_ptr = Players[i];
			break;
		}
	}

	/* Check for existance */
	if (p_ptr == NULL)
	{
		/* Oops */
		msg_print(q_ptr, "That player is not currently in the game.");

		return FALSE;
	}

	/* Make sure this is the owner */
	if (!streq(parties[party_id].owner, q_ptr->name))
	{
		/* Message */
		msg_print(q_ptr, "You must be the owner to delete someone.");

		/* Abort */
		return FALSE;
	}

	/* Make sure they were in the party to begin with */
	if (!player_in_party(party_id, p_ptr))
	{
		/* Message */
		msg_print(q_ptr, "You can only delete party members.");

		/* Abort */
		return FALSE;
	}

	/* See if this is the owner we're deleting */
	if (same_player(q_ptr, p_ptr))
	{
		/* Remove the party altogether */

		/* Set the number of people in this party to zero */
		parties[party_id].num = 0;

		/* Remove everyone else */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Check if they are in here */
			if (player_in_party(party_id, Players[i]))
			{
				Players[i]->party = 0;
				msg_print(Players[i], "Your party has been disbanded.");
				send_party_info(Players[i]);
			}
		}

		/* Set the creation time to "disbanded time" */
		parties[party_id].created = turn;
		
		/* Empty the name */
		strcpy(parties[party_id].name, "");
	}

	/* Keep the party, just lose a member */
	else
	{
		/* Lose a member */
		parties[party_id].num--;

		/* Set his party number back to "neutral" */
		p_ptr->party = 0;

		/* Messages */
		msg_print(p_ptr, "You have been removed from your party.");
		party_msg_format(party_id, "%s has been removed from the party.", p_ptr->name);

		/* Resend info */
		send_party_info(p_ptr);
	}

	return TRUE;
}


/*
 * A player wants to leave a party.
 */
void party_leave(player_type *p_ptr)
{
	int party_id = p_ptr->party;

	/* Make sure he belongs to a party */
	if (!party_id)
	{
		msg_print(p_ptr, "You don't belong to a party.");
		return;
	}

	/* If he's the owner, use the other function */
	if (streq(p_ptr->name, parties[party_id].owner))
	{
		/* Call party_remove */
		party_remove(p_ptr, p_ptr->name);
		return;
	}

	/* Lose a member */
	parties[party_id].num--;

	/* Set him back to "neutral" */
	p_ptr->party = 0;

	/* Inform people */
	msg_print(p_ptr, "You have been removed from your party.");
	party_msg_format(party_id, "%s has left the party.", p_ptr->name);

	/* Resend info */
	send_party_info(p_ptr);
}


/*
 * Send a message to everyone in a party.
 */
void party_msg(int party_id, cptr msg)
{
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this guy */
		if (player_in_party(party_id, Players[i]))
			msg_print_aux(Players[i], msg, MSG_WHISPER);
	}
}

/*
 * Send a formatted message to a party.
 */
void party_msg_format(int party_id, cptr fmt, ...)
{
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	party_msg(party_id, buf);
}

/* Message nearby party members */
void party_msg_format_near(player_type *p_ptr, u16b type, cptr fmt, ...)
{
	player_type *q_ptr; /* Player who does the messaging */
	va_list vp;
	char buf[1024];
	int i;
	int 	Depth = p_ptr->dun_depth;
	int 		y = p_ptr->py;
	int 		x = p_ptr->px;
	int 	party = p_ptr->party;

	/* Remember who started this */
	q_ptr = p_ptr;

	/* Not a member of any party */
	if (!party) return;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	for (i = 1; i <= NumPlayers; i++)
	{
		/* Check this player */
		p_ptr = Players[i];
		
		/* Don't send the message to the player who caused it */
		if (same_player(q_ptr, p_ptr)) continue;

		/* Make sure this player is at this depth */
		if (p_ptr->dun_depth != Depth) continue;

		/* Meh, different party */
		if (!player_in_party(party, p_ptr)) continue;

		/* Can he see this player? */
		if (p_ptr->cave_flag[y][x] & CAVE_VIEW)
		{
			/* Send the message */
			msg_print_aux(p_ptr, buf, type);
		}
	}
}


/*
 * Split some experience among party members.
 *
 * This should ONLY be used while killing monsters.  The amount
 * should be the monster base experience times the monster level.
 *
 * This algorithm may need some work....  However, it does have some nifty
 * features, such as:
 *
 * 1) A party with just one member functions identically as before.
 *
 * 2) A party with two equally-levelled members functions such that each
 * member gets half as much experience as he would have by killing the monster
 * by himself.
 *
 * 3) Higher-leveled members of a party get higher percentages of the
 * experience.
 */
 
 /* The XP distribution was too unfair for low level characters,
    it made partying a real pain. I am changing it so that if the players
    have a difference in level of less than 5 than there is no difference
    in XP distribution. 
    
    I am also changing it so it divides by each players level, AFTER
    it has been given to them.
    
    UPDATE: it appears that it may be giving too much XP to the low lvl chars,
    but I have been too lazy to change it... however, this doesnt appear to be being
    abused much, and the new system is regardless much nicer than the old one.
    
    -APD-
    */
 
void party_gain_exp(player_type *q_ptr, int party_id, s32b amount, int m_idx)
{
	int i, Depth = q_ptr->dun_depth;
	s32b new_exp, new_exp_frac, average_lev = 0, num_members = 0;
	s32b modified_level;

	/* Mark needed people */
	num_members = party_mark_members(q_ptr, m_idx);
	
	/* Calculate the average level */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];
		if (p_ptr->in_hack)
		{
			/* Increase the "divisor" */
			average_lev += p_ptr->lev;
			num_members++;
		}
	}

	/* Now, distribute the experience */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];

		/* Check for his involvment */
		if (p_ptr->in_hack)
		{
			/* Calculate this guy's experience */
			
			if (p_ptr->lev * num_members < average_lev) // below average
			{
				if ((average_lev - p_ptr->lev * num_members) > 2 * num_members )
				{
					modified_level = p_ptr->lev * num_members + 2 * num_members;
				}				
				else modified_level = average_lev;
			}
			else
			{
				if ((p_ptr->lev * num_members - average_lev) > 2 * num_members )
				{
					modified_level = p_ptr->lev * num_members - 2 * num_members;
				}				
				else modified_level = average_lev;
						
			}
			
			
			new_exp = (amount * modified_level) / (average_lev * num_members * p_ptr->lev);
			new_exp_frac = ((((amount * modified_level) % (average_lev * num_members * p_ptr->lev) )
			                * 0x10000L ) / (average_lev * num_members * p_ptr->lev)) + p_ptr->exp_frac;

			/* Keep track of experience */
			if (new_exp_frac >= 0x10000L)
			{
				new_exp++;
				p_ptr->exp_frac = new_exp_frac - 0x10000L;
			}
			else
			{
				p_ptr->exp_frac = new_exp_frac;
			}

			/* Gain experience */
			gain_exp(p_ptr, new_exp);
		}
	}
}
int party_mark_members(player_type *p_ptr, int m_idx)
{
	player_type *q_ptr;
	int i, total = 0;
	
	/* Determine players involved in killing */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];
		q_ptr->in_hack = FALSE;
		if (same_player(q_ptr, p_ptr) || ((p_ptr->party) &&
			(!m_idx || q_ptr->mon_hrt[m_idx]) &&
			(q_ptr->dun_depth == p_ptr->dun_depth) &&
			(q_ptr->party == p_ptr->party) &&
			((cfg_party_sharelevel == -1) || (abs(q_ptr->lev - p_ptr->lev) <= cfg_party_sharelevel))
			))
		{
			q_ptr->in_hack = TRUE;
			total++;
		}
	}
	
	return total;
}
void party_share_hurt(player_type *p_ptr, player_type  *q_ptr)
{
	int i;

	/* Not in party, or not the same party */
	if (!p_ptr->party || p_ptr->party != q_ptr->party)
		return;

	/* Copy hurt */
	for (i = 0; i < MAX_M_IDX; i++)
	{
		if (q_ptr->mon_hrt[i])
			p_ptr->mon_hrt[i] = TRUE;
	}
}

/*
 * Add a player to another player's list of hostilities.
 */
bool add_hostility(player_type *p_ptr, cptr name)
{
	player_type *q_ptr;
	hostile_type *h_ptr;
	int i;

	/* Check if PvP is possible at all */
	if (cfg_pvp_hostility > 2)
	{
		/* Message */
		msg_print(p_ptr, "You cannot be hostile.");

		return FALSE;
	}

	/* Check for sillyness */
	if (streq(name, p_ptr->name))
	{
		/* Message */
		msg_print(p_ptr, "You cannot be hostile toward yourself.");

		return FALSE;
	}

	/* Search for player to add */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];

		/* Check name */
		if (!streq(q_ptr->name, name)) continue;

		/* Make sure players aren't in the same party */
		if (p_ptr->party && player_in_party(p_ptr->party, q_ptr))
		{
			/* Message */
			msg_format(p_ptr, "%^s is in your party!", q_ptr->name);

			return FALSE;
		}

		/* Ensure we don't add the same player twice */
		for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next)
		{
			/* Check this ID */
			if (h_ptr->id == q_ptr->id)
			{
				/* Message */
				msg_format(p_ptr, "You are already hostile toward %s.", q_ptr->name);

				return FALSE;
			}
		}

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = q_ptr->id;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;

		/* Message */
		msg_format(p_ptr, "You are now hostile toward %s.", q_ptr->name);
		
		/* Notify the victim */
		if (cfg_pvp_notify) msg_format(q_ptr, "%s is now hostile towards you.", p_ptr->name);

		/* Success */
		return TRUE;
	}
#if 0
	/* Search for party to add */
	if ((i = party_lookup(name)) != -1)
	{
		/* Ensure we don't add the same party twice */
		for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next)
		{
			/* Check this ID */
			if (h_ptr->id == 0 - i)
			{
				/* Message */
				msg_format(p_ptr, "You are already hostile toward party '%s'.", parties[i].name);

				return FALSE;
			}
		}

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = 0 - i;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;

		/* Message */
		msg_format(p_ptr, "You are now hostile toward party '%s'.", parties[i].name);

		/* Success */
		return TRUE;
	}
#endif
	/* Couldn't find player */
	msg_format(p_ptr, "%^s is not currently in the game.", name);

	return FALSE;
}

/*
 * Remove an entry from a player's list of hostilities
 */
bool remove_hostility(player_type *p_ptr, cptr name)
{
	hostile_type *h_ptr, *i_ptr;
	cptr p;

	/* Initialize lock-step */
	i_ptr = NULL;

	/* Search entries */
	for (h_ptr = p_ptr->hostile; h_ptr; i_ptr = h_ptr, h_ptr = h_ptr->next)
	{
		/* Lookup name of this entry */
		if (h_ptr->id > 0)
		{
			/* Look up name */
			p = lookup_player_name(h_ptr->id);

			/* Check player name */
			if (p && streq(p, name))
			{
				/* Delete this entry */
				if (i_ptr)
				{
					/* Skip over */
					i_ptr->next = h_ptr->next;
				}
				else
				{
					/* Adjust beginning of list */
					p_ptr->hostile = h_ptr->next;
				}

				/* Message */
				msg_format(p_ptr, "No longer hostile toward %s.", name);

				/* Delete node */
				KILL(h_ptr);

				/* Success */
				return TRUE;
			}
		}
#if 0
		else
		{
			/* Assume this is a party */
			if (streq(parties[0 - h_ptr->id].name, name))
			{
				/* Delete this entry */
				if (i_ptr)
				{
					/* Skip over */
					i_ptr->next = h_ptr->next;
				}
				else
				{
					/* Adjust beginning of list */
					p_ptr->hostile = h_ptr->next;
				}

				/* Message */
				msg_format(p_ptr, "No longer hostile toward party '%s'.", name);

				/* Delete node */
				KILL(h_ptr, hostile_type);

				/* Success */
				return TRUE;
			}
		}
#endif
	}

	/* Message */
	msg_format(p_ptr, "You are not hostile toward %s.", name);

	/* Failure */
	return FALSE;
}

/*
 * Check if player "p_ptr" is hostile toward player "q_ptr"
 */
bool check_hostile(player_type *p_ptr, player_type *q_ptr)
{
	hostile_type *h_ptr;

	/* Scan list */
	for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next)
	{
		/* Check ID */
		if (h_ptr->id > 0)
		{
			/* Identical ID's yield hostility */
			if (h_ptr->id == q_ptr->id)
				return TRUE;
		}
#if 0
		else
		{
			/* Check if target belongs to hostile party */
			if (Players[target]->party == 0 - h_ptr->id)
				return TRUE;
		}
#endif
	}

	/* Not hostile */
	return FALSE;
}


/*
 * The following is a simple hash table, which is used for mapping a player's
 * ID number to his name.  Only players that are still alive are stored in
 * the table, thus the mapping from a 32-bit integer is very sparse.  Also,
 * duplicate ID numbers are prohibitied.
 *
 * The hash function is going to be h(x) = x % n, where n is the length of
 * the table.  For efficiency reasons, n will be a power of 2, thus the
 * hash function can be a bitwise "and" and get the relevant bits off the end.
 *
 * No "probing" is done; if any two ID's map to the same hash slot, they will
 * be chained in a linked list.  This will most likely be a very rare thing,
 * however.
 */

/* The struct to hold a data entry */
typedef struct hash_entry hash_entry;

struct hash_entry
{
	int id;				/* The ID */
	cptr name;			/* Player name */
	struct hash_entry *next;	/* Next entry in the chain */
};

/* The hash table itself */
static hash_entry *hash_table[NUM_HASH_ENTRIES];


/*
 * Return the slot in which an ID should be stored.
 */
static int hash_slot(int id)
{
	/* Be very efficient */
	return (id & (NUM_HASH_ENTRIES - 1));
}

/*
 * Lookup a player name by ID.  Will return NULL if the name doesn't exist.
 */
cptr lookup_player_name(int id)
{
	int slot;
	hash_entry *ptr;

	/* Get the slot */
	slot = hash_slot(id);

	/* Acquire the pointer to the first element in the chain */
	ptr = hash_table[slot];

	/* Search the chain, looking for the correct ID */
	while (ptr)
	{
		/* Check this entry */
		if (ptr->id == id)
			return ptr->name;

		/* Next entry in chain */
		ptr = ptr->next;
	}

	/* Not found */
	return NULL;
}

/*
 * Lookup a player's ID by name.  Return 0 if not found.
 */
int lookup_player_id(cptr name)
{
	hash_entry *ptr;
	int i;

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++)
	{
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr)
		{
			/* Check this name */
			if (!strcmp(ptr->name, name))
				return ptr->id;

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Not found */
	return 0;
}

/*
 * Add a name to the hash table.
 */
void add_player_name(cptr name, int id)
{
	int slot;
	hash_entry *ptr;

	/* Get the destination slot */
	slot = hash_slot(id);

	/* Create a new hash entry struct */
	MAKE(ptr, hash_entry);

	/* Make a copy of the player name in the entry */
	ptr->name = string_make(name);

	/* Set the entry's id */
	ptr->id = id;

	/* Add the rest of the chain to this entry */
	ptr->next = hash_table[slot];

	/* Put this entry in the table */
	hash_table[slot] = ptr;
}

/*
 * Delete an entry from the table, by ID.
 */
void delete_player_id(int id)
{
	int slot;
	hash_entry *ptr, *old_ptr;

	/* Get the destination slot */
	slot = hash_slot(id);

	/* Acquire the pointer to the entry chain */
	ptr = hash_table[slot];

	/* Keep a pointer one step behind this one */
	old_ptr = NULL;

	/* Attempt to find the ID to delete */
	while (ptr)
	{
		/* Check this one */
		if (ptr->id == id)
		{
			/* Delete this one from the table */
			if (old_ptr == NULL)
				hash_table[slot] = ptr->next;
			else old_ptr->next = ptr->next;

			/* Free the memory in the player name */
			free((char *)(ptr->name));

			/* Free the memory for this struct */
			KILL(ptr);

			/* Done */
			return;
		}

		/* Remember this entry */
		old_ptr = ptr;

		/* Advance to next entry in the chain */
		ptr = ptr->next;
	}

	/* Not found */
	return;
}

/*
 * Delete a player by name.
 *
 * This is useful for fault tolerance, as it is possible to have
 * two entries for one player name, if the server crashes hideously
 * or the machine has a power outage or something.
 */
void delete_player_name(cptr name)
{
	int id;

	/* Delete every occurence of this name */
	while ((id = lookup_player_id(name)))
	{
		/* Delete this one */
		delete_player_id(id);
	}
}

/*
 * Free player names from memory
 */
void wipe_player_names()
{
	int i;
	hash_entry *ptr;
	hash_entry *next;

	/* Entry points */
	for (i = 0; i < NUM_HASH_ENTRIES; i++)
	{
		/* Acquire this chain */
		ptr = hash_table[i];

		/* Check this chain */
		while (ptr)
		{
			next = ptr->next;

			if (ptr->name) 
				string_free(ptr->name);
			KILL(ptr);

			ptr = next;
		}
	}
}
/*
 * Return a list of the player ID's stored in the table.
 */
int player_id_list(int **list)
{
	int i, len = 0, k = 0;
	hash_entry *ptr;

	/* Count up the number of valid entries */
	for (i = 0; i < NUM_HASH_ENTRIES; i++)
	{
		/* Acquire this chain */
		ptr = hash_table[i];

		/* Check this chain */
		while (ptr)
		{
			/* One more entry */
			len++;

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Allocate memory for the list */
	C_MAKE((*list), len, int);

	/* Look again, this time storing ID's */
	for (i = 0; i < NUM_HASH_ENTRIES; i++)
	{
		/* Acquire this chain */
		ptr = hash_table[i];

		/* Check this chain */
		while (ptr)
		{
			/* Store this ID */
			(*list)[k++] = ptr->id;

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Return length */
	return len;
}

/* Make player "p_ptr" hostile toward player "q_ptr", if not already */
void add_hostility_p(player_type *p_ptr, player_type *q_ptr)
{
	if (!check_hostile(p_ptr, q_ptr))
	{
		add_hostility(p_ptr, q_ptr->name);
	}
}

/* 
 * Engage in PVP or Cancel it
 *
 * This function should be called in each instance of player-to-player damage possibility
 * i.e. melee, ranged attacks, magic, splash damage, etc.  It checks if pvp is desirable/possible 
 * and sets some hostility if nessecary. 
 *
 * Modes: 0 -- test		1 -- meele		2 -- direct ranged		3 -- inderect ranged
 *
 *	Returns FALSE if no damage should be dealt 
 */ 
bool pvp_okay(player_type *attacker, player_type *target, int mode)
{
	bool intentional = FALSE;
	int Depth = attacker->dun_depth;
	int party_id = attacker->party;
	int hostility = cfg_pvp_hostility;
	if (check_hostile(attacker, target)) intentional = TRUE;

	/* SAFE DEPTH */
	if (cfg_pvp_safedepth > -1 && Depth > -1 && Depth <= cfg_pvp_safedepth) hostility = cfg_pvp_safehostility;
	/* SAFE WILDERNESS RADIUS */
	if (cfg_pvp_saferadius > -1 && Depth < 0 && wild_info[Depth].radius <= cfg_pvp_saferadius) hostility = cfg_pvp_safehostility;
	/* SAFE LEVEL DIFFERENCE */
	if (cfg_pvp_safelevel > -1 && abs(attacker->lev - target->lev) > cfg_pvp_safelevel) hostility = cfg_pvp_safehostility;

	/* Safe Mode -- Both players must be hostile towards each other (1.1.0) */
	if (hostility == 2) {
		if (check_hostile(target, attacker) && intentional) return TRUE;
	}
	
	/* Normal Mode -- Attacker must be hostile towards his target */
	if (hostility == 1) {
		/* If this was intentional, make target hostile */ 
		if (intentional) 
		{ 
			if (mode) add_hostility_p(target, attacker);
			return TRUE; 
		}
	}

	/* Dangerous Mode -- No hostility needed at all (only for inderect damage) (0.7.2) */
	if (hostility == 0) {
		if (mode > 1) {
			if (!intentional) add_hostility_p(attacker, target);
			add_hostility_p(target, attacker);
			return TRUE;
		} else {
			if (mode && intentional) add_hostility_p(target, attacker);
			return intentional;
		}
	}

	/* Brutal mode -- all players always fight */	
	if (hostility == -1) {
		/* There's only one way not to fight: be in a party */
		if (!(party_id && player_in_party(party_id, target)))
		return TRUE;
	}

	return FALSE;
}










