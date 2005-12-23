//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "OrdinalDataSetHandler.h"
#include "SSException.h" 

const size_t OrdinalDataSetHandler::gtMinimumCategories        = 3;
const count_t OrdinalDataSetHandler::gtMinimumCases            = 4;

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t & OrdinalDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             :
       for (; itr != itr_end; ++itr)
         (*itr)->AllocateCategoryCasesArray(GetDataSet(std::distance(Container.begin(), itr)).GetPopulationData().GetNumOrdinalCategories());
       break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL :
       for (; itr != itr_end; ++itr)
         (*itr)->AllocatePTCategoryCasesArray(GetDataSet(std::distance(Container.begin(), itr)).GetPopulationData().GetNumOrdinalCategories());
       break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      :
       for (; itr != itr_end; ++itr) {
         (*itr)->AllocateCategoryCasesArray(GetDataSet(std::distance(Container.begin(), itr)).GetPopulationData().GetNumOrdinalCategories());
         if (gParameters.GetIncludePurelyTemporalClusters())
           (*itr)->AllocatePTCategoryCasesArray(GetDataSet(std::distance(Container.begin(), itr)).GetPopulationData().GetNumOrdinalCategories());
       }
       break;
    case SPATIALVARTEMPTREND       :
       ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
       ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data - caller resonsible for deletion object */
