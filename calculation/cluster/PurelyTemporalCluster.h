//*****************************************************************************
#ifndef __PURELYTTEMPORALCLUSTER_H
#define __PURELYTTEMPORALCLUSTER_H
//*****************************************************************************
#include "Tracts.h"
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"
#include "Parameters.h"

#include "TimeIntervalRange.h"

class CPurelyTemporalCluster : public CCluster {
  private:
    void                                Init() {m_TI=0;}
    void                                Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    void                                Setup(const CPurelyTemporalCluster& rhs);

  protected:
    int                                 m_nTotalIntervals;
    int                                 m_nIntervalCut;
    IncludeClustersType                 m_nTIType;
    CTimeIntervals                    * m_TI;

  public:
    CPurelyTemporalCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection);
    CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs);
    virtual ~CPurelyTemporalCluster();

    CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& cluster);

    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelyTemporalCluster&)rhs;}
    virtual CPurelyTemporalCluster    * Clone() const;
    inline virtual void                 CompareTopCluster(CPurelyTemporalCluster & TopShapeCluster, const CSaTScanData & Data);
    inline virtual void                 ComputeBestMeasures(const count_t* pCases, const measure_t* pMeasure, CMeasureList & MeasureList);
    virtual void                        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                                            int nCluster,  measure_t nMinMeasure,
                                                            int iNumSimulations, long lReportHistoryRunNumber,
                                                            bool bIncludeRelRisk, bool bIncludePVal,
                                                            int nLeftMargin, int nRightMargin,
                                                            char cDeliminator, char* szSpacesOnLeft, bool bFormat = true);
    virtual void                        DisplayCoordinates(FILE* fp, const CSaTScanData& Data, int nLeftMargin,
                                                           int nRightMargin, char cDeliminator, char* szSpacesOnLeft) {};
    virtual void                        DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {};
    virtual count_t                     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter);
};
//*****************************************************************************
#endif
