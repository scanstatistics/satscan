//***************************************************************************
#ifndef __OrdinalDataSetHandler_H
#define __OrdinalDataSetHandler_H
//***************************************************************************
#include "DataStreamHandler.h"
#include "OrdinalDataRandomizer.h"

class OrdinalDataSetHandler : public DataStreamHandler {
  protected:
    virtual void                        AllocateCaseStructures(unsigned int tSetIndex);
    virtual bool                        ParseCaseFileLine(StringParser& Parser, tract_t& tid,
                                                          count_t& nCount, Julian& nDate, measure_t& tContinuosVariable);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char*);
    virtual void                        SetRandomizers();

  public:
    OrdinalDataSetHandler(CSaTScanData& Data, BasePrint& Print);
    virtual ~OrdinalDataSetHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
    virtual void                        SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer);
};
//***************************************************************************
#endif