AbstractDataSetGateway & OrdinalDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
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
          Interface.SetPTCategoryCaseArray(DataSet.GetPTCategoryCasesArrayHandler().GetArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(DataSet.GetCasesByCategory());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(DataSet.GetPTCategoryCasesArrayHandler().GetArray());
          break;
       case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** returns new data gateway for simulation data - caller resonsible for deletion object */
AbstractDataSetGateway & OrdinalDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
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
          Interface.SetPTCategoryCaseArray(S_DataSet.GetPTCategoryCasesArrayHandler().GetArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(S_DataSet.GetCasesByCategory());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(S_DataSet.GetPTCategoryCasesArrayHandler().GetArray());
         break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely spatial monotone analysis.","GetSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimulationDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Returns memory needed to allocate data set objects. */
double OrdinalDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
  double        dRequirements(0);

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
       for (size_t t=0; t < GetNumDataSets(); ++t)
          for (unsigned int i=0; i < GetDataSet(t).GetPopulationData().GetNumOrdinalCategories(); ++i) {
            //case array for ordinal category
            dRequirements += (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                             (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       }
       break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
       for (size_t t=0; t < GetNumDataSets(); ++t) {
          const RealDataSet& Set = GetDataSet(t);
          for (unsigned int i=0; i < Set.GetPopulationData().GetNumOrdinalCategories(); ++i)
             //case array for ordinal category
             dRequirements += (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                              (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
          if (gParameters.GetIncludePurelyTemporalClusters()) {
             //purely temporal case array
             dRequirements += (double)sizeof(count_t*) * (double)Set.GetPopulationData().GetNumOrdinalCategories() +
                              (double)sizeof(count_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
          }
       }
       break;
    case PROSPECTIVEPURELYTEMPORAL :
    case PURELYTEMPORAL :
       //purely temporal analyses not of interest
       break;
    case SPATIALVARTEMPTREND :
       //svtt analysis not of interest at this time
       break;
     default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataSetAllocationRequirements()",gParameters.GetAnalysisType());
  };
  return dRequirements + (double)sizeof(SimDataSet) * (double)GetNumDataSets();
}

/** Parses current file record contained in StringParser object in expected
    parts: location, case count, date and ordinal category. Returns true if no
    errors in data were found, else returns false and prints error messages to
    BasePrint object. */
bool OrdinalDataSetHandler::ParseCaseFileLine(StringParser& Parser, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable) {
  short   iCategoryIndex;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(guLocationIndex))) == -1) {
      gPrint.Printf("Error: Unknown location ID in the %s, record %ld.\n"
                    "       Location ID '%s' was not specified in the coordinates file.\n",
                    BasePrint::P_READERROR, gPrint.GetImpliedFileTypeString().c_str(),
                    Parser.GetReadCount(), Parser.GetWord(guLocationIndex));
      return false;
    }
    //read case count
    if (Parser.GetWord(guCountIndex) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                    "       Case count must be an integer.\n", BasePrint::P_READERROR,
                    Parser.GetWord(guCountIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld, in the %s, does not contain case count.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gPrint.Printf("Error: Case count in record %ld, of the %s, is not greater than zero.\n",
                      BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Parser.GetWord(guCountIndex), std::numeric_limits<count_t>::max(),
                      Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    // read date
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;
    // read ordinal category
    iCategoryIndex = gParameters.GetPrecisionOfTimesType() == NONE ? guCountCategoryIndexNone : guCountCategoryIndex;
    if (!Parser.GetWord(iCategoryIndex)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing ordinal data field.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(iCategoryIndex), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The ordinal data '%s' in record %ld, of the %s, is not a number.\n",
                     BasePrint::P_READERROR, Parser.GetWord(iCategoryIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
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
  bool                  bReadSuccessful=true, bEmpty=true;
  Julian                Date;
  tract_t               tLocationIndex;
  count_t               tCount, tTotalCases=0, ** ppCategoryCounts;
  measure_t             tOrdinalVariable;
  std::vector<double>   vReadCategories;

  try {
    //get reference to RealDataSet object at index
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    //instanciate file record parser
    StringParser Parser(gPrint);

    //read, parse, validate and update data structures for each record in data file
    while (!gPrint.GetMaximumReadErrorsPrinted() && Parser.ReadString(fp)) {
         if (Parser.HasWords()) { // ignore records which contain no data
           bEmpty = false;
           //parse record into parts: location index, # of cases, date, ordinal catgory
           if (ParseCaseFileLine(Parser, tLocationIndex, tCount, Date, tOrdinalVariable)) {
             //note each category read from file. since we are ignoring records with zero cases,
             //we might need this information for error reporting
             if (vReadCategories.end() == std::find(vReadCategories.begin(), vReadCategories.end(), tOrdinalVariable))
               vReadCategories.push_back(tOrdinalVariable);
             if (tCount > 0) { //ignore records with zero cases
               //add count to cumulative total
               tTotalCases += tCount;
               //check that addition did not exceed data type limitations
               if (tTotalCases < 0)
                 GenerateResolvableException("Error: The total cases in dataset is greater than the maximum allowed of %ld.\n",
                                             "ReadCounts()", std::numeric_limits<count_t>::max());
               //record count and get category's 2-D array pointer
               ppCategoryCounts = DataSet.AddOrdinalCategoryCaseCount(tOrdinalVariable, tCount);
               //update location case counts such that 'tCount' is reprented cumulatively through
               //time from start date through specifed date in record
               ppCategoryCounts[0][tLocationIndex] += tCount;
               for (int i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
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
      gPrint.Printf("Please see the 'case file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bReadSuccessful = false;
    }
    //validate that input data contained minimum number of ordinal categories
    else if (DataSet.GetPopulationData().GetNumOrdinalCategories() < gtMinimumCategories) {
      if (DataSet.GetPopulationData().GetNumOrdinalCategories() == vReadCategories.size()) {
        gPrint.Printf("Error: Data set case file specifies %i categories with cases but a minimum\n"
                      "       of %i categories is required.\n", BasePrint::P_ERROR,
                      DataSet.GetPopulationData().GetNumOrdinalCategories(), gtMinimumCategories);
        bReadSuccessful = false;
      }
      else {
        gPrint.Printf("Error: The number of categories with cases is required to be a mimumum of %i.\n"
                      "       Data set case file specifies %i categories with %i of them containing no cases.\n",
                      BasePrint::P_ERROR, gtMinimumCategories, vReadCategories.size(),
                      vReadCategories.size() - DataSet.GetPopulationData().GetNumOrdinalCategories());
        bReadSuccessful = false;
      }
    }
    //validate that data set contains at least minimum number of cases
    else if (tTotalCases < gtMinimumCases) {
      gPrint.Printf("Error: Data set contains %i cases but a minimum of %i cases is required for ordinal data.\n",
                    BasePrint::P_ERROR, tTotalCases, gtMinimumCases);
      bReadSuccessful = false;
    }
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
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
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
void OrdinalDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  try {
    for (; itr != itr_end; ++itr)
       (*itr)->SetPTCategoryCasesArray();
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
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers[0] = new OrdinalPurelyTemporalDenominatorDataRandomizer(gParameters.GetRandomizationSeed());
          else
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


