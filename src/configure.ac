#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT(mangband, 1.0.0, jug@mangband.org)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])


# We don't like -02 in cflags
cflags_save="$CFLAGS"
# Checks for programs.
AC_PROG_CC
# But we do like our debug symbols and warnings
CFLAGS="$cflags_save -Wall -g -pipe"

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main], have_x11="yes")

AC_CHECK_LIB([SDL], [main], have_sdl="yes")

# Try finding ncurses or curses...
have_curs="yes"
AC_CHECK_LIB([ncurses], [main],,[
    AC_CHECK_LIB([curses], [main],, have_curs="no")
    ])

AC_CHECK_LIB([ws2_32], [main],[
    LIBS="$LIBS -lws2_32"
    ON_WINDOWS="yes"
])


AC_ARG_ENABLE(win, AC_HELP_STRING([--disable-win],[Ignore winsock presense]), ON_WINDOWS=$withval)

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
# Set flags
#GCU
if test "x$enabled_gcu" = xyes
then
    TERMDEFS="$TERMDEFS -DUSE_GCU"
fi
#X11
if test "x$enabled_x11" = xyes
then
    TERMDEFS="$TERMDEFS -DUSE_X11"
    LIBS="$LIBS -lX11"
fi
#SDL
if test "x$enabled_sdl" = xyes
then
    TERMDEFS="$TERMDEFS -DUSE_SDL"
    LIBS="$LIBS `sdl-config --libs`"
    CFLAGS="$CFLAGS `sdl-config --cflags`"
fi


# Entering Windows_Hacks zone
RES_FILE="#"
CLI_MAIN="#"
CLI_DMAIN="#"
CLI_LIBS="#"
SERV_LIBS="#"

# Winsock suggests windows!
if test "x$ON_WINDOWS" = xyes
then
    AC_MSG_NOTICE([Forcing Windows32 mode. Use --disable-win to change this.])
	if test "x$enabled_sdl" != xyes
	then
	    LIBS="$LIBS -mwindows"
	    AC_CHECK_LIB([winmm], [main], [LIBS="$LIBS -lwinmm"], AC_ERROR([lib winmm not found]))
	    AC_CHECK_LIB([gdi32], [main], [LIBS="$LIBS -lgdi32"], AC_ERROR([lib gdi32 not found]))
	    AC_CHECK_LIB([comdlg32], [main], [LIBS="$LIBS -lcomdlg32"], AC_ERROR([lib comdlg32 not found]))
	    CLI_MAIN="CLI_MAIN = client/main-win.c"
	else
	    CLI_DMAIN="CLI_DMAIN=-Dmain=SDL_main"
	fi
    CLI_LIBS="CLI_LIBS = $LIBS"
    SERV_LIBS="SERV_LIBS = -lws2_32 -lwinmm"
    LIBS=""
    RES_FILE="RES = windres -I client -o client/res.o client/Angband.rc"
fi


AC_SUBST(RES_FILE)
AC_SUBST(CLI_DMAIN)
AC_SUBST(CLI_MAIN)
AC_SUBST(CLI_LIBS)
AC_SUBST(SERV_LIBS)
# Leaving Windows_Hacks Zone

# Add Terminal Flags:
CFLAGS="$CFLAGS $TERMDEFS"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h memory.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h termio.h termios.h unistd.h values.h])

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
AC_CHECK_FUNCS([alarm atexit floor ftruncate gethostbyaddr gethostbyname gethostname gettimeofday inet_ntoa isascii memmove memset select socket strcasecmp strchr strdup strncasecmp strpbrk strrchr strspn strstr strtol])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_NOTICE([Configured. Type 'make' to build.])
#for debug you can use this:
#echo "SDL: $enabled_sdl X11: $enabled_x11 GCU: $enabled_gcu WIN32: $ON_WINDOWS"