//*****************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeTrend.h"

class SVTTClusterStreamData : public ClusterStreamData {
  public:
    SVTTClusterStreamData() : ClusterStreamData() {}
    virtual ~SVTTClusterStreamData() {}

    count_t                   * gpCasesInsideCluster;           /** cases inside the cluster */
    count_t                     gtTotalCasesInsideCluster;      /** total cases inside the cluster */
    measure_t                 * gpMeasureInsideCluster;         /** measure inside the cluster */
    measure_t                   gtTotalMeasureInsideCluster;    /** total measure inside the cluster */
    count_t                   * gpCasesOutsideCluster;          /** cases outside the cluster */
    count_t                     gtTotalCasesOutsideCluster;     /** total cases outside the cluster */
    measure_t                 * gpMeasureOutsideCluster;        /** measure outside the cluster */

    // are these stream specific ?
    CTimeTrend                  gTimeTrendInside;               /** time trend for defined cluster */
    CTimeTrend                  gTimeTrendOutside;              /** Time trend for area outside cluster */

//    virtual void        InitializeData() {gCases=0;gMeasure=0;gSqMeasure=0;}
};

  //std::deque<SVTTClusterStreamData> gStreamData; pass each strea data to poisson calcuclate SVTT

/** cluster class for spatial variation and temporal trends analysis */
class CSVTTCluster : public CCluster  {
  private:
    void                        AllocateArrays();
    void                        DeallocateArrays();
    void                        Init();
    void                        Setup(const DataStreamGateway & DataGateway, int iNumTimeIntervals);
    void                        Setup(const DataStreamInterface & Interface, int iNumTimeIntervals);
    void                        Setup(const CSVTTCluster & rhs);

  public:
    int                         giTotalIntervals;
    unsigned int                giNumDataStream;

    TwoDimCountArray_t        * gpCasesInsideCluster;           /** cases inside the cluster */
    count_t                   * gpTotalCasesInsideCluster;      /** total cases inside the cluster */
    TwoDimMeasureArray_t      * gpMeasureInsideCluster;         /** measure inside the cluster */
    measure_t                 * gpTotalMeasureInsideCluster;    /** total measure inside the cluster */
    TwoDimCountArray_t        * gpCasesOutsideCluster;          /** cases outside the cluster */
    count_t                   * gpTotalCasesOutsideCluster;     /** total cases outside the cluster */
    TwoDimMeasureArray_t      * gpMeasureOutsideCluster;        /** measure outside the cluster */
    CTimeTrend                  gTimeTrendInside;               /** time trend for defined cluster */
    CTimeTrend                  gTimeTrendOutside;              /** Time trend for area outside cluster */

  public:
    CSVTTCluster(const DataStreamGateway & DataGateway, int iNumTimeIntervals, BasePrint *pPrintDirection);
    CSVTTCluster(const DataStreamInterface & Interface, int iNumTimeIntervals, BasePrint *pPrintDirection);
    CSVTTCluster(const CSVTTCluster & rhs);
    ~CSVTTCluster();

    CSVTTCluster              & operator=(const CSVTTCluster& rhs);

    void                        AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway);
    void                        AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream);    
    virtual CSVTTCluster      * Clone() const;
    virtual void                DisplayAnnualTimeTrendWithoutTitle(FILE* fp);
    virtual void                DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType) {};
    virtual void                DisplayTimeTrend(FILE* fp, char* szSpacesOnLeft);
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                InitializeSVTT(tract_t nCenter, const DataStreamGateway & DataGateway);
    virtual void                InitializeSVTT(tract_t nCenter, const DataStreamInterface & Interface);
    virtual void                SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};

#endif