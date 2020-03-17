#include "c-angband.h"
#include "net-client.h"
#include "../common/md5.h"

char* item_prompt(cptr prompt, int item, int amt);

/* Handle custom commands */
#define advance_prompt() prompt = prompt + strlen(prompt) + 1
void cmd_custom(byte i)
{
	custom_command_type *cc_ptr;
	char dir;
	int item, item2;
	s32b value;
	cptr prompt;
	char entry[60];
	bool need_second, need_target;
	byte second_item_tester = 0;

	/* Byte is always 0, check if its > max */
	if (i > custom_commands) return;
	cc_ptr = &custom_command[i];
	dir = item = item2 = value = 0;
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
		advance_prompt();
	}
	if (cc_ptr->flag & COMMAND_TEST_DEAD)
	{
		if (!p_ptr->ghost)
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		advance_prompt();
	}
	if (cc_ptr->flag & COMMAND_TEST_SPELL)
	{
		if (c_info[pclass].spell_book != cc_ptr->tval)
		{
			if (!STRZERO(prompt)) c_msg_print(prompt);
			return;
		}
		advance_prompt();
	}
	/* if (cc_ptr->flag & COMMAND_SPECIAL_FILE)
	{
		special_line_type = cc_ptr->tval;
		strcpy(special_line_header, prompt);
		peruse_file();
		return;
	}
	else */ if (cc_ptr->flag & COMMAND_INTERACTIVE)
	{
		int line_type = cc_ptr->tval;
		bool use_anykey = (cc_ptr->flag & COMMAND_INTERACTIVE_ANYKEY) ? TRUE : FALSE;
		strcpy(special_line_header, prompt);
		cmd_interactive(line_type, use_anykey);
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
		advance_prompt();
	}
	/* Ask for a store item (interactive) ? */
	else if (cc_ptr->flag & COMMAND_ITEM_STORE)
	{
		if (!get_store_stock(&item, prompt)) return;
		advance_prompt();

		/* Get an amount */
		if ((cc_ptr->flag & COMMAND_ITEM_AMMOUNT))
		{
			value = 1;
			/* - from stock */
			if (store.stock[item].number > 1)
			{
				/* Hack -- note cost of "fixed" items */
				if (store_num != 7)
					c_msg_print(format("That costs %ld gold per item.", (long)store_prices[item]));

				if (STRZERO(prompt)) prompt = "How many? ";
				shopping_buying = TRUE;
				value = c_get_quantity(prompt, store.stock[item].number);
				shopping_buying = FALSE;
			}
			if (!value) return;
			advance_prompt();
        }

		/* Dirty Hack -- save multiplied price as "entry" */
		sprintf(entry, "%" PRId32, (u32b)(store_prices[item]*value));
	}
	/* Ask for an item (interactive) ? */
	else if (cc_ptr->flag & COMMAND_NEED_ITEM)
	{
		item_tester_tval = cc_ptr->tval;
		spellcasting = (cc_ptr->flag & COMMAND_SPELL_BOOK) ? TRUE : FALSE;
		if (!c_get_item(&item, prompt,
				(cc_ptr->flag & COMMAND_ITEM_EQUIP ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_ITEM_INVEN ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_ITEM_FLOOR ? TRUE : FALSE)))
				return;
		second_item_tester = c_secondary_tester(item);
		advance_prompt();

		/* Get an amount */
		if ((cc_ptr->flag & COMMAND_ITEM_AMMOUNT))
		{
			value = 1;
			/* - from inventory */
			if (item >= 0 && inventory[item].number > 1)
			{
				if (STRZERO(prompt)) prompt = "How many? ";
				value = c_get_quantity(prompt, inventory[item].number);
			}
			/* - from floor */
			if (item < 0 && floor_item.number > 1)
			{
				if (STRZERO(prompt)) prompt = "How many? ";
				value = c_get_quantity(prompt, floor_item.number);
			}
			if (!value) return;
			advance_prompt();
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
		advance_prompt();
		if (cc_ptr->flag & COMMAND_SPELL_BOOK)
		{
			if (!get_spell(&spell, p, prompt, &item, FALSE, FALSE)) return;
			index = item * SPELLS_PER_BOOK + spell;
		}
		else
		{
			int book = cc_ptr->tval;
			if (!get_spell(&spell, p, prompt, &book, FALSE, TRUE)) return;
			index = book * SPELLS_PER_BOOK + spell;
			indoff = cc_ptr->tval * SPELLS_PER_BOOK;
		}
		advance_prompt();
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
			if (spell >= SPELL_PROJECTED) need_target = TRUE;
			else
			{
				need_target = (spell_flag[index] & PY_SPELL_AIM  ? TRUE : FALSE);
				need_second = (spell_flag[index] & PY_SPELL_ITEM ? TRUE : FALSE);
				if (need_second) second_item_tester = spell_test[index];
			}
		}
	} 
	/* Second item? */
	if (need_second) /* cc_ptr->flag & COMMAND_NEED_SECOND) */
	{
		if (STRZERO(prompt)) prompt = "Which item? ";
		item_tester_tval = second_item_tester;
		if (!c_get_item(&item2, prompt,
				(cc_ptr->flag & COMMAND_SECOND_EQUIP ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_SECOND_INVEN ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_SECOND_FLOOR ? TRUE : FALSE)))
				return;
		advance_prompt();
	}
	/* Target? */
	if (need_target) /* cc_ptr->flag & COMMAND_NEED_TARGET) */
	{
		if (!c_get_dir(&dir, prompt,
				(cc_ptr->flag & COMMAND_TARGET_ALLOW ? TRUE : FALSE),
				(cc_ptr->flag & COMMAND_TARGET_FRIEND ? TRUE : FALSE)))
				return;
		advance_prompt();
	}
	/* Auto-modify prompt? */
	if (cc_ptr->flag & COMMAND_PROMPT_ITEM)
	{
		prompt = item_prompt(prompt, item, value);
	}
	/* Need values? */
	if (cc_ptr->flag & COMMAND_NEED_VALUE)
	{
		if (STRZERO(prompt)) prompt = "Quantity: ";
		value = c_get_quantity(prompt, 999000000);
		if (!value) return;
		advance_prompt();
	}
	if (cc_ptr->flag & COMMAND_NEED_CHAR)
	{
		if (STRZERO(prompt)) prompt = "Command: ";
		if (!get_com(prompt, &entry[0])) 
			return;
		entry[1] = '\0';
		advance_prompt();
	}
	else if (cc_ptr->flag & COMMAND_NEED_STRING)
	{
		if (STRZERO(prompt)) prompt = "Entry: ";
		if (!get_string(prompt, entry, sizeof(entry) - 1))
			return;
		advance_prompt();
	}
	if (cc_ptr->flag & COMMAND_NEED_CONFIRM)
	{
		if (STRZERO(prompt)) prompt = "Really perform said action ? ";
		if (!get_check(prompt))
			return;
		advance_prompt();
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


	send_custom_command(i, item, dir, value, entry);
}
/* Handle all commands */
void process_command()
{
	byte i;
	for (i = 0; i < custom_commands; i++) 
	{
		if (custom_command[i].flag & COMMAND_STORE) continue;
		if (custom_command[i].m_catch == command_cmd)
		{
			cmd_custom(i);
			return;
		}
	}

#ifndef MOBILE_UI
	/* Hack -- pick command from a menu */
	if (command_cmd == '\r')
	{
		command_cmd = do_cmd_menu();
		if (command_cmd != '\r')
		{
			process_command();
			return;
		}
	}
#endif

	/* Parse the command */
	switch (command_cmd)
	{
		/* Ignore */
		case ' ':
		{
			msg_flush();
		}
		case ESCAPE:
		{
			if (first_escape) 
				send_clear();
			first_escape = FALSE;
			break;
		}

		/* Ignore return */
		case '\r':
		{
			break;
		}

		/*** Movement Commands ***/
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
		/* Recenter map */
		case 'L':
		{
			cmd_locate();
			break;
		}
		/* Rest */
		case 'R':
		{
			cmd_rest();
			break;
		}
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
#if 0		
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
#endif
		case 'U':
		{
			cmd_ghost();
			break;
		}

		case KTRL('U'):
		{
			cmd_use_item();
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

		case KTRL('O'): /* Repeat last message */
		{
			do_cmd_message_one();
			break;
		}

		case KTRL('P'):
		{
	        do_cmd_messages();
	        break;
		}

		case KTRL('X'):
		{
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

		case '\xff':
		{
			cmd_mouseclick();
			break;
		}

		case KTRL('E'):
		{
			toggle_inven_equip();
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

#ifndef MOBILE_UI
		case '%':
		{
			interact_macros();
			break;
		}
#endif

		case '!':
		{
			do_cmd_port();
			break;
		}

		default:
		{
			prt("Hit '?' for help.", 0, 0);
			break;
		}
	}
}



/* Process server-side requests. We queue the actual requests in net-client.c
 * and process them here at a later time, so we don't unexpectedly change state
 * in the middle of a network frame. */
/* Note: this probably should be in some other file, but which? */
void process_requests()
{
	if (pause_requested)
	{
		pause_requested = FALSE;
		section_icky_row = Term->hgt;
		section_icky_col = Term->wid;
		//cmd_interactive();
		prepare_popup(0, TRUE);
	}
	if (local_browser_requested)
	{
		local_browser_requested = FALSE;
		peruse_file();
	}
	if (simple_popup_requested)
	{
		simple_popup_requested = FALSE;
		prepare_popup(0, TRUE);
	}
	if (special_line_requested)
	{
		//int type = special_line_requested;
		special_line_requested = FALSE;
		//cmd_interactive();
		prepare_popup(0, FALSE);
	}
	if (confirm_requested)
	{
		confirm_requested = FALSE;
		if (get_check(confirm_prompt)) send_confirm(confirm_type, confirm_id);
	}
	if (enter_store)
	{
		enter_store = FALSE;
		display_store();
	}
}



void cmd_walk(void)
{
	int dir = command_dir;

	if (!dir)
	{
		get_dir(&dir);
	}

	send_walk(dir);
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

void cmd_locate(void)
{
	int dir;
	char ch;

	/* Initialize */
	send_locate(5);

	/* Show panels until done */
	while (1)
	{
		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			/* Hack -- inform Term2 */
			if (z_ask_dir_aux) z_ask_dir_aux("Locate", FALSE, FALSE);

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
		send_locate(dir);
	}

	/* Done */
	send_locate(0);
	
	/* Clear */
	c_msg_print(NULL);
}

void cmd_rest(void)
{
	send_rest();
}

void cmd_inven(void)
{
	/* show_inven() might not show anything, yet we still pause the screen,
	 * using inkey() below. To avoid all that altogether, let's quit early */
	if (inventory[0].number == 0) return;

	/* Save the screen */
	Term_save();

	command_gap = 50;

	/* Show inven and *make screen icky* */
	show_inven();

	/* Pause */
	(void)inkey();

	/* Restore the screen */
	Term_load();

	/* The screen is OK now */
	section_icky_row = 0;
	section_icky_col = 0;

	/* Flush any events */
	Flush_queue();
}

void cmd_equip(void)
{
	/* Save the screen */
	Term_save();

	command_gap = 50;

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Show equip and *make screen icky */
	show_equip();

	/* Undo the hack above */
	item_tester_full = FALSE;

	(void)inkey();

	Term_load();

	/* The screen is OK now */
	section_icky_row = 0;
	section_icky_col = 0;

	/* Flush any events */
	Flush_queue();
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

/* Given a 'prompt' prefix, an 'item' id and ammount ('amt'), 
 * return a prompt string describing such item. */ 
char* item_prompt(cptr prompt, int item, int amt)
{
	static char out_val[160];

	/* Inventory/Equipment item */
	if (item >= 0)
	{
		/* Whole stack */
		if (inventory[item].number == amt)
			sprintf(out_val, "%s%s? ", prompt, inventory_name[item]);
		/* Part of stack */
		else
			sprintf(out_val, "%s%d of your %s? ", prompt, amt, inventory_name[item]);
	}
	/* Floor item */
	else
	{
		/* Whole stack */
		if (floor_item.number == amt)
			sprintf(out_val, "%s%s? ", prompt, floor_name);
		/* Part of stack */
		else
			sprintf(out_val, "%s%d of %s? ", prompt, amt, floor_name);
	}

	return out_val;
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
		send_msg(buf);
}

int cmd_target_interactive(int mode)
{
	bool done = FALSE;
	event_type ke;
	char ch;

	/* Save screen */
	Term_save();

	/* Set modes */
	looking = TRUE;
	target_recall = FALSE;
	cursor_icky = TRUE;
	topline_icky = TRUE;

	/* Tell the server to init targetting */
	Send_target_interactive(mode, 0);

	while (!done)
	{
		ke = inkey_ex();
		ch = ke.key;

		if (!ch)
			continue;

		if (ch == '\xff')
		{
			send_mouse(MCURSOR_META | mode
			  | (ke.index ? MCURSOR_KTRL : 0),
			  ke.mousex - DUNGEON_OFFSET_X,
			  ke.mousey - DUNGEON_OFFSET_Y);
			if (ke.index) done = TRUE;
			continue;
		}

		Send_target_interactive(mode, ch);

		switch (ch)
		{
			case 't':
			case '5':
			case '0':
			case '.':
			case 'g':
			case ESCAPE:
				done = TRUE;
				break;
		}
	}

	/* Fix screen */
	Term_load();

	/* Unset modes */
	looking = FALSE;
	topline_icky = FALSE;
	section_icky_row = 0;
	section_icky_col = 0;

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
				send_pass(pass1);
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

	u32b old_window;
	u32b tmp_window;

	old_window = window_flag[0];
	tmp_window = window_flag[0];

	tmp_window &= ~PW_PLAYER_2;
	tmp_window &= ~PW_STATUS;

	/* Screen is icky */
	screen_icky = TRUE;

	/* Save screen */
	Term_save();

	while (!done)
	{
		if (char_screen_mode == 0) window_flag[0] = tmp_window | PW_PLAYER_0;
		if (char_screen_mode == 1) window_flag[0] = tmp_window | PW_PLAYER_3;
		if (char_screen_mode == 2) window_flag[0] = tmp_window | PW_PLAYER_1;

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

	window_flag[0] = old_window;

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	redraw_indicators(old_window);

	/* Flush any events */
	Flush_queue();
}


void cmd_interactive(byte line_type, bool use_anykey)
{
	char ch;
	bool done = FALSE;

	/* Hack -- if the screen is already icky, ignore this command */
	if (screen_icky) return;

	/* The screen is icky */
	screen_icky = TRUE;
	special_line_onscreen = TRUE;
	//special_line_type = line_type;

	/* Save the screen */
	Term_save();

	/* Send the request */
	send_interactive(line_type);

	/* Wait until we get the whole thing */
	while (!done)
	{
		/* Wait for net input, or a key */
		ch = inkey();

		if (!ch)
			continue;

		if (use_anykey) ch = ESCAPE;

		send_term_key(ch);

		/* Check for user abort */
		if (ch == ESCAPE && !icky_levels)
			break;
	}

	/* Reload the screen */
	Term_load();

	/* The screen is OK now */
	screen_icky = FALSE;
	special_line_onscreen = FALSE;
	//special_line_type = 0;

	/* Flush any queued events */
	Flush_queue();
}

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
			if (!get_string("Channel: ", buf, 59)) break;

			/* Request channel join */
			if (buf[0] == '#')
			{
				send_channel(CHAN_JOIN, 0, buf);
			}
			/* Open tab locally */
			else
			{
				send_channel(CHAN_SELECT, 0, buf);

				do_chat_open(MAX_CHANNELS, buf);
			}

			break;

	}
}
void cmd_chat_close(int n)
{
	
	if (n)
	{
		/* Request channel leave */
		if (channels[n].name[0] == '#')
		{
			send_channel(CHAN_LEAVE, channels[n].id, channels[n].name); 
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

		if (new_channel >= MAX_CHANNELS || new_channel < 0) return;
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
	bool refocus_chat = FALSE;
#ifdef USE_WIN
#ifdef PMSG_TERM
	refocus_chat = win32_window_visible(PMSG_TERM) ? TRUE : FALSE;
#endif
#endif
	// [hack] hack to just change the window focus in WIN32 client
	if (refocus_chat)
	{
		set_chat_focus();
	}
	else
	{
		char buf[60];

		buf[0] = '\0';

		if (get_string("Message: ", buf, 59))
			if (buf[0] != '\0')
				send_msg(buf);
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
		Term_show_ui_cursor();

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Create party */
		else if (i == '1')
		{
			/* Get party name */
			if (get_string("Party name: ", buf, 79))
				send_party(PARTY_CREATE, buf);
		}

		/* Add player */
		else if (i == '2')
		{
			/* Get player name */
			if (get_string("Add player: ", buf, 79))
				send_party(PARTY_ADD, buf);
		}

		/* Delete player */
		else if (i == '3')
		{
			/* Get player name */
			if (get_string("Delete player: ", buf, 79))
				send_party(PARTY_DELETE, buf);
		}

		/* Leave party */
		else if (i == '4')
		{
			/* Send the command */
			send_party(PARTY_REMOVE_ME, "");
		}

		/* Attack player/party */
		else if (i == '5')
		{
			/* Get player name */
			if (get_string("Player/party to attack: ", buf, 79))
				send_party(PARTY_HOSTILE, buf);
		}

		/* Make peace with player/party */
		else if (i == '6')
		{
			/* Get player/party name */
			if (get_string("Make peace with: ", buf, 79))
				send_party(PARTY_PEACE, buf);
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

	Term_hide_ui_cursor();

	/* Reload screen */
	Term_load();

	/* Screen is no longer icky */
	screen_icky = FALSE;

	/* No longer in party mode */
	party_mode = FALSE;

	/* Flush any events */
	Flush_queue();
}


void cmd_browse(void)
{
	int item;

	if (p_ptr->ghost)
	{
		show_browse(10);
		return;
	}

	if (!c_info[pclass].spell_book)
	{
		c_msg_print("You cannot read books!");
		return;
	}

	item_tester_tval = c_info[pclass].spell_book;
	spellcasting = TRUE;
	if (!c_get_item(&item, "Browse which book? ", FALSE, TRUE, FALSE))
	{
		if (item == -2) c_msg_print("You have no books that you can read.");
		return;
	}

	/* Show it */
	show_browse(item);
}
#if 0
void cmd_study(void)
{
	int item;

	if (!c_info[pclass].spell_book)
	{
		c_msg_print("You cannot gain spells!");
		return;
	}

	item_tester_tval = c_info[pclass].spell_book;

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

	if (c_info[pclass].spell_book != TV_MAGIC_BOOK)
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

	if (c_info[pclass].spell_book != TV_PRAYER_BOOK)
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
#endif
void cmd_ghost(void)
{
	if (p_ptr->ghost)
		do_ghost();
	else
	{
		c_msg_print("You are not undead.");
	}
}

void toggle_inven_equip(void)
{
	flip_inven = !flip_inven;
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
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
	/* Request new data */
	send_redraw();
	/* Clear screen */
	Term_clear();
}

void cmd_suicide(void)
{
	int i;

	/* Verify */
	if (!get_check("Do you really want to commit suicide? ")) return;

	/* Check again */
	topline_icky = TRUE;
	prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
	flush();
	i = inkey();
	topline_icky = FALSE;
	prt("", 0, 0);
	if (i != '@') return;

	/* Send it */
	send_suicide();
}

void cmd_mouseclick()
{
	event_type ke = command_cmd_ex;
	int btn, mod = 0;
	btn = ke.index;
	if (btn & 16) { btn &= ~16; mod = MCURSOR_KTRL; }
	if (btn & 32) { btn &= ~32; mod = MCURSOR_SHFT; }
	if (btn & 64) { btn &= ~64; mod = MCURSOR_ALTR; }

	/* XXX HORRIBLE HACK XXX */
	if (btn) { /* Allow remacro */
		char ks[1024], *p;
		strnfmt(ks, sizeof(ks), "%c_TERMcave_MB%02x%c",
			 31, ke.index, 13);
		if (macro_find_exact(ks) >= 0) {
			for (p = ks; *p; p++) Term_keypress(*p);
			return;
		}
	} /* XXX XXX XXX */

	/* XXX HORRIBLE HACK#2 XXX*/
	if (btn) { /* Allow rebinding */
		byte map_from = btn | mod;
		if (mousemap[map_from])
		{

			btn = mousemap[map_from] & 0x0F;
			mod = mousemap[map_from] & 0xF0;
		}
	} /* XXX XXX XXX */

	/* If the mouse is outside the dungeon, do nothing. */
	if (ke.mousex - DUNGEON_OFFSET_X < 0
	 || ke.mousey - DUNGEON_OFFSET_Y < 0)
		return;

	send_mouse(0
	  | (btn == 1 ? MCURSOR_LMB : 0)
	  | (btn == 2 ? MCURSOR_MMB : 0)
	  | (btn == 3 ? MCURSOR_RMB : 0)
	  | mod,
	  ke.mousex - DUNGEON_OFFSET_X,
	  ke.mousey - DUNGEON_OFFSET_Y);
}
