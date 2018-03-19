
SATSCAN     := /prj/satscan/build.area/satscan

CC          := g++
M_CFLAGS    :=
APPLICATION := SaTScan
MAC_APPLICATION := SaTScan_mac
LINUX_LIBRARY := libsatscan.linux.so
SOLARIS_LIBRARY := libsatscan.solaris.so
MAC_LIBRARY := libsatscan.jnilib
DEBUG       := -ggdb
COMPILATION := -m32
OPTIMIZATION := -O3
COMPONENT_REPORT := #-DRPRTCMPT_RUNTIMES

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

XBASEDIR       := $(SATSCAN)/xbase/xbase_2.0.0/xbase
XBASEDIR2      := $(SATSCAN)/xbase/xbase_2.0.0
BOOSTDIR       := $(SATSCAN)/../boost/boost_1_39_0
NEWMAT         := $(SATSCAN)/newmat/newmat10
SHAPELIB       := $(SATSCAN)/shapelib/shapelib_1.2.10
ZLIB           := $(SATSCAN)/zlib/zlib-1.2.7
ZLIB_MINIZIP   := $(SATSCAN)/zlib/zlib-1.2.7/contrib/minizip
#JNI           :=
INFOPLIST_FILE :=

INCLUDEDIRS := -I$(CALCULATION) -I$(ANALYSIS) -I$(CLUSTER) -I$(UTILITY) -I$(XBASEDIR) -I$(XBASEDIR2)\
               -I$(OUTPUT) -I$(PRINT) -I$(PROBMODEL) -I$(NEWMAT) -I$(SHAPELIB) -I$(ZLIB) -I$(ZLIB_MINIZIP)\
	           -I$(SATDATA) -I$(UTILITY) -I$(RANDOMIZER) -I$(LOGLIKELIHOOD) -I$(ANALYSISRUN) -I$(BOOSTDIR) -I$(JNI)

DEFINES     := -D__BATCH_COMPILE \
               -DBOOST_ALL_NO_LIB

CFLAGS      := -c $(M_CFLAGS) $(COMPILATION) -Wno-deprecated $(OPTIMIZATION) $(DEBUG) $(INCLUDEDIRS) $(DEFINES) $(THREAD_DEFINE) $(COMPONENT_REPORT)
LFLAGS      := $(COMPILATION) -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -Wl,-Bstatic -lxbaseg -lnewmat -lshape -lz -lm -Wl,-Bdynamic -lrt -lpthread
# static libgcc flags
#LFLAGS      := $(COMPILATION) -static-libgcc -L. -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -Wl,-Bstatic -lstdc++ -lrt -lxbaseg -lnewmat -lshape -lm -lpthread

# Linux link flags
L_DLFLAGS   := -shared $(COMPILATION) -Wl,-soname,$(LINUX_LIBRARY).x.x -o $(LINUX_LIBRARY).x.x.0

# Solaris link flags
S_DLFLAGS   := -shared $(COMPILATION) -z text -o $(SOLARIS_LIBRARY).x.x.0

# Mac OS X flags
M_LFLAGS      := $(COMPILATION) -sectcreate __TEXT __info_plist $(INFOPLIST_FILE) -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -Wl,-dynamic -lxbase -lnewmat -lshape -lz -lstdc++ -lm
M_DLFLAGS     := -shared -sectcreate __TEXT __info_plist $(INFOPLIST_FILE) $(COMPILATION) -install_name $(MAC_LIBRARY)

