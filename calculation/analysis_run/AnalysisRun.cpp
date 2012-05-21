//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

#include "AnalysisRun.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "stsRunHistoryFile.h"
#include "LoglikelihoodRatioWriter.h"
#include "ClusterInformationWriter.h"
#include "ClusterLocationsWriter.h"
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneAnalysis.h"
#include "PurelyTemporalAnalysis.h"
#include "SpaceTimeAnalysis.h"
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePureAnalysis.h"
#include "SVTTAnalysis.h"
#include "PrintQueue.h"
#include "stsMonteCarloSimFunctor.h"
#include "stsMCSimJobSource.h"
#include "stsCentricAlgoJobSource.h"
#include "stsCentricAlgoFunctor.h"
#include "contractor.h"
#include "PurelySpatialCentricAnalysis.h"
#include "SpaceTimeCentricAnalysis.h"
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalCentricAnalysis.h"
#include "SpaceTimeIncludePureCentricAnalysis.h"
#include "ParametersPrint.h"
#include "SSException.h" 
#include "SVTTCentricAnalysis.h"
#include "PurelySpatialBruteForceAnalysis.h"
//#include "ClusterScatterChart.h"
//#include "ClusterKML.h"

/** constructor */
AnalysisRunner::AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection)
               :gParameters(Parameters), gStartTime(StartTime), gPrintDirection(PrintDirection),
                geExecutingType(Parameters.GetExecutionType()), giAnalysisCount(0), giPower_X_Count(0), giPower_Y_Count(0),
                gdMinRatioToReport(0.001), guwSignificantAt005(0), _clustersReported(false), _reportClusters(0)
 {
  try {
    macroRunTimeManagerInit();
    Setup();
    Execute();
  } catch (prg_exception& x) {
    x.addTrace("constructor()","AnalysisRunner");
    throw;
  }
}

/** calculates most likely clusters in real data */
void AnalysisRunner::CalculateMostLikelyClusters() {
  try {
    //display process heading
    PrintFindClusterHeading();
    //allocate date gateway object
    std::auto_ptr<AbstractDataSetGateway> pDataSetGateway(gpDataHub->GetDataSetHandler().GetNewDataGatewayObject());
    gpDataHub->GetDataSetHandler().GetDataGateway(*pDataSetGateway);
    //get analysis object
    std::auto_ptr<CAnalysis> pAnalysis(GetNewAnalysisObject());
    //allocate objects used in 'FindTopClusters()' process
    pAnalysis->AllocateTopClustersObjects(*pDataSetGateway);
    //calculate most likely clusters
    gpDataHub->SetActiveNeighborReferenceType(CSaTScanData::REPORTED);
    pAnalysis->FindTopClusters(*pDataSetGateway, gTopClustersContainers);
    //display the loglikelihood of most likely cluster
    if (!gPrintDirection.GetIsCanceled()) {
      rankClusterCollections();
    }
  } catch (prg_exception& x) {
    x.addTrace("CalculateMostLikelyClusters()","AnalysisRunner");
    throw;
  }
}

/** Returns indication of whether analysis is to be cancelled.
    Indication returned is false if:
    - the number of simulations requested have been completed
    - the user did not request early termination option
    - no 'most likely clusters' were retained
    Indication returned is true if:
    - number of simulation llr values greater than mlc llr is more than defined threshold. */
bool AnalysisRunner::CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const {
  double fCutOff;

  if (iNumSimulationsCompleted == gParameters.GetNumReplicationsRequested())
    return false;
  if (!gParameters.GetTerminateSimulationsEarly())
    return false;
  return  gSimVars.get_greater_llr_count() >= gParameters.GetExecuteEarlyTermThreshold();
}

/** Creates 'relative risks' output file(s) if requested by user through
    parameter settings. The created file details the relative risk for each
    location specified in coordinates file. Files types creates are ASCII text
    and dBase, with the user's parameter settings indicating which.
    Note that relative risk information is printed for only first iteration of
    iterative scan. This is primarily to prevent division by zero, since the
    squential scan feature zeros out data used in creating relative risks. */
void AnalysisRunner::CreateRelativeRiskFile() {
  macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);

  try {
    if (giAnalysisCount == 1 && gParameters.GetOutputRelativeRisksFiles()) {
      gPrintDirection.Printf("Reporting relative risk estimates...\n", BasePrint::P_STDOUT);
      gpDataHub->DisplayRelativeRisksForEachTract();
    }
  } catch (prg_exception& x) {
    x.addTrace("CreateRelativeRiskFile()","AnalysisRunner");
    throw;
  }

  macroRunTimeStopSerial();
}

/** Creates/overwrites result file specified by user in parameter settings. Only
    header summary information is printed. File pointer does not remain open. */
void AnalysisRunner::CreateReport() {
  macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
  FILE       * fp=0;
  std::string  sStartTime;

  try {
    OpenReportFile(fp, false);
    AsciiPrintFormat::PrintVersionHeader(fp);
    sStartTime = ctime(&gStartTime);
    fprintf(fp,"\nProgram run on: %s\n", sStartTime.c_str());
    ParametersPrint(gParameters).PrintAnalysisSummary(fp);
    ParametersPrint(gParameters).PrintAdjustments(fp, gpDataHub->GetDataSetHandler());
    gpDataHub->DisplaySummary(fp, "SUMMARY OF DATA", true);
    fclose(fp); fp=0;
  }
  catch (prg_exception& x) {
    if (fp) fclose(fp);
    x.addTrace("CreateReport()","AnalysisRunner");
    throw;
  }

  macroRunTimeStopSerial();
}

/** Executes analysis - conditionally running successive or centric processes. */
void AnalysisRunner::Execute() {
  try {
    //read data
    macroRunTimeStartSerial(SerialRunTimeComponent::DataRead);
    gpDataHub->ReadDataFromFiles();
    //validate that data set contains cases
    for (unsigned int i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i)
       if (gpDataHub->GetDataSetHandler().GetDataSet(i).getTotalCases() == 0)
         throw resolvable_error("Error: No cases found in data set %u.\n", i);
    macroRunTimeStopSerial();
    //calculation approxiate amount of memory required to run analysis
    std::pair<double, double> prMemory = GetMemoryApproxiation();
    if (geExecutingType == AUTOMATIC) {
      //prefer successive execution if: enough RAM, or memory needs less than centric, or centric execution not a valid option given parameters
      if (prMemory.first < GetAvailablePhysicalMemory() || prMemory.first < prMemory.second || !gParameters.GetPermitsCentricExecution())
        geExecutingType = SUCCESSIVELY;
      else
        geExecutingType = CENTRICALLY;
    }
    //start execution of analysis
    try {
      switch (geExecutingType) {
        case CENTRICALLY  : //gPrintDirection.Printf("Centric execution, using approxiately %.0lf MB of memory...\n", BasePrint::P_STDOUT, prMemory.second);
                            ExecuteCentricEvaluation(); break;
        case SUCCESSIVELY :
        default           : //gPrintDirection.Printf("Successive execution, using approxiately %.0lf MB of memory...\n", BasePrint::P_STDOUT, prMemory.first);
                            ExecuteSuccessively(); break;
      };
    }
    catch (std::bad_alloc &b) {
      std::string additional;
      //Potentially provide detailed options given user parameter settings:
      if (geExecutingType == SUCCESSIVELY && 
          gParameters.GetPermitsCentricExecution(true) &&
          gParameters.GetPValueReportingType() == TERMINATION_PVALUE && gParameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
              additional = "\nNote: SaTScan could not utilize the alternative memory allocation for\n"
                           "this analysis because of the P-Value reporting setting (sequential Monte Carlo).\n"
                           "Consider changing this setting, which will enable analysis to utilize the\n"
                           "alternative memory allocation and possibly execute without memory issues.\n";
      }

      throw resolvable_error("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                             "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                             "Note that memory needs are on the order of %.0lf MB.\n%s",
                             (geExecutingType == SUCCESSIVELY ? prMemory.first : prMemory.second), additional.c_str());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("Execute()","AnalysisRunner");
    throw;
  }
}

/** starts analysis execution - evaluating real data then replications */
void AnalysisRunner::ExecuteSuccessively() {
  try {
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //calculate number of neighboring locations about each centroid
    macroRunTimeStartSerial(SerialRunTimeComponent::NeighborCalcuation);
    if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON)
      gpDataHub->FindNeighbors();
    macroRunTimeStopSerial();
    //detect cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //create result file report
    CreateReport();
    //start analyzing data
    do {
      ++giAnalysisCount;
      guwSignificantAt005 = 0;
      giPower_X_Count = giPower_Y_Count = 0;
      //calculate most likely clusters
      macroRunTimeStartSerial(SerialRunTimeComponent::RealDataAnalysis);
      CalculateMostLikelyClusters();
      macroRunTimeStopSerial();
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
      gSimVars.reset(gParameters.GetNumReplicationsRequested() > 0 && getLargestMaximaClusterCollection().GetNumClustersRetained() > 0 ? getLargestMaximaClusterCollection().GetTopRankedCluster().GetRatio() : 0.0);
      //Do Monte Carlo replications.
      if (getLargestMaximaClusterCollection().GetNumClustersRetained())
        ExecuteSuccessiveSimulations();
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
      // report clusters to output files
      reportClusters();
      //log history for first analysis run
      if (giAnalysisCount == 1) {
        LogRunHistory();
      }
      //report additional output file: 'relative risks for each location'
      CreateRelativeRiskFile();
      if (gPrintDirection.GetIsCanceled()) return;
    } while (RepeatAnalysis()); //repeat analysis - iterative scan
    //finish report
    FinalizeReport();
  } catch (prg_exception& x) {
    x.addTrace("ExecuteSuccessively()","AnalysisRunner");
    throw;
  }
}

