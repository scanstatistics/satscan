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
BASIS       := /home/hostovic/prj/satscan/basisdev/
INSTALLDIR  := ~/FV

##########
# you shouldn't have to change any of this.

CC          := g++
APPLICATION := SaTScan
DEBUG       := -gstabs+ 

CALCULATION    := $(SATSCAN)/calculation
ANALYSIS       := $(SATSCAN)/calculation/analysis
ANALYSISRUN    := $(SATSCAN)/calculation/analysis_run
CLUSTER        := $(SATSCAN)/calculation/cluster
OUTPUT         := $(SATSCAN)/calculation/Output
PRINT          := $(SATSCAN)/calculation/print
PROBMODEL      := $(SATSCAN)/calculation/probability_model
SATDATA        := $(SATSCAN)/calculation/satscan_data
UTILITY        := $(SATSCAN)/calculation/utility
RANDOMIZER     := $(SATSCAN)/calculation/randomization
LOGLIKELIHOOD  := $(SATSCAN)/calculation/loglikelihood


ZDANNEX     := /sprj/imsdev/zd
ZDDIR       := /opt/net/utils/imsdev
#ZDANNEX     := /opt/net/utils/imsdev/gcc3.2.2
#ZDDIR       := /opt/net/utils/imsdev/gcc3.2.2
ZDVER       := 543
XBASEDIR    := $(SATSCAN)/xbase/xbase_2.0.0/xbase
XBASEDIR2   := $(SATSCAN)/xbase/xbase_2.0.0
BOOSTDIR    := $(SATSCAN)/../boost/release_1_29_0



INCLUDEDIRS := -I$(CALCULATION) -I$(ANALYSIS) -I$(ZDDIR) -I$(ZDANNEX) -I$(CLUSTER) -I$(UTILITY) -I$(XBASEDIR) -I$(XBASEDIR2)\
               -I$(BASIS)/include -I$(BASIS)/source/nonvcl -I$(BASIS)/source/vcldep -I$(OUTPUT) -I$(PRINT) -I$(PROBMODEL)\
	       -I$(SATDATA) -I$(UTILITY) -I$(RANDOMIZER) -I$(LOGLIKELIHOOD) -I$(ANALYSISRUN) -I$(BOOSTDIR)

DEFINES     := -D__BASIS_NO_VCL \
               -D__BATCH_COMPILE

CFLAGS      := -c -Wno-deprecated -O3 $(DEBUG) $(INCLUDEDIRS) $(DEFINES) $(BASE_SYSTEM_FLAG)
              

LFLAGS      := -L$(XBASEDIR) -L$(XBASEDIR2) -L$(ZDDIR) -L$(ZDANNEX) -Wl,-Bstatic -lxbaseg -lm -lzd$(ZDVER)g -Wl,-Bdynamic -lpthread

