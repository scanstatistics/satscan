#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAlive.h"
#include "MeasureList.h"
#include "Cluster.h"

/** constructor */
CTIAlive::CTIAlive(const CSaTScanData& Data)
         :CTimeIntervals(Data.GetNumTimeIntervals(), Data.m_nIntervalCut), gData(Data) {}

/** copy constructor */
CTIAlive::CTIAlive(const CTIAlive& rhs) : CTimeIntervals(rhs), gData(rhs.gData) {}

/** returns newly cloned CTIAlive */
CTIAlive * CTIAlive::Clone() const {
  return new CTIAlive(*this);
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void CTIAlive::CompareClusters(CCluster & Running, CCluster & TopShapeCluster, const count_t* pCases,
                               const measure_t* pMeasure, const measure_t* ppMeasureSquared) {
                               
  int           iWindowStart, iWindowEnd;
  CModel      & ProbabilityModel(gData.GetProbabilityModel());
  count_t       tCases, tTotalCases(gData.GetTotalCases());
  measure_t	tMeasure, tTotalMeasure(gData.GetTotalMeasure());

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart) {
     tCases = pCases[iWindowStart];
     tMeasure = Running.g_Measure_(iWindowStart, pMeasure, ppMeasureSquared);
     if (Running.RateIsOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
       Running.m_nRatio = ProbabilityModel.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure, Running.m_DuczmalCorrection);
       if (Running.m_nRatio > TopShapeCluster.m_nRatio) {
         TopShapeCluster.AssignAsType(Running);
         TopShapeCluster.m_nFirstInterval = iWindowStart;
         TopShapeCluster.m_nLastInterval = iWindowEnd;
         TopShapeCluster.SetCaseCount(0, tCases);
         TopShapeCluster.SetMeasure(0, tMeasure);
       }
     }
  }
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void CTIAlive::CompareDataStreamClusters(CCluster & Running,
                                                 CCluster & TopShapeCluster,
                                                 ZdPointerVector<AbstractTemporalClusterStreamData> & StreamData) {

  int                                   iWindowStart, iWindowEnd;
  CModel                              & Model(gData.GetProbabilityModel());
  AbstractTemporalClusterStreamData   * pStreamData;


  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart) {
     Running.m_nRatio = 0;
     for (size_t t=0; t < StreamData.size(); ++t) {
       pStreamData = StreamData[t];
       pStreamData->gCases = pStreamData->gpCases[iWindowStart];
       pStreamData->gMeasure = Running.g_Measure_(iWindowStart, pStreamData->gpMeasure, pStreamData->gpSqMeasure);
       if (Running.RateIsOfInterest(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure))
          Running.m_nRatio += Model.CalcLogLikelihoodRatio(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure, Running.m_DuczmalCorrection);
     }
     if (Running.m_nRatio && Running.m_nRatio > TopShapeCluster.m_nRatio) {
       TopShapeCluster.AssignAsType(Running);
       TopShapeCluster.m_nFirstInterval = iWindowStart;
       TopShapeCluster.m_nLastInterval = iWindowEnd;
     }
  }
}

/** Iterates through all possible time windows to determine the most significant
    measure for number of cases.*/
void CTIAlive::ComputeBestMeasures(const count_t* pCases, const measure_t* pMeasure,
                                   const measure_t* pMeasureSquared, CMeasureList & MeasureList) {
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


