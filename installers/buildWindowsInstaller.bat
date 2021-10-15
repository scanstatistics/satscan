@echo off

REM Script which code signs SaTScan executable then builds SaTScan installer and finally code signs that installer.
REM This steps can't be executed from build environment -- currently Linux.

REM script definitions
set fileshare=\\oriole-03-int

set satscanversion=10.0
set satscanversionf=10_0
set satscanexe=%fileshare%\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe
set satscaninstaller=%fileshare%\satscan\installers\v.%satscanversion%.x\install-%satscanversionf%_windows.exe

set javajdkx64=%fileshare%\satscan\installers\install.applications\java\jdk-16.0.2+7_adopt_windows_x64
set runtimeoutputx64=%fileshare%\satscan\build.area\satscan\installers\java\jre_x64
set javajdkx86=%fileshare%\satscan\installers\install.applications\java\jdk-16.0.2+7_adopt_windows_x86
set runtimeoutputx86=%fileshare%\satscan\build.area\satscan\installers\java\jre_x86

set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\satscan\build.area\satscan\installers\inno-setup\satscan.iss

                         
set signtool=%fileshare%\imsadmin\code.sign.cert.ms.auth\signtool.exe
set certificate=%fileshare%\imsadmin\code.sign.cert.ms.auth\ims.pfx
set timestamp=http://timestamp.digicert.com/
set password="&4L(JyhyOmwF)$Z"


REM Codesigning a GUI exe file.
%fileshare%\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose %signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %password% %satscanexe%

REM Verify the GUI exe file is codesigned correctly.
%signtool% verify /pa /v %satscanexe%

REM Create Java 64-bit runtime
if exist %runtimeoutputx64% rmdir %runtimeoutputx64% /s /q
%javajdkx64%\bin\jlink.exe --module-path %javajdkx64%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto,jdk.crypto.cryptoki --output %runtimeoutputx64% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Create Java 32-bit runtime
if exist %runtimeoutputx86% rmdir %runtimeoutputx86% /s /q
%javajdkx86%\bin\jlink.exe --module-path %javajdkx86%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto,jdk.crypto.cryptoki --output %runtimeoutputx86% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %password% %satscaninstaller%

REM Verify the installer exe file is codesigned correctly.
%signtool% verify /pa /v %satscaninstaller%
