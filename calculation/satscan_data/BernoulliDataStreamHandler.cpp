//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "BernoulliDataStreamHandler.h"
//---------------------------------------------------------------------------

/** constructor */
BernoulliDataStreamHandler::BernoulliDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                           :DataStreamHandler(Data, pPrint) {}

/** destructor */
BernoulliDataStreamHandler::~BernoulliDataStreamHandler() {}

/** allocates cases structures for stream*/
void BernoulliDataStreamHandler::AllocateControlStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream].AllocateControlsArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateControlStructures()","BernoulliDataStreamHandler");
    throw;
  }
}

/** allocates structures used during simulations - based particularly upon analysis type */
void BernoulliDataStreamHandler::AllocateSimulationStructures() {
  try {
    switch (gParameters.GetAnalysisType()) {
       case PURELYSPATIAL :
         AllocateSimulationCases();
         break;
       case PURELYSPATIALMONOTONE :
         ZdGenerateException("AllocateSimulationStructures() not implemented for purely spatial monotone analysis.","AllocateSimulationStructures()");
       case PURELYTEMPORAL :
       case PROSPECTIVEPURELYTEMPORAL :
         AllocateSimulationCases();
         AllocatePTSimulationCases();
         break;
       case SPACETIME :
       case PROSPECTIVESPACETIME :
         AllocateSimulationCases();
         if (gParameters.GetIncludePurelyTemporalClusters())
           AllocatePTSimulationCases();
         break;
       case SPATIALVARTEMPTREND :
         ZdGenerateException("AllocateSimulationStructures() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationStructures()");
      default :
         ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationStructures()", gParameters.GetAnalysisType());
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","BernoulliDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * BernoulliDataStreamHandler::GetNewDataGateway() {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      DataStream & thisStream = gvDataStreams[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          break;
        case PURELYSPATIALMONOTONE      :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewDataGateway()","BernoulliDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * BernoulliDataStreamHandler::GetNewSimulationDataGateway() {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      DataStream & thisStream = gvDataStreams[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          break;
        case PURELYSPATIALMONOTONE      :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","BernoulliDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** randomizes each data streams */
void BernoulliDataStreamHandler::RandomizeData(unsigned int iSimulationNumber) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     gvDataStreamRandomizers[t].RandomizeData(gvDataStreams[t], iSimulationNumber);
}

/** Read the control data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool BernoulliDataStreamHandler::ReadControlFile(size_t tStream) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    if ((fp = fopen(gParameters.GetControlFileName(tStream + 1).c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open control file:\n'%s'.\n",
                                   gParameters.GetControlFileName(tStream + 1).c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::CONTROLFILE, (GetNumStreams() == 1 ? 0 : tStream + 1));
    AllocateControlStructures(tStream);
    bValid = ReadCounts(tStream, fp, "control");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadControlFile()","BernoulliDataStreamHandler");
    throw;
  }
  return bValid;
}

/** */
bool BernoulliDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       GetStream(t).SetAggregateCategories(true);
       if (GetNumStreams() == 1)
         gpPrint->SatScanPrintf("Reading the case file\n");
       else
         gpPrint->SatScanPrintf("Reading input stream %u case file\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
       if (GetNumStreams() == 1)
         gpPrint->SatScanPrintf("Reading the control file\n");
       else
         gpPrint->SatScanPrintf("Reading input stream %u control file\n", t + 1);
       if (!ReadControlFile(t))
         return false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","BernoulliDataStreamHandler");
    throw;
  }
  return true;
}

/** allocates randomizers for each data stream */
void BernoulliDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), BernoulliNullHypothesisRandomizer());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","BernoulliDataStreamHandler");
    throw;
  }
}


