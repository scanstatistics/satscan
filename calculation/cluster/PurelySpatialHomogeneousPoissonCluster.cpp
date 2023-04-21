//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialHomogeneousPoissonCluster.h"
#include "SSException.h"

/** constructor */
PurelySpatialHomogeneousPoissonCluster::PurelySpatialHomogeneousPoissonCluster(const AbstractDataSetGateway & DataGateway)
                                       :CCluster() {
  try {
    gpClusterData = new SpatialHomogeneousData(DataGateway);
    //The last time interval for a purely spatial cluster equals the number of
    //calculated time intervals. This would be 1 for a purely spatial analysis but
    //for a space-time analysis, the index would be dependent on # of intervals requested.
    m_nLastInterval = DataGateway.GetDataSetInterface().GetNumTimeIntervals();
  }
  catch (prg_exception& x) {
    delete gpClusterData;
    x.addTrace("constructor()","PurelySpatialHomogeneousPoissonCluster");
    throw;
  }
}

/** copy constructor */
PurelySpatialHomogeneousPoissonCluster::PurelySpatialHomogeneousPoissonCluster(const PurelySpatialHomogeneousPoissonCluster& rhs)
                      :CCluster() {
  try {
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (prg_exception& x) {
    delete gpClusterData; gpClusterData=0;
    x.addTrace("constructor()","PurelySpatialHomogeneousPoissonCluster");
    throw;
  }
}

/** destructor */
PurelySpatialHomogeneousPoissonCluster::~PurelySpatialHomogeneousPoissonCluster() {
  try { delete gpClusterData; } catch (...){}  
}

/** overloaded assignment operator */
PurelySpatialHomogeneousPoissonCluster& PurelySpatialHomogeneousPoissonCluster::operator=(const PurelySpatialHomogeneousPoissonCluster& rhs) {
  m_Center                = rhs.m_Center;
  m_MostCentralLocation   = rhs.m_MostCentralLocation;
  m_nTracts               = rhs.m_nTracts;
  m_CartesianRadius       = rhs.m_CartesianRadius;
  m_nRatio                = rhs.m_nRatio;
  _ratio_sets             = rhs._ratio_sets;
  m_nRank                 = rhs.m_nRank;
  m_NonCompactnessPenalty = rhs.m_NonCompactnessPenalty;
  m_nFirstInterval        = rhs.m_nFirstInterval;
  m_nLastInterval         = rhs.m_nLastInterval;
  m_iEllipseOffset        = rhs.m_iEllipseOffset;
  gpClusterData->Assign(*(rhs.gpClusterData));
  return *this;
}

void PurelySpatialHomogeneousPoissonCluster::CopyEssentialClassMembers(const CCluster& rhs) {
  m_Center                = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_Center;
  m_MostCentralLocation   = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_MostCentralLocation;
  m_nTracts               = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_nTracts;
  m_CartesianRadius       = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_CartesianRadius;
  m_nRatio                = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_nRatio;
  _ratio_sets             = ((const PurelySpatialHomogeneousPoissonCluster&)rhs)._ratio_sets;
  m_nRank                 = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_nRank;
  m_NonCompactnessPenalty = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_NonCompactnessPenalty;
  m_nFirstInterval        = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_nFirstInterval;
  m_nLastInterval         = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_nLastInterval;
  m_iEllipseOffset        = ((PurelySpatialHomogeneousPoissonCluster&)rhs).m_iEllipseOffset;
  gpClusterData->CopyEssentialClassMembers(*(rhs.GetClusterData()));
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and calculates loglikelihood ratio. 
    Updates cluster set given current state of this cluster object. */
void PurelySpatialHomogeneousPoissonCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                       const CentroidNeighbors& CentroidDef,
                                                                       const CentroidNeighborCalculator::LocationDistContainer_t& locDist,
                                                                       CClusterSet& clusterSet,
                                                                       AbstractLikelihoodCalculator& Calculator) {
  measure_t tAdjustment = DataGateway.GetDataSetInterface().GetTotalMeasureAuxCount();
  for (tract_t t=0, tNumNeighbors=CentroidDef.GetNumNeighbors(); t < tNumNeighbors; ++t) {
    ++m_nTracts;
    gpClusterData->gtCases = m_nTracts;
    gpClusterData->gtMeasure = pow(locDist[t].GetDistance(),2.0) * PI * tAdjustment;
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    clusterSet.update(*this);
  }
  clusterSet.maximizeClusterSet();
}

/** returns newly cloned CPurelySpatialCluster */
PurelySpatialHomogeneousPoissonCluster * PurelySpatialHomogeneousPoissonCluster::Clone() const {
  return new PurelySpatialHomogeneousPoissonCluster(*this);
}

void PurelySpatialHomogeneousPoissonCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  if (DataHub.GetParameters().GetIsSpaceTimeAnalysis())
    CCluster::DisplayTimeFrame(fp, DataHub, PrintFormat);
}

/** returns end date of defined cluster as formated string */
std::string& PurelySpatialHomogeneousPoissonCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1, eDatePrint, sep);
}

/** Returns the measure for location as defined by cluster. */
measure_t PurelySpatialHomogeneousPoissonCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex, bool adjusted) const {
  //return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
  std::vector<double>   ClusterCenter,TractCoords;
  DataHub.GetGInfo()->retrieveCoordinates(m_Center, ClusterCenter);
  CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, DataHub.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), TractCoords);
  double distance = std::sqrt(DataHub.GetTInfo()->getDistanceSquared(ClusterCenter, TractCoords));
  measure_t tAdjustment = DataHub.GetDataSetHandler().GetDataSet().getTotalMeasureAux();
  return pow(distance,2.0) * PI * tAdjustment;
}

/** returns the number of cases for location as defined by cluster */
count_t PurelySpatialHomogeneousPoissonCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return 1;//Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** returns start date of defined cluster as formated string */
std::string& PurelySpatialHomogeneousPoissonCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0], eDatePrint, sep);
}

/** re-initializes cluster data */
void PurelySpatialHomogeneousPoissonCluster::Initialize(tract_t nCenter) {
  m_Center = nCenter;
  m_MostCentralLocation = -1;
  m_nTracts = 0;
  m_nRatio = 0;
  m_CartesianRadius = -1;
  gpClusterData->InitializeData();
}

