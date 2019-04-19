AUTOMAKE_OPTIONS = subdir-objects

dist_sysconf_DATA = mangband.cfg

EXTRA_DIST = src/makefile.bcc src/h-config.h

EXTRA_PROGRAMS = 

bin_PROGRAMS = mangclient mangband

include lib/Makefile.am
include src/common/Makefile.am
include src/client/Makefile.am
include src/server/Makefile.am

if ON_XDG

desktopdir = $(datadir)/applications
desktop_DATA = lib/xtra/icon/mangclient.desktop

endif

EXTRA_DIST += lib/xtra/icon/mangclient.desktop
EXTRA_DIST += \
	lib/xtra/icon/map-16.png lib/xtra/icon/map-32.png \
	lib/xtra/icon/map-64.png lib/xtra/icon/map-128.png
