#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=8 # Script requires 8 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target> <target rename> <satscan source directory> <boost source directory> <compilation flag> <optimization flag> <compiler> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` libsatscan.linux.so ../satscan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/satscan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"  
  exit 1
fi

echo building xbase library ...
cd $3/xbase/xbase_2.0.0/xbase
make clean CFLAGS=-fPIC   
nice -n 19 make COMPILATION=$5 CC=$7 $8 CFLAGS=-fPIC 
echo xbase done
echo

echo building newmat library ...
cd $3/newmat/newmat10
make -f nm_gnu.mak clean
nice -n 19 make -f nm_gnu.mak libnewmat.a CXX=$7 CXXFLAGS="-O2 -Wall $5 -fPIC" $8
echo newmat done
echo

echo building shapelib library ...
cd $3/shapelib/shapelib_1.2.10
make clean
nice -n 19 make libshape.a CXX=$7 CXXFLAGS="-O2 -Wall $5 -fPIC" $8
echo shapelib done
echo

echo building zlib library ...
cd $3/zlib/zlib-1.2.7
make clean
nice -n 19 make libz.a CC=$7 CFLAGS="-O3 -Wall $5 -fPIC -DHAVE_HIDDEN" $8
echo zlib done
echo

echo building SaTScan binary ...
cd $3
make clean SATSCAN=$3 BOOSTDIR=$4 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 M_CFLAGS=-fPIC
nice -n 19 make $1 SATSCAN=$3 BOOSTDIR=$4 THREAD_DEFINE=$9 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 $8 M_CFLAGS=-fPIC
strip $1.x.x.0
mv $1.x.x.0 $2

echo SaTScan done
