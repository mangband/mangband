/*
 * File: cmd0.c
 * Purpose: Deal with command processing.
 *
 * Copyright (c) 2007 Andrew Sidwell, Ben Harrison
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "c-angband.h"
/*#include "cmds.h"*/

#include <assert.h>


/*
 * This file contains (several) big lists of commands, so that they can be
 * easily maniuplated for e.g. help displays, or if a port wants to provide a
 * native menu containing a command list.
 *
 * Consider a two-paned layout for the command menus. XXX
 *
 * This file still needs some clearing up. XXX
 */
/* MAngband-specific flavor: the "big lists" are mostly empty, as the
 * commands are transfered from the server. We use "cmd_init()" (see below),
 * to actually populate the lists. */

/*** Big list of commands ***/

/* Useful typedef */
typedef void do_cmd_type(void);


#if 0 //unused -flm-
/* Forward declare these, because they're really defined later */
static do_cmd_type do_cmd_wizard, do_cmd_try_debug,
            do_cmd_cast_or_pray, do_cmd_quit, do_cmd_mouseclick, do_cmd_port,
            do_cmd_xxx_options, do_cmd_menu, do_cmd_monlist;
#endif

#ifdef ALLOW_BORG
static do_cmd_type do_cmd_try_borg;
#endif

/*
 * Holds a generic command.
 */
typedef struct
{
	const char *desc;
	unsigned char key;
	do_cmd_type *hook;
} command_type;


/* Magic use */
static command_type cmd_magic[128] =
{
#if 0
	{ "Gain new spells or prayers", 'G', do_cmd_study },
	{ "Browse a book",              'b', do_cmd_browse },
	{ "Cast a spell",               'm', do_cmd_cast_or_pray },
	{ "Pray a prayer",              'p', do_cmd_cast_or_pray }
#endif
	{ "", 0, NULL },
};

/* General actions */
static command_type cmd_action[128] =
{
#if 0
	{ "Search for traps/doors",     's', do_cmd_search },
	{ "Disarm a trap or chest",     'D', do_cmd_disarm },
	{ "Rest for a while",           'R', do_cmd_rest },
	{ "Look around",                'l', do_cmd_look },
	{ "Target monster or location", '*', do_cmd_target },
	{ "Dig a tunnel",               'T', do_cmd_tunnel },
	{ "Go up staircase",            '<', do_cmd_go_up },
	{ "Go down staircase",          '>', do_cmd_go_down },
	{ "Toggle search mode",         'S', do_cmd_toggle_search },
	{ "Open a door or a chest",     'o', do_cmd_open },
	{ "Close a door",               'c', do_cmd_close },
	{ "Jam a door shut",            'j', do_cmd_spike },
	{ "Bash a door open",           'B', do_cmd_bash }
#endif
	{ "", 0, NULL },
};

/* Item use commands */
static command_type cmd_item_use[128] =
{
#if 0
	{ "Read a scroll",            'r', do_cmd_read_scroll },
	{ "Quaff a potion",           'q', do_cmd_quaff_potion },
	{ "Use a staff",              'u', do_cmd_use_staff },
	{ "Aim a wand",               'a', do_cmd_aim_wand },
	{ "Zap a rod",                'z', do_cmd_zap_rod },
	{ "Activate an object",       'A', do_cmd_activate },
	{ "Eat some food",            'E', do_cmd_eat_food },
	{ "Fuel your light source",   'F', do_cmd_refill },
	{ "Fire your missile weapon", 'f', do_cmd_fire },
	{ "Throw an item",            'v', do_cmd_throw }
#endif
	{ "", 0, NULL },
};

/* Item management commands */
static command_type cmd_item_manage[128]  =
{
	{ "Display equipment listing", 'e', NULL/*do_cmd_equip*/ },
	{ "Display inventory listing", 'i', NULL/*do_cmd_inven*/ },
#if 0
	{ "Pick up objects",           'g', do_cmd_pickup },
	{ "Wear/wield an item",        'w', do_cmd_wield },
	{ "Take/unwield off an item",  't', do_cmd_takeoff },
	{ "Drop an item",              'd', do_cmd_drop },
	{ "Destroy an item",           'k', do_cmd_destroy },
	{ "Mark an item as squelch",   'K', do_cmd_mark_squelch },
	{ "Examine an item",           'I', do_cmd_observe },
	{ "Inscribe an object",        '{', do_cmd_inscribe },
	{ "Uninscribe an object",      '}', do_cmd_uninscribe }
#endif
	{ "", 0, NULL },
};

/* Information access commands */
static command_type cmd_info[128] =
{
	{ "Full dungeon map",             'M', NULL /*do_cmd_view_map*/ },
	{ "Locate player on map",         'L', NULL, /*do_cmd_locate*/ },
#if 0
	{ "Display visible monster list", '[', do_cmd_monlist },
	{ "Help",                         '?', do_cmd_help },
	{ "Identify symbol",              '/', do_cmd_query_symbol },
	{ "Character description",        'C', do_cmd_change_name },
	{ "Check knowledge",              '~', do_cmd_knowledge },
	{ "Repeat level feeling",   KTRL('F'), do_cmd_feeling },
	{ "Show previous message",  KTRL('O'), do_cmd_message_one },
	{ "Show previous messages", KTRL('P'), do_cmd_messages },
#endif
	{ "", 0, NULL },
};

