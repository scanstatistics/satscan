//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "RankDataStreamHandler.h"

/** constructor */
RankDataStreamHandler::RankDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                      :DataStreamHandler(Data, pPrint) {}

/** destructor */
RankDataStreamHandler::~RankDataStreamHandler() {}

/** allocates cases structures for stream*/
void RankDataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream]->AllocateCasesArray();
    gvDataStreams[iStream]->AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","RankDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * RankDataStreamHandler::GetNewDataGateway() const {
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
          break;
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
    x.AddCallpath("GetNewDataGateway()","RankDataStreamHandler");
    throw;
  }
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * RankDataStreamHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
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
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisRealStream.GetPTCasesArray());
          Interface.SetPTMeasureArray(thisSimulationStream.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisRealStream.GetCaseArray());
          Interface.SetMeasureArray(thisSimulationStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisRealStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisSimulationStream.GetPTMeasureArray());
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
    x.AddCallpath("GetNewSimulationDataGateway()","RankDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** Returns a collection of cloned randomizers maintained by data stream handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& RankDataStreamHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  ZdPointerVector<RankRandomizer>::const_iterator itr;

  try {
    Container.DeleteAllElements();
    for (itr=gvDataStreamRandomizers.begin(); itr != gvDataStreamRandomizers.end(); ++itr)
       Container.push_back((*itr)->Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","RankDataStreamHandler");
    throw;
  }
  return Container;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& RankDataStreamHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataStreams(); ++t)
    Container.push_back(new SimulationDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateMeasureArray();
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateMeasureArray();
          Container[t]->AllocatePTMeasureArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateMeasureArray();
          if (gParameters.GetIncludePurelyTemporalClusters())
            Container[t]->AllocatePTMeasureArray();
        }
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

bool RankDataStreamHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                              count_t& nCount, Julian& nDate,
                                              measure_t& tContinuosVariable) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gpPrint->PrintInputWarning("Error: Unknown location ID in %s, record %ld.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      gpPrint->PrintInputWarning("       Location ID '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gpPrint->PrintInputWarning("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n",
                                  Parser.GetWord(1), Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
       gpPrint->PrintInputWarning("       Case count must be an integer.\n");
       return false;
      }
    }
    else {
      gpPrint->PrintInputWarning("Error: Record %ld, in the %s, does not contain case count.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gpPrint->PrintInputWarning("Error: Record %ld, of the %s, contains a negative case count.\n",
                                   Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      else
        gpPrint->PrintInputWarning("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                                   Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;

    // read continuos variable
    if (!Parser.GetWord(3)) {
      gpPrint->PrintInputWarning("Error: Record %d, of the %s, is missing the continuos variable.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuosVariable) != 1) {
       gpPrint->PrintInputWarning("Error: The continuos variable value '%s' in record %ld, of %s, is not a number.\n",
                                  Parser.GetWord(3), Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
       return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","RankDataStreamHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool RankDataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i;
  count_t       Count, tCensored, ** ppCounts, tTotalCases=0;
  measure_t     tContinuosVariable, ** ppMeasure, ** ppSqMeasure, tTotalMeasure=0;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    StringParser Parser(*gpPrint);
    RankRandomizer & Randomizer = *gvDataStreamRandomizers[tStream];

    ppCounts = thisStream.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable)) {
             //cumulatively add count to time by location structure
             ppCounts[0][TractIndex] += Count;
             if (ppCounts[0][TractIndex] < 0)
               GenerateResolvableException("Error: The total number of cases, in data stream %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
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
      gpPrint->SatScanPrintWarning("Please see the 'case file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: The %s does not contain data.\n", gpPrint->GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    else {
     Randomizer.AssignMeasure(thisStream.GetMeasureArray(), thisStream.GetNumTimeIntervals(), thisStream.GetNumTracts());
     thisStream.SetTotalCases(tTotalCases);
     thisStream.SetTotalMeasure(tTotalMeasure);
    }

  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","RankDataStreamHandler");
    throw;
  }
  return bValid;
}


bool RankDataStreamHandler::ReadData() {
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
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","RankDataStreamHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void RankDataStreamHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t)
       SimDataContainer[t]->SetPTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","RankDataStreamHandler");
    throw;
  }
}

void RankDataStreamHandler::SetRandomizers() {
  try {
    for (size_t t=0; t < gParameters.GetNumDataStreams(); ++t)
      gvDataStreamRandomizers.push_back(new RankRandomizer());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","RankDataStreamHandler");
    throw;
  }
}


