//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "UniformTimeDataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"
#include "DataSource.h"
#include "SSException.h"
#include "PoissonRandomizer.h"
#include "UniformTimeModel.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& UniformTimeDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
    switch (gParameters.GetAnalysisType()) {
        case PURELYTEMPORAL            :
        case PROSPECTIVEPURELYTEMPORAL : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT)); break;
        case SPACETIME                 :
        case PROSPECTIVESPACETIME      : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                         if (gParameters.GetIncludePurelyTemporalClusters())
                                            std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                                         break;
        default : throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
    };
    return Container;
}

/** For each data set, assigns data at meta location indexes. */
void UniformTimeDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
    for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
        // Set the case data for meta locations unless we're doing a power evaluation without reading the case file.
        if (!gParameters.getPerformPowerEvaluation() || !(gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))
            (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
        (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    }
}

/** Instead reading population data from a population, assigns the same arbitrary population
    date and population for each location specified in coordinates file. This routine is
    intended to be used by purely temporal analyses when the population file is omitted. */
bool UniformTimeDataSetHandler::CreatePopulationData(RealDataSet& DataSet) {
  float                                                 fPopulation = 1000; /** arbitrarily selected population */
  PopulationData::PopulationDateContainer_t             vprPopulationDates;
  const TractHandler&                                   theTracts = *(gDataHub.GetTInfo());
  tract_t                                               t, tNumTracts = theTracts.getLocations().size();
  int                                                   iCategoryIndex;

  try {
    // Make the dataset aggregate categories - this will way the reading of case data can proceed without problems.
    // Normally the population data dictates all possible population catgories and the case file data must follow suit.
    DataSet.setAggregateCovariateCategories(true);
    iCategoryIndex = 0; /* with aggregation, only one population category with index of zero */
    // Use the same arbitrarily selected population date for each location - we'll use the study period start date.
    vprPopulationDates.push_back(std::make_pair(gDataHub.GetStudyPeriodStartDate(), YEAR));
    DataSet.getPopulationData().SetPopulationDates(vprPopulationDates, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    // for each location, assign the same population count and date
    for (t=0; t < tNumTracts; ++t)
      DataSet.getPopulationData().AddCovariateCategoryPopulation(t, iCategoryIndex, vprPopulationDates.back(), fPopulation);
  }
  catch (prg_exception& x) {
    x.addTrace("CreatePopulationData()","UniformTimeDataSetHandler");
    throw;
  }
  return true;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters (real data) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & UniformTimeDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
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
        default :
          throw prg_error("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetDataGateway()","UniformTimeDataSetHandler");
    throw;
  }  
  return DataGatway;
}


/** Returns newly allocated data gateway object that references structures
    utilized in performing simulations (Monte Carlo) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & UniformTimeDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
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
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(S_DataSet.getCaseData_PT());
          Interface.SetPTMeasureArray(R_DataSet.getMeasureData_PT());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(S_DataSet.getCaseData_PT());
            Interface.SetPTMeasureArray(R_DataSet.getMeasureData_PT());
          }
          break;
        default :
          throw prg_error("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetSimulationDataGateway()","UniformTimeDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void UniformTimeDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations())
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
}

/** Attempts to read population and case data files into class RealDataSet objects. */
bool UniformTimeDataSetHandler::ReadData() {
    DataSetHandler::CountFileReadStatus readStaus;
    try {
        SetRandomizers();
        size_t numDataSet = GetNumDataSets();
        for (size_t t = 0; t < numDataSet; ++t) {
            if (GetNumDataSets() == 1) gPrint.Printf("Creating the population\n", BasePrint::P_STDOUT);
            else gPrint.Printf("Creating the population for data set %u\n", BasePrint::P_STDOUT, t + 1);
            if (!CreatePopulationData(GetDataSet(t))) return false;
            printFileReadMessage(BasePrint::CASEFILE, t, numDataSet == 1);
            readStaus = ReadCaseFile(GetDataSet(t));
            printReadStatusMessage(readStaus, false, t, numDataSet == 1);
            if (readStaus == DataSetHandler::ReadError || (readStaus != DataSetHandler::ReadSuccess && numDataSet == 1))
                return false;
        }
        removeDataSetsWithNoData();
    } catch (prg_exception& x) {
        x.addTrace("ReadData()","UniformTimeDataSetHandler");
        throw;
    }
    return true;
}

/** Allocates randomizers for each dataset. There are currently 6 randomization types
    for the Poisson model: time stratified, spatially stratified, null hypothesis,
                           purely temporal optimized null hypothesis, alternate hypothesis
                           and file source. */
void UniformTimeDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.getNumFileSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsSpaceTimeAnalysis())
            gvDataSetRandomizers.at(0) = new PoissonSpatialStratifiedRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers.at(0) = new PoissonPurelyTemporalNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else
            throw prg_error("Cannot determine randomizer class.", "SetRandomizers()");
          break;
      case FILESOURCE :
          gvDataSetRandomizers.at(0) = new FileSourceRandomizer(gParameters, gParameters.GetSimulationDataSourceFilename(), gParameters.GetRandomizationSeed());
          break;
      default :
          throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.getNumFileSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (prg_exception& x) {
    x.addTrace("SetRandomizers()","UniformTimeDataSetHandler");
    throw;
  }
}
