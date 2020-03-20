#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT(mangband, 1.5.3, team@mangband.org)
AM_INIT_AUTOMAKE(mangband, 1.5.3)
AM_CONFIG_HEADER(src/config.h)
AC_CANONICAL_HOST

# Enables SILENT RULES for automake >= 1.11
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([no])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O	# stops automake from complaining
AC_PROG_OBJC	# same, as long as we have .m files
AC_PROG_RANLIB
#CFLAGS="$CFLAGS -Wall -g -pipe"

# Defaults
CLIENT_CFLAGS=""
CLIENT_LDFLAGS=""
SERVER_CFLAGS=""
SERVER_LDFLAGS=""

# Evil Hack -- Extra framework param for OSX/SDL2
CLIENT_LDFLAG_SDL2=""

# Evil Hack -- Check for presence of Windres
AM_CONDITIONAL(HAVE_WINDRES, which windres > /dev/null)

# Evil Hack -- Check for Host OS
case "${host_os}" in
	mingw*)
	ON_WINDOWS="yes"
	;;
	cygwin*)
	ON_WINDOWS="yes"
	;;
	darwin*)
	ON_OSX="yes"
	LIBS="$LIBS -L/usr/X11R6/lib"
	CLIENT_LDFLAG_SDL2="-framework Cocoa"
	CLIENT_LDFLAGS="$CLIENT_LDFLAGS -framework CoreFoundation"
	;;
	freebsd*)
	CFLAGS="$CFLAGS -I/usr/local/include"
	LIBS="$LIBS -L/usr/local/lib" 
	;;
esac

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main], have_x11="yes")

AC_CHECK_LIB([SDL], [main], have_sdl="yes")

AC_CHECK_LIB([SDL2], [main], have_sdl2="yes")

AC_CHECK_LIB([SDL_image], [IMG_Load], have_sdl_image="yes")
AC_CHECK_LIB([SDL_ttf], [TTF_Init], have_sdl_ttf="yes")

AC_CHECK_LIB([SDL2_image], [IMG_Load], have_sdl2_image="yes")
AC_CHECK_LIB([SDL2_ttf], [TTF_Init], have_sdl2_ttf="yes")

# Try finding ncurses or curses...
have_curs="yes"
AC_CHECK_LIB([ncurses], [main], have_curs_lib="ncurses", [
    AC_CHECK_LIB([curses], [main], have_curs_lib="curses", have_curs="no")
    ])

# Evil Hack -- assume Carbon on OSX.
# We can't use AC_CHECK_LIB to test for Carbon, as it's invoked with
# -framework switch and not -l.
# We *could* use AC_CHECK_HEADER([Carbon/Carbon.h], have_crb="yes"),
# but that messes with ./configure tests order.
have_crb="$ON_OSX" # Note, that the "ON_OSX" itself is NOT reliable!!!

AC_ARG_ENABLE(win, AC_HELP_STRING([--disable-win],[Disable MS-Windows libraries and hacks]), ON_WINDOWS=$enableval)
AC_ARG_ENABLE(osx, AC_HELP_STRING([--disable-osx],[Disable Apple-OSX application bundling]), ON_OSX=$enableval)
AC_ARG_ENABLE(xdg, AC_HELP_STRING([--disable-xdg],[Disable Freedesktop icon bundling]), ON_XDG=$enableval, ON_XDG="yes")

AC_ARG_WITH(crb, AC_HELP_STRING([--with-crb],[Build with CRB support [default=yes]]), with_crb=$withval, with_crb="yes")
AC_ARG_WITH(x11, AC_HELP_STRING([--with-x11],[Build with X11 support [default=yes]]), with_x11=$withval, with_x11="yes")
AC_ARG_WITH(gcu, AC_HELP_STRING([--with-gcu],[Build with GCU support [default=yes]]), with_gcu=$withval, with_gcu="yes")
AC_ARG_WITH(sdl, AC_HELP_STRING([--with-sdl],[Build with SDL support [default=yes]]), with_sdl=$withval, with_sdl="yes")
AC_ARG_WITH(sdl2, AC_HELP_STRING([--with-sdl2],[Build with SDL2 support [default=yes]]), with_sdl2=$withval, with_sdl2="yes")

AC_ARG_WITH(sdl-image, AC_HELP_STRING([--with-sdl-image],[Build with SDL_Image support [default=yes]]), with_sdl_image=$withval, with_sdl_image="no")
AC_ARG_WITH(sdl-ttf, AC_HELP_STRING([--with-sdl-ttf],[Build with SDL_TTF support [default=yes]]), with_sdl_ttf=$withval, with_sdl_ttf="yes")
AC_ARG_WITH(sdl2-image, AC_HELP_STRING([--with-sdl2-image],[Build with SDL2_Image support [default=yes]]), with_sdl2_image=$withval, with_sdl2_image="no")
AC_ARG_WITH(sdl2-ttf, AC_HELP_STRING([--with-sdl2-ttf],[Build with SDL2_TTF support [default=yes]]), with_sdl2_ttf=$withval, with_sdl2_ttf="yes")