/* Utility/assorted commands */
/* MAngband-specific: all callbacks are NULL */
static command_type cmd_util[128] =
{
	{ "Send chat message",            ':', NULL },
	{ "Manage chat channels",        '\'', NULL },
	{ "Describe item to chat",  KTRL('D'), NULL },
	{ "Party menu",                   'P', NULL },
	{ "Interact with options",        '=', NULL/*do_cmd_xxx_options*/ },
	{ "Interact with macros",         '%', NULL },
	{ "Port-specific preferences",    '!', NULL/*do_cmd_port*/ },
#if 0
	{ "Save and don't quit",  KTRL('S'), do_cmd_save_game },
#endif
	{ "Save and quit",        KTRL('X'), NULL /*do_cmd_quit*/ },
	{ "Quit (commit suicide)",      'Q', NULL /*do_cmd_suicide*/ },
	{ "Flip Inven/Equip windows", KTRL('E'), NULL /* toggle_inven_equip*/},
	{ "Redraw the screen",    KTRL('R'), NULL /*do_cmd_redraw*/ },
#if 0
	{ "Load \"screen dump\"",       '(', do_cmd_load_screen },
	{ "Save \"screen dump\"",       ')', do_cmd_save_screen },
#endif
	{ "", 0, NULL },
};

/* Commands that shouldn't be shown to the user */ 
static command_type cmd_hidden[] =
{
	{ "Use item (primary action)", KTRL('U'), NULL /*cmd_use_item*/ },
#if 0
	{ "Take notes",               ':', do_cmd_note },
	{ "Version info",             'V', do_cmd_version },
	{ "Load a single pref line",  '"', do_cmd_pref },
	{ "Mouse click",           '\xff', do_cmd_mouseclick },
	{ "Enter a store",            '_', do_cmd_store },

	{ "Alter a grid",             '+', do_cmd_alter },
	{ "Walk",                     ';', do_cmd_walk },
	{ "Jump into a trap",         '-', do_cmd_jump },
	{ "Start running",            '.', do_cmd_run },
	{ "Stand still",              ',', do_cmd_hold },
	{ "Check knowledge",          '|', do_cmd_knowledge },
	{ "Display menu of actions", '\n', do_cmd_menu },
	{ "Display menu of actions", '\r', do_cmd_menu },

	{ "Toggle wizard mode",  KTRL('W'), do_cmd_wizard },

#ifdef ALLOW_DEBUG
	{ "Debug mode commands", KTRL('A'), do_cmd_try_debug },
#endif
#ifdef ALLOW_BORG
	{ "Borg commands",       KTRL('Z'), do_cmd_try_borg },
#endif
#endif
	{ "", 0, NULL },
};


/*
 * A categorised list of all the command lists.
 */
typedef struct
{
	const char *name;
	command_type *list;
	size_t len;
} command_list;


static command_list cmds_all[] =
{
	{ "Use magic/Pray",  cmd_magic,       N_ELEMENTS(cmd_magic) },
	{ "Action commands", cmd_action,      N_ELEMENTS(cmd_action) },
	{ "Use item",        cmd_item_use,    N_ELEMENTS(cmd_item_use) },
	{ "Manage items",    cmd_item_manage, N_ELEMENTS(cmd_item_manage) },
	{ "Information",     cmd_info,        N_ELEMENTS(cmd_info) },
	{ "Utility",         cmd_util,        N_ELEMENTS(cmd_util) },
	{ "Hidden",          cmd_hidden,      N_ELEMENTS(cmd_hidden) }
};

/* We'll also need to index this list, so */
enum {
	CMD_LIST_MAGIC,
	CMD_LIST_ACTION,
	CMD_LIST_ITEM_USE,
	CMD_LIST_ITEM_MANAGE,
	CMD_LIST_INFO,
	CMD_LIST_UTIL,
	CMD_LIST_HIDDEN,
};




#if 0
/*
 * Toggle wizard mode
 */
static void do_cmd_wizard(void)
{
	/* Verify first time */
	if (!(p_ptr->noscore & NOSCORE_WIZARD))
	{
		/* Mention effects */
		msg_print("You are about to enter 'wizard' mode for the very first time!");
		msg_print("This is a form of cheating, and your game will not be scored!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_WIZARD;
	}

	/* Toggle mode */
	if (p_ptr->wizard)
	{
		p_ptr->wizard = FALSE;
		msg_print("Wizard mode off.");
	}
	else
	{
		p_ptr->wizard = TRUE;
		msg_print("Wizard mode on.");
	}

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw "title" */
	p_ptr->redraw |= (PR_TITLE);
}
#endif


#if 0
#ifdef ALLOW_DEBUG

/*
 * Verify use of "debug" mode
 */
static void do_cmd_try_debug(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_DEBUG))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, debug commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_DEBUG;
	}

	/* Okay */
	do_cmd_debug();
}

#endif /* ALLOW_DEBUG */
#endif


#if 0
#ifdef ALLOW_BORG

/*
 * Verify use of "borg" mode
 */
static bool do_cmd_try_borg(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_BORG))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, borg commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_BORG;
	}

	/* Okay */
	do_cmd_borg();
}

#endif /* ALLOW_BORG */
#endif


/*
 * Helper -- cast or pray, depending on the character.
 */
#if 0
static void do_cmd_cast_or_pray(void)
{
	if (cp_ptr->spell_book == TV_PRAYER_BOOK)
		do_cmd_pray();
	else
		do_cmd_cast();

}
#endif

/*
 * Quit the game.
 */
#if 0
static void do_cmd_quit(void)
{

	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;

}
#endif

