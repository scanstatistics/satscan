//*****************************************************************************
#ifndef __SATSCANDATA_H
#define __SATSCANDATA_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "Parameters.h"
#include "JulianDates.h"
#include "ProbabilityModel.h"
#include "PopulationCategories.h"
#include "GridTractCoordinates.h"
#include "UtilityFunctions.h"
#include "CalculateMeasure.h"
#include "MakeNeighbors.h"
#include "TimeTrend.h"
#include "MultipleDimensionArrayHandler.h"
#include "MaxWindowLengthIndicator.h"

class CPoissonModel;
class CBernoulliModel;
class CSpaceTimePermutationModel;

class CSaTScanData {
  friend class CPoissonModel;
  friend class CBernoulliModel;
  friend class CSpaceTimePermutationModel;

  private:
    void                                        Init();
    virtual void                                SetProbabilityModel() = 0;
    void                                        Setup(CParameters* pParameters, BasePrint *pPrintDirection);

  protected:
    BasePrint                                 * gpPrint;
    CParameters                               * m_pParameters;
    CModel                                    * m_pModel;
    GInfo                                     * gpGInfo;
    TractHandler                              * gpTInfo;
    double                                      m_nTotalPop,
                                                m_nMaxCircleSize,
                                                m_nMaxReportedCircleSize,
                                                m_nAnnualRatePop,
                                              * mdE_Angles,
                                              * mdE_Shapes;                     /* temp storage for the angles, shapes of each "possible" ellipsoid */
    Julian                                      m_nStartDate, m_nEndDate,       /* study period start/end dates */
                                              * m_pIntervalStartTimes;          /* time interval start times */
    PopulationCategories                        gPopulationCategories;
    std::vector<measure_t>                      gvCircleMeasure;
    tract_t                                     m_nTracts;
    TwoDimensionArrayHandler<count_t>         * gpCasesHandler,
                                              * gpCasesNonCumulativeHandler,    /* Non-cumulative array of cases */
                                              * gpCasesByTimeByCategoryHandler,
                                              * gpControlsHandler,
                                              * gpControlsByTimeByCategoryHandler,
                                              * gpSimCasesHandler,
                                              * gpSimCasesNonCumulativeHandler; /* Non-cumulative array of cases */
    count_t                                     m_nTotalCases,
                                                m_nTotalControls,
                                                m_nTotalCasesAtStart,
                                                m_nTotalControlsAtStart,
                                              * m_pCases_TotalByTimeInt,        /* Cases for all tracts by interval */
                                              * m_pSimCases_TotalByTimeInt,     /* Sim Cases for all tracts by interval */
                                              * m_pPTCases,
                                              * m_pPTSimCases;
    TwoDimensionArrayHandler<tract_t>         * gpNeighborCountHandler;
    TwoDimensionArrayHandler<measure_t>       * gpMeasureHandler,
                                              * gpMeasureNonCumulativeHandler,
                                              * gpMeasureByTimeByCategoryHandler;
    measure_t                                   m_nTotalMeasure,
                                                m_nTotalMeasureAtStart,
                                                m_nTotalTractsAtStart,
                                              * m_pMeasure_TotalByTimeInt,      /* Measure for all tracts by interval */
                                              * m_pPTMeasure;
    ThreeDimensionArrayHandler<tract_t>       * gpSortedIntHandler;
    ThreeDimensionArrayHandler<unsigned short>* gpSortedUShortHandler;
    ThreeDimensionArrayHandler<count_t>       * gpCategoryCasesHandler,
                                              * gpCategoryControlsHandler;
    ThreeDimensionArrayHandler<measure_t>     * gpCategoryMeasureHandler;

    void                                        AllocateCaseStructures();
    void                                        AllocateControlStructures();
    void                                        AllocateNeighborArray();
    void                                        AllocateSortedArray();
    bool                                        ConvertCountDateToJulian(StringParser & Parser, const char * szDescription, Julian & JulianDate);
    bool                                        ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate);
    bool                                        ReadCartesianCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                         int & iScanCount, int iWordOffSet, const char * sSourceFile);
    bool                                        ReadCoordinatesFileAsCartesian(FILE * fp);
    bool                                        ReadCoordinatesFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadCounts(FILE * fp, const char* szDescription);
    bool                                        ReadGridFileAsCartiesian(FILE * fp);
    bool                                        ReadGridFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                                 int iWordOffSet, const char * sSourceFile);
    virtual void                                SetAdditionalCaseArrays();
    virtual void                                SetCasesByTimeIntervalArray();
    void                                        SetCumulativeMeasure();
    virtual void                                SetIntervalCut();
    virtual void                                SetIntervalStartTimes();
    virtual void                                SetMaxTemporalWindowLengthIndicator();
    virtual void                                SetMeasureAsCumulative(measure_t ** pMeasure);
    void                                        SetMeasureByTimeIntervalArray();
    void                                        SetMeasureByTimeIntervalArray(measure_t ** pNonCumulativeMeasure);
    void                                        SetNonCumulativeMeasure();
    virtual void                                SetNumTimeIntervals();
    void                                        SetProspectiveIntervalStart();
    void                                        SetPurelyTemporalCases();
    void                                        SetPurelyTemporalMeasures();
    void                                        SetScanningWindowEndRangeIndex(Julian EndRangeDate, int & iEndRangeDateIndex);
    void                                        SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex);
    void                                        SetStartAndEndDates();
    void                                        SetTimeIntervalRangeIndexes();

  public:
    CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSaTScanData();

    AbstractMaxWindowLengthIndicator          * gpMaxWindowLengthIndicator;   

    tract_t                                     m_nGridTracts;
    int                                         m_nTimeIntervals,
                                                m_nStartRangeStartDateIndex,
                                                m_nStartRangeEndDateIndex,
                                                m_nEndRangeStartDateIndex,
                                                m_nEndRangeEndDateIndex,
                                                m_nIntervalCut, // Maximum time intervals allowed in a cluster (base on TimeSize)
                                                m_nProspectiveIntervalStart; // interval where start of prospective space-time begins
    CTimeTrend                                  m_nTimeTrend,     // Time trend for whole dataset
                                                m_nTimeTrend_Sim;  // Time trend for simulated dataset

    virtual void                                AdjustNeighborCounts(); // For sequential analysis, after top cluster removed
    virtual void                                AllocSimCases();
    virtual bool                                CalculateMeasure();
    virtual void                                DeAllocSimCases();
    const PopulationCategories                & GetPopulationCategories() const {return gPopulationCategories;}
    int                                         ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate);
    virtual void                                DisplayCases(FILE* pFile);
    virtual void                                DisplayControls(FILE* pFile);
    virtual void                                DisplayMeasure(FILE* pFile);
    virtual void                                DisplayNeighbors(FILE* pFile);
    virtual void                                DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput);
    virtual void                                DisplaySimCases(FILE* pFile);
    void                                        DisplaySummary(FILE* fp);
    void                                        DisplaySummary2(FILE* fp);
    virtual bool                                FindNeighbors(bool bSimulations);
    const double                              * GetAnglesArray() const {return mdE_Angles;}
    double                                      GetAnnualRate() const;
    double                                      GetAnnualRateAtStart() const;
    double                                      GetAnnualRatePop() const {return m_nAnnualRatePop;}
    inline count_t                           ** GetCasesArray() {return gpCasesHandler->GetArray();}
    inline count_t                           ** GetCasesArray() const {return gpCasesHandler->GetArray();}
    const count_t                             * GetCasesByTimeInterlalArray() const {return m_pCases_TotalByTimeInt;}
    inline count_t                           ** GetCasesNonCumulativeArray() {return gpCasesNonCumulativeHandler->GetArray();}
    inline count_t                           ** GetCasesNonCumulativeArray() const {return gpCasesNonCumulativeHandler->GetArray();}
    const count_t                             * GetCasesPTArray() const {return m_pPTCases;}
    inline count_t                           ** GetControlsArray() {return gpControlsHandler->GetArray();}
    inline count_t                           ** GetControlsArray() const {return gpControlsHandler->GetArray();}
    inline const GInfo                        * GetGInfo() const { return gpGInfo;}
    double                                      GetMaxCircleSize() {return m_nMaxCircleSize;};
    inline measure_t                         ** GetMeasureArray() {return gpMeasureHandler->GetArray();}
    inline measure_t                         ** GetMeasureArray() const {return gpMeasureHandler->GetArray();}
    const measure_t                           * GetMeasureByTimeIntervalArray() const {return m_pMeasure_TotalByTimeInt;}
    inline measure_t                         ** GetMeasureNCArray() {return gpMeasureNonCumulativeHandler->GetArray();}
    inline measure_t                         ** GetMeasureNCArray() const {return gpMeasureNonCumulativeHandler->GetArray();}
    double                                      GetMeasureAdjustment() const;
    const measure_t                           * GetMeasurePTArray() const {return m_pPTMeasure;}
    virtual tract_t                             GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    inline tract_t                           ** GetNeighborCountArray() {return gpNeighborCountHandler->GetArray();}
    inline tract_t                           ** GetNeighborCountArray() const {return gpNeighborCountHandler->GetArray();}
    inline count_t                              GetNumCases() const {return m_nTotalCases;}
    count_t                                     GetNumCategoryCases(int iCategory) const {return gPopulationCategories.GetNumCategoryCases(iCategory);}
    inline count_t                              GetNumControls() const {return m_nTotalControls;}
    inline tract_t                              GetNumTracts() const {return m_nTracts;}
    const CParameters                         & GetParameters() const {return *m_pParameters;}
    CModel                                    & GetProbabilityModel() const {return *m_pModel;}  
    const double                              * GetShapesArray() const {return mdE_Shapes;}
    inline count_t                           ** GetSimCasesArray() {return gpSimCasesHandler->GetArray();}
    inline count_t                           ** GetSimCasesArray() const {return gpSimCasesHandler->GetArray();}
    const count_t                             * GetSimCasesByTimeIntervalArray() const {return m_pSimCases_TotalByTimeInt;}
    inline count_t                           ** GetSimCasesNCArray() {return gpSimCasesNonCumulativeHandler->GetArray();}
    inline count_t                           ** GetSimCasesNCArray() const {return gpSimCasesNonCumulativeHandler->GetArray();}
    const count_t                             * GetSimCasesPTArray() const {return m_pPTSimCases;}
    Julian                                      GetStudyPeriodStartDate() const {return m_nStartDate;}
    const Julian                              * GetTimeIntervalStartTimes() const {return m_pIntervalStartTimes;}
    inline const TractHandler                 * GetTInfo() const { return gpTInfo;}
    inline measure_t                            GetTotalMeasure() const {return m_nTotalMeasure;}
    double                                      GetTotalPopulationCount() const;
    virtual void                                MakeData(int iSimulationNumber);
    bool                                        ParseCountLine(const char*  szDescription, StringParser & Parser, tract_t& tid, count_t& nCount, Julian& nDate, int& iCategoryIndex);    bool                                        ParseCovariates(int& iCategoryIndex, int iCovariatesOffset, const char*  szDescription, int nRec, StringParser & Parser);
    bool                                        ReadCaseFile();
    bool                                        ReadControlFile();
    bool                                        ReadCoordinatesFile();
    virtual void                                ReadDataFromFiles();
    bool                                        ReadGridFile();
    bool                                        ReadMaxCirclePopulationFile();
    void                                        RemoveTractSignificance(tract_t tTractIndex);
    bool                                        ReadPopulationFile();
    void                                        SetPurelyTemporalSimCases();
    void                                        SetMaxCircleSize();
};
//*****************************************************************************
#endif
