//******************************************************************************
#ifndef DataSetGatewayH
#define DataSetGatewayH
//******************************************************************************
#include "DataSetInterface.h"

class AbstractDataSetGateway {
  public:
    AbstractDataSetGateway() {}
    virtual ~AbstractDataSetGateway() {}

    virtual void                               AddDataSetInterface(DataSetInterface & Interface) = 0;
    virtual void                               Clear() = 0;
    virtual const DataSetInterface           & GetDataSetInterface(size_t tSetIndex=0) const = 0;
    virtual DataSetInterface                 & GetDataSetInterface(size_t tSetIndex) = 0;
    virtual size_t                             GetNumInterfaces() const = 0;
};

/** Collection of dataset interfaces. */
class DataSetGateway : public AbstractDataSetGateway {
  private:
    DataSetInterface         gInterface;

  public:
    DataSetGateway();
    virtual ~DataSetGateway();

    virtual void                               AddDataSetInterface(DataSetInterface & Interface);
    virtual void                               Clear() {}
    inline virtual const DataSetInterface    & GetDataSetInterface(size_t tSetIndex=0) const {return gInterface;}
    inline virtual DataSetInterface          & GetDataSetInterface(size_t tSetIndex) {return gInterface;}
    inline virtual size_t                      GetNumInterfaces() const {return 1;}
};

/** Collection of dataset interfaces. */
class MultipleDataSetGateway : public AbstractDataSetGateway {
  private:
    std::vector<DataSetInterface>              gvDataSetInterfaces;

  public:
    MultipleDataSetGateway();
    virtual ~MultipleDataSetGateway();

    virtual void                               AddDataSetInterface(DataSetInterface& Interface);
    virtual void                               Clear() {gvDataSetInterfaces.clear();}
    inline virtual const DataSetInterface    & GetDataSetInterface(size_t tSetIndex=0) const {return gvDataSetInterfaces[tSetIndex];}
    inline virtual DataSetInterface          & GetDataSetInterface(size_t tSetIndex) {return gvDataSetInterfaces[tSetIndex];}
    inline virtual size_t                      GetNumInterfaces() const {return gvDataSetInterfaces.size();}
};
//******************************************************************************
#endif

