#include "angband.h"
#include "netclient.h"
#include "../common/md5.h"

/* Handle custom commands */
void cmd_custom(byte i)
{
	custom_command_type *cc_ptr;
	char dir;
	int item, item2;
	s32b value;
	cptr prompt;
	char entry[60];
	bool need_second, need_target;

	/* Byte is always 0, check if its > max */
	if (i > custom_commands) return;
	cc_ptr = &custom_command[i];
	dir = item = value = 0;
	prompt = cc_ptr->prompt;
	entry[0] = '\0';
	
	need_second = (cc_ptr->flag & COMMAND_NEED_SECOND ? TRUE : FALSE);
	need_target = (cc_ptr->flag & COMMAND_NEED_TARGET ? TRUE : FALSE);	
	
	/* Pre-tests */
	if (cc_ptr->flag & COMMAND_TEST_ALIVE)
	{
		if (p_ptr->ghost)
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		prompt = prompt + strlen(prompt) + 1;
	}
	if (cc_ptr->flag & COMMAND_TEST_DEAD)
	{
		if (!p_ptr->ghost)
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		prompt = prompt + strlen(prompt) + 1;
	}
	if (cc_ptr->flag & COMMAND_TEST_SPELL)
	{
		if (c_info[class].spell_book != cc_ptr->tval)
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		prompt = prompt + strlen(prompt) + 1;
	}
	if (cc_ptr->flag & COMMAND_SPECIAL_FILE)
	{
		special_line_type = cc_ptr->tval;
		strcpy(special_line_header, prompt);
		peruse_file();
		return;
	}
	else if (cc_ptr->flag & COMMAND_INTERACTIVE)
	{
		special_line_type = cc_ptr->tval;
		strcpy(special_line_header, prompt);
		cmd_interactive();
		return;
	}
	/* Search for an item (automatic) */
	if (cc_ptr->flag & COMMAND_ITEM_QUICK)
	{
		if (!c_check_item(&item, cc_ptr->tval))
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		prompt = prompt + strlen(prompt) + 1;
	}
	/* Ask for an item (interactive) ? */
	else if (cc_ptr->flag & COMMAND_NEED_ITEM)
	{
		item_tester_tval = cc_ptr->tval;
		if (!c_get_item(&item, prompt, 
				(cc_ptr->flag & COMMAND_ITEM_EQUIP ? TRUE : FALSE), 
				(cc_ptr->flag & COMMAND_ITEM_INVEN ? TRUE : FALSE), 
				(cc_ptr->flag & COMMAND_ITEM_FLOOR ? TRUE : FALSE)))
				return;
		prompt = prompt + strlen(prompt) + 1;

		/* Get an amount - from inventory */
		if ((cc_ptr->flag & COMMAND_ITEM_AMMOUNT) && item >= 0 && inventory[item].number > 1)
		{
			if (STRZERO(prompt)) prompt = "How many? ";
			value = c_get_quantity(prompt, inventory[item].number);
		}
		/* Get an amount - from floor */
		if ((cc_ptr->flag & COMMAND_ITEM_AMMOUNT) && item < 0 && floor_item.number > 1)
		{
			if (STRZERO(prompt)) prompt = "How many? ";
			value = c_get_quantity(prompt, floor_item.number);
		}
		
		/* Dirty Hack -- Reset! */
		if (cc_ptr->flag & COMMAND_ITEM_RESET)
		{
			need_second = need_target = FALSE;
			dir = item2 = 0;
			if (item >= 0)
			{
				need_target = (inventory[item].ident & ITEM_ASK_AIM  ? TRUE : FALSE);
				need_second = (inventory[item].ident & ITEM_ASK_ITEM ? TRUE : FALSE);
			}
			else
			{
				need_target = (floor_item.ident & ITEM_ASK_AIM  ? TRUE : FALSE);
				need_second = (floor_item.ident & ITEM_ASK_ITEM ? TRUE : FALSE);
			}
		}		
	}
	/* Spell? */
	if (cc_ptr->flag & COMMAND_NEED_SPELL)
	{
		int index, spell, indoff = 0;
		cptr p = prompt;
		prompt = prompt + strlen(prompt) + 1;
		if (cc_ptr->flag & COMMAND_SPELL_BOOK)
		{
			if (!get_spell(&spell, p, prompt, &item, FALSE)) return;
			index = item * SPELLS_PER_BOOK + spell;
		}
		else
		{
			int book = cc_ptr->tval;
			if (!get_spell(&spell, p, prompt, &book, FALSE)) return;
			index = book * SPELLS_PER_BOOK + spell;
			indoff = cc_ptr->tval * SPELLS_PER_BOOK;
		}
		prompt = prompt + strlen(prompt) + 1;
		if (cc_ptr->flag & COMMAND_SPELL_INDEX)
		{
			value = index - indoff;
		}
		else
		{
			value = spell;
		}

		/* Dirty Hack -- Reset! */
		if (cc_ptr->flag & COMMAND_SPELL_RESET)
		{
			need_second = need_target = FALSE;
			dir = item2 = 0;
			if (spell >= SPELL_PROJECTED)	need_target = TRUE;
			else
			{
				need_target = (spell_flag[index] & PY_SPELL_AIM  ? TRUE : FALSE);
				need_second = (spell_flag[index] & PY_SPELL_ITEM ? TRUE : FALSE);
			}
		}
	} 
	/* Second item? */
	if (need_second) /* cc_ptr->flag & COMMAND_NEED_SECOND) */
	{
		if (STRZERO(prompt)) prompt = "Which item? ";
		if (!c_get_item(&item2, prompt, 
				(cc_ptr->flag & COMMAND_SECOND_EQUIP ? TRUE : FALSE), 
				(cc_ptr->flag & COMMAND_SECOND_INVEN ? TRUE : FALSE), 
				(cc_ptr->flag & COMMAND_SECOND_FLOOR ? TRUE : FALSE)))
				return;
		prompt = prompt + strlen(prompt) + 1;
	}
	/* Target? */
	if (need_target) /* cc_ptr->flag & COMMAND_NEED_TARGET) */ 
	{
		if (!c_get_dir(&dir, prompt, 
				(cc_ptr->flag & COMMAND_TARGET_ALLOW ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_TARGET_FRIEND ? TRUE : FALSE)))
				return;
		prompt = prompt + strlen(prompt) + 1;
	}
	/* Need values? */
	if (cc_ptr->flag & COMMAND_NEED_VALUE)
	{
		if (STRZERO(prompt)) prompt = "Quantity: ";
		value = c_get_quantity(prompt, 999000000);
		prompt = prompt + strlen(prompt) + 1;
	}		
	if (cc_ptr->flag & COMMAND_NEED_CHAR)
	{
		if (STRZERO(prompt)) prompt = "Command: ";
		if (!get_com(prompt, &entry[0])) 
			return;
		entry[1] = '\0';
		prompt = prompt + strlen(prompt) + 1;
	}	
	else if (cc_ptr->flag & COMMAND_NEED_STRING)
	{
		if (STRZERO(prompt)) prompt = "Entry: ";
		if (!get_string(prompt, entry, sizeof(entry)))
			return;
		prompt = prompt + strlen(prompt) + 1;
	}
	if (cc_ptr->flag & COMMAND_NEED_CONFIRM)
	{
		if (STRZERO(prompt)) prompt = "Really perform said action ? ";
		if (!get_check(prompt))
			return;
		prompt = prompt + strlen(prompt) + 1;
	}
	/* Post-effects */
	if (cc_ptr->flag & COMMAND_SECOND_VALUE)
	{
		if (!value) value = (s32b)item2;
		else		value = -value;
	}
	if (cc_ptr->flag & COMMAND_SECOND_DIR)
	{
		if (!dir) dir = item2;
		else 	  dir = -dir;
	}	
	if (cc_ptr->flag & COMMAND_SECOND_CHAR)
	{
		entry[0] = item2;
		entry[1] = '\0';
	}


	Send_custom_command(i, item, dir, value, entry);
}
/* Handle all commands */
void process_command()
{
	byte i;
	for (i = 0; i < custom_commands; i++) 
	{
		if (custom_command[i].catch == command_cmd) 
		{
			cmd_custom(i);
			return;	
		}
	}

	/* Parse the command */
	switch (command_cmd)
	{
		/* Ignore */
		case ESCAPE:
		case ' ':
		{
			if (first_escape) 
				Send_clear();
			first_escape = FALSE;
			break;
		}

		/* Ignore return */
		case '\r':
		{
			break;
		}

		/*** Movement Commands ***/
#ifndef COMMAND_OVERLOAD
		/* Dig a tunnel*/
		case '+':
		case 'T':		
		{
			cmd_tunnel();
			break;
		}
#endif
		/* Move */
		case ';':
		{
			cmd_walk();
			break;
		}

		/*** Running, Staying, Resting ***/
		case '.':
		{
			cmd_run();
			break;
		}

		case ',':
		case 'g':
		{
			cmd_stay();
			break;
		}

		/* Get the mini-map */
		case 'M':
		{
			cmd_map();
			break;
		}

		/* Recenter map */
		case 'L':
		{
			cmd_locate();
			break;
		}
#ifndef COMMAND_OVERLOAD
		/* Search */
		case 's':
		{
			cmd_search();
			break;
		}

		/* Toggle Search Mode */
		case 'S':
		{
			cmd_toggle_search();
			break;
		}
#endif
		/* Rest */
		case 'R':
		{
			cmd_rest();
			break;
		}
#ifndef COMMAND_OVERLOAD
		/*** Stairs and doors and chests ***/

		/* Go up */
		case '<':
		{
			cmd_go_up();
			break;
		}

		/* Go down */
		case '>':
		{
			cmd_go_down();
			break;
		}

		/* Open a door */
		case 'o':
		{
			cmd_open();
			break;
		}

		/* Close a door */
		case 'c':
		{
			cmd_close();
			break;
		}

		/* Bash a door */
		case 'B':
		{
			cmd_bash();
			break;
		}

		/* Disarm a trap or chest */
		case 'D':
		{
			cmd_disarm();
			break;
		}
#endif
		/*** Inventory commands ***/
		case 'i':
		{
			cmd_inven();
			break;
		}

		case 'e':
		{
			cmd_equip();
			break;
		}

		case '$':
		{
			cmd_drop_gold();
			break;
		}
		case 'k':
		{
			cmd_destroy();
			break;
		}
		/*** Spell casting ***/
		case 'b':
		{
			cmd_browse();
			break;
		}
		
		case 'G':
		{
			cmd_study();
			break;
		}

		case 'm':
		{
			cmd_cast();
			break;
		}

		case 'p':
		{
			cmd_pray();
			break;
		}

		case 'U':
		{
			cmd_ghost();
			break;
		}

		/*** Looking/Targetting ***/
		case '*':
		{
			cmd_target();
			break;
		}

		case '(':
		{
			cmd_target_friendly();
			break;
		}

		case 'l':
		{
			cmd_look();
			break;
		}

		/*** Information ***/
		case 'C':
		{
			cmd_character();
			break;
		}
#if 0
		case '~':
		{
			cmd_artifacts();
			break;
		}

		case '|':
		{
			cmd_uniques();
			break;
		}

		case '@':
		{
			cmd_players();
			break;
		}

		case '#':
		{
			cmd_knowledge();
			/* cmd_high_scores(); */
			break;
		}

		case '?':
		{
			cmd_help();
			break;
		}
#endif
		/*** Miscellaneous ***/
		case ':':
		{
			cmd_message();
			break;
		}

		case 'P':
		{
			cmd_party();
			break;
		}

		case '&':
		{
			/* Dungeon master commands, normally only accessible to 
			 * a valid dungeon master.  These commands only are 
			 * effective for a valid dungeon master.
			 */
			cmd_master();
			break;
		}

		case '\'': /* Handle chat */
		{
			cmd_chat();
			break;
		}

		case KTRL('D'): /* 'Describe item. This means "brag about it in chat" */
		{
			cmd_describe();
			break;
		}

		case KTRL('P'):
		{
	        do_cmd_messages();
	        break;
		}

		case KTRL('X'):
		{
	        Net_cleanup();
	        quit(NULL);
		}

		case KTRL('R'):
		{
			cmd_redraw();
			break;
		}

		case 'Q':
		{
			cmd_suicide();
			break;
		}

		case '=':
		{
			do_cmd_options();
			break;
		}

		case '\"':
		{
			cmd_load_pref();
			break;
		}

		case '%':
		{
			interact_macros();
			break;
		}

		default:
		{
			prt("Hit '?' for help.", 0, 0);
			break;
		}
	}
}






