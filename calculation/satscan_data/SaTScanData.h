//*****************************************************************************
#ifndef __SATSCANDATA_H
#define __SATSCANDATA_H
//*****************************************************************************
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
#include "MaxWindowLengthIndicator.h"
#include "DataStreamHandler.h"

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
    DataStreamHandler                         * gpDataStreams;

    GInfo                                     * gpGInfo;
    TractHandler                              * gpTInfo;
    TwoDimensionArrayHandler<tract_t>         * gpNeighborCountHandler;
    ThreeDimensionArrayHandler<tract_t>       * gpSortedIntHandler;
    ThreeDimensionArrayHandler<unsigned short>* gpSortedUShortHandler;
    double                                      m_nMaxCircleSize;
    double                                      m_nMaxReportedCircleSize;
    double                                      m_nAnnualRatePop;
    double                                    * mdE_Angles;
    double                                    * mdE_Shapes;                     /* temp storage for the angles, shapes of each "possible" ellipsoid */
    Julian                                      m_nStartDate;
    Julian                                      m_nEndDate;       /* study period start/end dates */
    Julian                                    * m_pIntervalStartTimes;          /* time interval start times */
    std::vector<measure_t>                      gvCircleMeasure;
    tract_t                                     m_nTracts;
    measure_t                                   m_nTotalTractsAtStart;
    measure_t                                   m_nTotalMaxCirclePopulation;    /** total population as defined in max circle population file */
    measure_t                                   gtTotalMeasure;                 /** total measure for all data streams */
    count_t                                     gtTotalCases;                   /** total cases for all data streams */
    measure_t                                   gtTotalPopulation;              /** total population for all streams */

    bool                                        AdjustMeasure(measure_t ** pNonCumulativeMeasure, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate);
    void                                        AllocateNeighborArray();
    void                                        AllocateSortedArray();
    measure_t                                   CalcMeasureForTimeInterval(PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate);
    bool                                        ConvertAdjustmentDateToJulian(StringParser & Parser, Julian & JulianDate, bool bStartDate);
    measure_t                                   DateMeasure(PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract);    
    count_t                                     GetCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const;
    int                                         LowerPopIndex(Julian Date) const;
    bool                                        ReadAdjustmentsByRelativeRisksFile(measure_t ** pNonCumulativeMeasure);
    bool                                        ReadCartesianCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                         int & iScanCount, int iWordOffSet, const char * sSourceFile);
    bool                                        ReadCoordinatesFileAsCartesian(FILE * fp);
    bool                                        ReadCoordinatesFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadGridFileAsCartiesian(FILE * fp);
    bool                                        ReadGridFileAsLatitudeLongitude(FILE * fp);
    bool                                        ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                                                 int iWordOffSet, const char * sSourceFile);
    virtual void                                SetAdditionalCaseArrays(DataStream & thisStream);
    virtual void                                SetIntervalCut();
    virtual void                                SetIntervalStartTimes();
    virtual void                                SetMaxTemporalWindowLengthIndicator();
    void                                        SetMeasureByTimeIntervalArray();
    void                                        SetMeasureByTimeIntervalArray(measure_t ** pNonCumulativeMeasure);
    virtual void                                SetNumTimeIntervals();
    void                                        SetProspectiveIntervalStart();
    void                                        SetPurelyTemporalCases();
    void                                        SetScanningWindowEndRangeIndex(Julian EndRangeDate, int & iEndRangeDateIndex);
    void                                        SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex);
    void                                        SetStartAndEndDates();
    void                                        SetTimeIntervalRangeIndexes();
    virtual void                                ValidateObservedToExpectedCases(count_t ** ppCumulativeCases, measure_t ** ppNonCumulativeMeasure) const;

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

    virtual void                                AdjustNeighborCounts(); // For sequential analysis, after top cluster removed
    virtual void                                AllocateSimulationStructures();
    virtual bool                                CalculateMeasure(DataStream & thisStream);
    bool                                        CalculateExpectedCases();
    int                                         ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate);
    virtual void                                DisplayNeighbors(FILE* pFile);
    virtual void                                DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput);
    void                                        DisplaySummary(FILE* fp);
    void                                        DisplaySummary2(FILE* fp);
    virtual bool                                FindNeighbors(bool bSimulations);
    const double                              * GetAnglesArray() const {return mdE_Angles;}
    DataStreamHandler                         & GetDataStreamHandler() {return *gpDataStreams;}
    const DataStreamHandler                   & GetDataStreamHandler() const {return *gpDataStreams;}
    inline const GInfo                        * GetGInfo() const { return gpGInfo;}
    double                                      GetMaxCircleSize() {return m_nMaxCircleSize;}
    double                                      GetMeasureAdjustment() const;
    virtual tract_t                             GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    inline tract_t                           ** GetNeighborCountArray() {return gpNeighborCountHandler->GetArray();}
    inline tract_t                           ** GetNeighborCountArray() const {return gpNeighborCountHandler->GetArray();}
    inline size_t                               GetNumDataStreams() const {return gpDataStreams->GetNumStreams();}
    inline int                                  GetNumTimeIntervals() const {return m_nTimeIntervals;}
    inline tract_t                              GetNumTracts() const {return m_nTracts;}
    const CParameters                         & GetParameters() const {return *m_pParameters;}
    CModel                                    & GetProbabilityModel() const {return *m_pModel;}
    const double                              * GetShapesArray() const {return mdE_Shapes;}
    Julian                                      GetStudyPeriodEndDate() const {return m_nEndDate;}
    Julian                                      GetStudyPeriodStartDate() const {return m_nStartDate;}
    int                                         GetTimeIntervalOfDate(Julian Date) const;
    const Julian                              * GetTimeIntervalStartTimes() const {return m_pIntervalStartTimes;}
    inline const TractHandler                 * GetTInfo() const { return gpTInfo;}
    double                                      GetTotalPopulationCount() const {return gtTotalPopulation;}
    virtual void                                MakeData(int iSimulationNumber, DataStreamGateway & DataGateway);
    bool                                        ReadBernoulliData();
    bool                                        ReadCoordinatesFile();
    virtual void                                ReadDataFromFiles();
    bool                                        ReadGridFile();
    bool                                        ReadMaxCirclePopulationFile();
    bool                                        ReadPoissonData();
    bool                                        ReadSpaceTimePermutationData();
    void                                        RemoveTractSignificance(tract_t tTractIndex);
    void                                        SetMaxCircleSize();

    inline measure_t                            GetTotalDataStreamMeasure(unsigned int iStream) const {return gpDataStreams->GetStream(iStream).GetTotalMeasure();}
    inline measure_t                            GetTotalMeasure() const {return gtTotalMeasure;}
    inline count_t                              GetTotalCases() const {return gtTotalCases;}
    inline count_t                              GetTotalDataStreamCases(unsigned int iStream) const {return gpDataStreams->GetStream(iStream).GetTotalCases();}

    double                                      GetAnnualRate() const;
    double                                      GetAnnualRateAtStart() const;
    double                                      GetAnnualRatePop() const {return m_nAnnualRatePop;}

    virtual void                                DisplayCases(FILE* pFile);
    virtual void                                DisplayControls(FILE* pFile);
    virtual void                                DisplayMeasure(FILE* pFile);
    virtual void                                DisplaySimCases(FILE* pFile);
};
//*****************************************************************************
#endif
