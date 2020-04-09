#!/bin/bash

# This script does the massive build. Scroll down to see
# "build_it" invocations, which determine the actual build targets.
# The only argument to this script is optional "--flat", to force
# everything into a single directory. (Useful for github releases)
# Note: this can take hours!

## NON-DOCKER PATH
running_in_docker() {
    (awk -F/ '$2 == "docker"' /proc/self/cgroup | read non_empty_input)
}
build_it() {
    IMAGE=$1 ; BUILD_MODE=$2 ; TARGET_DIR=$3 ; GLOB=$4 ; FLATMODE=$5
    echo "Run docker with image {$IMAGE} to generate ${GLOB} files"
    rm -rf builds
    mkdir -p builds
    docker pull ${IMAGE}
    docker run -v`pwd`/../../:/io -w /io ${IMAGE} /io/dist/docker/build.sh ${BUILD_MODE}
    if [ "$?" = "0" ]; then
        if [ "$FLATMODE" = "--flat" ]; then
            # copy all made files into a SINGLE dir, but with a prefix
            find ./builds -name "${GLOB}" | while read file
            do
                FNAME=$(basename $file)
                FNAME=${FNAME/-1.x86/.x86} # remove "-1" release tag
                FNAME=${FNAME/-1.amd/.amd} # hacky, but should work
                DP=(${TARGET_DIR//-/ })
                echo " * ${FNAME} ${DP[0]} ( ${DP[1]} ) "
                cp $file flat/${DP[0]}_$FNAME
            done
        else
            # copy all made files into target dir
            mkdir -p ${TARGET_DIR}
            cp builds/${GLOB} ${TARGET_DIR}/.
        fi
    fi
}
if ! running_in_docker ; then
    if [ "$1" = "--flat" ]; then
        rm -rf flat
        mkdir -p flat
    fi

    # make dist once
    echo "Running initial make dist"
    cd ../..
    ./autogen.sh -n
    ./configure
    cd dist/tgz
    ./package.sh
    cd ../../dist/docker

    ################# ******************** #################

    build_it "centos:6" "APPIMAGE:amd64" "appimage-amd64" "*.AppImage" $1
# centos6 is broken by default, https://bugzilla.redhat.com/show_bug.cgi?id=1213602
# disable overlayfs or use the touch hack to enable it
#    build_it "i386/centos:6" "APPIMAGE:i386" "appimage-i386" "*.AppImage" $1
# to give you something, that works out of the box, here's centos7
    build_it "i386/centos:7" "APPIMAGE:i386" "appimage-i386" "*.AppImage" $1

    build_it "debian:stretch" "DEB" "debian9-amd64" "*.deb" $1
    build_it "i386/debian:stretch" "DEB" "debian9-i386" "*.deb" $1
    build_it "debian:jessie" "DEB" "debian8-amd64" "*.deb" $1
    build_it "i386/debian:jessie" "DEB" "debian8-i386" "*.deb" $1

    build_it "ubuntu:bionic" "DEB" "ubuntu18-amd64" "*.deb" $1
    build_it "i386/ubuntu:bionic" "DEB" "ubuntu18-i386" "*.deb" $1
    build_it "ubuntu:disco" "DEB" "ubuntu19-amd64" "*.deb" $1
    build_it "i386/ubuntu:disco" "DEB" "ubuntu19-i386" "*.deb" $1
    build_it "ubuntu:trusty" "DEB" "ubuntu14-amd64" "*.deb" $1
    build_it "i386/ubuntu:trusty" "DEB" "ubuntu14-i386" "*.deb" $1
    build_it "ubuntu:xenial" "DEB" "ubuntu16-amd64" "*.deb" $1
    build_it "i386/ubuntu:xenial" "DEB" "ubuntu16-i386" "*.deb" $1

    build_it "centos:7" "RPM" "centos7-amd64" "*.rpm" $1
    build_it "i386/centos:7" "RPM" "centos7-i386" "*.rpm" $1
    build_it "centos:6" "RPM" "centos6-amd64" "*.rpm" $1
# broken, for some reason
#    build_it "i386/centos:6" "RPM" "centos6-i386" "*.rpm" $1

# no i386 versions for fedora? :(
    build_it "fedora:26" "RPM" "fedora26-amd64" "*.rpm" $1
    build_it "fedora:27" "RPM" "fedora27-amd64" "*.rpm" $1
    build_it "fedora:28" "RPM" "fedora28-amd64" "*.rpm" $1
    build_it "fedora:29" "RPM" "fedora29-amd64" "*.rpm" $1
    build_it "fedora:30" "RPM" "fedora30-amd64" "*.rpm" $1

    ################# ******************** #################
    exit
fi

## DOCKER PATH START IS HERE
set -e -x

if [ $1 = "DEB" ]; then
	printf "\n *** Doing debian builds *** \n\n"
	apt-get update -qq
	apt-get install -yqq make gcc autoconf automake dh-make >/dev/null
	apt-get install -yqq dpkg-dev # needed on Ubuntu14 >/dev/null
	apt-get install -yqq libncurses5-dev libsdl1.2-dev libsdl2-dev >/dev/null
	apt-get install -yqq libsdl-ttf2.0-dev libsdl2-ttf-dev >/dev/null
	apt-get install -yqq libX11-dev >/dev/null || \
		apt-get install -y libx11-dev >/dev/null
# make dist
#	ls
#	./autogen.sh -n
#	./configure
#	cd dist/tgz
#	./package.sh
#	cd -
	cd dist/deb
	rm -f *.deb
	export USER=mangband
	./debit.sh # server
	./debit.sh gcu
	./debit.sh sdl
	./debit.sh sdl2
	./debit.sh x11
	cd -
	cp dist/deb/*.deb dist/docker/builds/.
	rm dist/docker/builds/*-dbgsym*.deb || : # throw away debug
fi

if [ $1 = "RPM" ]; then
	printf "\n *** Doing rpm builds *** \n\n"
	yum install -yq wget which make gcc automake autoconf rpmdevtools
	yum install -yq ncurses-devel libX11-devel SDL-devel SDL2-devel
	yum install -yq SDL_ttf-devel SDL2_ttf-devel
# make dist
#	ls
#	./autogen.sh -n
#	./configure
#	cd dist/tgz
#	./package.sh
#	cd -
	cd dist/rpm
	rm -f *.rpm
	echo "%packager MAngband Project Team <team@mangband.org>" >> /root/.rpmmacros
	echo "%vendor   MAngabnd Project Team" >> /root/.rpmmacros
	echo "%_topdir  /io/dist/rpm" >> /root/.rpmmacros
	./rpmit.sh # server
	./rpmit.sh gcu
	./rpmit.sh sdl
	./rpmit.sh sdl2
	./rpmit.sh x11
	cd -
	cp dist/rpm/*.rpm dist/docker/builds/.
	#cp dist/rpm/SRPMS/**/.rpm dist/docker/builds/.
fi

if [ "$1" = "APPIMAGE:i386" ] || [ "$1" = "APPIMAGE:amd64" ]; then
	if [ "$1" = "APPIMAGE:i386" ]; then
		ARCH="i386"
	else
		ARCH="x86_64"
	fi
	printf "\n *** Doing appimage builds *** \n\n"
	#https://bugzilla.redhat.com/show_bug.cgi?id=1213602
	#touch /var/lib/rpm/*
	yum install -yq wget which make gcc automake autoconf file
	yum install -yq ncurses-devel X11-devel SDL-devel SDL2-devel
	yum install -yq SDL_ttf-devel SDL2_ttf-devel || true
	rm -f *.AppImage
	wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage
	chmod +x ./*.AppImage
	export LINUX_DEPLOY="$(pwd)/linuxdeploy-${ARCH}.AppImage --appimage-extract-and-run"
	dist/appimage/appit.sh --without-x11 --with-gcu --with-sdl
	cp MAng*.AppImage dist/docker/builds/.
fi
