//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelyTemporalCluster.h"
#include "MostLikelyClustersContainer.h"
#include "SSException.h"

/** constructor */
SpaceTimeCentricAnalysis::SpaceTimeCentricAnalysis(
    const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
    const AbstractDataSetGateway& RealDataGateway,    const DataSetGatewayContainer_t& vSimDataGateways)
:AbstractCentricAnalysis(Parameters, Data, PrintDirection), _top_clusters(Data) {
    try {
        // Allocate objects used to evaluate real data.
        _cluster_compare.reset(new CSpaceTimeCluster(_cluster_data_factory.get(), RealDataGateway));
        _top_clusters.setTopClusters(*_cluster_compare.get());
        // Allocate time interval evaluation of real data.
        _time_intervals_r.reset(
            GetNewTemporalDataEvaluatorObject(_parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : _parameters.GetIncludeClustersType(), CENTRICALLY)
        );
        // Allocate objects used to evaluate simulation data.
        if (_parameters.GetNumReplicationsRequested()) {
            gvMeasureLists.killAll();
            //create simulation objects based upon which process used to perform simulations
            if (_replica_process_type == MeasureListEvaluation) {
                //create a measure list object for each requested replication - we do this inorder
                //to prevent excess calls to loglikelihood calculation method; unfortunately this
                //also consumes more memory...
                _cluster_data.reset(new SpaceTimeData(*(*vSimDataGateways.begin())));
                for (size_t t=0; t < vSimDataGateways.size(); ++t)
                    gvMeasureLists.push_back(GetNewMeasureListObject());
            } else {
                //Simulations performed using same process as real data set. Since we will be taking
                //the greatest loglikelihood ratios among all centroids, but analyzing each centroids
                //replications separately, we need to maintain a vector of llr values.
                gCalculatedRatios.reset(new std::vector<double>(_parameters.GetNumReplicationsRequested(), 0));
                _cluster_data_pt.reset(_cluster_data_factory->GetNewSpaceTimeClusterData(*(*vSimDataGateways.begin())));
            }
            // Allocate time interval evaluator for simulation process.
            _time_Intervals_s.reset(
                GetNewTemporalDataEvaluatorObject(_parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALLCLUSTERS : _parameters.GetIncludeClustersType(), CENTRICALLY)
            );
        }
    } catch (prg_exception& x) {
        gvMeasureLists.killAll();
        x.addTrace("constructor()","SpaceTimeCentricAnalysis");
        throw;
    }
}

/** Calculates greatest loglikelihood ratios about centroid for each defined data set gateway object,
    storing maximum ratio for each simulation in class member gCalculatedRatios.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeCentricAnalysis::CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    _time_Intervals_s->setIntervalRange(CentroidDef.GetCentroidIndex());
    //perform simulations about current centroid
    DataSetGatewayContainer_t::const_iterator itrGateway = vDataGateways.begin(), itrGatewayEnd = vDataGateways.end();
    std::vector<double>::iterator itrLoglikelihoodRatios = gCalculatedRatios->begin();
    tract_t t, tNumNeighbors;
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
        _cluster_data_pt->InitializeData();
        tNumNeighbors = CentroidDef.GetNumNeighbors();
        for (t=0; t < tNumNeighbors; ++t) {
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

/** Calculates clustering with greastest loglikelihood ratio, storing results in gTopCluster
    class member. Note that gTopCluster is re-intialized when centroid being evaluated is
    different than that of gTopCluster; this permits repeated calls to this function, inorder
    to compare clusterings about same centroid but with different ellipse shapes. */
void SpaceTimeCentricAnalysis::CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway) {
    if (_top_clusters.getClusterSet(0).getSet().front().getCluster().GetCentroidIndex() != CentroidDef.GetCentroidIndex())
        //re-intialize top cluster objects if evaluating data about new centroid
        _top_clusters.resetAboutCentroid(CentroidDef.GetCentroidIndex());
    _time_intervals_r->setIntervalRange(CentroidDef.GetCentroidIndex());
    _cluster_compare->Initialize(CentroidDef.GetCentroidIndex());
    _top_clusters.resetNeighborCounts(CentroidDef);
    _cluster_compare->SetEllipseOffset(CentroidDef.GetEllipseIndex(), _data_hub);
    _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_time_intervals_r);
    //if top cluster was found in this centroid/ellipse, calculate radius now - CentroidNeighbors object wont' be available later
    _top_clusters.setClusterNonPersistantNeighborInfo(CentroidDef);
}

