//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStreamGateway.h"
//---------------------------------------------------------------------------
/** constructor */
DataStreamGateway::DataStreamGateway() : AbtractDataStreamGateway() {}

/** destructor */                            
DataStreamGateway::~DataStreamGateway() {}

/** adds data stream interface to list of interfaces */
void DataStreamGateway::AddDataSetInterface(DataStreamInterface& Interface) {
  gInterface = Interface;
}



/** constructor */
MultipleDataStreamGateway::MultipleDataStreamGateway() : AbtractDataStreamGateway() {}

/** destructor */                            
MultipleDataStreamGateway::~MultipleDataStreamGateway() {}

/** adds data stream interface to list of interfaces */
void MultipleDataStreamGateway::AddDataSetInterface(DataStreamInterface & Interface) {
  gvDataSetInterfaces.push_back(Interface);
}

