#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAlive.h"
#include "MeasureList.h"
#include "Cluster.h"

/** constructor */
CTIAlive::CTIAlive(int nTotal, int nCut) : CTimeIntervals(nTotal, nCut) {}

/** copy constructor */
CTIAlive::CTIAlive(const CTIAlive& rhs) : CTimeIntervals(rhs) {}

/** returns newly cloned CTIAlive */
CTIAlive * CTIAlive::Clone() const {
  return new CTIAlive(*this);
}

/** Iterates through all possible 'alive' time windows for runnning cluster,
    comparing against current top cluster. Reassigns top cluster if running
    cluster ever has a greater loglikelihood.*/
void CTIAlive::CompareClusters(CCluster & Running, CCluster & TopShapeCluster, const CSaTScanData& Data,
                                        const count_t* pCases, const measure_t* pMeasure) {
  int           iWindowStart, iWindowEnd;
  CModel      & ProbabilityModel(Data.GetProbabilityModel());
  count_t       tTotalCases(Data.GetNumCases());
  double	dTotalMeasure(Data.GetTotalMeasure();

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart) {
       Running.m_nCases = pCases[iWindowStart];
       Running.m_nMeasure = pMeasure[iWindowStart];
       if (Running.RateIsOfInterest(tTotalCases, dTotalMeasure)) {
          Running.m_nLogLikelihood = ProbabilityModel.CalcLogLikelihood(Running.m_nCases, Running.m_nMeasure);
          if (Running.m_nLogLikelihood  > TopShapeCluster.m_nLogLikelihood) {
            TopShapeCluster.AssignAsType(Running);
            TopShapeCluster.m_nFirstInterval = iWindowStart;
            TopShapeCluster.m_nLastInterval = iWindowEnd;
          }
       }
  }
}

/** Iterates through all possible time windows to determine the most significant
    measure for number of cases.*/
void CTIAlive::ComputeBestMeasures(const count_t* pCases, const measure_t* pMeasure,
                                   CMeasureList & MeasureList) {
  int  iWindowStart, iWindowEnd;

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart)
       MeasureList.AddMeasure(pCases[iWindowStart], pMeasure[iWindowStart]);
}

/** Returns the number of cases that tract attributed to accumulated case count. */
count_t CTIAlive::GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const
{
   count_t      tCaseCount;

   if (Cluster.m_nFirstInterval == giNumIntervals)
     tCaseCount = 0;
   else
     tCaseCount = pCases[Cluster.m_nFirstInterval][tTract];

   return tCaseCount;
}                                                                                

/** Returns the measure that tract attributed to accumulated measure. */
measure_t CTIAlive::GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const
{
   measure_t      tMeasure;

   if (Cluster.m_nFirstInterval == giNumIntervals)
     tMeasure = 0;
   else
     tMeasure = pMeasure[Cluster.m_nFirstInterval][tTract];

   return tMeasure;
}


