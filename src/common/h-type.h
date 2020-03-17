
/* File: h-type.h */

#ifndef INCLUDED_H_TYPE_H
#define INCLUDED_H_TYPE_H

/*
 * Basic "types".
 *
 * Note the attempt to make all basic types have 4 letters.
 * This improves readibility and standardizes the code.
 *
 * Likewise, all complex types are at least 4 letters.
 * Thus, almost every three letter word is a legal variable.
 * But beware of certain reserved words ('for' and 'if' and 'do').
 *
 * Note that the type used in structures for bit flags should be uint.
 * As long as these bit flags are sequential, they will be space smart.
 *
 * Note that on some machines, apparently "signed char" is illegal.
 *
 * It must be true that char/byte takes exactly 1 byte
 * It must be true that sind/uind takes exactly 2 bytes
 * It must be true that sbig/ubig takes exactly 4 bytes
 *
 * On Sparc's, a sint takes 4 bytes (2 is legal)
 * On Sparc's, a uint takes 4 bytes (2 is legal)
 * On Sparc's, a long takes 4 bytes (8 is legal)
 * On Sparc's, a huge takes 4 bytes (8 is legal)
 * On Sparc's, a vptr takes 4 bytes (8 is legal)
 * On Sparc's, a real takes 8 bytes (4 is legal)
 *
 * Note that some files have already been included by "h-include.h"
 * These include <stdio.h> and <sys/types>, which define some types
 * In particular, uint is defined so we do not have to define it
 *
 */



/*** Special 4 letter names for some standard types ***/


/* A standard pointer (to "void" because ANSI C says so) */
typedef void *vptr;

/* A simple pointer (to unmodifiable strings) */
typedef const char *cptr;



/* Error codes for function return values */
/* Success = 0, Failure = -N, Problem = +N */
typedef int errr;


/*
 * Hack -- prevent problems with non-MACINTOSH
 */
#undef uint
#define uint uint_hack

/*
 * Hack -- prevent problems with MSDOS and WINDOWS
 */
#undef huge
#define huge huge_hack

/*
 * Hack -- prevent problems with AMIGA
 */
#undef byte
#define byte byte_hack

/*
 * Hack -- prevent problems with C++
 */
#undef bool
#define bool bool_hack


/* Note that "signed char" is not always "defined" */
/* So always use "s16b" to hold small signed values */
/* A signed byte of memory */
/* typedef signed char syte; */

/* Note that unsigned values can cause math problems */
/* An unsigned byte of memory */
typedef unsigned char byte;

/* Note that a bool is smaller than a full "int" */
/* Simple True/False type */
typedef char bool;


/* A signed, standard integer (at least 2 bytes) */
typedef int sint;

/* An unsigned, "standard" integer (often pre-defined) */
typedef unsigned int uint;


/* The largest possible signed integer (pre-defined) */
/* typedef long long; */

/*
 * NOT A HACK
 *
 * Gets basic "types" from <inttypes.h>/"stdint.h".
 * When those files are in place, our task is *extremely* simplified. "stdin.h"
 * already contains all the types we need, and we simply re-map them to Angband
 * equivalents.
 *
 * To use functions like "printf" and "scanf", we also need to know correct
 * format specifiers (i.e. "%d" might mean either 16, either 32, either 64 bits
 * depending on a machine, so we can't use it reliably). Those are provided
 * by <inttypes.h> too, excplicitly requested by __STDC_FORMAT_MACROS define.
 * This gives us: 
 *  PRId16 SCNd16	s16b
 *  PRIu16 SCNu16	u16b
 *  PRId32 SCNd32	s32b
 *  PRIu32 SCNu32	u32b
 *  PRId64 SCNd64	s64b
 *  PRIu64 SCNu64	u64b, huge
 * With usage like:
 *  printf("%" PRIu16, (u16b)variable);
 *
 * (The _rest_ of this file indeed is a hack!)
 */
#ifdef HAVE_INTTYPES_H
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

typedef uint16_t u16b;
typedef  int16_t s16b;
typedef uint32_t u32b;
typedef  int32_t s32b;
typedef uint64_t u64b;
typedef  int64_t s64b;

/* Hack -- mingw32 on windows is likely to defer to msvcrt.dll,
 * which doesn't support "hh", although they ARE present in inttypes. */
#if defined(__MINGW32__) && defined(WINDOWS)
#undef SCNd8
#undef SCNu8
#endif

#else
/* If we don't have <inttypes.h>, revert to old method: */

