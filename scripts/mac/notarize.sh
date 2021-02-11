#!/bin/bash

XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"

#if [[ $# -gt 0 ]]; then
#  PASSWORD=$1
#else
#  read -p "Apple developer password: " PASSWORD
#fi

$XCRUN $ALTOOL --notarize-app \
               --primary-bundle-id "org.satscan.gui.SaTScanApplication.dmg" \
               --username "meagherk@imsweb.com" \
               --file SaTScan.dmg

#$XCRUN $ALTOOL --notarize-app \
#               --primary-bundle-id "org.satscan.gui.SaTScanApplication.dmg" \
#               --username "meagherk@imsweb.com" \
#               --password "${PASSWORD}" \
#               --file SaTScan.dmg
