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


## This script is used to build versions: 4.0.x series, part of 5.0.x series

#make zlib
echo making zlib ...
cd /prj/satscan/linux.unix.build.dir/zd/zd543/zlib
make clean
make CC=/usr/local/gcc2.95.3_32bit/bin/gcc
echo zlib done
echo

#make zd
echo making zd ...						    
cd /prj/satscan/linux.unix.build.dir/zd/zd543
make clean
make -r ZDVER=543 BASE_SYSTEM_FLAG=-DLINUX_BASED ZDSOURCEDIR=. ZDTARGETDIR=.. CC=/usr/local/gcc2.95.3_32bit/bin/g++
echo zd done
echo

# make xbase
echo making xbase ...
cd /prj/satscan/linux.unix.build.dir/satscan/xbase/xbase_2.0.0/xbase
make clean
make CC=/usr/local/gcc2.95.3_32bit/bin/g++
echo xbase done
echo

#make satscan
echo making SaTScan
cd /prj/satscan/linux.unix.build.dir/satscan/batch_application
make clean SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED CC=/usr/local/gcc2.95.3_32bit/bin/g++

## # see http://www.trilithium.com/johan/2005/06/static-libstdc/
## rm -f libstdc++.a
## ln -s /usr/lib/libstdc++.a libstdc++.a 

make SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED THREAD_DEFINE=-pthread CC=/usr/local/gcc2.95.3_32bit/bin/g++
strip SaTScan
mv SaTScan ../../linux.scripts/satscan_linux_32

## rm -f libstdc++.a

echo SaTScan done

