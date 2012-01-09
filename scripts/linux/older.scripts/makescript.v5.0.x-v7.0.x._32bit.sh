#!/bin/sh

# This shell script is intended to be a quick way to build SaTScan and supporting
# libraries. Normally, I would have build a make file instead of this script but 
# the zd library make file produces an error when attempting to change user groups.
# The zd make error is harmless and I didn't want to start updating library files
# unless absolutely needed.
#
# I recommend running this script through some editor such as crisp so that the 
# output can be easily captured to an output window. Since this script just keeps
# on going should a make error occur, it may otherwise be difficult to determine
# which make invocation failed.

## This script is used to build versions: 5.0.x series through 7.0.x series.

#make zlib
echo making zlib ...
cd /prj/satscan/build.area/zd/zd546/zlib
make clean
make test CC="/usr/local/gcc3.3.5/bin/gcc-3.3.5 -m32" -j2
echo zlib done
echo

#make zd
echo making zd ...						    
cd /prj/satscan/build.area/zd/zd546
make clean
make -r ZDVER=546 BASE_SYSTEM_FLAG=-DLINUX_BASED ZDSOURCEDIR=. ZDTARGETDIR=.. CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
echo zd done
echo

# make xbase
echo making xbase ...
cd /prj/satscan/build.area/satscan.7.0/xbase/xbase_2.0.0/xbase
make clean
make CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
echo xbase done
echo

#make satscan
echo making SaTScan
cd /prj/satscan/build.area/satscan.7.0/batch_application
make clean SATSCAN=/prj/satscan/build.area/satscan.7.0 BOOSTDIR=/prj/satscan/build.area/boost/boost_1_33_1 BASIS=/prj/satscan/build.area/basisdev ZDVER=546 ZDDIR=/prj/satscan/build.area/zd ZDANNEX=/prj/satscan/build.area/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED CC=/usr/local/gcc3.3.5/bin/g++-3.3.5

## # see http://www.trilithium.com/johan/2005/06/static-libstdc/
## rm -f libstdc++.a
## ln -s /usr/lib/libstdc++.a libstdc++.a 

make SATSCAN=/prj/satscan/build.area/satscan.7.0 BOOSTDIR=/prj/satscan/build.area/boost/boost_1_33_1 BASIS=/prj/satscan/build.area/basisdev ZDVER=546 ZDDIR=/prj/satscan/build.area/zd ZDANNEX=/prj/satscan/build.area/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED THREAD_DEFINE=-pthread CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
strip SaTScan
#mv SaTScan ../../linux.scripts/satscan_linux_32

## rm -f libstdc++.a

echo SaTScan done

