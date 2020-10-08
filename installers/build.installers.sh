#!/bin/sh

############ Script Defines #######################################################################
build="/prj/satscan/build.area"
installer_version="/prj/satscan/installers/v.9.7.x"

launch4j="/prj/satscan/installers/install.applications/launch4j/launch4j-3.12"
IzPack="/prj/satscan/installers/install.applications/IzPack/IzPack.5.1.3"

#### Windows ##############################################################################

# Build Windows SaTScan updater executable from java jar file ... this executable is needed when updating from v7.0.3 and earlier.
$launch4j/launch4j $build/satscan/installers/izpack/windows/launch4j_updater.xml

# prompt user to sign the exe file created by launch4j
echo
echo "Run the Windows batch file 'signUpdateApplication.bat' now to sign update_app.exe. Hit <enter> once done ..."
read dummy

# Build Windows SaTScan executable from java jar file ... SaTScan.jar -> SaTScan.exe.
$launch4j/launch4j $build/satscan/installers/izpack/windows/launch4j_app.xml

# prompt user to build gui exe and codesign it, then build installer and codesign that file as well.
echo
echo "Run the Windows batch file ' buildWindowsInstaller.bat' now to build and sign SaTScanScan.exe, then build/sign installer. Hit <enter> once done ..."
read dummy

