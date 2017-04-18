/*
 * Client-side store stuff.
 */

#include "angband.h"

static bool leave_store;
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
		strcpy(o_name, store_names[pos]);
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
		strcpy(o_name, store_names[pos]);
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
								c_put_fmt(TERM_WHITE, 5, 20, "(Page %d)", store_top/12 + 1);
        }
}

/*
 * Get the ID of a store item and return its value      -RAK-
 */
static int get_stock(int *com_val, cptr pmt, int i, int j)
{
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
                if (!get_com(out_val, &command)) break;

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

        object_type             *o_ptr;

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

        /* Find out how many the player wants */
        if (o_ptr->number > 1)
        {
                /* Hack -- note cost of "fixed" items */
                if (store_num != 7)
                {
									char buffer[80];
									strnfmt(buffer, sizeof buffer, "That costs %ld gold per item.", (long)(store_prices[item]));
									c_msg_print(buffer);
                }

                /* Get a quantity */
                amt = c_get_quantity(NULL, o_ptr->number);

                /* Allow user abort */
                if (amt <= 0) return;
        }

	/* Tell the server */
	Send_store_purchase(item, amt);
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
	cptr feature = "feature variable";
	
	/* The screen is "icky" */
	screen_icky = TRUE;

	/* We are "shopping" */
	shopping = TRUE;

	/* Clear screen */
	Term_clear();

	/* Find the "store name" */
	switch (store_num)
	{
		case 0: feature = "General store"; break;
		case 1: feature = "Armoury"; break;
		case 2: feature = "Weapon Smith"; break;
		case 3: feature = "Temple"; break;
		case 4: feature = "Alchemist"; break;
		case 5: feature = "Magic Shop"; break;
		case 6: feature = "Black Market"; break;
		case 7: feature = "Your home"; break;
		case 8: feature = "The Back Room"; break;
	}
	
	/* Put the owner name and race */
	if (store_num != 8)
	{
		sprintf(buf, "%s (%s)", store_owner.owner_name, p_name + race_info[store_owner.owner_race].name);
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		sprintf(buf, "%s (%ld)", feature, (long)(store_owner.max_cost));
		prt(buf, 3, 50);

	}
	else
	{
		/* A player owned store */
		sprintf(buf, "%s's Store", player_owner );
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

	/* Don't leave */
	leave_store = FALSE;

        /* Interact with player */
        while (!leave_store)
        {
		/* Keep the screen icky */
		screen_icky = TRUE;

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
                if (store_num == 7)
                {
                        prt(" g) Get an item.", 22, 30);
                        prt(" d) Drop an item.", 23, 30);
                }

                /* Shop commands XXX XXX XXX */
                else
                {
                        prt(" p) Purchase an item.", 22, 30);
						/* We don't sell things in some shops  */
						if (store_num != 8)
						{
                        	prt(" s) Sell an item.", 23, 30);
                        }
                }
					 prt (" l) Look at an item.", 22, 56);
					 
                /* Prompt */
                prt("You may: ", 21, 0);

		/* Get a command */
		while (!command_cmd)
		{
			/* Re-fresh the screen */
			Term_fresh();

			/* Update our timer and send a keepalive packet if
			 * neccecary */
			update_ticks();
			do_keepalive();

			if (Net_flush() == -1)
			{
				plog("Bad net flush");
				return;
			}

			/* Set the timeout */
			SetTimeout(0, 1000);

			/* Only take input if we got some */
			if (SocketReadable(Net_fd()))
				if (Net_input() == -1)
					return;

	                /* Get a command */
	                request_command(TRUE);

			/* Flush */
			flush_now();

			/* Redraw windows if necessary */
			if (p_ptr->window)
			{
				window_stuff();
			}
		}

                /* Process the command */
                store_process_command();

		/* Clear the old command */
		command_cmd = 0;
	}

	/* Tell the server that we're outta here */
	Send_store_leave();

	/* Clear the screen */
	Term_clear();

	/* The screen is no longer icky */
	screen_icky = FALSE;

	/* We are no longer "shopping" */
	shopping = FALSE;

	/* Flush any events that happened */
	Flush_queue();
}
