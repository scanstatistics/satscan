//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "OrdinalDataSetHandler.h"
#include "SSException.h"
#include "DataSource.h"

const size_t OrdinalDataSetHandler::gtMinimumCategories        = 3;
const count_t OrdinalDataSetHandler::gtMinimumCases            = 4;

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model) and analysis type. */
SimulationDataContainer_t & OrdinalDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             :
       for (; itr != itr_end; ++itr)
         (*itr)->allocateCaseData_Cat(GetDataSet(std::distance(Container.begin(), itr)).getPopulationData().GetNumOrdinalCategories());
       break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL :
       for (; itr != itr_end; ++itr)
         (*itr)->allocateCaseData_PT_Cat(GetDataSet(std::distance(Container.begin(), itr)).getPopulationData().GetNumOrdinalCategories());
       break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      :
       for (; itr != itr_end; ++itr) {
         (*itr)->allocateCaseData_Cat(GetDataSet(std::distance(Container.begin(), itr)).getPopulationData().GetNumOrdinalCategories());
         if (gParameters.GetIncludePurelyTemporalClusters())
           (*itr)->allocateCaseData_PT_Cat(GetDataSet(std::distance(Container.begin(), itr)).getPopulationData().GetNumOrdinalCategories());
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
void OrdinalDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr)
    (*itr)->setCaseData_Cat_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
}

