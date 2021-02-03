#!/bin/bash

OPTION=$1

# Configs
MOUNT_DIR="/Volumes/SaTScan"
DMG_NAME="SaTScan.dmg"
RW_DMG_NAME="SaTScan-rw.dmg"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
APP=BSI.app
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"

function permissions_fix {
  # Permissions fix:
  
  echo "no permissions to change yet"
  # chmod +x ${MOUNT_DIR}/SaTScan.app/Contents/MacOS/launchbsi
  # chmod -R +x ${MOUNT_DIR}/SaTScan.app/Contents/Resources/jre/bin/*
  # chmod -R +x ${MOUNT_DIR}/SaTScan.app/Contents/Resources/jre/lib/*
}

function create_dmg {
  # Attach writable dmg
  hdiutil attach "${RW_DMG_NAME}"
  # Copy the app in
  #cp -r "${APP}" "${MOUNT_DIR}"
  rm -rf "${MOUNT_DIR}/${APP}/Contents"
  ditto "${APP}" "${MOUNT_DIR}/${APP}"
  # Set permissions
  chmod -Rf go-w "${MOUNT_DIR}"
  permissions_fix
  # Make dmg open automatically
  bless --folder "${MOUNT_DIR}" --openfolder "${MOUNT_DIR}"
 
  # Unmount writable image
  hdiutil detach "${MOUNT_DIR}"
  unmounted=$?
  while [[ $unmounted != 1 ]]; do
    sleep 2
    hdiutil detach "${MOUNT_DIR}"
    unmounted=$?
  done

  # Remove old final DMG
  rm "${DMG_NAME}"
  # Convert to RO
  hdiutil convert "${RW_DMG_NAME}" -format UDZO -o "${DMG_NAME}" 
  # Sign the dmg
  xattr -cr "${DMG_NAME}"
  #codesign --entitlements Entitlements.plist --verbose --deep --force -s "${SIGN_KEY}" --timestamp "${DMG_NAME}"
  ## Check Signature
  #spctl -a -t open --context context:primary-signature -v "${DMG_NAME}"
}

if [[ $OPTION == "pull" ]]; then
  ###################
  # Pre-build Clean #
  ###################
  rm "${DMG_NAME}"
  rm -rf "${APP}"
  
  ##################
  # Retrieve Build #
  ##################
  scp -r gen-btp-01.imsweb.com:/Users/satscsvc/prj/satscan.development/satscan.home/build.area/satscan/installers/izpack/mac/satscan2app/SaTScan.app .
  
  ############
  # Sign app #
  ############
  # Unlock the keychain
  security unlock-keychain $HOME/Library/Keychains/login.keychain
  # Remove extended attributes
  xattr -cr BSI.app
  # # Sign java resources
  ## codesign --options runtime --force --deep --timestamp --verbose -s "${SIGN_KEY}" SaTScan.app/Contents/Resources/jre/lib/{libdecora_sse,*fx*,libglass,libglib-lite,libgstreamer-lite,libprism*}.dylib
  #codesign --entitlements ./Entitlements.plist --options runtime --force --timestamp --verbose -s "${SIGN_KEY}" BSI.app/Contents/MacOS/launchbsi
  codesign --entitlements ./Entitlements.plist --options runtime --force -vvv --deep --timestamp --verbose -s "${SIGN_KEY}" SaTScan.app
  # Check Signature
  codesign --options runtime --verify --deep --strict --verbose SaTScan.app
  spctl -a -t exec -vv SaTScan.app
  
  
  create_dmg
  
  echo "Please notarize the app, then run './pack.sh push'"
  exit

elif [[ $OPTION == "push" ]]; then
  # Staple and repackage
  $XCRUN $STAPLER staple SaTScan.app 
  rm $DMG_NAME
  create_dmg

  ssh satscsvc@gen-btp-01.imsweb.com "mkdir -p /Users/satscsvc/prj/satscan.development/satscan.home/build.area/satscan/installers/izpack/mac/satscan2app/signed"
  scp -r "${DMG_NAME}" satscsvc@gen-btp-01.imsweb.com:/home/bsiadmin/launcher/launcher/dist/signed/
  exit
fi

echo "pack.sh <option>"
echo "option: pull, push"
