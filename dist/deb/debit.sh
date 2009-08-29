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

rm -rf mangband-${VER}
cp ../tgz/mangband-${VER}.tar.gz ./mangband_${VER}.orig.tar.gz
tar -xzvf mangband_${VER}.orig.tar.gz
cd mangband-${VER}
cp -R src/* .
dh_make -m -n --email team@mangband.org
cd debian
rm *.ex *.EX

cp -f ../../control .
cp -f ../../rules .
chmod +x rules
echo "#!/bin/sh" >./postinst
echo chmod og+w -R /var/games/mangband >>./postinst
echo "var/games/" >>./dirs
echo "etc/" >>./dirs
echo Version: ${VER} >>./control
echo "/etc/mangband.cfg" >./conffiles
cd ..
dpkg-buildpackage -rfakeroot
#debuild -S -us -uc
#pbuilder build ../*.dsc

#Build binary from src
cd ..
dpkg-source -x *.dsc
cd mangband-${VER}
chmod +x ./configure
dpkg-buildpackage -rfakeroot -b
