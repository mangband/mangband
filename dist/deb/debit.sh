#!/bin/sh

which dh_make
if [ $? != 0 ]; then
    echo "dh_make not found. apt install dh-make"
    exit
fi

if [ ! -f ../tgz/version ]; then
    echo "You must first run dist/tgz script";
    exit
fi

VER=`cat ../tgz/version`

if [ ! -f ../tgz/mangband-${VER}.tar.gz ]; then
    echo "Unable to find dist/tgz/mangband-${VER}.tar.gz"
    exit
fi

# Note: all ports include ncurses!
PACKNAME="mangclient"
if [ "$1" = "sdl2" ]; then
	PORT="-sdl2"
	RECONFIG="--without-x11 --without-sdl"
	MANG_PACKAGE_DEPENDS="libsdl2-2.0-0, libsdl2-ttf-2.0-0"
elif [ "$1" = "sdl" ]; then
	PORT="-sdl"
	RECONFIG="--without-x11 --without-sdl2"
	MANG_PACKAGE_DEPENDS="libsdl1.2debian, libsdl-ttf2.0-0"
elif [ "$1" = "x11" ]; then
	PORT="-x11"
	RECONFIG="--without-sdl --without-sdl2"
	MANG_PACKAGE_DEPENDS="libx11-6"
elif [ "$1" = "gcu" ]; then
	PORT="-gcu"
	RECONFIG="--without-x11 --without-sdl --without-sdl2"
	MANG_PACKAGE_DEPENDS="libncurses5"
else
	PORT=""
	RECONFIG=""
	PACKNAME="mangband"
	MANG_PACKAGE_DEPENDS=""
	echo "Building server package."
	echo "Re-run this script with 'sdl2','sdl','x11' or 'gcu' to build client."
	sleep 1
fi


ARCH=`dpkg-architecture -qDEB_TARGET_ARCH 2>/dev/null || dpkg-architecture -qDEB_BUILD_ARCH`
INTERNAME="${PACKNAME}_${VER}_${ARCH}.deb"

FINALNAME="${PACKNAME}${PORT}-${VER}_${ARCH}.deb"
export ADDITIONAL_CONFIGURE_FLAGS="${RECONFIG}"
export ACTUAL_PACKAGE_NAME="${PACKNAME}"
echo "$INTERNAME ( $RECONFIG ) ->\n $FINALNAME"

MODERN_DH=`dh_make --help | grep -e "--copyrightfile"`
MODERN_DPB=`dpkg-buildpackage --help | grep -e "--no-sign"`
if [ "${MODERN_DH}" != "" ]; then
	DH_ARGS="-C i -c custom --copyrightfile ../COPYING"
else
	DH_ARGS="-C i -c bsd"
fi
if [ "${MODERN_DPB}" != "" ]; then
	DPB_ARGS="--no-sign"
else
	DPB_ARGS=""
fi

rm -rf mangband-${VER}
cp ../tgz/mangband-${VER}.tar.gz ./mangband_${VER}.orig.tar.gz
tar -xzvf mangband_${VER}.orig.tar.gz
cd mangband-${VER}
cp -R src/* .
export DEBFULLNAME="MAngband Project Team"
dh_make -y -n --email team@mangband.org ${DH_ARGS}
if [ $? != "0" ]; then
    exit
fi

cd debian
rm *.ex *.EX
rm README.Debian
rm README.source

cp -f ../../../../README .
export MANG_PACKAGE_DEPENDS
envsubst < ../../control.${PACKNAME} > control
cp -f ../../rules .
chmod +x rules
echo "README" > ${PACKNAME}-docs.docs
echo "#!/bin/sh" >./postinst
echo chmod og+w -R /usr/share/games/mangband >>./postinst
echo "/usr/share/games/" >>./dirs
if [ "${PACKNAME}" = "mangband" ]; then
echo "/var/local/" >> ./dirs
echo "etc/" >>./dirs
fi
cd ..
dpkg-buildpackage -rfakeroot ${DPB_ARGS}
if [ $? != "0" ]; then
    exit
fi
#debuild -S -us -uc
#pbuilder build ../*.dsc


#Build binary from src
cd ..
rm -rf mangband-${VER}
dpkg-source -x *.dsc
cd mangband-${VER}
chmod +x ./configure
dpkg-buildpackage -rfakeroot -b ${DPB_ARGS}
if [ $? != "0" ]; then
    exit
fi

#Rename final deb into something more appropriate
if [ "${INTERNAME}" != "${FINALNAME}" ]; then
	cd ..
	mv "${INTERNAME}" "${FINALNAME}"
	# also rename the debug package
	DEBUG_INTERNAME="${PACKNAME}-dbgsym_${VER}_${ARCH}.deb"
	DEBUG_FINALNAME="${PACKNAME}${PORT}-dbgsym-${VER}_${ARCH}.deb"
	mv "${DEBUG_INTERNAME}" "${DEBUG_FINALNAME}" || :
fi
