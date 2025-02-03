//******************************************************************************
#ifndef __DataSet_H
#define __DataSet_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "MultipleDimensionArrayHandler.h"
#include "PopulationData.h"
#include "TimeTrend.h"
#include "ptr_vector.h"
#include "boost/date_time/gregorian/gregorian.hpp"

//typedefs for multiple dimension arrays
typedef TwoDimensionArrayHandler<count_t>        TwoDimCountArray_t;
typedef ThreeDimensionArrayHandler<count_t>      ThreeDimCountArray_t;
typedef TwoDimensionArrayHandler<measure_t>      TwoDimMeasureArray_t;
typedef ThreeDimensionArrayHandler<measure_t>    ThreeDimMeasureArray_t;
typedef ptr_vector<TwoDimCountArray_t>           CasesByCategory_t;
typedef boost::dynamic_bitset<>                  BatchIndexes_t;
typedef TwoDimensionArrayHandler<BatchIndexes_t> TwoDimBitsetArray_t;

void printCountArray(const TwoDimCountArray_t& arrayClass, FILE * stream=stdout);
void printMeasureArray(const TwoDimMeasureArray_t& arrayClass, FILE * stream=stdout);

class CSaTScanData; /** forward class declaration */
class DataSetHandler; /** forward class declaration */
class MetaManagerProxy; /** forward class declaration */

/** Encapsulates data for each input data set. 
    PT = purely temporal,  NC = non-cummulative,  Aux = auxillary,  Cat = Category */
class DataSet {
  friend class DataSetHandler;
  protected:
    unsigned int                giIntervalsDimensions; // number of time intervals
    unsigned int                giLocationDimensions; // number of tracts
    unsigned int                giMetaLocations; // number of meta-tracts
    measure_t                   gtTotalMeasureAux; // sum auxillary entries in data set
    measure_t                   gtTotalMeasureAux2; // sum auxillary 2 entries in data set
    TwoDimCountArray_t        * gpCaseData; // case data by time interval and location, cumulative by intervals
    count_t                   * gpCaseData_PT; // case data by time interval, cumulatively by intervals
    TwoDimCountArray_t        * gpCaseData_NC; // case data by time interval and location, non-cumulative
    count_t                   * gpCaseData_PT_NC; // case data by time interval, non-cumulatively
    TwoDimMeasureArray_t      * gpMeasureData; // measured data by time interval and location, cumulative by intervals
    measure_t                 * gpMeasureData_PT; // measure data by time interval and location, cumulative by intervals
    TwoDimMeasureArray_t      * gpMeasureData_NC; // measured data by time interval and location, non-cumulative
    measure_t                 * gpMeasureData_PT_NC; // measure data by time interval, non-cumulatively
    TwoDimMeasureArray_t      * gpMeasureData_Aux; // auxillary data by time interval and location, cumulative by intervals
    measure_t                 * gpMeasureData_PT_Aux; // auxillary data by time interval, cumulative by intervals
    TwoDimMeasureArray_t      * gpMeasureData_Aux2; // auxillary 2 data by time interval and location, cumulative by intervals
    measure_t                 * gpMeasureData_PT_Aux2; // auxillary 2 data by time interval, cumulative by intervals
    TwoDimBitsetArray_t       * gpPositiveBatchIndexes; // positive batch index data by time interval and location, cumulative by intervals 
    BatchIndexes_t            * gpPositiveBatchIndexes_PT; // positive batch index data by time interval, cumulative by intervals 
    CasesByCategory_t           gvCaseData_Cat; // category case data by time interval and location, cumulatively by intervals 
    TwoDimCountArray_t        * gpCaseData_PT_Cat; // category case data by time interval, cumulatively by intervals 
    AbstractTimeTrend         * gpTimeTrend; // time trend data
    unsigned int                giSetIndex; // index of this data set

                                DataSet(const DataSet& thisSet);
    DataSet                   & operator=(const DataSet& rhs);

