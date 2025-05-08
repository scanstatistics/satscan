//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "TimeIntervalRange.h"
#include "SSException.h"
#include "SaTScanDataRead.h"
#include "NormalRandomizer.h"
#include "WeightedNormalRandomizer.h"
#include "ExponentialDataSetHandler.h"
#include "ParametersPrint.h"
#include <boost/dynamic_bitset.hpp>
#include "MetaTractManager.h"
#include "DateStringParser.h"
#include "BatchedRandomizer.h"
#include "SpaceTimeRandomizer.h"
#include "LocationRiskEstimateWriter.h"

/** class constructor */
CSaTScanData::CSaTScanData(const CParameters& Parameters, BasePrint& PrintDirection)
             : gParameters(Parameters), gPrint(PrintDirection) {
  Init();
  Setup();
}

/** class destructor */
CSaTScanData::~CSaTScanData() {
  try {
    delete m_pModel;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpReportedNeighborCountHandler; gpReportedNeighborCountHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
  }
  catch (...){}  
}

/** Iterative analyses will call this function to clear neighbor information and re-calculate neighbors. */
void CSaTScanData::AdjustNeighborCounts(ExecutionType geExecutingType) {
  try {
    bool bDistanceOnlyMax = gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses() &&
                            !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) &&
                            !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true);
    //We do not need to recalculate the number of neighbors when the max spatial size restriction is by distance only.
    if (!bDistanceOnlyMax && /*!gParameters.UseLocationNeighborsFile() &&*/ geExecutingType != CENTRICALLY) {
      //Re-calculate neighboring locations about each centroid.
      CentroidNeighborCalculator(*this, gPrint).CalculateNeighbors(*this);
      //possibly re-allocate and assign meta data contained in DataSet objects
      if (gParameters.UsingMultipleCoordinatesMetaLocations())
        for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t)
           gDataSets->GetDataSet(t).reassignMetaData(getIdentifierInfo().getMetaManagerProxy());
    }
    gvCentroidNeighborStore.killAll();
  }
  catch (prg_exception& x) {
    x.addTrace("AdjustNeighborCounts()", "CSaTScanData");
    throw;
  }
}

/** Allocates three dimensional array that stores tract index for each neighbor
    of each ellipse by grid point combination. Data type of array is conditionally
    either unsigned short or tract_t; based upon number of tracts in coordinates
    file. Note that third dimension is allocated with zero length. Later, when
    neighbors are calculated, the third dimension will be allocated based upon
    the number of calculated neighbors for each (circle/ellipse) and grid point pair.
    Repeated calls to this method cause the third dimension arrays to be re-initialized
    to zero. */
void CSaTScanData::AllocateSortedArray() {
  try {
    if (!gParameters.UsingMultipleCoordinatesMetaLocations() && _num_identifiers + (tract_t)(getIdentifierInfo().getMetaManagerProxy().getNumMeta()) < std::numeric_limits<unsigned short>::max()) {
      if (!gpSortedUShortHandler)
        gpSortedUShortHandler = new ThreeDimensionArrayHandler<unsigned short>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
      else
        gpSortedUShortHandler->FreeThirdDimension();
    }
    else {
      if (!gpSortedIntHandler)
        gpSortedIntHandler = new ThreeDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
      else
        gpSortedIntHandler->FreeThirdDimension();
    }
    //allocates two-dimensional array that will track the number of neighbors for each shape/grid point combination.
    if (gParameters.GetRestrictingMaximumReportedGeoClusterSize() || gParameters.getReportGiniOptimizedClusters()) {
      if (!gpReportedNeighborCountHandler)
        gpReportedNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts);
      gpReportedNeighborCountHandler->Set(0);
      if (!gpReportedMaximumsNeighborCountHandler)
          gpReportedMaximumsNeighborCountHandler = new TwoDimensionArrayHandler<MinimalGrowthArray<tract_t> >(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts);
    }
    if (!gpNeighborCountHandler)
      gpNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts);
    gpNeighborCountHandler->Set(0);
    //default neighbor reference type to MAXIMUM, if not set
    SetActiveNeighborReferenceType(geActiveNeighborReferenceType == NOT_SET ? MAXIMUM : geActiveNeighborReferenceType);
  }
  catch (prg_exception& x) {
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpReportedNeighborCountHandler; gpReportedNeighborCountHandler=0;
    delete gpReportedMaximumsNeighborCountHandler; gpReportedMaximumsNeighborCountHandler=0;
    x.addTrace("AllocateSortedArray()","CSaTScanData");
    throw;
  }
}

/** Calls down to tract handler class to set location at index to not evaluating; meaning that 
    this location will not be considered during neighbors calculating. */
//void CSaTScanData::setLocationNotEvaluated(tract_t tTractIndex) {
//  try {
//    //gTractHandler->setLocationNotEvaluating(tTractIndex);
//  } catch (prg_exception& x) {
//    x.addTrace("setLocationNotEvaluated()","CSaTScanData");
//    throw;
//  }
//}

/** Set the number of neighbors about ellipse/circle at centroid index. Care must be taken
    when calling this function since it sets the variables which detail the size of the sorted arrays.*/
void CSaTScanData::setNeighborCounts(int iEllipseIndex, tract_t iCentroidIndex, const std::vector<tract_t>& vMaxReported, tract_t iNumMaximumNeighbors) {
  //update neighbor array(s) for number of calculated neighbors
  if (!gpNeighborCountHandler)
    throw prg_error("Neighbor array not allocated.","setNeighborCounts()");

  if (gpReportedNeighborCountHandler && vMaxReported.size())
    gpReportedNeighborCountHandler->GetArray()[iEllipseIndex][iCentroidIndex] = vMaxReported.back();
  if (gpReportedMaximumsNeighborCountHandler)
    gpReportedMaximumsNeighborCountHandler->GetArray()[iEllipseIndex][iCentroidIndex].set(vMaxReported);
  gpNeighborCountHandler->GetArray()[iEllipseIndex][iCentroidIndex] = iNumMaximumNeighbors;
}

/** Allocates third dimension of sorted at 'array[iEllipseIndex][iCentroidIndex]'
    to length specifed by variable 'iNumMaximumNeighbors'; assigning locations indexes
    as detailed by 'vOrderLocations' variable. Sets multi-dimension arrays which detail
    the number actual and reported neighbors defined in just allocated sorted array. */
void CSaTScanData::AllocateSortedArrayNeighbors(const std::vector<DistanceToCentroid>& vOrderLocations,
                                                int iEllipseIndex, tract_t iCentroidIndex,
                                                const std::vector<tract_t>& vMaxReported, 
                                                tract_t iNumMaximumNeighbors) {
  try {
    if (gpSortedUShortHandler) {
      delete[] gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex];
      gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex]=0;
      gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex] = new unsigned short[iNumMaximumNeighbors];
      for (tract_t j=iNumMaximumNeighbors-1; j >= 0; j--) /* copy tract numbers */
         gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex][j] = static_cast<unsigned short>(vOrderLocations[j].GetTractNumber());
    }
    else if (gpSortedIntHandler) {
      delete[] gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex];
      gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex]=0;
      gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex] = new tract_t[iNumMaximumNeighbors];
      for (tract_t j=iNumMaximumNeighbors-1; j >= 0; j--) /* copy tract numbers */
         gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex][j] = vOrderLocations[j].GetTractNumber();
    }
    else
      throw prg_error("Sorted array not allocated.","AllocateSortedArrayNeighbors()");

    //update neighbor array(s) for number of calculated neighbors
    setNeighborCounts(iEllipseIndex, iCentroidIndex, vMaxReported, iNumMaximumNeighbors);
  }
  catch (prg_exception &x) {
    x.addTrace("AllocateSortedArrayNeighbors()","CSaTScanData");
    throw;
  }
}

