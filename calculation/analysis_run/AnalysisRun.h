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

/** Coordinates the execution of analysis defined by parameters. */
class AnalysisRunner {
  friend class stsMCSimJobSource;

  private:
    const CParameters                 & gParameters;
    BasePrint                         & gPrintDirection;
    CSaTScanData                      * gpDataHub;
    CSignificantRatios05              * gpSignificantRatios;
    unsigned short                      guwSignificantAt005;
    time_t                              gStartTime;
    double                              gdMinRatioToReport;
    unsigned int                        giAnalysisCount;
    int                                 giPower_X_Count;
    int                                 giPower_Y_Count;
    unsigned int                        giNumSimsExecuted;
    unsigned int                        giClustersReported;
    MostLikelyClustersContainer         gTopClustersContainer;
    ExecutionType                       geExecutingType;

    void                                Execute();
    void                                ExecuteCentrically();
    void                                ExecuteSuccessively();
    void                                CalculateMostLikelyClusters();
    void                                CreateRelativeRiskFile();
    void                                CreateReport();
    void                                FinalizeReport();
    double                              GetAvailablePhysicalMemory() const;
    std::pair<double, double>           GetMemoryApproxiation() const;
    void                                Init();
    void                                OpenReportFile(FILE*& fp, bool bOpenAppend);
    void                                PerformCentric_Parallel();
    void                                PerformCentric_Serial();
    void                                PerformSuccessiveSimulations_Parallel();
    void                                PerformSuccessiveSimulations_Serial();
    void                                PerformSuccessiveSimulations();
    void                                PrintCriticalValuesStatus(FILE* fp);
    void                                PrintEarlyTerminationStatus(FILE* fp);
    void                                PrintFindClusterHeading();
    void                                PrintPowerCalculationsStatus(FILE* fp);
    void                                PrintRetainedClustersStatus(FILE* fp, bool bClusterReported);
    void                                PrintTopClusters();
    void                                PrintTopClusterLogLikelihood();
    void                                PrintTopIterativeScanCluster();
    void                                RemoveTopClusterData();
    bool                                RepeatAnalysis();
    void                                Setup();
    void                                UpdatePowerCounts(double r);
    void                                UpdateReport();
    void                                UpdateSignificantRatiosList(double dRatio);

  public:
    AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection);
    virtual ~AnalysisRunner();

    virtual bool                        CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const;

    const MostLikelyClustersContainer & GetClusterContainer() const {return gTopClustersContainer;}
    const CSaTScanData                & GetDataHub() const {return *gpDataHub;}
    bool                                GetIsCalculatingSignificantRatios() const {return gpSignificantRatios;}
    CAnalysis                         * GetNewAnalysisObject() const;
    AbstractCentricAnalysis           * GetNewCentricAnalysisObject(const AbstractDataSetGateway& RealDataGateway,
                                                                    const ZdPointerVector<AbstractDataSetGateway>& vSimDataGateways) const;
    unsigned short                      GetNumSignificantAt005() const {return guwSignificantAt005;}
    unsigned int                        GetNumSimulationsExecuted() const {return giNumSimsExecuted;}
    double                              GetSimRatio01() const;
    double                              GetSimRatio05() const;
    const time_t                      * GetStartTime() const {return &gStartTime;}
};


//***************************************************************************
#endif

