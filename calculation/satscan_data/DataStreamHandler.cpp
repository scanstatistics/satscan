//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "DataStreamHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"

const short DataStreamHandler::COUNT_DATE_OFFSET        = 2;

/** constructor */
DataStreamHandler::DataStreamHandler(CSaTScanData& DataHub, BasePrint& Print)
                  : gDataHub(DataHub), gParameters(DataHub.GetParameters()), gPrint(Print) {
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

/** allocates cases structures for stream */
void DataStreamHandler::AllocateCaseStructures(unsigned int iStream) {
  try {
    gvDataStreams[iStream]->AllocateCasesArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","DataStreamHandler");
    throw;
  }
}

/** Converts passed string specifiying a count date to a julian date. Since
    errors/warnings are accumulated when reading input files, indication of
    successful conversion to Julian date is returned and any messages sent
    to print direction. */
bool DataStreamHandler::ConvertCountDateToJulian(StringParser & Parser, Julian & JulianDate) {
  DateStringParser                      DateParser;
  DateStringParser::ParserStatus        eStatus;  

  //If parameters indicate that case data does not contain dates, don't try to
  //read a date, or validate that there isn't one (could be covariate), and set
  //Julian reference to study period start date.  
  if (gParameters.GetPrecisionOfTimesType() == NONE) {
    JulianDate = gDataHub.GetStudyPeriodStartDate();
    return true;
  }
  else {
    //Parameter settings indicate that there should be a date in each case record.
    if (!Parser.GetWord(COUNT_DATE_OFFSET)) {
      gPrint.PrintInputWarning("Error: Record %ld in %s does not contain a date.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    //Attempt to convert string into Julian equivalence.
    eStatus = DateParser.ParseCountDateString(Parser.GetWord(COUNT_DATE_OFFSET), gParameters.GetTimeIntervalUnitsType(),
                                              gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodStartDate(), JulianDate);
    switch (eStatus) {
      case DateStringParser::VALID_DATE       : break;
      case DateStringParser::AMBIGUOUS_YEAR   :
        gPrint.PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n"
                                   "       to determine century for two digit year in %s, record %ld.\n"
                                   "       Please use four digit years.\n",
                                   gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
        return false;
      case DateStringParser::LESSER_PRECISION :
         //Dates in the case/control files must be at least as precise as specified time interval units.
         gPrint.PrintInputWarning("Error: Date '%s' of record %ld in %s must be precise to %s, as specified by time interval units.\n",
                                    Parser.GetWord(COUNT_DATE_OFFSET), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                                    gParameters.GetDatePrecisionAsString(gParameters.GetTimeIntervalUnitsType()));
        return false;
      case DateStringParser::INVALID_DATE     :
      default                                 :
        gPrint.PrintInputWarning("Error: Invalid date '%s' in %s, record %ld.\n", Parser.GetWord(COUNT_DATE_OFFSET),
                                   gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
        return false;
    };
    //validate that date is between study period start and end dates
    if (!(gDataHub.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gDataHub.GetStudyPeriodEndDate())) {
      gPrint.PrintInputWarning("Error: Date '%s' in record %ld of %s is not\n", Parser.GetWord(2),
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      gPrint.PrintInputWarning("       within the study period beginning %s and ending %s.\n",
                                 gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
      return false;
    }
  }
  return true;
}

/** Returns new data gateway. Caller is responsible for deleting object.
    If number of data streams is more than one, a MultipleDataStreamGateway
    object is returned. Else a  DataStreamGateway object is returned. */
AbtractDataStreamGateway * DataStreamHandler::GetNewDataGatewayObject() const {
  if (gvDataStreams.size() > 1)
    return new MultipleDataStreamGateway();
  else
    return new DataStreamGateway();
}

/** Attempts to parses passed string into tract identifier, count,
    and based upon settings, date and covariate information.
    Returns whether parse completed without errors. */
bool DataStreamHandler::ParseCountLine(PopulationData & thePopulation, StringParser & Parser,
                                       tract_t& tid, count_t& nCount,
                                       Julian& nDate, int& iCategoryIndex) {
                                       
  int                          iCategoryOffSet, iScanPrecision;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(0))) == -1) {
      gPrint.PrintInputWarning("Error: Unknown location ID in %s, record %ld.\n",
                                 gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      gPrint.PrintInputWarning("       Location ID '%s' was not specified in the coordinates file.\n", Parser.GetWord(0));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(1) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.PrintInputWarning("Error: The value '%s' of record %ld in %s could not be read as case count.\n",
                                  Parser.GetWord(1), Parser.GetReadCount(),
                                  gPrint.GetImpliedFileTypeString().c_str());
       gPrint.PrintInputWarning("       Case count must be an integer.\n");
       return false;
      }
    }
    else {
      gPrint.PrintInputWarning("Error: Record %ld in %s does not contain case count.\n",
                                 Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(1), "-"))
        gPrint.PrintInputWarning("Error: Record %ld, of the %s, contains a negative case count.\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.PrintInputWarning("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                                   Parser.GetWord(1), std::numeric_limits<count_t>::max(), Parser.GetReadCount(),
                                   gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;
    iCategoryOffSet = gParameters.GetPrecisionOfTimesType() == NONE ? 2 : 3;
    if (! ParseCovariates(thePopulation, iCategoryIndex, iCategoryOffSet, Parser))
        return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCountLine()","DataStreamHandler");
    throw;
  }
  return true;
}

/** Parses count file data line to determine category index given covariates contained in line.*/
bool DataStreamHandler::ParseCovariates(PopulationData & thePopulation, int& iCategoryIndex, int iCovariatesOffset, StringParser & Parser) {
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
        gPrint.PrintInputWarning("Error: Record %ld of %s contains %d covariate%s but the population file\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                                   iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"));
        gPrint.PrintInputWarning("       defined the number of covariates as %d.\n", thePopulation.GetNumPopulationCategoryCovariates());
        return false;
      }
      //category should already exist
      if ((iCategoryIndex = thePopulation.GetPopulationCategoryIndex(vCategoryCovariates)) == -1) {
        gPrint.PrintInputWarning("Error: Record %ld of %s refers to a population category that\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
        gPrint.PrintInputWarning("       does not match an existing category as read from the population file.");
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
        if ((iCategoryIndex = thePopulation.MakePopulationCategory(Parser, iCovariatesOffset, gPrint)) == -1)
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

/** Randomizes data of passed collection of simulation streams in concert with
    real data through passed collection of passed randomizers. */
void DataStreamHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
     Container[t]->RandomizeData(*gvDataStreams[t], *SimDataContainer[t], iSimulationNumber);
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataStreamHandler::ReadCaseFile(size_t tStream) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    if ((fp = fopen(gParameters.GetCaseFileName(tStream + 1).c_str(), "r")) == NULL) {
      gPrint.SatScanPrintWarning("Error: Could not open the case file:\n'%s'.\n",
                                   gParameters.GetCaseFileName(tStream + 1).c_str());
      return false;
    }                                                                  
    gPrint.SetImpliedInputFileType(BasePrint::CASEFILE, (GetNumStreams() == 1 ? 0 : tStream + 1));
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

/** Read the count(either case or control) data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataStreamHandler::ReadCounts(size_t tStream, FILE * fp, const char* szDescription) {
  int                                   i, j, iCategoryIndex;
  bool                                  bCaseFile, bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(gPrint);
  std::string                           sBuffer;
  count_t                               Count, ** pCounts;

  try {
    RealDataStream& thisStream = *gvDataStreams[tStream];

    bCaseFile = !strcmp(szDescription, "case");
    pCounts = (bCaseFile ? thisStream.gpCasesHandler->GetArray() : thisStream.gpControlsHandler->GetArray());

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(thisStream.gPopulation, Parser, TractIndex, Count, Date, iCategoryIndex)) {
             //cumulatively add count to time by location structure
             pCounts[0][TractIndex] += Count;
             if (pCounts[0][TractIndex] < 0)
               GenerateResolvableException("Error: The total %s, in data stream %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                           (bCaseFile ? "cases" : "controls"), tStream, std::numeric_limits<count_t>::max());
             for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
               pCounts[i][TractIndex] += Count;
             //record count as a case or control  
             if (bCaseFile)
               thisStream.gPopulation.AddCaseCount(iCategoryIndex, Count);
             else
               thisStream.gPopulation.AddControlCount(iCategoryIndex, Count);
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.SatScanPrintWarning("Please see the '%s file' section in the user guide for help.\n", szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.SatScanPrintWarning("Error: The %s file does not contain data.\n", gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","DataStreamHandler");
    throw;
  }
  return bValid;
}

/** reports whether any data stream has cases with a zero population. */
void DataStreamHandler::ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) {
  for (size_t t=0; t < gvDataStreams.size(); ++t)
    gvDataStreams[t]->GetPopulationData().ReportZeroPops(Data, pDisplay, *pPrintDirection);
}

void DataStreamHandler::SetPurelyTemporalMeasureData(RealDataStream & thisRealStream) {
  try {
    thisRealStream.SetPTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasureData()","DataStreamHandler");
    throw;
  }
}

/** sets temporal simulation case array from data in simulation case array */
void DataStreamHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t)
       SimDataContainer[t]->SetPTCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","DataStreamHandler");
    throw;
  }
}

/** internal initialization */
void DataStreamHandler::Setup() {
  try {
    for (unsigned int i=0; i < gParameters.GetNumDataStreams(); ++i)
      gvDataStreams.push_back(new RealDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), i + 1));
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataStreamHandler");
    throw;
  }
}

