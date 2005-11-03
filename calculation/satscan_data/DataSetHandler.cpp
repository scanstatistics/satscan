//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"

const short DataSetHandler::guLocationIndex             = 0;
const short DataSetHandler::guCountIndex                = 1;
const short DataSetHandler::guCountDateIndex            = 2;
const short DataSetHandler::guCountCategoryIndexNone    = 2;
const short DataSetHandler::guCountCategoryIndex        = 3;

/** constructor */
DataSetHandler::DataSetHandler(CSaTScanData& DataHub, BasePrint& Print)
               :gDataHub(DataHub), gParameters(DataHub.GetParameters()), gPrint(Print) {
  try {
    Setup();
  }
  catch(ZdException &x) {
    x.AddCallpath("constructor()","DataSetHandler");
    throw;
  }
}

/** destructor */
DataSetHandler::~DataSetHandler() {}

/** allocates cases structures for data set at index */
void DataSetHandler::AllocateCaseStructures(size_t iSetIndex) {
  try {
    gvDataSets[iSetIndex]->AllocateCasesArray();
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateCaseStructures()","DataSetHandler");
    throw;
  }
}

/** Converts passed string specifiying a count date to a julian date. Since
    errors/warnings are accumulated when reading input files, indication of
    successful conversion to Julian date is returned and any messages sent
    to print direction. */
bool DataSetHandler::ConvertCountDateToJulian(StringParser & Parser, Julian & JulianDate) {
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
  if (!Parser.GetWord(guCountDateIndex)) {
    gPrint.Printf("Error: Record %ld in %s does not contain a date.\n",
                  BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
    return false;
  }
  //Attempt to convert string into Julian equivalence.
  eStatus = DateParser.ParseCountDateString(Parser.GetWord(guCountDateIndex), ePrecision,
                                            gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodStartDate(), JulianDate);
  switch (eStatus) {
    case DateStringParser::VALID_DATE       : break;
    case DateStringParser::AMBIGUOUS_YEAR   :
      gPrint.Printf("Error: Due to the study period being greater than 100 years, unable\n"
                    "       to determine century for two digit year in %s, record %ld.\n"
                    "       Please use four digit years.\n", BasePrint::P_READERROR,
                    gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      return false;
    case DateStringParser::LESSER_PRECISION : {
       ZdString sBuffer;
       //Dates in the case/control files must be at least as precise as ePrecision units.
       gPrint.Printf("Error: The date '%s' of record %ld in the %s must be precise to %s,\n"
                     "       as specified by %s units.\n", BasePrint::P_READERROR,
                     Parser.GetWord(guCountDateIndex), Parser.GetReadCount(),
                     gPrint.GetImpliedFileTypeString().c_str(),
                     GetDatePrecisionAsString(ePrecision, sBuffer, false, false),
                     (gParameters.GetCreationVersionMajor() == 4 ? "time interval" : "time precision"));
      return false; }
    case DateStringParser::INVALID_DATE     :
    default                                 :
      gPrint.Printf("Error: Invalid date '%s' in the %s, record %ld.\n", BasePrint::P_READERROR,
                    Parser.GetWord(guCountDateIndex), gPrint.GetImpliedFileTypeString().c_str(), Parser.GetReadCount());
      return false;
  };
  //validate that date is between study period start and end dates
  if (!(gDataHub.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gDataHub.GetStudyPeriodEndDate())) {
    gPrint.Printf("Error: The date '%s' in record %ld of the %s is not\n"
                  "       within the study period beginning %s and ending %s.\n",
                  BasePrint::P_READERROR, Parser.GetWord(2), Parser.GetReadCount(),
                  gPrint.GetImpliedFileTypeString().c_str(), gParameters.GetStudyPeriodStartDate().c_str(),
                  gParameters.GetStudyPeriodEndDate().c_str());
    return false;
  }
  return true;
}

/** Returns new data gateway. Caller is responsible for deleting object.
    If number of data sets is more than one, a MultipleDataSetGateway
    object is returned. Else a  DataSetGateway object is returned. */
AbstractDataSetGateway * DataSetHandler::GetNewDataGatewayObject() const {
  if (gvDataSets.size() > 1)
    return new MultipleDataSetGateway();
  else
    return new DataSetGateway();
}

/** Returns const pointer to randomizer object associated with data set at iSetIndex. */
const AbstractRandomizer * DataSetHandler::GetRandomizer(size_t iSetIndex) const {
   if (iSetIndex >= gvDataSetRandomizers.size())
     ZdGenerateException("Index '%u' is out of range, %u randomizers exist.","GetRandomizer()",
                         iSetIndex, gvDataSetRandomizers.size());

   return gvDataSetRandomizers[iSetIndex];
}

/** Returns a collection of cloned randomizers maintained by data set handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& DataSetHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  try {
    Container = gvDataSetRandomizers;
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRandomizerContainer()","DataSetHandler");
    throw;
  }
  return Container;
}

/** Fills passed container with simulation data objects, with appropriate members
    of data object allocated. */
SimulationDataContainer_t& DataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gParameters.GetNumDataSets(); ++t)
    Container.push_back(new SimDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), t + 1));
  return AllocateSimulationData(Container);
}

