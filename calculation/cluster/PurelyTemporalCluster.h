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
  public:
    CPurelyTemporalCluster(int nTIType, int nIntervals, int nMaxIntervals, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalCluster();

    CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& cluster);
    virtual void Initialize(tract_t nCenter);

    virtual void InitTimeIntervalIndeces();

    inline virtual bool SetNextTimeInterval(const count_t*& pCases,
                                     const measure_t*& pMeasure);

    virtual void DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {};
    virtual void DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                    int nLeftMargin, int nRightMargin,
                                    char cDeliminator, char* szSpacesOnLeft) {};
    virtual void DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                     int nCluster,  measure_t nMinMeasure,
                                     int nReplicas, long lReportHistoryRunNumber,
                                     bool bIncludeRelRisk, bool bIncludePVal,
                                     int nLeftMargin, int nRightMargin,
                                     char cDeliminator, char* szSpacesOnLeft,
                                     bool bFormat = true);
  protected:
    int m_nTotalIntervals;
    int m_nIntervalCut;

    CTimeIntervals* m_TI;

};
//*****************************************************************************
#endif
