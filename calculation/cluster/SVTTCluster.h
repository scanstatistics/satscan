//*****************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "TimeTrend.h"

class ClusterSetData {
  private:
    void                                Init() {gTotalCases=0;gTotalMeasure=0;}

  public:
    ClusterSetData() {Init(); InitializeData();}
    virtual ~ClusterSetData() {}

    count_t                             gCases;
    measure_t                           gMeasure;

    count_t                             gTotalCases;
    measure_t                           gTotalMeasure;

    virtual void        InitializeData() {gCases=0;gMeasure=0;}
};

class SVTTClusterSetData : public ClusterSetData {
  private:
     int                        giAllocationSize;

     void                       Init();
     void                       Setup();

  public:
    SVTTClusterSetData(int unsigned iAllocationSize);
    SVTTClusterSetData(const SVTTClusterSetData& rhs);
    virtual ~SVTTClusterSetData();

    SVTTClusterSetData     * Clone() const;
    SVTTClusterSetData     & operator=(const SVTTClusterSetData& rhs);

    count_t                   * gpCasesInsideCluster;           /** cases inside the cluster */
    count_t                     gtTotalCasesInsideCluster;      /** total cases inside the cluster */
    measure_t                 * gpMeasureInsideCluster;         /** measure inside the cluster */
    measure_t                   gtTotalMeasureInsideCluster;    /** total measure inside the cluster */
    count_t                   * gpCasesOutsideCluster;          /** cases outside the cluster */
    count_t                     gtTotalCasesOutsideCluster;     /** total cases outside the cluster */
    measure_t                 * gpMeasureOutsideCluster;        /** measure outside the cluster */
    CTimeTrend                  gTimeTrendInside;               /** time trend for defined cluster */
    CTimeTrend                  gTimeTrendOutside;              /** Time trend for area outside cluster */

    virtual void                InitializeSVTTData(const DataSetInterface & Interface);
};

  //std::deque<SVTTClusterSetData> gSetData; pass each strea data to poisson calcuclate SVTT

/** cluster class for spatial variation and temporal trends analysis */
class CSVTTCluster : public CCluster  {
  typedef std::vector<SVTTClusterSetData>           SetDataContainer_t;
  typedef std::vector<SVTTClusterSetData>::iterator SetDataContainerIterator_t;

  private:
    void                        Init();
    void                        Setup(const AbtractDataSetGateway & DataGateway, int iNumTimeIntervals);
    void                        Setup(const DataSetInterface & Interface, int iNumTimeIntervals);
    void                        Setup(const CSVTTCluster & rhs);

  public:
    int                         giTotalIntervals;
    SetDataContainer_t          gvSetData;
    double                      m_nLogLikelihood;     // Log Likelihood

  public:
    CSVTTCluster(const AbtractDataSetGateway & DataGateway, int iNumTimeIntervals);
    CSVTTCluster(const DataSetInterface & Interface, int iNumTimeIntervals);
    CSVTTCluster(const CSVTTCluster & rhs);
    ~CSVTTCluster();

    CSVTTCluster              & operator=(const CSVTTCluster& rhs);

    void                        AddNeighbor(tract_t tNeighbor, const AbtractDataSetGateway & DataGateway);
    void                        AddNeighbor(tract_t tNeighbor, const DataSetInterface & Interface, size_t tSetIndex);    
    virtual CSVTTCluster      * Clone() const;
    virtual void                DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {};
    virtual void                DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const;
    virtual count_t             GetCaseCount(size_t tSetIndex) const {return gvSetData[tSetIndex].gtTotalCasesInsideCluster;}
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual AbstractClusterData * GetClusterData();
    virtual const AbstractClusterData * GetClusterData() const;
    virtual ClusterType         GetClusterType() const {return SPATIALVARTEMPTRENDCLUSTER;}
    virtual ZdString          & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t           GetMeasure(size_t tSetIndex) const {return gvSetData[tSetIndex].gtTotalMeasureInsideCluster;}
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual ZdString          & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    SVTTClusterSetData        & GetDataSet(size_t tSetIndex) {return gvSetData[tSetIndex];}
    virtual void                InitializeSVTT(tract_t nCenter, const AbtractDataSetGateway & DataGateway);
    virtual void                InitializeSVTT(tract_t nCenter, const DataSetInterface & Interface);
    void                        SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen);
};

#endif
