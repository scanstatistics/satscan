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
void DataStreamGateway::AddDataStreamInterface(DataStreamInterface & Interface) {
  gInterface = Interface;
}



/** constructor */
MultipleDataStreamGateway::MultipleDataStreamGateway() : AbtractDataStreamGateway() {}

/** destructor */                            
MultipleDataStreamGateway::~MultipleDataStreamGateway() {}

/** adds data stream interface to list of interfaces */
void MultipleDataStreamGateway::AddDataStreamInterface(DataStreamInterface & Interface) {
  gvDataStreams.push_back(Interface);
}

