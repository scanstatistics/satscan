#You must use GNU make (/opt/net/utils/bin/make) to run this script
#setting a line like 
#  alias gmake '/opt/net/utils/bin/make'
#in your .setup file will let you run GNU make by typing 'gmake'
#
#
# alter these two lines to be the full path to FAIRView and Basis
#
# I modified the paths below so that you must change them to a valid path.
# When you make changes to this file, replace your account name with
# "<your path>" before you upload via RCS. For example if the BASIS path
# you use is /home/beavis/basisdev, make sure you change it back to 
# /home/<your path>/basisdev prior to uploading it to RCS.  -- Jim
#  
SATSCAN     := /home/hostovic/prj/satscan
BASIS       := /home/hostovic/prj/basisdev/
INSTALLDIR  := ~/FV

##########
# you shouldn't have to change any of this.

SHELL       := /usr/bin/csh
CC          := g++
APPLICATION :=SaTScan
DEBUG       := -gstabs+ 

CALCULATION    := $(SATSCAN)/calculation/
ANALYSIS       := $(SATSCAN)/calculation/analysis/
CLUSTER        := $(SATSCAN)/calculation/cluster/
OUTPUT         := $(SATSCAN)/calculation/output/
PRINT          := $(SATSCAN)/calculation/print/
PROBMODEL      := $(SATSCAN)/calculation/probability_model/
SATDATA        := $(SATSCAN)/calculation/satscan_data/
UTILITY        := $(SATSCAN)/calculation/utility


ZDANNEX     := /sprj/imsdev/zd
ZDDIR       := /opt/net/utils/imsdev
#ZDANNEX     := /opt/net/utils/imsdev/gcc3.2.2
#ZDDIR       := /opt/net/utils/imsdev/gcc3.2.2
ZDVER       := 540
XBASEDIR    := /home/hostovic/prj/satscan/xbase/xbase_2.0.0/xbase
XBASEDIR2   := /home/hostovic/prj/satscan/xbase/xbase_2.0.0



INCLUDEDIRS := -I$(CALCULATION) -I$(ANALYSIS) -I$(ZDDIR) -I$(ZDANNEX) -I$(CLUSTER) -I$(UTILITY) -I$(XBASEDIR) -I$(XBASEDIR2)\
               -I$(BASIS)/include -I$(BASIS)/source/nonvcl -I$(BASIS)/source/vcldep -I$(OUTPUT) -I$(PRINT) -I$(PROBMODEL) -I$(SATDATA) -I$(UTILITY)

DEFINES     := -D__BASIS_NO_VCL \
               -D__BATCH_COMPILE

CFLAGS      := -c -O3 $(DEBUG) $(INCLUDEDIRS) $(DEFINES)
              

LFLAGS      := -L$(XBASEDIR) -L$(XBASEDIR2) -L$(ZDDIR) -L$(ZDANNEX) -lxbaseg -lm -ldl -lsocket -lnsl -lzd$(ZDVER)g -lcrypto -lct -lcomn -lcs -lintl \
		-lsdce -lskrb -lsrv -lsybdb -ltcl -ltli

