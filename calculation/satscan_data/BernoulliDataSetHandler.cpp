//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "BernoulliDataSetHandler.h"

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

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& BernoulliDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  SimulationDataContainer_t::iterator itr=Container.begin(), itr_end=Container.end();

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : for (; itr != itr_end; ++itr)
                                       (*itr)->AllocateCasesArray();
                                     break;
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : for (; itr != itr_end; ++itr)
                                       (*itr)->AllocatePTCasesArray();
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : for (; itr != itr_end; ++itr) {
                                       (*itr)->AllocateCasesArray();
                                       if (gParameters.GetIncludePurelyTemporalClusters())
                                         (*itr)->AllocatePTCasesArray();
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
AbstractDataSetGateway & BernoulliDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
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
    x.AddCallpath("GetDataGateway()","BernoulliDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** returns new data gateway for simulation data */
AbstractDataSetGateway & BernoulliDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
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

/** Returns memory needed to allocate data set objects. */
double BernoulliDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
  double        dRequirements(0);

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
       //case array
       dRequirements = (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       break;
    case SPACETIME :
    case PROSPECTIVESPACETIME :
       //case array
       dRequirements = (double)sizeof(count_t*) * (double)gDataHub.GetNumTimeIntervals() +
                       (double)gDataHub.GetNumTimeIntervals() * (double)sizeof(count_t) * (double)gDataHub.GetNumTracts();
       if (gParameters.GetIncludePurelyTemporalClusters())
         //purely temporal case array
         dRequirements += (double)sizeof(count_t) * (double)(gDataHub.GetNumTimeIntervals()+1);
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

/** Read the control data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool BernoulliDataSetHandler::ReadControlFile(size_t tSetIndex) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    if ((fp = fopen(gParameters.GetControlFileName(tSetIndex + 1).c_str(), "r")) == NULL) {
      gPrint.Printf("Error: Could not open the control file:\n'%s'.\n",
                    BasePrint::P_ERROR, gParameters.GetControlFileName(tSetIndex + 1).c_str());
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
         gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
       if (!ReadCaseFile(t))
         return false;
       if (GetNumDataSets() == 1)
         gPrint.Printf("Reading the control file\n", BasePrint::P_STDOUT);
       else
         gPrint.Printf("Reading the control file for data set %u\n", BasePrint::P_STDOUT, t + 1);
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
          if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers[0] = new BernoulliPurelyTemporalNullHypothesisRandomizer(gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers[0] = new BernoulliNullHypothesisRandomizer(gParameters.GetRandomizationSeed());
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
    x.AddCallpath("SetRandomizers()","BernoulliDataSetHandler");
    throw;
  }
}


