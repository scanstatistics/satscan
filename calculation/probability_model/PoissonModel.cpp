// CPoissonModel.cpp

#include "PoissonModel.h"
#include "CalcMsr.h"
#include "randdist.h"
#include "tinfo.h"
#include "cats.h"

CPoissonModel::CPoissonModel(CParameters* pParameters, CSaTScanData* pData)
              :CModel(pParameters, pData)
{
}

CPoissonModel::~CPoissonModel()
{
}

bool CPoissonModel::ReadData()
{
  if (!m_pData->ReadGeo())
    return false;

  if (!m_pData->ReadPops())
    return false;

  if (!tiCheckZeroPops(stderr))
    return false;

  if (!m_pData->ReadCounts(m_pParameters->m_szCaseFilename,
                           "case",
                           &m_pData->m_pCases))
    return false;

  if (!(tiCheckCasesHavePop()))
    return false;

  if (m_pParameters->m_bSpecialGridFile)
  {
    if (!m_pData->ReadGrid())
      return false;
  }

  return true;
}

bool CPoissonModel::CalculateMeasure()
{
  bool bResult = AssignMeasure(m_pData->m_pCases,
                       m_pData->m_pTimes,
                       m_pData->m_nTracts,
                       m_pData->m_nStartDate,
                       m_pData->m_nEndDate,
                       m_pData->m_pIntervalStartTimes,
       					  m_pParameters->m_bExactTimes,
                       m_pParameters->m_nTimeAdjustType,
                       m_pParameters->m_nTimeAdjPercent,
                       m_pData->m_nTimeIntervals,
                       m_pParameters->m_nIntervalUnits,
                       m_pParameters->m_nIntervalLength,
         				  &m_pData->m_pMeasure,
                       &m_pData->m_nTotalCases,
                       &m_pData->m_nTotalPop,
                       &m_pData->m_nTotalMeasure);

//  m_pData->m_nTotalTractsAtStart   = m_pData->m_nTracts;
//  m_pData->m_nTotalCasesAtStart    = m_pData->m_nTotalCases;
//  m_pData->m_nTotalControlsAtStart = m_pData->m_nTotalControls;
//  m_pData->m_nTotalMeasureAtStart  = m_pData->m_nTotalMeasure;

//  m_pData->SetMaxCircleSize();

  return bResult;
}

double CPoissonModel::GetLogLikelihoodForTotal() const
{
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

  return N*log(N/U);
}

double CPoissonModel::CalcLogLikelihood(count_t n, measure_t u)
{
  double    nLogLikelihood;
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

  if (n != N && n != 0)
    nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
  else if (n == 0)
    nLogLikelihood = (N-n) * log((N-n)/(U-u));
  else
    nLogLikelihood = n*log(n/u);

  return (nLogLikelihood);
}

double CPoissonModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster)
{
  double nLogLikelihood = 0;

  for (int i=0; i<PSMCluster.m_nSteps; i++)
  {
    if (PSMCluster.m_pCasesList[i] != 0)
      nLogLikelihood += PSMCluster.m_pCasesList[i] *
                        log(PSMCluster.m_pCasesList[i]/PSMCluster.m_pMeasureList[i]);
  }

  return nLogLikelihood;
}

void CPoissonModel::MakeData()
{
  count_t   cumcases = 0;                       // simulated cases so far
  measure_t cummeasure = 0;                             // measure so far
  tract_t   tract;                                       // current tract
  int       interval;                            // current time interval
  count_t   c;                                          // cases in tract
  count_t   d;                                       // cases in interval

  // Generate case counts
  for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++)
  {
    if (m_pData->m_nTotalMeasure-cummeasure > 0)
      c = Binomial(m_pData->m_nTotalCases - cumcases,
          (float)(m_pData->m_pMeasure[0][tract] / (m_pData->m_nTotalMeasure-cummeasure)));
    else
      c = 0;

    m_pData->m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += m_pData->m_pMeasure[0][tract];

    for(interval=0;interval<m_pData->m_nTimeIntervals-1;interval++)
    {
      if(m_pData->m_pMeasure[interval][tract]>0)
        d = Binomial(m_pData->m_pSimCases[interval][tract],
            (float)(1 - m_pData->m_pMeasure[interval+1][tract] / m_pData->m_pMeasure[interval][tract]));
      else
        d = 0;

      m_pData->m_pSimCases[interval+1][tract] = m_pData->m_pSimCases[interval][tract] - d;
    } // for interval

  } // for tract

}

/* Not yet working - KR980325
void MakePurelyTemporalData(measure_t* pMeasure,
                            int        nTimeIntervals,
                            count_t    nTotalCases,
                            measure_t  nTotalMeasure,
                            count_t*   pSimCases)
{
  count_t   nCases   = 0;
  measure_t nMeasure = 0;
  count_t   c;

  for (int i=0; i<nTimeIntervals-1; i++)
  {
    if (nTotalMeasure-nMeasure > 0)
      c = Binomial(nTotalCases - nCases,
                   pMeasure[i] / (nTotalMeasure-nMeasure));
    else
      c = 0;

    pSimCases[i] = c;
    nCases      += c;
    nMeasure    += pMeasure[i];
  }

}
*/

double CPoissonModel::GetPopulation(tract_t nCenter, tract_t nTracts,
                                    int nStartInterval, int nStopInterval)
{
   tract_t T, t;
   int     c, n;
   double* pAlpha;
   int     ncats = catNumCats();
   int     nPops = tiGetNumPopDates();
   double  nPopulation = 0.0;

   tiCalcAlpha(&pAlpha, m_pData->m_nStartDate, m_pData->m_nEndDate);

   for (T = 1; T <= nTracts; T++)
   {
      t = m_pData->GetNeighbor(nCenter, T);
      for (c = 0; c < ncats; c++)
         for (n=0; n < nPops; n++)
            nPopulation += (pAlpha[n]*tiGetPop(t, c, n));
   }

   free(pAlpha);

   return nPopulation;
}


