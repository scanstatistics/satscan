//*****************************************************************************
#ifndef __PURELYTEMPORALANALYSIS_H
#define __PURELYTEMPORALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "MeasureList.h"
#include "TimeIntervalRange.h"
#include "PurelyTemporalData.h"
#include "PurelyTemporalCluster.h"

class CPurelyTemporalAnalysis : public CAnalysis {
  private:
    CPurelyTemporalCluster      * gpTopCluster;
    CPurelyTemporalCluster      * gpClusterComparator;
    TemporalData                * gpClusterData;
    CMeasureList                * gpMeasureList;
    CTimeIntervals              * gpTimeIntervals;    

    void                          Init();
    void                          SetMaxNumClusters() {m_nMaxClusters = 1;}
    void                          Setup();

  protected:
    virtual void                  AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual void                  AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster     &  CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway);
    virtual bool                  FindTopClusters(const AbtractDataStreamGateway & DataGateway);
    virtual double                FindTopRatio(const AbtractDataStreamGateway & DataGateway);
    virtual double                MonteCarlo(const DataStreamInterface & Interface);

  public:
    CPurelyTemporalAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalAnalysis();
};
//*****************************************************************************
#endif

