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

class CModel;
class CPoissonModel;
class CBernoulliModel;
class CSpaceTimePermutationModel;

class CSaTScanData {
  friend class CModel;
  friend class CPoissonModel;
  friend class CBernoulliModel;
  friend class CSpaceTimePermutationModel;

  private:
    void                        Init();
    virtual void                SetProbabilityModel() = 0;
    void                        Setup(CParameters* pParameters, BasePrint *pPrintDirection);

  protected:
    BasePrint                 * gpPrint;
    double                      m_nAnnualRatePop;
    PopulationCategories        gPopulationCategories;
    std::vector<measure_t>      gvCircleMeasure;

    void                        AllocateCountStructure(count_t***  pCounts);
    void                        AllocateNeighborArray();
    void                        AllocateSortedArray();
    void                        DeallocateCountStructure(count_t***  pCounts);
    bool                        ReadCartesianCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                         int & iScanCount, int iWordOffSet, long lRecNum, const char * sSourceFile);
    bool                        ReadCoordinatesFileAsCartesian(FILE * fp);
    bool                        ReadCoordinatesFileAsLatitudeLongitude(FILE * fp);
    bool                        ReadCounts(FILE * fp, const char* szDescription, count_t**  pCounts);
    bool                        ReadGridFileAsCartiesian(FILE * fp);
    bool                        ReadGridFileAsLatitudeLongitude(FILE * fp);
    bool                        ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                 int iWordOffSet, long lRecNum, const char * sSourceFile);
    virtual void                SetAdditionalCaseArrays();
    virtual void                SetCasesByTimeIntervalArray();
    void                        SetCumulativeMeasure();
    virtual void                SetIntervalCut();
    virtual void                SetIntervalStartTimes();
    virtual void                SetMeasureAsCumulative(measure_t ** pMeasure);
    void                        SetMeasureByTimeIntervalArray();
    void                        SetMeasureByTimeIntervalArray(measure_t ** pNonCumulativeMeasure);
    void                        SetNonCumulativeMeasure();
    virtual void                SetNumTimeIntervals();
    void                        SetProspectiveIntervalStart();
    void                        SetPurelyTemporalCases();
    void                        SetPurelyTemporalMeasures();
    void                        SetScanningWindowEndRangeIndex(Julian EndRangeDate, int & iEndRangeDateIndex);
    void                        SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex);
    void                        SetStartAndEndDates();
    void                        SetTimeIntervalRangeIndexes();

  public:
    CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSaTScanData();

    CParameters               * m_pParameters;
    CModel                    * m_pModel;
    TractHandler              * gpTInfo;
    GInfo                     * gpGInfo;
    tract_t                     m_nGridTracts, m_nTracts;  // tract_t defined as short in SaTScan.h ?Ever more than 32,000 tracts?
    short                       m_nNumEllipsoids;
    double                    * mdE_Angles,
                              * mdE_Shapes;  //temp storage for the angles, shapes of each "possible" ellipsoid...
    int                         m_nTimeIntervals;
    Julian                    * m_pIntervalStartTimes; // Not nec. for purely spatial?
    Julian                      m_nStartDate, m_nEndDate;
    int                         m_nStartRangeStartDateIndex, m_nStartRangeEndDateIndex;
    int                         m_nEndRangeStartDateIndex, m_nEndRangeEndDateIndex;
    int                         m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)
    int                         m_nProspectiveIntervalStart; // interval where start of prospective space-time begins
    count_t                     m_nTotalCasesAtStart, m_nTotalControlsAtStart;
    measure_t                   m_nTotalMeasureAtStart, m_nTotalTractsAtStart;
    count_t                     m_nTotalCases, m_nTotalControls;
    double                      m_nTotalPop, m_nMaxCircleSize, m_nMaxReportedCircleSize;
    measure_t                   m_nTotalMeasure;
    count_t                  ** m_pCases,
                             ** m_pControls,
                             ** m_pSimCases;
    measure_t                ** m_pMeasure;
    count_t                   * m_pPTCases, *m_pPTSimCases;
    measure_t                 * m_pPTMeasure;

    count_t                  ** m_pCases_NC;        // Non-cumulative array of cases
    count_t                  ** m_pSimCases_NC;     // Non-cumulative array of simulated cases
    measure_t                ** m_pMeasure_NC;      // Non-cumulative array of measures
    count_t                   * m_pCases_TotalByTimeInt;     // Cases for all tracts by interval
    count_t                   * m_pSimCases_TotalByTimeInt;  // Sim Cases for all tracts by interval
    measure_t                 * m_pMeasure_TotalByTimeInt;   // Measure for all tracts by interval
    CTimeTrend                  m_nTimeTrend;     // Time trend for whole dataset
    CTimeTrend                  m_nTimeTrend_Sim;  // Time trend for simulated dataset

    tract_t                   * (** m_pSortedInt);
    unsigned short            * (** m_pSortedUShort);
    tract_t                  ** m_NeighborCounts;
    Julian                   ** m_pTimes; // Used for exact times...

    virtual void                AllocSimCases();
    virtual bool                CalculateMeasure();
    virtual void                DeAllocSimCases();
    virtual bool                FindNeighbors(bool bSimulations);
    virtual void                MakeData(int iSimulationNumber);
    virtual void                ReadDataFromFiles();

    virtual void                AdjustNeighborCounts(); // For sequential analysis, after top cluster removed
    const PopulationCategories& GetPopulationCategories() const {return gPopulationCategories;}
    int                                 ComputeNewCutoffInterval(Julian jStartDate, Julian& jEndDate);
    bool                        ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate);
    virtual void                DisplayCases(FILE* pFile);
    virtual void                DisplayControls(FILE* pFile);
    virtual void                DisplayMeasure(FILE* pFile);
    virtual void                DisplayNeighbors(FILE* pFile);
    virtual void                DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput);
    virtual void                DisplaySimCases(FILE* pFile);
    void                        DisplaySummary(FILE* fp);
    void                        DisplaySummary2(FILE* fp);
    double                      GetAnnualRate() const;
    double                      GetAnnualRateAtStart() const;
    double                      GetAnnualRatePop() const {return m_nAnnualRatePop;};
    double                      GetMaxCircleSize() {return m_nMaxCircleSize;};
    double                      GetMeasureAdjustment() const;
    virtual tract_t             GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    inline const                TractHandler *GetTInfo() const { return gpTInfo;} ;          // DTG
    inline const GInfo        * GetGInfo() const { return gpGInfo;} ;
    void                        IncrementCount(tract_t nTID, int nCount, Julian nDate, count_t** pCounts);
    bool                        ParseCountLine(const char*  szDescription, int nRec, StringParser & Parser, tract_t& tid, count_t& nCount, Julian& nDate);
    bool                        ParseCountLineCategories(const char*   szDescription, const int     nRec, char*   szData, const int     nCats,
                                                         const int     nDataElements, const tract_t tid, const count_t nCount, const Julian  nDate);
    virtual void                PrintNeighbors();
    bool                        ReadCaseFile();
    bool                        ReadControlFile();
    bool                        ReadCoordinatesFile();
    bool                        ReadGridFile();
    bool                        ReadMaxCirclePopulationFile();
    bool                        ReadPopulationFile();
    void                        SetPurelyTemporalSimCases();
    void                        SetMaxCircleSize();
};

//*****************************************************************************
#endif
