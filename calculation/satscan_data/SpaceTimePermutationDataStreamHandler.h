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

    virtual void                        AllocateSimulationStructures();
    virtual AbtractDataStreamGateway  * GetNewDataGateway();
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway();
    virtual void                        RandomizeData(unsigned int iSimulationNumber);
    virtual bool                        ReadData();
};

#endif
