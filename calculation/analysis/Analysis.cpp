#include "SaTScan.h"
#pragma hdrstop                                       
#include "Analysis.h"

#define INCLUDE_RUN_HISTORY    // define to determine whether or not we should log run history, if included
                                  // then we will, if not then we won't - AJV 10/4/2002

#include "stsRunHistoryFile.h"
#include "stsClusterData.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"
#include "stsLogLikelihood.h"
#include "stsAreaSpecificData.h"
#include "PrintQueue.h"
#include "PoissonLikelihoodCalculation.h"
#include "BernoulliLikelihoodCalculation.h"
#include "WilcoxonLikelihoodCalculation.h"
#include "NormalLikelihoodCalculation.h"
#include "ExponentialLikelihoodCalculation.h"

/** constructor */
TopClustersContainer::TopClustersContainer(const CSaTScanData & Data)
                     :gData(Data){}

/** constructor */
TopClustersContainer::~TopClustersContainer(){}

/** Returns reference to top cluster for shape offset. This function is tightly
    coupled to the presumption that CAnalysis::GetTopCluster() iterates over
    circles and ellispes in such a way that an offset of zero is a circle, and
    anything greater offset corresponds to an particular rotation of an ellipse shape.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead, a copy should be made, using operator= method
          and appropriate casting, to store cluster information. */
CCluster & TopClustersContainer::GetTopCluster(int iShapeOffset) {
  CCluster    * pTopCluster=0;
  int           iEllipse, iBoundry=0;

  if (iShapeOffset && gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetDuczmalCorrectEllipses())
    for (iEllipse=0; iEllipse < gData.GetParameters().GetNumRequestedEllipses() && !pTopCluster; ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gData.GetParameters().GetEllipseRotations()[iEllipse];
       if (iShapeOffset <= iBoundry)
         pTopCluster = gvTopShapeClusters[iEllipse + 1];
    }
  else
    pTopCluster = gvTopShapeClusters[0];

  return *pTopCluster;
}

/** Returns the top cluster for all shapes, taking into account the option of
    Duczmal Compactness correction for ellispes.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead a copy should be made, using operator= method
          and appropriate casting, to store cluster information. 
    NOTE: This function should only be called after all iterations in
          CAnalysis::GetTopCluster() are completed. */
CCluster & TopClustersContainer::GetTopCluster() {
  CCluster    * pTopCluster;

  //set the maximum cluster to the circle shape initially
  pTopCluster = gvTopShapeClusters[0];
  //apply compactness correction
  pTopCluster->m_nRatio *= pTopCluster->m_DuczmalCorrection;
  //if the there are ellipses, compare current top cluster against them
  //note: we don't have to be concerned with whether we are comparing circles and ellipses,
  //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
  for (size_t t=1; t < gvTopShapeClusters.size(); t++) {
     if (gvTopShapeClusters[t]->ClusterDefined()) {
       //apply compactness correction
       gvTopShapeClusters[t]->m_nRatio *= gvTopShapeClusters[t]->m_DuczmalCorrection;
       //compare against current top cluster
       if (gvTopShapeClusters[t]->m_nRatio > pTopCluster->m_nRatio)
         pTopCluster = gvTopShapeClusters[t];
     }    
  }

  pTopCluster->SetStartAndEndDates(gData.GetTimeIntervalStartTimes(), gData.m_nTimeIntervals);
  return *pTopCluster;
}

void TopClustersContainer::Reset(int iCenter) {
  for (size_t t=0; t < gvTopShapeClusters.size(); ++t)
     gvTopShapeClusters[t]->Initialize(iCenter);
}

/** Initialzies the vector of top clusters to cloned copies of cluster,
    taking into account whether spatial shape will be a factor in analysis. */
void TopClustersContainer::SetTopClusters(const CCluster& InitialCluster) {
  int   i, iNumTopClusters;

  try {
    gvTopShapeClusters.DeleteAllElements();
    //if there are ellipses and duczmal correction is true, then we need
    //a top cluster for the circle and each ellipse shape
    if (gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetDuczmalCorrectEllipses())
      iNumTopClusters = gData.GetParameters().GetNumRequestedEllipses() + 1;
    else
    //else there is only one top cluster - regardless of whether there are ellipses
      iNumTopClusters = 1;

    for (i=0; i < iNumTopClusters; i++)
       gvTopShapeClusters.push_back(InitialCluster.Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","TopClustersContainer");
    throw;
  }
}

/** constructor */
CAnalysis::CAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
          :SimRatios(pParameters->GetNumReplicationsRequested(), pPrintDirection),
           m_pParameters(pParameters), m_pData(pData), gpPrintDirection(pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CAnalysis");
    throw;
  }
}

/** destructor */
CAnalysis::~CAnalysis() {
  try {
    InitializeTopClusterList();
    delete [] m_pTopClusters;
    delete gpClusterDataFactory;
    delete gpLikelihoodCalculator;
#ifdef DEBUGPROSPECTIVETIME
    fclose(m_pDebugFile);
#endif
  }
  catch(...){}
}

/** allocates Likelihood object
    - this function should not be called prior to calculation of total cases
      and total measure                                                       */
void CAnalysis::AllocateLikelihoodObject() {
  try {
    //create likelihood calculator
    if (m_pParameters->GetProbabiltyModelType() == BERNOULLI)
      gpLikelihoodCalculator = new BernoulliLikelihoodCalculator(m_pData->GetTotalCases(), m_pData->GetTotalMeasure());
    else if (m_pParameters->GetProbabiltyModelType() == NORMAL)
      gpLikelihoodCalculator = new NormalLikelihoodCalculator(m_pData->GetTotalCases(), m_pData->GetTotalMeasure());
    else if (m_pParameters->GetProbabiltyModelType() == SURVIVAL)
      gpLikelihoodCalculator = new ExponentialLikelihoodCalculator(m_pData->GetTotalCases(), m_pData->GetTotalMeasure());
    else if (m_pParameters->GetProbabiltyModelType() == RANK)
      gpLikelihoodCalculator = new WilcoxonLikelihoodCalculator(m_pData->GetTotalCases(), m_pData->GetTotalMeasure());
    else
      gpLikelihoodCalculator = new PoissonLikelihoodCalculator(*m_pData);
  }
  catch (ZdException &x) {
    delete gpLikelihoodCalculator;
    x.AddCallpath("AllocateLikelihoodObject()","CAnalysis");
    throw;
  }
}

