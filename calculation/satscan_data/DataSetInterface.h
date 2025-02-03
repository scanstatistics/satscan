//******************************************************************************
#ifndef __DataSetInterface_H
#define __DataSetInterface_H
//******************************************************************************
#include "DataSet.h"
#include "Parameters.h"

class AbstractRandomizer;
typedef std::vector<count_t**>    CategoryCasesArrays_t;

/** Interface for accessing dataset structures. Whether looking for most
    likely clusters or performing simulations, the analyses algorithms only
    need know, for instance, that it accesses a count_t ** structure. With this
    interface, each real/simulation algorithm can use either simulation or real
    data. This is particularly useful with multiple datasets, since the same
    process is used with both the real and simulation data.
    NOTE: By default, this interface sets it's pointers to structures of real data */
class DataSetInterface {
  private:
    unsigned int                giNumTimeIntervals;     /** number of time intervals */
    unsigned int                giNumTracts;            /** number of tracts */
    count_t                     gTotalCases;            /** number of cases in dataset */
    count_t                     gTotalControls;         /** number of controls in dataset */
    measure_t                   gTotalMeasure;          /** number of expected cases in dataset */
    measure_t                   gTotalMeasureAux;       /** number of auxillary entries in dataset */
    measure_t                   gTotalMeasureAux2;      /** number of auxillary entries in dataset */
    unsigned int                giNumOrdinalCategories; /** number of ordinal categories in dataset */
    const AbstractRandomizer  * _randomizer;            /** pointer to data set randomizer */

    unsigned int               _add_data_start_index;

    void                        Init();

  public:
    DataSetInterface(unsigned int iNumTimeIntervals=0, unsigned int iNumTracts=0, unsigned int add_data_start_index=0);
    virtual ~DataSetInterface();

    count_t                  ** gppCaseArray;           /** pointer to dataset case array */
    count_t                  ** gppNCCaseArray;         /** pointer to dataset non-cumulative case array */
    count_t                 *** gpppCategoryCaseArray;  /** pointer to dataset categorized case array */
    CategoryCasesArrays_t       gvCategoryCaseArrays;
    count_t                   * gpPTCaseArray;          /** pointer to dataset temporal case array */
    count_t                   * gpPSCaseArray;          /** pointer to dataset spatial case array */
    count_t                  ** gppPTCategoryCaseArray; /** pointer to dataset temporal categorized case array */
    measure_t                ** gppMeasureArray;        /** pointer to dataset measure array */
    measure_t                ** gppNCMeasureArray;      /** pointer to dataset non-cumulative measure array */
    measure_t                 * gpPTMeasureArray;       /** pointer to dataset temporal measure array */
    measure_t                 * gpPSMeasureArray;       /** pointer to dataset spatial measure array */
    measure_t                ** gppMeasureAuxArray;      /** */
    measure_t                ** gppMeasureAux2Array;      /** */

    boost::dynamic_bitset<>  ** gppBatchIndexesArray;
    boost::dynamic_bitset<>   * gpPsBatchIndexesArray;
    boost::dynamic_bitset<>   * gpPtBatchIndexesArray;
    boost::dynamic_bitset<>  ** gppPositiveBatchIndexesArray;
    boost::dynamic_bitset<>  * gpPsPositiveBatchIndexesArray;
    boost::dynamic_bitset<>  * gpPtPositiveBatchIndexesArray;

    measure_t                 * gpPTMeasureAuxArray;      /** */
    measure_t                 * gpPTMeasureAux2Array;      /** */
    measure_t                 * gpPSMeasureAuxArray;     /** spatial */
    measure_t                 * gpPSMeasureAux2Array;     /** spatial */
    const AbstractTimeTrend   * gpTimeTrend;            /** pointer to dataset time trend structure */

    inline count_t                    ** GetCaseArray() const {return gppCaseArray;}
    inline count_t                    ** GetNCCaseArray() const {return gppNCCaseArray;}
    inline const CategoryCasesArrays_t & GetCategoryCaseArrays() const {return gvCategoryCaseArrays;}
    inline count_t                     * GetPSCaseArray() const {return gpPSCaseArray;}
    inline count_t                     * GetPTCaseArray() const {return gpPTCaseArray;}
    inline count_t                    ** GetPTCategoryCaseArray() const {return gppPTCategoryCaseArray;}
    inline measure_t                  ** GetMeasureArray() const {return gppMeasureArray;}
    inline measure_t                  ** GetNCMeasureArray() const {return gppNCMeasureArray;}
    unsigned int                         GetNumOrdinalCategories() const {return giNumOrdinalCategories;}
    unsigned int                         GetNumTimeIntervals() const {return giNumTimeIntervals;}
    inline measure_t                   * GetPSMeasureArray() const {return gpPSMeasureArray;}
    inline measure_t                   * GetPTMeasureArray() const {return gpPTMeasureArray;}
    inline measure_t                  ** GetMeasureAuxArray() const {return gppMeasureAuxArray;}
    inline measure_t                  ** GetMeasureAux2Array() const { return gppMeasureAux2Array; }
    inline measure_t                   * GetPSMeasureAuxArray() const {return gpPSMeasureAuxArray;}
    inline measure_t                   * GetPSMeasureAux2Array() const { return gpPSMeasureAux2Array; }
    inline measure_t                   * GetPTMeasureAuxArray() const {return gpPTMeasureAuxArray;}
    inline measure_t                   * GetPTMeasureAux2Array() const { return gpPTMeasureAux2Array; }

