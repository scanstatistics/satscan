//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "ExponentialDataSetHandler.h"
#include "SSException.h"
#include "DataSource.h" 

const count_t ExponentialDataSetHandler::gtMinimumNotCensoredCases         = 1;

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model) and analysis type. */
SimulationDataContainer_t & ExponentialDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateCasesArray();
                                       (*itr)->AllocateMeasureArray();
                                     }
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocatePTCasesArray();
                                       (*itr)->AllocatePTMeasureArray();
                                     }
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateCasesArray();
                                       (*itr)->AllocateMeasureArray();
                                       if (gParameters.GetIncludePurelyTemporalClusters()) {
                                         (*itr)->AllocatePTCasesArray();
                                         (*itr)->AllocatePTMeasureArray();
                                       }  
                                     }
                                     break;
    case SPATIALVARTEMPTREND       :
       ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
       ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data */
AbstractDataSetGateway & ExponentialDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets.at(t);
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
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const SimDataSet& S_DataSet = *Container.at(t);
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

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool ExponentialDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source, const char*) {
  bool                                  bReadSuccessful=true, bEmpty=true;
  Julian                                Date;
  tract_t                               tTractIndex;
  count_t                               tPatients, tCensorAttribute, tTotalPopuation=0, tTotalCases=0;
  measure_t                             tContinuousVariable, tTotalMeasure=0;
  AbstractExponentialRandomizer       * pRandomizer;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    // if randomization data created by reading from file, we'll need to use temporary randomizer to create real data set
    pRandomizer = dynamic_cast<AbstractExponentialRandomizer*>(gvDataSetRandomizers.at(DataSet.GetSetIndex() - 1));
    if (!pRandomizer)
      ZdGenerateException("Data set randomizer not AbstractExponentialRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(Source, tTractIndex, tPatients, Date, tContinuousVariable, tCensorAttribute);
           if (eRecordStatus == DataSetHandler::Accepted) {
             bEmpty = false;
             pRandomizer->AddPatients(tPatients, gDataHub.GetTimeIntervalOfDate(Date), tTractIndex, tContinuousVariable, tCensorAttribute);
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
             if (tContinuousVariable * tPatients > std::numeric_limits<measure_t>::max() - tTotalMeasure)
               GenerateResolvableException("Error: The total summation of survival times exceeds the maximum value allowed of %lf.\n",
                                           "ReadCounts()", std::numeric_limits<measure_t>::max());
             tTotalMeasure += tContinuousVariable * tPatients;
           }
           else if (eRecordStatus == DataSetHandler::Ignored)
             continue;
           else
             bReadSuccessful = false;
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bReadSuccessful)
      gPrint.Printf("Please see the 'case file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bReadSuccessful = false;
    }
    //validate that data set contains minimum number of non-censored cases
    else if (tTotalCases < gtMinimumNotCensoredCases) {
      gPrint.Printf("Error: Data set does not contain the required minimum of %i non-censored case%s.\n",
                    BasePrint::P_ERROR, gtMinimumNotCensoredCases, (gtMinimumNotCensoredCases == 1 ? "" : "s"));
      bReadSuccessful = false;
    }
    else {
      pRandomizer->AssignFromAttributes(tTotalCases, tTotalMeasure, DataSet);
      DataSet.SetTotalPopulation(tTotalPopuation); //total censored and non-censored cases
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","ExponentialDataSetHandler");
    throw;
  }
  return bReadSuccessful;
}

/** Attempts to read case data file into class RealDataSet objects. */
bool ExponentialDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (GetNumDataSets() == 1)
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (!ReadCaseFile(GetDataSet(t)))
         return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","ExponentialDataSetHandler");
    throw;
  }
  return true;
}

/** Parses current file record contained in DataSource object in expected
    parts: location, # patients, date, survival time and censor attribute. Returns
    DataSetHandler::Accepted if no errors in data were found, else prints error
    messages to BasePrint object and returns RecordStatusType. */
DataSetHandler::RecordStatusType ExponentialDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& tPatients, Julian& nDate, measure_t& tContinuousVariable, count_t& tCensorAttribute) {
  short   iContiVariableIndex, iCensoredAttributeIndex;

  try {
    //read and validate that tract identifier exists in coordinates file
    DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(Source, tid);
    if (eStatus != DataSetHandler::Accepted) return eStatus;
    //read and validate count
    if (Source.GetValueAt(guCountIndex) != 0) {
      if (!sscanf(Source.GetValueAt(guCountIndex), "%ld", &tPatients)) {
       gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                     "       Case count must be an integer.\n", BasePrint::P_READERROR, Source.GetValueAt(guCountIndex),
                     Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld, in the %s, does not contain case count.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (tPatients < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Source.GetValueAt(guCountIndex), "-"))
        gPrint.Printf("Error: Case count in record %ld, of the %s, is negative.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Source.GetValueAt(guCountIndex), std::numeric_limits<count_t>::max(),
                      Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (tPatients == 0) return DataSetHandler::Ignored;    
    DataSetHandler::RecordStatusType eDateStatus = RetrieveCountDate(Source, nDate);
    if (eDateStatus != DataSetHandler::Accepted)
      return eDateStatus;

    // read continuous variable
    iContiVariableIndex = gParameters.GetPrecisionOfTimesType() == NONE ? (short)2 : (short)3;
    if (!Source.GetValueAt(iContiVariableIndex)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing the survival time.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (sscanf(Source.GetValueAt(iContiVariableIndex), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The survival time value '%s' in record %ld, of the %s, is not a number.\n",
                                BasePrint::P_READERROR, Source.GetValueAt(iContiVariableIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }
    if (tContinuousVariable <= 0) {
       gPrint.Printf("Error: The survival time '%g' in record %ld of the %s, is not greater than zero.\n",
                     BasePrint::P_READERROR, tContinuousVariable, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }

    //read and validate censore attribute
    iCensoredAttributeIndex = gParameters.GetPrecisionOfTimesType() == NONE ? (short)3 : (short)4;
    if (Source.GetValueAt(iCensoredAttributeIndex) != 0) {
      if (!sscanf(Source.GetValueAt(iCensoredAttributeIndex), "%ld", &tCensorAttribute) || !(tCensorAttribute == 0 || tCensorAttribute == 1)) {
       gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as a censoring attribute.\n"
                     "       Censoring attribute must be either 0 or 1.\n", BasePrint::P_READERROR,
                     Source.GetValueAt(iCensoredAttributeIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
      }
      //treat values greater than one as indication that patient was censored
      tCensorAttribute = (tCensorAttribute > 1 ? 1 : tCensorAttribute);
    }
    else
      //censored attribute optional - default to not censored
      tCensorAttribute = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("RetrieveCaseRecordData()","ExponentialDataSetHandler");
    throw;
  }
  return DataSetHandler::Accepted;
}

/** sets purely temporal structures used in simulations */
void ExponentialDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  try {
    for (; itr != itr_end; ++itr) {
       (*itr)->SetPTCasesArray();
       (*itr)->SetPTMeasureArray();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","ExponentialDataSetHandler");
    throw;
 }
}

/** Allocates randomizers for each dataset. There are currently 2 randomization types
    for the Exponential model: null hypothesis and  purely temporal optimized null hypothesis. */
void ExponentialDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers.at(0) = new ExponentialPurelyTemporalRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers.at(0) = new ExponentialRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
      case HA_RANDOMIZATION :
      default : ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","ExponentialDataSetHandler");
    throw;
  }
}


