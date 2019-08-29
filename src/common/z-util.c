/* File: z-util.c */

/* Purpose: Low level utilities -BEN- */

#include "z-util.h"


/*
 * Global variables for temporary use
 */
char char_tmp = 0;
byte byte_tmp = 0;
sint sint_tmp = 0;
uint uint_tmp = 0;
long long_tmp = 0;
huge huge_tmp = 0;
errr errr_tmp = 0;


/*
 * Global pointers for temporary use
 */
cptr cptr_tmp = NULL;
vptr vptr_tmp = NULL;


/*
 * Constant bool meaning true
 */
bool bool_true = 1;

/*
 * Constant bool meaning false
 */
bool bool_false = 0;


/*
 * Global NULL cptr
 */
cptr cptr_null = NULL;


/*
 * Global NULL vptr
 */
vptr vptr_null = NULL;



/*
 * Global SELF vptr
 */
vptr vptr_self = (vptr)(&vptr_self);



/*
 * Convenient storage of the program name
 */
cptr argv0 = NULL;



/*
 * A routine that does nothing
 */
void func_nothing(void)
{
	/* Do nothing */
}


/*
 * A routine that always returns "success"
 */
errr func_success(void)
{
	return (0);
}


/*
 * A routine that always returns a simple "problem code"
 */
errr func_problem(void)
{
	return (1);
}


/*
 * A routine that always returns a simple "failure code"
 */
errr func_failure(void)
{
	return (-1);
}



/*
 * A routine that always returns "true"
 */
bool func_true(void)
{
	return (1);
}


/*
 * A routine that always returns "false"
 */
bool func_false(void)
{
	return (0);
}


/*
 * Determine if "passed" turns passed since "old_ht" in "new_ht"
 */
int ht_passed(hturn *new_ht, hturn *old_ht, huge passed)
{
	hturn temp;
	temp.era = new_ht->era;
	temp.turn = new_ht->turn;
	ht_subst_ht(&temp, old_ht);
	ht_subst(&temp, passed);
	return ((signed)temp.era >= 0);
}

/*
 * Format "hturn" into string (NOTE: static buffer!)
 */
char* ht_show(hturn *ht_ptr, int mode)
{
	static char ret[1024];
	if (!mode) {
		if (!ht_ptr->era) sprintf (ret, "%ld", (long)ht_ptr->turn);
		else sprintf (ret, "%ld%ld", (long)ht_ptr->era, (long)ht_ptr->turn);
	}
	else if (mode == 1) sprintf (ret, "%ld %ld", (long)ht_ptr->era, (long)ht_ptr->turn);
	else sprintf (ret, "%04x %ld", (int)ht_ptr->era, (long)ht_ptr->turn);
	return &ret[0];
}

/*
 * Determine if string "t" is equal to string "t"
 */
bool streq(cptr a, cptr b)
{
	return (!strcmp(a, b));
}

#ifndef HAVE_MEMSET
/*
 * For those systems that don't have "memset()"
 *
 * Set the value of each of 'n' bytes starting at 's' to 'c', return 's'
 * If 'n' is negative, you will erase a whole lot of memory.
 */
void *memset(void *s, int c, size_t n)
{
	char *t;
	for (t = s; len--; ) *t++ = c;
	return (s);
}
#endif

#ifndef HAVE_STRICMP
/*
 * For those systems that don't have "stricmp()"
 *
 * Compare the two strings "a" and "b" ala "strcmp()" ignoring case.
 */
int stricmp(cptr a, cptr b)
{
	cptr s1, s2;
	char z1, z2;

	/* Scan the strings */
	for (s1 = a, s2 = b; TRUE; s1++, s2++)
	{
		z1 = FORCEUPPER(*s1);
		z2 = FORCEUPPER(*s2);
		if (z1 < z2) return (-1);
		if (z1 > z2) return (1);
		if (!z1) return (0);
	}
}
#endif

#ifndef HAVE_STRNLEN
/*
 * A copy of "strnlen".
 */
size_t strnlen(char *s, size_t n) {
	size_t i = n, j = 0;
	while (i--) 
	{
		if (*s++ == '\0') 
		{
			return j;
		}
		j++;
	}
	return n;
}
#endif