/** Allocates third dimension of sorted at 'array[0][*(vLocations.begin())]'
    to length of vLocations vector; assigning locations indexes as detailed by
    vLocations variable. Sets multi-dimension arrays which detail the size of array. */
void CSaTScanData::AllocateSortedArrayNeighbors(tract_t iCentroidIndex, const std::vector<tract_t>& vLocations) {
  
  try {
    if (gpSortedUShortHandler) {
      delete[] gpSortedUShortHandler->GetArray()[0][iCentroidIndex];
      gpSortedUShortHandler->GetArray()[0][iCentroidIndex]=0;
      gpSortedUShortHandler->GetArray()[0][iCentroidIndex] = new unsigned short[vLocations.size()];
      for (tract_t j=(int)vLocations.size()-1; j >= 0; j--) /* copy tract numbers */
         gpSortedUShortHandler->GetArray()[0][iCentroidIndex][j] = static_cast<unsigned short>(vLocations[j]);
    }
    else if (gpSortedIntHandler) {
      delete[] gpSortedIntHandler->GetArray()[0][iCentroidIndex];
      gpSortedIntHandler->GetArray()[0][iCentroidIndex]=0;
      gpSortedIntHandler->GetArray()[0][iCentroidIndex] = new tract_t[vLocations.size()];
      for (tract_t j=(int)vLocations.size()-1; j >= 0; j--) /* copy tract numbers */
         gpSortedIntHandler->GetArray()[0][iCentroidIndex][j] = vLocations[j];
    }
    else
      throw prg_error("Sorted array not allocated.","AllocateSortedArrayNeighbors()");

    //update neighbor array(s) for number of calculated neighbors
    setNeighborCounts(0, iCentroidIndex, std::vector<tract_t>(1, vLocations.size()), vLocations.size());
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSortedArrayNeighbors()","CSaTScanData");
    throw;
  }
}

/** Calculates expected number of cases for each dataset. Records total
    measure, cases, and population for all datasets. Calls method to determines
    the maximum spatial cluster size. */
void CSaTScanData::CalculateExpectedCases() {
  gPrint.Printf("Calculating the expected number of cases\n", BasePrint::P_STDOUT);
  //calculates expected cases for each dataset
  for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
     CalculateMeasure(gDataSets->GetDataSet(t));
     gtTotalMeasure += gDataSets->GetDataSet(t).getTotalMeasure();
     gtTotalMeasureAux += gDataSets->GetDataSet(t).getTotalMeasureAux();
     gtTotalCases += gDataSets->GetDataSet(t).getTotalCases();
     gtTotalPopulation += gDataSets->GetDataSet(t).getTotalPopulation();
  }
  // release any adjustments now, we are done with them
  if (gRelativeRiskAdjustments) gRelativeRiskAdjustments->empty();
}

/** Calculates expected number of cases for dataset. */
void CSaTScanData::CalculateMeasure(RealDataSet& DataSet) {
  try {
    m_pModel->CalculateMeasure(DataSet, *this);
    //record totals at start, the optional iterative scan feature modifies start values
    DataSet.setTotalCasesAtStart(DataSet.getTotalCases());
    DataSet.setTotalMeasureAtStart(DataSet.getTotalMeasure());
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateMeasure()","CSaTScanData");
    throw;
  }
}

/** Calculates time interval indexes given study period and other time aggregation
    settings. */
void CSaTScanData::CalculateTimeIntervalIndexes() {
  int           iNumCollapsibleIntervals;
  double        dProspectivePeriodLength, dMaxTemporalLengthInUnits;

  // calculate time interval start time indexes
  SetIntervalStartTimes();
  // calculate maximum temporal cluster size, in terms of interval indexes
  SetIntervalCut();
  // calculate date indexes for flexible scanning window
  SetTimeIntervalRangeIndexes();
  // calculate date index for prospective surveillance start date
  if (gParameters.GetIsProspectiveAnalysis()) {
    m_nProspectiveIntervalStart = CalculateProspectiveIntervalStart();
    // If analysis performs simulations and adjusts for earlier analyses, then we can potentially
    // collapse unused time intervals into one based upon the prospective start date and maximum
    // temporal cluster size.
    if (gParameters.GetNumReplicationsRequested() > 0 && gParameters.GetAdjustForEarlierAnalyses()) {
      // Check the length of the shortest prospective window is not less than the minimum temporal cluster size.
      dProspectivePeriodLength = CalculateNumberOfTimeIntervals(m_nStartDate, gvTimeIntervalStartTimes[m_nProspectiveIntervalStart] - 1,
                                                                gParameters.GetTimeAggregationUnitsType(), 1);
      if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
        dProspectivePeriodLength = std::floor(dProspectivePeriodLength * gParameters.GetMaximumTemporalClusterSize()/100.0);
      if (dProspectivePeriodLength < static_cast<double>(gParameters.getMinimumTemporalClusterSize())) {
          std::string buffer;
          GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), buffer, dProspectivePeriodLength > 1.0, false);
          throw resolvable_error("Error: The minimum temporal cluster size is longer than the initial prospective period of %.1lf %s.",
                                 dProspectivePeriodLength, buffer.c_str());
      }
      // For prospective analyses, not all time intervals may be evaluated; consequently some of the
      // initial intervals can be combined into one interval. When evaluating real data, we will only
      // consider 'alive' clusters (clusters where the end date range equals the study period end date). For
      // the simulated data, we will consider historical clusters from prospective start date.
      if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
        // If the maximum temporal cluster size is defined as a percentage of the population at risk,
        // then the maximum cluster size must be calculated for each prospective period.
        for (int iWindowEnd=m_nProspectiveIntervalStart; iWindowEnd <= m_nTimeIntervals; ++iWindowEnd) {
            dProspectivePeriodLength = CalculateNumberOfTimeIntervals(m_nStartDate, gvTimeIntervalStartTimes[iWindowEnd] - 1,
                                                                      gParameters.GetTimeAggregationUnitsType(), 1);
            dMaxTemporalLengthInUnits = floor(dProspectivePeriodLength * gParameters.GetMaximumTemporalClusterSize()/100.0);
            //now calculate number of those time units a cluster can contain with respects to the specified aggregation length
            gvProspectiveIntervalCuts.push_back(static_cast<int>(floor(dMaxTemporalLengthInUnits / gParameters.GetTimeAggregationLength())));
        }

        // Now we know the index of the earliest accessed time interval, we can determine the
        // number of time intervals that can be collapsed.
        iNumCollapsibleIntervals = m_nProspectiveIntervalStart - *(gvProspectiveIntervalCuts.begin());
      }
      else
        // When the maximum temporal cluster size is a fixed period, the number of intervals
        // to collapse is simplier to calculate.
        iNumCollapsibleIntervals = m_nProspectiveIntervalStart - m_nIntervalCut;
    } else {
      // Else we are either not performing simulations, and therefore not evaluating prospective clusters,
      // or we are not adjusting for previous analyses, and therefore only evaluating 'alive' clusters
      // in both real data and simulated data.
      iNumCollapsibleIntervals = m_nTimeIntervals - m_nIntervalCut;
    }
    // Once we're determined which intervals are not evaluated, store that index for use in data infterface code.
    if (iNumCollapsibleIntervals > 1)
        _data_interface_start_idex = std::max(0, iNumCollapsibleIntervals);
  }
  // Calculate the temporal adjustment windows if performing time-stratified nonparametric and 
  // the adjustment length isn't the same as the time aggregation length.
  if (gParameters.isTimeStratifiedWithLargerAdjustmentLength()) {
      WindowRange_t range(m_nTimeIntervals - static_cast<int>(gParameters.GetNonparametricAdjustmentSize()), m_nTimeIntervals - 1);
      while (range.second >= 0) {
          _adjustment_window_ranges.push_back(range);
          range = WindowRange_t(std::max(range.first - static_cast<int>(gParameters.GetNonparametricAdjustmentSize()), 0), range.first - 1);
      }
      std::reverse(_adjustment_window_ranges.begin(), _adjustment_window_ranges.end());
  }
}

