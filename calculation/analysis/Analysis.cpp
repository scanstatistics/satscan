// analysis.cpp

#include <float.h>
#include "analysis.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "TimeEstimate.h"
//#include "MakeData.h"
#include "display.h"
#include "ginfo.h"
#include "tinfo.h"
#include "error.h"

//static int CompClust(const void *a, const void *b);

CAnalysis::CAnalysis(CParameters* pParameters, CSaTScanData* pData)
          :SimRatios(pParameters->m_nReplicas)
{
  m_pParameters = pParameters;
  m_pData       = pData;

//  switch (m_pParameters->m_nModel)
//  {
//    case POISSON   : m_pModel = new CPoissonModel(pParameters, pData);   break;
//    case BERNOULLI : m_pModel = new CBernoulliModel(pParameters, pData); break;
//  }

  m_nClustersRetained  = 0;
  m_nAnalysisCount     = 0;
  m_nClustersReported  = 0;
  m_nMinRatioToReport  = 0.001;
  m_nPower_X_Count     = 0;
  m_nPower_Y_Count     = 0;

  if (m_pParameters->m_bSequential)
    m_nClustersToKeepEachPass = 1;
  else
    m_nClustersToKeepEachPass = (m_pData->m_nGridTracts <= NUM_RANKED ?
                                 m_pData->m_nGridTracts : NUM_RANKED);

  #if DEBUGANALYSIS
  if ((m_pDebugFile = fopen("DebugSaTScan.TXT", "w")) == NULL)
  {
    fprintf(stderr, "  Error: Unable to create debug file.\n");
    FatalError(0);
  }
  #endif

}

CAnalysis::~CAnalysis()
{
  InitializeTopClusterList();
  delete [] m_pTopClusters;

//  delete m_pModel;

  #if DEBUGANALYSIS
  fclose(m_pDebugFile);
  #endif
}

bool CAnalysis::Execute(time_t RunTime)
{
  bool bContinue;

  SetMaxNumClusters();
  AllocateTopClusterList();
  m_pData->AllocSimCases();

  if (!m_pData->CalculateMeasure())
    return false;

  if (m_pData->m_nTotalCases < 1)
  {
//KR V.2.1
    printf("  Error: No cases found in input data.\n");
//KR V.2.1
    return false;
  }

  if (!m_pData->FindNeighbors())
    return false;

  if (!CreateReport(RunTime))
    return false;

  #if DEBUGANALYSIS
  DisplayVersion(m_pDebugFile, 0);
  fprintf(m_pDebugFile, "Program run on: %s", ctime(&RunTime));
  m_pParameters->DisplayParameters(m_pDebugFile);
  #endif

  do
  {
    m_nAnalysisCount++;

    #if DEBUGANALYSIS
    fprintf(m_pDebugFile, "Analysis Loop #%i\n", m_nAnalysisCount);
    m_pData->DisplaySummary2(m_pDebugFile);
    fprintf(m_pDebugFile, "\n---- Actual Data ----------------------\n\n");
    m_pData->DisplayCases(m_pDebugFile);
    if (m_pParameters->m_nModel==BERNOULLI)
      m_pData->DisplayControls(m_pDebugFile);
    m_pData->DisplayMeasure(m_pDebugFile);
    m_pData->DisplayNeighbors(m_pDebugFile);
    #endif

    FindTopClusters();

    DisplayTopClusterLogLikelihood(stdout);

    #if DEBUGANALYSIS
    DisplayTopClustersLogLikelihoods(m_pDebugFile);
    #endif

    if (m_nClustersRetained > 0)
      PerformSimulations();
    else
      printf("No clusters retained.\n"); // USE return valuse from FindTopClusters to indicate this.

    #if DEBUGANALYSIS
    DisplayTopClusters(-DBL_MAX, INT_MAX, m_pDebugFile, NULL);
    #endif

    if (!UpdateReport())
      return false;

    bContinue = RepeatAnalysis();
    if (bContinue)
      RemoveTopClusterData();

  } while (bContinue);

  m_pData->DeAllocSimCases();
  FinalizeReport(RunTime);

  return true;
}

