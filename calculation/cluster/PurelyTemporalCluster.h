//*****************************************************************************
#ifndef __PURELYTTEMPORALCLUSTER_H
#define __PURELYTTEMPORALCLUSTER_H
//*****************************************************************************
#include "Tracts.h"
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"
#include "Parameters.h"


class CPurelyTemporalCluster : public CCluster
{
  protected:
    int m_nTotalIntervals;
    int m_nIntervalCut;

    CTimeIntervals* m_TI;

  public:
    CPurelyTemporalCluster(int nTIType, int nIntervals, int nMaxIntervals, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalCluster();

    CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& cluster);

    virtual void        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                            int nCluster,  measure_t nMinMeasure,
                                     int nReplicas, long lReportHistoryRunNumber,
                                            bool bIncludeRelRisk, bool bIncludePVal,
                                            int nLeftMargin, int nRightMargin,
                                            char cDeliminator, char* szSpacesOnLeft,
                                            bool bFormat = true);
    virtual void        DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                           int nLeftMargin, int nRightMargin,
                                           char cDeliminator, char* szSpacesOnLeft) {};
    virtual void        DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {};
    virtual count_t     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void        Initialize(tract_t nCenter);
    virtual void        InitTimeIntervalIndeces();
    inline virtual bool SetNextTimeInterval(const count_t*& pCases,
                                            const measure_t*& pMeasure);
};
//*****************************************************************************
#endif
