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

  if (iShapeOffset && gData.m_pParameters->GetNumRequestedEllipses() && gData.m_pParameters->GetDuczmalCorrectEllipses())
    for (iEllipse=0; iEllipse < gData.m_pParameters->GetNumRequestedEllipses() && !pTopCluster; ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gData.m_pParameters->GetEllipseRotations()[iEllipse];
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

  //first set ratios
  for (size_t t=0; t < gvTopShapeClusters.size(); t++)
    gvTopShapeClusters[t]->SetRatioAndDates(gData);
  //set the maximum cluster to the circle shape initially
  pTopCluster = gvTopShapeClusters[0];
  //if the there are ellipses, compare current top cluster against them
  //note: we don't have to be concerned with whether we are comparing circles and ellipses,
  //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
  for (size_t t=1; t < gvTopShapeClusters.size(); t++)
     if (gvTopShapeClusters[t]->ClusterDefined() && /* could be that no cluster was defined as max for iteration*/
         gvTopShapeClusters[t]->GetDuczmalCorrectedLogLikelihoodRatio() > pTopCluster->GetDuczmalCorrectedLogLikelihoodRatio())
       pTopCluster = gvTopShapeClusters[t];

  return *pTopCluster;
}

/** Initialzies the vector of top clusters to cloned copies of cluster,
    taking into account whether spatial shape will be a factor in analysis. */