/** Returns the most likely cluster collection associated with largest spatial maxima. */
const MostLikelyClustersContainer & AnalysisRunner::getLargestMaximaClusterCollection() const {
    return _reportClusters.GetNumClustersRetained() ? _reportClusters : gTopClustersContainers.back();
}

/** Finalizes the reporting to result output file.
    - indicates whether clusters were found
    - indicates whether no clusters were reported because their loglikelihood
      ratios are less than defined minimum value
    - if the number of simulations are less 98, reported that the reported
      clusters intentially do not contain p-values */
void AnalysisRunner::FinalizeReport() {
  FILE              * fp=0;
  time_t              CompletionTime;
  double              nTotalTime,  nSeconds,  nMinutes,  nHours;
  const char        * szHours = "hours";
  const char        * szMinutes = "minutes";
  const char        * szSeconds = "seconds";
  AsciiPrintFormat    PrintFormat;
  std::string         buffer;

  try {
    gPrintDirection.Printf("Printing analysis settings to the results file...\n", BasePrint::P_STDOUT);
    OpenReportFile(fp, true);
    PrintFormat.SetMarginsAsOverviewSection();
    if (_clustersReported && gParameters.GetNumReplicationsRequested() == 0) {
      fprintf(fp, "\n");
      buffer = "Note: As the number of Monte Carlo replications was set to "
               "zero, no hypothesis testing was done and no p-values are reported.";
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
    if (_clustersReported && gParameters.GetNumReplicationsRequested() > 0 && gParameters.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE) {
      fprintf(fp, "\n");
      buffer = "Note: The number of Monte Carlo replications was set too low, "
               "and a meaningful hypothesis test cannot be done. Consequently, "
               "no p-values are reported.";
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
    if (gParameters.GetProbabilityModelType() == POISSON)
      gpDataHub->GetDataSetHandler().ReportZeroPops(*gpDataHub, fp, &gPrintDirection);
    gpDataHub->GetTInfo()->reportCombinedLocations(fp);
    ParametersPrint(gParameters).Print(fp);
    macroRunTimeManagerPrint(fp);
    time(&CompletionTime);
    nTotalTime = difftime(CompletionTime, gStartTime);
    nHours     = floor(nTotalTime/(60*60));
    nMinutes   = floor((nTotalTime - nHours*60*60)/60);
    nSeconds   = nTotalTime - (nHours*60*60) - (nMinutes*60);
    fprintf(fp,"\nProgram completed  : %s", ctime(&CompletionTime));
    if (0 < nHours && nHours < 1.5)
      szHours = "hour";
    if (0 < nMinutes && nMinutes < 1.5)
      szMinutes = "minute";
    if (0.5 <= nSeconds && nSeconds < 1.5)
      szSeconds = "second";
    if (nHours > 0)
      fprintf(fp,"Total Running Time : %.0f %s %.0f %s %.0f %s", nHours, szHours,
              nMinutes, szMinutes, nSeconds, szSeconds);
    else if (nMinutes > 0)
      fprintf(fp,"Total Running Time : %.0f %s %.0f %s", nMinutes, szMinutes, nSeconds, szSeconds);
    else
      fprintf(fp,"Total Running Time : %.0f %s",nSeconds, szSeconds);

    if (gParameters.GetNumParallelProcessesToExecute() > 1)
      fprintf(fp,"\nProcessor Usage    : %u processors", gParameters.GetNumParallelProcessesToExecute());

    fclose(fp); fp=0;
  }
  catch (prg_exception& x) {
    if (fp) fclose(fp);
    x.addTrace("FinalizeReport()","AnalysisRunner");
    throw;
  }
}

/** Returns available random access memory om mega-bytes. */
double AnalysisRunner::GetAvailablePhysicalMemory() const {
  double /*dTotalPhysicalMemory(0),*/ dAvailablePhysicalMemory(0);

  //need process for handling failure from system call

#ifdef _WINDOWS_
  MEMORYSTATUS stat;
  GlobalMemoryStatus (&stat);
  //dTotalPhysicalMemory = stat.dwTotalPhys;
  dAvailablePhysicalMemory = stat.dwAvailPhys;
#elif defined(__APPLE__)
    int physmem;
    size_t len = sizeof physmem;
    static int mib[2] = { CTL_HW, HW_USERMEM };
    if (sysctl (mib, 2, &physmem, &len, NULL, 0) == 0 && len == sizeof (physmem)) {
        dAvailablePhysicalMemory = static_cast<double>(physmem);
    }
#else
  //dTotalPhysicalMemory = sysconf(_SC_PHYS_PAGES);
  //dTotalPhysicalMemory *= sysconf(_SC_PAGESIZE);
  dAvailablePhysicalMemory = sysconf(_SC_AVPHYS_PAGES);
  dAvailablePhysicalMemory *= sysconf(_SC_PAGESIZE);
#endif

  return std::ceil(dAvailablePhysicalMemory/1000000);
}

/** Approxiates the amount of memory (in MB) that will be required to run this ananlysis,
    both Standard Memory Allocation and Special Memory Allocation. Note that these formulas
    are largely identical to that of user guide. */
std::pair<double, double> AnalysisRunner::GetMemoryApproxiation() const {
  std::pair<double, double>  prMemoryAppoxiation;

   //the number of location IDs in the coordinates file
  double L = gpDataHub->GetNumTracts();
  //the number of coordinates in the grid file (G=L if no grid file is specified)
  //double G = gpDataHub->GetGInfo()->getNumGridPoints();
  //maximum geographical cluster size, as a proportion of the population ( 0 < mg = ½ , mg=1 for a purely temporal analysis)
  double mg=0;
  if (gParameters.GetIsPurelyTemporalAnalysis()) mg = 0.0;
  else if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) {
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false))
      mg = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
    else
      mg = gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
  }
  else if (gParameters.UseLocationNeighborsFile()) mg = 0.25; // pure guess
  else mg = gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false)/100.0;
  //number of time intervals into which the temporal data is aggregated (TI=1 for a purely spatial analysis)               
  double TI = gpDataHub->GetNumTimeIntervals();
  //read data structures
  double b=1;
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON:
    case SPACETIMEPERMUTATION:
    case EXPONENTIAL:  b = sizeof(count_t) + sizeof(measure_t); break;
    case BERNOULLI: b = 2 * sizeof(count_t) + sizeof(measure_t); break;
    case CATEGORICAL:
    case ORDINAL: b = sizeof(count_t); break;
    case NORMAL: b = sizeof(count_t) + sizeof(measure_t) + sizeof(measure_t); break;
    case HOMOGENEOUSPOISSON: b = 1; break; // ??
    default : throw prg_error("Unknown model type '%d'.\n", "GetMemoryApproxiation()", gParameters.GetProbabilityModelType());
  };
  //the number of categories in the ordinal model (CAT=1 for other models)
  double CAT = (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL ? 0 : 1);
  for (size_t i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i)
     CAT += gpDataHub->GetDataSetHandler().GetDataSet(i).getPopulationData().GetNumOrdinalCategories();
  //for exponential model, EXP =1 one for all other models
  double EXP = 1; //EXP is mulitplied by 4 bytes
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON:
    case SPACETIMEPERMUTATION:
    case BERNOULLI:
    case HOMOGENEOUSPOISSON: 
    case CATEGORICAL:
    case ORDINAL: EXP = 1; break;
    case EXPONENTIAL: EXP = 3; break; //cases and measure
    case NORMAL: EXP = 4; break; //cases, measure and measure squared
    default : throw prg_error("Unknown model type '%d'.\n", "GetMemoryApproxiation()", gParameters.GetProbabilityModelType());
  };
  //the total number of cases (for the ordinal model or multiple data sets, C=0)
  double C = (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL || gpDataHub->GetDataSetHandler().GetNumDataSets() > 1 ? 0 : gpDataHub->GetDataSetHandler().GetDataSet(0).getTotalCases());
  //1 when scanning for high rates only or low rates only, R=2 when scanning for either high or low rates
  double R = (gParameters.GetAreaScanRateType() == HIGHANDLOW ? 2 : 1);
  //number of data sets
  double D = gpDataHub->GetDataSetHandler().GetNumDataSets();
  //number of processors available on the computer for SaTScan use
  double P = gParameters.GetNumParallelProcessesToExecute();
  //is the number of Monte Carlo simulations
  double MC = gParameters.GetNumReplicationsRequested();
  //sort array data type size
  double SortedDataTypeSize(gpDataHub->GetNumTracts() < (int)std::numeric_limits<unsigned short>::max() ? sizeof(unsigned short) : sizeof(int));
  //size of sorted array -- this formula deviates from the user guide slightly
  double SortedNeighborsArray = (gParameters.GetIsPurelyTemporalAnalysis() ? 0 :
                                 (double)sizeof(void**) * (gParameters.GetNumTotalEllipses()+1) +
                                 (double)sizeof(void*) * (gParameters.GetNumTotalEllipses()+1) * gpDataHub->m_nGridTracts +
                                 (double)(gParameters.GetNumTotalEllipses()+1) * gpDataHub->m_nGridTracts * SortedDataTypeSize * gpDataHub->GetNumTracts() * mg);
  //Standard Memory Allocation
  prMemoryAppoxiation.first = std::ceil((SortedNeighborsArray + (b + 4.0 * EXP * P) * L * TI * CAT * D + sizeof(measure_t) * C * R * P)/1000000);
  //Special Memory Allocation
  prMemoryAppoxiation.second = std::ceil((4 * L * TI * CAT * EXP * D * MC + sizeof(measure_t) * MC * C * R * P)/1000000);
  return prMemoryAppoxiation;
}

