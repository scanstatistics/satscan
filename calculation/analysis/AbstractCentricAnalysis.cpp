//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "cluster.h"
#include "AbstractCentricAnalysis.h"
#include "MeasureList.h"
#include "SaTScanData.h"
#include "Toolkit.h"
#include "SSException.h"

/** constructor */
AbstractCentricAnalysis::AbstractCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection)
                        :AbstractAnalysis(Parameters, Data, PrintDirection) {
    gRetainedClusters.resize(Parameters.getExecuteSpatialWindowStops().size());
}

/** Calculates most likely cluster and evaluates simulated data about 'tCentroidIndex'.
    Cluster object added to TopClustersContainer if significant cluster found. Simulated
    loglikelihood ratios recorded to CalculatedRatioContainer_t class member. */
void AbstractCentricAnalysis::ExecuteAboutCentroid(
    tract_t tCentroidIndex, CentroidNeighborCalculator& CentroidCalculator, 
    const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways) {
    try {
        CentroidNeighbors CentroidDef;
        for (int iEllipseIndex=0; iEllipseIndex <= _parameters.GetNumTotalEllipses(); ++iEllipseIndex) {
            //calculate neigbor about current centroid
            macroRunTimeStartSerial(SerialRunTimeComponent::NeighborCalcuation);
            CentroidCalculator.CalculateNeighborsAboutCentroid(iEllipseIndex, tCentroidIndex, CentroidDef);
            macroRunTimeStopSerial();
	        // skip this centroid if it doesn't have neighbors
	        if (CentroidDef.GetNumNeighbors() == 0) continue;
            // find top cluster about current centroid
            macroRunTimeStartSerial(SerialRunTimeComponent::RealDataAnalysis);
            CalculateTopClusterAboutCentroidDefinition(CentroidDef, RealDataGateway);
            macroRunTimeStopSerial();
            //perform simulations about current centroid
            CentroidDef.SetMaximumClusterSize_SimulatedData();
            if (_parameters.GetNumReplicationsRequested()) {
                macroRunTimeStartSerial(SerialRunTimeComponent::ScanningSimulatedData);
                ExecuteSimulationsAboutCentroidDefinition(CentroidDef, vSimDataGateways);
                macroRunTimeStopSerial();
            }
        }
        SharedClusterVector_t clusters = GetTopCalculatedClusters();
        for (size_t t = 0; t < clusters.size(); ++t) {
            if (_parameters.getReportGiniOptimizedClusters()) {
                gRetainedClusters[t].push_back(clusters[t].get()->Clone());
                gRetainedClusters[t].back()->DeallocateEvaluationAssistClassMembers();
            } else {
                gRetainedClusters.front().push_back(clusters[t].get()->Clone());
                gRetainedClusters.front().back()->DeallocateEvaluationAssistClassMembers();
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ExecuteAboutCentroid()","AbstractCentricAnalysis");
        throw;
    }
}

/** Calculates most likely temporal cluster - no action taken in base class. */
void AbstractCentricAnalysis::ExecuteAboutPurelyTemporalCluster(const AbstractDataSetGateway&, const DataSetGatewayContainer_t&) {}

/** Executes simulation. Calls MonteCarlo() for analyses that can utilize
    CMeasureList class or perform simulations by the same algorithm as the real data. */
void AbstractCentricAnalysis::ExecuteSimulationsAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways) {
    if (_replica_process_type == MeasureListEvaluation)
        MonteCarloAboutCentroidDefinition(CentroidDef, vDataGateways);
    else
        CalculateRatiosAboutCentroidDefinition(CentroidDef, vDataGateways);
}

/** Retrieves calculated most likely clusters. After successful retrieval of clusters,
    ownership of cluster objects in gRetainedClusters class member will have been
    given to MostLikelyClustersContainer object and gRetainedClusters will be empty. */
void AbstractCentricAnalysis::RetrieveClusters(MLC_Collections_t& topClustersContainers) {
    std::auto_ptr<CCluster> Cluster;
    for (size_t s=0; s < gRetainedClusters.size(); ++s) {
        ptr_vector<CCluster>& clusters = gRetainedClusters[s];
        for (size_t t=0; t < clusters.size(); ++t) {
            Cluster.reset(clusters[t]); clusters[t] = 0;
            if (_parameters.getReportGiniOptimizedClusters())
                topClustersContainers[s].Add(Cluster);
            else
                topClustersContainers.front().Add(Cluster);
        }
    }
    gRetainedClusters.clear();
}

/** Retrieves calculated loglikehood ratio from class CalculatedRatioContainer_t object.
    If passed CalculatedRatioContainer_t already contains data, combines data with class
    object; else calculates ratios as needed and assigns to passed object.

    NOTE: All internal class members related to calculates ratios are freed in this function body, thus
          this should be the last method called to this object.

          Memory is freed particularly for the situation where simulations where done with CMeasureList
          objects. In this situation, these objects are likely taking up considerable memory. Allocating
          an additional vector for ratios, while retaining the CMeasureList objects puts an unnecessary
          burden on the system, since we will not need CMeasureList objects again after each has
          calculated the llr value.

          The remaining situations also free class members related to calculating ratio to be consistant. */
void AbstractCentricAnalysis::RetrieveLoglikelihoodRatios(CalculatedRatioContainer_t& RatioContainer) {
    if (RatioContainer.get()) {
        //if RatioContainer already contains data - combine with the results of this analysis object
        std::vector<double>::iterator   itrRatios=RatioContainer->begin()/*, itrRatios_end=RatioContainer->end()*/;
        if (_replica_process_type == MeasureListEvaluation) {
            //if simulations done using CMeasureList objects, calculate ratios now and take maximums
            MeasureListContainer_t::iterator itr=gvMeasureLists.begin(), itr_end=gvMeasureLists.end();
            for (; itr != itr_end; ++itr, ++itrRatios) {
                *itrRatios = std::max(*itrRatios, (*itr)->GetMaximumLogLikelihoodRatio());
                delete (*itr); (*itr)=0; //free CMeasureList object
            }
            gvMeasureLists.clear();
        } else {
            //else take maximums from RatioContainer and analysis objects calculated ratios
            std::vector<double>::iterator itr=gCalculatedRatios->begin(), itr_end=gCalculatedRatios->end();
            for (; itr != itr_end; ++itr, ++itrRatios)
                *itrRatios = std::max(*itrRatios, *itr);
            gCalculatedRatios.reset(); //free gCalculatedRatios object
        }
    } else {// RatioContainer is empty
        if (_replica_process_type == MeasureListEvaluation) {
            //if simulations done using CMeasureList objects, calculate ratios now and assign to RatioContainer
            RatioContainer.reset(new std::vector<double>());
            MeasureListContainer_t::iterator itr=gvMeasureLists.begin(), itr_end=gvMeasureLists.end();
            for (; itr != itr_end; ++itr) {
                RatioContainer->push_back((*itr)->GetMaximumLogLikelihoodRatio());
                delete (*itr); (*itr)=0; //free CMeasureList object
            }
            gvMeasureLists.clear();
        } else {
            //share ownership with analysis objects container of calculated ratios
            RatioContainer = gCalculatedRatios;
            gCalculatedRatios.reset(); //release shared ownership
        }
    }
}

