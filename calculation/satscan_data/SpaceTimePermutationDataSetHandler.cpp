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
        ZdGenerateException("AllocateSimulationData() not implemented for purely spatial analysis.","AllocateSimulationData()");
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        ZdGenerateException("AllocateSimulationData() not implemented for purely temporal analysis.","AllocateSimulationData()");
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void SpaceTimePermutationDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
  for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
    (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
    (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
  }
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in internal data set collection. */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTractsReferenced());

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
          ZdGenerateException("GetDataGateway() not implemented for purely spatial analysis.","GetDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetDataGateway() not implemented for purely temporal analysis.","GetDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetDataGateway() not implemented for spatial variation and temporal trends analysis.","GetDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Creates a new collection of DataSetInterface objects that reference appropriate
    data structures contained in passed simulation data collection. */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface        Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTractsReferenced());
  size_t                     t;

  try {
    DataGatway.Clear();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets.at(t);
      const DataSet& S_DataSet = *Container.at(t);
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.getTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.getTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely spatial analysis.","GetSimulationDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely temporal analysis.","GetSimulationDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetSimulationDataGateway() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimulationDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void SpaceTimePermutationDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile())
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaLocations());
}

/** Read the count data source, storing data in respective DataSet object. As a
    means to help user clean-up there data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
bool SpaceTimePermutationDataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
  int                                   i, iCategoryIndex;
  bool                                  bReadSuccess=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  count_t                               Count, ** ppCounts, ** ppCategoryCounts;
  DataSetHandler::RecordStatusType      eRecordStatus;  

  try {
    ppCounts = DataSet.allocateCaseData().GetArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           eRecordStatus = RetrieveCaseRecordData(DataSet.getPopulationData(), Source, TractIndex, Count, Date, iCategoryIndex);
           if (eRecordStatus == DataSetHandler::Accepted) {
              bEmpty = false;
              //cumulatively add count to time by location structure
              ppCounts[0][TractIndex] += Count;
              if (ppCounts[0][TractIndex] < 0)
                GenerateResolvableException("Error: The total number of cases, in data set %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                            DataSet.getSetIndex(), std::numeric_limits<count_t>::max());
              for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                ppCounts[i][TractIndex] += Count;
              //record count as a case
              DataSet.getPopulationData().AddCovariateCategoryCaseCount(iCategoryIndex, Count);
              //record count in structure(s) based upon population category
              ppCategoryCounts = DataSet.getCategoryCaseData(iCategoryIndex, true).GetArray();
              ppCategoryCounts[0][TractIndex] += Count;
              for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                 ppCategoryCounts[i][TractIndex] += Count;
           }
           else if (eRecordStatus == DataSetHandler::Ignored)
             continue;
           else   
             bReadSuccess = false;
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (!bReadSuccess)
      gPrint.Printf("Please see the '%s' section in the user guide for help.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bReadSuccess = false;
    }
    //if no errors in data read, create randomization data in respective randomizer object
    if (bReadSuccess && gParameters.GetSimulationType() != FILESOURCE)
      ((SpaceTimeRandomizer*)gvDataSetRandomizers.at(DataSet.getSetIndex() - 1))->CreateRandomizationData(DataSet);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return bReadSuccess;
}

/** Read input data in data set handler class objects. */
bool SpaceTimePermutationDataSetHandler::ReadData() {
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
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return true;
}

/** Allocates a data randomizer object for each data set. User could have
   opted to read randomization data from file. */
void SpaceTimePermutationDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
      case HA_RANDOMIZATION :
          gvDataSetRandomizers.at(0) = new SpaceTimeRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers.at(0) = new FileSourceRandomizer(gParameters, gParameters.GetRandomizationSeed());
          break;
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","SpaceTimePermutationDataSetHandler");
    throw;
  }
}


