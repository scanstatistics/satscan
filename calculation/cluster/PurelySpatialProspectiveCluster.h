//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"

class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    count_t                   * m_pCumCases;
    measure_t                 * m_pCumMeasure;
    int                         m_nTotalIntervals;

    void                        Init() {m_pCumCases=0;m_pCumMeasure=0;}
    void                        Setup();
	
  public:
    CPurelySpatialProspectiveCluster(BasePrint *pPrintDirection, int nTotalIntervals);
    ~CPurelySpatialProspectiveCluster();

    CPurelySpatialProspectiveCluster          & operator =(const CPurelySpatialProspectiveCluster& cluster);

    virtual void                        	AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CPurelySpatialProspectiveCluster  * Clone() const;
    virtual void                        	DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual count_t                     	GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                   	GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        	Initialize(tract_t nCenter);
    virtual void                        	SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    virtual void                        	SetTimeIntervalEndDate(int iTimeIntervalEndDateIndex);    
};
//*****************************************************************************
#endif