# Build Windows command-line only archive
rm -f $installer_version/satscan.9.7_windows.zip
zip $installer_version/satscan.9.7_windows.zip -j $build/satscan/batch_application/Win32/Release/SaTScanBatch.exe
zip $installer_version/satscan.9.7_windows.zip -j $build/satscan/batch_application/x64/Release/SaTScanBatch64.exe
cd $build/satscan/installers
zip $installer_version/satscan.9.7_windows.zip -j documents/*
zip $installer_version/satscan.9.7_windows.zip sample_data/*

#######   ############ Linux ################################################################################
# Build the IzPack Java installer for Linux.
$IzPack/bin/compile $build/satscan/installers/izpack/linux/install_linux.xml -b $installer_version -o $installer_version/install-9_7_linux.jar -k standard
chmod a+x $installer_version/install-9_7_linux.jar

# Build batch binaries archive for Linux.
rm -f $installer_version/satscan.9.7_linux.tar.bz2
cd $build/binaries/linux
tar -cf $installer_version/satscan.9.7_linux.tar satscan*
cd $build/satscan/installers
tar -rf $installer_version/satscan.9.7_linux.tar documents/*
tar -rf $installer_version/satscan.9.7_linux.tar sample_data/*
bzip2 -f $installer_version/satscan.9.7_linux.tar

############ Mac OS X #############################################################################
# Build SaTScan Mac OS X Application Bundle Directory
rm -rf $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app
python $build/satscan/installers/izpack/mac/satscan2app/satscan2app.py $build/satscan/java_application/jni_application/dist/SaTScan.jar $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app
# copy jni libraries into app directory
cp $build/binaries/mac/libsatscan.jnilib $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/Java/libsatscan.jnilib
# copy additional Java libraries into app directory
cp -r $build/satscan/java_application/jni_application/dist/lib $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/Java
# copy jre into app directory
# http://www.balthisar.com/blog/bundle_the_jre/
#mkdir $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns
#mkdir $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns/Java.runtime
#mkdir $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns/Java.runtime/Contents/
#mkdir $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns/Java.runtime/Contents/Home
#mkdir $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns/Java.runtime/Contents/MacOS
#cp $build/satscan/installers/java/mac-jre $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app/Contents/PlugIns/Java.runtime/Contents/Home

# prompt user to sign the SaTScan.app on Mac with Developer ID certificated installed (Squish https://www.squishlist.com/ims/satscan/66329/)
echo
echo "1) Run the script .../satscan/scripts/mac/codesign_remote_appbundle.sh on SaTScan.app"
echo "2) Hit <enter> once done ..."
read dummy

# Build the IzPack Java installer for Mac OS X.
$IzPack/bin/compile $build/satscan/installers/izpack/mac/install_mac.xml -b $installer_version -o $installer_version/install-9_7_mac.jar -k standard

# Build Mac OS X Application Bundle from IzPack Java Installer
rm -rf $installer_version/install-9_7_mac.zip
rm -rf $build/satscan/installers/izpack/mac/Install.app
python $build/satscan/installers/izpack/mac/izpack2app/izpack2app.py $installer_version/install-9_7_mac.jar $build/satscan/installers/izpack/mac/Install.app

# prompt user to sign the Install.app on Mac with Developer ID certificated installed (Squish https://www.squishlist.com/ims/satscan/66329/)
echo
echo "1) Run the script .../satscan/scripts/mac/codesign_remote_appbundle.sh on Install.app"
echo "2) Hit <enter> once done ..."
read dummy

cd $build/satscan/installers/izpack/mac
zip $installer_version/install-9_7_mac.zip -r ./Install.app/*
rm $installer_version/install-9_7_mac.jar
rm -rf $build/satscan/installers/izpack/mac/Install.app
chmod a+x $installer_version/install-9_7_mac.zip

# Build batch binaries archive for Mac OS X.
rm -f $installer_version/satscan.9.7_mac.tar.bz2
cd $build/binaries/mac
tar -cf $installer_version/satscan.9.7_mac.tar satscan
cd $build/satscan/installers
tar -rf $installer_version/satscan.9.7_mac.tar documents/*
tar -rf $installer_version/satscan.9.7_mac.tar sample_data/*
bzip2 -f $installer_version/satscan.9.7_mac.tar

#rm -rf $build/satscan/installers/izpack/mac/satscan2app/SaTScan.app

############ Java Application Update Archive ######################################################
# Build update archive files -- relative paths are important; must be the same as installation

# Combined Windows/Linux update archive
#  -- Starting with the release featuring the Mac, this archive was not needed;
#     so only add Windows and Linux relevant files.
rm -f $installer_version/update_data_combined.zip

zip $installer_version/update_data_combined.zip -j $build/satscan/batch_application/Win32/Release/SaTScanBatch.exe
zip $installer_version/update_data_combined.zip -j $build/satscan/shared_library/Release/satscan32.dll
zip $installer_version/update_data_combined.zip -j $build/satscan/batch_application/x64/Release/SaTScanBatch64.exe
zip $installer_version/update_data_combined.zip -j $build/satscan/shared_library/x64/Release/satscan64.dll
zip $installer_version/update_data_combined.zip -j $build/binaries/linux/*
zip $installer_version/update_data_combined.zip -j $build/satscan/installers/documents/*
zip $installer_version/update_data_combined.zip -j $build/satscan/java_application/jni_application/dist/SaTScan.jar
zip $installer_version/update_data_combined.zip -j $build/satscan/java_application/jni_application/dist/SaTScan.exe
cd $build/satscan/java_application/jni_application/dist
zip $installer_version/update_data_combined.zip -r lib
cd $build/satscan/installers/java
zip $installer_version/update_data_combined.zip -r win32-jre
zip $installer_version/update_data_combined.zip -r win64-jre
cd $build/satscan/installers
zip $installer_version/update_data_combined.zip -r sample_data

# Windows update archive
rm -f $installer_version/update_data_windows.zip

zip $installer_version/update_data_windows.zip -j $build/satscan/batch_application/Win32/Release/SaTScanBatch.exe
zip $installer_version/update_data_windows.zip -j $build/satscan/shared_library/Release/satscan32.dll
zip $installer_version/update_data_windows.zip -j $build/satscan/batch_application/x64/Release/SaTScanBatch64.exe
zip $installer_version/update_data_windows.zip -j $build/satscan/shared_library/x64/Release/satscan64.dll
zip $installer_version/update_data_windows.zip -j $build/satscan/installers/documents/*
zip $installer_version/update_data_windows.zip -j $build/satscan/java_application/jni_application/dist/SaTScan.jar
zip $installer_version/update_data_windows.zip -j $build/satscan/java_application/jni_application/dist/SaTScan.exe
cd $build/satscan/java_application/jni_application/dist
zip $installer_version/update_data_windows.zip -r lib
cd $build/satscan/installers/java
zip $installer_version/update_data_windows.zip -r win32-jre
zip $installer_version/update_data_windows.zip -r win64-jre
cd $build/satscan/installers
zip $installer_version/update_data_windows.zip -r sample_data

# Linux update archive
rm -f $installer_version/update_data_linux.zip

zip $installer_version/update_data_linux.zip -j $build/binaries/linux/*
zip $installer_version/update_data_linux.zip -j $build/satscan/installers/documents/*
zip $installer_version/update_data_linux.zip -j $build/satscan/java_application/jni_application/dist/SaTScan.jar
cd $build/satscan/java_application/jni_application/dist
zip $installer_version/update_data_linux.zip -r lib
cd $build/satscan/installers
zip $installer_version/update_data_linux.zip -r sample_data

# Mac update archive
rm -f $installer_version/update_data_mac.zip

cd $build/satscan/installers/izpack/mac/satscan2app
zip $installer_version/update_data_mac.zip -r SaTScan.app
zip $installer_version/update_data_mac.zip -j $build/binaries/mac/satscan
zip $installer_version/update_data_mac.zip -j $build/satscan/installers/documents/*
cd $build/satscan/installers
zip $installer_version/update_data_mac.zip -r sample_data
