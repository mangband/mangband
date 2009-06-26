/* File: cmd4.c */

/* Purpose: Interface commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"



/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This doesn't need to be in the server --KLJ--
 */
void do_cmd_redraw(void)
{
}


/*
 * Hack -- change name
 *
 * This isn't allowed anymore --KLJ--
 */
void do_cmd_change_name(void)
{
}


/*
 * Recall the most recent message
 *
 * This should be handled by the client --KLJ--
 */
void do_cmd_message_one(void)
{
}


/*
 * Show previous messages to the user	-BEN-
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
 *
 * This is in the client code --KLJ--
 */
void do_cmd_messages(void)
{
}


/*
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 *
 * Any options that can be changed will be client-only --KLJ--
 */
void do_cmd_options(void)
{
}



/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 *
 * This is client-side --KLJ--
 */
void do_cmd_pref(void)
{
}


/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * XXX XXX XXX Need messages for success, plus "helpful" info
 *
 * Macros are handled by the client --KLJ--
 */
void do_cmd_macros(void)
{
}



/*
 * Interact with "visuals"
 *
 * This is (of course) a client-side thing --KLJ--
 */
void do_cmd_visuals(void)
{
}


/*
 * Interact with "colors"
 *
 * This is kind of client-side, but maybe we should allow players to select
 * the color and character of objects, like is done in single-player Angband.
 * Right now, the colors and characters of objects are fixed, but the user
 * will be able to change the overall colors (if his visual modules supports
 * that.)    --KLJ--
 */
void do_cmd_colors(void)
{
}


/*
 * Note something in the message recall
 *
 * This is a (I think) useless command.  It will later be used to "talk" to
 * other players on the same level as the "talker".  --KLJ--
 */
void do_cmd_note(void)
{
}


/*
 * Mention the current version
 *
 * The client handles this, and it also knows the server version, and prints
 * that out as well.  --KLJ--
 */
void do_cmd_version(void)
{
}



/*
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
{
	"Looks like any other level.",
	"You feel there is something special about this level.",
	"You have a superb feeling about this level.",
	"You have an excellent feeling...",
	"You have a very good feeling...",
	"You have a good feeling...",
	"You feel strangely lucky...",
	"You feel your luck is turning...",
	"You like the look of this place...",
	"This level can't be all bad...",
	"What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 *
 * Level feelings are tricky.  Say level 1 (50') has been around for a long
 * time, because there's always been at least one player on it.  Say that
 * it was "special" when it was generated, because the Phial was on it.  But
 * it has long since been picked up, but any players who come later onto this
 * level will still get a "special" feeling.  So, should the level feeling
 * be recomputed whenever it is asked for?  Right now, level feelings are
 * disabled.  --KLJ--
 *
 * We give a level feeling to the player who causes the level to be
 * generated. See dungeon.c [grk]
 *
 */
