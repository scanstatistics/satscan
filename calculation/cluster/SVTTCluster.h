//*****************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeTrend.h"

class SVTTClusterStreamData : public ClusterStreamData {
  private:
     int                        giAllocationSize;

     void                       Init();
     void                       Setup();

  public:
    SVTTClusterStreamData(int unsigned iAllocationSize);
    SVTTClusterStreamData(const SVTTClusterStreamData& rhs);
    virtual ~SVTTClusterStreamData();

    SVTTClusterStreamData     * Clone() const;
    SVTTClusterStreamData     & operator=(const SVTTClusterStreamData& rhs);

    count_t                   * gpCasesInsideCluster;           /** cases inside the cluster */
    count_t                     gtTotalCasesInsideCluster;      /** total cases inside the cluster */
    measure_t                 * gpMeasureInsideCluster;         /** measure inside the cluster */
    measure_t                   gtTotalMeasureInsideCluster;    /** total measure inside the cluster */
    count_t                   * gpCasesOutsideCluster;          /** cases outside the cluster */
    count_t                     gtTotalCasesOutsideCluster;     /** total cases outside the cluster */
    measure_t                 * gpMeasureOutsideCluster;        /** measure outside the cluster */
    CTimeTrend                  gTimeTrendInside;               /** time trend for defined cluster */
    CTimeTrend                  gTimeTrendOutside;              /** Time trend for area outside cluster */

    virtual void                InitializeSVTTData(const DataStreamInterface & Interface);
};

  //std::deque<SVTTClusterStreamData> gStreamData; pass each strea data to poisson calcuclate SVTT

/** cluster class for spatial variation and temporal trends analysis */
class CSVTTCluster : public CCluster  {
  typedef std::vector<SVTTClusterStreamData>           StreamDataContainer_t;
  typedef std::vector<SVTTClusterStreamData>::iterator StreamDataContainerIterator_t;

  private:
    void                        Init();
    void                        Setup(const DataStreamGateway & DataGateway, int iNumTimeIntervals);
    void                        Setup(const DataStreamInterface & Interface, int iNumTimeIntervals);
    void                        Setup(const CSVTTCluster & rhs);

  public:
    int                         giTotalIntervals;
    StreamDataContainer_t       gvStreamData;

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
    SVTTClusterStreamData     & GetStream(unsigned int tStream) {return gvStreamData[tStream];}
    virtual void                InitializeSVTT(tract_t nCenter, const DataStreamGateway & DataGateway);
    virtual void                InitializeSVTT(tract_t nCenter, const DataStreamInterface & Interface);
    virtual void                SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    void                        SetTimeTrend(int nIntervalUnits, double nIntervalLen);
};

#endif