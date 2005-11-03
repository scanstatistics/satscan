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

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& SpaceTimePermutationDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        ZdGenerateException("AllocateSimulationData() not implemented for purely spatial analysis.","AllocateSimulationData()");
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        ZdGenerateException("AllocateSimulationData() not implemented for purely temporal analysis.","AllocateSimulationData()");
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCasesArray();
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                     t;

  try {
    DataGatway.Clear();
    for (t=0; t < gvDataSets.size(); ++t) {
      //get reference to dataset
      const RealDataSet& DataSet = *gvDataSets[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(DataSet.GetTotalCases());
      Interface.SetTotalMeasureCount(DataSet.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          ZdGenerateException("GetDataGateway() not implemented for purely spatial analysis.","GetDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetDataGateway() not implemented for purely temporal analysis.","GetDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
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

/** returns new data gateway for simulation data */
AbstractDataSetGateway & SpaceTimePermutationDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface        Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                     t;

  try {
    DataGatway.Clear();
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
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely spatial analysis.","GetSimulationDataGateway()");
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          ZdGenerateException("GetSimulationDataGateway() not implemented for purely temporal analysis.","GetSimulationDataGateway()");
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetMeasureArray(R_DataSet.GetMeasureArray());
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

/** Returns memory needed to allocate data set objects. */
double SpaceTimePermutationDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
  double        dRequirements(0);

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL : break;
    case SPATIALVARTEMPTREND : break;
    case PROSPECTIVEPURELYTEMPORAL : break;
    case PURELYTEMPORAL : break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
       //case array
       dRequirements = (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       if (gParameters.GetIncludePurelyTemporalClusters())
         //purely temporal case array
         dRequirements += (double)sizeof(count_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
       break;
     default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataSetAllocationRequirements()",gParameters.GetAnalysisType());
  };
  return dRequirements * (double)GetNumDataSets() + (double)sizeof(SimDataSet) * (double)GetNumDataSets();
}

/** Read the count data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool SpaceTimePermutationDataSetHandler::ReadCounts(size_t tSetIndex, FILE * fp, const char* szDescription) {
  int                                   i, iCategoryIndex;
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
    while (!gPrint.GetMaximumReadErrorsPrinted() && Parser.ReadString(fp)) {
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
      gPrint.Printf("Please see the '%s file' section in the user guide for help.\n", BasePrint::P_ERROR, szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The %s file does not contain data.\n", BasePrint::P_ERROR, szDescription);
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
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
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
          gvDataSetRandomizers[0] = new SpaceTimeRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters, gParameters.GetRandomizationSeed());
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


