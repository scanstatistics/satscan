// CBernoulliModel.cpp

#include "BernoulliModel.h"
#include "data.h"
#include "randdist.h"
#include "tinfo.h"
#include "cats.h"
#include "rng.h"
#include "display.h"
#include "error.h"
#define DEBUG 1

CBernoulliModel::CBernoulliModel(CParameters* pParameters, CSaTScanData* pData)
                :CModel(pParameters, pData)
{
}

CBernoulliModel::~CBernoulliModel()
{
}

bool CBernoulliModel::ReadData()
{
  //int i, j;

  if (!m_pData->ReadGeo())
    return false;

  if (!m_pData->ReadCounts(m_pParameters->m_szCaseFilename,
                           "case",
                           &m_pData->m_pCases))
    return false;

  if (!m_pData->ReadCounts(m_pParameters->m_szControlFilename,
                           "control",
                           &m_pData->m_pControls))
    return false;

  if (m_pParameters->m_bSpecialGridFile)
  {
    if (!m_pData->ReadGrid())
      return false;
  }

  return true;
}

bool CBernoulliModel::CalculateMeasure()
{
  int i, j;

  printf("Calculating expected number of cases\n");

  // Why allocated +1?  KR-980417
  m_pData->m_pMeasure = (double**)Smalloc((m_pData->m_nTimeIntervals+1) * sizeof(measure_t *));
  for (i=0; i<m_pData->m_nTimeIntervals+1; i++)
	 m_pData->m_pMeasure[i] = (double*)Smalloc(m_pData->m_nTracts * sizeof(measure_t));

  m_pData->m_nTotalCases    = 0;
  m_pData->m_nTotalControls = 0;
  m_pData->m_nTotalMeasure  = 0;

  for (j=0; j<m_pData->m_nTracts; j++)
  {
    m_pData->m_nTotalCases    += m_pData->m_pCases[0][j];
    m_pData->m_nTotalControls += m_pData->m_pControls[0][j];

    for (i=0; i<m_pData->m_nTimeIntervals/*+1*/; i++)
    {
      m_pData->m_pMeasure[i][j]  = m_pData->m_pCases[i][j] + m_pData->m_pControls[i][j];
    }

    m_pData->m_nTotalMeasure += m_pData->m_pMeasure[0][j];
    m_pData->m_pMeasure[i][j] = 0;

    // Check to see if total case or control values have wrapped
    if (m_pData->m_nTotalCases < 0)
    {
      fprintf(stderr, "  Error: Total cases greater than maximum allowed.\n");
      return false;
    }

    if (m_pData->m_nTotalControls < 0)
    {
      fprintf(stderr, "  Error: Total controls greater than maximum allowed.\n");
      return false;
    }

  }

  if (m_pData->m_nTotalControls == 0)
  {
    printf("  Error: No controls found in input data.\n");
    return false;
  }

  m_pData->m_nTotalPop = m_pData->m_nTotalMeasure;

  return true;
}

double CBernoulliModel::GetLogLikelihoodForTotal() const
{
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

  return N*log(N/U) + (U-N)*log((U-N)/U);
}

double CBernoulliModel::CalcLogLikelihood(count_t n, measure_t u)
{
  double    nLogLikelihood;
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

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

double CBernoulliModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster)
{
  double    nLogLikelihood = 0;
  count_t   n;
  measure_t u;

  for (int i=0; i<PSMCluster.m_nSteps; i++)
  {
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

void CBernoulliModel::MakeData()
{
  count_t nCumCounts;
  count_t nCumMeasure;
  tract_t   tract;                                       // current tract
  int       interval;                            // current time interval
  count_t* RandCounts;

	if (m_pData->m_nTotalCases < m_pData->m_nTotalControls)
  {
  	RandCounts = MakeDataB(m_pData->m_nTotalCases, RandCounts);
    nCumCounts = m_pData->m_nTotalCases;
  }
  else
  {
  	RandCounts = MakeDataB(m_pData->m_nTotalControls, RandCounts);
    nCumCounts = m_pData->m_nTotalControls;
  }
  // The following works if Cases < Controls but what about the other way

  //nCumCounts = m_pData->m_nTotalCases;
  nCumMeasure = (count_t)(m_pData->m_nTotalMeasure-1);

  for (tract = (tract_t)(m_pData->m_nTotalTractsAtStart-1); tract >= 0; tract--)
  {
  	for (interval = m_pData->m_nTimeIntervals-1; interval >= 0; interval--)
		{
  		m_pData->m_pSimCases[interval][tract] = 0;
      if (interval == m_pData->m_nTimeIntervals-1)
   			nCumMeasure -= (count_t)(m_pData->m_pMeasure[interval][tract]);
      else
        nCumMeasure -= (count_t)(m_pData->m_pMeasure[interval][tract] - m_pData->m_pMeasure[interval+1][tract]);

    	while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure)
    	{
    		m_pData->m_pSimCases[interval][tract]++;
      	nCumCounts--;
    	}
      if (interval != m_pData->m_nTimeIntervals-1)
				m_pData->m_pSimCases[interval][tract] += m_pData->m_pSimCases[interval+1][tract];

      #if (DEBUGMODEL)
      fprintf(m_pDebugModelFile,"SimCases[%d][%d] = %d\n", interval, tract,
      	m_pData->m_pSimCases[interval][tract]);
      #endif
		}
//    #if (DEBUG)
//    HoldForEnter();
//    #endif
  }
  // Now reverse everything if Controls < Cases
  if (m_pData->m_nTotalCases >= m_pData->m_nTotalControls)
  {
  	for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++)
      for (interval = 0; interval < m_pData->m_nTimeIntervals; interval++)
      {
    		m_pData->m_pSimCases[interval][tract] = (long)(m_pData->m_pMeasure[interval][tract]) -
        	m_pData->m_pSimCases[interval][tract];
      }
  }
  free(RandCounts);
}

count_t * CBernoulliModel::MakeDataB(count_t nTotalCounts, count_t* RandCounts)
{
  count_t nCumCounts = 0;
  //count_t nCumMeasure = 0;
	int i;
  double x;
  double ratio;

  RandCounts = (count_t*)Smalloc(nTotalCounts * sizeof(count_t));
  for (i=0; i < m_pData->m_nTotalMeasure; i++)
  {
   	x = rngRand();
    ratio = (double) (nTotalCounts-nCumCounts)/
      (m_pData->m_nTotalMeasure-i);
		if (x <= ratio)
    {
			RandCounts[nCumCounts] = i;

//			#if (DEBUG)
//    	printf("RandCounts[%d] = %d\n", nCumCounts, RandCounts[nCumCounts]);
//			#endif

			nCumCounts++;
		}
  }	//end of for(i=0...)
  return(RandCounts);
}

double CBernoulliModel::GetPopulation(tract_t nCenter, tract_t nTracts,
                                      int nStartInterval, int nStopInterval)
{
  double  nPop = 0.0;
  count_t nNeighbor;

  for (int i=1; i<=nTracts; i++)
  {
    nNeighbor = m_pData->GetNeighbor(nCenter, i);
    nPop += m_pData->m_pMeasure[nStartInterval][nNeighbor] -
            m_pData->m_pMeasure[nStopInterval][nNeighbor];
  }
  return nPop;
}

