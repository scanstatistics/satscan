

REM Codesigning a GUI exe file.
\\oriole-03-int\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.digicert.com/ /v \\oriole-03-int\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe

REM Verify the GUI exe file is codesigned correctly.
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe


REM Compile the InnoSetup installer.
"C:\Program Files (x86)\Inno Setup 6\iscc.exe" \\oriole-03-int\satscan\build.area\satscan\installers\inno-setup\satscan.iss


REM Codesigning a installer exe file.
\\oriole-03-int\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.digicert.com/ /v \\oriole-03-int\satscan\installers\v.10.0.x\install-10_0_windows.exe

REM Verify the installer exe file is codesigned correctly.
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\satscan\installers\v.10.0.x\install-10_0_windows.exe