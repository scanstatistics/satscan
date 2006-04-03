//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "RankDataSetHandler.h"
#include "DataSource.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t & RankDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : for (; itr != itr_end; ++itr)
                                       (*itr)->AllocateMeasureArray();
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : for (; itr != itr_end; ++itr)
                                       (*itr)->AllocatePTMeasureArray();
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateMeasureArray();
                                       if (gParameters.GetIncludePurelyTemporalClusters())
                                          (*itr)->AllocatePTMeasureArray();
                                     }
                                     break;
    case SPATIALVARTEMPTREND       :
      ZdGenerateException("AllocateSimulationData() not implemented for spatial variation and temporal trends analysis.","AllocateSimulationData()");
    default                        :
      ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** returns new data gateway for real data */
AbstractDataSetGateway & RankDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
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
  DataSetInterface      Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());

  try {
    DataGatway.Clear();
    for (size_t t=0; t < gvDataSets.size(); ++t) {
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

bool RankDataSetHandler::ParseCaseFileLine(DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, measure_t& tContinuousVariable) {
  const short   uContinuousVariableIndex=3;
  
  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Source.GetValueAt(guLocationIndex))) == -1) {
      gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                    "       Location ID '%s' was not specified in the coordinates file.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(), Source.GetValueAt(guLocationIndex));
      return false;
    }
    //read and validate count
    if (Source.GetValueAt(guCountIndex) != 0) {
      if (!sscanf(Source.GetValueAt(guCountIndex), "%ld", &nCount)) {
       gPrint.Printf("Error: The value '%s' of record %ld, in the %s, could not be read as case count.\n"
                     "       Case count must be an integer.\n", BasePrint::P_READERROR,
                     Source.GetValueAt(guCountIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld, in the %s, does not contain case count.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Source.GetValueAt(guCountIndex), "-"))
        gPrint.Printf("Error: Record %ld, of the %s, contains a negative case count.\n",
                      BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Source.GetValueAt(guCountIndex), std::numeric_limits<count_t>::max(),
                      Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Source, nDate))
      return false;

    // read continuous variable
    if (!Source.GetValueAt(uContinuousVariableIndex)) {
      gPrint.Printf("Error: Record %d, of the %s, is missing the continuous variable.\n",
                    BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (sscanf(Source.GetValueAt(uContinuousVariableIndex), "%lf", &tContinuousVariable) != 1) {
       gPrint.Printf("Error: The continuous variable value '%s' in record %ld, of %s, is not a number.\n",
                     BasePrint::P_READERROR, Source.GetValueAt(uContinuousVariableIndex), Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
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
bool RankDataSetHandler::ReadCounts(size_t tSetIndex, DataSource& Source, const char*) {
  bool                          bValid=true, bEmpty=true;
  Julian                        Date;
  tract_t                       TractIndex;
  int                           i;
  count_t                       Count, tTotalCases=0;
  measure_t                     tContinuousVariable, tTotalMeasure=0;
  AbstractRankRandomizer      * pRandomizer;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];

    if ((pRandomizer = dynamic_cast<AbstractRankRandomizer*>(gvDataSetRandomizers[tSetIndex])) == 0)
      ZdGenerateException("Data set randomizer not AbstractRankRandomizer type.", "ReadCounts()");
    //Read data, parse and if no errors, increment count for tract at date.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
           bEmpty = false;
           if (ParseCaseFileLine(Source, TractIndex, Count, Date, tContinuousVariable)) {
             pRandomizer->AddCase(Count, gDataHub.GetTimeIntervalOfDate(Date), TractIndex, tContinuousVariable);
             tTotalCases += Count;
             tTotalMeasure += tContinuousVariable;
           }
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
    else {
      pRandomizer->AssignFromAttributes(DataSet);
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
void RankDataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& Container) {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  try {
    for (; itr != itr_end; ++itr)
       (*itr)->SetPTMeasureArray();
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
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers[0] = new RankPurelyTemporalRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers[0] = new RankRandomizer(gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
      case HA_RANDOMIZATION :
      default : ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
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

