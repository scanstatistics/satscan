//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStreamInterface.h"
//---------------------------------------------------------------------------

/** constructor */
DataStreamInterface::DataStreamInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts)
                    :giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts) {
  Init();
}

/** destructor */
DataStreamInterface::~DataStreamInterface() {}

/** internal initialization */
void DataStreamInterface::Init() {
  gTotalCases=0;
  gTotalControls=0;
  gTotalMeasure=0;
  gppCaseArray=0;
  gppNCCaseArray=0;
  gpPTCaseArray=0;
  gppMeasureArray=0;
  gpPTMeasureArray=0;
  gppSqMeasureArray=0;
  gppNCMeasureArray=0;
  gpTimeTrend=0;
}

/** initializes case array */
void DataStreamInterface::ResetCaseArray(count_t t) {
  unsigned int  i, j;

  for (i=0; i < giNumTimeIntervals; ++i)
     for (j=0; j < giNumTracts; ++j)
        gppCaseArray[i][j] = 0;
}

/** Based upon parameters settings, particularly analysis type, assigns class
    pointers to data stream structures. */
void DataStreamInterface::Set(DataStream & thisStream, const CParameters & Parameters) {
  try {
    switch (Parameters.GetAnalysisType()) {
      case PURELYSPATIAL              :
      case PURELYSPATIALMONOTONE      : SetPurelySpatialAnalysisInterface(thisStream, Parameters); break;
      case PROSPECTIVEPURELYTEMPORAL  :
      case PURELYTEMPORAL             : SetPurelyTemporalAnalysisInterface(thisStream, Parameters); break;
      case SPACETIME                  :
      case PROSPECTIVESPACETIME       : SetSpaceTimeAnalysisInterface(thisStream, Parameters); break;
      case SPATIALVARTEMPTREND        : SetSVTTAnalysisInterface(thisStream, Parameters); break;
      default : ZdGenerateException("Unknown analysis type.","Set()");
    }

  }
  catch (ZdException &x) {
    x.AddCallpath("Set()","DataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for purely spatial analysis. (real data) */
void DataStreamInterface::SetPurelySpatialAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetCaseArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelySpatialAnalysisInterface()","DataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for purely temporal analysis. (real data) */
void DataStreamInterface::SetPurelyTemporalAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetPTMeasureArray(thisStream.GetPTMeasureArray());
    SetPTCaseArray(thisStream.GetPTCasesArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelySpatialAnalysisInterface()","DataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for space-time analysis. (real data) */
void DataStreamInterface::SetSpaceTimeAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetCaseArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    if (Parameters.GetIncludePurelyTemporalClusters()) {
      SetPTMeasureArray(thisStream.GetPTMeasureArray());
      SetPTCaseArray(thisStream.GetPTCasesArray());
    }  
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpaceTimeAnalysisInterface()","DataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for spatial variation and temporal trends analysis. (real data) */
void DataStreamInterface::SetSVTTAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetCaseArray());
    SetNCCaseArray(thisStream.GetNCCaseArray());
    SetPTCaseArray(thisStream.GetPTCasesArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    SetNCMeasureArray(thisStream.GetNCMeasureArray());
    SetPTMeasureArray(thisStream.GetPTMeasureArray());
    SetTimeTrend(&thisStream.GetTimeTrend());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSVTTAnalysisInterface()","DataStreamInterface");
    throw;
  }
}


/** constructor */
SimulationDataStreamInterface::SimulationDataStreamInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts)
                              :DataStreamInterface(iNumTimeIntervals, iNumTracts) {}

/** destructor */
SimulationDataStreamInterface::~SimulationDataStreamInterface() {}

/** Assigns class pointers to data stream structures for purely spatial analysis. (simulation data) */
void SimulationDataStreamInterface::SetPurelySpatialAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetSimCaseArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelySpatialAnalysisInterface()","SimulationDataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for purely temporal analysis. (simulation data) */
void SimulationDataStreamInterface::SetPurelyTemporalAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetSimCaseArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    SetPTMeasureArray(thisStream.GetPTMeasureArray());
    SetPTCaseArray(thisStream.GetPTSimCasesArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelySpatialAnalysisInterface()","DataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for space-time analysis. (simulation data) */
void SimulationDataStreamInterface::SetSpaceTimeAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetSimCaseArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    if (Parameters.GetIncludePurelyTemporalClusters()) {
      SetPTMeasureArray(thisStream.GetPTMeasureArray());
      SetPTCaseArray(thisStream.GetPTSimCasesArray());
    }
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpaceTimeAnalysisInterface()","SimulationDataStreamInterface");
    throw;
  }
}

/** Assigns class pointers to data stream structures for spatial variation and temporal trends analysis. (simulation data) */
void SimulationDataStreamInterface::SetSVTTAnalysisInterface(DataStream & thisStream, const CParameters & Parameters) {
  try {
    SetCaseArray(thisStream.GetSimCaseArray());
    SetNCCaseArray(thisStream.GetNCSimCaseArray());
    SetPTCaseArray(thisStream.GetPTSimCasesArray());
    SetMeasureArray(thisStream.GetMeasureArray());
    SetNCMeasureArray(thisStream.GetNCMeasureArray());
    SetPTMeasureArray(thisStream.GetPTMeasureArray());
    SetTimeTrend(&thisStream.GetSimTimeTrend());
    //SetSqMeasureArray(thisStream.GetSqMeasureArray());
    SetTotalCasesCount(thisStream.GetTotalCases());
    SetTotalControlsCount(thisStream.GetTotalControls());
    SetTotalMeasureCount(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSVTTAnalysisInterface()","SimulationDataStreamInterface");
    throw;
  }
}

