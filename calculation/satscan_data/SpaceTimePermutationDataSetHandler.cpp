//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimePermutationDataSetHandler.h"
#include "SaTScanData.h"

/** constructor */
SpaceTimePermutationDataSetHandler::SpaceTimePermutationDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                                      :DataSetHandler(DataHub, Print) {}

/** destructor */
SpaceTimePermutationDataSetHandler::~SpaceTimePermutationDataSetHandler() {}

/** allocates cases structures for dataset at index */
void SpaceTimePermutationDataSetHandler::AllocateCaseStructures(size_t tSetIndex) {
  try {
    gvDataSets[tSetIndex]->AllocateCasesArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","SpaceTimePermutationDataSetHandler");
    throw;
  }
}

/** returns new data gateway for real data */
AbtractDataSetGateway * SpaceTimePermutationDataSetHandler::GetNewDataGateway() const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                     t;

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
          ZdGenerateException("GetNewDataGateway() not implemented for purely spatial analysis.","GetNewDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetNewDataGateway() not implemented for purely temporal analysis.","GetNewDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewDataGateway() not implemented for spatial variation and temporal trends analysis.","GetNewDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }  
  return pDataSetGateway;
}

/** returns new data gateway for simulation data */
AbtractDataSetGateway * SpaceTimePermutationDataSetHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataSetGateway    * pDataSetGateway=0;
  DataSetInterface        Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                     t;

  try {
    pDataSetGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to datasets
      const RealDataSet& R_DataSet = *gvDataSets[t];
      const SimDataSet& S_DataSet = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(R_DataSet.GetTotalCases());
      Interface.SetTotalMeasureCount(R_DataSet.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for purely spatial analysis.","GetNewSimulationDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for purely temporal analysis.","GetNewSimulationDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(R_DataSet.GetMeasureArray());
          break;
        case SPATIALVARTEMPTREND        :
          ZdGenerateException("GetNewSimulationDataGateway() not implemented for spatial variation and temporal trends analysis.","GetNewSimulationDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      pDataSetGateway->AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataSetGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return pDataSetGateway;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& SpaceTimePermutationDataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataSets(); ++t)
    Container.push_back(new SimDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        ZdGenerateException("GetSimulationDataContainer() not implemented for purely spatial analysis.","GetSimulationDataContainer()");
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        ZdGenerateException("GetSimulationDataContainer() not implemented for purely temporal analysis.","GetSimulationDataContainer()");
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCasesArray();
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("GetSimulationDataContainer() not implemented for spatial variation and temporal trends analysis.","GetSimulationDataContainer()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** Read the count data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SpaceTimePermutationDataSetHandler::ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription) {
  int                                   i, j, iCategoryIndex;
  bool                                  bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(gPrint);
  std::string                           sBuffer;
  count_t                               Count, ** ppCounts, ** ppCategoryCounts;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    ppCounts = DataSet.GetCaseArray();

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(DataSet.GetPopulationData(), Parser, TractIndex, Count, Date, iCategoryIndex)) {
              if (Count > 0) {//ignore records that specify a count of zero
                //cumulatively add count to time by location structure
                ppCounts[0][TractIndex] += Count;
                if (ppCounts[0][TractIndex] < 0)
                  GenerateResolvableException("Error: The total number of cases, in data set %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                              tSetIndex, std::numeric_limits<count_t>::max());
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                  ppCounts[i][TractIndex] += Count;
                //record count as a case
                DataSet.GetPopulationData().AddCovariateCategoryCaseCount(iCategoryIndex, Count);
                //record count in structure(s) based upon population category
                ppCategoryCounts = DataSet.GetCategoryCaseArray(iCategoryIndex, true);
                ppCategoryCounts[0][TractIndex] += Count;
                for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                   ppCategoryCounts[i][TractIndex] += Count;
              }
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.SatScanPrintWarning("Please see the '%s file' section in the user guide for help.\n", szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: The %s file does not contain data.\n", szDescription);
      bValid = false;
    }

    if (gParameters.GetSimulationType() != FILESOURCE)
      ((SpaceTimeRandomizer*)gvDataSetRandomizers[tSetIndex])->CreateRandomizationData(DataSet);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return bValid;
}

/** */
bool SpaceTimePermutationDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (GetNumDataSets() == 1)
         gPrint.SatScanPrintf("Reading the case file\n");
       else
         gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","SpaceTimePermutationDataSetHandler");
    throw;
  }
  return true;
}

void SpaceTimePermutationDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
      case HA_RANDOMIZATION :
          gvDataSetRandomizers[0] = new SpaceTimeRandomizer();
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters);
          break;
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers[t] = gvDataSetRandomizers[0]->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","SpaceTimePermutationDataSetHandler");
    throw;
  }
}