#ifndef HAVE_STRDUP
/*
 * A copy of "strdup"
 *
 * This code contributed by Hao Chen <hao@mit.edu>
 */
char *strdup(cptr s)
{
	char *dup;
	dup = (char *)malloc(sizeof(char) * (strlen(s) + 1));
	strcpy(dup, s);
	return dup;
}
#endif

#ifndef HAVE_USLEEP
/*
 * For those systems that don't have "usleep()" but need it.
 */
int usleep(huge microSeconds)
{
#ifdef HAVE_SELECT
	/* Use select() to wait. New version, inspired by Borg. */
	struct timeval Timer;

	/* Paranoia -- No excessive sleeping */
	if (microSeconds > 4000000L) core("Illegal usleep() call");

	/* Setup our timeval */
	Timer.tv_sec = (microSeconds / 1000000L);
	Timer.tv_usec = (microSeconds % 1000000L);

	/* Wait for it */
	if (select(0, NULL, NULL, NULL, &Timer) < 0)
	{
		/* Hack -- ignore interrupts */
		if (errno != EINTR) return -1;
	}
	return 0;
#else /* HAVE_SELECT */
	/* If we don't have select(), resort to other
	 * methods */
# ifdef WINDOWS
	/* On WIN32, we use Sleep() */
	/* meassured in milliseconds not microseconds */
	DWORD milliseconds = (DWORD)(microSeconds / 1000);
	Sleep(milliseconds);
	return 0;
# else /* WINDOWS */
	/* There is nothing we can do :( Patches welcome. */
	return -1;
# endif
#endif
}
#endif



/*
 * Case insensitive comparison between two strings
 */
int my_stricmp(const char *s1, const char *s2)
{
	char ch1 = 0;
	char ch2 = 0;

	/* Just loop */
	while (TRUE)
	{
		/* We've reached the end of both strings simultaneously */
		if ((*s1 == 0) && (*s2 == 0))
		{
			/* We're still here, so s1 and s2 are equal */
			return (0);
		}

		ch1 = toupper(*s1);
		ch2 = toupper(*s2);

		/* If the characters don't match */
		if (ch1 != ch2)
		{
			/* return the difference between them */
			return ((int)(ch1 - ch2));
		}

		/* Step on through both strings */
		s1++;
		s2++;
	}
}


/*
 * Case insensitive comparison between the first n characters of two strings
 */
int my_strnicmp(cptr a, cptr b, int n)
{
	cptr s1, s2;
	char z1, z2;

	/* Scan the strings */
	for (s1 = a, s2 = b; n > 0; s1++, s2++, n--)
	{
		z1 = toupper((unsigned char)*s1);
		z2 = toupper((unsigned char)*s2);
		if (z1 < z2) return (-1);
		if (z1 > z2) return (1);
		if (!z1) return (0);
	}

	return 0;
}

/*
 * Case insensitive strstr by Dave Sinkula
 */
const char *my_stristr(const char *haystack, const char *needle)
{
   if ( !*needle )
   {
      return haystack;
   }
   for ( ; *haystack; ++haystack )
   {
      if ( toupper(*haystack) == toupper(*needle) )
      {
         /*
          * Matched starting char -- loop through remaining chars.
          */
         const char *h, *n;
         for ( h = haystack, n = needle; *h && *n; ++h, ++n )
         {
            if ( toupper(*h) != toupper(*n) )
            {
               break;
            }
         }
         if ( !*n ) /* matched all of 'needle' to null termination */
         {
            return haystack; /* return the start of the match */
         }
      }
   }
   return NULL;
}

/*
 * The my_strcpy() function copies up to 'bufsize'-1 characters from 'src'
 * to 'buf' and NUL-terminates the result.  The 'buf' and 'src' strings may
 * not overlap.
 *
 * my_strcpy() returns strlen(src).  This makes checking for truncation
 * easy.  Example: if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
size_t my_strcpy(char *buf, const char *src, size_t bufsize)
{
	size_t len = strlen(src);
	size_t ret = len;

	/* Paranoia */
	if (bufsize == 0) return ret;

	/* Truncate */
	if (len >= bufsize) len = bufsize - 1;

	/* Copy the string and terminate it */
	(void)memcpy(buf, src, len);
	buf[len] = '\0';

	/* Return strlen(src) */
	return ret;
}


