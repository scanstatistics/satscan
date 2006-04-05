//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "NormalDataSetHandler.h"
#include "DataSource.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    needed by data set handler (probability model) and analysis type during evaluation
    of simulated data. */
SimulationDataContainer_t & NormalDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();
  
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateMeasureArray();
                                       (*itr)->AllocateSqMeasureArray();
                                     }
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocatePTMeasureArray();
                                       (*itr)->AllocatePTSqMeasureArray();
                                     }
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateMeasureArray();
                                       (*itr)->AllocateSqMeasureArray();
                                       if (gParameters.GetIncludePurelyTemporalClusters()) {
                                         (*itr)->AllocatePTMeasureArray();
                                         (*itr)->AllocatePTSqMeasureArray();
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

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in internal data set collection. */
AbstractDataSetGateway & NormalDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
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
          Interface.SetSqMeasureArray(DataSet.GetSqMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(DataSet.GetPTMeasureArray());
          Interface.SetPTCaseArray(DataSet.GetPTCasesArray());
          Interface.SetPTSqMeasureArray(DataSet.GetPTSqMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
          Interface.SetSqMeasureArray(DataSet.GetSqMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(DataSet.GetPTMeasureArray());
            Interface.SetPTSqMeasureArray(DataSet.GetPTSqMeasureArray());
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
    x.AddCallpath("GetDataGateway()","NormalDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in passed simulation data collection. */
AbstractDataSetGateway & NormalDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
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
          Interface.SetCaseArray(R_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          Interface.SetSqMeasureArray(S_DataSet.GetSqMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(R_DataSet.GetPTCasesArray());
          Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
          Interface.SetPTSqMeasureArray(S_DataSet.GetPTSqMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(R_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          Interface.SetSqMeasureArray(S_DataSet.GetSqMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(R_DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
            Interface.SetPTSqMeasureArray(S_DataSet.GetPTSqMeasureArray());
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
    x.AddCallpath("GetSimulationDataGateway()","NormalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool NormalDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source, const char* szDescription) {
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  count_t                               Count, tTotalCases=0;
  measure_t                             tContinuousVariable, tTotalMeasure=0;
  AbstractNormalRandomizer            * pRandomizer;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gvDataSetRandomizers.at(DataSet.GetSetIndex() - 1))) == 0)
      ZdGenerateException("Data set randomizer not AbstractNormalRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tContinuousVariable);
           if (eRecordStatus == DataSetHandler::Accepted) {
             bEmpty = false;
             pRandomizer->AddCase(Count, gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tContinuousVariable);
             tTotalCases += Count;
             tTotalMeasure += tContinuousVariable;
           }
           else if (eRecordStatus == DataSetHandler::Ignored)
             continue;
           else
             bValid = false;
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'case file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    else {
      pRandomizer->AssignFromAttributes(DataSet);
      DataSet.SetTotalCases(tTotalCases);
      DataSet.SetTotalMeasure(tTotalMeasure);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","NormalDataSetHandler");
    throw;
  }
  return bValid;
}

/** Attempts to read case data file into class RealDataSet objects. */
bool NormalDataSetHandler::ReadData() {
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
    x.AddCallpath("ReadData()","NormalDataSetHandler");
    throw;
  }
  return true;
}

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
DataSetHandler::RecordStatusType NormalDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable) {
  const short   uContinuousVariableIndex=3;
  try {
    //read and validate that tract identifier exists in coordinates file
    DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(Source, tid);
    if (eStatus != DataSetHandler::Accepted) return eStatus;
    //read and validate count
    if (Source.GetValueAt(guCountIndex) != 0) {
      if (!sscanf(Source.GetValueAt(guCountIndex), "%ld", &nCount)) {
       gPrint.Printf("Error: The value '%s' of record %ld in the %s could not be read as case count.\n"
                     "       Case count must be an integer.\n", BasePrint::P_READERROR,
                     Source.GetValueAt(guCountIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld in the %s does not contain case count.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Source.GetValueAt(guCountIndex), "-"))
        gPrint.Printf("Error: Record %ld, of the %s, contains a negative case count.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: The case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Source.GetValueAt(guCountIndex), std::numeric_limits<count_t>::max(),
                      Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (nCount == 0) return DataSetHandler::Ignored;
    DataSetHandler::RecordStatusType eDateStatus = RetrieveCountDate(Source, nDate);
    if (eDateStatus != DataSetHandler::Accepted)
      return eDateStatus;

    // read continuous variable
    if (!Source.GetValueAt(uContinuousVariableIndex)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing the continuous variable.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (sscanf(Source.GetValueAt(uContinuousVariableIndex), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The continuous variable value '%s' in record %ld, of %s, is not a number.\n",
                     BasePrint::P_READERROR, Source.GetValueAt(uContinuousVariableIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RetrieveCaseRecordData()","NormalDataSetHandler");
    throw;
  }
  return DataSetHandler::Accepted;
}

void NormalDataSetHandler::SetPurelyTemporalMeasureData(RealDataSet& DataSet) {
  try {
    DataSet.SetPTMeasureArray();
    DataSet.SetPTSqMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasureData()","NormalDataSetHandler");
    throw;
  }
}

/** sets purely temporal structures used in simulations */
void NormalDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  try {
    for (; itr != itr_end; ++itr) {
      (*itr)->SetPTMeasureArray();
      (*itr)->SetPTSqMeasureArray();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","NormalDataSetHandler");
    throw;
  }
}

/** Allocates randomizers for each dataset. There are currently 3 randomization types
    for the Rank model: null hypothesis and purely temporal optimized null hypothesis. */
void NormalDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers.at(0) = new NormalPurelyTemporalRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers.at(0) = new NormalRandomizer(gParameters.GetRandomizationSeed());
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
    x.AddCallpath("SetRandomizers()","NormalDataSetHandler");
    throw;
  }
}

