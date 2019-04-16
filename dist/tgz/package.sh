#!/bin/sh

BASEDIR=../..

VERSION=`grep "AC_INIT" ${BASEDIR}/configure.ac | sed -e s/AC_INIT\(mangband,\ //g -e s/,\ team@mangband.org\)//g`

echo -n ${VERSION} > ./version

cd ${BASEDIR} ; make dist ; cp mangband-${VERSION}.tar.gz dist/tgz/. ; cd -

echo "MAngband ${VERSION} > mangband-${VERSION}.tar.gz"
