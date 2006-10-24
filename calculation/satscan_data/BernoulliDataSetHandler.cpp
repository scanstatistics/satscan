//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "BernoulliDataSetHandler.h"
#include "DataSource.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& BernoulliDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                     if (gParameters.GetIncludePurelyTemporalClusters())
                                       std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                                     break;
    case SPATIALVARTEMPTREND       :
       ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
       ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void BernoulliDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
    (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
    (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
  }
}

/** returns new data gateway for real data */
AbstractDataSetGateway & BernoulliDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTractsReferenced());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets[t];
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
          ZdGenerateException("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","BernoulliDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & BernoulliDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTractsReferenced());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
      //get reference to real and simulation datasets
      const RealDataSet& R_DataSet = *gvDataSets[t];
      const DataSet& S_DataSet = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          break;
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
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely spatial monotone analysis.","GetSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimulationDataGateway()","BernoulliDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void BernoulliDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile())
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
}

/** Attempts to read control file data into RealDataSet object. Returns boolean indication of read success. */
bool BernoulliDataSetHandler::ReadControlFile(RealDataSet& DataSet) {
  try {
    gPrint.SetImpliedInputFileType(BasePrint::CONTROLFILE);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetControlFileName(DataSet.getSetIndex()), gPrint));
    return ReadCounts(DataSet, *Source);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadControlFile()","BernoulliDataSetHandler");
    throw;
  }
}

/** Attempts to read case and control data files into class RealDataSet objects. */
bool BernoulliDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       GetDataSet(t).setAggregateCovariateCategories(true);
       if (GetNumDataSets() == 1)
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (!ReadCaseFile(GetDataSet(t)))
         return false;
       if (GetNumDataSets() == 1)
         gPrint.Printf("Reading the control file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the control file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (!ReadControlFile(GetDataSet(t)))
         return false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","BernoulliDataSetHandler");
    throw;
  }
  return true;
}

/** Allocates randomizers for each dataset. There are currently 3 randomization types
    for the Bernoulli model: null hypothesis, purely temporal optimized null hypothesis and
    file source. */
void BernoulliDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers.at(0) = new BernoulliPurelyTemporalNullHypothesisRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers.at(0) = new BernoulliNullHypothesisRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers.at(0) = new FileSourceRandomizer(gParameters, gParameters.GetRandomizationSeed());
          break;
      case HA_RANDOMIZATION :
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","BernoulliDataSetHandler");
    throw;
  }
}


