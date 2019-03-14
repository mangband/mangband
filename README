BUILDING
--------

 mangband and mangclient use a generic autotools build process.
 See the INSTALL file for more information.

 Basically, just run `./configure; make; make install`.

 If you're building from the git repository, you'll have to generate
 the `./configure` script first, by running `./autogen.sh`.

 Please note, that mangclient can be compiled with several
 different display frontends, which may or may not conflict
 with each other. In addition, we support some weird configurations
 and build options. Please run `./configure --help` to see
 the full list of available features.

 For additional hints regarding your specific platform (Windows,
 OSX), please see the developer Wiki:

	https://github.com/mangband/mangband/wiki

RUNNING
-------

 Both mangband and mangclient binaries rely on "lib dir" - a directory
 with all the game data.

 First, they'll look for the ANGBAND_PATH environment variable,
 then they'll try the PKGDATADIR constant, then the directories
 specified in config files (.mangrc and mangband.cfg).

 ANGBAND_PATH can be used to select any directory in this manner:

	ANGBAND_PATH=./lib ./mangband
	ANGBAND_PATH=./lib ./mangclient

 Config file directives are "LibDir" in "$HOME/.mangrc" (for client)
 and "EDIT_DIR", "DATA_DIR", "SAVE_DIR", "BONE_DIR", "HELP_DIR" in
 "mangband.cfg" (for server). Those overwrite any other paths.

 PKGDATADIR is set during the `./configure` step, and defaults to
 something system-specific like "/usr/local/games/mangband". To
 adjust it, the --datadir option can be used:

	./configure --datadir=$PWD/lib
