//******************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//******************************************************************************
#include "cluster.h"
#include "TimeTrend.h"
#include "ptr_vector.h"

/** Abstract interface for spatial variation of temporal trend cluster data. */
class AbtractSVTTClusterData : public AbstractClusterData {
   protected:
     EvaluationAssistDataStatus geEvaluationAssistDataStatus;
   
   public:
     AbtractSVTTClusterData() : geEvaluationAssistDataStatus(Allocated) {}

     virtual AbtractSVTTClusterData * CloneSVTT() const = 0;
     virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0) = 0;
     virtual double              CalculateSVTTLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator, const AbstractDataSetGateway& DataGateway) = 0;
     virtual void                InitializeSVTTData(const DataSetInterface & Interface) = 0;
     virtual void                InitializeSVTTData(const AbstractDataSetGateway& DataGateway) = 0;
     virtual void                InitializeData(){/* nop */}
     virtual AbstractTimeTrend        & getInsideTrend(size_t tSetIndex=0) = 0;
     virtual AbstractTimeTrend        & getOutsideTrend(size_t tSetIndex=0) = 0;
     virtual const AbstractTimeTrend  & getInsideTrend(size_t tSetIndex=0) const = 0;
     virtual const AbstractTimeTrend  & getOutsideTrend(size_t tSetIndex=0) const = 0;
};

/** Spatial variation of temporal trend cluster data. */
class SVTTClusterData : public AbtractSVTTClusterData {
  private:
     int                        giAllocationSize;

     void                       Init();
     void                       Setup(const DataSetInterface& Interface);

  public:
    SVTTClusterData(const AbstractDataSetGateway& DataGateway);
    SVTTClusterData(const DataSetInterface& Interface);
    SVTTClusterData(const SVTTClusterData& rhs);
    virtual ~SVTTClusterData();

    SVTTClusterData           * Clone() const;
    SVTTClusterData           & operator=(const SVTTClusterData& rhs);

    count_t                   * gpCasesInsideCluster;           /** cases inside the cluster */
    count_t                     gtTotalCasesInsideCluster;      /** total cases inside the cluster */
    measure_t                 * gpMeasureInsideCluster;         /** measure inside the cluster */
    measure_t                   gtTotalMeasureInsideCluster;    /** total measure inside the cluster */
    count_t                   * gpCasesOutsideCluster;          /** cases outside the cluster */
    count_t                     gtTotalCasesOutsideCluster;     /** total cases outside the cluster */
    measure_t                 * gpMeasureOutsideCluster;        /** measure outside the cluster */
    measure_t                   gtTotalMeasureOutsideCluster;   /** total measure outside the cluster */
    AbstractTimeTrend         * gpTimeTrendInside;               /** time trend for defined cluster */
    AbstractTimeTrend         * gpTimeTrendOutside;              /** Time trend for area outside cluster */

    virtual SVTTClusterData * CloneSVTT() const {return new SVTTClusterData(*this);}

    virtual void                InitializeSVTTData(const DataSetInterface & Interface);
    virtual void                InitializeSVTTData(const AbstractDataSetGateway& DataGateway);

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double              CalculateSVTTLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator, const AbstractDataSetGateway& DataGateway);
    virtual void                CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void                DeallocateEvaluationAssistClassMembers();
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void                setCaseCount(count_t t, unsigned int tSetIndex=0) {gtTotalCasesInsideCluster = t;}
    virtual AbstractTimeTrend        & getInsideTrend(size_t tSetIndex=0) {return *gpTimeTrendInside;}
    virtual AbstractTimeTrend        & getOutsideTrend(size_t tSetIndex=0) {return *gpTimeTrendOutside;}
    virtual const AbstractTimeTrend  & getInsideTrend(size_t tSetIndex=0) const {return *gpTimeTrendInside;}
    virtual const AbstractTimeTrend  & getOutsideTrend(size_t tSetIndex=0) const {return *gpTimeTrendOutside;}
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                setMeasure(measure_t m, unsigned int tSetIndex=0) {gtTotalMeasureInsideCluster = m;}
};