void cmd_tunnel(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_tunnel(dir);
}

void cmd_walk(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_walk(dir);
}

void cmd_run(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_run(dir);
}

void cmd_stay(void)
{
	Send_stay();
}

void cmd_map(void)
{
	char ch;

	/* Hack -- if the screen is already icky, ignore this command */
	if (screen_icky) return;

	/* The screen is icky */
	screen_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Send the request */
	Send_map();

	/* Reset the line counter */
	last_line_info = 0;

	/* Wait until we get the whole thing */
	while (last_line_info < Term->hgt-SCREEN_CLIP_L)
	{
		/* Wait for net input, or a key */
		ch = inkey();

		/* Check for user abort */
		if (ch == ESCAPE)
			break;
	}

	/* Reload the screen */
	Term_load();

	/* The screen is OK now */
	screen_icky = FALSE;

	/* Flush any queued events */
	Flush_queue();
}

void cmd_locate(void)
{
	int dir;
	char ch;

	/* Initialize */
	Send_locate(5);

	/* Show panels until done */
	while (1)
	{
		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			/* Get a command (or Cancel) */
			ch = inkey();

			/* Check for cancel */
			if (ch == ESCAPE) break;

			/* Extract direction */
			dir = target_dir(ch);

			/* Error */
			if (!dir) bell();
		}

		/* No direction */
		if (!dir) break;

		/* Send the command */
		Send_locate(dir);
	}

	/* Done */
	Send_locate(0);
	
	/* Clear */
	c_msg_print(NULL);
}

