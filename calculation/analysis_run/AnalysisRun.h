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
        void sort() {
            if (_addStatus != closed) { // already closed? - them it is already sorted
            // sort cluster collection by cluster LLR -- adding clusters after this call is undefined behavior.
            std::sort(_rankCollection.begin(), _rankCollection.end(), MostLikelyClustersContainer::CompareClustersRatios());
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

/** Coordinates the execution of analysis defined by parameters. */
class AnalysisRunner {
  friend class stsMCSimJobSource;
  friend class OliveiraJobSource;

  protected:
    const CParameters                 & gParameters;
    BasePrint                         & gPrintDirection;
    std::auto_ptr<CSaTScanData>         gpDataHub;
    std::auto_ptr<SignificantRatios>    gpSignificantRatios;
    unsigned short                      guwSignificantAt005;
    time_t                              gStartTime;
    double                              gdMinRatioToReport;
    unsigned int                        giAnalysisCount;
    int                                 giPower_X_Count;
    int                                 giPower_Y_Count;
    bool                                _clustersReported;
    MLC_Collections_t                   gTopClustersContainers;
    MostLikelyClustersContainer         _reportClusters;
    ClusterRankHelper                   _clusterRanker;
    ExecutionType                       geExecutingType;
    SimulationVariables                 gSimVars;
    std::auto_ptr<LocationRelevance>    _relevance_tracker;

    typedef std::pair<const MostLikelyClustersContainer*,unsigned int> OptimalGiniByLimit_t;

    void                                addGiniClusters(MLC_Collections_t& mlc_collections, MostLikelyClustersContainer& mlc, double p_value_cutoff);
    void                                ExecuteCentrically();
    void                                ExecuteSuccessively();
    void                                ExecutePowerEvaluations();
    void                                CalculateMostLikelyClusters();
    void                                calculateOliveirasF();
    void                                calculateOverlappingClusters(const MostLikelyClustersContainer& mlc, ClusterSupplementInfo& clusterSupplement);
    void                                CreateReport();
    void                                FinalizeReport();
    double                              GetAvailablePhysicalMemory() const;
    std::pair<double, double>           GetMemoryApproxiation() const;
    const MostLikelyClustersContainer * getOptimalGiniContainerByPValue(const MLC_Collections_t& mlc_collections, double p_value_cutoff) const;
    OptimalGiniByLimit_t                getOptimalGiniContainerByLimit(const MLC_Collections_t& mlc_collections, const std::vector<unsigned int>& atmost) const;
    void                                LogRunHistory();
    void                                OpenReportFile(FILE*& fp, bool bOpenAppend);
    void                                ExecuteCentricEvaluation();
    void                                runSuccessiveSimulations(boost::shared_ptr<RandomizerContainer_t>& randomizers, unsigned int num_relica, const std::string& writefile, bool isPowerStep);
    void                                ExecuteSuccessiveSimulations();
    void                                PrintCriticalValuesStatus(FILE* fp);
    void                                PrintEarlyTerminationStatus(FILE* fp);
    void                                PrintFindClusterHeading();
    void                                PrintGiniCoefficients(FILE* fp);
    void                                PrintRetainedClustersStatus(FILE* fp, bool bClusterReported);
    void                                PrintTopClusters(const MostLikelyClustersContainer& mlc);
    void                                PrintTopClusterLogLikelihood(const MostLikelyClustersContainer& mlc);
    void                                PrintTopIterativeScanCluster(const MostLikelyClustersContainer& mlc);
    virtual bool                        RepeatAnalysis();
    void                                reportClusters();
    void                                Setup();
    void                                UpdateSignificantRatiosList(double dRatio);

  public:
    AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection);
    virtual ~AnalysisRunner() {}

    virtual bool                        CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const;

    void                                Execute();
    //const MLC_Collections_t           & GetClusterContainer() const {return gTopClustersContainers;}
    const MostLikelyClustersContainer & getLargestMaximaClusterCollection() const;
    const CSaTScanData                & GetDataHub() const {return *gpDataHub;}
    bool                                GetIsCalculatingSignificantRatios() const {return gpSignificantRatios.get() != 0;}
    CAnalysis                         * GetNewAnalysisObject() const;
    AbstractCentricAnalysis           * GetNewCentricAnalysisObject(const AbstractDataSetGateway& RealDataGateway,
                                                                    const ptr_vector<AbstractDataSetGateway>& vSimDataGateways) const;
    unsigned short                      GetNumSignificantAt005() const {return guwSignificantAt005;}
    unsigned int                        GetNumSimulationsExecuted() const {return gSimVars.get_sim_count();}
    double                              GetSimRatio01() const;
    double                              GetSimRatio05() const;
    const SimulationVariables         & GetSimVariables() const {return gSimVars;}
    const time_t                      * GetStartTime() const {return &gStartTime;}
    void                                rankClusterCollections(MLC_Collections_t& mlc_collection, MostLikelyClustersContainer& mlc, ClusterRankHelper * ranker, BasePrint& print) const;
};


//***************************************************************************
#endif