void CAnalysis::DisplayFindClusterHeading()
{
  if (!m_pParameters->m_bSequential)
    printf("Finding the most likely clusters\n");
  else
  {
    switch(m_nAnalysisCount)
    {
      case  1: printf("Finding the most likely cluster\n"); break;
      case  2: printf("Finding the second most likely cluster\n"); break;
      case  3: printf("Finding the third most likely cluster\n"); break;
      default: printf("Finding the %ith most likely cluster\n", m_nAnalysisCount); break;
    }
  }
}

void CAnalysis::FindTopClusters()
{
  DisplayFindClusterHeading();

  clock_t nStartTime = clock();

  for (int i = 0; i<m_pData->m_nGridTracts; i++)
  {
    m_pTopClusters[i] = GetTopCluster(i);
    m_nClustersRetained++;
    if (i==9)
      ReportTimeEstimate(nStartTime, m_pData->m_nGridTracts, i+1);
  }

//  #if DEBUGANALYSIS
//  fprintf(m_pDebugFile, "\nTop Cluster Prior to Ranking\n\n");
//  DisplayTopClusters(-DBL_MAX, INT_MAX, m_pDebugFile, NULL);
//  #endif

  RankTopClusters();
}

void CAnalysis::DisplayTopClusterLogLikelihood(FILE* fp)
{
  if (m_nClustersRetained > 0)
    fprintf(fp,"  Log likelihood ratio for the most likely cluster: %7.2f\n\n",
               m_pTopClusters[0]->m_nRatio);
}

void CAnalysis::DisplayTopClustersLogLikelihoods(FILE* fp)
{
  for (tract_t i=0; i<m_nClustersRetained; i++)
  {
    fprintf(fp,"  Log likelihood ratio for the most likely cluster: %7.21f\n\n",
               m_pTopClusters[i]->m_nRatio);
  }
  fprintf(fp, "\n");
}

void CAnalysis::PerformSimulations()
{
  printf("Doing the Monte Carlo replications\n");
  double  r;
  FILE* fpLLR;

  if (m_pParameters->m_bSaveSimLogLikelihoods)
    OpenLLRFile(fpLLR, "w");

  clock_t nStartTime = clock();
  SimRatios.Initialize();

  for (int i = 0; i<m_pParameters->m_nReplicas; i++)
  {
    m_pData->MakeData();
    r = MonteCarlo();

    UpdateTopClustersRank(r);
    SimRatios.AddRatio(r);

    UpdatePowerCounts(r);

//    if (!(i % 200)) KR-980326 Limit printing to increase speed of program
      printf("Log Likelihood Ratio for #%ld of %ld Replications: %7.2f\n",
              i+1, m_pParameters->m_nReplicas, r);

    if (m_pParameters->m_bSaveSimLogLikelihoods)
      fprintf(fpLLR, "%7.2f\n", r);

    #if DEBUGANALYSIS
    fprintf(m_pDebugFile, "---- Replication #%ld ----------------------\n\n",i+1);
    m_pData->DisplaySimCases(m_pDebugFile);
    fprintf(m_pDebugFile, "Log Likelihood Ratio = %7.21f\n\n", r);
    #endif

    if (i==0)
      ReportTimeEstimate(nStartTime, m_pParameters->m_nReplicas, i+1);
  }

  if (m_pParameters->m_bSaveSimLogLikelihoods)
    fclose(fpLLR);

//  m_pData->DeAllocSimCases();
}

void CAnalysis::UpdatePowerCounts(double r)
{
  if (m_pParameters->m_bPowerCalc)
  {
    if (r > m_pParameters->m_nPower_X)
      m_nPower_X_Count++;
    if (r > m_pParameters->m_nPower_Y)
      m_nPower_Y_Count++;
  }
}

