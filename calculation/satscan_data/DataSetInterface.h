//******************************************************************************
#ifndef __DataSetInterface_H
#define __DataSetInterface_H
//******************************************************************************
#include "DataSet.h"
#include "Parameters.h"

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
    unsigned int                giNumOrdinalCategories; /** number of ordinal categories in dataset */

    void                        Init();

  public:
    DataSetInterface(unsigned int iNumTimeIntervals=0, unsigned int iNumTracts=0);
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
    measure_t                ** gppSqMeasureArray;      /** */
    measure_t                 * gpPTSqMeasureArray;      /** */
    measure_t                 * gpPSSqMeasureArray;     /** spatial */
    const CTimeTrend          * gpTimeTrend;            /** pointer to dataset time trend structure */

    inline count_t                    ** GetCaseArray() const {return gppCaseArray;}
    inline count_t                    ** GetNCCaseArray() const {return gppNCCaseArray;}
    inline const CategoryCasesArrays_t & GetCategoryCaseArrays() const {return gvCategoryCaseArrays;}
    inline count_t                     * GetPSCaseArray() const {return gpPSCaseArray;}
    inline count_t                     * GetPTCaseArray() const {return gpPTCaseArray;}
    inline count_t                    ** GetPTCategoryCaseArray() const {return gppPTCategoryCaseArray;}
    inline measure_t                  ** GetMeasureArray() const {return gppMeasureArray;}
    inline measure_t                  ** GetNCMeasureArray() const {return gppNCMeasureArray;}
    unsigned int                         GetNumOrdinalCategories() const {return giNumOrdinalCategories;}
    unsigned int                         GetNumTimIntervals() const {return giNumTimeIntervals;}
    inline measure_t                   * GetPSMeasureArray() const {return gpPSMeasureArray;}
    inline measure_t                   * GetPTMeasureArray() const {return gpPTMeasureArray;}
    inline measure_t                  ** GetSqMeasureArray() const {return gppSqMeasureArray;}
    inline measure_t                   * GetPSSqMeasureArray() const {return gpPSSqMeasureArray;}
    inline measure_t                   * GetPTSqMeasureArray() const {return gpPTSqMeasureArray;}
    inline const CTimeTrend            * GetTimeTrend() const {return gpTimeTrend;}
    inline count_t                       GetTotalCasesCount() const {return gTotalCases;}
    inline count_t                       GetTotalControlsCount() const {return gTotalControls;}
    inline measure_t                     GetTotalMeasureCount() const {return gTotalMeasure;}
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
    void                                 SetPTSqMeasureArray(measure_t * pPTSqMeasure) {gpPTSqMeasureArray = pPTSqMeasure;}
    void                                 SetSqMeasureArray(measure_t ** ppSqMeasure) {gppSqMeasureArray = ppSqMeasure;gpPSSqMeasureArray = ppSqMeasure[0];}
    void                                 SetTimeTrend(const CTimeTrend * pTimeTrend) {gpTimeTrend = pTimeTrend;}
    void                                 SetTotalCasesCount(count_t tCases) {gTotalCases = tCases;}
    void                                 SetTotalControlsCount(count_t tControls) {gTotalControls = tControls;}
    void                                 SetTotalMeasureCount(measure_t tMeasure) {gTotalMeasure = tMeasure;}
};
//******************************************************************************
#endif

