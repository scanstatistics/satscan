//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimePermutationDataStreamHandler.h"
#include "SaTScanData.h"
//---------------------------------------------------------------------------

/** constructor */
SpaceTimePermutationDataStreamHandler::SpaceTimePermutationDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                                      :DataStreamHandler(Data, pPrint) {}

/** destructor */
SpaceTimePermutationDataStreamHandler::~SpaceTimePermutationDataStreamHandler() {}

/** allocates cases structures for stream */
void SpaceTimePermutationDataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream]->AllocateCasesArray();
    gvDataStreams[iStream]->AllocateCategoryCasesArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * SpaceTimePermutationDataStreamHandler::GetNewDataGateway() const {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      const RealDataStream& thisStream = *gvDataStreams[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetNewDataGateway() not implemented for purely temporal analysis.","GetNewDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for spatial variation and temporal trends analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewDataGateway()","SpaceTimePermutationDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * SpaceTimePermutationDataStreamHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      const RealDataStream& thisRealStream = *gvDataStreams[t];
      const SimulationDataStream& thisSimulationStream = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisRealStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisRealStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for purely spatial analysis.","GetNewSimulationDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for purely temporal analysis.","GetNewSimulationDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisSimulationStream.GetCaseArray());
          Interface.SetMeasureArray(thisRealStream.GetMeasureArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for spatial variation and temporal trends analysis.","GetNewSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
  return pDataStreamGateway;
}

/** Returns a collection of cloned randomizers maintained by data stream handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& SpaceTimePermutationDataStreamHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  std::vector<SpaceTimeRandomizer>::const_iterator itr;

  try {
    Container.DeleteAllElements();
    for (itr=gvDataStreamRandomizers.begin(); itr != gvDataStreamRandomizers.end(); ++itr)
       Container.push_back(itr->Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
  return Container;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& SpaceTimePermutationDataStreamHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataStreams(); ++t)
    Container.push_back(new SimulationDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        ZdGenerateException("GetSimulationDataContainer() not implemented for purely spatial analysis.","GetSimulationDataContainer()");
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        ZdGenerateException("GetSimulationDataContainer() not implemented for purely temporal analysis.","GetSimulationDataContainer()");
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCasesArray();
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** Read the count data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SpaceTimePermutationDataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  int                                   i, j, iCategoryIndex;
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(*gpPrint);
  std::string                           sBuffer;
  count_t                               Count, ** pCounts;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    SpaceTimeRandomizer & Randomizer = gvDataStreamRandomizers[tStream];

    pCounts = thisStream.GetCaseArray();
    ThreeDimCountArray_t & CategoryHandler = thisStream.GetCategoryCaseArrayHandler();

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(thisStream.GetPopulationData(), Parser, TractIndex, Count, Date, iCategoryIndex)) {
              if (Count > 0) {//ignore records that specify a count of zero
                //cumulatively add count to time by location structure
                pCounts[0][TractIndex] += Count;
                if (pCounts[0][TractIndex] < 0)
                  GenerateResolvableException("Error: The total number of cases, in data stream %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                              tStream, std::numeric_limits<count_t>::max());
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                  pCounts[i][TractIndex] += Count;
                //record count as a case
                thisStream.GetPopulationData().AddCaseCount(iCategoryIndex, Count);
                //record count in structure(s) based upon population category
                if (iCategoryIndex >= static_cast<int>(CategoryHandler.Get3rdDimension()))
                  CategoryHandler.ExpandThirdDimension(0);
                CategoryHandler.GetArray()[0][TractIndex][iCategoryIndex] += Count;
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                   CategoryHandler.GetArray()[i][TractIndex][iCategoryIndex] += Count;
              }
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->SatScanPrintWarning("Please see the '%s file' section in the user guide for help.\n", szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: The %s file does not contain data.\n", szDescription);
      bValid = false;
    }

    Randomizer.CreateRandomizationData(thisStream);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
  return bValid;
}

/** */
bool SpaceTimePermutationDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       if (GetNumStreams() == 1)
         gpPrint->SatScanPrintf("Reading the case file\n");
       else
         gpPrint->SatScanPrintf("Reading the case file for input stream %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
  return true;
}

void SpaceTimePermutationDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), SpaceTimeRandomizer());
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","SpaceTimePermutationDataStreamHandler");
    throw;
  }
}


