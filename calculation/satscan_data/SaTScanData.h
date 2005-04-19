//******************************************************************************
#ifndef __SATSCANDATA_H
#define __SATSCANDATA_H
//******************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "Parameters.h"
#include "JulianDates.h"
#include "ProbabilityModel.h"
#include "GridTractCoordinates.h"
#include "UtilityFunctions.h"
#include "CalculateMeasure.h"
#include "MakeNeighbors.h"
#include "TimeTrend.h"
#include "MultipleDimensionArrayHandler.h"
#include "DataSetHandler.h"
#include "AdjustmentHandler.h"

/** Central data hub class which contains all data either read or created from
    input files. Defines public interface for reading and accessing contained data. */
class CSaTScanData {
  private:
    void                                        Init();
    virtual void                                SetProbabilityModel() = 0;
    void                                        Setup();

  protected:
    BasePrint                                 & gPrint;
    const CParameters                         & gParameters;
    CModel                                    * m_pModel;
    DataSetHandler                            * gpDataSets;

    GInfo                                       gCentroidsHandler;
    TractHandler                                gTractHandler;
    TwoDimensionArrayHandler<tract_t>         * gpNeighborCountHandler;
    ThreeDimensionArrayHandler<tract_t>       * gpSortedIntHandler;
    ThreeDimensionArrayHandler<unsigned short>* gpSortedUShortHandler;
    double                                      m_nMaxCircleSize;
    double                                      m_nMaxReportedCircleSize;
    double                                      m_nAnnualRatePop;
    std::vector<double>                         gvEllipseAngles;
    std::vector<double>                         gvEllipseShapes;                /* temp storage for the angles, shapes of each "possible" ellipsoid */
    const Julian                                m_nStartDate;
    const Julian                                m_nEndDate;                     /* study period start/end dates */
    std::vector<Julian>                         gvTimeIntervalStartTimes;       /* time interval start times */
    std::vector<measure_t>                      gvMaxCirclePopulation;          /* population by locations */
    tract_t                                     m_nTracts;
    measure_t                                   m_nTotalTractsAtStart;
    measure_t                                   m_nTotalMaxCirclePopulation;    /** total population as defined in gvMaxCirclePopulation */
    measure_t                                   gtTotalMeasure;                 /** total measure for all data sets */
    count_t                                     gtTotalCases;                   /** total cases for all data sets */
    measure_t                                   gtTotalPopulation;              /** total population for all sets */
    RelativeRiskAdjustmentHandler               gRelativeRiskAdjustments;
    std::vector<int>                            gvProspectiveIntervalCuts;
    int                                         m_nProspectiveIntervalStart; // interval where start of prospective space-time begins
    int                                         m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)
    int                                         m_nFlexibleWindowStartRangeStartIndex;
    int                                         m_nFlexibleWindowStartRangeEndIndex;
    int                                         m_nFlexibleWindowEndRangeStartIndex;
    int                                         m_nFlexibleWindowEndRangeEndIndex;
    std::vector<tract_t>                        gvNullifiedLocations;

    bool                                        AdjustMeasure(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate);
    measure_t                                   CalcMeasureForTimeInterval(PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate);
    int                                         CalculateProspectiveIntervalStart() const;
    void                                        CalculateTimeIntervalIndexes();
    measure_t                                   DateMeasure(PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract);
    count_t                                     GetCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const;
    int                                         LowerPopIndex(Julian Date) const;
    bool                                        ReadAdjustmentsByRelativeRisksFile();
    bool                                        ReadCartesianCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                         int & iScanCount, int iWordOffSet);
    bool                                        ReadCoordinatesFileAsCartesian(FILE * fp);
    bool                                        ReadCoordinatesFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadGridFileAsCartiesian(FILE * fp);
    bool                                        ReadGridFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                                 int iWordOffSet, const char * sSourceFile);
    virtual void                                SetAdditionalCaseArrays(RealDataSet & thisSet);
    virtual void                                SetIntervalCut();
    virtual void                                SetIntervalStartTimes();
    void                                        SetMeasureByTimeIntervalArray();
    void                                        SetMeasureByTimeIntervalArray(measure_t ** pNonCumulativeMeasure);
    void                                        SetPurelyTemporalCases();
    void                                        SetTimeIntervalRangeIndexes();

  public:
    CSaTScanData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CSaTScanData();


    void                                        AllocateSortedArray();
    
    tract_t                                     m_nGridTracts;
    int                                         m_nTimeIntervals;

    virtual void                                AdjustForKnownRelativeRisks(RealDataSet& DataSet, measure_t ** ppNonCumulativeMeasure);
    virtual void                                AdjustNeighborCounts(); // For sequential analysis, after top cluster removed
    virtual void                                CalculateMeasure(RealDataSet& thisSet);
    void                                        CalculateExpectedCases();
    bool                                        ConvertAdjustmentDateToJulian(StringParser& Parser, Julian& JulianDate, bool bStartDate);
    virtual void                                DisplayNeighbors(FILE* pFile);
    virtual void                                DisplayRelativeRisksForEachTract() const;
    void                                        DisplaySummary(FILE* fp);
    void                                        DisplaySummary2(FILE* fp);
    virtual void                                FindNeighbors(bool bSimulations);
    void                                        FreeRelativeRisksAdjustments() {gRelativeRiskAdjustments.Empty();}
    DataSetHandler                            & GetDataSetHandler() {return *gpDataSets;}
    const DataSetHandler                      & GetDataSetHandler() const {return *gpDataSets;}
    double                                      GetEllipseAngle(int iEllipseIndex) const;
    double                                      GetEllipseShape(int iEllipseIndex) const;
    int                                         GetFlexibleWindowEndRangeEndIndex() const {return m_nFlexibleWindowEndRangeEndIndex;}
    int                                         GetFlexibleWindowEndRangeStartIndex() const {return m_nFlexibleWindowEndRangeStartIndex;}
    int                                         GetFlexibleWindowStartRangeEndIndex() const {return m_nFlexibleWindowStartRangeEndIndex;}
    int                                         GetFlexibleWindowStartRangeStartIndex() const {return m_nFlexibleWindowStartRangeStartIndex;}
    inline const GInfo                        * GetGInfo() const { return &gCentroidsHandler;}
    bool                                        GetIsNullifiedLocation(tract_t tLocationIndex) const;
    const std::vector<measure_t>              & GetMaxCirclePopulationArray() const {return gvMaxCirclePopulation;}
    double                                      GetMaxCircleSize() const {return m_nMaxCircleSize;}
    double                                      GetMaxReportedCircleSize() const {return m_nMaxReportedCircleSize;}
    double                                      GetMeasureAdjustment(size_t iSetIndex) const;
    inline virtual tract_t                      GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    inline tract_t                           ** GetNeighborCountArray() {return gpNeighborCountHandler->GetArray();}
    inline tract_t                           ** GetNeighborCountArray() const {return gpNeighborCountHandler->GetArray();}
    inline size_t                               GetNumDataSets() const {return gpDataSets->GetNumDataSets();}
    inline int                                  GetNumTimeIntervals() const {return m_nTimeIntervals;}
    inline tract_t                              GetNumTracts() const {return m_nTracts;}
    const CParameters                         & GetParameters() const {return gParameters;}
    CModel                                    & GetProbabilityModel() const {return *m_pModel;}
    const std::vector<int>                    & GetProspectiveIntervalCuts() const {return gvProspectiveIntervalCuts;}
    int                                         GetProspectiveStartIndex() const {return m_nProspectiveIntervalStart;}  
    Julian                                      GetStudyPeriodEndDate() const {return m_nEndDate;}
    Julian                                      GetStudyPeriodStartDate() const {return m_nStartDate;}
    int                                         GetTimeIntervalOfDate(Julian Date) const;
    int                                         GetTimeIntervalOfEndDate(Julian EndDate) const;
    int                                         GetTimeIntervalCut() const {return m_nIntervalCut;}
    const std::vector<Julian>                 & GetTimeIntervalStartTimes() const {return gvTimeIntervalStartTimes;}
    inline const TractHandler                 * GetTInfo() const {return &gTractHandler;}
    double                                      GetTotalPopulationCount() const {return gtTotalPopulation;}
    virtual void                                RandomizeData(RandomizerContainer_t& RandomizerContainer, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    bool                                        ReadBernoulliData();
    bool                                        ReadCoordinatesFile();
    virtual void                                ReadDataFromFiles();
    bool                                        ReadExponentialData();
    bool                                        ReadGridFile();
    bool                                        ReadMaxCirclePopulationFile();
    bool                                        ReadNormalData();
    bool                                        ReadOrdinalData();
    bool                                        ReadPoissonData();
    bool                                        ReadRankData();
    bool                                        ReadSpaceTimePermutationData();
    void                                        RemoveTractSignificance(tract_t tTractIndex);
    void                                        SetMaxCircleSize();
    virtual void                                ValidateObservedToExpectedCases(count_t ** ppCumulativeCases, measure_t ** ppNonCumulativeMeasure) const;

    inline measure_t                            GetTotalDataSetMeasure(size_t iSetIndex) const {return gpDataSets->GetDataSet(iSetIndex).GetTotalMeasure();}
    inline measure_t                            GetTotalMeasure() const {return gtTotalMeasure;}
    inline count_t                              GetTotalCases() const {return gtTotalCases;}
    inline count_t                              GetTotalDataSetCases(size_t iSetIndex) const {return gpDataSets->GetDataSet(iSetIndex).GetTotalCases();}
    double                                      GetAnnualRateAtStart(size_t iSetIndex) const;
    double                                      GetAnnualRatePop() const {return m_nAnnualRatePop;}

    virtual void                                DisplayCases(FILE* pFile);
    virtual void                                DisplayControls(FILE* pFile);
    virtual void                                DisplayMeasure(FILE* pFile);
    virtual void                                DisplaySimCases(FILE* pFile);

    inline tract_t                           ** GetSortedArrayAsTract_T(int iEllipse) const;
    inline unsigned short                    ** GetSortedArrayAsUShort_T(int iEllipse) const;
};

inline tract_t ** CSaTScanData::GetSortedArrayAsTract_T(int iEllipse) const {
  return (gpSortedIntHandler ? gpSortedIntHandler->GetArray()[iEllipse] : 0);
}

inline unsigned short ** CSaTScanData::GetSortedArrayAsUShort_T(int iEllipse) const {
  return (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray()[iEllipse] : 0);
}

/** Return "nearness"-th closest neighbor to "t" (nearness == 1 returns "t"). */
inline tract_t CSaTScanData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
  if (gpSortedUShortHandler)
    return (tract_t)gpSortedUShortHandler->GetArray()[iEllipse][t][nearness - 1];
  else
    return gpSortedIntHandler->GetArray()[iEllipse][t][nearness - 1];
}
//*****************************************************************************
#endif
