// analysis.h

#ifndef __ANALYSIS_H
#define __ANALYSIS_H

#include "param.h"
#include "data.h"
//#include "model.h"
#include "cluster.h"
#include "SigRatios05.h"

#define  DEBUGANALYSIS 1

class CAnalysis
{
  public:
    CAnalysis(CParameters* pParameters, CSaTScanData* pData);
    virtual ~CAnalysis();

  protected:
    CParameters*  m_pParameters;
    CSaTScanData* m_pData;
//    CModel*       m_pModel;

    int     m_nAnalysisCount;
    tract_t m_nClustersToKeepEachPass;

    CCluster** m_pTopClusters;
    tract_t    m_nMaxClusters;
    tract_t    m_nClustersRetained;
    tract_t    m_nClustersReported;
    double     m_nMinRatioToReport;

    CSignificantRatios05 SimRatios;

    int m_nPower_X_Count;
    int m_nPower_Y_Count;

    FILE* m_pDebugFile;

  public:
    bool Execute(time_t RunTime);

    void DisplayFindClusterHeading();
    void DisplayTopClusterLogLikelihood(FILE* fp);
    void DisplayTopClustersLogLikelihoods(FILE* fp);

    virtual void DisplayTopClusters(double nMinRatio, int nReps,
                                    FILE* fp=stdout, FILE* fpGIS=NULL);
    virtual void DisplayTopCluster(double nMinRatio, int nReps,
                                   FILE* fp=stdout, FILE* fpGIS=NULL);

    virtual CCluster* GetTopCluster(tract_t nCenter) = 0;

  protected:
    bool RepeatAnalysis();
    void RemoveTopClusterData();

    virtual void SetMaxNumClusters() {m_nMaxClusters=m_pData->m_nGridTracts;};
    tract_t      GetMaxNumClusters() {return m_nMaxClusters;};

    virtual void AllocateTopClusterList();
    void         InitializeTopClusterList();

    virtual void FindTopClusters();
    virtual void PerformSimulations();
//    virtual void MakeData();
    virtual double MonteCarlo() = 0;

    void SortTopClusters();
    static int CompareClusters(const void *a, const void *b);

    virtual void RankTopClusters();
    void UpdateTopClustersRank(double r);
    void UpdatePowerCounts(double r);

    void OpenReportFile(FILE*& fp, const char* szType);
    virtual void OpenGISFile(FILE*& fpGIS, const char* szType);
// Start V.2.0.4.1
    void OpenLLRFile(FILE*& fpLLR, const char* szType);
// End V.2.0.4.1

    bool CreateReport(time_t RunTime);
    bool UpdateReport();
    bool FinalizeReport(time_t RunTime);

};

#endif
