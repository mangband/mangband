#!/bin/sh

THISDIR=$(dirname $0)

BASEDIR=${THISDIR}/../../
VERSION=`grep "AC_INIT" ${BASEDIR}/configure.ac | sed -e s/AC_INIT\(mangband,\ //g -e s/,\ team@mangband.org\)//g`

# When building a devel/alpha/beta/etc versions,
# please use those:
V_FILE="a" # 'a'
V_DISPLAY="alpha" # 'alpha'
V_CLIENT="_dev" # '_dev'

RELEASE_NAME="mangclient-${VERSION}${V_FILE}"
DISPLAY_NAME="MAngband Client ${VERSION} ${V_DISPLAY}"

echo "[" $RELEASE_NAME "]" $DISPLAY_NAME

STAGE_DIR=$THISDIR/stage

echo $STAGE_DIR

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"
cp ${THISDIR}/README-client.htm "$STAGE_DIR"/README.html
cp -r ${BASEDIR}/mangclient.app "$STAGE_DIR"/MAngbandClient$V_CLIENT.app
ln -s /Applications "$STAGE_DIR"/.

hdiutil create -fs HFS+ -srcfolder "$STAGE_DIR" \
 -volname "$DISPLAY_NAME" "$RELEASE_NAME-osx-intel.dmg"
