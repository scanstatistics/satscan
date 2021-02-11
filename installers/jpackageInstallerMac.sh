#!/bin/bash

javajdk="/Users/satscsvc/prj/java/jdk-15.0.2.jdk"
version="10.0"
srcdir="/Users/satscsvc/prj/satscan.development/satscan"
bundledir="/Users/satscsvc/prj/satscan.development/jpackaged"
binaries="/Users/satscsvc/prj/satscan.development/binaries/mac"
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"
installer_dir="/prj/satscan/installers/v.${version}.x"

rm -rf $bundledir/SaTScan.app
rm -rf $bundledir/bin

# Build SaTScan app bundle
$javajdk/jpackage --verbose --type app-image --input $srcdir/java_application/jni_application/dist --main-jar SaTScan.jar --icon $srcdir/installers/izpack/mac/satscan2app/Mac-App-Template/Contents/Resources/SaTScan.icns --app-version ${version} --name SaTScan --dest $bundledir --java-options "-Djava.library.path=\$APPDIR"

# Add additional files to bundle - command-line executables, so, sample data, user guide, etc.
cp -rf $srcdir/installers/sample_data $bundledir/SaTScan.app/sample_data
cp -f $srcdir/installers/documents/SaTScan_Users_Guide.pdf $bundledir/SaTScan.app
cp -f $srcdir/installers/documents/eula.html $bundledir/SaTScan.app
cp -f $srcdir/installers/documents/eula/License.txt $bundledir/SaTScan.app
cp -f $binaries/satscan $bundledir/SaTScan.app
cp -f $binaries/libsatscan.jnilib $bundledir/SaTScan.app/Contents/app

# Codesign binaries and SaTScan.app
$srcdir/scripts/mac/codesign.sh $bundledir/SaTScan.app/Contents/app/satscan
$srcdir/scripts/mac/codesign.sh $bundledir/SaTScan.app/Contents/app/libsatscan.jnilib
# xattr -cr $bundledir/SaTScan.app
$srcdir/scripts/mac/codesign.sh $bundledir/SaTScan.app
# $XCRUN $STAPLER staple $bundledir/SaTScan.app 

### -- code sign java?

#  Create application installer (dmg or pkg).
$javajdk/jpackage --verbose --type dmg --app-image $bundledir/SaTScan.app --app-version $version --name SaTScan --dest $bundledir/bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"

# notorize SaTScan.app
$srcdir/scripts/mac/notarize.sh $bundledir/bin/SaTScan-${version}.dmg

# push over to fileshare
scp -r $bundledir/bin/SaTScan-${version}.dmg satscsvc@gen-btp-01.imsweb.com:${installer_dir}

