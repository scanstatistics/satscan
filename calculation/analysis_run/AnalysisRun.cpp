//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "AnalysisRun.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "stsRunHistoryFile.h"
#include "stsClusterData.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"
#include "stsLogLikelihood.h"
#include "stsAreaSpecificData.h"
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
#include "stsMCSimReporter.h"
#include "stsMCSimContinuationPolicy.h"
#include "contractor.h"

/** constructor */
AnalysisRunner::AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection)
               :gParameters(Parameters),
                gStartTime(StartTime),
                gPrintDirection(PrintDirection),
                gSimulatedRatios(Parameters.GetNumReplicationsRequested(), &PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","AnalysisRunner");
    throw;
  }
}

/** destructor */
AnalysisRunner::~AnalysisRunner() {
  try {
    delete gpDataHub;
  }
  catch (...) {}
}

/** calculates most likely clusters in real data */
void AnalysisRunner::CalculateMostLikelyClusters() {
  CAnalysis                  * pAnalysis=0;
  AbtractDataStreamGateway   * pDataStreamGateway=0;

  try {
    //display process heading
    DisplayFindClusterHeading();
    //allocate date gateway object
    pDataStreamGateway = gpDataHub->GetDataStreamHandler().GetNewDataGateway();
    //get analysis object
    pAnalysis = GetNewAnalysisObject();
    //allocate objects used in 'FindTopClusters()' process
    pAnalysis->AllocateTopClustersObjects(*pDataStreamGateway);
    //calculate most likely clusters
    pAnalysis->FindTopClusters(*pDataStreamGateway, gTopClustersContainer);
    delete pDataStreamGateway; pDataStreamGateway=0;
    delete pAnalysis; pAnalysis=0;
    //display the loglikelihood of most likely cluster
    if (!gPrintDirection.GetIsCanceled())
      DisplayTopClusterLogLikelihood();
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    delete pAnalysis;
    x.AddCallpath("CalculateMostLikelyClusters()","CAnalysis");
    throw;
  }
}

/** Returns indication of whether analysis is to be cancelled.
    Indication returned is false if:
    - the number of simulations requested is have been completed
    - the user did not request early termination option
    - the number of simulations completed is not 99, 199, 499, or 999
    - no 'most likely clusters' were retained
    - the p-value of most likely cluster is not greater than defined cutoff,
      given the current number of simulations completed
    Indication returned is true if:
    - the p-value of most likely cluster is greater than defined cutoff,
      given the current number of simulations completed */
bool AnalysisRunner::CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const {
  float fCutOff;

  if (iNumSimulationsCompleted == gParameters.GetNumReplicationsRequested())
    return false;
  if (!gParameters.GetTerminateSimulationsEarly())
    return false;
  if (gTopClustersContainer.GetNumClustersRetained() > 0) {
    switch (iNumSimulationsCompleted) {
      case 99   : fCutOff = .5; break;
      case 199  : fCutOff = .4; break;
      case 499  : fCutOff = .2; break;
      case 999  : fCutOff = .1; break;
      default   : return false;
    }
    return (gTopClustersContainer.GetTopRankedCluster().GetPValue(iNumSimulationsCompleted) > fCutOff);
  }
  return false;
}

/** Creates 'cluster information' output file(s) if requested by user through
    parameter settings. If no clusters of significance are found, file will still
    be created, but will be empty. Files types creates are ASCII text and dBase,
    with the user's parameter settings indicating which. */
