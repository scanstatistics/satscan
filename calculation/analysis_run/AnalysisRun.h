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

/** Coordinates the execution of analysis defined by parameters. */
class AnalysisRunner {
  friend class stsMCSimJobSource;

  private:
    const CParameters                 & gParameters;
    BasePrint                         & gPrintDirection;
    std::auto_ptr<CSaTScanData>         gpDataHub;
    std::auto_ptr<CSignificantRatios05> gpSignificantRatios;
    unsigned short                      guwSignificantAt005;
    time_t                              gStartTime;
    double                              gdMinRatioToReport;
    unsigned int                        giAnalysisCount;
    int                                 giPower_X_Count;
    int                                 giPower_Y_Count;
    bool                                _clustersReported;
    MLC_Collections_t                   gTopClustersContainers;
    ExecutionType                       geExecutingType;
	SimulationVariables                 gSimVars;

    void                                Execute();
    void                                ExecuteCentrically();
    void                                ExecuteSuccessively();
    void                                CalculateMostLikelyClusters();
    void                                CreateRelativeRiskFile();
    void                                CreateReport();
    void                                FinalizeReport();
    double                              GetAvailablePhysicalMemory() const;
    std::pair<double, double>           GetMemoryApproxiation() const;
    void                                LogRunHistory();
    void                                OpenReportFile(FILE*& fp, bool bOpenAppend);
    void                                PerformCentric_Parallel();
    void                                PerformCentric_Serial();
    void                                PerformSuccessiveSimulations_Parallel();
    void                                PerformSuccessiveSimulations_Serial();
    void                                PerformSuccessiveSimulations();
    void                                PrintCriticalValuesStatus(FILE* fp);
    void                                PrintEarlyTerminationStatus(FILE* fp);
    void                                PrintFindClusterHeading();
    void                                PrintGiniCoefficients(FILE* fp);
    void                                PrintPowerCalculationsStatus(FILE* fp);
    void                                PrintRetainedClustersStatus(FILE* fp, bool bClusterReported);
    void                                PrintTopClusters();
    void                                PrintTopClusterLogLikelihood();
    void                                PrintTopIterativeScanCluster();
    bool                                RepeatAnalysis();
    void                                Setup();
    void                                UpdatePowerCounts(double r);
    void                                UpdateReport();
    void                                UpdateSignificantRatiosList(double dRatio);

  public:
    AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection);

    virtual bool                        CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const;

    const MLC_Collections_t           & GetClusterContainer() const {return gTopClustersContainers;}
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
};


//***************************************************************************
#endif