  public:
    DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations, const CParameters& parameters, unsigned int iSetIndex);
    virtual ~DataSet();
    virtual DataSet * Clone() const;

    TwoDimCountArray_t        & allocateCaseData();
    CasesByCategory_t         & allocateCaseData_Cat(unsigned int iNumCategories);
    TwoDimCountArray_t        & allocateCaseData_NC();
    count_t                   * allocateCaseData_PT();
    TwoDimCountArray_t        & allocateCaseData_PT_Cat(unsigned int iNumCategories);
    count_t                   * allocateCaseData_PT_NC();
    TwoDimMeasureArray_t      & allocateMeasureData();
    TwoDimMeasureArray_t      & allocateMeasureData_NC();
    measure_t                 * allocateMeasureData_PT();
    measure_t                 * allocateMeasureData_PT_NC();
    measure_t                 * allocateMeasureData_PT_Aux();
    measure_t                 * allocateMeasureData_PT_Aux2();
    TwoDimMeasureArray_t      & allocateMeasureData_Aux();
    TwoDimMeasureArray_t      & allocateMeasureData_Aux2();
    TwoDimBitsetArray_t       & allocatePositiveBatchData(unsigned int setSize);
    BatchIndexes_t            * allocatePositiveBatchData_PT(unsigned int setSize);
    TwoDimCountArray_t        & getCaseData() const;
    const CasesByCategory_t   & getCaseData_Cat() const {return gvCaseData_Cat;}
    TwoDimCountArray_t        & getCaseData_NC() const;
    count_t                   * getCaseData_PT() const;
    TwoDimCountArray_t        & getCaseData_PT_Cat() const;
    count_t                   * getCaseData_PT_NC() const;
    TwoDimMeasureArray_t      & getMeasureData() const;
    TwoDimMeasureArray_t      & getMeasureData_NC() const;
    measure_t                 * getMeasureData_PT(bool check=true) const;
    measure_t                 * getMeasureData_PT_NC(bool check=true) const;
    measure_t                 * getMeasureData_PT_Aux() const;
    measure_t                 * getMeasureData_PT_Aux2() const;
    TwoDimMeasureArray_t      & getMeasureData_Aux() const;
    TwoDimMeasureArray_t      & getMeasureData_Aux2() const;
    unsigned int                getIntervalDimension() const {return giIntervalsDimensions;}
    unsigned int                getLocationDimension() const {return giLocationDimensions;}
    unsigned int                getMetaLocationDimension() const {return giMetaLocations;}
    TwoDimBitsetArray_t       & getPositiveBatchData() const;
    BatchIndexes_t            * getPositiveBatchIndexes_PT() const;
    measure_t                   getTotalMeasureAux() const { return gtTotalMeasureAux; }
    measure_t                   getTotalMeasureAux2() const { return gtTotalMeasureAux2; }
    AbstractTimeTrend         & getTimeTrend() {return *gpTimeTrend;}
    const AbstractTimeTrend   & getTimeTrend() const {return *gpTimeTrend;}
    unsigned int                getSetIndex() const {return giSetIndex;}
    virtual void                reassignMetaData(const MetaManagerProxy& MetaLocations);
    void                        setCaseDataToCumulative();
    void                        setCaseDataMeta(const MetaManagerProxy& MetaProxy);
    void                        setCaseDataCatMeta(const MetaManagerProxy& MetaProxy);
    void                        setCaseData_NC();
    void                        setCaseData_PT();
    void                        setCaseData_PT_Cat();
    void                        setCaseData_PT_NC();
    void                        setMeasureDataMeta(const MetaManagerProxy& MetaProxy);
    void                        setMeasureDataAuxMeta(const MetaManagerProxy& MetaProxy);
    void                        setMeasureDataAux2Meta(const MetaManagerProxy& MetaProxy);
    void                        setPositiveBatchIndexes_PT(unsigned int setSize);
    void                        setPositiveBitsetDataMeta(const MetaManagerProxy& MetaProxy);
    void                        setMeasureData_NC();
    void                        setMeasureData_PT();
    void                        setMeasureData_PT_NC();
    void                        setMeasureData_PT_Aux();
    void                        setMeasureData_PT_Aux2();
    void                        setMeasureDataToCumulative();
    void                        setMeasureData_Aux(TwoDimMeasureArray_t& other);
    void                        setMeasureData_Aux2(TwoDimMeasureArray_t& other);
    void                        setTotalMeasureAux(measure_t tTotalMeasureAux) { gtTotalMeasureAux = tTotalMeasureAux; }
    void                        setTotalMeasureAux2(measure_t tTotalMeasureAux) { gtTotalMeasureAux2 = tTotalMeasureAux; }

};

/** Encapsulates real data of dataset. */
class RealDataSet : public DataSet {
  friend class DataSetHandler;

  public:
    typedef std::pair<boost::shared_ptr<TwoDimMeasureArray_t>, boost::shared_ptr<PopulationData> > PopulationDataPair_t;
    typedef std::map<boost::gregorian::greg_weekday,count_t> CountsByWeekDay_t;
    typedef std::map<unsigned int, count_t> CategoryCaseCount_t;
    typedef std::map<boost::gregorian::greg_weekday,CategoryCaseCount_t> CategoryCountsByWeekDay_t;

  private:
    RealDataSet(const RealDataSet& thisSet);

  protected:
    boost::shared_ptr<PopulationData> _population;
    measure_t                   gtTotalMeasure; // sum measure data
    count_t                     gtTotalCases; // sum case data
    CountsByWeekDay_t           _totalCasesByWeekDay;
    CountsByWeekDay_t           _totalControlsByWeekDay;
    CategoryCountsByWeekDay_t   _totalCategoryCasesByWeekDay;
    double                      gdTotalPop; // sum population
    count_t                     gtTotalCasesAtStart; // sum case data at analysis start
    count_t                     gtTotalControls; // sum control data
    measure_t                   gtTotalMeasureAtStart; // sum measure data at analysis start
    TwoDimCountArray_t        * gpControlData; // control data by time interval and location, cumulative by intervals
    TwoDimCountArray_t        * gpCaseData_Censored; // censored case data by time interval and location, cumulative by intervals
    double                      gdCalculatedTimeTrendPercentage; // calculated time trend percentage used to temporal adjust expected cases
    std::string                 _calculatedQuadraticTrend; // calculated quadratic trend used to adjust temporal expected cases
    PopulationDataPair_t        _populationData; // population measure data and PopulationData
    TwoDimBitsetArray_t        * gpBatchIndexes; // batch index data by time interval and location, cumulative by intervals
    BatchIndexes_t             * gpBatchIndexes_PT; // batch index data by time interval, cumulative by intervals 