void cmd_search(void)
{
	Send_search();
}

void cmd_toggle_search(void)
{
	Send_toggle_search();
}

void cmd_rest(void)
{
	Send_rest();
}

void cmd_go_up(void)
{
	Send_go_up();
}

void cmd_go_down(void)
{
	Send_go_down();
}

void cmd_open(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_open(dir);
}

void cmd_close(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_close(dir);
}

void cmd_bash(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_bash(dir);
}

void cmd_disarm(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	Send_disarm(dir);
}

void cmd_inven(void)
{
	/* The whole screen is "icky" */
	screen_icky = TRUE;

	/* First, erase our current location */

	/* Then, save the screen */
	Term_save();

	command_gap = 50;

	show_inven();

	(void)inkey();

	/* restore the screen */
	Term_load();
	/* print our new location */

	/* The screen is OK now */
	screen_icky = FALSE;

	/* Flush any events */
	Flush_queue();
}

void cmd_equip(void)
{
	/* The whole screen is "icky" */
	screen_icky = TRUE;

	Term_save();

	command_gap = 50;

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	show_equip();

	/* Undo the hack above */
	item_tester_full = FALSE;

	(void)inkey();

	Term_load();

	/* The screen is OK now */
	screen_icky = FALSE;

	/* Flush any events */
	Flush_queue();
}

void cmd_drop(void)
{
	int item, amt;

	if (!c_get_item(&item, "Drop what? ", TRUE, TRUE, FALSE))
	{
		return;
	}

	/* Get an amount */
	if (inventory[item].number > 1)
	{
		amt = c_get_quantity("How many? ", inventory[item].number);
	}
	else amt = 1;

	/* Send it */
	Send_drop(item, amt);
}

void cmd_drop_gold(void)
{
	s32b amt = 0;

	/* Get how much */
	if (p_ptr->au)
		amt = c_get_quantity("How much gold? ", p_ptr->au);

	/* Send it */
	if (amt)
		Send_drop_gold(amt);
}

