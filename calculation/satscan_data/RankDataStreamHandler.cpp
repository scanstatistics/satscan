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
    gvDataStreams[iStream].AllocateCasesArray();
    gvDataStreams[iStream].AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","RankDataStreamHandler");
    throw;
  }
}

void RankDataStreamHandler::AllocatePTSimulationMeasures() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocatePTSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTSimulationMeasures()","RankDataStreamHandler");
    throw;
  }
}

/** Allocates two dimensional array for expected case counts (number of time intervals by number of tracts). */
void RankDataStreamHandler::AllocateSimulationMeasures() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationMeasure()","RankDataStreamHandler");
    throw;
  }
}

/** allocates structures used during simulations - based particularly upon analysis type */
void RankDataStreamHandler::AllocateSimulationStructures() {
  try {
    switch (gParameters.GetAnalysisType()) {
       case PURELYSPATIAL :
         AllocateSimulationMeasures();
         break;
       case PURELYSPATIALMONOTONE :
         ZdGenerateException("AllocateSimulationStructures() not implemented for purely spatial monotone analysis.","AllocateSimulationStructures()");
       case PURELYTEMPORAL :
       case PROSPECTIVEPURELYTEMPORAL :
         AllocateSimulationMeasures();
         AllocatePTSimulationMeasures();
         break;
       case SPACETIME :
       case PROSPECTIVESPACETIME :
         AllocateSimulationMeasures();
         if (gParameters.GetIncludePurelyTemporalClusters())
           AllocatePTSimulationMeasures();
         break;
       case SPATIALVARTEMPTREND :
         ZdGenerateException("AllocateSimulationStructures() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationStructures()");
      default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationStructures()", gParameters.GetAnalysisType());
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","RankDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * RankDataStreamHandler::GetNewDataGateway() {
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
    x.AddCallpath("GetNewDataGateway()","RankDataStreamHandler");
    throw;
  }
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * RankDataStreamHandler::GetNewSimulationDataGateway() {
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
          Interface.SetMeasureArray(thisStream.GetSimMeasureArray());
          break;
        case PURELYSPATIALMONOTONE      :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
          Interface.SetPTMeasureArray(thisStream.GetPTSimMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetSimMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTSimMeasureArray());
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

bool RankDataStreamHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                              count_t& nCount, Julian& nDate,
                                              measure_t& tContinuosVariable) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gData.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gpPrint->PrintInputWarning("Error: Unknown location id in %s, record %ld.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      gpPrint->PrintInputWarning("       Location '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s could not be read as count.\n",
                                  Parser.GetWord(1), Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
       gpPrint->PrintInputWarning("       Count must be an integer.\n");
       return false;
      }
    }
    else {
      gpPrint->PrintInputWarning("Error: Record %ld in %s does not contain case count.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gpPrint->PrintInputWarning("Error: Negative count in record %ld of %s.\n",
                                   Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      else
        gpPrint->PrintInputWarning("Error: Count '%s' exceeds maximum value of %ld in record %ld of %s.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                                   Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;

    // read continuos variable
    if (!Parser.GetWord(3)) {
      gpPrint->PrintInputWarning("Error: Record %d of %s missing continuos variable.\n",
                                 Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuosVariable) != 1) {
       gpPrint->PrintInputWarning("Error: Continuos variable value '%s' in record %ld, of %s, is not a number.\n",
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

/** randomizes each data streams */
void RankDataStreamHandler::RandomizeData(unsigned int iSimulationNumber) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     gvDataStreamRandomizers[t].RandomizeData(gvDataStreams[t], iSimulationNumber);
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
    DataStream & thisStream = gvDataStreams[tStream];
    StringParser Parser(*gpPrint);
    RankRandomizer & Randomizer = gvDataStreamRandomizers[tStream];

    ppCounts = thisStream.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable)) {
             //cumulatively add count to time by location structure
             ppCounts[0][TractIndex] += Count;
             if (ppCounts[0][TractIndex] < 0)
               SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "ReadCounts()",
                                   std::numeric_limits<count_t>::max());
             for (i=1; Date >= gData.GetTimeIntervalStartTimes()[i]; ++i)
               ppCounts[i][TractIndex] += Count;
             //record count as a case or control  
//             thisStream.GetPopulationData().AddCaseCount(0, Count);
             for (i=0; i < Count; ++i)
                Randomizer.AddCase(gData.GetTimeIntervalOfDate(Date), TractIndex, tContinuosVariable);

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
      gpPrint->SatScanPrintWarning("Please see 'case file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: %s does not contain data.\n", gpPrint->GetImpliedFileTypeString().c_str());
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
         gpPrint->SatScanPrintf("Reading input stream %u case file\n", t + 1);
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
void RankDataStreamHandler::SetPurelyTemporalSimulationData() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].SetPTSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","RankDataStreamHandler");
    throw;
  }
}

void RankDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), RankRandomizer());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","RankDataStreamHandler");
    throw;
  }
}


