#!/bin/bash

############ Script Defines #######################################################################
version="10.0"
versionf="10_0"
build="/prj/satscan/build.area"
installer_version="/prj/satscan/installers/v.${version}.x"
binaries="/prj/satscan/build.area/binaries/linux"

javajdk="/prj/satscan/installers/install.applications/java/jdk-15.0.2/jdk-15.0.2-linux_x64"
launch4j="/prj/satscan/installers/install.applications/launch4j/launch4j-3.12"
IzPack="/prj/satscan/installers/install.applications/IzPack/IzPack.5.1.3"

#### Windows ##############################################################################
# Build the Inno Setup installer for Windows. (Note that someday we might replace this process with jpackageInstallerWindows.bat)

# Build Windows SaTScan executable from java jar file ... SaTScan.jar -> SaTScan.exe.
$launch4j/launch4j $build/satscan/installers/izpack/windows/launch4j_app.xml

# Prompt user to codesign SaTScan.exe then build installer and codesign that file as well.
echo
echo "Run the Windows batch file ' buildWindowsInstaller.bat' now to sign SaTScan.exe then build and sign the Windows installer. Hit <enter> once done ..."
read dummy

# Build Windows command-line only archive. This is an alternative download option that is command-line only (no GUI/Java).
rm -f $installer_version/satscan.${version}_windows.zip
zip $installer_version/satscan.${version}_windows.zip -j $build/satscan/batch_application/Win32/Release/SaTScanBatch.exe
zip $installer_version/satscan.${version}_windows.zip -j $build/satscan/batch_application/x64/Release/SaTScanBatch64.exe
cd $build/satscan/installers
zip $installer_version/satscan.${version}_windows.zip -j documents/*
zip $installer_version/satscan.${version}_windows.zip sample_data/*

#######   ############ Linux ################################################################################
# Build the IzPack Java installer for Linux. (Note that someday we might replace this process with jpackageInstallerLinux.sh)

# Build Linux installer. 
$IzPack/bin/compile $build/satscan/installers/izpack/linux/install_linux.xml -b $installer_version -o $installer_version/install-${versionf}_linux.jar -k standard
chmod a+x $installer_version/install-${versionf}_linux.jar

# Build batch binaries archive for Linux. This is an alternative download option that is command-line only (no GUI/Java).
rm -f $installer_version/satscan.${version}_linux.tar.gz
cd $build/binaries/linux
tar -cf $installer_version/satscan.${version}_linux.tar satscan*
cd $build/satscan/installers
tar -rf $installer_version/satscan.${version}_linux.tar documents/*
tar -rf $installer_version/satscan.${version}_linux.tar sample_data/*
gzip -f $installer_version/satscan.${version}_linux.tar

### ############ Mac OS X #############################################################################
### # Build SaTScan Mac OS X Application Bundle Directory

# Prompt user to execute Mac installer build process -- work in progress
echo
echo "1) Execute Mac dmg build script ...<???>. Hit <enter> once done ..."
read dummy

############ Java Application Update Archive ######################################################
# Build update archive files -- relative paths are important; must be the same as installation.
#
# Note: With the move to bundling Java into installation (Windows and Mac currently), this process
#       is not supported currently. The SaTScan application only notifies user of any update and
#       references the website for download.
#       The problem with the current update process is:
#       1) The update application is written in Java. So that application would need redesign.
#       2) I'm no longer certain how to update Mac dmg currently.

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
mv jre_windows_x64 jre
zip $installer_version/update_data_windows.zip -r jre
# We can delete the generated Windows Java runtime now.
rm -f $build/satscan/installers/java/jre
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

