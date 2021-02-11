#!/bin/sh

javabin="/Users/satscsvc/prj/java/jdk-15.0.2.jdk/Contents/Home/bin"
version="10.0"
srcdir="/Users/satscsvc/prj/satscan.development/satscan"
bundledir="/Users/satscsvc/prj/satscan.development/jpackage"
binaries="/Users/satscsvc/prj/satscan.development/binaries/mac"

rm -rf $bundledir/SaTScan.app
rm -rf $bundledir/bin

# Build SaTScan app bundle
$javabin/jpackage --verbose --type app-image --input $srcdir/java_application/jni_application/dist --main-jar SaTScan.jar --icon $srcdir/installers/izpack/mac/satscan2app/Mac-App-Template/Contents/Resources/SaTScan.icns --app-version 10.0 --name SaTScan --dest $bundledir --java-options "-Djava.library.path=\$APPDIR"

# Add additional files to bundle - command-line executables, so, sample data, user guide, etc.
cp -rf $srcdir/installers/sample_data $bundledir/SaTScan.app/sample_data
cp -f $srcdir/installers/documents/SaTScan_Users_Guide.pdf $bundledir/SaTScan.app
cp -f $srcdir/installers/documents/eula.html $bundledir/SaTScan.app
cp -f $srcdir/installers/documents/eula/License.txt $bundledir/SaTScan.app
cp -f $binaries/satscan $bundledir/SaTScan.app
cp -f $binaries/libsatscan.jnilib $bundledir/SaTScan.app/Contents/app/libsatscan.jnilib

### TODO 
### -- code sign SaTScan.app and notorize
### -- code binries
### -- code sign java?

#  Create application installer (dmg or pkg).
$javabin/jpackage --verbose --type dmg --app-image $bundledir/SaTScan.app --app-version 10.0 --name SaTScan --dest $bundledir/bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"

### TODO 
### -- code sign (notorize?)
### -- change path to user guide

