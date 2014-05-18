#!/bin/sh

if [ ! -f ../tgz/version ]; then
    echo "You must first run install/tgz script";
    exit
fi

VER=`cat ../tgz/version`

if [ ! -f ../tgz/mangband-${VER}.tar.gz ]; then
    echo "Unable to find install/tgz/mangband-${VER}.tar.gz"
    exit
fi

rm -rf work
rm -rf distfiles
mkdir work
mkdir distfiles
cp ../tgz/mangband-${VER}.tar.gz ./distfiles/mangband-${VER}.tar.gz

echo "Preparing makefile"

echo "# Created by:" `whoami` >./Makefile
echo "# \$FreeBSD\$" >>./Makefile
echo "" >>./Makefile

echo "PORTNAME=	mangband" >>./Makefile
echo "PORTVERSION=	$VER" >>./Makefile
#echo "DISTNAME=	mangband-$VER" >>./Makefile
cat Makefile.rest >>./Makefile

echo "Generating checksum"
make makesum DISTDIR=${PWD}

echo "Generating PR and DIFF"
rm -rf mangband
mkdir mangband
cp ./pkg-* ./mangband
mv Makefile mangband/
mv distinfo mangband/
cp -r files mangband/
shar `find ./mangband` > mangband-${VER}-PR
diff -ruN /usr/ports/games/mangband ./mangband > mangband-${VER}-DIFF