bool CAnalysis::Execute(time_t RunTime) {
   bool bContinue;
   long lRunNumber = 0;

   try {
      SetMaxNumClusters();
      AllocateTopClusterList();                //Allocate array which will store most likely clusters.
      m_pData->GetDataStreamHandler().AllocateSimulationStructures(); 
      if (!m_pData->CalculateExpectedCases())        //Calculate expected number of cases.
         return false;
      if (m_pData->GetTotalCases() < 1)
         ZdException::Generate("Error: No cases found in input data.\n","CAnalysis");    //KR V.2.1
      AllocateLikelihoodObject();
      //For circle and each ellipse, find closest neighboring tracts points for
      //each grid point limiting distance by max circle size and cumulated measure.
      if (gpPrintDirection->GetIsCanceled() || !m_pData->FindNeighbors(false))
        return false;

      if (gpPrintDirection->GetIsCanceled() || !CreateReport(RunTime))
        return false;

#ifdef DEBUGANALYSIS
      DisplayVersion(m_pDebugFile, 0);
      fprintf(m_pDebugFile, "Program run on: %s", ctime(&RunTime));
      m_pParameters->DisplayParameters(m_pDebugFile, giSimulationNumber);
#endif
     // only create one history object and record to it only once ( first iteration if sequential analysis ) -- AJV
     stsRunHistoryFile historyFile(m_pParameters->GetRunHistoryFilename(), *gpPrintDirection,
                                   m_pParameters->GetNumReplicationsRequested() > 98, m_pParameters->GetIsSequentialScanning());
     lRunNumber = historyFile.GetRunNumber();
     do {
         ++m_nAnalysisCount;
#ifdef DEBUGANALYSIS
        fprintf(m_pDebugFile, "Analysis Loop #%i\n", m_nAnalysisCount);
        m_pData->DisplaySummary2(m_pDebugFile);
        fprintf(m_pDebugFile, "\n---- Actual Data ----------------------\n\n");
        m_pData->DisplayCases(m_pDebugFile);
        if (m_pParameters->GetProbabiltyModelType() == BERNOULLI)
          m_pData->DisplayControls(m_pDebugFile);
        m_pData->DisplayMeasure(m_pDebugFile);
        m_pData->DisplayNeighbors(m_pDebugFile);
#endif
        //For each grid point, find the cluster with the greatest loglikihood.
        //Removes any clusters that violate criteria for reporting secondary clusters.
        if (gpPrintDirection->GetIsCanceled() || !CalculateMostLikelyClusters())
          return false;

        //Do Monte Carlo replications.
        if (m_nClustersRetained > 0)
          PerformSimulations();
        else
          gpPrintDirection->SatScanPrintf("No clusters retained.\n"); // USE return valuse from FindTopClusters to indicate this.

        if (gpPrintDirection->GetIsCanceled() || !UpdateReport(lRunNumber))
          return false;

#ifdef INCLUDE_RUN_HISTORY
        // log history for first analysis run - AJV 12/27/2002
        if (m_nAnalysisCount == 1) {
           gpPrintDirection->SatScanPrintf("\nLogging run history...\n");
           double dPVal((m_nClustersRetained > 0) ? m_pTopClusters[0]->GetPVal(giSimulationNumber) : 0.0);
           historyFile.LogNewHistory(*this, guwSignificantAt005, dPVal);
        }
#endif

        guwSignificantAt005 = 0;      // reset the number of significants back to zero for each iteration - AJV 9/10/2002

        // report to output files here before clusters are readjusted in sequential scan function RemoveTopClusters() - AJV 12/30/2002
        CreateGridOutputFile(lRunNumber);
        // report relative risks output only on first iteration - AJV
        if (m_nAnalysisCount == 1 && m_pParameters->GetOutputRelativeRisksFiles())
           m_pData->DisplayRelativeRisksForEachTract(m_pParameters->GetOutputRelativeRisksAscii(), m_pParameters->GetOutputRelativeRisksDBase());

        bContinue = RepeatAnalysis();
        if (bContinue)
          RemoveTopClusterData();
    
        if (gpPrintDirection->GetIsCanceled())
           return false;

      } while (bContinue);

      m_pData->GetDataStreamHandler().FreeSimulationStructures();
      FinalizeReport(RunTime);
   }
   catch (ZdException & x) {
      x.AddCallpath("Execute(time_t)", "CAnalysis");
      throw;
   }
  return true;
}

void CAnalysis::AllocateTopClusterList() {
   try {
      m_pTopClusters = new CCluster* [m_nMaxClusters + 1];
      if (m_pTopClusters == 0)
         SSGenerateException("Could not allocate memory for m_pTopClusters.", "CAnalysis");
      memset(m_pTopClusters, 0, (m_nMaxClusters+1) * sizeof(CCluster*));
   }
   catch (ZdException & x) {
      x.AddCallpath("AllocateTopClusterList()", "CAnalysis");
      throw;
   }
}

/** creates data stream gateway for real data set and calls FindTopClusters()
    to determine and collect most likely clusters
    - returns false if process is cancelled, else true */
bool CAnalysis::CalculateMostLikelyClusters() {
  bool                         bSuccess; 
  AbtractDataStreamGateway   * pDataStreamGateway=0;

  try {
    //display process heading
    DisplayFindClusterHeading();
    //allocate date gateway object
    pDataStreamGateway = m_pData->GetDataStreamHandler().GetNewDataGateway();
    //allocate objects used in 'FindTopClusters()' process
    AllocateTopClustersObjects(*pDataStreamGateway);
    //calculate most likely clusters
    bSuccess = FindTopClusters(*pDataStreamGateway);
    delete pDataStreamGateway; pDataStreamGateway=0;
    //display the loglikelihood of most likely cluster
    DisplayTopClusterLogLikelihood();
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("CalculateMostLikelyClusters()","CAnalysis");
    throw;
  }
  return bSuccess;
}

/**********************************************************************
 Comparison function for sorting clusters by descending m_ratio
 **********************************************************************/
/*static*/ int CAnalysis::CompareClustersByRatio(const void *a, const void *b) {
  double rdif = (*(CCluster**)b)->m_nRatio - (*(CCluster**)a)->m_nRatio;
  if (rdif < 0.0)   return -1;
  if (rdif > 0.0)   return 1;
  return 0;
} /* CompareClusters() */

