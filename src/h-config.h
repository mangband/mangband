/* File: h-config.h */

#ifndef INCLUDED_H_CONFIG_H
#define INCLUDED_H_CONFIG_H

/*
 * Choose the hardware, operating system, and compiler.
 * Also, choose various "system level" compilation options.
 * A lot of these definitions take effect in "h-system.h"
 *
 * Note that you may find it simpler to define some of these
 * options in the "Makefile", especially any options describing
 * what "system" is being used.
 */


/*
 * no system definitions are needed for 4.3BSD, SUN OS, DG/UX
 */

/*
 * OPTION: Compile on a Macintosh (see "A-mac-h" or "A-mac-pch")
 */
#ifndef MACINTOSH
/* #define MACINTOSH */
#endif

/*
 * OPTION: Compile on Windows (automatic)
 */
#ifndef WINDOWS
/* #define WINDOWS */
#endif

/*
 * OPTION: Compile on an IBM (automatic)
 */
#ifndef MSDOS
/* #define MSDOS */
#endif

/*
 * OPTION: Compile on a SYS III version of UNIX
 */
#ifndef SYS_III
/* #define SYS_III */
#endif

/*
 * OPTION: Compile on a SYS V version of UNIX (not Solaris)
 */
#ifndef SYS_V
/* #define SYS_V */
#endif

/*
 * OPTION: Compile on a HPUX version of UNIX
 */
#ifndef HPUX
/* #define HPUX */
#endif

/*
 * OPTION: Compile on an SGI running IRIX
 */
#ifndef SGI
/* #define SGI */
#endif

/*
 * OPTION: Compile on Solaris, treat it as System V
 */
#ifndef SOLARIS
/* #define SOLARIS */
#endif

/*
 * OPTION: Compile on an ultrix/4.2BSD/Dynix/etc. version of UNIX,
 * Do not define this if you are on any kind of SUN OS.
 */
#ifndef ultrix
/* #define ultrix */
#endif



/*
 * OPTION: Compile on Pyramid, treat it as Ultrix
 */
#if defined(Pyramid)
# ifndef ultrix
#  define ultrix
# endif
#endif

/*
 * Extract the "ATARI" flag from the compiler [cjh]
 */
#if defined(__atarist) || defined(__atarist__)
# ifndef ATARI
#  define ATARI
# endif
#endif

/*
 * Extract the "ACORN" flag from the compiler
 */
#ifdef __riscos
# ifndef ACORN
#  define ACORN
# endif
#endif

/*
 * Extract the "SGI" flag from the compiler
 */
#ifdef sgi
# ifndef SGI
#  define SGI
# endif
#endif

/*
 * Extract the "MSDOS" flag from the compiler
 */
#ifdef __MSDOS__
# ifndef MSDOS
#  define MSDOS
# endif
#endif

/*
 * Extract the "ON_IOS" flag from the environment
 */
#ifdef __APPLE__
# include <TargetConditionals.h>
# if TARGET_OS_IPHONE
#  define ON_IOS
# elif TARGET_OS_OSX
#  define ON_OSX
# endif
#endif

/*
 * Extract the "WINDOWS" flag from the compiler
 */
#if defined(_Windows) || defined(__WINDOWS__) || \
    defined(__WIN32__) || defined(WIN32) || \
    defined(__WINNT__) || defined(__NT__)
# ifndef WINDOWS
#  define WINDOWS
# endif
#endif

/*
 * HACK: Extract some configure-like defines from Xcode
 */
#if defined(ON_OSX) && !defined(HAVE_CONFIG_H)
#define HAVE_MEMSET
#define HAVE_STRNLEN
#define HAVE_DIRENT_H
#define HAVE_SELECT 1
#define PKGDATADIR "./lib/"
#endif

/*
 * HACK: Extract some configure-like defines from the compiler
 */
#ifdef WINDOWS
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA 0x0600 /* _WIN32_WINNT_LONGHORN */
#endif
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
#define HAVE_INET_NTOP
#endif
#if defined (_MSC_VER) /* Any VS */
#define HAVE_MEMSET
#define HAVE_STRICMP
#define HAVE_STRDUP
#define HAVE_STRNLEN
#define HAVE___INT64
#define HAVE_STAT
#define HAVE_SELECT 1
#endif
#if defined (_MSC_VER) && (_MSC_VER >= 1900) /* VS2015 or later */
#define HAVE_INTTYPES_H
#define HAVE_INET_NTOP
#endif
#endif

