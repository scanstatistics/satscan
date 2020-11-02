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
#include "SaTScanDataRead.h"
#include "ClusterLocationsWriter.h"
#include "LocationRelevance.h"
#include "LocationNetwork.h"

/** Central data hub class which contains all data either read or created from
    input files. Defines public interface for reading and accessing contained data. */
class CSaTScanData {
  friend class SaTScanDataReader;
  friend class CentroidNeighborCalculator;
  friend class BernoulliAnalysisDrilldown;

  public:
    typedef boost::shared_ptr<RelativeRiskAdjustmentHandler> RiskAdjustments_t;
    enum           ActiveNeighborReferenceType  {NOT_SET, REPORTED, MAXIMUM};

  private:
    void                                        AllocateSortedArray();
    void                                        AllocateSortedArrayNeighbors(const std::vector<LocationDistance>& vOrderLocations,
                                                                             int iEllipseIndex, tract_t iCentroidIndex,
                                                                             const std::vector<tract_t>& vMaxReported, 
                                                                             tract_t iNumMaximumNeighbors);
    void                                        AllocateSortedArrayNeighbors(tract_t iCentroidIndex, const std::vector<tract_t>& vLocations);
    void                                        Init();
    virtual void                                SetProbabilityModel() = 0;
    void                                        Setup();