void AnalysisRunner::CreateClusterInformationFile() {
  try {
    if (gParameters.GetOutputClusterLevelFiles()) {
      //create file record data buffers
      stsClusterData ClusterDataBuffers(gParameters, giNumSimsExecuted < 99);
      //print progress to print direction                                                        
      if (gTopClustersContainer.GetNumClustersRetained())
        gPrintDirection.SatScanPrintf("Recording results for %i cluster%s...\n",
                                      gTopClustersContainer.GetNumClustersRetained(),
                                     (gTopClustersContainer.GetNumClustersRetained() == 1 ? "" : "s"));
      //collect most likely cluster data in record buffers
      for (int i=0; i < gTopClustersContainer.GetNumClustersRetained(); ++i)
         ClusterDataBuffers.RecordClusterData(gTopClustersContainer.GetCluster(i), *gpDataHub, i+1, giNumSimsExecuted);
      //print record buffers to ASCII file
      if (gParameters.GetOutputClusterLevelAscii())
        ASCIIFileWriter(ClusterDataBuffers, gPrintDirection, gParameters, giAnalysisCount > 1);
      //print record buffers to dBase file
      if (gParameters.GetOutputClusterLevelDBase())
        DBaseFileWriter(ClusterDataBuffers, gPrintDirection, gParameters, giAnalysisCount > 1);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateClusterInformationFile()","AnalysisRunner");
    throw;
  }
}

/** Creates 'relative risks' output file(s) if requested by user through
    parameter settings. The created file details the relative risk for each
    location specified in coordinates file. Files types creates are ASCII text
    and dBase, with the user's parameter settings indicating which.
    Note that relative risk information is printed for only first iteration of
    sequential scan. This is primarily to prevent division by zero, since the
    squential scan feature zeros out data used in creating relative risks. */
void AnalysisRunner::CreateRelativeRiskFile() {
  if (giAnalysisCount == 1 && gParameters.GetOutputRelativeRisksFiles())
    gpDataHub->DisplayRelativeRisksForEachTract();
}

/** Creates/overwrites result file specified by user in parameter settings. Only
    header summary information is printed. File pointer does not remain open. */
void AnalysisRunner::CreateReport() {
  FILE       * fp=0;
  ZdString     sStartTime;

  try {
    OpenReportFile(fp, false);
    AsciiPrintFormat::PrintVersionHeader(fp);
    sStartTime = ctime(&gStartTime);
    fprintf(fp,"\nProgram run on: %s\n", sStartTime.GetCString());
    gParameters.DisplayAnalysisType(fp);
    gParameters.DisplayAdjustments(fp, gpDataHub->GetDataStreamHandler());
    gpDataHub->DisplaySummary(fp);
    fclose(fp);
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("CreateReport()","AnalysisRunner");
    throw;
  }
}

/** Displays progress information to print direction indicating that analysis
    is calculating the most likely clusters in data. If sequential scan option
    was requested, the message printed reflects which iteration of the scan it
    is performing. */
void AnalysisRunner::DisplayFindClusterHeading() {
  if (!gParameters.GetIsSequentialScanning())
    gPrintDirection.SatScanPrintf("Finding the most likely clusters.\n");
  else {
    switch(giAnalysisCount) {
      case  1: gPrintDirection.SatScanPrintf("Finding the most likely cluster.\n"); break;
      case  2: gPrintDirection.SatScanPrintf("Finding the second most likely cluster.\n"); break;
      case  3: gPrintDirection.SatScanPrintf("Finding the third most likely cluster.\n"); break;
      default: gPrintDirection.SatScanPrintf("Finding the %ith most likely cluster.\n", giAnalysisCount);
    }
  }
}

/** Displays most likely clusters loglikelihood ratio(test statistic) to print
    direction. If no clusters were retained, indicating message is printed. */
void AnalysisRunner::DisplayTopClusterLogLikelihood() {
  //if any clusters were retained, display either loglikelihood or test statistic
  if (gTopClustersContainer.GetNumClustersRetained() == 0)
    gPrintDirection.SatScanPrintf("No clusters retained.\n");
  else
    gPrintDirection.SatScanPrintf("SaTScan %s for the most likely cluster: %7.2lf\n",
                                  (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"),
                                  gTopClustersContainer.GetTopRankedCluster().m_nRatio);
}

/** Prints most likely cluster information, if retained, to result file. This
    function only prints THE most likely cluster, as part of reporting with
    the sequential scan option. So printing is directed by the particular
    iteration of the sequential scan. 
    If user requested 'location information' output file(s), they are created
    simultaneously with reported clusters. */
void AnalysisRunner::DisplayTopCluster() {
  measure_t                             nMinMeasure = 0;
  std::auto_ptr<stsAreaSpecificData>    pData;
  FILE                                * fp=0;

  try {
    if (gTopClustersContainer.GetNumClustersRetained() > 0) {
      //open result output file
      OpenReportFile(fp, true);
      //get most likely cluster
      const CCluster& TopCluster = gTopClustersContainer.GetTopRankedCluster();
      //if creating 'location information files, create record data buffers
      if (gParameters.GetOutputAreaSpecificFiles())
        pData.reset(new stsAreaSpecificData(gParameters, gParameters.GetNumReplicationsRequested() < 99));
      //only report clutser if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
      if (TopCluster.m_nRatio > gdMinRatioToReport && (giNumSimsExecuted == 0 || TopCluster.GetRank()  <= giNumSimsExecuted)) {
        ++giClustersReported;
        switch(giAnalysisCount) {
          case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
          case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
          default : fprintf(fp,"                  _____________________________\n\n");
        }
        //print cluster definition to file stream
        TopCluster.Display(fp, *gpDataHub, giClustersReported, nMinMeasure, giNumSimsExecuted);
        //check track of whether this cluster was significant in top five percentage
        if (TopCluster.m_nRatio > gSimulatedRatios.GetAlpha05())
          ++guwSignificantAt005;
        //print cluster definition to 'location information' record buffer
        if (gParameters.GetOutputAreaSpecificFiles())
           TopCluster.Write(*(pData.get()), *gpDataHub, giClustersReported, giNumSimsExecuted);
      }
      fprintf(fp, "\n");
      fclose(fp); fp=0;
      //print 'location information' record buffers to file(s)
      if (gParameters.GetOutputAreaSpecificAscii())
         ASCIIFileWriter(*(pData.get()), gPrintDirection, gParameters, giAnalysisCount > 1);
      if (gParameters.GetOutputAreaSpecificDBase())
         DBaseFileWriter(*(pData.get()), gPrintDirection, gParameters, giAnalysisCount > 1);
    }
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("DisplayTopCluster()","AnalysisRunner");
    throw;
  }
}

/** Prints most likely cluster information, if any retained, to result file. 
    If user requested 'location information' output file(s), they are created
    simultaneously with reported clusters. */
void AnalysisRunner::DisplayTopClusters() {
  double                               dSignifRatio05;
  std::auto_ptr<stsAreaSpecificData>   pData;
  clock_t                              lStartTime;
  measure_t                            nMinMeasure = -1;
  FILE                                * fp=0;

  try {
    //if creating 'location information' files, create record data buffers
    if (gParameters.GetOutputAreaSpecificFiles())
      pData.reset(new stsAreaSpecificData(gParameters, giNumSimsExecuted < 99));
    dSignifRatio05 = gSimulatedRatios.GetAlpha05();
    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(giNumSimsExecuted == 0 ? std::min(10, gTopClustersContainer.GetNumClustersRetained()) : gTopClustersContainer.GetNumClustersRetained());
    lStartTime = clock(); //get clock for calculating output time
    //open result output file
    OpenReportFile(fp, true);
    for (tract_t i=0; i < tNumClustersToDisplay; ++i) {
       //get next most likely cluster
       const CCluster& TopCluster = gTopClustersContainer.GetCluster(i);
       //report progress
       if (i==1)
         ReportTimeEstimate(lStartTime, tNumClustersToDisplay, i, &gPrintDirection);
       //only report clutser if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
       if (TopCluster.m_nRatio > gdMinRatioToReport && (giNumSimsExecuted == 0 || TopCluster.GetRank()  <= giNumSimsExecuted)) {
         ++giClustersReported;
         switch (giClustersReported) {
           case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
           case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
           default : fprintf(fp, "\n"); break;
         }
         //print cluster definition to file stream
         TopCluster.Display(fp, *gpDataHub, giClustersReported, nMinMeasure, giNumSimsExecuted);
         //check track of whether this cluster was significant in top five percentage
         if (TopCluster.m_nRatio > dSignifRatio05)
           ++guwSignificantAt005;
         //print cluster definition to 'location information' record buffer
         if (gParameters.GetOutputAreaSpecificFiles())
           TopCluster.Write(*(pData.get()), *gpDataHub, giClustersReported, giNumSimsExecuted);
       }
    }
    fprintf(fp, "\n");
    fclose(fp); fp=0;
    //print 'location information' record buffers to file(s)
    if (gParameters.GetOutputAreaSpecificAscii()) // print area ASCII
      ASCIIFileWriter(*(pData.get()), gPrintDirection, gParameters);
    if (gParameters.GetOutputAreaSpecificDBase()) // print area dBase
      DBaseFileWriter(*(pData.get()), gPrintDirection, gParameters);
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("DisplayTopClusters()","AnalysisRunner");
    throw;
  }
}

/** starts analysis execution */
void AnalysisRunner::Execute() {
  bool  bContinue;

  try {
    //read data
    gpDataHub->ReadDataFromFiles();
    //calculate expected cases
    gpDataHub->CalculateExpectedCases();
    //validate that data set contains cases
    for (unsigned int i=0; i < gpDataHub->GetDataStreamHandler().GetNumStreams(); ++i)
       if (gpDataHub->GetDataStreamHandler().GetStream(i).GetTotalCases() == 0)
         GenerateResolvableException("Error: No cases found in data set %u.\n","Execute()", i);
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //calculate number of neighboring locations about each centroid
    gpDataHub->FindNeighbors(false);
    //detect cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //create result file report
    CreateReport();
    //start analyzing data
    do {
      ++giAnalysisCount;
      guwSignificantAt005 = 0;
      //calculate most likely clusters
      CalculateMostLikelyClusters();
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
      //Do Monte Carlo replications.
      if (gTopClustersContainer.GetNumClustersRetained())
        PerformSimulations();
      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
      //update report
      UpdateReport();
      //log history for first analysis run
      if (giAnalysisCount == 1) {
        gPrintDirection.SatScanPrintf("Logging run history...\n");
        stsRunHistoryFile(gParameters, gPrintDirection).LogNewHistory(*this);
      }
      //report additional output file: 'cluster information'
      CreateClusterInformationFile();
      //report additional output file: 'relative risks for each location'
      CreateRelativeRiskFile();
      //repeat analysis - sequential scan
      if ((bContinue = RepeatAnalysis()) == true) {
        RemoveTopClusterData();
        //detect user cancellation
        if (gPrintDirection.GetIsCanceled())
          return;
      }
    } while (bContinue);
    //finish report
    FinalizeReport();
  }
  catch (ZdException &x) {
    x.AddCallpath("Execute()","AnalysisRunner");
    throw;
  }
}

/** Finalizes the reporting to result output file.
    - indicates whether clusters were found
    - indicates whether no clusters were reported because their loglikelihood
      ratios are less than defined minimum value
    - if the number of simulations are less 98, reported that the reported
      clusters intentially do not contain p-values */
void AnalysisRunner::FinalizeReport() {
  FILE        * fp=0;
  time_t        CompletionTime;
  double        nTotalTime,  nSeconds,  nMinutes,  nHours;
  char        * szHours = "hours";
  char        * szMinutes = "minutes";
  char        * szSeconds = "seconds";

  try {
    gPrintDirection.SatScanPrintf("Printing analysis settings to the results file...\n");
    OpenReportFile(fp, true);
    if (gTopClustersContainer.GetNumClustersRetained() == 0) {
      fprintf(fp, "\nNo clusters were found.\n");
      if (gParameters.GetAreaScanRateType() == HIGH)
        fprintf(fp, "All areas scanned had equal or fewer cases than expected.\n");
      else if (gParameters.GetAreaScanRateType() == LOW)
        fprintf(fp, "All areas scanned had equal or greater cases than expected.\n");
      else
        fprintf(fp, "All areas scanned had cases equal to expected.\n");
    }
    else if (giClustersReported == 0) {
      fprintf(fp, "\nNo clusters reported.\n");
      fprintf(fp, "All clusters had a ratio less than %lf or\n"
                  "a rank greater than %i.\n", gdMinRatioToReport, gParameters.GetNumReplicationsRequested());
    }
    else if (gParameters.GetNumReplicationsRequested() == 0) {
      fprintf(fp, "\nNote: As the number of Monte Carlo replications was set to\n");
      fprintf(fp, "zero, no hypothesis testing was done and no p-values were\n");
      fprintf(fp, "printed.\n");
    }
    else if (gParameters.GetNumReplicationsRequested() <= 98) {
      fprintf(fp, "\nNote: The number of Monte Carlo replications was set too low,\n");
      fprintf(fp, "and a meaningful hypothesis test cannot be done.  Consequently,\n");
      fprintf(fp, "no p-values were printed.\n");
    }

    if (gParameters.GetProbabiltyModelType() == POISSON)
      gpDataHub->GetDataStreamHandler().ReportZeroPops(*gpDataHub, fp, &gPrintDirection);

    gpDataHub->GetTInfo()->tiReportDuplicateTracts(fp);
    gParameters.DisplayParameters(fp, giNumSimsExecuted, gpDataHub->GetDataStreamHandler());
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
    fclose(fp);
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("FinalizeReport()","AnalysisRunner");
    throw;
  }
}

/** returns new CAnalysis object */
CAnalysis * AnalysisRunner::GetNewAnalysisObject() const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL :
          if (gParameters.GetRiskType() == STANDARDRISK)
            return new CPurelySpatialAnalysis(gParameters, *gpDataHub, gPrintDirection);
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
    };
   } 
  catch (ZdException &x) {
    x.AddCallpath("GetNewAnalysisObject()","AnalysisRunner");
    throw;
  }
  return 0;
}

