#!/bin/bash

# Secret Server #25934 (satscan)
if [[ $# -gt 0 ]]; then
  PASSWORD=$1
else
  read -p "Apple developer password (SaTScan Mac App Specific): " PASSWORD
fi

APPVERSION="10.1"
SRCDIR="/Users/satscsvc/prj/satscan.development/satscan"
INSTALLER_DIR="/prj/satscan/installers/v.${APPVERSION}.x"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
TEAM_ID="VF82MCMA83"
BUNDLEDIR="/Users/satscsvc/prj/satscan.development/jpackaged"
BINARIES="/Users/satscsvc/prj/satscan.development/binaries/mac"
JAVAJDK="/Users/satscsvc/prj/java/jdk-17.0.9+9_x86_64/Contents/Home"
ENTITLEMENTS="${SRCDIR}/installers/macosentitlements.plist"
XCRUN="/usr/bin/xcrun"
NOTARYTOOL="notarytool"
STAPLER="stapler"

notarizeOrFail() {
    # Notorize binary.
    echo "Requesting package notarization... this may take some time."
    n_time=$(date +%s)
    response=$($XCRUN $NOTARYTOOL submit ${1} --wait --apple-id "meagherk@imsweb.com" --password ${2} --team-id ${3})
    # Get the notarization job ID from the response
    e_time=$(date +%s)
    job_id_line=$(grep -m 1 '  id:' < <(echo -e "${response}"))
    job_id=$(echo "${job_id_line}" | cut -d ":" -s -f 2 | cut -d " " -f 2)
    n_time=$((e_time - n_time))
    echo "Notarization completed after ${n_time} seconds. Job ID: ${job_id}"
    # Get the notarization status from the response
    status_line=$(grep -m 1 '  status:' < <(echo -e "${response}"))
    status_result=$(echo "${status_line}" | cut -d ":" -s -f 2 | cut -d " " -f 2)
    # Check response status and exit
    if [[ ${status_result} != "Accepted" ]]; then
        echo "Notarization failed with status ${status_result}. Hit enter to see log results ..."
        $XCRUN $NOTARYTOOL log ${job_id} --apple-id "meagherk@imsweb.com" --password ${2} --team-id ${3}
        exit 1
    fi
    echo "Notarization -${status_line}"
}

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
$JAVAJDK/bin/jpackage --verbose --type app-image --input $BUNDLEDIR/imagesrc --main-jar SaTScan.jar \
                      --icon $SRCDIR/installers/resources/SaTScan.icns \
                      --app-version ${APPVERSION} --name SaTScan --dest $BUNDLEDIR --java-options "-Djava.library.path=\$APPDIR" \
                      --mac-sign --mac-package-signing-prefix org.satscan.SaTScan --mac-signing-key-user-name "${SIGN_KEY}" --mac-package-name "SaTScan" --mac-entitlements ${ENTITLEMENTS} \
                      --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto,jdk.crypto.cryptoki,jdk.accessibility

# Create zip file from SaTScan.app notarize application alone.
ditto -c -k --sequesterRsrc --keepParent $BUNDLEDIR/SaTScan.app $BUNDLEDIR/SaTScan.zip

echo Any problems creating app and codesigning?
read APPLES_TEST

# Notorize app file.
notarizeOrFail ${BUNDLEDIR}/SaTScan.zip ${PASSWORD} ${TEAM_ID}

# Staple application.
$XCRUN $STAPLER staple $BUNDLEDIR/SaTScan.app
# Test notarization.
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/SaTScan.app

echo Any problems notarizing app?
read APPLES_TEST

# Create dmg with notarized application - but codesign separately.
$JAVAJDK/bin/jpackage --type pkg  --verbose --app-image $BUNDLEDIR/SaTScan.app --app-version $APPVERSION \
                      --name SaTScan --dest $BUNDLEDIR/bin --description "Software for the spatial, temporal, and space-time scan statistics" \
					  --vendor "Information Management Services, Inc." --copyright "Copyright 2005, All rights reserved" \
					  --resource-dir $BUNDLEDIR/dmgresources --type dmg

# codesign and check SaTScan.dmg
codesign --entitlements  ${ENTITLEMENTS} --options runtime -vvvv --deep --timestamp -s "${SIGN_KEY}" $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

echo Any problems creating dmg and codesigning?
read APPLES_TEST

# Notorize SaTScan.dmg
notarizeOrFail $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg ${PASSWORD} ${TEAM_ID}

# Staple dmg.
$XCRUN $STAPLER staple $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

# Test dmg notarized.
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/bin/SaTScan-${APPVERSION}.dmg

echo Any problems notarizing dmg file?
read APPLES_TEST

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

