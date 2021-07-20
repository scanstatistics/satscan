//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialCluster.h"
#include "SSException.h"

/** constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory,
                                             const AbstractDataSetGateway & DataGateway)
                      :CCluster() {
  try {
    gpClusterData = pClusterFactory->GetNewSpatialClusterData(DataGateway);
    //The last time interval for a purely spatial cluster equals the number of
    //calculated time intervals. This would be 1 for a purely spatial analysis but
    //for a space-time analysis, the index would be dependent on # of intervals requested.
    m_nLastInterval = DataGateway.GetDataSetInterface().GetNumTimeIntervals();
  } catch (prg_exception& x) {
    delete gpClusterData;
    x.addTrace("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialCluster::CPurelySpatialCluster(const CPurelySpatialCluster& rhs)
                      :CCluster() {
  try {
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  } catch (prg_exception& x) {
    delete gpClusterData; gpClusterData=0;
    x.addTrace("constructor()","CPurelySpatialCluster");
    throw;
  }
}

/** destructor */
CPurelySpatialCluster::~CPurelySpatialCluster() {
  try { delete gpClusterData; } catch (...){}  
}

/** overloaded assignment operator */
CPurelySpatialCluster& CPurelySpatialCluster::operator=(const CPurelySpatialCluster& rhs) {
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

void CPurelySpatialCluster::CopyEssentialClassMembers(const CCluster& rhs) {
  m_Center                = ((CPurelySpatialCluster&)rhs).m_Center;
  m_MostCentralLocation   = ((CPurelySpatialCluster&)rhs).m_MostCentralLocation;
  m_nTracts               = ((CPurelySpatialCluster&)rhs).m_nTracts;
  m_CartesianRadius       = ((CPurelySpatialCluster&)rhs).m_CartesianRadius;
  m_nRatio                = ((CPurelySpatialCluster&)rhs).m_nRatio;
  _ratio_sets             = ((const CPurelySpatialCluster&)rhs)._ratio_sets;
  m_nRank                 = ((CPurelySpatialCluster&)rhs).m_nRank;
  m_NonCompactnessPenalty = ((CPurelySpatialCluster&)rhs).m_NonCompactnessPenalty;
  m_nFirstInterval        = ((CPurelySpatialCluster&)rhs).m_nFirstInterval;
  m_nLastInterval         = ((CPurelySpatialCluster&)rhs).m_nLastInterval;
  m_iEllipseOffset        = ((CPurelySpatialCluster&)rhs).m_iEllipseOffset;
  gpClusterData->CopyEssentialClassMembers(*(rhs.GetClusterData()));
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and calculates loglikelihood ratio. 
    Updates cluster set given current state of this cluster object. */
void CPurelySpatialCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                       const CentroidNeighbors& CentroidDef,
                                                                       CClusterSet& clusterSet,
                                                                       AbstractLikelihoodCalculator& Calculator) {
  tract_t * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
  for (tract_t t=0, tNumNeighbors=CentroidDef.GetNumNeighbors(); t < tNumNeighbors; ++t) {
    ++m_nTracts;
    gpClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    _ratio_sets = gpClusterData->getRatioSets(Calculator);
    clusterSet.update(*this);
  }
  clusterSet.maximizeClusterSet();
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialCluster * CPurelySpatialCluster::Clone() const {
  return new CPurelySpatialCluster(*this);
}

void CPurelySpatialCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  if (DataHub.GetParameters().GetIsSpaceTimeAnalysis())
    CCluster::DisplayTimeFrame(fp, DataHub, PrintFormat);
}

/** returns end date of defined cluster as formated string */
std::string& CPurelySpatialCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1, eDatePrint, sep);
}

/** Returns the measure for location as defined by cluster. */
measure_t CPurelySpatialCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** returns the number of cases for location as defined by cluster */
count_t CPurelySpatialCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** returns start date of defined cluster as formated string */
std::string& CPurelySpatialCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0], eDatePrint, sep);
}

/** re-initializes cluster data */
void CPurelySpatialCluster::Initialize(tract_t nCenter) {
  m_Center = nCenter;
  m_MostCentralLocation = -1;
  m_nTracts = 0;
  m_nRatio = -std::numeric_limits<double>::max();
  m_CartesianRadius = -1;
  gpClusterData->InitializeData();
}