SRC         := Main.cpp \
               ../calculation/analysis/Analysis.cpp \
               ../calculation/analysis/MeasureList.cpp \
               ../calculation/analysis/PurelySpatialAnalysis.cpp \
               ../calculation/analysis/PurelySpatialMonotoneAnalysis.cpp \
               ../calculation/analysis/PurelyTemporalAnalysis.cpp \
               ../calculation/analysis/SignificantRatios05.cpp \
               ../calculation/analysis/SpaceTimeAnalysis.cpp \
               ../calculation/analysis/SpaceTimeIncludePureAnalysis.cpp \
               ../calculation/analysis/SpaceTimeIncludePurelySpatialAnalysis.cpp \
               ../calculation/analysis/SpaceTimeIncludePurelyTemporalAnalysis.cpp \
	       ../calculation/analysis/SVTTAnalysis.cpp \
               ../calculation/cluster/Cluster.cpp \
               ../calculation/cluster/IncidentRate.cpp \
               ../calculation/cluster/PurelySpatialCluster.cpp \
               ../calculation/cluster/PurelySpatialMonotoneCluster.cpp \
               ../calculation/cluster/PurelyTemporalCluster.cpp \
               ../calculation/cluster/SpaceTimeCluster.cpp \
	       ../calculation/cluster/SVTTCluster.cpp \
	       ../calculation/cluster/PurelySpatialProspectiveCluster.cpp \
               ../calculation/cluster/TimeEstimate.cpp \
               ../calculation/cluster/TimeIntervalAlive.cpp \
               ../calculation/cluster/TimeIntervalAll.cpp \
	       ../calculation/cluster/TimeTrend.cpp \
	       ../calculation/cluster/TimeIntervalRange.cpp \
               ../calculation/output/stsAreaSpecificData.cpp \
               ../calculation/output/stsASCIIFileWriter.cpp \
               ../calculation/output/stsClusterData.cpp \
               ../calculation/output/stsDBaseFileWriter.cpp \
               ../calculation/output/stsLogLikelihood.cpp \
               ../calculation/output/stsOutputFileData.cpp \
               ../calculation/output/stsOutputFileWriter.cpp \
               ../calculation/output/stsRelativeRisk.cpp \
	       ../calculation/output/stsRunHistoryFile.cpp \
	       ../calculation/print/BasePrint.cpp \
	       ../calculation/print/PrintScreen.cpp \
	       ../calculation/probability_model/ProbabilityModel.cpp \
	       ../calculation/probability_model/BernoulliModel.cpp \
	       ../calculation/probability_model/CalculateMeasure.cpp \
	       ../calculation/probability_model/PoissonModel.cpp \
	       ../calculation/probability_model/SpaceTimePermutationModel.cpp \
	       ../calculation/satscan_data/GridTractCoordinates.cpp \
	       ../calculation/satscan_data/MakeNeighbors.cpp \
	       ../calculation/satscan_data/PopulationCategories.cpp \
	       ../calculation/satscan_data/PurelySpatialData.cpp \
	       ../calculation/satscan_data/PurelyTemporalData.cpp \
	       ../calculation/satscan_data/SaTScanData.cpp \
	       ../calculation/satscan_data/SaTScanDataDisplay.cpp \
	       ../calculation/satscan_data/SaTScanDataRead.cpp \
	       ../calculation/satscan_data/SpaceTimeData.cpp \
	       ../calculation/satscan_data/SVTTData.cpp \
	       ../calculation/satscan_data/Tracts.cpp \
	       ../calculation/utility/DBFFile.cpp \
	       ../calculation/utility/JulianDates.cpp \
	       ../calculation/utility/RandomDistribution.cpp \
	       ../calculation/utility/RandomNumberGenerator.cpp \
	       ../calculation/utility/Salloc.cpp \
	       ../calculation/utility/ScanfFile.cpp \
	       ../calculation/utility/SSException.cpp \
	       ../calculation/utility/UtilityFunctions.cpp \
	       ../calculation/utility/MultipleDimensionArrayHandler.cpp \
	       ../calculation/Parameters.cpp \
	       ../calculation/SaTScanBasis.cpp \
	       ../calculation/Toolkit.cpp


OBJS        := $(SRC:.cpp=.o)

.PHONY: clean all thin

all : $(APPLICATION)

thin : all
	strip SaTScan

$(APPLICATION) : $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $@

%.o : %.cpp 
	$(CC) $(CFLAGS) $< -o $@ 

clean :
	rm -f core $(OBJS)
	rm -f $(APPLICATION)
	
#install : thin
#	mkdir $(INSTALLDIR)
#	mkdir $(INSTALLDIR)/db
#	cp ./initial/fvserv.cfg $(INSTALLDIR)
#	cp ./initial/fair.zdd $(INSTALLDIR)/db
#	cp fvServer $(INSTALLDIR)