bool CAnalysis::CheckForEarlyTermination(int iSimulation) const {
  float fCutOff;

  if (iSimulation == m_pParameters->GetNumReplicationsRequested())
    return false;
  if (m_nClustersRetained > 0) {
    switch (iSimulation) {
      case 99   : fCutOff = .5; break;
      case 199  : fCutOff = .4; break;
      case 499  : fCutOff = .2; break;
      case 999  : fCutOff = .1; break;
      default   : return false;
    }
    return (m_pTopClusters[0]->GetPVal(iSimulation) > fCutOff);
  }
  return false;
}

//******************************************************************************
//  Create the Most Likely Cluster Output File
//******************************************************************************
void CAnalysis::CreateGridOutputFile(const long lReportHistoryRunNumber) {
   try {
      if (m_pParameters->GetOutputClusterLevelFiles()) {
         std::auto_ptr<stsClusterData> pData( new stsClusterData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(),
                                                                 lReportHistoryRunNumber, m_pParameters->GetCoordinatesType(), m_pParameters->GetProbabiltyModelType(),
                                                                 m_pParameters->GetDimensionsOfData(), giSimulationNumber > 98,
                                                                 m_pParameters->GetNumRequestedEllipses() > 0, m_pParameters->GetDuczmalCorrectEllipses()));
         if (m_nClustersRetained)
           gpPrintDirection->SatScanPrintf("Recording results for %i cluster%s...", m_nClustersRetained, (m_nClustersRetained == 1 ? "" : "s"));
         for (int i = 0; i < m_nClustersRetained; ++i)
            pData->RecordClusterData(*m_pTopClusters[i], *m_pData, i+1, giSimulationNumber);

         // only append if analysis count is greater than 1 which implies sequential scan is taking place with
         // more than one iteration -- AJV 12/30/2002
         bool bAppend(m_nAnalysisCount > 1);        
         if (m_pParameters->GetOutputClusterLevelAscii())
            ASCIIFileWriter(pData.get(), bAppend).Print();
         if (m_pParameters->GetOutputClusterLevelDBase())
            DBaseFileWriter(pData.get(), bAppend).Print();
      }
/* testing mechanism for output files - this is a generic test class which houses several different types of fields
   test by creating an instance of the class and setting various test values to the fields and try to print out in
   each of the outputfilewriters - AJV 11/2002
      std::auto_ptr<TestOutputClass> pTest( new TestOutputClass(gpPrintDirection, m_pParameters->m_szOutputFilename) );
      pTest->AddBlankRecord();
      pTest->SetTestValues("", 0, 0.000, 0.000, 0, true);
      pTest->AddBlankRecord();
      pTest->SetTestValues("test string", 69, 4.1698321, 3.14159257136, -42, false);
      pTest->SetTestValues("very long string which might be truncated well hopefully", 12, 6.000, 6.5, 0, true);
      pTest->SetTestValues("W%#$#^#$^%#FDSF", 3049858, 0.0001, 487.623, -48763, false);
      pTest->SetTestValues("kusdyh", 654, 10, -7, 3, true);
      pTest->AddBlankRecord();

      ASCIIFileWriter(pTest.get()).Print();
      DBaseFileWriter(pTest.get()).Print();
*/
   }
   catch (ZdException & x) {
      x.AddCallpath("CreateGridOutputFile()", "CAnalysis");
      throw;
   }
}

// opens the results file and prints the header to it
// pre: none
// post: creates/overwrites results file and prints the file header
bool CAnalysis::CreateReport(time_t RunTime) {
   FILE* fp;

   try {
      OpenReportFile(fp, "w");

      fprintf(fp, "                 _____________________________\n\n");
      DisplayVersion(fp, 1);
      fprintf(fp, "                 _____________________________\n\n");

      gsStartTime = ctime(&RunTime);
      fprintf(fp,"\nProgram run on: %s\n", gsStartTime.GetCString());

      m_pParameters->DisplayAnalysisType(fp);
      m_pParameters->DisplayTimeAdjustments(fp);
      m_pData->DisplaySummary(fp);

      fclose(fp);
   }
   catch (ZdException & x) {
      fclose(fp);
      x.AddCallpath("CreateReport(time_t)", "CAnalysis");
      throw;
   }
  return true;
}

// prints message to progress output letting user know which cluster the program is currently looking for
// pre: none
// post: prints a progress message to the user
void CAnalysis::DisplayFindClusterHeading() {
   try {
      if (!m_pParameters->GetIsSequentialScanning())
         gpPrintDirection->SatScanPrintf("Finding the most likely clusters\n");
      else {
         switch(m_nAnalysisCount) {
            case  1: gpPrintDirection->SatScanPrintf("Finding the most likely cluster\n"); break;
            case  2: gpPrintDirection->SatScanPrintf("Finding the second most likely cluster\n"); break;
            case  3: gpPrintDirection->SatScanPrintf("Finding the third most likely cluster\n"); break;
            default: gpPrintDirection->SatScanPrintf("Finding the %ith most likely cluster\n", m_nAnalysisCount); break;
         }
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayFindClusterHeading()", "CAnalysis");
      throw;
   }
}

// function used by the sequential scan to report the top cluster to file, also responsible for reporting
// to area specific output file, if applicable
// pre: none
// post: prints the information for the top cluster to the area, if applicable, and result files
void CAnalysis::DisplayTopCluster(double nMinRatio, const long lReportHistoryRunNumber, FILE* fp) {
  measure_t                             nMinMeasure = 0;
  std::auto_ptr<stsAreaSpecificData>    pData;

  try {
    if (m_nClustersRetained > 0) {
      if (m_pParameters->GetOutputAreaSpecificFiles())
        pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(), lReportHistoryRunNumber, m_pParameters->GetNumReplicationsRequested() > 98));
      if (m_pTopClusters[0]->m_nRatio > nMinRatio && (giSimulationNumber == 0 || m_pTopClusters[0]->m_nRank  <= giSimulationNumber)) {
        ++m_nClustersReported;
        switch(m_nAnalysisCount) {
          case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
          case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
          default : fprintf(fp,"                  _____________________________\n\n");
        }
        m_pTopClusters[0]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure, giSimulationNumber);
        if (m_pTopClusters[0]->m_nRatio > SimRatios.GetAlpha05())
          ++guwSignificantAt005;
        // if we want area specific report, set the report pointer in cluster
        if (pData.get()) {
          m_pTopClusters[0]->SetAreaReport(pData.get());
          m_pTopClusters[0]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, giSimulationNumber,
                                                  lReportHistoryRunNumber, true, giSimulationNumber > 98, 0, 0, ' ', NULL, false);
        }
      }
      fprintf(fp, "\n");

      // need to do append here because this is a sequential function, but don't append on the first analysis so that
      // the file can be deleted if it exists and started over fresh - AJV
      bool bAppendData(m_nAnalysisCount > 1);
      if (m_pParameters->GetOutputAreaSpecificAscii())
         ASCIIFileWriter(pData.get(), bAppendData).Print();// print area ASCII
      if (m_pParameters->GetOutputAreaSpecificDBase())
         DBaseFileWriter(pData.get(), bAppendData).Print();// print area dBase
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayTopCluster(double, int, File *)", "CAnalysis");
    throw;
  }
}