void do_cmd_feeling(int Ind)
{
	player_type *p_ptr = Players[Ind];
	
	/* Verify the feeling */
	if (p_ptr->feeling < 0) p_ptr->feeling = 0;
	if (p_ptr->feeling > 10) p_ptr->feeling = 10;

	/* No useful feeling in town */
	if (!p_ptr->dun_depth)
	{
		msg_print(Ind, "Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg_print(Ind, do_cmd_feeling_text[p_ptr->feeling]);
}





/*
 * Encode the screen colors
 */
/*static char hack[17] = "dwsorgbuDWvyRGBU";*/


/*
 * Hack -- load a screen dump from a file
 *
 * This can be client side --KLJ--
 */
void do_cmd_load_screen(void)
{
}


/*
 * Hack -- save a screen dump to a file
 *
 * This can also be client side --KLJ--
 */
void do_cmd_save_screen(void)
{
}



/* MAngband-specific: show list of owned houses */
void display_houses(int Ind, char query)
{
	player_type *p_ptr = Players[Ind];
	int i, j = 0;
	char buf[160];
	
	int sy, sx;
	char dpt[8];

	if (query == ESCAPE) return;

	/* Prepare */
	text_out_init(Ind);
	text_out("Owned Houses");
	text_out("\n");
	text_out("\n");

	for (i = 0; i < num_houses; i++) 
	{
		if (house_owned_by(Ind, i)) 
		{
			if (j++ < p_ptr->interactive_line) continue;
			
			dpt[0] = '\0';
			wild_cat_depth(houses[i].depth, &dpt[0]);
			
			sx = (houses[i].x_1 / SCREEN_WID) * 2;
			sy = (houses[i].y_1 / SCREEN_HGT) * 2;
			
			sprintf(buf, "  %c) House %d %s %s, sector [%d,%d]", index_to_label(j-1), j, 
					(!houses[i].depth ? "in" : "at"), dpt, sy, sx);
			text_out(buf);
			text_out("\n");
		}
	}
	if (!j)
		text_out("You do not own any.\n");

	/* Done */
	text_out_done();
	
	/* Send */
	Send_term_info(Ind, NTERM_CLEAR, 1);
	for (i = 0; i < MAX_TXT_INFO; i++)
	{
		if (i >= p_ptr->last_info_line) break;
		Send_remote_line(Ind, i);
	}
	Send_term_info(Ind, NTERM_FLUSH, 0);
	
}

/*
 * Check the status of "artifacts"
 *
 * Should every artifact that is held by anyone be listed?  If so, should this
 * list the holder?  Doing so might induce wars to take hold of relatively
 * worthless artifacts (like the Phial), simply because there are very few
 * (three) artifact lites.  Right now, the holder isn't listed.
 *
 * Also, (for simplicity), artifacts lying in the dungeon, or artifacts that
 * are in a player's inventory but not identified are put in the list.
 *
 * Why does Ben save the list to a file and then display it?  Seems like a
 * strange way of doing things to me.  --KLJ--
 */
void do_cmd_check_artifacts(int Ind, int line)
{
	player_type *p_ptr = Players[Ind];
	int i, j, k, z, Depth, y, x;

	FILE *fff;

	char file_name[1024];

	char base_name[80];

	bool *okay;


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");
	
	/* Paranoia */
	if (!fff) 
	{
		plog(format("ERROR! %s (writing %s)", strerror(errno), file_name));
		return;
	}
	
	/* Init Array */
	C_MAKE(okay, z_info->a_max, bool);

	/* Scan the artifacts */
	for (k = 0; k < z_info->a_max; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* Default */
		okay[k] = FALSE;

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Hack -- allow found artifacts */
		if (!p_ptr->a_info[k])
		/* Skip "uncreated" artifacts */
		if (!a_ptr->cur_num) continue;

		/* Assume okay */
		okay[k] = TRUE;
	}

	/* Check the dungeon */
	for (Depth = 0; Depth < MAX_DEPTH; Depth++)
	{
		/* Skip uncreated levels */
		if (!cave[Depth]) continue;

		/* Scan this level */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				cave_type *c_ptr = &cave[Depth][y][x];

				/* Process objects */
				if (c_ptr->o_idx)
				{
					object_type *o_ptr = &o_list[c_ptr->o_idx];

					/* Ignore non-artifacts */
					if (!artifact_p(o_ptr)) continue;

					/* Ignore known items */
					if (object_known_p(Ind, o_ptr)) continue;

					/* Note the artifact */
					okay[o_ptr->name1] = FALSE;
				}
			}
		}
	}

	/* Check the inventories */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *p_ptr = Players[i];
		
		/* Check this guy's */
		for (j = 0; j < INVEN_PACK; j++)
		{
			object_type *o_ptr = &p_ptr->inventory[j];

			/* Ignore non-objects */
			if (!o_ptr->k_idx) continue;

			/* Ignore non-artifacts */
			if (!artifact_p(o_ptr)) continue;

			/* Ignore known items */
			if (object_known_p(Ind, o_ptr)) continue;

			/* Note the artifact */
			okay[o_ptr->name1] = FALSE;
		}
	}

	/* Scan the artifacts */
	for (k = 0; k < z_info->a_max; k++)
	{
		artifact_type *a_ptr = &a_info[k];
		char highlite = 'D';

		/* List "dead" ones */
		if (!okay[k]) continue;

		/* Paranoia */
		strcpy(base_name, "Unknown Artifact");

		/* Obtain the base object type */
		z = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Real object */
		if (z)
		{
			object_type forge;

			/* Create the object */
			invcopy(&forge, z);

			/* Create the artifact */
			forge.name1 = k;

			/* Describe the artifact */
			object_desc_store(Ind, base_name, &forge, FALSE, 0);
		}

		/* Determine if it's relevant to the player asking */
		if (p_ptr->a_info[k]) highlite = 'w';
		if (p_ptr->a_info[k] >= cfg_preserve_artifacts) highlite = 'W';

		/* Hack -- Build the artifact name */
		fprintf(fff, "%c     The %s\n", highlite, base_name);
	}
	
	/* Free array */
	FREE(okay, bool);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Artifacts Seen", line, 1);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Check the status of "uniques"
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 *
 * Any unique seen by any player will be shown.  Also, I plan to add the name
 * of the slayer (if any) to the list, so the others will know just how
 * powerful any certain player is.  --KLJ--
 */
