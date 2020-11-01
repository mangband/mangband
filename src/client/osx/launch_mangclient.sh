#!/bin/bash

DIR=$(cd "$(dirname "$0")"; pwd)
APPDIR=$(cd "${DIR}/../../."; pwd)
APPBASENAME=$(basename "${APPDIR}")
SHORTVERSION=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersion" "${DIR}/../Info.plist")

#echo "DIR: ${DIR}"
#echo "SHORTVER: ${SHORTVERSION}"

if [[ "${DIR}" =~ ^/Applications/.* ]]; then
	:
else
	CONFIRM_MOVE=$(osascript -e 'tell app "System Events" to display dialog "Move MAngband client to Applications folder?" buttons {"Yes", "No"} with title "MAngband Client" default button "Yes"')
	if [[ "${CONFIRM_MOVE}" == "button returned:Yes" ]]; then
		cp -r "${APPDIR}" "/Applications/${APPBASENAME}"
		open "/Applications/${APPBASENAME}"
		exit 0
	fi
fi

LIBDIR="${DIR}/../Resources/lib/"
USERDIR="${HOME}/Library/Application Support/MAngband${SHORTVERSION}/"
mkdir -p "${USERDIR}/user"
mkdir -p "${USERDIR}/bone"
touch "${USERDIR}/mangclient.ini"
cp -n "${LIBDIR}/user/"* "${USERDIR}/user"
"${DIR}/mangclient" \
 --libdir "${LIBDIR}" \
 --userdir "${USERDIR}" \
 --config "${USERDIR}/mangclient.ini" \
 $1 $2 $3 $4 $5 $6 $7 $8 $9
