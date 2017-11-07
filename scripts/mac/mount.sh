#!/bin/sh

REQUIRED_ARGS=2
if [ $# -lt "$REQUIRED_ARGS" ]
then
echo "Usage: `basename $0` <nfsf username> <username>"
echo "   example: `basename $0` hostovic satscsvc"
exit 1
fi

# Mount nfse.omni.imsweb.com/prj/satscan onto this Mac

echo "mounting satscan home directory"
mount -t smbfs //OMNI\;$1@oriole-04-int/satscan /Users/$2/prj/satscan.development/satscan.home