/*void CAnalysis::MakeData()
{
  ::MakeData(m_pData->m_pMeasure,  m_pData->m_nTotalCases,
             m_pData->m_nTotalTractsAtStart, m_pData->m_nTotalMeasure,
             m_pData->m_pSimCases, m_pData->m_nTimeIntervals);
}
*/
void CAnalysis::DisplayTopClusters(double nMinRatio, int nReps,
                                   FILE* fp, FILE* fpGIS)
{
  m_nClustersReported = 0;
  measure_t nMinMeasure = -1;

  for (tract_t i=0; i<m_nClustersRetained; i++)
  {
    if (m_pTopClusters[i]->m_nRatio > nMinRatio &&
        m_pTopClusters[i]->m_nRank  <= nReps)
    {
      m_nClustersReported++;

      switch(m_nClustersReported)
      {
        case 1 : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
        case 2 : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
        default: fprintf(fp, "\n"); break;
      }

      m_pTopClusters[i]->Display(fp, *m_pParameters, *m_pData,
                                 m_nClustersReported, nMinMeasure);

      if (fpGIS != NULL)
        m_pTopClusters[i]->DisplayCensusTracts(fpGIS, *m_pData,
                                               m_nClustersReported, nMinMeasure,
                                               m_pParameters->m_nReplicas,
                                               true, m_pParameters->m_nReplicas>99, 0, 0, ' ', NULL);
    }
  }

  fprintf(fp, "\n");

}

void CAnalysis::DisplayTopCluster(double nMinRatio, int nReps,
                                  FILE* fp, FILE* fpGIS)
{
  measure_t nMinMeasure = 0;

  if (m_nClustersRetained == 0)
    return;

  if (m_pTopClusters[0]->m_nRatio > nMinRatio &&
      m_pTopClusters[0]->m_nRank  <= nReps)
  {
    m_nClustersReported++;
    switch(m_nAnalysisCount)
    {
      case 1 : fprintf(fp, "\nMOST LIKELY CLUSTER\n\n"); break;
      case 2 : fprintf(fp, "\nSECONDARY CLUSTERS\n\n");  break;
      default: fprintf(fp,"                  _____________________________\n\n");
    }
    m_pTopClusters[0]->Display(fp, *m_pParameters, *m_pData,
                               m_nClustersReported, nMinMeasure);
    if (fpGIS != NULL)
      m_pTopClusters[0]->DisplayCensusTracts(fpGIS, *m_pData,
                                             m_nClustersReported, nMinMeasure,
                                             m_pParameters->m_nReplicas,
                                             true, m_pParameters->m_nReplicas>99, 0, 0, ' ', NULL);
  }

  fprintf(fp, "\n");
}

void CAnalysis::OpenReportFile(FILE*& fp, const char* szType)
{
  if ((fp = fopen(m_pParameters->m_szOutputFilename, szType)) == NULL)
  {
    if (szType == "w")
      printf("  Error: Unable to create report file.");
    else if (szType == "a")
      printf("  Error: Unable to open report file.");
    FatalError(0);
  }
}

void CAnalysis::OpenGISFile(FILE*& fpGIS, const char* szType)
{
  if ((fpGIS = fopen(m_pParameters->m_szGISFilename, szType)) == NULL)
  {
    if (szType == "w")
      printf("  Error: Unable to create GIS file.");
    else if (szType == "a")
      printf("  Error: Unable to open GIS file.");
    FatalError(0);
  }
}
// Start V.2.0.4.1
void CAnalysis::OpenLLRFile(FILE*& fpLLR, const char* szType)
{
  if ((fpLLR = fopen(m_pParameters->m_szLLRFilename, szType)) == NULL)
  {
    if (szType == "w")
      printf("  Error: Unable to create LLR file.");
    else if (szType == "a")
      printf("  Error: Unable to open LLR file.");
    FatalError(0);
  }
}
// End V.2.0.4.1

bool CAnalysis::CreateReport(time_t RunTime)
{
//  printf("Create report.\n");

  FILE* fp;
  FILE* fpGIS;

  OpenReportFile(fp, "w");
  OpenGISFile(fpGIS, "w");

  fprintf(fp,  "                 _____________________________\n\n");
  DisplayVersion(fp, 1);
  fprintf(fp,"                 _____________________________ \n\n");

  fprintf(fp,"\nProgram run on: %s\n", ctime(&RunTime));

  m_pParameters->DisplayAnalysisType(fp);
  m_pParameters->DisplayTimeAdjustments(fp);
  m_pData->DisplaySummary(fp);

  fclose(fp);
  fclose(fpGIS);

  return true;
}

