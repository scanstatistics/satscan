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
    BernoulliDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint);
    virtual ~BernoulliDataStreamHandler();

    virtual void                        AllocateSimulationStructures();
    virtual AbtractDataStreamGateway  * GetNewDataGateway();
    virtual AbtractDataStreamGateway  * GetNewSimulationDataGateway();
    virtual void                        RandomizeData(unsigned int iSimulationNumber);
    virtual bool                        ReadData();
};
#endif
