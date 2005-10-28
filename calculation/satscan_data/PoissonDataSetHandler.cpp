//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonDataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"

/** class constructor */
PoissonDataSetHandler::PoissonDataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
                         :DataSetHandler(DataHub, Print) {}

/** class destructor */
PoissonDataSetHandler::~PoissonDataSetHandler() {}

/** For each element in SimulationDataContainer_t, allocates appropriate data structures
    as needed by data set handler (probability model). */
SimulationDataContainer_t& PoissonDataSetHandler::AllocateSimulationData(SimulationDataContainer_t& Container) const {
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
        for (size_t t=0; t < Container.size(); ++t) {
          Container[t]->AllocateCasesArray();
          Container[t]->AllocateNCCasesArray();
          Container[t]->AllocatePTCasesArray();
        }
        break;
    default :
        ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationData()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** Converts passed string specifiying a population date to a julian date using
    DateStringParser object. Since we accumulate errors/warnings when reading
    input files, indication of a bad date is returned as false and message
    sent to print direction. Caller is responsible for ensuring that passed
    'const char *' points to a valid string. */
bool PoissonDataSetHandler::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber,
                                                             std::pair<Julian, DatePrecisionType>& PopulationDate) {
  bool                                  bValidDate=true;
  DateStringParser                      DateParser;
  DateStringParser::ParserStatus        eStatus;

  try {
    eStatus = DateParser.ParsePopulationDateString(sDateString, gDataHub.GetStudyPeriodStartDate(),
                                                   gDataHub.GetStudyPeriodStartDate(), PopulationDate.first, PopulationDate.second);
    switch (eStatus) {
      case DateStringParser::VALID_DATE       :
        bValidDate = true; break;
      case DateStringParser::AMBIGUOUS_YEAR   :
        gPrint.PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n"
                                   "       to determine century for two digit year in %s, record %ld.\n"
                                   "       Please use four digit years.\n",
                                   gPrint.GetImpliedFileTypeString().c_str(), iRecordNumber);
                                   bValidDate = false;
                                   break;
      case DateStringParser::INVALID_DATE     :
      case DateStringParser::LESSER_PRECISION :
      default                                 :
        gPrint.PrintInputWarning("Error: Invalid date '%s' in %s, record %ld.\n",
                                   sDateString, gPrint.GetImpliedFileTypeString().c_str(), iRecordNumber);
        bValidDate = false;                           
    };
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertPopulationDateToJulian()","PoissonDataSetHandler");
    throw;
  }
  return bValidDate;
}

/** Instead reading population data from a population, assigns the same arbitrary population
    date and population for each location specified in coordinates file. This routine is
    intended to be used by purely temporal analyses when the population file is omitted. */
bool PoissonDataSetHandler::CreatePopulationData(size_t tSetIndex) {
  float                                                 fPopulation = 1000; /** arbitrarily selected population */
  std::vector<std::pair<Julian, DatePrecisionType> >    vprPopulationDates;
  const TractHandler&                                   theTracts = *(gDataHub.GetTInfo());
  tract_t                                               t, tNumTracts = theTracts.tiGetNumTracts();
  int                                                   iCategoryIndex;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    // Make the dataset aggregate categories - this will way the reading of case data can proceed without problems.
    // Normally the population data dictates all possible population catgories and the case file data must follow suit.
    DataSet.SetAggregateCovariateCategories(true);
    iCategoryIndex = 0; /* with aggregation, only one population category with index of zero */
    // Use the same arbitrarily selected population date for each location - we'll use the study period start date.
    vprPopulationDates.push_back(std::pair<Julian, DatePrecisionType>(gDataHub.GetStudyPeriodStartDate(), YEAR));
    DataSet.GetPopulationData().SetPopulationDates(vprPopulationDates, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    // for each location, assign the same population count and date
    for (t=0; t < tNumTracts; ++t)
      DataSet.GetPopulationData().AddCovariateCategoryPopulation(t, iCategoryIndex, vprPopulationDates.back(), fPopulation);
  }
  catch (ZdException &x) {
    x.AddCallpath("CreatePopulationData()","PoissonDataSetHandler");
    throw;
  }
  return true;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters (real data) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & PoissonDataSetHandler::GetDataGateway(AbstractDataSetGateway& DataGatway) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

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
          Interface.SetCaseArray(DataSet.GetCaseArray());
          Interface.SetNCCaseArray(DataSet.GetNCCaseArray());
          Interface.SetPTCaseArray(DataSet.GetPTCasesArray());
          Interface.SetMeasureArray(DataSet.GetMeasureArray());
          Interface.SetNCMeasureArray(DataSet.GetNCMeasureArray());
          Interface.SetPTMeasureArray(DataSet.GetPTMeasureArray());
          Interface.SetTimeTrend(&DataSet.GetTimeTrend());
          break;
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetDataGateway()","PoissonDataSetHandler");
    throw;
  }  
  return DataGatway;
}

