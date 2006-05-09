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
    std::auto_ptr<CMeasureList>                gMeasureList;   /** measure list object utilized in calculating simulation llr */
    std::auto_ptr<CTimeIntervals>              gTimeIntervals; /** iterates through temporal windows of cluster data */
    std::auto_ptr<AbstractTemporalClusterData> gClusterData;

    void                          Init();
    void                          Setup();

  protected:
    virtual void                  AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
    virtual void                  AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {/*nop*/}
    virtual const CCluster     &  CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway);
    virtual double                MonteCarlo(const DataSetInterface& Interface);
    virtual double                MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPurelyTemporalAnalysis();

    virtual void                  FindTopClusters(const AbstractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer);
};
//******************************************************************************
#endif

