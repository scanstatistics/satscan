//---------------------------------------------------------------------------
#ifndef SpaceTimePermutationDataStreamHandlerH
#define SpaceTimePermutationDataStreamHandlerH
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "SpaceTimeRandomizer.h"

class SpaceTimePermutationDataStreamHandler : public DataStreamHandler {
  protected:
    virtual void                        AllocateCaseStructures(unsigned int tSetIndex);
    virtual bool                        ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription);
    virtual void                        SetRandomizers();

  public:
    SpaceTimePermutationDataStreamHandler(CSaTScanData& Data, BasePrint& Print);
    virtual ~SpaceTimePermutationDataStreamHandler();

    virtual AbtractDataStreamGateway  * GetNewDataGateway() const;
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const;
    virtual SimulationDataContainer_t & GetSimulationDataContainer(SimulationDataContainer_t& Container) const;
    virtual bool                        ReadData();
};

#endif