/* Clears the cluster locations caches. */ 
void CSaTScanData::clearClusterLocationsCache() {
    _cluster_locations_cache.clear();
    _cluster_network_locations_cache.erase(_cluster_network_locations_cache.begin(), _cluster_network_locations_cache.end());
}

/** Allocates/deallocates memory to store neighbor information.
    Calls MakeNeighbor() function to calculate neighbors for each centroid. */
void CSaTScanData::FindNeighbors() {
  try {
    CentroidNeighborCalculator(*this, gPrint).CalculateNeighbors(*this);
    //possibly re-allocate and assign meta data contained in DataSet objects
    if (gParameters.UsingMultipleCoordinatesMetaLocations())
      for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t)
         gDataSets->GetDataSet(t).reassignMetaData(getIdentifierInfo().getMetaManagerProxy());
  }
  catch (prg_exception& x) {
    x.addTrace("FindNeighbors()","CSaTScanData");
    throw;
  }
}

double CSaTScanData::GetAnnualRate(size_t tSetIndex) const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double dTotalCases = gDataSets->GetDataSet(tSetIndex).getTotalCases();
  double dTotalPopulation = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
  double nAnnualRate = (m_nAnnualRatePop*dTotalCases) / (dTotalPopulation*nYears);

  return nAnnualRate;
}

double CSaTScanData::GetAnnualRateAtStart(size_t tSetIndex) const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double dTotalCasesAtStart = gDataSets->GetDataSet(tSetIndex).getTotalCasesAtStart();
  double dTotalPopulation = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
  double nAnnualRate = (m_nAnnualRatePop*dTotalCasesAtStart) / (dTotalPopulation*nYears);

  return nAnnualRate;
}

/** Returns the number of cases for a specified tract and time interval.
    Note: iInterval and tTract should be valid indexes of the cases array .**/
count_t CSaTScanData::GetCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const {
  if (iInterval + 1 == m_nTimeIntervals)
    return ppCumulativeCases[iInterval][tTract];
  else
    return ppCumulativeCases[iInterval][tTract] - ppCumulativeCases[iInterval + 1][tTract];
}

/* Calculates the network locations for this cluster. */
NetworkLocationContainer_t& CSaTScanData::getClusterNetworkLocations(const CCluster& cluster, NetworkLocationContainer_t& networkLocations) const {
    // First check for this information in the cache.
    auto itrNetworkLocations = _cluster_network_locations_cache.find(cluster.getIdentifyingKey());
    if (itrNetworkLocations != _cluster_network_locations_cache.end()) {
        networkLocations = itrNetworkLocations->second;
        return networkLocations;
    }
    // Obtain the clusters centroid location.
    const Location& centroidLocation = dynamic_cast<const NetworkCentroidHandlerPassThrough*>(GetGInfo())->getCentroidLocation(cluster.GetCentroidIndex());
    const NetworkNode& centroidNode = refLocationNetwork().getNodes().find(getLocationsManager().getLocation(centroidLocation.name()).first.get())->second;
    // Calculate the network expanding out from this centroid node.
    networkLocations.clear();
    refLocationNetwork().buildNeighborsAboutNode(centroidNode, networkLocations, getLocationsManager().locations().size());

    // Figure out which location matches the user's multiple coordinates type.
    tract_t lastTract = GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), cluster.getNumIdentifiers(), cluster.GetCartesianRadius());
    if (lastTract >= GetNumIdentifiers()) {
        std::vector<tract_t> atomicIndexes;
        getIdentifierInfo().getMetaManagerProxy().getIndexes(lastTract - GetNumIdentifiers(), atomicIndexes);
        lastTract = atomicIndexes.back();
    }
    NetworkLocationContainer_t::iterator itrLastNode = networkLocations.end();
    double dCurrent = -1;
    for (unsigned int i = 0; i < getIdentifierInfo().getIdentifiers()[lastTract]->getLocations().size(); ++i) {
        const Location * tractLocation = getIdentifierInfo().getIdentifiers()[lastTract]->getLocations()[i];
        const auto& tractNode = &(refLocationNetwork().getNodes().find(getLocationsManager().getLocation(tractLocation->name()).first.get())->second);
        double dDistance = -1;
        for (auto itr = networkLocations.begin(); itr != networkLocations.end(); ++itr) {
            if (itr->first == tractNode) {
                dDistance = itr->second;
                switch (GetParameters().GetMultipleCoordinatesType()) {
                case ONEPERLOCATION: itrLastNode = itr; dCurrent = dDistance; break;
                case ATLEASTONELOCATION: // Searching for the closest location.
                    if (itrLastNode == networkLocations.end() || dCurrent > dDistance) { itrLastNode = itr; dCurrent = dDistance; } break;
                case ALLLOCATIONS: //Searching for the farthest location.
                    if (itrLastNode == networkLocations.end() || dCurrent < dDistance) { itrLastNode = itr; dCurrent = dDistance; } break;
                default: throw prg_error("Unknown multiple coordinates type '%d'.", "getClusterNetworkLocations()", GetParameters().GetMultipleCoordinatesType());
                }
                break;
            }
        }
        if (dDistance == -1) throw prg_error("Unable to determine distance between locations '%s' and '%s'.", "getClusterNetworkLocations()", centroidLocation.name().c_str(), tractLocation->name().c_str());
    }
    // Reduce the network locations collection to the last node in cluster.
    if (itrLastNode != networkLocations.end()) networkLocations.erase(itrLastNode + 1, networkLocations.end());
    // Add network locations to cache since we'll be asking for this information repeatedly in output files.
    _cluster_network_locations_cache.insert(std::make_pair(cluster.getIdentifyingKey(), networkLocations));
    return networkLocations;
}

