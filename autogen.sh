#!/bin/sh

# Greet user.
echo "MAngband Autogen/Helper"

# Use getopt(s) to fetch flags.
usage=0; norma=0; devel=0; distr=0; clean=0;
force=0; verbo=0; autoc=0; gitdl=0; dodoc=0;
# you can use either line depending of getopt(s) availiablity
#flags=`getopt hndrcfva $*`; set -- $flags; for flag; do
while getopts  "hndrucfva" flag; do flag="-$flag"
    case "$flag" in
	-\? ) usage=1 ;;
	-h ) usage=1 ;;
	-n ) norma=1 ;;
	-d ) devel=1 ;;
	-r ) distr=1 ;;
	-c ) clean=1 ;;
	-f ) force=1 ;;
	-v ) verbo=1 ;;
	-a ) autoc=1 ;;
	-u ) dodoc=1 ;;
	-- ) break ;;
    esac
done
# Detect .git checkout
if [ -d .git ]; then
echo " * Seeing .git, course-correcting."
#cat README
gitdl=1;
fi
# Test flags for errors
TMODE=$((${norma} + ${devel} + ${distr} + ${clean} + ${dodoc} + ${usage}));
#echo "NDR: $TMODE N: $norma D: $devel R: $distr C: $clean V: $verbo F: $force A: $autoc V: $verbo ?: $usage"


if [ ${TMODE} -eq 0 ]; then
echo " * No mode specified. Try -h flag to find out more!"
norma=1;
	if [ ${gitdl} -eq 1 ]; then
		norma=0;
		echo "--FAILED"
		echo "Please specify one of the -n, -d, -r options explicitly. See -h for details."
		#usage=1;
		exit 1
	fi
fi
if [ ${TMODE} -ge 2 ]; then
echo " -- FAILED"
echo " The -n, -d, -r, -c and -h flags are in conflict. See -h for details:"
usage=1;
fi


# Display help and exit
if [ "$usage" = "1" ]; then
echo "Usage: ./autogen.sh [MODE] [OPTION]"
echo " MODE flag selects the task:"
echo " -n  NORMAL mode. Will run autotools to create a './configure' script."
echo "     (default) "
echo " -d  DEBUG/DEVELOPER mode. Similar to NORMAL, but, several hacks are added"
echo "     to the config file, allowing MAngband to run from 'current' directory."
echo "     NOTE: Using it makes your working copy unsuitable for release !"
echo " -r  RELEASE mode. Prepares the package for distribution."
echo " -c  CLEANING mode. Pretty useless. Deletes excessive autotool files."
echo " -h  HELP mode. Displays this message and exits."
echo " OPTION flags control mode behavior:"
echo " -f  Force download of latest GNU config.* files."
echo " -v  Verbose logging - display additional information."
echo " -a  Automaticly invoke ./configure when it's ready."
exit 0
fi

# Select logger
if [ "$verbo" = "1" ]; then
echo " * Verbose logging enabled"
VLOG="/dev/stdout"
else
VLOG="/dev/null"
fi
# Auto-configure when Releasing
[ "$distr" = "1" ] && autoc=1
# Auto-force when Cleaning
[ "$clean" = "1" ] && force=1


# Show some info
[ "$norma" = "1" ] && echo " * Using NORMAL mode"
[ "$devel" = "1" ] && echo " * Using DEVELOPER mode"
[ "$distr" = "1" ] && echo " * Using RELEASE mode"
[ "$clean" = "1" ] && echo " * Using CLEANING mode"
#echo " * Using $1 mode."


# Delete downloadable files
if [ "$force" = "1" ]; then
    echo " * Removing 'config.sub' and 'config.guess'"
    rm ./config.sub
    rm ./config.guess
fi

