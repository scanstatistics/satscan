//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "OrdinalDataSetHandler.h"

/** constructor */
OrdinalDataSetHandler::OrdinalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                         :DataSetHandler(DataHub, Print) {}

/** destructor */
OrdinalDataSetHandler::~OrdinalDataSetHandler() {}

/** allocates cases structures for dataset */
void OrdinalDataSetHandler::AllocateCaseStructures(size_t) {
  //no action
}

/** returns new data gateway for real data */
AbtractDataSetGateway * OrdinalDataSetHandler::GetNewDataGateway() const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets[t];
      //set total cases
      Interface.SetTotalCasesCount(DataSet.GetTotalCases());
      Interface.SetNumOrdinalCategories(DataSet.GetPopulationData().GetNumOrdinalCategories());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCategoryCaseArrays(DataSet.GetCasesByCategory());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
       case PURELYTEMPORAL             :
          Interface.SetPTCategoryCaseArray(DataSet.GetPTCategoryCasesArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(DataSet.GetCasesByCategory());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(DataSet.GetPTCategoryCasesArray());
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
    x.AddCallpath("GetNewDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return pDataSetGateway;
}

/** returns new data gateway for simulation data */
AbtractDataSetGateway * OrdinalDataSetHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets[t];
      const SimDataSet& S_DataSet = *Container[t];
      //set total cases
      Interface.SetTotalCasesCount(R_DataSet.GetTotalCases());
      Interface.SetNumOrdinalCategories(R_DataSet.GetPopulationData().GetNumOrdinalCategories());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCategoryCaseArrays(S_DataSet.GetCasesByCategory());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCategoryCaseArray(S_DataSet.GetPTCategoryCasesArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(S_DataSet.GetCasesByCategory());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(S_DataSet.GetPTCategoryCasesArray());
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
    x.AddCallpath("GetNewSimulationDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return pDataSetGateway;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& OrdinalDataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataSets(); ++t)
    Container.push_back(new SimDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCategoryCasesArray(gDataHub.GetDataSetHandler().GetDataSet(t).GetPopulationData().GetNumOrdinalCategories());
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCategoryCasesArray(gDataHub.GetDataSetHandler().GetDataSet(t).GetPopulationData().GetNumOrdinalCategories());
          Container[t]->AllocatePTCategoryCasesArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCategoryCasesArray(gDataHub.GetDataSetHandler().GetDataSet(t).GetPopulationData().GetNumOrdinalCategories());
          if (gParameters.GetIncludePurelyTemporalClusters())
             Container[t]->AllocatePTCategoryCasesArray();
        }
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

bool OrdinalDataSetHandler::ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable) {
  int   iSurvivalTimeIndex;

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
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
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
    iSurvivalTimeIndex = gParameters.GetPrecisionOfTimesType() == NONE ? 2 : 3;
    if (!Parser.GetWord(iSurvivalTimeIndex)) {
      gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing ordinal data field.\n",
                               Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(iSurvivalTimeIndex), "%lf", &tContinuosVariable) != 1) {
       gPrint.PrintInputWarning("Error: The ordinal data '%s' in record %ld, of the %s, is not a number.\n",
                                  Parser.GetWord(iSurvivalTimeIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","OrdinalDataSetHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool OrdinalDataSetHandler::ReadCounts(size_t tSetIndex, FILE * fp, const char*) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i, iDateIndex;
  count_t       Count, tTotalCases=0, ** ppCategoryCounts;
  measure_t     tContinuosVariable;
  size_t        tOrdinalCategoryIndex;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    StringParser Parser(gPrint);

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuosVariable)) {
             if (Count) { //ignore records with zero cases
               //record cases to randomizer object
               iDateIndex = gDataHub.GetTimeIntervalOfDate(Date);
               //record count and get index of ordinal category
               ppCategoryCounts = DataSet.AddOrdinalCategoryCaseCount(tContinuosVariable, Count);
               ppCategoryCounts[0][TractIndex] += Count;
               if (ppCategoryCounts[0][TractIndex] < 0)
                 GenerateResolvableException("Error: The total number of cases, in data set %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                             tSetIndex, std::numeric_limits<count_t>::max());
               for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                  ppCategoryCounts[i][TractIndex] += Count;
               tTotalCases += Count;
             }
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
    else {
      DataSet.SetTotalCases(tTotalCases);
      DataSet.SetTotalPopulation(tTotalCases);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","OrdinalDataRandomizer");
    throw;
  }
  return bValid;
}

/** Read data that is particular (case file) to 'Ordinal' model into data set(s). */
bool OrdinalDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (GetNumDataSets() == 1)
         gPrint.SatScanPrintf("Reading the case file\n");
       else
         gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","OrdinalDataSetHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void OrdinalDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t)
       SimDataContainer[t]->SetPTCategoryCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","OrdinalDataSetHandler");
    throw;
 }
}

/** Instanciates OrdinalDataRandomizer for each data set. */
void OrdinalDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers[0] = new OrdinalDenominatorDataRandomizer(gParameters.GetRandomizationSeed());
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
    x.AddCallpath("SetRandomizers()","OrdinalDataSetHandler");
    throw;
  }
}


