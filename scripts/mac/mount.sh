#!

# Mount nfse.omni.imsweb.com/prj/satscan onto this Mac

echo "mounting satscan home directory"
mount -t smbfs //OMNI\;hostovic@nfse.omni.imsweb.com/satscan /Users/hostovic/prj/satscan.development/satscan.home
