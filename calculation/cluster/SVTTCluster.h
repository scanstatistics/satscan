//*****************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "TimeTrend.h"

class ClusterStreamData {
  private:
    void                                Init() {gTotalCases=0;gTotalMeasure=0;}

  public:
    ClusterStreamData() {Init(); InitializeData();}
    virtual ~ClusterStreamData() {}

    count_t                             gCases;
    measure_t                           gMeasure;

    count_t                             gTotalCases;
    measure_t                           gTotalMeasure;

    virtual void        InitializeData() {gCases=0;gMeasure=0;}
};

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
    void                        Setup(const AbtractDataStreamGateway & DataGateway, int iNumTimeIntervals);
    void                        Setup(const DataStreamInterface & Interface, int iNumTimeIntervals);
    void                        Setup(const CSVTTCluster & rhs);

  public:
    int                         giTotalIntervals;
    StreamDataContainer_t       gvStreamData;
    double                      m_nLogLikelihood;     // Log Likelihood

  public:
    CSVTTCluster(const AbtractDataStreamGateway & DataGateway, int iNumTimeIntervals);
    CSVTTCluster(const DataStreamInterface & Interface, int iNumTimeIntervals);
    CSVTTCluster(const CSVTTCluster & rhs);
    ~CSVTTCluster();

    CSVTTCluster              & operator=(const CSVTTCluster& rhs);

    void                        AddNeighbor(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway);
    void                        AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream);    
    virtual CSVTTCluster      * Clone() const;
    virtual void                DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {};
    virtual void                DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const;
    virtual count_t             GetCaseCount(unsigned int iStream) const {return gvStreamData[iStream].gtTotalCasesInsideCluster;}
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    virtual AbstractClusterData * GetClusterData();
    virtual const AbstractClusterData * GetClusterData() const;
    virtual ClusterType         GetClusterType() const {return SPATIALVARTEMPTRENDCLUSTER;}
    virtual ZdString          & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t           GetMeasure(unsigned int iStream) const {return gvStreamData[iStream].gtTotalMeasureInsideCluster;}
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    virtual ZdString          & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    SVTTClusterStreamData     & GetStream(unsigned int tStream) {return gvStreamData[tStream];}
    virtual void                InitializeSVTT(tract_t nCenter, const AbtractDataStreamGateway & DataGateway);
    virtual void                InitializeSVTT(tract_t nCenter, const DataStreamInterface & Interface);
    void                        SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen);
};

#endif
