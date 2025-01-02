//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "SSException.h"

/** constructor */
SpaceTimeIncludePurelySpatialCentricAnalysis::SpaceTimeIncludePurelySpatialCentricAnalysis(
    const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
    const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways)
:SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways), _top_ps_Clusters(Data) {
    try {
        // Allocate objects used to evaluate real data.
        _ps_cluster_compare.reset(new CPurelySpatialCluster(_cluster_data_factory.get(), RealDataGateway));
        _top_ps_Clusters.setTopClusters(*_ps_cluster_compare.get());
        // Allocate objects used to evaluate simulation data.
        if (_parameters.GetNumReplicationsRequested()) {
            if (_replica_process_type == MeasureListEvaluation) {
                if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
                    _psp_cluster_data.reset(new ProspectiveSpatialData(_data_hub, *(*vSimDataGateways.begin())));
                else
                    _ps_cluster_data.reset(new SpatialData(*(*vSimDataGateways.begin())));
            } else {
                if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
                    _ps_cluster_data_cl_eval.reset(_cluster_data_factory->GetNewProspectiveSpatialClusterData(_data_hub, *(*vSimDataGateways.begin())));
                else
                    _ps_cluster_data_cl_eval.reset(_cluster_data_factory->GetNewSpatialClusterData(*(*vSimDataGateways.begin())));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("constructor()","SpaceTimeIncludePurelySpatialCentricAnalysis");
        throw;
    }
}

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    _time_Intervals_s->setIntervalRange(CentroidDef.GetCentroidIndex());
    //perform simulations about current centroid
    DataSetGatewayContainer_t::const_iterator itrGateway = vDataGateways.begin(), itrGatewayEnd = vDataGateways.end();
    std::vector<double>::iterator itrLoglikelihoodRatios = gCalculatedRatios->begin();
    tract_t t, tNumNeighbors;
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
        _ps_cluster_data_cl_eval->InitializeData();
        _cluster_data_pt->InitializeData();
        tNumNeighbors = CentroidDef.GetNumNeighbors();
        for (t=0; t < tNumNeighbors; ++t) {
            //update cluster data
            _ps_cluster_data_cl_eval->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
            //calculate loglikehood ratio and compare against current top cluster
            *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, _ps_cluster_data_cl_eval->CalculateLoglikelihoodRatio(*_likelihood_calculator));
            //update cluster data
            _cluster_data_pt->AddNeighborData(CentroidDef.GetNeighborTractIndex(t), *(*itrGateway));
            //calculate loglikehood ratio and compare against current top cluster
            *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, _time_Intervals_s->ComputeMaximizingValue(*_cluster_data_pt));
        }
        //NOTE: This process assumes no-compactness correction for ellipses - otherwise we would
        //      use an IntermediateClustersContainer, as used in other top cluster method to
        //      calculates most likely cluster among circle and ellipses. Not sure how to code this
        //      should we want compactnes correction later on.
    }
}

/** Calculates clustering with greatest loglikelihood ratio, storing results in gTopCluster
    class member. Note that gTopCluster is re-intialized when centroid being evaluated is
    different than that of gTopCluster; this permits repeated calls to this function, inorder
    to compare clusterings about same centroid but with different ellipse shapes. */
void SpaceTimeIncludePurelySpatialCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
    if (_top_ps_Clusters.getClusterSet(0).getSet().front().getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
        _top_ps_Clusters.resetAboutCentroid(CentroidDef.GetCentroidIndex()); //re-intialize top cluster objects if evaluating data about new centroid
    _ps_cluster_compare->Initialize(CentroidDef.GetCentroidIndex());
    _top_ps_Clusters.resetNeighborCounts(CentroidDef);
    _ps_cluster_compare->SetEllipseOffset(CentroidDef.GetEllipseIndex(), _data_hub);
    _ps_cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_ps_Clusters.getClusterSet(0), *_likelihood_calculator);
    //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
    _top_ps_Clusters.setClusterNonPersistantNeighborInfo(CentroidDef);

    if (_top_clusters.getClusterSet(0).getSet().front().getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
        _top_clusters.resetAboutCentroid(CentroidDef.GetCentroidIndex()); //re-intialize top cluster objects if evaluating data about new centroid
    _time_intervals_r->setIntervalRange(CentroidDef.GetCentroidIndex());
    _cluster_compare->Initialize(CentroidDef.GetCentroidIndex());
    _top_clusters.resetNeighborCounts(CentroidDef);
    _cluster_compare->SetEllipseOffset(CentroidDef.GetEllipseIndex(), _data_hub);
    _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_time_intervals_r);
    //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
    _top_clusters.setClusterNonPersistantNeighborInfo(CentroidDef);
}

