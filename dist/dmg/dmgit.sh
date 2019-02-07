#!/bin/sh

THISDIR=$(dirname $0)

BASEDIR=${THISDIR}/../../
VERSION=`grep "AC_INIT" ${BASEDIR}/configure.ac | sed -e s/AC_INIT\(mangband,\ //g -e s/,\ team@mangband.org\)//g`
VERSUFF=`grep "#define CLIENT_VERSION_EXTRA" ${BASEDIR}/src/client/c-defines.h | sed -e s/\#define\ CLIENT_VERSION_EXTRA[[:space:]]*//g -e s/[[:space:]]+//g`

if [ $VERSUFF -eq "0" ]; then
    V_FILE=""
    V_DISPLAY=""
    V_CLIENT=""
elif [ $VERSUFF -eq "1" ]; then
    V_FILE="a"
    V_DISPLAY="alpha"
    V_CLIENT="_dev"
elif [ $VERSUFF -eq "2" ]; then
    V_FILE="b"
    V_DISPLAY="beta"
    V_CLIENT="_dev"
else
    V_FILE="dev$VERSUFF"
    V_DISPLAY="devel$VERSUFF"
    V_CLIENT="_dev"
fi

RELEASE_NAME="mangclient-${VERSION}${V_FILE}"
DISPLAY_NAME="MAngband Client ${VERSION} ${V_DISPLAY}"

FINAL_NAME="$RELEASE_NAME-osx-intel.dmg"

if [ "$1" = "--filename" ]; then
    echo "$FINAL_NAME"
    exit
fi

echo "[" $RELEASE_NAME "]" $DISPLAY_NAME

STAGE_DIR=$THISDIR/stage

echo $STAGE_DIR

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"
cp ${THISDIR}/README-client.htm "$STAGE_DIR"/README.html
cp -r ${BASEDIR}/mangclient.app "$STAGE_DIR"/MAngbandClient$V_CLIENT.app
ln -s /Applications "$STAGE_DIR"/.

hdiutil create -fs HFS+ -srcfolder "$STAGE_DIR" \
 -volname "$DISPLAY_NAME" "$FINAL_NAME"
