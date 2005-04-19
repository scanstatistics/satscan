//******************************************************************************
#ifndef __DataSet_H
#define __DataSet_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "MultipleDimensionArrayHandler.h"
#include "PopulationData.h"
#include "TimeTrend.h"
#include <iostream>
#include <fstream>

//typedefs for multiple dimension arrays
typedef TwoDimensionArrayHandler<count_t>      TwoDimCountArray_t;
typedef ThreeDimensionArrayHandler<count_t>    ThreeDimCountArray_t;
typedef TwoDimensionArrayHandler<measure_t>    TwoDimMeasureArray_t;
typedef ThreeDimensionArrayHandler<measure_t>  ThreeDimMeasureArray_t;
typedef ZdPointerVector<TwoDimCountArray_t>    CasesByCategory_t;

class CSaTScanData; /** forward class declaration */
class DataSetHandler; /** forward class declaration */

/** Encapsulates data for each input data set. */
class DataSet {
  friend class DataSetHandler;
  private:
    void                        Init();

  protected:
    unsigned int                giNumTimeIntervals;                     /** number of time intervals */
    unsigned int                giNumTracts;                            /** number of tracts*/

    count_t                   * gpPTCasesArray;                         /** number of cases, cumulatively stratified by time intervals */
    count_t                   * gpCasesPerIntervalArray;                /** number of cases in each time interval */
    TwoDimCountArray_t        * gpCasesHandler;                         /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpNCCasesHandler;                       /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are NOT distributed in time intervals cumulatively */
    measure_t                 * gpPTMeasureArray;                       /** number of expected cases, cumulatively stratified by time intervals */
    measure_t                 * gpMeasurePerIntervalArray;              /** number of expected cases in each time interval */
    measure_t                 * gpPTSqMeasureArray;                     /** number of expected cases squared, cumulatively stratified by time intervals and
                                                                            as gotten from gpSqMeasureHandler */
    TwoDimMeasureArray_t      * gpMeasureHandler;                       /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpSqMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively and are the square
                                                                              of entries accumulated in gpMeasureHandler */
    TwoDimMeasureArray_t      * gpNCMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are NOT distributed in time intervals cumulatively */
    CasesByCategory_t           gvCasesByCategory;                      /** number of cases stratified with respect to time intervals by tract index for each category
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpPTCategoryCasesHandler;               /** number of cases stratified with respect to time intervals by category index
                                                                            - cases are distributed in time intervals cumulatively */
    CTimeTrend                  gTimeTrend;                             /** time trend data */

    unsigned int                giSetIndex;

    DataSet(const DataSet& thisSet);
    DataSet                   & operator=(const DataSet& rhs);

  public:
    DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iSetIndex);
    virtual ~DataSet();

    void                        AllocateCasesArray();
    void                        AllocateCategoryCasesArray(unsigned int iNumCategories);
    void                        AllocateMeasureArray();
    void                        AllocateSqMeasureArray();
    void                        AllocatePTCasesArray();
    void                        AllocatePTCategoryCasesArray();
    void                        AllocatePTMeasureArray();
    void                        AllocatePTSqMeasureArray();
    void                        AllocateNCMeasureArray();
    void                        AllocateNCCasesArray();
    count_t                  ** GetCaseArray() const;
    const CasesByCategory_t   & GetCasesByCategory() const {return gvCasesByCategory;}
    count_t                   * GetCasesPerTimeIntervalArray() const;
    measure_t                ** GetMeasureArray() const;
    TwoDimMeasureArray_t      & GetMeasureArrayHandler();
    measure_t                 * GetMeasurePerTimeIntervalArray() const;
    count_t                  ** GetNCCaseArray() const;
    measure_t                ** GetNCMeasureArray() const;
    TwoDimMeasureArray_t      & GetNCMeasureArrayHandler();
    inline unsigned int         GetNumTimeIntervals() const {return giNumTimeIntervals;}
    inline unsigned int         GetNumTracts() const {return giNumTracts;}
    count_t                   * GetPTCasesArray() const;
    count_t                  ** GetPTCategoryCasesArray() const;
    measure_t                 * GetPTMeasureArray() const;
    measure_t                 * GetPTSqMeasureArray() const {return gpPTSqMeasureArray;}
    CTimeTrend                & GetTimeTrend() {return gTimeTrend;}
    const CTimeTrend          & GetTimeTrend() const {return gTimeTrend;}
    measure_t                ** GetSqMeasureArray() const;
    TwoDimMeasureArray_t      & GetSqMeasureArrayHandler();
    unsigned int                GetSetIndex() const {return giSetIndex;}
    void                        SetNonCumulativeCaseArrays();
    void                        SetPTCasesArray();
    void                        SetPTCategoryCasesArray();
    void                        SetPTMeasureArray();
    void                        SetPTSqMeasureArray();
};

/** Encapsulates real data of dataset. */
class RealDataSet : public DataSet {
  friend class DataSetHandler;

  private:
    void                        Init();
    void                        Setup();

    RealDataSet(const RealDataSet& thisSet);

