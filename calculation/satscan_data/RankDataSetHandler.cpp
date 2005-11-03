//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "RankDataSetHandler.h"

/** constructor */
RankDataSetHandler::RankDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                   :DataSetHandler(DataHub, Print) {}

/** destructor */
RankDataSetHandler::~RankDataSetHandler() {}

/** allocates cases structures for dataset at index */
void RankDataSetHandler::AllocateCaseStructures(size_t iSetIndex) {
  try {
    gvDataSets[iSetIndex]->AllocateCasesArray();
    gvDataSets[iSetIndex]->AllocateMeasureArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","RankDataSetHandler");
    throw;
  }
}

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t & RankDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateMeasureArray();
        break;
    case PURELYTEMPORAL :
    case PROSPECTIVEPURELYTEMPORAL :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateMeasureArray();
          Container[t]->AllocatePTMeasureArray();
        }
        break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateMeasureArray();
          if (gParameters.GetIncludePurelyTemporalClusters())
            Container[t]->AllocatePTMeasureArray();
        }
        break;
    case SPATIALVARTEMPTREND :
        ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data */
AbstractDataSetGateway & RankDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
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
          ZdGenerateException("GetDataGateway() not implemented for purely spatial monotone analysis.","GetDataGateway()");
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","RankDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Returns memory needed to allocate data set objects. */
double RankDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
  double        dRequirements(0);

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
       //measure array
       dRequirements = (double)sizeof(measure_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(measure_t) * (double)gDataHub.GetNumTracts();
       break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
       //measure array
       dRequirements = (double)sizeof(measure_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(measure_t) * (double)gDataHub.GetNumTracts();
       if (gParameters.GetIncludePurelyTemporalClusters())
         //purely temporal measure array
         dRequirements += (double)sizeof(measure_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
       break;
    case PROSPECTIVEPURELYTEMPORAL :
    case PURELYTEMPORAL :
       //purely temporal analyses not of interest
       break;
    case SPATIALVARTEMPTREND :
       //svtt analysis not of interest at this time
       break;
     default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataSetAllocationRequirements()",gParameters.GetAnalysisType());
  };
  return dRequirements * (double)GetNumDataSets() + (double)sizeof(SimDataSet) * (double)GetNumDataSets();
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & RankDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                     t;

  try {
    DataGatway.Clear();
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
          Interface.SetCaseArray(R_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(R_DataSet.GetPTCasesArray());
          Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(R_DataSet.GetCaseArray());
          Interface.SetMeasureArray(S_DataSet.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(R_DataSet.GetPTCasesArray());
            Interface.SetPTMeasureArray(S_DataSet.GetPTMeasureArray());
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
    x.AddCallpath("GetSimulationDataGateway()","RankDataSetHandler");
    throw;
  }  
  return DataGatway;
}

bool RankDataSetHandler::ParseCaseFileLine(StringParser & Parser, tract_t& tid,
                                              count_t& nCount, Julian& nDate,
                                              measure_t& tContinuousVariable) {
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                    "       Location ID '%s' was not specified in the coordinates file.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(), Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                     "       Case count must be an integer.\n", BasePrint::P_READERROR,
                     Parser.GetWord(1), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld, in the %s, does not contain case count.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gPrint.Printf("Error: Record %ld, of the %s, contains a negative case count.\n",
                      BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                      Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;

    // read continuous variable
    if (!Parser.GetWord(3)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing the continuous variable.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Parser.GetWord(3), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The continuous variable value '%s' in record %ld, of %s, is not a number.\n",
                     BasePrint::P_READERROR, Parser.GetWord(3), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCaseFileLine()","RankDataSetHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool RankDataSetHandler::ReadCounts(size_t tSetIndex, FILE* fp, const char*) {
  bool          bValid=true, bEmpty=true;
  Julian        Date;
  tract_t       TractIndex;
  int           i;
  count_t       Count, ** ppCounts, tTotalCases=0;
  measure_t     tContinuousVariable, tTotalMeasure=0;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    StringParser Parser(gPrint);

    ppCounts = DataSet.GetCaseArray();
    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCaseFileLine(Parser, TractIndex, Count, Date, tContinuousVariable)) {
             //cumulatively add count to time by location structure
             ppCounts[0][TractIndex] += Count;
             if (ppCounts[0][TractIndex] < 0)
               GenerateResolvableException("Error: The total number of cases, in data set %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                           tSetIndex, std::numeric_limits<count_t>::max());
             for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
               ppCounts[i][TractIndex] += Count;
             //record count as a case or control  
//             DataSet.GetPopulationData().AddCaseCount(0, Count);
             if (gParameters.GetSimulationType() != FILESOURCE)
               for (i=0; i < Count; ++i)
                  ((RankRandomizer*)gvDataSetRandomizers[tSetIndex])->AddCase(gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tContinuousVariable);

             tTotalCases += Count;
             tTotalMeasure += tContinuousVariable;
           }
           else
             bValid = false;
         }
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
    else if (gParameters.GetSimulationType() != FILESOURCE) {
     ((RankRandomizer*)gvDataSetRandomizers[tSetIndex])->AssignMeasure(DataSet.GetMeasureArray(),
                                                                       DataSet.GetNumTimeIntervals(),
                                                                       DataSet.GetNumTracts());
     DataSet.SetTotalCases(tTotalCases);
     DataSet.SetTotalMeasure(tTotalMeasure);
    }

  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","RankDataSetHandler");
    throw;
  }
  return bValid;
}


bool RankDataSetHandler::ReadData() {
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
  catch (ZdException &x) {
    x.AddCallpath("ReadData()","RankDataSetHandler");
    throw;
  }
  return true;
}

/** sets purely temporal structures used in simulations */
void RankDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t)
       SimDataContainer[t]->SetPTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","RankDataSetHandler");
    throw;
  }
}

void RankDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          gvDataSetRandomizers[0] = new RankRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers[0] = new FileSourceRandomizer(gParameters, gParameters.GetRandomizationSeed());
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
    x.AddCallpath("SetRandomizers()","RankDataSetHandler");
    throw;
  }
}


