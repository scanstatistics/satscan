#include "SaTScan.h"
#pragma hdrstop
#include "BernoulliModel.h"

#define DEBUG 1

/** Constructor */
CBernoulliModel::CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                :CModel(Parameters, Data, PrintDirection) {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

bool CBernoulliModel::ReadData() {
  try {
    if (!gData.ReadCoordinatesFile())
      return false;
    if (! gData.ReadCaseFile())
      return false;
    if (! gData.ReadControlFile())
      return false;
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()", "CBernoulliModel");
    throw;
  }
  return true;
}

bool CBernoulliModel::CalculateMeasure() {
  int i, j;

  try {
    gPrintDirection.SatScanPrintf("Calculating expected number of cases\n");

    // Why allocated +1?  KR-980417
    gData.m_pMeasure = (double**)Smalloc((gData.m_nTimeIntervals+1) * sizeof(measure_t *), &gPrintDirection);
    for (i=0; i<gData.m_nTimeIntervals+1; i++)
       gData.m_pMeasure[i] = 0;
    for (i=0; i<gData.m_nTimeIntervals+1; i++) {
       gData.m_pMeasure[i] = (double*)Smalloc(gData.m_nTracts * sizeof(measure_t), &gPrintDirection);
       if (! gData.m_pMeasure[i])
         SSGenerateException("Could not allocate memory for m_pMeasure[].","CalculateMeasure()");
    }

    gData.m_nTotalCases    = 0;
    gData.m_nTotalControls = 0;
    gData.m_nTotalMeasure  = 0;

    for (j=0; j<gData.m_nTracts; j++) {
       gData.m_nTotalCases    += gData.m_pCases[0][j];
       gData.m_nTotalControls += gData.m_pControls[0][j];
       for (i=0; i<gData.m_nTimeIntervals/*+1*/; i++) {
          gData.m_pMeasure[i][j]  = gData.m_pCases[i][j] + gData.m_pControls[i][j];
       }
       gData.m_nTotalMeasure += gData.m_pMeasure[0][j];
       gData.m_pMeasure[i][j] = 0;

       // Check to see if total case or control values have wrapped
        if (gData.m_nTotalCases < 0)
          SSGenerateException("Error: Total cases greater than maximum allowed.\n", "CBernoulliModel");
        if (gData.m_nTotalControls < 0)
          SSGenerateException("Error: Total controls greater than maximum allowed.\n", "CBernoulliModel");
    }

    if (gData.m_nTotalControls == 0)
      SSGenerateException("Error: No controls found in input data.\n", "CBernoulliModel");

    gData.m_nTotalPop = gData.m_nTotalMeasure;
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CBernoulliModel");
    throw;
  }
  return true;
}

double CBernoulliModel::GetLogLikelihoodForTotal() const {
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;
  
  return N*log(N/U) + (U-N)*log((U-N)/U);
}

double CBernoulliModel::CalcLogLikelihood(count_t n, measure_t u) {
  double    nLogLikelihood;
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

  double    nLL_A = 0.0;
  double    nLL_B = 0.0;
  double    nLL_C = 0.0;
  double    nLL_D = 0.0;

  if (n != 0)
    nLL_A = n*log(n/u);
  if (n != u)
    nLL_B = (u-n)*log(1-(n/u));
  if (N-n != 0)
    nLL_C = (N-n)*log((N-n)/(U-u));
   if (N-n != U-u)
    nLL_D = ((U-u)-(N-n))*log(1-((N-n)/(U-u)));

  nLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

  return (nLogLikelihood);
}

double CBernoulliModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  double    nLogLikelihood = 0;
  count_t   n;
  measure_t u;

  for (int i=0; i<PSMCluster.m_nSteps; i++) {
     n = PSMCluster.m_pCasesList[i];
     u = PSMCluster.m_pMeasureList[i];
     if (n != 0  && n != u)
       nLogLikelihood += n*log(n/u) + (u-n)*log(1-(n/u));
     else if (n == 0)
       nLogLikelihood += (u-n)*log(1-(n/u));
     else if (n == u)
       nLogLikelihood += n*log(n/u);
  }

  return nLogLikelihood;
}