bool CAnalysis::UpdateReport()
{
  FILE* fp;
  FILE* fpGIS;

  OpenReportFile(fp, "a");
  OpenGISFile(fpGIS, "a");

  if (m_pParameters->m_bSequential)
    DisplayTopCluster(m_nMinRatioToReport, m_pParameters->m_nReplicas, fp, fpGIS);
  else
    DisplayTopClusters(m_nMinRatioToReport, m_pParameters->m_nReplicas, fp, fpGIS);

  if (m_pParameters->m_nReplicas>=19)
  {
    fprintf(fp, "The log likelihood ratio value required for an observed\n");
    fprintf(fp, "cluster to be significant at level\n");

    if (m_pParameters->m_nReplicas>=99)
      fprintf(fp,"... 0.01: %f\n", SimRatios.GetAlpha01());
    if (m_pParameters->m_nReplicas>=19)
      fprintf(fp,"... 0.05: %f\n", SimRatios.GetAlpha05());
    fprintf(fp, "\n");
  }

  if (m_pParameters->m_bPowerCalc)
  {
    fprintf(fp,"Percentage of Monte Carlo replications with a likelihood greater than\n");
    fprintf(fp,"... X (%f) : %f\n", m_pParameters->m_nPower_X,
            ((double)m_nPower_X_Count)/m_pParameters->m_nReplicas);
    fprintf(fp,"... Y (%f) : %f\n\n", m_pParameters->m_nPower_Y,
            ((double)m_nPower_Y_Count)/m_pParameters->m_nReplicas);
  }

  fclose(fp);
  if (fpGIS != NULL)
    fclose(fpGIS);

  return true;
}

