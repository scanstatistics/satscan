//*****************************************************************************
#ifndef __DataStream_H
#define __DataStream_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "MultipleDimensionArrayHandler.h"
#include "PopulationData.h"
#include "TimeTrend.h"

//typedefs for multiple dimension arrays
typedef TwoDimensionArrayHandler<count_t>      TwoDimCountArray_t;
typedef ThreeDimensionArrayHandler<count_t>    ThreeDimCountArray_t;
typedef TwoDimensionArrayHandler<measure_t>    TwoDimMeasureArray_t;
typedef ThreeDimensionArrayHandler<measure_t>  ThreeDimMeasureArray_t;

class CSaTScanData; /** forward class declaration */
class DataStreamHandler; /** forward class declaration */

/** Encapsulates data for each stream of data.
    NOTE: Look into derived DataStream classes for each handler class. */
class DataStream {
  friend class DataStreamHandler;
  private:
    void                        Init();
    void                        Setup();

  protected:
    PopulationData              gPopulation;                            /** population data */
    measure_t                   gtTotalMeasure;                         /** number of expected cases in data stream */
    count_t                     gtTotalCases;                           /** number of cases in data stream */
    double                      gdTotalPop;                             /** population in data stream */
    unsigned int                giNumTimeIntervals;                     /** number of time intervals */
    unsigned int                giNumTracts;                            /** number of tracts*/
    count_t                     gtTotalCasesAtStart;                    /** number of cases as defined at analysis start */
    count_t                     gtTotalControls;                        /** number of controls in data stream */
    count_t                     gtTotalControlsAtStart;                 /** number of controls as defined at analysis start */
    count_t                   * gpPTCasesArray;                         /** number of cases, stratified by time intervals */
    count_t                   * gpPTSimCasesArray;                      /** number of simulated cases, stratified by time intervals */
    TwoDimCountArray_t        * gpCasesHandler;                         /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpNCCasesHandler;                       /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are NOT distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpControlsHandler;                      /** number of controls stratified with respect to time intervals by tract index
                                                                            - controls are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpSimCasesHandler;                      /** number of simulated cases stratified with respect to time intervals by tract index
                                                                            - simulated cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpNCSimCasesHandler;                    /** number of simulated cases stratified with respect to time intervals by tract index
                                                                            - simulated cases are NOT distributed in time intervals cumulatively */
    measure_t                   gtTotalMeasureAtStart;                  /** number of expected cases as defined at analysis start */
    measure_t                 * gpPTMeasureArray;                       /** number of expected cases, stratified by time intervals */
    measure_t                 * gpPTSqMeasureArray;                     /** number of expected cases squared, stratified by time intervals and
                                                                            as gotten from gpSqMeasureHandler */
    measure_t                 * gpPTSimMeasureArray;                    /** number of simulated expected cases, stratified by time intervals */
    TwoDimMeasureArray_t      * gpMeasureHandler;                       /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpSimMeasureHandler;                    /** number of simulated expected cases stratified with respect to time intervals by tract index
                                                                           - expected cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpSqMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively and are the square
                                                                              of entries accumulated in gpMeasureHandler */
    TwoDimMeasureArray_t      * gpSimSqMeasureHandler;                  /** number of simulated expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively and are the square
                                                                              of entries accumulated in gpSimMeasureHandler */
    measure_t                 * gpPTSimSqMeasureArray;                  /** number of simulated expected cases, stratified by time intervals
                                                                            -are the square of entries accumulated in gpPTSimMeasureArray */
    TwoDimMeasureArray_t      * gpNCMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are NOT distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpCategoryMeasureHandler;               /** number of expected cases stratified with respect to time intervals by category index
                                                                            - expected cases are distributed in time intervals cumulatively */
    ThreeDimCountArray_t      * gpCategoryCasesHandler;                 /** number of cases stratified with respect to time intervals by tract index by category index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpPopulationMeasureHandler;             /** expected number of cases in time intervals by population dates */
    CTimeTrend                  gTimeTrend;                             /** time trend data */
    CTimeTrend                  gSimTimeTrend;                          /** time trend data fore simulations */

    unsigned int                giStreamIndex;