  protected:
    PopulationData              gPopulation;                            /** population data */
    measure_t                   gtTotalMeasure;                         /** number of expected cases in data set */
    count_t                     gtTotalCases;                           /** number of cases in data set */
    double                      gdTotalPop;                             /** population in data set */
    count_t                     gtTotalCasesAtStart;                    /** number of cases as defined at analysis start */
    count_t                     gtTotalControls;                        /** number of controls in data set */
    count_t                     gtTotalControlsAtStart;                 /** number of controls as defined at analysis start */
    measure_t                   gtTotalMeasureAtStart;                  /** number of expected cases as defined at analysis start */
    TwoDimCountArray_t        * gpControlsHandler;                      /** number of controls stratified with respect to time intervals by tract index
                                                                            - controls are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpCensoredCasesHandler;                 /** number of censored individuals stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpPopulationMeasureHandler;             /** expected number of cases in time intervals by population dates */
    double                      gdCalculatedTimeTrendPercentage;        /** calculated time trend percentage used to temporal adjust expected cases*/

  public:
    RealDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iSetIndex);
    virtual ~RealDataSet();

    void                        AllocateCensoredCasesArray();
    void                        AllocateControlsArray();
    measure_t                ** AllocatePopulationMeasureArray();
    count_t                  ** AddOrdinalCategoryCaseCount(double dOrdinalNumber, count_t Count);
    void                        CheckPopulationDataCases(CSaTScanData& Data);
    void                        FreePopulationMeasureArray();
    double                      GetCalculatedTimeTrendPercentage() const {return gdCalculatedTimeTrendPercentage;}
    count_t                  ** GetCategoryCaseArray(unsigned int iCategoryIndex) const;
    count_t                  ** GetCategoryCaseArray(unsigned int iCategoryIndex, bool bCreateable=false);
    count_t                  ** GetCensoredCasesArray() const;
    TwoDimCountArray_t        & GetCensoredCasesArrayHandler();
    count_t                  ** GetControlArray() const;
    measure_t                ** GetPopulationMeasureArray() const;
    PopulationData            & GetPopulationData() {return gPopulation;}
    const PopulationData      & GetPopulationData() const {return gPopulation;}
    count_t                     GetTotalCases() const {return gtTotalCases;}
    count_t                     GetTotalCasesAtStart() const {return gtTotalCasesAtStart;}
    count_t                     GetTotalControls() const {return gtTotalControls;}
    count_t                     GetTotalControlsAtStart() const {return gtTotalControlsAtStart;}
    measure_t                   GetTotalMeasure() const {return gtTotalMeasure;}
    measure_t                   GetTotalMeasureAtStart() const {return gtTotalMeasureAtStart;}
    double                      GetTotalPopulation() const {return gdTotalPop;}
    void                        SetAggregateCovariateCategories(bool b) {gPopulation.SetAggregateCovariateCategories(b);}
    void                        SetCalculatedTimeTrendPercentage(double dTimeTrend) {gdCalculatedTimeTrendPercentage=dTimeTrend;}
    void                        SetCasesPerTimeIntervalArray();
    void                        SetCumulativeMeasureArrayFromNonCumulative();
    void                        SetMeasureArrayAsCumulative();
    void                        SetMeasurePerTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure);
    void                        SetNonCumulativeMeasureArrayFromCumulative();
    void                        SetTotalCases(count_t tTotalCases) {gtTotalCases = tTotalCases;}
    void                        SetTotalCasesAtStart(count_t tTotalCases) {gtTotalCasesAtStart = tTotalCases;}
    void                        SetTotalControls(count_t tTotalControls) {gtTotalControls = tTotalControls;}
    void                        SetTotalControlsAtStart(count_t tTotalControls) {gtTotalControlsAtStart = tTotalControls;}
    void                        SetTotalMeasure(measure_t tTotalMeasure) {gtTotalMeasure = tTotalMeasure;}
    void                        SetTotalMeasureAtStart(measure_t tTotalMeasure) {gtTotalMeasureAtStart = tTotalMeasure;}
    void                        SetTotalPopulation(measure_t tTotalPopulation) {gdTotalPop = tTotalPopulation;}
};

/** Encapsulates simulation data of a data set. */
class SimDataSet : public DataSet {
  private:
    SimDataSet(const SimDataSet& thisSet);

  protected:  
    void                        ReadSimulationDataOrdinal(std::ifstream& filestream, unsigned int iSimulation);
    void                        ReadSimulationDataStandard(std::ifstream& filestream, unsigned int iSimulation);
    void                        WriteSimulationDataOrdinal(std::ofstream& filestream) const;
    void                        WriteSimulationDataStandard(std::ofstream& filestream) const;

  public:
    SimDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iSetIndex);
    virtual ~SimDataSet();

    virtual void                ReadSimulationData(const CParameters& Parameters, unsigned int iSimulation);
    void                        ResetCumulativeCaseArray();
    virtual void                WriteSimulationData(const CParameters& Parameters, int iSimulation) const;
};
//*****************************************************************************
#endif
