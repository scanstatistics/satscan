//*****************************************************************************
#ifndef __ANALYSIS_H
#define __ANALYSIS_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeEstimate.h"
#include "SignificantRatios05.h"

//#define  DEBUGANALYSIS 0

class CAnalysis
{
  protected:
    CParameters*                m_pParameters;
    CSaTScanData*               m_pData;
    BasePrint   *               gpPrintDirection;
    CSignificantRatios05        SimRatios;

    CCluster**                  m_pTopClusters;
    tract_t                     m_nMaxClusters, m_nClustersToKeepEachPass, m_nClustersRetained, m_nClustersReported;
    double                      m_nMinRatioToReport;
    int                         m_nAnalysisCount, m_nPower_X_Count, m_nPower_Y_Count;
    ZdString                    gsStartTime;
    FILE*                       m_pDebugFile;
    unsigned short              guwSignificantAt005;

    virtual void                AllocateTopClusterList();
    static int                  CompareClusters(const void *a, const void *b);
    void                        CreateGridOutputFile();
    bool                        CreateReport(time_t RunTime);
    bool                        FinalizeReport(time_t RunTime);
    virtual bool                FindTopClusters();
    tract_t                     GetMaxNumClusters() {return m_nMaxClusters;};      // why have a protected function to get a protected variable? AJV
    void                        InitializeTopClusterList();

    virtual double              MonteCarlo() = 0;
    virtual double              MonteCarloProspective() = 0;
    virtual void                OpenGISFile(FILE*& fpGIS, const char* szType);
    void                        OpenGridOutputFile(FILE*& fpMLC, const char* szType);
    void                        OpenLLRFile(FILE*& fpLLR, const char* szType);    //  V.2.0.4.1
    void                        OpenReportFile(FILE*& fp, const char* szType);
    void                        OpenRREFile(FILE*& fpRRE, const char* szType);
    virtual void                PerformSimulations();
    void                        PrintTopClusters(int nHowMany);
    virtual void                RankTopClusters();  
    void                        RemoveTopClusterData();
    bool                        RepeatAnalysis();
    virtual void                SetMaxNumClusters() {m_nMaxClusters=m_pData->m_nGridTracts;};
    void                        SortTopClusters();
    void                        UpdatePowerCounts(double r);
    bool                        UpdateReport(const ZdString& sReportHistoryFilename);
    void                        UpdateTopClustersRank(double r);

//    CModel*       m_pModel;
//    virtual void MakeData();
    //void ObtainEllipsoidSettings();

  public:
    CAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CAnalysis();

    void                        DisplayFindClusterHeading();
    void                        DisplayTopClusterLogLikelihood(FILE* fp);
    void                        DisplayTopClustersLogLikelihoods(FILE* fp);
    virtual void                DisplayTopClusters(double nMinRatio, int nReps, FILE* fp=stdout, FILE* fpGIS=NULL, const ZdString& sReportHistoryFilename = "");
    virtual void                DisplayTopCluster(double nMinRatio, int nReps, FILE* fp=stdout, FILE* fpGIS=NULL, const ZdString& sReportHistoryFilename = "");

    bool                        Execute(time_t RunTime);

    const CSaTScanData*         GetSatScanData() const;
    const double&               GetSimRatio01() const;
    const double&               GetSimRatio05() const;
    const ZdString&             GetStartTime() const;
    virtual CCluster*           GetTopCluster(tract_t nCenter) = 0;
    const int&                  GetCoordinateType() const;

};
//*****************************************************************************
#endif
