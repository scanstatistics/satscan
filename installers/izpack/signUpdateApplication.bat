
REM Signing a file: (cross-certificate)
\\oriole-04-int\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /v /ac \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\MSCV-VSClass3.cer /f \\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign-2017.pfx /p "YKtNv&otBfX7" /n "Information Management Services, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll \\oriole-04-int\satscan\installers\v.9.6.x\update_app.exe

REM To verify the file is signed correctly:
\\oriole-04-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-04-int\satscan\installers\v.9.6.x\update_app.exe
