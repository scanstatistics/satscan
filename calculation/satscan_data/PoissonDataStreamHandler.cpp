//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "PoissonDataStreamHandler.h"
#include "SaTScanData.h"
//---------------------------------------------------------------------------
const int POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY   = 15;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY    = 1;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH  = 7;
//---------------------------------------------------------------------------

/** constructor */
PoissonDataStreamHandler::PoissonDataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                         :DataStreamHandler(Data, pPrint) {}

/** destructor */
PoissonDataStreamHandler::~PoissonDataStreamHandler() {}

/** Converts passed string specifiying a population date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool PoissonDataStreamHandler::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate) {
  bool          bValidDate=true;
  int           iYear, iMonth, iDay, iPrecision=0;
  const char  * ptr;

  try {
    //determine precision
    ptr = strchr(sDateString, '/');
    while (ptr) {
         iPrecision++;
         ptr = strchr(++ptr, '/');
    }
    //scan string
    switch (iPrecision) {
      case 0  : iMonth = POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH;
                iDay = POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY;
                bValidDate = (sscanf(sDateString, "%d", &iYear) == 1 && iYear > 0);
                break;
      case 1  : iDay = POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY;
                bValidDate = (sscanf(sDateString, "%d/%d", &iYear, &iMonth) == 2 && iYear > 0 && iMonth > 0);
                break;
      case 2  : bValidDate = (sscanf(sDateString, "%d/%d/%d", &iYear, &iMonth, &iDay) == 3 && iYear > 0 && iMonth > 0 && iDay > 0);
                break;
      default : bValidDate = false;
    }
    if (! bValidDate)
      gpPrint->PrintInputWarning("Error: Invalid date '%s' in %s, record %ld.\n",
                                 sDateString, gpPrint->GetImpliedFileTypeString().c_str(), iRecordNumber);
    else {
      iYear = Ensure4DigitYear(iYear, const_cast<char*>(gParameters.GetStudyPeriodStartDate().c_str()), const_cast<char*>(gParameters.GetStudyPeriodEndDate().c_str()));
      switch (iYear) {
        case -1 : gpPrint->PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n");
                  gpPrint->PrintInputWarning("       to determine century for two digit year '%d' in %s, record %ld.\n",
                                                      iYear, gpPrint->GetImpliedFileTypeString().c_str(), iRecordNumber);
                  gpPrint->PrintInputWarning("       Please use four digit years.\n");
                  bValidDate = false;
        case -2 : gpPrint->PrintInputWarning("Error: Invalid year '%d' in %s, record %ld.\n",
                                             iYear, gpPrint->GetImpliedFileTypeString().c_str(), iRecordNumber);
                  bValidDate = false;
        default : if ((JulianDate = MDYToJulian(iMonth, iDay, iYear)) == 0) {
                    gpPrint->PrintInputWarning("Error: Invalid date '%s' in %s, record %ld.\n",
                                               sDateString, gpPrint->GetImpliedFileTypeString().c_str(), iRecordNumber);
                    bValidDate = false;
                  }
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertPopulationDateToJulian()","PoissonDataStreamHandler");
    throw;
  }
  return bValidDate;
}

/** returns new data gateway for real data */
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
        case PURELYSPATIALMONOTONE      :
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
          Interface.SetTimeTrend(&(const_cast<RealDataStream&>(thisStream)).GetTimeTrend());
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

/** returns new data gateway for simulation data */
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
        case PURELYSPATIALMONOTONE      :
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
          Interface.SetTimeTrend(&(const_cast<SimulationDataStream&>(thisSimulationStream).GetTimeTrend()));
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

/** Returns a collection of cloned randomizers maintained by data stream handler.
    All previous elements of list are deleted. */
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

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& PoissonDataStreamHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataStreams(); ++t)
    Container.push_back(new SimulationDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));

  switch (gParameters.GetAnalysisType()) {
    case PURELYSPATIAL :
        for (size_t t=0; t < Container.size(); ++t)
          Container[t]->AllocateCasesArray();
        break;
    case PURELYSPATIALMONOTONE :
        ZdGenerateException("GetSimulationDataContainer() not implemented for purely spatial monotone analysis.","GetSimulationDataContainer()");
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
          if (gParameters.GetIncludePurelyTemporalClusters())
            Container[t]->AllocatePTCasesArray();
        }
        break;
    default :
        ZdGenerateException("Unknown analysis type '%d'.","GetSimulationDataContainer()", gParameters.GetAnalysisType());
  };
  return Container;
}

/** randomizes each data streams */
void PoissonDataStreamHandler::RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     gvDataStreamRandomizers[t]->RandomizeData(*gvDataStreams[t], *SimDataContainer[t], iSimulationNumber);
}

