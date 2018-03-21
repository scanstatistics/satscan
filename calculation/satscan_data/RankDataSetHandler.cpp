//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "RankDataSetHandler.h"
#include "DataSource.h"
#include "SSException.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t & RankDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData));
                                     if (gParameters.GetIncludePurelyTemporalClusters())
                                       std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateMeasureData_PT));
                                     break;
    case SPATIALVARTEMPTREND       :
      throw prg_error("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
      throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void RankDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
    (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }
}

/** returns new data gateway for real data */
AbstractDataSetGateway & RankDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(DataSet.getTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(DataSet.getMeasureData_PT());
          Interface.SetPTCaseArray(DataSet.getCaseData_PT());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(DataSet.getCaseData_PT());
            Interface.SetPTMeasureArray(DataSet.getMeasureData_PT());
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
    x.addTrace("GetDataGateway()","RankDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & RankDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to real and simulation datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(R_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(S_DataSet.getMeasureData().GetArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(R_DataSet.getCaseData_PT());
          Interface.SetPTMeasureArray(S_DataSet.getMeasureData_PT());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(R_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(S_DataSet.getMeasureData().GetArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(R_DataSet.getCaseData_PT());
            Interface.SetPTMeasureArray(S_DataSet.getMeasureData_PT());
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
    x.addTrace("GetSimulationDataGateway()","RankDataSetHandler");
    throw;
  }  
  return DataGatway;
}

DataSetHandler::RecordStatusType RankDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable) {
  const short   uContinuousVariableIndex=3;
  
  try {
    //read and validate that tract identifier exists in coordinates file
    DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(Source, tid);
    if (eStatus != DataSetHandler::Accepted) return eStatus;
    //read and validate count
    if (Source.GetValueAt(guCountIndex) != 0) {
      if (!string_to_type<count_t>(Source.GetValueAt(guCountIndex), nCount) || nCount < 0) {
         gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                       "       Case count must be a whole number in range 0 - %u.\n", BasePrint::P_READERROR,
                       Source.GetValueAt(guCountIndex), Source.GetCurrentRecordIndex(), 
                       gPrint.GetImpliedFileTypeString().c_str(), std::numeric_limits<count_t>::max());
         return DataSetHandler::Rejected;
      } 
      if (nCount == 0) return DataSetHandler::Ignored;
    }
    else {
      gPrint.Printf("Error: Record %ld, in the %s, does not contain case count.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    DataSetHandler::RecordStatusType eDateStatus = RetrieveCountDate(Source, nDate);
    if (eDateStatus != DataSetHandler::Accepted)
      return eDateStatus;

    // read continuous variable
    if (!Source.GetValueAt(uContinuousVariableIndex)) {
      gPrint.Printf("Error: Record %d of the %s is missing the continuous variable.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (!string_to_type<measure_t>(Source.GetValueAt(uContinuousVariableIndex), tContinuousVariable)) {
       gPrint.Printf("Error: The continuous variable value '%s' in record %ld, of %s, is not a decimal number.\n",
                     BasePrint::P_READERROR, Source.GetValueAt(uContinuousVariableIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RetrieveCaseRecordData()","RankDataSetHandler");
    throw;
  }
  return DataSetHandler::Accepted;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void RankDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations()) {
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool RankDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  count_t                               Count, tTotalCases=0;
  measure_t                             tContinuousVariable, tTotalMeasure=0;
  AbstractRankRandomizer              * pRandomizer;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    if ((pRandomizer = dynamic_cast<AbstractRankRandomizer*>(gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))) == 0)
      throw prg_error("Data set randomizer not AbstractRankRandomizer type.", "ReadCounts()");
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           bEmpty = false;
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tContinuousVariable);
           if (eRecordStatus == DataSetHandler::Accepted) {
             pRandomizer->AddCase(Count, Date, TractIndex, tContinuousVariable);
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
      DataSet.setTotalCases(tTotalCases);
      DataSet.setTotalMeasure(tTotalMeasure);
    }

  }
  catch (prg_exception& x) {
    x.addTrace("ReadCounts()","RankDataSetHandler");
    throw;
  }
  return bValid;
}

/** Attempts to read case data file into class RealDataSet objects. */
bool RankDataSetHandler::ReadData() {
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
    x.addTrace("ReadData()","RankDataSetHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void RankDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setMeasureData_PT));
}

/** Allocates randomizers for each dataset. There are currently 2 randomization types
    for the Rank model: null hypothesis and purely temporal optimized null hypothesis. */
void RankDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis())
              gvDataSetRandomizers.at(0) = new RankPurelyTemporalRandomizer(gDataHub, gParameters.GetRandomizationSeed());
          else
              gvDataSetRandomizers.at(0) = new RankRandomizer(gDataHub, gParameters.GetRandomizationSeed());
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
    x.addTrace("SetRandomizers()","RankDataSetHandler");
    throw;
  }
}

