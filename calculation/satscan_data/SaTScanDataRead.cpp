#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"

#define MAXWORDBUFFER 256
const int POPULATION_DATE_PRECISION_MONTH_DEFAULT_DAY   = 15;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_DAY    = 1;
const int POPULATION_DATE_PRECISION_YEAR_DEFAULT_MONTH  = 7;

/** Allocates count structures */
void CSaTScanData::AllocateCaseStructures() {
  try {
    gpCasesHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 0);
    if (m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION || m_pParameters->GetProbabiltyModelType() == BERNOULLI)
      gpCategoryCasesHandler = new ThreeDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 1/*assume only one category -- i.e. no covariates*/, 0);
    if (m_pParameters->GetProbabiltyModelType() == BERNOULLI && m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
      gpCasesByTimeByCategoryHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, 1/*assume only one category -- i.e. no covariates*/, 0);
  }
  catch(ZdException &x) {
    delete gpCasesHandler; gpCasesHandler=0;
    delete gpCategoryCasesHandler; gpCategoryCasesHandler=0;
    delete gpCasesByTimeByCategoryHandler; gpCasesByTimeByCategoryHandler=0;
    throw;
  }
}

void CSaTScanData::AllocateControlStructures() {
  try {
    gpControlsHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 0);
    if (m_pParameters->GetProbabiltyModelType() == BERNOULLI)
      gpCategoryControlsHandler = new ThreeDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 1/*assume only one category -- i.e. no covariates*/, 0);
    if (m_pParameters->GetProbabiltyModelType() == BERNOULLI && m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
      gpControlsByTimeByCategoryHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, 1/*assume only one category -- i.e. no covariates*/, 0);
  }
  catch(ZdException &x) {
    delete gpControlsHandler; gpControlsHandler=0;
    delete gpCategoryControlsHandler; gpCategoryControlsHandler=0;
    delete gpControlsByTimeByCategoryHandler; gpControlsByTimeByCategoryHandler=0;
    throw;
  }
}

/** Converts passed string specifiying a population date to a julian date.
    Precision is determined by date formats( YYYY/MM/DD, YYYY/MM, YYYY, YY/MM/DD,
    YY/MM, YY ) which is the complete set of valid formats that SaTScan currently
    supports. Since we accumulate errors/warnings when reading input files,
    indication of a bad date is returned and any messages sent to print direction. */
bool CSaTScanData::ConvertPopulationDateToJulian(const char * sDateString, int iRecordNumber, Julian & JulianDate) {
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
      iYear = Ensure4DigitYear(iYear, const_cast<char*>(m_pParameters->GetStudyPeriodStartDate().c_str()), const_cast<char*>(m_pParameters->GetStudyPeriodEndDate().c_str()));
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
    x.AddCallpath("ConvertPopulationDateToJulian()","CSaTScanData");
    throw;
  }
  return bValidDate;
}

/** Attempts to parses passed string into tract identifier, count,
    and based upon settings, date and covariate information.
    Returns whether parse completed without errors. */