/* Format specifiers for 8 bit values */
#define PRIu8 "u"
#define PRId8 "d"
/* We leave SCNu8 and SCNd8 undefined. There is no way to tell... :(
 * This shall lead to compile-time failures, which is good.
 * To sscanf for 8-bit values, read into a 16-bit variable, then truncate! */

/* Signed/Unsigned 16 bit value */
typedef signed short s16b;
typedef unsigned short u16b;
#define SCNu16 "hu"
#define PRIu16 "hu"
#define SCNd16 "hd"
#define PRId16 "hd"

/* Signed/Unsigned 32 bit value */
#ifdef L64	/* 64 bit longs */
typedef signed int s32b;
typedef unsigned int u32b;
#define SCNu32 "u"
#define PRIu32 "u"
#define SCNd32 "d"
#define PRId32 "d"
#else
typedef signed long s32b;
typedef unsigned long u32b;
#define SCNu32 "lu"
#define PRIu32 "lu"
#define SCNd32 "ld"
#define PRId32 "ld"
#endif

/* Signed/Unsigned 64 bit value */
#if defined(HAVE___INT64)
typedef __int64 s64b;
typedef unsigned __int64 u64b;
#define PRIu64 "I64u"
#define SCNu64 "I64u"
#define PRId64 "I64d"
#define SCNd64 "I64d"
#elif defined(L64) /* Long is 64-bit */
typedef signed long s64b;
typedef unsigned long u64b;
#define PRIu64 "lu"
#define SCNu64 "lu"
#define PRId64 "ld"
#define SCNd64 "ld"
#else /* Long is 32-bit, hopefully "long long" is 64-bit */
typedef signed long long s64b;
typedef unsigned long long u64b;
#define PRIu64 "llu"
#define SCNu64 "llu"
#define PRId64 "lld"
#define SCNd64 "lld"
#endif

#endif /* !HAVE_INTTYPES_H */

/* The largest possible unsigned integer */
typedef u64b huge;

/* Turn counter type "huge turn" (largest number ever) */
#define HTURN_ERA_FLIP 1000000
typedef struct hturn {
	huge era;
	huge turn;
} hturn;
/* You must use those function instead of math operations with hturns: */
#define ht_eq(A,B) (((A)->turn == (B)->turn) && ((A)->era == (B)->era))
#define ht_fix_up(H) while((H)->turn >= HTURN_ERA_FLIP) { \
	(H)->turn -= HTURN_ERA_FLIP; (H)->era++; }
#define ht_fix_down(H) while((signed)(H)->turn < 0) { \
	(H)->turn += HTURN_ERA_FLIP; (H)->era--; }
#define ht_add_ht(H,Q) (H)->era += (Q)->era;(H)->turn += (Q)->turn;ht_fix_up((H))
#define ht_subst_ht(H,Q) (H)->era -= (Q)->era;(H)->turn -= (Q)->turn;ht_fix_down((H))
#define ht_subst(H,T) (H)->turn -= (T);ht_fix_down((H))
#define ht_add(H,T) (H)->turn += (T);ht_fix_up((H))
#define ht_clr(H) (H)->era = (H)->turn = 0;



/*** Pointers to all the basic types defined above ***/

typedef errr *errr_ptr;
typedef char *char_ptr;
typedef byte *byte_ptr;
typedef bool *bool_ptr;
typedef sint *sint_ptr;
typedef uint *uint_ptr;
typedef huge *huge_ptr;
typedef s16b *s16b_ptr;
typedef u16b *u16b_ptr;
typedef s32b *s32b_ptr;
typedef u32b *u32b_ptr;
typedef vptr *vptr_ptr;
typedef cptr *cptr_ptr;



/*** Pointers to Functions with simple return types and any args ***/

typedef void	(*func_void)(void);
typedef errr	(*func_errr)(void);
typedef char	(*func_char)(void);
typedef byte	(*func_byte)(void);
typedef bool	(*func_bool)(void);
typedef sint	(*func_sint)(void);
typedef uint	(*func_uint)(void);
typedef vptr	(*func_vptr)(void);
typedef cptr	(*func_cptr)(void);



/*** Pointers to Functions of special types (for various purposes) ***/

/* A generic function takes a user data and a special data */
typedef errr	(*func_gen)(vptr, vptr);

/* An equality testing function takes two things to compare (bool) */
typedef bool	(*func_eql)(vptr, vptr);

/* A comparison function takes two things and to compare (-1,0,+1) */
typedef sint	(*func_cmp)(vptr, vptr);

/* A hasher takes a thing (and a max hash size) to hash (0 to siz - 1) */
typedef uint	(*func_hsh)(vptr, uint);

/* A key extractor takes a thing and returns (a pointer to) some key */
typedef vptr	(*func_key)(vptr);



#endif
