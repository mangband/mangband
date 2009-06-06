/* File: init2.c */

/* Purpose: Initialization (part 2) -BEN- */

#define SERVER

#include "angband.h"


/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * The "init1.c" file is used only to parse the ascii template files,
 * to create the binary image files.  If you include the binary image
 * files instead of the ascii template files, then you can undefine
 * "ALLOW_TEMPLATES", saving about 20K by removing "init1.c".  Note
 * that the binary image files are extremely system dependant.
 */

#include "init.h"

/*
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variable.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Mega-Hack -- support fat raw files under NEXTSTEP, using special
 * "suffixed" directories for the "ANGBAND_DIR_DATA" directory, but
 * requiring the directories to be created by hand by the user.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".
 */
void init_file_paths(char *path)
{
	char *tail;


	/*** Free everything ***/

	/* Free the main path */
	string_free(ANGBAND_DIR);

	/* Free the sub-paths */
/*	string_free(ANGBAND_DIR_APEX);*/
/*	string_free(ANGBAND_DIR_BONE);*/
	string_free(ANGBAND_DIR_DATA);
	string_free(ANGBAND_DIR_EDIT);
/*	string_free(ANGBAND_DIR_FILE);*/
/*	string_free(ANGBAND_DIR_HELP);*/
/*	string_free(ANGBAND_DIR_INFO);*/
	string_free(ANGBAND_DIR_SAVE);
/*	string_free(ANGBAND_DIR_PREF);*/
	string_free(ANGBAND_DIR_USER);
/*	string_free(ANGBAND_DIR_XTRA);*/
/*	string_free(ANGBAND_DIR_SCRIPT);*/

	string_free(ANGBAND_DIR_GAME);
	string_free(ANGBAND_DIR_TEXT);


	/*** Prepare the "path" ***/

	/* Load in the mangband.cfg file.  This is a file that holds many
	 * options thave have historically been #defined in config.h.
	 */

	/* Hack -- save the main directory */
	ANGBAND_DIR = string_make(path);

	/* Prepare to append to the Base Path */
	tail = path + strlen(path);


#ifdef VM


	/*** Use "flat" paths with VM/ESA ***/

	/* Use "blank" path names */
	ANGBAND_DIR_DATA = string_make("");
	ANGBAND_DIR_GAME = string_make("");
	ANGBAND_DIR_SAVE = string_make("");
	ANGBAND_DIR_TEXT = string_make("");
	ANGBAND_DIR_USER = string_make("");


#else /* VM */


	/*** Build the sub-directory names ***/

	/* Build a path name */
	strcpy(tail, "data");
	ANGBAND_DIR_DATA = string_make(path);

	/* Build a path name */
	strcpy(tail, "edit");
	ANGBAND_DIR_EDIT = string_make(path);
#if 0
	/* Build a path name */
	strcpy(tail, "file");
	ANGBAND_DIR_FILE = string_make(path);

	/* Build a path name */
	strcpy(tail, "help");
	ANGBAND_DIR_HELP = string_make(path);

	/* Build a path name */
	strcpy(tail, "info");
	ANGBAND_DIR_INFO = string_make(path);

	/* Build a path name */
	strcpy(tail, "pref");
	ANGBAND_DIR_PREF = string_make(path);
#endif
	/* Build a path name */
	strcpy(tail, "save");
	ANGBAND_DIR_SAVE = string_make(path);
	
	/* Build a path name */
	strcpy(tail, "text");
	ANGBAND_DIR_TEXT = string_make(path);

	/* Build a path name */
	strcpy(tail, "user");
	ANGBAND_DIR_USER = string_make(path);
#if 0
	/* Build a path name */
	strcpy(tail, "apex");
	ANGBAND_DIR_APEX = string_make(path);

	/* Build a path name */
	strcpy(tail, "bone");
	ANGBAND_DIR_BONE = string_make(path);

	/* Build a path name */
	strcpy(tail, "xtra");
	ANGBAND_DIR_XTRA = string_make(path);
#endif
#endif /* VM */


#ifdef NeXT

	/* Allow "fat binary" usage with NeXT */
	if (TRUE)
	{
		cptr next = NULL;

# if defined(m68k)
		next = "m68k";
# endif

# if defined(i386)
		next = "i386";
# endif

# if defined(sparc)
		next = "sparc";
# endif

# if defined(hppa)
		next = "hppa";
# endif

		/* Use special directory */
		if (next)
		{
			/* Forget the old path name */
			string_free(ANGBAND_DIR_DATA);

			/* Build a new path name */
			sprintf(tail, "data-%s", next);
			ANGBAND_DIR_DATA = string_make(path);
		}
	}

#endif /* NeXT */

}



/*
 * Hack -- Explain a broken "lib" folder and quit (see below).
 *
 * XXX XXX XXX This function is "messy" because various things
 * may or may not be initialized, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 */
static void show_news_aux(cptr why)
{
	/* Why */
	plog(why);

	/* Explain */
	plog("The 'lib' directory is probably missing or broken.");

	/* More details */
	plog("Perhaps the archive was not extracted correctly.");

	/* Explain */
	plog("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
}


/*
 * Hack -- verify some files, and display the "news.txt" file
 *
 * This function is often called before "init_some_arrays()",
 * but after the "term.c" package has been initialized, so
 * be aware that many functions will not be "usable" yet.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure.
 *
 * Note that one of the most common "extraction" errors involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 */
void show_news(void)
{
#if 0
	int		fd = -1;

	int		mode = 0644;

	FILE        *fp;

	char	buf[1024];


	/*** Verify the "news" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_TEXT, "news.txt");

	/* Attempt to open the file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		char why[1024];

		/* Message */
		sprintf(why, "Cannot access the '%s' file!", buf);

		/* Crash and burn */
		show_news_aux(why);
	}

	/* Close it */
	(void)fd_close(fd);


	/*** Display the "news" file ***/

	/* Clear screen */
	/*Term_clear();*/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_TEXT, "news.txt");

	/* Open the News file */
	fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, 1024))
		{
			/* Display and advance */
			s_printf(buf);
			s_printf("\n");
		}

		/* Close */
		my_fclose(fp);
	}

	/* Flush it */
	/*Term_fresh();*/


	/*** Verify (or create) the "high score" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "scores.raw");

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		/* File type is "DATA" */
		FILE_TYPE(FILE_TYPE_DATA);

		/* Create a new high score file */
		fd = fd_make(buf, mode);

		/* Failure */
		if (fd < 0)
		{
			char why[1024];

			/* Message */
			sprintf(why, "Cannot create the '%s' file!", buf);

			/* Crash and burn */
			show_news_aux(why);
		}
	}

	/* Close it */
	(void)fd_close(fd);
#endif
}



/*
 * Hack -- take notes on line 23
 */
#if 0
static void note(cptr str)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, str);
	Term_fresh();
}
#endif




#ifdef ALLOW_TEMPLATES


/*
 * Hack -- help give useful error messages
 */
s16b error_idx;
s16b error_line;


/*
 * Hack -- help initialize the fake "name" and "text" arrays when
 * parsing an "ascii" template file.
 */
u32b fake_name_size;
u32b fake_text_size;


/*
 * Standard error message text
 */