/** returns new CAnalysis object */
CAnalysis * AnalysisRunner::GetNewAnalysisObject() const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL :
          if (gParameters.GetRiskType() == STANDARDRISK) {
              if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)
                return new CPurelySpatialBruteForceAnalysis(gParameters, *gpDataHub, gPrintDirection);
              else
                return new CPurelySpatialAnalysis(gParameters, *gpDataHub, gPrintDirection);
          }
          else
            return new CPSMonotoneAnalysis(gParameters, *gpDataHub, gPrintDirection);
      case PURELYTEMPORAL :
      case PROSPECTIVEPURELYTEMPORAL :
          return new CPurelyTemporalAnalysis(gParameters, *gpDataHub, gPrintDirection);
      case SPACETIME :
      case PROSPECTIVESPACETIME :
          if (gParameters.GetIncludePurelySpatialClusters() && gParameters.GetIncludePurelyTemporalClusters())
            return new C_ST_PS_PT_Analysis(gParameters, *gpDataHub, gPrintDirection);
          else if (gParameters.GetIncludePurelySpatialClusters())
            return new C_ST_PS_Analysis(gParameters, *gpDataHub, gPrintDirection);
          else if (gParameters.GetIncludePurelyTemporalClusters())
            return new C_ST_PT_Analysis(gParameters, *gpDataHub, gPrintDirection);
          else
            return new CSpaceTimeAnalysis(gParameters, *gpDataHub, gPrintDirection);
      case SPATIALVARTEMPTREND :
          return new CSpatialVarTempTrendAnalysis(gParameters, *gpDataHub, gPrintDirection);
      default :
        throw prg_error("Unknown analysis type '%d'.\n", "GetNewCentricAnalysisObject()", gParameters.GetAnalysisType());
    };
   }
  catch (prg_exception& x) {
    x.addTrace("GetNewAnalysisObject()","AnalysisRunner");
    throw;
  }
}

/** returns new AbstractCentricAnalysis object */
AbstractCentricAnalysis * AnalysisRunner::GetNewCentricAnalysisObject(const AbstractDataSetGateway& RealDataGateway,
                                                                      const ptr_vector<AbstractDataSetGateway>& vSimDataGateways) const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL :
          if (gParameters.GetRiskType() == STANDARDRISK)
            return new PurelySpatialCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
          else
            throw prg_error("No implementation for purely spatial analysis with isotonic scan for centric evaluation.\n", "GetNewCentricAnalysisObject()");
      case PURELYTEMPORAL :
      case PROSPECTIVEPURELYTEMPORAL :
            throw prg_error("No implementation for purely temporal analysis for centric evaluation.\n", "GetNewCentricAnalysisObject()");
      case SPACETIME :
      case PROSPECTIVESPACETIME :
          if (gParameters.GetIncludePurelySpatialClusters() && gParameters.GetIncludePurelyTemporalClusters())
            return new SpaceTimeIncludePureCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
          else if (gParameters.GetIncludePurelySpatialClusters())
            return new SpaceTimeIncludePurelySpatialCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
          else if (gParameters.GetIncludePurelyTemporalClusters())
            return new SpaceTimeIncludePurelyTemporalCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
          else
            return new SpaceTimeCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
      case SPATIALVARTEMPTREND :
            return new SpatialVarTempTrendCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
      default :
        throw prg_error("Unknown analysis type '%d'.\n", "GetNewCentricAnalysisObject()", gParameters.GetAnalysisType());
    };
   }
  catch (prg_exception& x) {
    x.addTrace("GetNewCentricAnalysisObject()","AnalysisRunner");
    throw;
  }
}

double AnalysisRunner::GetSimRatio01() const {
  return gpSignificantRatios.get() ? gpSignificantRatios->GetAlpha01() : 0;
}
double AnalysisRunner::GetSimRatio05() const {
  return gpSignificantRatios.get() ? gpSignificantRatios->GetAlpha05() : 0;
}

/** Logs run to history file. */
void  AnalysisRunner::LogRunHistory() {
    try {
      if (gParameters.GetIsLoggingHistory()) {
        gPrintDirection.Printf("Logging run history...\n", BasePrint::P_STDOUT);
        macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
        stsRunHistoryFile(gParameters, gPrintDirection).LogNewHistory(*this);
        macroRunTimeStopSerial();
      }
    } catch (...) {
        // If fails for any reason, notify user and continue ...
        gPrintDirection.Printf("Notice: Logging run history failed.\n", BasePrint::P_NOTICE);
    }
}

/** Attempts to open result output file stream and assign to passed file pointer
    address. Open mode is determined to boolean paramter. */