SRC         := $(ANALYSIS)/Analysis.cpp \
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
               $(ANALYSIS)/SVTTCentricAnalysis.cpp \
               $(ANALYSIS)/IntermediateClustersContainer.cpp \
               $(ANALYSIS)/AbstractAnalysis.cpp \
               $(ANALYSIS)/AbstractCentricAnalysis.cpp \
               $(ANALYSIS)/PurelySpatialCentricAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeCentricAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePureCentricAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePurelySpatialCentricAnalysis.cpp \
               $(ANALYSIS)/SpaceTimeIncludePurelyTemporalCentricAnalysis.cpp \
               $(ANALYSIS)/AbstractBruteForceAnalysis.cpp \
               $(ANALYSIS)/PurelySpatialBruteForceAnalysis.cpp \
               $(ANALYSIS)_run/AnalysisRun.cpp \
               $(ANALYSIS)_run/MostLikelyClustersContainer.cpp \
               $(ANALYSIS)_run/LocationRelevance.cpp \
               $(CLUSTER)/Cluster.cpp \
               $(CLUSTER)/IncidentRate.cpp \
               $(CLUSTER)/PurelySpatialCluster.cpp \
               $(CLUSTER)/PurelySpatialMonotoneCluster.cpp \
               $(CLUSTER)/PurelyTemporalCluster.cpp \
               $(CLUSTER)/SpaceTimeCluster.cpp \
               $(CLUSTER)/SVTTCluster.cpp \
               $(CLUSTER)/PurelySpatialProspectiveCluster.cpp \
               $(CLUSTER)/TimeTrend.cpp \
               $(CLUSTER)/TimeIntervalRange.cpp \
               $(CLUSTER)/TimeIntervals.cpp \
               $(CLUSTER)/AbstractClusterData.cpp \
               $(CLUSTER)/ClusterData.cpp \
               $(CLUSTER)/NormalClusterData.cpp \
               $(CLUSTER)/CategoricalClusterData.cpp \
               $(CLUSTER)/AbstractClusterDataFactory.cpp \
               $(CLUSTER)/ClusterDataFactory.cpp \
               $(CLUSTER)/NormalClusterDataFactory.cpp \
               $(CLUSTER)/CategoricalClusterDataFactory.cpp \
               $(CLUSTER)/MultiSetClusterData.cpp \
               $(CLUSTER)/MultiSetCategoricalClusterData.cpp \
               $(CLUSTER)/MultiSetNormalClusterData.cpp \
               $(CLUSTER)/PurelySpatialHomogeneousPoissonCluster.cpp \
               $(OUTPUT)/ClusterLocationsWriter.cpp \
               $(OUTPUT)/stsASCIIFileWriter.cpp \
               $(OUTPUT)/ClusterInformationWriter.cpp \
               $(OUTPUT)/stsDBaseFileWriter.cpp \
               $(OUTPUT)/LoglikelihoodRatioWriter.cpp \
               $(OUTPUT)/AbstractDataFileWriter.cpp \
               $(OUTPUT)/LocationRiskEstimateWriter.cpp \
               $(OUTPUT)/stsRunHistoryFile.cpp \
               $(OUTPUT)/AsciiPrintFormat.cpp \
               $(OUTPUT)/ClusterScatterChart.cpp \
               $(OUTPUT)/ShapeFileWriter.cpp \
               $(OUTPUT)/ClusterKML.cpp \
               $(OUTPUT)/ChartGenerator.cpp \
               $(OUTPUT)/ChartMap.cpp \
               $(OUTPUT)/GisUtils.cpp \
               $(PRINT)/BasePrint.cpp \
               $(PRINT)/PrintScreen.cpp \
               $(PRINT)/PrintQueue.cpp \
               $(PROBMODEL)/ProbabilityModel.cpp \
               $(PROBMODEL)/BernoulliModel.cpp \
               $(PROBMODEL)/CalculateMeasure.cpp \
               $(PROBMODEL)/PoissonModel.cpp \
               $(PROBMODEL)/SpaceTimePermutationModel.cpp \
               $(PROBMODEL)/NormalModel.cpp \
               $(PROBMODEL)/ExponentialModel.cpp \
               $(PROBMODEL)/RankModel.cpp \
               $(PROBMODEL)/OrdinalModel.cpp \
               $(PROBMODEL)/HomogeneousPoissonModel.cpp \
               $(RANDOMIZER)/BernoulliRandomizer.cpp \
               $(RANDOMIZER)/PoissonRandomizer.cpp \
               $(RANDOMIZER)/DenominatorDataRandomizer.cpp \
               $(RANDOMIZER)/PermutationDataRandomizer.cpp \
               $(RANDOMIZER)/ExponentialRandomizer.cpp \
               $(RANDOMIZER)/Randomizer.cpp \
               $(RANDOMIZER)/SpaceTimeRandomizer.cpp \
               $(RANDOMIZER)/OrdinalDataRandomizer.cpp \
               $(RANDOMIZER)/RankRandomizer.cpp \
               $(RANDOMIZER)/NormalRandomizer.cpp \
               $(RANDOMIZER)/WeightedNormalRandomizer.cpp \
               $(RANDOMIZER)/HomogeneousPoissonRandomizer.cpp \
               $(LOGLIKELIHOOD)/BernoulliLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/LikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/LoglikelihoodRatioUnifier.cpp \
               $(LOGLIKELIHOOD)/NormalLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/PoissonLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/PoissonSVTTLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/WilcoxonLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/OrdinalLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/WeightedNormalLikelihoodCalculation.cpp \
               $(LOGLIKELIHOOD)/WeightedNormalCovariatesLikelihoodCalculation.cpp \
               $(SATDATA)/GridTractCoordinates.cpp \
               $(SATDATA)/CentroidNeighbors.cpp \
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
               $(SATDATA)/DataSet.cpp \
               $(SATDATA)/DataSetGateway.cpp \
               $(SATDATA)/DataSetHandler.cpp \
               $(SATDATA)/DataSetInterface.cpp \
               $(SATDATA)/BernoulliDataSetHandler.cpp \
               $(SATDATA)/NormalDataSetHandler.cpp \
               $(SATDATA)/PoissonDataSetHandler.cpp \
               $(SATDATA)/RankDataSetHandler.cpp \
               $(SATDATA)/SpaceTimePermutationDataSetHandler.cpp \
               $(SATDATA)/ExponentialDataSetHandler.cpp \
               $(SATDATA)/OrdinalDataSetHandler.cpp \
               $(SATDATA)/DataSource.cpp \
               $(SATDATA)/DataSetReader.cpp \
               $(SATDATA)/DataSetWriter.cpp \
               $(SATDATA)/MetaTractManager.cpp \
               $(SATDATA)/HomogeneousPoissonDataSetHandler.cpp \
               $(SATDATA)/ObservableRegion.cpp \
               $(SATDATA)/ClosedLoopData.cpp \
               $(UTILITY)/dBaseFile.cpp \
               $(UTILITY)/JulianDates.cpp \
               $(UTILITY)/RandomDistribution.cpp \
               $(UTILITY)/RandomNumberGenerator.cpp \
               $(UTILITY)/SSException.cpp \
               $(UTILITY)/UtilityFunctions.cpp \
               $(UTILITY)/MultipleDimensionArrayHandler.cpp \
               $(UTILITY)/stsMonteCarloSimFunctor.cpp \
               $(UTILITY)/stsCentricAlgoFunctor.cpp \
               $(UTILITY)/stsCentricAlgoJobSource.cpp \
               $(UTILITY)/OliveiraFunctor.cpp \
               $(UTILITY)/OliveiraJobSource.cpp \
               $(UTILITY)/DateStringParser.cpp \
               $(UTILITY)/stsMCSimJobSource.cpp \
               $(UTILITY)/FileName.cpp \
               $(UTILITY)/Ini.cpp \
               $(UTILITY)/TimeStamp.cpp \
               $(UTILITY)/FieldDef.cpp \
               $(UTILITY)/ShapeFile.cpp \
               $(UTILITY)/ZipUtils.cpp \
               $(CALCULATION)/Parameters.cpp \
               $(CALCULATION)/ParametersPrint.cpp \
               $(CALCULATION)/ParametersValidate.cpp \
               $(CALCULATION)/Toolkit.cpp \
               $(CALCULATION)/ParameterFileAccess.cpp \
               $(CALCULATION)/ScanLineParameterFileAccess.cpp \
               $(CALCULATION)/IniParameterSpecification.cpp \
               $(CALCULATION)/IniParameterFileAccess.cpp \
               $(CALCULATION)/RunTimeComponents.cpp \
               $(BOOSTDIR)/libs/thread/src/pthread/once.cpp \
               $(BOOSTDIR)/libs/thread/src/pthread/thread.cpp \
               $(BOOSTDIR)/libs/regex/src/c_regex_traits.cpp \
               $(BOOSTDIR)/libs/regex/src/cpp_regex_traits.cpp \
               $(BOOSTDIR)/libs/regex/src/cregex.cpp \
               $(BOOSTDIR)/libs/regex/src/fileiter.cpp \
               $(BOOSTDIR)/libs/regex/src/icu.cpp \
               $(BOOSTDIR)/libs/regex/src/instances.cpp \
               $(BOOSTDIR)/libs/regex/src/posix_api.cpp \
               $(BOOSTDIR)/libs/regex/src/regex.cpp \
               $(BOOSTDIR)/libs/regex/src/regex_debug.cpp \
               $(BOOSTDIR)/libs/regex/src/regex_raw_buffer.cpp \
               $(BOOSTDIR)/libs/regex/src/regex_traits_defaults.cpp \
               $(BOOSTDIR)/libs/regex/src/static_mutex.cpp \
               $(BOOSTDIR)/libs/regex/src/usinstances.cpp \
               $(BOOSTDIR)/libs/regex/src/w32_regex_traits.cpp \
               $(BOOSTDIR)/libs/regex/src/wc_regex_traits.cpp \
               $(BOOSTDIR)/libs/regex/src/wide_posix_api.cpp \
               $(BOOSTDIR)/libs/regex/src/winstances.cpp \
               $(BOOSTDIR)/libs/program_options/src/cmdline.cpp \
               $(BOOSTDIR)/libs/program_options/src/config_file.cpp \
               $(BOOSTDIR)/libs/program_options/src/convert.cpp \
               $(BOOSTDIR)/libs/program_options/src/options_description.cpp \
               $(BOOSTDIR)/libs/program_options/src/positional_options.cpp \
               $(BOOSTDIR)/libs/program_options/src/split.cpp \
               $(BOOSTDIR)/libs/program_options/src/utf8_codecvt_facet.cpp \
               $(BOOSTDIR)/libs/program_options/src/value_semantic.cpp \
               $(BOOSTDIR)/libs/program_options/src/variables_map.cpp \
               $(BOOSTDIR)/libs/chrono/src/chrono.cpp \
               $(BOOSTDIR)/libs/chrono/src/process_cpu_clocks.cpp \
               $(BOOSTDIR)/libs/chrono/src/thread_clock.cpp \
               $(BOOSTDIR)/libs/system/src/error_code.cpp \
               $(BOOSTDIR)/libs/filesystem/src/codecvt_error_category.cpp \
               $(BOOSTDIR)/libs/filesystem/src/operations.cpp \
               $(BOOSTDIR)/libs/filesystem/src/path.cpp \
               $(BOOSTDIR)/libs/filesystem/src/path_traits.cpp \
               $(BOOSTDIR)/libs/filesystem/src/portability.cpp \
               $(BOOSTDIR)/libs/filesystem/src/unique_path.cpp \
               $(BOOSTDIR)/libs/filesystem/src/utf8_codecvt_facet.cpp

