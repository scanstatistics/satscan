//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PoissonDataStreamHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"

/** class constructor */
PoissonDataStreamHandler::PoissonDataStreamHandler(CSaTScanData& Data, BasePrint& Print)
                         :DataStreamHandler(Data, Print) {}

/** class destructor */
PoissonDataStreamHandler::~PoissonDataStreamHandler() {}

/** Converts passed string specifiying a population date to a julian date using
    DateStringParser object. Since we accumulate errors/warnings when reading
    input files, indication of a bad date is returned as false and message
    sent to print direction. Caller is responsible for ensuring that passed
    'const char *' points to a valid string. */
bool PoissonDataStreamHandler::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber,
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
    x.AddCallpath("ConvertPopulationDateToJulian()","PoissonDataStreamHandler");
    throw;
  }
  return bValidDate;
}

/** Instead reading population data from a population, assigns the same arbitrary population
    date and population for each location specified in coordinates file. This routine is
    intended to be used by purely temporal analyses when the population file is omitted. */
bool PoissonDataStreamHandler::CreatePopulationData(size_t tStream) {
  float                                                 fPopulation = 1000; /** arbitrarily selected population */
  std::vector<std::pair<Julian, DatePrecisionType> >    vprPopulationDates;
  const TractHandler&                                   theTracts = *(gDataHub.GetTInfo());
  tract_t                                               t, tNumTracts = theTracts.tiGetNumTracts();
  int                                                   iCategoryIndex;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    // Make the data stream aggregate categories - this will way the reading of case data can proceed without problems.
    // Normally the population data dictates all possible population catgories and the case file data must follow suit.
    thisStream.SetAggregateCategories(true);
    iCategoryIndex = 0; /* with aggregation, only one population category with index of zero */
    // Use the same arbitrarily selected population date for each location - we'll use the study period start date.
    vprPopulationDates.push_back(std::pair<Julian, DatePrecisionType>(gDataHub.GetStudyPeriodStartDate(), YEAR));
    thisStream.GetPopulationData().SetPopulationDates(vprPopulationDates, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    // for each location, assign the same population count and date
    for (t=0; t < tNumTracts; ++t)
      thisStream.GetPopulationData().AddCategoryToTract(t, iCategoryIndex, vprPopulationDates.back(), fPopulation);
  }
  catch (ZdException &x) {
    x.AddCallpath("CreatePopulationData()","PoissonDataStreamHandler");
    throw;
  }
  return true;
}

/** Returns newly allocated data gateway object that references structures
    utilized in calculating most likely clusters (real data) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbtractDataStreamGateway * PoissonDataStreamHandler::GetNewDataGateway() const {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      const RealDataStream& thisStream = *gvDataStreams[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          Interface.SetCaseArray(thisStream.GetCaseArray());
          Interface.SetNCCaseArray(thisStream.GetNCCaseArray());
          Interface.SetPTCaseArray(thisStream.GetPTCasesArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          Interface.SetNCMeasureArray(thisStream.GetNCMeasureArray());
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          Interface.SetTimeTrend(&thisStream.GetTimeTrend());
          break;
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewDataGateway()","PoissonDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** Returns newly allocated data gateway object that references structures
    utilized in performing simulations (Monte Carlo) for the Poisson
    probablity model, analysis type and possibly inclusion purely temporal
    clusters. Caller is responsible for destructing returned object. */
AbtractDataStreamGateway * PoissonDataStreamHandler::GetNewSimulationDataGateway(const SimulationDataContainer_t& Container) const {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      const RealDataStream& thisRealStream = *gvDataStreams[t];
      const SimulationDataStream& thisSimulationStream = *Container[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisRealStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisRealStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
          Interface.SetCaseArray(thisSimulationStream.GetCaseArray());
          Interface.SetMeasureArray(thisRealStream.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisSimulationStream.GetPTCasesArray());
          Interface.SetPTMeasureArray(thisRealStream.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisSimulationStream.GetCaseArray());
          Interface.SetMeasureArray(thisRealStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisSimulationStream.GetPTCasesArray());
            Interface.SetPTMeasureArray(thisRealStream.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          Interface.SetCaseArray(thisSimulationStream.GetCaseArray());
          Interface.SetNCCaseArray(thisSimulationStream.GetNCCaseArray());
          Interface.SetPTCaseArray(thisSimulationStream.GetPTCasesArray());
          Interface.SetMeasureArray(thisRealStream.GetMeasureArray());
          Interface.SetNCMeasureArray(thisRealStream.GetNCMeasureArray());
          Interface.SetPTMeasureArray(thisRealStream.GetPTMeasureArray());
          Interface.SetTimeTrend(&thisSimulationStream.GetTimeTrend());
          break;
        default :
          ZdGenerateException("Unknown analysis type '%d'.","GetNewDataGateway()",gParameters.GetAnalysisType());
      };
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","PoissonDataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** Returns a collection of cloned randomizers maintained internally. Number of
    randomizers cloned equals the number of data streams. All previous elements
    of container are deleted. */
RandomizerContainer_t& PoissonDataStreamHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  ZdPointerVector<AbstractRandomizer>::const_iterator itr;

  try {
    Container.DeleteAllElements();
    for (itr=gvDataStreamRandomizers.begin(); itr != gvDataStreamRandomizers.end(); ++itr)
       Container.push_back((*itr)->Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","PoissonDataStreamHandler");
    throw;
  }
  return Container;
}

/** Return a collection of new created simulation data stream objects for the
    Poisson probability model, analysis type and possibly inclusion purely
    temporal clusters.                                                        */
SimulationDataContainer_t& PoissonDataStreamHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataStreams(); ++t)
    Container.push_back(new SimulationDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

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
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** Refined process for reading input data from files into respective data
    streams. For the Poisson probability model, inputs file of interest are the
    case and population. Prior to reading input data randomization objects are
    allocated. Echos progress and warnings/errors during read to BasePrint
    object. It is not recommended to call this function more than once. */
bool PoissonDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       if (gParameters.UsePopulationFile()) { //read population data file
         if (GetNumStreams() == 1) gPrint.SatScanPrintf("Reading the population file\n");
         else gPrint.SatScanPrintf("Reading the population file for data set %u\n", t + 1);
         if (!ReadPopulationFile(t)) return false;
       }
       else { //create population data without input data
         if (GetNumStreams() == 1) gPrint.SatScanPrintf("Creating the population\n");
         else gPrint.SatScanPrintf("Creating the population for data set %u\n", t + 1);
         if (!CreatePopulationData(t)) return false;
       }
       //read case data file
       if (GetNumStreams() == 1) gPrint.SatScanPrintf("Reading the case file\n");
       else gPrint.SatScanPrintf("Reading the case file for data set %u\n", t + 1);
       if (!ReadCaseFile(t)) return false;
       //validate population data against case data (if population was read from file)  
       if (gParameters.UsePopulationFile()) GetStream(t).CheckPopulationDataCases(gDataHub);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","PoissonDataStreamHandler");
    throw;
  }
  return true;
}

