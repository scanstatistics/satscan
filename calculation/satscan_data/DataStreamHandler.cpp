//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStreamHandler.h"
#include "SaTScanData.h"
//---------------------------------------------------------------------------
const int POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY   = 15;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY    = 1;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH  = 7;
//---------------------------------------------------------------------------

/** constructor */
DataStreamHandler::DataStreamHandler(CSaTScanData & Data, BasePrint * pPrint)
                  : gData(Data), gParameters(Data.GetParameters()), gpPrint(pPrint) {
  try {
    Setup();
  }
  catch(ZdException &x) {
    x.AddCallpath("constructor()","DataStreamHandler");
    throw;
  }
}

/** destructor */
DataStreamHandler::~DataStreamHandler() {}

/** Allocates two dimensional array for case counts (number of time intervals by number of tracts). */
void DataStreamHandler::AllocateNCSimCases() {
  size_t        t;

  try {
    for (t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimulationNCCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateNCSimCases()","DataStreamHandler");
    throw;
  }
}

/** allocates cases structures for stream*/
void DataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream].AllocateCasesArray();
    if (gParameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION)
      gvDataStreams[iStream].AllocateCategoryCasesArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","DataStreamHandler");
    throw;
  }
}

/** allocates cases structures for stream*/
void DataStreamHandler::AllocateControlStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream].AllocateControlsArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateControlStructures()","DataStreamHandler");
    throw;
  }
}

/** Allocates two dimensional array for case counts (number of time intervals by number of tracts). */
void DataStreamHandler::AllocateSimulationCases() {
  size_t        t;

  try {
    for (t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimulationCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationCases()","DataStreamHandler");
    throw;
  }
}

/** Allocates two dimensional array for expected case counts (number of time intervals by number of tracts). */
void DataStreamHandler::AllocateSimulationMeasure() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationMeasure()","DataStreamHandler");
    throw;
  }
}

/** Allocates array for case counts in each time interval (summoned over all tracts). */
void DataStreamHandler::AllocatePTSimCases() {
  size_t        t;

  try {
    for (t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocateSimulationPTCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTSimCases()","DataStreamHandler");
    throw;
  }
}

/** Allocates array for expected case counts in each time interval (summoned over all tracts). */
void DataStreamHandler::AllocateSimulationPTMeasure() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].AllocatePTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationPTMeasure()","DataStreamHandler");
    throw;
  }
}

