//---------------------------------------------------------------------------
#ifndef BernoulliDataStreamHandlerH
#define BernoulliDataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "BernoulliRandomizer.h"

class BernoulliDataStreamHandler : public DataStreamHandler {
  protected:
    std::vector<BernoulliNullHypothesisRandomizer>       gvDataStreamRandomizers;

    void                                AllocateControlStructures(unsigned int iStream);
    bool                                ReadControlFile(size_t tStream);
    virtual void                        SetRandomizers();

  public:
    BernoulliDataStreamHandler(CSaTScanData& Data, BasePrint& Print);
    virtual ~BernoulliDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};
#endif
