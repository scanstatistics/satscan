#You must use GNU make (/opt/net/utils/bin/make) to run this script
#setting a line like 
#  alias gmake '/opt/net/utils/bin/make'
#in your .setup file will let you run GNU make by typing 'gmake'
#
#
# alter these two lines to be the full path to FAIRView and Basis
#
#
SATSCAN     := /home/hostovic/prj/satscan
BASIS       := /home/hostovic/prj/basisdev/
INSTALLDIR  := ~/FV

##########
# you shouldn't have to change any of this.

CC          := g++
APPLICATION :=SaTScan
DEBUG       := -gstabs+ 

CALCULATION    := $(SATSCAN)/calculation/
ANALYSIS       := $(SATSCAN)/calculation/analysis/
CLUSTER        := $(SATSCAN)/calculation/cluster/
OUTPUT         := $(SATSCAN)/calculation/Output/
PRINT          := $(SATSCAN)/calculation/print/
PROBMODEL      := $(SATSCAN)/calculation/probability_model/
SATDATA        := $(SATSCAN)/calculation/satscan_data/
UTILITY        := $(SATSCAN)/calculation/utility/
RANDOMIZER     := $(SATSCAN)/calculation/randomization/
LOGLIKELIHOOD  := $(SATSCAN)/calculation/loglikelihood/


ZDANNEX     := /sprj/imsdev/zd
ZDDIR       := /opt/net/utils/imsdev
#ZDANNEX     := /opt/net/utils/imsdev/gcc3.2.2
#ZDDIR       := /opt/net/utils/imsdev/gcc3.2.2
ZDVER       := 540
XBASEDIR    := $(SATSCAN)/xbase/xbase_2.0.0/xbase
XBASEDIR2   := $(SATSCAN)/xbase/xbase_2.0.0



INCLUDEDIRS := -I$(CALCULATION) -I$(ANALYSIS) -I$(ZDDIR) -I$(ZDANNEX) -I$(CLUSTER) -I$(UTILITY) -I$(XBASEDIR) -I$(XBASEDIR2)\
               -I$(BASIS)/include -I$(BASIS)/source/nonvcl -I$(BASIS)/source/vcldep -I$(OUTPUT) -I$(PRINT) -I$(PROBMODEL)\
	       -I$(SATDATA) -I$(UTILITY) -I$(RANDOMIZER) -I$(LOGLIKELIHOOD)

DEFINES     := -D__BASIS_NO_VCL \
               -D__BATCH_COMPILE

CFLAGS      := -c -O3 $(DEBUG) $(INCLUDEDIRS) $(DEFINES)
              

LFLAGS      := -L$(XBASEDIR) -L$(XBASEDIR2) -L$(ZDDIR) -L$(ZDANNEX) -lxbaseg -lm -ldl -lnsl -lzd$(ZDVER)g -lcrypto

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
               ../calculation/cluster/TimeTrend.cpp \
	       ../calculation/cluster/TimeIntervalRange.cpp \
	       ../calculation/cluster/AbstractClusterData.cpp \
	       ../calculation/cluster/ClusterData.cpp \
	       ../calculation/cluster/ClusterDataFactory.cpp \
	       ../calculation/cluster/MultipleStreamClusterData.cpp \
	       ../calculation/cluster/NormalClusterData.cpp \
	       ../calculation/Output/stsAreaSpecificData.cpp \
               ../calculation/Output/stsASCIIFileWriter.cpp \
               ../calculation/Output/stsClusterData.cpp \
               ../calculation/Output/stsDBaseFileWriter.cpp \
               ../calculation/Output/stsLogLikelihood.cpp \
               ../calculation/Output/stsOutputFileData.cpp \
               ../calculation/Output/stsOutputFileWriter.cpp \
               ../calculation/Output/stsRelativeRisk.cpp \
	       ../calculation/Output/stsRunHistoryFile.cpp \
	       ../calculation/print/BasePrint.cpp \
	       ../calculation/print/PrintScreen.cpp \
	       ../calculation/print/PrintQueue.cpp \
	       ../calculation/probability_model/ProbabilityModel.cpp \
	       ../calculation/probability_model/BernoulliModel.cpp \
	       ../calculation/probability_model/CalculateMeasure.cpp \
	       ../calculation/probability_model/PoissonModel.cpp \
	       ../calculation/probability_model/SpaceTimePermutationModel.cpp \
	       ../calculation/probability_model/NormalModel.cpp \
	       ../calculation/probability_model/SurvivalModel.cpp \
	       ../calculation/probability_model/RankModel.cpp \
	       ../calculation/randomization/BernoulliRandomizer.cpp \
	       ../calculation/randomization/PoissonRandomizer.cpp \
	       ../calculation/randomization/ContinuousVariableRandomizer.cpp \
	       ../calculation/randomization/DenominatorDataRandomizer.cpp \
	       ../calculation/randomization/PermutationDataRandomizer.cpp \
	       ../calculation/randomization/Randomizer.cpp \
	       ../calculation/loglikelihood/BernoulliLikelihoodCalculation.cpp \
	       ../calculation/loglikelihood/ExponentialLikelihoodCalculation.cpp \
	       ../calculation/loglikelihood/LikelihoodCalculation.cpp \
	       ../calculation/loglikelihood/NormalLikelihoodCalculation.cpp \
	       ../calculation/loglikelihood/PoissonLikelihoodCalculation.cpp \
	       ../calculation/loglikelihood/WilcoxonLikelihoodCalculation.cpp \
	       ../calculation/randomization/SpaceTimeRandomizer.cpp \
	       ../calculation/satscan_data/GridTractCoordinates.cpp \
	       ../calculation/satscan_data/MakeNeighbors.cpp \
	       ../calculation/satscan_data/PopulationData.cpp \
	       ../calculation/satscan_data/PurelySpatialData.cpp \
	       ../calculation/satscan_data/PurelyTemporalData.cpp \
	       ../calculation/satscan_data/SaTScanData.cpp \
	       ../calculation/satscan_data/SaTScanDataDisplay.cpp \
	       ../calculation/satscan_data/SaTScanDataRead.cpp \
	       ../calculation/satscan_data/SpaceTimeData.cpp \
	       ../calculation/satscan_data/SVTTData.cpp \
	       ../calculation/satscan_data/Tracts.cpp \
	       ../calculation/satscan_data/MaxWindowLengthIndicator.cpp \
	       ../calculation/satscan_data/AdjustmentHandler.cpp \
	       ../calculation/satscan_data/DataStream.cpp \
	       ../calculation/satscan_data/DataStreamGateway.cpp \
	       ../calculation/satscan_data/DataStreamHandler.cpp \
	       ../calculation/satscan_data/DataStreamInterface.cpp \
	       ../calculation/satscan_data/BernoulliDataStreamHandler.cpp \
	       ../calculation/satscan_data/NormalDataStreamHandler.cpp \
	       ../calculation/satscan_data/PoissonDataStreamHandler.cpp \
	       ../calculation/satscan_data/RankDataStreamHandler.cpp \
	       ../calculation/satscan_data/SpaceTimePermutationDataStreamHandler.cpp \
	       ../calculation/satscan_data/SurvivalDataStreamHandler.cpp \
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

