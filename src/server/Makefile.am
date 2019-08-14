
mangband_LDADD = src/libcommon.a $(SERVER_LDFLAGS)
mangband_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -DLOCALSTATEDIR=\"$(localstatedir)/mangband\" -DCONFIG_PATH=\"$(sysconfdir)/mangband.cfg\" $(SERVER_CFLAGS)

mangband_SOURCES = \
		src/server/birth.c src/server/cave.c src/server/pathfind.c \
		src/server/cmd1.c src/server/cmd2.c src/server/cmd3.c \
		src/server/cmd4.c src/server/cmd5.c src/server/cmd6.c \
		src/server/control.c src/server/dungeon.c src/server/files.c \
		src/server/generate.c src/server/init1.c src/server/init2.c \
		src/server/load2.c src/server/main.c src/server/melee1.c \
		src/server/melee2.c src/server/monster1.c src/server/monster2.c \
		src/server/net-server.c \
		src/server/obj-info.c src/server/object1.c src/server/object2.c \
		src/server/randart.c \
		src/server/party.c src/server/save.c src/server/net-game.c \
		src/server/spells1.c src/server/spells2.c src/server/store.c \
		src/server/tables.c src/server/use-obj.c src/server/util.c \
		src/server/variable.c src/server/wilderness.c src/server/x-spell.c \
		src/server/xtra1.c src/server/xtra2.c \
		src/server/externs.h src/server/init.h src/server/mangband.h \
		src/server/mdefines.h src/server/net-server.h src/server/net-game.h
