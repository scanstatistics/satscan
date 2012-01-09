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

#make zlib
echo making zlib ...
cd /prj/satscan/linux.unix.build.dir/zd/zd543/zlib
make -f Makefile.fpic clean
make -f Makefile.fpic test CC=/usr/local/gcc3.3.5/bin/gcc-3.3.5 -j2
echo zlib done
echo

#make zd
echo making zd ...						    
cd /prj/satscan/linux.unix.build.dir/zd/zd543
make -f Makefile.fpic clean
make -f Makefile.fpic -r ZDVER=543 BASE_SYSTEM_FLAG=-DLINUX_BASED ZDSOURCEDIR=. ZDTARGETDIR=.. CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
echo zd done
echo

# make xbase
echo making xbase ...
cd /prj/satscan/linux.unix.build.dir/satscan/xbase/xbase_2.0.0/xbase
make -f Makefile.fpic clean
make -f Makefile.fpic CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
echo xbase done
echo

#make satscan
echo making SaTScan
cd /prj/satscan/linux.unix.build.dir/satscan/shared.library
make -f Makefile.fpic clean SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED CC=/usr/local/gcc3.3.5/bin/g++-3.3.5
make -f Makefile.fpic SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh BASE_SYSTEM_FLAG=-DLINUX_BASED THREAD_DEFINE=-pthread CC=/usr/local/gcc3.3.5/bin/g++-3.3.5 -j2
##strip SaTScan
##mv SaTScan ../../satscan_linux_32
echo SaTScan done
