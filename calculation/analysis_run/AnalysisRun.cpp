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
#include "stsMCSimReporter.h"
#include "stsMCSimContinuationPolicy.h"
#include "contractor.h"
#include "PurelySpatialCentricAnalysis.h"
#include "SpaceTimeCentricAnalysis.h"
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalCentricAnalysis.h"
#include "SpaceTimeIncludePureCentricAnalysis.h"

/** constructor */
AnalysisRunner::AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection)
               :gParameters(Parameters), gStartTime(StartTime), gPrintDirection(PrintDirection) {
  try {
    Init();
    Setup();
    Execute();
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
    delete gpSignificantRatios;
  }
  catch (...) {}
}

/** calculates most likely clusters in real data */
void AnalysisRunner::CalculateMostLikelyClusters() {
  CAnalysis                  * pAnalysis=0;
  AbtractDataSetGateway      * pDataSetGateway=0;

  try {
    //display process heading
    DisplayFindClusterHeading();
    //allocate date gateway object
    pDataSetGateway = gpDataHub->GetDataSetHandler().GetNewDataGatewayObject();
    gpDataHub->GetDataSetHandler().GetDataGateway(*pDataSetGateway);
    //get analysis object
    pAnalysis = GetNewAnalysisObject();
    //allocate objects used in 'FindTopClusters()' process
    pAnalysis->AllocateTopClustersObjects(*pDataSetGateway);
    //calculate most likely clusters
    gpDataHub->SetActiveNeighborReferenceType(CSaTScanData::REPORTED);
    pAnalysis->FindTopClusters(*pDataSetGateway, gTopClustersContainer);
    delete pDataSetGateway; pDataSetGateway=0;
    delete pAnalysis; pAnalysis=0;
    //display the loglikelihood of most likely cluster
    if (!gPrintDirection.GetIsCanceled())
      DisplayTopClusterLogLikelihood();
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
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
    gParameters.DisplayAnalysisSummary(fp);
    gParameters.DisplayAdjustments(fp, gpDataHub->GetDataSetHandler());
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
  FILE          * fp=0;

  try {
    if (gTopClustersContainer.GetNumClustersRetained() > 0) {
      //open result output file
      OpenReportFile(fp, true);
      //get most likely cluster
      const CCluster& TopCluster = gTopClustersContainer.GetTopRankedCluster();
      //only report clutser if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
      if (TopCluster.m_nRatio >= gdMinRatioToReport && (giNumSimsExecuted == 0 || TopCluster.GetRank()  <= giNumSimsExecuted)) {
        ++giClustersReported;
        switch(giAnalysisCount) {
          case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
          case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
          default : fprintf(fp, "                  _____________________________\n\n");
        }
        //print cluster definition to file stream
        TopCluster.Display(fp, *gpDataHub, giClustersReported, giNumSimsExecuted);
        //print cluster definition to 'cluster information' record buffer
        if (gParameters.GetOutputClusterLevelFiles())
          ClusterInformationWriter(*gpDataHub, giNumSimsExecuted < 99, giAnalysisCount > 1).Write(TopCluster, 1, giNumSimsExecuted);
        //print cluster definition to 'location information' record buffer
        if (gParameters.GetOutputAreaSpecificFiles()) {
          LocationInformationWriter Writer(gParameters, giNumSimsExecuted < 99, giAnalysisCount > 1);
          TopCluster.Write(Writer, *gpDataHub, giClustersReported, giNumSimsExecuted);
        }
        //check track of whether this cluster was significant in top five percentage
        if (GetIsCalculatingSignificantRatios() && TopCluster.m_nRatio > gpSignificantRatios->GetAlpha05())
          ++guwSignificantAt005;
      }
      fprintf(fp, "\n");
      fclose(fp); fp=0;
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
  std::auto_ptr<LocationInformationWriter> ClusterLocationWriter;
  std::auto_ptr<ClusterInformationWriter>  ClusterWriter;
  clock_t                                  lStartTime;
  FILE                                   * fp=0;

  try {
    //if creating 'location information' files, create record data buffers
    if (gParameters.GetOutputAreaSpecificFiles())
      ClusterLocationWriter.reset(new LocationInformationWriter(gParameters, giNumSimsExecuted < 99));

    //if creating 'cluster information' files, create record data buffers
    if (gParameters.GetOutputClusterLevelFiles())
      ClusterWriter.reset(new ClusterInformationWriter(*gpDataHub, giNumSimsExecuted < 99));

    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(giNumSimsExecuted == 0 ? std::min(10, gTopClustersContainer.GetNumClustersRetained()) : gTopClustersContainer.GetNumClustersRetained());
    lStartTime = clock(); //get clock for calculating output time
    //open result output file
    OpenReportFile(fp, true);

    for (int i=0; i < gTopClustersContainer.GetNumClustersRetained(); ++i) {
       gPrintDirection.SatScanPrintf("Reporting cluster %i of %i\n", i + 1, gTopClustersContainer.GetNumClustersRetained());
       //report estimate of time to report all clusters
       if (i==9)
         ReportTimeEstimate(lStartTime, gTopClustersContainer.GetNumClustersRetained(), i, &gPrintDirection);
       //get reference to i'th top cluster  
       const CCluster& TopCluster = gTopClustersContainer.GetCluster(i);
       //write cluster details to 'cluster information' file
       if (ClusterWriter.get())
         ClusterWriter->Write(TopCluster, i+1, giNumSimsExecuted);
       //write cluster details to results file and 'location information' files -- only report
       //cluster if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
       if (i < tNumClustersToDisplay && TopCluster.m_nRatio >= gdMinRatioToReport && (giNumSimsExecuted == 0 || TopCluster.GetRank() <= giNumSimsExecuted)) {
           ++giClustersReported;
           switch (giClustersReported) {
             case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
             case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
             default : fprintf(fp, "\n"); break;
           }
           //print cluster definition to file stream
           TopCluster.Display(fp, *gpDataHub, giClustersReported, giNumSimsExecuted);
           //check track of whether this cluster was significant in top five percentage
           if (GetIsCalculatingSignificantRatios() && TopCluster.m_nRatio > gpSignificantRatios->GetAlpha05())
             ++guwSignificantAt005;
           //print cluster definition to 'location information' record buffer
           if (gParameters.GetOutputAreaSpecificFiles())
             TopCluster.Write(*ClusterLocationWriter, *gpDataHub, giClustersReported, giNumSimsExecuted);
       }
       //we no longer will be requesting neighbor information for this centroid - we can delete
       //neighbor information - which might have been just calculated at beginning of this loop
       gpDataHub->FreeNeighborInfo(TopCluster.GetCentroidIndex());
    }
    fprintf(fp, "\n");
    fclose(fp); fp=0;
  }
  catch (ZdException &x) {
    fclose(fp);
    x.AddCallpath("DisplayTopClusters()","AnalysisRunner");
    throw;
  }
}

/** Executes analysis - conditionally running successive or centric processes. */
void AnalysisRunner::Execute() {
  double        dSuccessiveMemoryDemands(0), dCentricMemoryDemands(0), dPercentage;

  //read data
  gpDataHub->ReadDataFromFiles();
  //calculate expected cases
  gpDataHub->CalculateExpectedCases();
  //validate that data set contains cases
  for (unsigned int i=0; i < gpDataHub->GetDataSetHandler().GetNumDataSets(); ++i)
     if (gpDataHub->GetDataSetHandler().GetDataSet(i).GetTotalCases() == 0)
       GenerateResolvableException("Error: No cases found in data set %u.\n","Execute()", i);

  switch (gParameters.GetMaxGeographicClusterSizeType()) {
    case PERCENTOFPOPULATIONTYPE     :
    case PERCENTOFPOPULATIONFILETYPE : dPercentage = gParameters.GetMaximumGeographicClusterSize() / 100.0; break;
    case DISTANCETYPE                : //Purely as a guess, we'll assume that the distance the user specified
                                       //equates to 10% of the population. There might be a better way to due this!
                                       dPercentage = 0.1; break;
    default                          :
       ZdGenerateException("Unknown maximum spatial cluster size type '%d'.\n", "Execute()", gParameters.GetMaxGeographicClusterSizeType());
  };

  switch (gParameters.GetExecutionType()) {
    case SUCCESSIVELY : ExecuteSuccessively(); break;
    case CENTRICALLY  : ExecuteCentrically(); break;
    case AUTOMATIC    :
    default           :
      if (gpDataHub->GetNumTracts() < std::numeric_limits<unsigned short>::max())
        dSuccessiveMemoryDemands = (double)sizeof(unsigned short**) * (double)(gParameters.GetNumTotalEllipses()+1) +
                                   (double)(gParameters.GetNumTotalEllipses()+1) * (double)sizeof(unsigned short*) * (double)gpDataHub->m_nGridTracts +
                                   (double)(gParameters.GetNumTotalEllipses()+1) * (double)gpDataHub->m_nGridTracts * (double)sizeof(unsigned short) * (double)gpDataHub->GetNumTracts() * dPercentage;
      else
        dSuccessiveMemoryDemands = (double)sizeof(tract_t**) * (double)(gParameters.GetNumTotalEllipses()+1) +
                                   (double)(gParameters.GetNumTotalEllipses()+1) * (double)sizeof(tract_t*) * (double)gpDataHub->m_nGridTracts +
                                   (double)(gParameters.GetNumTotalEllipses()+1) * (double)gpDataHub->m_nGridTracts * (double)sizeof(tract_t) * (double)gpDataHub->GetNumTracts() * dPercentage;

      dCentricMemoryDemands = (double)gParameters.GetNumReplicationsRequested() *
                              gpDataHub->GetDataSetHandler().GetSimulationDataSetAllocationRequirements();
      if (gpDataHub->GetDataSetHandler().GetNumDataSets() == 1)
        dCentricMemoryDemands += (double)gParameters.GetNumReplicationsRequested() *
                                 (double)sizeof(measure_t) * (double)gpDataHub->GetDataSetHandler().GetDataSet().GetTotalCases() *
                                 (gParameters.GetAreaScanRateType() == HIGHANDLOW ? 2.0 : 1.0);
     if (gpDataHub->GetNumTracts() < std::numeric_limits<unsigned short>::max())
       dCentricMemoryDemands += (double)(gParameters.GetNumTotalEllipses()+1) * (double)sizeof(unsigned short) * (double)gpDataHub->GetNumTracts() * dPercentage;
     else
       dCentricMemoryDemands += (double)(gParameters.GetNumTotalEllipses()+1) * (double)sizeof(tract_t) * (double)gpDataHub->GetNumTracts() * dPercentage;

     if ((gParameters.GetIsPurelyTemporalAnalysis() || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND ||
         (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK)) ||
          dSuccessiveMemoryDemands < GetAvailablePhysicalMemory() || dSuccessiveMemoryDemands < dCentricMemoryDemands)
       ExecuteSuccessively();
     else
       ExecuteCentrically();
  };
}

/** starts analysis execution - evaluating real data then replications */
void AnalysisRunner::ExecuteSuccessively() {
  bool  bContinue;

  try {
    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;
    //calculate number of neighboring locations about each centroid
    gpDataHub->FindNeighbors();
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
    x.AddCallpath("ExecuteSuccessively()","AnalysisRunner");
    throw;
  }
}

/** starts analysis execution - development */
void AnalysisRunner::ExecuteCentrically() {
  bool                  bContinue;

  try {
    DataSetHandler      & DataHandler = gpDataHub->GetDataSetHandler();

    //detect user cancellation
    if (gPrintDirection.GetIsCanceled())
      return;

    //create result file report
    CreateReport();

    //start analyzing data
    do {
      ++giAnalysisCount;
      guwSignificantAt005 = 0;
      giNumSimsExecuted = 0;

      //simualtion data randomizer
      RandomizerContainer_t                       RandomizationContainer;
      //allocate a simulation data set for each requested replication
      std::vector<SimulationDataContainer_t>      vRandomizedDataSets(gParameters.GetNumReplicationsRequested());
      //allocate a data gateway for each requested replication
      ZdPointerVector<AbtractDataSetGateway>      vSimDataGateways(gParameters.GetNumReplicationsRequested());
      //allocate an array to contain simulation llr values
      AbstractCentricAnalysis::CalculatedRatioContainer_t SimulationRatios;
      //data gateway object for real data
      std::auto_ptr<AbtractDataSetGateway>        DataSetGateway(DataHandler.GetNewDataGatewayObject());
      //centroid neighbor information objects
      std::vector<CentroidNeighbors>              CentroidNeighbors(gpDataHub->GetParameters().GetNumTotalEllipses() + 1);
      std::auto_ptr<LoglikelihoodRatioWriter>     RatioWriter;
      std::auto_ptr<AbstractCentricAnalysis>      CentricAnalysis;
      //centroid neighbors calculator
      std::auto_ptr<CentroidNeighborCalculator>   CentroidCalculator;

      //get data randomizers
      DataHandler.GetRandomizerContainer(RandomizationContainer);
      //set data gateway object
      DataHandler.GetDataGateway(*DataSetGateway);

      gPrintDirection.SatScanPrintf("Calculating simulation data for %u simulations\n\n", gParameters.GetNumReplicationsRequested());
      //create simulation data sets -- randomize each and set corresponding data gateway object
      for (unsigned int i=0; i < gParameters.GetNumReplicationsRequested() && !gPrintDirection.GetIsCanceled(); ++i) {
         SimulationDataContainer_t& thisDataCollection = vRandomizedDataSets[i];
         //create new simulation data set object for each data set of this simulation
         for (unsigned int j=0; j < DataHandler.GetNumDataSets(); ++j)
            thisDataCollection.push_back(new SimDataSet(gpDataHub->GetNumTimeIntervals(), gpDataHub->GetNumTracts(), j + 1));
         //allocate appropriate data structure for given data set handler (probablility model)
         DataHandler.AllocateSimulationData(thisDataCollection);
         //randomize data
         gpDataHub->RandomizeData(RandomizationContainer, thisDataCollection, i + 1);
         //allocate and set data gateway object
         vSimDataGateways[i] = DataHandler.GetNewDataGatewayObject();
         DataHandler.GetSimulationDataGateway(*vSimDataGateways[i], thisDataCollection);
      }

      //detect user cancellation
      if (gPrintDirection.GetIsCanceled())
        return;
        
      //allocate analysis object
      CentricAnalysis.reset(GetNewCentricAnalysisObject(*DataSetGateway, vSimDataGateways));
      //allocate centroid neigbor calculator
      if (gParameters.GetMaxGeographicClusterSizeType() == DISTANCETYPE)
        CentroidCalculator.reset(new CentroidNeighborCalculatorByDistance(*gpDataHub, gPrintDirection));
      else
        CentroidCalculator.reset(new CentroidNeighborCalculatorByPopulation(*gpDataHub, gPrintDirection));

      //analyze real and simulation data about each centroid
      clock_t  tStartTime = clock();
      for (int c=0; c < gpDataHub->m_nGridTracts && !gPrintDirection.GetIsCanceled(); ++c) {
         gPrintDirection.SatScanPrintf("Calculating top cluster about centroid %i of %i\n", c + 1, gpDataHub->m_nGridTracts);
         CentricAnalysis->ExecuteAboutCentroids(c, gTopClustersContainer, *CentroidCalculator, *DataSetGateway, vSimDataGateways);
         //report estimation of total execution time
         if (c==9)
           ReportTimeEstimate(tStartTime, gpDataHub->m_nGridTracts, c+1, &gPrintDirection);
     }
     //detect user cancellation
     if (gPrintDirection.GetIsCanceled())
       return;
     if (gParameters.GetIncludePurelyTemporalClusters())
       CentricAnalysis->CalculatePurelyTemporalCluster(gTopClustersContainer, *DataSetGateway);

     CentricAnalysis->RetrieveLoglikelihoodRatios(SimulationRatios);
     giNumSimsExecuted = gParameters.GetNumReplicationsRequested();
     //free memory of objects that will no longer be used
     // - we might need the memory for recalculating neighbors in geographical overlap code 
     vRandomizedDataSets.clear();
     CentricAnalysis.reset(0);
     vSimDataGateways.DeleteAllElements();
     //detect user cancellation
     if (gPrintDirection.GetIsCanceled())
       return;
     //rank top cluster and apply criteria for reporting secondary clusters
     gTopClustersContainer.RankTopClusters(gParameters, *gpDataHub, gPrintDirection);

     //report calculated simulation llr values
     if (GetIsCalculatingSignificantRatios())
       gpSignificantRatios->Initialize();
     if (gParameters.GetOutputSimLoglikeliRatiosFiles() && giAnalysisCount == 1)
       RatioWriter.reset(new LoglikelihoodRatioWriter(gParameters));
     std::vector<double>::iterator  itr=SimulationRatios->begin(), itr_end=SimulationRatios->end();
     for (; itr != itr_end; ++itr) {
       //update most likely clusters given latest simulated loglikelihood ratio
       gTopClustersContainer.UpdateTopClustersRank(*itr);
       //update significance indicator
       UpdateSignificantRatiosList(*itr);
       //update power calculations
       UpdatePowerCounts(*itr);
       //update simulated loglikelihood record buffer
       if(RatioWriter.get()) RatioWriter->Write(*itr);
     }
     SimulationRatios.reset();

     //report clusters
     UpdateReport();
     //log history for first analysis run
     if (giAnalysisCount == 1) {
       gPrintDirection.SatScanPrintf("Logging run history...\n");
       stsRunHistoryFile(gParameters, gPrintDirection).LogNewHistory(*this);
     }
     //report additional output file: 'relative risks for each location'
     CreateRelativeRiskFile();
     //repeat analysis - sequential scan
     if ((bContinue = RepeatAnalysis()) == true) {
       RemoveTopClusterData();
       //detect user cancellation
       if (gPrintDirection.GetIsCanceled()) return;
     }
    } while (bContinue);
    
    //finish report
    FinalizeReport();
  }
  catch (ZdException &x) {
    x.AddCallpath("ExecuteCentrically()","AnalysisRunner");
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
  FILE              * fp=0;
  time_t              CompletionTime;
  double              nTotalTime,  nSeconds,  nMinutes,  nHours;
  const char        * szHours = "hours";
  const char        * szMinutes = "minutes";
  const char        * szSeconds = "seconds";
  AsciiPrintFormat    PrintFormat;
  ZdString            sBuffer;

  try {
    gPrintDirection.SatScanPrintf("Printing analysis settings to the results file...\n");
    OpenReportFile(fp, true);
    PrintFormat.SetMarginsAsOverviewSection();
    //if zero clusters retained in real data, then no clusters of significance were retained.  
    if (gTopClustersContainer.GetNumClustersRetained() == 0) {
      fprintf(fp, "\nNo clusters were found.\n");
      if (gParameters.GetAreaScanRateType() == HIGH)
        sBuffer = "All areas scanned had either only one case or equal or fewer cases than expected.";
      else if (gParameters.GetAreaScanRateType() == LOW)
        sBuffer = "All areas scanned had either only one case or equal or greater cases than expected.";
      else
        sBuffer = "All areas scanned had either only one case or cases equal to expected.";
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);  
    }
    else if (giClustersReported == 0) {
      fprintf(fp, "\nNo clusters reported.\n");
      if (gTopClustersContainer.GetTopRankedCluster().GetRatio() < gdMinRatioToReport)
        sBuffer.printf("All clusters had a ratio less than %g.\n", gdMinRatioToReport);
      else
        sBuffer.printf("All clusters had a rank greater than %i.\n", gParameters.GetNumReplicationsRequested());
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);  
    }
    else if (gParameters.GetNumReplicationsRequested() == 0) {
      fprintf(fp, "\n");
      sBuffer = "Note: As the number of Monte Carlo replications was set to "
                "zero, no hypothesis testing was done and no p-values were printed.";
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }
    else if (gParameters.GetNumReplicationsRequested() <= 98) {
      fprintf(fp, "\n");
      sBuffer = "Note: The number of Monte Carlo replications was set too low, "
                "and a meaningful hypothesis test cannot be done. Consequently, "
                "no p-values were printed.";
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }

    if (gParameters.GetProbabilityModelType() == POISSON)
      gpDataHub->GetDataSetHandler().ReportZeroPops(*gpDataHub, fp, &gPrintDirection);

    gpDataHub->GetTInfo()->tiReportDuplicateTracts(fp);
    gParameters.DisplayParameters(fp, giNumSimsExecuted, gpDataHub->GetDataSetHandler());
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

/** Returns available random access memory. */
double AnalysisRunner::GetAvailablePhysicalMemory() const {
  double dTotalPhysicalMemory(0), dAvailablePhysicalMemory(0);

#ifdef INTEL_BASED
  MEMORYSTATUS stat;
  GlobalMemoryStatus (&stat);
  dTotalPhysicalMemory = stat.dwTotalPhys;
  dAvailablePhysicalMemory = stat.dwAvailPhys;
#else
  dTotalPhysicalMemory = sysconf(_SC_PHYS_PAGES);
  dTotalPhysicalMemory *= sysconf(_SC_PAGESIZE);
  dAvailablePhysicalMemory = sysconf(_SC_AVPHYS_PAGES);   
  dAvailablePhysicalMemory *= sysconf(_SC_PAGESIZE);
#endif

  return dAvailablePhysicalMemory;
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
      default :
        ZdException::Generate("Unknown analysis type '%d'.\n", "GetNewCentricAnalysisObject()", gParameters.GetAnalysisType());
    };
   }
  catch (ZdException &x) {
    x.AddCallpath("GetNewAnalysisObject()","AnalysisRunner");
    throw;
  }
  return 0;
}

/** returns new AbstractCentricAnalysis object */
AbstractCentricAnalysis * AnalysisRunner::GetNewCentricAnalysisObject(const AbtractDataSetGateway& RealDataGateway,
                                                                      const ZdPointerVector<AbtractDataSetGateway>& vSimDataGateways) const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL :
          if (gParameters.GetRiskType() == STANDARDRISK)
            return new PurelySpatialCentricAnalysis(gParameters, *gpDataHub, gPrintDirection, RealDataGateway, vSimDataGateways);
          else
            ZdGenerateException("No implementation for purely spatial analysis with isotonic scan for centric evaluation.\n", "GetNewCentricAnalysisObject()");
      case PURELYTEMPORAL :
      case PROSPECTIVEPURELYTEMPORAL :
            ZdGenerateException("No implementation for purely temporal analysis for centric evaluation.\n", "GetNewCentricAnalysisObject()");
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
            ZdGenerateException("No implementation for svtt analysis for centric evaluation.\n", "GetNewCentricAnalysisObject()");
      default :
        ZdException::Generate("Unknown analysis type '%d'.\n", "GetNewCentricAnalysisObject()", gParameters.GetAnalysisType());
    };
   }
  catch (ZdException &x) {
    x.AddCallpath("GetNewCentricAnalysisObject()","AnalysisRunner");
    throw;
  }
  return 0;
}

