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

    virtual AbtractDataSetGateway  * GetNewDataGateway() const;
    virtual AbtractDataSetGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalMeasureData(RealDataSet & DataSet);
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif

