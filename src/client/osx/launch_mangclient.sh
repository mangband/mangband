#!/bin/sh

DIR=$(cd "$(dirname "$0")"; pwd)
LIBDIR="${DIR}/../Resources/lib/"
USERDIR="${HOME}/Library/Application Support/MAngband1.2/"
mkdir -p "${USERDIR}/user"
cp -n "${LIBDIR}/user/"* "${USERDIR}/user"
${DIR}/mangclient \
 --libdir "$LIBDIR" \
 --userdir "${USERDIR}" \
 $1 $2 $3 $4 $5 $6 $7 $8 $9
