//***************************************************************************
#ifndef __AnalysisRun_H
#define __AnalysisRun_H
//***************************************************************************
#include "SaTScanData.h"
#include "cluster.h"
#include "SignificantRatios05.h"
#include "Analysis.h"
#include "MostLikelyClustersContainer.h"
#include "AbstractCentricAnalysis.h"
#include "ptr_vector.h"
#include "SimulationVariables.h"
#include "ClusterSupplement.h"
#include "ClusterLocationsWriter.h"
#include "LocationRelevance.h"
#include <boost/optional.hpp>
#include "ClusterKML.h"
#include "ClusterScatterChart.h"
#include "ClusterMap.h"
#include "DataDemographics.h"

class ClusterRankHelper {
    private:
        typedef MostLikelyClustersContainer::ClusterList_t RankList_t;
        enum AddStatus {accepting=0,closed};

        RankList_t _rankCollection;
        AddStatus  _addStatus;

    public:
        ClusterRankHelper() : _addStatus(accepting) {}

        void add(MostLikelyClustersContainer& mlc) {
            if (_addStatus != accepting)
                throw prg_error("Cluster collection is closed to additions.", "ClusterRankHelper");
            // add clusters from passed cluster collection -- maintaining sor order and cluster uniqueness
            for (tract_t t=0; t < mlc.GetNumClustersRetained(); ++t) {
                MostLikelyClustersContainer::Cluster_t& cluster = mlc.GetClusterRef(t);
                RankList_t::iterator itr=std::lower_bound(_rankCollection.begin(), _rankCollection.end(), cluster);
                if (itr == _rankCollection.end() || itr->get() != cluster.get())
                    _rankCollection.insert(itr, cluster);
            }
        }
        void clear() {
            _rankCollection.clear();
            _addStatus = accepting;
        }
        void sort(const CSaTScanData& data_hub) {
            if (_addStatus != closed) { // already closed? - them it is already sorted
            // sort cluster collection by cluster LLR -- adding clusters after this call is undefined behavior.
            std::sort(_rankCollection.begin(), _rankCollection.end(), MostLikelyClustersContainer::CompareClustersRatios(data_hub));
            _addStatus = closed;
            }
        }
        void update(double llr) {
            if (_addStatus == accepting)
                throw prg_error("Cluster collection is not sorted.", "ClusterRankHelper");
            // update rank of each cluster in collection with LLR >= passed LLR.
            RankList_t::reverse_iterator rev(_rankCollection.end()), rev_end(_rankCollection.begin());
            for (; rev != rev_end; rev++) {
                if (macro_less_than(llr, (*rev)->GetRatio(), DBL_CMP_TOLERANCE)) return;
                (*rev)->IncrementRank();
            }
        }
};

class AnalysisExecution {
    friend class stsMCSimJobSource;
    friend class OliveiraJobSource;

    protected:
        typedef std::pair<const MostLikelyClustersContainer*, unsigned int> OptimalGiniByLimit_t;

        const CParameters                 & _parameters;
        BasePrint                         & _print_direction;
        CSaTScanData                      & _data_hub;
        time_t                              _start_time;
        bool                                _clustersReported;
        ExecutionType                       _executing_type;
        unsigned int                        _analysis_count;
        unsigned short                      _significant_at005;
        unsigned int                        _significant_clusters;
        MLC_Collections_t                   _top_clusters_containers;
        MostLikelyClustersContainer         _reportClusters;
        ClusterRankHelper                   _clusterRanker;
        SimulationVariables                 _sim_vars;
        std::auto_ptr<LocationRelevance>    _relevance_tracker;
        std::auto_ptr<ClusterKML>           _cluster_kml;
        std::auto_ptr<CartesianGraph>       _cluster_graph;
        std::auto_ptr<ClusterMap>           _cluster_map;
        std::auto_ptr<SignificantRatios>    _significant_ratios;
        std::auto_ptr<ClusterSupplementInfo> _clusterSupplement;
        boost::shared_ptr<DataDemographicsProcessor> _data_demographic_processor;

