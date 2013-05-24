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
      Interface.SetRandomizer(*gvDataSetRandomizers.at(t)); // really only needed for purely spatial analysis
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
AbstractDataSetGateway & NormalDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
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
      Interface.SetRandomizer(*rContainer.at(t)); // really only needed for purely spatial analysis
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
    return gParameters.getIsWeightedNormal() ? ReadCountsWeighted(DataSet, Source) : ReadCountsStandard(DataSet, Source);
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
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tContinuousVariable, 0, 0);
           if (eRecordStatus == DataSetHandler::Accepted) {
             bEmpty = false;
             pRandomizer->AddCase(Count, Date, TractIndex, tContinuousVariable);
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
  std::vector<double>                   vCovariates;
  size_t                                tCovariatesFirst=0;

  try {
    if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))) == 0)
      throw prg_error("Data set randomizer not AbstractWeightedNormalRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(Source, TractIndex, Count, Date, tWeightVariable, &dRateVariable, &vCovariates);
           if (eRecordStatus == DataSetHandler::Accepted) {
             if (bEmpty) {
                 tCovariatesFirst = vCovariates.size();
                 const_cast<CParameters&>(gParameters).SetIsWeightedNormalCovariates(tCovariatesFirst > 0);
                 if (gParameters.getAdjustForWeeklyTrends() && tCovariatesFirst > 0)
                    throw resolvable_error("Error: The weighted normal model does not implement the day of week adjustment with covariates defined in case file.\n");
             } else {
               //if not first record, check covariates count against first record
               if (tCovariatesFirst != vCovariates.size())
                throw resolvable_error("Error: The number of covariates is not consistant in all case records.\n"
                                       "        First record has %ld covariates while %ld record contains %ld covariates." ,
                                       tCovariatesFirst, Source.GetCurrentRecordIndex(), vCovariates.size());
             }
             bEmpty = false;

             //TODO: Doing this to make same as SAS program for now!
             ///double var = dRateVariable * dRateVariable;
             ///dRateVariable = 1/var;

             // -- input is expected to be actual weight variable; not variance -- dRateVariable = 1/dRateVariable;
             if (vCovariates.size())
                pRandomizer->AddCase(Count, Date, TractIndex, tWeightVariable, dRateVariable, vCovariates);
             else
                pRandomizer->AddCase(Count, Date, TractIndex, tWeightVariable, dRateVariable);

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

/** Scans case input files to determine if data set is weighted. Returns indication
    of whether it was able to determine weighted status. */
bool NormalDataSetHandler::setIsWeighted() {
  std::vector<int> setColumns(GetNumDataSets(), 0);

  try {
    for (size_t t=0; t < GetNumDataSets(); ++t) {
        const RealDataSet& DataSet = GetDataSet(t);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetCaseFileName(DataSet.getSetIndex()), gPrint));
        if (Source->ReadRecord()) {
            setColumns.at(t) = Source->GetNumValues();
        }
    }

    int currentColumnsSize = setColumns.front();
    const_cast<CParameters&>(gParameters).SetIsWeightedNormal(currentColumnsSize > (gParameters.GetPrecisionOfTimesType() == NONE ? 3 : 4));
    if (gParameters.getIsWeightedNormal() && gParameters.getReportGiniOptimizedClusters()) {
      gPrint.Printf("Error: The Gini index based collection feature is not implemented with Normal probability model with weighted data.\n", BasePrint::P_ERROR);
      return false;
    }
    for (size_t t=1; t < setColumns.size(); ++t) {
        if (currentColumnsSize != setColumns.at(t)) {
            gPrint.Printf("Error: Data sets do not have same number of columns.\n"
                          "Data set 1 has %d columns while data set %d has %d.\n",
                          BasePrint::P_READERROR, currentColumnsSize, t + 1, setColumns.at(t));
            return false;
        }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("setIsWeighted()","NormalDataSetHandler");
    throw;
  }
  return true;
}

/** Attempts to read case data file into class RealDataSet objects. */
bool NormalDataSetHandler::ReadData() {
  try {
    if (!setIsWeighted())
        return false;
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (GetNumDataSets() == 1)
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (gParameters.getIsWeightedNormalCovariates()) {
         gPrint.Printf("Error: The Normal model with weights permits only one data set when covariates are specified.\n", BasePrint::P_READERROR);
         return false;
       }
       if (!ReadCaseFile(GetDataSet(t)))
         return false;

       if (gParameters.getIsWeightedNormalCovariates()) {
          //Check that analysis type is purely spatial.
          if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
            gPrint.Printf("Error: The Normal model with weights is implemented only for the\npurely spatial analysis when covariates are specified.\n", BasePrint::P_READERROR);
            return false;
          }

          //Meta locations are not implemented with covariates. Inorder for meta locations to function
          //properly, the current process would have to be significantly refactored from what I can tell.
          //Would probably need to:
          // - check for overlapping meta locations
          // - refactor how cluster data 'AddNeighborData' works for NormalCovariateSpatialData
          // - probably more that I can't think of at the moment
          if (gParameters.UseMetaLocationsFile()) {
            gPrint.Printf("Error: The Normal model with weights does not permit meta locations when covariates are specified.\n", BasePrint::P_READERROR);
            return false;
          }

          //When case data has covariates, we need to verify that every location defined in coordinates file is represented in case file, but only once.
          // NOTE: In terms of missing data (case records), we would need to implement a delete location feature; not easy, major TractHandler updates:
          //        - changes to CentroidHandlerPassThrough (not special grid file)
          //        - changes to non-Euclidian neighbors (structures already allocated)
          AbstractWeightedNormalRandomizer * pRandomizer=dynamic_cast<AbstractWeightedNormalRandomizer*>(gvDataSetRandomizers.at(t));
          if (pRandomizer && !pRandomizer->hasUniqueLocationsCoverage(gDataHub)) {
            gPrint.Printf("Error: The Normal model with weights requires every location defined in the coordinates\n"
                          "       file to be defined once, only only once, in the case file when covariates are specified.\n"
                          "       Verify that your coordinates file locations are one to one with the case file locations.\n", BasePrint::P_READERROR);
            return false;
          }
       }
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
DataSetHandler::RecordStatusType NormalDataSetHandler::RetrieveCaseRecordData(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable, double * pRateVariable, std::vector<double> * pvCovariates) {
  const short   uContinuousVariableIndex=3, uWeightVariableIndex=4, uCovariatesIndex=5;
  short         uOffset;
  const char  * pCovariate;
  double        dCovariate;

  try {
    //read and validate that tract identifier exists in coordinates file
    DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(Source, tid);
    if (eStatus != DataSetHandler::Accepted) return eStatus;
    //read and validate count
    if (Source.GetValueAt(guCountIndex) != 0) {
        if (!string_to_type<count_t>(Source.GetValueAt(guCountIndex), nCount) || nCount < 0) {
            gPrint.Printf("Error: The value '%s' of record %ld in the %s could not be read as case count.\n"
                          "       Case count must be a whole number in range 0 - %u.\n", BasePrint::P_READERROR,
                          Source.GetValueAt(guCountIndex), Source.GetCurrentRecordIndex(),
                          gPrint.GetImpliedFileTypeString().c_str(), std::numeric_limits<count_t>::max());
            return DataSetHandler::Rejected;
        }
	    if (gParameters.getIsWeightedNormal() && nCount > 1) {
		    // For weighted normal data, the count column can only be zero or one. This was decided due to
		    // users incorrectly using this column in their data.
            gPrint.Printf("Error: The case count for the Normal model with weights can be either 0 or 1. Incorrect value of '%s' in record %ld of %s.\n",
                          BasePrint::P_READERROR, Source.GetValueAt(guCountIndex),
                          Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            return DataSetHandler::Rejected;
	    }
        if (nCount == 0) return DataSetHandler::Ignored;
    }
    else {
      gPrint.Printf("Error: Record %ld in the %s does not contain case count.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return DataSetHandler::Rejected;
    }

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
    if (!string_to_type<measure_t>(Source.GetValueAt(uOffset), tContinuousVariable)) {
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
      if (!string_to_type<double>(Source.GetValueAt(uOffset), *pRateVariable) || *pRateVariable < 0) {
        gPrint.Printf("Error: The rate variable value '%s' in record %ld, of %s, is not a number greater than zero.\n",
                      BasePrint::P_READERROR, Source.GetValueAt(uOffset), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
        return DataSetHandler::Rejected;
      }
    }

    if (pvCovariates) {
      pvCovariates->clear();
      uOffset = (gParameters.GetPrecisionOfTimesType() == NONE ? uCovariatesIndex - 1 : uCovariatesIndex);
      while ((pCovariate = Source.GetValueAt(static_cast<short>(pvCovariates->size()) + uOffset)) != 0) {
        if (!string_to_type<double>(pCovariate, dCovariate)) {
            gPrint.Printf("Error: The value '%s' of record %ld in %s could not be read as covariate.\n"
                          "       Case covariate must be numeric.\n", BasePrint::P_READERROR,
                          pCovariate, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            return DataSetHandler::Rejected;
        }
        pvCovariates->push_back(dCovariate);
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
              if (gParameters.getIsWeightedNormal())
                  gvDataSetRandomizers.at(0) = new WeightedNormalPurelyTemporalRandomizer(gDataHub, gParameters.GetRandomizationSeed());
            else
                gvDataSetRandomizers.at(0) = new NormalPurelyTemporalRandomizer(gDataHub, gParameters.GetRandomizationSeed());
          }
          else {
            if (gParameters.getIsWeightedNormal())
              gvDataSetRandomizers.at(0) = new WeightedNormalRandomizer(gDataHub, gParameters.GetRandomizationSeed());
            else
              gvDataSetRandomizers.at(0) = new NormalRandomizer(gDataHub, gParameters.GetRandomizationSeed());
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