// function used by the non-sequential scan to report the top clusters to file, also responsible for reporting
// to area specific output file, if applicable
// pre: none
// post: prints the information for the top clusters to the area, if applicable, and result files
void CAnalysis::DisplayTopClusters(double nMinRatio, const long lReportHistoryRunNumber, FILE* fp) {
  double                               dSignifRatio05;
  std::auto_ptr<stsAreaSpecificData>   pData;
  clock_t                              lStartTime;
  measure_t                            nMinMeasure = -1;

  try {
    m_nClustersReported = 0;
    if (m_pParameters->GetOutputAreaSpecificFiles())
      pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(), lReportHistoryRunNumber, giSimulationNumber > 98));
    dSignifRatio05 = SimRatios.GetAlpha05();
    //If  no replications, attempt to display up to top 10 clusters.
    tract_t tNumClustersToDisplay(giSimulationNumber == 0 ? std::min(10, m_nClustersRetained) : m_nClustersRetained);
    lStartTime = clock(); //get clock for calculating output time
    for (tract_t i=0; i < tNumClustersToDisplay; ++i) {
       if (i==1)
         ReportTimeEstimate(lStartTime, tNumClustersToDisplay, i, gpPrintDirection);
       if (m_pTopClusters[i]->m_nRatio > nMinRatio && (giSimulationNumber == 0 || m_pTopClusters[i]->m_nRank  <= giSimulationNumber)) {
         ++m_nClustersReported;
         switch (m_nClustersReported) {
           case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
           case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
           default : fprintf(fp, "\n"); break;
         }
         m_pTopClusters[i]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure, giSimulationNumber);
         if (m_pTopClusters[i]->m_nRatio > dSignifRatio05)
           ++guwSignificantAt005;
         // if doing area specific output, set the report pointer - not the most effective way to do this, but the least intrusive - AJV
         if (pData.get()) {
           m_pTopClusters[i]->SetAreaReport(pData.get());
           m_pTopClusters[i]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, giSimulationNumber,
                                                    lReportHistoryRunNumber, true, giSimulationNumber > 98, 0, 0, ' ', NULL, false);
         }
       }   // end if top cluster > minratio
    }   // end for loop
    fprintf(fp, "\n");

    // no need to worry about appending here because this function is not called in sequential analysis - AJV 12/30/2002
    if (m_pParameters->GetOutputAreaSpecificAscii()) // print area ASCII
      ASCIIFileWriter(pData.get()).Print();
    if (m_pParameters->GetOutputAreaSpecificDBase()) // print area dBase
      DBaseFileWriter(pData.get()).Print();
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayTopClusters(double, int, File*)", "CAnalysis");
    throw;
  }
}

