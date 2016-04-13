/* Client-side spell stuff */

#include "c-angband.h"

static void print_spells(int book)
{
	int	i, col, rows, j = 0;

	/* Print column */
	col = 20; /* Term->wid - 68; */
	rows = 20;

	/* Title the list */
	prt("", 1, col);
	put_str("Name", 1, col + 5);
	put_str("Lv Mana Fail", 1, col + 35);

	/* Dump the spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* Clear line */
		prt("", 2 + i, col);
 
 		/* End of terminal */
		if (i > rows) break; 

		/* Check for end of the book */
		if (spell_info[book][j][0] == '\0')
			break;
 
 		/* Dump the info */
		prt(spell_info[book][j], 2 + i, col);

		/* Next */
		j++;
	}

	/* Clear the bottom line */
	prt("", 2 + i, col);
}

int count_spells_in_book(int book, int *book_over)
{
	int i, j = 0, num = 0;
	int rows = 20;

	/* Paranoia */
	if (book < 0) return -1;

	/* Check for available spells */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* End of terminal */
		if (i > rows) break;

		/* Check for end of the book */
		if (spell_info[book][j][0] == '\0') 
		{
			(*book_over)++;
			break;
		}

		/* Spell is available */
		num++;

		/* Next */
		j++;
	}

	return num;
}

/*
 * Allow user to choose a spell/prayer from the given book.
 */

/* modified to accept certain capital letters for priest spells. -AD- */ 
 
int get_spell(int *sn, cptr p, cptr prompt, int *bn, bool known)
{
	int		i, num = 0;
	bool		flag, redraw;
	char		choice;
	char		out_val[160];
	int			book = (*bn);
	int			book_over = 0;
	int			book_start = book;

	/* Assume no spells available */
	(*sn) = -2;

	/* Check for available spells */
	num = count_spells_in_book(book, &book_over);

	/* No "okay" spells */
	if (!num) return (FALSE);

	/* Assume cancelled */
	(*sn) = -1;

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%s %c-%c, *=List, ESC=exit) %s",
		p, I2A(0), I2A(num - 1), prompt);

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Flip page */
		if (choice == '/')
		{
			int book_new = book; 
			int tmp = 0;
			
			/* End of list */
			if (!(num = count_spells_in_book(book + book_over, &tmp)))
			{
				/* Set 0 */ 
				book_new = book_start;
				tmp = 0;
				num = count_spells_in_book(book_new, &tmp);
				book_over = tmp;
			}
			/* Next book available */
			else
			{
				/* Advance */
				book_new = book + book_over;
				book_over = tmp;
			}
			/* Notice flip! */
			if (book_new != book)
			{
				/* Set */
				book = book_new;
				
				/* Re-Build a prompt (accept all spells) */
				strnfmt(out_val, 78, "(%s %c-%c, *=List, ESC=exit) %s",
					p, I2A(0), I2A(num - 1), prompt);

				/* Must redraw list */
				if (redraw)
				{
					/* Restore the screen */
					Term_load();
					Term_save();
	
					/* Display a list of spells */
					print_spells(book);
				}
			}
			/* Ask again */
			continue;
		}
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?'))
		{
			/* Show the list */
			if (!redraw)
			{
				/* Show list */
				redraw = TRUE;

				/* The screen is icky */
				screen_icky = TRUE;

				/* Save the screen */
				Term_save();

				/* Display a list of spells */
				print_spells(book);
			}

			/* Hide the list */
			else
			{
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();

				/* The screen is OK now */
				screen_icky = FALSE;

				/* Flush any events */
				Flush_queue();
			}

			/* Ask again */
			continue;
		}

		/* hack for CAPITAL prayers (heal other) */
		if (isupper(choice))
		{
			i = (choice - 'A');
			if (i >= num) i = -1;
			else if (!(spell_flag[(book * SPELLS_PER_BOOK + i)] & PY_SPELL_PROJECT)) i = -1;
			if (i != -1)
				i += SPELL_PROJECTED;
		}
		/* lowercase */
		else if (islower(choice))
		{
			i = A2I(choice);
			if (i >= num) i = -1;
		}
		/* not a letter */
		else i = -1;

		/* Totally Illegal */
		if (i < 0)
		{
			bell();
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
	if (redraw)
	{
		Term_load();
		screen_icky = FALSE;

		/* Flush any events */
		Flush_queue();
	}


	/* Abort if needed */
	if (!flag) return (FALSE);

	/* Save the choice */
	(*sn) = i;
	(*bn) = book;

	/* Success */
	return (TRUE);
}


/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 */
void show_browse(int book)
{
	/* The screen is icky */
	screen_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Display the spells */
	print_spells(book);

	/* Clear the top line */
	prt("", 0, 0);

	/* Prompt user */
	put_str("[Press any key to continue]", 0, 23);

	/* Wait for key */
	(void)inkey();

	/* Restore the screen */
	Term_load();

	/* Screen is OK now */
	screen_icky = FALSE;

	/* Flush any events */
	Flush_queue();
}

#if 0
/*
 * Study a book to gain a new spell/prayer
 */
void do_study(int book)
{
	int j;

	/* Mage -- Learn a selected spell */
	if (c_info[pclass].spell_book == TV_MAGIC_BOOK)
	{
		/* Ask for a spell, allow cancel */
		if (!get_spell(&j, "spell", "Study which spell? ", &book, FALSE)) return;
	}

	/* Priest -- Learn random spell */
	else j = -1;

	/* Tell the server */
	/* Note that if we are a priest, the server ignores the spell parameter */
	Send_gain(book, j);
}
#endif

/*
 * Extra arguments for spell-casting
 */
bool do_cast_xtra(int book, int spell)
{
	int dir, item;
	int index = book * SPELLS_PER_BOOK + spell;

	/* Aimed OR Projected */
	if ((spell_flag[index] & PY_SPELL_AIM) || spell >= PY_MAX_SPELLS)
	{
		if (!get_dir(&dir))	return FALSE;

		if (dir)
			Send_direction(dir);
	}

	/* Apply to Item */
	if (spell_flag[index] & PY_SPELL_ITEM)
	{
		/* c_msg_print(NULL); */
		item_tester_tval = 0;
		if (!c_get_item(&item, "Which item? ", TRUE, TRUE, TRUE)) return FALSE;
		Send_item(item);
	}

	return TRUE;
}
/*
 * Cast a spell
 */
void do_cast(int book)
{
	int j;

	/* Ask for a spell, allow cancel */
	if (!get_spell(&j, "spell", "Cast which spell? ", &book, FALSE)) return;

	/* Additional */
	if (!do_cast_xtra(book, j)) return;

	/* Tell the server */
	Send_cast(book, j);
}

/*
 * Pray a spell
 */
void do_pray(int book)
{
	int j;

	/* Ask for a spell, allow cancel */
	if (!get_spell(&j, "prayer", "Pray which prayer? ", &book, FALSE)) return;

	/* Additional */
	if (!do_cast_xtra(book, j)) return;

	/* Tell the server */
	Send_pray(book, j);
}

/*
 * Use a ghost ability
 */
void do_ghost(void)
{
	int book = 10; /* HACK -- USE BOOK 10 -- */
	int j;

	/* Ask for an ability, allow cancel */ 
	if (!get_spell(&j, "power", "Use which power? ", &book, FALSE)) return;

	/* Additional */
	if (!do_cast_xtra(book, j)) return;

	/* Tell the server */
	Send_ghost(j);
}