/*
 * Handle a mouseclick, using the horrible hack that is '\xff'.
 */
#if 0
static void do_cmd_mouseclick(void)
{
	int x, y;

	if (!mouse_movement) return;

	y = KEY_GRID_Y(command_cmd_ex);
	x = KEY_GRID_X(command_cmd_ex);

	/* Check for a valid location */
	if (!in_bounds_fully(y, x)) return;

	/* XXX We could try various things here like going up/down stairs */
	if ((p_ptr->py == y) && (p_ptr->px == x) /* && (p_ptr->command_cmd_ex.mousebutton) */)
	{
		do_cmd_rest();
	}
	else /* if (p_ptr->command_cmd_ex.mousebutton == 1) */
	{
		if (p_ptr->confused)
		{
			do_cmd_walk();
		}
		else
		{
			do_cmd_pathfind(y, x);
		}
	}
	/*
	else if (p_ptr->command_cmd_ex.mousebutton == 2)
	{
		target_set_location(y, x);
		msg_print("Target set.");
	}
	*/
}
#endif



/*
 * Port-specific options
 *
 * Should be moved to the options screen. XXX
 */
void do_cmd_port(void)
{
	(void)Term_user(0);
}



/*
 * Display the options and redraw afterward.
 */
#if 0
static void do_cmd_xxx_options(void)
{
	do_cmd_options();
	do_cmd_redraw();
}
#endif



/*
 * Display the main-screen monster list.
 */
#if 0
static void do_cmd_monlist(void)
{
	/* Save the screen and display the list */
	Term_save(); /*screen_save();*/
	display_monlist();

	/* Wait */
	inkey();

	/* Return */
	Term_load(); /*screen_load();*/

}
#endif

/*
 * Invoked when the command isn't recognised.
 */
static void do_cmd_unknown(void)
{
	prt("Type '?' for help.", 0, 0);
}



/* List indexed by char */
do_cmd_type *converted_list[UCHAR_MAX+1];


/*** Menu functions ***/

/* Display an entry on a command menu */
static void cmd_sub_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	const command_type *commands = menu->menu_data;

	/* Write the description */
	Term_putstr(col, row, -1, attr, commands[oid].desc);

	/* Include keypress */
	Term_addch(attr, ' ');
	Term_addch(attr, '(');

	/* KTRL()ing a control character does not alter it at all */
	if (KTRL(commands[oid].key) == commands[oid].key)
	{
		Term_addch(attr, '^');
		Term_addch(attr, UN_KTRL(commands[oid].key));
	}
	else
	{
		Term_addch(attr, commands[oid].key);
	}

	Term_addch(attr, ')');
}


