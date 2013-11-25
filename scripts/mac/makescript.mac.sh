#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=7 # Script requires 7 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <satscan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <arch> <minimum os version> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` ../satscan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/satscan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"  
  exit 1
fi

echo building xbase library ...
cd $2/xbase/xbase_2.0.0/xbase
make clean
make libxbase.dylib DEBUG= COMPILATION=$4 CC="$6 $8 $9 -dynamiclib" ${10}
echo xbase done
echo
 
echo building newmat library ...
cd $2/newmat/newmat10
make -f nm_gnu.mak clean
make -f nm_gnu.mak libnewmat.dylib CXX="$6 $8 $9 -dynamiclib" CXXFLAGS="-O2 -Wall $4" ${10}
echo newmat done
echo 

echo building shapelib library ...
cd $2/shapelib/shapelib_1.2.10
make clean
make libshape.dylib CXX="$6 $8 $9 -dynamiclib" CXXFLAGS="-O2 -Wall $4" ${10}
echo shapelib done
echo

echo building zlib library ...
cd $2/zlib/zlib-1.2.7
make clean
make libz.dylib CC="$7 $8 $9 -dynamiclib" CFLAGS="-O3 -Wall $4 -DHAVE_HIDDEN" ${10}
echo zlib done
echo

echo building SaTScan binary ...
cd $2
make clean SATSCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6 $8 $9"
make SaTScan_mac SATSCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6 $8 $9" ${10}
strip SaTScan_mac
mv SaTScan_mac $1

echo SaTScan done