/** */
bool PoissonDataStreamHandler::ReadData() {
  try {
    SetRandomizers();
    for (size_t t=0; t < GetNumStreams(); ++t) {
       if (GetNumStreams() == 1)
         gpPrint->SatScanPrintf("Reading the population file\n");
       else
         gpPrint->SatScanPrintf("Reading input stream %u population file\n", t + 1);
       if (!ReadPopulationFile(t))
         return false;
       if (GetNumStreams() == 1)
         gpPrint->SatScanPrintf("Reading the case file\n");
       else
         gpPrint->SatScanPrintf("Reading input stream %u case file\n", t + 1);
       if (!ReadCaseFile(t))
         return false;
       GetStream(t).CheckPopulationDataCases(gDataHub);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()","PoissonDataStreamHandler");
    throw;
  }
  return true;
}

/** Read the population file.
    The number of category variables is determined by the first record.
    Any records deviating from this number will cause an error.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues. The tract-id MUST match
    one read in ReadCoordinatesFile().
    Return value: true = success, false = errors encountered */
bool PoissonDataStreamHandler::ReadPopulationFile(size_t tStream) {
  int                           iCategoryIndex;
  bool                          bValid=true, bEmpty=true;
  tract_t                       TractIdentifierIndex;
  float                         fPopulation;
  Julian                        PopulationDate;
  FILE                        * fp=0; // Ptr to population file
  std::vector<Julian>           vPopulationDates;
  std::vector<Julian>::iterator itrdates;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];
    gpPrint->SetImpliedInputFileType(BasePrint::POPFILE, (GetNumStreams() == 1 ? 0 : tStream + 1));
    StringParser Parser(*gpPrint);

    if ((fp = fopen(gParameters.GetPopulationFileName(tStream + 1).c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open population file:\n'%s'.\n",
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
            gpPrint->PrintInputWarning("Error: Record %ld of %s missing date.\n",
                                       Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
        }
        if (!ConvertPopulationDateToJulian(Parser.GetWord(1), Parser.GetReadCount(), PopulationDate)) {
            bValid = false;
            continue;
        }
        //if date is unique, add it to the list in sorted order
        itrdates = lower_bound(vPopulationDates.begin(), vPopulationDates.end(), PopulationDate);
        if (! (itrdates != vPopulationDates.end() && (*itrdates) == PopulationDate))
          vPopulationDates.insert(itrdates, PopulationDate);
    }

    //2nd pass, read data in structures.
    if (bValid && !bEmpty) {
      //Set tract handlers population date structures since we already now all the dates from above.
      thisStream.GetPopulationData().SetupPopDates(vPopulationDates, gParameters.GetStudyPeriodStartDateAsJulian(),
                                                   gParameters.GetStudyPeriodEndDateAsJulian(), gpPrint);
      //reset for second read
      fseek(fp, 0L, SEEK_SET);
      //We can ignore error checking for population date and population since we already did this above.
      while (Parser.ReadString(fp)) {
          if (!Parser.HasWords()) // Skip Blank Lines
            continue;
          ConvertPopulationDateToJulian(Parser.GetWord(1), Parser.GetReadCount(), PopulationDate);
          if (!Parser.GetWord(2)) {
            gpPrint->PrintInputWarning("Error: Record %d of %s missing population.\n",
                                       Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          if (sscanf(Parser.GetWord(2), "%f", &fPopulation) != 1) {
            gpPrint->PrintInputWarning("Error: Population value '%s' in record %ld, of %s, is not a number.\n",
                                       Parser.GetWord(2), Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          //validate that population is not negative or exceeding type precision
          if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
            if (strstr(Parser.GetWord(2), "-"))
              gpPrint->PrintInputWarning("Error: Negative population in record %ld of %s.\n",
                                         Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
            else
              gpPrint->PrintInputWarning("Error: Population '%s' exceeds maximum value of %i in record %ld of %s.\n",
                                         Parser.GetWord(2), std::numeric_limits<float>::max(),
                                         Parser.GetReadCount(), gpPrint->GetImpliedFileTypeString().c_str());
            bValid = false;
            continue;
          }
          //Scan for covariates to create population categories or find index.
          //First category created sets precedence as to how many covariates remaining records must have.
          if ((iCategoryIndex = thisStream.GetPopulationData().MakePopulationCategory(Parser, 3, *gpPrint)) == -1) {
            bValid = false;
            continue;
          }
          //Validate that tract identifer is one of those defined in the coordinates file.
          if ((TractIdentifierIndex = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
            gpPrint->PrintInputWarning("Error: Unknown location identifier in %s, record %ld.\n",
                                       gpPrint->GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
            gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
            bValid = false;
            continue;
          }
          //Add population count for this tract/category/year
          thisStream.GetPopulationData().AddCategoryToTract(TractIdentifierIndex, iCategoryIndex, PopulationDate, fPopulation);
      }
    }
    //close file pointer
    fclose(fp); fp=0;
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'population file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: %s contains no data.\n", gpPrint->GetImpliedFileTypeString().c_str());
      bValid = false;
    }
    if (!thisStream.GetPopulationData().CheckZeroPopulations(stderr, gpPrint))
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

/** allocates randomizers for each data stream */
void PoissonDataStreamHandler::SetRandomizers() {
  try {
    gvDataStreamRandomizers.resize(gParameters.GetNumDataStreams(), 0);
    switch (gParameters.GetSimulationType()) {
      case STANDARD :
          if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
            gvDataStreamRandomizers[0] = new PoissonTimeStratifiedRandomizer(gParameters);
          else
            gvDataStreamRandomizers[0] = new PoissonNullHypothesisRandomizer(gParameters);
          break;
      case HA_RANDOMIZATION :
          gvDataStreamRandomizers[0] = new AlternateHypothesisRandomizer(gDataHub);
          break;
      case FILESOURCE :
          gvDataStreamRandomizers[0] = new FileSourceRandomizer(gDataHub);
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
 
