//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONECLUSTER_H
#define __PURELYSPATIALMONOTONECLUSTER_H
//*****************************************************************************
#include "PurelySpatialCluster.h"

/** Purely spatial monotone cluster */
class CPSMonotoneCluster : public CCluster {
    friend class PoissonLikelihoodCalculator;
    friend class BernoulliLikelihoodCalculator;

  protected:
    tract_t                     m_nMaxCircles;        // Maximum number of circles possible
    count_t                   * m_pCasesList;         // Number of cases in each circle
    measure_t                 * m_pMeasureList;       // Expected count for each circle
    tract_t                   * m_pFirstNeighborList; // 1st neighbor in circle
    tract_t                   * m_pLastNeighborList;  // Last neighbor in circle
    bool                        m_bRatioSet;          // Has the loglikelihood ratio been set?
    count_t                     m_nCases;             // Number of cases in cluster
    measure_t                   m_nMeasure;           // Expected count for cluster
    tract_t                     m_nSteps;             // Number of concentric steps in cluster    

    void                        ConcatLastCircles();
    void                        SetCasesAndMeasures();
    double                      SetLogLikelihood();
    double                      SetRatio(double nLogLikelihoodForTotal);
    void                        SetTotalTracts();

  public:
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, int iRate);
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const DataStreamInterface & Interface, int iRate);
    CPSMonotoneCluster(const CPSMonotoneCluster& rhs);
    ~CPSMonotoneCluster();

    CPSMonotoneCluster         & operator=(const CPSMonotoneCluster& rhs);
    virtual CPSMonotoneCluster * Clone() const;

    double                      m_nLogLikelihood;     // Log Likelihood

    void                        AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    void                        AddRemainder(count_t nTotalCases, measure_t nTotalMeasure);
    void                        AllocateForMaxCircles(tract_t nCircles);
    inline virtual void         AssignAsType(const CCluster& rhs) {*this = (CPSMonotoneCluster&)rhs;}
    void                        CheckCircle(tract_t n);
    virtual bool                ClusterDefined() const {return (m_nSteps > 0);}
    virtual void                DefineTopCluster(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, count_t** pCases);
    virtual void                DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, measure_t nMinMeasure, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayRelativeRisk(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplaySteps(FILE* fp, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {}
    virtual void                Initialize(tract_t nCenter=0);
    void                        RemoveRemainder();
    virtual count_t             GetCaseCount(unsigned int iStream) const {return m_nCases;}
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    virtual AbstractClusterData * GetClusterData();
    virtual ClusterType         GetClusterType() const {return PURELYSPATIALMONOTONECLUSTER;}
    virtual ZdString          & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    tract_t                     GetLastCircleIndex() const {return m_nSteps-1;};
    virtual measure_t           GetMeasure(unsigned int iStream) const {return m_nMeasure;}
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    tract_t                     GetNumCircles() const {return m_nSteps;}
    virtual tract_t             GetNumTractsInnerCircle() const {return m_pLastNeighborList[0];}
    double                      GetRelativeRisk(tract_t nStep, double nMeasureAdjustment) const;
    double                      GetRatio() const;
    double                      GetLogLikelihood() const;
    virtual ZdString          & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual void                Write(stsAreaSpecificData& AreaData, const CSaTScanData& Data,
                                      unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const;
};
//*****************************************************************************
#endif