  protected:
    BasePrint                                 & gPrint;
    const CParameters                         & gParameters;
    CModel                                    * m_pModel;
    std::auto_ptr<DataSetHandler>               gDataSets;
    ActiveNeighborReferenceType                 geActiveNeighborReferenceType;
    std::auto_ptr<GInfo>                        gCentroidsHandler;
    std::auto_ptr<TractHandler>                 gTractHandler;
	Network                                     _locations_network;
    tract_t                                  ** gppActiveNeighborArray;
    TwoDimensionArrayHandler<tract_t>         * gpReportedNeighborCountHandler;
    TwoDimensionArrayHandler<MinimalGrowthArray<tract_t> > * gpReportedMaximumsNeighborCountHandler;
    TwoDimensionArrayHandler<tract_t>         * gpNeighborCountHandler;
    ThreeDimensionArrayHandler<tract_t>       * gpSortedIntHandler;
    ThreeDimensionArrayHandler<unsigned short>* gpSortedUShortHandler;
    double                                      m_nAnnualRatePop;
    std::vector<double>                         gvEllipseAngles;
    std::vector<double>                         gvEllipseShapes;                /* temp storage for the angles, shapes of each "possible" ellipsoid */
    Julian                                      m_nStartDate;
    Julian                                      m_nEndDate;                     /* study period start/end dates */
    std::vector<Julian>                         gvTimeIntervalStartTimes;       /* time interval start times */
    std::vector<measure_t>                      gvMaxCirclePopulation;          /* population by locations */
    int                                         m_nTimeIntervals;
    tract_t                                     m_nTracts;
    measure_t                                   m_nTotalMaxCirclePopulation;    /** total population as defined in gvMaxCirclePopulation */
    measure_t                                   gtTotalMeasure;                 /** total measure for all data sets */
    measure_t                                   gtTotalMeasureAux;              /** total auxillary measure for all data sets */
    count_t                                     gtTotalCases;                   /** total cases for all data sets */
    measure_t                                   gtTotalPopulation;              /** total population for all sets */
    RiskAdjustments_t                           gRelativeRiskAdjustments;
    std::vector<int>                            gvProspectiveIntervalCuts;
    int                                         m_nProspectiveIntervalStart; // interval where start of prospective space-time begins
    int                                         m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)
    int                                         _min_iterval_cut; // minimum time intervals allowed in a cluster
    int                                         m_nFlexibleWindowStartRangeStartIndex;
    int                                         m_nFlexibleWindowStartRangeEndIndex;
    int                                         m_nFlexibleWindowEndRangeStartIndex;
    int                                         m_nFlexibleWindowEndRangeEndIndex;
    std::vector<tract_t>                        gvNullifiedLocations;
    mutable ptr_vector<CentroidNeighbors>       gvCentroidNeighborStore;
	bool                                        _network_can_report_coordinates;

    int                                         CalculateProspectiveIntervalStart() const;
    void                                        CalculateTimeIntervalIndexes();
    count_t                                     GetCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const;
	virtual void                                PostDataRead();
    virtual void                                RemoveTractSignificance(const CCluster& Cluster, tract_t tTractIndex);
    virtual void                                SetIntervalCut();
    virtual void                                SetIntervalStartTimes();
    void                                        setNeighborCounts(int iEllipseIndex, tract_t iCentroidIndex, const std::vector<tract_t>& vMaxReported, tract_t iNumMaximumNeighbors);
    void                                        SetPurelyTemporalCases();
    virtual void                                SetTimeIntervalRangeIndexes();

  public:
    CSaTScanData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CSaTScanData();

    inline void                                 FreeNeighborInfo(tract_t iCentroidIndex) const;

    tract_t                                     m_nGridTracts;

	Network & getLocationNetwork() {
		return  _locations_network;
	}

	const Network & refLocationNetwork() const {
		return  _locations_network;
	}


    virtual void                                AdjustNeighborCounts(ExecutionType geExecutingType); // For iterative scanning analysis, after top cluster removed
    virtual void                                CalculateMeasure(RealDataSet& thisSet);
    void                                        CalculateExpectedCases();
    virtual void                                DisplayNeighbors(FILE* pFile) const;
    virtual void                                DisplayRelativeRisksForEachTract(const LocationRelevance& location_relevance) const;
    void                                        DisplaySummary(FILE* fp, std::string sSummaryText, bool bPrintPeriod);
    virtual void                                FindNeighbors();
	bool                                        networkCanReportLocationCoordinates() const { return _network_can_report_coordinates;  }
    DataSetHandler                            & GetDataSetHandler() {return *gDataSets;}
    const DataSetHandler                      & GetDataSetHandler() const {return *gDataSets;}
    double                                      GetEllipseAngle(int iEllipseIndex) const;
    double                                      GetEllipseShape(int iEllipseIndex) const;
    int                                         GetFlexibleWindowEndRangeEndIndex() const {return m_nFlexibleWindowEndRangeEndIndex;}
    int                                         GetFlexibleWindowEndRangeStartIndex() const {return m_nFlexibleWindowEndRangeStartIndex;}
    int                                         GetFlexibleWindowStartRangeEndIndex() const {return m_nFlexibleWindowStartRangeEndIndex;}
    int                                         GetFlexibleWindowStartRangeStartIndex() const {return m_nFlexibleWindowStartRangeStartIndex;}
    inline const GInfo                        * GetGInfo() const { return gCentroidsHandler.get();}
    bool                                        GetIsNullifiedLocation(tract_t tLocationIndex) const;
    const std::vector<measure_t>              & GetMaxCirclePopulationArray() const {return gvMaxCirclePopulation;}
    measure_t                                   GetMaxCirclePopulationSize() const {return m_nTotalMaxCirclePopulation;}
    double                                      GetMeasureAdjustment(size_t iSetIndex) const;
    inline virtual tract_t                      GetNeighbor(int iEllipse, tract_t t, unsigned int nearness, double dClusterRadius=-1) const;
    inline tract_t                           ** GetNeighborCountArray() {return gppActiveNeighborArray;/*gpNeighborCountHandler->GetArray();*/}
    inline tract_t                           ** GetNeighborCountArray() const {return gppActiveNeighborArray;/*gpNeighborCountHandler->GetArray();*/}
    inline MinimalGrowthArray<tract_t>       ** GetReportedNeighborMaxsCountArray() const {return gpReportedMaximumsNeighborCountHandler->GetArray();}
    inline size_t                               GetNumDataSets() const {return gDataSets->GetNumDataSets();}
    inline tract_t                              GetNumMetaTracts() const {return (tract_t)gTractHandler->getMetaLocations().getLocations().size();}
    inline tract_t                              GetNumMetaTractsReferenced() const {return (tract_t)gTractHandler->getMetaLocations().getNumReferencedLocations();}
    size_t                                      GetNumNullifiedLocations() const {return gvNullifiedLocations.size();}
    virtual int                                 GetNumTimeIntervals() const {return m_nTimeIntervals;}
    inline tract_t                              GetNumTracts() const {return m_nTracts;}
    const CParameters                         & GetParameters() const {return gParameters;}
    CModel                                    & GetProbabilityModel() const {return *m_pModel;}
    const std::vector<int>                    & GetProspectiveIntervalCuts() const {return gvProspectiveIntervalCuts;}
    int                                         GetProspectiveStartIndex() const {return m_nProspectiveIntervalStart;}  
    const RiskAdjustments_t                   & getRiskAdjustments() const {return gRelativeRiskAdjustments;}
    Julian                                      GetStudyPeriodEndDate() const {return m_nEndDate;}
    Julian                                      GetStudyPeriodStartDate() const {return m_nStartDate;}
    virtual int                                 GetTimeIntervalOfDate(Julian Date) const;
    virtual int                                 GetTimeIntervalOfEndDate(Julian EndDate) const;
    int                                         GetTimeIntervalCut() const {return m_nIntervalCut;}
    int                                         getMinTimeIntervalCut() const {return _min_iterval_cut;}
    virtual const std::vector<Julian>         & GetTimeIntervalStartTimes() const {return gvTimeIntervalStartTimes;}
    inline const TractHandler                 * GetTInfo() const {return gTractHandler.get();}
    double                                      GetTotalPopulationCount() const {return gtTotalPopulation;}
    virtual Julian                              intervalIndexToJulian(unsigned int intervalIdx) const;

    virtual Julian                              convertToSeasonalDate(Julian j) const {return j;}

    virtual void                                RandomizeData(RandomizerContainer_t& RandomizerContainer, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const;
    virtual void                                ReadDataFromFiles();
    virtual void                                RemoveClusterSignificance(const CCluster& ClusterObj);
    void                                        SetActiveNeighborReferenceType(ActiveNeighborReferenceType eType);
    virtual void                                ValidateObservedToExpectedCases(const DataSet& Set) const;

    inline measure_t                            GetTotalDataSetMeasure(size_t iSetIndex) const {return gDataSets->GetDataSet(iSetIndex).getTotalMeasure();}
    inline measure_t                            GetTotalMeasure() const {return gtTotalMeasure;}
    inline measure_t                            GetTotalMeasureAux() const {return gtTotalMeasureAux;}
    inline count_t                              GetTotalCases() const {return gtTotalCases;}
    inline count_t                              GetTotalDataSetCases(size_t iSetIndex) const {return gDataSets->GetDataSet(iSetIndex).getTotalCases();}
    double                                      GetAnnualRate(size_t iSetIndex) const;
    double                                      GetAnnualRateAtStart(size_t iSetIndex) const;
    double                                      GetAnnualRatePop() const {return m_nAnnualRatePop;}

    virtual void                                DisplayCases(FILE* pFile) const;
    virtual void                                DisplayControls(FILE* pFile) const;
    virtual void                                DisplayMeasure(FILE* pFile) const;
    virtual void                                DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile) const;

    inline tract_t                           ** GetSortedArrayAsTract_T(int iEllipse) const;
    inline unsigned short                    ** GetSortedArrayAsUShort_T(int iEllipse) const;
    bool                                        isSortArrayAllocoated() const {return gpSortedIntHandler != 0 || gpSortedUShortHandler != 0;}

    inline void                                 addStore(std::auto_ptr<CentroidNeighbors>& NeighborInfo);

    //virtual void                                setLocationNotEvaluated(tract_t tTractIndex);
};

