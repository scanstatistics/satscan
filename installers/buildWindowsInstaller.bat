@echo off

REM Script which code signs SaTScan executables then builds SaTScan installer and finally code signs that installer.

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
)
if %argCount% NEQ 1 (
  REM Secret Server #26118 - Note maybe need to escape characters on command-line (https://www.robvanderwoude.com/escapechars.php).
	echo Missing code signing certificate password in double-quotes.
	exit /b 1
)

REM script definitions
set fileshare=\\oriole-03-int

set satscanversion=10.1
set satscanversionf=10_1

set satscan32exe=%fileshare%\satscan\build.area\satscan\batch_application\Win32\Release\SaTScanBatch.exe
set satscan32dll=%fileshare%\satscan\build.area\satscan\shared_library\Release\satscan32.dll
set satscan64exe=%fileshare%\satscan\build.area\satscan\batch_application\x64\Release\SaTScanBatch64.exe
set satscan64dll=%fileshare%\satscan\build.area\satscan\shared_library\x64\Release\satscan64.dll
set satscanguiexe=%fileshare%\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe

set satscaninstaller=%fileshare%\satscan\installers\v.%satscanversion%.x\install-%satscanversionf%_windows.exe

set javajdkx64=%fileshare%\satscan\installers\install.applications\java\jdk-17.0.9+9_windows_x64
set runtimeoutputx64=%fileshare%\satscan\build.area\satscan\installers\java\jre_x64
set javajdkx86=%fileshare%\satscan\installers\install.applications\java\jdk-17.0.9+9_windows_x86
set runtimeoutputx86=%fileshare%\satscan\build.area\satscan\installers\java\jre_x86

set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\satscan\build.area\satscan\installers\inno-setup\satscan.iss
                         
set signtool=%fileshare%\imsadmin\code.sign.cert.ms.auth\signtool.exe
set certificate=%fileshare%\imsadmin\code.sign.cert.ms.auth\ims.pfx
set timestamp=http://timestamp.digicert.com/

REM Codesigning 32-bit command-line exe.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscan32exe%
REM Verify signiture
%signtool% verify /pa /v %satscan32exe%

REM Codesigning 32-bit dll.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscan32dll%
REM Verify signiture
%signtool% verify /pa /v %satscan32dll%

REM Codesigning 64-bit command-line exe.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscan64exe%
REM Verify signiture
%signtool% verify /pa /v %satscan64exe%

REM Codesigning 64-bit dll.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscan64dll%
REM Verify signiture
%signtool% verify /pa /v %satscan64dll%

REM Codesigning the GUI exe.
%fileshare%\satscan\build.area\satscan\installers\sign4j\sign4j.exe --verbose %signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscanguiexe%
REM Verify signiture
%signtool% verify /pa /v %satscanguiexe%

REM Create Java 64-bit runtime
if exist %runtimeoutputx64% rmdir %runtimeoutputx64% /s /q
%javajdkx64%\bin\jlink.exe --module-path %javajdkx64%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx64% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Create Java 32-bit runtime
if exist %runtimeoutputx86% rmdir %runtimeoutputx86% /s /q
%javajdkx86%\bin\jlink.exe --module-path %javajdkx86%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx86% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %1 %satscaninstaller%

REM Verify the installer exe file is codesigned correctly.
%signtool% verify /pa /v %satscaninstaller%
