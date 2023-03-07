@echo off

REM Script which creates an installer using Java jpackage.
REM At this time we're using launch4j, creating javatime and Inno Setup for the installer.
REM The installer created by jpacket is good and we might switch to it but there are a few issues to resolve:
REM   1) License isn't presented to user -- not sure what I'm doing wrong.
REM   2) I need to test update process - installing over previous installation.
REM   3) How to properly identify beta releases vs public releases?

set javabin=c:\jdk\jdk-17.0.6+10\bin
set version=10.1
set srcdir=C:\Users\hostovic\projects\satscan.development\satscan
set bundledir=C:\Users\hostovic\projects\satscan.development\jpackage

if exist %bundledir%\SaTScan rmdir %bundledir%\SaTScan /s /q
if exist %bundledir%\bin rmdir %bundledir%\bin /s /q

REM Build SaTScan app bundle
%javabin%\jpackage.exe  --verbose --type app-image --input %srcdir%\java_application\jni_application\dist --main-jar SaTScan.jar --icon %srcdir%\installers\resources\SaTScan.ico --app-version %version% --name SaTScan --dest %bundledir% --java-options "'-Djava.library.path=$APPDIR'"

REM Add additional files to bundle - command-line executables, dlls, sample data, user guide, etc.
xcopy /E /I /Y %srcdir%\installers\sample_data %bundledir%\SaTScan\sample_data
xcopy /Y %srcdir%\installers\documents\SaTScan_Users_Guide.pdf %bundledir%\SaTScan
xcopy /Y %srcdir%\installers\documents\eula.html %bundledir%\SaTScan
xcopy /Y %srcdir%\installers\documents\eula\License.txt %bundledir%\SaTScan
xcopy /Y %srcdir%\batch_application\Win32\Release\SaTScanBatch.exe %bundledir%\SaTScan
xcopy /Y %srcdir%\batch_application\x64\Release\SaTScanBatch64.exe %bundledir%\SaTScan
xcopy /Y %srcdir%\shared_library\Release\satscan32.dll %bundledir%\SaTScan\app
xcopy /Y %srcdir%\shared_library\x64\Release\satscan64.dll %bundledir%\SaTScan\app

REM Sign launcher exe but first toggle off read-only flag.
attrib -r %bundledir%\SaTScan\SaTScan.exe
call %srcdir%\signbinary.bat %bundledir%\SaTScan\SaTScan.exe
REM Toggle read-only flag on again.
attrib +r %bundledir%\SaTScan\SaTScan.exe

REM  Create application installer.
%javabin%\jpackage.exe  --verbose --type msi --app-image %bundledir%\SaTScan --app-version %version% --name SaTScan --dest %bundledir%\bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --win-shortcut --win-dir-chooser --win-menu-group --win-upgrade-uuid\"AD0046EA-ADC2-4AD7-B623-ADC246EA46EA" --license-file  %bundledir%\SaTScan\License.txt

REM Codesigning a installer exe but first toggle off read-only flag.
attrib -r %bundledir%\bin\SaTScan-%version%.msi
call %srcdir%\signbinary.bat %bundledir%\bin\SaTScan-%version%.msi
REM Toggle read-only flag on again.
attrib +r %bundledir%\bin\SaTScan-%version%.msi
