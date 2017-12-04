//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialModel.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "SSException.h"
#include "ClosedLoopData.h"

/** constructor */
ExponentialModel::ExponentialModel() : CModel() {}

/** destructor */
ExponentialModel::~ExponentialModel() {}

/** Calculate expected number of cases -- no action taken in this function.
    Exponential model use the measure structures but data calculated in read process. */
void ExponentialModel::CalculateMeasure(RealDataSet&, const CSaTScanData&) {/* no action here */}

/** Returns population as defined in CCluster object. */
double ExponentialModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    double dPopulation=0.0;
    tract_t tNeighborIndex;
    count_t ** ppCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray(),
            ** ppCensoredCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_Censored().GetArray();

    try {
        const ClosedLoopData * seasonalhub = dynamic_cast<const ClosedLoopData*>(&DataHub);
        if (!seasonalhub && DataHub.GetParameters().GetAnalysisType() == SEASONALTEMPORAL) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "GetPopulation()");

        switch (Cluster.GetClusterType()) {
            case PURELYTEMPORALCLUSTER            :
                for (tract_t t=0; t < DataHub.GetNumTracts(); ++t) {
                    if (seasonalhub) {
                        int end_interval = std::min(Cluster.m_nLastInterval, seasonalhub->getExtendedPeriodStart());
                        dPopulation += ppCases[Cluster.m_nFirstInterval][t] - (end_interval == seasonalhub->getExtendedPeriodStart() ? 0 : ppCases[end_interval][t]);
                        dPopulation += ppCases[0][t] - ppCases[std::max(0, Cluster.m_nLastInterval - seasonalhub->getExtendedPeriodStart())][t];
                        dPopulation += ppCensoredCases[Cluster.m_nFirstInterval][t] - (end_interval == seasonalhub->getExtendedPeriodStart() ? 0 : ppCensoredCases[end_interval][t]);
                        dPopulation += ppCensoredCases[0][t] - ppCensoredCases[std::max(0, Cluster.m_nLastInterval - seasonalhub->getExtendedPeriodStart())][t];
                    } else {
                        dPopulation += ppCases[Cluster.m_nFirstInterval][t] - (Cluster.m_nLastInterval == DataHub.GetNumTimeIntervals() ? 0 : ppCases[Cluster.m_nLastInterval][t]);
                        dPopulation += ppCensoredCases[Cluster.m_nFirstInterval][t] - (Cluster.m_nLastInterval == DataHub.GetNumTimeIntervals() ? 0 : ppCensoredCases[Cluster.m_nLastInterval][t]);
                    }
                }
                break;
            case SPACETIMECLUSTER                 :
                if (Cluster.m_nLastInterval != DataHub.GetNumTimeIntervals()) {
                    for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
                        tNeighborIndex = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i);
                        dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex] - ppCases[Cluster.m_nLastInterval][tNeighborIndex];
                        dPopulation += ppCensoredCases[Cluster.m_nFirstInterval][tNeighborIndex] - ppCensoredCases[Cluster.m_nLastInterval][tNeighborIndex];
                    }
                    break;
                }
            case PURELYSPATIALCLUSTER             :
                for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
                    tNeighborIndex = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
                    dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex];
                    dPopulation += ppCensoredCases[Cluster.m_nFirstInterval][tNeighborIndex];
                }
                break;
            case PURELYSPATIALMONOTONECLUSTER     :
            case SPATIALVARTEMPTRENDCLUSTER       :
            case PURELYSPATIALPROSPECTIVECLUSTER  :
            default : throw prg_error("Unknown cluster type '%d'.","GetPopulation()", Cluster.GetClusterType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetPopulation()","ExponentialModel");
        throw;
    }
    return dPopulation;
}
