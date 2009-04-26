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
    echo "You must first run install/tgz script";
    exit
fi

VER=`cat ../tgz/version`

if [ ! -f ../tgz/mangband-${VER}.tar.gz ]; then
    echo "Unable to find install/tgz/mangband-${VER}.tar.gz"
    exit
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
echo "" >> ./SPECS/mangband.spec
cat ./spec.rest >> ./SPECS/mangband.spec

# Use -bb to build binary package only 
rpmbuild -ba --clean ./SPECS/mangband.spec

# TEST (yum install rpmlint)
#rpmlint SPECS/mangband.spec
#rpmlint RPMS/*

# Display generated rpms
ls -1 -R RPMS/
