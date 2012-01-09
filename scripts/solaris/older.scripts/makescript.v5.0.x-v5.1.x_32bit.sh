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
make clean 
make test CC=/opt/net/utils/gcc3.3.1/bin/gcc-3.3.1 -j8
echo zlib done
echo

#make zd
echo making zd ...						    
cd /prj/satscan/linux.unix.build.dir/zd/zd543
make clean
make -r ZDVER=543 ZDSOURCEDIR=. ZDTARGETDIR=.. CC=/opt/net/utils/gcc3.3.1/bin/g++-3.3.1 -j8
echo zd done
echo

#make xbase
echo making xbase ...
cd /prj/satscan/linux.unix.build.dir/satscan/xbase/xbase_2.0.0/xbase
make clean
make CC=/opt/net/utils/gcc3.3.1/bin/g++-3.3.1 -j8
echo xbase done
echo

#make satscan
echo making SaTScan
cd /prj/satscan/linux.unix.build.dir/satscan/batch_application
make clean SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh CC=/opt/net/utils/gcc3.3.1/bin/g++-3.3.1

# see http://www.trilithium.com/johan/2005/06/static-libstdc/
## rm -f libstdc++.a
## ln -s /usr/lib/libstdc++.a libstdc++.a 

make SATSCAN=/prj/satscan/linux.unix.build.dir/satscan BOOSTDIR=/prj/satscan/linux.unix.build.dir/boost/boost_1_33_1 BASIS=/prj/satscan/linux.unix.build.dir/basisdev ZDVER=543 ZDDIR=/prj/satscan/linux.unix.build.dir/zd ZDANNEX=/prj/satscan/linux.unix.build.dir/zd SHELL=/bin/sh THREAD_DEFINE=-pthreads CC=/opt/net/utils/gcc3.3.1/bin/g++-3.3.1 -j8
strip SaTScan
mv SaTScan ../../solaris.scripts/satscan_solaris_32

## rm -f libstdc++.a

echo SaTScan done
