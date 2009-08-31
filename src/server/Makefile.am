
bin_PROGRAMS = mangband

LDADD = ../common/libcommon.a $(SERVER_LDFLAGS)
AM_CFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -DCONFIG_PATH=\"$(sysconfdir)/mangband.cfg\" $(SERVER_CFLAGS)

mangband_SOURCES = birth.c cave.c cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c \
		   cmd6.c control.c dungeon.c files.c generate.c init1.c init2.c \
		   load2.c main.c melee1.c melee2.c monster1.c monster2.c netserver.c \
		   obj-info.c object1.c object2.c party.c save.c sched-win.c \
		   sched.c spells1.c spells2.c store.c tables.c use-obj.c util.c \
		   variable.c wilderness.c x-spell.c xtra1.c xtra2.c \
		   externs.h init.h mangband.h mdefines.h netserver.h