void do_cmd_check_uniques(int Ind, int line)
{
	int k, l, i, space, namelen, total = 0, width = 78;
	FILE *fff;
	char file_name[1024], buf[1024];
	u16b *idx;
	monster_race *r_ptr, *curr_ptr;

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Paranoia */
	if (!fff) 
	{
		plog(format("ERROR! %s (writing %s)", strerror(errno), file_name));
		return;
	}

	/* Allocate the "idx" array */
	C_MAKE(idx, z_info->r_max, u16b);

	/* Scan the monster races */
	for (k = 1; k < z_info->r_max; k++)
	{
		r_ptr = &r_info[k];

		/* Only print Uniques */
		if (r_ptr->flags1 & RF1_UNIQUE)
		{
			/* Only display "known" uniques */
			if ((Players[Ind]->dm_flags & DM_SEE_MONSTERS) || r_ptr->r_sights)
			{
				l = 0;
				while (l < total)
				{
					curr_ptr = &r_info[idx[l]];
					if (r_ptr->level > curr_ptr->level)
						break;
					l++;
				}				
				for (i = total; i > l; i--)	
					idx[i] = idx[i - 1];
				idx[l] = k;
				total++;
			}
		}
	}
								
	if (total)
	{
		/* for each unique */
		for (l = total - 1; l >= 0; l--)
		{
			bool ok = FALSE;
			char highlight = 'D';
			r_ptr = &r_info[idx[l]];
			sprintf(buf, "%s killed by: ", r_name + r_ptr->name);
			space = width - strlen(buf);
					
			/* Do we need to highlight this unique? */
			for (i = 1; i <= NumPlayers; i++)
			{
				player_type *q_ptr = Players[i];
				if (q_ptr->r_killed[idx[l]])
				{
					if (i == Ind) highlight = 'w';
				}
			}
			
			/* Append all players who killed this unique */
			k = 0;
			for (i = 1; i <= NumPlayers; i++)
			{
				player_type *q_ptr = Players[i];
				if (q_ptr->r_killed[idx[l]])
				{
					ok = TRUE;
					namelen = strlen(q_ptr->name)+2;
					if (space - namelen < 0 )
					{
						/* Out of space, flush the line */
						fprintf(fff, "%c%s\n", highlight, buf);
						strcpy(buf, "  \0");
						k = 0;
						space = width;
					}
					if (k++) strcat(buf, ", ");
					strcat(buf, q_ptr->name);
					space -= namelen;
				}
			}
			if(ok)
			{
				fprintf(fff, "%c%s\n", highlight, buf);
			}
			else
			{
				if (r_ptr->r_tkills)
				{
					fprintf(fff, "D%s has been killed by somebody.\n", r_name + r_ptr->name);
				}
				else
				{
					fprintf(fff, "D%s has never been killed!\n", r_name + r_ptr->name);
				}
			}

		}
	}
	else fprintf(fff, "wNo uniques are witnessed so far.\n");

	/* Free the "ind" array */
	FREE(idx, u16b);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Known Uniques", line, 1);

	/* Remove the file */
	fd_kill(file_name);
}

