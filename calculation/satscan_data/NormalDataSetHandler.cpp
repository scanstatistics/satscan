//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "NormalDataSetHandler.h"
#include "WeightedNormalRandomizer.h"
#include "DataSource.h"
#include "SSException.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    needed by data set handler (probability model) and analysis type during evaluation
    of simulated data. */
SimulationDataContainer_t & NormalDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
                                     std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux));
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
                                     std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux));
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
                                     std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_Aux));
                                     if (gParameters.GetIncludePurelyTemporalClusters()) {
                                       std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
                                       std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT_Aux));
                                     }
                                     break;
    case SPATIALVARTEMPTREND       :
       throw prg_error("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
       throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void NormalDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
    (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    (*itr)->setMeasureData_Aux_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in internal data set collection. */
AbstractDataSetGateway & NormalDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(DataSet.getTotalMeasure());
      Interface.SetTotalMeasureAuxCount(DataSet.getTotalMeasureAux());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          Interface.SetMeasureAuxArray(DataSet.getMeasureData_Aux().GetArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(DataSet.getMeasureData_PT());
          Interface.SetPTCaseArray(DataSet.getCaseData_PT());
          Interface.SetPTMeasureAuxArray(DataSet.getMeasureData_PT_Aux());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          Interface.SetMeasureAuxArray(DataSet.getMeasureData_Aux().GetArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(DataSet.getCaseData_PT());
            Interface.SetPTMeasureArray(DataSet.getMeasureData_PT());
            Interface.SetPTMeasureAuxArray(DataSet.getMeasureData_PT_Aux());
          }
          break;
        case SPATIALVARTEMPTREND        :
          throw prg_error("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
        default :
          throw prg_error("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetDataGateway()","NormalDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in passed simulation data collection. */
AbstractDataSetGateway & NormalDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      Interface.SetTotalMeasureAuxCount(R_DataSet.getTotalMeasureAux());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(R_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(S_DataSet.getMeasureData().GetArray());
          Interface.SetMeasureAuxArray(S_DataSet.getMeasureData_Aux().GetArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(R_DataSet.getCaseData_PT());
          Interface.SetPTMeasureArray(S_DataSet.getMeasureData_PT());
          Interface.SetPTMeasureAuxArray(S_DataSet.getMeasureData_PT_Aux());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(R_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(S_DataSet.getMeasureData().GetArray());
          Interface.SetMeasureAuxArray(S_DataSet.getMeasureData_Aux().GetArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(R_DataSet.getCaseData_PT());
            Interface.SetPTMeasureArray(S_DataSet.getMeasureData_PT());
            Interface.SetPTMeasureAuxArray(S_DataSet.getMeasureData_PT_Aux());
          }
          break;
        case SPATIALVARTEMPTREND        :
          throw prg_error("GetSimulationDataGateway() not implemented for purely spatial monotone analysis.","GetSimulationDataGateway()");
        default :
          throw prg_error("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetSimulationDataGateway()","NormalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void NormalDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations()) {
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr) {
      (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
      (*itr)->setMeasureData_Aux_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    }
  }
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool NormalDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
   return gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL ? ReadCountsWeighted(DataSet, Source) : ReadCountsStandard(DataSet, Source);
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool NormalDataSetHandler::ReadCountsStandard(RealDataSet& DataSet, DataSource& Source) {
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  count_t                               Count, tTotalCases=0;
  measure_t                             tContinuousVariable;
  double                                tTotalMeasure=0, tTotalMeasureAux=0;
  AbstractNormalRandomizer            * pRandomizer=0;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))) == 0)
      throw prg_error("Data set randomizer not AbstractNormalRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tContinuousVariable, 0);
           if (eRecordStatus == DataSetHandler::Accepted) {
             bEmpty = false;
             pRandomizer->AddCase(Count, gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tContinuousVariable);
             tTotalCases += Count;
             //check that addition did not exceed data type limitations
             if (tTotalCases < 0)
               throw resolvable_error("Error: The total number of individuals in dataset is greater than the maximum allowed of %ld.\n",
                                      std::numeric_limits<count_t>::max());
             for (count_t t=0; t < Count; ++t) {
               //check numeric limits of data type will not be exceeded
               if (tContinuousVariable > std::numeric_limits<measure_t>::max() - tTotalMeasure)
                 throw resolvable_error("Error: The total summation of observed values exceeds the maximum value allowed of %lf.\n",
                                        std::numeric_limits<measure_t>::max());
               tTotalMeasure += tContinuousVariable;
               //check numeric limits of data type will not be exceeded
               if (std::pow(tContinuousVariable, 2) > std::numeric_limits<measure_t>::max() - tTotalMeasureAux)
                 throw resolvable_error("Error: The total summation of observed values squared exceeds the maximum value allowed of %lf.\n",
                                        std::numeric_limits<measure_t>::max());
               tTotalMeasureAux += std::pow(tContinuousVariable, 2);
             }
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
    else
      pRandomizer->AssignFromAttributes(DataSet);
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCountsStandard()","NormalDataSetHandler");
    throw;
  }
  return bValid;
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool NormalDataSetHandler::ReadCountsWeighted(RealDataSet& DataSet, DataSource& Source) {
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  count_t                               Count, tTotalCases=0;
  measure_t                             tWeightVariable;
  double                                tTotalMeasure=0, tTotalMeasureAux=0, dRateVariable;
  AbstractWeightedNormalRandomizer    * pRandomizer=0;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))) == 0)
      throw prg_error("Data set randomizer not AbstractWeightedNormalRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tWeightVariable, &dRateVariable);
           if (eRecordStatus == DataSetHandler::Accepted) {
             bEmpty = false;
             dRateVariable = 1/dRateVariable; // see Joe's email -- question for Lan/Martin
             pRandomizer->AddCase(Count, gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tWeightVariable, dRateVariable);
             tTotalCases += Count;
             //check that addition did not exceed data type limitations
             if (tTotalCases < 0)
               throw resolvable_error("Error: The total number of individuals in dataset is greater than the maximum allowed of %ld.\n",
                                      std::numeric_limits<count_t>::max());
             for (count_t t=0; t < Count; ++t) {
               //check numeric limits of data type will not be exceeded
               if (tWeightVariable > std::numeric_limits<measure_t>::max() - tTotalMeasure)
                 throw resolvable_error("Error: The total summation of observed values exceeds the maximum value allowed of %lf.\n",
                                        std::numeric_limits<measure_t>::max());
               tTotalMeasure += tWeightVariable * dRateVariable;
               //check numeric limits of data type will not be exceeded
               if (std::pow(tWeightVariable, 2) > std::numeric_limits<measure_t>::max() - tTotalMeasureAux)
                 throw resolvable_error("Error: The total summation of observed values squared exceeds the maximum value allowed of %lf.\n",
                                        std::numeric_limits<measure_t>::max());
                 tTotalMeasureAux += dRateVariable;
             }
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
    else  
      pRandomizer->AssignFromAttributes(DataSet);
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCountsWeighted()","NormalDataSetHandler");
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
  catch (prg_exception& x) {
    x.addTrace("ReadData()","NormalDataSetHandler");
    throw;
  }
  return true;
}

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
DataSetHandler::RecordStatusType NormalDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable, double * pRateVariable) {
  const short   uContinuousVariableIndex=3, uWeightVariableIndex=4;
  short         uOffset;

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

    uOffset = (gParameters.GetPrecisionOfTimesType() == NONE ? uContinuousVariableIndex - 1 : uContinuousVariableIndex);
    // read continuous variable
    if (!Source.GetValueAt(uOffset)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing the continuous variable.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (sscanf(Source.GetValueAt(uOffset), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The continuous variable value '%s' in record %ld, of %s, is not a number.\n",
                     BasePrint::P_READERROR, Source.GetValueAt(uOffset), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }

    if (pRateVariable) {
      uOffset = (gParameters.GetPrecisionOfTimesType() == NONE ? uWeightVariableIndex - 1 : uWeightVariableIndex);
      // read continuous variable
      if (!Source.GetValueAt(uOffset)) {
        gPrint.Printf("Error: Record %d, of the %s, is missing the weight variable.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
        return DataSetHandler::Rejected;
      }
      if (sscanf(Source.GetValueAt(uOffset), "%lf", pRateVariable) != 1) {
         gPrint.Printf("Error: The rate variable value '%s' in record %ld, of %s, is not a number.\n",
                       BasePrint::P_READERROR, Source.GetValueAt(uOffset), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
         return DataSetHandler::Rejected;
      }
      if (*pRateVariable < 0) {
         gPrint.Printf("Error: The rate variable value '%s' in record %ld, of %s, is less than zero.\n",
                       BasePrint::P_READERROR, Source.GetValueAt(uOffset), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
         return DataSetHandler::Rejected;
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RetrieveCaseRecordData()","NormalDataSetHandler");
    throw;
  }
  return DataSetHandler::Accepted;
}

void NormalDataSetHandler::SetPurelyTemporalMeasureData(RealDataSet& DataSet) {
  try {
    DataSet.setMeasureData_PT();
    DataSet.setMeasureData_PT_Aux();
  }
  catch (prg_exception& x) {
    x.addTrace("SetPurelyTemporalMeasureData()","NormalDataSetHandler");
    throw;
  }
}

/** sets purely temporal structures used in simulations */
void NormalDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT));
  std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT_Aux));
}

/** Allocates randomizers for each dataset. There are currently 3 randomization types
    for the Rank model: null hypothesis and purely temporal optimized null hypothesis. */
void NormalDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis()) {
            if (gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL)
              gvDataSetRandomizers.at(0) = new WeightedNormalPurelyTemporalRandomizer(gParameters.GetRandomizationSeed());
            else
              gvDataSetRandomizers.at(0) = new NormalPurelyTemporalRandomizer(gParameters.GetRandomizationSeed());
          }
          else {
            if (gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL)
              gvDataSetRandomizers.at(0) = new WeightedNormalRandomizer(gParameters.GetRandomizationSeed());
            else
              gvDataSetRandomizers.at(0) = new NormalRandomizer(gParameters.GetRandomizationSeed());
          }
          break;
      case FILESOURCE :
      case HA_RANDOMIZATION :
      default : throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (prg_exception& x) {
    x.addTrace("SetRandomizers()","NormalDataSetHandler");
    throw;
  }
}