inline void CSaTScanData::addStore(std::auto_ptr<CentroidNeighbors>& NeighborInfo) {
    tract_t tCentroidIndex = NeighborInfo->GetCentroidIndex();
    if (gvCentroidNeighborStore.size() < (size_t)tCentroidIndex + 1)
        gvCentroidNeighborStore.resize(tCentroidIndex + 1, 0);
    gvCentroidNeighborStore[tCentroidIndex] = NeighborInfo.release();
}                                                                   

/** Returns pointer to 2 dimensional array, representing sorted neighbors
    for all locations for specified ellipse offset. Returns null if array
    not allocated. Caller is responsible for ensuring that 'iEllipseIndex'
    is in range [0 - (# of ellipses)]. */
inline tract_t ** CSaTScanData::GetSortedArrayAsTract_T(int iEllipseIndex) const {
  return (gpSortedIntHandler ? gpSortedIntHandler->GetArray()[iEllipseIndex] : 0);
}

/** Returns pointer to 2 dimensional array, representing sorted neighbors
    for all locations for specified ellipse offset. Returns null if array
    not allocated. Caller is responsible for ensuring that 'iEllipseIndex'
    is in range [0 - (# of ellipses)]. */
inline unsigned short ** CSaTScanData::GetSortedArrayAsUShort_T(int iEllipseIndex) const {
  return (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray()[iEllipseIndex] : 0);
}

