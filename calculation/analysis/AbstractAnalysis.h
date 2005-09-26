//******************************************************************************
#ifndef __AbstractAnalysis_H
#define __AbstractAnalysis_H
//******************************************************************************
#include "Parameters.h"

/* forward class declarations */
class AbstractLikelihoodCalculator;
class AbstractClusterDataFactory;
class BasePrint;
class CMeasureList;
class CSaTScanData;
class CTimeIntervals;

/** Abstract base class which defines methods for calculating top clusters and
    simulated log likelihood ratios. */
class AbstractAnalysis {
  private:
    void                                Setup();

  protected:
    enum ReplicationsProcessType          {MeasureListEvaluation=0, ClusterEvaluation};

    const CParameters                   & gParameters;
    const CSaTScanData                  & gDataHub;
    BasePrint                           & gPrintDirection;
    AbstractClusterDataFactory          * gpClusterDataFactory;
    AbstractLikelihoodCalculator        * gpLikelihoodCalculator;
    ReplicationsProcessType               geReplicationsProcessType;

    CMeasureList                        * GetNewMeasureListObject() const;
    CTimeIntervals                      * GetNewTemporalDataEvaluatorObject(IncludeClustersType eType) const;

  public:
    AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~AbstractAnalysis();

    static AbstractLikelihoodCalculator * GetNewLikelihoodCalculator(const CSaTScanData& DataHub);
};    
//******************************************************************************
#endif
