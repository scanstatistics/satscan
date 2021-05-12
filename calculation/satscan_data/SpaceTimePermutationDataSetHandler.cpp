//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimePermutationDataSetHandler.h"
#include "SaTScanData.h"
#include "SSException.h"
#include "DataSource.h"  

/** constructor */
SpaceTimePermutationDataSetHandler::SpaceTimePermutationDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                                      :DataSetHandler(DataHub, Print) {}

/** destructor */
SpaceTimePermutationDataSetHandler::~SpaceTimePermutationDataSetHandler() {}

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    needed by data set handler (probability model) and analysis type during evaluation
    of simulated data. */
SimulationDataContainer_t& SpaceTimePermutationDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        throw prg_error("AllocateSimulationData() not implemented for purely spatial analysis.","AllocateSimulationData()");
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        throw prg_error("AllocateSimulationData() not implemented for purely temporal analysis.","AllocateSimulationData()");
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
        break;
    case SPATIALVARTEMPTREND :
        throw prg_error("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default :
        throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void SpaceTimePermutationDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
    (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in internal data set collection. */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface Interface(
      gDataHub.GetNumTimeIntervals(),
      gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations(),
      gDataHub.getDataInterfaceIntervalStartIndex()
  );

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
          throw prg_error("GetDataGateway() not implemented for purely spatial analysis.","GetDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          throw prg_error("GetDataGateway() not implemented for purely temporal analysis.","GetDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          break;
        case SPATIALVARTEMPTREND        :
          throw prg_error("GetDataGateway() not implemented for spatial variation and temporal trends analysis.","GetDataGateway()");
        default :
          throw prg_error("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in passed simulation data collection. */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
  DataSetInterface Interface(
      gDataHub.GetNumTimeIntervals(), 
      gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations(),
      gDataHub.getDataInterfaceIntervalStartIndex()
  );

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          throw prg_error("GetSimulationDataGateway() not implemented for purely spatial analysis.","GetSimulationDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          throw prg_error("GetSimulationDataGateway() not implemented for purely temporal analysis.","GetSimulationDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          break;
        case SPATIALVARTEMPTREND        :
          throw prg_error("GetSimulationDataGateway() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataGateway()");
        default :
          throw prg_error("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetSimulationDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void SpaceTimePermutationDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations()) {
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
  }    
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
DataSetHandler::CountFileReadStatus SpaceTimePermutationDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
    int                                   i, iCategoryIndex;
    Julian                                Date;
    tract_t                               TractIndex;
    count_t                               Count, ** ppCounts, ** ppCategoryCounts, totalCount = 0;
    DataSetHandler::CountFileReadStatus   readStatus = DataSetHandler::NoCounts;

    try {
        ppCounts = DataSet.allocateCaseData().GetArray();
        //Read data, parse and if no errors, increment count for tract at date.
        while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
            DataSetHandler::RecordStatusType eRecordStatus = RetrieveCaseRecordData(DataSet.getPopulationData(), Source, TractIndex, Count, Date, iCategoryIndex);
            if (eRecordStatus == DataSetHandler::Accepted) {
                readStatus = readStatus == DataSetHandler::NoCounts ? DataSetHandler::ReadSuccess : readStatus;
                //cumulatively add count to time by location structure
                ppCounts[0][TractIndex] += Count;
                if (ppCounts[0][TractIndex] < 0)
                    throw resolvable_error("Error: The total number of cases, in data set %u, is greater than the maximum allowed of %ld.\n",
                        DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                    ppCounts[i][TractIndex] += Count;
                // add to totals and check against numeric limits
                totalCount += Count;
                // check that total count does not exceed data type limitations
                if (totalCount < 0)
                    throw resolvable_error(
                        "Error: The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                        DataSet.getSetIndex() + 1, std::numeric_limits<count_t>::max()
                    );
                //record count as a case
                DataSet.getPopulationData().AddCovariateCategoryCaseCount(iCategoryIndex, Count);
                //record count in structure(s) based upon population category
                ppCategoryCounts = DataSet.getCategoryCaseData(iCategoryIndex, true).GetArray();
                ppCategoryCounts[0][TractIndex] += Count;
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                    ppCategoryCounts[i][TractIndex] += Count;
            } else if (eRecordStatus == DataSetHandler::Ignored)
                continue;
           else   
               readStatus = DataSetHandler::ReadError;
        }
        // Record total in data set.
        if (readStatus == DataSetHandler::ReadSuccess) {
            DataSet.setTotalCases(totalCount);
            //if no errors in data read, create randomization data in respective randomizer object
            if (gParameters.GetSimulationType() != FILESOURCE)
                ((SpaceTimeRandomizer*)gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))->CreateRandomizationData(DataSet);
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadCounts()","SpaceTimePermutationDataSetHandler");
        throw;
    }
    return readStatus;
}

/** Read input data in data set handler class objects. */
bool SpaceTimePermutationDataSetHandler::ReadData() {
    DataSetHandler::CountFileReadStatus readStaus;
    try {
        SetRandomizers();
        size_t numDataSet = GetNumDataSets();
        for (size_t t = 0; t < numDataSet; ++t) {
            printFileReadMessage(BasePrint::CASEFILE, t, numDataSet == 1);
            readStaus = ReadCaseFile(GetDataSet(t));
            printReadStatusMessage(readStaus, false, t, numDataSet == 1);
            if (readStaus == DataSetHandler::ReadError || (readStaus != DataSetHandler::ReadSuccess && numDataSet == 1))
                return false;
        }
        removeDataSetsWithNoData();
    } catch (prg_exception& x) {
        x.addTrace("ReadData()","SpaceTimePermutationDataSetHandler");
        throw;
    }
  return true;
}

/** Allocates a data randomizer object for each data set. User could have
   opted to read randomization data from file. */
void SpaceTimePermutationDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.getNumFileSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers.at(0) = new SpaceTimeRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers.at(0) = new FileSourceRandomizer(gParameters, getFilenameFormatTime(gParameters.GetSimulationDataSourceFilename(), gParameters.getTimestamp(), true), gParameters.GetRandomizationSeed());
          break;
      case HA_RANDOMIZATION :
      default :
          throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.getNumFileSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","SpaceTimePermutationDataSetHandler");
    throw;
  }
}