/** Returns cluster object with greatest llr value among gTopPSCluster and gTopCluster.
    Note that these objects will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const SharedClusterVector_t SpaceTimeIncludePurelySpatialCentricAnalysis::GetTopCalculatedClusters() {
    SharedClusterVector_t stClusters, psClusters;
    _top_clusters.getTopClusters(stClusters);
    _top_ps_Clusters.getTopClusters(psClusters);
    if (!psClusters.front()->ClusterDefined()) return stClusters; // Skip if purely spatial cluster is not defined - no gini since space-time.
    if (!stClusters.size() || !stClusters.front()->ClusterDefined()) return psClusters; // no space-time clusters, only purely spatial
    // Figure out whether the space-time cluster(s) or purely spatial cluster is the best for this centroid.
    // If this is a retrospective space-time analysis, there could be more than one space-time cluster, with greatest first in list.
    return macro_less_than(stClusters.front()->m_nRatio, psClusters.front()->m_nRatio, DBL_CMP_TOLERANCE) ? psClusters : stClusters;
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
        return MonteCarloProspectiveAboutCentroidDefinition(CentroidDef, vDataGateways);
    _time_Intervals_s->setIntervalRange(CentroidDef.GetCentroidIndex());
    //perform simulations about current centroid
    DataSetGatewayContainer_t::const_iterator itrGateway = vDataGateways.begin(), itrGatewayEnd = vDataGateways.end();
    MeasureListContainer_t::iterator itrMeasureList = gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
        _ps_cluster_data->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
        _cluster_data->AddNeighborDataAndCompare(CentroidDef, (*itrGateway)->GetDataSetInterface(), *_time_Intervals_s, *(*itrMeasureList));
        //NOTE: This process assumes no-compactness correction for ellipses - otherwise
        //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
        //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
        //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
        //      would increase dramtically compared to other Monte Carlo method. We would need an additional
        //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
        //      that would increase the amount of memory needed.
    }
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeIncludePurelySpatialCentricAnalysis::MonteCarloProspectiveAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    _time_Intervals_s->setIntervalRange(CentroidDef.GetCentroidIndex());
    //perform simulations about current centroid
    DataSetGatewayContainer_t::const_iterator itrGateway = vDataGateways.begin(), itrGatewayEnd = vDataGateways.end();
    MeasureListContainer_t::iterator itrMeasureList = gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
        _psp_cluster_data->AddMeasureList(CentroidDef, (*itrGateway)->GetDataSetInterface(), (*itrMeasureList));
        _cluster_data->AddNeighborDataAndCompare(CentroidDef, (*itrGateway)->GetDataSetInterface(), *_time_Intervals_s, *(*itrMeasureList));
        //NOTE: This process assumes no-compactness correction for ellipses - otherwise
        //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
        //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
        //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
        //      would increase dramtically compared to other Monte Carlo method. We would need an additional
        //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
        //      that would increase the amount of memory needed.
    }
}

/** constructor */
SpaceTimeIncludePureCentricAnalysis::SpaceTimeIncludePureCentricAnalysis(
    const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
    const AbstractDataSetGateway& RealDataGateway, const ptr_vector<AbstractDataSetGateway>& vSimDataGateways)
    :SpaceTimeIncludePurelySpatialCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {}