/** Attempts to parses passed string into tract identifier, count,
    and based upon settings, date and covariate information.
    Returns whether parse completed without errors. */
bool DataSetHandler::ParseCountLine(PopulationData & thePopulation, StringParser & Parser,
                                       tract_t& tid, count_t& nCount,
                                       Julian& nDate, int& iCategoryIndex) {
                                       
  short         iCategoryOffSet;

  try {
    //read and validate that tract identifier exists in coordinates file
    //caller function already checked that there is at least one record
    if ((tid = gDataHub.GetTInfo()->tiGetTractIndex(Parser.GetWord(guLocationIndex))) == -1) {
      gPrint.Printf("Error: Unknown location ID in %s, record %ld.\n"
                    "       Location ID '%s' was not specified in the coordinates file.\n",
                    BasePrint::P_READERROR, gPrint.GetImpliedFileTypeString().c_str(),
                    Parser.GetReadCount(), Parser.GetWord(guLocationIndex));
      return false;
    }
    //read and validate count
    if (Parser.GetWord(guCountIndex) != 0) {
      if (!sscanf(Parser.GetWord(1), "%ld", &nCount)) {
       gPrint.Printf("Error: The value '%s' of record %ld in %s could not be read as case count.\n"
                     "       Case count must be an integer.\n", BasePrint::P_READERROR,
                     Parser.GetWord(guCountIndex), Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
       return false;
      }
    }
    else {
      gPrint.Printf("Error: Record %ld in %s does not contain case count.\n",
                    BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (nCount < 0) {//validate that count is not negative or exceeds type precision
      if (strstr(Parser.GetWord(guCountIndex), "-"))
        gPrint.Printf("Error: Record %ld, of the %s, contains a negative case count.\n",
                      BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      else
        gPrint.Printf("Error: Case count '%s' exceeds the maximum allowed value of %ld in record %ld of %s.\n",
                      BasePrint::P_READERROR, Parser.GetWord(1), std::numeric_limits<count_t>::max(),
                      Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
      return false;
    }
    if (!ConvertCountDateToJulian(Parser, nDate))
      return false;
    iCategoryOffSet = gParameters.GetPrecisionOfTimesType() == NONE ? guCountCategoryIndexNone : guCountCategoryIndex;
    if (! ParseCovariates(thePopulation, iCategoryIndex, iCategoryOffSet, Parser))
        return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ParseCountLine()","DataSetHandler");
    throw;
  }
  return true;
}

/** Parses count file data line to determine category index given covariates contained in line.*/
bool DataSetHandler::ParseCovariates(PopulationData & thePopulation, int& iCategoryIndex, short iCovariatesOffset, StringParser & Parser) {
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
        gPrint.Printf("Error: Record %ld of %s contains %d covariate%s but covariates are not permitted\n"
                      "       in the %s when a population file is not specified.\n", BasePrint::P_READERROR,
                      Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                      iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"),
                      gPrint.GetImpliedFileTypeString().c_str());
        return false;
      }
      if (iNumCovariatesScanned != thePopulation.GetNumCovariatesPerCategory()) {
        gPrint.Printf("Error: Record %ld of %s contains %d covariate%s but the population file\n"
                      "       defined the number of covariates as %d.\n", BasePrint::P_READERROR,
                      Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str(),
                      iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"),
                      thePopulation.GetNumCovariatesPerCategory());
        return false;
      }
      //category should already exist
      if ((iCategoryIndex = thePopulation.GetCovariateCategoryIndex(vCategoryCovariates)) == -1) {
        gPrint.Printf("Error: Record %ld of %s refers to a population category that\n"
                      "       does not match an existing category as read from the population file.",
                      BasePrint::P_READERROR, Parser.GetReadCount(), gPrint.GetImpliedFileTypeString().c_str());
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
    x.AddCallpath("ParseCovariates()","DataSetHandler");
    throw;
  }
  return true;
}

/** Randomizes data of passed collection of simulation sets in concert with
    real data through passed collection of passed randomizers. */
void DataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  for (size_t t=0; t < gvDataSets.size(); ++t)
     Container[t]->RandomizeData(*gvDataSets[t], *SimDataContainer[t], iSimulationNumber);
}

/** Read the case data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataSetHandler::ReadCaseFile(size_t iSetIndex) {
  bool          bValid=true;
  FILE        * fp=0;

  try {
    if ((fp = fopen(gParameters.GetCaseFileName(iSetIndex + 1).c_str(), "r")) == NULL) {
      gPrint.Printf("Error: Could not open the case file:\n'%s'.\n",
                    BasePrint::P_ERROR, gParameters.GetCaseFileName(iSetIndex + 1).c_str());
      return false;
    }                                                                  
    gPrint.SetImpliedInputFileType(BasePrint::CASEFILE, (GetNumDataSets() == 1 ? 0 : iSetIndex + 1));
    AllocateCaseStructures(iSetIndex);
    bValid = ReadCounts(iSetIndex, fp, "case");
    fclose(fp); fp=0;
  }
  catch (ZdException & x) {
    if (fp) fclose(fp);
    x.AddCallpath("ReadCaseFile()","DataSetHandler");
    throw;
  }
  return bValid;
}

/** Read the count(either case or control) data file.
    If invalid data is found in the file, an error message is printed,
    that record is ignored, and reading continues.
    Return value: true = success, false = errors encountered           */
bool DataSetHandler::ReadCounts(size_t iSetIndex, FILE* fp, const char* szDescription) {
  int                                   i, iCategoryIndex;
  bool                                  bCaseFile, bValid=true, bEmpty=true;
  Julian                                Date;
  tract_t                               TractIndex;
  StringParser                          Parser(gPrint);
  std::string                           sBuffer;
  count_t                               Count, ** pCounts;

  try {
    RealDataSet& DataSet = *gvDataSets[iSetIndex];

    bCaseFile = !strcmp(szDescription, "case");
    pCounts = (bCaseFile ? DataSet.GetCaseArray() : DataSet.GetControlArray());

    //Read data, parse and if no errors, increment count for tract at date.
    while (Parser.ReadString(fp)) {
         if (Parser.HasWords()) {
           bEmpty = false;
           if (ParseCountLine(DataSet.GetPopulationData(), Parser, TractIndex, Count, Date, iCategoryIndex)) {
             //cumulatively add count to time by location structure
             pCounts[0][TractIndex] += Count;
             if (pCounts[0][TractIndex] < 0)
               GenerateResolvableException("Error: The total %s, in dataset %u, is greater than the maximum allowed of %ld.\n", "ReadCounts()",
                                           (bCaseFile ? "cases" : "controls"), iSetIndex, std::numeric_limits<count_t>::max());
             for (i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
               pCounts[i][TractIndex] += Count;
             //record count as a case or control  
             if (bCaseFile)
               DataSet.GetPopulationData().AddCovariateCategoryCaseCount(iCategoryIndex, Count);
             else
               DataSet.GetPopulationData().AddCovariateCategoryControlCount(iCategoryIndex, Count);
           }
           else
             bValid = false;
         }
    }
    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (! bValid)
      gPrint.Printf("Please see the '%s file' section in the user guide for help.\n", BasePrint::P_ERROR, szDescription);
    //print indication if file contained no data
    else if (bEmpty) {
      gPrint.Printf("Error: The %s file does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
      bValid = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadCounts()","DataSetHandler");
    throw;
  }
  return bValid;
}

/** reports whether any dataset has cases with a zero population. */
void DataSetHandler::ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) {
  if (!gParameters.GetSuppressingWarnings())
    for (size_t t=0; t < gvDataSets.size(); ++t)
      gvDataSets[t]->GetPopulationData().ReportZeroPops(Data, pDisplay, *pPrintDirection);
}

void DataSetHandler::SetPurelyTemporalMeasureData(RealDataSet& DataSet) {
  try {
    DataSet.SetPTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasureData()","DataSetHandler");
    throw;
  }
}

/** sets temporal simulation case array from data in simulation case array */
void DataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  try {
    for (size_t t=0; t < SimDataContainer.size(); ++t)
       SimDataContainer[t]->SetPTCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimulationData()","DataSetHandler");
    throw;
  }
}

/** internal initialization */
void DataSetHandler::Setup() {
  try {
    for (unsigned int i=0; i < gParameters.GetNumDataSets(); ++i)
      gvDataSets.push_back(new RealDataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts(), i + 1));
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataSetHandler");
    throw;
  }
}