/** returns data gateway for real data */
AbstractDataSetGateway & OrdinalDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets.at(t);
      //set total cases
      Interface.SetTotalCasesCount(DataSet.getTotalCases());
      Interface.SetNumOrdinalCategories(DataSet.getPopulationData().GetNumOrdinalCategories());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCategoryCaseArrays(DataSet.getCaseData_Cat());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
       case PURELYTEMPORAL             :
          Interface.SetPTCategoryCaseArray(DataSet.getCaseData_PT_Cat().GetArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(DataSet.getCaseData_Cat());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(DataSet.getCaseData_PT_Cat().GetArray());
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
    x.addTrace("GetDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** returns data gateway for simulation data */
AbstractDataSetGateway & OrdinalDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
  DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetNumOrdinalCategories(R_DataSet.getPopulationData().GetNumOrdinalCategories());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCategoryCaseArrays(S_DataSet.getCaseData_Cat());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCategoryCaseArray(S_DataSet.getCaseData_PT_Cat().GetArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCategoryCaseArrays(S_DataSet.getCaseData_Cat());
          if (gParameters.GetIncludePurelyTemporalClusters())
            Interface.SetPTCategoryCaseArray(S_DataSet.getCaseData_PT_Cat().GetArray());
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
    x.addTrace("GetSimulationDataGateway()","OrdinalDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Parses current file record contained in DataSource object in expected
    parts: location, case count, date and ordinal category. Returns true if no
    errors in data were found, else returns false and prints error messages to
    BasePrint object. */
DataSetHandler::RecordStatusType OrdinalDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable) {
  short   iCategoryIndex;

  try {
    //read and validate that tract identifier exists in coordinates file
    DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(Source, tid);
    if (eStatus != DataSetHandler::Accepted) return eStatus;
    //read case count
    if (Source.GetValueAt(guCountIndex) != 0) {
      if (!string_to_type<count_t>(Source.GetValueAt(guCountIndex), nCount) || nCount < 0) {
         gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                       "       Case count must be an integer in range 0 - %u.\n", BasePrint::P_READERROR,
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
    if (eDateStatus != DataSetHandler::Accepted) return eDateStatus;
    // read ordinal category
    iCategoryIndex = gParameters.GetPrecisionOfTimesType() == NONE ? guCountCategoryIndexNone : guCountCategoryIndex;
    if (!Source.GetValueAt(iCategoryIndex)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing ordinal data field.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }
    if (!string_to_type<measure_t>(Source.GetValueAt(iCategoryIndex), tContinuousVariable)) {
       gPrint.Printf("Error: The ordinal data '%s' in record %ld, of the %s, is not a decimal number.\n",
                     BasePrint::P_READERROR, Source.GetValueAt(iCategoryIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return DataSetHandler::Rejected;
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RetrieveCaseRecordData()","OrdinalDataSetHandler");
    throw;
  }
  return DataSetHandler::Accepted;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void OrdinalDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations()) {
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_Cat_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }
}

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool OrdinalDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
  bool                                  bReadSuccessful=true, bEmpty=true;
  Julian                                Date;
  tract_t                               tLocationIndex;
  count_t                               tCount, tTotalCases=0, ** ppCategoryCounts;
  measure_t                             tOrdinalVariable;
  std::vector<double>                   vReadCategories;
  DataSetHandler::RecordStatusType      eRecordStatus;

  try {
    //read, parse, validate and update data structures for each record in data file
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           //parse record into parts: location index, # of cases, date, ordinal catgory
           eRecordStatus = RetrieveCaseRecordData(Source, tLocationIndex, tCount, Date, tOrdinalVariable);
           if (eRecordStatus == Accepted) {
             bEmpty = false;
             //note each category read from file. since we are ignoring records with zero cases,
             //we might need this information for error reporting
             if (vReadCategories.end() == std::find(vReadCategories.begin(), vReadCategories.end(), tOrdinalVariable))
               vReadCategories.push_back(tOrdinalVariable);
             if (tCount > 0) { //ignore records with zero cases
               //add count to cumulative total
               tTotalCases += tCount;
               //check that addition did not exceed data type limitations
               if (tTotalCases < 0)
                 throw resolvable_error("Error: The total cases in dataset is greater than the maximum allowed of %ld.\n",
                                        std::numeric_limits<count_t>::max());
               //record count and get category's 2-D array pointer
               ppCategoryCounts = DataSet.addOrdinalCategoryCaseCount(tOrdinalVariable, tCount).GetArray();
               //update location case counts such that 'tCount' is reprented cumulatively through
               //time from start date through specifed date in record
               ppCategoryCounts[0][tLocationIndex] += tCount;
               for (int i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                  ppCategoryCounts[i][tLocationIndex] += tCount;
             }
           }
           else if (eRecordStatus == DataSetHandler::Ignored)
             continue;
           else   
             bReadSuccessful = false;
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
    else if (DataSet.getPopulationData().GetNumOrdinalCategories() < gtMinimumCategories) {
      if (DataSet.getPopulationData().GetNumOrdinalCategories() == vReadCategories.size()) {
        gPrint.Printf("Error: Data set case file specifies %i categories with cases but a minimum\n"
                      "       of %i categories is required.\n", BasePrint::P_ERROR,
                      DataSet.getPopulationData().GetNumOrdinalCategories(), gtMinimumCategories);
        bReadSuccessful = false;
      }
      else {
        gPrint.Printf("Error: The number of categories with cases is required to be a mimumum of %i.\n"
                      "       Data set case file specifies %i categories with %i of them containing no cases.\n",
                      BasePrint::P_ERROR, gtMinimumCategories, vReadCategories.size(),
                      vReadCategories.size() - DataSet.getPopulationData().GetNumOrdinalCategories());
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
      DataSet.setTotalCases(tTotalCases);
      DataSet.setTotalPopulation(tTotalCases);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadCounts()","OrdinalDataSetHandler");
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
       if (!ReadCaseFile(GetDataSet(t)))
         return false;
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadData()","OrdinalDataSetHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void OrdinalDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::setCaseData_PT_Cat));
}

/** Instanciates randomizer object for each data set. Currently there are two
    possible randomizers: OrdinalDenominatorDataRandomizer and FileSourceRandomizer */
void OrdinalDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers[0] = new OrdinalPurelyTemporalDenominatorDataRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers[0] = new OrdinalDenominatorDataRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters, gParameters.GetSimulationDataSourceFilename(), gParameters.GetRandomizationSeed());
          break;
      case HA_RANDOMIZATION :
      default :
          throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers[t] = gvDataSetRandomizers[0]->Clone();
  }
  catch (prg_exception& x) {
    x.addTrace("SetRandomizers()","OrdinalDataSetHandler");
    throw;
  }
}


