//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalModel.h"
#include "SaTScanData.h"
#include "cluster.h"

/** constructor */
OrdinalModel::OrdinalModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
             :CModel(Parameters, DataHub, PrintDirection) {}

/** destructor */
OrdinalModel::~OrdinalModel() {}

void OrdinalModel::CalculateMeasure(RealDataSet&) {
   /* no action here */
}

/** Returns population as defined in CCluster object. */
double OrdinalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster) const {
  double                dPopulation=0;
  tract_t               tNeighborIndex;
  count_t            ** ppCases;

  const PopulationData& Population = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetPopulationData();
  for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
     ppCases = gDataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetCategoryCaseArray(t);
     for (int j=1; j <= Cluster.GetNumTractsInnerCircle(); ++j) {
        tNeighborIndex = gDataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), j);
        dPopulation += ppCases[Cluster.m_nFirstInterval][tNeighborIndex] - ppCases[Cluster.m_nLastInterval][tNeighborIndex];
     }
  }
  return dPopulation;
}

