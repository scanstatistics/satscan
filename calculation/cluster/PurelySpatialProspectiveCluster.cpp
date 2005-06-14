//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialProspectiveCluster.h"
#include "MeasureList.h"

/** Purely spatial cluster to be used in simulations of prospective analysis.*/

/** constructor */
CPurelySpatialProspectiveCluster::CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory,
                                                                   const AbtractDataSetGateway & DataGateway,
                                                                   const CSaTScanData & Data)
                                 :CCluster() {
  try {
    Init();
    Setup(pClusterFactory, DataGateway, Data);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialProspectiveCluster");
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
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("copy constructor()","CPurelySpatialProspectiveCluster");
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
CPurelySpatialProspectiveCluster& CPurelySpatialProspectiveCluster::operator=(const CPurelySpatialProspectiveCluster& cluster) {
  m_Center                      = cluster.m_Center;
  m_nTracts                     = cluster.m_nTracts;
  m_CartesianRadius             = rhs.m_CartesianRadius;   
  m_nRatio                      = cluster.m_nRatio;
  m_nRank                       = cluster.m_nRank;
  m_NonCompactnessPenalty       = cluster.m_NonCompactnessPenalty;
  m_nFirstInterval              = cluster.m_nFirstInterval;
  m_nLastInterval               = cluster.m_nLastInterval;
  m_iEllipseOffset              = cluster.m_iEllipseOffset;
  gpClusterData->Assign(*cluster.gpClusterData);
  return *this;
}

/** returns newly cloned CPurelySpatialCluster */
CPurelySpatialProspectiveCluster * CPurelySpatialProspectiveCluster::Clone() const {
  return new CPurelySpatialProspectiveCluster(*this);
}

/** returns end date of defined cluster as formated string */
ZdString& CPurelySpatialProspectiveCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  sDateString.Clear();
  return sDateString;
}

/** Returns the measure for tract as defined by cluster - not implemented - throws exception. */
measure_t CPurelySpatialProspectiveCluster::GetExpectedCountForTract(tract_t, const CSaTScanData&, size_t) const {
  ZdGenerateException("GetExpectedCountForTract() not implemented.","CPurelySpatialProspectiveCluster");
  return 0;
}

/** Returns the number of case for tract as defined by cluster - not implemented - throws exception. */
count_t CPurelySpatialProspectiveCluster::GetObservedCountForTract(tract_t, const CSaTScanData&, size_t) const {
  ZdGenerateException("GetObservedCountForTract() not implemented.","CPurelySpatialProspectiveCluster");
  return 0;
}

/** returns start date of defined cluster as formated string */
ZdString& CPurelySpatialProspectiveCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  sDateString.Clear();
  return sDateString;
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and
    evaluates for significant clusterings. Assigns greastest clustering to 'TopCluster'. */
void CPurelySpatialProspectiveCluster::CalculateTopClusterAboutCentroidDefinition(const AbtractDataSetGateway& DataGateway,
                                                                                  const CentroidNeighbors& CentroidDef,
                                                                                  CPurelySpatialProspectiveCluster& TopCluster,
                                                                                  AbstractLikelihoodCalculator& Calculator) {
  tract_t       t, tNumNeighbors = CentroidDef.GetNumNeighbors();

  for (t=0; t < tNumNeighbors; ++t) {
    //update cluster data
    ++m_nTracts;
    gpClusterData->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), DataGateway);
    //calculate loglikehood ratio and compare against current top cluster
    m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
    if (m_nRatio > TopCluster.m_nRatio)
      TopCluster.AssignAsType(*this);
  }
}

/** re-initializes cluster data */
void CPurelySpatialProspectiveCluster::Initialize(tract_t nCenter = 0) {
  CCluster::Initialize(nCenter);
  gpClusterData->InitializeData();
}

void CPurelySpatialProspectiveCluster::Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataSetGateway & DataGateway, const CSaTScanData & Data) {
  try {
    gpClusterData = pClusterFactory->GetNewProspectiveSpatialClusterData(Data, DataGateway);
  }
  catch (ZdException &x) {
    delete gpClusterData;
    x.AddCallpath("Setup()","CPurelySpatialProspectiveCluster");
    throw;
  }
}