# We need to know if we're building for OSX, badly.
if test "x$ON_OSX" = xyes
then
	CLIENT_CFLAGS="$CLIENT_CFLAGS -DTARGET_OS_OSX=1"
	SERVER_CFLAGS="$SERVER_CFLAGS -DTARGET_OS_OSX=1"
fi

# HAVE + WITH = ENABLED [
if test "x$have_x11" = xyes && test "x$with_x11" = xyes 
then
    enabled_x11="yes"
fi
if test "x$have_curs" = xyes && test "x$with_gcu" != xno 
then
    enabled_gcu="yes"
fi
if test "x$have_sdl" = xyes && test "x$with_sdl" = xyes 
then
    enabled_sdl="yes"
fi
if test "x$have_sdl2" = xyes && test "x$with_sdl2" = xyes 
then
    enabled_sdl2="yes"
fi
if test "x$have_sdl_image" = xyes && test "x$with_sdl_image" = xyes
then
    enabled_sdl_image="yes"
fi
if test "x$have_sdl_ttf" = xyes && test "x$with_sdl_ttf" = xyes
then
    enabled_sdl_ttf="yes"
fi
if test "x$have_sdl2_image" = xyes && test "x$with_sdl2_image" = xyes 
then
    enabled_sdl2_image="yes"
fi
if test "x$have_sdl2_ttf" = xyes && test "x$with_sdl2_ttf" = xyes 
then
    enabled_sdl2_ttf="yes"
fi

if test "x$have_crb" = xyes && test "x$with_crb" = xyes 
then
    enabled_crb="yes"
fi
# ]
# If we have SDL+X11, disable X11
if test "x$enabled_x11" = xyes && test "x$enabled_sdl" = xyes
then
    enabled_x11="no"
fi
# NOTE: reverting this for now! switch back when ready -flm-
# If we have SDL2, disable SDL
if test "x$enabled_sdl" = xyes && test "x$enabled_sdl2" = xyes
then
    enabled_sdl2="no"
fi

# If we don't have SDL, disable SDL_image and SDL_ttf
if test "x$enabled_sdl" != xyes
then
    enabled_sdl_image="no"
    enabled_sdl_ttf="no"
    AC_MSG_NOTICE([Disabling SDL helper libs.])
fi
# If we don't have SDL2, disable SDL2_image and SDL2_ttf
if test "x$enabled_sdl2" != xyes
then
    enabled_sdl2_image="no"
    enabled_sdl2_ttf="no"
    AC_MSG_NOTICE([Disabling SDL2 helper libs.])
fi

# If we have nothing, enable GCU
if test "x$enabled_x11" != xyes && test "x$enabled_sdl" != xyes && test "x$enabled_sdl2" != xyes && test "x$enabled_crb" != xyes && test "x$ON_WINDOWS" != xyes
then
    enabled_gcu="yes"
fi

# If we have CRB, disable everything else
if test "x$enabled_crb" = xyes
then
    enabled_gcu="no"
    enabled_x11="no"
fi
    
# But if we try GCU without ncurses - quit with an error
if test "x$enabled_gcu" = xyes && test "x$have_curs" != xyes
then
    AC_ERROR([For GCU visual module, ncurses library is required. Try getting libncurses5-dev package.])
fi

# If we're building SDL or X11, add Freedesktop bundling
bundle_xdg="no"
if test "x$enabled_x11" = xyes || test "x$enabled_sdl" = xyes
then
    bundle_xdg="yes"
fi
# (But not on WINDOWS/OSX) or when explicitly off
if test "x$ON_WINDOWS" = xyes || test "x$ON_OSX" = xyes || test "x$ON_XDG" != xyes
then
    bundle_xdg="no"
fi

# Prepare conditionals
AM_CONDITIONAL(USE_CRB, test "x$enabled_crb" = xyes)
AM_CONDITIONAL(USE_X11, test "x$enabled_x11" = xyes)
AM_CONDITIONAL(USE_SDL, test "x$enabled_sdl" = xyes)
AM_CONDITIONAL(USE_SDL2, test "x$enabled_sdl2" = xyes)
# Set flags
#GCU
if test "x$enabled_gcu" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_GCU -I/usr/include/ncurses"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -l$have_curs_lib"
    DISPMOD="$DISPMOD GCU"
fi
#X11
if test "x$enabled_x11" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_X11"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lX11"
    DISPMOD="$DISPMOD X11"
fi
#SDL
if test "x$enabled_sdl" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL `sdl-config --cflags`"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS `sdl-config --libs`"
    DISPMOD="$DISPMOD SDL"
fi
#SDL2
if test "x$enabled_sdl2" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL2 `sdl2-config --cflags`"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS `sdl2-config --libs` -lm $CLIENT_LDFLAG_SDL2"
    DISPMOD="$DISPMOD SDL2"
fi
#SDL_image
if test "x$enabled_sdl_image" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL_IMAGE"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lSDL_image"
    DISPMOD="$DISPMOD SDL_image"