/** Return "nearness"-th closest neighbor to "t" (nearness == 1 returns "t").
    If neither sorted array are allocated:
      - first, checked whether neighbor information exists in gvCentroidNeighborStore object
      - second, if specified radius is not negative one, calculates neigbors for centroid;
        stores results in gvCentroidNeighborStore object (note that locations will not be
        ordered by distance from centroid)
      - third, calculate neighbor information, either by distance or population, stores
        results in gvCentroidNeighborStore object */
inline tract_t CSaTScanData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness, double dClusterRadius) const {
  if (gpSortedUShortHandler)
    return (tract_t)gpSortedUShortHandler->GetArray()[iEllipse][t][nearness - 1];
  else if (gpSortedIntHandler)
    return gpSortedIntHandler->GetArray()[iEllipse][t][nearness - 1];
  else {//not storing neighbor information in sorted array
    //first, look for neighbor information in store
    if (!gvCentroidNeighborStore.size())
      gvCentroidNeighborStore.resize(m_nGridTracts, 0);
    // check the centroid neighbor store but only if ellipse/centroid match that of stored
    if (gvCentroidNeighborStore[t] && gvCentroidNeighborStore[t]->GetEllipseIndex() == iEllipse)
      return gvCentroidNeighborStore[t]->GetNeighborTractIndex(nearness - 1);
    else {//else calculate
      delete gvCentroidNeighborStore[t]; gvCentroidNeighborStore[t]=0;
      gvCentroidNeighborStore[t] = new CentroidNeighbors();
      CentroidNeighbors& NeighborInfo = *gvCentroidNeighborStore[t];
      if (dClusterRadius != -1 && !gParameters.getReportGiniOptimizedClusters())
          // can't use cluster radius with index based cluster reporting since this cluster radius might be a lesser maxima than another for cluster about same centroid
        CentroidNeighborCalculator(*this, gPrint).CalculateNeighborsAboutCentroid(iEllipse, t, NeighborInfo, dClusterRadius);
      else  
        CentroidNeighborCalculator(*this, gPrint).CalculateNeighborsAboutCentroid(iEllipse, t, NeighborInfo);
      return NeighborInfo.GetNeighborTractIndex(nearness - 1);
    }
  }
}

/** Deletes CentroidNeighbor object for iCentroidIndex, if exists. */
inline void CSaTScanData::FreeNeighborInfo(tract_t iCentroidIndex) const {
  if (gvCentroidNeighborStore.size() > (size_t)iCentroidIndex) {
    delete gvCentroidNeighborStore[iCentroidIndex]; gvCentroidNeighborStore[iCentroidIndex]=0;
  }
}

//*****************************************************************************
#endif
