#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=9 # Script requires 9 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <satscan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <processor flag> (optional)[<pthread>])"
  exit 1
fi

echo building xbase library ...
cd $2/xbase/xbase_2.0.0/xbase
make clean
nice -n 19 make COMPILATION=$4 CC=$6 $8
echo xbase done
echo
  
echo building newmat library ...
cd $2/newmat/newmat10
make -f nm_gnu.mak clean
nice -n 19 make -f nm_gnu.mak libnewmat.a CXX=$6 CXXFLAGS="-O2 -Wall $4" $8
echo newmat done
echo

echo building shapelib library ...
cd $2/shapelib/shapelib_1.2.10
make clean
nice -n 19 make libshape.a CXX=$6 CXXFLAGS="-O2 -Wall $4" $8
echo shapelib done
echo

echo building zlib library ...
cd $2/zlib/zlib-1.3.1
make clean
nice -n 19 make libz.a CC=$7 CFLAGS="-O3 -Wall $4 -DHAVE_HIDDEN" $8
echo zlib done
echo

echo building SaTScan binary ...
cd $2
make clean SATSCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC=$6
nice -n 19 make SaTScan SATSCAN=$2 BOOSTDIR=$3 THREAD_DEFINE=$9 COMPILATION=$4 OPTIMIZATION=$5 CC=$6 PLAT_DEFINES=-DBOOST_NO_INCLASS_MEMBER_INITIALIZATION $8
strip SaTScan
mv SaTScan $1

echo SaTScan done
