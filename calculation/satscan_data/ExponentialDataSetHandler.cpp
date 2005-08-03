//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "ExponentialDataSetHandler.h"

const count_t ExponentialDataSetHandler::gtMinimumNotCensoredCases         = 1;

/** constructor */
ExponentialDataSetHandler::ExponentialDataSetHandler(CSaTScanData& Data, BasePrint& Print)
                          :DataSetHandler(Data, Print) {}

/** destructor */
ExponentialDataSetHandler::~ExponentialDataSetHandler() {}

/** allocates cases structures for dataset */
void ExponentialDataSetHandler::AllocateCaseStructures(size_t tSetIndex) {
  try {
    gvDataSets[tSetIndex]->AllocateCasesArray();
    gvDataSets[tSetIndex]->AllocateCensoredCasesArray();
    gvDataSets[tSetIndex]->AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","ExponentialDataSetHandler");
    throw;
  }
}

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t & ExponentialDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
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
        ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data */
AbstractDataSetGateway & ExponentialDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    DataGatway.Clear();
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
          ZdGenerateException("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","ExponentialDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & ExponentialDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    DataGatway.Clear();
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
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely spatial monotone analysis.","GetSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimulationDataGateway()","ExponentialDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Returns memory needed to allocate data set objects. */
double ExponentialDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
  double        dRequirements(0);

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
       //case array
       dRequirements = (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       //measure array
       dRequirements += (double)sizeof(measure_t*) * (double)(gDataHub.GetNumTimeIntervals()+1) +
                        (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(measure_t) * (double)gDataHub.GetNumTracts();
       break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
       //case array
       dRequirements = (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       //measure array
       dRequirements += (double)sizeof(measure_t*) * (double)(gDataHub.GetNumTimeIntervals()+1) +
                        (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(measure_t) * (double)gDataHub.GetNumTracts();
       if (gParameters.GetIncludePurelyTemporalClusters()) {
         //purely temporal case array
         dRequirements += (double)sizeof(count_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
         //purely temporal measure array
         dRequirements += (double)sizeof(measure_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
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
  return dRequirements * (double)sizeof(SimDataSet) * (double)GetNumDataSets();
}

/** Parses current file record contained in StringParser object in expected
    parts: location, case count, date, survival time and censor attribute. Returns true if no
    errors in data were found, else returns false and prints error messages to
    BasePrint object. */
bool ExponentialDataSetHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                                  count_t& nCount, Julian& nDate,
                                                  measure_t& tContinuosVariable, count_t& tCensorAttribute) {
  int   iContiVariableIndex, iCensoredAttributeIndex;

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
        gPrint.PrintInputWarning("Error: Case count in record %ld, of the %s, is negative.\n",
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
    iContiVariableIndex = gParameters.GetPrecisionOfTimesType() == NONE ? 2 : 3;
    if (!Parser.GetWord(iContiVariableIndex)) {
      gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing the continuos variable.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(iContiVariableIndex), "%lf", &tContinuosVariable) != 1) {
       gPrint.PrintInputWarning("Error: The continuos variable value '%s' in record %ld, of the %s, is not a number.\n",
                                Parser.GetWord(iContiVariableIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
    if (tContinuosVariable <= 0) {
       gPrint.PrintInputWarning("Error: The continuos variable '%lf' in record %ld of the %s, is not greater than zero.\n",
                                tContinuosVariable, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }

    //read and validate censore attribute
    iCensoredAttributeIndex = gParameters.GetPrecisionOfTimesType() == NONE ? 3 : 4;
    if (Parser.GetWord(iCensoredAttributeIndex) != 0) {
      if (!sscanf(Parser.GetWord(iCensoredAttributeIndex), "%ld", &tCensorAttribute) || tCensorAttribute < 0) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld, in the %s, could not be read as a censoring attribute.\n",
                                  Parser.GetWord(iCensoredAttributeIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       gPrint.PrintInputWarning("       Censoring attribute must be either 0 or 1.\n");
       return false;
      }
      //treat values greater than one as indication that patient is not censored
      tCensorAttribute = (tCensorAttribute > 1 ? 1 : tCensorAttribute);
    }
    else
      //censored attribute optional - default to not censored
      tCensorAttribute = 0;
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
bool ExponentialDataSetHandler::ReadCounts(size_t tSetIndex, FILE * fp, const char*) {
  bool                   bReadSuccessful=true, bEmpty=true;
  Julian                 Date;
  tract_t                tTractIndex;
  count_t                tPatients, tCensorAttribute, tTotalPopuation=0, tTotalCases=0;
  measure_t              tContinuosVariable, tTotalMeasure=0;
  ExponentialRandomizer* pRandomizer;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    StringParser Parser(gPrint);

    // if randomization data created by reading from file, we'll need to use temporary randomizer to create real data set
    pRandomizer = dynamic_cast<ExponentialRandomizer*>(gvDataSetRandomizers[tSetIndex]);         
    if (!pRandomizer)
      ZdGenerateException("Data set randomizer not ExponentialRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, tTractIndex, tPatients, Date, tContinuosVariable, tCensorAttribute)) {
             pRandomizer->AddPatients(tPatients, gDataHub.GetTimeIntervalOfDate(Date), tTractIndex, tContinuosVariable, tCensorAttribute);
             tTotalPopuation += tPatients;
             //check that addition did not exceed data type limitations
             if (tTotalPopuation < 0)
               GenerateResolvableException("Error: The total number of non-censored cases in dataset is greater than the maximum allowed of %ld.\n",
                                           "ReadCounts()", std::numeric_limits<count_t>::max());
             tTotalCases += tPatients * (tCensorAttribute ? 0 : 1);
             //check that addition did not exceed data type limitations
             if (tTotalCases < 0)
               GenerateResolvableException("Error: The total number of non-censored cases in dataset is greater than the maximum allowed of %ld.\n",
                                           "ReadCounts()", std::numeric_limits<count_t>::max());
             //check numeric limits of data type will not be exceeded
             if (tContinuosVariable > std::numeric_limits<measure_t>::max() - tTotalMeasure)
               GenerateResolvableException("Error: The total summation of survival times exceeds the maximum value allowed of %lf.\n",
                                           "ReadCounts()", std::numeric_limits<measure_t>::max());
             tTotalMeasure += tContinuosVariable;
           }
           else
             bReadSuccessful = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bReadSuccessful)
      gPrint.SatScanPrintWarning("Please see the 'case file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: %s does not contain data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bReadSuccessful = false;
    }
    //validate that data set contains minimum number of non-censored cases
    else if (tTotalCases < gtMinimumNotCensoredCases) {
      gPrint.SatScanPrintWarning("Error: Data set does not contain the required minimum of %i non-censored case%s.\n",
                                 gtMinimumNotCensoredCases, (gtMinimumNotCensoredCases == 1 ? "" : "s"));
      bReadSuccessful = false;
    }
    else {
      //calibrate measure -- multiply by (tTotalCases/tTotalMeasure)
      pRandomizer->Calibrate(tTotalCases/tTotalMeasure);
      //assign data accumulated in randomizer to data set case and measure arrays
      pRandomizer->Assign(DataSet.GetCaseArray(), DataSet.GetMeasureArray(), DataSet.GetNumTimeIntervals(), DataSet.GetNumTracts());
      //assign data accumulated in randomizer to data set censored case array
      pRandomizer->AssignCensoredIndividuals(DataSet.GetCensoredCasesArrayHandler());
      DataSet.SetTotalCases(tTotalCases); //total non-censored cases
      DataSet.SetTotalMeasure(tTotalMeasure); //total survival times
      DataSet.SetTotalPopulation(tTotalPopuation); //total censored and non-censored cases
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","ExponentialDataSetHandler");
    throw;
  }
  return bReadSuccessful;
}

/** Allocated data randomizer for each data set and reads case input data for each data set. */
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

/** Allocates randomizer object for each data set. */
void ExponentialDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers[0] = new ExponentialRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
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


