#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT(mangband, 1.2.0, team@mangband.org)
AM_INIT_AUTOMAKE(mangband, 1.2.0)
AM_CONFIG_HEADER(src/config.h)
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_RANLIB
#CFLAGS="$CFLAGS -Wall -g -pipe"

# Defaults
CLIENT_CFLAGS=""
CLIENT_LDFLAGS=""
SERVER_CFLAGS=""
SERVER_LDFLAGS=""

# Evil Hack -- Check for presence of Windres
AM_CONDITIONAL(HAVE_WINDRES, which windres > /dev/null)

# Evil Hack -- Check for Host OS
case "${host_os}" in
	mingw*)
	ON_WINDOWS="yes"
	;;
	darwin*)
	LIBS="$LIBS -L/usr/X11R6/lib"
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

# Try finding ncurses or curses...
have_curs="yes"
AC_CHECK_LIB([ncurses], [main],,[
    AC_CHECK_LIB([curses], [main],, have_curs="no")
    ])


AC_ARG_ENABLE(win, AC_HELP_STRING([--disable-win],[Disable MS-Windows libraries and hacks]), ON_WINDOWS=$withval)

AC_ARG_WITH(x11, AC_HELP_STRING([--with-x11],[Build with X11 support [default=yes]]), with_x11=$withval, with_x11="yes")
AC_ARG_WITH(gcu, AC_HELP_STRING([--with-gcu],[Build with GCU support [default=yes]]), with_gcu=$withval, with_gcu="yes")
AC_ARG_WITH(sdl, AC_HELP_STRING([--with-sdl],[Build with SDL support [default=yes]]), with_sdl=$withval, with_sdl="yes")

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
# ]
# If we have SDL+X11, disable X11
if test "x$enabled_x11" = xyes && test "x$enabled_sdl" = xyes
then
    enabled_x11="no"
fi

# If we have nothing, enable GCU
if test "x$enabled_x11" != xyes && test "x$enabled_sdl" != xyes && test "x$ON_WINDOWS" != xyes
then
    enabled_gcu="yes"
fi

# But if we try GCU without ncurses - quit with an error
if test "x$enabled_gcu" = xyes && test "x$have_curs" != xyes
then
    AC_ERROR([For GCU visual module, ncurses library is required. Try getting libncurses5-dev package.])
fi

# Prepare conditionals
AM_CONDITIONAL(USE_CRB, test "x$enabled_crb" = xyes)
AM_CONDITIONAL(USE_X11, test "x$enabled_x11" = xyes)
AM_CONDITIONAL(USE_SDL, test "x$enabled_sdl" = xyes)
# Set flags
#GCU
if test "x$enabled_gcu" = xyes
then
    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_GCU -I/usr/include/ncurses"
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


#WIN (might be SDL on Windows)
if test "x$ON_WINDOWS" = xyes
then
    AC_MSG_NOTICE([Forcing Windows32 mode. Use --disable-win to change this.])
	AC_CHECK_LIB([ws2_32], [main],[LIBS="$LIBS -lws2_32"], AC_ERROR([lib ws2_32 not found]))
	AC_CHECK_LIB([winmm], [main], [LIBS="$LIBS -lwinmm"], AC_ERROR([lib winmm not found]))
	if test "x$enabled_sdl" != xyes
	then
	    CLIENT_LDFLAGS="$CLIENT_LDFLAGS -mwindows"
	    AC_CHECK_LIB([gdi32], [main], [CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lgdi32"], AC_ERROR([lib gdi32 not found]))
	    AC_CHECK_LIB([comdlg32], [main], [CLIENT_LDFLAGS="$CLIENT_LDFLAGS -lcomdlg32"], AC_ERROR([lib comdlg32 not found]))
	    CLIENT_CFLAGS="$CLIENT_CFLAGS -DUSE_WIN"
	    DISPMOD="$DISPMOD WIN32"
	fi
fi

# Add Terminal Flags:
AC_SUBST(CLIENT_CFLAGS)
AC_SUBST(CLIENT_LDFLAGS)
AC_SUBST(SERVER_CFLAGS)
AC_SUBST(SERVER_LDFLAGS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h dirent.h memory.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h termio.h termios.h unistd.h values.h])

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
AC_CHECK_FUNCS([alarm atexit floor ftruncate getcurx gethostbyaddr gethostbyname gethostname gettimeofday inet_ntop inet_ntoa isascii memmove memset select socket strcasecmp strchr strdup strnlen strncasecmp strpbrk strrchr strspn strstr strtol])

AC_MSG_NOTICE([enabled -$DISPMOD])
AC_OUTPUT( src/client/Makefile src/client/osx/Makefile src/common/Makefile src/Makefile src/server/Makefile lib/bone/Makefile lib/edit/Makefile lib/save/Makefile lib/help/Makefile lib/data/Makefile lib/pref/Makefile lib/user/Makefile lib/xtra/sound/Makefile lib/xtra/graf/Makefile lib/xtra/font/Makefile lib/xtra/Makefile lib/Makefile Makefile ) 
AC_MSG_NOTICE([Configured. Type 'make' to build.])
#for debug you can use this:
#echo "SDL: $enabled_sdl X11: $enabled_x11 GCU: $enabled_gcu WIN32: $ON_WINDOWS ; $host_os"
