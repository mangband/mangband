#!/bin/sh

DIR=$(cd "$(dirname "$0")"; pwd)
${DIR}/mangclient \
 --libdir "${DIR}/../Resources/lib/" \
 --userdir "${HOME}/Library/MAngband" \
 $1 $2 $3 $4 $5 $6 $7 $8 $9
