
REM Signing a file: (cross-certificate)
\\oriole-04-int\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.verisign.com/scripts/timstamp.dll /v \\oriole-04-int\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe

REM To verify the file is signed correctly:
\\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-04-int\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe
