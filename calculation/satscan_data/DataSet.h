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

//typedefs for multiple dimension arrays
typedef TwoDimensionArrayHandler<count_t>      TwoDimCountArray_t;
typedef ThreeDimensionArrayHandler<count_t>    ThreeDimCountArray_t;
typedef TwoDimensionArrayHandler<measure_t>    TwoDimMeasureArray_t;
typedef ThreeDimensionArrayHandler<measure_t>  ThreeDimMeasureArray_t;
typedef ptr_vector<TwoDimCountArray_t>         CasesByCategory_t;

class CSaTScanData; /** forward class declaration */
class DataSetHandler; /** forward class declaration */
class MetaManagerProxy;

/** Encapsulates data for each input data set. */
class DataSet {
  friend class DataSetHandler;
  protected:
    unsigned int                giIntervalsDimensions;                     /** number of time intervals */
    unsigned int                giLocationDimensions;                            /** number of tracts*/
    unsigned int                giMetaLocations;                            /** number of meta-tracts*/

    count_t                   * gpCaseData_PT;                         /** number of cases, cumulatively stratified by time intervals */
    count_t                   * gpCaseData_PT_NC;                       /** number of cases in each time interval */
    TwoDimCountArray_t        * gpCaseData;                         /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpCaseData_NC;                       /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are NOT distributed in time intervals cumulatively */
    measure_t                 * gpMeasureData_PT;                       /** number of expected cases, cumulatively stratified by time intervals */
    measure_t                 * gpMeasureData_PT_NC;              /** number of expected cases in each time interval */
    measure_t                 * gpMeasureData_PT_Aux;                     /** number of expected cases , cumulatively stratified by time intervals and
                                                                            as gotten from gpMeasureData_Aux */
    TwoDimMeasureArray_t      * gpMeasureData;                       /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpMeasureData_Aux;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively  */
    TwoDimMeasureArray_t      * gpMeasureData_NC;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are NOT distributed in time intervals cumulatively */
    CasesByCategory_t           gvCaseData_Cat;                      /** number of cases stratified with respect to time intervals by tract index for each category
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpCaseData_PT_Cat;               /** number of cases stratified with respect to time intervals by category index
                                                                            - cases are distributed in time intervals cumulatively */
    CTimeTrend                  gTimeTrend;                             /** time trend data */
    unsigned int                giSetIndex;

                                DataSet(const DataSet& thisSet);
    DataSet                   & operator=(const DataSet& rhs);

  public:
    DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations,unsigned int iSetIndex);
    virtual ~DataSet();

    virtual DataSet           * Clone() const;

    // PT = purely temporal,  NC = non-cummulative,  Aux = auxillary,  Cat = Category

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
    TwoDimMeasureArray_t      & allocateMeasureData_Aux();
    TwoDimCountArray_t        & getCaseData() const;
    const CasesByCategory_t   & getCaseData_Cat() const {return gvCaseData_Cat;}
    TwoDimCountArray_t        & getCaseData_NC() const;
    count_t                   * getCaseData_PT() const;
    TwoDimCountArray_t        & getCaseData_PT_Cat() const;
    count_t                   * getCaseData_PT_NC() const;
    TwoDimMeasureArray_t      & getMeasureData() const;
    TwoDimMeasureArray_t      & getMeasureData_NC() const;
    measure_t                 * getMeasureData_PT() const;
    measure_t                 * getMeasureData_PT_NC() const;
    measure_t                 * getMeasureData_PT_Aux() const;
    TwoDimMeasureArray_t      & getMeasureData_Aux() const;
    unsigned int                getIntervalDimension() const {return giIntervalsDimensions;}
    unsigned int                getLocationDimension() const {return giLocationDimensions;}
    CTimeTrend                & getTimeTrend() {return gTimeTrend;}
    const CTimeTrend          & getTimeTrend() const {return gTimeTrend;}
    unsigned int                getSetIndex() const {return giSetIndex;}
    virtual void                reassignMetaLocationData(const MetaManagerProxy& MetaLocations);
    void                        setCaseData_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setCaseData_Cat_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setCaseData_NC();
    void                        setCaseData_PT();
    void                        setCaseData_PT_Cat();
    void                        setCaseData_PT_NC();
    void                        setMeasureData_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setMeasureData_Aux_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setMeasureData_NC();
    void                        setMeasureData_PT();
    void                        setMeasureData_PT_NC();
    void                        setMeasureData_PT_Aux();
    void                        setMeasureDataToCumulative();
};

