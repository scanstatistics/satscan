//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "ExponentialDataSetHandler.h"

/** constructor */
ExponentialDataSetHandler::ExponentialDataSetHandler(CSaTScanData& Data, BasePrint& Print)
                          :DataSetHandler(Data, Print) {}

/** destructor */
ExponentialDataSetHandler::~ExponentialDataSetHandler() {}

/** allocates cases structures for dataset */
void ExponentialDataSetHandler::AllocateCaseStructures(size_t tSetIndex) {
  try {
    gvDataSets[tSetIndex]->AllocateCasesArray();
    gvDataSets[tSetIndex]->AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","ExponentialDataSetHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataSetGateway * ExponentialDataSetHandler::GetNewDataGateway() const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(DataSet.GetTotalCases());
      Interface.SetTotalMeasureCount(DataSet.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(DataSet.GetPTMeasureArray());
          Interface.SetPTCaseArray(DataSet.GetPTCasesArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(DataSet.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewDataGateway()","ExponentialDataSetHandler");
    throw;
  }
  return pDataSetGateway;
}

/** returns new data gateway for simulation data */
AbtractDataSetGateway * ExponentialDataSetHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets[t];
      const SimDataSet& S_DataSet = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.GetTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(S_DataSet.GetPTCasesArray());
          Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(S_DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","ExponentialDataSetHandler");
    throw;
  }  
  return pDataSetGateway;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& ExponentialDataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataSets(); ++t)
    Container.push_back(new SimDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          Container[t]->AllocateMeasureArray();
        }
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          Container[t]->AllocatePTCasesArray();
          Container[t]->AllocateMeasureArray();
          Container[t]->AllocatePTMeasureArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          Container[t]->AllocateMeasureArray();
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Container[t]->AllocatePTCasesArray();
            Container[t]->AllocatePTMeasureArray();
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

bool ExponentialDataSetHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                                  count_t& nCount, Julian& nDate,
                                                  measure_t& tContinuosVariable, count_t& tCensored) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gPrint.PrintInputWarning("Error: Unknown location ID in the %s, record %ld.\n", gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      gPrint.PrintInputWarning("       Location ID '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n",
                                  Parser.GetWord(1), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       gPrint.PrintInputWarning("       Case count must be an integer.\n");
       return false;
      }
    }
    else {
      gPrint.PrintInputWarning("Error: Record %ld, in the %s, does not contain case count.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount <= 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gPrint.PrintInputWarning("Error: Case count in record %ld, of the %s, is not greater than zero.\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.PrintInputWarning("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;

    // read continuos variable
    if (!Parser.GetWord(3)) {
      gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing the continuos variable.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuosVariable) != 1) {
       gPrint.PrintInputWarning("Error: The continuos variable value '%s' in record %ld, of the %s, is not a number.\n",
                                Parser.GetWord(3), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
    if (tContinuosVariable <= 0) {
       gPrint.PrintInputWarning("Error: The continuos variable '%ld' in record %ld of the %s, is not greater than zero.\n",
                                tContinuosVariable, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
    //read and validate censore attribute
    if (Parser.GetWord(4) != 0) {
      if (!sscanf(Parser.GetWord(4), "%ld", &tCensored) || tCensored < 0) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld, in the %s, could not be read as a censoring attribute.\n",
                                  Parser.GetWord(4), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       gPrint.PrintInputWarning("       Censoring attribute must be either 0 or 1.\n");
       return false;
      }
    }
    else {
      gPrint.PrintInputWarning("Error: Record %ld, in the %s, does not contain censoring attibute.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","ExponentialDataSetHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool ExponentialDataSetHandler::ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i;
  count_t       Count, tCensored, ** ppCounts, tTotalCases=0;
  measure_t     tContinuosVariable, ** ppMeasure, ** ppSqMeasure, tTotalMeasure=0;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    StringParser Parser(gPrint);

    ppCounts = DataSet.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable, tCensored)) {
             if (gParameters.GetSimulationType() != FILESOURCE)
               for (i=0; i < Count; ++i)
                  ((ExponentialRandomizer*)gvDataSetRandomizers[tSetIndex])->AddCase(gDataHub.GetTimeIntervalOfDate(Date),
                                                                                     TractIndex, tContinuosVariable, tCensored);

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
      gPrint.SatScanPrintWarning("Please see the 'case file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: %s does not contain data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    else if (gParameters.GetSimulationType() != FILESOURCE) {
     //$$ calibrate measure -- multiply by (tTotalCases/tTotalMeasure)

     ((ExponentialRandomizer*)gvDataSetRandomizers[tSetIndex])->Assign(DataSet.GetCaseArray(),
                                                                     DataSet.GetMeasureArray(),
                                                                     DataSet.GetNumTimeIntervals(),
                                                                     DataSet.GetNumTracts());
     DataSet.SetTotalCases(tTotalCases);
     DataSet.SetTotalMeasure(tTotalMeasure);
    }

  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","ExponentialDataSetHandler");
    throw;
  }
  return bValid;
}


bool ExponentialDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (GetNumDataSets() == 1)
         gPrint.SatScanPrintf("Reading the case file\n");
       else
         gPrint.SatScanPrintf("Reading the cae file for data set %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","ExponentialDataSetHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void ExponentialDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t) {
       SimDataContainer[t]->SetPTCasesArray();
       SimDataContainer[t]->SetPTMeasureArray();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","ExponentialDataSetHandler");
    throw;
 }
}

void ExponentialDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers[0] = new ExponentialRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters, gParameters.GetRandomizationSeed());
          break;
      case HA_RANDOMIZATION :
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers[t] = gvDataSetRandomizers[0]->Clone();

  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","ExponentialDataSetHandler");
    throw;
  }
}


