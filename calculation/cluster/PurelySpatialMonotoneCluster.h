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
    std::vector<count_t>        gvCasesList;          // Number of cases in each circle
    std::vector<measure_t>      gvMeasureList;        // Expected count for each circle
    std::vector<tract_t>        gvFirstNeighborList;  // 1st neighbor in circle
    std::vector<tract_t>        gvLastNeighborList;   // Last neighbor in circle
    bool                        m_bRatioSet;          // Has the loglikelihood ratio been set?
    count_t                     m_nCases;             // Number of cases in cluster
    measure_t                   m_nMeasure;           // Expected count for cluster
    tract_t                     m_nSteps;             // Number of concentric steps in cluster    

    void                        ConcatLastCircles();
    void                        RemoveRemainder();
    void                        SetCasesAndMeasures();
    double                      SetLogLikelihood();
    double                      SetRatio(double nLogLikelihoodForTotal);
    void                        SetTotalTracts();

  public:
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, int iRate);
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const DataSetInterface & Interface, int iRate);
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
    virtual void                DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {}
    virtual void                Initialize(tract_t nCenter=0);
    virtual AbstractClusterData * GetClusterData();
    virtual const AbstractClusterData * GetClusterData() const;
    virtual ClusterType         GetClusterType() const {return PURELYSPATIALMONOTONECLUSTER;}
    virtual ZdString          & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t           GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual measure_t           GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    tract_t                     GetLastCircleIndex() const {return m_nSteps-1;};
    tract_t                     GetNumCircles() const {return m_nSteps;}
    virtual count_t             GetObservedCount(size_t tSetIndex=0) const {return m_nCases;}
    virtual count_t             GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    double                      GetRelativeRisk(tract_t nStep, const CSaTScanData& DataHub) const;
    double                      GetRatio() const;
    double                      GetLogLikelihood() const;
    virtual ZdString          & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual void                PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const;
    virtual void                Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data,
                                      unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const;
};
//*****************************************************************************
#endif
