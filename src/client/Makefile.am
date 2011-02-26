
bin_PROGRAMS = mangclient

LDADD = ../common/libcommon.a $(CLIENT_LDFLAGS)
AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(CLIENT_CFLAGS)

if HAVE_WINDRES

mangclient_RESOURCES = Angband.rc

endif

mangclient_SUBDIRS = osx

mangclient_SOURCES = c-birth.c c-cmd.c c-files.c c-init.c c-inven.c c-spell.c c-store.c \
		     c-tables.c c-util.c c-xtra1.c c-xtra2.c client.c \
		     main-cap.c main-gcu.c main-ibm.c main-win.c \
		     main-xaw.c net-client.c readdib.c set_focus.c \
		     variable.c z-term.c \
		     $(mangclient_RESOURCES) \
		     c-angband.h c-defines.h c-externs.h net-client.h \
	             readdib.h resource.h z-term.h

if USE_CRB

mangclient_SOURCES += main-crb.c osx/osx_tables.h

endif

if USE_X11

mangclient_SOURCES += maid-x11.c main-x11.c maid-x11.h

endif

if USE_SDL

mangclient_SOURCES += maim-sdl.c main-sdl.c sdl-xxx.c main-sdl.h

endif

.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@