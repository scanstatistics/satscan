//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONECLUSTER_H
#define __PURELYSPATIALMONOTONECLUSTER_H
//*****************************************************************************
#include "PurelySpatialCluster.h"

class CPSMonotoneCluster : public CPurelySpatialCluster
{
  public:

    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, int iRate);
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const DataStreamInterface & Interface, int iRate);
    CPSMonotoneCluster(const CPSMonotoneCluster& rhs);
    ~CPSMonotoneCluster();

    friend class PoissonLikelihoodCalculator;
    friend class BernoulliLikelihoodCalculator;
    
  protected:
    tract_t    m_nMaxCircles;        // Maximum number of circles possible
    count_t*   m_pCasesList;         // Number of cases in each circle
    measure_t* m_pMeasureList;       // Expected count for each circle
    tract_t*   m_pFirstNeighborList; // 1st neighbor in circle
    tract_t*   m_pLastNeighborList;  // Last neighbor in circle
    bool       m_bRatioSet;          // Has the loglikelihood ratio been set?

  public:
    CPSMonotoneCluster& operator =(const CPSMonotoneCluster& cluster);
    virtual CPSMonotoneCluster * Clone() const;

    count_t                     m_nCases;             // Number of cases in cluster
    measure_t                   m_nMeasure;           // Expected count for cluster
    tract_t                     m_nSteps;             // Number of concentric steps in cluster    
    double                      m_nLogLikelihood;     // Log Likelihood

    void                AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    void                AddRemainder(count_t nTotalCases, measure_t nTotalMeasure);
    void                AllocateForMaxCircles(tract_t nCircles);
    inline virtual void AssignAsType(const CCluster& rhs) {*this = (CPSMonotoneCluster&)rhs;}
    void                CheckCircle(tract_t n);
    virtual bool        ClusterDefined() const {return (m_nSteps > 0);};
    virtual void        DefineTopCluster(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, count_t** pCases);
    virtual void        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, measure_t nMinMeasure, const ClusterPrintFormat& PrintFormat) const;
    virtual void        DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const ClusterPrintFormat& PrintFormat) const;
    virtual void        DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const ClusterPrintFormat& PrintFormat) const;
    virtual void        DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment, const ClusterPrintFormat& PrintFormat) const;
    virtual void        DisplaySteps(FILE* fp, const ClusterPrintFormat& PrintFormat) const;
    virtual void        Initialize(tract_t nCenter);
    void                RemoveRemainder();
    virtual AbstractClusterData * GetClusterData();
    virtual int         GetClusterType() const {return PURELYSPATIALMONOTONE;}
    tract_t             GetLastCircleIndex() const {return m_nSteps-1;};
    tract_t             GetNumCircles() const {return m_nSteps;}
    virtual tract_t     GetNumTractsInnerCircle() const { return m_pLastNeighborList[0]; };
    double              GetRelativeRisk(tract_t nStep, double nMeasureAdjustment) const;
    double              GetRatio() const;
    double              GetLogLikelihood() const;
    virtual void        Write(stsAreaSpecificData& AreaData, const CSaTScanData& Data,
                              unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const;

  protected:
    void   SetCasesAndMeasures();
    void   SetTotalTracts();
    double SetLogLikelihood();
    double SetRatio(double nLogLikelihoodForTotal);

    void   ConcatLastCircles();
};

//*****************************************************************************
#endif