/** Returns angle to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CSaTScanData::GetEllipseAngle(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 0.0 : gvEllipseAngles[iEllipseIndex - 1]);
}

/** Returns shape to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CSaTScanData::GetEllipseShape(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 1.0 : gvEllipseShapes[iEllipseIndex - 1]);
}

/** convert interval index to Julian date. */
Julian CSaTScanData::intervalIndexToJulian(unsigned int intervalIdx) const {
    Julian date = gvTimeIntervalStartTimes.at(intervalIdx);
    return date;
}

/** Returns whether data of location at index has been removed as a result of
    being part of most likely cluster in a iterative scan.*/
bool CSaTScanData::isNullifiedIdentifier(tract_t tLocationIndex) const {
   return std::find(_nullified_identifiers.begin(), _nullified_identifiers.end(), tLocationIndex) != _nullified_identifiers.end();
}

/** For Bernoulli model, returns ratio of total cases / total population for
    iDataSet'th dataset. For all other models, returns 1.*/
double CSaTScanData::GetMeasureAdjustment(size_t tSetIndex) const {
  if (gParameters.GetProbabilityModelType() == BERNOULLI) {
    double population = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
    return population ? gDataSets->GetDataSet(tSetIndex).getTotalCases() / population : 0.0;
  } else
    return 1.0;
}

boost::shared_ptr<LocationsReportHelper> CSaTScanData::getLocationReportHelper() const {
    if (!_report_helper.get()) _report_helper.reset(new LocationsReportHelper(*this));
    return _report_helper;
}

/** Input: Date.                                                    **/
/** Returns: Index of the time interval to which the date belongs.   **/
/** If Date does not belong to any time interval, -1 is returned. **/
/** Note: First time interval has index 0.                          **/
int CSaTScanData::GetTimeIntervalOfDate(Julian Date) const {
  int   i=0;

  //check that date is within study period
  if (Date < gvTimeIntervalStartTimes[0] || Date >= gvTimeIntervalStartTimes[m_nTimeIntervals])
    return -1;

  while (Date >=  gvTimeIntervalStartTimes[i+1])
       ++i;

  return i;
}

/** Gets time interval index into interval start times array for end date. */
int CSaTScanData::GetTimeIntervalOfEndDate(Julian EndDate) const {
  int   i, iDateIndex = -1;

  //find index for end date, the interval beyond where date fits
  for (i=m_nTimeIntervals; i > 0  && iDateIndex == -1; --i)
     if (EndDate <= gvTimeIntervalStartTimes[i] - 1 && EndDate > gvTimeIntervalStartTimes[i - 1] - 1)
        iDateIndex = i;
        
  return iDateIndex;
}

/** internal class initializaton */
void CSaTScanData::Init() {
  m_pModel = 0;
  geActiveNeighborReferenceType=NOT_SET;
  gppActiveNeighborArray=0;
  gpNeighborCountHandler=0;
  gpReportedNeighborCountHandler=0;
  gpReportedMaximumsNeighborCountHandler=0;
  gpSortedIntHandler=0;
  gpSortedUShortHandler=0;
  m_nAnnualRatePop = 100000;
  m_nTotalMaxCirclePopulation = 0;
  gtTotalMeasure=0;
  gtTotalCases=0;
  gtTotalPopulation=0;
  m_nFlexibleWindowStartRangeStartIndex=0;
  m_nFlexibleWindowStartRangeEndIndex=0;
  m_nFlexibleWindowEndRangeStartIndex=0;
  m_nFlexibleWindowEndRangeEndIndex=0;
  m_nGridTracts = 0;
  _num_identifiers = 0;
  gtTotalMeasureAux = 0;
  m_nStartDate = 0;
  m_nEndDate = 0;
  m_nIntervalCut=1;
  _min_iterval_cut=4;
  _network_can_report_coordinates = false;
  _drilldown_level = 0;
  _drilldown_runid = 0;
  _data_interface_start_idex = 0;
}

/** Randomizes collection of simulation data in concert with passed collection
    of randomizers. This method for creating randomized data is primarily
    utilized in conjunction with threaded simulations. */
/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSaTScanData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                 SimulationDataContainer_t& SimDataContainer,
                                 unsigned int iSimulationNumber) const {
  try {
    gDataSets->RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("RandomizeData()","CSaTScanData");
    throw;
  }
}

/** First calls internal methods to prepare internal structure:
    - calculate time interval start times
    - calculate a clusters maximum temporal window length in terms of time intervals
    - calculate indexes of flexible scanning window, if requested
    - calculate start interval index of start date of prospective analyses
    - read input data from files base upon probability model
    Throws ResolvableException if read fails. */
void CSaTScanData::ReadDataFromFiles() {
  try {
    SaTScanDataReader(*this).Read();
	PostDataRead();
  } catch (prg_exception& x) {
	  x.addTrace("ReadDataFromFiles()", "CSaTScanData");
	  throw;
  }
}

/* Class specific actions to perform after data has been read. */
void CSaTScanData::PostDataRead() {
	try {
		if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION || 
            gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC ||
            gParameters.GetTimeTrendAdjustmentType() == CALCULATED_QUADRATIC)
			std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_NC));
		CalculateExpectedCases();
        if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
            std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_NC));

        if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION && gParameters.GetProbabilityModelType() == BATCHED) {
            std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux));
            std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux2));
            for (unsigned int i = 0; i < gDataSets->getDataSets().size(); ++i) {
                gDataSets->GetDataSet(i).setPositiveBatchIndexes_PT(static_cast<unsigned int>(gDataSets->GetDataSet(i).getTotalMeasure()));
                gDataSets->GetDataSet(i).setBatchData_PT(static_cast<unsigned int>(gDataSets->GetDataSet(i).getTotalMeasure()));
            }
        }
        if (gParameters.UseMetaLocationsFile())
			gDataSets->assignMetaData(gDataSets->getDataSets());
	} catch (prg_exception& x) {
		x.addTrace("PostDataRead()", "CSaTScanData");
		throw;
	}
}

