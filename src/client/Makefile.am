
bin_PROGRAMS = mangclient

LDADD = ../common/libcommon.a $(CLIENT_LDFLAGS)
AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(CLIENT_CFLAGS)

if HAVE_WINDRES

mangclient_RESOURCES = Angband.rc

endif

mangclient_SOURCES = c-birth.c c-cmd.c c-files.c c-init.c c-inven.c c-spell.c c-store.c \
		     c-tables.c c-util.c c-xtra1.c c-xtra2.c client.c maid-x11.c \
		     maim-sdl.c main-cap.c main-gcu.c main-ibm.c main-sdl.c main-win.c \
		     main-x11.c main-xaw.c net-client.c readdib.c sdl-xxx.c set_focus.c \
		     variable.c z-term.c \
		     $(mangclient_RESOURCES) \
		     c-angband.h c-defines.h c-externs.h maid-x11.h main-sdl.h net-client.h \
	             readdib.h resource.h z-term.h

.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@