/*
 * HACK: autoconf guessing wrong on mingw32, fix it */
#ifdef __MINGW32__
#ifndef HAVE_ARPA_INET_H
#ifdef HAVE_INET_NTOP
#undef HAVE_INET_NTOP
#endif
#endif
#endif
/*
 * HACK: on old mingw32, select exists, but is close to useless
 */
#ifdef WINDOWS
# if __MINGW32__ && (__GNUC__ < 3)
#  ifdef HAVE_SELECT
#   undef HAVE_SELECT
#  endif
# endif
#endif

/*
 * OPTION: Define "L64" if a "long" is 64-bits.  See "h-types.h".
 * (automatic for some platforms)
 */
#if defined(__alpha) || defined(__amd64__) \
 || defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) \
 || defined(_LP64) || defined(__LP64__) || defined(__ia64__) \
 || defined(__x86_64__)
# define L64
#endif



/*
 * OPTION: set "SET_UID" if the machine is a "multi-user" machine.
 * This option is used to verify the use of "uids" and "gids" for
 * various "Unix" calls, and of "pids" for getting a random seed,
 * and of the "umask()" call for various reasons, and to guess if
 * the "kill()" function is available, and for permission to use
 * functions to extract user names and expand "tildes" in filenames.
 * It is also used for "locking" and "unlocking" the score file.
 * Basically, SET_UID should *only* be set for "Unix" machines,
 * or for the "Atari" platform which is Unix-like, apparently
 */
#if !defined(MACINTOSH) && !defined(WINDOWS) && \
    !defined(MSDOS) && \
    !defined(AMIGA) && !defined(ACORN) && !defined(VM)
# define SET_UID
#endif


/*
 * OPTION: Set "USG" for "System V" versions of Unix
 * This is used to choose a "lock()" function, and to choose
 * which header files ("string.h" vs "strings.h") to include.
 * It is also used to allow certain other options, such as options
 * involving userid's, or multiple users on a single machine, etc.
 */
#ifdef SET_UID
# if defined(SYS_III) || defined(SYS_V) || defined(SOLARIS) || \
     defined(HPUX) || defined(SGI) || defined(ATARI)
#  ifndef USG
#   define USG
#  endif
# endif
#endif


/*
 * Every system seems to use its own symbol as a path separator.
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.  Note that any system that uses the
 * "period" as a separator (i.e. ACORN) will have to pretend that
 * it uses the slash, and do its own mapping of period <-> slash.
 * Note that the VM system uses a "flat" directory, and thus uses
 * the empty string for "PATH_SEP".
 */
#undef PATH_SEP
#define PATH_SEP "/"
#ifdef MACINTOSH
# undef PATH_SEP
# define PATH_SEP ":"
#endif
#if defined(WINDOWS)
# undef DEFAULT_PATH
# define DEFAULT_PATH ".\\lib\\"
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#if defined(MSDOS) || defined(OS2) || defined(USE_EMX)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#ifdef AMIGA
# undef PATH_SEP
# define PATH_SEP "/"
#endif
#ifdef __GO32__
# undef PATH_SEP
# define PATH_SEP "/"
#endif
#ifdef VM
# undef PATH_SEP
# define PATH_SEP ""
#endif

/* Hack -- for systems with no PATH_MAX (Windows?) */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* Hack -- DOS has 8.3 filesystem */
#undef FS_MAX_BASE_LEN
#if defined(MSDOS)
# define FS_MAX_BASE_LEN 8
#endif

/* Hack -- DOS and Win3.11 can't handle filename cases */
#undef FS_CASE_IGNORE
#if defined(MSDOS)
# define FS_CASE_IGNORE 2
#else
# if defined(WINDOWS) && !defined(WIN32)
#  define FS_CASE_IGNORE 1
# endif
#endif



/*
 * OPTION: Hack -- Make sure "strchr()" and "strrchr()" will work
 */
#if defined(SYS_III) || defined(SYS_V) || defined(MSDOS)
# if !defined(__TURBOC__) && !defined(__WATCOMC__)
#  define strchr index
#  define strrchr rindex
# endif
#endif




/*
 * OPTION: Define "HAVE_USLEEP" only if "usleep()" exists.
 * Note that this is only relevant for "SET_UID" machines
 */
#ifndef HAVE_CONFIG_H
#ifdef SET_UID
# if !defined(ultrix) && !defined(SOLARIS) && \
     !defined(SGI) && !defined(ISC) && !defined(USE_EMX)
#  define HAVE_USLEEP
# endif
#endif
#endif


#endif