fi
#SDL_ttf
if test "x$enabled_sdl_ttf" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL_TTF"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lSDL_ttf"
    DISPMOD="$DISPMOD SDL_ttf"
fi
#SDL2_image
if test "x$enabled_sdl2_image" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL2_IMAGE"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lSDL2_image"
    #DISPMOD="$DISPMOD SDL2_image"
fi
#SDL2_ttf
if test "x$enabled_sdl2_ttf" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_SDL2_TTF"
    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lSDL2_ttf"
    #DISPMOD="$DISPMOD SDL2_ttf"
fi

# Prepare XDG conditional
if test "x$bundle_xdg" = xyes
then
    AC_MSG_NOTICE([Enabling Freedesktop bundling. Use --disable-xdg to change this.])
    DISPMOD="$DISPMOD XDG-bundle"
fi
AM_CONDITIONAL(ON_XDG, test "x$bundle_xdg" = xyes)

#added this conditional for WINDRES causing linux build to bail - is this even right? -kts
AM_CONDITIONAL(ON_WINDOWS, test "x$ON_WINDOWS" = xyes)

#WIN (might be SDL on Windows)
if test "x$ON_WINDOWS" = xyes
then
    AC_MSG_NOTICE([Forcing Windows32 mode. Use --disable-win to change this.])
	AC_CHECK_LIB([ws2_32], [main],[LIBS="$LIBS -lws2_32"], AC_ERROR([lib ws2_32 not found]))
	AC_CHECK_LIB([winmm], [main], [LIBS="$LIBS -lwinmm"], AC_ERROR([lib winmm not found]))
	AC_CHECK_LIB([iphlpapi], [main], [LIBS="$LIBS -liphlpapi"], AC_ERROR([lib iphlpapi not found]))
	if test "x$enabled_sdl" != xyes && test "x$enabled_sdl2" != xyes
	then
	    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -mwindows"
	    AC_CHECK_LIB([gdi32], [main], [CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lgdi32"], AC_ERROR([lib gdi32 not found]))
	    AC_CHECK_LIB([comdlg32], [main], [CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lcomdlg32"], AC_ERROR([lib comdlg32 not found]))
	    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_WIN"
	    DISPMOD="$DISPMOD WIN32"
	fi
fi

#CRB
if test "x$enabled_crb" = xyes
then
	AC_MSG_NOTICE([Building Carbon version. Use --without-crb to change this.])
	#AC_PROG_OBJC
	#AC_LANG_PUSH([Objective C])

	ARCH="-arch i386 -arch ppc"
	WARNINGS="-W -Wall -Wno-unused-parameter -Wno-missing-field-initializers"
	JUST_CFLAGS="-std=c99 -Wdeclaration-after-statement"
	OBJ_CFLAGS="-std=c99 -x objective-c -mmacosx-version-min=10.0"
	CLIENT_CFLAGS="$CLIENT_CFLAGS -I. $WARNINGS -DUSE_CRB -DMACH_O_CARBON -DHAVE_MKSTEMP -DPRIVATE_USER_PATH=\"~/Library/Preferences\" -DUSE_PRIVATE_PATHS $(ARCH)"
	CLIENT_LDFLAGS="$CLIENT_LDFLAGS -framework CoreFoundation -framework QuickTime -framework Carbon -framework Cocoa"
	DISPMOD="$DISPMOD Carbon"
	AC_SUBST(JUST_CFLAGS)
	AC_SUBST(OBJ_CFLAGS)
	AC_SUBST(OBJ_CLIBS)
fi

#OSX -- App Bundle "support" (see src/client/osx/Makefile.am)
CLIENT_BUNDLE=""
if test "x$ON_OSX" = xyes
then
	AC_MSG_NOTICE([Going to create Application Bundle. Use --disable-osx to change this.])
	CLIENT_BUNDLE="mangclient.app"
	DISPMOD="$DISPMOD App.Bundle"
fi
AC_SUBST(CLIENT_BUNDLE)

# Add Terminal Flags:
AC_SUBST(CLIENT_CFLAGS)
AC_SUBST(CLIENT_LDFLAGS)
AC_SUBST(SERVER_CFLAGS)
AC_SUBST(SERVER_LDFLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h dirent.h memory.h netdb.h netinet/in.h ifaddrs.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termio.h termios.h unistd.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm atexit gethostbyaddr gethostbyname gethostname gettimeofday inet_ntop inet_ntoa isascii memmove memset select socket stat strcasecmp strchr strdup strnlen strncasecmp stricmp strpbrk strrchr strspn strstr strtol usleep])

AC_MSG_NOTICE([enabled -$DISPMOD])
AC_OUTPUT( Makefile )
AC_MSG_NOTICE([Configured. Type 'make' to build.])
#for debug you can use this:
#echo "SDL: $enabled_sdl X11: $enabled_x11 GCU: $enabled_gcu CRB: $enabled_crb WIN32: $ON_WINDOWS OSX: $ON_OSX ; $host_os"
