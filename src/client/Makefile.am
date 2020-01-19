
mangclient_LDADD = src/libcommon.a $(CLIENT_LDFLAGS)
mangclient_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(CLIENT_CFLAGS)

include src/client/osx/Makefile.am

if HAVE_WINDRES
# Added this conditional as it broke Linux - why is it needed for *nix? -kts
if ON_WINDOWS

mangclient_RESOURCES = src/client/win/Angband.rc
endif
endif

mangclient_SOURCES = \
		src/client/c-birth.c src/client/c-cmd.c src/client/c-files.c \
		src/client/c-init.c src/client/c-inven.c src/client/c-spell.c \
		src/client/c-store.c src/client/c-tables.c src/client/c-util.c \
		src/client/c-xtra1.c src/client/c-xtra2.c src/client/client.c \
		src/client/ui.c src/client/ui.h src/client/c-cmd0.c \
		src/client/main-cap.c src/client/main-gcu.c src/client/main-ibm.c \
		src/client/main-win.c src/client/main-xaw.c src/client/net-client.c \
		src/client/set_focus.c src/client/c-variable.c \
		src/client/grafmode.c \
		src/client/z-term.c \
		$(mangclient_RESOURCES) \
		src/client/c-angband.h src/client/c-defines.h src/client/c-externs.h \
		src/client/net-client.h \
		src/client/grafmode.h \
		src/client/z-term.h

mangclient_SOURCES += src/client/win/resource.h \
		src/client/win/readpng.c \
		src/client/win/readdib.c \
		src/client/win/readdib.h

mangclient_SOURCES += src/client/lupng/lupng.c src/client/lupng/miniz.c \
		src/client/lupng/lupng.h src/client/lupng/miniz.h

if USE_CRB

mangclient_SOURCES += src/client/main-crb.c src/client/osx/osx_tables.h \
		src/client/osx/appl-dir.m src/client/osx/appl-dir.h

# HACK -- Isolate JUST_CFLAGS from OBJ_CFLAGS
src/client/mangclient-main-crb.o: src/client/main-crb.c
	$(CC) $(AM_CFLAGS) $(OBJ_CFLAGS) -c -o $@ $<
.o:.c
	$(CC)  $(AM_CFLAGS) $(JUST_CFLAGS) -c -o $@ $<


endif

if USE_X11

mangclient_SOURCES += src/client/maid-x11.c src/client/main-x11.c src/client/maid-x11.h

endif

if USE_SDL

mangclient_SOURCES += src/client/maim-sdl.c src/client/main-sdl.c src/client/main-sdl.h src/client/sdl-font.c src/client/sdl-font.h src/client/sdl-sound.c src/client/sdl-sound.h

endif

if USE_SDL2

mangclient_SOURCES += src/client/main-sdl2.c src/client/main-sdl2.h src/client/sdl-font.c src/client/sdl-font.h src/client/sdl-sound.c src/client/sdl-sound.h

endif

.rc.o:
	windres $< -o $@
%.o : %.rc
	windres $< -o $@
