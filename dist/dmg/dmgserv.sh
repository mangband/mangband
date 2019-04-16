#!/bin/sh

THISDIR=$(cd $(dirname $0); pwd )

BASEDIR="${THISDIR}/../../"
VERSION=`grep "AC_INIT" ${BASEDIR}/configure.ac | sed -e s/AC_INIT\(mangband,\ //g -e s/,\ team@mangband.org\)//g`
VERSUFF=`grep "#define CLIENT_VERSION_EXTRA" ${BASEDIR}/src/client/c-defines.h | sed -e s/\#define\ CLIENT_VERSION_EXTRA[[:space:]]*//g -e s/[[:space:]]+//g`

if [ $VERSUFF -eq "0" ]; then
    V_FILE="" ; V_DISPLAY="" ; V_SERVER=""
elif [ $VERSUFF -eq "1" ]; then
    V_FILE="a" ; V_DISPLAY="alpha" ; V_SERVER="_dev"
elif [ $VERSUFF -eq "2" ]; then
    V_FILE="b" ; V_DISPLAY="beta" ; V_SERVER="_dev"
else
    V_FILE="dev$VERSUFF" ; V_DISPLAY="devel$VERSUFF" ; V_SERVER="_dev"
fi

RELEASE_NAME="mangband-${VERSION}${V_FILE}"
DISPLAY_NAME="MAngband Server ${VERSION} ${V_DISPLAY}"

FINAL_NAME="$RELEASE_NAME-osx-intel.dmg"

if [ "$1" = "--filename" ]; then
    echo "$FINAL_NAME"
    exit
fi

echo "[" $RELEASE_NAME "]" $DISPLAY_NAME

STAGE_DIR=$THISDIR/stage
PACKDIR="${STAGE_DIR}/MAngbandServer${VERSION}${V_SERVER}"

rm -rf "${STAGE_DIR}"
mkdir -p "${PACKDIR}"
cp "${THISDIR}/README-server.htm" "${STAGE_DIR}/README.html"
cp -r "${BASEDIR}/lib" "${PACKDIR}/lib"
find "${PACKDIR}/lib" -type f -name 'Makefile.am' -delete
find "${PACKDIR}/lib" -type f -name 'delete.me' -delete
cp "${BASEDIR}"/mangband "${PACKDIR}"/.
cp "${THISDIR}"/run-server.command "${PACKDIR}"/.

CFG="${BASEDIR}/mangband.cfg"
TMP="${PACKDIR}/mangband.cfg"
#sed "/\*\{0,3\}\(BONE\|EDIT\|DATA\|SAVE\|USER\|HELP\|PREF\)_DIR = /d" ${CFG} > ${TMP}
cp $CFG ${TMP}
echo "EDIT_DIR = \"./lib/edit\"" >> ${TMP}
echo "HELP_DIR = \"./lib/help\"" >> ${TMP}
echo "PREF_DIR = \"./lib/pref\"" >> ${TMP}
echo "DATA_DIR = \"./lib/data\"" >> ${TMP}
echo "SAVE_DIR = \"./lib/save\"" >> ${TMP}
echo "BONE_DIR = \"./lib/user\"" >> ${TMP}

hdiutil create -fs HFS+ -srcfolder "${STAGE_DIR}" \
 -volname "$DISPLAY_NAME" "$FINAL_NAME"
