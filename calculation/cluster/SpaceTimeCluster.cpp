//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeCluster.h"
#include "SSException.h"

/** constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway)
:CCluster(), gpClusterData(0){
    try {
        gpClusterData = pClusterFactory->GetNewSpaceTimeClusterData(DataGateway);
    } catch (prg_exception& x) {
        delete gpClusterData;
        x.addTrace("constructor()","CSpaceTimeCluster");
        throw;
    }
}

/** copy constructor */
CSpaceTimeCluster::CSpaceTimeCluster(const CSpaceTimeCluster& rhs) : gpClusterData(0) /* : CCluster()*/ {
    try {
        m_Center = rhs.m_Center;
        _central_identifier = rhs._central_identifier;
        _num_identifiers = rhs._num_identifiers;
        m_CartesianRadius = rhs.m_CartesianRadius;
        m_nRatio = rhs.m_nRatio;
        _ratio_sets = rhs._ratio_sets;
        m_nRank = rhs.m_nRank;
        m_NonCompactnessPenalty = rhs.m_NonCompactnessPenalty;
        m_nFirstInterval = rhs.m_nFirstInterval;
        m_nLastInterval = rhs.m_nLastInterval;
        m_iEllipseOffset = rhs.m_iEllipseOffset;
        gpClusterData = rhs.gpClusterData->Clone();
        //*this = rhs;
    } catch (prg_exception& x) {
        delete gpClusterData;
        x.addTrace("copy constructor()","CSpaceTimeCluster");
        throw;
    }
}

/** destructor */
CSpaceTimeCluster::~CSpaceTimeCluster() {
    try { delete gpClusterData; } catch(...){}
}

/** overloaded assignment operator */
CSpaceTimeCluster& CSpaceTimeCluster::operator=(const CSpaceTimeCluster& rhs) {
    m_Center                      = rhs.m_Center;
    _central_identifier           = rhs._central_identifier;
    _num_identifiers              = rhs._num_identifiers;
    m_CartesianRadius             = rhs.m_CartesianRadius;
    m_nRatio                      = rhs.m_nRatio;
    _ratio_sets                   = rhs._ratio_sets;
    m_nRank                       = rhs.m_nRank;
    m_NonCompactnessPenalty       = rhs.m_NonCompactnessPenalty;
    m_nFirstInterval              = rhs.m_nFirstInterval;
    m_nLastInterval               = rhs.m_nLastInterval;
    m_iEllipseOffset              = rhs.m_iEllipseOffset;
    gpClusterData->Assign(*(rhs.gpClusterData));
    return *this;
}

/** returns newly cloned CSpaceTimeCluster */
CSpaceTimeCluster * CSpaceTimeCluster::Clone() const {
    return new CSpaceTimeCluster(*this);
}

void CSpaceTimeCluster::CopyEssentialClassMembers(const CCluster& rhs) {
    m_Center                      = ((const CSpaceTimeCluster&)rhs).m_Center;
    _central_identifier           = ((const CSpaceTimeCluster&)rhs)._central_identifier;
    _num_identifiers              = ((const CSpaceTimeCluster&)rhs)._num_identifiers;
    m_CartesianRadius             = ((const CSpaceTimeCluster&)rhs).m_CartesianRadius;
    m_nRatio                      = ((const CSpaceTimeCluster&)rhs).m_nRatio;
    _ratio_sets                   = ((const CSpaceTimeCluster&)rhs)._ratio_sets;
    m_nRank                       = ((const CSpaceTimeCluster&)rhs).m_nRank;
    m_NonCompactnessPenalty       = ((const CSpaceTimeCluster&)rhs).m_NonCompactnessPenalty;
    m_nFirstInterval              = ((const CSpaceTimeCluster&)rhs).m_nFirstInterval;
    m_nLastInterval               = ((const CSpaceTimeCluster&)rhs).m_nLastInterval;
    m_iEllipseOffset              = ((const CSpaceTimeCluster&)rhs).m_iEllipseOffset;
    gpClusterData->CopyEssentialClassMembers(*(rhs.GetClusterData()));
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSpaceTimeCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex, bool adjusted) const {
    measure_t tMeasure, ** ppMeasure = Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
    const CParameters& params = Data.GetParameters();
    if (m_nLastInterval == Data.GetNumTimeIntervals())
        tMeasure = ppMeasure[m_nFirstInterval][tTractIndex];
    else
        tMeasure = ppMeasure[m_nFirstInterval][tTractIndex] - ppMeasure[m_nLastInterval][tTractIndex];
    // The adjusted parameter is intended for Bernoulli space-time time-stratified with in the context of calculating relative risk.
    if (adjusted) {
        if (params.GetProbabilityModelType() == BERNOULLI && params.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
            double totalCases = Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nFirstInterval] - Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nLastInterval];
            double totalMeasure = Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nFirstInterval] - Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nLastInterval];
            return tMeasure * (totalCases / totalMeasure);
        }
        return Data.GetMeasureAdjustment(tSetIndex) * tMeasure;
    }
    return tMeasure;
}

/** returns the number of cases for tract as defined by cluster */
count_t CSpaceTimeCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
    count_t      tCaseCount, ** ppCases = Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray();
    if (m_nLastInterval == Data.GetNumTimeIntervals())
        tCaseCount = ppCases[m_nFirstInterval][tTractIndex];
    else
        tCaseCount = ppCases[m_nFirstInterval][tTractIndex] - ppCases[m_nLastInterval][tTractIndex];
  return tCaseCount;
}

/** returns the number of cases for tract outside cluster window */
count_t CSpaceTimeCluster::GetCountForTractOutside(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
	count_t      tCaseCount, ** ppCases = Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray();
	if (m_nLastInterval == Data.GetNumTimeIntervals())
		tCaseCount = ppCases[0][tTractIndex] - ppCases[m_nFirstInterval][tTractIndex];
	else
		tCaseCount = (ppCases[0][tTractIndex] - ppCases[m_nFirstInterval][tTractIndex]) + ppCases[m_nLastInterval][tTractIndex];
	return tCaseCount;
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and calculates loglikelihood ratio. 
    Updates cluster set given current state of this cluster object. */
void CSpaceTimeCluster::CalculateTopClusterAboutCentroidDefinition(
    const AbstractDataSetGateway & DataGateway, const CentroidNeighbors& CentroidDef, CClusterSet& clusterSet, CTimeIntervals& TimeIntervals
) {
    tract_t * pIntegerArray = CentroidDef.GetRawIntegerArray();
    unsigned short * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
    for (tract_t t=0, tNumNeighbors = CentroidDef.GetNumNeighbors(); t < tNumNeighbors; ++t) {
        ++_num_identifiers;
        gpClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        TimeIntervals.CompareClusterSet(*this, clusterSet);
    }
}

/** re-initializes cluster data */
void CSpaceTimeCluster::Initialize(tract_t nCenter) {
    m_Center = nCenter;
    _central_identifier = -1;
    _num_identifiers = 0;
    m_nRatio = 0;
    m_CartesianRadius = -1;  
    gpClusterData->InitializeData();
}