/** Returns newly allocated data gateway object that references structures
    utilized in performing simulations (Monte Carlo) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbstractDataSetGateway & PoissonDataSetHandler::GetSimulationDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
  DataSetInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

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
          Interface.SetCaseArray(S_DataSet.GetCaseArray());
          Interface.SetNCCaseArray(S_DataSet.GetNCCaseArray());
          Interface.SetPTCaseArray(S_DataSet.GetPTCasesArray());
          Interface.SetMeasureArray(R_DataSet.GetMeasureArray());
          Interface.SetNCMeasureArray(R_DataSet.GetNCMeasureArray());
          Interface.SetPTMeasureArray(R_DataSet.GetPTMeasureArray());
          Interface.SetTimeTrend(&S_DataSet.GetTimeTrend());
          break;
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataGateway()",gParameters.GetAnalysisType());
      };
      DataGatway.AddDataSetInterface(Interface);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimulationDataGateway()","PoissonDataSetHandler");
    throw;
  }
  return DataGatway;
}

/** Returns memory needed to allocate data set objects. */
double PoissonDataSetHandler::GetSimulationDataSetAllocationRequirements() const {
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

/** Refined process for reading input data from files into respective data
    set. For the Poisson probability model, inputs file of interest are the
    case and population. Prior to reading input data randomization objects are
    allocated. Echos progress and warnings/errors during read to BasePrint
    object. It is not recommended to call this function more than once. */
bool PoissonDataSetHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumDataSets(); ++t) {
       if (gParameters.UsePopulationFile()) { //read population data file
         if (GetNumDataSets() == 1) gPrint.SatScanPrintf("Reading the population file\n");
         else gPrint.SatScanPrintf("Reading the population file for data set %u\n", t + 1);
         if (!ReadPopulationFile(t)) return false;
       }
       else { //create population data without input data
         if (GetNumDataSets() == 1) gPrint.SatScanPrintf("Creating the population\n");
         else gPrint.SatScanPrintf("Creating the population for data set %u\n", t + 1);
         if (!CreatePopulationData(t)) return false;
       }
       //read case data file
       if (GetNumDataSets() == 1) gPrint.SatScanPrintf("Reading the case file\n");
       else gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t)) return false;
       //validate population data against case data (if population was read from file)  
       if (gParameters.UsePopulationFile()) GetDataSet(t).CheckPopulationDataCases(gDataHub);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","PoissonDataSetHandler");
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
bool PoissonDataSetHandler::ReadPopulationFile(size_t tSetIndex) {
  int                                                           iCategoryIndex;
  bool                                                          bValid=true, bEmpty=true;
  tract_t                                                       TractIdentifierIndex;
  float                                                         fPopulation;
  FILE                                                        * fp=0; // Ptr to population file
  std::pair<Julian, DatePrecisionType>                          prPopulationDate;
  std::vector<std::pair<Julian, DatePrecisionType> >            vprPopulationDates;
  std::vector<std::pair<Julian, DatePrecisionType> >::iterator  itr;

  try {
    RealDataSet& DataSet = *gvDataSets[tSetIndex];
    gPrint.SetImpliedInputFileType(BasePrint::POPFILE, (GetNumDataSets() == 1 ? 0 : tSetIndex + 1));
    StringParser Parser(gPrint);

    if ((fp = fopen(gParameters.GetPopulationFileName(tSetIndex + 1).c_str(), "r")) == NULL) {
      gPrint.SatScanPrintWarning("Error: Could not open the population file:\n'%s'.\n",
                                   gParameters.GetPopulationFileName(tSetIndex + 1).c_str());
      return false;
    }

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (Parser.ReadString(fp)) {
        //skip lines that do not contain data
        if (!Parser.HasWords())
          continue;
        bEmpty=false;
        //scan values and validate - population file records must contain tract id, date and population.
        if (!Parser.GetWord(1)) {
            gPrint.PrintInputWarning("Error: Record %ld, of the %s, is missing the date.\n",
                                       Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
        }
        if (!ConvertPopulationDateToJulian(Parser.GetWord(1), Parser.GetReadCount(), prPopulationDate)) {
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
      DataSet.GetPopulationData().SetPopulationDates(vprPopulationDates,
                                                        CharToJulian(gParameters.GetStudyPeriodStartDate().c_str()),
                                                        CharToJulian(gParameters.GetStudyPeriodEndDate().c_str()));
      vprPopulationDates.clear(); //dump memory
      Parser.Reset(); //reset parser for second pass
      //reset for second read
      fseek(fp, 0L, SEEK_SET);
      //We can ignore error checking for population date and population since we already did this above.
      while (Parser.ReadString(fp)) {
          if (!Parser.HasWords()) // Skip Blank Lines
            continue;
          ConvertPopulationDateToJulian(Parser.GetWord(1), Parser.GetReadCount(), prPopulationDate);
          if (!Parser.GetWord(2)) {
            gPrint.PrintInputWarning("Error: Record %d, of the %s, is missing the population number.\n",
                                       Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          if (sscanf(Parser.GetWord(2), "%f", &fPopulation) != 1) {
            gPrint.PrintInputWarning("Error: Population value '%s' in record %ld, of %s, is not a number.\n",
                                       Parser.GetWord(2), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          //validate that population is not negative or exceeding type precision
          if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
            if (strstr(Parser.GetWord(2), "-"))
              gPrint.PrintInputWarning("Error: Negative population in record %ld of %s.\n",
                                         Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
            else
              gPrint.PrintInputWarning("Error: The population '%s', in record %ld of the %s, exceeds the maximum allowed value of %i.\n",
                                         Parser.GetWord(2), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                                         std::numeric_limits<float>::max());
            bValid = false;
            continue;
          }
          //Scan for covariates to create population categories or find index.
          //First category created sets precedence as to how many covariates remaining records must have.
          if ((iCategoryIndex = DataSet.GetPopulationData().CreateCovariateCategory(Parser, 3, gPrint)) == -1) {
            bValid = false;
            continue;
          }
          //Validate that tract identifer is one of those defined in the coordinates file.
          if ((TractIdentifierIndex = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
            gPrint.PrintInputWarning("Error: Unknown location ID in %s, record %ld.\n",
                                       gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
            gPrint.PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
            bValid = false;
            continue;
          }
          //Add population count for this tract/category/year
          DataSet.GetPopulationData().AddCovariateCategoryPopulation(TractIdentifierIndex, iCategoryIndex, prPopulationDate, fPopulation);
      }
    }
    //close file pointer
    fclose(fp); fp=0;
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.PrintWarningLine("Please see the 'population file' section in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: %s contains no data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    if (!DataSet.GetPopulationData().CheckZeroPopulations(stderr, gPrint))
      return false;
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadPopulationFile()","PoissonDataSetHandler");
    throw;
  }
  return bValid;
}

/** Allocates randomizers for each dataset. Type of randomizer instantiated
    is determined by parameter settings. */
void PoissonDataSetHandler::SetRandomizers() {
  try {
    gvDataSetRandomizers.DeleteAllElements();
    gvDataSetRandomizers.resize(gParameters.GetNumDataSets(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
            gvDataSetRandomizers[0] = new PoissonTimeStratifiedRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
            gvDataSetRandomizers[0] = new PoissonSpatialStratifiedRandomizer(gParameters, gParameters.GetRandomizationSeed());
          else
            gvDataSetRandomizers[0] = new PoissonNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed());
          break;
      case HA_RANDOMIZATION :
          gvDataSetRandomizers[0] = new AlternateHypothesisRandomizer(gDataHub, gParameters.GetRandomizationSeed());
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
    x.AddCallpath("SetRandomizers()","PoissonDataSetHandler");
    throw;
  }
}
 
