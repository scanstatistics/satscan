//******************************************************************************
#ifndef __PURELYTEMPORALANALYSIS_H
#define __PURELYTEMPORALANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "ClusterData.h"
#include "MeasureList.h"
#include "TimeIntervalRange.h"
#include "PurelyTemporalData.h"
#include "PurelyTemporalCluster.h"

/** Derives from base class to re-define methods for a purely temporal analysis. */
class CPurelyTemporalAnalysis : public CAnalysis {
  private:
    CPurelyTemporalCluster      * gpTopCluster;        /** cluster object utilized in calculating simulation llr */
    CPurelyTemporalCluster      * gpClusterComparator; /** cluster object utilized in calculating simulation llr */
    TemporalData                * gpClusterData;       /** cluster data object utilized in calculating simulation llr */
    CMeasureList                * gpMeasureList;       /** measure list object utilized in calculating simulation llr */
    CTimeIntervals              * gpTimeIntervals;     /** iterates through temporal windows of cluster data */

    void                          Init();
    void                          Setup();

  protected:
    virtual void                  AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
    virtual void                  AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway);
    virtual const CCluster     &  CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway);

  public:
    CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPurelyTemporalAnalysis();

    virtual void                  FindTopClusters(const AbstractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double                FindTopRatio(const AbstractDataSetGateway& DataGateway);
    virtual double                MonteCarlo(const DataSetInterface& Interface);
};
//******************************************************************************
#endif