static cptr err_str[8] =
{
	NULL,
	"parse error",
	"obsolete file",
	"missing record header",
	"non-sequential records",
	"invalid flag specification",
	"undefined directive",
	"out of memory"
};


#endif


/*
 * File headers
 */
header z_head;
/*header v_head;
header f_head;
header k_head;
header a_head;
header e_head;
header r_head;*/
header p_head;
header c_head;
header h_head;
header b_head;
header g_head;
header flavor_head;



/*** Initialize from binary image files ***/


/*
 * Initialize a "*_info" array, by parsing a binary "image" file
 */
static errr init_info_raw(int fd, header *head)
{
	header test;


	/* Read and verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != head->v_major) ||
	    (test.v_minor != head->v_minor) ||
	    (test.v_patch != head->v_patch) ||
	    (test.v_extra != head->v_extra) ||
	    (test.info_num != head->info_num) ||
	    (test.info_len != head->info_len) ||
	    (test.head_size != head->head_size) ||
	    (test.info_size != head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	COPY(head, &test, header);


	/* Allocate the "*_info" array */
	C_MAKE(head->info_ptr, head->info_size, char);

	/* Read the "*_info" array */
	fd_read(fd, head->info_ptr, head->info_size);

	if (head->name_size)
	{
		/* Allocate the "*_name" array */
		C_MAKE(head->name_ptr, head->name_size, char);

		/* Read the "*_name" array */
		fd_read(fd, head->name_ptr, head->name_size);
	}

	if (head->text_size)
	{
		/* Allocate the "*_text" array */
		C_MAKE(head->text_ptr, head->text_size, char);

		/* Read the "*_text" array */
		fd_read(fd, head->text_ptr, head->text_size);
	}

	/* Success */
	return (0);
}


/*
 * Initialize the header of an *_info.raw file.
 */
static void init_header(header *head, int num, int len)
{
	/* Save the "version" */
	head->v_major = VERSION_MAJOR;
	head->v_minor = VERSION_MINOR;
	head->v_patch = VERSION_PATCH;
	head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	head->info_num = num;
	head->info_len = len;

	/* Save the size of "*_head" and "*_info" */
	head->head_size = sizeof(header);
	head->info_size = head->info_num * head->info_len;
}


#ifdef ALLOW_TEMPLATES

/*
 * Display a parser error message.
 */
static void display_parse_error(cptr filename, errr err, cptr buf)
{
	cptr oops;

	/* Error string */
	oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

	/* Oops */
	plog(format("Error at line %d of '%s.txt'.", error_line, filename));
	plog(format("Record %d contains a '%s' error.", error_idx, oops));
	plog(format("Parsing '%s'.", buf));
/*	message_flush();*/

	/* Quit */
	quit_fmt("Error in '%s.txt' file.", filename);
}

#endif /* ALLOW_TEMPLATES */


/*
 * Initialize a "*_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_info(cptr filename, header *head)
{
	int fd;

	errr err = 1;

	FILE *fp;

	/* General buffer */
	char buf[1024];


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format("%s.raw", filename));

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, format("%s.txt", filename));

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_info_raw(fd, head);

		/* Close it */
		fd_close(fd);
	}

	/* Do we have to parse the *.txt file? */
	if (err)
	{
		/*** Make the fake arrays ***/

		/* Allocate the "*_info" array */
		C_MAKE(head->info_ptr, head->info_size, char);

		/* MegaHack -- make "fake" arrays */
		if (z_info)
		{
			C_MAKE(head->name_ptr, z_info->fake_name_size, char);
			C_MAKE(head->text_ptr, z_info->fake_text_size, char);
		}


		/*** Load the ascii template file ***/

		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, format("%s.txt", filename));

		/* Open the file */
		fp = my_fopen(buf, "r");

		/* Parse it */
		if (!fp) quit(format("Cannot open '%s' file.", buf));

		/* Parse the file */
		err = init_info_txt(fp, buf, head, head->parse_info_txt);

		/* Close it */
		my_fclose(fp);

		/* Errors */
		if (err) display_parse_error(filename, err, buf);


		/*** Dump the binary image file ***/

		/* File type is "DATA" */
		FILE_TYPE(FILE_TYPE_DATA);

		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format("%s.raw", filename));


		/* Attempt to open the file */
		fd = fd_open(buf, O_RDONLY);

		/* Failure */
		if (fd < 0)
		{
			int mode = 0644;

			/* Grab permissions */
			safe_setuid_grab();

			/* Create a new file */
			fd = fd_make(buf, mode);

			/* Drop permissions */
			safe_setuid_drop();

			/* Failure */
			if (fd < 0)
			{
				/* Complain */
				plog_fmt("Cannot create the '%s' file!", buf);

				/* Continue */
				return (0);
			}
		}

		/* Close it */
		fd_close(fd);

		/* Grab permissions */
		safe_setuid_grab();

		/* Attempt to create the raw file */
		fd = fd_open(buf, O_WRONLY);

		/* Drop permissions */
		safe_setuid_drop();

		/* Failure */
		if (fd < 0)
		{
			/* Complain */
			plog_fmt("Cannot write the '%s' file!", buf);

			/* Continue */
			return (0);
		}

		/* Dump to the file */
		if (fd >= 0)
		{
			/* Dump it */
			fd_write(fd, (cptr)head, head->head_size);

			/* Dump the "*_info" array */
			if (head->info_size > 0)
				fd_write(fd, head->info_ptr, head->info_size);

			/* Dump the "*_name" array */
			if (head->name_size > 0)
				fd_write(fd, head->name_ptr, head->name_size);

			/* Dump the "*_text" array */
			if (head->text_size > 0)
				fd_write(fd, head->text_ptr, head->text_size);

			/* Close */
			fd_close(fd);
		}


		/*** Kill the fake arrays ***/

		/* Free the "*_info" array */
		KILL(head->info_ptr, vptr);

		/* MegaHack -- Free the "fake" arrays */
		if (z_info)
		{
			KILL(head->name_ptr, cptr);
			KILL(head->text_ptr, cptr);
		}

#endif /* ALLOW_TEMPLATES */


		/*** Load the binary image file ***/

		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, format("%s.raw", filename));

		/* Attempt to open the "raw" file */
		fd = fd_open(buf, O_RDONLY);

		/* Process existing "raw" file */
		if (fd < 0) quit(format("Cannot load '%s.raw' file.", filename));

		/* Attempt to parse the "raw" file */
		err = init_info_raw(fd, head);

		/* Close it */
		fd_close(fd);

		/* Error */
		if (err) quit(format("Cannot parse '%s.raw' file.", filename));

#ifdef ALLOW_TEMPLATES
	}
#endif /* ALLOW_TEMPLATES */

	/* Success */
	return (0);
}


/*
 * Free the allocated memory for the info-, name-, and text- arrays.
 */
static errr free_info(header *head)
{
	if (head->info_size)
		FREE(head->info_ptr, vptr);

	if (head->name_size)
		FREE(head->name_ptr, cptr);

	if (head->text_size)
		FREE(head->text_ptr, cptr);

	/* Success */
	return (0);
}


/*
 * Initialize the "z_info" array
 */