SRC         := Main.cpp \
               $(ANALYSIS)/Analysis.cpp \
	       $(ANALYSIS)/MeasureList.cpp \
               $(ANALYSIS)/PurelySpatialAnalysis.cpp \
               $(ANALYSIS)/PurelySpatialMonotoneAnalysis.cpp \
               $(ANALYSIS)/PurelyTemporalAnalysis.cpp \
               $(ANALYSIS)/SignificantRatios05.cpp \
               $(ANALYSIS)/SpaceTimeAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePureAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePurelySpatialAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePurelyTemporalAnalysis.cpp \
	       $(ANALYSIS)/SVTTAnalysis.cpp \
	       $(ANALYSIS)_run/AnalysisRun.cpp \
	       $(ANALYSIS)_run/MostLikelyClustersContainer.cpp\
               $(CLUSTER)/Cluster.cpp \
               $(CLUSTER)/IncidentRate.cpp \
               $(CLUSTER)/PurelySpatialCluster.cpp \
               $(CLUSTER)/PurelySpatialMonotoneCluster.cpp \
               $(CLUSTER)/PurelyTemporalCluster.cpp \
               $(CLUSTER)/SpaceTimeCluster.cpp \
	       $(CLUSTER)/SVTTCluster.cpp \
	       $(CLUSTER)/PurelySpatialProspectiveCluster.cpp \
               $(CLUSTER)/TimeEstimate.cpp \
               $(CLUSTER)/TimeTrend.cpp \
	       $(CLUSTER)/TimeIntervalRange.cpp \
	       $(CLUSTER)/AbstractClusterData.cpp \
	       $(CLUSTER)/ClusterData.cpp \
	       $(CLUSTER)/ClusterDataFactory.cpp \
	       $(CLUSTER)/MultipleStreamClusterData.cpp \
	       $(CLUSTER)/NormalClusterData.cpp \
	       $(OUTPUT)/stsAreaSpecificData.cpp \
               $(OUTPUT)/stsASCIIFileWriter.cpp \
               $(OUTPUT)/stsClusterData.cpp \
               $(OUTPUT)/stsDBaseFileWriter.cpp \
               $(OUTPUT)/stsLogLikelihood.cpp \
               $(OUTPUT)/stsOutputFileData.cpp \
               $(OUTPUT)/stsOutputFileWriter.cpp \
               $(OUTPUT)/stsRelativeRisk.cpp \
	       $(OUTPUT)/stsRunHistoryFile.cpp \
	       $(OUTPUT)/AsciiPrintFormat.cpp \
	       $(PRINT)/BasePrint.cpp \
	       $(PRINT)/PrintScreen.cpp \
	       $(PRINT)/PrintQueue.cpp \
	       $(PROBMODEL)/ProbabilityModel.cpp \
	       $(PROBMODEL)/BernoulliModel.cpp \
	       $(PROBMODEL)/CalculateMeasure.cpp \
	       $(PROBMODEL)/PoissonModel.cpp \
	       $(PROBMODEL)/SpaceTimePermutationModel.cpp \
	       $(PROBMODEL)/NormalModel.cpp \
	       $(PROBMODEL)/SurvivalModel.cpp \
	       $(PROBMODEL)/RankModel.cpp \
	       $(RANDOMIZER)/BernoulliRandomizer.cpp \
	       $(RANDOMIZER)/PoissonRandomizer.cpp \
	       $(RANDOMIZER)/ContinuousVariableRandomizer.cpp \
	       $(RANDOMIZER)/DenominatorDataRandomizer.cpp \
	       $(RANDOMIZER)/PermutationDataRandomizer.cpp \
	       $(RANDOMIZER)/Randomizer.cpp \
	       $(RANDOMIZER)/SpaceTimeRandomizer.cpp \
	       $(LOGLIKELIHOOD)/BernoulliLikelihoodCalculation.cpp \
	       $(LOGLIKELIHOOD)/ExponentialLikelihoodCalculation.cpp \
	       $(LOGLIKELIHOOD)/LikelihoodCalculation.cpp \
	       $(LOGLIKELIHOOD)/NormalLikelihoodCalculation.cpp \
	       $(LOGLIKELIHOOD)/PoissonLikelihoodCalculation.cpp \
	       $(LOGLIKELIHOOD)/WilcoxonLikelihoodCalculation.cpp \
	       $(SATDATA)/GridTractCoordinates.cpp \
	       $(SATDATA)/MakeNeighbors.cpp \
	       $(SATDATA)/PopulationData.cpp \
	       $(SATDATA)/PurelySpatialData.cpp \
	       $(SATDATA)/PurelyTemporalData.cpp \
	       $(SATDATA)/SaTScanData.cpp \
	       $(SATDATA)/SaTScanDataDisplay.cpp \
	       $(SATDATA)/SaTScanDataRead.cpp \
	       $(SATDATA)/SpaceTimeData.cpp \
	       $(SATDATA)/SVTTData.cpp \
	       $(SATDATA)/Tracts.cpp \
	       $(SATDATA)/MaxWindowLengthIndicator.cpp \
	       $(SATDATA)/AdjustmentHandler.cpp \
	       $(SATDATA)/DataStream.cpp \
	       $(SATDATA)/DataStreamGateway.cpp \
	       $(SATDATA)/DataStreamHandler.cpp \
	       $(SATDATA)/DataStreamInterface.cpp \
	       $(SATDATA)/BernoulliDataStreamHandler.cpp \
	       $(SATDATA)/NormalDataStreamHandler.cpp \
	       $(SATDATA)/PoissonDataStreamHandler.cpp \
	       $(SATDATA)/RankDataStreamHandler.cpp \
	       $(SATDATA)/SpaceTimePermutationDataStreamHandler.cpp \
	       $(SATDATA)/SurvivalDataStreamHandler.cpp \
	       $(UTILITY)/DBFFile.cpp \
	       $(UTILITY)/JulianDates.cpp \
	       $(UTILITY)/RandomDistribution.cpp \
	       $(UTILITY)/RandomNumberGenerator.cpp \
	       $(UTILITY)/Salloc.cpp \
	       $(UTILITY)/ScanfFile.cpp \
	       $(UTILITY)/SSException.cpp \
	       $(UTILITY)/UtilityFunctions.cpp \
	       $(UTILITY)/MultipleDimensionArrayHandler.cpp \
	       $(UTILITY)/stsMonteCarloSimFunctor.cpp \
	       $(UTILITY)/stsMCSimReporter.cpp \
	       $(UTILITY)/stsMCSimContinuationPolicy.cpp \
	       $(UTILITY)/DateStringParser.cpp \
	       $(CALCULATION)/Parameters.cpp \
	       $(CALCULATION)/SaTScanBasis.cpp \
	       $(CALCULATION)/Toolkit.cpp \
	       $(BOOSTDIR)/libs/thread/src/xtime.cpp \
	       $(BOOSTDIR)/libs/thread/src/exceptions.cpp \
	       $(BOOSTDIR)/libs/thread/src/mutex.cpp \
	       $(BOOSTDIR)/libs/thread/src/once.cpp \
	       $(BOOSTDIR)/libs/thread/src/recursive_mutex.cpp \
	       $(BOOSTDIR)/libs/thread/src/thread.cpp \
	       $(BOOSTDIR)/libs/thread/src/condition.cpp


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

