//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "cluster.h"
#include "TimeEstimate.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"
#include "LikelihoodCalculation.h"
#include "MostLikelyClustersContainer.h"

/** Container class to store top clusters for a spatial analysis during
    method - GetTopCluster(). This class stores the top cluster for a
    shape, circle and ellipses, inorder to postpone determination of top
    cluster by adjusted loglikelihood ratio until all possibilities
    have been calculated and ranked by loglikelihood. For most analyses, the
    shape will not be be a factor, but when penalizing for non-compactness,
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

/** Abstract base class which defines methods for calculating top clusters and
    simulated log likelihood ratios. */
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
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway & DataGateway) = 0;
    CMeasureList                      * GetNewMeasureListObject() const;
    CTimeIntervals                    * GetNewTemporalDataEvaluatorObject(IncludeClustersType eType) const;

  public:
    CAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CAnalysis();

    virtual void                        AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway)  = 0;
    virtual void                        AllocateTopClustersObjects(const AbtractDataSetGateway & DataGateway) = 0;
    double                              ExecuteSimulation(const AbtractDataSetGateway& DataGateway);
    virtual void                        FindTopClusters(const AbtractDataSetGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double                      FindTopRatio(const AbtractDataSetGateway & DataGateway);
    virtual double                      MonteCarlo(const DataSetInterface & Interface) = 0;
};
//*****************************************************************************
#endif