# Delete generated files
if [ "$clean" = "1" ]; then
echo " * Removing makefiles"
find . -name "Makefile.in" -exec rm {} \;
find . -name "Makefile" -exec rm {} \;
echo " * Removing various generated files"
rm aclocal.m4
rm src/config.h
rm config.h
rm stamp-h1
rm config.log
rm config.status
rm install-sh
rm missing
rm depcomp
exit 0
fi

HAS_WGET=`which wget`
HAS_CURL=`which curl`
download_file() {
    NAME=$1
    URL=$2
    if [ "$HAS_WGET" != "" ]; then
        wget -O "$NAME" "$URL" >${VLOG} 2>${VLOG}
    elif [ "$HAS_CURL" != "" ]; then
        curl -o "$NAME" "$URL" >${VLOG} 2>${VLOG}
    else
        "No wget, no curl, can't download file $NAME"
        return 1
    fi
    return $?
}
# Update GNU config.* files
[ -f ./config.sub ] || {
    echo " * Downloading 'config.sub' from GNU.org";
    download_file "config.sub" "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD" ||
    echo " -- FAILED"; }
[ -f ./config.guess ] || {
    echo " * Downloading 'config.guess' from GNU.org";
    download_file "config.guess" "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" ||
    echo " -- FAILED"; }

# Update AUTHORS and ChangeLog files
if [ "$dodoc" = "1" ]; then
    echo " * Downloading 'AUTHORS' from mangband.org";
    download_file "AUTHORS" "http://www.mangband.org/docs/people.txt" ||
    echo " -- FAILED";

#    sed -e"s/\[\[h[^']*//g" -e"s/\]\]//g" -e"s/'''//g" AUTHORS.src > AUTHORS && rm AUTHORS.src

    echo " * Downloading 'ChangeLog' from github";
    download_file "ChangeLog" "https://raw.githubusercontent.com/wiki/mangband/mangband/ReleaseChangeLog.md" ||
    echo " -- FAILED";
fi


# The real purpose of autogen:
echo " * Running 'autoreconf -i'"
autoreconf -i >${VLOG} 2>${VLOG} || { echo " -- FAILED"; exit 1; }

# Prepare for debug
if [ "$devel" = "1" ]; then
echo " * Preparing debug environment"
CFG="./mangband.cfg"
TMP="./$(basename $0).$$.tmp"
sed "/*\{0,3\}\(BONE\|EDIT\|DATA\|SAVE\|USER\|HELP\|PREF\)_DIR = /d" ${CFG} > ${TMP}
echo "EDIT_DIR = \"./lib/edit\"" >> ${TMP}
echo "HELP_DIR = \"./lib/help\"" >> ${TMP}
echo "PREF_DIR = \"./lib/pref\"" >> ${TMP}
echo "DATA_DIR = \"./lib/data\"" >> ${TMP}
echo "SAVE_DIR = \"./lib/save\"" >> ${TMP}
echo "BONE_DIR = \"./lib/user\"" >> ${TMP}
mv ${TMP} ${CFG}
CFG="${HOME}/.mangrc"
TMP="./$(basename $0).$$.tmp"
sed "/\(LibDir \|\[MAngband\]\)/d" ${CFG} > ${TMP}
echo "[MAngband]" > ${HOME}/.mangrc
echo "LibDir ./lib/" >> ${HOME}/.mangrc
cat ${TMP} >> ${HOME}/.mangrc
fi

# Auto-run configure
if [ "$autoc" = "1" ]; then
    echo " * Running './configure'"
    ./configure >${VLOG} 2>${VLOG} || { echo " -- FAILED"; exit 1; }
fi

# Prepare release
if [ "$distr" = "1" ]; then
echo " * Running 'make dist'"
make dist >${VLOG} 2>${VLOG} || { echo " -- FAILED"; exit 1; }
echo -n " * Ready! "
ls *.tar.gz
exit 0
fi

# DONE
if [ "$autoc" = "1" ]; then
    echo " * Ready!"
else
    echo " * Ready! You can use './configure' now!"
fi