bool CAnalysis::FinalizeReport(time_t RunTime)
{
  FILE* fp;
  time_t CompletionTime;
  double nTotalTime;
  double nSeconds;
  double nMinutes;
  double nHours;
  char* szHours = "hours";
  char* szMinutes = "minutes";
  char* szSeconds = "seconds";

  OpenReportFile(fp, "a");

  if (m_nClustersRetained == 0)
  {
    fprintf(fp, "No clusters were found.\n");
    fprintf(fp, "All areas scanned had fewer cases than expected.\n");
  }
  else if (m_nClustersReported == 0)
  {
    fprintf(fp, "No clusters reported.\n");
    fprintf(fp, "All clusters had a ratio less than %f or\n"
                "a rank greater than %i.\n", m_nMinRatioToReport, m_pParameters->m_nReplicas);
  }
  else if (m_pParameters->m_nReplicas==0)
  {
    fprintf(fp, "Note: As the number of Monte Carlo replications was set to\n");
    fprintf(fp, "zero, no hypothesis testing was done and no p-values were\n");
    fprintf(fp, "printed.\n");
  }
  else if (m_pParameters->m_nReplicas<=99)
  {
    fprintf(fp, "Note: The number of Monte Carlo replications was set too low,\n");
    fprintf(fp, "and a meaningful hypothesis test cannot be done.  Consequently,\n");
    fprintf(fp, "no p-values were printed.\n");
  }

  if (m_pParameters->m_nModel == POISSON)
    tiReportZeroPops(fp);

  if (m_pParameters->m_bOutputRelRisks)
    m_pData->DisplayRelativeRisksForEachTract(fp);

  fprintf(fp, "________________________________________________________________\n\n");
  fprintf(fp,"For further study using a GIS or database program, an ASCII\n"
             "format GIS file has been created, describing the detected clusters.\n"
             "The name of this file is %s.\n", m_pParameters->m_szGISFilename);

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

  return true;
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
void CAnalysis::RankTopClusters()
{

  tract_t t, j;
  float newradius;                      /* Radius of new cluster tested */
  float clusterdistance;                /* Distance between the centers */
                                        /* of old and new clusters      */
  float radius[NUM_RANKED];             /* Array of old cluster radiia  */
  //float* x = new float[NUM_RANKED];
  //float* y = new float[NUM_RANKED];
  //float* z = new float[NUM_RANKED];     /* Coordinates for old clusters */
  //float x1, y1, z1;                     /* Coordinates for new cluster  */
  //float x2, y2, z2;                     /* Coordinates for edge tract   */
                                        	/* in the new cluster           */
  float** pCoords = (float**)Smalloc(NUM_RANKED * sizeof(float*));
  float* pCoords1;
  float* pCoords2;

  bool bInclude;                 /* 0/1 variable put to zero when a new */
                                 /* cluster should not be incuded.      */
  int i;

	// Initialize pCoords[] to NULL for now
  for(i=0; i<NUM_RANKED; i++)
  	pCoords[i] = NULL;

  /* Note: "Old clusters" are clusters already included on the list, while */
  /* a "new cluster" is the present candidate for inclusion.               */

  /* Sort by descending m_ratio, without regard to overlap */
//  SortTopClusters();
//  if (m_nClustersToKeepEachPass != 1)
    qsort(m_pTopClusters, m_nClustersRetained/*m_nMaxClusters/*NumClusters*/, sizeof(CCluster*), CompareClusters);

  // Remove "Undefined" clusters that have been sorted to bottome of list because ratio=-DBL_MAX
  tract_t nClustersAssigned = m_nClustersRetained;
  for (tract_t k=0; k<nClustersAssigned; k++)
  {
    if (!m_pTopClusters[k]->ClusterDefined())
    {
      m_nClustersRetained--;
      delete m_pTopClusters[k];
      m_pTopClusters[k] = NULL;
    }
  }

  if (m_nClustersRetained != 0)
  {
    /* Remove certain types of overlapping clusters from later printout */
    giGetCoords(m_pTopClusters[0]->m_Center, &pCoords[0]);
    tiGetCoords(m_pData->GetNeighbor(m_pTopClusters[0]->m_Center,m_pTopClusters[0]->GetNumTractsInnerCircle()/*m_nTracts*/),
                &pCoords2);

    //radius[0]=sqrt((x[0]-x2)*(x[0]-x2)+(y[0]-y2)*(y[0]-y2)+(z[0]-z2)*(z[0]-z2));
    radius[0] = (float)sqrt(tiGetDistanceSq(pCoords[0],pCoords2));
    free(pCoords2);
    t=1;

    while (t<m_nClustersToKeepEachPass && m_pTopClusters[t] != NULL) //*m_pTopClusters[t]->m_nRatio > -DBL_MAX
    {
      giGetCoords(m_pTopClusters[t]->m_Center, &pCoords1);
      tiGetCoords(m_pData->GetNeighbor(m_pTopClusters[t]->m_Center,m_pTopClusters[t]->GetNumTractsInnerCircle()),
                  &pCoords2);
      //newradius=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
      newradius = (float)sqrt(tiGetDistanceSq(pCoords1,pCoords2));
      free(pCoords2);
      bInclude=1;

      j=0;
      while (bInclude && j<t)
      {
        //clusterdistance = sqrt((x1-x[j])*(x1-x[j])+(y1-y[j])*(y1-y[j])+(z1-z[j])*(z1-z[j]));
        clusterdistance = (float)sqrt(tiGetDistanceSq(pCoords1,pCoords[j]));
        if (clusterdistance <= radius[j]+newradius)
          bInclude=0;
        j++;
      } // while bInclude

      if (bInclude)
      {
        radius[t] = newradius;
        //x[t] = x1;
        //y[t] = y1;
        //z[t] = z1;
        // Now allocate new pCoords[]
        pCoords[t] = (float*)Smalloc(m_pParameters->m_nDimension * sizeof(float));

        // Loop through values of pCoords1
        for (i=0; i<m_pParameters->m_nDimension; i++)
        {
        	pCoords[t][i] = pCoords1[i];
        }
        // Don't need pCoords1 anymore
        free(pCoords1);
        t++;
      } // if bInclude
      else
      {
        m_nClustersRetained--;
        delete m_pTopClusters[t];
        for (j = t; j < m_nClustersRetained; //m_nMaxClusters - 1
             j++)
          m_pTopClusters[j] = m_pTopClusters[j + 1];
        m_pTopClusters[m_nClustersRetained] = NULL;
        //m_pTopClusters[NumClusters-1]->m_nRatio = -DBL_MAX;
        free(pCoords1);
      } // if-else

    	//free(pCoords1);  // NO GOOD-access freed mem

    }  // while t
    //free(pCoords1);  // NO GOOD-doesn't free "enough"
    m_nClustersRetained = t;

    // Delete unused clusters
    for (t = m_nClustersToKeepEachPass; t < nClustersAssigned; t++)
      delete m_pTopClusters[t];
    //    m_pTopClusters[t]->m_nRatio = -DBL_MAX;
  }

  //delete [] x;
  //delete [] y;
  //delete [] z;
  //Clean up pCoords allocation
  for (i=0; i<NUM_RANKED; i++)
  {
  	if(pCoords[i] != NULL)
	  	free(pCoords[i]);
  }
  free(pCoords);

  //free(pCoords1);
} /* RankClusters() */

/**********************************************************************
 Comparison function for sorting clusters by descending m_ratio
 **********************************************************************/
/*static*/ int CAnalysis::CompareClusters(const void *a, const void *b)
{
   double rdif = (*(CCluster**)b)->m_nRatio - (*(CCluster**)a)->m_nRatio;
   if (rdif < 0.0)   return -1;
   if (rdif > 0.0)   return 1;
   return 0;
} /* CompClust() */


void CAnalysis::AllocateTopClusterList()
{
  m_pTopClusters = new CCluster* [m_nMaxClusters];
}

void CAnalysis::UpdateTopClustersRank(double r)
{
  for (int i=m_nClustersRetained-1; i>=0; i--)
  {
    if (m_pTopClusters[i]->m_nRatio > r)
      break;
    m_pTopClusters[i]->m_nRank++;
  }
}

void CAnalysis::SortTopClusters()
{
  qsort(m_pTopClusters, m_nClustersRetained, sizeof(CCluster*), CompareClusters);
}

bool CAnalysis::RepeatAnalysis()
{
  if (!m_pParameters->m_bSequential)
    return false;
  else
   return ((m_nAnalysisCount < m_pParameters->m_nAnalysisTimes)    &&
           (m_pTopClusters[0]->GetPVal(m_pParameters->m_nReplicas) <
            m_pParameters->m_nCutOffPVal)                           &&
           (m_pData->m_nTracts > 1));
}

void CAnalysis::RemoveTopClusterData()
{
  tract_t nNeighbor;

//  printf("Removeing Top Clustser Data.\n");

  for (int i=1; i <= m_pTopClusters[0]->m_nTracts; i++)
  {
    nNeighbor = m_pData->GetNeighbor(m_pTopClusters[0]->m_Center, i);

    m_pData->m_nTotalCases    -= m_pData->m_pCases[0][nNeighbor];
    m_pData->m_nTotalMeasure  -= m_pData->m_pMeasure[0][nNeighbor];

    m_pData->m_pCases[0][nNeighbor]   = 0;
    m_pData->m_pMeasure[0][nNeighbor] = 0;

    if (m_pParameters->m_nModel==BERNOULLI)
    {
      m_pData->m_nTotalControls -= m_pData->m_pControls[0][nNeighbor];
      m_pData->m_pControls[0][nNeighbor]   = 0;
    }

    m_pData->m_nTracts--;
  }

  InitializeTopClusterList();
  m_nClustersRetained = 0;

  m_pData->SetMaxCircleSize();
  m_pData->AdjustNeighborCounts();  
}

void CAnalysis::InitializeTopClusterList()
{
  for (int i=0; i<m_nClustersRetained; i++)
    delete m_pTopClusters[i];
}


