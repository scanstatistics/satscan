//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStreamGateway.h"
//---------------------------------------------------------------------------
/** constructor */
DataStreamGateway::DataStreamGateway() {}

/** destructor */                            
DataStreamGateway::~DataStreamGateway() {}

/** adds data stream interface to list of interfaces */
void DataStreamGateway::AddDataStreamInterface(DataStreamInterface & Interface) {
  gvDataStreams.push_back(Interface);
}

