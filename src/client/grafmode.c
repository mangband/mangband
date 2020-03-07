/*
 * File: grafmode.c
 * Purpose: load a list of possible graphics modes.
 *
 * Copyright (c) 2011 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of:
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "c-angband.h"
#include "grafmode.h"
#include "../common/parser.h"

/* Forward-compatibility with V4XX */
#define mem_zalloc ralloc
#define mem_free rnfree
/* XXX */

graphics_mode *graphics_modes = NULL;
graphics_mode *current_graphics_mode = NULL;
int graphics_mode_high_id = 0;
int num_graphics_modes = 0;

static enum parser_error parse_graf_n(struct parser *p) {
	int i;
	graphics_mode *list = parser_priv(p);
	graphics_mode *mode = malloc(sizeof(graphics_mode));
	if (!mode) {
		return PARSE_ERROR_OUT_OF_MEMORY;
	}
	mode->pNext = list;
	mode->grafID = parser_getuint(p, "index");
	strncpy(mode->menuname, parser_getstr(p, "menuname"), 32);

	mode->alphablend = 0;
	mode->overdrawRow = 0;
	mode->overdrawMax = 0;
	strncpy(mode->file, "", 32);

	strncpy(mode->pref, "none", 32);

	/* MAngband-specific properties: */
	strncpy(mode->mask, "", 32);
	mode->transparent = 0;
	mode->lightmap = 0;
	for (i = 0; i < 4; i++) {
		mode->light_offset[i][0] = 0;
		mode->light_offset[i][1] = 0;
	}
	parser_setpriv(p, mode);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_i(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->cell_width = parser_getuint(p, "wid");
	mode->cell_height = parser_getuint(p, "hgt");
	strncpy(mode->file, parser_getstr(p, "filename"), 32);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_m(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	strncpy(mode->mask, parser_getstr(p, "filename"), 32);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_p(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	strncpy(mode->pref, parser_getstr(p, "prefname"), 32);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_x(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->alphablend = parser_getuint(p, "alpha");
	mode->overdrawRow = parser_getuint(p, "row");
	mode->overdrawMax = parser_getuint(p, "max");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_t(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->transparent = parser_getuint(p, "transparent");
	mode->lightmap    = parser_getuint(p, "lightmap");
	if (!mode->transparent) mode->lightmap = 0;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_l(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	u32b level;
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	level = parser_getuint(p, "level");
	if (level > 3) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->light_offset[level][0] = parser_getuint(p, "yoffset");
	mode->light_offset[level][1] = parser_getuint(p, "xoffset");
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_grafmode(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str menuname", parse_graf_n);
	parser_reg(p, "I uint wid uint hgt str filename", parse_graf_i);
	parser_reg(p, "M str maskfilename", parse_graf_m);
	parser_reg(p, "P str prefname", parse_graf_p);
	parser_reg(p, "X uint alpha uint row uint max", parse_graf_x);
	parser_reg(p, "T uint transparent uint lightmap", parse_graf_t);
	parser_reg(p, "L uint level int yoffset int xoffset", parse_graf_l);

	return p;
}

errr finish_parse_grafmode(struct parser *p) {
	graphics_mode *mode, *n;
	int max = 0;
	int count = 0;
	int i;
	
	/* see how many graphics modes we have and what the highest index is */
	if (p) {
		mode = parser_priv(p);
		while (mode) {
			if (mode->grafID > max) {
				max = mode->grafID;
			}
			count++;
			mode = mode->pNext;
		}
	}

	/* copy the loaded modes to the global variable */
	graphics_modes = realloc(graphics_modes, sizeof(graphics_mode) * (count+1+num_graphics_modes));
	for (i = 0; i < num_graphics_modes; i++) {
		graphics_modes[i].pNext = &(graphics_modes[i+1]);
	}
	if (p) {
		mode = parser_priv(p);
		for (i = count-1; i >= 0; i--, mode = mode->pNext) {
			int j = i + num_graphics_modes;
			memcpy(&(graphics_modes[j]), mode, sizeof(graphics_mode));
			/* HACK -- auto-assign grafID, unlike V. */
			graphics_modes[j].grafID = j + 1;
			graphics_modes[j].pNext = &(graphics_modes[j+1]);
		}
	}
	num_graphics_modes += count;
	count = num_graphics_modes;
	
	/* hardcode the no graphics option */
	graphics_modes[count].pNext = NULL;
	graphics_modes[count].grafID = GRAPHICS_NONE;
	graphics_modes[count].alphablend = 0;
	graphics_modes[count].overdrawRow = 0;
	graphics_modes[count].overdrawMax = 0;
	strncpy(graphics_modes[count].pref, "none", 8);
	strncpy(graphics_modes[count].file, "", 32);
	strncpy(graphics_modes[count].menuname, "None", 32);
	
	graphics_mode_high_id = count;

	/* set the default graphics mode to be no graphics */
	current_graphics_mode = &(graphics_modes[count]);
	
	if (p) {
		mode = parser_priv(p);
		while (mode) {
			n = mode->pNext;
			mem_free(mode);
			mode = n;
		}
	
		parser_setpriv(p, NULL);
		parser_destroy(p);
	}
	return PARSE_ERROR_NONE;
}

static void print_error(const char *name, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	printf("Parse error in %s line %d column %d: %s: %s\n", name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	/* FIXME? message_flush(); */
}

bool add_graphics_modes(const char *filename, bool buildpath) {
	char buf[1024];

	ang_file *f;
	struct parser *p;
	errr e = 0;

	int line_no = 0;

	/* Build the filename */
	if (buildpath)
		path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, filename);
	else
		my_strcpy(buf, filename, sizeof(buf));

	f = file_open(buf, MODE_READ, -1);
	if (!f) {
		e = PARSE_ERROR_GENERIC;
	} else {
		char line[1024];

		p = init_parse_grafmode();
		while (file_getl(f, line, sizeof line)) {
			line_no++;
			e = parser_parse(p, line);
			if (e != PARSE_ERROR_NONE) {
				print_error(buf, p);
				break;
			}
		}

		finish_parse_grafmode(p);
		file_close(f);
	}

	/* Result */
	return e == PARSE_ERROR_NONE;
}

bool init_graphics_modes(const char *filename) {
	char buf[1024];
	ang_dir *dir;
	errr e = 0;

	/* Read the 'main' file first */
	if (filename)
		add_graphics_modes(filename, TRUE);

	/** Scan for definitions **/

	/* Open the directory */
	dir = my_dopen(ANGBAND_DIR_XTRA_GRAF);
	if (!dir) return FALSE;

	/* Read every filename */
	while (my_dread(dir, buf, sizeof buf)) {
		/* Skip 'main' file (it has already been read) */
		if (filename && !my_stricmp(buf, filename))
			continue;
		/* Check for file extension */
		if (isuffix(buf, ".txt"))
			add_graphics_modes(buf, TRUE);
	}

	/* Done */
	my_dclose(dir);

	/* Result */
	return e == PARSE_ERROR_NONE;
}

#if 0
bool init_graphics_modes(const char *filename) {
	char buf[1024];

	ang_file *f;
	struct parser *p;
	errr e = 0;

	int line_no = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, filename);

	f = file_open(buf, MODE_READ, -1);
	if (!f) {
		finish_parse_grafmode(NULL);
	} else {
		char line[1024];

		p = init_parse_grafmode();
		while (file_getl(f, line, sizeof line)) {
			line_no++;

			e = parser_parse(p, line);
			if (e != PARSE_ERROR_NONE) {
				print_error(buf, p);
				break;
			}
		}

		finish_parse_grafmode(p);
		file_close(f);
	}

	/* Result */
	return e == PARSE_ERROR_NONE;
}
#endif

void close_graphics_modes(void) {
	if (graphics_modes) {
		mem_free(graphics_modes);
		graphics_modes = NULL;
	}
	num_graphics_modes = 0;
	graphics_mode_high_id = 0;
}

graphics_mode* get_graphics_mode(byte id) {
	graphics_mode *test = graphics_modes;
	while (test) {
		if (test->grafID == id) {
			return test;
		}
		test = test->pNext;
	}
	return NULL;
}

size_t get_num_graphics_modes(void) {
	return num_graphics_modes;
}
