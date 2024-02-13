//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialCentricAnalysis.h"
#include "ClusterDataFactory.h"
#include "NormalClusterDataFactory.h"
#include "CategoricalClusterDataFactory.h"
#include "SSException.h"

/** constructor */
PurelySpatialCentricAnalysis::PurelySpatialCentricAnalysis(
    const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
    const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways)
:AbstractCentricAnalysis(Parameters, Data, PrintDirection), _top_clusters(Data) {
    try {
        //allocate objects used to evaluate real data
        _cluster_compare.reset(new CPurelySpatialCluster(_cluster_data_factory, RealDataGateway));
        _top_clusters.setTopClusters(*_cluster_compare.get());
        //allocate objects used to evaluate simulation data
        if (_parameters.GetNumReplicationsRequested()) {
            gvMeasureLists.killAll();
            //create simulation objects based upon which process used to perform simulations
            if (_replica_process_type == MeasureListEvaluation) {
                //create a measure list object for each requested replication - we do this inorder
                //to prevent excess calls to loglikelihood calculation method; unfortunately this
                //also consumes more memory...
                _cluster_data.reset(new SpatialData(*(*vSimDataGateways.begin())));
                for (size_t t=0; t < vSimDataGateways.size(); ++t)
                    gvMeasureLists.push_back(GetNewMeasureListObject());
            } else {
                //Simulations performed using same process as real data set. Since we will be taking
                //the greatest loglikelihood ratios among all centroids, but analyzing each centroids
                //replications separately, we need to maintain a vector of llr values.
                gCalculatedRatios.reset(new std::vector<double>(_parameters.GetNumReplicationsRequested(), 0));
                _cluster_data_sim.reset(_cluster_data_factory->GetNewSpatialClusterData(*(*vSimDataGateways.begin())));
            }
        }
    } catch (prg_exception& x) {
        gvMeasureLists.killAll(); gCalculatedRatios.reset();
        x.addTrace("constructor()","PurelySpatialCentricAnalysis");
        throw;
    }
}

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void PurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  std::vector<double>::iterator              itrLoglikelihoodRatios;
  tract_t                                    t, tNumNeighbors;

  //perform simulations about current centroid
  itrGateway=vDataGateways.begin();
  itrLoglikelihoodRatios=gCalculatedRatios->begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
     _cluster_data_sim->InitializeData();
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     for (t=0; t < tNumNeighbors; ++t) {
        //update cluster data
        _cluster_data_sim->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
        //calculate loglikehood ratio and compare against current top cluster
        *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, _cluster_data_sim->CalculateLoglikelihoodRatio(*_likelihood_calculator));
     }
     //NOTE: This process assumes no-compactness correction for ellipses - otherwise we would
     //      use an IntermediateClustersContainer, as used in other top cluster method to
     //      calculates most likely cluster among circle and ellipses. Not sure how to code this
     //      should we want compactnes correction later on.
  }
}

/** Calculates clustering with greastest loglikelihood ratio, storing results in gTopCluster
    class member. Note that gTopCluster is re-intialized when centroid being evaluated is
    different than that of gTopCluster; this permits repeated calls to this function, inorder
    to compare clusterings about same centroid but with different ellipse shapes.

    Caller is responsible for ensuring:
    1) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data */
void PurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
  if (_top_clusters.getClusterSet(0).getSet().front().getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
    //re-intialize top cluster objects if evaluating data about new centroid
    _top_clusters.resetAboutCentroid(CentroidDef.GetCentroidIndex());
  _cluster_compare->Initialize(CentroidDef.GetCentroidIndex());
  _cluster_compare->SetEllipseOffset(CentroidDef.GetEllipseIndex(), _data_hub);
  _top_clusters.resetNeighborCounts(CentroidDef);
  _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_likelihood_calculator);
  //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
  _top_clusters.setClusterNonPersistantNeighborInfo(CentroidDef);
}

/** Returns cluster object with greatest llr value as specified by gTopCluster.
    Note that this object will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const SharedClusterVector_t PurelySpatialCentricAnalysis::GetTopCalculatedClusters() {
  SharedClusterVector_t topClusters; 
  return _top_clusters.getTopClusters(topClusters);
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void PurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
  DataSetGatewayContainer_t::const_iterator  itrGateway, itrGatewayEnd=vDataGateways.end();
  MeasureListContainer_t::const_iterator     itrMeasureList;

  //perform simulations about current centroid
  itrGateway = vDataGateways.begin();
  itrMeasureList=gvMeasureLists.begin();
  for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList)
     _cluster_data->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
     //NOTE: This process assumes no-compactness correction for ellipses - otherwise
     //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
     //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
     //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
     //      would increase dramtically compared to other Monte Carlo method. We would need an additional
     //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
     //      that would increase the amount of memory needed.
}