/** class initialization */
void AnalysisRunner::Init() {
  gpDataHub=0;
  giAnalysisCount=0;
  giPower_X_Count=0;
  giPower_Y_Count=0;
  gdMinRatioToReport=0.001;
  guwSignificantAt005=0;
  giClustersReported=0;
}

/** Attempts to open result output file stream and assign to passed file pointer
    address. Open mode is determined to boolean paramter. */
void AnalysisRunner::OpenReportFile(FILE*& fp, bool bOpenAppend) {
  try {
    if ((fp = fopen(gParameters.GetOutputFileName().c_str(), (bOpenAppend ? "a" : "w"))) == NULL) {
      if (!bOpenAppend)
        ResolvableException::Generate("Error: Results file '%s' could not be created.\n",
                                      "OpenReportFile()", gParameters.GetOutputFileName().c_str());
      else if (bOpenAppend)
        ResolvableException::Generate("Error: Results file '%s' could not be opened.\n",
                                      "OpenReportFile()", gParameters.GetOutputFileName().c_str());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenReportFile()","AnalysisRunner");
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
void AnalysisRunner::PerformParallelSimulations() {
  static const int iParallelProcessCount = 2; /** Target enviroment for parallel simulations
                                                  is Windows w/ dual processors, so hard code
                                                  to 2 threads. Later, we will want to determine
                                                  this programmatically, possibly with limits
                                                  specified by user (e.g. 8 processors available
                                                  but only utilize 6). */

  double                               dSimulatedRatio;
  unsigned int                         iSimulationNumber;
  char                               * sReplicationFormatString;
  std::auto_ptr<LogLikelihoodData>     pLLRData;
  AbtractDataStreamGateway           * pDataGateway=0;
  CAnalysis                          * pAnalysis=0;
  SimulationDataContainer_t            SimulationDataContainer;
  RandomizerContainer_t                RandomizationContainer;

  try {
    if (gParameters.GetNumReplicationsRequested() == 0)
      return;
    //set print message format string
    if (gParameters.GetLogLikelihoodRatioIsTestStatistic())
      sReplicationFormatString = "SaTScan test statistic for #%u of %u replications: %7.2lf\n";
    else
      sReplicationFormatString = "SaTScan log likelihood ratio for #%u of %u replications: %7.2lf\n";
    //create record buffers for simulated loglikelihood ratios, if user requested these output files
    if (gParameters.GetOutputSimLoglikeliRatiosFiles() && giAnalysisCount == 1)
      pLLRData.reset(new LogLikelihoodData(gParameters));
    //set/reset loglikelihood ratio significance indicator
    gSimulatedRatios.Initialize();
    giNumSimsExecuted = 0;

    std::deque< std::pair<unsigned int, double> > qParamsAndResults;
    for (unsigned int ui = 0; ui < gParameters.GetNumReplicationsRequested(); ++ui)
    {
       qParamsAndResults.push_back(std::make_pair(ui + 1,0));
    }
    stsMCSimContinuationPolicy CtPlcy(gPrintDirection);
    stsMCSimReporter Rptr(gParameters, CtPlcy, gSimulatedRatios, gTopClustersContainer, gPrintDirection, sReplicationFormatString);
    typedef contractor<unsigned int, double, stsMCSimReporter, stsMCSimContinuationPolicy> contractor_type;
    contractor_type theContractor(qParamsAndResults, Rptr, CtPlcy);
    //run threads:
    boost::thread_group tg;
    boost::mutex        thread_mutex;
    for (int i = 0; i < iParallelProcessCount; ++i)
    {
      stsMonteCarloSimFunctor mcsf(thread_mutex, GetDataHub(), boost::shared_ptr<CAnalysis>(GetNewAnalysisObject()), boost::shared_ptr<SimulationDataContainer_t>(new SimulationDataContainer_t()), boost::shared_ptr<RandomizerContainer_t>(new RandomizerContainer_t()));
      tg.create_thread(subcontractor<contractor_type,stsMonteCarloSimFunctor>(theContractor,mcsf));
    }
    tg.join_all();

    if (!CtPlcy.UserCancelConditionExists()) {
      giNumSimsExecuted = (Rptr.ShortCircuitConditionExists() ? Rptr.ResultsNotShortCircuitedCount() : qParamsAndResults.size());
      for (unsigned int ui = 0; ui < giNumSimsExecuted; ++ui)
      {
        double dSimulatedRatio = qParamsAndResults[ui].second;
        //update power calculations
        UpdatePowerCounts(dSimulatedRatio);
        //update simulated loglikelihood record buffer
        if(pLLRData.get()) pLLRData->AddLikelihoodRatio(dSimulatedRatio);
      }
      //write to additional data to files
      if (gParameters.GetOutputSimLoglikeliRatiosAscii() && pLLRData.get())
        ASCIIFileWriter(*(pLLRData.get()), gPrintDirection, gParameters);
      if (gParameters.GetOutputSimLoglikeliRatiosDBase() && pLLRData.get())
        DBaseFileWriter(*(pLLRData.get()), gPrintDirection, gParameters);
    }
  }
  catch (ZdException &x) {
    delete pDataGateway;
    delete pAnalysis;
    x.AddCallpath("PerformParallelSimulations()","CAnalysis");
    throw;
  }
}

/** Calculates simulated loglikelihood ratios and updates:
    - most likely clusters rank
    - significant loglikelihood ratio indicator
    - power calculation data, if requested by user
    - additional output file(s)                             */
void AnalysisRunner::PerformSerializedSimulations() {
  double                               dSimulatedRatio;
  unsigned int                         iSimulationNumber;
  char                               * sReplicationFormatString;
  std::auto_ptr<LogLikelihoodData>     pLLRData;
  AbtractDataStreamGateway           * pDataGateway=0;
  CAnalysis                          * pAnalysis=0;
  SimulationDataContainer_t            SimulationDataContainer;
  RandomizerContainer_t                RandomizationContainer;

  try {
    if (gParameters.GetNumReplicationsRequested() == 0)
      return;
    //set print message format string
    if (gParameters.GetLogLikelihoodRatioIsTestStatistic())
      sReplicationFormatString = "SaTScan test statistic for #%u of %u replications: %7.2lf\n";
    else
      sReplicationFormatString = "SaTScan log likelihood ratio for #%u of %u replications: %7.2lf\n";
    //create record buffers for simulated loglikelihood ratios, if user requested these output files
    if (gParameters.GetOutputSimLoglikeliRatiosFiles() && giAnalysisCount == 1)
      pLLRData.reset(new LogLikelihoodData(gParameters));
    //set/reset loglikelihood ratio significance indicator
    gSimulatedRatios.Initialize();
    //get container for simulation data - this data will be modified in the randomize process
    GetDataHub().GetDataStreamHandler().GetSimulationDataContainer(SimulationDataContainer);
    //get container of data randomizers - these will modify the simulation data
    GetDataHub().GetDataStreamHandler().GetRandomizerContainer(RandomizationContainer);
    //get data gateway given data stream handler's real data and simulated data structures
    pDataGateway = GetDataHub().GetDataStreamHandler().GetNewSimulationDataGateway(SimulationDataContainer);
    //get new analysis object for which to defined simulation algorithm
    pAnalysis = GetNewAnalysisObject();
    //allocate appropriate data members for simulation algorithm
    pAnalysis->AllocateSimulationObjects(*pDataGateway);
    //start clock for estimating approximate time to complete
    clock_t nStartTime = clock();
    {//block for the scope of SimulationPrintDirection
      PrintQueue SimulationPrintDirection(gPrintDirection);

      for (giNumSimsExecuted=0, iSimulationNumber=1; (iSimulationNumber <= gParameters.GetNumReplicationsRequested()) && !gPrintDirection.GetIsCanceled(); iSimulationNumber++) {
        ++giNumSimsExecuted;
        //randomize data
        GetDataHub().RandomizeData(RandomizationContainer, SimulationDataContainer, iSimulationNumber);
        //print simulation data to file, if requested
        if (gParameters.GetOutputSimulationData())
          for (size_t t=0; t < SimulationDataContainer.size(); ++t)
             SimulationDataContainer[t]->WriteSimulationData(gParameters, iSimulationNumber);
        //perform simulation to get loglikelihood ratio
        dSimulatedRatio = (pAnalysis->IsMonteCarlo() ? pAnalysis->MonteCarlo(pDataGateway->GetDataStreamInterface(0)) : pAnalysis->FindTopRatio(*pDataGateway));
        //update most likely clusters given latest simulated loglikelihood ratio
        gTopClustersContainer.UpdateTopClustersRank(dSimulatedRatio);
        //update significance indicator
        gSimulatedRatios.AddRatio(dSimulatedRatio);
        //update power calculations
        UpdatePowerCounts(dSimulatedRatio);
        //update simulated loglikelihood record buffer
        if(pLLRData.get()) pLLRData->AddLikelihoodRatio(dSimulatedRatio);
        //if first simulation, report approximate time to complete simulations and print queue threshold
        if (giNumSimsExecuted==1) {
          //***** time to complete approximate will need modified with incorporation of thread code ******
          ReportTimeEstimate(nStartTime, gParameters.GetNumReplicationsRequested(), iSimulationNumber, &SimulationPrintDirection);
          ZdTimestamp tsReleaseTime;
          tsReleaseTime.Now();
          tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
          SimulationPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
        }
        //report simulated loglikelihood ratio to print direction
        SimulationPrintDirection.SatScanPrintf(sReplicationFormatString, iSimulationNumber,
                                               gParameters.GetNumReplicationsRequested(), dSimulatedRatio);
        //check that simulations are not terminated early
        if (CheckForEarlyTermination(iSimulationNumber))
          break;
      }
    }//end scope of SimulationPrintDirection
    delete pDataGateway; pDataGateway=0;
    delete pAnalysis; pAnalysis=0;
    if (!gPrintDirection.GetIsCanceled()) {
      //write to additional data to files
      if (gParameters.GetOutputSimLoglikeliRatiosAscii() && pLLRData.get())
        ASCIIFileWriter(*(pLLRData.get()), gPrintDirection, gParameters);
      if (gParameters.GetOutputSimLoglikeliRatiosDBase() && pLLRData.get())
        DBaseFileWriter(*(pLLRData.get()), gPrintDirection, gParameters);
    }    
  }
  catch (ZdException &x) {
    delete pDataGateway;
    delete pAnalysis;
    x.AddCallpath("PerformSerializedSimulations()","CAnalysis");
    throw;
  }
}

/** Prepares data for simulations and contracts simulation process. */
void AnalysisRunner::PerformSimulations() {
  try {
    if (gParameters.GetNumReplicationsRequested() > 0) {
      //recompute neighbors if settings indicate that smaller clusters are reported
      if (gParameters.GetRestrictingMaximumReportedGeoClusterSize())
        gpDataHub->FindNeighbors(true);
      gPrintDirection.SatScanPrintf("Doing the Monte Carlo replications\n");
#ifdef PARALLEL_SIMULATIONS
      PerformParallelSimulations();
#else
      PerformSerializedSimulations();
#endif
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PerformSimulations()","CAnalysis");
    throw;
  }
}

/** If a most likely cluster is defined, removes data of each location contained
    within that cluster from consideration in next iteration of sequential scan.
    This function is not defined for a most likely cluster that is purely temporal.

    NOTE: There maybe more work here considering randomizers that are based off
          information that is now being removed. Look at space-time permutation
          randomizer and other similar randomizers. */
void AnalysisRunner::RemoveTopClusterData() {
   try {
     if (gParameters.GetAnalysisType() != PURELYSPATIAL)
       ZdGenerateException("Error: The sequential scan feature is currently defined for only\n"
                           "       a purely spatial analysis.","RemoveTopClusterData()");

     if (gTopClustersContainer.GetNumClustersRetained()) {
       const CCluster& TopCluster = gTopClustersContainer.GetTopRankedCluster();
       for (int i=1; i <= TopCluster.GetNumTractsInnerCircle(); i++)  {
         gpDataHub->RemoveTractSignificance(gpDataHub->GetNeighbor(0, TopCluster.GetCentroidIndex(), i));
         //The next statement is believed to have been a mistake. There was no
         //documentation as to it's purpose but I believe the coder was intending
         //to remove the actual tract from the total tracts. Thinking this, the
         //statement is actually assuming that tracts of cluster are always the
         //last x tracts in set. The only results that are effected by its removal
         //are the Relative Risks output file. But reporting the file has been moved
         //to just after the first iteration of sequential scan. So, this file now
         //reports all tracts relative risks before any data manipulation occurs.
         //Also, potentially after many iterations of scan, the number of tracts
         //could be negative.
         //--m_pData->m_nTracts;
       }
       //re-calculate max circle size 
       gpDataHub->SetMaxCircleSize();
       //re-calculate neighbors counts about each centroid 
       gpDataHub->AdjustNeighborCounts();
     }
     //clear top clusters container
     gTopClustersContainer.Empty();
  }
  catch (ZdException &x) {
    x.AddCallpath("RemoveTopClusterData()","AnalysisRunner");
    throw;
  }
}

/** Returns indication of whether analysis repeats.
    Indication of true is returned if user requested sequential scan option and :
    - analysis type is purely spatial or monotone purely spatial
    - a most likely cluster was retained
    - most likely cluster's p-value is not less than user specified cutoff p- value
    - after removing most likely cluster's contained locations, there are still locations
    - the number of requested sequential scans has not been already reached
    - last iteration of simulations did not terminate early
    Indication of false is returned if user did not request sequential scan option. */
bool AnalysisRunner::RepeatAnalysis() {
   bool bCorrectAnalysisType, bTopCluster, bHasMoreSequentialScans,
        bNotEarlyTerminated, bHasTractsAfterTopCluster, bReturn=false;

   try {
      if (gParameters.GetIsSequentialScanning()) {
        bCorrectAnalysisType = gParameters.GetAnalysisType() == PURELYSPATIAL; 
        bTopCluster = gTopClustersContainer.GetNumClustersRetained() &&
                      gTopClustersContainer.GetTopRankedCluster().GetPValue(giNumSimsExecuted) < gParameters.GetSequentialCutOffPValue();
        bHasTractsAfterTopCluster = bTopCluster && gpDataHub->GetNumTracts() - gTopClustersContainer.GetTopRankedCluster().GetNumTractsInnerCircle() > 0;
        bHasMoreSequentialScans = giAnalysisCount < gParameters.GetNumSequentialScansRequested();
        bNotEarlyTerminated = giNumSimsExecuted == gParameters.GetNumReplicationsRequested();

        bReturn = bCorrectAnalysisType && bTopCluster && bHasMoreSequentialScans
                  && bNotEarlyTerminated && bHasTractsAfterTopCluster;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("RepeatAnalysis()","AnalysisRunner");
      throw;
   }
   return bReturn;
}

/** internal class setup - allocate CSaTScanData object(the data hub) */
void AnalysisRunner::Setup() {
  try {
    //create data hub
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL             : gpDataHub = new CPurelySpatialData(gParameters, gPrintDirection);  break;
      case PURELYTEMPORAL            :
      case PROSPECTIVEPURELYTEMPORAL : gpDataHub = new CPurelyTemporalData(gParameters, gPrintDirection); break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : gpDataHub = new CSpaceTimeData(gParameters, gPrintDirection);break;
      case SPATIALVARTEMPTREND       : gpDataHub = new CSVTTData(gParameters, gPrintDirection); break;
      default : ZdGenerateException("Unknown Analysis Type '%d'.", "Setup()", gParameters.GetAnalysisType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","AnalysisRunner");
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
    - prints most likely cluster(s) (optionally for sequential scan)
    - significant loglikelihood ratio indicator
    - power calculation results, if option requested by user
    - indication of when simulations terminated early */
void AnalysisRunner::UpdateReport() {
  FILE         * fp=0;

  try {
    gPrintDirection.SatScanPrintf("\nPrinting analysis results to file...\n");
    if (gParameters.GetIsSequentialScanning())
      DisplayTopCluster();
    else
      DisplayTopClusters();
    //open result output file stream  
    OpenReportFile(fp, true);
    if (giNumSimsExecuted >= 19 && giClustersReported > 0) {
      // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
      fprintf(fp, "The %s value required for an observed\n",
             (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"));
      fprintf(fp, "cluster to be significant at level\n");
      if (giNumSimsExecuted >= 99)
        fprintf(fp,"... 0.01: %f\n", gSimulatedRatios.GetAlpha01());
      if (giNumSimsExecuted >= 19)
        fprintf(fp,"... 0.05: %f\n", gSimulatedRatios.GetAlpha05());
      fprintf(fp, "\n");
    }
    if (gParameters.GetIsPowerCalculated()) {
      fprintf(fp,"Percentage of Monte Carlo replications with a likelihood greater than\n");
      fprintf(fp,"... X (%f) : %f\n", gParameters.GetPowerCalculationX(),
              ((double)giPower_X_Count)/giNumSimsExecuted);
      fprintf(fp,"... Y (%f) : %f\n\n", gParameters.GetPowerCalculationY(),
              ((double)giPower_Y_Count)/giNumSimsExecuted);
    }
    if (giClustersReported > 0 && giNumSimsExecuted < gParameters.GetNumReplicationsRequested()) {
      fprintf(fp, "\nNOTE: The optional sequential procedure was used to terminate the\n");
      fprintf(fp, "      simulations early for large p-values. This means that the\n");
      fprintf(fp, "      reported p-values are slightly conservative.\n");
    }
    fclose(fp);
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("UpdateReport()","AnalysisRunner");
    throw;
  }
}