        std::string                         _base_output;
        std::string                         _cluster_path;
        unsigned int                      & _drilldowns;

        void                                addGiniClusters(MLC_Collections_t& mlc_collections, MostLikelyClustersContainer& mlc, double p_value_cutoff);
        void                                calculateMostLikelyClusters();
        void                                calculateOverlappingClusters(const MostLikelyClustersContainer& mlc, ClusterSupplementInfo& clusterSupplement);
        void                                calculateOliveirasF();
        void                                createReport();
        void                                executeCentricEvaluation();
        void                                executePowerEvaluations();
        void                                executeSuccessively();
        void                                executeSuccessiveSimulations();
        CAnalysis                         * getNewAnalysisObject(BasePrint& print) const;
        AbstractCentricAnalysis           * getNewCentricAnalysisObject(const AbstractDataSetGateway& RealDataGateway, const ptr_vector<AbstractDataSetGateway>& vSimDataGateways) const;
        const MostLikelyClustersContainer * getOptimalGiniContainerByPValue(const MLC_Collections_t& mlc_collections, double p_value_cutoff) const;
        OptimalGiniByLimit_t                getOptimalGiniContainerByLimit(const MLC_Collections_t& mlc_collections, const std::vector<unsigned int>& atmost) const;
        void                                logRunHistory();
        void                                openReportFile(FILE*& fp, bool bOpenAppend);
        void                                printFindClusterHeading();
        void                                printCriticalValuesStatus(FILE* fp);
        void                                printEarlyTerminationStatus(FILE* fp);
        void                                printGiniCoefficients(FILE* fp);
        void                                printRetainedClustersStatus(FILE* fp, bool bClusterReported);
        void                                printIgnoredDataSets(FILE* fp);
        void                                printTopClusterLogLikelihood(const MostLikelyClustersContainer& mlc);
        void                                printTopClusters(const MostLikelyClustersContainer& mlc);
        void                                printTopIterativeScanCluster(const MostLikelyClustersContainer& mlc);
        virtual bool                        repeatAnalysis();
        void                                reportClusters();
        void                                runSuccessiveSimulations(boost::shared_ptr<RandomizerContainer_t>& randomizers, unsigned int num_relica, const std::string& writefile, bool isPowerStep, unsigned int iteration);
        void                                updateSignificantRatiosList(double dRatio);

    public:
        AnalysisExecution(CSaTScanData& data_hub, ExecutionType executing_type, time_t start, unsigned int& drilldowns);
        virtual ~AnalysisExecution() {}

        void                                execute();
        void                                finalize();
        const ClusterSupplementInfo       & getClusterSupplement() const { return *_clusterSupplement; }
        const CSaTScanData                & getDataHub() const { return _data_hub; }
        const std::string                 & getDrilldownBaseOutput() const { return _base_output; }
        const std::string                 & getDrilldownClusterPath() const { return _cluster_path; }
        ExecutionType                       getExecutioningType() const { return _executing_type; }
        bool                                getIsCalculatingSignificantRatios() const { return _significant_ratios.get() != 0; }
        const MostLikelyClustersContainer & getLargestMaximaClusterCollection() const;
        unsigned short                      getNumSignificantAt005() const { return _significant_at005; }
        unsigned int                        getNumSignificantClusters() const { return _significant_clusters; }
        unsigned int                        getNumSimulationsExecuted() const { return _sim_vars.get_sim_count(); }
        const CParameters                 & getParameters() const { return _parameters; }
        double                              getSimRatio01() const { return _significant_ratios.get() ? _significant_ratios->getAlpha01().second : 0.0; }
        double                              getSimRatio05() const { return _significant_ratios.get() ? _significant_ratios->getAlpha05().second : 0; }
        const SimulationVariables         & getSimVariables() const { return _sim_vars; }
        const time_t                      * getStartTime() const { return &_start_time; }
        void                                rankClusterCollections(MLC_Collections_t& mlc_collection, MostLikelyClustersContainer& mlc, ClusterRankHelper * ranker, BasePrint& print) const;
        void                                setDrilldownAttributes(const std::string& base_output, const std::string& cluster_path);
};

