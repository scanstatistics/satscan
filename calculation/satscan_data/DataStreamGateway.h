//---------------------------------------------------------------------------
#ifndef DataStreamGatewayH
#define DataStreamGatewayH
//---------------------------------------------------------------------------
#include "DataStreamInterface.h"

/** Collection of data stream interfaces. */
class DataStreamGateway {
  private:
    std::vector<DataStreamInterface>         gvDataStreams;

  public:
            DataStreamGateway();
    virtual ~DataStreamGateway();

    void                        AddDataStreamInterface(DataStreamInterface & Interface);
    const DataStreamInterface & GetDataStreamInterface(size_t tStream) const {return gvDataStreams[tStream];}
    DataStreamInterface       & GetDataStreamInterface(size_t tStream) {return gvDataStreams[tStream];}
    size_t                      GetNumInterfaces() const {return gvDataStreams.size();}
};
#endif
 