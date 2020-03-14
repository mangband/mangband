/*
 * Client-side store stuff.
 */

#include "c-angband.h"

static int store_top;

static void display_entry(int pos)
{
	object_type *o_ptr;
	int i, x;
	char o_name[80];
	char out_val[160];

	int maxwid = 75;

	/* Get the item */
	o_ptr = &store.stock[pos];

	/* Get the "offset" */
	i = (pos % 12);

	/* Label it, clear the line --(-- */
	(void)sprintf(out_val, "%c) ", I2A(i));
	prt(out_val, i+6, 0);

	/* Describe an item in the home */
	if (store_num == 7)
	{
		maxwid = 75;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 10;

                /* Describe the object */
		my_strcpy(o_name, store_names[pos], sizeof(o_name));
		o_name[maxwid] = '\0';
		c_put_str(o_ptr->sval, o_name, i+6, 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			(void)sprintf(out_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(out_val, i+6, 68);
		}
	}

	else
	{
		/* Must leave room for the "price" */
		maxwid = 65;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 7;

		/* Describe the object (fully) */
		my_strcpy(o_name, store_names[pos], sizeof(o_name));
		o_name[maxwid] = '\0';
		c_put_str(o_ptr->sval, o_name, i+6, 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			(void)sprintf(out_val, "%3d.%d", wgt / 10, wgt % 10);
			put_str(out_val, i+6, 61);
		}

		x = store_prices[pos];

		/* Actually draw the price (not fixed) */
		(void)sprintf(out_val, "%9ld  ", (long)x);
		put_str(out_val, i+6, 68);
	}
}



void display_inventory(void)
{
	int i, k;

	/* Check for empty page */
	if (store_top == store.stock_num)
	{
		/* Go to previous page if possible */
		if (store_top > 0) store_top -= 12;
	}

	for (k = 0; k < 12; k++)
	{
		/* Do not display "dead" items */
		if (store_top + k >= store.stock_num) break;

		/* Display that one */
		display_entry(store_top + k);
	}

	/* Erase the extra lines and the "more" prompt */
	for (i = k; i < 13; i++) prt("", i + 6, 0);

	/* Assume "no current page" */
	put_str("        ", 5, 20);

	/* Visual reminder of "more items" */
	if (store.stock_num > 12)
	{
		/* Show "more" reminder (after the last item) */
		prt("-more-", k + 6, 3);

		/* Indicate the "current page" */
		put_str(format("(Page %d)", store_top/12 + 1), 5, 20);
	}
}

/*
 * Get the ID of a store item and return its value      -RAK-
 */
static int get_stock(int *com_val, cptr pmt, int i, int j)
{
	event_type ke;
	char    command;

	char    out_val[160];


	/* Paranoia XXX XXX XXX */
	c_msg_print(NULL);


	/* Assume failure */
	*com_val = (-1);

	/* Build the prompt */
	(void)sprintf(out_val, "(Items %c-%c, ESC to exit) %s",
		I2A(i), I2A(j), pmt);

	/* Ask until done */
	while (TRUE)
	{
		int k;

		/* Escape */
		if (!get_com_ex(out_val, &command, &ke)) break;

		/* Hack -- mouse click */
		if (command == '\xff')
		{
			command = 'a' + ke.mousey - 6;
		}

		/* Convert */
		k = (islower(command) ? A2I(command) : -1);

		/* Legal responses */
		if ((k >= i) && (k <= j))
		{
			*com_val = k;
			break;
		}

		/* Oops */
		bell();
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Cancel */
	if (command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}

/* Public interface to get_stock function. */
int get_store_stock(int *citem, cptr prompt)
{
	int                     i, item;

	/* Empty? */
	if (store.stock_num <= 0)
	{
		if (store_num == 7) c_msg_print("Your home is empty.");
		else c_msg_print("I am currently out of stock.");
		return 0;
	}

	/* Find the number of objects on this and following pages */
	i = (store.stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Get the item number to be bought */
	if (!get_stock(&item, prompt, 0, i-1)) return 0;

	/* Get the actual index */
	item = item + store_top;

	*citem = item;

	return 1;
}

static void store_examine(void) 
{
	int                     i;
	int                     item;

	char            out_val[160];

	/* Empty? */
	if (store.stock_num <= 0)
	{
		if (store_num == 7) c_msg_print("Your home is empty.");
		else c_msg_print("I am currently out of stock.");
		return;
	}

	/* Find the number of objects on this and following pages */
	i = (store.stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	sprintf(out_val, "Which item do you want to examine? ");

	/* Get the item number to be bought */
	if (!get_stock(&item, out_val, 0, i-1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Tell the server */
	Send_observe(item);
}

static void store_purchase(void)
{
	int                     i, amt;
	int                     item;
	u32b                    price;

	object_type             *o_ptr;

	char                    out_val[160];


	/* Empty? */
	if (store.stock_num <= 0)
	{
		if (store_num == 7) c_msg_print("Your home is empty.");
		else c_msg_print("I am currently out of stock.");
		return;
	}


	/* Find the number of objects on this and following pages */
	i = (store.stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	if (store_num == 7)
	{
		sprintf(out_val, "Which item do you want to take? ");
	}
	else
	{
		sprintf(out_val, "Which item are you interested in? ");
	}

	/* Get the item number to be bought */
	if (!get_stock(&item, out_val, 0, i-1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	o_ptr = &store.stock[item];

	/* Assume the player wants just one of them */
	amt = 1;

	/* Hack -- save price */
	price = store_prices[item];

	/* Find out how many the player wants */
	if (o_ptr->number > 1)
	{
		/* Hack -- note cost of "fixed" items */
		if (store_num != 7)
		{
			c_msg_print(format("That costs %ld gold per item.", (long)price));
		}

		/* Hack -- set buying */
		shopping_buying = TRUE;

		/* Get a quantity */
		amt = c_get_quantity(NULL, o_ptr->number);

                /* Hack -- unset buying */
		shopping_buying = FALSE;

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Hack -- multiply price */
	price *= amt;

	/* Tell the server */
	Send_store_purchase(item, amt, price);
}

static void store_sell(void)
{
	int item, amt;

	if (store_num != 7)
	{
		if (!c_get_item(&item, "Sell what? ", TRUE, TRUE, FALSE))
		{
			return;
		}
	}
	else
	{
		if (!c_get_item(&item, "Drop what? ", TRUE, TRUE, FALSE))
		{
			return;
		}
	}

	/* Get an amount */
	if (inventory[item].number > 1)
	{
		amt = c_get_quantity("How many? ", inventory[item].number);
	}
	else amt = 1;

	/* Tell the server */
	Send_store_sell(item, amt);
}

static void store_process_command(void)
{
	/* Try custom commands */
	byte i;
	for (i = 0; i < custom_commands; i++)
	{
		if (custom_command[i].flag & COMMAND_STORE
		 && custom_command[i].m_catch == command_cmd)
		{
			cmd_custom(i);
			return;
		}
	}

	/* Parse the command */
	switch (command_cmd)
	{
			/* Leave */
		case ESCAPE:
		{
			leave_store = TRUE;
			break;
		}
			/* Browse */
		case ' ':
		{
			if (store.stock_num <= 12)
			{
				c_msg_print("Entire inventory is shown.");
			}
			else
			{
				store_top += 12;
				if (store_top >= store.stock_num) store_top = 0;
				display_inventory();
			}
			break;
		}
#if 0
                       /* Look (examine) */
		case 'l':
		{
			store_examine();
			break;
		}

		/* Get (purchase) */
		case 'g':
		{
			store_purchase();
			break;
		}

			/* Drop (Sell) */
		case 'd':
		{
			if (store_num != 8)
				store_sell();
			break;
		}
#endif
			/* Ignore return */
		case '\r':
		{
			break;
		}

			/* Equipment list */
		case 'e':
		{
			cmd_equip();
			break;
		}

			/* Inventory list */
		case 'i':
		{
			cmd_inven();
			break;
		}
			/* Mouse click */
		case '\xff':
		{
			do_cmd_term_mousepress(PW_STORE, command_cmd_ex.mousex, command_cmd_ex.mousey, command_cmd_ex.index);
			break;
		}
		default:
		{
			c_msg_print("That command does not work in stores.");
			break;
		}
	}
}


void display_store(void)
{
	char buf[1024];
	u32b old_flag;
	bool redraw = TRUE;

	/* Entering store (called first time) */
	if (shopping == FALSE)
	{
		old_flag = window_flag[0];
		Term_save();

		window_flag[0] |= PW_STORE;
		window_flag[0] &= ~PW_STATUS;
		window_flag[0] &= ~PW_PLAYER_2;
	}

	/* The screen is "icky" */
	screen_icky = TRUE;

	/* We are "shopping" */
	shopping = TRUE;

	/* Reset 'buying' */
	shopping_buying = FALSE;

	/* Clear screen */
	Term_clear();

	/* Store name and owner (race) */
	if (store_flag & STORE_NPC)
	{
		/* NPC store */
		sprintf(buf, "%s", store_owner_name);
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		sprintf(buf, "%s (%ld)", store_name, (long)(store_owner.max_cost));
		prt(buf, 3, 50);
	}
	else if (store_flag & STORE_PC)
	{
		/* A player owned store */
		sprintf(buf, "%s's %s", store_owner_name, store_name );
		put_str(buf, 3, 10);
	}

	/* Label the item descriptions */
	put_str("Item Description", 5, 3);

	/* If showing weights, show label */
	if (show_weights)
	{
		put_str("Weight", 5, 60);
	}

	/* Label the asking price (in stores) */
	put_str("Price", 5, 72);

	/* Display the players remaining gold */
	prt("Gold Remaining: ", 19, 53);

	sprintf(buf, "%9ld", (long) p_ptr->au);
	prt(buf, 19, 68);


	/* Start at the top */
	store_top = 0;

	/* Display the inventory */
	display_inventory();

	redraw_indicators(PW_STORE);

	/* Don't leave */
	leave_store = FALSE;

	/* Interact with player */
	while (!leave_store)
	{
		/* Keep the screen icky */
		screen_icky = TRUE;

		if (redraw)
		{
			redraw = FALSE;

			/* Hack -- Clear line 1 */
			prt("", 1, 0);

			/* Clear */
			clear_from(21);

			/* Basic commands */
			prt(" ESC) Exit from Building.", 22, 0);

			/* Browse if necessary */
			if (store.stock_num > 12)
			{
				prt(" SPACE) Next page of stock.", 23, 0);
			}

			/* Home commands */
			if (store_flag & STORE_HOME)
			{
				prt(" g) Get an item.", 22, 30);
				prt(" d) Drop an item.", 23, 30);
			}

			/* Shop commands XXX XXX XXX */
			else
			{
				prt(" p) Purchase an item.", 22, 30);
				/* We don't sell things in some shops  */
				if (!(store_flag & STORE_PC))
				{
					prt(" s) Sell an item.", 23, 30);
				}
			}
			prt (" l) Look at an item.", 22, 56);

			/* Prompt */
			prt("You may: ", 21, 0);

			/* Ugh */
			Term_fresh();
		}

		/* Hack -- shopping command */
		if (z_ask_command_aux) z_ask_command_aux(NULL, TRUE);

		inkey_nonblock = TRUE;
		command_cmd_ex = inkey_ex();
		command_cmd = command_cmd_ex.key;
		inkey_nonblock = FALSE;

		/* Hack -- ignore mouse motion */
		if (command_cmd == '\xff' && command_cmd_ex.index == 0) command_cmd = 0;

		if (command_cmd)
		{
			/* Process it */
			store_process_command();

			/* Clear previous command */
			command_cmd = 0;

			/* Redraw screen */
			redraw = TRUE;
		}

		/* Process server-side requests */
		process_requests();
	}

	/* Tell the server that we're outta here */
	send_store_leave();

	/* Clear the screen */
	Term_clear();

	/* The screen is no longer icky */
	screen_icky = FALSE;

	/* We are no longer "shopping" */
	shopping = FALSE;

	/* Fix screen */
	Term_load();
	window_flag[0] = old_flag;
	redraw_indicators(old_flag);

	/* Flush any events that happened */
	Flush_queue();
}
