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

CAnalysis::CAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
          :SimRatios(pParameters->m_nReplicas, pPrintDirection) {
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

      if (m_pParameters->m_bSequential)
        m_nClustersToKeepEachPass = 1;
      else {
         if (m_pParameters->m_iCriteriaSecondClusters == NORESTRICTIONS)
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
      //Allocate array which will store most likely clusters.
      AllocateTopClusterList();
      //Allocate two-dimensional array to be used in replications.
      m_pData->AllocSimCases();
      //Calculate expected number of cases.
      if (!m_pData->CalculateMeasure())
         return false;
    
      if (m_pData->m_nTotalCases < 1)
         SSGenerateException("  Error: No cases found in input data.\n", "CAnalysis");    //KR V.2.1

      //For circle and each ellipse, find closest neighboring tracts points for
      //each grid point limiting distance by max circle size and cumulated measure.
      //
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

      do {
        ++m_nAnalysisCount;
#ifdef INCLUDE_RUN_HISTORY
         // declare variable for each analysis - AJV 9/10/2002
        stsRunHistoryFile historyFile(m_pParameters->GetRunHistoryFilename(), *gpPrintDirection, m_pParameters->m_nReplicas > 99);
#endif

#ifdef DEBUGANALYSIS
        fprintf(m_pDebugFile, "Analysis Loop #%i\n", m_nAnalysisCount);
        m_pData->DisplaySummary2(m_pDebugFile);
        fprintf(m_pDebugFile, "\n---- Actual Data ----------------------\n\n");
        m_pData->DisplayCases(m_pDebugFile);
        if (m_pParameters->m_nModel==BERNOULLI)
          m_pData->DisplayControls(m_pDebugFile);
        m_pData->DisplayMeasure(m_pDebugFile);
        m_pData->DisplayNeighbors(m_pDebugFile);
#endif
    
        if (! gpPrintDirection->GetIsCanceled()) {
           //For each grid point, find the cluster with the greatest loglikihood.
           //Removes any clusters that violate criteria for reporting secondary clusters.
           if (! FindTopClusters())
             return false;
        }
        else return false;
    
        DisplayTopClusterLogLikelihood();

//#ifdef DEBUGANALYSIS
//        DisplayTopClustersLogLikelihoods(m_pDebugFile);
//#endif

        //Do Monte Carlo replications.
        if (m_nClustersRetained > 0)
          PerformSimulations();
        else
          gpPrintDirection->SatScanPrintf("No clusters retained.\n"); // USE return valuse from FindTopClusters to indicate this.

//#ifdef DEBUGANALYSIS
//        DisplayTopClusters(-DBL_MAX, INT_MAX, m_pDebugFile, NULL);
//#endif
#ifdef INCLUDE_RUN_HISTORY
        lRunNumber = historyFile.GetRunNumber();
#endif
        if (gpPrintDirection->GetIsCanceled() || !UpdateReport(lRunNumber))
          return false;

#ifdef INCLUDE_RUN_HISTORY
        // log new history for each analysis run - AJV 9/10/2002
        gpPrintDirection->SatScanPrintf("\nLogging run history...");
        
        double dPVal((m_nClustersRetained > 0) ? m_pTopClusters[0]->GetPVal(m_pParameters->m_nReplicas): 0.0);
        historyFile.LogNewHistory(*this, guwSignificantAt005, dPVal);
        // reset the number of significants back to zero for each analysis - AJV 9/10/2002
        guwSignificantAt005 = 0;
#endif

        bContinue = RepeatAnalysis();
        if (bContinue)
          RemoveTopClusterData();
    
        if (gpPrintDirection->GetIsCanceled())
           return false;
           
      } while (bContinue);

      m_pData->DeAllocSimCases();
      FinalizeReport(RunTime, lRunNumber);
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
/*static*/ int CAnalysis::CompareClusters(const void *a, const void *b) {
  double rdif = (*(CCluster**)b)->m_nRatio - (*(CCluster**)a)->m_nRatio;
  if (rdif < 0.0)   return -1;
  if (rdif > 0.0)   return 1;
  return 0;
} /* CompareClusters() */

//******************************************************************************
//  Create the Grid Output File (Most Likely Cluster for each Centroid)
//******************************************************************************
void CAnalysis::CreateGridOutputFile(const long& lReportHistoryRunNumber) {
   std::auto_ptr<stsClusterData>        pData;

   try {
      if (m_pParameters->m_bMostLikelyClusters || m_pParameters->GetOutputClusterLevelDBF()) {
         pData.reset( new stsClusterData(gpPrintDirection, m_pParameters->m_szOutputFilename, lReportHistoryRunNumber, GetCoordinateType(),
                                    m_pParameters->m_nModel, m_pParameters->m_nDimension, m_pParameters->m_nReplicas > 99,
                                    m_pParameters->m_nNumEllipses > 0) );

         for (int i = 0; i < m_nClustersRetained; ++i) {
            // print out user notification every 20 recorded clusters to let user know program is still working - AJV 10/3/2002
            if((i%20) == 0)
               gpPrintDirection->SatScanPrintf("%i out of %i records recorded to cluster file so far...", i, m_nClustersRetained);
            pData->RecordClusterData(*m_pTopClusters[i], *m_pData, i+1);
         }

         if (m_pParameters->m_bMostLikelyClusters) {
            ASCIIFileWriter writer(pData.get());
            writer.Print();
         }
         if (m_pParameters->GetOutputClusterLevelDBF()) {
            DBaseFileWriter writer(pData.get());
            writer.Print();
         }
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

      ASCIIFileWriter writer(pTest.get());
      writer.Print();

      DBaseFileWriter Dwriter(pTest.get());
      Dwriter.Print();
*/
   }
   catch (ZdException & x) {
      x.AddCallpath("CreateGridOutputFile()", "CAnalysis");
      throw;
   }
}

bool CAnalysis::CreateReport(time_t RunTime) {
   FILE* fp;

   try {
      OpenReportFile(fp, "w");

      fprintf(fp,  "                 _____________________________\n\n");
      DisplayVersion(fp, 1);
      fprintf(fp,"                 _____________________________ \n\n");

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

void CAnalysis::DisplayFindClusterHeading() {
   try {
      if (!m_pParameters->m_bSequential)
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

void CAnalysis::DisplayTopCluster(double nMinRatio, int nReps, const long& lReportHistoryRunNumber, FILE* fp) {
   std::auto_ptr<stsAreaSpecificData>   pData;

   try {
      if(m_pParameters->GetOutputAreaSpecificDBF() || m_pParameters->m_bOutputCensusAreas)
         pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->m_szOutputFilename, lReportHistoryRunNumber, m_pParameters->m_nReplicas > 99));

      measure_t nMinMeasure = 0;

      if (m_nClustersRetained == 0)
        return;

      if (m_pTopClusters[0]->m_nRatio > nMinRatio && (nReps == 0 || m_pTopClusters[0]->m_nRank  <= nReps)) {
        ++m_nClustersReported;

        switch(m_nAnalysisCount) {
          case 1 : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
          case 2 : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
          default: fprintf(fp,"                  _____________________________\n\n");
        }

        m_pTopClusters[0]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure);

        if(m_pTopClusters[0]->m_nLogLikelihood < SimRatios.GetAlpha05())
           ++guwSignificantAt005;
        
        // if we want dBase report, set the report pointer in cluster
        if(m_pParameters->GetOutputAreaSpecificDBF())
           m_pTopClusters[0]->SetAreaReport(pData.get());
        
        // if we are doing dBase or ASCII   
        if(m_pParameters->GetOutputAreaSpecificDBF() || m_pParameters->m_bOutputCensusAreas) {
            m_pTopClusters[0]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, m_pParameters->m_nReplicas,
                                                     lReportHistoryRunNumber, true, m_pParameters->m_nReplicas>99, 0, 0, ' ', NULL, false);                                      
        }
      }

      fprintf(fp, "\n");

      // print area ASCII
      if (m_pParameters->m_bOutputCensusAreas) {
         ASCIIFileWriter AWriter(pData.get());
         AWriter.Print();
      }
      // print area dBase
      if (m_pParameters->GetOutputAreaSpecificDBF()) {
         DBaseFileWriter DWriter(pData.get());
         DWriter.Print();
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayTopCluster(double, int, File *)", "CAnalysis");
      throw;
   }
}

void CAnalysis::DisplayTopClusters(double nMinRatio, int nReps, const long& lReportHistoryRunNumber, FILE* fp) {
   double                       dSignifRatio05;
   tract_t                      tNumClustersToDisplay;
   std::auto_ptr<stsAreaSpecificData>   pData;

   try {
      m_nClustersReported = 0;
      measure_t nMinMeasure = -1;
      
      if(m_pParameters->GetOutputAreaSpecificDBF() || m_pParameters->m_bOutputCensusAreas)
         pData.reset(new stsAreaSpecificData(gpPrintDirection, m_pParameters->m_szOutputFilename, lReportHistoryRunNumber, m_pParameters->m_nReplicas > 99));

      dSignifRatio05 = SimRatios.GetAlpha05();

      //if  no replications, attempt to display up to top 10 clusters
      tNumClustersToDisplay = (nReps == 0 ? min(10, m_nClustersRetained) : m_nClustersRetained);

      for (tract_t i=0; i < tNumClustersToDisplay; ++i) {
        // print out user notification for every 15th cluster recorded so user knows program is still working - AJV 10/3/2002
        if((i%15)== 0)
             gpPrintDirection->SatScanPrintf("%i out of %i clusters recorded so far...", i, tNumClustersToDisplay);

        if (m_pTopClusters[i]->m_nRatio > nMinRatio && (nReps == 0 || m_pTopClusters[i]->m_nRank  <= nReps)) {
          ++m_nClustersReported;

          switch(m_nClustersReported) {
            case 1 : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
            case 2 : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
            default: fprintf(fp, "\n"); break;
          }   // end switch

          
          m_pTopClusters[i]->Display(fp, *m_pParameters, *m_pData, m_nClustersReported, nMinMeasure);

          if(m_pTopClusters[i]->m_nLogLikelihood < dSignifRatio05)
             ++guwSignificantAt005;
          
          // if doing dBase output, set the report pointer - not the most effective way to do this, but the least intrusive - AJV
          if(m_pParameters->GetOutputAreaSpecificDBF() || m_pParameters->m_bOutputCensusAreas)
             m_pTopClusters[i]->SetAreaReport(pData.get());

          // if were are doing census areas and theres a file pointer OR we are doing dBase output
          if (m_pParameters->m_bOutputCensusAreas || m_pParameters->GetOutputAreaSpecificDBF()) {
              m_pTopClusters[i]->DisplayCensusTracts(0, *m_pData, m_nClustersReported, nMinMeasure, m_pParameters->m_nReplicas,
                                                   lReportHistoryRunNumber, true, m_pParameters->m_nReplicas>99, 0, 0, ' ', NULL, false);
          }
        }   // end if top cluster > minratio
      }   // end for loop

      fprintf(fp, "\n");

      // print area ASCII
      if (m_pParameters->m_bOutputCensusAreas) {
         ASCIIFileWriter AWriter(pData.get());
         AWriter.Print();
      }
      // print AREA dBase
      if (m_pParameters->GetOutputAreaSpecificDBF()) {
         DBaseFileWriter DWriter(pData.get());
         DWriter.Print();
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayTopClusters(double, int, File*)", "CAnalysis");
      throw;
   }
}

void CAnalysis::DisplayTopClusterLogLikelihood() {
   try {
      if (m_nClustersRetained > 0) {
         gpPrintDirection->SatScanPrintf("  %s for the most likely cluster: %7.2f\n\n",
                                         (m_pParameters->m_nModel == SPACETIMEPERMUTATION ? "Test statistic" : "Log likelihood ratio" ),
                                         m_pTopClusters[0]->m_nRatio);
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayTopClusterLogLikelihood()", "CAnalysis");
      throw;
   }
}

void CAnalysis::DisplayTopClustersLogLikelihoods(FILE* fp) {
  try {
     for (tract_t i = 0; i<m_nClustersRetained; ++i) {
       fprintf(fp,"  %s for the most likely cluster: %7.21f\n\n",
            (m_pParameters->m_nModel == SPACETIMEPERMUTATION ? "Test statistic" : "Log likelihood ratio" ),
            m_pTopClusters[i]->m_nRatio);
     }
     fprintf(fp, "\n");
  }
  catch (ZdException &x) {
     x.AddCallpath("DisplayTopClusterLogLikelihoods()", "CAnalysis");
     throw;
  }
}

bool CAnalysis::FinalizeReport(time_t RunTime, const long& lReportHistoryRunNumber) {
   FILE* fp;
   time_t CompletionTime;
   double nTotalTime,  nSeconds,  nMinutes,  nHours;
   char* szHours = "hours";
   char* szMinutes = "minutes";
   char* szSeconds = "seconds";

   try {
      gpPrintDirection->SatScanPrintf("\nFinishing up reports...");
      OpenReportFile(fp, "a");
      
      CreateGridOutputFile(lReportHistoryRunNumber);

      if (m_nClustersRetained == 0) {
        fprintf(fp, "\nNo clusters were found.\n");
        if (m_pParameters->m_nAreas == HIGH)
          fprintf(fp, "All areas scanned had equal or fewer cases than expected.\n");
        else if (m_pParameters->m_nAreas == LOW)
          fprintf(fp, "All areas scanned had equal or greater cases than expected.\n");
        else
          fprintf(fp, "All areas scanned had cases equal to expected.\n");
      }
      else if (m_nClustersReported == 0) {
        fprintf(fp, "\nNo clusters reported.\n");
        fprintf(fp, "All clusters had a ratio less than %f or\n"
                    "a rank greater than %i.\n", m_nMinRatioToReport, m_pParameters->m_nReplicas);
      }
      else if (m_pParameters->m_nReplicas==0) {
        fprintf(fp, "\nNote: As the number of Monte Carlo replications was set to\n");
        fprintf(fp, "zero, no hypothesis testing was done and no p-values were\n");
        fprintf(fp, "printed.\n");
      }
      else if (m_pParameters->m_nReplicas<=99) {
        fprintf(fp, "\nNote: The number of Monte Carlo replications was set too low,\n");
        fprintf(fp, "and a meaningful hypothesis test cannot be done.  Consequently,\n");
        fprintf(fp, "no p-values were printed.\n");
      }

      if (m_pParameters->m_nModel == POISSON ||
          (m_pParameters->m_nModel == SPACETIMEPERMUTATION &&
           m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE))
        (m_pData->GetTInfo())->tiReportZeroPops(fp);

      m_pData->GetTInfo()->tiReportDuplicateTracts(fp);

      if (m_pParameters->m_bOutputRelRisks || m_pParameters->GetDBaseOutputRelRisks())
        m_pData->DisplayRelativeRisksForEachTract(m_pParameters->m_bOutputRelRisks, m_pParameters->GetDBaseOutputRelRisks());

      //if (m_pParameters->m_bOutputCensusAreas)
      //   {
      //   fprintf(fp, "________________________________________________________________\n\n");
      //   fprintf(fp,"For further study using a GIS or database program, an ASCII\n"
      //           "format GIS file has been created, describing the detected clusters.\n"
      //           "The name of this file is %s.\n", m_pParameters->m_szGISFilename);
      //   }        

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

void CAnalysis::OpenReportFile(FILE*& fp, const char* szType) {
   try {
      if ((fp = fopen(m_pParameters->m_szOutputFilename, szType)) == NULL) {
        if (!strcmp(szType, "w"))
          SSGenerateException("  Error: Unable to create report file.", "OpenReportFile");
        else if (!strcmp(szType, "a"))
          SSGenerateException("  Error: Unable to open report file.", "OpenReportFile");
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("OpenReportFile(File *, const char *)", "CAnalysis");
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
      gpPrintDirection->SatScanPrintf("Doing the Monte Carlo replications\n");

      // assign replication format string here to prevent another check in loop
      if (m_pParameters->m_nModel == SPACETIMEPERMUTATION)
        sReplicationFormatString = "Test statistic for #%ld of %ld Replications: %7.2f\n";
      else
        sReplicationFormatString = "Log Likelihood Ratio for #%ld of %ld Replications: %7.2f\n";

      if (m_pParameters->m_bSaveSimLogLikelihoods || m_pParameters->GetDBaseOutputLogLikeli())
         pLLRData.reset( new LogLikelihoodData(gpPrintDirection, m_pParameters->m_szOutputFilename) );

      clock_t nStartTime = clock();
      SimRatios.Initialize();

      for (iSimulationNumber=1; (iSimulationNumber <= m_pParameters->m_nReplicas) && !gpPrintDirection->GetIsCanceled(); iSimulationNumber++) {
        m_pData->MakeData(iSimulationNumber);
        if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
           r = MonteCarloProspective();
        else
           r = MonteCarlo();

        UpdateTopClustersRank(r);
        SimRatios.AddRatio(r);
    
        UpdatePowerCounts(r);

    //    if (!(i % 200)) KR-980326 Limit printing to increase speed of program
        gpPrintDirection->SatScanPrintf(sReplicationFormatString, iSimulationNumber, m_pParameters->m_nReplicas, r);

        if (m_pParameters->m_bSaveSimLogLikelihoods)
           pLLRData->AddLikelihood(r);

        #ifdef DEBUGANALYSIS
        fprintf(m_pDebugFile, "---- Replication #%ld ----------------------\n\n",i+1);
        m_pData->DisplaySimCases(m_pDebugFile);
        // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
        fprintf(m_pDebugFile, "%s = %7.21f\n\n",
                (Parameters.m_nModel == SPACETIMEPERMUTATION ? "Test statistic" : "Log Likelihood Ratio"), r);
        #endif

        if (iSimulationNumber==1)
          ReportTimeEstimate(nStartTime, m_pParameters->m_nReplicas, iSimulationNumber, gpPrintDirection);
      }

      if (m_pParameters->m_bSaveSimLogLikelihoods) {
         ASCIIFileWriter Awriter(pLLRData.get());
         Awriter.Print();
      }
      if (m_pParameters->GetDBaseOutputLogLikeli()) {
         DBaseFileWriter Dwriter(pLLRData.get());
         Dwriter.Print();
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
   //float radius[NUM_RANKED];             /* Array of old cluster radiia  */
   float *pRadius = 0;
   //float* x = new float[NUM_RANKED];
   //float* y = new float[NUM_RANKED];
   //float* z = new float[NUM_RANKED];     /* Coordinates for old clusters */
   //float x1, y1, z1;                     /* Coordinates for new cluster  */
   //float x2, y2, z2;                     /* Coordinates for edge tract   */
                                         	/* in the new cluster           */
   double** pCoords = 0;
   double* pCoords1 = 0;
   double* pCoords2 = 0;
   int     iNumElements;
   bool bInclude;                 /* 0/1 variable put to zero when a new */
                                  /* cluster should not be incuded.      */
   int i;

   try {
      //if no restrictions then need array to have m_nNumTracts number of elements
      //else just set it to NUM_RANKED (500) elements.....
      iNumElements = NUM_RANKED;
      if (m_pParameters->m_iCriteriaSecondClusters == NORESTRICTIONS)
         iNumElements = m_pData->m_nTracts;
      pRadius = new float[iNumElements];
      pCoords = (double**)Smalloc(iNumElements * sizeof(double*),gpPrintDirection);
      memset(pCoords, NULL, iNumElements * sizeof(double*));

      /* Note: "Old clusters" are clusters already included on the list, while */
      /* a "new cluster" is the present candidate for inclusion.               */

      /* Sort by descending m_ratio, without regard to overlap */
      //  SortTopClusters();
      //  if (m_nClustersToKeepEachPass != 1)
        qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClusters);

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
            if (m_pParameters->m_nNumEllipses > 0)
                bInclude = 1;
            else {
               switch (m_pParameters->m_iCriteriaSecondClusters) {
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
            pCoords[t] = (double*)Smalloc(m_pParameters->m_nDimension * sizeof(double),gpPrintDirection);

            // Loop through values of pCoords1
            for (i=0; i<m_pParameters->m_nDimension; i++) {
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

void CAnalysis::RemoveTopClusterData()
{
   tract_t nNeighbor;

   try {
      for (int i=1; i <= m_pTopClusters[0]->m_nTracts; i++)  {
        nNeighbor = m_pData->GetNeighbor(0, m_pTopClusters[0]->m_Center, i);

        m_pData->m_nTotalCases    -= m_pData->m_pCases[0][nNeighbor];
        m_pData->m_nTotalMeasure  -= m_pData->m_pMeasure[0][nNeighbor];

        m_pData->m_pCases[0][nNeighbor]   = 0;
        m_pData->m_pMeasure[0][nNeighbor] = 0;

        if (m_pParameters->m_nModel==BERNOULLI) {
          m_pData->m_nTotalControls -= m_pData->m_pControls[0][nNeighbor];
          m_pData->m_pControls[0][nNeighbor]   = 0;
        }

        --m_pData->m_nTracts;
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
      if (m_pParameters->m_bSequential)
         bReturn = ((m_nAnalysisCount < m_pParameters->m_nAnalysisTimes) && (m_pTopClusters[0]->GetPVal(m_pParameters->m_nReplicas)
                    < m_pParameters->m_nCutOffPVal) && (m_pData->m_nTracts > 1));
   }
   catch (ZdException & x) {
      x.AddCallpath("RepeatAnalysis()", "CAnalysis");
      throw;
   }
   return bReturn;
}

void CAnalysis::SortTopClusters()
{
  qsort(m_pTopClusters, m_nClustersRetained, sizeof(CCluster*), CompareClusters);
}

void CAnalysis::UpdatePowerCounts(double r)
{
  if (m_pParameters->m_bPowerCalc) {
    if (r > m_pParameters->m_nPower_X)
      ++m_nPower_X_Count;
    if (r > m_pParameters->m_nPower_Y)
      ++m_nPower_Y_Count;
  }
}

bool CAnalysis::UpdateReport(const long& lReportHistoryRunNumber) {
   FILE* fp;

   try {
      gpPrintDirection->SatScanPrintf("\nRecording results to file...");
      OpenReportFile(fp, "a");

      if (m_pParameters->m_bSequential)
        DisplayTopCluster(m_nMinRatioToReport, m_pParameters->m_nReplicas, lReportHistoryRunNumber, fp);
      else
        DisplayTopClusters(m_nMinRatioToReport, m_pParameters->m_nReplicas, lReportHistoryRunNumber, fp);

      if (m_pParameters->m_nReplicas>=19 && m_nClustersReported > 0) {
        // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
        fprintf(fp, "The %s value required for an observed\n",
               (m_pParameters->m_nModel == SPACETIMEPERMUTATION ? "test statistic" : "log likelihood ratio"));
        fprintf(fp, "cluster to be significant at level\n");
        if (m_pParameters->m_nReplicas>=99)
          fprintf(fp,"... 0.01: %f\n", SimRatios.GetAlpha01());
        if (m_pParameters->m_nReplicas>=19)
          fprintf(fp,"... 0.05: %f\n", SimRatios.GetAlpha05());
        fprintf(fp, "\n");
      }

      if (m_pParameters->m_bPowerCalc) {
        fprintf(fp,"Percentage of Monte Carlo replications with a likelihood greater than\n");
        fprintf(fp,"... X (%f) : %f\n", m_pParameters->m_nPower_X,
                ((double)m_nPower_X_Count)/m_pParameters->m_nReplicas);
        fprintf(fp,"... Y (%f) : %f\n\n", m_pParameters->m_nPower_Y,
                ((double)m_nPower_Y_Count)/m_pParameters->m_nReplicas);
      }

      if ( !strlen(m_pParameters->m_szGridFilename) && m_pParameters->m_nAnalysisType ==PROSPECTIVESPACETIME) {
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

void CAnalysis::UpdateTopClustersRank(double r)
{
  for (int i=m_nClustersRetained-1; i>=0; i--) {
     if (m_pTopClusters[i]->m_nRatio > r)
       break;
     m_pTopClusters[i]->m_nRank++;
  }
}

