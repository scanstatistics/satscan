//---------------------------------------------------------------------------
#ifndef DataStreamGatewayH
#define DataStreamGatewayH
//---------------------------------------------------------------------------
#include "DataStreamInterface.h"

class AbtractDataStreamGateway {
  public:
    AbtractDataStreamGateway() {}
    virtual ~AbtractDataStreamGateway() {}

    virtual void                               AddDataSetInterface(DataStreamInterface & Interface) = 0;
    virtual const DataStreamInterface        & GetDataSetInterface(size_t tSetIndex=0) const = 0;
    virtual DataStreamInterface              & GetDataSetInterface(size_t tSetIndex) = 0;
    virtual size_t                             GetNumInterfaces() const = 0;
};

/** Collection of data stream interfaces. */
class DataStreamGateway : public AbtractDataStreamGateway {
  private:
    DataStreamInterface         gInterface;

  public:
    DataStreamGateway();
    virtual ~DataStreamGateway();

    virtual void                               AddDataSetInterface(DataStreamInterface & Interface);
    inline virtual const DataStreamInterface & GetDataSetInterface(size_t tSetIndex=0) const {return gInterface;}
    inline virtual DataStreamInterface       & GetDataSetInterface(size_t tSetIndex) {return gInterface;}
    inline virtual size_t                      GetNumInterfaces() const {return 1;}
};

/** Collection of data stream interfaces. */
class MultipleDataStreamGateway : public AbtractDataStreamGateway {
  private:
    std::vector<DataStreamInterface>         gvDataSetInterfaces;

  public:
    MultipleDataStreamGateway();
    virtual ~MultipleDataStreamGateway();

    virtual void                               AddDataSetInterface(DataStreamInterface& Interface);
    inline virtual const DataStreamInterface & GetDataSetInterface(size_t tSetIndex=0) const {return gvDataSetInterfaces[tSetIndex];}
    inline virtual DataStreamInterface       & GetDataSetInterface(size_t tSetIndex) {return gvDataSetInterfaces[tSetIndex];}
    inline virtual size_t                      GetNumInterfaces() const {return gvDataSetInterfaces.size();}
};
#endif