static errr init_z_info(void)
{
	errr err;

	/* Init the header */
	init_header(&z_head, 1, sizeof(maxima));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	z_head.parse_info_txt = parse_z_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("limits", &z_head);

	/* Set the global variables */
	z_info = z_head.info_ptr;

	return (err);
}


/*
 * Initialize the "f_info" array (Terrain handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_f_info(void)
{
	int i;
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(f_head, header);

	/* Save the "version" */
	f_head->v_major = VERSION_MAJOR;
	f_head->v_minor = VERSION_MINOR;
	f_head->v_patch = VERSION_PATCH;
	f_head->v_extra = 0;

	/* Save the "record" information */
	f_head->info_num = MAX_F_IDX;
	f_head->info_len = sizeof(feature_type);

	/* Save the size of "f_head" and "f_info" */
	f_head->head_size = sizeof(header);
	f_head->info_size = f_head->info_num * f_head->info_len;


	/*** Make the fake arrays ***/

	/* Fake the size of "f_name" and "f_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 60 * 1024L;

	/* Allocate the "f_info" array */
	C_MAKE(f_info, f_head->info_num, feature_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(f_name, fake_name_size, char);
	C_MAKE(f_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "terrain.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'terrain.txt' file.");

	/* Parse the file */
	err = init_f_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'terrain.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'terrain.txt' file.");
	}

	/* Keep a copy of server side char/attrs in the same format as in  the player 
	 * structure for use later in map_info() rendering server side scenes */
	for(i=0; i<MAX_F_IDX; i++)
	{
		f_char_s[i] = f_info[i].f_char;
		f_attr_s[i] = f_info[i].f_attr;
	}

	/* Success */
	return (0);
}




/*
 * Initialize the "k_info" array (Object handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_k_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];

	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(k_head, header);

	/* Save the "version" */
	k_head->v_major = VERSION_MAJOR;
	k_head->v_minor = VERSION_MINOR;
	k_head->v_patch = VERSION_PATCH;
	k_head->v_extra = 0;

	/* Save the "record" information */
	k_head->info_num = MAX_K_IDX;
	k_head->info_len = sizeof(object_kind);

	/* Save the size of "k_head" and "k_info" */
	k_head->head_size = sizeof(header);
	k_head->info_size = k_head->info_num * k_head->info_len;

	/*** Make the farrays ***/

	/* Fake the size of "k_name" and "k_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 60 * 1024L;

	/* Allocate the "k_info" array */
	C_MAKE(k_info, k_head->info_num, object_kind);

	/* Hack -- make "fake" arrays */
	C_MAKE(k_name, fake_name_size, char);
	C_MAKE(k_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "object.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'object.txt' file.");

	/* Parse the file */
	err = init_k_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'object.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'object.txt' file.");
	}

	/* Success */
	return (0);
}




/*
 * Initialize the "a_info" array (Artifact handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_a_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(a_head, header);

	/* Save the "version" */
	a_head->v_major = VERSION_MAJOR;
	a_head->v_minor = VERSION_MINOR;
	a_head->v_patch = VERSION_PATCH;
	a_head->v_extra = 0;

	/* Save the "record" information */
	a_head->info_num = MAX_A_IDX;
	a_head->info_len = sizeof(artifact_type);

	/* Save the size of "a_head" and "a_info" */
	a_head->head_size = sizeof(header);
	a_head->info_size = a_head->info_num * a_head->info_len;

	/*** Make the fake arrays ***/

	/* Fake the size of "a_name" and "a_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 60 * 1024L;

	/* Allocate the "a_info" array */
	C_MAKE(a_info, a_head->info_num, artifact_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(a_name, fake_name_size, char);
	C_MAKE(a_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "artifact.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'artifact.txt' file.");

	/* Parse the file */
	err = init_a_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'artifact.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'artifact.txt' file.");
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "c_info" array
 */
static errr init_c_info(void)
{
	errr err;

	/* Init the header */
	init_header(&c_head, z_info->c_max, sizeof(player_class));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	c_head.parse_info_txt = parse_c_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_class", &c_head);

	/* Set the global variables */
	c_info = c_head.info_ptr;
	c_name = c_head.name_ptr;
	c_text = c_head.text_ptr;

	return (err);
}



/*
 * Initialize the "h_info" array
 */
static errr init_h_info(void)
{
	errr err;

	/* Init the header */
	init_header(&h_head, z_info->h_max, sizeof(hist_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	h_head.parse_info_txt = parse_h_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_hist", &h_head);

	/* Set the global variables */
	h_info = h_head.info_ptr;
	h_text = h_head.text_ptr;

	return (err);
}



/*
 * Initialize the "e_info" array (Ego item handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_e_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(e_head, header);

	/* Save the "version" */
	e_head->v_major = VERSION_MAJOR;
	e_head->v_minor = VERSION_MINOR;
	e_head->v_patch = VERSION_PATCH;
	e_head->v_extra = 0;

	/* Save the "record" information */
	e_head->info_num = MAX_E_IDX;
	e_head->info_len = sizeof(ego_item_type);

	/* Save the size of "e_head" and "e_info" */
	e_head->head_size = sizeof(header);
	e_head->info_size = e_head->info_num * e_head->info_len;


	/*** Make the fake arrays ***/

	/* Fake the size of "e_name" and "e_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 60 * 1024L;

	/* Allocate the "e_info" array */
	C_MAKE(e_info, e_head->info_num, ego_item_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(e_name, fake_name_size, char);
	C_MAKE(e_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ego_item.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'ego_item.txt' file.");

	/* Parse the file */
	err = init_e_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'ego_item.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'ego_item.txt' file.");
	}

	/* Success */
	return (0);
}

/*
 * Initialize the "r_info" array (Monster handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_r_info(void)
{
	errr err;
	int i;
	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(r_head, header);

	/* Save the "version" */
	r_head->v_major = VERSION_MAJOR;
	r_head->v_minor = VERSION_MINOR;
	r_head->v_patch = VERSION_PATCH;
	r_head->v_extra = 0;

	/* Save the "record" information */
	r_head->info_num = MAX_R_IDX;
	r_head->info_len = sizeof(monster_race);

	/* Save the size of "r_head" and "r_info" */
	r_head->head_size = sizeof(header);
	r_head->info_size = r_head->info_num * r_head->info_len;


	/*** Make the fake arrays ***/

	/* Assume the size of "r_name" and "r_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 60 * 1024L;

	/* Allocate the "r_info" array */
	C_MAKE(r_info, r_head->info_num, monster_race);

	/* Hack -- make "fake" arrays */
	C_MAKE(r_name, fake_name_size, char);
	C_MAKE(r_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "monster.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'monster.txt' file.");

	/* Parse the file */
	err = init_r_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'monster.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'monster.txt' file.");
	}

	/* Keep a copy of server side char/attrs in the same format as in  the player 
	 * structure for use later in map_info() rendering server side scenes */
	for(i=0; i<MAX_R_IDX; i++)
	{
		r_char_s[i] = r_info[i].d_char;
		r_attr_s[i] = r_info[i].d_attr;
	}

	/* Success */
	return (0);
}




/*
 * Initialize the "v_info" array (Vault handling)
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_v_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(v_head, header);

	/* Save the "version" */
	v_head->v_major = VERSION_MAJOR;
	v_head->v_minor = VERSION_MINOR;
	v_head->v_patch = VERSION_PATCH;
	v_head->v_extra = 0;

	/* Save the "record" information */
	v_head->info_num = MAX_V_IDX;
	v_head->info_len = sizeof(vault_type);

	/* Save the size of "v_head" and "v_info" */
	v_head->head_size = sizeof(header);
	v_head->info_size = v_head->info_num * v_head->info_len;

	/*** Make the fake arrays ***/

	/* Fake the size of "v_name" and "v_text" */
	fake_name_size = 20 * 1024L;
	fake_text_size = 120 * 1024L;

	/* Allocate the "k_info" array */
	C_MAKE(v_info, v_head->info_num, vault_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(v_name, fake_name_size, char);
	C_MAKE(v_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "vault.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'vault.txt' file.");

	/* Parse the file */
	err = init_v_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		plog(format("Error %d at line %d of 'vault.txt'.", err, error_line));
		plog(format("Record %d contains a '%s' error.", error_idx, oops));
		plog(format("Parsing '%s'.", buf));
		/*msg_print(NULL);*/

		/* Quit */
		quit("Error in 'vault.txt' file.");
	}

	/* Success */
	return (0);
}


/*
 * Initialize the "flavor_info" array
 */
static errr init_flavor_info(void)
{
	errr err;

	/* Init the header */
	init_header(&flavor_head, z_info->flavor_max, sizeof(flavor_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	flavor_head.parse_info_txt = parse_flavor_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("flavor", &flavor_head);

	/* Set the global variables */
	flavor_info = flavor_head.info_ptr;
	flavor_name = flavor_head.name_ptr;
	flavor_text = flavor_head.text_ptr;

	return (err);
}


/*** Initialize others ***/



/*
 * Hack -- Objects sold in the stores -- by tval/sval pair.
 */
static byte store_table[MAX_STORES-3][STORE_CHOICES][2] =
{
	{
		/* General Store */

		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_BISCUIT },
		{ TV_FOOD, SV_FOOD_JERKY },
		{ TV_FOOD, SV_FOOD_JERKY },

		{ TV_FOOD, SV_FOOD_PINT_OF_WINE },
		{ TV_FOOD, SV_FOOD_PINT_OF_ALE },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_LANTERN },
		{ TV_LITE, SV_LITE_LANTERN },

		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },

		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
		{ TV_DIGGING, SV_SHOVEL },
		{ TV_DIGGING, SV_PICK },
		{ TV_CLOAK, SV_CLOAK },
		{ TV_CLOAK, SV_CLOAK },
		{ TV_CLOAK, SV_CLOAK }
	},

	{
		/* Armoury */

		{ TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
		{ TV_HELM, SV_HARD_LEATHER_CAP },
		{ TV_HELM, SV_HARD_LEATHER_CAP },
		{ TV_HELM, SV_METAL_CAP },
		{ TV_HELM, SV_IRON_HELM },

		{ TV_SOFT_ARMOR, SV_ROBE },
		{ TV_SOFT_ARMOR, SV_ROBE },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },
		{ TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },

		{ TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
		{ TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
		{ TV_HARD_ARMOR, SV_METAL_SCALE_MAIL },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_AUGMENTED_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_BAR_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL },

		{ TV_HARD_ARMOR, SV_METAL_BRIGANDINE_ARMOUR },
		{ TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
		{ TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
		{ TV_GLOVES, SV_SET_OF_GAUNTLETS },
		{ TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
		{ TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
		{ TV_SHIELD, SV_LARGE_LEATHER_SHIELD },
		{ TV_SHIELD, SV_SMALL_METAL_SHIELD }
	},

	{
		/* Weaponsmith */

		{ TV_SWORD, SV_DAGGER },
		{ TV_SWORD, SV_MAIN_GAUCHE },
		{ TV_SWORD, SV_RAPIER },
		{ TV_SWORD, SV_SMALL_SWORD },
		{ TV_SWORD, SV_SHORT_SWORD },
		{ TV_SWORD, SV_SABRE },
		{ TV_SWORD, SV_CUTLASS },
		{ TV_SWORD, SV_TULWAR },

		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_SWORD, SV_LONG_SWORD },
		{ TV_SWORD, SV_SCIMITAR },
		{ TV_SWORD, SV_KATANA },
		{ TV_SWORD, SV_BASTARD_SWORD },
		{ TV_POLEARM, SV_SPEAR },
		{ TV_POLEARM, SV_AWL_PIKE },
		{ TV_POLEARM, SV_TRIDENT },

		{ TV_POLEARM, SV_PIKE },
		{ TV_POLEARM, SV_BEAKED_AXE },
		{ TV_POLEARM, SV_BROAD_AXE },
		{ TV_POLEARM, SV_LANCE },
		{ TV_POLEARM, SV_BATTLE_AXE },
		{ TV_HAFTED, SV_WHIP },
		{ TV_BOW, SV_SLING },
		{ TV_BOW, SV_SHORT_BOW },

		{ TV_BOW, SV_LONG_BOW },
		{ TV_BOW, SV_LIGHT_XBOW },
		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
	},

	{
		/* Temple */

		{ TV_HAFTED, SV_WHIP },
		{ TV_HAFTED, SV_QUARTERSTAFF },
		{ TV_HAFTED, SV_MACE },
		{ TV_HAFTED, SV_MACE },
		{ TV_HAFTED, SV_BALL_AND_CHAIN },
		{ TV_HAFTED, SV_WAR_HAMMER },
		{ TV_HAFTED, SV_LUCERN_HAMMER },
		{ TV_HAFTED, SV_MORNING_STAR },

		{ TV_HAFTED, SV_FLAIL },
		{ TV_HAFTED, SV_LEAD_FILLED_MACE },
		{ TV_SCROLL, SV_SCROLL_REMOVE_CURSE },
		{ TV_SCROLL, SV_SCROLL_BLESSING },
		{ TV_SCROLL, SV_SCROLL_HOLY_CHANT },
		{ TV_SCROLL, SV_SCROLL_LIFE	},
		{ TV_POTION, SV_POTION_BOLDNESS },
		{ TV_POTION, SV_POTION_HEROISM },

		{ TV_SCROLL, SV_SCROLL_LIFE	},
		{ TV_POTION, SV_POTION_HEALING },
		{ TV_POTION, SV_POTION_HEALING },
		{ TV_POTION, SV_POTION_HEALING },
		{ TV_POTION, SV_POTION_CURE_CRITICAL },
		{ TV_POTION, SV_POTION_CURE_CRITICAL },
		{ TV_POTION, SV_POTION_RESTORE_EXP },
		{ TV_POTION, SV_POTION_RESTORE_EXP },

		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_PRAYER_BOOK, 2 },
		{ TV_PRAYER_BOOK, 2 },
		{ TV_PRAYER_BOOK, 3 }
	},

	{
		/* Alchemy shop */

		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_LIGHT },

		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION },
		{ TV_SCROLL, SV_SCROLL_MAPPING },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },

		{ TV_SCROLL, SV_SCROLL_DETECT_DOOR },
		{ TV_SCROLL, SV_SCROLL_DETECT_INVIS },
		{ TV_SCROLL, SV_SCROLL_RECHARGING },
		{ TV_SCROLL, SV_SCROLL_SATISFY_HUNGER },

		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },

		{ TV_POTION, SV_POTION_RESIST_HEAT },
		{ TV_POTION, SV_POTION_RESIST_COLD },
		{ TV_POTION, SV_POTION_RES_STR },
		{ TV_POTION, SV_POTION_RES_INT },
		{ TV_POTION, SV_POTION_RES_WIS },
		{ TV_POTION, SV_POTION_RES_DEX },
		{ TV_POTION, SV_POTION_RES_CON },
		{ TV_POTION, SV_POTION_RES_CHR }
	},

	{
		/* Magic-User store */

		{ TV_RING, SV_RING_SEARCHING },
		{ TV_RING, SV_RING_FEATHER_FALL },
		{ TV_RING, SV_RING_PROTECTION },
		{ TV_AMULET, SV_AMULET_CHARISMA },
		{ TV_AMULET, SV_AMULET_SLOW_DIGEST },
		{ TV_AMULET, SV_AMULET_RESIST_ACID },
		{ TV_WAND, SV_WAND_SLOW_MONSTER },
		{ TV_WAND, SV_WAND_CONFUSE_MONSTER },

		{ TV_WAND, SV_WAND_SLEEP_MONSTER },
		{ TV_WAND, SV_WAND_MAGIC_MISSILE },
		{ TV_WAND, SV_WAND_STINKING_CLOUD },
		{ TV_STAFF, SV_STAFF_LITE },
		{ TV_STAFF, SV_STAFF_MAPPING },
		{ TV_STAFF, SV_STAFF_DETECT_TRAP },
		{ TV_STAFF, SV_STAFF_DETECT_DOOR },
		{ TV_STAFF, SV_STAFF_DETECT_ITEM },

		{ TV_STAFF, SV_STAFF_DETECT_INVIS },
		{ TV_STAFF, SV_STAFF_DETECT_EVIL },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_IDENTIFY },
		{ TV_STAFF, SV_STAFF_IDENTIFY },
        { TV_MAGIC_BOOK, 0 },
        { TV_MAGIC_BOOK, 0 },

        { TV_MAGIC_BOOK, 1 },
        { TV_MAGIC_BOOK, 2 },
        { TV_MAGIC_BOOK, 3 }
	
    }
};

static byte ironman_store_table[MAX_STORES-3][STORE_CHOICES][2] =
{
    {
        /* General Store */

        { TV_FOOD, SV_FOOD_RATION },
        { TV_FOOD, SV_FOOD_RATION },
        { TV_FOOD, SV_FOOD_RATION },
        { TV_FOOD, SV_FOOD_RATION },
        { TV_FOOD, SV_FOOD_RATION },
        { TV_FOOD, SV_FOOD_BISCUIT },
        { TV_FOOD, SV_FOOD_JERKY },
        { TV_FOOD, SV_FOOD_JERKY },

        { TV_FOOD, SV_FOOD_PINT_OF_WINE },
        { TV_FOOD, SV_FOOD_PINT_OF_ALE },
        { TV_LITE, SV_LITE_TORCH },
        { TV_LITE, SV_LITE_TORCH },
        { TV_LITE, SV_LITE_TORCH },
        { TV_LITE, SV_LITE_TORCH },
        { TV_LITE, SV_LITE_LANTERN },
        { TV_LITE, SV_LITE_LANTERN },

        { TV_FLASK, 0 },
        { TV_FLASK, 0 },
        { TV_FLASK, 0 },
        { TV_FLASK, 0 },
        { TV_FLASK, 0 },
        { TV_FLASK, 0 },
        { TV_ARROW, SV_AMMO_NORMAL },
        { TV_ARROW, SV_AMMO_NORMAL },

        { TV_SHOT, SV_AMMO_NORMAL },
        { TV_BOLT, SV_AMMO_NORMAL },
        { TV_BOLT, SV_AMMO_NORMAL },
        { TV_DIGGING, SV_SHOVEL },
        { TV_DIGGING, SV_PICK },
        { TV_CLOAK, SV_CLOAK },
        { TV_CLOAK, SV_CLOAK },
        { TV_CLOAK, SV_CLOAK }
    },

    {
        /* Armoury */

        { TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
        { TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
        { TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
        { TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
        { TV_HELM, SV_HARD_LEATHER_CAP },
        { TV_HELM, SV_HARD_LEATHER_CAP },
        { TV_HELM, SV_METAL_CAP },
        { TV_HELM, SV_IRON_HELM },

        { TV_SOFT_ARMOR, SV_ROBE },
        { TV_SOFT_ARMOR, SV_ROBE },
        { TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
        { TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
        { TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
        { TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
        { TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },
        { TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },

        { TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
        { TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
        { TV_HARD_ARMOR, SV_METAL_SCALE_MAIL },
        { TV_HARD_ARMOR, SV_CHAIN_MAIL },
        { TV_HARD_ARMOR, SV_CHAIN_MAIL },
        { TV_HARD_ARMOR, SV_AUGMENTED_CHAIN_MAIL },
        { TV_HARD_ARMOR, SV_BAR_CHAIN_MAIL },
        { TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL },

        { TV_HARD_ARMOR, SV_METAL_BRIGANDINE_ARMOUR },
        { TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
        { TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
        { TV_GLOVES, SV_SET_OF_GAUNTLETS },
        { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
        { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
        { TV_SHIELD, SV_LARGE_LEATHER_SHIELD },
        { TV_SHIELD, SV_SMALL_METAL_SHIELD }
    },

    {
        /* Weaponsmith */

        { TV_SWORD, SV_DAGGER },
        { TV_SWORD, SV_MAIN_GAUCHE },
        { TV_SWORD, SV_RAPIER },
        { TV_SWORD, SV_SMALL_SWORD },
        { TV_SWORD, SV_SHORT_SWORD },
        { TV_SWORD, SV_SABRE },
        { TV_SWORD, SV_CUTLASS },
        { TV_SWORD, SV_TULWAR },

        { TV_SWORD, SV_BROAD_SWORD },
        { TV_SWORD, SV_LONG_SWORD },
        { TV_SWORD, SV_SCIMITAR },
        { TV_SWORD, SV_KATANA },
        { TV_SWORD, SV_BASTARD_SWORD },
        { TV_POLEARM, SV_SPEAR },
        { TV_POLEARM, SV_AWL_PIKE },
        { TV_POLEARM, SV_TRIDENT },

        { TV_POLEARM, SV_PIKE },
        { TV_POLEARM, SV_BEAKED_AXE },
        { TV_POLEARM, SV_BROAD_AXE },
        { TV_POLEARM, SV_LANCE },
        { TV_POLEARM, SV_BATTLE_AXE },
        { TV_HAFTED, SV_WHIP },
        { TV_BOW, SV_SLING },
        { TV_BOW, SV_SHORT_BOW },

        { TV_BOW, SV_LONG_BOW },
        { TV_BOW, SV_LIGHT_XBOW },
        { TV_SHOT, SV_AMMO_NORMAL },
        { TV_SHOT, SV_AMMO_NORMAL },
        { TV_ARROW, SV_AMMO_NORMAL },
        { TV_ARROW, SV_AMMO_NORMAL },
        { TV_BOLT, SV_AMMO_NORMAL },
        { TV_BOLT, SV_AMMO_NORMAL },
    },

    {
        /* Temple */

        { TV_HAFTED, SV_WHIP },
        { TV_HAFTED, SV_QUARTERSTAFF },
        { TV_HAFTED, SV_MACE },
        { TV_HAFTED, SV_MACE },
        { TV_HAFTED, SV_BALL_AND_CHAIN },
        { TV_HAFTED, SV_WAR_HAMMER },
        { TV_HAFTED, SV_LUCERN_HAMMER },
        { TV_HAFTED, SV_MORNING_STAR },

        { TV_HAFTED, SV_FLAIL },
        { TV_HAFTED, SV_LEAD_FILLED_MACE },
        { TV_SCROLL, SV_SCROLL_REMOVE_CURSE },
        { TV_SCROLL, SV_SCROLL_BLESSING },
        { TV_SCROLL, SV_SCROLL_HOLY_CHANT },
        { TV_SCROLL, SV_SCROLL_LIFE	},
        { TV_POTION, SV_POTION_BOLDNESS },
        { TV_POTION, SV_POTION_HEROISM },

        /* In Ironman, temple sells healing pots */
        { TV_POTION, SV_POTION_HEALING },
        { TV_POTION, SV_POTION_HEALING },
        { TV_POTION, SV_POTION_HEALING },
        { TV_POTION, SV_POTION_CURE_SERIOUS },
        { TV_POTION, SV_POTION_CURE_SERIOUS },
        { TV_POTION, SV_POTION_CURE_CRITICAL },
        { TV_POTION, SV_POTION_CURE_CRITICAL },
        { TV_POTION, SV_POTION_RESTORE_EXP },

        { TV_PRAYER_BOOK, 0 },
        { TV_PRAYER_BOOK, 0 },
        { TV_PRAYER_BOOK, 0 },
        { TV_PRAYER_BOOK, 1 },
        { TV_PRAYER_BOOK, 1 },
        { TV_PRAYER_BOOK, 2 },
        { TV_PRAYER_BOOK, 2 },
        { TV_PRAYER_BOOK, 3 }
    },

    {
        /* Alchemy shop */

        { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
        { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
        { TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },
        { TV_SCROLL, SV_SCROLL_IDENTIFY },
        { TV_SCROLL, SV_SCROLL_IDENTIFY },
        { TV_SCROLL, SV_SCROLL_IDENTIFY },
        { TV_SCROLL, SV_SCROLL_IDENTIFY },
        { TV_SCROLL, SV_SCROLL_LIGHT },

        { TV_SCROLL, SV_SCROLL_PHASE_DOOR },
        { TV_SCROLL, SV_SCROLL_PHASE_DOOR },
        { TV_SCROLL, SV_SCROLL_PHASE_DOOR },
        { TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION },
        { TV_SCROLL, SV_SCROLL_MAPPING },
        { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
        { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
        { TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },

        { TV_SCROLL, SV_SCROLL_DETECT_DOOR },
        { TV_SCROLL, SV_SCROLL_DETECT_INVIS },
        { TV_SCROLL, SV_SCROLL_RECHARGING },
        { TV_SCROLL, SV_SCROLL_SATISFY_HUNGER },

        /* Alchemy stocks tele scrolls instead of useless WoR in Ironman */
        { TV_SCROLL, SV_SCROLL_TELEPORT },
        { TV_SCROLL, SV_SCROLL_TELEPORT },
        { TV_SCROLL, SV_SCROLL_TELEPORT },
        { TV_SCROLL, SV_SCROLL_TELEPORT },

        { TV_POTION, SV_POTION_RESIST_HEAT },
        { TV_POTION, SV_POTION_RESIST_COLD },
        { TV_POTION, SV_POTION_RES_STR },
        { TV_POTION, SV_POTION_RES_INT },
        { TV_POTION, SV_POTION_RES_WIS },
        { TV_POTION, SV_POTION_RES_DEX },
        { TV_POTION, SV_POTION_RES_CON },
        { TV_POTION, SV_POTION_RES_CHR }
    },

    {
        /* Magic-User store */

        { TV_RING, SV_RING_SEARCHING }, 
	{ TV_RING, SV_RING_FEATHER_FALL },
		{ TV_RING, SV_RING_PROTECTION },
		{ TV_AMULET, SV_AMULET_CHARISMA },
		{ TV_AMULET, SV_AMULET_SLOW_DIGEST },
		{ TV_AMULET, SV_AMULET_RESIST_ACID },
        { TV_WAND, SV_WAND_SLOW_MONSTER },
        { TV_WAND, SV_WAND_CONFUSE_MONSTER },

		{ TV_WAND, SV_WAND_SLEEP_MONSTER },
        { TV_WAND, SV_WAND_MAGIC_MISSILE },
        { TV_WAND, SV_WAND_STINKING_CLOUD },
		{ TV_STAFF, SV_STAFF_LITE },
		{ TV_STAFF, SV_STAFF_MAPPING },
        { TV_STAFF, SV_STAFF_DETECT_TRAP },
		{ TV_STAFF, SV_STAFF_DETECT_DOOR },
		{ TV_STAFF, SV_STAFF_DETECT_ITEM },

        { TV_STAFF, SV_STAFF_DETECT_INVIS },
		{ TV_STAFF, SV_STAFF_DETECT_EVIL },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_IDENTIFY },
		{ TV_STAFF, SV_STAFF_IDENTIFY },
		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 0 },

		{ TV_MAGIC_BOOK, 1 },
		{ TV_MAGIC_BOOK, 2 },
		{ TV_MAGIC_BOOK, 3 }
	}
};




/*
 * Initialize some other arrays
 */
static errr init_other(void)
{
	int i, k, n;


	/*** Prepare the "dungeon" information ***/

	/* Allocate and Wipe the object list */
	C_MAKE(o_list, MAX_O_IDX, object_type);

	/* Allocate and Wipe the monster list */
	C_MAKE(m_list, MAX_M_IDX, monster_type);


	/* Allocate "permament" space for the town */
	C_MAKE(cave[0], MAX_HGT, cave_type *);

	/* Allocate and wipe each line of the town level */
	for (i = 0; i < MAX_HGT; i++)
	{
		/* Allocate one row of the cave */
		C_MAKE(cave[0][i], MAX_WID, cave_type);
	}


	/*** Init the wild_info array... for more information see wilderness.c ***/
	init_wild_info();

	/*** Prepare the various "bizarre" arrays ***/

	/* Macro variables */
	C_MAKE(macro__pat, MACRO_MAX, cptr);
	C_MAKE(macro__act, MACRO_MAX, cptr);
	C_MAKE(macro__cmd, MACRO_MAX, bool);

	/* Macro action buffer */
	C_MAKE(macro__buf, 1024, char);

	/* Quark variables */
	C_MAKE(quark__str, QUARK_MAX, cptr);

	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u16b);
	C_MAKE(message__buf, MESSAGE_BUF, char);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;


	/*** Prepare the Player inventory ***/

	/* Allocate it */
	/* This is done on player initialization --KLJ-- */
	/*C_MAKE(inventory, INVEN_TOTAL, object_type);*/


	/*** Prepare the Stores ***/

	/* Allocate the stores */
	C_MAKE(store, MAX_STORES, store_type);

	/* Fill in each store */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Access the store */
		store_type *st_ptr = &store[i];

		/* Assume full stock */
		st_ptr->stock_size = STORE_INVEN_MAX;

		/* Allocate the stock */
		C_MAKE(st_ptr->stock, st_ptr->stock_size, object_type);

		/* No table for the black market or home */
		if ((i == 6) || (i == 7) || (i == 8) ) continue;

		/* Assume full table */
		st_ptr->table_size = STORE_CHOICES;

		/* Allocate the stock */
		C_MAKE(st_ptr->table, st_ptr->table_size, s16b);

		/* Scan the choices */
		for (k = 0; k < STORE_CHOICES; k++)
		{
			int k_idx;

			/* Extract the tval/sval codes */
			int tv = store_table[i][k][0];
			int sv = store_table[i][k][1];
	    if (cfg_ironman)
	    {
		tv = ironman_store_table[i][k][0];
		sv = ironman_store_table[i][k][1];
	    }

			/* Look for it */
			for (k_idx = 1; k_idx < MAX_K_IDX; k_idx++)
			{
				object_kind *k_ptr = &k_info[k_idx];

				/* Found a match */
				if ((k_ptr->tval == tv) && (k_ptr->sval == sv)) break;
			}

			/* Catch errors */
			if (k_idx == MAX_K_IDX) continue;

			/* Add that item index to the table */
			st_ptr->table[st_ptr->table_num++] = k_idx;
		}
	}


	/*** Pre-allocate the basic "auto-inscriptions" ***/

	/* The "basic" feelings */
	(void)quark_add("cursed");
	(void)quark_add("broken");
	(void)quark_add("average");
	(void)quark_add("good");

	/* The "extra" feelings */
	(void)quark_add("excellent");
	(void)quark_add("worthless");
	(void)quark_add("special");
	(void)quark_add("terrible");

	/* Some extra strings */
	(void)quark_add("uncursed");
	(void)quark_add("on sale");


	/*** Prepare the options ***/

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_set;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (option_info[i].o_var)
		{
			/* Accept */
			option_mask[os] |= (1L << ob);
			
			/* Set */
			if (option_info[i].o_norm)
			{
				/* Set */
				option_flag[os] |= (1L << ob);
			}
			
			/* Clear */
			else
			{
				/* Clear */
				option_flag[os] &= ~(1L << ob);
			}
		}
	}

	/* Turn on color */
	use_color = TRUE;
	view_bright_lite = TRUE;
	view_granite_lite = TRUE;
	view_special_lite = TRUE;
	view_perma_grids = TRUE;

	/* Analyze the windows */
	for (n = 0; n < 8; n++)
	{
		/* Analyze the options */
		for (i = 0; i < 32; i++)
		{
			/* Accept */
			if (window_flag_desc[i])
			{
				/* Accept */
				window_mask[n] |= (1L << i);
			}
		}
	}

	/* Success */
	return (0);
}



/*
 * Initialize some other arrays
 */
static errr init_alloc(void)
{
	int i, j;

	object_kind *k_ptr;

	monster_race *r_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH];

	s16b aux[MAX_DEPTH];


	/*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	C_WIPE(&aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	C_WIPE(&num, MAX_DEPTH, s16b);

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (i = 1; i < MAX_K_IDX; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town objects!");


	/*** Initialize object allocation info ***/

	/* Allocate the alloc_kind_table */
	C_MAKE(alloc_kind_table, alloc_kind_size, alloc_entry);

	/* Access the table entry */
	table = alloc_kind_table;

	/* Scan the objects */
	for (i = 1; i < MAX_K_IDX; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				int p, x, y, z;

				/* Extract the base level */
				x = k_ptr->locale[j];

				/* Extract the base probability */
				p = (100 / k_ptr->chance[j]);

				/* Skip entries preceding our locale */
				y = (x > 0) ? num[x-1] : 0;

				/* Skip previous entries at this locale */
				z = y + aux[x];

				/* Load the entry */
				table[z].index = i;
				table[z].level = x;
				table[z].prob1 = p;
				table[z].prob2 = p;
				table[z].prob3 = p;

				/* Another entry complete for this locale */
				aux[x]++;
			}
		}
	}


	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	C_WIPE(&aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	C_WIPE(&num, MAX_DEPTH, s16b);

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < MAX_R_IDX - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity)
		{
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");


	/*** Initialize monster allocation info ***/

	/* Allocate the alloc_race_table */
	C_MAKE(alloc_race_table, alloc_race_size, alloc_entry);

	/* Access the table entry */
	table = alloc_race_table;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < MAX_R_IDX - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x-1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}


	/* Success */
	return (0);
}

bool str_to_boolean(char * str)
{
	/* false by default */
	return !(strcasecmp(str, "true"));
}

/*
 * Initialize the "p_info" array
 */
static errr init_p_info(void)
{
	errr err;

	/* Init the header */
	init_header(&p_head, z_info->p_max, sizeof(player_race));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	p_head.parse_info_txt = parse_p_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_race", &p_head);

	/* Set the global variables */
	p_info = p_head.info_ptr;
	p_name = p_head.name_ptr;
	p_text = p_head.text_ptr;

	return (err);
}

/* Try to set a server option.  This is handled very sloppily right now,
 * since server options are manually mapped to global variables.  Maybe
 * the handeling of this will be unified in the future with some sort of 
 * options structure.
 */
void set_server_option(char * option, char * value)
{
	/* Due to the lame way that C handles strings, we can't use a switch statement */
	if (!strcmp(option,"REPORT_TO_METASERVER"))
	{
		cfg_report_to_meta = str_to_boolean(value);
	}
	else if (!strcmp(option,"DATA_DIR"))
	{
		ANGBAND_DIR_DATA = strdup(value);
	}
	else if (!strcmp(option,"EDIT_DIR"))
	{
		ANGBAND_DIR_EDIT = strdup(value);
	}
	else if (!strcmp(option,"SAVE_DIR"))
	{
		ANGBAND_DIR_SAVE = strdup(value);
	}
	else if (!strcmp(option,"META_ADDRESS"))
	{
		cfg_meta_address = strdup(value);
	}
	else if (!strcmp(option,"BIND_NAME"))
	{
		cfg_bind_name = strdup(value);
	}
	else if (!strcmp(option,"REPORT_ADDRESS"))
	{
		cfg_report_address = strdup(value);
	}
	else if (!strcmp(option,"CONSOLE_PASSWORD"))
	{
		cfg_console_password = strdup(value);
	}
	else if (!strcmp(option,"DUNGEON_MASTER_NAME"))
	{
		cfg_dungeon_master = strdup(value);
	plog(format("Dugeon Master Set as [%s]",cfg_dungeon_master));
    }
	else if (!strcmp(option,"SECRET_DUNGEON_MASTER"))
	{
		cfg_secret_dungeon_master = str_to_boolean(value);
	}
	else if (!strcmp(option,"FPS"))
	{
		cfg_fps = atoi(value);
		/* Hack -- reinstall the timer handler to match the new FPS */
		install_timer_tick(dungeon, cfg_fps);
	}
	else if (!strcmp(option,"TCP_PORT"))
	{
		cfg_tcp_port = atoi(value);
		/* We probably ought to do some sanity check here */
		if (cfg_tcp_port & 0x01) /* Odd number */
			cfg_tcp_port++;
		if ((cfg_tcp_port > 65535) || (cfg_tcp_port < 1))
			cfg_tcp_port = 18346;
	}
	else if (!strcmp(option,"MAGE_HITPOINT_BONUS"))
	{
		cfg_mage_hp_bonus = str_to_boolean(value);
	}
	else if (!strcmp(option,"NO_STEAL"))
	{
		cfg_no_steal = str_to_boolean(value);
	}
	else if (!strcmp(option,"NEWBIES_CANNOT_DROP"))
	{
		cfg_newbies_cannot_drop = str_to_boolean(value);
	}
	else if (!strcmp(option,"GHOST_DIVING"))
	{
		cfg_ghost_diving = str_to_boolean(value);
	}
	else if (!strcmp(option,"DOOR_BUMP_OPEN"))
	{
		cfg_door_bump_open = str_to_boolean(value);
	}
	else if (!strcmp(option,"LEVEL_UNSTATIC_CHANCE"))
	{
		cfg_level_unstatic_chance = atoi(value);
	}
	else if (!strcmp(option,"RETIRE_TIMER"))
	{
		cfg_retire_timer = atoi(value);
	}
    else if (!strcmp(option,"ALLOW_RANDOM_ARTIFACTS"))
    {
        cfg_random_artifacts = str_to_boolean(value);
    }
    else if (!strcmp(option,"PRESERVE_ARTIFACTS"))
    {
        cfg_preserve_artifacts = atoi(value);
    }
    else if (!strcmp(option,"IRONMAN"))
    {
        cfg_ironman = str_to_boolean(value);
    }
    else if (!strcmp(option,"TOWN_WALL"))
    {
        cfg_town_wall = str_to_boolean(value);
    }
    else if (!strcmp(option,"BASE_UNIQUE_RESPAWN_TIME"))
    {
        cfg_unique_respawn_time = atoi(value);
    }
    else if (!strcmp(option,"MAX_UNIQUE_RESPAWN_TIME"))
    {
        cfg_unique_max_respawn_time = atoi(value);
    }
    else if (!strcmp(option,"MAX_TOWNIES"))
    {
        cfg_max_townies = atoi(value);
    }
    else if (!strcmp(option,"MAX_TREES"))
    {
        cfg_max_trees = atoi(value);
    }
    else if (!strcmp(option,"MAX_HOUSES"))
    {
        cfg_max_houses = atoi(value);
    }
    else if (!strcmp(option,"CHARACTER_DUMP_COLOR"))
    {
        cfg_chardump_color = str_to_boolean(value);
    }


	else plog(format("Error : unrecognized mangband.cfg option %s", option));
}


/* Parse the loaded mangband.cfg file, and if a valid expression was found
 * try to set it using set_server_option.
 */
void load_server_cfg_aux(FILE * cfg)
{
	char line[256];

	char * newword;
	char * option;
	char * value;
	bool first_token;

	/* Read in lines until we hit EOF */
	while (fgets(line, 256, cfg) != NULL)
	{
		// Chomp off the end of line character
		line[strlen(line)-1] = '\0';

		/* Parse the line that has been read in */
		// If the line begins with a # or is empty, ignore it
		if ((line[0] == '#') || (line[0] == '\0')) continue;

		// Reset option and value
		option = NULL;
		value = NULL;
		
		// Split the line up into words
		// We pass the string to be parsed to strsep on the first call,
		// and subsequently pass it null.
		first_token = 1;
		while ((newword = strtok(first_token ? line : NULL, " ")))
		{
			first_token = 0;

			/* Set the option or value */
			if (!option) option = newword;
			else if ((!value) && (newword[0] != '=')) 
			{
				value = newword;
				/* Hack -- ignore "" around values */
				if (value[0] == '"') value++;
				if (value[strlen(value)-1] == '"') value[strlen(value)-1] = '\0';
			}

			// If we have a completed option and value, then try to set it
			if (option && value)
			{
				set_server_option(option, value);
				break;
			}
		}
	}
}

/* Load in the mangband.cfg file.  This is a file that holds many
 * options thave have historically been #defined in config.h.
 */
cptr possible_cfg_dir[] = 
{
	"mangband.cfg",
	"/etc/mangband.cfg",
	"/usr/local/etc/mangband.cfg",
	"/usr/etc/mangband.cfg",
	NULL
};
void load_server_cfg(void)
{
	FILE * cfg = 0;
	int i = 0;

	/* Attempt to open the file */
	while (cfg <= 0 && possible_cfg_dir[i++])
	{
		cfg = fopen(possible_cfg_dir[i-1], "r");
	}

	/* Failure, try several dirs, then stop trying */
	if (cfg <= 0)
	{
		plog("Error : cannot open file mangband.cfg");
		return;
	}

	plog(format("Loading %s", possible_cfg_dir[i-1]));

	/* Actually parse the file */
	load_server_cfg_aux(cfg);

	/* Close it */
	fclose(cfg);

}


/*
 * Initialize various Angband variables and arrays.
 *
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 */
void init_some_arrays(void)
{
	/* Initialize size info */
	plog("[Initializing array sizes...]");
	if (init_z_info()) quit("Cannot initialize sizes");

	/* Initialize feature info */
	plog("[Initializing arrays... (features)]");
	if (init_f_info()) quit("Cannot initialize features");

	/* Initialize object info */
	plog("[Initializing arrays... (objects)]");
	if (init_k_info()) quit("Cannot initialize objects");

	/* Initialize artifact info */
	plog("[Initializing arrays... (artifacts)]");
	if (init_a_info()) quit("Cannot initialize artifacts");

	/* Initialize ego-item info */
	plog("[Initializing arrays... (ego-items)]");
	if (init_e_info()) quit("Cannot initialize ego-items");

	/* Initialize monster info */
	plog("[Initializing arrays... (monsters)]");
	if (init_r_info()) quit("Cannot initialize monsters");

	/* Initialize vault info */
	plog("[Initializing arrays... (vaults)]");
	if (init_v_info()) quit("Cannot initialize vaults");

	/* Initialize history info */
	plog("[Initializing arrays... (histories)]");
	if (init_h_info()) quit("Cannot initialize histories");

	/* Initialize race info */
	plog("[Initializing arrays... (races)]");
	if (init_p_info()) quit("Cannot initialize races");

	/* Initialize class info */
	plog("[Initializing arrays... (classes)]");
	if (init_c_info()) quit("Cannot initialize classes");

	/* Initialize flavor info */
	plog("[Initializing arrays... (flavors)]");
	if (init_flavor_info()) quit("Cannot initialize flavors");

	/* Initialize some other arrays */
	plog("[Initializing arrays... (other)]");
	if (init_other()) quit("Cannot initialize other stuff");

	/* Initialize some other arrays */
	plog("[Initializing arrays... (alloc)]");
	if (init_alloc()) quit("Cannot initialize alloc stuff");

	/* Hack -- all done */
	plog("[Initializing arrays... done]");
}