/*
 * Check the status of "players"
 *
 * The player's name, race, class, and experience level are shown.
 */
void do_cmd_check_players(int Ind, int line)
{
	int k;

	FILE *fff;

	char file_name[1024];

	player_type *p_ptr = Players[Ind];

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");
	
	/* Paranoia */
	if (!fff) 
	{
		plog(format("ERROR! %s (writing %s)", strerror(errno), file_name));
		return;
	}

	/* Scan the player races */
	for (k = 1; k < NumPlayers + 1; k++)
	{
		player_type *q_ptr = Players[k];
		byte attr = 'w';

		/* Only print connected players */
		if (q_ptr->conn == NOT_CONNECTED)
			continue;

		/* don't display the dungeon master if the secret_dungeon_master
		 * option is set (unless you're a DM yourself)
		 */
        if ((q_ptr->dm_flags & DM_SECRET_PRESENCE) && !(p_ptr->dm_flags & DM_SEE_PLAYERS)) continue;

		/*** Determine color ***/

		/* Print self in green */
		if (Ind == k) attr = 'G';

		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';

		/* Print hostile players in red */
		else if (cfg_pvp_notify == -1 || check_hostile(Ind, k) || (cfg_pvp_notify && check_hostile(k, Ind))) attr = 'r';

		/* Output color byte */
		fprintf(fff, "%c", attr);

		/* Print a message */
		if(option_p(q_ptr, NO_GHOST))
		{
			fprintf(fff, "     %s the Brave %s %s (Level %d, %s)",
			q_ptr->name, p_name + p_info[q_ptr->prace].name,
			c_name + c_info[q_ptr->pclass].name, q_ptr->lev,
			parties[q_ptr->party].name);
		} 
		else 
		{
			fprintf(fff, "     %s the %s %s (Level %d, %s)",
			q_ptr->name, p_name + p_info[q_ptr->prace].name,
			c_name + c_info[q_ptr->pclass].name, q_ptr->lev,
			parties[q_ptr->party].name);
		}


		/* Print extra info if these people are not 'red' aka hostile */
		/* Hack -- always show extra info to dungeon master */
		if ((attr != 'r' && cfg_pvp_hostility > 0) || (p_ptr->dm_flags & DM_SEE_PLAYERS))
		{
			fprintf(fff, " at %d ft", q_ptr->dun_depth * 50);
		}

		/* Newline */
		// -AD- will this work?
		fprintf(fff, "\n");
		fprintf(fff, "         %s@%s\n", q_ptr->realname, q_ptr->hostname);

	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Player list", line, 1);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Display known objects
 */
static void do_cmd_knowledge_object(int Ind, int line)
{
	int k;

	FILE *fff;

	char o_name[80];

	char file_name[1024];

	player_type *p_ptr = Players[Ind];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Failure */
	if (!fff) return;

	/* Scan the object kinds */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Hack -- skip artifacts */
		if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

		/* List known flavored objects */
		if (k_ptr->flavor && p_ptr->obj_aware[k])
		{
			object_type *i_ptr;
			object_type object_type_body;
			char flav[80];

			/* Get local object */
			i_ptr = &object_type_body;

			/* Create fake object */
			object_prep(i_ptr, k);

			/* Describe the object */
			object_desc(Ind, o_name, i_ptr, FALSE, 0);
			/*object_desc_spoil(o_name, sizeof(o_name), i_ptr, FALSE, 0);*/

			/* HACK -- Append flavour */
			strcat(o_name, " (");
			flavor_copy(flav, k_ptr->flavor, i_ptr); 
			strcat(o_name, flav);
			strcat(o_name, ")");

			/* Print a message */
			fprintf(fff, "     %s\n", o_name);
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Known Objects", line, 0);

	/* Remove the file */
	fd_kill(file_name);
}



/*
 * Display kill counts
 */
static void do_cmd_knowledge_kills(int Ind, int line)
{
	int n, i;

	FILE *fff;

	char file_name[1024];

	u16b *who;
	u16b why = (SORT_EASY);

	player_type *p_ptr = Players[Ind];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Failure */
	if (!fff) return;


	/* Allocate the "who" array */
	C_MAKE(who, z_info->r_max, u16b);

	/* Collect matching monsters */
	for (n = 0, i = 1; i < z_info->r_max - 1; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = p_ptr->l_list + i;

		/* Require non-unique monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Collect "appropriate" monsters */
		if (l_ptr->pkills > 0) who[n++] = i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_monsters;
	ang_sort_swap = ang_sort_swap_u16b;

	/* Sort by kills (and level) */
	ang_sort(Ind, who, &why, n);

	/* Print the monsters (highest kill counts first) */
	for (i = n - 1; i >= 0; i--)
	{
		monster_race *r_ptr = &r_info[who[i]];
		monster_lore *l_ptr = p_ptr->l_list + who[i];

		/* Print a message */
		fprintf(fff, "     %-40s  %5d\n",
		        (r_name + r_ptr->name), l_ptr->pkills);
	}

	/* Free the "who" array */
	FREE(who, u16b);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Kill counts", line, 0);

	/* Remove the file */
	fd_kill(file_name);
}

void do_cmd_knowledge_history(int Ind, int line)
{
	FILE *fff;

	char file_name[1024];

	player_type *p_ptr = Players[Ind];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Failure */
	if (!fff) return;


	/* Dump character history */
	if(p_ptr->birth_turn.turn || p_ptr->birth_turn.era)
	{
		history_event *evt;
		fprintf(fff, "Time       Depth   CLev  Event\n");
		//fprintf(fff, "           Level   Level\n\n");
		for(evt = p_ptr->charhist; evt; evt = evt->next)
		{
			fprintf(fff, format_history_event(evt));
			fprintf(fff, "\n");
		}
		fprintf(fff, "\n\n");
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Character History", line, 0);

	/* Remove the file */
	fd_kill(file_name);
}



/*
 * Scroll through *ID* or Self Knowledge information.
 */
void do_cmd_check_other(int Ind, int line)
{
	player_type *p_ptr = Players[Ind];

	char buf[1024];
	int i, j, size = 80;

	/* Make sure the player is allowed to */
	if (!p_ptr->special_file_type) return;

	/* Dump the next 20 lines of the file */
	for (i = 0; i < 20; i++)
	{
		byte attr = TERM_WHITE;

		/* We're done */
		if (line + i > MAX_TXT_INFO) break;
		if (line + i > p_ptr->last_info_line) break; 
		
		/* Extract string */
		for (j = 0; j < size; j++)
		{
			buf[j] = p_ptr->info[line+i][j].c;
		}
		attr = p_ptr->info[line+i][0].a;
		buf[j] = '\0';

		/* Dump the line */
		Send_special_line(Ind, p_ptr->last_info_line+1, i, attr, &buf[0]);
	}
#if 0
	int n = 0;
	FILE *fff;
	char file_name[1024];
	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Paranoia */
	if (!fff) 
	{
		plog(format("ERROR! %s (writing %s)", strerror(errno), file_name));
		return;
	}

	/* Scan "info" */
	while (n < 128 && p_ptr->info[n] && strlen(p_ptr->info[n]))
	{
		/* Dump a line of info */
		fprintf(fff, p_ptr->info[n]);

		/* Newline */
		fprintf(fff, "\n");

		/* Next line */
		n++;
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Extra Info", line, 0);

	/* Remove the file */
	fd_kill(file_name);
#endif
}

void common_peruse(int Ind, char query)
{
	player_type *p_ptr = Players[Ind];
	switch (query) {
		case 0:case '7': 
			p_ptr->interactive_line = 0;
			break;
		case '2':case '\n':case '\r':
			p_ptr->interactive_line++;
			break;
		case ' ':case '3':
			p_ptr->interactive_line += 20;
			break;
		case '9':
			p_ptr->interactive_line -= 20;
			break;
		case '8':case '=':
			p_ptr->interactive_line--;
			break;
		case '-':
			p_ptr->interactive_line -= 10;
			break;
		case ESCAPE:
			p_ptr->special_file_type = SPECIAL_FILE_NONE;
			break;
	}
	if (p_ptr->interactive_line < 0) 
		p_ptr->interactive_line = 0;
}

void do_cmd_interactive_aux(int Ind, int type, char query)
{
	player_type *p_ptr = Players[Ind];

	switch (type)
	{
		case SPECIAL_FILE_NONE:
			p_ptr->special_file_type = FALSE;
			break;
		case SPECIAL_FILE_UNIQUE:
			common_peruse(Ind, query);
			do_cmd_check_uniques(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_ARTIFACT:
			common_peruse(Ind, query);
			do_cmd_check_artifacts(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_PLAYER:
			common_peruse(Ind, query);
			do_cmd_check_players(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_OTHER:
			common_peruse(Ind, query);
			if (p_ptr->interactive_line > p_ptr->last_info_line)
				p_ptr->interactive_line = p_ptr->last_info_line;
			do_cmd_check_other(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_OBJECT:
			common_peruse(Ind, query);
			do_cmd_knowledge_object(Ind, p_ptr->interactive_line);	
			break;
		case SPECIAL_FILE_KILL:
			common_peruse(Ind, query);
			do_cmd_knowledge_kills(Ind, p_ptr->interactive_line);	
			break;
		case SPECIAL_FILE_HISTORY:
			common_peruse(Ind, query);
			do_cmd_knowledge_history(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_SCORES:
			common_peruse(Ind, query);
			display_scores(Ind, p_ptr->interactive_line);
			break;
		case SPECIAL_FILE_HOUSES:
			common_peruse(Ind, query);
			display_houses(Ind, query);
			break;
		case SPECIAL_FILE_HELP:
			common_file_peruse(Ind, query);
			do_cmd_check_other(Ind, p_ptr->interactive_line - p_ptr->interactive_next);
			break;	
		case SPECIAL_FILE_KNOWLEDGE:
			do_cmd_knowledge(Ind, query);
			break;
		case SPECIAL_FILE_MASTER:
			do_cmd_dungeon_master(Ind, query);
			break;
		case SPECIAL_FILE_INPUT:
			do_cmd_interactive_input(Ind, query);			
			break;
	}
}

void do_cmd_knowledge(int Ind, char query)
{
	player_type *p_ptr = Players[Ind];
	bool changed = FALSE;
	int i;
	
	/* Display */
	if (query == 0)
	{
		/* Prepare */
		text_out_init(Ind);
		
		/* Ask for a choice */
		text_out(" \n");
		text_out(" \n");
		text_out(" \n");
		text_out("Display current knowledge\n");
		text_out(" \n");

		/* Give some choices */
		text_out("    (1) Display known artifacts\n");
		text_out("    (2) Display known uniques\n");
		text_out("    (3) Display known objects\n");
		text_out("    (4) Display hall of fame\n");
		text_out("    (5) Display kill counts\n");
		text_out("    (6) Display owned houses\n");
		text_out("    (7) Display character history\n");
		text_out("    (8) Display self-knowledge\n");

		/* Prompt */
		text_out(" \n");
		text_out("Command: \n");
		
		text_out_done();
		
		/* Send */
		Send_term_info(Ind, NTERM_CLEAR, 1);
		for (i = 0; i < MAX_TXT_INFO; i++)
		{
			if (i >= p_ptr->last_info_line) break;
			Send_remote_line(Ind, i);
		}
		Send_term_info(Ind, NTERM_FLUSH, 0);
	}

	/* Proccess command - Switch mode */	
	switch (query)
	{
		case '1':
			Send_special_other(Ind, "Artifacts");
			p_ptr->special_file_type = SPECIAL_FILE_ARTIFACT;
			changed = TRUE;
			break;
		case '2':
			Send_special_other(Ind, "Uniques");
			p_ptr->special_file_type = SPECIAL_FILE_UNIQUE;
			changed = TRUE;
			break;
		case '3':
			Send_special_other(Ind, "Objects");
			p_ptr->special_file_type = SPECIAL_FILE_OBJECT;
			changed = TRUE;
			break;
		case '4':
			Send_special_other(Ind, "High Scores");
			p_ptr->special_file_type = SPECIAL_FILE_SCORES;
			changed = TRUE;
			break;
		case '5':
			Send_special_other(Ind, "Kill Count");
			p_ptr->special_file_type = SPECIAL_FILE_KILL;
			changed = TRUE;
			break;
		case '6':
			Send_special_other(Ind, "Owned Houses");
			p_ptr->special_file_type = SPECIAL_FILE_HOUSES;
			changed = TRUE;
			break;
		case '7':
			Send_special_other(Ind, "Character History");
			p_ptr->special_file_type = SPECIAL_FILE_HISTORY;
			changed = TRUE;
			break;
		case '8':
			self_knowledge(Ind, FALSE);
			p_ptr->special_file_type = SPECIAL_FILE_OTHER;
			changed = TRUE;
			break;			
	}
	
	/* HACK! - Move to another menu */
	if (changed)
	{	
		do_cmd_interactive_aux(Ind, p_ptr->special_file_type, 0);
	}
}

void do_cmd_interactive_input(int Ind, char query)
{
	player_type *p_ptr = Players[Ind];
	int i;
	bool done = FALSE;

	/* XXX HACK XXX Will be using "hook" to store info */
	char * old_file_type = &(p_ptr->interactive_hook[0][0]);
	char * mark = &(p_ptr->interactive_hook[0][1]);
	char * len = &(p_ptr->interactive_hook[0][2]);
	char * y = &(p_ptr->interactive_hook[0][3]);
	char * x = &(p_ptr->interactive_hook[0][4]);
	char * attr = &(p_ptr->interactive_hook[0][5]);
	char * mlen = &(p_ptr->interactive_hook[0][6]);	
	char * str = p_ptr->interactive_hook[1];

	switch(query)
	{
		case 0:

		*old_file_type = p_ptr->special_file_type;
		p_ptr->special_file_type = SPECIAL_FILE_INPUT;
		Send_term_info(Ind, NTERM_HOLD, 1);

		break;

		case ESCAPE:

		*len = 0;

		case '\n':
		case '\r':

		done = TRUE;
		
		break;

		case 0x7F:
		case '\010':

		if (*len) (*len)--;

		break;

		default:

		if (*len < 80 && isprint(query))
		{	
			str[(byte)(*len)++] = query;
		}
	}

	if (done || (*mlen && (*len >= *mlen)))
	{
		p_ptr->special_file_type = *old_file_type;
		Send_term_info(Ind, NTERM_HOLD, 2);
		do_cmd_interactive_aux(Ind, *old_file_type, *mark);
		return; 
	}

	/* Refresh client screen */
	for (i = 0; i < *len; i++)
	{
		Send_char(Ind, *x + i, *y, *attr, str[i], *attr, str[i]);
	}
	Send_char(Ind, *x + i, *y, TERM_WHITE, ' ', TERM_WHITE, ' ');

	Send_term_info(Ind, NTERM_FLUSH, 0);	
}

void do_cmd_interactive(int Ind, char query)
{
	player_type *p_ptr = Players[Ind];

	/* Hack -- use special term */
	Send_term_info(Ind, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);

	/* Perform action */	
	do_cmd_interactive_aux(Ind, p_ptr->special_file_type, query);

	/* Hack -- return to main term */
	Send_term_info(Ind, NTERM_ACTIVATE, NTERM_WIN_OVERHEAD);
}
