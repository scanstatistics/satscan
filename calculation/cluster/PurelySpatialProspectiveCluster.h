//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"

class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    void                        Init() {m_pCumulatedCases=0;m_pCumulatedMeasure=0;}
    void                        Setup();

  protected:
    count_t                   * m_pCumulatedCases;
    measure_t                 * m_pCumulatedMeasure;
    int                         m_nTotalIntervals;

  public:
    CPurelySpatialProspectiveCluster(BasePrint *pPrintDirection, int nTotalIntervals);
    virtual ~CPurelySpatialProspectiveCluster();

    CPurelySpatialProspectiveCluster          & operator =(const CPurelySpatialProspectiveCluster& cluster);

    virtual void                        	AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CPurelySpatialProspectiveCluster  * Clone() const;
    virtual void                        	DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual count_t                     	GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                   	GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        	Initialize(tract_t nCenter);
    virtual void                        	SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    void                        	        SetForProspectiveEndDate(int iProspectiveEndDateIndex);    
};
//*****************************************************************************
#endif