/* Handle user input from a command menu */
static bool cmd_sub_action(char cmd, void *db, int oid)
{
/* XXX - Doesn't work, better not handle ANYTHING - XXX */
return FALSE;
	/* Only handle enter */
	if (cmd == '\n' || cmd == '\r')
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 * Display a list of commands.
 */
static bool cmd_menu(command_list *list, void *selection_p)
{
	menu_type menu;
	menu_iter commands_menu = { 0, 0, 0, cmd_sub_entry, cmd_sub_action };
	region area = { 23, 4, 37, 13 };

	event_type evt;
	int cursor = 0;
	command_type *selection = selection_p;

	/* MAngband-specific: handle term sizes */
	int x = 21, y = 3, w = 38, h = 13;
	x = Term->wid - w - 1; /* Hack -- top-right corner */
	y = 1;
	area.col = x + 2;
	area.row = y + 1;
	area.width = w - 2;
	area.page_rows = h - 1;

	/* Set up the menu */
	WIPE(&menu, menu);
	menu.cmd_keys = "\x8B\x8C\n\r";
	menu.count = list->len;
	menu.menu_data = list->list;
	menu_init2(&menu, find_menu_skin(MN_SCROLL), &commands_menu, &area);

	/* Set up the screen */
	Term_save(); /*screen_save();*/
	window_make(x, y, x + w, y + h);

	/* MAngband-specific: icky-fy section */
	if (!screen_icky)
	{
		section_icky_row = h + 2;
		section_icky_col = -w - 1;
	}

	/* Select an entry */
	evt = menu_select(&menu, &cursor, 0);

	/* Load de screen */
	Term_load(); /*screen_load();*/
	section_icky_row = section_icky_col = 0; /* Unicky-fy */

	if (evt.type == EVT_SELECT)
	{
		*selection = list->list[evt.index];
	}

	if (evt.type == EVT_ESCAPE)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


#if 0
static bool cmd_list_action(char cmd, void *db, int oid)
{
	if (cmd == '\n' || cmd == '\r' || cmd == '\xff')
	{
		/* NOTE -- "cmd_list_action" is currently unused,
		 * but had it been used, this is the proper way
		 * to handle screen ickyness for nested menus: */
		bool ok;
		s16b old_icky_row = section_icky_row;
		s16b old_icky_col = section_icky_col;
		Term_load();
		Term_save();
		ok = cmd_menu(&cmds_all[oid], db);
		Term_load();
		Term_save();
		section_icky_row = old_icky_row;
		section_icky_col = old_icky_col;
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}
#endif

static void cmd_list_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	Term_putstr(col, row, -1, attr, cmds_all[oid].name);
}

/*
 * Display a list of command types, allowing the user to select one.
 */
char do_cmd_menu(void)
{
	menu_type menu;
	menu_iter commands_menu = { 0, 0, 0, cmd_list_entry, NULL/* cmd_list_action*/ };
	region area = { 21, 5, 37, 6 };

	event_type evt;
	int cursor = 0;
	command_type chosen_command = { NULL, 0, NULL };

	/* MAngband-specific YUCKY HACK: instead of recursing, we loop here */
	bool done = FALSE;

	/* MAngband-specific: handle term sizes */
	int x = 19, y = 4, w = 18, h = 7;
	x = Term->wid - w - 1; /* Hack -- top-right corner */
	y = 1;
	area.col = x + 2;
	area.row = y + 1;
	area.width = w - 2;
	area.page_rows = h - 1;

	/* Set up the menu */
	WIPE(&menu, menu);
	menu.cmd_keys = "\x8B\x8C\n\r";
	menu.count = N_ELEMENTS(cmds_all) - 1;
	menu.menu_data = &chosen_command;
	menu_init2(&menu, find_menu_skin(MN_SCROLL), &commands_menu, &area);

  /* MAngband-specific hack: loop instead of recursion */
  while (!done) {

	/* Set up the screen */
	Term_save(); /*screen_save();*/
	window_make(x, y, x + w, y + h);

	/* MAngband-specific: icky-fy section */
	if (!screen_icky)
	{
		section_icky_row = h + 2;
		section_icky_col = -w - 1;
	}

	/* Select an entry */
	evt = menu_select(&menu, &cursor, 0);

	/* Load de screen */
	Term_load();/*screen_load();*/
	section_icky_row = section_icky_col = 0; /* Unicky-fy */

	/* Enter submenu */
	if (evt.type == EVT_SELECT)
	{
		done = cmd_menu(&cmds_all[evt.index], &chosen_command);
		cursor = evt.index;
	}
	/* Anything else probably means we're quitting the men */
	else done = TRUE;

  } /* End yucky hack. */

	/* If a command was chosen, do it. */
	if (chosen_command.key)
	{
		/* command_cmd = chosen_command.key; */
		/* Our version of the function returns the value,
		 * instead of injecting it into the queue */
		return chosen_command.key;
	}

	return '\r';
}


/*** Exported functions ***/

static void cmd_init_one(int list_id, const char* name, char key)
{
	command_list *l = &cmds_all[list_id];
	command_type *c = &(l->list[l->len]);

	c->desc = name;
	c->key = key;
	c->hook = NULL;

	l->len++;
}


/*
 * Initialise the command list.
 */
void cmd_init(void)
{
	size_t i, j;

	/*** MAngband-specific hack begins here ***/
	/* Wipe lists */
	for (j = 0; j < N_ELEMENTS(cmds_all); j++)
	{
		cmds_all[j].len = 0;
		for (i = 0; i < 128; i++)
		{
			if (cmds_all[j].list[i].key == 0) break;
			cmds_all[j].len += 1;
		}
	}

	/* Hack -- add moving and resting commands */
	cmd_init_one(CMD_LIST_ACTION, "Look around", 'l');
	cmd_init_one(CMD_LIST_ACTION, "Target monster or location", '*');
	cmd_init_one(CMD_LIST_ACTION, "Target a friend", '(');
	cmd_init_one(CMD_LIST_ACTION, "Resting mode", 'R');
	cmd_init_one(CMD_LIST_ACTION, "Walk", ';');
	/* Hack -- add more local commands */
	cmd_init_one(CMD_LIST_INFO, "Character description",'C');
	cmd_init_one(CMD_LIST_INFO, "Show previous message",KTRL('O'));
	cmd_init_one(CMD_LIST_INFO, "Show previous messages",KTRL('P'));
	/* Go through every command */
	for (i = 0; i < custom_commands; i++)
	{
		custom_command_type *c_ptr = &custom_command[i];
		int list_id = CMD_LIST_ACTION;

		/* Skip store commands */
		if ((c_ptr->flag & COMMAND_STORE)) continue;

		/* XXX Hack -- ignore minimap */
		if (c_ptr->m_catch == 'M') continue;

		/* Hack -- hardcode some commands */
		if (strchr("wtdkI${}", c_ptr->m_catch))
		{
			list_id = CMD_LIST_ITEM_MANAGE;
		}
		/* Hack -- try to guess */
		else if ((c_ptr->flag & COMMAND_SPELL_BOOK) || (c_ptr->flag & COMMAND_SPELL_CUSTOM))
		{
			list_id = CMD_LIST_MAGIC;
		}
		else if ((c_ptr->flag & COMMAND_TARGET_DIR))
		{
			list_id = CMD_LIST_ACTION;
		}
		else if ((c_ptr->flag & COMMAND_NEED_ITEM))
		{
			list_id = CMD_LIST_ITEM_USE;
		}
		else if ((c_ptr->flag & COMMAND_INTERACTIVE) || (c_ptr->flag & COMMAND_NEED_CHAR))
		{
			list_id = CMD_LIST_INFO;
		}
		if (c_ptr->m_catch == 'G') list_id = CMD_LIST_MAGIC;
		if (strchr("[]", c_ptr->m_catch)) list_id = CMD_LIST_INFO;
		if (c_ptr->m_catch == KTRL('F')) list_id = CMD_LIST_INFO;

		cmd_init_one(list_id, c_ptr->display, c_ptr->m_catch);
	}
	/** MAngband-specific hack ends here **/

	/* Go through all the lists of commands */
	for (j = 0; j < N_ELEMENTS(cmds_all); j++)
	{
		command_type *commands = cmds_all[j].list;

		/* Fill everything in */
		for (i = 0; i < cmds_all[j].len; i++)
		{
			unsigned char key = commands[i].key;

			/* Note: at present converted_list is UCHAR_MAX + 1 
			   large, so 'key' is always a valid index. */
			converted_list[key] = commands[i].hook;
		}
	}

	/* Fill in the rest */
	for (i = 0; i < N_ELEMENTS(converted_list); i++)
	{
		switch (i)
		{
			/* Ignore */
			case ESCAPE:
			case ' ':
			case '\a':
			{
				break;
			}

			default:
			{
				if (!converted_list[i])
					converted_list[i] = do_cmd_unknown;
			}
		}
	}
}


#if 0
/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void process_command(bool no_request)
{
	if (!no_request)
		request_command(FALSE);

	/* Handle repeating the last command */
	repeat_check();

	/* Handle resize events XXX */
	if (command_cmd_ex.type == EVT_RESIZE)
	{
		do_cmd_redraw();
	}
	else
	{
		/* Within these boundaries, the cast to unsigned char will have the desired effect */
		assert(command_cmd >= CHAR_MIN && command_cmd <= CHAR_MAX);

		/* Execute the command */
		if (converted_list[(unsigned char) command_cmd])
			converted_list[(unsigned char) command_cmd]();
	}
}
#endif

/** Macro helpers **/

char* macro_find_by_action(cptr buf)
{
	int i, t, n = -1, s = -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; i++)
	{
		/* Skip inactive macros */
		if (macro__cmd[i]) continue;

		/* Check for "prefix" */
		if (!streq(buf, macro__act[i])) continue;

		/* Return the trigger */
		return (char*)macro__pat[i];
	}

	/* Return the result */
	return NULL;
}

/*
 * In theory, we should have some kind of a "command_struct",
 * with all the possible data we might ever need.
 * However, what we actually have is:
 * - custom_command_type, with most of the data, which covers
     90% of commands
 * - command_type, with remaining 10% of commands and no usable
     data at all.
 */
custom_command_type *match_custom_command(char cmd, bool shop)
{
	byte i;
	for (i = 0; i < custom_commands; i++)
	{
		if (shop != ((custom_command[i].flag & COMMAND_STORE) ? TRUE : FALSE))
			continue;
		if (custom_command[i].m_catch == cmd)
		{
			return &custom_command[i];
		}
	}
	return NULL;
}
command_type *match_builtin_command(char cmd)
{
	size_t i, j;
	for (j = 0; j < N_ELEMENTS(cmds_all); j++)
	{
		for (i = 0; i < cmds_all[j].len; i++)
		{
			if (cmds_all[j].list[i].key == cmd)
			{
				return &cmds_all[j].list[i];
			}
		}
	}
	return NULL;
}
int command_to_display_name(char cmd, char *dst, size_t len)
{
	custom_command_type *cc_ptr;
	command_type *cmd_ptr;

	if ((cc_ptr = match_custom_command(cmd, FALSE)))
	{
		my_strcpy(dst, cc_ptr->display, len);
		return strlen(dst);
	}
	else if ((cmd_ptr = match_builtin_command(cmd)))
	{
		my_strcpy(dst, cmd_ptr->desc, len);
		return strlen(dst);
	}
	return 0;
}
char command_from_keystroke(char *buf)
{
	char *p;
	for (p = buf; *p; p++)
	{
		if (*p == '\\') continue;
		if ((KTRL(*p) == *p) && (isalpha(UN_KTRL(*p))))
		{
			return (*p);
		}
		if ((char)(*p) <= 32)
		{
			continue;
		}
		/* Return anything viable */
		return (*p);
	}
	return (0);
}

/* This is like "item_tester_okay", in reverse. Given an item
 * and a command, we check if this command could be applied
 * to a given item. */
bool command_tester_okay(int custom_command_id, int item)
{
	object_type *o_ptr;
	custom_command_type *cc_ptr = &custom_command[custom_command_id];

	if (item < 0) o_ptr = &floor_item;
	else o_ptr = &inventory[item];

	/* Skip non-item commands */
	if (!(cc_ptr->flag & COMMAND_NEED_ITEM)) return FALSE;

	/* Generic item commands (like drop or inscribe) */
	if (!cc_ptr->tval) return TRUE;

	/* Check the fake hook */
	if (cc_ptr->tval > TV_MAX)
	{
		if (!item_tester_hack(o_ptr, cc_ptr->tval - TV_MAX - 1))
		{
			return FALSE;
		}
	}
	/* Or direct (mis)match */
	else if (!(cc_ptr->tval == o_ptr->tval))
	{
		return FALSE;
	}
	return TRUE;
}

char command_by_item(int item, bool agressive)
{
	byte i;
	object_type *o_ptr;

	o_ptr = &inventory[item];
	for (i = 0; i < custom_commands; i++)
	{
		custom_command_type *cc_ptr = &custom_command[i];
		if (!command_tester_okay(i, item)) continue;

		/* Skip generic commands */
		if (!cc_ptr->tval) continue;

		/* Spell command hacks */
		if (cc_ptr->flag & COMMAND_SPELL_BOOK)
		{
			if (!agressive) return 'b';
			if (agressive && !(cc_ptr->flag & COMMAND_TARGET_ALLOW)) continue;
		}
		return cc_ptr->m_catch;
	}
	return 0;
}

/* Populate buffer "dst" (of length "len") with keystrokes
 * that could be used to invoke command "cmd".
 */
int command_as_keystroke(char cmd, char *dst, size_t len)
{
	char tmp[32];
	char *p = tmp;
	/* Hack -- dump the value */
	*p++ = '\\';
	*p++ = 'e';
	if (rogue_like_commands)
	{
		*p++ = '\\';
		*p++ = '\\';
	}
	if (KTRL(cmd) == cmd)
	{
		*p++ = '^';
		*p++ = UN_KTRL(cmd);
	}
	else {
		*p++ = cmd;
	}
	*p++ = '\0';
	my_strcpy(dst, tmp, len);
	return strlen(dst);
}

/* Given an "item", populate buffer "dst" (of length "len") with
 * keystrokes that could be used to select this item.
 *
 * For items tagged @x1, the tagged version will be returned.
 * For non-tagged items, full item name in quotes will be returned.
 */
int item_as_keystroke(int item, char cmd, char *dst, size_t len, byte ctxt_flag)
{
	/* Step one, see if it's tagged */
	char *s, *p;
	char buf[32];
	char buf2[1024];
	int tag = -1;

	if (item < 0 && item > -FLOOR_TOTAL-1)
	{
		my_strcpy(buf2, floor_name_one, 1024);
	}
	else
	{
		my_strcpy(buf2, inventory_name_one[item], 1024);
	}

	if (ctxt_flag & CTXT_WITH_CMD)
	{
		command_as_keystroke(cmd, dst, len);
	}

	if (!(ctxt_flag & CTXT_PREFER_NAME))
	{
		/* Find a '@' */
		s = strchr(buf2, '@');

		/* Process all tags */
		while (s)
		{
			/* Check the special tags */
			if ((s[1] == cmd) && (isdigit(s[2])))
			{
				tag = s[2] - '0';
				/* Success */
				break;
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}
	/* We have a nice tag */
	if (tag > -1)
	{
		buf[0] = '0' + tag;
		buf[1] = '\0';
		my_strcat(dst, buf, len);
		return strlen(dst);
	}

	/* To hell with it, let's use inventory index */
	if (ctxt_flag & CTXT_PREFER_SHORT)
	{
		char key = 'a';
		if (item < 0) key = '-' + 1;
		if (item < INVEN_WIELD)
		{
			buf[0] = key + item;
			buf[1] = '\0';
			my_strcat(dst, buf, len);
			return 1;
		}
		else
		{
			custom_command_type *cc_ptr = match_custom_command(cmd, shopping);
			if (cc_ptr && (cc_ptr->flag & COMMAND_ITEM_INVEN)) my_strcat(dst, "/", len);
			buf[0] = key + item - INVEN_WIELD;
			buf[1] = '\0';
			my_strcat(dst, buf, len);
			return 1;
		}
	}

	/* Trim full name */
	p = buf2;
	if ((s = strchr(p, '[')) && *(s+1) && !isdigit(*(s+1))) p = s;
	else if ((s = strchr(p, '['))) *s = '\0';
	else if ((s = strstr(p, " of "))) p = s + 4;
	if ((s = strchr(p, '('))) *s = '\0';
	if ((s = strchr(p, '{'))) *s = '\0';
	if (prefix(p, "a ")) p += 2;
	while (*p && isdigit(*p)) p++;
	if (*p == ' ') p++;

	/* Use full name */
	my_strcat(dst, "\"", len);
	my_strcat(dst, p, len);
	my_strcat(dst, "\"", len);

	return strlen(dst);
}

/* Given a "spell" in item "book", populate buffer "dst"
 * (of length "len") with keystrokes that could be used to
 * select this spell.
 *
 * To expand the sequence with command, book selection and/or
 * targeting keystrokes, set CTXT_WITH_CMD, CTXT_WITH_ITEM,
 * CTXT_WITH_DIR bit flags in "ctxt_flag".
 */
int spell_as_keystroke(int spell, int book, char cmd, char *dst, size_t len, byte ctxt_flag)
{
	int index;
	char base_key;
	char buf[2] = { '"', '\0' };
	char tmp[1024];
	tmp[0] = '\0';

	/* Projected spells use uppercase letter */
	base_key = 'a';
	if (spell >= SPELL_PROJECTED)
	{
		spell -= SPELL_PROJECTED;
		base_key = 'A';
		ctxt_flag |= CTXT_PREFER_SHORT;
	}

	/* Prefix */
	if (ctxt_flag & CTXT_WITH_CMD)
	{
		command_as_keystroke(cmd, tmp, 1024);
		ctxt_flag &= ~(CTXT_WITH_CMD);
	}
	if (ctxt_flag & CTXT_WITH_ITEM)
	{
		item_as_keystroke(book, cmd, tmp, 1024, ctxt_flag);
	}

	/* The spell itself */
	if (ctxt_flag & CTXT_PREFER_SHORT)
	{
		/* A spell has a simple index */
		buf[0] = spell + base_key;
		my_strcat(tmp, buf, 1024);
	}
	else
	{
		char *s, *p;
		char buf2[1024];
		my_strcpy(buf2, spell_info[book][spell], 1024);
		/* Trim full name */
		p = buf2;
		p += 4;
		if ((s = strchr(p, '('))) p = s + 1;
		if ((s = strstr(p, "  "))) s[0] = '\0';
		if (*p == ' ') p++;

		/* Use spell name */
		my_strcat(tmp, "\"", 1024);
		my_strcat(tmp, p, 1024);
		my_strcat(tmp, "\"", 1024);
	}

	/* Finish it */
	my_strcpy(dst, tmp, len);
	return strlen(dst);
}

/* The following functions handle clicks on sub-windows */
static int mouse_button_mods(int button, int *mods)
{
	*mods = 0;
	if ((button & 16)) { button &= ~(16); *mods |= MCURSOR_KTRL; }
	if ((button & 32)) { button &= ~(32); *mods |= MCURSOR_SHFT; }
	if ((button & 64)) { button &= ~(64); *mods |= MCURSOR_ALTR; }
	return button;
}
static void do_cmd_term_inject_header(cptr termdesc, int termuni, int button)
{
	char tmp[1024];
	char *p;
	strnfmt(tmp, sizeof(tmp),
		"%c_TERM%s_%02x_MB%02x%c", 31,
		 termdesc, termuni, button, 13);
	for (p = tmp; *p; p++) Term_keypress(*p);
	Term_keypress(28);/* Enter arcane macro mode: default action */
}
static void do_cmd_term_inject(char *buf)
{
	char tmp[1024];
	int i, n;
	text_to_ascii(tmp, sizeof(tmp), buf);
	n = strlen(tmp);
	Term_keypress(29);
	for (i = 0; i < n; i++) Term_keypress(tmp[i]);
	Term_keypress(30);
}
static bool do_cmd_term_inject_item(int item, char cmd)
{
	char ks[1024];
	if (item_as_keystroke(item, cmd, ks, sizeof(ks), CTXT_WITH_CMD | CTXT_PREFER_SHORT))
	{
		/* Hack -- when destroying, dropping or selling */
		if (cmd == 'k' || cmd == 'd' || cmd == 's')
		{
			int num = item < 0 ? floor_item.number : inventory[item].number;
			/* Will we enter item number prompt? */
			if (num > 1)
			{
				if (cmd == 'd') my_strcat(ks, "1\\r", sizeof(ks)); /* 1 item */
				else my_strcat(ks, "A\\r", sizeof(ks)); /* 'A'll items */
			}
			/* Will we enter confirmation mode? */
			if (cmd == 'k' && !auto_accept)
			{
				my_strcat(ks, "y", sizeof(ks));
			}
		}
		/* Hack -- do not \eSCAPE sell commands */
		if (cmd == 's' || shopping) ks[1] = 'r';

		do_cmd_term_inject(ks);
		return TRUE;
	}
	return FALSE;
}
static u32b do_cmd_term_inven(int x, int y, int button)
{
	int item = y;
	char cmd = 0;
	int btn, mods = 0;

	if (item >= INVEN_WIELD) return (0);

	btn = mouse_button_mods(button, &mods);

	/* HACK -- item prompt */
	if (btn == 1 && in_item_prompt)
	{
		Term_keypress(item + 'a');
		return (PW_INVEN);
	}

	/* Get rid of */
	if ((btn == 1) && (mods & MCURSOR_KTRL))
	{
		cmd = 'd';
		if (shopping) cmd = 's';
	}
	/* Apply item */
	else if (((btn == 1) && (mods & MCURSOR_SHFT)))
	{
		cmd = command_by_item(item, FALSE);
		if (shopping) cmd = 0;
	}
	/* Inspect item */
	else if (btn == 1)
	{
		cmd = 'I';
		if (shopping) cmd = 0;
	}

	do_cmd_term_inject_header(shopping ? "invenshop" : "inven", item, button);
	if (cmd && do_cmd_term_inject_item(item, cmd)) return (PW_INVEN);
	else Term_keypress(30); /* footer */

	return (0);
}
static u32b do_cmd_term_floor(int x, int y, int button)
{
	int item = -1;
	char cmd = 0;
	int btn, mods = 0;

	if (shopping) return 0;

	btn = mouse_button_mods(button, &mods);

	/* Hack -- item prompt */
	if (btn == 1 && in_item_prompt)
	{
		Term_keypress('-');
		return (PW_EQUIP);
	}

	/* Get rid of */
	if ((btn == 1) && (mods & MCURSOR_KTRL))
	{
		cmd = 'k';
	}
	/* Pick it up */
	else if (((btn == 1) && (mods & MCURSOR_SHFT)))
	{
		cmd = 'g';
	}
	/* Inspect item */
	else if (btn == 1)
	{
		cmd = 'I';
	}
	do_cmd_term_inject_header("floor", 0 - item, button);
	if (cmd && do_cmd_term_inject_item(item, cmd)) return (PW_EQUIP);
	else Term_keypress(30); /* footer */
	return (0);
}
static u32b do_cmd_term_equip(int x, int y, int button)
{
	int item = y + INVEN_WIELD;
	char cmd = 0;
	int btn, mods = 0;

	if (y == INVEN_TOTAL - INVEN_WIELD)
	{
		return do_cmd_term_floor(x, y, button);
	}
	if (y > INVEN_TOTAL - INVEN_WIELD) return 0;

	btn = mouse_button_mods(button, &mods);

	/* Hack -- item prompt */
	if (btn == 1 && in_item_prompt)
	{
		if (!command_wrk) Term_keypress('/');
		Term_keypress(item - INVEN_WIELD + 'a');
		return (PW_EQUIP);
	}

	/* Get rid of */
	if ((btn == 1) && (mods & MCURSOR_KTRL))
	{
		cmd = 't';
		if (shopping) cmd = 's';
	}
	/* Activate item */
	else if (((btn == 1) && (mods & MCURSOR_SHFT)))
	{
		cmd = 'A';
		if (shopping) cmd = 0;
	}
	/* Inspect item */
	else if (btn == 1)
	{
		cmd = 'I';
		if (shopping) cmd = 0;
	}

	do_cmd_term_inject_header(shopping ? "equipshop" : "equip", item - INVEN_WIELD, button);
	if (cmd && do_cmd_term_inject_item(item, cmd)) return (PW_EQUIP);
	else Term_keypress(30); /* footer */

	return (0);
}
static u32b do_cmd_term_spells(int x, int y, int button)
{
	byte old_tester;
	int i, b, l = 0;
	int found = -1, book = -1;
	int h = Term->hgt;

	/* Save/Switch tester */
	old_tester = item_tester_tval;
	item_tester_tval = c_info[pclass].spell_book;

	/* For each book */
	for (b = 0; b < INVEN_PACK - 1; b++)
	{
		if (found != -1) break;
		if (item_tester_okay(&inventory[b]))
		{
			/* Dump the spells */
			for (i = 0; i < SPELLS_PER_BOOK; i++)
			{
				/* End of terminal */
				if (y >= h) break;

				/* Check for end of the book */
				if (spell_info[b][i][0] == '\0')
					break;
				
				/* skip Illegible */
				if (strstr(spell_info[b][i], "(illegible)"))
					continue;
					
				/* skip uncastable */
				if (!(spell_flag[b*SPELLS_PER_BOOK+i] & PY_SPELL_LEARNED))
					continue;
				
				/* skip forgotten */
				if (spell_flag[b*SPELLS_PER_BOOK+i] & PY_SPELL_FORGOTTEN)
					continue;

				if (y - 1 == l)
				{
					book = b;
					found = i;
					break;
				}
				l++;
			}
		}
	}

	/* Restore old tester */
	item_tester_tval = old_tester;

	if (found != -1)
	{
		char ks[1024];
		char cmd;
		cmd = command_by_item(book, TRUE);
		spell_as_keystroke(found, book, cmd, ks, 1024, CTXT_FULL | CTXT_PREFER_SHORT);

		do_cmd_term_inject(ks);
	}

	return (0);
}
static u32b do_cmd_term_charsheet(int x, int y, int button)
{
	flip_charsheet++;
	if (flip_charsheet >= 3) flip_charsheet = 0;
	return PW_PLAYER_0;
}
static u32b do_cmd_term_chat(int x, int y, int button)
{
	int j, n;
	int c, t;
	int w = Term->wid;
	int i = -1;

	/* Chat header */
	c = t = 0; /* Hor. & Vert. Offsets */
	for (j = 0; j < MAX_CHANNELS; j++)
	{
		/* Skip empty */
		if (STRZERO(channels[j].name)) continue;

		/* Carriage return */
		n = (int)strlen(channels[j].name);
		if (n + c + 1 >= w)
		{
			c = 0;
			t++;
		}
		if (x >= c && x <= c + n && y == t)
		{
			i = j;
			break;
		}
		c += n + 1;
	}
	if (i != -1)
	{
		view_channel = i;
		return (PW_MESSAGE_CHAT);
	}
	return (0);
}
static u32b do_cmd_term_store(int x, int y, int button)
{
	int item = y - 6;
	char cmd = 0;
	int btn, mods = 0;

	if (item < 0 || item >= 12) return 0;

	btn = mouse_button_mods(button, &mods);

	/* Purchase item */
	if ((btn == 1) && (mods & MCURSOR_SHFT))
	{
		cmd = 'p';
	}
	/* Inspect item */
	else if (btn == 1)
	{
		cmd = 'l';
	}
	if (cmd)
	{
		do_cmd_term_inject_header("store", item, button);
		if (do_cmd_term_inject_item(item, cmd)) return PW_STORE;
		else Term_keypress(30); /* footer */
	}
	return PW_STORE;
}

void do_cmd_term_mousepress(u32b termflag, int x, int y, int button)
{
	u32b update = 0;
	if ((termflag & PW_INVEN))
	{
		if (!flip_inven) update |= do_cmd_term_inven(x, y, button);
		else update |= do_cmd_term_equip(x, y, button);
	}
	else if ((termflag & PW_EQUIP))
	{
		if (!flip_inven) update |= do_cmd_term_equip(x, y, button);
		else update |= do_cmd_term_inven(x, y, button);
	}
	else if ((termflag & PW_SPELL))
	{
		update |= do_cmd_term_spells(x, y, button);
	}
	else if ((termflag & PW_PLAYER_0))
	{
		update |= do_cmd_term_charsheet(x, y, button);
	}
	else if ((termflag & PW_MESSAGE_CHAT))
	{
		update |= do_cmd_term_chat(x, y, button);
	}
	else if ((termflag & PW_STORE))
	{
		update |= do_cmd_term_store(x, y, button);
	}
	p_ptr->window |= update;
}
void cmd_term_mousepress(int i, int x, int y, int button)
{
	u32b flag = window_flag[i];
	if (state != PLAYER_PLAYING) return;
	do_cmd_term_mousepress(flag, x, y, button);
}

void do_cmd_use_item(int item, bool agressive)
{
	char cmd;
	cmd = command_by_item(item, agressive);
	/* HACK -- should wire into process_command() instead */
	do_cmd_term_inject_item(item, cmd);
}

void cmd_use_item(void)
{
	int item;
	if (!c_get_item(&item, "Use which item? ", TRUE, TRUE, TRUE))
	{
		return;
	}
	do_cmd_use_item(item, TRUE);
}