void TopClustersContainer::SetTopClusters(const CCluster& InitialCluster) {
  int   i, iNumTopClusters;

  try {
    gvTopShapeClusters.DeleteAllElements();
    //if there are ellipses and duczmal correction is true, then we need
    //a top cluster for the circle and each ellipse shape
    if (gData.m_pParameters->GetNumRequestedEllipses() && gData.m_pParameters->GetDuczmalCorrectEllipses())
      iNumTopClusters = gData.m_pParameters->GetNumRequestedEllipses() + 1;
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

CAnalysis::CAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
          :SimRatios(pParameters->GetNumReplicationsRequested(), pPrintDirection) {
   try {
      m_pParameters = pParameters;
      m_pData       = pData;
      gpPrintDirection = pPrintDirection;
    
      m_nClustersRetained  = 0;
      m_nAnalysisCount     = 0;
      m_nClustersReported  = 0;
      m_nMinRatioToReport  = 0.001;
      m_nPower_X_Count     = 0;
      m_nPower_Y_Count     = 0;

      guwSignificantAt005       = 0;

      if (m_pParameters->GetIsSequentialScanning())
        m_nClustersToKeepEachPass = 1;
      else {
         if (m_pParameters->GetCriteriaSecondClustersType() == NORESTRICTIONS)
            m_nClustersToKeepEachPass = m_pData->m_nTracts;
         else
              m_nClustersToKeepEachPass = (m_pData->m_nGridTracts <= NUM_RANKED ? m_pData->m_nGridTracts : NUM_RANKED);
      }
    
//#ifdef DEBUGANALYSIS
#ifdef DEBUGPROSPECTIVETIME
      if ((m_pDebugFile = fopen("DebugSaTScan.TXT", "w")) == NULL) {
         fprintf(stderr, "  Error: Unable to create debug file.\n");
         SSGenerateException("  Error: Unable to create debug file.\n", "CAnalysis constructor");
      }
#endif
   }
   catch (ZdException & x) {
      x.AddCallpath("CAnalysis(CParameters *, CSaTScanData *)", "CAnalysis");
      throw;
   }
}

CAnalysis::~CAnalysis() {
   InitializeTopClusterList();
   delete [] m_pTopClusters;

  //  delete m_pModel;

//#ifdef DEBUGANALYSIS
#ifdef DEBUGPROSPECTIVETIME
   fclose(m_pDebugFile);
#endif
}

bool CAnalysis::Execute(time_t RunTime) {
   bool bContinue;
   long lRunNumber = 0;

   try {
      SetMaxNumClusters();
      AllocateTopClusterList();                //Allocate array which will store most likely clusters.
      m_pData->AllocSimCases();                //Allocate two-dimensional array to be used in replications.

      if (!m_pData->CalculateMeasure())        //Calculate expected number of cases.
         return false;
    
      if (m_pData->m_nTotalCases < 1)
         ZdException::Generate("Error: No cases found in input data.\n","CAnalysis");    //KR V.2.1

      //For circle and each ellipse, find closest neighboring tracts points for
      //each grid point limiting distance by max circle size and cumulated measure.
      if (gpPrintDirection->GetIsCanceled() || !m_pData->FindNeighbors())
        return false;

      if (gpPrintDirection->GetIsCanceled() || !CreateReport(RunTime))
        return false;

      m_pData->m_pModel->ReCalculateMeasure();
#ifdef DEBUGANALYSIS
      DisplayVersion(m_pDebugFile, 0);
      fprintf(m_pDebugFile, "Program run on: %s", ctime(&RunTime));
      m_pParameters->DisplayParameters(m_pDebugFile);
#endif

#ifdef INCLUDE_RUN_HISTORY
     // only create one history object and record to it only once ( first iteration if sequential analysis ) -- AJV
     stsRunHistoryFile historyFile(m_pParameters->GetRunHistoryFilename(), *gpPrintDirection,
                                   m_pParameters->GetNumReplicationsRequested() > 99, m_pParameters->GetIsSequentialScanning());
     lRunNumber = historyFile.GetRunNumber();
#endif

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
        if (gpPrintDirection->GetIsCanceled() || !FindTopClusters())
          return false;

        DisplayTopClusterLogLikelihood();

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
           gpPrintDirection->SatScanPrintf("\nLogging run history...");
           double dPVal((m_nClustersRetained > 0) ? m_pTopClusters[0]->GetPVal(m_pParameters->GetNumReplicationsRequested()) : 0.0);
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

      m_pData->DeAllocSimCases();
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
      memset(m_pTopClusters, 0, m_nMaxClusters+1);
   }
   catch (ZdException & x) {
      x.AddCallpath("AllocateTopClusterList()", "CAnalysis");
      throw;
   }
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

/**********************************************************************
 Comparison function for sorting clusters by descending duczmal corrected ratio.
 **********************************************************************/
/*static*/ int CAnalysis::CompareClustersByDuzcmalCorrectedRatio(const void *a, const void *b) {
  double rdif = (*(CCluster**)b)->GetDuczmalCorrectedLogLikelihoodRatio() - (*(CCluster**)a)->GetDuczmalCorrectedLogLikelihoodRatio() ;
  if (rdif < 0.0)   return -1;
  if (rdif > 0.0)   return 1;
  return 0;
} /* CompareClusters() */

//******************************************************************************
//  Create the Most Likely Cluster Output File
//******************************************************************************
void CAnalysis::CreateGridOutputFile(const long lReportHistoryRunNumber) {
   try {
      if (m_pParameters->GetOutputClusterLevelFiles()) {
         std::auto_ptr<stsClusterData> pData( new stsClusterData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(),
                                                                 lReportHistoryRunNumber, m_pParameters->GetCoordinatesType(), m_pParameters->GetProbabiltyModelType(),
                                                                 m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumReplicationsRequested() > 99,
                                                                 m_pParameters->GetNumRequestedEllipses() > 0, m_pParameters->GetDuczmalCorrectEllipses()));

         for (int i = 0; i < m_nClustersRetained; ++i) {
            // print out user notification every 20 recorded clusters to let user know program is still working - AJV 10/3/2002
            if((i%20) == 0)
               gpPrintDirection->SatScanPrintf("%i out of %i records recorded to cluster file so far...", i, m_nClustersRetained);
            pData->RecordClusterData(*m_pTopClusters[i], *m_pData, i+1);
         }

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
void CAnalysis::DisplayTopCluster(double nMinRatio, int nReps, const long lReportHistoryRunNumber, FILE* fp) {
  measure_t                             nMinMeasure = 0;
  std::auto_ptr<stsAreaSpecificData>    pData;

  try {
    if (m_nClustersRetained > 0) {
      if (m_pParameters->GetOutputAreaSpecificFiles())
        pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(), lReportHistoryRunNumber, m_pParameters->GetNumReplicationsRequested() > 99));
      if (m_pTopClusters[0]->m_nRatio > nMinRatio && (nReps == 0 || m_pTopClusters[0]->m_nRank  <= nReps)) {
        ++m_nClustersReported;
        switch(m_nAnalysisCount) {
          case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
          case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
          default : fprintf(fp,"                  _____________________________\n\n");
        }
        m_pTopClusters[0]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure);
        if (m_pTopClusters[0]->m_nRatio > SimRatios.GetAlpha05())
          ++guwSignificantAt005;
        // if we want area specific report, set the report pointer in cluster
        if (pData.get()) {
          m_pTopClusters[0]->SetAreaReport(pData.get());
          m_pTopClusters[0]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, m_pParameters->GetNumReplicationsRequested(),
                                                  lReportHistoryRunNumber, true, m_pParameters->GetNumReplicationsRequested() > 99, 0, 0, ' ', NULL, false);
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
void CAnalysis::DisplayTopClusters(double nMinRatio, int nReps, const long lReportHistoryRunNumber, FILE* fp) {
  double                               dSignifRatio05;
  std::auto_ptr<stsAreaSpecificData>   pData;
  clock_t                              lStartTime;
  measure_t                            nMinMeasure = -1;

  try {
    m_nClustersReported = 0;
    if (m_pParameters->GetOutputAreaSpecificFiles())
      pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->GetOutputFileName().c_str(), lReportHistoryRunNumber, m_pParameters->GetNumReplicationsRequested() > 99));
    dSignifRatio05 = SimRatios.GetAlpha05();
    //If  no replications, attempt to display up to top 10 clusters.
    tract_t tNumClustersToDisplay(nReps == 0 ? min(10, m_nClustersRetained) : m_nClustersRetained);
    lStartTime = clock(); //get clock for calculating output time
    for (tract_t i=0; i < tNumClustersToDisplay; ++i) {
       if (i==1)
         ReportTimeEstimate(lStartTime, tNumClustersToDisplay, i, gpPrintDirection);
       if (m_pTopClusters[i]->m_nRatio > nMinRatio && (nReps == 0 || m_pTopClusters[i]->m_nRank  <= nReps)) {
         ++m_nClustersReported;
         switch (m_nClustersReported) {
           case 1  : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
           case 2  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
           default : fprintf(fp, "\n"); break;
         }
         m_pTopClusters[i]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure);
         if (m_pTopClusters[i]->m_nRatio > dSignifRatio05)
           ++guwSignificantAt005;
         // if doing area specific output, set the report pointer - not the most effective way to do this, but the least intrusive - AJV
         if (pData.get()) {
           m_pTopClusters[i]->SetAreaReport(pData.get());
           m_pTopClusters[i]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, m_pParameters->GetNumReplicationsRequested(),
                                                    lReportHistoryRunNumber, true, m_pParameters->GetNumReplicationsRequested() > 99, 0, 0, ' ', NULL, false);
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
        gpPrintDirection->SatScanPrintf("  SaTScan test statistic for the most likely cluster: %7.2f\n\n",
                                        m_pTopClusters[0]->m_nRatio);
      else
        gpPrintDirection->SatScanPrintf("  SaTScan log likelihood ratio for the most likely cluster: %7.2f\n\n",
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
      else if (m_pParameters->GetNumReplicationsRequested() <= 99) {
        fprintf(fp, "\nNote: The number of Monte Carlo replications was set too low,\n");
        fprintf(fp, "and a meaningful hypothesis test cannot be done.  Consequently,\n");
        fprintf(fp, "no p-values were printed.\n");
      }

      if (m_pParameters->GetProbabiltyModelType() == POISSON ||
          (m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION &&
           m_pParameters->GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE))
        (m_pData->GetTInfo())->tiReportZeroPops(fp);

      m_pData->GetTInfo()->tiReportDuplicateTracts(fp);

      m_pParameters->DisplayParameters(fp);

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
        fprintf(fp,"Total Running Time : %.0f %s %.0f %s", nMinutes, szMinutes,
                   nSeconds, szSeconds);
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

bool CAnalysis::FindTopClusters() {
   bool         bReturn=true;
   
   try {
      DisplayFindClusterHeading();

      clock_t nStartTime = clock();

      for (int i = 0; (i<m_pData->m_nGridTracts) && !gpPrintDirection->GetIsCanceled(); i++) {
        m_pTopClusters[i] = GetTopCluster(i);
        ++m_nClustersRetained;
        if (i==9)
          ReportTimeEstimate(nStartTime, m_pData->m_nGridTracts, i+1, gpPrintDirection);
      }

      if (gpPrintDirection->GetIsCanceled())
         bReturn = false;
      else
         RankTopClusters();          // DTG  -- MODIFY THIS FUNCTION !!!!!!!!
   }
   catch (ZdException & x) {
      x.AddCallpath("FindTopClusters()", "CAnalysis");
      throw;
   }
  return bReturn;
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
   double               r;
   int                  iSimulationNumber;
   char               * sReplicationFormatString;
   std::auto_ptr<LogLikelihoodData>     pLLRData;

   try {
      if (m_pParameters->GetNumReplicationsRequested() > 0) {
        gpPrintDirection->SatScanPrintf("Doing the Monte Carlo replications\n");

        // assign replication format string here to prevent another check in loop
        if (m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION ||
            (m_pParameters->GetNumRequestedEllipses() && m_pParameters->GetDuczmalCorrectEllipses()))
          sReplicationFormatString = "SaTScan test statistic for #%ld of %ld replications: %7.2f\n";
        else
          sReplicationFormatString = "SaTScan log likelihood ratio for #%ld of %ld replications: %7.2f\n";

        // record only the first set of log likelihoods - AJV 12/27/2002  
        if (m_pParameters->GetOutputSimLoglikeliRatiosFiles() && m_nAnalysisCount == 1)
           pLLRData.reset(new LogLikelihoodData(gpPrintDirection, *m_pParameters));

        clock_t nStartTime = clock();
        SimRatios.Initialize();

        for (iSimulationNumber=1; (iSimulationNumber <= m_pParameters->GetNumReplicationsRequested()) && !gpPrintDirection->GetIsCanceled(); iSimulationNumber++) {
          m_pData->MakeData(iSimulationNumber);
          r = (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME) ? MonteCarloProspective() : MonteCarlo();
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
          if (iSimulationNumber==1) {
            ReportTimeEstimate(nStartTime, m_pParameters->GetNumReplicationsRequested(), iSimulationNumber, gpPrintDirection);
            //Simulations taking less than one second to complete hinder user seeing most likely clusters
            //loglikelihood ratio, so pause program.
            if ((clock() - nStartTime)/CLK_TCK < 1)
              Sleep(5000);
          }
          gpPrintDirection->SatScanPrintf(sReplicationFormatString, iSimulationNumber, m_pParameters->GetNumReplicationsRequested(), r);

          // best to just check if the data pointer is set instead of checking the criteria for setting again so will
          // only have to check that criteria in one place instead of two -- AJV 12/30/2002
          if(pLLRData.get())
             pLLRData->AddLikelihood(r);
        }

        // only write to output formats on the first analysis/iteration -- AJV
        if (m_pParameters->GetOutputSimLoglikeliRatiosAscii() && pLLRData.get())
           ASCIIFileWriter(pLLRData.get()).Print();
        if (m_pParameters->GetOutputSimLoglikeliRatiosDBase() && pLLRData.get())
           DBaseFileWriter(pLLRData.get()).Print();
      }
   }
   catch (ZdException & x) {
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
           fprintf(pFile, "        Measure:  %f\n", m_pTopClusters[i]->m_nMeasure);      //measure_t
           fprintf(pFile, "         Tracts:  %i\n", m_pTopClusters[i]->m_nTracts);
           fprintf(pFile, "     Likelihood:  %f\n", m_pTopClusters[i]->m_nLogLikelihood );
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
         iNumElements = m_pData->m_nTracts;
      pRadius = new float[iNumElements];
      pCoords = (double**)Smalloc(iNumElements * sizeof(double*),gpPrintDirection);
      memset(pCoords, NULL, iNumElements * sizeof(double*));

      /* Note: "Old clusters" are clusters already included on the list, while */
      /* a "new cluster" is the present candidate for inclusion.               */

      /* Sort by descending m_ratio, without regard to overlap */
      //  SortTopClusters();
      //  if (m_nClustersToKeepEachPass != 1)
      if (!m_pParameters->GetDuczmalCorrectEllipses())
        qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClustersByRatio);
      else  
        qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClustersByDuzcmalCorrectedRatio);

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
   tract_t nNeighbor;

   try {
      for (int i=1; i <= m_pTopClusters[0]->m_nTracts; i++)  {
        nNeighbor = m_pData->GetNeighbor(0, m_pTopClusters[0]->m_Center, i);

        m_pData->m_nTotalCases    -= m_pData->m_pCases[0][nNeighbor];
        m_pData->m_nTotalMeasure  -= m_pData->m_pMeasure[0][nNeighbor];

        m_pData->m_pCases[0][nNeighbor]   = 0;
        m_pData->m_pMeasure[0][nNeighbor] = 0;

        if (m_pParameters->GetProbabiltyModelType() == BERNOULLI) {
          m_pData->m_nTotalControls -= m_pData->m_pControls[0][nNeighbor];
          m_pData->m_pControls[0][nNeighbor]   = 0;
        }

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

bool CAnalysis::RepeatAnalysis()
{
   bool bReturn = false;

   try {
      if (m_pParameters->GetIsSequentialScanning())
         bReturn = ( m_pTopClusters[0] &&
                    (m_nAnalysisCount < m_pParameters->GetNumSequentialScansRequested()) &&
                    (m_pTopClusters[0]->GetPVal(m_pParameters->GetNumReplicationsRequested()) < m_pParameters->GetSequentialCutOffPValue()) &&
                    (m_pData->m_nTracts > 1));
   }
   catch (ZdException & x) {
      x.AddCallpath("RepeatAnalysis()", "CAnalysis");
      throw;
   }
   return bReturn;
}

void CAnalysis::SortTopClusters()
{
  if (!m_pParameters->GetDuczmalCorrectEllipses())
    qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClustersByDuzcmalCorrectedRatio);
  else
    qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClustersByDuzcmalCorrectedRatio);
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
        DisplayTopCluster(m_nMinRatioToReport, m_pParameters->GetNumReplicationsRequested(), lReportHistoryRunNumber, fp);
      else
        DisplayTopClusters(m_nMinRatioToReport, m_pParameters->GetNumReplicationsRequested(), lReportHistoryRunNumber, fp);

      if (m_pParameters->GetNumReplicationsRequested() >= 19 && m_nClustersReported > 0) {
        // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
        fprintf(fp, "The %s value required for an observed\n",
               (m_pParameters->GetLogLikelihoodRatioIsTestStatistic() ? "test statistic" : "log likelihood ratio"));
        fprintf(fp, "cluster to be significant at level\n");
        if (m_pParameters->GetNumReplicationsRequested() >= 99)
          fprintf(fp,"... 0.01: %f\n", SimRatios.GetAlpha01());
        if (m_pParameters->GetNumReplicationsRequested() >= 19)
          fprintf(fp,"... 0.05: %f\n", SimRatios.GetAlpha05());
        fprintf(fp, "\n");
      }

      if (m_pParameters->GetIsPowerCalculated()) {
        fprintf(fp,"Percentage of Monte Carlo replications with a likelihood greater than\n");
        fprintf(fp,"... X (%f) : %f\n", m_pParameters->GetPowerCalculationX(),
                ((double)m_nPower_X_Count)/m_pParameters->GetNumReplicationsRequested());
        fprintf(fp,"... Y (%f) : %f\n\n", m_pParameters->GetPowerCalculationY(),
                ((double)m_nPower_Y_Count)/m_pParameters->GetNumReplicationsRequested());
      }

      if (!m_pParameters->UseSpecialGrid() && m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME) {
        fprintf(fp, "\nIMPORTANT:\nFor the prospective analysis to be correct, it is important\n");
        fprintf(fp, "that the scanning spatial window is the same for each analysis that is\n");
        fprintf(fp, "performed once a day, week, year, etc. This means that the grid points\n");
        fprintf(fp, "defining the circle centroids must remain the same. If the location IDs in\n");
        fprintf(fp, "the coordinates file remain the same in each time-periodic analysis, then\n");
        fprintf(fp, "there is no problem. On the other hand, if new IDs are added to the\n");
        fprintf(fp, "coordinates file over time, then you must use a special grid file and\n");
        fprintf(fp, "retain this file through all the analyses.\n");
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
       if (m_pTopClusters[i]->GetDuczmalCorrectedLogLikelihoodRatio() > r)
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

