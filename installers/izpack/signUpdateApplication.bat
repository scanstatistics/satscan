
REM Signing a file: (cross-certificate)
\\oriole-04-int\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.digicert.com/ /v \\oriole-04-int\satscan\installers\v.9.7.x\update_app.exe

REM To verify the file is signed correctly:
\\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-04-int\satscan\installers\v.9.7.x\update_app.exe
