#include "c-angband.h"

char index_to_label(int i)
{
	/* Indices for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indices for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


bool item_tester_hack(object_type *o_ptr, int i)
{
	int j;

	/* STOP on flag mismatch */
	if (item_tester[i].flag && !(o_ptr->ident & item_tester[i].flag)) return (FALSE);
	
	/* OK on tval match */
	for (j = 0; j < MAX_ITH_TVAL; j++)
	{
		if (item_tester[i].tval[j] == 0) break;
		if (item_tester[i].tval[j] == o_ptr->tval) return (TRUE);
	}

	return ( (!j) ? (TRUE) : (FALSE) );
}

bool item_tester_okay(object_type *o_ptr)
{
	/* Hack -- allow testing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->tval) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_MAX) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		/* Check the fake hook */
		if (item_tester_tval > TV_MAX)
		{
			return (item_tester_hack(o_ptr, item_tester_tval - TV_MAX - 1)); 
		}
		/* Or direct (mis)match */
		else if (!(item_tester_tval == o_ptr->tval)) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}


static bool get_item_okay(int i)
{
	/* Illegal items */
	if ((i < 0) || (i >= INVEN_TOTAL)) return (FALSE);

	/* Verify the item */
	if (!item_tester_okay(&inventory[i])) return (FALSE);

	/* Assume okay */
	return (TRUE);
}


static bool verify(cptr prompt, int item)
{
	char	o_name[80];

	char	out_val[160];


	/* Describe */
	my_strcpy(o_name, inventory_name[item], 80);

	/* Prompt */
	(void)sprintf(out_val, "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


s16b c_label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].tval) return (-1);

	/* Return the index */
	return (i);
}

s16b c_label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].tval) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" command_cmd code.
 */
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;


	/* Check every object */
	for (i = 0; i < INVEN_TOTAL; ++i)
	{
		char *buf = inventory_name[i];
		char *buf2;

		/* Skip empty objects */
		if (!buf[0]) continue;

		/* Skip empty inscriptions */
		if (!(buf2 = strchr(buf, '{'))) continue;

		/* Find a '@' */
		s = strchr(buf2, '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return (FALSE);
}

/* Prompt player for a string, then try to find an item matching it */
/* Returns "0" if an item was found, "1" if user has abort input, and
 *"-1" if nothing could be found. */
static errr get_item_by_name(int *k, bool inven, bool equip, bool floor_, char *force_str)
{
	char buf[256];
	char *tok;
	int i;
	size_t len;
	char *prompt = "Item name: ";

	/* HACK -- spellcasting mode (select book by spell) */
	if (spellcasting)
	{
		int sn = -1;
		errr failed = get_spell_by_name(k, &sn, inven, equip, TRUE);
		/* Remember spell index */
		spellcasting_spell = sn;
		/* Don't do any other tests */
		return failed;
	}

	/* Hack -- show opening quote symbol */
	if (prompt_quote_hack) prompt = "Item name: \"";

	/* Already have a symbol */
	if (force_str)
	{
		my_strcpy(buf, force_str, sizeof(buf));
	}
	/* Ask player */
	else
	{
		buf[0] = '\0';
		if (!get_string(prompt, buf, 80))
		{
			return 1;
		}
	}

	/* Hack -- remove final quote */
	len = strlen(buf);
	if (len == 0) return 1;
	if (buf[len-1] == '"') buf[len-1] = '\0';

	/* Split entry */
	tok = strtok(buf, "|");
	while (tok)
	{
		if (STRZERO(tok)) continue;
		/* Match against valid items */
		for (i = 0; i < INVEN_TOTAL; i++)
		{
			if (!inven && i < INVEN_WIELD) continue;
			if (!equip && i >= INVEN_WIELD) continue;
			if (inventory[i].tval == 0) continue;
			if (!get_item_okay(i)) continue;

			if (my_stristr(inventory_name[i], tok))
			{
				(*k) = i;
				return 0;
			}
		}
		/* Also try floor */
		if (floor_)
		{
			if (floor_item.tval
			&& item_tester_okay(&floor_item)
			&& my_stristr(floor_name, tok))
			{
				(*k) = FLOOR_INDEX;
				return 0;
			}
		}
		tok = strtok(NULL, "|");
	}
	return -1;
}

int count_items_by_name(char* name, bool inven, bool equip, bool floor_)
{
	byte old_tester;
	bool old_spellcasting;
	int k;
	int num = 0;

	/* Hack -- save old tester, reset current */
	old_tester = item_tester_tval;
	item_tester_tval = 0;
	/* Same for "spellcasting" flag */
	old_spellcasting = spellcasting;
	spellcasting = FALSE;

	if (!get_item_by_name(&k, inven, equip, floor_, name))
	{
		num = inventory[k].number;
	}

	/* Restore old tester */
	item_tester_tval = old_tester;
	/* Restore spellcasting flag */
	spellcasting = old_spellcasting;

	return num;
}


bool c_check_item(int *item, byte tval)
{
	int i;
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->number) continue;
		
		/* Check the "tval" code */
		if (o_ptr->tval == tval)
		{
			(*item) = i;
			return TRUE;
		}
	}	

	/* Oops */
	return FALSE;
}

byte c_secondary_tester(int item)
{
	if ((item < 0 && FLOOR_NEGATIVE) || (item >= INVEN_TOTAL && !FLOOR_NEGATIVE))
	{
		byte floor_item_pos = (FLOOR_NEGATIVE ? 0 - item + FLOOR_INDEX : item - FLOOR_INDEX);
		/* Hack -- discard floor_item_pos, we don't support piles */
		return floor_secondary_tester;
	}
	else if (item < 0 || item >= INVEN_TOTAL)
	{
		/* Don't know what that item is */
		return 0;
	}
	else
	{
		/* It's an inven/equip item */
		return inventory_secondary_tester[item];
	}
}

bool c_get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor_)
{
	char	n1, n2, which = ' ';

	event_type ke;
	int	k, i1, i2, e1, e2;
	bool	ver, done, item;
	bool	equip_up, inven_up, window_up;
	
	bool allow_floor = FALSE;

	char	tmp_val[160];
	char	out_val[160];
	errr	r;

	/* Paranoia */
	if (!inven && !equip) return (FALSE);

	/* Selecting an item */
	in_item_prompt = TRUE;

	/* The top line is icky */
	topline_icky = TRUE;

	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;

	/* No window updates needed */
	window_up = equip_up = inven_up = FALSE;

	/* Default to "no item" */
	*cp = -1;

	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;

	/* Forbid inventory */
	if (!inven) i2 = -1;

	/* Restrict inventory indices */
	while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2))) i2--;

	/* Update window (later, twice) */
	if ((i1 != 0) || (i2 != INVEN_PACK - 1)) inven_up = TRUE;

	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = INVEN_TOTAL - 1;

	/* Restrict equipment indices */
	while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2))) e2--;

	/* Update window (later, twice) */
	if ((e1 != INVEN_WIELD) || (e2 != INVEN_TOTAL - 1)) equip_up = TRUE;

	/* Hack -- restrict floor choice */
	if (floor_item.tval && floor_) allow_floor = item_tester_okay(&floor_item);
	
	if ((i1 > i2) && (e1 > e2) && !allow_floor)
	{
		/* Hack -- traditionally, (M)Angband doesn't sound a bell
		 * at this error. But we still want to call bell() so it can
		 * flush input. */
		bool old_ring_bell = ring_bell;
		ring_bell = FALSE;

		/* Cancel command_see */
		command_see = FALSE;

		/* Hack -- Nothing to choose */
		*cp = -2;

		/* Input error! */
		bell();

		/* End Hack -- restore user option */
		ring_bell = old_ring_bell;

		/* Done */
		done = TRUE;
	}

	/* Analyze choices */
	else
	{
		/* Hack -- reset display width */
		if (!command_see) command_gap = 50;

		/* Hack -- Start on equipment if requested */
		if (command_see && command_wrk && equip)
		{
			command_wrk = TRUE;
		}

		/* Use inventory if allowed */
		else if (inven)
		{
			command_wrk = FALSE;
		}

		/* Use equipment if allowed */
		else if (equip)
		{
			command_wrk = TRUE;
		}
	}

	/* Start with the listing ON */
	if ((inven || equip) && auto_itemlist)
	{
		command_see = TRUE;
	}

	/* Hack -- start out in "display" mode */
	if (command_see)
	{
		Term_save();
	}

	/* Update windows with possible choices */
	if (!done)
	{
		if (inven_up) p_ptr->window |= PW_INVEN;
		if (equip_up) p_ptr->window |= PW_EQUIP;
		window_stuff();
		window_up = TRUE;
	}
	
	/* Repeat while done */
	while (!done)
	{
		if (z_ask_item_aux) z_ask_item_aux(pmt, command_wrk, inven, equip, allow_floor);

		if (!command_wrk)
		{
			/* Extract the legal requests */
			n1 = I2A(i1);
			n2 = I2A(i2);

			/* Redraw if needed, make screen icky */
			if (command_see) show_inven();
		}

		/* Equipment screen */
		else
		{
			/* Extract the legal requests */
			n1 = I2A(e1 - INVEN_WIELD);
			n2 = I2A(e2 - INVEN_WIELD);

			/* Redraw if needed, make screen icky */
			if (command_see) show_equip();
		}

		/* Viewing inventory */
		if (!command_wrk)
		{
			/* Begin the prompt */
			sprintf(out_val, "Inven:");

			/* Some legal items */
			if (i1 <= i2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
					index_to_label(i1), index_to_label(i2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate ability to "view" */
			if (!command_see) strcat(out_val, " * to see,");

			/* Append */
			if (equip) strcat(out_val, " / for Equip,");
		}

		/* Viewing equipment */
		else
		{
			/* Begin the prompt */
			sprintf(out_val, "Equip:");

			/* Some legal items */
			if (e1 <= e2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c",
					index_to_label(e1), index_to_label(e2));

				/* Append */
				strcat(out_val, tmp_val);
			}

			/* Indicate the ability to "view" */
			if (!command_see) strcat(out_val, " * to see,");

			/* Append */	
			if (inven) strcat(out_val, " / for Inven,");
		}

		/* Append floor if possible */
		if (allow_floor) strcat(out_val, " - for floor,");

		/* Finish the prompt */
		strcat(out_val, " ESC");

		/* Build the prompt */
		sprintf(tmp_val, "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);

		/* Show the cursor */
		Term_show_ui_cursor();

		/* Get a key */
		do {
			ke = inkey_ex();
			/* Hack -- ignore mouse motion */
			if (ke.key == '\xff' && ke.index == 0) continue;
			break;
		} while(1);
		which = ke.key;

		/* Parse it */
		switch (which)
		{
			case ESCAPE:
			{
				command_gap = 50;
				done = TRUE;
				break;
			}

			case '*':
			case '?':
			case ' ':
			{
				/* Show/hide the list */
				if (!command_see)
				{
					Term_save();
					command_see = TRUE;
				}
				else
				{
					Term_load();
					command_see = FALSE;

					section_icky_row = 0;
					section_icky_col = 0;

					/* Flush any events */
					if (!shopping)
						Flush_queue();
				}
				break;
			}
			case '-':
			{
				/* Hack -- we only have 1 floor item */
				s16b floor_item_pos = 0;

				/* Floor */
				if (allow_floor)
				{
					(*cp) = FLOOR_INDEX + (FLOOR_NEGATIVE ? 0 - floor_item_pos : floor_item_pos);
					item = TRUE;
					done = TRUE;
				}
				else
				{
					bell();
				}

				break;
			}
			case '/':
			{
				/* Verify legality */
				if (!inven || !equip)
				{
					bell();
					break;
				}

				/* Fix screen */
				if (command_see)
				{
					Term_load();
					if (!shopping)
						Flush_queue();
					Term_save();
				}

				/* Switch inven/equip */
				command_wrk = !command_wrk;

				/* Need to redraw */
				break;
			}

			case '"':
				/* Allow '"' to be used as terminator */
				prompt_quote_hack = TRUE;
				/* fallthrough */
			case '@':
				/* XXX Lookup item by name */
				if (!(r = get_item_by_name(&k, inven, equip, floor_, (NULL))))
				{
					(*cp) = k;
					item = TRUE;
					done = TRUE;
				}
				else if (r < 0)
				{
					bell();
				}
				break;

			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* XXX XXX Look up that tag */
				if (!get_tag(&k, which))
				{
					bell();
					break;
				}

				/* Hack -- verify item */
				if ((k < INVEN_WIELD) ? !inven : !equip)
				{
					bell();
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell();
					break;
				}

#if 0
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
#endif

				/* Use that item */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '\n':
			case '\r':
			{
				/* Choose "default" inventory item */
				if (!command_wrk)
				{
					k = ((i1 == i2) ? i1 : -1);
				}

				/* Choose "default" equipment item */
				else
				{
					k = ((e1 == e2) ? e1 : -1);
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell();
					break;
				}

#if 0
				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
#endif

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '\xff':
			{
				if (!command_see) continue;
				if (ke.index == 0) continue;
				else if (ke.index != 1)
				{
					bell();
					break;
				}
				if (command_wrk)
					which = 'a' + inven_out_index[ke.mousey - 1] - INVEN_WIELD;
				else
					which = 'a' + inven_out_index[ke.mousey - 1];
				/* fallthrough */
			}

			default:
			{
				/* Extract "query" setting */
				ver = ( isupper(which) ? TRUE : FALSE );
				if (ver) which = tolower(which);

				/* Convert letter to inventory index */
				if (!command_wrk)
				{
					k = c_label_to_inven(which);
				}

				/* Convert letter to equipment index */
				else
				{
					k = c_label_to_equip(which);
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell();
					break;
				}

				/* Verify, abort if requested */
				if (ver && !verify("Try", k))
				{
					done = TRUE;
					break;
				}

#if 0
				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
#endif

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}


	/* Fix the screen if necessary */
	if (command_see) 
	{
		Term_load();
		section_icky_row = 0;
		section_icky_col = 0;
	}

	/* Hide UI cursor */
	Term_hide_ui_cursor();

	/* Fix the top line */
	topline_icky = FALSE;

	/* Flush any events */
	if (!shopping)
		Flush_queue();

	/* Hack -- Cancel "display" */
	command_see = FALSE;

	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;

	/* Forget the item_tester_hook restriction */
	item_tester_hook = 0;

	/* Discard spellcasting mode */
	spellcasting = FALSE;

	/* Fix windows */
	if (window_up)
	{
		if (inven_up) p_ptr->window |= PW_INVEN;
		if (equip_up) p_ptr->window |= PW_EQUIP;
		window_stuff();
	}

	/* Clear the prompt line */
	prt("", 0, 0);

	/* Not selecting item anymore */
	in_item_prompt = FALSE;

	/* Return TRUE if something was picked */
	return (item);
}

void do_cmd_inscribe_auto(char *name, char *inscription)
{
	int item;
	int cc_ind = -1, i;
	/* Hack -- find inscribe command (assume it's called '{') */
	for (i = 0; i < custom_commands; i++) {
		custom_command_type *cc_ptr = &custom_command[i];
		if ((cc_ptr->m_catch == '{')
		&& (cc_ptr->scheme == SCHEME_ITEM_STRING))
		{
			cc_ind = i;
			break;
		}
	}
	if (cc_ind == -1) return;
	if (!get_item_by_name(&item, TRUE, TRUE, TRUE, name)) return;
	send_custom_command(cc_ind, item, 0, 0, inscription);
}

void do_cmd_uniscribe_by_tag(char cmd, char tag)
{
	char old_command_cmd;
	int item = 0;
	bool done = FALSE;

	int cc_ind = -1, i;
	/* Hack -- find uninscribe command (assume it's called '}') */
	for (i = 0; i < custom_commands; i++) {
		custom_command_type *cc_ptr = &custom_command[i];
		if (cc_ptr->m_catch == '{')
		{
			cc_ind = i;
			break;
		}
	}

	/* Not found, abort */
	if (cc_ind < 0) return;

	old_command_cmd = command_cmd;
	command_cmd = cmd;
	while (!done)
	{
		bool found;
		found = get_tag(&i, tag);
		if (found == TRUE)
		{
			send_custom_command(cc_ind, item, 0, 0, NULL);
			done = FALSE;
		}
		else done = TRUE;
	}
	command_cmd = old_command_cmd;
}