/** Removes all cases/controls/measure from data sets, geographically and temporally, for passed cluster object. */
void CSaTScanData::RemoveClusterSignificance(const CCluster& Cluster) {
    tract_t stopNeighbor(0), thisNeighbor(-1), iNeighborIndex=0;
    try {
        // update total data set population now for the Poisson model -- before cluster tracts are nullified
        if (gParameters.GetProbabilityModelType() == POISSON && Cluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
            for (size_t dIdx=0; dIdx < gDataSets->GetNumDataSets(); ++dIdx) {
                double clusterSetPopulation = GetProbabilityModel().GetPopulation(dIdx, Cluster, *this);
                // round the total population now for reporting reasons
                gDataSets->GetDataSet(dIdx).setTotalPopulation(macro_round(gDataSets->GetDataSet(dIdx).getTotalPopulation()) - macro_round(clusterSetPopulation));
            }
        }
        // Remove the data of each location within this cluster.
        stopNeighbor = (Cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? _num_identifiers - 1 : GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), Cluster.getNumIdentifiers()));
        while (thisNeighbor != stopNeighbor) {
            thisNeighbor = (Cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? thisNeighbor + 1 : GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), ++iNeighborIndex));
            // Previous iterations of iterative scan could have had this location as part of the most likely cluster.
            if ((Cluster.GetClusterType() == PURELYSPATIALCLUSTER || Cluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER || 
                Cluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER) && isNullifiedIdentifier(thisNeighbor))
                continue;
            if (thisNeighbor < _num_identifiers)
                RemoveTractSignificance(Cluster, thisNeighbor);
            else {//tract is a meta location
                std::vector<tract_t> atomicIndexes;
		        _identifiers_manager->getMetaManagerProxy().getIndexes(thisNeighbor - _num_identifiers, atomicIndexes);
                for (auto a: atomicIndexes) { 
                    if (!isNullifiedIdentifier(a)) RemoveTractSignificance(Cluster, a);
                }
            }
        }
        //now that cluster data has now been removed, update data sets by probability model
        if (gParameters.GetProbabilityModelType() == POISSON) {
            gtTotalMeasure = 0;
            for (size_t d=0; d < gDataSets->GetNumDataSets(); ++d) {
                RealDataSet& dataset = gDataSets->GetDataSet(d);
                measure_t tAdjustedTotalMeasure=0, tCalibration = (measure_t)(dataset.getTotalCases())/(dataset.getTotalMeasure());
                measure_t ** ppMeasure = dataset.getMeasureData().GetArray(); //recalibrate the measure array to equal expected cases
                for (int i=0; i < GetNumTimeIntervals()-1; ++i) for (tract_t t=0; t < _num_identifiers; ++t) ppMeasure[i][t] = (ppMeasure[i][t] - ppMeasure[i+1][t]) * tCalibration;
                for (tract_t t=0; t < _num_identifiers; ++t) ppMeasure[GetNumTimeIntervals() - 1][t] *= tCalibration;
                dataset.setMeasureDataToCumulative();
                for (tract_t t=0; t < _num_identifiers; ++t) tAdjustedTotalMeasure += ppMeasure[0][t];
                gDataSets->GetDataSet(d).setTotalMeasure(tAdjustedTotalMeasure);
                gtTotalMeasure += tAdjustedTotalMeasure;
            }
        } else if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
            SpaceTimeRandomizer * randomizer;
            gtTotalMeasure = 0;
            for (size_t d = 0; d < gDataSets->GetNumDataSets(); ++d) {
                RealDataSet& dataset = gDataSets->GetDataSet(d);
                if ((randomizer = dynamic_cast<SpaceTimeRandomizer*>(gDataSets->GetRandomizer(d))) == 0)
                    throw prg_error("Randomizer could not be dynamically casted to SpaceTimeRandomizer type.\n", "RemoveClusterSignificance()");
                randomizer->CreateRandomizationData(dataset);
                m_pModel->CalculateMeasure(dataset, *this);
                gtTotalMeasure += dataset.getTotalMeasure();
            }
        } else if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
            //recalibrate the measure to equal expected cases
            AbstractExponentialRandomizer *pRandomizer;
            gtTotalCases = 0;
            gtTotalMeasure = 0;
            for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
                RealDataSet& dataset = gDataSets->GetDataSet(t);
                if ((pRandomizer = dynamic_cast<AbstractExponentialRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                    throw prg_error("Randomizer could not be dynamically casted to AbstractExponentialRandomizer type.\n", "RemoveClusterSignificance()");
                pRandomizer->AssignFromAttributes(dataset);
                gtTotalCases += dataset.getTotalCases();
                gtTotalMeasure += dataset.getTotalMeasure();
            }
        } else if (gParameters.GetProbabilityModelType() == BATCHED) {
            // obtain data sets from updated randomizer
            BatchedRandomizer* pRandomizer;
            gtTotalCases = 0;
            gtTotalMeasure = gtTotalMeasureAux = 0;
            for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t) {
                RealDataSet& dataset = gDataSets->GetDataSet(t);
                if ((pRandomizer = dynamic_cast<BatchedRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                    throw prg_error("Randomizer could not be dynamically casted to BatchedRandomizer type.\n", "RemoveClusterSignificance()");
                pRandomizer->AssignFromAttributes(dataset);
                gtTotalCases += dataset.getTotalCases();
                gtTotalMeasure += dataset.getTotalMeasure();
            }
        } else if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
            //recalculate the data set cases/measure given updated randomizer data
            AbstractNormalRandomizer *pRandomizer;
            gtTotalCases=0;
            gtTotalMeasureAux=gtTotalMeasureAux=0;
            for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
                RealDataSet& dataset = gDataSets->GetDataSet(t);
                if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                    throw prg_error("Randomizer could not be dynamically casted to AbstractNormalRandomizer type.\n", "RemoveClusterSignificance()");
                pRandomizer->AssignFromAttributes(dataset);
                //update class variables that defines totals across all data sets
                gtTotalCases += dataset.getTotalCases();
                gtTotalMeasure += dataset.getTotalMeasure();
                gtTotalMeasureAux += dataset.getTotalMeasureAux();
            }
        } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
            //recalculate the data set cases/measure given updated randomizer data
            AbstractWeightedNormalRandomizer *pRandomizer;
            gtTotalCases=0;
            gtTotalMeasureAux=gtTotalMeasureAux=0;
            for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
                RealDataSet& dataset = gDataSets->GetDataSet(t);
                if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                    throw prg_error("Randomizer could not be dynamically casted to AbstractNormalDataRandomizer type.\n", "RemoveClusterSignificance()");
                pRandomizer->AssignFromAttributes(dataset);
                //update class variables that defines totals across all data sets
                gtTotalCases += dataset.getTotalCases();
                gtTotalMeasure += dataset.getTotalMeasure();
                gtTotalMeasureAux += dataset.getTotalMeasureAux();
            }
        }
        //now recalculate purely temporal arrays, as needed
        if (gParameters.GetIncludePurelyTemporalClusters() || gParameters.GetIsPurelyTemporalAnalysis()) {
            switch (gParameters.GetProbabilityModelType()) {
                case BATCHED    : for (unsigned int i = 0; i < gDataSets->getDataSets().size(); ++i)
                                    gDataSets->GetDataSet(i).setPositiveBatchIndexes_PT(static_cast<unsigned int>(gDataSets->GetDataSet(i).getTotalMeasure()));
                                  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux2));
                case NORMAL     : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux));
                case EXPONENTIAL:
                case BERNOULLI  :
                case POISSON    : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT));
                                  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT)); break;
                case CATEGORICAL:
                case ORDINAL    : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_Cat)); break;
                default : throw prg_error("Unknown probability %d model.", "RemoveClusterSignificance()", gParameters.GetProbabilityModelType());
            }
        }
        //now recalculate meta data as needed
        if (gParameters.UseMetaLocationsFile())
            gDataSets->assignMetaData(gDataSets->getDataSets());
    } catch (prg_exception& x) {
        x.addTrace("RemoveClusterSignificance()", "CSaTScanData");
        throw;
    }
}

