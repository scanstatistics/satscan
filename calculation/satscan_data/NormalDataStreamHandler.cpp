//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "NormalDataStreamHandler.h"

/** constructor */
NormalDataStreamHandler::NormalDataStreamHandler(CSaTScanData& Data, BasePrint& Print)
                        :DataStreamHandler(Data, Print) {}

/** destructor */
NormalDataStreamHandler::~NormalDataStreamHandler() {}

/** allocates cases structures for stream*/
void NormalDataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream]->AllocateCasesArray();
    gvDataStreams[iStream]->AllocateMeasureArray();
    gvDataStreams[iStream]->AllocateSqMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","NormalDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * NormalDataStreamHandler::GetNewDataGateway() const {
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
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          Interface.SetSqMeasureArray(thisStream.GetSqMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
          Interface.SetPTSqMeasureArray(thisStream.GetPTSqMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          Interface.SetSqMeasureArray(thisStream.GetSqMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
            Interface.SetPTSqMeasureArray(thisStream.GetPTSqMeasureArray());
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
    x.AddCallpath("GetNewDataGateway()","NormalDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * NormalDataStreamHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
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
          Interface.SetCaseArray(thisRealStream.GetCaseArray());
          Interface.SetMeasureArray(thisSimulationStream.GetMeasureArray());
          Interface.SetSqMeasureArray(thisSimulationStream.GetSqMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisRealStream.GetPTCasesArray());
          Interface.SetPTMeasureArray(thisSimulationStream.GetPTMeasureArray());
          Interface.SetPTSqMeasureArray(thisSimulationStream.GetPTSqMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisRealStream.GetCaseArray());
          Interface.SetMeasureArray(thisSimulationStream.GetMeasureArray());
          Interface.SetSqMeasureArray(thisSimulationStream.GetSqMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisRealStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisSimulationStream.GetPTMeasureArray());
            Interface.SetPTSqMeasureArray(thisSimulationStream.GetPTSqMeasureArray());
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
    x.AddCallpath("GetNewSimulationDataGateway()","NormalDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** Returns a collection of cloned randomizers maintained by data stream handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& NormalDataStreamHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  std::vector<NormalRandomizer>::const_iterator itr;

  try {
    Container.DeleteAllElements();
    for (itr=gvDataStreamRandomizers.begin(); itr != gvDataStreamRandomizers.end(); ++itr)
       Container.push_back(itr->Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","NormalDataStreamHandler");
    throw;
  }
  return Container;
}

SimulationDataContainer_t& NormalDataStreamHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataStreams(); ++t)
    Container.push_back(new SimulationDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t) {
           Container[t]->AllocateMeasureArray();
           Container[t]->AllocateSqMeasureArray();
        }
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
           Container[t]->AllocateMeasureArray();
           Container[t]->AllocateSqMeasureArray();
           Container[t]->AllocatePTMeasureArray();
           Container[t]->AllocatePTSqMeasureArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateMeasureArray();
          Container[t]->AllocateSqMeasureArray();
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Container[t]->AllocatePTMeasureArray();
            Container[t]->AllocatePTSqMeasureArray();
          }
        }
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

bool NormalDataStreamHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gPrint.PrintInputWarning("Error: Unknown location ID in %s, record %ld.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      gPrint.PrintInputWarning("       Location ID '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld in the %s could not be read as case count.\n",
                                  Parser.GetWord(1), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       gPrint.PrintInputWarning("       Case count must be an integer.\n");
       return false;
      }
    }
    else {
      gPrint.PrintInputWarning("Error: Record %ld in the %s does not contain case count.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gPrint.PrintInputWarning("Error: Record %ld, of the %s, contains a negative case count.\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.PrintInputWarning("Error: The case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;

    // read continuos variable 
    if (!Parser.GetWord(3)) {
      gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing the continuous variable.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuosVariable) != 1) {
       gPrint.PrintInputWarning("Error: The continuos variable value '%s' in record %ld, of %s, is not a number.\n",
                                  Parser.GetWord(3), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
//    //validate that population is not negative or exceeding type precision
//    if (tContinuosVariable < 0) {//validate that count is not negative or exceeds type precision
//      if (strstr(Parser.GetWord(3), "-"))
//        gPrint.PrintInputWarning("Error: Negative continuos variable in record %ld of cases file.\n", Parser.GetReadCount());
//      else
//        gPrint.PrintInputWarning("Error: Continuos variable '%s' exceeds maximum value of %i in record %ld of population file.\n",
//                                   Parser.GetWord(3), std::numeric_limits<measure_t>::max(), Parser.GetReadCount());
//      return false;
//     }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","NormalDataStreamHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool NormalDataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i;
  count_t       Count, ** ppCounts, tTotalCases=0;
  measure_t     tContinuosVariable, ** ppMeasure, ** ppSqMeasure, tTotalMeasure=0;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    StringParser Parser(gPrint);
    NormalRandomizer & Randomizer = gvDataStreamRandomizers[tStream];

    ppCounts = thisStream.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable)) {
             //cumulatively add count to time by location structure
             ppCounts[0][TractIndex] += Count;
             if (ppCounts[0][TractIndex] < 0)
               GenerateResolvableException("Error:: The total number of cases, in data stream %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                           tStream, std::numeric_limits<count_t>::max());
             for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
               ppCounts[i][TractIndex] += Count;
             //record count as a case or control  
//             thisStream.GetPopulationData().AddCaseCount(0, Count);
             for (i=0; i < Count; ++i)
                Randomizer.AddCase(gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tContinuosVariable);

             tTotalCases += Count;
             tTotalMeasure += tContinuosVariable;
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.SatScanPrintWarning("Please see the 'case file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: The %s does not contain data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    else {
     Randomizer.AssignMeasure(thisStream.GetMeasureArray(), thisStream.GetSqMeasureArray(),
                              thisStream.GetNumTimeIntervals(), thisStream.GetNumTracts());
     thisStream.SetTotalCases(tTotalCases);
     thisStream.SetTotalMeasure(tTotalMeasure);
     //thisStream.SetTotalCasesAtStart(tTotalCases);
    }

  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","NormalDataStreamHandler");
    throw;
  }
  return bValid;
}


bool NormalDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       if (GetNumStreams() == 1)
         gPrint.SatScanPrintf("Reading the case file\n");
       else
         gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","NormalDataStreamHandler");
    throw;
  }
  return true;
}

void NormalDataStreamHandler::SetPurelyTemporalMeasureData(RealDataStream & thisRealStream) {
  try {
    thisRealStream.SetPTMeasureArray();
    thisRealStream.SetPTSqMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasureData()","NormalDataStreamHandler");
    throw;
  }
}

/** sets purely temporal structures used in simulations */
void NormalDataStreamHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t) {
       SimDataContainer[t]->SetPTMeasureArray();
       SimDataContainer[t]->SetPTSqMeasureArray();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","NormalDataStreamHandler");
    throw;
  }
}


void NormalDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), NormalRandomizer(gParameters.GetRandomizationSeed()));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","NormalDataStreamHandler");
    throw;
  }
}

