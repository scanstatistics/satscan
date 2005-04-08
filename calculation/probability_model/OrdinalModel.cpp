//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalModel.h"
#include "SaTScanData.h"
#include "cluster.h"

/** constructor */
OrdinalModel::OrdinalModel() : CModel() {}

/** destructor */
OrdinalModel::~OrdinalModel() {}

/** empty function - for the ordinal model, the routine to calculate expected cases
    is not needed (generally the measure array) since calculation of loglikelihood
    based upon cases in each category and total cases in each category */
void OrdinalModel::CalculateMeasure(RealDataSet&) {/* no action here */}

/** Returns population as defined in CCluster object for data set at index. */
double OrdinalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
  double                dPopulation=0;
  tract_t               tNeighborIndex;

  const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetPopulationData();

  if (DataHub.GetParameters().GetIsPurelyTemporalAnalysis()) {
    for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
       count_t * pCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetPTCategoryCasesArray()[t];
       dPopulation += pCases[Cluster.m_nFirstInterval] - pCases[Cluster.m_nLastInterval];
    }
  }
  else {
    for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
       count_t ** ppCases = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetCategoryCaseArray(t);
       for (int j=1; j <= Cluster.GetNumTractsInnerCircle(); ++j) {
          tNeighborIndex = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), j);
          dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex] - ppCases[Cluster.m_nLastInterval][tNeighborIndex];
       }
    }
  }
  return dPopulation;
}