/** Converts passed string specifiying a count date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool DataStreamHandler::ConvertCountDateToJulian(StringParser & Parser, const char * szDescription, Julian & JulianDate) {
  bool          bValidDate=true;
  int           iYear, iMonth=1, iDay=1, iPrecision=1;
  const char  * ptr;


  if (gParameters.GetPrecisionOfTimesType() == NONE)
    JulianDate = gData.GetStudyPeriodStartDate();
  else {
    //read and validate date
    if (!Parser.GetWord(2)) {
      gpPrint->PrintInputWarning("Error: Record %ld in %s file does not contain a date.\n", Parser.GetReadCount(), szDescription);
      return false;
    }
    //determine precision - must be as accurate as time interval units
    ptr = strchr(Parser.GetWord(2), '/');
    while (ptr) {
         iPrecision++;
         ptr = strchr(++ptr, '/');
    }
    if (iPrecision < gParameters.GetTimeIntervalUnitsType()) {
      gpPrint->PrintInputWarning("Error: Date '%s' of record %ld in %s file must be precise to %s, as specified by time interval units.\n",
                                 Parser.GetWord(2), Parser.GetReadCount(), szDescription,
                                 gParameters.GetDatePrecisionAsString(gParameters.GetTimeIntervalUnitsType()));
      return false;
    }
    switch (iPrecision) {
      //case NONE  : JulianToMDY(&uiMonth, &uiDay, &uiYear, m_nStartDate); iScanPrecision = 3; break;
      case YEAR  : bValidDate = (sscanf(Parser.GetWord(2), "%d", &iYear) == 1 && iYear > 0); break;
      case MONTH : bValidDate = (sscanf(Parser.GetWord(2), "%d/%d", &iYear, &iMonth) == 2 && iYear > 0 && iMonth > 0); break;
      case DAY   : bValidDate = (sscanf(Parser.GetWord(2), "%d/%d/%d", &iYear, &iMonth, &iDay) == 3 && iYear > 0 && iMonth > 0 && iDay > 0); break;
      default    : bValidDate = false;
    };
    if (! bValidDate)
      gpPrint->PrintInputWarning("Error: Invalid date '%s' in %s file, record %ld.\n", Parser.GetWord(2), szDescription, Parser.GetReadCount());
    else {
      //Ensure four digit years
      iYear = Ensure4DigitYear(iYear, gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
      switch (iYear) {
        case -1 : gpPrint->PrintInputWarning("Error: Due to study period greater than 100 years, unable\n"
                                             "       to convert two digit year '%d' in %s file, record %ld.\n"
                                             "       Please use four digit years.\n", iYear, szDescription, Parser.GetReadCount());
                  return false;
        case -2 : gpPrint->PrintInputWarning("Error: Invalid year '%d' in %s file, record %ld.\n", iYear, szDescription, Parser.GetReadCount());
                  return false;
      }
      //validate that date is between study period start and end dates
      JulianDate = MDYToJulian(iMonth, iDay, iYear);
      if (!(gData.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gData.GetStudyPeriodEndDate())) {
        gpPrint->PrintInputWarning("Error: Date '%s' in record %ld of %s file is not\n", Parser.GetWord(2), Parser.GetReadCount(), szDescription);
        gpPrint->PrintInputWarning("       within study period beginning %s and ending %s.\n",
                                   gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
        return false;
      }
    }  
  }
  return bValidDate;
}

/** Converts passed string specifiying a population date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool DataStreamHandler::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate) {
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
      gpPrint->PrintInputWarning("Error: Invalid date '%s' in population file, record %ld.\n", sDateString, iRecordNumber);
    else {
      iYear = Ensure4DigitYear(iYear, const_cast<char*>(gParameters.GetStudyPeriodStartDate().c_str()), const_cast<char*>(gParameters.GetStudyPeriodEndDate().c_str()));
      switch (iYear) {
        case -1 : gpPrint->PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n");
                  gpPrint->PrintInputWarning("       to determine century for two digit year '%d' in population file, record %ld.\n",
                                                      iYear, iRecordNumber);
                  gpPrint->PrintInputWarning("       Please use four digit years.\n");
                  bValidDate = false;
        case -2 : gpPrint->PrintInputWarning("Error: Invalid year '%d' in population file, record %ld.\n", iYear, iRecordNumber);
                  bValidDate = false;
        default : if ((JulianDate = MDYToJulian(iMonth, iDay, iYear)) == 0) {
                    gpPrint->PrintInputWarning("Error: Invalid date '%s' in population file, record %ld.\n", sDateString, iRecordNumber);
                    bValidDate = false;
                  }
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertPopulationDateToJulian()","DataStreamHandler");
    throw;
  }
  return bValidDate;
}

/** frees all memory allocated to simulation strcutures */
void DataStreamHandler::FreeSimulationStructures() {
  size_t        t;

  try {
    for (t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].FreeSimulationStructures();
  }
  catch (ZdException &x) {
    x.AddCallpath("FreeSimulationStructures()","DataStreamHandler");
    throw;
  }
}