    boost::dynamic_bitset<>           ** getBatchIndexesArray() const { return gppBatchIndexesArray; }
    boost::dynamic_bitset<>            * getPsBatchIndexesArray() const { return gpPsBatchIndexesArray; }
    boost::dynamic_bitset<>            * getPtBatchIndexesArray() const { return gpPtBatchIndexesArray; }
    boost::dynamic_bitset<>           ** getPositiveBatchIndexesArray() const { return gppPositiveBatchIndexesArray; }
    boost::dynamic_bitset<>            * getPsPositiveBatchIndexesArray() const { return gpPsPositiveBatchIndexesArray; }
    boost::dynamic_bitset<>            * getPtPositiveBatchIndexesArray() const { return gpPtPositiveBatchIndexesArray; }

    void                                 setBatchIndexesArray(boost::dynamic_bitset<>** pp) { gppBatchIndexesArray = pp; gpPsBatchIndexesArray = pp[0]; }
    void                                 setPtBatchIndexesArray(boost::dynamic_bitset<>* p) { gpPtBatchIndexesArray = p; }
    void                                 setPositiveBatchIndexesArray(boost::dynamic_bitset<>** pp) { gppPositiveBatchIndexesArray = pp; gpPsPositiveBatchIndexesArray = pp[0]; }
    void                                 setPtPositiveBatchIndexesArray(boost::dynamic_bitset<>* p) { gpPtPositiveBatchIndexesArray = p; }


    inline const AbstractRandomizer    * GetRandomizer() const {return _randomizer;}
    unsigned int                         getDataStartIndex() const { return _add_data_start_index; }
    inline const AbstractTimeTrend     * GetTimeTrend() const {return gpTimeTrend;}
    inline count_t                       GetTotalCasesCount() const {return gTotalCases;}
    inline count_t                       GetTotalControlsCount() const {return gTotalControls;}
    inline measure_t                     GetTotalMeasureCount() const {return gTotalMeasure;}
    inline measure_t                     GetTotalMeasureAuxCount() const {return gTotalMeasureAux;}
    inline measure_t                     GetTotalMeasureAux2Count() const { return gTotalMeasureAux2; }
    void                                 ResetCaseArray(count_t t);
    void                                 SetCaseArray(count_t ** ppCases) {gppCaseArray = ppCases;gpPSCaseArray = ppCases[0];}
    void                                 SetNumOrdinalCategories(unsigned int u) {giNumOrdinalCategories = u;}
    void                                 SetNCCaseArray(count_t ** ppCases) {gppNCCaseArray = ppCases;}
    void                                 SetCategoryCaseArrays(const CasesByCategory_t& vCategoryCases);
    void                                 SetPTCaseArray(count_t * pPTCase) {gpPTCaseArray = pPTCase;}
    void                                 SetPTCategoryCaseArray(count_t ** ppPTCategoryCase) {gppPTCategoryCaseArray = ppPTCategoryCase;}
    void                                 SetMeasureArray(measure_t ** ppMeasure) {gppMeasureArray = ppMeasure;gpPSMeasureArray = ppMeasure[0];}
    void                                 SetNCMeasureArray(measure_t ** ppMeasure) {gppNCMeasureArray = ppMeasure;}
    void                                 SetPTMeasureArray(measure_t * pMeasure) {gpPTMeasureArray = pMeasure;}
    void                                 SetPTMeasureAuxArray(measure_t * pPTMeasureAux) {gpPTMeasureAuxArray = pPTMeasureAux;}
    void                                 SetPTMeasureAux2Array(measure_t* pPTMeasureAux) { gpPTMeasureAux2Array = pPTMeasureAux; }
    void                                 SetMeasureAuxArray(measure_t ** ppMeasureAux) {gppMeasureAuxArray = ppMeasureAux;gpPSMeasureAuxArray = ppMeasureAux[0];}
    void                                 SetMeasureAux2Array(measure_t** ppMeasureAux) { gppMeasureAux2Array = ppMeasureAux; gpPSMeasureAux2Array = ppMeasureAux[0]; }
    void                                 SetRandomizer(const AbstractRandomizer& randomizer) {_randomizer = &randomizer;}
    void                                 SetTimeTrend(const AbstractTimeTrend * pTimeTrend) {gpTimeTrend = pTimeTrend;}
    void                                 SetTotalCasesCount(count_t tCases) {gTotalCases = tCases;}
    void                                 SetTotalControlsCount(count_t tControls) {gTotalControls = tControls;}
    void                                 SetTotalMeasureCount(measure_t tMeasure) {gTotalMeasure = tMeasure;}
    void                                 SetTotalMeasureAuxCount(measure_t tMeasureAux) {gTotalMeasureAux = tMeasureAux;}
    void                                 SetTotalMeasureAux2Count(measure_t tMeasureAux) { gTotalMeasureAux2 = tMeasureAux; }
};
//******************************************************************************
#endif

