//---------------------------------------------------------------------------
#ifndef DataStreamInterfaceH
#define DataStreamInterfaceH
//---------------------------------------------------------------------------
#include "DataStream.h"
#include "Parameters.h"

/** Interface for accessing data stream structures. Whether looking for most
    likely clusters or performing simulations, the analyses algorithms only
    need know, for instance, that it accesses a count_t ** structure. With this
    interface, each real/simulation algorithm can use either simulation or real
    data. This is particularly useful with multiple data streams, since the same
    process is used with both the real and simulation data.
    NOTE: By default, this interface sets it's pointers to structures of real data */
class DataStreamInterface {
  private:
    unsigned int        giNumTimeIntervals;     /** number of time intervals */
    unsigned int        giNumTracts;            /** number of tracts */
    count_t             gTotalCases;            /** number of cases in data stream */
    count_t             gTotalControls;         /** number of controls in data stream */
    measure_t           gTotalMeasure;          /** number of expected cases in data stream */

    count_t          ** gppCaseArray;           /** pointer to data stream case array */
    count_t          ** gppNCCaseArray;         /** pointer to data stream non-cumulative case array */
    count_t           * gpPTCaseArray;          /** pointer to data stream temporal case array */
    measure_t        ** gppMeasureArray;        /** pointer to data stream measure array */
    measure_t        ** gppNCMeasureArray;      /** pointer to data stream non-cumulative measure array */
    measure_t         * gpPTMeasureArray;       /** pointer to data stream temporal measure array */
    measure_t        ** gppSqMeasureArray;      /** */
    CTimeTrend        * gpTimeTrend;            /** pointer to data stream time trend structure */

    void                Init();
    virtual void        SetPurelySpatialAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
    virtual void        SetPurelyTemporalAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
    virtual void        SetSpaceTimeAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
    virtual void        SetSVTTAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);

  protected:
    void                SetCaseArray(count_t ** ppCases) {gppCaseArray = ppCases;}
    void                SetNCCaseArray(count_t ** ppCases) {gppNCCaseArray = ppCases;}
    void                SetPTCaseArray(count_t * pPTCase) {gpPTCaseArray = pPTCase;}
    void                SetMeasureArray(measure_t ** ppMeasure) {gppMeasureArray = ppMeasure;}
    void                SetNCMeasureArray(measure_t ** ppMeasure) {gppNCMeasureArray = ppMeasure;}
    void                SetPTMeasureArray(measure_t * pMeasure) {gpPTMeasureArray = pMeasure;}
    void                SetSqMeasureArray(measure_t ** ppSqMeasure) {gppSqMeasureArray = ppSqMeasure;}
    void                SetTimeTrend(CTimeTrend * pTimeTrend) {gpTimeTrend = pTimeTrend;}
    void                SetTotalCasesCount(count_t tCases) {gTotalCases = tCases;}
    void                SetTotalControlsCount(count_t tControls) {gTotalControls = tControls;}
    void                SetTotalMeasureCount(measure_t tMeasure) {gTotalMeasure = tMeasure;}
    
  public:
    DataStreamInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts);
    virtual ~DataStreamInterface();

    inline count_t   ** GetCaseArray() const {return gppCaseArray;}
    inline count_t   ** GetNCCaseArray() const {return gppNCCaseArray;}
    inline count_t    * GetPTCaseArray() const {return gpPTCaseArray;}
    inline measure_t ** GetMeasureArray() const {return gppMeasureArray;}
    inline measure_t ** GetNCMeasureArray() const {return gppNCMeasureArray;}
    inline measure_t  * GetPTMeasureArray() const {return gpPTMeasureArray;}
    inline bool         IsSqMeasureArray() const {return gppSqMeasureArray;}
    inline measure_t ** GetSqMeasureArray() const {return gppSqMeasureArray;}
    inline CTimeTrend * GetTimeTrend() const {return gpTimeTrend;}
    inline count_t      GetTotalCasesCount() const {return gTotalCases;}
    inline count_t      GetTotalControlsCount() const {return gTotalControls;}
    inline measure_t    GetTotalMeasureCount() const {return gTotalMeasure;}
    void                ResetCaseArray(count_t t);
    void                Set(DataStream & thisStream, const CParameters & Parameters);
};

/** Interface for accessing simulation data stream structures. */
class SimulationDataStreamInterface : public DataStreamInterface {
  private:
    virtual void        SetPurelySpatialAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
    virtual void        SetPurelyTemporalAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
    virtual void        SetSpaceTimeAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);    
    virtual void        SetSVTTAnalysisInterface(DataStream & thisStream, const CParameters & Parameters);
  
  public:
    SimulationDataStreamInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts);
    virtual ~SimulationDataStreamInterface();
};
#endif
 