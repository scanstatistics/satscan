//***************************************************************************
#ifndef __AnalysisRun_H
#define __AnalysisRun_H
//***************************************************************************
#include "SaTScanData.h"
#include "cluster.h"
#include "SignificantRatios05.h"
#include "Analysis.h"
#include "MostLikelyClustersContainer.h"

/** Coordinates the execution of analysis defined by parameters. */
class AnalysisRunner {
  friend class stsMCSimReporter;
  
  private:
    void                                Init();
    void                                Setup();

  protected:
    const CParameters                 & gParameters;
    BasePrint                         & gPrintDirection;
    CSaTScanData                      * gpDataHub;
    CSignificantRatios05                gSimulatedRatios;
    unsigned short                      guwSignificantAt005;
    time_t                              gStartTime;
    double                              gdMinRatioToReport;
    unsigned int                        giAnalysisCount;
    int                                 giPower_X_Count;
    int                                 giPower_Y_Count;
    unsigned int                        giNumSimsExecuted;
    unsigned int                        giClustersReported;
    MostLikelyClustersContainer         gTopClustersContainer;

    void                                DisplayFindClusterHeading();
    void                                DisplayTopClusterLogLikelihood();
    virtual void                        CalculateMostLikelyClusters();
    virtual void                        CreateClusterInformationFile();
    virtual void                        CreateRelativeRiskFile();
    void                                CreateReport();
    virtual void                        DisplayTopClusters();
    virtual void                        DisplayTopCluster();
    void                                FinalizeReport();
    void                                OpenReportFile(FILE*& fp, bool bOpenAppend);
    virtual void                        PerformParallelSimulations();
    virtual void                        PerformSerializedSimulations();
    virtual void                        PerformSimulations();
    void                                RemoveTopClusterData();
    bool                                RepeatAnalysis();
    void                                UpdatePowerCounts(double r);
    void                                UpdateReport();

  public:
    AnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection);
    virtual ~AnalysisRunner();

    virtual bool                        CheckForEarlyTermination(unsigned int iNumSimulationsCompleted) const;
    virtual void                        Execute();
    const MostLikelyClustersContainer & GetClusterContainer() const {return gTopClustersContainer;}
    const CSaTScanData                & GetDataHub() const {return *gpDataHub;}
    CAnalysis                         * GetNewAnalysisObject() const;
    unsigned short                      GetNumSignificantAt005() const {return guwSignificantAt005;}
    unsigned int                        GetNumSimulationsExecuted() const {return giNumSimsExecuted;}
    const double                        GetSimRatio01() const {return gSimulatedRatios.GetAlpha01();}
    const double                        GetSimRatio05() const {return gSimulatedRatios.GetAlpha05();}
    const time_t                      * GetStartTime() const {return &gStartTime;}
};
//***************************************************************************
#endif