  public:
    RealDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations, const CParameters& parameters, unsigned int iSetIndex);
    virtual ~RealDataSet();

    TwoDimBitsetArray_t       & allocateBatchData(unsigned int setSize);
    BatchIndexes_t            * allocateBatchData_PT(unsigned int setSize);
    TwoDimCountArray_t        & allocateCaseData_Censored();
    TwoDimCountArray_t        & allocateControlData();
    TwoDimCountArray_t        & addCategoryTypeCaseCount(const std::string& categoryTypeLabel, count_t Count, Julian date, bool asOrdinal);
    void                        checkPopulationDataCases(CSaTScanData& Data);
    TwoDimBitsetArray_t       & getBatchData() const;
    BatchIndexes_t            * getBatchIndexes_PT() const;
    double                      getCalculatedTimeTrendPercentage() const {return gdCalculatedTimeTrendPercentage;}
    const std::string         & getCalculatedQuadraticTimeTrend() const { return _calculatedQuadraticTrend; }
    TwoDimCountArray_t        & getCategoryCaseData(unsigned int iCategoryIndex) const;
    TwoDimCountArray_t        & getCategoryCaseData(unsigned int iCategoryIndex, bool bCreateable=false);
    TwoDimCountArray_t        & getCaseData_Censored() const;
    TwoDimCountArray_t        & getControlData() const;
    PopulationData            & getPopulationData() {return *_population;}
    const PopulationData      & getPopulationData() const {return *_population;}
    count_t                     getTotalCases() const {return gtTotalCases;}
    count_t                     getTotalCasesAtStart() const {return gtTotalCasesAtStart;}
    count_t                     getTotalControls() const {return gtTotalControls;}
    measure_t                   getTotalMeasure() const {return gtTotalMeasure;}
    measure_t                   getTotalMeasureAtStart() const {return gtTotalMeasureAtStart;}
    double                      getTotalPopulation() const {return gdTotalPop;}
    virtual void                reassignMetaData(const MetaManagerProxy& MetaLocations);
    void                        resetPopulationData();
    void                        setAggregateCovariateCategories(bool b) {_population->SetAggregateCovariateCategories(b);}
    void                        setBitsetDataMeta(const MetaManagerProxy& MetaProxy);
    void                        setBatchData_PT(unsigned int setSize);
    void                        setCalculatedTimeTrendPercentage(double dTimeTrend) {gdCalculatedTimeTrendPercentage=dTimeTrend;}
    void                        setCalculatedQuadraticTimeTrend(std::string& functionStr, std::string& definitionStr);
    void                        setCaseData_Censored_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setControlData_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setTotalCases(count_t tTotalCases) {gtTotalCases = tTotalCases;}
    void                        setTotalCasesByWeekDay(CountsByWeekDay_t weekDayCounts) {_totalCasesByWeekDay = weekDayCounts;}
    CountsByWeekDay_t           getTotalCasesByWeekDay() const {return _totalCasesByWeekDay;}
    void                        setTotalCasesAtStart(count_t tTotalCases) {gtTotalCasesAtStart = tTotalCases;}
    void                        setTotalCategoryCasesByWeekDay(CategoryCountsByWeekDay_t weekDayCategoryCounts) {_totalCategoryCasesByWeekDay = weekDayCategoryCounts;}
    CategoryCountsByWeekDay_t   getTotalCategoryCasesByWeekDay() const {return _totalCategoryCasesByWeekDay;}
    CountsByWeekDay_t           getTotalControlByWeekDay() const {return _totalControlsByWeekDay;}
    void                        reassign(TwoDimCountArray_t& cases, TwoDimMeasureArray_t& measure);
    void                        setTotalControls(count_t tTotalControls) {gtTotalControls = tTotalControls;}
    void                        setTotalControlByWeekDay(CountsByWeekDay_t weekDayCounts) {_totalControlsByWeekDay = weekDayCounts;}
    void                        setTotalMeasure(measure_t tTotalMeasure) {gtTotalMeasure = tTotalMeasure;}
    void                        setTotalMeasureAtStart(measure_t tTotalMeasure) {gtTotalMeasureAtStart = tTotalMeasure;}
    void                        setTotalPopulation(measure_t tTotalPopulation) {gdTotalPop = tTotalPopulation;}
    void                        setPopulationMeasureData(TwoDimMeasureArray_t& otherMeasure, boost::shared_ptr<PopulationData> * otherPopulation=0);
    PopulationDataPair_t        getPopulationMeasureData() const;
};
//*****************************************************************************
#endif
