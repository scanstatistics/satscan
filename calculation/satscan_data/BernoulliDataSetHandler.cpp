//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "BernoulliDataSetHandler.h"

/** constructor */
BernoulliDataSetHandler::BernoulliDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                           :DataSetHandler(DataHub, Print) {}

/** destructor */
BernoulliDataSetHandler::~BernoulliDataSetHandler() {}

/** allocates cases structures for dataset*/
void BernoulliDataSetHandler::AllocateControlStructures(size_t tSetIndex) {
  try {
    gvDataSets[tSetIndex]->AllocateControlsArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateControlStructures()","BernoulliDataSetHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataSetGateway * BernoulliDataSetHandler::GetNewDataGateway() const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets[t];
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
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewDataGateway()","BernoulliDataSetHandler");
    throw;
  }  
  return pDataSetGateway;
}

/** returns new data gateway for simulation data */
AbtractDataSetGateway * BernoulliDataSetHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to real and simulation datasets
      const RealDataSet& R_DataSet = *gvDataSets[t];
      const SimDataSet& S_DataSet = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.GetTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(R_DataSet.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(S_DataSet.GetPTCasesArray());
          Interface.SetPTMeasureArray(R_DataSet.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(R_DataSet.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(S_DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(R_DataSet.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial monotone analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","BernoulliDataSetHandler");
    throw;
  }  
  return pDataSetGateway;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& BernoulliDataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear(); 
  for (unsigned int t=0; t < gParameters.GetNumDataSets(); ++t)
    Container.push_back(new SimDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCasesArray();
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          Container[t]->AllocatePTCasesArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          if (gParameters.GetIncludePurelyTemporalClusters())
            Container[t]->AllocatePTCasesArray();
        }
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** Read the control data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool BernoulliDataSetHandler::ReadControlFile(size_t tSetIndex) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    if ((fp = fopen(gParameters.GetControlFileName(tSetIndex + 1).c_str(), "r")) == NULL) {
      gPrint.SatScanPrintWarning("Error: Could not open the control file:\n'%s'.\n",
                                   gParameters.GetControlFileName(tSetIndex + 1).c_str());
      return false;
    }
    gPrint.SetImpliedInputFileType(BasePrint::CONTROLFILE, (GetNumDataSets() == 1 ? 0 : tSetIndex + 1));
    AllocateControlStructures(tSetIndex);
    bValid = ReadCounts(tSetIndex, fp, "control");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadControlFile()","BernoulliDataSetHandler");
    throw;
  }
  return bValid;
}

/** */
bool BernoulliDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       GetDataSet(t).SetAggregateCovariateCategories(true);
       if (GetNumDataSets() == 1)
         gPrint.SatScanPrintf("Reading the case file\n");
       else
         gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
       if (GetNumDataSets() == 1)
         gPrint.SatScanPrintf("Reading the control file\n");
       else
         gPrint.SatScanPrintf("Reading the control file for data set %u\n", t + 1);
       if (!ReadControlFile(t))
         return false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","BernoulliDataSetHandler");
    throw;
  }
  return true;
}

/** allocates randomizers for each dataset */
void BernoulliDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers[0] = new BernoulliNullHypothesisRandomizer();
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters);
          break;
      case HA_RANDOMIZATION :
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers[t] = gvDataSetRandomizers[0]->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","BernoulliDataSetHandler");
    throw;
  }
}


