#!/bin/bash

# https://pwvault.imsweb.com/SecretServer/app/#/secret/25934/general
if [[ $# -gt 0 ]]; then
  PASSWORD=$1
else
  read -p "Apple developer password (SaTScan Mac App Specific): " PASSWORD
fi

APPVERSION="10.0"
SRCDIR="/Users/satscsvc/prj/satscan.development/satscan"
INSTALLER_DIR="/prj/satscan/installers/v.${APPVERSION}.x"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
BUNDLEDIR="/Users/satscsvc/prj/satscan.development/jpackaged"
BINARIES="/Users/satscsvc/prj/satscan.development/binaries/mac"
JAVAJDK="/Users/satscsvc/prj/java/jdk-15.0.2+7/Contents/Home" # AdoptJDK
ENTITLEMENTS="${SRCDIR}/installers/macosentitlements.plist"
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"

# Clean up output directory
rm -rf $BUNDLEDIR
mkdir $BUNDLEDIR

# Create collection of files that will be the application.
mkdir $BUNDLEDIR/imagesrc
# Copy SaTScan.jar from fileshare -- maybe we can build this locally at some point.
echo Copying SaTScan.jar from fileshare
scp -r satscsvc@gen-btp-01.imsweb.com:/prj/satscan/build.area/satscan/java_application/jni_application/dist/SaTScan.jar $BUNDLEDIR/imagesrc
#cp -rf $SRCDIR/java_application/jni_application/dist/SaTScan.jar $BUNDLEDIR/imagesrc
cp -rf $SRCDIR/java_application/jni_application/libs $BUNDLEDIR/imagesrc
cp -rf $SRCDIR/installers/sample_data $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/SaTScan_Users_Guide.pdf $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/eula.html $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/eula/License.txt $BUNDLEDIR/imagesrc
cp -f $BINARIES/satscan $BUNDLEDIR/imagesrc
cp -f $BINARIES/libsatscan.jnilib $BUNDLEDIR/imagesrc

# Create overrride resources for DMG - https://docs.oracle.com/en/java/javase/15/jpackage/override-jpackage-resources.html#GUID-1B718F8B-B68D-4D46-B1DB-003D7729AAB6
mkdir $BUNDLEDIR/dmgresources
cp -f $SRCDIR/installers/resources/SaTScan.icns $BUNDLEDIR/dmgresources
cp -f $SRCDIR/installers/resources/SaTScan-volume.icns $BUNDLEDIR/dmgresources

# Ensure that our binaries/main jar are codesigned and runtime hardened. 
# This is just a safe guard to ensure they are runtime hardened since jpackage skips already signed.
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/satscan
codesign -vvv --strict $BUNDLEDIR/imagesrc/satscan
codesign --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/libsatscan.jnilib
codesign -vvv --strict $BUNDLEDIR/imagesrc/libsatscan.jnilib
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/SaTScan.jar
codesign -vvv --strict $BUNDLEDIR/imagesrc/SaTScan.jar

# Technically we should be able to just call the following to create the app, codesign and build dmg.
# Unfortunately the notarization fails - complaining about signatures on the launcher and dylib being invalid.
# After a lot of trial and error, I decided to try just uploading the app for notarization -- motivated by the following link:
# https://blog.frostwire.com/2019/08/27/apple-notarization-the-signature-of-the-binary-is-invalid-one-other-reason-not-explained-in-apple-developer-documentation/
# Notarizing the app alone succeeds. So my best guess, at this point, is that jpackage isn't creating the app within the dmg without mistakenly invalidating
# launcher/dylib code signatures. I tried many variations of building and code signing in pieces (similar to https://github.com/rokstrnisa/unattach/blob/master/package.sh)
# but always the notarization failed on the launcher/dylib code signatures.
#$JAVAJDK/bin/jpackage --verbose --type dmg --input $BUNDLEDIR/imagesrc --main-jar SaTScan.jar --icon $SRCDIR/installers/izpack/mac/satscan2app/Mac-App-Template/Contents/Resources/SaTScan.icns --app-version ${APPVERSION} --name SaTScan --dest $BUNDLEDIR/bin --java-options "-Djava.library.path=\$APPDIR" --mac-sign --mac-package-signing-prefix VF82MCMA83 --mac-signing-key-user-name "Information Management Services, Inc." --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --resource-dir $BUNDLEDIR/dmgresources

# Create SaTScan app directory
$JAVAJDK/bin/jpackage --verbose --type app-image --input $BUNDLEDIR/imagesrc --main-jar SaTScan.jar --icon $SRCDIR/installers/izpack/mac/satscan2app/Mac-App-Template/Contents/Resources/SaTScan.icns --app-version ${APPVERSION} --name SaTScan --dest $BUNDLEDIR --java-options "-Djava.library.path=\$APPDIR" --mac-sign --mac-package-signing-prefix VF82MCMA83 --mac-signing-key-user-name "Information Management Services, Inc."

# Clear any extended attributes - not really sure if this is needed.
xattr -cr $BUNDLEDIR/SaTScan.app

# Set application not writeable??
# chmod -R o-w $BUNDLEDIR/SaTScan.app

# Create zip file from SaTScan.app notarize application alone.
ditto -c -k --sequesterRsrc --keepParent $BUNDLEDIR/SaTScan.app $BUNDLEDIR/SaTScan.zip

# Notorize SaTScan.app - via SaTScan.zip
$XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.satscan.gui.SaTScanApplication" --username "meagherk@imsweb.com" --password "${PASSWORD}" --file $BUNDLEDIR/SaTScan.zip
#REQUEST_UUID=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.satscan.gui.SaTScanApplication" --username "meagherk@imsweb.com" --password "${PASSWORD}" --file $BUNDLEDIR/SaTScan.zip | grep RequestUUID | awk '{print $3}')

# Prompt user to import request uuid. We could probably get this value using grep and awk but the proxy message ... 
Echo What is the request uuid?
read REQUEST_UUID

# Poll for verification completion.
while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
    echo "Verification in progress..."
    sleep 30
done

echo Results of notarization
$XCRUN $ALTOOL --notarization-info "$REQUEST_UUID" -u "meagherk@imsweb.com" -p "${PASSWORD}"

# staple application -- assumes notarization succeeds.
$XCRUN $STAPLER staple $BUNDLEDIR/SaTScan.app

# Test notarized
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/SaTScan.app

# Create dmg with notarized application - but codesign separately.
$JAVAJDK/bin/jpackage --verbose --type dmg --app-image $BUNDLEDIR/SaTScan.app --app-version $APPVERSION --name SaTScan --dest $BUNDLEDIR/bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved" --resource-dir $BUNDLEDIR/dmgresources

# codesign and check SaTScan.dmg
# xattr -cr $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg
codesign -vvv --deep --force $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# Update on splitting this into 2 steps - build app/notarize then build dmg. When I tested downloading from our website
# the dmg was marked as quarantined, suggested to move to trash and wouldn't allow aapp to open. I tried adding the code
# below to notarize the dmg but no surprises there failed with same problem:
# "path": "SaTScan-10.0.dmg/SaTScan.app/Contents/MacOS/SaTScan", "message": "The signature of the binary is invalid.",
# "path": "SaTScan-10.0.dmg/SaTScan.app/Contents/runtime/Contents/MacOS/libjli.dylib", "message": "The signature of the binary is invalid.",
#
# Well we've got the app notarized and a zip file is considered a valid delivery method in some Apple documentation. So we can just run with
# that for now and keep revisiting the problem as time permits. And note that this doesn't entirely sidestep the issue since the downloaded zip,
# in Safari, automatically opens and extracts SaTScan.app in the Downloads directory. On top of that, the app is still labeled as quarantined
# and displays a warning to user – but at least it will open and allow the app to run!
# see https://squishlist.com/ims/satscan/66528/?search=469678&page=1#comment-1607792 for more details.

## echo notarizing dmg now ...
## 
## # Notorize dmg
## $XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.satscan.gui.SaTScanApplication" --username "meagherk@imsweb.com" --password "${PASSWORD}" --file $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg
## #REQUEST_UUID=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.satscan.gui.SaTScanApplication" --username "meagherk@imsweb.com" --password "${PASSWORD}" --file $BUNDLEDIR/SaTScan.zip | grep RequestUUID | awk '{print $3}')
## 
## # Prompt user to import request uuid. We could probably get this value using grep and awk but the proxy message ... 
## Echo What is the request uuid?
## read REQUEST_UUID
## 
## # Poll for verification completion.
## while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
##     echo "Verification in progress..."
##     sleep 30
## done
## 
## echo Results of notarization
## $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID" -u "meagherk@imsweb.com" -p "${PASSWORD}"
## 
## # staple application -- assumes notarization succeeds.
## $XCRUN $STAPLER staple $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# push over to fileshare installers directory
echo Copying dmg to fileshare
scp -r $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg $BUNDLEDIR/SaTScan.zip satscsvc@gen-btp-01.imsweb.com:${INSTALLER_DIR}

