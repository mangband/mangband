/* $Id: const.h,v 1.1.1.1 1999/10/26 19:20:49 root Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef CONST_H
#define	CONST_H

#include <limits.h>
#include <math.h>

/*
 * FLT_MAX and RAND_MAX is ANSI C standard, but some systems (BSD) use
 * MAXFLOAT and INT_MAX instead.
 */
#ifndef	FLT_MAX
#   if defined(__sgi) || defined(__FreeBSD__)
#       include <float.h>	/* FLT_MAX for SGI Personal Iris or FreeBSD */
#   else
#	if defined(__sun__)
#           include <values.h>	/* MAXFLOAT for suns */
#	endif
#	ifdef VMS
#	    include <float.h>
#	endif
#   endif
#   if !defined(FLT_MAX)
#	if defined(MAXFLOAT)
#	    define FLT_MAX	MAXFLOAT
#	else
#	    define FLT_MAX	1e30f	/* should suffice :-) */
#	endif
#   endif
#endif
#ifndef	RAND_MAX
    /*
     * Ough!  If this is possible then we shouldn't be using RAND_MAX!
     * Older systems which don't have RAND_MAX likely should have it
     * to be defined as 32767, not as INT_MAX!
     * We better get our own pseudo-random library to overcome this mess
     * and get a uniform solution for everything.
     */
#   define  RAND_MAX	INT_MAX
#endif

#undef ABS
#define ABS(x)			( (x)<0 ? -(x) : (x) )
#ifndef MAX
#   define MIN(x, y)		( (x)>(y) ? (y) : (x) )
#   define MAX(x, y)		( (x)>(y) ? (x) : (y) )
#endif
#define sqr(x)			( (x)*(x) )
#define LENGTH(x, y)		( hypot( (double) (x), (double) (y) ) )
#define VECTOR_LENGTH(v)	( hypot( (double) (v).x, (double) (v).y ) )

#define MAX_ID			4096		    /* Should suffice :) */

#define MAX_MSGS		8
#define MAX_CHARS		80
#define MSG_LEN			256

#ifdef __GNUC__
#define	INLINE	inline
#else
#define INLINE
#endif /* __GNUC__ */

#if defined(__sun__) && !defined(__svr4__)
#  define srand(s)	srandom(s)
#  define rand()	random()
#endif /* __sun__ */

#if defined(ultrix) || defined(AIX)
/* STDRUP_OBJ should be uncomented in Makefile also */
extern char* strdup(const char*);
#endif

#endif