/** Class representing accumulated data of spatial variation of temporal trend in multiple data sets.
    NOTE: Multi-set svtt cluster data is a work in progress. Large */
class MultiSetSVTTClusterData : public AbtractSVTTClusterData {
  protected:
    ptr_vector<SVTTClusterData>           gvSetClusterData;

  public:
    MultiSetSVTTClusterData(const AbstractDataSetGateway& DataGateway);
    virtual ~MultiSetSVTTClusterData();

    virtual MultiSetSVTTClusterData * CloneSVTT() const {return new MultiSetSVTTClusterData(*this);}

    virtual void                InitializeSVTTData(const DataSetInterface & Interface);
    virtual void                InitializeSVTTData(const AbstractDataSetGateway& DataGateway);

    virtual void                AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex=0);
    virtual double              CalculateSVTTLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator, const AbstractDataSetGateway& DataGateway);    
    virtual void                CopyEssentialClassMembers(const AbstractClusterData& rhs);
    virtual void                DeallocateEvaluationAssistClassMembers();
    virtual MultiSetSVTTClusterData * Clone() const;
    virtual count_t             GetCaseCount(unsigned int tSetIndex=0) const;
    virtual void                setCaseCount(count_t t, unsigned int tSetIndex=0);
    virtual AbstractTimeTrend        & getInsideTrend(size_t tSetIndex=0);
    virtual AbstractTimeTrend        & getOutsideTrend(size_t tSetIndex=0);
    virtual const AbstractTimeTrend  & getInsideTrend(size_t tSetIndex=0) const;
    virtual const AbstractTimeTrend  & getOutsideTrend(size_t tSetIndex=0) const;
    virtual void                GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                                  AbstractLikelihoodCalculator& Calculator,
                                                                  std::vector<unsigned int>& vDataSetIndexes) const;
    virtual measure_t           GetMeasure(unsigned int tSetIndex=0) const;
    virtual void                setMeasure(measure_t m, unsigned int tSetIndex=0);
    virtual size_t              getNumSets() const {return gvSetClusterData.size();}
};

class CClusterSet;

/** cluster class for spatial variation and temporal trends analysis */
class CSVTTCluster : public CCluster  {
  public:
    std::auto_ptr<AbtractSVTTClusterData> gClusterData;

  public:
    CSVTTCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway);
    CSVTTCluster(const CSVTTCluster & rhs);
    virtual ~CSVTTCluster();

    CSVTTCluster              & operator=(const CSVTTCluster& rhs);
    virtual void                CopyEssentialClassMembers(const CCluster& rhs) {*this = (CSVTTCluster&)rhs;}


    void                        AddNeighbor(tract_t tNeighbor, const AbstractDataSetGateway & DataGateway);
    void                        CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                           const CentroidNeighbors& CentroidDef,
                                                                           CClusterSet& clusterSet,
                                                                           AbstractLikelihoodCalculator& Calculator);
    virtual CSVTTCluster      * Clone() const;
    virtual void                DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {};
    virtual void                DisplayTimeTrend(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual AbstractClusterData * GetClusterData();
    virtual const AbstractClusterData * GetClusterData() const;
    virtual ClusterType         GetClusterType() const {return SPATIALVARTEMPTRENDCLUSTER;}
    virtual std::string       & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t           GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual measure_t           GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    std::string               & GetFormattedTimeTrend(std::string& buffer, const AbstractTimeTrend& Trend) const;
    virtual count_t             GetObservedCount(size_t tSetIndex=0) const {return gClusterData->GetCaseCount(tSetIndex);}
    virtual count_t             GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual std::string       & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual void                InitializeSVTT(tract_t nCenter, const AbstractDataSetGateway & DataGateway);
    virtual void                InitializeSVTT(tract_t nCenter, const DataSetInterface & Interface);
    void                        SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen);
};
//******************************************************************************
#endif

