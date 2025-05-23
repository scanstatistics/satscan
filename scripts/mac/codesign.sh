#!/bin/bash

SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"

REQUIRED_ARGS=1
if [ $# -lt "$REQUIRED_ARGS" ]
then
echo "Usage: `basename $0` <file>"
echo "   example: `basename $0` ./libsatscan.jnilib"
echo "   example: `basename $0` ./satscan"
echo "   example: `basename $0` ./SaTScan.app"
echo "   example: `basename $0` ./Install.app"
exit 1
fi

security unlock-keychain $HOME/Library/Keychains/login.keychain && codesign --options runtime --timestamp --force -v --deep -s "${SIGN_KEY}" $1

spctl --assess --verbose=4 --raw $1
# spctl -a -t exec -vv $1