void AnalysisRunner::OpenReportFile(FILE*& fp, bool bOpenAppend) {
  try {
    if ((fp = fopen(gParameters.GetOutputFileName().c_str(), (bOpenAppend ? "a" : "w"))) == NULL) {
      if (!bOpenAppend)
        throw resolvable_error("Error: Results file '%s' could not be created.\n", gParameters.GetOutputFileName().c_str());
      else if (bOpenAppend)
        throw resolvable_error("Error: Results file '%s' could not be opened.\n", gParameters.GetOutputFileName().c_str());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("OpenReportFile()","AnalysisRunner");
    throw;
  }
}

/** starts analysis execution - development */
void AnalysisRunner::ExecuteCentricEvaluation() {
  unsigned long         ulParallelProcessCount = std::min(gParameters.GetNumParallelProcessesToExecute(), static_cast<unsigned>(gpDataHub->m_nGridTracts));
  DataSetHandler      & DataHandler = gpDataHub->GetDataSetHandler();

  try {
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled()) return;
    //create result file report
    CreateReport();
    //start analyzing data
    do {
      ++giAnalysisCount;
      guwSignificantAt005 = 0;
      giPower_X_Count = giPower_Y_Count = 0;
      gSimVars.reset(0.0);

      //simualtion data randomizer
      RandomizerContainer_t                       RandomizationContainer;
      //allocate a simulation data set for each requested replication
      std::vector<SimulationDataContainer_t>      vRandomizedDataSets(gParameters.GetNumReplicationsRequested());
      //allocate a data gateway for each requested replication
      ptr_vector<AbstractDataSetGateway>          vSimDataGateways(gParameters.GetNumReplicationsRequested());
      //allocate an array to contain simulation llr values
      AbstractCentricAnalysis::CalculatedRatioContainer_t SimulationRatios;
      //data gateway object for real data
      std::auto_ptr<AbstractDataSetGateway>        DataSetGateway(DataHandler.GetNewDataGatewayObject());
      std::auto_ptr<LoglikelihoodRatioWriter>      RatioWriter;
      std::auto_ptr<AbstractDataSetWriter>         DataSetWriter;

      //get data randomizers
      DataHandler.GetRandomizerContainer(RandomizationContainer);
      //set data gateway object
      DataHandler.GetDataGateway(*DataSetGateway);
      if (gParameters.GetNumReplicationsRequested())
        gPrintDirection.Printf("Calculating simulation data for %u simulations\n\n", BasePrint::P_STDOUT, gParameters.GetNumReplicationsRequested());
      if (gParameters.GetOutputSimulationData()) {
        remove(gParameters.GetSimulationDataOutputFilename().c_str());
        DataSetWriter.reset(AbstractDataSetWriter::getNewDataSetWriter(gParameters));
      }
      //create simulation data sets -- randomize each and set corresponding data gateway object
      for (unsigned int i=0; i < gParameters.GetNumReplicationsRequested() && !gPrintDirection.GetIsCanceled(); ++i) {
         SimulationDataContainer_t& thisDataCollection = vRandomizedDataSets[i];
         //create new simulation data set object for each data set of this simulation
         for (unsigned int j=0; j < DataHandler.GetNumDataSets(); ++j)
			 thisDataCollection.push_back(new DataSet(gpDataHub->GetNumTimeIntervals(), gpDataHub->GetNumTracts(),  gpDataHub->GetNumMetaTractsReferenced(), gParameters, j + 1));
         //allocate appropriate data structure for given data set handler (probablility model)
         DataHandler.AllocateSimulationData(thisDataCollection);
         //randomize data
         macroRunTimeStartSerial(SerialRunTimeComponent::RandomDataGeneration);
         gpDataHub->RandomizeData(RandomizationContainer, thisDataCollection, i + 1);
         macroRunTimeStopSerial();
         //print simulation data to file, if requested
         if (gParameters.GetOutputSimulationData())
           for (size_t t=0; t < thisDataCollection.size(); ++t)
              DataSetWriter->write(*thisDataCollection[t], gParameters);
         //allocate and set data gateway object
         vSimDataGateways[i] = DataHandler.GetNewDataGatewayObject();
         DataHandler.GetSimulationDataGateway(*vSimDataGateways[i], thisDataCollection, RandomizationContainer);
      }
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled()) return;

      //construct centric-analyses and centroid calculators for each thread:
      std::deque<boost::shared_ptr<AbstractCentricAnalysis> > seqCentricAnalyses(ulParallelProcessCount);
      std::deque<boost::shared_ptr<CentroidNeighborCalculator> > seqCentroidCalculators(ulParallelProcessCount);
      for (unsigned u=0; u<ulParallelProcessCount; ++u) {
        seqCentricAnalyses[u].reset(GetNewCentricAnalysisObject(*DataSetGateway, vSimDataGateways));
        seqCentroidCalculators[u].reset(new CentroidNeighborCalculator(*gpDataHub, gPrintDirection));
      }

      //analyze real and simulation data about each centroid
      {
        stsCentricAlgoJobSource::result_type purelyTemporalExecutionExceptionStatus;//if (.first) then (.second) is the exception message and callpath.
        PrintQueue tmpPrintDirection(gPrintDirection, gParameters.GetSuppressingWarnings());
        AsynchronouslyAccessible<PrintQueue> tmpThreadsafePrintDirection(tmpPrintDirection);
        stsCentricAlgoJobSource jobSource(gpDataHub->m_nGridTracts, ::GetCurrentTime_HighResolution(), tmpThreadsafePrintDirection);
        typedef contractor<stsCentricAlgoJobSource> contractor_type;
        contractor_type theContractor(jobSource);
        //run threads:
        boost::thread_group tg;
        boost::mutex        thread_mutex;
        unsigned uThreadIdx = 0;
        if (gParameters.GetIncludePurelyTemporalClusters()) {
          //launch specialized first thread:
          stsPurelyTemporal_Plus_CentricAlgoThreadFunctor Functor(theContractor, jobSource, purelyTemporalExecutionExceptionStatus,
                                                                  tmpThreadsafePrintDirection, *(seqCentricAnalyses[uThreadIdx]),
                                                                  *(seqCentroidCalculators[uThreadIdx]), *DataSetGateway, vSimDataGateways);
          tg.create_thread(Functor);
          ++uThreadIdx;
        }
        //launch the remaining threads:
        for (; uThreadIdx < ulParallelProcessCount; ++uThreadIdx) {
          stsCentricAlgoFunctor mcsf(*(seqCentricAnalyses[uThreadIdx]), *(seqCentroidCalculators[uThreadIdx]), *DataSetGateway, vSimDataGateways);
          tg.create_thread(subcontractor<contractor_type,stsCentricAlgoFunctor>(theContractor,mcsf));
        }
        tg.join_all();

        // Since we are evaluating real and simulation data simultaneuosly, there is no early termination option. 
        gSimVars.set_sim_count_explicit(gParameters.GetNumReplicationsRequested());

        //propagate exceptions if needed:
        if (gParameters.GetIncludePurelyTemporalClusters() && purelyTemporalExecutionExceptionStatus.bExceptional) {
          if (purelyTemporalExecutionExceptionStatus.eException_type == stsCentricAlgoJobSource::result_type::memory)
            throw memory_exception(purelyTemporalExecutionExceptionStatus.Exception.what());
          throw purelyTemporalExecutionExceptionStatus.Exception;
        }  
        jobSource.Assert_NoExceptionsCaught();
        if (jobSource.GetUnregisteredJobCount() > 0)
          throw prg_error("At least %d jobs remain uncompleted.", "AnalysisRunner", jobSource.GetUnregisteredJobCount());
      }
      if (gPrintDirection.GetIsCanceled()) return;
      //retrieve top clusters and simulated loglikelihood ratios from analysis object
      for (unsigned u=0; u<ulParallelProcessCount; ++u) {
        seqCentricAnalyses[u]->RetrieveClusters(gTopClustersContainers);
        seqCentricAnalyses[u]->RetrieveLoglikelihoodRatios(SimulationRatios);
      }
      //free memory of objects that will no longer be used
      // - we might need the memory for recalculating neighbors in geographical overlap code
      vRandomizedDataSets.clear();
      seqCentricAnalyses.clear();
      seqCentroidCalculators.clear();
      vSimDataGateways.killAll();
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
      //rank top clusters and apply criteria for reporting secondary clusters
      rankClusterCollections();
      //report calculated simulation llr values
      if (SimulationRatios) {
        if (GetIsCalculatingSignificantRatios()) gpSignificantRatios->Initialize();
        RatioWriter.reset(new LoglikelihoodRatioWriter(gParameters, giAnalysisCount > 1));
        std::vector<double>::const_iterator itr=SimulationRatios->begin(), itr_end=SimulationRatios->end();
        for (; itr != itr_end; ++itr) {
          //update most likely clusters given latest simulated loglikelihood ratio
          _clusterRanker.update(*itr);
          //update significance indicator
          UpdateSignificantRatiosList(*itr);
          //update power calculations
          UpdatePowerCounts(*itr);
          //update simulation variables
          gSimVars.add_llr(*itr);
          //update simulated loglikelihood record buffer
          if(RatioWriter.get()) RatioWriter->Write(*itr);
        }
        SimulationRatios.reset();
      }  
      //report clusters to output files
      reportClusters();
      //log history for first analysis run
      if (giAnalysisCount == 1) {
        LogRunHistory();
      }
      //report additional output file: 'relative risks for each location'
      CreateRelativeRiskFile();
      if (gPrintDirection.GetIsCanceled()) return;
    } while (RepeatAnalysis() == true); //repeat analysis - iterative scan

    //finish report
    FinalizeReport();
  } catch (prg_exception& x) {
    x.addTrace("ExecuteCentricEvaluation()","AnalysisRunner");
    throw;
  }
}

