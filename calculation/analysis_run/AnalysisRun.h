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

    void                                Execute();
    void                                ExecuteCentrically();
    void                                ExecuteSuccessively();
    void                                DisplayFindClusterHeading();
    void                                DisplayTopClusterLogLikelihood();
    void                                CalculateMostLikelyClusters();
    void                                CreateRelativeRiskFile();
    void                                CreateReport();
    void                                DisplayTopClusters();
    void                                DisplayTopCluster();
    void                                FinalizeReport();
    double                              GetAvailablePhysicalMemory() const;
    void                                Init();
    void                                OpenReportFile(FILE*& fp, bool bOpenAppend);
    void                                PerformParallelSimulations();
    void                                PerformSerializedSimulations();
    void                                PerformSimulations();
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

//holds exception info for jobs run simultaneously:
class ExceptionInfo
{
public:
  enum Type { etNone, etUnknown, etStd, etZd };
private:
  Type geType;
  void * gpException;
public:
  ExceptionInfo() : geType(etNone), gpException(0) {}
  template <typename ExceptionType> ExceptionInfo(ExceptionType const &);
  ExceptionInfo(ExceptionInfo const & to_be_copied);
  ~ExceptionInfo();
  ExceptionInfo& operator=(ExceptionInfo const & to_be_copied);
  void swap(ExceptionInfo & other);

  Type GetType() const { return geType; }
  template <typename ExceptionType>
  ExceptionType const & GetException() const;
  template <typename ExceptionType>
  void SetException(ExceptionType const & e);
  void SetUnknownException();
};

//***************************************************************************
#endif

