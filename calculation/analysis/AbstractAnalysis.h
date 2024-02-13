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

/** Abstract base class which defines methods for calculating top clusters and simulated log likelihood ratios. */
class AbstractAnalysis {
    protected:
        enum ReplicationsProcessType {MeasureListEvaluation=0, ClusterEvaluation};

        const CParameters & _parameters;
        const CSaTScanData & _data_hub;
        BasePrint & _print;
        AbstractClusterDataFactory * _cluster_data_factory;
        AbstractLikelihoodCalculator * _likelihood_calculator;
        ReplicationsProcessType _replica_process_type;

        CMeasureList                        * GetNewMeasureListObject() const;
        CTimeIntervals                      * GetNewTemporalDataEvaluatorObject(IncludeClustersType eType, ExecutionType eExecutionType) const;

    public:
        AbstractAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection);
        virtual ~AbstractAnalysis();

        static AbstractLikelihoodCalculator * GetNewLikelihoodCalculator(const CSaTScanData& DataHub);
};    
//******************************************************************************
#endif
