#!/bin/sh

# Check if environment is ready
if [ ! -f ~/.rpmmacros ]; then
	USER=`whoami`
	FULL=`grep "^${USER}:" /etc/passwd | cut -d: -f5 | cut -d, -f1`
	echo "# Can't find \$HOME/.rpmmacros file"
	echo "# Suggested file:   (HINT! 'yum install rpmdevtools')"
	echo "%packager	${FULL} <email@address>"
	echo "%vendor  	MAngabnd Project Team"
	echo "%_topdir 	`pwd`"
	exit
fi 

if [ ! -f ../tgz/version ]; then
    echo "You must first run dist/tgz script";
    exit
fi

VER=`cat ../tgz/version`

if [ ! -f ../tgz/mangband-${VER}.tar.gz ]; then
    echo "Unable to find install/tgz/mangband-${VER}.tar.gz"
    exit
fi

CONFIGURE_FLAGS=""
SUBPACKAGE_NAME=""
if [ "$1" = "sdl2" ]; then
    CONFIGURE_FLAGS="--with-sdl2 --without-gcu --without-x11 --without-sdl"
    SUBPACKAGE_NAME="mangclient-sdl2"
elif [ "$1" = "sdl" ]; then
    CONFIGURE_FLAGS="--with-sdl --without-gcu --without-x11 --without-sdl2"
    SUBPACKAGE_NAME="mangclient-sdl"
elif [ "$1" = "x11" ]; then
    CONFIGURE_FLAGS="--with-x11 --without-gcu --without-sdl --without-sdl2"
    SUBPACKAGE_NAME="mangclient-x11"
elif [ "$1" = "gcu" ]; then
    CONFIGURE_FLAGS="--with-gcu --without-x11 --without-sdl --without-sdl2"
    SUBPACKAGE_NAME="mangclient-gcu"
else
    CONFIGURE_FLAGS="--without-gcu" # placeholder, rpm macros can't be empty
    SUBPACKAGE_NAME="mangband"
    echo "Building server package."
    echo "Re-run this script with 'sdl2','sdl','x11' or 'gcu' to build client."
    sleep 1
fi

rm -rf ./BUILD
rm -rf ./RPMS
rm -rf ./SOURCES
rm -rf ./SPECS
rm -rf ./SRPMS

mkdir ./BUILD
mkdir ./RPMS
mkdir ./SOURCES
mkdir ./SPECS
mkdir ./SRPMS

rm -rf ./SOURCES/mangband-${VER}
#cp ../ ./mangband_${VER}.tar.gz
cp ../tgz/mangband-${VER}.tar.gz ./SOURCES/mangband-${VER}.tar.gz

echo "%define name    mangband" > ./SPECS/mangband.spec
echo "%define version ${VER}" >>  ./SPECS/mangband.spec
echo "%define release 1" >>  ./SPECS/mangband.spec
echo "%define subpackage_name ${SUBPACKAGE_NAME}" >>  ./SPECS/mangband.spec
echo "%define configure_flags ${CONFIGURE_FLAGS}" >> ./SPECS/mangband.spec
echo "" >> ./SPECS/mangband.spec
cat ./spec.rest >> ./SPECS/mangband.spec

# Use -bb to build binary package only 
rpmbuild -ba --clean ./SPECS/mangband.spec

# TEST (yum install rpmlint)
#rpmlint SPECS/mangband.spec
#rpmlint RPMS/*

# Copy single useful rpm
cp RPMS/**/${SUBPACKAGE_NAME}*.rpm .
ls -1 *.rpm

# Display generated rpms
#ls -1 -R RPMS/
