#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=7 # Script requires 7 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <satscan source directory> <boost source directory> <compilation flag> <optimization flag> <compiler> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` ../satscan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/satscan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"  
  exit 1
fi

echo building xbase library ...
cd $2/xbase/xbase_2.0.0/xbase
make clean
nice -n 19 make COMPILATION=$4 CC=$6 $7
echo xbase done
echo
 
echo building newmat library ...
cd $2/newmat/newmat10
make -f nm_gnu.mak clean
nice -n 19 make -f nm_gnu.mak libnewmat.a CXX=$6 CXXFLAGS="-O2 -Wall $4" $7
echo newmat done
echo

echo building shapelib library ...
cd $2/shapelib/shapelib_1.2.10
make clean
nice -n 19 make libshape.a CXX=$6 CXXFLAGS="-O2 -Wall $4" $7
echo shapelib done
echo

echo building SaTScan binary ...
cd $2
make clean SATSCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC=$6
nice -n 19 make SaTScan SATSCAN=$2 BOOSTDIR=$3 THREAD_DEFINE=$8 COMPILATION=$4 OPTIMIZATION=$5 CC=$6 $7
strip SaTScan
mv SaTScan $1

echo SaTScan done