void CBernoulliModel::MakeData(int iSimulationNumber)
{
   count_t nCumCounts;
   count_t nCumMeasure;
   tract_t   tract;                                       // current tract
   int       interval;                            // current time interval
   count_t* RandCounts = 0;

   try
      {
      //reset seed to simulation number
      m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
      if (gData.m_nTotalCases < gData.m_nTotalControls)
         {
  	 RandCounts = MakeDataB(gData.m_nTotalCases, RandCounts);
         nCumCounts = gData.m_nTotalCases;
         }
      else
         {
  	 RandCounts = MakeDataB(gData.m_nTotalControls, RandCounts);
         nCumCounts = gData.m_nTotalControls;
         }
      // The following works if Cases < Controls but what about the other way

      //nCumCounts = gData.m_nTotalCases;
      nCumMeasure = (count_t)(gData.m_nTotalMeasure-1);
    
      for (tract = (tract_t)(gData.m_nTotalTractsAtStart-1); tract >= 0; tract--)
      {
      	for (interval = gData.m_nTimeIntervals-1; interval >= 0; interval--)
           {
      	   gData.m_pSimCases[interval][tract] = 0;
           if (interval == gData.m_nTimeIntervals-1)
       	      nCumMeasure -= (count_t)(gData.m_pMeasure[interval][tract]);
           else
              nCumMeasure -= (count_t)(gData.m_pMeasure[interval][tract] - gData.m_pMeasure[interval+1][tract]);

           while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure)
           {
              gData.m_pSimCases[interval][tract]++;
              nCumCounts--;
           }
          if (interval != gData.m_nTimeIntervals-1)
    	   	gData.m_pSimCases[interval][tract] += gData.m_pSimCases[interval+1][tract];
    
          #ifdef DEBUGMODEL
          fprintf(m_pDebugModelFile,"SimCases[%d][%d] = %d\n", interval, tract,
          	gData.m_pSimCases[interval][tract]);
          #endif
    		}
//    #if (DEBUG)
    //    HoldForEnter();
//    #endif
      }
      // Now reverse everything if Controls < Cases
      if (gData.m_nTotalCases >= gData.m_nTotalControls)
      {
      	for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++)
          for (interval = 0; interval < gData.m_nTimeIntervals; interval++)
          {
          gData.m_pSimCases[interval][tract] = (long)(gData.m_pMeasure[interval][tract]) -
          gData.m_pSimCases[interval][tract];
          }
      }
      free(RandCounts);
      }
   catch (ZdException & x)
      {
      free(RandCounts);
      x.AddCallpath("MakeData()", "CBernoulliModel");
      throw;
      }
}

count_t * CBernoulliModel::MakeDataB(count_t nTotalCounts, count_t* RandCounts)
{
   count_t nCumCounts = 0;
   //count_t nCumMeasure = 0;
   int i;
   double x;
   double ratio;

   try
      {
      RandCounts = (count_t*)Smalloc(nTotalCounts * sizeof(count_t), &gPrintDirection);
      if (!RandCounts)
         SSGenerateException("Could not allocate memory for RandCounts.", "MakeDataB()");
      for (i=0; i < gData.m_nTotalMeasure; i++)
      {
       	x = m_RandomNumberGenerator.GetRandomDouble();
        ratio = (double) (nTotalCounts-nCumCounts)/
          (gData.m_nTotalMeasure-i);
    		if (x <= ratio)
        {
    			RandCounts[nCumCounts] = i;
    
    //			#if (DEBUG)
    //    	printf("RandCounts[%d] = %d\n", nCumCounts, RandCounts[nCumCounts]);
    //			#endif
    
    			nCumCounts++;
    		}
      }	//end of for(i=0...)
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MakeDataB(count_t, count_t *)", "CBernoulliModel");
      throw;
      }
   return(RandCounts);
}

double CBernoulliModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  double  nPop = 0.0;
  count_t nNeighbor;

  for (int i=1; i<=nTracts; i++) {
     nNeighbor = gData.GetNeighbor(m_iEllipseOffset, nCenter, i);
     nPop += gData.m_pMeasure[nStartInterval][nNeighbor] - gData.m_pMeasure[nStopInterval][nNeighbor];
  }

  return nPop;
}

