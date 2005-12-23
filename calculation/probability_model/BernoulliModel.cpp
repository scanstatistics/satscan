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
                     ** ppCases(DataSet.GetCaseArray()),
                     ** ppControls(DataSet.GetControlArray());
  measure_t          ** ppMeasure, tTotalMeasure(0);

  try {
    DataSet.AllocateMeasureArray();
    ppMeasure = DataSet.GetMeasureArray();

    for (j=0; j < DataSet.GetNumTracts(); ++j) {
       tTotalCases    += ppCases[0][j];
       tTotalControls += ppControls[0][j];
       for (i=0; i < DataSet.GetNumTimeIntervals(); ++i) {
          ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
       }
       tTotalMeasure += ppMeasure[0][j];

       // Check to see if total case or control values have wrapped
        if (tTotalCases < 0)
          GenerateResolvableException("Error: : The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                                      "CBernoulliModel", DataSet.GetSetIndex(), std::numeric_limits<count_t>::max());
        if (tTotalControls < 0)
          GenerateResolvableException("Error: The total number of controls in dataset %u is greater than the maximum allowed of %ld.\n",
                                      "CBernoulliModel", DataSet.GetSetIndex(), std::numeric_limits<count_t>::max());
    }

    if (tTotalControls == 0)
      GenerateResolvableException("Error: No controls found in data set %u.\n", "CBernoulliModel", DataSet.GetSetIndex());

    DataSet.SetTotalCases(tTotalCases);
    DataSet.SetTotalControls(tTotalControls);
    DataSet.SetTotalMeasure(tTotalMeasure);
    DataSet.SetTotalPopulation(tTotalMeasure);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CBernoulliModel");
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
          pPTMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetPTMeasureArray();
          dPopulation = pPTMeasure[Cluster.m_nFirstInterval] - pPTMeasure[Cluster.m_nLastInterval];
        break;
     case SPACETIMECLUSTER                 :
        if (Cluster.m_nLastInterval != DataHub.GetNumTimeIntervals()) {
          ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetMeasureArray();
          for (int i=1; i <= Cluster.GetNumTractsInnerCircle(); ++i) {
            nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
            dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor] - ppMeasure[Cluster.m_nLastInterval][nNeighbor];
          }
          break;
        }
     case PURELYSPATIALCLUSTER             :
     case PURELYSPATIALMONOTONECLUSTER     :
        ppMeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetMeasureArray();
        for (int i=1; i <= Cluster.GetNumTractsInnerCircle(); ++i) {
          nNeighbor = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), i, Cluster.GetCartesianRadius());
          dPopulation += ppMeasure[Cluster.m_nFirstInterval][nNeighbor];
        }
        break;
     case SPATIALVARTEMPTRENDCLUSTER       :
     case PURELYSPATIALPROSPECTIVECLUSTER  :
     default                               :
       ZdException::GenerateNotification("Unknown cluster type '%d'.","GetPopulation()", Cluster.GetClusterType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulation()","CBernoulliModel");
    throw;
  }

  return dPopulation;
}

