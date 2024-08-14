#!/bin/bash

# Script which creates an installer using Java jpackage.
# Still a work in progress. We can build rpm file on gen-btp-01
# and that prm is convertable to deb file on Ubuntu. Still:
# The default template.spec file might not be 100% what we want.
# I tested this on Ubuntu and it installed but:
# - permissions are only for root
# - icon still wrong
# - probably other things when looking closer

javajdk="/prj/satscan/installers/install.applications/java/jdk-17.0.12+7_linux_x64"
version=$1
srcdir="/prj/satscan/build.area/satscan"
bundleinputdir="/prj/satscan/build.area/jpackage/satscanbundlesrc"
bundledir="/prj/satscan/build.area/jpackage"
binaries="/prj/satscan/build.area/binaries/linux"

rm -rf $bundleinputdir
rm -rf $bundledir/SaTScan
rm -rf $bundledir/bin


# copy all input files to bundle input dir
mkdir -p $bundleinputdir
cp $srcdir/java_application/jni_application/dist/SaTScan.jar $bundleinputdir
cp -rf $srcdir/java_application/jni_application/dist/lib $bundleinputdir
cp -rf $srcdir/installers/sample_data $bundleinputdir
cp -f $srcdir/installers/documents/SaTScan_Users_Guide.pdf $bundleinputdir
cp -f $srcdir/installers/documents/eula.html $bundleinputdir
cp -f $srcdir/installers/documents/eula/License.txt $bundleinputdir
cp -f $binaries/satscan32 $bundleinputdir
cp -f $binaries/libsatscan32.so $bundleinputdir
cp -f $binaries/satscan64 $bundleinputdir
cp -f $binaries/libsatscan64.so $bundleinputdir

# Build SaTScan app bundle
$javajdk/bin/jpackage --verbose --type app-image --input $bundleinputdir \
            --main-jar SaTScan.jar --icon $srcdir/installers/resources/SaTScan.png \
            --app-version $version --name SaTScan --dest $bundledir \
            --java-options "-Djava.library.path=\$APPDIR"

#  Create application rpm
$javajdk/bin/jpackage --verbose --type rpm --app-image $bundledir/SaTScan --app-version $version \
            --name SaTScan --resource-dir $srcdir/installers/resources --dest $2 \
            --description "Software for the spatial, temporal, and space-time scan statistics" \
            --vendor "Martin Kulldorff together with Information Management Services Inc." \
            --linux-shortcut --linux-rpm-license-type "see SaTScan License Agreement @ https://www.satscan.org/techdoc.html" \
            --license-file $srcdir/installers/documents/eula/License.txt --linux-app-category misc \
            --linux-app-release "0" --copyright "Copyright 2021, All rights reserved"

#  Create application deb
#$javajdk/bin/jpackage --verbose --type deb --app-image $bundledir/SaTScan --app-version $version \
#           --name SaTScan --resource-dir $srcdir/installers/resources --dest $2 \
#           --description "Software for the spatial, temporal, and space-time scan statistics" \
#           --about-url https://www.treescan.org/ \
#           --vendor "Martin Kulldorff together with Information Management Services Inc." \
#           --linux-shortcut --linux-rpm-license-type "see SaTScan License Agreement @ https://www.satscan.org/techdoc.html" \
#           --linux-app-release "0" --copyright "Copyright 2021, All rights reserved" \
#           --license-file $srcdir/installers/documents/eula/License.txt \
#           --linux-deb-maintainer techsupport@satscan.org  --linux-app-category misc