double AnalysisRunner::GetSimRatio01() const {
  return gpSignificantRatios ? gpSignificantRatios->GetAlpha01() : 0;
}
double AnalysisRunner::GetSimRatio05() const {
  return gpSignificantRatios ? gpSignificantRatios->GetAlpha05() : 0;
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
  gpSignificantRatios=0;
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

  double                      dSimulatedRatio;
  unsigned int                iSimulationNumber;
  char                      * sReplicationFormatString;
  AbtractDataSetGateway     * pDataGateway=0;
  CAnalysis                 * pAnalysis=0;
  SimulationDataContainer_t   SimulationDataContainer;
  RandomizerContainer_t       RandomizationContainer;

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
    giNumSimsExecuted = 0;

    std::deque< std::pair<unsigned int, double> > qParamsAndResults;
    for (unsigned int ui = 0; ui < gParameters.GetNumReplicationsRequested(); ++ui)
    {
       qParamsAndResults.push_back(std::make_pair(ui + 1,0));
    }
    stsMCSimContinuationPolicy CtPlcy(gPrintDirection);
    stsMCSimReporter Rptr(gParameters, CtPlcy, gTopClustersContainer, gPrintDirection, sReplicationFormatString, *this);
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
  double                                  dSimulatedRatio;
  unsigned int                            iSimulationNumber;
  char                                  * sReplicationFormatString;
  AbtractDataSetGateway                 * pDataGateway=0;
  CAnalysis                             * pAnalysis=0;
  SimulationDataContainer_t               SimulationDataContainer;
  RandomizerContainer_t                   RandomizationContainer;
  std::auto_ptr<LoglikelihoodRatioWriter> RatioWriter;

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
      RatioWriter.reset(new LoglikelihoodRatioWriter(gParameters));
    //set/reset loglikelihood ratio significance indicator
    if (GetIsCalculatingSignificantRatios()) gpSignificantRatios->Initialize();
    //get container for simulation data - this data will be modified in the randomize process
    GetDataHub().GetDataSetHandler().GetSimulationDataContainer(SimulationDataContainer);
    //get container of data randomizers - these will modify the simulation data
    GetDataHub().GetDataSetHandler().GetRandomizerContainer(RandomizationContainer);
    //get data gateway given dataset handler's real data and simulated data structures
    pDataGateway = GetDataHub().GetDataSetHandler().GetNewDataGatewayObject();
    GetDataHub().GetDataSetHandler().GetSimulationDataGateway(*pDataGateway, SimulationDataContainer);
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
        dSimulatedRatio = pAnalysis->ExecuteSimulation(*pDataGateway);
        //update most likely clusters given latest simulated loglikelihood ratio
        gTopClustersContainer.UpdateTopClustersRank(dSimulatedRatio);
        //update significance indicator
        UpdateSignificantRatiosList(dSimulatedRatio);
        //update power calculations
        UpdatePowerCounts(dSimulatedRatio);
        //update simulated loglikelihood record buffer
        if(RatioWriter.get()) RatioWriter->Write(dSimulatedRatio);
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
      gpDataHub->SetActiveNeighborReferenceType(CSaTScanData::MAXIMUM);
      //$$if (gParameters.GetRestrictingMaximumReportedGeoClusterSize())
      //$$  gpDataHub->FindNeighbors(true);
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
    if (gParameters.GetReportCriticalValues() && gParameters.GetNumReplicationsRequested() >= 19)
      gpSignificantRatios = new CSignificantRatios05(gParameters.GetNumReplicationsRequested());
  }
  catch (ZdException &x) {
    delete gpSignificantRatios; gpSignificantRatios=0;
    delete gpDataHub; gpDataHub=0;
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
  FILE                * fp=0;
  AsciiPrintFormat      PrintFormat;
  ZdString              sBuffer;

  try {
    gPrintDirection.SatScanPrintf("\nPrinting analysis results to file...\n");
    if (gParameters.GetIsSequentialScanning())
      DisplayTopCluster();
    else
      DisplayTopClusters();
    //open result output file stream
    OpenReportFile(fp, true);
    PrintFormat.SetMarginsAsOverviewSection();
    if (GetIsCalculatingSignificantRatios() && giNumSimsExecuted >= 19 && giClustersReported > 0) {
      // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
      sBuffer.printf("A cluster is statistically significant when its %s "
                     "is greater than the critical value, which is, for significance level:",
                     (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"));
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);             
      if (giNumSimsExecuted >= 99)
        fprintf(fp,"... 0.01: %f\n", gpSignificantRatios->GetAlpha01());
      if (giNumSimsExecuted >= 19)
        fprintf(fp,"... 0.05: %f\n", gpSignificantRatios->GetAlpha05());
    }
    if (gParameters.GetIsPowerCalculated()) {
      sBuffer = "Percentage of Monte Carlo replications with a likelihood greater than";
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
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

/** Updates list of significant ratio, if structure allocated. */
void AnalysisRunner::UpdateSignificantRatiosList(double dRatio) {
  if (gpSignificantRatios) gpSignificantRatios->AddRatio(dRatio);
}