/** returns new data gateway for real data */
DataStreamGateway * DataStreamHandler::GetNewDataGateway() {
  DataStreamGateway           * pDataStreamGateway=0;
  DataStreamInterface           Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = new DataStreamGateway();
    for (t=0; t < gvDataStreams.size(); ++t) {
      Interface.Set(gvDataStreams[t], gParameters);
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewDataGateway()","DataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** returns new data gateway for simulation data */
DataStreamGateway * DataStreamHandler::GetNewSimulationDataGateway() {
  DataStreamGateway           * pDataStreamGateway=0;
  SimulationDataStreamInterface Interface(gData.GetNumTimeIntervals(), gData.GetNumTracts());
  size_t                        t;

  try {
    pDataStreamGateway = new DataStreamGateway();
    for (t=0; t < gvDataStreams.size(); ++t) {
      Interface.Set(gvDataStreams[t], gParameters);
      pDataStreamGateway->AddDataStreamInterface(Interface);
    }
  }
  catch (ZdException &x) {
    delete pDataStreamGateway;
    x.AddCallpath("GetNewSimulationDataGateway()","DataStreamHandler");
    throw;
  }  
  return pDataStreamGateway;
}

/** Attempts to parses passed string into tract identifier, count,
    and based upon settings, date and covariate information.
    Returns whether parse completed without errors. */
bool DataStreamHandler::ParseCountLine(PopulationData & thePopulation, const char*  szDescription,
                                       StringParser & Parser, tract_t& tid, count_t& nCount,
                                       Julian& nDate, int& iCategoryIndex) {
                                       
  int                          iCategoryOffSet, iScanPrecision;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gData.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gpPrint->PrintInputWarning("Error: Unknown location id in %s file, record %ld.\n", szDescription, Parser.GetReadCount());
      gpPrint->PrintInputWarning("       Location '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as count.\n", Parser.GetWord(1), Parser.GetReadCount(), szDescription);
       gpPrint->PrintInputWarning("       Count must be an integer.\n");
       return false;
      }
    }
    else {
      gpPrint->PrintInputWarning("Error: Record %ld in %s file does not contain %s count.\n", Parser.GetReadCount(), szDescription, szDescription);
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gpPrint->PrintInputWarning("Error: Negative count in record %ld of %s file.\n", Parser.GetReadCount(), szDescription);
      else
        gpPrint->PrintInputWarning("Error: Count '%s' exceeds maximum value of %ld in record %ld of %s file.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(), Parser.GetReadCount(), szDescription);
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, szDescription, nDate))
      return false;
    iCategoryOffSet = gParameters.GetPrecisionOfTimesType() == NONE ? 2 : 3;
    if (! ParseCovariates(thePopulation, iCategoryIndex, iCategoryOffSet, szDescription, Parser))
        return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCountLine()","DataStreamHandler");
    throw;
  }
  return true;
}