/*
 * The my_strcat() tries to append a string to an existing NUL-terminated string.
 * It never writes more characters into the buffer than indicated by 'bufsize' and
 * NUL-terminates the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 * if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
size_t my_strcat(char *buf, const char *src, size_t bufsize)
{
	size_t dlen = strlen(buf);

	/* Is there room left in the buffer? */
	if (dlen < bufsize - 1)
	{
		/* Append as much as possible  */
		return (dlen + my_strcpy(buf + dlen, src, bufsize - dlen));
	}
	else
	{
		/* Return without appending */
		return (dlen + strlen(src));
	}
}


/*
 * Determine if string "t" is a suffix of string "s"
 */
bool suffix(cptr s, cptr t)
{
	size_t tlen = strlen(t);
	size_t slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/*
 * Determine if string "t" is a prefix of string "s"
 */
bool prefix(cptr s, cptr t)
{
	/* Scan "t" */
	while (*t)
	{
		/* Compare content and length */
		if (*t++ != *s++) return (FALSE);
	}

	/* Matched, we have a prefix */
	return (TRUE);
}


/*
 * Determine if string "t" is a suffix of string "s",
 * case-insensitive.
 */
bool isuffix(cptr s, cptr t)
{
	size_t tlen = strlen(t);
	size_t slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!my_stricmp(s + slen - tlen, t));
}


/*
 * Redefinable "plog" action
 */
void (*plog_aux)(cptr) = NULL;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(cptr str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s: %s\n", argv0 ? argv0 : "???", str));
}



/*
 * Redefinable "quit" action
 */
void (*quit_aux)(cptr) = NULL;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(0)".
 * If 'str' begins with "+" or "-", do "exit(atoi(str))".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(cptr str)
{
        char buf[1024];

        /* Save exit string */
        if (str) my_strcpy(buf, str, 1024);
        else buf[0] = '\0';

        /* Attempt to use the aux function */
        if (quit_aux) (*quit_aux)(buf);

        /* Success */
        if (buf[0] == '\0') (void)(exit(0));

        /* Extract a "special error code" */
        if ((buf[0] == '-') || (buf[0] == '+')) (void)(exit(atoi(buf)));

        /* Send the string to plog() */
        plog(buf);

        /* Failure */
        (void)(exit(-1));
}


/*
 * Redefinable "core" action
 */
void (*core_aux)(cptr) = NULL;

/*
 * Dump a core file, after printing a warning message
 * As with "quit()", try to use the "core_aux()" hook first.
 */
void core(cptr str)
{
	char *crash = NULL;

	/* Use the aux function */
	if (core_aux) (*core_aux)(str);

	/* Dump the warning string */
	if (str) plog(str);

	/* Attempt to Crash */
	(*crash) = (*crash);

	/* Be sure we exited */
	quit("core() failed");
}


/* convert a multibyte string to a wide-character string */
size_t (*mbcs_hook)(wchar_t *dest, const char *src, int n) = NULL;
size_t z_mbstowcs(wchar_t *dest, const char *src, int n)
{
	if (mbcs_hook)
		return (*mbcs_hook)(dest, src, n);
	else
		return mbstowcs(dest, src, n);
}

/* integer square root */
u32b isqrt(u32b x)
{
	u32b op  = x;
	u32b res = 0;
	u32b one = 1uL << 30; /* Set second-to-top bit */
	while (one > op)
	{
		one >>= 2;
	}
	while (one != 0)
	{
		if (op >= res + one)
		{
			op = op - (res + one);
			res = res +  2 * one;
		}
		res >>= 1;
		one >>= 2;
	}
	return res;
}
/* And a "hypot" function. */
u32b ihypot(u32b x, u32b y)
{
	return isqrt(x * x + y * y);
}


/* Compare and swap hooks */
bool (*ang_sort_comp)(void* player_context, vptr u, vptr v, int a, int b);
void (*ang_sort_swap)(void* player_context, vptr u, vptr v, int a, int b);

/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(void* player_context, vptr u, vptr v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp)(player_context, u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp)(player_context, u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap)(player_context, u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(player_context, u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(player_context, u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(void* player_context, vptr u, vptr v, int n)
{
	/* Sort the array */
	ang_sort_aux(player_context, u, v, 0, n-1);
}