/** Reads data from population file into data stream structures. Reading of the
    file is done in two passes:
    - first pass  ; Determines all unique population dates specified in file.
                    Then adjusts list in accordance with study period and later
                    needs for interpolation.
    - second pass ; Updates PopulationData object (of data stream) structures for
                    each population record. Each record is validated with
                    warnings/errors printed to BasePrint object.
    Returns indication of successful read (true = valid, false = errors).
    Location identifiers of population file are matched to location identifiers
    specifed in coordinates file; so this function should not be called before
    the coordinates file has been read.                                         */
bool PoissonDataStreamHandler::ReadPopulationFile(size_t tStream) {
  int                                                           iCategoryIndex;
  bool                                                          bValid=true, bEmpty=true;
  tract_t                                                       TractIdentifierIndex;
  float                                                         fPopulation;
  Julian                                                        PopulationDate;
  FILE                                                        * fp=0; // Ptr to population file
  std::pair<Julian, DatePrecisionType>                          prPopulationDate;
  std::vector<std::pair<Julian, DatePrecisionType> >            vprPopulationDates;
  std::vector<std::pair<Julian, DatePrecisionType> >::iterator  itr;
  ComparePopulationDates                                        JulianCompare;  

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    gPrint.SetImpliedInputFileType(BasePrint::POPFILE, (GetNumStreams() == 1 ? 0 : tStream + 1));
    StringParser Parser(gPrint);

    if ((fp = fopen(gParameters.GetPopulationFileName(tStream + 1).c_str(), "r")) == NULL) {
      gPrint.SatScanPrintWarning("Error: Could not open the population file:\n'%s'.\n",
                                   gParameters.GetPopulationFileName(tStream + 1).c_str());
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
        itr = std::lower_bound(vprPopulationDates.begin(), vprPopulationDates.end(), prPopulationDate, JulianCompare);
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
      thisStream.GetPopulationData().SetPopulationDates(vprPopulationDates,
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
          if ((iCategoryIndex = thisStream.GetPopulationData().MakePopulationCategory(Parser, 3, gPrint)) == -1) {
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
          thisStream.GetPopulationData().AddCategoryToTract(TractIdentifierIndex, iCategoryIndex, prPopulationDate, fPopulation);
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
    if (!thisStream.GetPopulationData().CheckZeroPopulations(stderr, gPrint))
      return false;
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadPopulationFile()","PoissonDataStreamHandler");
    throw;
  }
  return bValid;
}

/** Allocates randomizers for each data stream. Type of randomizer instantiated
    is determined by parameter settings. */
void PoissonDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.DeleteAllElements();
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
            gvDataStreamRandomizers[0] = new PoissonTimeStratifiedRandomizer(gParameters);
          else if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
            gvDataStreamRandomizers[0] = new PoissonSpatialStratifiedRandomizer(gParameters);
          else
            gvDataStreamRandomizers[0] = new PoissonNullHypothesisRandomizer(gParameters);
          break;
      case HA_RANDOMIZATION :
          gvDataStreamRandomizers[0] = new AlternateHypothesisRandomizer(gDataHub);
          break;
      case FILESOURCE :
          gvDataStreamRandomizers[0] = new FileSourceRandomizer(gParameters);
          break;
      default :
          ZdGenerateException("Unknown simulation type '%d'.","SetRandomizers()", gParameters.GetSimulationType());
    };
    //create more if needed
    for (size_t t=1; t < gParameters.GetNumDataStreams(); ++t)
       gvDataStreamRandomizers[t] = gvDataStreamRandomizers[0]->Clone();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRandomizers()","PoissonDataStreamHandler");
    throw;
  }
}
 
