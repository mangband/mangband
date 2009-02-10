#!/bin/sh
# (2009) MAngband Project Team

echo "This script will update files needed for autoconf from it's CVS"
echo "This might seem strange to you, but is an appropiriate procedure"
echo "until next autotools version. (See automake)"
echo ""
echo "You should only run this script before doing a major release."
echo ""
echo "[Press any key to continue, Ctrl+C to cancel]"

read TMP_PAUSE

wget -O config.sub "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"
wget -O config.guess "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"

mv config.sub ../../src/config.sub
mv config.guess ../../src/config.guess
