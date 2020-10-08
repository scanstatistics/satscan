//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonDataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"
#include "DataSource.h"
#include "SSException.h"
#include "PoissonModel.h"

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& PoissonDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL             : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                     break;
    case SEASONALTEMPORAL          :
    case PURELYTEMPORAL            :
    case PROSPECTIVEPURELYTEMPORAL : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                                     break;
    case SPACETIME                 :
    case PROSPECTIVESPACETIME      : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                     if (gParameters.GetIncludePurelyTemporalClusters())
                                       std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT));
                                     break;
    case SPATIALVARTEMPTREND       : std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData));
                                     std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_NC));
                                     std::for_each(Container.begin(), Container.end(), std::mem_fun(&DataSet::allocateCaseData_PT_NC));
                                     break;
    default : throw prg_error("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** For each data set, assigns data at meta location indexes. */
void PoissonDataSetHandler::assignMetaLocationData(RealDataContainer_t& Container) const {
    for (RealDataContainer_t::iterator itr=Container.begin(); itr != Container.end(); ++itr) {
        // Set the case data for meta locations unless we're doing a power evaluation without reading the case file.
        if (!gParameters.getPerformPowerEvaluation() || !(gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))
            (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
        (*itr)->setMeasureData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
    }
}

/* virtual method which builds Oliveira data sets. */
const RealDataContainer_t & PoissonDataSetHandler::buildOliveiraDataSets() {
    try {
        if (gParameters.GetAnalysisType() != PURELYSPATIAL)
            throw prg_error("buildOliveiraDataSets() is not implemented for analysis type '%d'.", "DataSetHandler::buildOliveiraDataSets()", gParameters.GetAnalysisType());
        _oliveira_data_sets.killAll();
        for (unsigned int i=0; i < gParameters.GetNumDataSets(); ++i) {
            _oliveira_data_sets.push_back(new RealDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), gDataHub.GetNumMetaTracts(), gParameters, i + 1));
            // allocate measure data and initialize from case data in real data set
            RealDataSet & oliveira_dataset = *_oliveira_data_sets.back();
            oliveira_dataset.allocateMeasureData();
            RealDataSet & real_dataset = *gvDataSets.at(i);
            TwoDimCountArray_t& caseArray = real_dataset.getCaseData();
            TwoDimMeasureArray_t& measureArray = oliveira_dataset.getMeasureData();
            for (unsigned int a=0; a < caseArray.Get1stDimension(); ++a) {
                for (unsigned int b=0; b < caseArray.Get2ndDimension(); ++b) {
                    measureArray.GetArray()[a][b] = static_cast<double>(caseArray.GetArray()[a][b]);
                }
            }
            oliveira_dataset.setTotalCases(real_dataset.getTotalCases());
            oliveira_dataset.setTotalMeasure(real_dataset.getTotalMeasure());
        }
    } catch (prg_exception& x) {
        x.addTrace("buildOliveiraDataSets()","PoissonDataSetHandler");
        throw;
    }
    return _oliveira_data_sets;
}

/** Converts passed string specifiying a population date to a julian date using
    DateStringParser object. Since we accumulate errors/warnings when reading
    input files, indication of a bad date is returned as false and message
    sent to print direction. Caller is responsible for ensuring that passed
    'const char *' points to a valid string. */
bool PoissonDataSetHandler::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, PopulationData::PopulationDate_t & PopulationDate) {
  bool                                  bValidDate=true;
  DateStringParser                      DateParser(gDataHub.GetParameters().GetPrecisionOfTimesType());
  DateStringParser::ParserStatus        eStatus;

  try {
    eStatus = DateParser.ParsePopulationDateString(sDateString, gDataHub.GetStudyPeriodStartDate(),
                                                   gDataHub.GetStudyPeriodStartDate(), PopulationDate.first, PopulationDate.second);
    switch (eStatus) {
      case DateStringParser::VALID_DATE       :
        bValidDate = true; break;
      case DateStringParser::AMBIGUOUS_YEAR   :
        gPrint.Printf("Error: Due to the study period being greater than 100 years, unable\n"
                      "       to determine century for two digit year in %s, record %ld.\n"
                      "       Please use four digit years.\n", BasePrint::P_READERROR,
                      gPrint.GetImpliedFileTypeString().c_str(), iRecordNumber);
        bValidDate = false;
        break;
      case DateStringParser::INVALID_DATE     :
      case DateStringParser::LESSER_PRECISION :
      default                                 :
        gPrint.Printf("Error: Invalid date '%s' in %s, record %ld.\n%s",
                      BasePrint::P_READERROR, sDateString, gPrint.GetImpliedFileTypeString().c_str(), iRecordNumber, DateParser.getLastParseError().c_str());
        bValidDate = false;                           
    };
  }
  catch (prg_exception& x) {
    x.addTrace("ConvertPopulationDateToJulian()","PoissonDataSetHandler");
    throw;
  }
  return bValidDate;
}