/** Encapsulates real data of dataset. */
class RealDataSet : public DataSet {
  friend class DataSetHandler;

  private:
    RealDataSet(const RealDataSet& thisSet);

  protected:
    PopulationData              gPopulation;                            /** population data */
    measure_t                   gtTotalMeasure;                         /** number of expected cases in data set */
    measure_t                   gtTotalMeasureAux;                       /** number of auxillary entries in data set */
    count_t                     gtTotalCases;                           /** number of cases in data set */
    double                      gdTotalPop;                             /** population in data set */
    count_t                     gtTotalCasesAtStart;                    /** number of cases as defined at analysis start */
    count_t                     gtTotalControls;                        /** number of controls in data set */
    measure_t                   gtTotalMeasureAtStart;                  /** number of expected cases as defined at analysis start */
    TwoDimCountArray_t        * gpControlData;                      /** number of controls stratified with respect to time intervals by tract index
                                                                            - controls are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpCaseData_Censored;                 /** number of censored individuals stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    double                      gdCalculatedTimeTrendPercentage;        /** calculated time trend percentage used to temporal adjust expected cases*/

  public:
    RealDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations, unsigned int iSetIndex);
    virtual ~RealDataSet();

    TwoDimCountArray_t        & allocateCaseData_Censored();
    TwoDimCountArray_t        & allocateControlData();
    TwoDimCountArray_t        & addOrdinalCategoryCaseCount(double dOrdinalNumber, count_t Count);
    void                        checkPopulationDataCases(CSaTScanData& Data);
    double                      getCalculatedTimeTrendPercentage() const {return gdCalculatedTimeTrendPercentage;}
    TwoDimCountArray_t        & getCategoryCaseData(unsigned int iCategoryIndex) const;
    TwoDimCountArray_t        & getCategoryCaseData(unsigned int iCategoryIndex, bool bCreateable=false);
    TwoDimCountArray_t        & getCaseData_Censored() const;
    TwoDimCountArray_t        & getControlData() const;
    PopulationData            & getPopulationData() {return gPopulation;}
    const PopulationData      & getPopulationData() const {return gPopulation;}
    count_t                     getTotalCases() const {return gtTotalCases;}
    count_t                     getTotalCasesAtStart() const {return gtTotalCasesAtStart;}
    count_t                     getTotalControls() const {return gtTotalControls;}
    measure_t                   getTotalMeasure() const {return gtTotalMeasure;}
    measure_t                   getTotalMeasureAux() const {return gtTotalMeasureAux;}
    measure_t                   getTotalMeasureAtStart() const {return gtTotalMeasureAtStart;}
    double                      getTotalPopulation() const {return gdTotalPop;}
    virtual void                reassignMetaLocationData(const MetaManagerProxy& MetaLocations);
    void                        setAggregateCovariateCategories(bool b) {gPopulation.SetAggregateCovariateCategories(b);}
    void                        setCalculatedTimeTrendPercentage(double dTimeTrend) {gdCalculatedTimeTrendPercentage=dTimeTrend;}
    void                        setCaseData_Censored_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setControlData_MetaLocations(const MetaManagerProxy& MetaProxy);
    void                        setTotalCases(count_t tTotalCases) {gtTotalCases = tTotalCases;}
    void                        setTotalCasesAtStart(count_t tTotalCases) {gtTotalCasesAtStart = tTotalCases;}
    void                        setTotalControls(count_t tTotalControls) {gtTotalControls = tTotalControls;}
    void                        setTotalMeasure(measure_t tTotalMeasure) {gtTotalMeasure = tTotalMeasure;}
    void                        setTotalMeasureAux(measure_t tTotalMeasureAux) {gtTotalMeasureAux = tTotalMeasureAux;}
    void                        setTotalMeasureAtStart(measure_t tTotalMeasure) {gtTotalMeasureAtStart = tTotalMeasure;}
    void                        setTotalPopulation(measure_t tTotalPopulation) {gdTotalPop = tTotalPopulation;}
};
//*****************************************************************************
#endif
