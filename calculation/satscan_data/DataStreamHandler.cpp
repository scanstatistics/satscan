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
    gvDataSets[iStream]->AllocateCasesArray();
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
  DatePrecisionType                     ePrecision;

  //If parameters indicate that case data does not contain dates, don't try to
  //read a date, or validate that there isn't one (could be covariate), and set
  //Julian reference to study period start date.  
  if (gParameters.GetPrecisionOfTimesType() == NONE) {
    JulianDate = gDataHub.GetStudyPeriodStartDate();
    return true;
  }

  //If parameter file was created with version 4 of SaTScan, use time interval
  //units as specifier for date precision. This was the behavior in v4 but it
  //was decided to revert to time precision units. Note that for a purely spatial
  //analysis, we have no way of knowing what the time precision should be; settings
  //to YEAR is safe since it is permittable to have more precise dates.
  if (gParameters.GetPrecisionOfTimesType() == NONE)
    ePrecision = NONE;
  else if (gParameters.GetCreationVersionMajor() == 4)
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? YEAR : gParameters.GetTimeAggregationUnitsType());
  else
    ePrecision =  gParameters.GetPrecisionOfTimesType();

  //Parameter settings indicate that there should be a date in each case record.
  if (!Parser.GetWord(COUNT_DATE_OFFSET)) {
    gPrint.PrintInputWarning("Error: Record %ld in %s does not contain a date.\n",
                              Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
    return false;
  }
  //Attempt to convert string into Julian equivalence.
  eStatus = DateParser.ParseCountDateString(Parser.GetWord(COUNT_DATE_OFFSET), ePrecision,
                                            gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodStartDate(), JulianDate);
  switch (eStatus) {
    case DateStringParser::VALID_DATE       : break;
    case DateStringParser::AMBIGUOUS_YEAR   :
      gPrint.PrintInputWarning("Error: Due to the study period being greater than 100 years, unable\n"
                               "       to determine century for two digit year in %s, record %ld.\n"
                               "       Please use four digit years.\n",
                               gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      return false;
    case DateStringParser::LESSER_PRECISION : {
       ZdString sBuffer;
       //Dates in the case/control files must be at least as precise as ePrecision units.
       gPrint.PrintInputWarning("Error: The date '%s' of record %ld in the %s must be precise to %s,\n"
                                "       as specified by %s units.\n",
                                Parser.GetWord(COUNT_DATE_OFFSET), Parser.GetReadCount(),
                                gPrint.GetImpliedFileTypeString().c_str(),
                                GetDatePrecisionAsString(ePrecision, sBuffer, false, false),
                                (gParameters.GetCreationVersionMajor() == 4 ? "time interval" : "time precision"));
      return false; }
    case DateStringParser::INVALID_DATE     :
    default                                 :
      gPrint.PrintInputWarning("Error: Invalid date '%s' in the %s, record %ld.\n", Parser.GetWord(COUNT_DATE_OFFSET),
                               gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      return false;
  };
  //validate that date is between study period start and end dates
  if (!(gDataHub.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gDataHub.GetStudyPeriodEndDate())) {
    gPrint.PrintInputWarning("Error: The date '%s' in record %ld of the %s is not\n", Parser.GetWord(2),
                             Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
    gPrint.PrintInputWarning("       within the study period beginning %s and ending %s.\n",
                             gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
    return false;
  }
  return true;
}

/** Returns new data gateway. Caller is responsible for deleting object.
    If number of data streams is more than one, a MultipleDataStreamGateway
    object is returned. Else a  DataStreamGateway object is returned. */
AbtractDataStreamGateway * DataStreamHandler::GetNewDataGatewayObject() const {
  if (gvDataSets.size() > 1)
    return new MultipleDataStreamGateway();
  else
    return new DataStreamGateway();
}

/** Returns a collection of cloned randomizers maintained by data stream handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& DataStreamHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
//  RandomizerContainer_t::const_iterator itr;

  try {
    Container = gvDataStreamRandomizers;
//    Container.DeleteAllElements();
//    for (itr=gvDataStreamRandomizers.begin(); itr != gvDataStreamRandomizers.end(); ++itr)
//       Container.push_back((*itr)->Clone());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","DataStreamHandler");
    throw;
  }
  return Container;
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

    if (gParameters.GetProbabilityModelType() == POISSON) {
      while ((pCovariate = Parser.GetWord(iNumCovariatesScanned + iCovariatesOffset)) != 0) {
           vCategoryCovariates.push_back(pCovariate);
           iNumCovariatesScanned++;
      }
      if (!gParameters.UsePopulationFile() && iNumCovariatesScanned) {
        //If the population data was not gotten from a population file, then there can not
        //be covariates in other files, namely the case file.
        gPrint.PrintInputWarning("Error: Record %ld of %s contains %d covariate%s but covariates are not permitted\n"
                                 "       in the %s when a population file is not specified.\n" ,
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                                   iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"),
                                   gPrint.GetImpliedFileTypeString().c_str());
        return false;
      }
      if (iNumCovariatesScanned != thePopulation.GetNumCovariatesPerCategory()) {
        gPrint.PrintInputWarning("Error: Record %ld of %s contains %d covariate%s but the population file\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                                   iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"));
        gPrint.PrintInputWarning("       defined the number of covariates as %d.\n", thePopulation.GetNumCovariatesPerCategory());
        return false;
      }
      //category should already exist
      if ((iCategoryIndex = thePopulation.GetCovariateCategoryIndex(vCategoryCovariates)) == -1) {
        gPrint.PrintInputWarning("Error: Record %ld of %s refers to a population category that\n",
                                   Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
        gPrint.PrintInputWarning("       does not match an existing category as read from the population file.");
        return false;
      }
    }
    else if (gParameters.GetProbabilityModelType() == BERNOULLI) {
      //For the Bernoulli model, ignore covariates in the case and control files
      //All population categories are aggregated in one category.
      iCategoryIndex = 0;
    }
    else if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
        //First category created sets precedence as to how many covariates remaining records must have.
        if ((iCategoryIndex = thePopulation.CreateCovariateCategory(Parser, iCovariatesOffset, gPrint)) == -1)
          return false;
    }
    else
      ZdGenerateException("Unknown probability model type '%d'.","ParseCovariates()", gParameters.GetProbabilityModelType());
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
  for (size_t t=0; t < gvDataSets.size(); ++t)
     Container[t]->RandomizeData(*gvDataSets[t], *SimDataContainer[t], iSimulationNumber);
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
    gPrint.SetImpliedInputFileType(BasePrint::CASEFILE, (GetNumDataSets() == 1 ? 0 : tStream + 1));
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
    RealDataStream& thisStream = *gvDataSets[tStream];

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
               thisStream.gPopulation.AddCovariateCategoryCaseCount(iCategoryIndex, Count);
             else
               thisStream.gPopulation.AddCovariateCategoryControlCount(iCategoryIndex, Count);
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
  for (size_t t=0; t < gvDataSets.size(); ++t)
    gvDataSets[t]->GetPopulationData().ReportZeroPops(Data, pDisplay, *pPrintDirection);
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
      gvDataSets.push_back(new RealDataStream(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), i + 1));
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataStreamHandler");
    throw;
  }
}

