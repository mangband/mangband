#!/bin/sh

THISDIR=$(dirname $0)
BASEDIR=${THISDIR}/../../

CLIENT=$($THISDIR/dmgit.sh --filename)
SERVER=$($THISDIR/dmgserv.sh --filename)

cat <<"HTML"
<html>
<title>MAngband OSX builds</title>
<head><meta charset="utf-8"/></head>
<body>
HTML
echo "<p><a href='${CLIENT}'>$CLIENT</a></p>"
echo "<p><a href='${SERVER}'>$SERVER</a></p>"
echo Built on $(date)
cat <<"HTML"
</body>
</html>
HTML
