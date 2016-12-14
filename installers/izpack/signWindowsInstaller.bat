
REM Signing a file: (cross-certificate)
\\nfsk.omni.imsweb.com\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /v /ac \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\MSCV-VSClass3.cer /f \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p %%iaUzoA0cl!p /n "Information Management Services, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll \\nfsk.omni.imsweb.com\satscan\installers\v.9.5.x\install-9_5_windows.exe

REM To verify the file is signed correctly:
\\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\nfsk.omni.imsweb.com\satscan\installers\v.9.5.x\install-9_5_windows.exe