/** Instead reading population data from a population, assigns the same arbitrary population
    date and population for each location specified in coordinates file. This routine is
    intended to be used by purely temporal analyses when the population file is omitted. */
bool PoissonDataSetHandler::CreatePopulationData(RealDataSet& DataSet) {
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
    x.addTrace("CreatePopulationData()","PoissonDataSetHandler");
    throw;
  }
  return true;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters (real data) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & PoissonDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
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
        case PURELYSPATIAL              :
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          break;
        case SEASONALTEMPORAL           :
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
          Interface.SetCaseArray(DataSet.getCaseData().GetArray());
          Interface.SetNCCaseArray(DataSet.getCaseData_NC().GetArray());
          Interface.SetPTCaseArray(DataSet.getCaseData_PT_NC());
          Interface.SetMeasureArray(DataSet.getMeasureData().GetArray());
          Interface.SetNCMeasureArray(DataSet.getMeasureData_NC().GetArray());
          Interface.SetPTMeasureArray(DataSet.getMeasureData_PT_NC());
          Interface.SetTimeTrend(&DataSet.getTimeTrend());
          break;
        default :
          throw prg_error("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetDataGateway()","PoissonDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters in conjunction with Olivera's
    location relavance for locations and the Poisson probablity model.
    Caller is responsible for destructing returned object. */
AbstractDataSetGateway & PoissonDataSetHandler::GetOliveraDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
    DataSetInterface Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());

    try {
        DataGatway.Clear();
        for (size_t t=0; t < gvDataSets.size(); ++t) {
            //get reference to dataset
            const RealDataSet& real_DataSet = *gvDataSets.at(t);
            const DataSet& SimDataSet = *Container.at(t);

            //set total cases and measure
            Interface.SetTotalCasesCount(real_DataSet.getTotalCases());
            Interface.SetTotalMeasureCount(real_DataSet.getTotalMeasure());
            //set pointers to data structures
            switch (gParameters.GetAnalysisType()) {
                case PURELYSPATIAL             :
                    Interface.SetCaseArray(SimDataSet.getCaseData().GetArray());
                    Interface.SetMeasureArray(real_DataSet.getMeasureData().GetArray());
                    break;
                case SEASONALTEMPORAL          :
                case PROSPECTIVEPURELYTEMPORAL :
                case PURELYTEMPORAL            :
                case SPACETIME                 :
                case PROSPECTIVESPACETIME      :
                case SPATIALVARTEMPTREND       : 
                    throw prg_error("GetOliveraDataGateway() is not implemented for analysis type '%d'.","GetOliveraDataGateway()",gParameters.GetAnalysisType());
                default : throw prg_error("Unknown analysis type '%d'.","GetOliveraDataGateway()",gParameters.GetAnalysisType());
            };
            DataGatway.AddDataSetInterface(Interface);
        }
    } catch (prg_exception& x) {
        x.addTrace("GetOliveraDataGateway()","PoissonDataSetHandler");
        throw;
    }
    return DataGatway;
}

/** Returns newly allocated data gateway object that references structures
    utilized in performing simulations (Monte Carlo) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & PoissonDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container, const RandomizerContainer_t& rContainer) const {
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
        case PURELYSPATIAL              :
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          break;
        case SEASONALTEMPORAL           :
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
          Interface.SetCaseArray(S_DataSet.getCaseData().GetArray());
          Interface.SetNCCaseArray(S_DataSet.getCaseData_NC().GetArray());
          Interface.SetPTCaseArray(S_DataSet.getCaseData_PT_NC());
          Interface.SetMeasureArray(R_DataSet.getMeasureData().GetArray());
          Interface.SetNCMeasureArray(R_DataSet.getMeasureData_NC().GetArray());
          Interface.SetPTMeasureArray(R_DataSet.getMeasureData_PT_NC());
          Interface.SetTimeTrend(&S_DataSet.getTimeTrend());
          break;
        default :
          throw prg_error("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetSimulationDataGateway()","PoissonDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Randomizes data and assigns data at meta location indexes (if using meta locations file)*/
void PoissonDataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  DataSetHandler::RandomizeData(Container, SimDataContainer, iSimulationNumber);
  if (gParameters.UseMetaLocationsFile() || gParameters.UsingMultipleCoordinatesMetaLocations())
    for (SimulationDataContainer_t::iterator itr=SimDataContainer.begin(); itr != SimDataContainer.end(); ++itr)
      (*itr)->setCaseData_MetaLocations(gDataHub.GetTInfo()->getMetaManagerProxy());
}

/** Attempts to read population and case data files into class RealDataSet objects. */
bool PoissonDataSetHandler::ReadData() {
    try {
        SetRandomizers();

        for (size_t t=0; t < GetNumDataSets(); ++t) {
            if (gParameters.UsePopulationFile()) { //read population data file
                // Set the covariate categories to aggregated if we're doing a power evaluation without reading the case file.
                if (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)
                    GetDataSet(t).setAggregateCovariateCategories(true);
                if (GetNumDataSets() == 1) gPrint.Printf("Reading the population file\n", BasePrint::P_STDOUT);
                else gPrint.Printf("Reading the population file for data set %u\n", BasePrint::P_STDOUT, t + 1);
                if (!ReadPopulationFile(GetDataSet(t))) return false;
            } else { //create population data without input data - uniform population
                if (GetNumDataSets() == 1) gPrint.Printf("Creating the population\n", BasePrint::P_STDOUT);
                else gPrint.Printf("Creating the population for data set %u\n", BasePrint::P_STDOUT, t + 1);
                if (!CreatePopulationData(GetDataSet(t))) return false;
            }
            if (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)
                // Set the population covariate category equal to the number of user specified power cases.
                GetDataSet(t).getPopulationData().AddCovariateCategoryCaseCount(0, gParameters.getPowerEvaluationCaseCount());
            else {
                //  read case data file
                if (GetNumDataSets() == 1) gPrint.Printf("Reading the case file\n", BasePrint::P_STDOUT);
                else gPrint.Printf("Reading the case file for data set %u\n", BasePrint::P_STDOUT, t + 1);
                if (!ReadCaseFile(GetDataSet(t))) return false;
                //  validate population data against case data (if population was read from file)  
                if (gParameters.UsePopulationFile()) GetDataSet(t).checkPopulationDataCases(gDataHub);
            } 
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadData()","PoissonDataSetHandler");
        throw;
    }
    return true;
}

/** Reads data from population file into dataset structures. Reading of the
    file is done in two passes:
    - first pass  ; Determines all unique population dates specified in file.
                    Then adjusts list in accordance with study period and later
                    needs for interpolation.
    - second pass ; Updates PopulationData object (of dataset) structures for
                    each population record. Each record is validated with
                    warnings/errors printed to BasePrint object.
    Returns indication of successful read (true = valid, false = errors).
    Location identifiers of population file are matched to location identifiers
    specifed in coordinates file; so this function should not be called before
    the coordinates file has been read.                                         */
bool PoissonDataSetHandler::ReadPopulationFile(RealDataSet& DataSet) {
  int                                                           iCategoryIndex;
  bool                                                          bValid=true, bEmpty=true;
  tract_t                                                       TractIdentifierIndex;
  float                                                         fPopulation;
  const short                                                   uPopulationDateIndex=1, uPopulationIndex=2, uCovariateIndex=3;
  PopulationData::PopulationDate_t                              prPopulationDate;
  PopulationData::PopulationDateContainer_t                     vprPopulationDates;
  PopulationData::PopulationDateContainer_t::iterator           itr;

  try {
    gPrint.SetImpliedInputFileType(BasePrint::POPFILE);
    std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(gParameters.GetPopulationFileName(DataSet.getSetIndex()), gParameters.getInputSource(POPFILE, DataSet.getSetIndex()), gPrint));
    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
		// Skip ignored records based on locations.
		DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(*Source, TractIdentifierIndex);
		if (eStatus == DataSetHandler::Ignored)
			continue;
		bEmpty=false;
        //scan values and validate - population file records must contain tract id, date and population.
        if (!Source->GetValueAt(uPopulationDateIndex)) {
            gPrint.Printf("Error: Record %ld of the %s is missing the date.\n",
                          BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
        }
        if (!ConvertPopulationDateToJulian(Source->GetValueAt(uPopulationDateIndex), Source->GetCurrentRecordIndex(), prPopulationDate)) {
            bValid = false;
            continue;
        }
        //Insert population date in sorted order.
        itr = std::lower_bound(vprPopulationDates.begin(), vprPopulationDates.end(), prPopulationDate, ComparePopulationDates());
        if (itr == vprPopulationDates.end()) //List is empty, just add.
          vprPopulationDates.push_back(prPopulationDate);
        else if ((*itr).first == prPopulationDate.first) //replace more precise dates
          (*itr).second = std::max((*itr).second, prPopulationDate.second);
        else //insert into sorted position
          vprPopulationDates.insert(itr, prPopulationDate);
    }

    //2nd pass, read data in structures.
    if (bValid && !bEmpty) {
      //Set tract handlers population date structures since we already now all the dates from above.
      DataSet.getPopulationData().SetPopulationDates(vprPopulationDates,
                                                     DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetPrecisionOfTimesType()),
                                                     DateStringParser::getDateAsJulian(gParameters.GetStudyPeriodEndDate().c_str(), gParameters.GetPrecisionOfTimesType()));
      vprPopulationDates.clear(); //dump memory
      Source->GotoFirstRecord();
      bEmpty = true;
      //We can ignore error checking for population date and population since we already did this above.
      while (!gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
          //Validate that tract identifer is one of those defined in the coordinates file.
          DataSetHandler::RecordStatusType eStatus = RetrieveLocationIndex(*Source, TractIdentifierIndex);
          if (eStatus == DataSetHandler::Ignored)
            continue;
          if (eStatus == DataSetHandler::Rejected) {
            bValid = false;
            continue;
          }
          ConvertPopulationDateToJulian(Source->GetValueAt(uPopulationDateIndex), Source->GetCurrentRecordIndex(), prPopulationDate);
          if (!Source->GetValueAt(uPopulationIndex)) {
            gPrint.Printf("Error: Record %d of the %s is missing the population number.\n",
                          BasePrint::P_READERROR, Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          if (!string_to_type<float>(Source->GetValueAt(uPopulationIndex), fPopulation) || fPopulation < 0) {
            gPrint.Printf("Error: Population value '%s' in record %ld, of %s, is not a positive decimal number.\n",
                          BasePrint::P_READERROR, Source->GetValueAt(uPopulationIndex), Source->GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          //Scan for covariates to create population categories or find index.
          //First category created sets precedence as to how many covariates remaining records must have.
          if ((iCategoryIndex = DataSet.getPopulationData().CreateCovariateCategory(*Source, uCovariateIndex, gPrint)) == -1) {
            bValid = false;
            continue;
          }
          //Add population count for this tract/category/year
          DataSet.getPopulationData().AddCovariateCategoryPopulation(TractIdentifierIndex, iCategoryIndex, prPopulationDate, fPopulation);
          bEmpty = false;
      }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the 'population file' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: %s contains no data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    if (!DataSet.getPopulationData().CheckZeroPopulations(gPrint))
      return false;
  }
  catch (prg_exception& x) {
    x.addTrace("ReadPopulationFile()","PoissonDataSetHandler");
    throw;
  }
  return bValid;
}

/** Allocates randomizers for each dataset. There are currently 6 randomization types
    for the Poisson model: time stratified, spatially stratified, null hypothesis,
                           purely temporal optimized null hypothesis, alternate hypothesis
                           and file source. */
void PoissonDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.killAll();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
            gvDataSetRandomizers.at(0) = new PoissonTimeStratifiedRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
            gvDataSetRandomizers.at(0) = new PoissonSpatialStratifiedRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else if (gParameters.GetIsPurelyTemporalAnalysis())
            gvDataSetRandomizers.at(0) = new PoissonPurelyTemporalNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers.at(0) = new PoissonNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed());
          break;
      case FILESOURCE :
          gvDataSetRandomizers.at(0) = new FileSourceRandomizer(gParameters, gParameters.GetSimulationDataSourceFilename(), gParameters.GetRandomizationSeed());
          break;
      default :
          throw prg_error("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataSets(); ++t)
       gvDataSetRandomizers.at(t) = gvDataSetRandomizers.at(0)->Clone();
  }
  catch (prg_exception& x) {
    x.addTrace("SetRandomizers()","PoissonDataSetHandler");
    throw;
  }
}
