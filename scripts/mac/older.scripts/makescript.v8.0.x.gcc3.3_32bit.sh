#!/bin/sh

# This shell script is intended to be a quick way to build SaTScan and supporting libraries.

# This script was used to build version 8.0.x on remote PPC machine during 8.0 development.

echo building xbase library ...
cd /Users/hostovic/my_nfsc/build.area/satscan/xbase/xbase_2.0.0/xbase
make clean
make COMPILATION="" CC=/usr/bin/gcc-3.3
echo xbase done
echo

echo building SaTScan binary ...
cd /Users/hostovic/my_nfsc/build.area/satscan/batch_application
make clean SATSCAN=/Users/hostovic/my_nfsc/build.area/satscan BOOSTDIR=/Users/hostovic/my_nfsc/build.area/boost/boost_1_33_1 SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED CC=/usr/bin/gcc-3.3
make COMPILATION="" SATSCAN=/Users/hostovic/my_nfsc/build.area/satscan BOOSTDIR=/Users/hostovic/my_nfsc/build.area/boost/boost_1_33_1 SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED CC=/usr/bin/gcc-3.3
strip SaTScan
mv ./SaTScan ../../scripts/mac.scripts/satscan_apple_8.0_gcc3.3.ppc_32bit
echo SaTScan done 
