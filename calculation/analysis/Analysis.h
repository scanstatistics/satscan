//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "cluster.h"
#include "TimeEstimate.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "LikelihoodCalculation.h"
#include "MostLikelyClustersContainer.h"

/** Container class to store top clusters for a spatial analysis during
    method - GetTopCluster(). This class stores the top cluster for a
    shape, circle and ellipses, inorder to postpone determination of top
    cluster by adjusted loglikelihood ratio until all possibilities
    have been calculated and ranked by loglikelihood. For most analyses, the
    shape will not be be a factor, but for the Duczmal Compactness correction,
    the top cluster for circles and each ellipse shape will have be retained
    until all other calculations have been completed for each iteration of
    function. */
class TopClustersContainer {
  private:
    const CSaTScanData                & gData;
    ZdPointerVector<CCluster>           gvTopShapeClusters;

  public:
    TopClustersContainer(const CSaTScanData & Data);
    ~TopClustersContainer();

    CCluster                          & GetTopCluster(int iShapeOffset);
    CCluster                          & GetTopCluster();
    void                                Reset(int iCenter);
    void                                SetTopClusters(const CCluster& InitialCluster);
};

class CAnalysis {
  private:
    void                                Init();
    void                                Setup();

  protected:
    const CParameters                 & gParameters;
    const CSaTScanData                & gDataHub;
    BasePrint                         & gPrintDirection;
    AbstractClusterDataFactory        * gpClusterDataFactory;
    AbstractLikelihoodCalculator      * gpLikelihoodCalculator;
    bool                                gbMeasureListReplications;

    void                                AllocateLikelihoodObject();
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) = 0;
    CMeasureList                      * GetNewMeasureListObject() const;
    CTimeIntervals                    * GetNewTimeIntervalsObject(IncludeClustersType eType) const;

  public:
    CAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CAnalysis();

    virtual void                        AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway)  = 0;
    virtual void                        AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) = 0;
    virtual void                        FindTopClusters(const AbtractDataStreamGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double                      FindTopRatio(const AbtractDataStreamGateway & DataGateway);
    bool                                IsMonteCarlo() const {return gbMeasureListReplications;}
    virtual double                      MonteCarlo(const DataStreamInterface & Interface) = 0;
};
//*****************************************************************************
#endif