/** Calculates simulated loglikelihood ratios and updates:
    - most likely clusters rank
    - significant loglikelihood ratio indicator
    - power calculation data, if requested by user
    - additional output file(s)
*****************************************************
*/
void AnalysisRunner::PerformSuccessiveSimulations_Parallel() {
  char                * sReplicationFormatString;
  unsigned long         ulParallelProcessCount = std::min(gParameters.GetNumParallelProcessesToExecute(), gParameters.GetNumReplicationsRequested());

  try {
    if (gParameters.GetNumReplicationsRequested() == 0)
      return;
    //set print message format string
    if (gParameters.GetLogLikelihoodRatioIsTestStatistic())
      sReplicationFormatString = "SaTScan test statistic for #%u of %u replications: %7.2lf\n";
    else
      sReplicationFormatString = "SaTScan log likelihood ratio for #%u of %u replications: %7.2lf\n";
    //set/reset loglikelihood ratio significance indicator
    if (GetIsCalculatingSignificantRatios()) gpSignificantRatios->Initialize();
    //if writing simulation data to file, delete file now
    if (gParameters.GetOutputSimulationData())
      remove(gParameters.GetSimulationDataOutputFilename().c_str());

    {
      PrintQueue lclPrintDirection(gPrintDirection, gParameters.GetSuppressingWarnings());
      stsMCSimJobSource jobSource(gParameters, ::GetCurrentTime_HighResolution(), lclPrintDirection, sReplicationFormatString, *this);
      typedef contractor<stsMCSimJobSource> contractor_type;
      contractor_type theContractor(jobSource);
      //run threads:
      boost::thread_group tg;
      boost::mutex        thread_mutex;
      for (unsigned u=0; u < ulParallelProcessCount; ++u) {
         try {
            stsMCSimSuccessiveFunctor mcsf(thread_mutex, GetDataHub(), boost::shared_ptr<CAnalysis>(GetNewAnalysisObject()), boost::shared_ptr<SimulationDataContainer_t>(new SimulationDataContainer_t()), boost::shared_ptr<RandomizerContainer_t>(new RandomizerContainer_t()));
            tg.create_thread(subcontractor<contractor_type,stsMCSimSuccessiveFunctor>(theContractor,mcsf));
         } catch (std::bad_alloc &b) {             
             if (u == 0) throw; // if this is the first thread, re-throw exception
             gPrintDirection.Printf("Notice: Insufficient memory to create %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", 
                                    BasePrint::P_NOTICE, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
             break;
         } catch (prg_exception& x) {
             if (u == 0) throw; // if this is the first thread, re-throw exception
             gPrintDirection.Printf("Error: Program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\nException:%s\n", 
                                    BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u, x.what());
             break;
         } catch (...) {
             if (u == 0) throw prg_error("Unknown program error occurred.\n","PerformSuccessiveSimulations_Parallel()"); // if this is the first thread, throw exception
             gPrintDirection.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", 
                                    BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
             break;
         }
      }

      tg.join_all();

      //propagate exceptions if needed:
      theContractor.throw_unhandled_exception();
      jobSource.Assert_NoExceptionsCaught();
      if (jobSource.GetUnregisteredJobCount() > 0)
        throw prg_error("At least %d jobs remain uncompleted.", "AnalysisRunner", jobSource.GetUnregisteredJobCount());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("PerformSuccessiveSimulations_Parallel()","AnalysisRunner");
    throw;
  }
}

/** Prepares data for simulations and contracts simulation process. */
void AnalysisRunner::ExecuteSuccessiveSimulations() {
  try {                                                           
    if (gParameters.GetNumReplicationsRequested() > 0) {
      //recompute neighbors if settings indicate that smaller clusters are reported
      gpDataHub->SetActiveNeighborReferenceType(CSaTScanData::MAXIMUM);
      gPrintDirection.Printf("Doing the Monte Carlo replications\n", BasePrint::P_STDOUT);
      PerformSuccessiveSimulations_Parallel();
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ExecuteSuccessiveSimulations()","AnalysisRunner");
    throw;
  }
}

/** Prints calculated critical values to report file. */
void AnalysisRunner::PrintCriticalValuesStatus(FILE* fp) {
  AsciiPrintFormat      PrintFormat;
  std::string           buffer;

  if (GetIsCalculatingSignificantRatios() && gSimVars.get_sim_count() >= 19) {
    PrintFormat.SetMarginsAsOverviewSection();
    fprintf(fp,"\n");
    printString(buffer, "A cluster is statistically significant when its %s "
                        "is greater than the critical value, which is, for significance level:",
                        (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"));
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    if (gSimVars.get_sim_count() >= 99)
      fprintf(fp,"... 0.01: %f\n", gpSignificantRatios->GetAlpha01());
    if (gSimVars.get_sim_count() >= 19)
      fprintf(fp,"... 0.05: %f\n", gpSignificantRatios->GetAlpha05());
  }
}

/** Prints early termination status to report file. */
void AnalysisRunner::PrintEarlyTerminationStatus(FILE* fp) {
  bool anyClusters=false;
  for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end() && !anyClusters; ++itrMLC)
      anyClusters = itrMLC->GetNumClustersRetained() > 0; 
  if (anyClusters && gSimVars.get_sim_count() < gParameters.GetNumReplicationsRequested()) {
    fprintf(fp, "\nNOTE: The sequential Monte Carlo procedure was used to terminate\n");
    fprintf(fp, "      the calculations after %u replications.\n", gSimVars.get_sim_count());
  }
}

/** Displays progress information to print direction indicating that analysis
    is calculating the most likely clusters in data. If iterative scan option
    was requested, the message printed reflects which iteration of the scan it
    is performing. */
void AnalysisRunner::PrintFindClusterHeading() {
  if (!gParameters.GetIsIterativeScanning())
    gPrintDirection.Printf("Finding the most likely clusters.\n", BasePrint::P_STDOUT);
  else {
    switch(giAnalysisCount) {
      case  1: gPrintDirection.Printf("Finding the most likely cluster.\n", BasePrint::P_STDOUT); break;
      case  2: gPrintDirection.Printf("Finding the second most likely cluster.\n", BasePrint::P_STDOUT); break;
      case  3: gPrintDirection.Printf("Finding the third most likely cluster.\n", BasePrint::P_STDOUT); break;
      default: gPrintDirection.Printf("Finding the %ith most likely cluster.\n", BasePrint::P_STDOUT, giAnalysisCount);
    }
  }
}

/** Print GINI coefficients */
void AnalysisRunner::PrintGiniCoefficients(FILE* fp) {
  if (!(gParameters.getIsReportingIndexBasedClusters() && gParameters.getOutputIndexBasedCoefficents())) return;
  AsciiPrintFormat printFormat;
  std::string buffer;
  printFormat.SetMarginsAsClusterSection(0);
  printFormat.PrintSectionSeparatorString(fp, 0, 2);
  printString(buffer, "Gini Indexes");
  printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
  printString(buffer, "----------------------------------------------------------");
  printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
  double minSize=std::numeric_limits<double>::max(), maxSize=0, minGINI=std::numeric_limits<double>::max(), maxGINI = 0;
  const MostLikelyClustersContainer* maximizedCollection = 0;
  for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end(); ++itrMLC) {
      printString(buffer, "%g percent", itrMLC->getMaximumWindowSize());
      printFormat.PrintSectionLabel(fp, buffer.c_str(), false, false);  
      double gini = itrMLC->getGiniCoefficient(GetDataHub(), gSimVars, gParameters.getIndexBasedPValueCutoff());
      printFormat.PrintAlignedMarginsDataString(fp, getValueAsString(gini, buffer, 4));
      if (gini > maxGINI) {maximizedCollection = &(*itrMLC); maxGINI = gini;}
      minSize = std::min(minSize, itrMLC->getMaximumWindowSize()); 
      maxSize = std::max(maxSize, itrMLC->getMaximumWindowSize());
      minGINI = std::min(minGINI, gini); 
  }
  if (maximizedCollection) {
    printString(buffer, "Optimal Gini coefficient found at %g%% maxima.", maximizedCollection->getMaximumWindowSize());
    printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
  }
  if (gParameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
    std::string buffer2;
    printString(buffer, "Coefficients based on clusters with p<%s.", getValueAsString(gParameters.getIndexBasedPValueCutoff(), buffer).c_str());
    printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
  }

  // create Gnuplot file for Gini coeffiecents
  std::ofstream pg;
  //open output file
  std::string name(gParameters.GetOutputFileName());
  name += ".pg";
  pg.open(name.c_str());
  if (!pg) throw resolvable_error("Error: Could not open file '%s'.\n", name.c_str());

  pg << "reset" << std::endl << "set terminal pngcairo" << std::endl << "set output '" << gParameters.GetOutputFileName().c_str() << ".png'" << std::endl << "set xtics 5" << std::endl;
  if (maximizedCollection) 
      pg << "set label 1 'Optimal Gini at " << maximizedCollection->getMaximumWindowSize() <<"% maxima.'" << std::endl;
  else
      pg << "set label 1 'No Optimal Gini Found" << std::endl;
  pg << "set label 1 at graph 0.025, 0.95 tc lt 3" << std::endl;
  pg << "set title 'Plot of Spatial Window Size to Gini Coefficient'" << std::endl << "set ylabel 'Gini'" << std::endl << "set yrange [" << std::max(0.0, minGINI - 0.05) << ":" << (maxGINI + 0.05) << "]" << std::endl;
  pg << "set mxtics" << std::endl << "set xlabel 'Spatial Window Size'" << std::endl << "set xrange [" << std::max(0.0, minSize - 2) << ":" << (maxSize + 2) << "]" << std::endl << "set grid" << std::endl;
  pg << "set style data lines" << std::endl << "set style line 1 lt 1 lc rgb '#0000A0' lw 2" << std::endl << "set style line 2 lc rgb '#0000FF' pt 7 ps 1"<< std::endl;
  pg << "set style line 3 lc rgb '#0000A0' pt 7 ps 2" << std::endl << "plot '-' with linespoints ls 1 title 'Gini',";
  // need to determine the maximum gini coefficent
  std::stringstream lineData, pointData;
  for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end(); ++itrMLC) {
      double gini = itrMLC->getGiniCoefficient(GetDataHub(), gSimVars, gParameters.getIndexBasedPValueCutoff());
      pg << "'-' w p ls " << (maximizedCollection == &(*itrMLC) ? 3 : 2) << " notitle" << (itrMLC + 1 != gTopClustersContainers.end() ? "," : "");
      lineData << itrMLC->getMaximumWindowSize() << "  " << gini << std::endl;
      pointData << itrMLC->getMaximumWindowSize() << "  " << gini << std::endl << "e" << std::endl;
  }
  pg << std::endl << lineData.str() << "e" << std::endl << pointData.str() << "#";
  pg.close();

  //change to parameter
  std::string gnuBinary("c:/prj/gnuplot/bin/gnuplot");
  std::stringstream command;
  command << gnuBinary << " " << name;
  if (system (command.str().c_str()))
    gPrintDirection.Printf("Failed to create GINI graphic.\n", BasePrint::P_STDOUT);
  remove(name.c_str());
}

/** Prints power calculations status to report file. */
void AnalysisRunner::PrintPowerCalculationsStatus(FILE* fp) {
  AsciiPrintFormat      PrintFormat;
  std::string           buffer;

  if (gParameters.GetIsPowerCalculated() && gSimVars.get_sim_count()) {
    fprintf(fp, "\n");
    buffer = "Percentage of Monte Carlo replications with a likelihood greater than";
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    fprintf(fp,"... X (%f) : %f\n", gParameters.GetPowerCalculationX(), ((double)giPower_X_Count)/gSimVars.get_sim_count());
    fprintf(fp,"... Y (%f) : %f\n", gParameters.GetPowerCalculationY(), ((double)giPower_Y_Count)/gSimVars.get_sim_count());
  }
}

/** Prints indication of whether no clusters were retained nor reported. */
void AnalysisRunner::PrintRetainedClustersStatus(FILE* fp, bool bClusterReported) {
  AsciiPrintFormat    PrintFormat;
  std::string         buffer;

  PrintFormat.SetMarginsAsOverviewSection();
  //if zero clusters retained in real data, then no clusters of significance were retained.

  bool anyClusters=false;
  for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end() && !anyClusters; ++itrMLC)
      anyClusters = itrMLC->GetNumClustersRetained() > 0; 
  if (!anyClusters) {
    if (gParameters.GetIsIterativeScanning() && giAnalysisCount > 1)
      fprintf(fp, "\nNo further clusters were found.\n");
    else
      fprintf(fp, "\nNo clusters were found.\n");
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON :
      case BERNOULLI :
      case SPACETIMEPERMUTATION :
      case HOMOGENEOUSPOISSON :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : buffer = "All areas scanned had either only one case or an equal or fewer number of cases than expected."; break;
            case LOW        : buffer = "All areas scanned had either only one case or an equal or greater number of cases than expected."; break;
            case HIGHANDLOW : buffer = "All areas scanned had either only one case or an equal cases to expected."; break;
            default : throw prg_error("Unknown area scan rate type '%d'.\n", "PrintRetainedClustersStatus()", gParameters.GetAreaScanRateType());
         }
         break;
      case CATEGORICAL:
            buffer = "All areas scanned had either only one case or an equal number of low or high value cases to expected for any cut-off."; break;
            break;
      case ORDINAL :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : buffer = "All areas scanned had either only one case or an equal or lower number of high value cases than expected for any cut-off."; break;
            case LOW        : buffer = "All areas scanned had either only one case or an equal or higher number of low value cases than expected for any cut-off."; break;
            case HIGHANDLOW : buffer = "All areas scanned had either only one case or an equal number of low or high value cases to expected for any cut-off."; break;
            default : throw prg_error("Unknown area scan rate type '%d'.\n", "PrintRetainedClustersStatus()", gParameters.GetAreaScanRateType());
         }
         break;
      case NORMAL :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : buffer = "All areas scanned had either only one case or an equal or lower mean than outside the area."; break;
            case LOW        : buffer = "All areas scanned had either only one case or an equal or higher mean than outside the area."; break;
            case HIGHANDLOW : buffer = "All areas scanned had either only one case or an equal mean to outside the area."; break;
            default : throw prg_error("Unknown area scan rate type '%d'.\n", "PrintRetainedClustersStatus()", gParameters.GetAreaScanRateType());
         }
         break;
      case EXPONENTIAL :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : buffer = "All areas scanned had either only one case or equal or longer survival than outside the area."; break;
            case LOW        : buffer = "All areas scanned had either only one case or equal or shorter survival than outside the area."; break;
            case HIGHANDLOW : buffer = "All areas scanned had either only one case or equal survival to outside the area."; break;
            default : throw prg_error("Unknown area scan rate type '%d'.\n", "PrintRetainedClustersStatus()", gParameters.GetAreaScanRateType());
         }
         break;
      default : throw prg_error("Unknown probability model '%d'.", "PrintRetainedClustersStatus()", gParameters.GetProbabilityModelType());
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  else if (!bClusterReported) {
    fprintf(fp, "\nNo clusters reported.\n");
    anyClusters=false;
    for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end() && !anyClusters; ++itrMLC)
        anyClusters = itrMLC->GetNumClustersRetained() && itrMLC->GetTopRankedCluster().GetRatio() < gdMinRatioToReport;
    if (!anyClusters)
      printString(buffer, "All clusters had a %s less than %g.", (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"), gdMinRatioToReport);
    else
      printString(buffer, "All clusters had a rank greater than %i.", gParameters.GetNumReplicationsRequested());
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
}

/** Prints most likely cluster information, if any retained, to result file.
    If user requested 'location information' output file(s), they are created
    simultaneously with reported clusters. */
void AnalysisRunner::PrintTopClusters(const MostLikelyClustersContainer& mlc) {
  std::auto_ptr<LocationInformationWriter> ClusterLocationWriter;
  std::auto_ptr<ClusterInformationWriter>  ClusterWriter;
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  FILE * fp=0;

  try {
    //if creating 'location information' files, create record data buffers
    if (gParameters.GetOutputAreaSpecificFiles())
      ClusterLocationWriter.reset(new LocationInformationWriter(*gpDataHub, giAnalysisCount > 1));
    //if creating 'cluster information' files, create record data buffers
    if (gParameters.GetOutputClusterLevelFiles() || gParameters.GetOutputClusterCaseFiles())
      ClusterWriter.reset(new ClusterInformationWriter(*gpDataHub));
    //open result output file
    OpenReportFile(fp, true);
    unsigned int clustersReported=0;
    //if no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(gSimVars.get_sim_count() == 0 ? std::min(10, mlc.GetNumClustersRetained()) : mlc.GetNumClustersRetained());
    for (int i=0; i < mlc.GetNumClustersRetained(); ++i) {
        gPrintDirection.Printf("Reporting cluster %i of %i\n", BasePrint::P_STDOUT, i + 1, mlc.GetNumClustersRetained());
        if (i==9) //report estimate of time to report all clusters
            ReportTimeEstimate(StartTime, mlc.GetNumClustersRetained(), i, &gPrintDirection);
        //get reference to i'th top cluster
        const CCluster& TopCluster = mlc.GetCluster(i);
        //write cluster details to 'cluster information' file
        if (ClusterWriter.get() && TopCluster.m_nRatio >= gdMinRatioToReport)
            ClusterWriter->Write(TopCluster, i+1, gSimVars);
        //write cluster details to results file and 'location information' files -- always report most likely cluster but only report
        //secondary clusters if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
        if (i == 0 || (i < tNumClustersToDisplay && TopCluster.m_nRatio >= gdMinRatioToReport && (gSimVars.get_sim_count() == 0 || TopCluster.GetRank() <= gSimVars.get_sim_count()))) {
            ++clustersReported;
            switch (clustersReported) {
                case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
                case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n"); break;
                default : fprintf(fp, "\n"); break;
            }
            //print cluster definition to file stream
            TopCluster.Display(fp, *gpDataHub, clustersReported, gSimVars);
            //check track of whether this cluster was significant in top five percentage
            if (GetIsCalculatingSignificantRatios() && macro_less_than(gpSignificantRatios->GetAlpha05(), TopCluster.m_nRatio, DBL_CMP_TOLERANCE))
                ++guwSignificantAt005;
            //print cluster definition to 'location information' record buffer
            if (gParameters.GetOutputAreaSpecificFiles())
                TopCluster.Write(*ClusterLocationWriter, *gpDataHub, clustersReported, gSimVars);
            _clustersReported = true;
        }
    }
    PrintRetainedClustersStatus(fp, _clustersReported);
    PrintCriticalValuesStatus(fp);
    PrintPowerCalculationsStatus(fp);
    PrintEarlyTerminationStatus(fp);
    PrintGiniCoefficients(fp);
    fclose(fp); fp=0;
  }
  catch (prg_exception& x) {
    if (fp) fclose(fp);
    x.addTrace("PrintTopClusters()","AnalysisRunner");
    throw;
  }
}

/** Displays most likely clusters loglikelihood ratio(test statistic) to print
    direction. If no clusters were retained, indicating message is printed. */
void AnalysisRunner::PrintTopClusterLogLikelihood(const MostLikelyClustersContainer& mlc) {
    //if any clusters were retained, display either loglikelihood or test statistic
    if (mlc.GetNumClustersRetained())
        gPrintDirection.Printf("SaTScan %s for the most likely cluster: %7.2lf\n", BasePrint::P_STDOUT, 
                               gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio",
                               mlc.GetTopRankedCluster().m_nRatio);
    else
        gPrintDirection.Printf("No clusters retained.\n", BasePrint::P_STDOUT);
}

/** Prints most likely cluster information, if retained, to result file. This
    function only prints THE most likely cluster, as part of reporting with
    the iterative scan option. So printing is directed by the particular
    iteration of the iterative scan.
    If user requested 'location information' output file(s), they are created
    simultaneously with reported clusters. */
void AnalysisRunner::PrintTopIterativeScanCluster(const MostLikelyClustersContainer& mlc) {
  FILE        * fp=0;
  unsigned int  clustersReported=0;

  try {
    //open result output file
    OpenReportFile(fp, true);
    if (mlc.GetNumClustersRetained()) {
      //get most likely cluster
      const CCluster& TopCluster = mlc.GetTopRankedCluster();
      ++clustersReported; 
      switch (clustersReported) {
       case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
       case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n");
       default : {std::string s; printString(s, "REMAINING DATA WITH %d CLUSTER%s REMOVED", giAnalysisCount - 1, (giAnalysisCount - 1 == 1 ? "" : "S"));
                  gpDataHub->DisplaySummary(fp, s, false);
                  fprintf(fp, "\n");
                 }
       }
      //print cluster definition to file stream
      TopCluster.Display(fp, *gpDataHub, clustersReported, gSimVars);
      //print cluster definition to 'cluster information' record buffer
      if (gParameters.GetOutputClusterLevelFiles() || gParameters.GetOutputClusterCaseFiles())
        ClusterInformationWriter(*gpDataHub, giAnalysisCount > 1).Write(TopCluster, clustersReported, gSimVars);
      //print cluster definition to 'location information' record buffer
      if (gParameters.GetOutputAreaSpecificFiles()) {
        LocationInformationWriter Writer(*gpDataHub, giAnalysisCount > 1);
        TopCluster.Write(Writer, *gpDataHub, clustersReported, gSimVars);
      }
      //check track of whether this cluster was significant in top five percentage
      if (GetIsCalculatingSignificantRatios() && macro_less_than(gpSignificantRatios->GetAlpha05(), TopCluster.m_nRatio, DBL_CMP_TOLERANCE))
        ++guwSignificantAt005;
      _clustersReported=true;
    }

    //if no clusters reported in this iteration but clusters were reported previuosly, print spacer
    if (!clustersReported && _clustersReported)
      fprintf(fp, "                  _____________________________\n\n");

    PrintRetainedClustersStatus(fp, clustersReported);
    PrintCriticalValuesStatus(fp);
    PrintPowerCalculationsStatus(fp);
    PrintEarlyTerminationStatus(fp);
    fclose(fp); fp=0;
  } catch (prg_exception& x) {
     if (fp) fclose(fp);
    x.addTrace("PrintTopIterativeScanCluster()","AnalysisRunner");
    throw;
  }
}

/** Performs ranking on each collection of clusters. */
void AnalysisRunner::rankClusterCollections() {
    // If reporting hierarchical clusters, clone the collection of clusters associated with the greatest maxima.
    // We need to maintain a copy since geographical overlap might be different that index based ranking (no overlap).
    if (gParameters.getIsReportingHierarchicalClusters() || gParameters.GetIsPurelyTemporalAnalysis()) {
        _reportClusters = gTopClustersContainers.back();
        _reportClusters.rankClusters(*gpDataHub, gParameters.GetCriteriaSecondClustersType(), gPrintDirection);
        // don't need to add clusters to cluster ranker if not performing simulations
        if (gParameters.GetNumReplicationsRequested()) _clusterRanker.add(_reportClusters);
    }
    if (gParameters.getIsReportingIndexBasedClusters()) {
        // Index based clusters always use 'No Geographical Overlap'.
        for (MLC_Collections_t::iterator itr=gTopClustersContainers.begin(); itr != gTopClustersContainers.end(); ++itr) {
            itr->rankClusters(*gpDataHub, NOGEOOVERLAP, gPrintDirection);
            // don't need to add clusters to cluster ranker if not performing simulations
            if (gParameters.GetNumReplicationsRequested()) _clusterRanker.add(*itr);
        }
    }
    // cause the cluster ranker to sort clusters by LLR for ranking during simulations
    _clusterRanker.sort();
    // Note: If we're not reporting hierarchical, then this might report a cluster that is not displayed in final output.
    //       We can't perform index based ordering here since we need to perform simulations first ... correct?
    PrintTopClusterLogLikelihood(getLargestMaximaClusterCollection());
}

/** Returns indication of whether analysis repeats.
    Indication of true is returned if user requested iterative scan option and :
    - analysis type is purely spatial or monotone purely spatial
    - a most likely cluster was retained
    - most likely cluster's p-value is not less than user specified cutoff p- value
    - after removing most likely cluster's contained locations, there are still locations
    - the number of requested iterative scans has not been already reached
    - last iteration of simulations did not terminate early
    Indication of false is returned if user did not request iterative scan option. */
bool AnalysisRunner::RepeatAnalysis() {
  //NOTE: Still in the air as to the minimum for STP model, set to 2 for now.
  count_t      tMinCases = (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL ? 4 : 2);

  try {
    if (!gParameters.GetIsIterativeScanning()) return false;
      if (giAnalysisCount >= gParameters.GetNumIterativeScansRequested())
        return false;

      //determine whether a top cluster was found and it's p-value mets cutoff
      if (!getLargestMaximaClusterCollection().GetNumClustersRetained())
        return false;
      //if user requested replications, validate that p-value does not exceed user defined cutoff 
      if (gParameters.GetNumReplicationsRequested()) {
          const CCluster& topCluster = getLargestMaximaClusterCollection().GetTopRankedCluster();
          if (topCluster.getReportingPValue(gParameters, gSimVars, true) > gParameters.GetIterativeCutOffPValue())
             return false;
      }

      //now we need to modify the data sets - removing data of locations in top cluster
      gpDataHub->RemoveClusterSignificance(getLargestMaximaClusterCollection().GetTopRankedCluster());

      //for SVTT analyses, are data set global time trends converging?
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
         for (unsigned int i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i)
            if (gpDataHub->GetDataSetHandler().GetDataSet(i).getTimeTrend().GetStatus() != AbstractTimeTrend::CONVERGED)
              return false;   
      }

      //does the minimum number of cases remain in all data sets?
      unsigned int iSetWithMinimumCases=0;
      for (unsigned int i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i)
         if (gpDataHub->GetDataSetHandler().GetDataSet(i).getTotalCases() < tMinCases) ++iSetWithMinimumCases;
      if (gpDataHub->GetDataSetHandler().GetNumDataSets() == iSetWithMinimumCases)
         return false;

      //are there locations left?
      if (!gParameters.GetIsPurelyTemporalAnalysis() && ((size_t)gpDataHub->GetNumTracts() + gpDataHub->GetNumMetaTractsReferenced() - gpDataHub->GetNumNullifiedLocations()) < 2)
         return false;
      //is the minimum number of cases per data set remaining, as required by probability model?
      if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
        int iCategoriesWithCases=0;
        for (unsigned int i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i) {
           const PopulationData& Population = gpDataHub->GetDataSetHandler().GetDataSet(i).getPopulationData();
           for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t)
              if (Population.GetNumOrdinalCategoryCases(static_cast<int>(t)) > 0)
                ++iCategoriesWithCases;
           if (iCategoriesWithCases < 2)
             return false;
        }
      }
      if (!gParameters.GetIsPurelyTemporalAnalysis())
        gpDataHub->AdjustNeighborCounts(geExecutingType);
      //clear top clusters container
      for (MLC_Collections_t::iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end(); ++itrMLC)
        itrMLC->Empty();
      _reportClusters.Empty();
  }
  catch (prg_exception& x) {
    x.addTrace("RepeatAnalysis()","AnalysisRunner");
    throw;
  }
  return true;
}

