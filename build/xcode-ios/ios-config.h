#ifndef INCLUDED_IOS_CONFIG_H
#define INCLUDED_IOS_CONFIG_H

/* Since iOS builds do not go through autoconf, we have to
 * define some things ourselves. */

/**
 * HAVE_ defines
 */

#ifndef HAVE_MEMSET
#define HAVE_MEMSET
#endif

#define HAVE_STRNLEN

#ifndef HAVE_INTTYPES_H
#define HAVE_INTTYPES_H
#endif

#define HAVE_DIRENT_H

/**
 * Client config
 */

#define PKGDATADIR "./"

#ifndef USE_SDL2
#define USE_SDL2
#endif

#define MOBILE_UI
#define ON_IOS

#endif /* INCLUDED_IOS_CONFIG_H */
