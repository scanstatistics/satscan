//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetGateway.h"

/** constructor */
DataSetGateway::DataSetGateway() : AbtractDataSetGateway() {}

/** destructor */                            
DataSetGateway::~DataSetGateway() {}

/** adds dataset interface to list of interfaces */
void DataSetGateway::AddDataSetInterface(DataSetInterface& Interface) {
  gInterface = Interface;
}



/** constructor */
MultipleDataSetGateway::MultipleDataSetGateway() : AbtractDataSetGateway() {}

/** destructor */                            
MultipleDataSetGateway::~MultipleDataSetGateway() {}

/** adds dataset interface to list of interfaces */
void MultipleDataSetGateway::AddDataSetInterface(DataSetInterface & Interface) {
  gvDataSetInterfaces.push_back(Interface);
}

