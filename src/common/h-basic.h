/* File: h-basic.h */

#ifndef INCLUDED_H_BASIC_H
#define INCLUDED_H_BASIC_H

/*
 * The most basic "include" file.
 *
 * This file simply includes other low level header files.
 */

/* System Configuration (autocollected) */
#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#if !defined(_MSC_VER) && !defined(ON_IOS) && !defined(ON_ANDROID)
//#pragma message( "warning: Your build lacks 'src/config.h', reverting to crazy defaults" ) /* portable but stupid */
#warning "Your build lacks 'src/config.h', reverting to crazy defaults" /* good but not portable */
#endif
#endif

/* System Configuration */
#include "../h-config.h"

/* For systems lacking CONFIG_H */
#ifdef ON_IOS
#define HAVE_MEMSET 1
#define HAVE_STRNLEN 1
#define HAVE_INTTYPES_H 1
#define HAVE_DIRENT_H
#define HAVE_SELECT 1

#define PKGDATADIR "./"
#ifndef USE_SDL2
#define USE_SDL2
#endif
#ifndef USE_SDL2_TTF
#define USE_SDL2_TTF
#endif
#define MOBILE_UI
#endif

#ifdef ON_ANDROID
#define HAVE_MEMSET 1
#define HAVE_STRNLEN 1
#define HAVE_DIRENT_H
#define HAVE_INTTYPES_H 1
#define HAVE_SELECT 1
#define MOBILE_UI
#define USE_SDL2 1
#define USE_SDL2_TTF 1
#define PKGDATADIR "lib/"
#define USE_SDL_RWOPS
#endif

/* System includes/externs */
#include "h-system.h"

/* Basic types */
#include "h-type.h"

/* Basic constants and macros */
#include "h-define.h"

/* Basic networking stuff */
#include "h-net.h"


#endif
