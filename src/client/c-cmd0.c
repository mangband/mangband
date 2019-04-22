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
#if 0
	{ "Port-specific preferences",    '!', do_cmd_port },
	{ "Save and don't quit",  KTRL('S'), do_cmd_save_game },
#endif
	{ "Save and quit",        KTRL('X'), NULL /*do_cmd_quit*/ },
	{ "Quit (commit suicide)",      'Q', NULL /*do_cmd_suicide*/ },
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
#if 0
	{ "Take notes",               ':', do_cmd_note },
	{ "Version info",             'V', do_cmd_version },
	{ "Load a single pref line",  '"', do_cmd_pref },
	{ "Mouse click",           '\xff', do_cmd_mouseclick },
	{ "Enter a store",            '_', do_cmd_store },
	{ "Toggle windows",     KTRL('E'), toggle_inven_equip }, /* XXX */
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
#if 0
static void do_cmd_port(void)
{
	(void)Term_user(0);
}
#endif


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
	section_icky_row = h + 2;
	section_icky_col = -w - 1;

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
	section_icky_row = h + 2;
	section_icky_col = -w - 1;

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
		if (strchr("[", c_ptr->m_catch)) list_id = CMD_LIST_INFO;
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