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

/** allocates structures used during simulations - based particularly upon analysis type */
void PoissonDataStreamHandler::AllocateSimulationStructures() {
  try {
    switch (gParameters.GetAnalysisType()) {
       case PURELYSPATIAL :
         AllocateSimulationCases();
         break;
       case PURELYSPATIALMONOTONE :
         ZdGenerateException("AllocateSimulationStructures() not implemented for purely spatial monotone analysis.","AllocateSimulationStructures()");
       case PURELYTEMPORAL :
       case PROSPECTIVEPURELYTEMPORAL :
         AllocateSimulationCases();
         AllocatePTSimulationCases();
         break;
       case SPACETIME :
       case PROSPECTIVESPACETIME :
         AllocateSimulationCases();
         if (gParameters.GetIncludePurelyTemporalClusters())
           AllocatePTSimulationCases();
         break;
       case SPATIALVARTEMPTREND :
         AllocateSimulationCases();
         AllocatePTSimulationCases();
         AllocateNCSimulationCases();
         break;
      default :
         ZdGenerateException("Unknown analysis type '%d'.","AllocateSimulationStructures()", gParameters.GetAnalysisType());
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","PoissonDataStreamHandler");
    throw;
  }
}

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
AbtractDataStreamGateway * PoissonDataStreamHandler::GetNewDataGateway() {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      DataStream & thisStream = gvDataStreams[t];
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

/** returns new data gateway for simulation data */
AbtractDataStreamGateway * PoissonDataStreamHandler::GetNewSimulationDataGateway() {
  AbtractDataStreamGateway    * pDataStreamGateway=0;
  DataStreamInterface           Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = GetNewDataGatewayObject();
    for (t=0; t < gvDataStreams.size(); ++t) {
      //get reference to stream
      DataStream & thisStream = gvDataStreams[t];
      //set total cases and measure
      Interface.SetTotalCasesCount(thisStream.GetTotalCases());
      Interface.SetTotalMeasureCount(thisStream.GetTotalMeasure());
      //set pointers to data structures
      switch (gParameters.GetAnalysisType()) {
        case PURELYSPATIAL              :
        case PURELYSPATIALMONOTONE      :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          break;
        case PROSPECTIVEPURELYTEMPORAL  :
        case PURELYTEMPORAL             :
          Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          break;
        case SPACETIME                  :
        case PROSPECTIVESPACETIME       :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          if (gParameters.GetIncludePurelyTemporalClusters()) {
            Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
            Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          }
          break;
        case SPATIALVARTEMPTREND        :
          Interface.SetCaseArray(thisStream.GetSimCaseArray());
          Interface.SetNCCaseArray(thisStream.GetNCSimCaseArray());
          Interface.SetPTCaseArray(thisStream.GetPTSimCasesArray());
          Interface.SetMeasureArray(thisStream.GetMeasureArray());
          Interface.SetNCMeasureArray(thisStream.GetNCMeasureArray());
          Interface.SetPTMeasureArray(thisStream.GetPTMeasureArray());
          Interface.SetTimeTrend(&thisStream.GetSimTimeTrend());
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

/** randomizes each data streams */
void PoissonDataStreamHandler::RandomizeData(unsigned int iSimulationNumber) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     gvDataStreamRandomizers[t]->RandomizeData(gvDataStreams[t], iSimulationNumber);
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
       GetStream(t).CheckPopulationDataCases(gData);
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
    DataStream & thisStream = gvDataStreams[tStream];
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
      thisStream.GetPopulationData().SetupPopDates(vPopulationDates,
                             const_cast<CParameters&>(gParameters).GetStudyPeriodStartDateAsJulian(),
                             const_cast<CParameters&>(gParameters).GetStudyPeriodEndDateAsJulian(), gpPrint);
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
          if ((TractIdentifierIndex = gData.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
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
          gvDataStreamRandomizers[0] = new AlternateHypothesisRandomizer(gData);
          break;
      case FILESOURCE :
          gvDataStreamRandomizers[0] = new FileSourceRandomizer(gData);
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
 