APP_SRC     := $(SATSCAN)/batch_application/Main.cpp \
               $(SATSCAN)/batch_application/ParameterProgramOptions.cpp
LIB_SRC     := $(SATSCAN)/shared_library/SharedLibrary.cpp \
               $(SATSCAN)/shared_library/stsJNIPrintWindow.cpp \
               $(SATSCAN)/shared_library/stsParametersUtility.cpp \
               $(SATSCAN)/shared_library/JNIException.cpp \
               $(SATSCAN)/shared_library/PrintCallback.cpp

OBJS        := $(SRC:.cpp=.o)
APP_OBJS    := $(APP_SRC:.cpp=.o)
LIB_OBJS    := $(LIB_SRC:.cpp=.o)

.PHONY: clean all thin

all : $(APPLICATION) $(LINUX_LIBRARY)

thin : all
	strip $(APPLICATION)
	strip $(LINUX_LIBRARY).x.x.0

$(APPLICATION) : $(OBJS) $(APP_OBJS)
	$(CC) $(OBJS) $(APP_OBJS) $(LFLAGS) -o $@

$(MAC_APPLICATION) : $(OBJS) $(APP_OBJS)
	$(CC) $(OBJS) $(APP_OBJS) $(M_LFLAGS) -o $@

$(LINUX_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(L_DLFLAGS) $(OBJS) $(LIB_OBJS) -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -lxbaseg -lnewmat -lshape -lz -lm -lrt -lpthread

$(SOLARIS_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(S_DLFLAGS) $(OBJS) $(LIB_OBJS) -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -lxbaseg -lnewmat -lshape -lz -lm -lrt -lpthread

$(MAC_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(M_DLFLAGS) $(OBJS) $(LIB_OBJS) -L$(XBASEDIR) -L$(XBASEDIR2) -L$(NEWMAT) -L$(SHAPELIB) -L$(ZLIB) -L$(ZLIB_MINIZIP) -lxbase -lnewmat -lshape -lz -lstdc++ -lm -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean :
	rm -f core $(OBJS)
	rm -f $(APPLICATION)
	rm -f core $(APP_OBJS)
	rm -f $(LINUX_LIBRARY).x.x.0
	rm -f $(SOLARIS_LIBRARY).x.x.0
	rm -f core $(LIB_OBJS)
	rm -f $(MAC_APPLICATION)
	rm -f $(MAC_LIBRARY)
