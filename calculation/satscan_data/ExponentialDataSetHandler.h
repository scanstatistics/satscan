//******************************************************************************
#ifndef __ExponentialDataSetHandler_H
#define __ExponentialDataSetHandler_H
//******************************************************************************
#include "DataSetHandler.h"
#include "ExponentialRandomizer.h"

class ExponentialDataSetHandler : public DataSetHandler {
  protected:
    static const count_t                gtMinimumNotCensoredCases;

    virtual void                        AllocateCaseStructures(size_t tSetIndex);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate,
                                                          measure_t& tContinuosVariable,
                                                          count_t& tCensored);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    ExponentialDataSetHandler(CSaTScanData& DataHub, BasePrint& Print);
    virtual ~ExponentialDataSetHandler();

    virtual SimulationDataContainer_t & AllocateSimulationData(SimulationDataContainer_t& Container) const;
    virtual AbtractDataSetGateway     & GetDataGateway(AbtractDataSetGateway& DataGatway) const;
    virtual AbtractDataSetGateway     & GetSimulationDataGateway(AbtractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const;
    virtual double                      GetSimulationDataSetAllocationRequirements() const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//******************************************************************************
#endif
