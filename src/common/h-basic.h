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
#if !defined(_MSC_VER)
//#pragma message( "warning: Your build lacks 'src/config.h', reverting to crazy defaults" ) /* portable but stupid */
#warning "Your build lacks 'src/config.h', reverting to crazy defaults" /* good but not portable */
#endif
#endif

/* System Configuration */
#include "../h-config.h"

/* System includes/externs */
#include "h-system.h"

/* Basic types */
#include "h-type.h"

/* Basic constants and macros */
#include "h-define.h"

/* Basic networking stuff */
#include "h-net.h"


#endif
