//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialProspectiveCluster.h"
#include "MeasureList.h"
#include "SSException.h"

/** Purely spatial cluster to be used in simulations of prospective analysis.*/

/** constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory,
                                                                   const AbstractDataSetGateway & DataGateway,
                                                                   const CSaTScanData & Data)
                                 :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway, Data);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** copy constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs)
                                 :CCluster() {
  try {
    Init();
    gpClusterData = rhs.gpClusterData->Clone();
    *this = rhs;
  }
  catch (prg_exception& x) {
    delete gpClusterData;
    x.addTrace("copy constructor()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

/** destructor */
CPurelySpatialProspectiveCluster::~CPurelySpatialProspectiveCluster() {
  try {
    delete gpClusterData;
  }
  catch(...){}
}

/** overloaded assignment operator */
CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator=(const CPurelySpatialProspectiveCluster& rhs) {
  m_Center                      = rhs.m_Center;
  m_MostCentralLocation         = rhs.m_MostCentralLocation;
  m_nTracts                     = rhs.m_nTracts;
  m_CartesianRadius             = rhs.m_CartesianRadius;
  m_nRatio                      = rhs.m_nRatio;
  m_nRank                       = rhs.m_nRank;
  m_NonCompactnessPenalty       = rhs.m_NonCompactnessPenalty;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  m_iEllipseOffset              = rhs.m_iEllipseOffset;
  gpClusterData->Assign(*rhs.gpClusterData);
  return *this;
}

void CPurelySpatialProspectiveCluster::CopyEssentialClassMembers(const CCluster& rhs) {
  m_Center                      = ((CPurelySpatialProspectiveCluster&)rhs).m_Center;
  m_MostCentralLocation         = ((CPurelySpatialProspectiveCluster&)rhs).m_MostCentralLocation;
  m_nTracts                     = ((CPurelySpatialProspectiveCluster&)rhs).m_nTracts;
  m_CartesianRadius             = ((CPurelySpatialProspectiveCluster&)rhs).m_CartesianRadius;
  m_nRatio                      = ((CPurelySpatialProspectiveCluster&)rhs).m_nRatio;
  m_nRank                       = ((CPurelySpatialProspectiveCluster&)rhs).m_nRank;
  m_NonCompactnessPenalty       = ((CPurelySpatialProspectiveCluster&)rhs).m_NonCompactnessPenalty;
  m_nFirstInterval              = ((CPurelySpatialProspectiveCluster&)rhs).m_nFirstInterval;
  m_nLastInterval               = ((CPurelySpatialProspectiveCluster&)rhs).m_nLastInterval;
  m_iEllipseOffset              = ((CPurelySpatialProspectiveCluster&)rhs).m_iEllipseOffset;
  gpClusterData->CopyEssentialClassMembers(*rhs.GetClusterData());
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  return new CPurelySpatialProspectiveCluster(*this);
}

/** returns end date of defined cluster as formated string */
std::string& CPurelySpatialProspectiveCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  sDateString.clear();
  return sDateString;
}

/** Returns the measure for tract as defined by cluster - not implemented - throws exception. */
measure_t CPurelySpatialProspectiveCluster::GetExpectedCountForTract(tract_t, const CSaTScanData&, size_t) const {
  throw prg_error("GetExpectedCountForTract() not implemented.","CPurelySpatialProspectiveCluster");
}

/** Returns the number of case for tract as defined by cluster - not implemented - throws exception. */
count_t CPurelySpatialProspectiveCluster::GetObservedCountForTract(tract_t, const CSaTScanData&, size_t) const {
  throw prg_error("GetObservedCountForTract() not implemented.","CPurelySpatialProspectiveCluster");
}

/** returns start date of defined cluster as formated string */
std::string& CPurelySpatialProspectiveCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  sDateString.clear();
  return sDateString;
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and
    evaluates for significant clusterings. Assigns greastest clustering to 'TopCluster'. */
void CPurelySpatialProspectiveCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                                  const CentroidNeighbors& CentroidDef,
                                                                                  CPurelySpatialProspectiveCluster& TopCluster,
                                                                                  AbstractLikelihoodCalculator& Calculator) {
  tract_t               t, tNumNeighbors = CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  for (t=0; t < tNumNeighbors; ++t) {
    //update cluster data
    ++m_nTracts;
    gpClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
    //calculate loglikehood ratio and compare against current top cluster
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    if (m_nRatio > TopCluster.m_nRatio)
      TopCluster.CopyEssentialClassMembers(*this);
  }
}

/** re-initializes cluster data */
void CPurelySpatialProspectiveCluster::Initialize(tract_t nCenter = 0) {
  CCluster::Initialize(nCenter);
  gpClusterData->InitializeData();
}

void CPurelySpatialProspectiveCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, const CSaTScanData & Data) {
  try {
    gpClusterData = pClusterFactory->GetNewProspectiveSpatialClusterData(Data, DataGateway);
  }
  catch (prg_exception& x) {
    delete gpClusterData;
    x.addTrace("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