    void                        SetCaseArrays(count_t** pCases, count_t** pCases_NC, count_t* pCasesByTimeInt);
  public:
    DataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex);
    virtual ~DataStream();

    void                        AllocateCategoryCasesArray();
    void                        AllocateCasesArray();
    void                        AllocateControlsArray();
    void                        AllocateMeasureArray();
    void                        AllocateSimMeasureArray();
    void                        AllocateSqMeasureArray();    
    void                        AllocateSqSimMeasureArray();
    void                        AllocatePTMeasureArray();
    void                        AllocatePTSimMeasureArray();
    void                        AllocatePTSimSqMeasureArray();
    void                        AllocateNCMeasureArray();
    void                        AllocatePopulationMeasureArray();
    void                        AllocateSimulationCasesArray();    
    void                        AllocateSimulationNCCasesArray();    
    void                        AllocateSimulationPTCasesArray();
    void                        CheckPopulationDataCases(CSaTScanData& Data);
    void                        FreePopulationMeasureArray();
    void                        FreeSimulationStructures();
    count_t                 *** GetCategoryCaseArray() const;
    ThreeDimCountArray_t      & GetCategoryCaseArrayHandler();
    count_t                  ** GetCaseArray() const;
    count_t                  ** GetNCCaseArray() const;
    count_t                  ** GetControlArray()const;
    measure_t                ** GetMeasureArray() const;
    count_t                  ** GetSimCaseArray() const;
    measure_t                ** GetSimMeasureArray() const;
    TwoDimMeasureArray_t      & GetSimMeasureArrayHandler();
    TwoDimMeasureArray_t      & GetSimSqMeasureArrayHandler();
    count_t                  ** GetNCSimCaseArray() const;
    measure_t                ** GetNCMeasureArray() const;
    inline unsigned int         GetNumTimeIntervals() const {return giNumTimeIntervals;}
    inline unsigned int         GetNumTracts() const {return giNumTracts;}
    measure_t                ** GetPopulationMeasureArray() const;
    PopulationData            & GetPopulationData() {return gPopulation;}
    const PopulationData      & GetPopulationData() const {return gPopulation;}
    count_t                   * GetPTCasesArray() const;
    measure_t                 * GetPTMeasureArray() const;
    measure_t                 * GetPTSimMeasureArray() const {return gpPTSimMeasureArray;}
    measure_t                 * GetPTSqMeasureArray() const {return gpPTSqMeasureArray;}
    count_t                   * GetPTSimCasesArray() const;
    CTimeTrend                & GetTimeTrend() {return gTimeTrend;}
    CTimeTrend                & GetSimTimeTrend() {return gSimTimeTrend;}
    measure_t                 * GetSimPTSqMeasureArray() const {return gpPTSimSqMeasureArray;}
    measure_t                ** GetSimSqMeasureArray();
    measure_t                ** GetSqMeasureArray();
    unsigned int                GetStreamIndex() const {return giStreamIndex;}
    count_t                     GetTotalCases() const {return gtTotalCases;}
    count_t                     GetTotalCasesAtStart() const {return gtTotalCasesAtStart;}
    count_t                     GetTotalControls() const {return gtTotalControls;}
    count_t                     GetTotalControlsAtStart() const {return gtTotalControlsAtStart;}
    measure_t                   GetTotalMeasure() const {return gtTotalMeasure;}
    measure_t                   GetTotalMeasureAtStart() const {return gtTotalMeasureAtStart;}
    double                      GetTotalPopulation() const {return gdTotalPop;}
    void                        ResetCumulativeSimCaseArray();
    void                        SetAggregateCategories(bool b) {gPopulation.SetAggregateCategories(b);}
    void                        SetCaseArrays();
    void                        SetSimCaseArrays();
    void                        SetCasesByTimeInterval();
    void                        SetCumulativeMeasureArrayFromNonCumulative();
    void                        SetMeasureByTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure);
    void                        SetMeasureArrayAsCumulative();
    void                        SetNonCumulativeMeasureArrayFromCumulative();
    void                        SetPTCasesArray();
    void                        SetPTMeasureArray();
    void                        SetPTSqMeasureArray();
    void                        SetPTSimCasesArray();
    void                        SetPTSimMeasureArray();
    void                        SetPTSqSimMeasureArray();
    void                        SetTotalCases(count_t tTotalCases) {gtTotalCases = tTotalCases;}
    void                        SetTotalCasesAtStart(count_t tTotalCases) {gtTotalCasesAtStart = tTotalCases;}
    void                        SetTotalControls(count_t tTotalControls) {gtTotalControls = tTotalControls;}
    void                        SetTotalControlsAtStart(count_t tTotalControls) {gtTotalControlsAtStart = tTotalControls;}
    void                        SetTotalMeasure(measure_t tTotalMeasure) {gtTotalMeasure = tTotalMeasure;}
    void                        SetTotalMeasureAtStart(measure_t tTotalMeasure) {gtTotalMeasureAtStart = tTotalMeasure;}
    void                        SetTotalPopulation(measure_t tTotalPopulation) {gdTotalPop = tTotalPopulation;}
};
//*****************************************************************************
#endif
