//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "Cluster.h"
#include "DataStream.h"
#include "TimeEstimate.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "SignificantRatios05.h"

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
    const CSaTScanData        & gData;
    ZdPointerVector<CCluster>   gvTopShapeClusters;

  public:
    TopClustersContainer(const CSaTScanData & Data);
    ~TopClustersContainer();

    CCluster                  & GetTopCluster(int iShapeOffset);
    CCluster                  & GetTopCluster();
    void                        Reset(int iCenter);
    void                        SetTopClusters(const CCluster& InitialCluster);
};

class CAnalysis {
  private:
    double                      GetMonteCarloLoglikelihoodRatio(const AbtractDataStreamGateway & DataGateway);
    void                        Init();
    virtual void                SetMaxNumClusters() {m_nMaxClusters = m_pData->m_nGridTracts;}
    void                        Setup();

  protected:
    CParameters*                m_pParameters;
    CSaTScanData*               m_pData;
    BasePrint   *               gpPrintDirection;
    CSignificantRatios05        SimRatios;
    CCluster**                  m_pTopClusters;
    tract_t                     m_nMaxClusters, m_nClustersToKeepEachPass, m_nClustersRetained, m_nClustersReported;
    double                      m_nMinRatioToReport;
    int                         m_nAnalysisCount, m_nPower_X_Count, m_nPower_Y_Count;
    int                         giSimulationNumber;
    ZdString                    gsStartTime;
    FILE*                       m_pDebugFile;
    unsigned short              guwSignificantAt005;
    AbstractClusterDataFactory* gpClusterDataFactory;
    bool                        gbMeasureListReplications;

    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway)  = 0;
    virtual void                AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) = 0;
    virtual bool                CalculateMostLikelyClusters();
    virtual const CCluster    & CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) = 0;
    virtual bool                FindTopClusters(const AbtractDataStreamGateway & DataGateway);
    virtual double              FindTopRatio(const AbtractDataStreamGateway & DataGateway); //MonteCarlo_BruteForce


    virtual void                AllocateTopClusterList();
    static int                  CompareClustersByRatio(const void *a, const void *b);
    bool                        CheckForEarlyTermination(int iSimulation) const;
    void                        CreateGridOutputFile(const long lReportHistoryRunNumber);
    bool                        CreateReport(time_t RunTime);
    bool                        FinalizeReport(time_t RunTime);
    tract_t                     GetMaxNumClusters() {return m_nMaxClusters;}
    CMeasureList              * GetNewMeasureListObject() const;
    CTimeIntervals            * GetNewTimeIntervalsObject(IncludeClustersType eType) const;
    void                        InitializeTopClusterList();
    virtual double              MonteCarlo(const DataStreamInterface & Interface) = 0;
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface) = 0;
    void                        OpenReportFile(FILE*& fp, const char* szType);
    virtual void                PerformSimulations();
    void                        PrintTopClusters(int nHowMany);
    virtual void                RankTopClusters();
    void                        RemoveTopClusterData();
    bool                        RepeatAnalysis();
    void                        SortTopClusters();
    void                        UpdatePowerCounts(double r);
    bool                        UpdateReport(const long lReportHistoryRunNumber);
    void                        UpdateTopClustersRank(double r);

  public:
    CAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CAnalysis();

    void                        DisplayFindClusterHeading();
    void                        DisplayTopClusterLogLikelihood();
    void                        DisplayTopClustersLogLikelihoods(FILE* fp);
    virtual void                DisplayTopClusters(double nMinRatio, const long lReportHistoryRunNumber, FILE* fp=stdout);
    virtual void                DisplayTopCluster(double nMinRatio, const long lReportHistoryRunNumber, FILE* fp=stdout);
    bool                        Execute(time_t RunTime);
    const int                   GetNumSimulationsExecuted() const {return giSimulationNumber;}
    const CSaTScanData*         GetSatScanData() const { return m_pData; }
    const double                GetSimRatio01() const { return SimRatios.GetAlpha01(); }
    const double                GetSimRatio05() const { return SimRatios.GetAlpha05(); }
    const ZdString&             GetStartTime() const { return gsStartTime; }
};
//*****************************************************************************
#endif