/** Parses count file data line to determine category index given covariates contained in line.*/
bool DataStreamHandler::ParseCovariates(PopulationData & thePopulation, int& iCategoryIndex, int iCovariatesOffset, const char*  szDescription, StringParser & Parser) {
  int                          iNumCovariatesScanned=0;
  std::vector<std::string>     vCategoryCovariates;
  const char                 * pCovariate;

  try {

    if (gParameters.GetProbabiltyModelType() == POISSON) {
      while ((pCovariate = Parser.GetWord(iNumCovariatesScanned + iCovariatesOffset)) != 0) {
           vCategoryCovariates.push_back(pCovariate);
           iNumCovariatesScanned++;
      }
      if (iNumCovariatesScanned != thePopulation.GetNumPopulationCategoryCovariates()) {
        gpPrint->PrintInputWarning("Error: Record %ld of case file contains %d covariate%s but the population file\n",
                                   Parser.GetReadCount(), iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"));
        gpPrint->PrintInputWarning("       defined the number of covariates as %d.\n", thePopulation.GetNumPopulationCategoryCovariates());
        return false;
      }
      //category should already exist
      if ((iCategoryIndex = thePopulation.GetPopulationCategoryIndex(vCategoryCovariates)) == -1) {
        gpPrint->PrintInputWarning("Error: Record %ld of case file refers to a population category that\n", Parser.GetReadCount());
        gpPrint->PrintInputWarning("       does not match an existing category as read from population file.");
        return false;
      }
    }
    else if (gParameters.GetProbabiltyModelType() == BERNOULLI) {
      //For the Bernoulli model, ignore covariates in the case and control files
      //All population categories are aggregated in one category.
      iCategoryIndex = 0;
    }
    else if (gParameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION) {
        //First category created sets precedence as to how many covariates remaining records must have.
        if ((iCategoryIndex = thePopulation.MakePopulationCategory(szDescription, Parser, iCovariatesOffset, *gpPrint)) == -1)
          return false;
    }
    else
      ZdGenerateException("Unknown probability model type '%d'.","ParseCovariates()", gParameters.GetProbabiltyModelType());
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCovariates()","DataStreamHandler");
    throw;
  }
  return true;
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataStreamHandler::ReadCaseFile(size_t tStream) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the case file\n");
    if ((fp = fopen(gParameters.GetCaseFileName(tStream + 1).c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open case file:\n'%s'.\n",
                                   gParameters.GetCaseFileName(tStream + 1).c_str());
      return false;
    }                                                                  
    gpPrint->SetImpliedInputFileType(BasePrint::CASEFILE);
    AllocateCaseStructures(tStream);
    bValid = ReadCounts(tStream, fp, "case");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadCaseFile()","DataStreamHandler");
    throw;
  }
  return bValid;
}

/** Read the control data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataStreamHandler::ReadControlFile(size_t tStream) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the control file\n");
    if ((fp = fopen(gParameters.GetControlFileName(tStream + 1).c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open control file:\n'%s'.\n",
                                   gParameters.GetControlFileName(tStream + 1).c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::CONTROLFILE);
    AllocateControlStructures(tStream);
    bValid = ReadCounts(tStream, fp, "control");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadControlFile()","DataStreamHandler");
    throw;
  }
  return bValid;
}

/** Read the count(either case or control) data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  int                                   i, j, iCategoryIndex;
  bool                                  bCaseFile, bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(gpPrint->GetImpliedInputFileType());
  std::string                           sBuffer;
  count_t                               Count, ** pCounts;
  ThreeDimensionArrayHandler<count_t> * pCategoryCounts;

  try {
    DataStream & thisStream = gvDataStreams[tStream];

    bCaseFile = !strcmp(szDescription, "case");
    pCounts = (bCaseFile ? thisStream.gpCasesHandler->GetArray() : thisStream.gpControlsHandler->GetArray());
    pCategoryCounts = (bCaseFile ? thisStream.gpCategoryCasesHandler : 0);

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(thisStream.gPopulation, szDescription, Parser, TractIndex, Count, Date, iCategoryIndex)) {
             //cumulatively add count to time by location structure
             pCounts[0][TractIndex] += Count;
             if (pCounts[0][TractIndex] < 0)
               SSGenerateException("Error: Total %s greater than maximum allowed of %ld.\n", "ReadCounts()",
                                   (bCaseFile ? "cases" : "controls"), std::numeric_limits<count_t>::max());
             for (i=1; Date >= gData.GetTimeIntervalStartTimes()[i]; ++i)
               pCounts[i][TractIndex] += Count;
             //record count as a case or control  
             if (bCaseFile)
               thisStream.gPopulation.AddCaseCount(iCategoryIndex, Count);
             else
               thisStream.gPopulation.AddControlCount(iCategoryIndex, Count);
             //record count in structure(s) based upon population category
             switch (gParameters.GetProbabiltyModelType()) {
               case POISSON              :
               case BERNOULLI            : break;
               case SPACETIMEPERMUTATION :
                 if (iCategoryIndex >= static_cast<int>(pCategoryCounts->Get3rdDimension()))
                   pCategoryCounts->ExpandThirdDimension(0);
                 pCategoryCounts->GetArray()[0][TractIndex][iCategoryIndex] += Count;
                 for (i=1; Date >= gData.GetTimeIntervalStartTimes()[i]; ++i)
                    pCategoryCounts->GetArray()[i][TractIndex][iCategoryIndex] += Count;
                 break;
             }
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->SatScanPrintWarning("Please see '%s file format' in the user guide for help.\n", szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->SatScanPrintWarning("Error: %s file does not contain data.\n", szDescription);
      bValid = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","DataStreamHandler");
    throw;
  }
  return bValid;
}

/** Read the population file.
    The number of category variables is determined by the first record.
    Any records deviating from this number will cause an error.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues. The tract-id MUST match
    one read in ReadCoordinatesFile().
    Return value: true = success, false = errors encountered */
bool DataStreamHandler::ReadPopulationFile(size_t tStream) {
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
    gpPrint->SetImpliedInputFileType(BasePrint::POPFILE);
    StringParser Parser(gpPrint->GetImpliedInputFileType());

    gpPrint->SatScanPrintf("Reading the population file\n");
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
            gpPrint->PrintInputWarning("Error: Record %ld of population file missing date.\n", Parser.GetReadCount());
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
      thisStream.gPopulation.SetupPopDates(vPopulationDates,
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
            gpPrint->PrintInputWarning("Error: Record %d of population file missing population.\n", Parser.GetReadCount());
            bValid = false;
            continue;
          }
          if (sscanf(Parser.GetWord(2), "%f", &fPopulation) != 1) {
            gpPrint->PrintInputWarning("Error: Population value '%s' in record %ld, of population file, is not a number.\n",
                                       Parser.GetWord(2), Parser.GetReadCount());
            bValid = false;
            continue;
          }
          //validate that population is not negative or exceeding type precision
          if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
            if (strstr(Parser.GetWord(2), "-"))
              gpPrint->PrintInputWarning("Error: Negative population in record %ld of population file.\n", Parser.GetReadCount());
            else
              gpPrint->PrintInputWarning("Error: Population '%s' exceeds maximum value of %i in record %ld of population file.\n",
                                         Parser.GetWord(2), std::numeric_limits<float>::max(), Parser.GetReadCount());
            bValid = false;
            continue;
          }
          //Scan for covariates to create population categories or find index.
          //First category created sets precedence as to how many covariates remaining records must have.
          if ((iCategoryIndex = thisStream.gPopulation.MakePopulationCategory("population", Parser, 3, *gpPrint)) == -1) {
            bValid = false;
            continue;
          }
          //Validate that tract identifer is one of those defined in the coordinates file.
          if ((TractIdentifierIndex = gData.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
            gpPrint->PrintInputWarning("Error: Unknown location identifier in population file, record %ld.\n", Parser.GetReadCount());
            gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
            bValid = false;
            continue;
          }
          //Add population count for this tract/category/year
          thisStream.gPopulation.AddCategoryToTract(TractIdentifierIndex, iCategoryIndex, PopulationDate, fPopulation);
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
      gpPrint->PrintWarningLine("Error: Population file contains no data.\n");
      bValid = false;
    }
    if (!thisStream.gPopulation.CheckZeroPopulations(stderr, gpPrint))
      return false;
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadPopulationFile()","DataStreamHandler");
    throw;
  }
  return bValid;
}

/** reports whether any data stream has cases with a zero population. */
void DataStreamHandler::ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
    gvDataStreams[t].GetPopulationData().ReportZeroPops(Data, pDisplay, pPrintDirection);
}

/** sets temporal simulation case array from data in simulation case array */
void DataStreamHandler::SetPurelyTemporalSimCases() {
  try {
    for (size_t t=0; t < gvDataStreams.size(); ++t)
       gvDataStreams[t].SetPTSimCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimCases()","DataStreamHandler");
    throw;
  }
}

/** internal initialization */
void DataStreamHandler::Setup() {
  unsigned int  i;

  try {
    for (i=0; i < gParameters.GetNumDataStreams(); ++i)
      gvDataStreams.push_back(DataStream(gData.GetNumTimeIntervals(), gData.GetNumTracts()));
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataStreamHandler");
    throw;
  }
}

