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

/** Encapsulates data for each stream of data. */
class DataStream {
  friend class DataStreamHandler;
  private:
    void                        Init();

  protected:
    unsigned int                giNumTimeIntervals;                     /** number of time intervals */
    unsigned int                giNumTracts;                            /** number of tracts*/

    count_t                   * gpPTCasesArray;                         /** number of cases, stratified by time intervals */
    TwoDimCountArray_t        * gpCasesHandler;                         /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimCountArray_t        * gpNCCasesHandler;                       /** number of cases stratified with respect to time intervals by tract index
                                                                            - cases are NOT distributed in time intervals cumulatively */
    measure_t                 * gpPTMeasureArray;                       /** number of expected cases, stratified by time intervals */
    measure_t                 * gpPTSqMeasureArray;                     /** number of expected cases squared, stratified by time intervals and
                                                                            as gotten from gpSqMeasureHandler */
    TwoDimMeasureArray_t      * gpMeasureHandler;                       /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpSqMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are distributed in time intervals cumulatively and are the square
                                                                              of entries accumulated in gpMeasureHandler */
    TwoDimMeasureArray_t      * gpNCMeasureHandler;                     /** number of expected cases stratified with respect to time intervals by tract index
                                                                            - expected cases are NOT distributed in time intervals cumulatively */
    CTimeTrend                  gTimeTrend;                             /** time trend data */

    unsigned int                giStreamIndex;

    void                        SetCaseArrays(count_t** pCases, count_t** pCases_NC, count_t* pCasesByTimeInt);
  public:
    DataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex);
    virtual ~DataStream();

    void                        AllocateCasesArray();
    void                        AllocateMeasureArray();
    void                        AllocateSqMeasureArray();
    void                        AllocatePTCasesArray();
    void                        AllocatePTMeasureArray();
    void                        AllocatePTSqMeasureArray();
    void                        AllocateNCMeasureArray();
    void                        AllocateNCCasesArray();
    count_t                  ** GetCaseArray() const;
    count_t                  ** GetNCCaseArray() const;
    measure_t                ** GetMeasureArray() const;
    TwoDimMeasureArray_t      & GetMeasureArrayHandler();
    TwoDimMeasureArray_t      & GetSqMeasureArrayHandler();
    measure_t                ** GetNCMeasureArray() const;
    inline unsigned int         GetNumTimeIntervals() const {return giNumTimeIntervals;}
    inline unsigned int         GetNumTracts() const {return giNumTracts;}
    count_t                   * GetPTCasesArray() const;
    measure_t                 * GetPTMeasureArray() const;
    measure_t                 * GetPTSqMeasureArray() const {return gpPTSqMeasureArray;}
    CTimeTrend                & GetTimeTrend() {return gTimeTrend;}
    measure_t                ** GetSqMeasureArray() const;
    unsigned int                GetStreamIndex() const {return giStreamIndex;}
    void                        SetCaseArrays();
    void                        SetPTCasesArray();
    void                        SetPTMeasureArray();
    void                        SetPTSqMeasureArray();
};

/** Encapsulates real data of a data stream. */
class RealDataStream : public DataStream {
  friend class DataStreamHandler;

  private:
    void                        Init();
    void                        Setup();

  protected:
    PopulationData              gPopulation;                            /** population data */
    measure_t                   gtTotalMeasure;                         /** number of expected cases in data stream */
    count_t                     gtTotalCases;                           /** number of cases in data stream */
    double                      gdTotalPop;                             /** population in data stream */
    count_t                     gtTotalCasesAtStart;                    /** number of cases as defined at analysis start */
    count_t                     gtTotalControls;                        /** number of controls in data stream */
    count_t                     gtTotalControlsAtStart;                 /** number of controls as defined at analysis start */
    measure_t                   gtTotalMeasureAtStart;                  /** number of expected cases as defined at analysis start */
    TwoDimCountArray_t        * gpControlsHandler;                      /** number of controls stratified with respect to time intervals by tract index
                                                                            - controls are distributed in time intervals cumulatively */
    ThreeDimCountArray_t      * gpCategoryCasesHandler;                 /** number of cases stratified with respect to time intervals by tract index by category index
                                                                            - cases are distributed in time intervals cumulatively */
    TwoDimMeasureArray_t      * gpPopulationMeasureHandler;             /** expected number of cases in time intervals by population dates */

  public:
    RealDataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex);
    virtual ~RealDataStream();

    void                        AllocateCategoryCasesArray();
    void                        AllocateControlsArray();
    void                        AllocatePopulationMeasureArray();
    void                        CheckPopulationDataCases(CSaTScanData& Data);
    void                        FreePopulationMeasureArray();
    count_t                 *** GetCategoryCaseArray() const;
    ThreeDimCountArray_t      & GetCategoryCaseArrayHandler();
    count_t                  ** GetControlArray()const;
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
    void                        SetAggregateCategories(bool b) {gPopulation.SetAggregateCategories(b);}
    void                        SetCasesByTimeInterval();
    void                        SetCumulativeMeasureArrayFromNonCumulative();
    void                        SetMeasureByTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure);
    void                        SetMeasureArrayAsCumulative();
    void                        SetNonCumulativeMeasureArrayFromCumulative();
    void                        SetTotalCases(count_t tTotalCases) {gtTotalCases = tTotalCases;}
    void                        SetTotalCasesAtStart(count_t tTotalCases) {gtTotalCasesAtStart = tTotalCases;}
    void                        SetTotalControls(count_t tTotalControls) {gtTotalControls = tTotalControls;}
    void                        SetTotalControlsAtStart(count_t tTotalControls) {gtTotalControlsAtStart = tTotalControls;}
    void                        SetTotalMeasure(measure_t tTotalMeasure) {gtTotalMeasure = tTotalMeasure;}
    void                        SetTotalMeasureAtStart(measure_t tTotalMeasure) {gtTotalMeasureAtStart = tTotalMeasure;}
    void                        SetTotalPopulation(measure_t tTotalPopulation) {gdTotalPop = tTotalPopulation;}
};

/** Encapsulates simulation data of a data stream. */
class SimulationDataStream : public DataStream {
  public:
    SimulationDataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex);
    virtual ~SimulationDataStream();

    void                        ResetCumulativeCaseArray();
};
//*****************************************************************************
#endif