/** internal class setup - allocate CSaTScanData object(the data hub) */
void AnalysisRunner::Setup() {
  try {
    for (std::vector<double>::const_iterator itr=gParameters.getExecuteSpatialWindowStops().begin(); itr != gParameters.getExecuteSpatialWindowStops().end(); ++itr)
        gTopClustersContainers.push_back(MostLikelyClustersContainer(*itr));
    //create data hub
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL             : gpDataHub.reset(new CPurelySpatialData(gParameters, gPrintDirection));  break;
      case PURELYTEMPORAL            :
      case PROSPECTIVEPURELYTEMPORAL : gpDataHub.reset(new CPurelyTemporalData(gParameters, gPrintDirection)); break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : gpDataHub.reset(new CSpaceTimeData(gParameters, gPrintDirection)); break;
      case SPATIALVARTEMPTREND       : gpDataHub.reset(new CSVTTData(gParameters, gPrintDirection)); break;
      default : throw prg_error("Unknown Analysis Type '%d'.", "Setup()", gParameters.GetAnalysisType());
    };
    if (gParameters.GetReportCriticalValues() && gParameters.GetNumReplicationsRequested() >= 19)
      gpSignificantRatios.reset(new CSignificantRatios05(gParameters.GetNumReplicationsRequested()));
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","AnalysisRunner");
    throw;
  }
}