/** Removes all cases/controls/measure from data sets, geographically and temporally, for location at tTractIndex in specified interval range. */
void CSaTScanData::RemoveTractSignificance(const CCluster& Cluster, tract_t tTractIndex) {
    count_t tCasesInInterval, ** ppCases;
    measure_t tMeasureInInterval, ** ppMeasure;

    if (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI) {
        gtTotalCases = 0;
        gtTotalMeasure = 0;
        if (gParameters.GetProbabilityModelType() == BERNOULLI) gtTotalPopulation = 0;
        for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t) {
            RealDataSet& DataSet = gDataSets->GetDataSet(t);
            ppCases = DataSet.getCaseData().GetArray();
            ppMeasure = DataSet.getMeasureData().GetArray();
            //get cases/measure in earliest interval - we'll need to remove these from intervals earlier than cluster window
            tCasesInInterval = ppCases[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
            tMeasureInInterval = ppMeasure[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppMeasure[Cluster.m_nLastInterval][tTractIndex]);
            //zero out cases/measure in clusters defined temporal window
            for (int i = Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i) {
                ppCases[i][tTractIndex] = (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
                ppMeasure[i][tTractIndex] = (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppMeasure[Cluster.m_nLastInterval][tTractIndex]);
            }
            //remove cases/measure from earlier intervals
            for (int i = 0; i < Cluster.m_nFirstInterval; ++i) {
                ppCases[i][tTractIndex] -= tCasesInInterval;
                ppMeasure[i][tTractIndex] -= tMeasureInInterval;
            }
            //update totals for data set
            DataSet.setTotalCases(DataSet.getTotalCases() - tCasesInInterval);
            DataSet.setTotalMeasure(DataSet.getTotalMeasure() - tMeasureInInterval);
            if (gParameters.GetProbabilityModelType() == BERNOULLI) DataSet.setTotalControls(DataSet.getTotalControls() - static_cast<count_t>(tMeasureInInterval - tCasesInInterval));
            if (gParameters.GetProbabilityModelType() == BERNOULLI) DataSet.setTotalPopulation(DataSet.getTotalPopulation() - tMeasureInInterval);
            //update class variables that defines totals across all data sets
            gtTotalCases += DataSet.getTotalCases();
            gtTotalMeasure += DataSet.getTotalMeasure();
            if (gParameters.GetProbabilityModelType() == BERNOULLI) gtTotalPopulation += DataSet.getTotalPopulation();
        }
    } else if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
        //remove location cases for the entire study period
        auto clearCases = [this, Cluster, tTractIndex](count_t ** ppcases) {
            count_t casesAtLocation = ppcases[0][tTractIndex];
            for (int i = 0; i < GetNumTimeIntervals(); ++i)
                ppcases[i][tTractIndex] = 0;
            return casesAtLocation;
        };
        gtTotalCases = 0;
        for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t) {
            RealDataSet& DataSet = gDataSets->GetDataSet(t);
            //remove cases and update totals for data set
            DataSet.setTotalCases(DataSet.getTotalCases() - clearCases(DataSet.getCaseData().GetArray()));
            //remove category cases then update totals for data set
            gtTotalCases += DataSet.getTotalCases();
            for (size_t c = 0; c < DataSet.getCaseData_Cat().size(); ++c) {
                DataSet.getPopulationData().AddCovariateCategoryCaseCount(c, 
                    clearCases(DataSet.getCategoryCaseData(c).GetArray()) * -1, true
                );
            }
        }
    } else if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
        gtTotalCases = 0;
        gtTotalPopulation = 0;
        for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
            RealDataSet& DataSet = gDataSets->GetDataSet(t);
            PopulationData& thisPopulation = DataSet.getPopulationData();
            // Remove observed cases for location from data set ordinal categories
            for (size_t c=0; c < DataSet.getCaseData_Cat().size(); ++c) {
             ppCases = DataSet.getCategoryCaseData(c).GetArray();
             //get cases in earliest interval - we'll need to remove these from intervals earlier than cluster window
             tCasesInInterval = ppCases[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
             //zero out cases/measure in clusters defined temporal window
             for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
               ppCases[i][tTractIndex] = (Cluster.m_nLastInterval == GetNumTimeIntervals() ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
             //remove cases/measure from earlier intervals
             for (int i=0; i < Cluster.m_nFirstInterval; ++i)
               ppCases[i][tTractIndex] -= tCasesInInterval;
             //update category population
             thisPopulation.RemoveCategoryTypeCases(c, tCasesInInterval);
             //update totals for data set
             DataSet.setTotalCases(DataSet.getTotalCases() - tCasesInInterval);
             DataSet.setTotalPopulation(DataSet.getTotalPopulation() - tCasesInInterval);
           }
            //update class variables that defines totals across all data sets
            gtTotalCases += DataSet.getTotalCases();
            gtTotalPopulation += DataSet.getTotalCases();
        }
    } else if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
        AbstractNormalRandomizer *pRandomizer;
        for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
            if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                throw prg_error("Randomizer could not be dynamically casted to AbstractNormalRandomizer type.\n", "RemoveClusterSignificance()");
            //zero out cases/measure in clusters defined spatial/temporal window
            for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
                pRandomizer->RemoveCase(i, tTractIndex);
        }
    } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
        AbstractWeightedNormalRandomizer *pRandomizer;
        for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
            if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "RemoveClusterSignificance()");
            //zero out cases/measure in clusters defined spatial/temporal window
            for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
                pRandomizer->RemoveCase(i, tTractIndex);
        }
    } else if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
           AbstractExponentialRandomizer* pRandomizer;
           for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t) {
               if ((pRandomizer = dynamic_cast<AbstractExponentialRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                   throw prg_error("Randomizer could not be dynamically casted to AbstractExponentialRandomizer type.\n", "RemoveClusterSignificance()");
               //zero out cases/measure in clusters defined spatial/temporal window
               for (int i = Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
                   pRandomizer->RemoveCase(i, tTractIndex);
           }
     } else if (gParameters.GetProbabilityModelType() == BATCHED) {
         BatchedRandomizer* pRandomizer;
         for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t) {
             if ((pRandomizer = dynamic_cast<BatchedRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
                 throw prg_error("Randomizer could not be dynamically casted to BatchedRandomizer type.\n", "RemoveClusterSignificance()");
             //zero out cases/measure in clusters defined spatial/temporal window
             for (int i = Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
                 pRandomizer->RemoveCase(i, tTractIndex);
         }
     } else
        throw prg_error("RemoveClusterSignificance() not implemented for %s model.", "RemoveClusterSignificance()", ParametersPrint(gParameters).GetProbabilityModelTypeAsString());

    // Remove location population data as specified in maximum circle population file
    if (gvMaxCirclePopulation.size()) {
        m_nTotalMaxCirclePopulation -= gvMaxCirclePopulation[tTractIndex];
        gvMaxCirclePopulation[tTractIndex] = 0;
    }
    
    // Add location to collection of nullified locations - note that we're just removing locations' data, not the location.
    if (!isNullifiedIdentifier(tTractIndex) && (
        Cluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER || Cluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER ||
        Cluster.GetClusterType() == PURELYSPATIALCLUSTER || Cluster.GetClusterType() == SPACETIMECLUSTER
        ))
        _nullified_identifiers.push_back(tTractIndex);
}

/** Set neighbor array pointer requested type. */
void CSaTScanData::SetActiveNeighborReferenceType(ActiveNeighborReferenceType eType) {
  if (gParameters.GetIsPurelyTemporalAnalysis() || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)
    return;
  switch (eType) {
    case REPORTED : if (gpReportedNeighborCountHandler) {
                      gppActiveNeighborArray = gpReportedNeighborCountHandler->GetArray();
                      break;
                    }
    case MAXIMUM  : if (!gpNeighborCountHandler)
                      throw prg_error("Neighbor array not allocated.","SetActiveNeighborReferenceType()");
                    gppActiveNeighborArray = gpNeighborCountHandler->GetArray();
                    break;
    case NOT_SET  : gppActiveNeighborArray=0; break;
    default : throw prg_error("Unknown active neighbor type '%d'.", "SetActiveNeighborReferenceType()", eType);
  };
  geActiveNeighborReferenceType = eType;
}

/* Calculates the number of time aggregation units to include in potential clusters without exceeding the maximum temporal cluster size.*/
void CSaTScanData::SetIntervalCut() {
    double dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

    try {
        if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
            //calculate the number of time interval units which comprise the maximum
            //temporal cluster size in the study period
            dStudyPeriodLengthInUnits = CalculateNumberOfTimeIntervals(m_nStartDate, m_nEndDate, gParameters.GetTimeAggregationUnitsType(), 1);
            dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gParameters.GetMaximumTemporalClusterSize()/100.0);
            //now calculate number of those time units a cluster can contain with respects to the specified aggregation length
            m_nIntervalCut = static_cast<int>(std::floor(dMaxTemporalLengthInUnits / gParameters.GetTimeAggregationLength()));
        } else
            m_nIntervalCut = static_cast<int>(gParameters.GetMaximumTemporalClusterSize() / gParameters.GetTimeAggregationLength());

        if (m_nIntervalCut==0)
        //Validation in CParameters should have produced error before this body of code.
        //Only a program error or user selecting not to validate parameters should cause
        //this error to occur.
        throw prg_error("The calculated number of time aggregations units in potential clusters is zero.","SetIntervalCut()");

        // calculate the minimum interval cut -- with the minimum being one interval
        _min_iterval_cut = static_cast<int>(std::ceil(static_cast<double>(gParameters.getMinimumTemporalClusterSize())/static_cast<double>(gParameters.GetTimeAggregationLength())));
    } catch (prg_exception& x) {
        x.addTrace("SetIntervalCut()","CSaTScanData");
        throw;
    }
}

/** Calculates the time interval start times given study period and time interval
    length. Start times are calculated from the study period end date backwards,
    which means that first time interval could possibly not be the requested time
    interval length. */
void CSaTScanData::SetIntervalStartTimes() {
  Julian                IntervalStartingDate = m_nEndDate+1;
  DecrementableEndDate  DecrementingDate(m_nEndDate, gParameters.GetTimeAggregationUnitsType());

  gvTimeIntervalStartTimes.clear();
  //latest interval start time is the day after study period end date
  gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
  IntervalStartingDate = DecrementingDate.Decrement(gParameters.GetAnalysisType() == SEASONALTEMPORAL ? 1 : gParameters.GetTimeAggregationLength());
  while (IntervalStartingDate > m_nStartDate) {
      //push interval start time onto vector
      gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
      //find the next prior interval start time from current, given length of time intervals
      IntervalStartingDate = DecrementingDate.Decrement(gParameters.GetAnalysisType() == SEASONALTEMPORAL ? 1 : gParameters.GetTimeAggregationLength());
  }

  // If the last calculated interval date is not the start date, remove last added so that the initial interval is at least
  // as long as aggregation length. This will prevent the initial time period from being shorter than aggregation
  // length -- but it will also have the effect of making the initial interval longer than the aggregation length.
  if (IntervalStartingDate < m_nStartDate) {
      gvTimeIntervalStartTimes.pop_back();
  }

  //push study period start date onto vector
  gvTimeIntervalStartTimes.push_back(m_nStartDate);
  //reverse elements of vector so that elements are ordered: study period start --> 'study period end + 1'
  std::reverse(gvTimeIntervalStartTimes.begin(), gvTimeIntervalStartTimes.end());
  //record number of time intervals, not including 'study period end date + 1' date
  m_nTimeIntervals = (int)gvTimeIntervalStartTimes.size() - 1;

  if (m_nTimeIntervals <= 1)
    //This error should be catch in the CParameters validation process.
    throw prg_error("The number of time intervals was calculated as one. Temporal\n"
                    "and space-time analyses can not be performed on less than two\n"
                    "time intervals.\n", "SetIntervalStartTimes()");
}

/* Calculates which time interval the prospective surveillance start date is in.*/
int CSaTScanData::CalculateProspectiveIntervalStart() const {
  int   iDateIndex = m_nTimeIntervals;

  try {
  	if (gParameters.GetAdjustForEarlierAnalyses()) {
      iDateIndex = GetTimeIntervalOfEndDate(DateStringParser::getDateAsJulian(gParameters.GetProspectiveStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
      if (iDateIndex < 0)
        throw resolvable_error("Error: : The start date for prospective analyses '%s' is prior to the study period start date '%s'.\n",
                               gParameters.GetProspectiveStartDate().c_str(), gParameters.GetStudyPeriodStartDate().c_str());
      if (iDateIndex > m_nTimeIntervals)
        throw resolvable_error("Error: The start date for prospective analyses '%s' occurs after the study period end date '%s'.\n",
                               gParameters.GetProspectiveStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
    }                                
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateProspectiveIntervalStart()","CSaTScanData");
    throw;
  }
  return iDateIndex;
}

/** For all datasets, causes temporal structures to be allocated and set. */
void CSaTScanData::SetPurelyTemporalCases() {
  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
      for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t)
        gDataSets->GetDataSet(t).setCaseData_PT_Cat();
    else
      for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t)
        gDataSets->GetDataSet(t).setCaseData_PT();
  }
  catch (prg_exception& x) {
    x.addTrace("SetPurelyTemporalCases()","CSaTScanData");
    throw;
  }
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  std::string   sTimeIntervalType, sMessage, sDateWST, sDateMaxWET; 
  int           iMaxEndWindow, iWindowStart;

  if (gParameters.GetIncludeClustersType() == CLUSTERSINRANGE) {
    //find start range date indexes
    m_nFlexibleWindowStartRangeStartIndex = GetTimeIntervalOfDate(DateStringParser::getDateAsJulian(gParameters.GetStartRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
    m_nFlexibleWindowStartRangeEndIndex = GetTimeIntervalOfDate(DateStringParser::getDateAsJulian(gParameters.GetStartRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType())) - 1;
    //find end range date indexes
    //m_nFlexibleWindowEndRangeStartIndex = GetTimeIntervalOfDate(DateStringParser::getDateAsJulian(gParameters.GetEndRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
    m_nFlexibleWindowEndRangeStartIndex = GetTimeIntervalOfEndDate(DateStringParser::getDateAsJulian(gParameters.GetEndRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
    m_nFlexibleWindowEndRangeEndIndex = GetTimeIntervalOfEndDate(DateStringParser::getDateAsJulian(gParameters.GetEndRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType()));
    //validate windows will be evaluated - check that there will be clusters evaluated...
    iMaxEndWindow = std::min(m_nFlexibleWindowEndRangeEndIndex, m_nFlexibleWindowStartRangeEndIndex + m_nIntervalCut);
    iWindowStart = std::max(m_nFlexibleWindowEndRangeStartIndex - m_nIntervalCut, m_nFlexibleWindowStartRangeStartIndex);
    if (iWindowStart >= iMaxEndWindow) {
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sTimeIntervalType, true, false);
      JulianToString(sDateWST, gvTimeIntervalStartTimes[iWindowStart], gParameters.GetPrecisionOfTimesType());
      JulianToString(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1, gParameters.GetPrecisionOfTimesType());
      throw resolvable_error("Error: No clusters will be evaluated.\n"
                             "       With the incorporation of a maximum temporal cluster size of %i %s,\n"
                             "       the temporal window scanned has a start time of %s (end range\n"
                             "       ending time minus %i %s) and a maximum window end time of %s\n"
                             "       (start range ending time plus %i %s), which results in no windows scanned.",
                             m_nIntervalCut * gParameters.GetTimeAggregationLength(),
                             sTimeIntervalType.c_str(), sDateWST.c_str(),
                             m_nIntervalCut * gParameters.GetTimeAggregationLength(),
                             sTimeIntervalType.c_str(), sDateMaxWET.c_str(),
                             m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    /*if (m_nFlexibleWindowEndRangeStartIndex > iMaxEndWindow) {
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sTimeIntervalType, true, false);
      JulianToString(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1, gParameters.GetPrecisionOfTimesType());
      throw resolvable_error("Error: No clusters will be evaluated.\n"
                             "       With the incorporation of a maximum temporal cluster size of %i %s\n"
                             "       the maximum window end time becomes %s (start range ending\n"
                             "       time plus %i %s), which does not intersect with scanning window end range.\n",
                             m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str(),
                             sDateMaxWET.c_str(), m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str());
    }*/

    /*
    if (!(gParameters.GetProbabilityModelType() == POISSON && gParameters.UseAdjustmentForRelativeRisksFile())) {
        // Collapse unused time intervals at end of study period, if possible.
        if (m_nTimeIntervals - iMaxEndWindow > 1)
            gvTimeIntervalStartTimes.erase(gvTimeIntervalStartTimes.end() - (m_nTimeIntervals - iMaxEndWindow), gvTimeIntervalStartTimes.end() - 1);
        // Collapse unused time intervals at beginning of study period, if possible.
        if (iWindowStart > 1)
            gvTimeIntervalStartTimes.erase(gvTimeIntervalStartTimes.begin() + 1, gvTimeIntervalStartTimes.begin() + iWindowStart);
        m_nTimeIntervals = gvTimeIntervalStartTimes.size() - 1;
        // recalculate flexable window indexes
        m_nFlexibleWindowStartRangeStartIndex = GetTimeIntervalOfDate(DateStringParser::getDateAsJulian(gParameters.GetStartRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
        m_nFlexibleWindowStartRangeEndIndex = GetTimeIntervalOfDate(DateStringParser::getDateAsJulian(gParameters.GetStartRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType()));
        m_nFlexibleWindowEndRangeStartIndex = GetTimeIntervalOfEndDate(DateStringParser::getDateAsJulian(gParameters.GetEndRangeStartDate().c_str(), gParameters.GetPrecisionOfTimesType()));
        m_nFlexibleWindowEndRangeEndIndex = GetTimeIntervalOfEndDate(DateStringParser::getDateAsJulian(gParameters.GetEndRangeEndDate().c_str(), gParameters.GetPrecisionOfTimesType()));
    }*/

    // verify that windows will be evaluated given the flexible window definition and the minimum cluster size
    if (_min_iterval_cut >= (m_nFlexibleWindowEndRangeEndIndex - m_nFlexibleWindowStartRangeStartIndex)) {
      throw resolvable_error("Error: No clusters will be evaluated.\n"
                             "       The flexible scanning window defines a range in which no windows will\n"
                             "       be evaluated given a minimum temporal cluster size of %i %s.\n",
                             gParameters.getMinimumTemporalClusterSize(), 
                             GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sTimeIntervalType, gParameters.getMinimumTemporalClusterSize() > 1, false));
    }

  }
}

/** internal setup function */
void CSaTScanData::Setup() {
  int es, ea, lCurrentEllipse=0;


  try {
    m_nStartDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType());
    m_nEndDate = DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType());

    //For now, compute the angle and store the angle and shape
    //for each ellipsoid.  Maybe transfer info to a different location in the
    //application or compute "on the fly" prior to printing.
    if (gParameters.GetNumTotalEllipses() > 0) {
        gvEllipseAngles.resize(gParameters.GetNumTotalEllipses());
        gvEllipseShapes.resize(gParameters.GetNumTotalEllipses());
        for (es=0; es < gParameters.GetNumRequestedEllipses(); ++es) {
            for (ea=0; ea < gParameters.GetEllipseRotations()[es]; ++ea) {
                gvEllipseAngles[lCurrentEllipse] = PI * ea/gParameters.GetEllipseRotations()[es];
                gvEllipseShapes[lCurrentEllipse] = gParameters.GetEllipseShapes()[es];
                ++lCurrentEllipse;
            }
        }
    }
	_identifiers_manager.reset(new IdentifiersManager(gParameters.GetIsPurelyTemporalAnalysis(), gParameters.GetMultipleCoordinatesType()));
    if (gParameters.UseSpecialGrid())
        gCentroidsHandler.reset(new CentroidHandler());
    else if (gParameters.getUseLocationsNetworkFile())
        gCentroidsHandler.reset(new NetworkCentroidHandlerPassThrough(_locations_network));
    else
		gCentroidsHandler.reset(new LocationsCentroidHandlerPassThrough(_identifiers_manager->getLocationsManager()));
    gRelativeRiskAdjustments = RiskAdjustments_t(new RelativeRiskAdjustmentHandler(*this));
  } catch (prg_exception& x) {
    x.addTrace("Setup()","CSaTScanData");
    throw;
  }
}

/** Throws exception if case(s) were observed for an interval/location
    but the expected number of cases for interval/location is zero. For
    the Poisson model, this situation is likely the result of incorrect
    data provided in the population file, possibly the case file. For the
    other probability models, this is probably a bug in the code itself. */
void CSaTScanData::ValidateObservedToExpectedCases(const DataSet& Set) const {
  int           i;
  tract_t       t;
  std::string   sStart, sEnd;

  //note that cases must be cummulative and measure non-cummulative
  count_t   ** ppCumulativeCases=Set.getCaseData().GetArray();
  measure_t ** ppNonCumulativeMeasure=Set.getMeasureData().GetArray();

  try {
    for (i=0; i < m_nTimeIntervals; ++i)
       for (t=0; t < _num_identifiers; ++t)
          if (!ppNonCumulativeMeasure[i][t] && GetCaseCount(ppCumulativeCases, i, t))
            throw resolvable_error("Error: For locationID '%s' in time interval %s - %s,\n"
                                   "       the expected number of cases is zero but there were cases observed.\n"
                                   "       Please review the correctness of population and case files.",
				                   _identifiers_manager->getIdentifiers().at(t)->name().c_str(),
                                   JulianToString(sStart, gvTimeIntervalStartTimes[i], gParameters.GetPrecisionOfTimesType()).c_str(),
                                   JulianToString(sEnd, gvTimeIntervalStartTimes[i + 1] - 1, gParameters.GetPrecisionOfTimesType()).c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateObservedToExpectedCases()","CSaTScanData");
    throw;
  }
}