/** Calculates top cluster and simulated llr values about purely temporal clusterings.

    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeCentricAnalysis::ExecuteAboutPurelyTemporalCluster(const AbstractDataSetGateway& DataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
    try {
        //calculate top purely temporal cluster
        IncludeClustersType includeType = _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : _parameters.GetIncludeClustersType();
        //create top cluster
        CPurelyTemporalCluster TopCluster(_cluster_data_factory.get(), DataGateway, includeType, _data_hub);
        //create comparator cluster
        CPurelyTemporalCluster ClusterComparator(_cluster_data_factory.get(), DataGateway, includeType, _data_hub);
        boost::shared_ptr<CClusterSet> clusterSet(CClusterSet::getNewCClusterSetObject(ClusterComparator, _data_hub));
        CClusterObject clusterObject(ClusterComparator);
        clusterSet->add(clusterObject);
        _time_intervals_r->resetIntervalRange();
        _time_intervals_r->CompareClusterSet(ClusterComparator, *clusterSet);
        // If there is a most likely cluster, possibly search for other clusters then add to MLC container.
        if (clusterSet->getSet().front().getCluster().ClusterDefined()) {
            CClusterSet::ClusterContainer_t clusters;
            clusters.push_back(clusterSet->getSet().front()); // Add MLC to the collection of purely temporal clusters.
            // Search for other purely temporal clusters that don't overlap temporally with the MLC, or each other.
            if (_parameters.GetAnalysisType() == SPACETIME && !_parameters.GetIsIterativeScanning()) // retrospective space-time analyses only
                CClusterSetTemporalOverlap::findOtherNotTemporalOverlapping(clusters, *clusterSet, _data_hub);
            for (auto& clusterObj : clusters)
                gRetainedClusters.front().push_back(clusterObj.getCluster().Clone());
        }
        //calculate simulated ratios
        if (_parameters.GetNumReplicationsRequested()) {
            ptr_vector<AbstractDataSetGateway>::const_iterator itrGateway=vSimDataGateways.begin(), itrGatewayEnd=vSimDataGateways.end();
            std::auto_ptr<AbstractTemporalClusterData> PTClusterData;
            PTClusterData.reset(_cluster_data_factory->GetNewTemporalClusterData(*(*vSimDataGateways.begin())));
            _time_Intervals_s->resetIntervalRange();
            if (_replica_process_type == MeasureListEvaluation) {
                MeasureListContainer_t::iterator  itrMeasureList=gvMeasureLists.begin();
                //perform simulation about purely temporal data
                for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList) {
                    PTClusterData->Reassociate(*(*itrGateway));
                    macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
                    _time_Intervals_s->CompareMeasures(*PTClusterData, *(*itrMeasureList));
                    macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
                }
            } else {
                std::vector<double>::iterator  itrLoglikelihoodRatios=gCalculatedRatios->begin();
                //perform simulation about purely temporal data
                for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrLoglikelihoodRatios) {
                    PTClusterData->Reassociate(*(*itrGateway));
                    *itrLoglikelihoodRatios = std::max(*itrLoglikelihoodRatios, _time_Intervals_s->ComputeMaximizingValue(*PTClusterData));
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ExecuteAboutPurelyTemporalCluster()","SpaceTimeCentricAnalysis");
        throw;
    }
}

/** Returns cluster object with greatest llr value as specified by gTopCluster.
    Note that this object will potentially be updated with next call to
    CalculateTopClusterAboutCentroidDefinition(), so calling Clone() on returned object
    is necessary if you want to keep object around in current state. */
const SharedClusterVector_t SpaceTimeCentricAnalysis::GetTopCalculatedClusters() {
    SharedClusterVector_t topClusters; 
    return _top_clusters.getTopClusters(topClusters);
}

/** Updates CMeasureList objects for each defined data set of gateway object.
    Caller is responsible for ensuring:
    1) 'vDataGateways' contains a number of elements equal to number of requested simulations
    2) DataSetInterface objects are assigned to appropriate structures used to accumulate cluster data
    3) RetrieveLoglikelihoodRatios() has not been called yet */
void SpaceTimeCentricAnalysis::MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    _time_Intervals_s->setIntervalRange(CentroidDef.GetCentroidIndex());
    //perform simulations about current centroid
    DataSetGatewayContainer_t::const_iterator itrGateway = vDataGateways.begin(), itrGatewayEnd = vDataGateways.end();
    MeasureListContainer_t::iterator itrMeasureList = gvMeasureLists.begin();
    for (; itrGateway != itrGatewayEnd; ++itrGateway, ++itrMeasureList)
        _cluster_data->AddNeighborDataAndCompare(CentroidDef, (*itrGateway)->GetDataSetInterface(), *_time_Intervals_s, *(*itrMeasureList));
    //NOTE: This process assumes no-compactness correction for ellipses - otherwise
    //      we would call: gpMeasureList->SetForNextIteration(itrCentroidEnd->GetCentroidIndex());
    //      to calculate the loglikelihood with accumulated measure list for current circle/ellipse.
     //      Note that if we did in fact call that method, the number of times the loglikelihood is calculated
     //      would increase dramtically compared to other Monte Carlo method. We would need an additional
     //      measurelist object for each ellipse/centroid pair to prevent to many loglikelihood calculations, but
     //      that would increase the amount of memory needed. (also PT cluster?)
}

/** constructor */
SpaceTimeIncludePurelyTemporalCentricAnalysis::SpaceTimeIncludePurelyTemporalCentricAnalysis(
    const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
    const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways)
    :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {}