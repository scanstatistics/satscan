//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "SurvivalDataStreamHandler.h"

/** constructor */
SurvivalDataStreamHandler::SurvivalDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                          :DataStreamHandler(Data, pPrint) {}

/** destructor */
SurvivalDataStreamHandler::~SurvivalDataStreamHandler() {}

/** allocates cases structures for stream*/
void SurvivalDataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream].AllocateCasesArray();
    gvDataStreams[iStream].AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","SurvivalDataStreamHandler");
    throw;
  }
}

void SurvivalDataStreamHandler::AllocatePTSimulationMeasures() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocatePTSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTSimulationMeasures()","SurvivalDataStreamHandler");
    throw;
  }
}

/** Allocates two dimensional array for expected case counts (number of time intervals by number of tracts). */
void SurvivalDataStreamHandler::AllocateSimulationMeasures() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationMeasure()","SurvivalDataStreamHandler");
    throw;
  }
}

/** allocates structures used during simulations - based particularly upon analysis type */
void SurvivalDataStreamHandler::AllocateSimulationStructures() {
  try {
    switch (gParameters.GetAnalysisType()) {
       case PURELYSPATIAL :
         AllocateSimulationCases();
         AllocateSimulationMeasures();
         break;
       case PURELYSPATIALMONOTONE :
         ZdGenerateException("AllocateSimulationStructures() not implemented for purely spatial monotone analysis.","AllocateSimulationStructures()");
       case PURELYTEMPORAL :
       case PROSPECTIVEPURELYTEMPORAL :
         AllocateSimulationCases();
         AllocatePTSimulationCases();
         AllocateSimulationMeasures();
         AllocatePTSimulationMeasures();
         break;
       case SPACETIME :
       case PROSPECTIVESPACETIME :
         AllocateSimulationCases();
         AllocateSimulationMeasures();
         if (gParameters.GetIncludePurelyTemporalClusters()) {
           AllocatePTSimulationMeasures();
           AllocatePTSimulationCases();
         }
         break;
       case SPATIALVARTEMPTREND :
         ZdGenerateException("AllocateSimulationStructures() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationStructures()");
      default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationStructures()", gParameters.GetAnalysisType());
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","SurvivalDataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataStreamGateway * SurvivalDataStreamHandler::GetNewDataGateway() {
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
    x.AddCallpath("GetNewDataGateway()","SurvivalDataStreamHandler");
    throw;
  }
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * SurvivalDataStreamHandler::GetNewSimulationDataGateway() {
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
          Interface.SetMeasureArray(thisStream.GetSimMeasureArray());
          break;
        case PURELYSPATIALMONOTONE      :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
          Interface.SetPTMeasureArray(thisStream.GetPTSimMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetMeasureArray(thisStream.GetSimMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
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
    x.AddCallpath("GetNewSimulationDataGateway()","SurvivalDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

bool SurvivalDataStreamHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                                  count_t& nCount, Julian& nDate,
                                                  measure_t& tContinuosVariable, count_t& tCensored) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gData.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gpPrint->PrintInputWarning("Error: Unknown location id in case file, record %ld.\n", Parser.GetReadCount());
      gpPrint->PrintInputWarning("       Location '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in case file could not be read as count.\n", Parser.GetWord(1), Parser.GetReadCount());
       gpPrint->PrintInputWarning("       Count must be an integer.\n");
       return false;
      }
    }
    else {
      gpPrint->PrintInputWarning("Error: Record %ld in case file does not contain case count.\n", Parser.GetReadCount());
      return false;
    }
    if (nCount <= 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gpPrint->PrintInputWarning("Error: Count in record %ld of case file, is not greater than zero.\n", Parser.GetReadCount());
      else
        gpPrint->PrintInputWarning("Error: Count '%s' exceeds maximum value of %ld in record %ld of case file.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(), Parser.GetReadCount());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, "case", nDate))
      return false;

    // read continuos variable
    if (!Parser.GetWord(3)) {
      gpPrint->PrintInputWarning("Error: Record %d of case file missing continuos variable.\n", Parser.GetReadCount());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuosVariable) != 1) {
       gpPrint->PrintInputWarning("Error: Continuos variable value '%s' in record %ld, of case file, is not a number.\n",
                                  Parser.GetWord(3), Parser.GetReadCount());
       return false;
    }
    //read and validate censore attribute
    if (Parser.GetWord(4) != 0) {
      if (!sscanf(Parser.GetWord(4), "%ld", &tCensored) || tCensored < 0) {
       gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in case file could not be read as censored attribute.\n", Parser.GetWord(4), Parser.GetReadCount());
       gpPrint->PrintInputWarning("       Censored attribute must be an integer in range [0 - 1].\n");
       return false;
      }
    }
    else {
      gpPrint->PrintInputWarning("Error: Record %ld in case file does not contain censored attibute.\n", Parser.GetReadCount());
      return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","SurvivalDataStreamHandler");
    throw;
  }
  return true;
}

/** randomizes each data streams */
void SurvivalDataStreamHandler::RandomizeData(unsigned int iSimulationNumber) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     gvDataStreamRandomizers[t].RandomizeData(gvDataStreams[t], iSimulationNumber);
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SurvivalDataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i;
  count_t       Count, tCensored, ** ppCounts, tTotalCases=0;
  measure_t     tContinuosVariable, ** ppMeasure, ** ppSqMeasure, tTotalMeasure=0;

  try {
    DataStream & thisStream = gvDataStreams[tStream];
    StringParser Parser(gpPrint->GetImpliedInputFileType());
    SurvivalRandomizer & Randomizer = gvDataStreamRandomizers[tStream];

    ppCounts = thisStream.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable, tCensored)) {
             ////cumulatively add count to time by location structure
             //ppCounts[0][TractIndex] += Count * tCensored;
             ///if (ppCounts[0][TractIndex] < 0)
             // SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "ReadCounts()",
             //                      std::numeric_limits<count_t>::max());
             //for (i=1; Date >= gData.GetTimeIntervalStartTimes()[i]; ++i)
             //  ppCounts[i][TractIndex] += Count * tCensored;
             //record count as a case or control
//           //  thisStream.GetPopulationData().AddCaseCount(0, Count);
             for (i=0; i < Count; ++i)
                Randomizer.AddCase(gData.GetTimeIntervalOfDate(Date), TractIndex, tContinuosVariable, tCensored);

             tTotalCases += Count * tCensored;
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
      gpPrint->SatScanPrintWarning("Error: Case file does not contain data.\n");
      bValid = false;
    }
    else {
     Randomizer.Assign(thisStream.GetCaseArray(), thisStream.GetMeasureArray(),
                       thisStream.GetNumTimeIntervals(), thisStream.GetNumTracts());
     thisStream.SetTotalCases(tTotalCases);
     thisStream.SetTotalMeasure(tTotalMeasure);
    }

  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","SurvivalDataStreamHandler");
    throw;
  }
  return bValid;
}


bool SurvivalDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","SurvivalDataStreamHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void SurvivalDataStreamHandler::SetPurelyTemporalSimulationData() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t) {
       gvDataStreams[t].SetPTSimCasesArray();
       gvDataStreams[t].SetPTSimMeasureArray();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","SurvivalDataStreamHandler");
    throw;
  }
}

void SurvivalDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), SurvivalRandomizer());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","SurvivalDataStreamHandler");
    throw;
  }
}