/** Either diplays top cluster's loglikelihood ratio or test statistic. */
void CAnalysis::DisplayTopClusterLogLikelihood() {
  try {
    //if any clusters were retained, display either loglikelihood or test statistic
    if (m_nClustersRetained > 0) {
      if (m_pParameters->GetLogLikelihoodRatioIsTestStatistic())
        gpPrintDirection->SatScanPrintf("SaTScan test statistic for the most likely cluster: %7.2f\n",
                                        m_pTopClusters[0]->m_nRatio);
      else
        gpPrintDirection->SatScanPrintf("SaTScan log likelihood ratio for the most likely cluster: %7.2f\n",
                                        m_pTopClusters[0]->m_nRatio);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayTopClusterLogLikelihood()", "CAnalysis");
    throw;
  }
}

// can't find any reference to this function ever being called, appears to be an old debugging function
// that was forgotten about
void CAnalysis::DisplayTopClustersLogLikelihoods(FILE* fp) {
  try {
     for (tract_t i = 0; i<m_nClustersRetained; ++i) {
       fprintf(fp,"  %s for the most likely cluster: %7.21f\n\n",
            (m_pParameters->GetLogLikelihoodRatioIsTestStatistic() ? "Test statistic" : "Log likelihood ratio" ),
            m_pTopClusters[i]->m_nRatio);
     }
     fprintf(fp, "\n");
  }
  catch (ZdException &x) {
     x.AddCallpath("DisplayTopClusterLogLikelihoods()", "CAnalysis");
     throw;
  }
}

// displays summary type data for the end of the results file
// pre: none
// post: prints the appropraite summary data to the results file
bool CAnalysis::FinalizeReport(time_t RunTime) {
   FILE* fp;
   time_t CompletionTime;
   double nTotalTime,  nSeconds,  nMinutes,  nHours;
   char* szHours = "hours";
   char* szMinutes = "minutes";
   char* szSeconds = "seconds";

   try {
      gpPrintDirection->SatScanPrintf("\nFinishing up results...");
      OpenReportFile(fp, "a");

      if (m_nClustersRetained == 0) {
        fprintf(fp, "\nNo clusters were found.\n");
        if (m_pParameters->GetAreaScanRateType() == HIGH)
          fprintf(fp, "All areas scanned had equal or fewer cases than expected.\n");
        else if (m_pParameters->GetAreaScanRateType() == LOW)
          fprintf(fp, "All areas scanned had equal or greater cases than expected.\n");
        else
          fprintf(fp, "All areas scanned had cases equal to expected.\n");
      }
      else if (m_nClustersReported == 0) {
        fprintf(fp, "\nNo clusters reported.\n");
        fprintf(fp, "All clusters had a ratio less than %f or\n"
                    "a rank greater than %i.\n", m_nMinRatioToReport, m_pParameters->GetNumReplicationsRequested());
      }
      else if (m_pParameters->GetNumReplicationsRequested() == 0) {
        fprintf(fp, "\nNote: As the number of Monte Carlo replications was set to\n");
        fprintf(fp, "zero, no hypothesis testing was done and no p-values were\n");
        fprintf(fp, "printed.\n");
      }
      else if (m_pParameters->GetNumReplicationsRequested() <= 98) {
        fprintf(fp, "\nNote: The number of Monte Carlo replications was set too low,\n");
        fprintf(fp, "and a meaningful hypothesis test cannot be done.  Consequently,\n");
        fprintf(fp, "no p-values were printed.\n");
      }

      if (m_pParameters->GetProbabiltyModelType() == POISSON)
         m_pData->GetDataStreamHandler().ReportZeroPops(*m_pData, fp, gpPrintDirection);

      m_pData->GetTInfo()->tiReportDuplicateTracts(fp);

      m_pParameters->DisplayParameters(fp, giSimulationNumber);

      time(&CompletionTime);
      nTotalTime = difftime(CompletionTime, RunTime);
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
   catch (ZdException & x) {
      fclose(fp);
      x.AddCallpath("FinalizeReport(time_t)", "CAnalysis");
      throw;
   }
  return true;
}

/** Given data gate way, calculates and collects most likely clusters about
    each grid point. Collection of clusters are sorted by loglikelihood ratio
    and condensed based upon overlapping geographical areas.                */
bool CAnalysis::FindTopClusters(const AbtractDataStreamGateway & DataGateway) {
  int                   i;
  clock_t               tStartTime;

  try {
    //start clock which will help determine how long this process will take
    tStartTime = clock();
    //calculate top cluster about each centroid(grid point) and store copy in top cluster array
    for (i=0; i < m_pData->m_nGridTracts && !gpPrintDirection->GetIsCanceled(); ++i) {
       m_pTopClusters[i] = CalculateTopCluster(i, DataGateway).Clone();
       m_pTopClusters[i]->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
       ++m_nClustersRetained;
       if (i==9)
         ReportTimeEstimate(tStartTime, m_pData->m_nGridTracts, i+1, gpPrintDirection);
    }
    if (gpPrintDirection->GetIsCanceled())
      return false;
    //now sort top cluster array by ratio and possibly remove overlapping clusters 
    RankTopClusters();
  }
  catch (ZdException &x) {
    x.AddCallpath("FindTopClusters()","CAnalysis");
    throw;
  }
  return true;
}

/** calculates greatest loglikelihood ratio about each centroid(grid point) */
double CAnalysis::FindTopRatio(const AbtractDataStreamGateway & DataGateway) {
  int                   i;
  double                dMaxLogLikelihoodRatio=0;

  //calculate greatest loglikelihood ratio about each centroid
  for (i=0; i < m_pData->m_nGridTracts && !gpPrintDirection->GetIsCanceled(); ++i)
    dMaxLogLikelihoodRatio = std::max(CalculateTopCluster(i, DataGateway).m_nRatio, dMaxLogLikelihoodRatio);
  return dMaxLogLikelihoodRatio;
}

CMeasureList * CAnalysis::GetNewMeasureListObject() const {
  switch (m_pParameters->GetAreaScanRateType()) {
    case HIGH       : return new CMinMeasureList(*m_pData, *gpLikelihoodCalculator);
    case LOW        : return new CMaxMeasureList(*m_pData, *gpLikelihoodCalculator);
    case HIGHANDLOW : return new CMinMaxMeasureList(*m_pData, *gpLikelihoodCalculator);
    default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                          m_pParameters->GetAreaScanRateType());
  }
  return 0;
}

CTimeIntervals * CAnalysis::GetNewTimeIntervalsObject(IncludeClustersType eType) const {
  if (m_pParameters->GetProbabiltyModelType() == NORMAL)
    return new NormalTimeIntervalRange(*m_pData, *gpLikelihoodCalculator, eType);
  else if (m_pParameters->GetNumDataStreams() > 1)
    return new MultiStreamTimeIntervalRange(*m_pData, *gpLikelihoodCalculator, eType);
  else
    return new TimeIntervalRange(*m_pData, *gpLikelihoodCalculator, eType);
}

/** internal initialization */
void CAnalysis::Init() {
  gpClusterDataFactory=0;
  m_nClustersRetained=0;
  m_nAnalysisCount=0;
  m_nClustersReported=0;
  m_nMinRatioToReport=0.001;
  m_nPower_X_Count=0;
  m_nPower_Y_Count=0;
  giSimulationNumber=0;
  guwSignificantAt005=0;
  gbMeasureListReplications=true;
  gpLikelihoodCalculator=0;
}

void CAnalysis::InitializeTopClusterList() {
  try {
     for (int i=0; i<m_nClustersRetained; ++i) {
        delete m_pTopClusters[i];
        m_pTopClusters[i] = 0;
     }
   }
   catch (ZdException & x) {
      x.AddCallpath("InitializeTopClusterList()", "CAnalysis");
      throw;
   }
}

// attempts to open the results file and return a handle to it
// pre: szType is either "a" or "w"
// post: will return a file pointer handle if successful in opening file, else will generate
//       exception if correct szType used
void CAnalysis::OpenReportFile(FILE*& fp, const char* szType) {
  try {
    if ((fp = fopen(m_pParameters->GetOutputFileName().c_str(), szType)) == NULL) {
      if (!strcmp(szType, "w"))
        ZdException::Generate("Error: Results file '%s' could not be created.\n",
                              "OpenReportFile()", m_pParameters->GetOutputFileName().c_str());
      else if (!strcmp(szType, "a"))
        ZdException::Generate("Error: Results file '%s' could not be opened.\n",
                              "OpenReportFile()", m_pParameters->GetOutputFileName().c_str());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("OpenReportFile()", "CAnalysis");
    throw;
  }
}

// performs Monte Carlo Simulations and prints out the results for each one
void CAnalysis::PerformSimulations() {
   double                               r;
   int                                  iSimulationNumber;
   char                               * sReplicationFormatString;
   std::auto_ptr<LogLikelihoodData>     pLLRData;
   AbtractDataStreamGateway           * pDataGateway=0;
                                                  
   try {
      if (m_pParameters->GetNumReplicationsRequested() > 0) {
        //recompute neighbors if settings indicate that smaller clusters are reported
        if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
          m_pData->FindNeighbors(true);

        gpPrintDirection->SatScanPrintf("Doing the Monte Carlo replications\n");

        // assign replication format string here to prevent another check in loop
        if (m_pParameters->GetLogLikelihoodRatioIsTestStatistic())
          sReplicationFormatString = "SaTScan test statistic for #%ld of %ld replications: %7.2f\n";
        else
          sReplicationFormatString = "SaTScan log likelihood ratio for #%ld of %ld replications: %7.2f\n";

        // record only the first set of log likelihoods - AJV 12/27/2002
        if (m_pParameters->GetOutputSimLoglikeliRatiosFiles() && m_nAnalysisCount == 1)
           pLLRData.reset(new LogLikelihoodData(gpPrintDirection, *m_pParameters));

        clock_t nStartTime = clock();
        SimRatios.Initialize();
        pDataGateway = m_pData->GetDataStreamHandler().GetNewSimulationDataGateway();
        AllocateSimulationObjects(*pDataGateway);
        {//block for the scope of SimulationPrintDirection
          PrintQueue SimulationPrintDirection(*gpPrintDirection);

          for (iSimulationNumber=1; (iSimulationNumber <= m_pParameters->GetNumReplicationsRequested()) && !gpPrintDirection->GetIsCanceled(); iSimulationNumber++) {
            giSimulationNumber = iSimulationNumber;
            m_pData->RandomizeData(giSimulationNumber);
            r = (gbMeasureListReplications ? MonteCarlo(pDataGateway->GetDataStreamInterface(0)) : FindTopRatio(*pDataGateway));
            UpdateTopClustersRank(r);
            SimRatios.AddRatio(r);
            UpdatePowerCounts(r);
 #ifdef DEBUGANALYSIS
            fprintf(m_pDebugFile, "---- Replication #%ld ----------------------\n\n",i+1);
            m_pData->DisplaySimCases(m_pDebugFile);
            // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
            fprintf(m_pDebugFile, "%s = %7.21f\n\n",
                    (Parameters.GetLogLikelihoodRatioIsTestStatistic() ? "Test statistic" : "Log Likelihood Ratio"), r);
 #endif
            if (giSimulationNumber==1) {
              ReportTimeEstimate(nStartTime, m_pParameters->GetNumReplicationsRequested(), giSimulationNumber, &SimulationPrintDirection);

                 ZdTimestamp tsReleaseTime;
                 tsReleaseTime.Now();
                 tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
                 SimulationPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
            }
            SimulationPrintDirection.SatScanPrintf(sReplicationFormatString, giSimulationNumber, m_pParameters->GetNumReplicationsRequested(), r);

            if (m_pParameters->GetTerminateSimulationsEarly() && CheckForEarlyTermination(giSimulationNumber))
              break;
            // best to just check if the data pointer is set instead of checking the criteria for setting again so will
            // only have to check that criteria in one place instead of two -- AJV 12/30/2002
            if(pLLRData.get())
               pLLRData->AddLikelihood(r);
          }
        }
        delete pDataGateway; pDataGateway=0;
        // only write to output formats on the first analysis/iteration -- AJV
        if (m_pParameters->GetOutputSimLoglikeliRatiosAscii() && pLLRData.get())
           ASCIIFileWriter(pLLRData.get()).Print();
        if (m_pParameters->GetOutputSimLoglikeliRatiosDBase() && pLLRData.get())
           DBaseFileWriter(pLLRData.get()).Print();
      }
   }
   catch (ZdException &x) {
     delete pDataGateway;
     x.AddCallpath("PerformSimulations()", "CAnalysis");
     throw;
   }
}

void CAnalysis::PrintTopClusters(int nHowMany) {
   FILE* pFile;

   try {
      if ((pFile = fopen("c:\\SatScan V.2.1.4\\Borland Calc\\topclusters.txt", "w")) == NULL)
        SSGenerateException("  Error: Unable to open top clusters file.\n", "PrintTopClusters()");
      else {
         for (int i = 0; i < nHowMany; ++i) {
           fprintf(pFile, "GridTract:  %i\n", i);
           fprintf(pFile, "  Ellipe Offset:  %i\n", m_pTopClusters[i]->m_iEllipseOffset);
           fprintf(pFile, "         Center:  %i\n", m_pTopClusters[i]->m_Center );
           fprintf(pFile, "        Measure:  %f\n", m_pTopClusters[i]->GetMeasure(0));      //measure_t
           fprintf(pFile, "         Tracts:  %i\n", m_pTopClusters[i]->m_nTracts);
           fprintf(pFile, "LikelihoodRatio:  %f\n", m_pTopClusters[i]->m_nRatio );
           fprintf(pFile, "           Rank:  %i\n", m_pTopClusters[i]->m_nRank );
           fprintf(pFile, " \n");
           fprintf(pFile, " \n");
         }
      }
      fclose(pFile);
   }
   catch (ZdException & x) {
      fclose(pFile);
      x.AddCallpath("PrintTopClusters()", "CAnalysis");
      throw;
   }
}

/**********************************************************************
 Ranks the top "NumKept" clusters, in descending cd_ratio, such that
 no ranked cluster overlaps a higher-ranked cluster.
 Ratio for TopClusters[NumKept .. NumClusters-1] is set to -DBL_MAX.
 Note: It is easy to modify the selection of clusters to be included
 by making changes in the "if(...) INCLUDE=0" line. For example:
 if(clusterdistance <= radius[j]) -> all clusters with centers not included
                                     in a higher cluster on the list.
 if(clusterdistance <= newradius) -> all clusters not containing the center
                                     of a higher cluster on the list.
 if(clusterdistance <= radius[j] &&
    clusterdistance <= newradius) -> all clusters such that their is no
                                     cluster pair both of which contain
                                     the center point of the other.
 if(clusterdistance <= radius[j] &&
    clusterdistance <= newradius) -> all clusters such that no center point
                                     is contained in any other cluster with
                                     lower or higher rank.
 if(clusterdistance <= radius[j]+
                       newradius) -> all non-overlapping clusters
 Parameters:
   TopClusters[]  - clusters to rank
   NumClusters    - length of TopClusters
   NumKept        - number to rank (<= NumClusters)
//
 **********************************************************************/
void CAnalysis::RankTopClusters() {
   tract_t t, j;
   float newradius;                      /* Radius of new cluster tested */
   float clusterdistance;                /* Distance between the centers */
                                         /* of old and new clusters      */
   float *pRadius = 0;
   double** pCoords = 0;
   double* pCoords1 = 0, *pCoords2 = 0;
   int     i, iNumElements;
   bool bInclude;                 /* 0/1 variable put to zero when a new */
                                  /* cluster should not be incuded.      */

   try {
      //if no restrictions then need array to have m_nNumTracts number of elements
      //else just set it to NUM_RANKED (500) elements.....
      iNumElements = NUM_RANKED;
      if (m_pParameters->GetCriteriaSecondClustersType() == NORESTRICTIONS)
         iNumElements = m_pData->GetNumTracts();
      pRadius = new float[iNumElements];
      pCoords = (double**)Smalloc(iNumElements * sizeof(double*),gpPrintDirection);
      memset(pCoords, 0, iNumElements * sizeof(double*));

      /* Note: "Old clusters" are clusters already included on the list, while */
      /* a "new cluster" is the present candidate for inclusion.               */

      /* Sort by descending m_ratio, without regard to overlap */
      qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClustersByRatio);

      // Remove "Undefined" clusters that have been sorted to bottome of list because ratio=-DBL_MAX
      tract_t nClustersAssigned = m_nClustersRetained;
      for (tract_t k=0; k<nClustersAssigned; k++) {
        if (!m_pTopClusters[k]->ClusterDefined()) {
          m_nClustersRetained--;
          delete m_pTopClusters[k];
          m_pTopClusters[k] = NULL;
        }
      }

      if (m_nClustersRetained != 0) {
        /* Remove certain types of overlapping clusters from later printout */
        (m_pData->GetGInfo())->giGetCoords(m_pTopClusters[0]->m_Center, &pCoords[0]);
        (m_pData->GetTInfo())->tiGetCoords(m_pData->GetNeighbor(m_pTopClusters[0]->m_iEllipseOffset, m_pTopClusters[0]->m_Center,m_pTopClusters[0]->GetNumTractsInnerCircle()/*m_nTracts*/),
                    &pCoords2);

        pRadius[0] = (float)sqrt((m_pData->GetTInfo())->tiGetDistanceSq(pCoords[0],pCoords2));
        free(pCoords2);
        t=1;

        while (t<m_nClustersToKeepEachPass && m_pTopClusters[t] != NULL) {
          (m_pData->GetGInfo())->giGetCoords(m_pTopClusters[t]->m_Center, &pCoords1);
          (m_pData->GetTInfo())->tiGetCoords(m_pData->GetNeighbor(m_pTopClusters[t]->m_iEllipseOffset, m_pTopClusters[t]->m_Center,m_pTopClusters[t]->GetNumTractsInnerCircle()),
                      &pCoords2);
          newradius = (float)sqrt((m_pData->GetTInfo())->tiGetDistanceSq(pCoords1,pCoords2));
          free(pCoords2);
          bInclude=1;

          j=0;
          while (bInclude && j<t)  {
            clusterdistance = (float)sqrt((m_pData->GetTInfo())->tiGetDistanceSq(pCoords1,pCoords[j]));
            //IF ELLIPSOID RUN, THEN SET TO "NO RESTRICTIONS"
            if (m_pParameters->GetNumRequestedEllipses() > 0)
                bInclude = 1;
            else {
               switch (m_pParameters->GetCriteriaSecondClustersType()) {
                  case NOGEOOVERLAP: //no geographical overlap
                     if (clusterdistance <= (pRadius[j]+newradius))
                       bInclude=0;
                     break;
                  case NOCENTROIDSINOTHER: //no cluster centroids in other clusters
                     if((clusterdistance <= pRadius[j]) || (clusterdistance <= newradius))
                        bInclude = 0;
                     break;
                  case NOCENTROIDSINMORELIKE: //no cluster centroids in more likely clusters
                     if (clusterdistance <= pRadius[j])
                        bInclude = 0;
                     break;
                  case NOCENTROIDSINLESSLIKE: //no cluster centroids in less likely clusters
                     if(clusterdistance <= newradius)
                        bInclude = 0;
                     break;
                  case NOPAIRSINEACHOTHERS: //no pairs of centroids in each others clusters
                     if((clusterdistance <= pRadius[j]) && (clusterdistance <= newradius))
                        bInclude = 0;
                     break;
                  case NORESTRICTIONS:   //No Restrictions
                     bInclude = 1;
                     break;
                  default:  SSGenerateException("Invalid value found for Criteria for Reporting Secondary Clusters.","RankTopClusters");
               }
            }
            ++j;
          } // while bInclude

          if (bInclude) {
            pRadius[t] = newradius;
            // Now allocate new pCoords[]
            pCoords[t] = (double*)Smalloc(m_pParameters->GetDimensionsOfData() * sizeof(double),gpPrintDirection);

            // Loop through values of pCoords1
            for (i=0; i<m_pParameters->GetDimensionsOfData(); i++) {
            	pCoords[t][i] = pCoords1[i];
            }
            // Don't need pCoords1 anymore
            free(pCoords1);
            ++t;
          } // if bInclude
          else {
            m_nClustersRetained--;
            delete m_pTopClusters[t];
            for (j = t; j < m_nClustersRetained; //m_nMaxClusters - 1
                 j++)
              m_pTopClusters[j] = m_pTopClusters[j + 1];
            m_pTopClusters[m_nClustersRetained] = NULL;
            free(pCoords1);
          } // if-else
        }  // while t

        m_nClustersRetained = t;

        // Delete unused clusters
        for (t = m_nClustersToKeepEachPass; t < nClustersAssigned; t++) {
          delete m_pTopClusters[t];
          m_pTopClusters[t] = 0;
        }
      }

      //Clean up pCoords allocation
      for (i=0; i<iNumElements; i++) {
      	if (pCoords[i] != NULL)
           free(pCoords[i]);
      }
      free(pCoords);
      delete [] pRadius;
   }
   catch (ZdException & x) {
      //Clean up pCoords allocation
      if (pCoords) {
         for (int i=0; i<iNumElements; i++) {
            if (pCoords[i] != NULL)
              free(pCoords[i]);
         }
         free(pCoords);
      }
      delete [] pRadius;
      x.AddCallpath("RankTopClusters()", "CAnalysis");
      throw;
   }
} /* RankTopClusters() */

void CAnalysis::RemoveTopClusterData() {
   try {
      for (int i=1; i <= m_pTopClusters[0]->m_nTracts; i++)  {
        m_pData->RemoveTractSignificance(m_pData->GetNeighbor(0, m_pTopClusters[0]->m_Center, i));
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

      InitializeTopClusterList();
      m_nClustersRetained = 0;

      m_pData->SetMaxCircleSize();
      m_pData->AdjustNeighborCounts();
   }
   catch (ZdException & x) {
      x.AddCallpath("RemoveTopClusterData()", "CAnalysis");
      throw;
   }
}

bool CAnalysis::RepeatAnalysis() {
   bool bTopCluster, bHasMoreSequentialScans,
        bNotEarlyTerminated, bHasTractsAfterTopCluster, bReturn=false;

   try {
      if (m_pParameters->GetIsSequentialScanning()) {
        bTopCluster = m_pTopClusters[0] &&
                      m_pTopClusters[0]->GetPVal(giSimulationNumber) < m_pParameters->GetSequentialCutOffPValue();
        bHasTractsAfterTopCluster = bTopCluster && m_pData->GetNumTracts() - m_pTopClusters[0]->m_nTracts > 0;
        bHasMoreSequentialScans = m_nAnalysisCount < m_pParameters->GetNumSequentialScansRequested();
        bNotEarlyTerminated = giSimulationNumber == m_pParameters->GetNumReplicationsRequested();

        bReturn = bTopCluster && bHasMoreSequentialScans && bNotEarlyTerminated && bHasTractsAfterTopCluster;
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("RepeatAnalysis()", "CAnalysis");
      throw;
   }
   return bReturn;
}

/** internal setup function */
void CAnalysis::Setup() {
  try {
    if (m_pParameters->GetIsSequentialScanning())
      m_nClustersToKeepEachPass = 1;
    else {
      if (m_pParameters->GetCriteriaSecondClustersType() == NORESTRICTIONS)
        m_nClustersToKeepEachPass = m_pData->GetNumTracts();
      else
        m_nClustersToKeepEachPass = (m_pData->m_nGridTracts <= NUM_RANKED ? m_pData->m_nGridTracts : NUM_RANKED);
    }
    //create cluster data factory
    if (m_pParameters->GetProbabiltyModelType() == NORMAL) {
      gpClusterDataFactory = new NormalClusterDataFactory();
      gbMeasureListReplications = false;
    }
    else if (m_pParameters->GetNumDataStreams() > 1) {
      gpClusterDataFactory = new MultipleStreamsClusterDataFactory(*m_pParameters);
      gbMeasureListReplications = false;
    }
    else {
      gpClusterDataFactory = new ClusterDataFactory();
      gbMeasureListReplications = true;
    }
      
#ifdef DEBUGPROSPECTIVETIME
    if ((m_pDebugFile = fopen("DebugSaTScan.TXT", "w")) == NULL) {
      fprintf(stderr, "  Error: Unable to create debug file.\n");
      SSGenerateException("  Error: Unable to create debug file.\n", "CAnalysis constructor");
    }
#endif
  }
  catch (ZdException &x) {
    delete gpClusterDataFactory;
    x.AddCallpath("CAnalysis()","CAnalysis");
    throw;
  }
}

void CAnalysis::SortTopClusters() {
  qsort(m_pTopClusters, m_nClustersRetained, sizeof(CCluster*), CompareClustersByRatio);
}

void CAnalysis::UpdatePowerCounts(double r)
{
  if (m_pParameters->GetIsPowerCalculated()) {
    if (r > m_pParameters->GetPowerCalculationX())
      ++m_nPower_X_Count;
    if (r > m_pParameters->GetPowerCalculationY())
      ++m_nPower_Y_Count;
  }
}

bool CAnalysis::UpdateReport(const long lReportHistoryRunNumber) {
   FILE* fp;

   try {
      gpPrintDirection->SatScanPrintf("\nRecording results to file...");
      OpenReportFile(fp, "a");

      if (m_pParameters->GetIsSequentialScanning())
        DisplayTopCluster(m_nMinRatioToReport, lReportHistoryRunNumber, fp);
      else
        DisplayTopClusters(m_nMinRatioToReport, lReportHistoryRunNumber, fp);

      if (giSimulationNumber >= 19 && m_nClustersReported > 0) {
        // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
        fprintf(fp, "The %s value required for an observed\n",
               (m_pParameters->GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"));
        fprintf(fp, "cluster to be significant at level\n");
        if (giSimulationNumber >= 99)
          fprintf(fp,"... 0.01: %f\n", SimRatios.GetAlpha01());
        if (giSimulationNumber >= 19)
          fprintf(fp,"... 0.05: %f\n", SimRatios.GetAlpha05());
        fprintf(fp, "\n");
      }

      if (m_pParameters->GetIsPowerCalculated()) {
        fprintf(fp,"Percentage of Monte Carlo replications with a likelihood greater than\n");
        fprintf(fp,"... X (%f) : %f\n", m_pParameters->GetPowerCalculationX(),
                ((double)m_nPower_X_Count)/giSimulationNumber);
        fprintf(fp,"... Y (%f) : %f\n\n", m_pParameters->GetPowerCalculationY(),
                ((double)m_nPower_Y_Count)/giSimulationNumber);
      }

      if (m_nClustersReported > 0 && giSimulationNumber < m_pParameters->GetNumReplicationsRequested()) {
        fprintf(fp, "\nNOTE: The optional sequential procedure was used to terminate the\n");
        fprintf(fp, "      simulations early for large p-values. This means that the\n");
        fprintf(fp, "      reported p-values are slightly conservative.\n");
      }
      fclose(fp);
   }
   catch (ZdException & x) {
      fclose(fp);
      x.AddCallpath("UpdateReport()", "CAnalysis");
      throw;
   }
  return true;
}

/** Updates rank of top clusters by comparing simulated loglikelihood ratio(LLR)
    with each remaining clusters LLR. If the analysis contains ellipses and is
    duczmal correcting their LLRs, then the duczmal corrected LLR is is used
    to rank the clusters instead of LLR. */
void CAnalysis::UpdateTopClustersRank(double r) {
  int   i;

  //if analysis contains ellipses and they are being duczmal corrected,
  //then we want to compare parameter 'r' with the duczmal corrected ratio
  if (m_pParameters->GetNumRequestedEllipses() && m_pParameters->GetDuczmalCorrectEllipses())
    for (i=m_nClustersRetained-1; i >= 0; i--) {
       if (m_pTopClusters[i]->m_nRatio > r)
         break;
       m_pTopClusters[i]->m_nRank++;
    }
  else
    for (i=m_nClustersRetained-1; i >= 0; i--) {
       if (m_pTopClusters[i]->m_nRatio > r)
         break;
       m_pTopClusters[i]->m_nRank++;
    }
}

