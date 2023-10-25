//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalModel.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "SSException.h"
#include "ClosedLoopData.h"

/** constructor */
OrdinalModel::OrdinalModel() : CModel() {}

/** destructor */
OrdinalModel::~OrdinalModel() {}

/** empty function - for the ordinal model, the routine to calculate expected cases
    is not needed (generally the measure array) since calculation of loglikelihood
    based upon cases in each category and total cases in each category */
void OrdinalModel::CalculateMeasure(RealDataSet&, const CSaTScanData&) {/* no action here */}

/** Returns population as defined in CCluster object for data set at index. */
double OrdinalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    double  dPopulation=0;
    tract_t tNeighborIndex;

    try {
        const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getPopulationData();
        const ClosedLoopData * seasonalhub = dynamic_cast<const ClosedLoopData*>(&DataHub);
        if (!seasonalhub && DataHub.GetParameters().GetAnalysisType() == SEASONALTEMPORAL) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "GetPopulation()");

        switch (Cluster.GetClusterType()) {
            case PURELYTEMPORALCLUSTER            :
                for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
                    count_t * pCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT_Cat().GetArray()[t];
                    if (seasonalhub) {
                        int end_interval = std::min(Cluster.m_nLastInterval, seasonalhub->getExtendedPeriodStart());
                        dPopulation += pCases[Cluster.m_nFirstInterval] - (end_interval == seasonalhub->getExtendedPeriodStart() ? 0 : pCases[end_interval]);
                        dPopulation += pCases[0] - pCases[std::max(0, Cluster.m_nLastInterval - seasonalhub->getExtendedPeriodStart())];
                    } else
                        dPopulation += pCases[Cluster.m_nFirstInterval] - pCases[Cluster.m_nLastInterval];
                } break;
            case SPACETIMECLUSTER                 :
                if (Cluster.m_nLastInterval != DataHub.GetNumTimeIntervals()) {
                    for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
                        count_t ** ppCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCategoryCaseData(t).GetArray();
                        for (int j=1; j <= Cluster.getNumIdentifiers(); ++j) {
                            tNeighborIndex = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), j, Cluster.GetCartesianRadius());
                            dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex] - ppCases[Cluster.m_nLastInterval][tNeighborIndex];
                        }
                    }
                    break;
                }
            case PURELYSPATIALCLUSTER             :
                for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
                    count_t ** ppCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCategoryCaseData(t).GetArray();
                    for (int j=1; j <= Cluster.getNumIdentifiers(); ++j) {
                        tNeighborIndex = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), j, Cluster.GetCartesianRadius());
                        dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex];
                    }
                }
                break;
            case PURELYSPATIALMONOTONECLUSTER     :
            case SPATIALVARTEMPTRENDCLUSTER       :
            case PURELYSPATIALPROSPECTIVECLUSTER  :
            default : throw prg_error("Unknown cluster type '%d'.","GetPopulation()", Cluster.GetClusterType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetPopulation()","OrdinalModel");
        throw;
    }
    return dPopulation;
}
