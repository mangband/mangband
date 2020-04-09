#!/bin/sh

CONFIGURE_FLAGS=$1
if [ "x${LINUX_DEPLOY}" = x"" ]; then
    LINUX_DEPLOY="linuxdeploy"
fi
DIR="dist/appimage/AppDir"

if [ ! -f autogen.sh ]; then
    echo "Please run this script from the top-most directory, like so:"
    echo "$ dist/appimage/appit.sh"
    exit 1
fi

# Grab version
#if [ ! -f dist/tgz/version ]; then
#    dist/tgz/package.sh
#fi
#VERSION=`cat dist/tgz/version`
VERSION=`grep "AC_INIT" configure.ac | sed -e s/AC_INIT\(mangband,\ //g -e s/,\ team@mangband.org\)//g`

# Build (prefix must be "/usr")
rm -rf dist/appimage/AppDir
mkdir dist/appimage/AppDir
./configure --prefix="/usr" ${CONFIGURE_FLAGS}
make clean
make mangclient

# Base install
make install mangclient DESTDIR=${DIR}

# Icons
mkdir -p ${DIR}/usr/share/icons/hicolor/16x16/apps
mkdir -p ${DIR}/usr/share/icons/hicolor/32x32/apps
mkdir -p ${DIR}/usr/share/icons/hicolor/64x64/apps
mkdir -p ${DIR}/usr/share/icons/hicolor/128x128/apps
cp lib/xtra/icon/map-16.png ${DIR}/usr/share/icons/hicolor/16x16/apps/mangclient.png
cp lib/xtra/icon/map-32.png ${DIR}/usr/share/icons/hicolor/32x32/apps/mangclient.png
cp lib/xtra/icon/map-64.png ${DIR}/usr/share/icons/hicolor/64x64/apps/mangclient.png
cp lib/xtra/icon/map-128.png ${DIR}/usr/share/icons/hicolor/128x128/apps/mangclient.png

# Launch script
cp dist/appimage/AppRun ${DIR}/.
chmod +x ${DIR}/AppRun

# Bake AppImage
rm -f MAng*.AppImage*
${LINUX_DEPLOY} --appdir ${DIR} --output appimage
mv MAngband_Client*.AppImage "MAngbandClient-${VERSION}.AppImage"
