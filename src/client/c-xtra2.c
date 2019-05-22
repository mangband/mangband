/* More extra client things */

#include "c-angband.h"

/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	byte attr;
	cptr s = message_str(0);
	message_color(s, &attr);
	/* Recall one message XXX XXX XXX */
	c_prt(attr, format( "> %s", s), 0, 0);
}


/*
 * Show previous messages to the user   -BEN-
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only hilite the matching portions of the string.
 */
void do_cmd_messages(void)
{
        int i, j, k, n, q;

        char shower[MAX_CHARS] = "";
        char finder[MAX_CHARS] = "";


        /* Total messages */
        n = message_num();

        /* Start on first message */
        i = 0;

        /* Start at leftmost edge */
        q = 0;


        /* Enter "icky" mode */
        screen_icky = topline_icky = TRUE;

        /* Save the screen */
        Term_save();

        /* Process requests until done */
        while (1)
        {
                /* Clear screen */
                Term_clear();

                /* Dump up to 20 lines of messages */
                for (j = 0; (j < 20) && (i + j < n); j++)
                {
                        byte a = TERM_WHITE;

                        cptr str = message_str(i+j);
                        
                        /* Determine color */
                        message_color(str, &a);

                        /* Apply horizontal scroll */
                        str = ((int)strlen(str) >= q) ? (str + q) : ""; /* strlen is very unlikely to ever return a negative int */

                        /* Handle "shower" */
                        if (shower[0] && strstr(str, shower)) a = TERM_YELLOW;

                        /* Dump the messages, bottom to top */
                        Term_putstr(0, 21-j, -1, a, str);
                }

                /* Display header XXX XXX XXX */
                prt(format("Message Recall (%d-%d of %d), Offset %d",
                           i, i+j-1, n, q), 0, 0);

                /* Display prompt (not very informative) */
                prt("[Press 'p' for older, 'n' for newer, ..., or ESCAPE]", 23, 0);

                /* Get a command */
                k = inkey();

                /* Exit on Escape */
                if (k == ESCAPE) break;

                /* Hack -- Save the old index */
                j = i;

                /* Horizontal scroll */
                if (k == '4')
                {
                        /* Scroll left */
                        q = (q >= 40) ? (q - 40) : 0;

                        /* Success */
                        continue;
                }

                /* Horizontal scroll */
                if (k == '6')
                {
                        /* Scroll right */
                        q = q + 40;

                        /* Success */
                        continue;
                }

                /* Hack -- handle show */
                if (k == '=')
                {
                        /* Prompt */
                        prt("Show: ", 23, 0);

                        /* Get a "shower" string, or continue */
                        if (!askfor_aux(shower, MAX_COLS, 0)) continue;

                        /* Okay */
                        continue;
                }

                /* Hack -- handle find */
                if (k == '/')
                {
                        int z;

                        /* Prompt */
                        prt("Find: ", 23, 0);

                        /* Get a "finder" string, or continue */
                        if (!askfor_aux(finder, MAX_COLS, 0)) continue;

                        /* Scan messages */
                        for (z = i + 1; z < n; z++)
                        {
                                cptr str = message_str(z);

                                /* Handle "shower" */
                                if (strstr(str, finder))
                                {
                                        /* New location */
                                        i = z;

                                        /* Done */
                                        break;
                                }
                        }
                }

                /* Recall 1 older message */
                if ((k == '8') || (k == '\n') || (k == '\r'))
                {
                        /* Go newer if legal */
                        if (i + 1 < n) i += 1;
                }

                /* Recall 10 older messages */
                if (k == '+')
                {
                        /* Go older if legal */
                        if (i + 10 < n) i += 10;
                }

                /* Recall 20 older messages */
                if ((k == 'p') || (k == KTRL('P')) || (k == ' ') || (k == '9'))
                {
                        /* Go older if legal */
                        if (i + 20 < n) i += 20;
                }

                /* Recall 20 newer messages */
                if ((k == 'n') || (k == KTRL('N')) || (k == '3'))
                {
                        /* Go newer (if able) */
                        i = (i >= 20) ? (i - 20) : 0;
                }

                /* Recall 10 newer messages */
                if (k == '-')
                {
                        /* Go newer (if able) */
                        i = (i >= 10) ? (i - 10) : 0;
                }

                /* Recall 1 newer messages */
                if (k == '2')
                {
                        /* Go newer (if able) */
                        i = (i >= 1) ? (i - 1) : 0;
                }

                /* Hack -- Error of some kind */
                if (i == j) bell();
        }

        /* Restore the screen */
        Term_load();

        /* Leave "icky" mode */
        screen_icky = topline_icky = FALSE;

	/* Flush any queued events */
	Flush_queue();
}

/** Open channel **/
void do_chat_open(int id, cptr name)
{
	int i, free = -1;

	/* Find free and duplicates */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (free == -1 && STRZERO(channels[i].name)) { free = i; continue; }
		if (!strcmp(channels[i].name, name)) return;
	}

	/* Found free slot */
	if ((i = free) != -1)
	{
		/* Copy info */
		my_strcpy(channels[i].name, name, sizeof(channels[0].name));
		channels[i].id = id;

		/* Highlight 
		p_ptr->on_channel[n] = TRUE; */

		/* Window fix */
		p_ptr->window |= PW_MESSAGE_CHAT;
	}
}

/** Enforce channel **/
void do_chat_select(int id)
{
	int i;
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (channels[i].id == id)
		{
			p_ptr->main_channel = view_channel = i;

			/* Window update */
			p_ptr->window |= PW_MESSAGE_CHAT;

			break;
		}
	}
}

/** Close channel **/
void do_chat_close(int id)
{
	int i, j;

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (channels[i].id == id)
		{
			if (view_channel == i)
				cmd_chat_cycle(-1);

			for (j = 0; j < message_num(); j++)
			{
				u16b type = message_type(j);
				if (type == MSG_CHAT + id)
				{
					c_message_del(j);
				}
			}

			channels[i].name[0] = '\0';
			channels[i].id = 0;

			if (p_ptr->main_channel == i)
				p_ptr->main_channel = 0;

			if (STRZERO(channels[view_channel].name))
				cmd_chat_cycle(+1);

			/* Window update */
			p_ptr->window |= PW_MESSAGE_CHAT;

			break;
		}
	}
}

/* Message from server */
void do_handle_message(cptr mesg, u16b type)
{

	//TODO: older code used tons of hacks here, maybe return them

	/* Hack -- we're shopping/party_managing, where icky mode doesn't matter
	 * or we're not in icky mode at all, so let's display the message. */
	if (!topline_icky && (party_mode || shopping || (!screen_icky && !section_icky_col)))
	{
		c_msg_print_aux(mesg, type);
	}
	else
	{
		c_message_add(mesg, type);
	}

	if (use_sound) Term_xtra(TERM_XTRA_SOUND, type);

}

