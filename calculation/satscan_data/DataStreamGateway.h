//---------------------------------------------------------------------------
#ifndef DataStreamGatewayH
#define DataStreamGatewayH
//---------------------------------------------------------------------------
#include "DataStreamInterface.h"

class AbtractDataStreamGateway {
  public:
    AbtractDataStreamGateway() {}
    virtual ~AbtractDataStreamGateway() {}

    virtual void                               AddDataStreamInterface(DataStreamInterface & Interface) = 0;
    inline virtual const DataStreamInterface & GetDataStreamInterface(size_t tStream=0) const = 0;
    inline virtual DataStreamInterface       & GetDataStreamInterface(size_t tStream) = 0;
    inline virtual size_t                      GetNumInterfaces() const = 0;
};

/** Collection of data stream interfaces. */
class DataStreamGateway : public AbtractDataStreamGateway {
  private:
    DataStreamInterface         gInterface;

  public:
    DataStreamGateway();
    virtual ~DataStreamGateway();

    virtual void                               AddDataStreamInterface(DataStreamInterface & Interface);
    inline virtual const DataStreamInterface & GetDataStreamInterface(size_t tStream=0) const {return gInterface;}
    inline virtual DataStreamInterface       & GetDataStreamInterface(size_t tStream) {return gInterface;}
    inline virtual size_t                      GetNumInterfaces() const {return 1;}
};

/** Collection of data stream interfaces. */
class MultipleDataStreamGateway : public AbtractDataStreamGateway {
  private:
    std::vector<DataStreamInterface>         gvDataStreams;

  public:
    MultipleDataStreamGateway();
    virtual ~MultipleDataStreamGateway();

    virtual void                               AddDataStreamInterface(DataStreamInterface & Interface);
    inline virtual const DataStreamInterface & GetDataStreamInterface(size_t tStream=0) const {return gvDataStreams[tStream];}
    inline virtual DataStreamInterface       & GetDataStreamInterface(size_t tStream) {return gvDataStreams[tStream];}
    inline virtual size_t                      GetNumInterfaces() const {return gvDataStreams.size();}
};
#endif