class AbstractAnalysisDrilldown {
    protected:
        CParameters                         _parameters;
        BasePrint                         & _print_direction;
        time_t                              _start_time;
        ExecutionType                       _executing_type;
        std::auto_ptr<CSaTScanData>         _data_hub;
        unsigned int                        _downlevel;
        std::vector<std::string>            _temp_files;
        const std::string                 & _base_output;
        std::string                         _cluster_path;
        unsigned int                        _significant_clusters;
        unsigned int                      & _drilldowns;

    public:
        AbstractAnalysisDrilldown(
            const CParameters& source_parameters, const std::string& base_output, ExecutionType executing_type,
            BasePrint& print, unsigned int downlevel, unsigned int parent_runid,
            unsigned int& drilldowns,
            boost::optional<const std::string&> cluster_path
        );
        virtual ~AbstractAnalysisDrilldown();

        std::string                       & createTempFilename(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const char * extension, std::string& filename);
        virtual void                        createReducedCoodinatesFile(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub, unsigned int downlevel);
        virtual void                        createReducedGridFile(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub, unsigned int downlevel);
        static  void                        drilldownCluster(const AnalysisExecution& execution, const CCluster& cluster, unsigned int clusterRptIdx, unsigned int& drilldowns);
        virtual void                        execute(const std::string& parentOutputFilename);
        unsigned int                        getNumSignificantClusters() const { return _significant_clusters; }
        virtual const std::string         & getTypeIdentifier() const = 0;
        const CParameters                 & getParameters() const { return _parameters; }
        virtual void                        setOutputFilename(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, unsigned int& drilldowns);
        static bool                         shouldDrilldown(const CCluster& cluster, unsigned int clusterRptIdx, const CSaTScanData& data, const CParameters& parameters, const SimulationVariables& simvars);

        static const double                 DEFAULT_CUTOFF_PVALUE;
};

class AnalysisDrilldown : public AbstractAnalysisDrilldown {

    public:
        AnalysisDrilldown(
            const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub,
            const std::string& base_output, ExecutionType executing_type, unsigned int downlevel, unsigned int& drilldowns,
            boost::optional<const std::string&> cluster_path = boost::none
        );
        virtual ~AnalysisDrilldown() {}

        virtual const std::string& getTypeIdentifier() const { return TYPE_IDENTIFIER; };
        static std::string TYPE_IDENTIFIER;
};

class BernoulliAnalysisDrilldown : public AbstractAnalysisDrilldown {

public:
    BernoulliAnalysisDrilldown(
        const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub,
        const std::string& base_output, ExecutionType executing_type, unsigned int downlevel, unsigned int& drilldowns,
        boost::optional<const std::string&> cluster_path = boost::none
    );
    virtual ~BernoulliAnalysisDrilldown() {}

    virtual const std::string & getTypeIdentifier() const { return TYPE_IDENTIFIER; };
    static std::string          TYPE_IDENTIFIER;
    static const int            DEFAULT_NUM_ITERATIVE_SCANS;
    static const double         DEFAULT_ITERATIVE_CUTOFF_PVALUE;
};

/** Coordinates the execution of analysis defined by parameters. */
class AnalysisRunner {
  protected:
    const CParameters                 & _parameters;
    BasePrint                         & _print_direction;
    std::auto_ptr<CSaTScanData>         _data_hub;
    time_t                              _start_time;
    ExecutionType                       _executing_type;
    mutable unsigned int                _drilldowns; // number of drilldowns, acting as a unique run id

    virtual AnalysisExecution         * getAnalysisExecution() const { 
        return new AnalysisExecution(*_data_hub, _executing_type, _start_time, _drilldowns);
    }
    static double                       getAvailablePhysicalMemory();
    static std::pair<double, double>    getMemoryApproxiation(const CParameters& parameters, const CSaTScanData& data_hub);

  public:
    AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection);
    virtual ~AnalysisRunner() {}

    static CSaTScanData               * getNewCSaTScanData(const CParameters& parameters, BasePrint& print);
    void                                run();
};


//***************************************************************************
#endif

