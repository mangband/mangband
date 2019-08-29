/* File z-util.h */

#ifndef INCLUDED_Z_UTIL_H
#define INCLUDED_Z_UTIL_H

#include "h-basic.h"


/*
 * Extremely basic stuff, like global temp and constant variables.
 * Also, some very useful low level functions, such as "streq()".
 * All variables and functions in this file are "addressable".
 */


/**** Available variables ****/

/* Temporary Vars */
extern char char_tmp;
extern byte byte_tmp;
extern sint sint_tmp;
extern uint uint_tmp;
extern long long_tmp;
extern huge huge_tmp;
extern errr errr_tmp;

/* Temporary Pointers */
extern cptr cptr_tmp;
extern vptr vptr_tmp;


/* Constant pointers (NULL) */
extern cptr cptr_null;
extern vptr vptr_null;


/* A bizarre vptr that always points at itself */
extern vptr vptr_self;


/* A cptr to the name of the program */
extern cptr argv0;

/* mbcs hook and function */
extern size_t (*mbcs_hook)(wchar_t *dest, const char *src, int n);
extern size_t z_mbstowcs(wchar_t *dest, const char *src, int n);

/* Aux functions */

extern void (*plog_aux)(cptr);
extern void (*quit_aux)(cptr);
extern void (*core_aux)(cptr);


/**** Available Functions ****/

/* Function that does nothing */
extern void func_nothing(void);

/* Functions that return basic "errr" codes */
extern errr func_success(void);
extern errr func_problem(void);
extern errr func_failure(void);

/* Functions that return bools */
extern bool func_true(void);
extern bool func_false(void);

/* Function that work with hturn */
extern int ht_passed(hturn *new_ht, hturn *old_ht, huge passed);
extern char* ht_show(hturn *ht_ptr, int mode);

/* Case insensitive comparison between two strings */
extern int my_stricmp(const char *s1, const char *s2);
extern int my_strnicmp(cptr a, cptr b, int n);
extern const char *my_stristr(const char *haystack, const char *needle);

/* Copy a string */
extern size_t my_strcpy(char *buf, const char *src, size_t bufsize);

/* Concatenate two strings */
extern size_t my_strcat(char *buf, const char *src, size_t bufsize);

/* Test equality, prefix, suffix */
extern bool streq(cptr s, cptr t);
extern bool prefix(cptr s, cptr t);
extern bool suffix(cptr s, cptr t);
/* Test for case-insensitive suffix */
extern bool isuffix(cptr s, cptr t);

/* Hack -- conditional (or "bizarre") externs */

#ifndef HAVE_MEMSET
extern void *memset(void*, int, size_t);
#endif

#ifndef HAVE_STRICMP
extern int stricmp(cptr a, cptr b);
#endif

#ifndef HAVE_STRDUP
extern char *strdup(cptr s);
#endif

#ifndef HAVE_STRNLEN
extern size_t strnlen(char *s, size_t maxlen);
#endif

#ifndef HAVE_USLEEP
extern int usleep(huge microSeconds);
#endif


/* Print an error message */
extern void plog(cptr str);

/* Exit, with optional message */
extern void quit(cptr str);

/* Dump core, with optional message */
extern void core(cptr str);


/* (integer) square root and hypot */
extern u32b isqrt(u32b x);
extern u32b ihypot(u32b x, u32b y);
#define IHYPOT(X, Y) isqrt((X) * (X) + (Y) * (Y))


/* Sorting functions */
/* TODO: make ang_sort() take comp and swap hooks rather than use globals */
extern void ang_sort(void *player_context, vptr u, vptr v, int n);
extern void ang_sort_aux(void *player_context, vptr u, vptr v, int p, int q);

extern bool (*ang_sort_comp)(void *player_context, vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(void *player_context, vptr u, vptr v, int a, int b);

#endif
