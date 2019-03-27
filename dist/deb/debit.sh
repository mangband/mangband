#!/bin/sh

which dh_make
if [ $? != 0 ]; then
    echo "dh_make not found. apt install dh-make"
    exit
fi

if [ ! -f ../tgz/version ]; then
    echo "You must first run install/tgz script";
    exit
fi

VER=`cat ../tgz/version`

if [ ! -f ../tgz/mangband-${VER}.tar.gz ]; then
    echo "Unable to find install/tgz/mangband-${VER}.tar.gz"
    exit
fi

ARCH=`dpkg-architecture -qDEB_TARGET_ARCH`
PORT=""
INTERNAME="mangband_${VER}_${ARCH}.deb"
RECONFIG=""

# Note: all ports include ncurses!
if [ "$1" = "sdl2" ]; then
	PORT="-sdl2"
	RECONFIG="--without-x11 --without-sdl"
elif [ "$1" = "sdl" ]; then
	PORT="-sdl"
	RECONFIG="--without-x11 --without-sdl2"
elif [ "$1" = "x11" ]; then
	PORT="-x11"
	RECONFIG="--without-sdl --without-sdl2"
elif [ "$1" = "gcu" ]; then
	PORT="-gcu"
	RECONFIG="--without-x11 --without-sdl --without-sdl2"
else
	echo "Building multi-port binary. This is probably not what you want!"
	echo "But it's a good starting point."
	echo "After you get it, re-run this script with 'sdl','x11' or 'gcu' arg."
	sleep 1
fi

FINALNAME="mangband${PORT}-${VER}_${ARCH}.deb"
export ADDITIONAL_CONFIGURE_FLAGS="${RECONFIG}"
echo "$INTERNAME ( $RECONFIG ) ->\n $FINALNAME"


rm -rf mangband-${VER}
cp ../tgz/mangband-${VER}.tar.gz ./mangband_${VER}.orig.tar.gz
tar -xzvf mangband_${VER}.orig.tar.gz
cd mangband-${VER}
cp -R src/* .
export DEBFULLNAME="MAngband Project Team"
dh_make -y -Ci -n --email team@mangband.org -c custom --copyrightfile ../COPYING
if [ $? != "0" ]; then
    exit
fi

cd debian
rm *.ex *.EX
rm README.Debian
rm README.source

cp -f ../../../../README .
cp -f ../../control .
cp -f ../../rules .
chmod +x rules
echo "README" > mangband-docs.docs
echo "#!/bin/sh" >./postinst
echo chmod og+w -R /usr/share/games/mangband >>./postinst
echo "/usr/share/games/" >>./dirs
echo "/var/local/" >> ./dirs
echo "etc/" >>./dirs
#echo Version: ${VER} >>./control
#echo "/etc/mangband.cfg" >./conffiles
cd ..
dpkg-buildpackage -rfakeroot --no-sign
if [ $? != "0" ]; then
    exit
fi
#debuild -S -us -uc
#pbuilder build ../*.dsc


#Build binary from src
cd ..
dpkg-source -x *.dsc
cd mangband-${VER}
chmod +x ./configure
dpkg-buildpackage -rfakeroot -b --no-sign
if [ $? != "0" ]; then
    exit
fi

#Rename final deb into something more appropriate
if [ "${ITERNAME}" != "${FINALNAME}" ]; then
	cd ..
	cp "${INTERNAME}" "${FINALNAME}"
fi
