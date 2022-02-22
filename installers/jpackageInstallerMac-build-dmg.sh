#!/bin/bash

# https://pwvault.imsweb.com/SecretServer/app/#/secret/25934/general
if [[ $# -gt 0 ]]; then
  PASSWORD=$1
else
  read -p "Apple developer password (SaTScan Mac App Specific): " PASSWORD
fi

APPVERSION="10.1"
SRCDIR="/Users/satscsvc/prj/satscan.development/satscan"
INSTALLER_DIR="/prj/satscan/installers/v.${APPVERSION}.x"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
BUNDLEDIR="/Users/satscsvc/prj/satscan.development/jpackaged"
BINARIES="/Users/satscsvc/prj/satscan.development/binaries/mac"
JAVAJDK="/Users/satscsvc/prj/java/jdk-17.0.2+8/Contents/Home" # AdoptJDK
ENTITLEMENTS="${SRCDIR}/installers/macosentitlements.plist"
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"

rm -rf $BUNDLEDIR/bin

# Create dmg with notarized application - but codesign separately.
$JAVAJDK/bin/jpackage --type pkg  --verbose --app-image $BUNDLEDIR/SaTScan.app --app-version $APPVERSION \
                      --name SaTScan --dest $BUNDLEDIR/bin --description "Software for the spatial, temporal, and space-time scan statistics" \
					  --vendor "Information Management Services, Inc." --copyright "Copyright 2005, All rights reserved" \
					  --resource-dir $BUNDLEDIR/dmgresources --type dmg
Echo How did the dmg build go?
read APPLES_TEST

# codesign and check SaTScan.dmg
codesign --entitlements  ${ENTITLEMENTS} --options runtime -vvvv --deep --timestamp -s "${SIGN_KEY}" $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# Notorize SaTScan.dmg
REQUEST_UUID_DMG=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.satscan" --username "meagherk@imsweb.com" --password "${PASSWORD}" --asc-provider "VF82MCMA83" --file $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg | grep RequestUUID | awk '{print $3}')

# Poll for verification completion.
while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
    echo "Verification in progress..."
    sleep 60
done

echo Results of notarization
$XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}"

Echo Any problems notarizing dmg file?
read APPLES_TEST

# staple application -- assumes notarization succeeds.
$XCRUN $STAPLER staple $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# test notarized
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# Build batch binaries archive for Mac OS X.
rm -f $BUNDLEDIR/satscan.${APPVERSION}_mac.tar.bz2
cd $BINARIES
tar -cf $BUNDLEDIR/satscan.${APPVERSION}_mac.tar satscan
cd $SRCDIR/installers
tar -rf $BUNDLEDIR/satscan.${APPVERSION}_mac.tar documents/*
tar -rf $BUNDLEDIR/satscan.${APPVERSION}_mac.tar sample_data/*
bzip2 -f $BUNDLEDIR/satscan.${APPVERSION}_mac.tar

# push over to fileshare installers directory
echo Copying dmg to fileshare
mv $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg $BUNDLEDIR/bin/SaTScan_${APPVERSION}_mac.dmg
mv $BUNDLEDIR/SaTScan.zip $BUNDLEDIR/SaTScan_${APPVERSION}_mac.zip
scp -r $BUNDLEDIR/bin/SaTScan_${APPVERSION}_mac.dmg $BUNDLEDIR/SaTScan_${APPVERSION}_mac.zip $BUNDLEDIR/satscan.${APPVERSION}_mac.tar.bz2 satscsvc@gen-btp-01.imsweb.com:${INSTALLER_DIR}