/** If user requested power calculation option, updates power calculation
    counters based upon passed simulated loglikelihood ratio. */
void AnalysisRunner::UpdatePowerCounts(double r) {
  if (gParameters.GetIsPowerCalculated()) {
    if (r > gParameters.GetPowerCalculationX())
      ++giPower_X_Count;
    if (r > gParameters.GetPowerCalculationY())
      ++giPower_Y_Count;
  }
}

/** Updates results output file.
    - prints most likely cluster(s) (optionally for iterative scan)
    - significant loglikelihood ratio indicator
    - power calculation results, if option requested by user
    - indication of when simulations terminated early */
void AnalysisRunner::reportClusters() {
    macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
    try {
        gPrintDirection.Printf("Printing analysis results to file...\n", BasePrint::P_STDOUT);
        if (gParameters.getIsReportingIndexBasedClusters()) {
            // cluster reporting for index based cluster collections can either be only the optimal collection or all collections
            if (gParameters.getIndexBasedReportType() == OPTIMAL_ONLY) {
                // iterate through cluster collections, finding the collection with the greatest GINI coeffiecent
                const MostLikelyClustersContainer* maximizedCollection = 0;
                double maximizedGINI = gTopClustersContainers.front().getGiniCoefficient(*gpDataHub, gSimVars, gParameters.getIndexBasedPValueCutoff());
                for (MLC_Collections_t::const_iterator itrMLC=gTopClustersContainers.begin() + 1; itrMLC != gTopClustersContainers.end(); ++itrMLC) {
                    double thisGini = itrMLC->getGiniCoefficient(*gpDataHub, gSimVars, gParameters.getIndexBasedPValueCutoff());
                    if (maximizedGINI < thisGini) {
                        maximizedCollection = &(*itrMLC);
                        maximizedGINI = thisGini;
                    }
                }
                // combine clusters from maximized GINI collection with reporting collection
                if (maximizedCollection) 
                    _reportClusters.combine(*maximizedCollection, *gpDataHub);
                else if (_reportClusters.GetNumClustersRetained() == 0) {
                    /* When reporting only Gini coefficients (optimal only) then if no significant gini collection found, 
                       then report cluster collection from largest maxima with clusters. */
                    MLC_Collections_t::reverse_iterator rev(gTopClustersContainers.end()), rev_end(gTopClustersContainers.begin());
                    for (; rev != rev_end; rev++) {
                        if (rev->GetNumClustersRetained()) {
                            _reportClusters.combine(*rev, *gpDataHub);
                            break;
                        }
                    }
                }
            } else {
                // combine clusters from each maxima collection with reporting collection
                for (MLC_Collections_t::iterator itrMLC=gTopClustersContainers.begin(); itrMLC != gTopClustersContainers.end(); ++itrMLC)
                    _reportClusters.combine(*itrMLC, *gpDataHub);
            }
            // now sort combined cluster collection by LLR
            _reportClusters.sort();
        }
        if (gParameters.GetIsIterativeScanning())
            PrintTopIterativeScanCluster(_reportClusters);
        else {
            PrintTopClusters(_reportClusters);
            /* Scatter chart code ...
            if (_reportClusters.GetNumClustersRetained() && 
                gParameters.GetCoordinatesType() == CARTESIAN && 
                !gParameters.GetIsPurelyTemporalAnalysis() &&
                !gParameters.UseLocationNeighborsFile()) {
                ClusterScatterChart plot(*gpDataHub, _reportClusters, gSimVars);
                plot.renderScatterChart();
            } */
            /* Google Earth code ... 
            if (_reportClusters.GetNumClustersRetained() && 
                gParameters.GetCoordinatesType() == LATLON && 
                !gParameters.GetIsPurelyTemporalAnalysis() &&
                !gParameters.UseLocationNeighborsFile()) {
                ClusterKML kmlOut(*gpDataHub, _reportClusters, gSimVars);
                kmlOut.renderKML();
            }*/
        }
    } catch (prg_exception& x) {
        x.addTrace("reportClusters()","AnalysisRunner");
        throw;
    }
    macroRunTimeStopSerial();
}

/** Updates list of significant ratio, if structure allocated. */
void AnalysisRunner::UpdateSignificantRatiosList(double dRatio) {
    if (gpSignificantRatios.get()) gpSignificantRatios->AddRatio(dRatio);
}



