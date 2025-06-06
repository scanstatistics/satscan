#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=10 # Script requires 10 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target> <target rename> <satscan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <processor flag> (optional)[<pthread>])"
  exit 1
fi

echo building xbase library ...
cd $3/xbase/xbase_2.0.0/xbase
make clean CFLAGS=-fPIC   
nice -n 19 make COMPILATION=$5 CC=$7 $9 CFLAGS=-fPIC 
echo xbase done
echo

echo building newmat library ...
cd $3/newmat/newmat10
make -f nm_gnu.mak clean
nice -n 19 make -f nm_gnu.mak libnewmat.a CXX=$7 CXXFLAGS="-O2 -Wall $5 -fPIC" $9
echo newmat done
echo

echo building shapelib library ...
cd $3/shapelib/shapelib_1.2.10
make clean
nice -n 19 make libshape.a CXX=$7 CXXFLAGS="-O2 -Wall $5 -fPIC" $9
echo shapelib done
echo

echo building zlib library ...
cd $3/zlib/zlib-1.3.1
make clean
nice -n 19 make libz.a CC=$8 CFLAGS="-O3 -Wall $5 -fPIC -DHAVE_HIDDEN" $9
echo zlib done
echo

echo building SaTScan binary ...
cd $3
make clean SATSCAN=$3 BOOSTDIR=$4 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 M_CFLAGS=-fPIC
nice -n 19 make $1 SATSCAN=$3 BOOSTDIR=$4 THREAD_DEFINE=${10} COMPILATION=$5 OPTIMIZATION=$6 CC=$7 $9 M_CFLAGS=-fPIC JNI=/etc/alternatives/java_sdk/include JNI_PLAT=/etc/alternatives/java_sdk/include/linux PLAT_DEFINES=-DBOOST_NO_INCLASS_MEMBER_INITIALIZATION
strip $1.x.x.0
mv $1.x.x.0 $2

echo SaTScan done