bool CSaTScanData::ParseCountLine(const char*  szDescription, StringParser & Parser,
                                  tract_t& tid, count_t& nCount, Julian& nDate, int& iCategoryIndex) {
  UInt   	               uiYear4, uiYear, uiMonth=1, uiDay=1;
  int                          iCategoryOffSet, iScanPrecision;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gpTInfo->tiGetTractIndex(Parser.GetWord(0))) == -1) {
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
    //read and validate date
    if (m_pParameters->GetPrecisionOfTimesType() > NONE && !Parser.GetWord(2)) {
      gpPrint->PrintInputWarning("Error: Record %ld in %s file does not contain a date, which is required for a precision of '%s'.\n",
                                 Parser.GetReadCount(), szDescription, m_pParameters->GetDatePrecisionAsString(m_pParameters->GetPrecisionOfTimesType()));
      return false;
    }
    switch (m_pParameters->GetPrecisionOfTimesType()) {
      case NONE  : JulianToMDY(&uiMonth, &uiDay, &uiYear, m_nStartDate); iScanPrecision = 3; break;
      case YEAR  : iScanPrecision = sscanf(Parser.GetWord(2), "%d", &uiYear); break;
      case MONTH : iScanPrecision = sscanf(Parser.GetWord(2), "%d/%d", &uiYear, &uiMonth); break;
      case DAY   : iScanPrecision = sscanf(Parser.GetWord(2), "%d/%d/%d", &uiYear, &uiMonth, &uiDay); break;
      default : ZdException::Generate("Unknown precision of time type '%d'.","ParseCountLine()", m_pParameters->GetPrecisionOfTimesType());
    };
    if (iScanPrecision < m_pParameters->GetPrecisionOfTimesType()) {
      gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file, could not be read as date with precision '%s'.\n",
                                 Parser.GetWord(2), Parser.GetReadCount(), szDescription,
                                 m_pParameters->GetDatePrecisionAsString(m_pParameters->GetPrecisionOfTimesType()));
      return false;
    }
    //Ensure four digit years 
    uiYear4 = Ensure4DigitYear(uiYear, m_pParameters->GetStudyPeriodStartDate().c_str(), m_pParameters->GetStudyPeriodEndDate().c_str());
    switch (uiYear4) {
      case -1 : gpPrint->PrintInputWarning("Error: Due to study period greater than 100 years, unable\n"
                                                    "       to convert two digit year '%d' in %s file, record %ld.\n"
                                                    "       Please use four digit years.\n", uiYear, szDescription, Parser.GetReadCount());
                return false;
      case -2 : gpPrint->PrintInputWarning("Error: Invalid year '%d' in %s file, record %ld.\n", uiYear, szDescription, Parser.GetReadCount());
                return false;
    }
    //validate that date is between study period start and end dates
    nDate = MDYToJulian(uiMonth, uiDay, uiYear4);
    if (!(m_nStartDate <= nDate && nDate <= m_nEndDate)) {
      gpPrint->PrintInputWarning("Error: Date '%s' in record %ld of %s file is not\n", Parser.GetWord(2), Parser.GetReadCount(), szDescription);
      gpPrint->PrintInputWarning("       within study period beginning %s and ending %s.\n",
                                 m_pParameters->GetStudyPeriodStartDate().c_str(), m_pParameters->GetStudyPeriodEndDate().c_str());
      return false;
    }
    iCategoryOffSet = m_pParameters->GetPrecisionOfTimesType() == NONE ? 2 : 3;
    if (! ParseCovariates(iCategoryIndex, iCategoryOffSet, szDescription, Parser.GetReadCount(), Parser))
        return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCountLine()","CSaTScanData");
    throw;
  }
  return true;
}

