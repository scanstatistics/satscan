//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliModel.h"
#include "SSException.h"                                               

/** Constructor */
CBernoulliModel::CBernoulliModel() : CModel() {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

/** calculates expected number of cases */
void CBernoulliModel::CalculateMeasure(RealDataSet& DataSet) {
  unsigned int          i, j;
  count_t               tTotalCases(0), tTotalControls(0),
                     ** ppCases = DataSet.getCaseData().GetArray(),
                     ** ppControls = DataSet.getControlData().GetArray();
  measure_t          ** ppMeasure, tTotalMeasure(0);

  try {
    ppMeasure = DataSet.allocateMeasureData().GetArray();
    for (j=0; j < DataSet.getLocationDimension(); ++j) {
       tTotalCases    += ppCases[0][j];
       tTotalControls += ppControls[0][j];
       for (i=0; i < DataSet.getIntervalDimension(); ++i) {
          ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
       }
       tTotalMeasure += ppMeasure[0][j];

       // Check to see if total case or control values have wrapped
        if (tTotalCases < 0)
          throw resolvable_error("Error: : The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                                  DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
        if (tTotalControls < 0)
          throw resolvable_error("Error: The total number of controls in dataset %u is greater than the maximum allowed of %ld.\n",
                                  DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
    }

    if (tTotalControls == 0)
      throw resolvable_error("Error: No controls found in data set %u.\n", DataSet.getSetIndex());

    DataSet.setTotalCases(tTotalCases);
    DataSet.setTotalControls(tTotalControls);
    DataSet.setTotalMeasure(tTotalMeasure);
    DataSet.setTotalPopulation(tTotalMeasure);
  }
  catch (prg_exception &x) {
    x.addTrace("CalculateMeasure()","CBernoulliModel");
    throw;
  }
}

/** Returns population as defined in CCluster object. */
double CBernoulliModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
  double                dPopulation=0.0;
  count_t               nNeighbor;
  measure_t           * pPTMeasure, ** ppMeasure;

  try {
    switch (Cluster.GetClusterType()) {
     case PURELYTEMPORALCLUSTER            :
          pPTMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT();
          dPopulation = pPTMeasure[Cluster.m_nFirstInterval] - pPTMeasure[Cluster.m_nLastInterval];
        break;
     case SPACETIMECLUSTER                 :
        if (Cluster.m_nLastInterval != DataHub.GetNumTimeIntervals()) {
          ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
          for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
            nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
            dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor] - ppMeasure[Cluster.m_nLastInterval][nNeighbor];
          }
          break;
        }
     case PURELYSPATIALCLUSTER             :
     case PURELYSPATIALMONOTONECLUSTER     :
        ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
        for (int i=1; i <= Cluster.GetNumTractsInCluster(); ++i) {
          nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
          dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor];
        }
        break;
     case SPATIALVARTEMPTRENDCLUSTER       :
     case PURELYSPATIALPROSPECTIVECLUSTER  :
     default                               :
       throw prg_error("Unknown cluster type '%d'.","GetPopulation()", Cluster.GetClusterType());
    }
  }
  catch (prg_exception &x) {
    x.addTrace("GetPopulation()","CBernoulliModel");
    throw;
  }

  return dPopulation;
}

