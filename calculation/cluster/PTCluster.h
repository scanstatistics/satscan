// PTCluster.h

#ifndef __PTCLUSTER_H
#define __PTCLUSTER_H

#include "Cluster.h"
#include "TimeIntervals.h"

class CPurelyTemporalCluster : public CCluster
{
  public:
    CPurelyTemporalCluster(int nTIType, int nIntervals, int nMaxIntervals);
    virtual ~CPurelyTemporalCluster();

    CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& cluster);
    virtual void Initialize(tract_t nCenter);

    virtual void InitTimeIntervalIndeces();

    virtual bool SetNextTimeInterval(const count_t*& pCases,
                                     const measure_t*& pMeasure);

    virtual void DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {};
    virtual void DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                    int nLeftMargin, int nRightMargin,
                                    char cDeliminator, char* szSpacesOnLeft) {};
    virtual void DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                     int nCluster,  measure_t nMinMeasure,
                                     int nReplicas,
                                     bool bIncludeRelRisk, bool bIncludePVal,
                                     int nLeftMargin, int nRightMargin,
                                     char cDeliminator, char* szSpacesOnLeft);
  protected:
    int m_nTotalIntervals;
    int m_nIntervalCut;

    CTimeIntervals* m_TI;

};

#endif
