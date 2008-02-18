#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(mangband, 1.0.0, jug@mangband.org)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])


# We don't like -02 in cflags
cflags_save="$CFLAGS"
# Checks for programs.
AC_PROG_CC
# But we do like our debug symbols and warnings
CFLAGS="$cflags_save -Wall -g"

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main], have_x11="yes")

AC_CHECK_LIB([SDL], [main], have_sdl="yes")

# Try finding ncurses or curses...
AC_CHECK_LIB([ncurses], [main],,[
    AC_CHECK_LIB([curses], [main], have_curs="yes")
    ])

AC_ARG_WITH(x11, AC_HELP_STRING([--with-x11],[Build with X11 support [default=yes]]), with_x11=$withval, with_x11="yes")
AC_ARG_WITH(gcu, AC_HELP_STRING([--with-gcu],[Build with GCU support [default=yes]]), with_gcu=$withval, with_gcu="yes")
AC_ARG_WITH(sdl, AC_HELP_STRING([--with-sdl],[Build with SDL support [default=yes]]), with_sdl=$withval, with_sdl="yes")

# HAVE + WITH = ENABLED [
if test "x$have_x11" = xyes && test "x$with_x11" = xyes 
then
    enabled_x11="yes"
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
if test "x$enabled_x11" != xyes && test "x$enabled_sdl" != xyes
then
    enabled_gcu="yes"
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

WINDOWS_HACKS="#"

AC_SUBST(WINDOWS_HACKS)

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