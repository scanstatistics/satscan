#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAlive.h"
#include "MeasureList.h"
#include "Cluster.h"

/** constructor */
CTIAlive::CTIAlive(const CSaTScanData& Data)
         :CTimeIntervals(Data.GetNumTimeIntervals(), Data.m_nIntervalCut, Data.GetParameters().GetAreaScanRateType()),
          gData(Data) {}


/** copy constructor */
CTIAlive::CTIAlive(const CTIAlive& rhs) : CTimeIntervals(rhs), gData(rhs.gData) {}

/** returns newly cloned CTIAlive */
CTIAlive * CTIAlive::Clone() const {
  return new CTIAlive(*this);
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void CTIAlive::CompareClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int           iWindowStart, iWindowEnd;
  CModel      & ProbabilityModel(gData.GetProbabilityModel());
  count_t       tCases, tTotalCases(gData.GetTotalCases());
  measure_t	tMeasure, tTotalMeasure(gData.GetTotalMeasure());
  AbstractTemporalClusterStreamData * pStreamData = StreamData[0];

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart) {
     tCases = pStreamData->gpCases[iWindowStart];
     tMeasure = pStreamData->gpMeasure[iWindowStart];
     if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
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
void CTIAlive::CompareClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int           iWindowStart, iWindowEnd;
  CModel      & ProbabilityModel(gData.GetProbabilityModel());
  count_t       tCases, tTotalCases(gData.GetTotalCases());
  measure_t	tMeasure, tMeasure2, tTotalMeasure(gData.GetTotalMeasure());
  AbstractTemporalClusterStreamData * pStreamData = StreamData[0];

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart) {
     tCases = pStreamData->gpCases[iWindowStart];
     tMeasure = pStreamData->gpMeasure[iWindowStart];
     tMeasure2 = pStreamData->gpSqMeasure[iWindowStart];
     if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
       //calculate with second measure
       Running.m_nRatio = ProbabilityModel.CalcLogLikelihoodRatioEx(tCases, tMeasure, tMeasure2, tTotalCases, tTotalMeasure, Running.m_DuczmalCorrection);
       if (Running.m_nRatio > TopShapeCluster.m_nRatio) {
         TopShapeCluster.AssignAsType(Running);
         TopShapeCluster.m_nFirstInterval = iWindowStart;
         TopShapeCluster.m_nLastInterval = iWindowEnd;
         TopShapeCluster.SetCaseCount(0, tCases);
         TopShapeCluster.SetMeasure(0, tMeasure);
         //set second measure
       }
     }
  }
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void CTIAlive::CompareDataStreamClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {
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
       pStreamData->gMeasure = pStreamData->gpMeasure[iWindowStart];
       if (fRateOfInterest(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure))
          Running.m_nRatio += Model.CalcLogLikelihoodRatio(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure, Running.m_DuczmalCorrection);
     }
     if (Running.m_nRatio && Running.m_nRatio > TopShapeCluster.m_nRatio) {
       TopShapeCluster.AssignAsType(Running);
       TopShapeCluster.m_nFirstInterval = iWindowStart;
       TopShapeCluster.m_nLastInterval = iWindowEnd;
     }
  }
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void CTIAlive::CompareDataStreamClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {
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
       pStreamData->gMeasure = pStreamData->gpMeasure[iWindowStart];
       pStreamData->gSqMeasure = pStreamData->gpSqMeasure[iWindowStart];;
       if (fRateOfInterest(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure))
          //calculate with second measure
          Running.m_nRatio += Model.CalcLogLikelihoodRatioEx(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gSqMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure, Running.m_DuczmalCorrection);
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
void CTIAlive::ComputeBestMeasures(AbstractTemporalClusterStreamData * pStreamData, CMeasureList & MeasureList) {
  int  iWindowStart, iWindowEnd;

  //iterate through all possible 'alive' windows
  iWindowStart = giNumIntervals - giMaxWindowLength;
  iWindowEnd  = giNumIntervals;
  for (; iWindowStart < iWindowEnd; ++iWindowStart)
       MeasureList.AddMeasure(pStreamData->gpCases[iWindowStart], pStreamData->gpMeasure[iWindowStart]);
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


