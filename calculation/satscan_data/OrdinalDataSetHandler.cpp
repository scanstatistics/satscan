//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "OrdinalDataSetHandler.h"

const size_t OrdinalDataSetHandler::gtMinimumCategories        = 3;

/** constructor */
OrdinalDataSetHandler::OrdinalDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                         :DataSetHandler(DataHub, Print) {}

/** destructor */
OrdinalDataSetHandler::~OrdinalDataSetHandler() {}

/** empty function - data set case structures allocated as new ordinal categories
    are encountered in read of case file */
void OrdinalDataSetHandler::AllocateCaseStructures(size_t) {/*no action*/}

/** returns new data gateway for real data - caller resonsible for deletion object */
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

/** returns new data gateway for simulation data - caller resonsible for deletion object */
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

/** Parses current file record contained in StringParser object in expected
    parts: location, case count, date and ordinal category. Returns true if no
    errors in data were found, else returns false and prints error messages to
    BasePrint object. */
bool OrdinalDataSetHandler::ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuosVariable) {
  int   iCategoryIndex;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(guLocationIndex))) == -1) {
      gPrint.PrintInputWarning("Error: Unknown location ID in the %s, record %ld.\n", gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      gPrint.PrintInputWarning("       Location ID '%s' was not specified in the coordinates file.\n", Parser.GetWord(guLocationIndex));
      return false;
    }
    //read case count
    if (Parser.GetWord(guCountIndex) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n",
                                  Parser.GetWord(guCountIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
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
                                   Parser.GetWord(guCountIndex), std::numeric_limits<count_t>::max(),
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    // read date
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;
    // read ordinal category
    iCategoryIndex = gParameters.GetPrecisionOfTimesType() == NONE ? guCountCategoryIndexNone : guCountCategoryIndex;
    if (!Parser.GetWord(iCategoryIndex)) {
      gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing ordinal data field.\n",
                               Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(iCategoryIndex), "%lf", &tContinuosVariable) != 1) {
       gPrint.PrintInputWarning("Error: The ordinal data '%s' in record %ld, of the %s, is not a number.\n",
                                  Parser.GetWord(iCategoryIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
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
  bool          bReadSuccessful=true, bEmpty=true;
  Julian        Date;
  tract_t       tLocationIndex;
  int           i, iDateIndex;
  count_t       tCount, tTotalCases=0, ** ppCategoryCounts;
  measure_t     tOrdinalVariable;
  size_t        tOrdinalCategoryIndex;

  try {
    //get reference to RealDataSet object at index
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    //instanciate file record parser
    StringParser Parser(gPrint);

    //read, parse, validate and update data structures for each record in data file
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) { // ignore records which contain no data
           bEmpty = false;
           //parse record into parts: location index, # of cases, date, ordinal catgory
           if (ParseCaseFileLine(Parser, tLocationIndex, tCount, Date, tOrdinalVariable)) {
             if (tCount > 0) { //ignore records with zero cases
               //add count to cumulative total
               tTotalCases += tCount;
               //check that addition did not exceed data type limitations
               if (tTotalCases < 0)
                 GenerateResolvableException("Error: The total cases in dataset is greater than the maximum allowed of %ld.\n",
                                             "ReadCounts()", std::numeric_limits<count_t>::max());
               //get calculated date index from read date
               iDateIndex = gDataHub.GetTimeIntervalOfDate(Date);
               //record count and get category's 2-D array pointer
               ppCategoryCounts = DataSet.AddOrdinalCategoryCaseCount(tOrdinalVariable, tCount);
               //update location case counts such that 'tCount' is reprented cumulatively through
               //time from start date through specifed date in record 
               ppCategoryCounts[0][tLocationIndex] += tCount;
               for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                  ppCategoryCounts[i][tLocationIndex] += tCount;
             }
           }
           else //denote that read process encountered an error, but keep reading records
             bReadSuccessful = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (!bReadSuccessful)
      gPrint.SatScanPrintWarning("Please see the 'case file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: %s does not contain data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bReadSuccessful = false;
    }
    //validate that input data contained minimum number of ordinal categories
    else if (DataSet.GetPopulationData().GetNumOrdinalCategories() < gtMinimumCategories) {
      gPrint.SatScanPrintWarning("Error: Data set contains %i categories but a minimum of %i categories\n"
                                 "       are required for the ordinal probabililty model.\n",
                                 DataSet.GetPopulationData().GetNumOrdinalCategories(), gtMinimumCategories);
      bReadSuccessful = false;
    }
    //else if (data set does not contain minimum number of cases either in total or in each category) {
    //  gPrint.SatScanPrintWarning("Error: \n");
    //  bReadSuccessful = false;
    //}
    //record total cases and total population to data set object
    else {
      DataSet.SetTotalCases(tTotalCases);
      DataSet.SetTotalPopulation(tTotalCases);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","OrdinalDataSetHandler");
    throw;
  }
  return bReadSuccessful;
}

/** Read data that is particular (case file) to 'Ordinal' model into data set(s). */
bool OrdinalDataSetHandler::ReadData() {
  try {
    //allocate randomizers objects - one for each data set
    SetRandomizers();
    //read case file of each data set
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

/** Instanciates randomizer object for each data set. Currently there are two
    possible randomizers: OrdinalDenominatorDataRandomizer and FileSourceRandomizer */
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