void cmd_wield(void)
{
	int item;

	if (!c_get_item(&item, "Wear/Wield which item? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_wield(item);
}

void cmd_take_off(void)
{
	int item;

	if (!c_get_item(&item, "Takeoff which item? ", TRUE, FALSE, FALSE))
	{
		return;
	}

	/* Send it */
	Send_take_off(item);
}


void cmd_destroy(void)
{
	int item, amt;
	char out_val[160];

	if (!c_get_item(&item, "Destroy what? ", TRUE, TRUE, TRUE))
	{
		return;
	}

	/* Not on-the-floor item */
	if (item >= 0) 
	{
		/* Get an amount */
		if (inventory[item].number > 1)
		{
			amt = c_get_quantity("How many? ", inventory[item].number);
		}
		else amt = 1;
	
		/* Sanity check */
		if (inventory[item].number == amt)
			sprintf(out_val, "Really destroy %s? ", inventory_name[item]);
		else
			sprintf(out_val, "Really destroy %d of your %s? ", amt, inventory_name[item]);
		if (!get_check(out_val)) return;
	
	}
	else 
	{
		/* Get an amount */
		if (floor_item.number > 1)
		{
			amt = c_get_quantity("How many? ", floor_item.number);
		}
		else amt = 1;

		/* Sanity check */
		if (floor_item.number == amt)
			sprintf(out_val, "Really destroy %s? ", floor_name);
		else 
			sprintf(out_val, "Really destroy %d of %s? ", amt, floor_name);
		if (!get_check(out_val)) return;
			
	} 

	/* Send it */
	Send_destroy(item, amt);
}


void cmd_observe(void)
{
	int item;

	if (!c_get_item(&item, "Examine what? ", TRUE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_observe(item);
}


void cmd_inscribe(void)
{
	int item;
	char buf[1024];

	if (!c_get_item(&item, "Inscribe what? ", TRUE, TRUE, TRUE))
	{
		return;
	}

	buf[0] = '\0';

	/* Get an inscription */
	if (get_string("Inscription: ", buf, 59))
		Send_inscribe(item, buf);
}

void cmd_uninscribe(void)
{
	int item;

	if (!c_get_item(&item, "Uninscribe what? ", TRUE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_uninscribe(item);
}

void cmd_describe(void)
{
	int item;
	char buf[80];

	if (!c_get_item(&item, "Describe what? ", TRUE, TRUE, TRUE))
	{
		return;
	}

	buf[0] = '\0';
	
	/* Copy item name */
	if (item < 0) 
		strcpy(buf, floor_name);
	else
		strcpy(buf, inventory_name[item]);		
	
	if (buf[0] != '\0')
				Send_msg(buf);
}

void cmd_spike(void)
{
	int dir;

	if (!c_get_spike()) 
	{
			/* Message */
			c_msg_print("You have no spikes!");
			return;
	}

	get_dir(&dir);

	/* Send it */
	Send_spike(dir);
}

void cmd_quaff(void)
{
	int item;

	item_tester_tval = TV_POTION;

	if (!c_get_item(&item, "Quaff which potion? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_quaff(item);
}

void cmd_read_scroll(void)
{
	int item;

	item_tester_tval = TV_SCROLL;

	if (!c_get_item(&item, "Read which scroll? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_read(item);
}

void cmd_aim_wand(void)
{
	int item, dir;

	item_tester_tval = TV_WAND;

	if (!c_get_item(&item, "Aim which wand? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	get_dir(&dir);
	
	/* Send it */
	if (dir)
		Send_aim(item, dir);
}

void cmd_use_staff(void)
{
	int item;

	item_tester_tval = TV_STAFF;

	if (!c_get_item(&item, "Use which staff? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_use(item);
}

void cmd_zap_rod(void)
{
	int item;

	item_tester_tval = TV_ROD;

	if (!c_get_item(&item, "Use which rod? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_zap(item);
}

void cmd_refill(void)
{
	int item;
	cptr p;

#if 0
	if (inventory[INVEN_LITE].tval == TV_TORCH)
	{
		item_tester_tval = TV_TORCH;
		p = "Refill with which torch? ";
	}

	else if (inventory[INVEN_LITE].tval == TV_LANTERN)
	{
		item_tester_tval = TV_FLASK;
		p = "Refill with which flask? ";
	}

	else
	{
		c_msg_print("Your light cannot be refilled.");
		return;
	}
#endif

	p = "Refill with which light? ";

	if (!c_get_item(&item, p, FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_fill(item);
}

void cmd_eat(void)
{
	int item;

	item_tester_tval = TV_FOOD;

	if (!c_get_item(&item, "Eat what? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	/* Send it */
	Send_eat(item);
}


void cmd_activate(void)
{
	int item;

	if (!c_get_item(&item, "Activate what? ", TRUE, FALSE, FALSE))
	{
		return;
	}

	/* Send it */
	Send_activate(item);
}

int cmd_target_interactive(int mode)
{
	bool done = FALSE;
	char ch;

	cursor_icky = TRUE;
	
	/* Tell the server to init targetting */
	Send_target_interactive(mode, 0);

	while (!done)
	{
		ch = inkey();

		if (target_recall)
		{
			target_recall = FALSE;

			topline_icky = FALSE;
			section_icky_row = section_icky_col = 0;
		}

		if (!ch)
			continue;

		Send_target_interactive(mode, ch);

		switch (ch)
		{
			case 't':
			case '5':
			case '0':
			case '.':
				done = TRUE;
				break;
		}
		if (ch == ESCAPE)
		{
			prt("", 0, 0);
			/* Very Dirty Hack -- Force Redraw */
			prt_player_hack(FALSE);
			prt_map_easy();
			break;
		}
	}

	/* Reset cursor stuff */
	cursor_icky = FALSE;
	Term_consolidate_cursor(FALSE, 0, 0);

	return done;
}

int cmd_target(void)
{
	return cmd_target_interactive(TARGET_KILL);
}

int cmd_target_friendly(void)
{
	return cmd_target_interactive(TARGET_FRND);
}

void cmd_look(void)
{
	(void)cmd_target_interactive(TARGET_LOOK);
}

void cmd_changepass(void) 
{
	char pass1[MAX_PASS_LEN];
	char pass2[MAX_PASS_LEN];
	int pause = 0;
	char ch;
	pass1[0] = '\0';
	pass2[0] = '\0';


	if (get_string_masked("New Password: ", pass1, MAX_PASS_LEN-1)) 
	{
		if (get_string_masked("Confirm It: ", pass2, MAX_PASS_LEN-1)) 
		{
			if (!strcmp(pass1,pass2)) {
				MD5Password(pass1);
				Send_pass(pass1);
				prt(" Password changed [press any key]",0,0);
			} else {
				prt(" Not matching [paused]",0,0);
			}

			while (!pause)
			{
				ch = inkey();
				if (ch) pause = 1;
			}
		}
	}
}

void cmd_character(void)
{
	char ch = 0;
	int done = 0;

	/* Screen is icky */
	screen_icky = TRUE;

	/* Save screen */
	Term_save();

	while (!done)
	{
		/* Display player info */
		display_player(char_screen_mode);

		/* Display message */
		prt("[ESC to quit, h to toggle history, p to change password]", 21, 12);

		/* Wait for key */
		ch = inkey();

		/* Check for "display history" */
		if (ch == 'h' || ch == 'H')
		{
			/* Toggle */
			char_screen_mode++;
			if (char_screen_mode > 2) char_screen_mode = 0;
		}
		
		/* Check for "change password" */
		if (ch == 'p' || ch == 'P') 
		{
			cmd_changepass();
		}

		/* Check for quit */
		if (ch == 'q' || ch == 'Q' || ch == ESCAPE)
		{
			/* Quit */
			done = 1;
		}
	}

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	/* Flush any events */
	Flush_queue();
}



void cmd_interactive()
{
	char ch;
	bool done = FALSE;

	/* Hack -- if the screen is already icky, ignore this command */
	if (screen_icky) return;

	/* The screen is icky */
	screen_icky = TRUE;
	
	special_line_onscreen = TRUE;

	/* Save the screen */
	Term_save();

	/* Send the request */
	Send_interactive(special_line_type);

	/* Wait until we get the whole thing */
	while (!done)
	{
		/* Wait for net input, or a key */
		ch = inkey();

		if (!ch)
			continue;

		Send_term_key(ch);

		/* Check for user abort */
		if (ch == ESCAPE)
			break;
	}

	/* Reload the screen */
	Term_load();

	/* The screen is OK now */
	screen_icky = FALSE;

	/* HACK -- FIXME -- When all special_files are like that!
	special_line_onscreen = FALSE; */
	
	special_line_type = 0;//SPECIAL_FILE_NONE;

	/* Flush any queued events */
	Flush_queue();
	
}
#if 0
void cmd_artifacts(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_ARTIFACT;

	/* Set the header */
	strcpy(special_line_header, "Artifacts");

	/* Call the file perusal */
	peruse_file();
}

void cmd_uniques(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_UNIQUE;
	
	/* Set the header */
	strcpy(special_line_header, "Uniques");

	/* Call the file perusal */
	peruse_file();
}

void cmd_players(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_PLAYER;

	/* Set the header */
	strcpy(special_line_header, "Players");


	/* Call the file perusal */
	peruse_file();
}

void cmd_knowledge(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_KNOWLEDGE;
	
	/* Set the header */
	strcpy(special_line_header, "Knowledge");

	/* Interactive terminal */
	cmd_interactive();
}

void cmd_high_scores(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_SCORES;

	/* Set the header */
	strcpy(special_line_header, "Highscores");

	/* Call the file perusal */
	peruse_file();
}

void cmd_help(void)
{
	/* Set the hook */
	special_line_type = SPECIAL_FILE_HELP;
	
	/* Set the header */
	strcpy(special_line_header, "Help");

	/* Call the file perusal */
	peruse_file();
}
#endif
void cmd_chat()
{
	char com;
	char buf[80];
		
	if (!get_com("Chat command [n - next, p - previous, c - close, o - open]:", &com)) return;
	
	switch (com)
	{
		case 'b':
		case 'p':
		case '4':
			cmd_chat_cycle(-1);
			break;
		case 'f':
		case 'n':
		case '6':
			cmd_chat_cycle(+1);
			break;
		case 'c':
		case 'l':
			cmd_chat_close(view_channel);
			break;
		case 'o':
		case 'j':
			buf[0] = '\0';
			if (get_string("Channel: ", buf, 59))
				Send_chan(buf);
			break;

	}
}
void cmd_chat_close(int n)
{
	char buf[80];
	
	if (n)
	{
		/* Request channel leave */
		if (channels[n].name[0] == '#')
		{
			sprintf(buf,"-%s",channels[n].name);
			Send_chan(buf);
		}
		/* Close locally */
		else
		{
			if (view_channel == n)
				cmd_chat_cycle(-1);
				
			channels[n].name[0] = '\0';
			channels[n].id = 0;
			
			if (p_ptr->main_channel == n)
				p_ptr->main_channel = 0;				
			if (STRZERO(channels[view_channel].name))
				cmd_chat_cycle(+1);
									
			/* Window update */
			p_ptr->window |= PW_MESSAGE_CHAT;
		}
	}
}
void cmd_chat_cycle(int dir)
{
	s16b new_channel = view_channel;
	bool done = FALSE;
	while (!done)
	{
		new_channel += dir;

		if (new_channel > MAX_CHANNELS || new_channel < 0) return;
		if (STRZERO(channels[new_channel].name)) continue; 

		break;
	}
	
	if (new_channel != view_channel)
	{
		/* Set new */
		view_channel = new_channel;
		p_ptr->on_channel[view_channel] = FALSE;
	
		/* Redraw */
		p_ptr->window |= (PW_MESSAGE_CHAT);
	}
}

void cmd_message(void)
{
	//[flm] powerhack to prevent next hack:
	bool refocus_chat = TRUE;
#ifdef USE_SDL
	refocus_chat = FALSE;
#endif
# define PMSG_TERM 4
	// [hack] hack to just change the window focus in WIN32 client
	if (refocus_chat && ang_term[PMSG_TERM]) {
		set_chat_focus();
	} else {
		char buf[60];

		buf[0] = '\0';

		if (get_string("Message: ", buf, 59))
			if (buf[0] != '\0')
				Send_msg(buf);
	};
}

void cmd_party(void)
{
	char i;
	char buf[80];

	/* Screen is icky */
	screen_icky = TRUE;

	/* We are now in party mode */
	party_mode = TRUE;

	/* Save screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Party commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Create a party");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Add a player to party");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Delete a player from party");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Leave your current party");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Specify player to attack");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Make peace");

		/* Show current party status */
		Term_putstr(0, 13, -1, TERM_WHITE, party_info);

		/* Prompt */
		Term_putstr(0, 11, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Create party */
		else if (i == '1')
		{
			/* Get party name */
			if (get_string("Party name: ", buf, 79))
				Send_party(PARTY_CREATE, buf);
		}

		/* Add player */
		else if (i == '2')
		{
			/* Get player name */
			if (get_string("Add player: ", buf, 79))
				Send_party(PARTY_ADD, buf);
		}

		/* Delete player */
		else if (i == '3')
		{
			/* Get player name */
			if (get_string("Delete player: ", buf, 79))
				Send_party(PARTY_DELETE, buf);
		}

		/* Leave party */
		else if (i == '4')
		{
			/* Send the command */
			Send_party(PARTY_REMOVE_ME, "");
		}

		/* Attack player/party */
		else if (i == '5')
		{
			/* Get player name */
			if (get_string("Player/party to attack: ", buf, 79))
				Send_party(PARTY_HOSTILE, buf);
		}

		/* Make peace with player/party */
		else if (i == '6')
		{
			/* Get player/party name */
			if (get_string("Make peace with: ", buf, 79))
				Send_party(PARTY_PEACE, buf);
		}

		/* Oops */
		else
		{
			/* Ring bell */
			bell();
		}

		/* Flush messages */
		c_msg_print(NULL);
	}

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	/* No longer in party mode */
	party_mode = FALSE;

	/* Flush any events */
	Flush_queue();
}


void cmd_fire(void)
{
	int item, dir;

	if (!c_get_item(&item, "Fire which ammo? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	if (!get_dir(&dir))
		return;
	
	/* Send it */
	Send_fire(item, dir);
}

void cmd_throw(void)
{
	int item, dir;

	if (!c_get_item(&item, "Throw what? ", FALSE, TRUE, TRUE))
	{
		return;
	}

	if (!get_dir(&dir))
		return;

	/* Send it */
	Send_throw(item, dir);
}

void cmd_browse(void)
{
	int item;

	if (p_ptr->ghost)
	{
		show_browse(10);
		return;
	}

	if (!c_info[class].spell_book)
	{
		c_msg_print("You cannot read books!");
		return;
	}

	item_tester_tval = c_info[class].spell_book;

	if (!c_get_item(&item, "Browse which book? ", FALSE, TRUE, FALSE))
	{
		if (item == -2) c_msg_print("You have no books that you can read.");
		return;
	}

	/* Show it */
	show_browse(item);
}

void cmd_study(void)
{
	int item;

	if (!c_info[class].spell_book)
	{
		c_msg_print("You cannot gain spells!");
		return;
	}

	item_tester_tval = c_info[class].spell_book;

	if (!c_get_item(&item, "Gain from which book? ", FALSE, TRUE, FALSE))
	{
		if (item == -2) c_msg_print("You have no books that you can read.");
		return;
	}

	/* Pick a spell and do it */
	do_study(item);
}

void cmd_cast(void)
{
	int item;

   if (c_info[class].spell_book != TV_MAGIC_BOOK)
	{
		c_msg_print("You cannot cast spells!");
		return;
	}

	item_tester_tval = TV_MAGIC_BOOK;

	if (!c_get_item(&item, "Cast from what book? ", FALSE, TRUE, FALSE))
	{
		if (item == -2) c_msg_print("You have no books that you can cast from.");
		return;
	}

	/* Pick a spell and do it */
	do_cast(item);
}

void cmd_pray(void)
{
	int item;

	if (c_info[class].spell_book != TV_PRAYER_BOOK)
	{
		c_msg_print("Pray hard enough and your prayers may be answered.");
		return;
	}

	item_tester_tval = TV_PRAYER_BOOK;

	if (!c_get_item(&item, "Pray from what book? ", FALSE, TRUE, FALSE))
	{
		if (item == -2) c_msg_print("You have no books that you can pray from.");
		return;
	}

	/* Pick a spell and do it */
	do_pray(item);
}

void cmd_ghost(void)
{
	if (p_ptr->ghost)
		do_ghost();
	else
	{
		c_msg_print("You are not undead.");
	}
}

void cmd_load_pref(void)
{
	char buf[80];

	buf[0] = '\0';

	if (get_string("Action: ", buf, 79))
		process_pref_file_command(buf);
}

void cmd_redraw(void)
{
	Send_redraw();
	//keymap_init();
}

void cmd_suicide(void)
{
	int i;

	/* Verify */
	if (!get_check("Do you really want to commit suicide? ")) return;

	/* Check again */
	prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
	flush();
	i = inkey();
	prt("", 0, 0);
	if (i != '@') return;

	/* Send it */
	Send_suicide();
}

void cmd_master_aux_level(void)
{
	char i;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Level commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Static your current level");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Unstatic your current level");

		/* Prompt */
		Term_putstr(0, 8, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* static the current level */
		else if (i == '1')
		{
			Send_master(MASTER_LEVEL, "s");
		}

		/* unstatic the current level */
		else if (i == '2')
		{
			Send_master(MASTER_LEVEL, "u");
		}

		/* Oops */
		else
		{
			/* Ring bell */
			bell();
		}

		/* Flush messages */
		c_msg_print(NULL);
	}
}
void cmd_master_aux_generate_item(void)
{
	char i, redo_hack;
	char buf[80];
	s32b tmp_quan;
	/* Process requests until done */
	
	/* Clear screen */
	Term_clear();
	
	/* Inform server about cleared screen */
	Send_master(MASTER_GENERATE, "ir");		
		
	while (1)
	{
		redo_hack = 0;
		
		/* Initialize buffer */
		buf[0] = 'i';
		buf[1] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Generate Item");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) By number" );
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) By name" );
		Term_putstr(5, 6, -1, TERM_WHITE, "(+) Next Item" );
		Term_putstr(5, 7, -1, TERM_WHITE, "(-) Previous Item" );
		Term_putstr(5, 8, -1, TERM_WHITE, "(>) Next Ego" );
		Term_putstr(5, 9, -1, TERM_WHITE, "(<) Previous Ego" );
		
		Term_putstr(50, 4, -1, TERM_WHITE, "(h)it, (d)am," );
		Term_putstr(50, 5, -1, TERM_WHITE, "(a)c,  (p)val," );
		Term_putstr(50, 6, -1, TERM_WHITE, "(x)tra2, " );
		Term_putstr(50, 7, -1, TERM_WHITE, "(i)dentified," );
		Term_putstr(50, 8, -1, TERM_WHITE, "(b)est kind" );
		
		// This is very confusing.
//		Term_putstr(21, 8, -1, TERM_WHITE, "item/ego - SHIFT - inc/decr " );
//		Term_putstr(34, 9, -1, TERM_WHITE, "|" );
//		Term_putstr(32, 10, -1, TERM_WHITE, "force" );
		

//		/* Prompt */
//		Term_putstr(0, 12, -1, TERM_WHITE, "Command: ");


		Term_putstr(5, 10, -1, TERM_WHITE, "(g) Generate");
		
		Term_putstr(0, 15, -1, TERM_WHITE, "Selection: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;
		else if (i == 'b')
		{
			buf[1] = 'b';
			buf[2] = 'k';
		}
		else if (i == 'B')
		{
			buf[1] = 'b';
			buf[2] = 'e';
		}
		else if (i == 'H' || i == 'D' || i == 'A' || i == 'X' || i == 'P' || i == 'I')
		{
			buf[1] = 'M';
			buf[2] = tolower(i);
		}		
		else if (i == 'h' || i == 'd' || i == 'a' || i == 'x' || i == 'p' || i == 'i')
		{
			buf[1] = 'I';
			buf[2] = i;
		}		
		else if (i == 'g')
		{
			buf[1] = 'd';
			buf[2] = c_get_quantity("How much? ", 127);
			if(!buf[2]) redo_hack = 1;
			buf[3] = 0;
		}
		else if (i == 'G')
		{
			buf[1] = 'd';
			buf[2] = 1;
			buf[3] = 0;
		}
		else if (i == '+')
		{
			/* Next Item*/
			buf[1] = 'k';
			buf[2] = '+';
		}
		else if (i == '-')
		{
			/* Prev. Item */
			buf[1] = 'k';
			buf[2] = '-';
		}
		else if (i == '>')
		{
			/* Next Ego */
			buf[1] = 'e';
			buf[2] = '+';
		}
		else if (i == '<')
		{
			/* Prev. Ego */
			buf[1] = 'e';
			buf[2] = '-';
		}
		else if (i == '1')
		{
			/* Kind by number */
			buf[1] = 'k';
			buf[2] = '#';
			tmp_quan = c_get_quantity("Item number? ", MAX_K_IDX);
			if (tmp_quan > 255) 
			{buf[3] = tmp_quan-255;buf[4] = tmp_quan-(tmp_quan-255);}
			else
			{buf[3] = tmp_quan;buf[4] = 0;}
			if(!tmp_quan) redo_hack = 1;
			buf[5] = 0;
			
		}
		else if (i == '2')
		{
			/* Kind by name */
			buf[1] = 'k';
			buf[2] = 'n';
			get_string("Enter item name: ", &buf[3], 79);
			if(!buf[3]) redo_hack = 1;
		}
		else if (i == '!')
		{
			/* Ego by number */
			buf[1] = 'e';
			buf[2] = '#';
			buf[3] = c_get_quantity("EGO id? ", MAX_E_IDX);
			if(!buf[3]) redo_hack = 1;
			buf[4] = 0;
		}
		else if (i == '@')
		{
			/* Ego by name */
			buf[1] = 'e';
			buf[2] = 'n';
			get_string("Enter ego name: ", &buf[3], 79);
			if(!buf[3]) redo_hack = 1;
		}

		/* Oops */
		else
		{
			/* Ring bell */
			bell(); redo_hack = 1;
		}

		/* hack -- don't do this if we hit an invalid key previously */
		if(redo_hack) continue;
		
		
		/* Clear screen again */
		Term_clear();

		/* Send choice to server */
		Send_master(MASTER_GENERATE, buf);

		/* Flush messages */
		c_msg_print(NULL);
	}
}

void cmd_master_aux_generate_vault(void)
{
	char i, redo_hack;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		redo_hack = 0;
		
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = 'v';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Generate Vault");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) By number");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) By name");

		/* Prompt */
		Term_putstr(0, 8, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Generate by number */
		else if (i == '1')
		{
			buf[1] = '#';
			buf[2] = c_get_quantity("Vault number? ", 127);
			if(!buf[2]) redo_hack = 1;
			buf[3] = 0;
		}
		
		/* Generate by name */
		else if (i == '2')
		{
			buf[1] = 'n';
			get_string("Enter vault name: ", &buf[2], 79);
			if(!buf[2]) redo_hack = 1;
		}

		/* Oops */
		else
		{
			/* Ring bell */
			bell(); redo_hack = 1;
		}

		/* hack -- don't do this if we hit an invalid key previously */
		if(redo_hack) continue;

		Send_master(MASTER_GENERATE, buf);

		/* Flush messages */
		c_msg_print(NULL);
	}
}

void cmd_master_aux_generate(void)
{
	char i;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Generation commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Vault");

		/* Selections */
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Item");

		/* Prompt */
		Term_putstr(0, 7, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Generate a vault */
		else if (i == '1')
		{
			cmd_master_aux_generate_vault();
		}
		else if (i == '2')
		{
			cmd_master_aux_generate_item();
		}



		/* Oops */
		else
		{
			/* Ring bell */
			bell();
		}

		/* Flush messages */
		c_msg_print(NULL);
	}
}

void cmd_master_aux_player_inside(void)
{
	char i;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Inform about screen update */
		Send_master(MASTER_PLAYER, ">r");
			
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Player commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(<) Previous");
		Term_putstr(5, 5, -1, TERM_WHITE, "(>) Next");
		Term_putstr(5, 6, -1, TERM_WHITE, "RET Change");
		
		Term_putstr(5, 8, -1, TERM_WHITE, "(g) Ghost");

		/* Prompt */
		Term_putstr(0, 15, -1, TERM_WHITE, "Selection: ");

		/* Get a key */
		i = inkey();
		
		/* Leave */
		if (i == ESCAPE) break;
		
		switch (i)
		{
			case '<': 
				/* Prev Sel */
				buf[0] = '>';
				buf[1] = 'p';
				break;
			case '>': 
				/* Next Sel */
				buf[0] = '>';
				buf[1] = 'n';
				break; 
			case '\r':
				/* Change */
				buf[0] = '>';
				buf[1] = 'x';
				break;
			case 'g':
				/* Toggle Ghost */
				buf[0] = '>';
				buf[1] = 'g';
				break; 
		}	
		
		if (!STRZERO(buf))
		{
			buf[2] = '\0';
			Send_master(MASTER_PLAYER, buf);
		}
	}
}
void cmd_master_aux_player(void)
{
	char i;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Player commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Self");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) By Name");

		/* Prompt */
		Term_putstr(0, 8, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;
		
		if (i == '1')
		{
			Send_master(MASTER_PLAYER, " ");
			cmd_master_aux_player_inside();
		}
		if (i == '2')
		{
			/* Get player name */
			if (get_string("Player: ", buf, 80)) 
			{
				Send_master(MASTER_PLAYER, buf);
				cmd_master_aux_player_inside();
			}
		}
	}
}

void cmd_master_aux_build(void)
{
	char i;
	char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = FEAT_FLOOR;

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Building commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Granite Mode");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Permanent Mode");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Tree Mode");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Evil Tree Mode");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Grass Mode");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Dirt Mode");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Floor Mode");
		Term_putstr(5, 11, -1, TERM_WHITE, "(8) Build Mode Off");

		/* Prompt */
		Term_putstr(0, 14, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		buf[1] = 'T';
		buf[2] = '\0';

		switch (i)
		{
			/* Granite mode on */
			case '1': buf[0] = FEAT_WALL_EXTRA; break;
			/* Perm mode on */
			case '2': buf[0] = FEAT_PERM_EXTRA; break;
			/* Tree mode on */
			case '3': buf[0] = FEAT_TREE; break;
			/* Evil tree mode on */
			case '4': buf[0] = FEAT_EVIL_TREE; break;
			/* Grass mode on */
			case '5': buf[0] = FEAT_GRASS; break;
			/* Dirt mode on */
			case '6': buf[0] = FEAT_DIRT; break;
			/* Floor mode on */
			case '7': buf[0] = FEAT_FLOOR; break;
			/* Build mode off */
			case '8': buf[0] = FEAT_FLOOR; buf[1] = 'F'; break;
			/* Oops */
			default : bell(); break;		
		}

		/* If we got a valid command, send it */
		if (buf[0]) Send_master(MASTER_BUILD, buf);

		/* Flush messages */
		c_msg_print(NULL);
	}
}

char * cmd_master_aux_summon_orcs(void)
{
	char i;
	static char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Summon which orcs?");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Snagas");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Cave Orcs");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Hill Orcs");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Black Orcs");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Half-Orcs");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Uruks");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Random");

		/* Prompt */
		Term_putstr(0, 13, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		buf[0] = '\0';

		/* get the type of orc */
		switch (i)
		{
			case '1': strcpy(buf,"Snaga"); break;
			case '2': strcpy(buf,"Cave orc"); break;
			case '3': strcpy(buf,"Cave orc"); break;
			case '4': strcpy(buf,"Black orc"); break;
			case '5': strcpy(buf,"Half-orc"); break;
			case '6': strcpy(buf, "Uruk"); break;
			case '7': strcpy(buf, "random"); break;
			default : bell(); break;
		}

		/* if we got an orc type, return it */
		if (buf[0]) return buf;

		/* Flush messages */
		c_msg_print(NULL);
	}

	/* escape was pressed, no valid orcs types specified */
	return NULL;
}

char * cmd_master_aux_summon_undead_low(void)
{
	char i;
	static char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Summon which low undead?");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Poltergeist");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Green glutton ghost");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Lost soul");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Skeleton kobold");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Skeleton orc");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Skeleton human");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Zombified orc");
		Term_putstr(5, 11, -1, TERM_WHITE, "(8) Zombified human");
		Term_putstr(5, 12, -1, TERM_WHITE, "(9) Mummified orc");
		Term_putstr(5, 13, -1, TERM_WHITE, "(a) Moaning spirit");
		Term_putstr(5, 14, -1, TERM_WHITE, "(b) Vampire bat");
		Term_putstr(5, 15, -1, TERM_WHITE, "(c) Random");

		/* Prompt */
		Term_putstr(0, 18, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		buf[0] = '\0';

		/* get the type of undead */
		switch (i)
		{
			case '1': strcpy(buf,"Poltergeist"); break;
			case '2': strcpy(buf,"Green glutton ghost"); break;
			case '3': strcpy(buf,"Loust soul"); break;
			case '4': strcpy(buf,"Skeleton kobold"); break;
			case '5': strcpy(buf,"Skeleton orc"); break;
			case '6': strcpy(buf, "Skeleton human"); break;
			case '7': strcpy(buf, "Zombified orc"); break;
			case '8': strcpy(buf, "Zombified human"); break;
			case '9': strcpy(buf, "Mummified orc"); break;
			case 'a': strcpy(buf, "Moaning spirit"); break;
			case 'b': strcpy(buf, "Vampire bat"); break;
			case 'c': strcpy(buf, "random"); break;

			default : bell(); break;
		}

		/* if we got an undead type, return it */
		if (buf[0]) return buf;

		/* Flush messages */
		c_msg_print(NULL);
	}

	/* escape was pressed, no valid types specified */
	return NULL;
}


char * cmd_master_aux_summon_undead_high(void)
{
	char i;
	static char buf[80];

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Summon which high undead?");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Vampire");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Giant Skeleton troll");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Lich");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Master vampire");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Dread");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Nether wraith");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Night mare");
		Term_putstr(5, 11, -1, TERM_WHITE, "(8) Vampire lord");
		Term_putstr(5, 12, -1, TERM_WHITE, "(9) Archpriest");
		Term_putstr(5, 13, -1, TERM_WHITE, "(a) Undead beholder");
		Term_putstr(5, 14, -1, TERM_WHITE, "(b) Dreadmaster");
		Term_putstr(5, 15, -1, TERM_WHITE, "(c) Nightwing");
		Term_putstr(5, 16, -1, TERM_WHITE, "(d) Nightcrawler");
		Term_putstr(5, 17, -1, TERM_WHITE, "(e) Random");

		/* Prompt */
		Term_putstr(0, 20, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		buf[0] = '\0';

		/* get the type of undead */
		switch (i)
		{
			case '1': strcpy(buf,"Vampire"); break;
			case '2': strcpy(buf,"Giant skeleton troll"); break;
			case '3': strcpy(buf,"Lich"); break;
			case '4': strcpy(buf,"Master vampire"); break;
			case '5': strcpy(buf,"Dread"); break;
			case '6': strcpy(buf, "Nether wraith"); break;
			case '7': strcpy(buf, "Night mare"); break;
			case '8': strcpy(buf, "Vampire lord"); break;
			case '9': strcpy(buf, "Archpriest"); break;
			case 'a': strcpy(buf, "Undead beholder"); break;
			case 'b': strcpy(buf, "Dreadmaster"); break;
			case 'c': strcpy(buf, "Nightwing"); break;
			case 'd': strcpy(buf, "Nightcrawler"); break;
			case 'e': strcpy(buf, "random"); break;

			default : bell(); break;
		}

		/* if we got an undead type, return it */
		if (buf[0]) return buf;

		/* Flush messages */
		c_msg_print(NULL);
	}

	/* escape was pressed, no valid orcs types specified */
	return NULL;
}


void cmd_master_aux_summon(void)
{
	char i, redo_hack;
	char buf[80];
	char * race_name;

	/* Process requests until done */
	while (1)
	{
		redo_hack = 0;

		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Summon . . .");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Orcs");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Low Undead");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) High Undead");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Depth");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Specific");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Mass Genocide");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Summoning mode off");



		/* Prompt */
		Term_putstr(0, 13, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* get the type of monster to summon */
		switch (i)
		{
			/* orc menu */
			case '1': 
			{
				/* get the specific kind of orc */
				race_name = cmd_master_aux_summon_orcs();
				/* if no string was specified */
				if (!race_name) 
				{
					redo_hack = 1;
					break;
				}
				buf[2] = 'o';
				strcpy(&buf[3], race_name);
				break;
			}
			/* low undead menu */
			case '2': 
			{	/* get the specific kind of low undead */
				race_name = cmd_master_aux_summon_undead_low();
				/* if no string was specified */
				if (!race_name)
				{
					redo_hack = 1;
					break;
				}
				buf[2] = 'u';
				strcpy(&buf[3], race_name);
				break;
			}/* high undead menu */
			case '3': 
			{	/* get the specific kind of low undead */
				race_name = cmd_master_aux_summon_undead_high();
				/* if no string was specified */
				if (!race_name)
				{
					redo_hack = 1;
					break;
				}
				buf[2] = 'U';
				strcpy(&buf[3], race_name);
				break;
			}
			/* summon from a specific depth */
			case '4':
			{
				buf[2] = 'd';
				buf[3] = c_get_quantity("Summon from which depth? ", 127);
				if (!buf[3]) redo_hack = 1;
				buf[4] = 0; /* terminate the string */
				break;
			}
			/* summon a specific monster or character */
			case '5':
			{
				buf[2] = 's';
				buf[3] = 0;
				get_string("Enter (partial) monster name: ", &buf[3], 79);
				if (!buf[3]) redo_hack = 1;
				break;
			}

			case '6':
			{
				/* delete all the monsters near us */
				/* turn summoning mode on */
				buf[0] = 'T';
				buf[1] = 1;
				buf[2] = '0';
				buf[3] = '\0'; /* null terminate the monster name */
				Send_master(MASTER_SUMMON, buf);

				redo_hack = 1;
				break;
			}	

			case '7':
			{
				/* disable summoning mode */
				buf[0] = 'F';
				buf[3] = '\0'; /* null terminate the monster name */
				Send_master(MASTER_SUMMON, buf);

				redo_hack = 1;
				break;
			}	

			
			/* Oops */
			default : bell(); redo_hack = 1; break;
		}

		/* get how it should be summoned */

		/* hack -- make sure our method is unset so we only send 
		 * a monster summon request if we get a valid summoning type
		 */

		/* hack -- don't do this if we hit an invalid key previously */
		if (redo_hack) continue;

		while (1)
		{
			/* make sure we get a valid summoning type before summoning */
			buf[0] = 0;

			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(0, 2, -1, TERM_WHITE, "Summon . . .");

			/* Selections */
			Term_putstr(5, 4, -1, TERM_WHITE, "(1) X here");
			Term_putstr(5, 5, -1, TERM_WHITE, "(2) X at random locations");
			Term_putstr(5, 6, -1, TERM_WHITE, "(3) Group here");
			Term_putstr(5, 7, -1, TERM_WHITE, "(4) Group at random location");
			Term_putstr(5, 8, -1, TERM_WHITE, "(5) Summoning mode");

			/* Prompt */
			Term_putstr(0, 10, -1, TERM_WHITE, "Command: ");

			/* Get a key */
			i = inkey();

			/* Leave */
			if (i == ESCAPE) break;

			/* get the type of summoning */
			switch (i)
			{
				/* X here */
				case '1': 
				{
					buf[0] = 'x';
					buf[1] = c_get_quantity("Summon how many? ", 127);
					break;
				}
				/* X in different places */
				case '2':
				{
					buf[0] = 'X';
					buf[1] = c_get_quantity("Summon how many? ", 127);
					break;
				}
				/* Group here */
				case '3':
				{
					buf[0] = 'g';
					break;
				}
				/* Group at random location */
				case '4':
				{
					buf[0] = 'G';
					break;
				}
				/* summoning mode on */
				case '5':
				{
					buf[0] = 'T';
					buf[1] = 1;
					break;
				}

				/* Oops */
				default : bell(); redo_hack = 1; break;
			}
		/* if we have a valid summoning type (escape was not just pressed)
		 * then summon the monster */
		if (buf[0]) Send_master(MASTER_SUMMON, buf);
		}


		/* Flush messages */
		c_msg_print(NULL);
	}
}



/* Dungeon Master commands */
void cmd_master(void)
{
	char i;
	char buf[80];

	/* Screen is icky */
	screen_icky = TRUE;

	party_mode = TRUE;

	/* Save screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Dungeon Master commands");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Level Commands");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Building Commands");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Summoning Commands");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Generation Commands");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Player Commands");

		/* Prompt */
		Term_putstr(0, 11, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Level commands */
		else if (i == '1')
		{
			cmd_master_aux_level();
		}

		/* Build commands */
		else if (i == '2')
		{
			cmd_master_aux_build();
		}

		/* Summon commands */
		else if (i == '3')
		{
			cmd_master_aux_summon();
		}
		
		/* Generate commands */
		else if (i == '4')
		{
			cmd_master_aux_generate();
		}

		/* Player commands */
		else if (i == '5')
		{
			cmd_master_aux_player();
		}

		/* Oops */
		else
		{
			/* Ring bell */
			bell();
		}

		/* Flush messages */
		c_msg_print(NULL);
	}

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	/* No longer in party mode */
	party_mode = FALSE;

	/* Flush any events */
	Flush_queue();
}
