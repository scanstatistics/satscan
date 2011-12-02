//******************************************************************************
#ifndef __SPACETIMEANALYSIS_H
#define __SPACETIMEANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "SpaceTimeCluster.h"
#include "PurelyTemporalCluster.h"
#include "MeasureList.h"
#include "IntermediateClustersContainer.h"

/** forward class declarations */
class SpaceTimeData;

/** Derives from base class to re-define methods for a space-time analysis. */
class CSpaceTimeAnalysis : public CAnalysis {
  protected:
    std::auto_ptr<CSpaceTimeCluster>            gClusterComparator;   /** cluster object utilized to find top cluster */
    std::auto_ptr<AbstractTemporalClusterData>  gAbstractClusterData;
    std::auto_ptr<CMeasureList>                 gMeasureList;         /** measure list object utilized in calculated simulation ratio */
    std::auto_ptr<CTimeIntervals>               gTimeIntervals;       /** iterates through temporal windows of cluster data */
    CClusterSetCollections                      _topClusters;         /** collection of clusters representing top cluster for each shape */

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway);
    virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
    virtual double                      MonteCarlo(const DataSetInterface& Interface);
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpaceTimeAnalysis();
};
//******************************************************************************
#endif
