//---------------------------------------------------------------------------
#ifndef SpaceTimePermutationDataStreamHandlerH
#define SpaceTimePermutationDataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "SpaceTimeRandomizer.h"

class SpaceTimePermutationDataStreamHandler : public DataStreamHandler {
  protected:
    std::vector<SpaceTimeRandomizer>    gvDataStreamRandomizers;

    virtual void                        AllocateCaseStructures(unsigned int iStream);
    virtual bool                        ReadCounts(size_t tStream, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    SpaceTimePermutationDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~SpaceTimePermutationDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual RandomizerContainer_t     & GetRandomizerContainer(RandomizerContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual void                        RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber);
    virtual bool                        ReadData();
};

#endif
