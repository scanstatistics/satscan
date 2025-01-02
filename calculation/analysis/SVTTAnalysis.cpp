//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SVTTAnalysis.h"
#include "SSException.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                             :CAnalysis(Parameters, DataHub, PrintDirection), _top_clusters(DataHub) {
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
    _cluster_data.reset(new SVTTClusterData(DataGateway));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
    try {
        _cluster_compare.reset(new CSVTTCluster(_cluster_data_factory.get(), DataGateway));
        _top_clusters.setTopClusters(*_cluster_compare);
    } catch (prg_exception& x) {
        x.addTrace("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
        throw;
    }
}

/** calculates most likely cluster about central location 'tCenter' */
const SharedClusterVector_t CSpatialVarTempTrendAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
    try {
        _cluster_compare->InitializeSVTT(0, DataGateway);
        _top_clusters.resetAboutCentroidSVTT(tCenter, DataGateway);
        //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
        for (int k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
            CentroidNeighbors CentroidDef(k, _data_hub, tCenter);
            _top_clusters.resetNeighborCounts(k);
            _cluster_compare->InitializeSVTT(tCenter, DataGateway);
            _cluster_compare->SetEllipseOffset(k, _data_hub);
            _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(k), *_likelihood_calculator);
        }
        SharedClusterVector_t topClusters;
        _top_clusters.getTopClusters(topClusters);
        for (size_t t=0; t < topClusters.size(); ++t) {
            if (topClusters[t]->ClusterDefined())
                dynamic_cast<CSVTTCluster*>(topClusters[t].get())->SetTimeTrend(_parameters.GetTimeAggregationUnitsType(), _parameters.GetTimeAggregationLength());
        }
        return topClusters;
    } catch (prg_exception& x) {
        x.addTrace("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
        throw;
    }
}

/** calculates loglikelihood ratio for simulated data pointed to by DataSetInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataSetInterface& Interface) {
    throw prg_error("MonteCarlo(const DataSetInterface&) not implemented.","CSpatialVarTempTrendAnalysis");
}

/** Returns calculates log likelihood ratio about centroid. Currently this function calls CalculateTopCluster()
    but will likely be updated in the future when this analysis type is made public. */
double CSpatialVarTempTrendAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  double                        dMaximumLogLikelihoodRatio;
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, 0);
  std::vector<double>::iterator itr, itr_end;

  try {
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (int k=0; k <= _parameters.GetNumTotalEllipses(); k++) {
       double& dShapeMaxValue = vMaximizingValues[k];
       CentroidNeighbors CentroidDef(k, _data_hub);
       CentroidDef.Set(tCenter);
       tNumNeighbors = CentroidDef.GetNumNeighbors();
       pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
       pIntegerArray = CentroidDef.GetRawIntegerArray();
       _cluster_data->InitializeSVTTData(DataGateway);
       for (tract_t j=0; j < tNumNeighbors; j++) {
          _cluster_data->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[j] : pIntegerArray[j]), DataGateway);
          dShapeMaxValue = std::max(dShapeMaxValue , _cluster_data->CalculateSVTTLoglikelihoodRatio(*_likelihood_calculator, DataGateway));
       }
    }
    //determine which ratio/test statistic is the greatest, be sure to apply compactness correction
    double dPenalty = _data_hub.GetParameters().GetNonCompactnessPenaltyPower();
    dMaximumLogLikelihoodRatio = vMaximizingValues.front() * CalculateNonCompactnessPenalty(_data_hub.GetEllipseShape(0), dPenalty);
    for (t=1,itr=vMaximizingValues.begin()+1,itr_end=vMaximizingValues.end(); itr != itr_end; ++itr, ++t) {
       *itr *= CalculateNonCompactnessPenalty(_data_hub.GetEllipseShape(t), dPenalty);
       dMaximumLogLikelihoodRatio = std::max(*itr, dMaximumLogLikelihoodRatio);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("MonteCarlo()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return dMaximumLogLikelihoodRatio;
}

