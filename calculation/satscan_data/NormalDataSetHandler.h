//******************************************************************************
#ifndef __NormalDataSetHandler_H
#define __NormalDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "ContinuousVariableRandomizer.h"

class NormalDataSetHandler : public DataSetHandler {
  protected:
    virtual void                        AllocateCaseStructures(size_t tSetIndex);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    NormalDataSetHandler(CSaTScanData& DataHub, BasePrint& pPrint);
    virtual ~NormalDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbstractDataSetGateway     & GetDataGateway(AbstractDataSetGateway& DataGatway) const;
    virtual AbstractDataSetGateway     & GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet & DataSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