/** Parses count file data line to determine category index given covariates contained in line.*/
bool CSaTScanData::ParseCovariates(int& iCategoryIndex, int iCovariatesOffset, const char*  szDescription, int nRec, StringParser & Parser) {
  int                          iNumCovariatesScanned=0;
  std::vector<std::string>     vCategoryCovariates;
  const char                 * pCovariate;

  try {

    if (m_pParameters->GetProbabiltyModelType() == POISSON) {
      while ((pCovariate = Parser.GetWord(iNumCovariatesScanned + iCovariatesOffset)) != 0) {
           vCategoryCovariates.push_back(pCovariate);
           iNumCovariatesScanned++;
      }
      if (iNumCovariatesScanned != gPopulationCategories.GetNumPopulationCategoryCovariates()) {
        gpPrint->PrintInputWarning("Error: Record %ld of case file contains %d covariate%s but the population file\n",
                                   nRec, iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"));
        gpPrint->PrintInputWarning("       defined the number of covariates as %d.\n", gPopulationCategories.GetNumPopulationCategoryCovariates());
        return false;
      }
      //category should already exist
      if ((iCategoryIndex = gPopulationCategories.GetPopulationCategoryIndex(vCategoryCovariates)) == -1) {
        gpPrint->PrintInputWarning("Error: Record %ld of case file refers to a population category that\n", nRec);
        gpPrint->PrintInputWarning("       does not match an existing category as read from population file.");
        return false;
      }
    }
    else if (m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION || m_pParameters->GetProbabiltyModelType() == BERNOULLI) {
        //First category created sets precedence as to how many covariates remaining records must have.
        if ((iCategoryIndex = gPopulationCategories.MakePopulationCategory(szDescription, Parser, nRec, iCovariatesOffset, *gpPrint)) == -1)
          return false;
    }
    else
      ZdGenerateException("Unknown probability model type '%d'.","ParseCovariates()", m_pParameters->GetProbabiltyModelType());
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCovariates()","CSaTScanData");
    throw;
  }
  return true;
}

/** Reads cartesian coordinates into vector.
    Note: coordinate vector should already be sized to defined dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Tracks number of words successfully scanned, the caller of
    function will use this information to confirm that coordinates scanned is
    not less than defined dimensions. The reason we don't check scanned dimensions
    here is that a generic error message could not be implemented. */
bool CSaTScanData::ReadCartesianCoordinates(StringParser & Parser, std::vector<double>& vCoordinates, int & iScanCount,
                                            int iWordOffSet, const char * sSourceFile) {
  const char  * pCoordinate;
  int           i;

  for (i=0, iScanCount=0; i < m_pParameters->GetDimensionsOfData(); ++i, ++iWordOffSet)
     if ((pCoordinate = Parser.GetWord(iWordOffSet)) != 0) {
       if (sscanf(pCoordinate, "%lf", &(vCoordinates[i])))
         iScanCount++; //track num successful scans, caller of function wants this information
       else {
         //unable to read word as double, print error to print direction and return false
         gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as ",
                                             pCoordinate, Parser.GetReadCount(), sSourceFile);
         //we can be specific about which dimension we are attending to read to                                    
         if (i < 2)
           gpPrint->PrintInputWarning("%s-coordinate.\n", (i == 0 ? "x" : "y"));
         else if (m_pParameters->GetDimensionsOfData() == 3)
           gpPrint->PrintInputWarning("z-coordinate.\n");
         else
           gpPrint->PrintInputWarning("z%d-coordinate.\n", i - 1);
         return false;
       }
     }
  return true;          
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadCaseFile() {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the case file\n");
    if ((fp = fopen(m_pParameters->GetCaseFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open case file:\n'%s'.\n",
                                   m_pParameters->GetCaseFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::CASEFILE);
    AllocateCaseStructures();
    bValid = ReadCounts(fp, "case");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadCaseFile()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the control data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadControlFile() {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the control file\n");
    if ((fp = fopen(m_pParameters->GetControlFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open control file:\n'%s'.\n",
                                   m_pParameters->GetControlFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::CONTROLFILE);
    AllocateControlStructures();
    bValid = ReadCounts(fp, "control");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadControlFile()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the geographic data file. Calls particular function for coordinate type. */
bool CSaTScanData::ReadCoordinatesFile() {
  bool          bReturn;
  FILE        * fp=0; // Ptr to coordinates file

  try {
    gpPrint->SatScanPrintf("Reading the geographic coordinates file\n");
    if ((fp = fopen(m_pParameters->GetCoordinatesFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Coordinates file '%s' could not be opened.\n",
                                   m_pParameters->GetCoordinatesFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::COORDFILE);

    switch (m_pParameters->GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadCoordinatesFileAsCartesian(fp); break;
      case LATLON    : bReturn = ReadCoordinatesFileAsLatitudeLongitude(fp); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadCoordinatesFile()",m_pParameters->GetCoordinatesType());
    };
    fclose(fp); fp=0;
  }
  catch (ZdException &x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadCoordinatesFile()","CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the geographic data file in Cartesian coordinate system.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadCoordinatesFileAsCartesian(FILE * fp) {
  int                           i, iScanCount=0;
  bool                          bValidRecord, bValid=true, bEmpty=true;
  const char                  * pCoordinate, * pDimension;
  ZdString                      TractIdentifier;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());

  try {
    while (Parser.ReadString(fp)) {
         //skip records with no data
         if (!Parser.HasWords())
           continue;
         //if empty and this record has data, then this is the first record w/ data
         if (bEmpty) {
           bEmpty = false;
           //determine number of dimensions from first record, 2 or more is valid
           iScanCount = Parser.GetNumberWords();
           //there must be at least two dimensions
           if (iScanCount < 3) {
             gpPrint->PrintInputWarning("Error: First record of coordinates file contains %s.\n",
                                                 iScanCount == 2 ? "only x-coordinate" : "no coordinates");
             bValid = false;
             break; //stop reading records, the first record defines remaining records format 
           }
           //ok, first record indicates that there are iScanCount - 1 dimensions (first scan is tract identifier)
           //data still could be invalid, but this will be determined like the remaining records
           m_pParameters->SetDimensionsOfData(iScanCount - 1);
           gpTInfo->tiSetDimensions(m_pParameters->GetDimensionsOfData());
           gpGInfo->giSetDimensions(m_pParameters->GetDimensionsOfData());
           vCoordinates.resize(m_pParameters->GetDimensionsOfData(), 0);
         }
         //read and validate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Parser, vCoordinates, iScanCount, 1, "coordinates")) {
           bValid = false;
           continue;
         }
         //validate that we read the correct number of coordinates
         if (iScanCount < m_pParameters->GetDimensionsOfData()) {
           //Note: since the first record defined the number of dimensions, this error could not happen.
           gpPrint->PrintInputWarning("Error: Record %ld in coordinates file contains %d dimension%s but the\n",
                                               Parser.GetReadCount(), iScanCount, (iScanCount == 1 ? "" : "s"));
           gpPrint->PrintInputWarning("       first record defined the number of dimensions as %d.\n", m_pParameters->GetDimensionsOfData());
           bValid = false;
           continue;
         }
         //add the tract identifier and coordinates to trac handler
         if (! gpTInfo->tiInsertTnode(Parser.GetWord(0), vCoordinates)) {
           gpPrint->PrintInputWarning("Error: For record %ld in coordinates file, location '%s' already exists.\n", Parser.GetReadCount(), Parser.GetWord(0));
           bValid = false;
           continue;
         }
         //add tract identifier and coordinates as centroid if a special grid file is not being used
         if (! m_pParameters->UseSpecialGrid())
           //no need to check return, we would have already gotten error from tract handler
           //for duplicate tract identifier
           gpGInfo->giInsertGnode(Parser.GetWord(0), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'coordinate file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Coordinates file contains no data.\n");
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gpTInfo->tiGetNumTracts() == 1 && m_pParameters->GetAnalysisType() != PURELYTEMPORAL) {
      gpPrint->SatScanPrintWarning("Error: For a %s analysis, the coordinates file must contain more than one location.\n",
                                   m_pParameters->GetAnalysisTypeAsString());
      bValid = false;
    }
    //record number of locations read
    m_nTracts = gpTInfo->tiGetNumTracts();
    //record number of centroids read
    m_nGridTracts = gpGInfo->giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsCartesian()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the latitude/longitude geographic data file.
    If invalid data is found in the file, an error message
    is printed, that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered   */
bool CSaTScanData::ReadCoordinatesFileAsLatitudeLongitude(FILE * fp) {
  int                           iScanCount;
  const char                  * pCoordinate;
  bool                          bValid=true, bEmpty=true;
  ZdString                      TractIdentifier;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    m_pParameters->SetDimensionsOfData(3/*for conversion*/);
    gpTInfo->tiSetDimensions(m_pParameters->GetDimensionsOfData());
    gpGInfo->giSetDimensions(m_pParameters->GetDimensionsOfData());
    while (Parser.ReadString(fp)) {
        //skip records with no data 
        if (! Parser.HasWords())
          continue;
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Parser, vCoordinates, 1, "coordinates")) {
           bValid = false;
           continue;
        }
        //add the tract identifier and coordinates to trac handler
        if (! gpTInfo->tiInsertTnode(Parser.GetWord(0), vCoordinates)) {
          gpPrint->PrintInputWarning("Error: For record %ld in coordinates file, location '%s' already exists.\n", Parser.GetReadCount(), Parser.GetWord(0));
          bValid = false;
          continue;
        }
        //add tract identifier and coordinates as centroid if a special grid file is not being used
        if (! m_pParameters->UseSpecialGrid())
          //no need to check return, we would have already gotten error from tract handler
          //for duplicate tract identifier
          gpGInfo->giInsertGnode(Parser.GetWord(0), vCoordinates);
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'coordinates file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Coordinates file contains no data.\n");
      bValid = false;
    }
    //validate that we have more than one tract, only a purely temporal analysis is the exception to this rule
    else if (gpTInfo->tiGetNumTracts() == 1 && m_pParameters->GetAnalysisType() != PURELYTEMPORAL) {
      gpPrint->PrintInputWarning("Error: For a %s analysis, the coordinates file must contain more than one record.\n",
                                          m_pParameters->GetAnalysisTypeAsString());
      bValid = false;
    }
    //record number of locations read
    m_nTracts = gpTInfo->tiGetNumTracts();
    //record number of centroids read
    m_nGridTracts = gpGInfo->giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadCoordinatesFileAsLatitudeLongitude()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the count(either case or control) data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadCounts(FILE * fp, const char* szDescription) {
  int                                   i, j, iCategoryIndex;
  bool                                  bCaseFile, bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(gpPrint->GetImpliedInputFileType());
  std::string                           sBuffer;
  count_t                               Count, ** pCounts;
  TwoDimensionArrayHandler<count_t>   * pCountsByTimeByCategory;
  ThreeDimensionArrayHandler<count_t> * pCategoryCounts;

  try {
    bCaseFile = !strcmp(szDescription, "case");
    pCounts = (bCaseFile ? gpCasesHandler->GetArray() : gpControlsHandler->GetArray());
    pCountsByTimeByCategory = (bCaseFile ? gpCasesByTimeByCategoryHandler : gpControlsByTimeByCategoryHandler);
    pCategoryCounts = (bCaseFile ? gpCategoryCasesHandler : gpCategoryControlsHandler);

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(szDescription, Parser, TractIndex, Count, Date, iCategoryIndex)) {
             //cumulatively add count to time by location structure
             pCounts[0][TractIndex] += Count;
             for (i=1; Date >= m_pIntervalStartTimes[i]; ++i)
               pCounts[i][TractIndex] += Count;
             //record count as a case or control  
             if (bCaseFile)
               gPopulationCategories.AddCaseCount(iCategoryIndex, Count);
             else
               gPopulationCategories.AddControlCount(iCategoryIndex, Count);
             //record count in structure(s) based upon population category
             switch (m_pParameters->GetProbabiltyModelType()) {
               case POISSON :
                 //Add count to tract for category. This could return false if record's tract
                 //identifier / covariates together don't match an existing population record.
                 if (! gpTInfo->tiAddCount(TractIndex, iCategoryIndex, Count)) {
                   gpPrint->PrintInputWarning("Error: Record %ld of case file refers to location '%s' with population category '%s',\n",
                                              Parser.GetReadCount(), Parser.GetWord(0), gPopulationCategories.GetPopulationCategoryAsString(iCategoryIndex, sBuffer));
                   gpPrint->PrintInputWarning("       but no matching population record with this combination exists.\n");
                   return false;
                 }
                 break;
               case SPACETIMEPERMUTATION :
                 if (iCategoryIndex >= static_cast<int>(pCategoryCounts->Get3rdDimension()))
                   pCategoryCounts->ExpandThirdDimension(0);
                 pCategoryCounts->GetArray()[0][TractIndex][iCategoryIndex] += Count;
                 for (i=1; Date >= m_pIntervalStartTimes[i]; ++i)
                    pCategoryCounts->GetArray()[i][TractIndex][iCategoryIndex] += Count;
                 break;
               case BERNOULLI :
                 if (iCategoryIndex >= static_cast<int>(pCategoryCounts->Get3rdDimension()))
                   pCategoryCounts->ExpandThirdDimension(0);
                 pCategoryCounts->GetArray()[0][TractIndex][iCategoryIndex] += Count;
                 for (i=1; Date >= m_pIntervalStartTimes[i]; ++i)
                    pCategoryCounts->GetArray()[i][TractIndex][iCategoryIndex] += Count;
                 if (m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION) {
                   if (iCategoryIndex >= static_cast<int>(pCountsByTimeByCategory->Get2ndDimension()))
                     pCountsByTimeByCategory->ExpandSecondDimension(0);
                   pCountsByTimeByCategory->GetArray()[0][iCategoryIndex] += Count;
                   for (i=1; Date >= m_pIntervalStartTimes[i]; ++i)
                      pCountsByTimeByCategory->GetArray()[i][iCategoryIndex] += Count;
                 }
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
    x.AddCallpath("ReadCounts()","CSaTScanData");
    throw;
  }
  return bValid;
};

/** Read the special grid file.  Calls particular read given coordinate type. */
bool CSaTScanData::ReadGridFile() {
  bool          bReturn;
  FILE        * fp=0;

  try {
    gpPrint->SatScanPrintf("Reading the grid file\n");
    if ((fp = fopen(m_pParameters->GetSpecialGridFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open grid file:\n'%s'.\n",
                                   m_pParameters->GetSpecialGridFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::GRIDFILE);
    switch (m_pParameters->GetCoordinatesType()) {
      case CARTESIAN : bReturn = ReadGridFileAsCartiesian(fp); break;
      case LATLON    : bReturn = ReadGridFileAsLatitudeLongitude(fp); break;
      default : ZdException::Generate("Unknown coordinate type '%d'.","ReadGrid()",m_pParameters->GetCoordinatesType());
    };
    fclose(fp);fp=0;
  }
  catch (ZdException &x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadGridFile()", "CSaTScanData");
    throw;
  }
  return bReturn;
}

/** Read the special grid data file as Cartesian coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered          */
bool CSaTScanData::ReadGridFileAsCartiesian(FILE * fp) {
  bool                          bValidRecord, bValid=true, bEmpty=true;
  int                           i, iScanCount;
  const char                  * pCoordinate;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());
  ZdString                      sId;

  try {
    vCoordinates.resize(m_pParameters->GetDimensionsOfData(), 0);
    while (Parser.ReadString(fp)) {
        //skip blank lines
        if (!Parser.HasWords())
          continue;
        //there are records with data, but not necessarily valid
        bEmpty = false;
         //read and vaidate dimensions skip to next record if error reading coordinates as double
         if (! ReadCartesianCoordinates(Parser, vCoordinates, iScanCount, 0, "grid")) {
           bValid = false;
           continue;
         }
        //validate that we read the correct number of coordinates as defined by coordinates system or coordinates file
        if (iScanCount < m_pParameters->GetDimensionsOfData()) {
          gpPrint->PrintInputWarning("Error: Record %ld in grid file contains %d dimension%s but the\n",
                                     Parser.GetReadCount(), iScanCount, (iScanCount == 1 ? "" : "s"));
          gpPrint->PrintInputWarning("       coordinates file defined the number of dimensions as %d.\n",
                                     m_pParameters->GetDimensionsOfData());
          bValid = false;
          continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Parser.GetReadCount();
        if (! gpGInfo->giInsertGnode(sId.GetCString(), vCoordinates))
          //If there are problems adding then either some other code has errored by
          //adding to this structure previously or this routine is doing something wrong.
          //When a special grid file is used to supply centroids, only the routines
          //read the special grid file should be adding to this structure.
          ZdException::Generate("Error: Duplicate identifier encountered reading centroids.","ReadGridFileAsCartiesian()");
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'grid file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Grid file does not contain data.\n");
      bValid = false;
    }
    //record number of centroids read
    m_nGridTracts = gpGInfo->giGetNumTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsCartiesian()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the special grid data file as latitude/longitude coordinates.
   If invalid data is found in the file, an error message is printed,
   that record is ignored, and reading continues.
   Return value: true = success, false = errors encountered           */
bool CSaTScanData::ReadGridFileAsLatitudeLongitude(FILE * fp) {
  bool    	                bValid=true, bEmpty=true;
  const char                  * pCoordinate;
  std::vector<double>           vCoordinates;
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());
  ZdString                      sId;

  try {
    vCoordinates.resize(3/*for conversion*/, 0);
    while (Parser.ReadString(fp)) {
        //skip lines with no data
        if (!Parser.HasWords())
          continue;
        //there are records with data, but not necessarily valid
        bEmpty=false;
        if (! ReadLatitudeLongitudeCoordinates(Parser, vCoordinates, 0, "grid")) {
           bValid = false;
           continue;
        }
        //add created tract identifer(record number) and read coordinates to structure that mantains list of centroids
        sId = Parser.GetReadCount();
        if (!gpGInfo->giInsertGnode(sId.GetCString(), vCoordinates))
          //If there are problems adding then either some other code has errored by
          //adding to this structure previously or this routine is doing something wrong.
          //When a special grid file is used to supply centroids, only the routines
          //read the special grid file should be adding to this structure.
          ZdException::Generate("Error: Duplicate identifier encountered reading centroids.","ReadGridFileAsLatitudeLongitude()");
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'grid file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Grid file is contains no data.\n");
      bValid = false;
    }
    //record number of centroids
    m_nGridTracts = gpGInfo->giGetNumTracts();
  }
  catch (ZdFileOpenFailedException &x) {
    gpPrint->SatScanPrintWarning("Error: Special Grid file '%s' could not be opened.\n",
                                          m_pParameters->GetSpecialGridFileName().c_str());
    return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadGridFileAsLatitudeLongitude()","CSaTScanData");
    throw;
  }
  return bValid;
}

/** Reads latitude/longitude coordinates into vector.
    Note: coordinate vector should already be sized to 3 dimensions.
    Returns indication of whether words in passed string could be converted to
    coordinates. Checks that coordinates are in range and converts to cartesian
    coordinates. */
bool CSaTScanData::ReadLatitudeLongitudeCoordinates(StringParser & Parser, std::vector<double> & vCoordinates,
                                                    int iWordOffSet, const char * sSourceFile) {
  const char  * pCoordinate;
  double        dLatitude, dLongitude;

  //read latitude, validating that string can be converted to double
  if ((pCoordinate = Parser.GetWord(iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLatitude)) {
      gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as latitude.\n", pCoordinate, Parser.GetReadCount(), sSourceFile);
      return false;
    }
  }
  else {
    gpPrint->PrintInputWarning("Error: Record %d in %s file missing latitude and longitude coordinates.\n", Parser.GetReadCount(), sSourceFile);
    return false;
  }
  //read longitude, validating that string can be converted to double
  if ((pCoordinate = Parser.GetWord(++iWordOffSet)) != 0) {
    if (! sscanf(pCoordinate, "%lf", &dLongitude)) {
      gpPrint->PrintInputWarning("Error: Value '%s' of record %ld in %s file could not be read as longitude.\n", pCoordinate, Parser.GetReadCount(), sSourceFile);
      return false;
    }
  }
  else {
    gpPrint->PrintInputWarning("Error: Record %ld in %s file missing longitude coordinate.\n", Parser.GetReadCount(), sSourceFile);
    return false;
  }
  //validate that there is not extra data for record
  if ((pCoordinate = Parser.GetWord(++iWordOffSet)) != 0) {
    gpPrint->PrintInputWarning("Error: Record %ld in %s file contains extra data: '%s'.\n", Parser.GetReadCount(), sSourceFile, pCoordinate);
    return false;
  }
  //validate range of latitude value
  if ((fabs(dLatitude) > 90.0)) {
    gpPrint->PrintInputWarning("Error: Latitude %lf, for record %ld in %s file, is out of range.\n",  dLatitude, Parser.GetReadCount(), sSourceFile);
    gpPrint->PrintInputWarning("       Latitude must be between -90 and 90.\n");
    return false;
  }
  //validate range of longitude value
  if ((fabs(dLongitude) > 180.0)) {
    gpPrint->PrintInputWarning("Error: Longitude %lf, for record %ld in %s file, is out of range.\n", dLongitude, Parser.GetReadCount(), sSourceFile);
    gpPrint->PrintInputWarning("       Longitude must be between -180 and 180.\n");
    return false;
  }
  //convert to 3 dimensions then can add tracts
  ConvertFromLatLong(dLatitude, dLongitude, vCoordinates);
  return true;
}

/** Read the population file.
    The number of category variables is determined by the first record.
    Any records deviating from this number will cause an error.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues. The tract-id MUST match
    one read in ReadCoordinatesFile().
    Return value: true = success, false = errors encountered */
bool CSaTScanData::ReadPopulationFile() {
  int                           iCategoryIndex, iRecNum=0;
  bool                          bValid=true, bEmpty=true;
  tract_t                       TractIdentifierIndex;
  float                         fPopulation;
  Julian                        PopulationDate;
  FILE                        * fp=0; // Ptr to population file
  std::vector<Julian>           vPopulationDates;
  std::vector<Julian>::iterator itrdates;
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());

  try {
    gpPrint->SatScanPrintf("Reading the population file\n");
    if ((fp = fopen(m_pParameters->GetPopulationFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open population file:\n'%s'.\n",
                                   m_pParameters->GetPopulationFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::POPFILE);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (Parser.ReadString(fp)) {
        ++iRecNum;
        //skip lines that do not contain data
        if (!Parser.HasWords())
          continue;
        bEmpty=false;
        //scan values and validate - population file records must contain tract id, date and population.
        if (!Parser.GetWord(1)) {
            gpPrint->PrintInputWarning("Error: Record %ld of population file missing date.\n", iRecNum);
            bValid = false;
            continue;
        }
        if (!ConvertPopulationDateToJulian(Parser.GetWord(1), iRecNum, PopulationDate)) {
            bValid = false;
            continue;
        }
        //if date is unique, add it to the list
        itrdates = lower_bound(vPopulationDates.begin(), vPopulationDates.end(), PopulationDate);
        if (! (itrdates != vPopulationDates.end() && (*itrdates) == PopulationDate))
          vPopulationDates.insert(itrdates, PopulationDate);
    }
    // When a prospective analysis is conducted and if a population file is used,
    //and if the population for a tract is defined at more than one time period,
    //error message should be shown in the running window and the application terminated.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME && m_pParameters->GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE && vPopulationDates.size() > 1) {
        bValid = false;
        gpPrint->PrintInputWarning("Error: For the prospective space-time analysis to be correct,\n"
                                   "       it is critical that the scanning spatial window is the\n"
                                   "       same for each of the analysis performed over time. If \n"
                                   "       there are multiple years in the population file, so that\n"
                                   "       the population size changes over time, as it does in your\n"
                                   "       data, then you must define the maximum circle size in\n"
                                   "       terms of a specific geographical radius rather than as a\n"
                                   "       percent of the total population at risk.\n\n\n");
    }
    //2nd pass, read data in structures.
    if (bValid && !bEmpty) {
      //Set tract handlers population date structures since we already now all the dates from above.
      gpTInfo->tiSetupPopDates(vPopulationDates, m_nStartDate, m_nEndDate);
      //reset for second read
      fseek(fp, 0L, SEEK_SET);
      iRecNum = 0;
      //We can ignore error checking for population date and population since we already did this above.
      while (Parser.ReadString(fp)) {
          ++iRecNum;
          if (!Parser.HasWords()) // Skip Blank Lines
            continue;
          ConvertPopulationDateToJulian(Parser.GetWord(1), iRecNum, PopulationDate);
          if (!Parser.GetWord(2)) {
            gpPrint->PrintInputWarning("Error: Record %d of population file missing population.\n", iRecNum);
            bValid = false;
            continue;
          }
          if (sscanf(Parser.GetWord(2), "%f", &fPopulation) != 1) {
            gpPrint->PrintInputWarning("Error: Population value '%s' in record %ld, of population file, is not a number.\n",
                                       Parser.GetWord(2), iRecNum);
            bValid = false;
            continue;
          }
          //validate that population is not negative or exceeding type precision
          if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
            if (strstr(Parser.GetWord(2), "-"))
              gpPrint->PrintInputWarning("Error: Negative population in record %ld of population file.\n", iRecNum);
            else
              gpPrint->PrintInputWarning("Error: Population '%s' exceeds maximum value of %i in record %ld of population file.\n",
                                         Parser.GetWord(2), std::numeric_limits<float>::max(), iRecNum);
            bValid = false;
            continue;
          }
          //Scan for covariates to create population categories or find index.
          //First category created sets precedence as to how many covariates remaining records must have.
          if ((iCategoryIndex = gPopulationCategories.MakePopulationCategory("population", Parser, iRecNum, 3, *gpPrint)) == -1) {
            bValid = false;
            continue;
          }
          //Validate that tract identifer is one of those defined in the coordinates file.
          if ((TractIdentifierIndex = gpTInfo->tiGetTractIndex(Parser.GetWord(0))) == -1) {
            gpPrint->PrintInputWarning("Error: Unknown location identifier in population file, record %ld.\n", iRecNum);
            gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
            bValid = false;
            continue;
          }
          //Add population count for this tract/category/year
          gpTInfo->tiAddCategoryToTract(TractIdentifierIndex, iCategoryIndex, PopulationDate, fPopulation);
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
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadPopulationFile()", "CSaTScanData");
    throw;
  }
  return bValid;
}

/** Read the special population that will be used to construct circles
    about grid points(centroids).                                      */
bool CSaTScanData::ReadMaxCirclePopulationFile() {
  int                           iRecNum=0;
  bool                          bValid=true, bEmpty=true;
  tract_t                       TractIdentifierIndex;
  float                         fPopulation;
  FILE                        * fp=0; // Ptr to population file
  StringParser                  Parser(gpPrint->GetImpliedInputFileType());

  try {
    gpPrint->SatScanPrintf("Reading the maximum circle population file\n");
    if ((fp = fopen(m_pParameters->GetMaxCirclePopulationFileName().c_str(), "r")) == NULL) {
      gpPrint->SatScanPrintWarning("Error: Could not open maximum circle population file:\n'%s'.\n",
                                   m_pParameters->GetMaxCirclePopulationFileName().c_str());
      return false;
    }
    gpPrint->SetImpliedInputFileType(BasePrint::MAXCIRCLEPOPFILE);

    //initialize circle-measure array
    gvCircleMeasure.resize(m_nTracts, 0);

    //1st pass, determine unique population dates. Notes errors with records and continues reading.
    while (Parser.ReadString(fp)) {
        ++iRecNum;
        //skip lines that do not contain data
        if (!Parser.HasWords())
          continue;
        bEmpty=false;
        //read tract identifier
        if ((TractIdentifierIndex = gpTInfo->tiGetTractIndex(Parser.GetWord(0))) == -1) {
          gpPrint->PrintInputWarning("Error: Unknown location identifier in special population file, record %ld.\n", iRecNum);
          gpPrint->PrintInputWarning("       '%s' not specified in the coordinates file.\n", Parser.GetWord(0));
          bValid = false;
          continue;
        }
        //read population
        if (!Parser.GetWord(1)) {
          gpPrint->PrintInputWarning("Error: Record %d of special population file missing population.\n", iRecNum);
          bValid = false;
          continue;
        }
        if (sscanf(Parser.GetWord(1), "%f", &fPopulation) != 1) {
          gpPrint->PrintInputWarning("Error: Population value '%s' in record %ld, of special population file, is not a number.\n",
                                     Parser.GetWord(1), iRecNum);
          bValid = false;
          continue;
        }
        //validate that population is not negative or exceeding type precision
        if (fPopulation < 0) {//validate that count is not negative or exceeds type precision
          if (strstr(Parser.GetWord(1), "-"))
             gpPrint->PrintInputWarning("Error: Negative population in record %ld of special population file.\n", iRecNum);
          else
             gpPrint->PrintInputWarning("Error: Population '%s' exceeds maximum value of %i in record %ld of special population file.\n",
                                        Parser.GetWord(1), std::numeric_limits<float>::max(), iRecNum);
           bValid = false;
           continue;
        }
        gvCircleMeasure[TractIdentifierIndex] += fPopulation;
    }
    //close file pointer
    fclose(fp); fp=0;
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gpPrint->PrintWarningLine("Please see 'special population file format' in the user guide for help.\n");
    //print indication if file contained no data
    else if (bEmpty) {
      gpPrint->PrintWarningLine("Error: Special Population file contains no data.\n");
      bValid = false;
    }
  }
  catch (ZdException &x) {
    //close file pointer
    if (fp) fclose(fp);
    x.AddCallpath("ReadMaxCirclePopulationFile()", "CSaTScanData");
    throw;
  }
  return bValid;
}
