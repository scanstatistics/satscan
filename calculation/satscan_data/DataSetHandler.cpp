//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetHandler.h"
#include "SaTScanData.h"
#include "DateStringParser.h"
#include "SSException.h"
#include "DataSource.h" 
#include "ClosedLoopData.h"
#include "DataDemographics.h"

const short DataSetHandler::_identifier_column_index    = 0;
const short DataSetHandler::guCountIndex                = 1;
const short DataSetHandler::guCountDateIndex            = 2;
const short DataSetHandler::guCountCategoryIndexNone    = 2;
const short DataSetHandler::guCountCategoryIndex        = 3;

/** constructor */
DataSetHandler::DataSetHandler(CSaTScanData& DataHub, BasePrint& Print) : gDataHub(DataHub), gParameters(DataHub.GetParameters()), gPrint(Print), _approximate_case_records(0) {
    try {
        Setup();
    } catch(prg_exception& x) {
        x.addTrace("constructor()","DataSetHandler");
        throw;
    }
}

/* virtual method which builds Oliveira data sets. */
const RealDataContainer_t & DataSetHandler::buildOliveiraDataSets() {
    throw prg_error("buildOliveiraDataSets() is not implemented.", "DataSetHandler::buildOliveiraDataSets()");
}

/** Returns new data gateway. Caller is responsible for deleting object.
    If number of data sets is more than one, a MultipleDataSetGateway
    object is returned, else a DataSetGateway object is returned. */
AbstractDataSetGateway * DataSetHandler::GetNewDataGatewayObject() const {
  if (gvDataSets.size() > 1)
    return new MultipleDataSetGateway();
  else
    return new DataSetGateway();
}

AbstractDataSetGateway & DataSetHandler::GetOliveraDataGateway(AbstractDataSetGateway& DataGatway, const SimulationDataContainer_t& Container) const {
    throw prg_error("GetOliveraDataGateway() is not implemented.", "DataSetHandler::GetOliveraDataGateway()");
}

/** Returns const pointer to randomizer object associated with data set at iSetIndex. */
AbstractRandomizer * DataSetHandler::GetRandomizer(size_t iSetIndex) {
   return gvDataSetRandomizers.at(iSetIndex);
}

/** Returns const pointer to randomizer object associated with data set at iSetIndex. */
const AbstractRandomizer * DataSetHandler::GetRandomizer(size_t iSetIndex) const {
   return gvDataSetRandomizers.at(iSetIndex);
}

/** Returns a collection of cloned randomizers maintained by data set handler.
    All previous elements of list are deleted. */
RandomizerContainer_t& DataSetHandler::GetRandomizerContainer(RandomizerContainer_t& Container) const {
  try {
    Container = gvDataSetRandomizers;
  }
  catch (prg_exception& x) {
    x.addTrace("GetRandomizerContainer()","DataSetHandler");
    throw;
  }
  return Container;
}

/** Fills passed container with DataSet objects. Calls AllocateSimulationData on this container. */
SimulationDataContainer_t& DataSetHandler::GetSimulationDataContainer(SimulationDataContainer_t& Container) const {
  Container.clear();
  for (unsigned int t=0; t < gDataHub.GetNumDataSets(); ++t)
	  Container.push_back(new DataSet(gDataHub.GetNumTimeIntervals(), gDataHub.GetNumIdentifiers(), gDataHub.getIdentifierInfo().getMetaManagerProxy().getNumMeta(), gParameters, t + 1));
  return AllocateSimulationData(Container);
}

/* Prints message to direction class for respective implied file read. */
void DataSetHandler::printFileReadMessage(BasePrint::eInputFileType impliedFile, size_t iSetIndex, bool oneDataSet) {
    gPrint.SetImpliedInputFileType(impliedFile);
    if (oneDataSet)
        gPrint.Printf("Reading the %s\n", BasePrint::P_STDOUT, gPrint.GetImpliedFileTypeString().c_str());
    else
        gPrint.Printf("Reading the %s for data set %u\n", BasePrint::P_STDOUT, gPrint.GetImpliedFileTypeString().c_str(), iSetIndex + 1);
}

/* Prints appropriate message for case status and data set to print direction class. */
void DataSetHandler::printReadStatusMessage(DataSetHandler::CountFileReadStatus status, bool isControls, size_t iSetIndex, bool oneDataSet) const {
    switch (status) {
        case DataSetHandler::ReadSuccess:
            break;
        case DataSetHandler::ReadError:
            gPrint.Printf("Please see the '%s' section in the user guide for help.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
            break;
        case DataSetHandler::NoCounts:
            // Skip this message for drilldown analyses since it will be redundantly reported.
            if (gDataHub.isDrilldown())
                break;
            if (oneDataSet)
                gPrint.Printf("Error: The %s does not contain data.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
            else
                gPrint.Printf("Warning: The %s in data set %u does not contain data. This data set will be ignored by analysis.\n", 
                    BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str(), iSetIndex + 1
                );
            break;
        case DataSetHandler::NotMinimum:
            if (oneDataSet)
                gPrint.Printf("Error: The %s does not contain the appropriate minimum.\n", BasePrint::P_ERROR, gPrint.GetImpliedFileTypeString().c_str());
            else
                gPrint.Printf("Warning: The %s in data set %u does not contain the appropriate minimum. This data set will be ignored by analysis.\n",
                    BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str(), iSetIndex + 1
                );
            break;
        default: throw prg_error("Unknown DataSetHandler::CountFileReadStatus enumeration (%d).", "printReadStatusMessage()", status);
    }
}

/** Randomizes data of passed collection of DataSet objects in concert with
    internal RealDataSet objects through passed collection of randomizer objects. */
void DataSetHandler::RandomizeData(RandomizerContainer_t& Container, SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) const {
  for (size_t t=0; t < gvDataSets.size(); ++t)
     Container.at(t)->RandomizeData(*gvDataSets.at(t), *SimDataContainer.at(t), iSimulationNumber);
}

/** Attempts to read case file data into RealDataSet object. Returns boolean indication of read success. */
DataSetHandler::CountFileReadStatus DataSetHandler::ReadCaseFile(RealDataSet& DataSet) {
    try {
        gPrint.SetImpliedInputFileType(BasePrint::CASEFILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(gParameters.GetCaseFileName(DataSet.getSetIndex()), gParameters.getTimestamp(), true),
            gParameters.getInputSource(CASEFILE, DataSet.getSetIndex()), gPrint)
        );
        DataSetHandler::CountFileReadStatus readStatus = ReadCounts(DataSet, *Source); // Now we're ready to read the case file.
        _approximate_case_records += static_cast<unsigned long>(Source->GetCurrentRecordIndex());
        return readStatus;
    } catch (prg_exception& x) {
        x.addTrace("ReadCaseFile()","DataSetHandler");
        throw;
    }
}

/** Reads the count data source, storing data in RealDataSet object. As a
    means to help user clean-up their data, continues to read records as errors
    are encountered. Returns boolean indication of read success. */
DataSetHandler::CountFileReadStatus DataSetHandler::ReadCounts(RealDataSet& DataSet, DataSource& Source) {
    int                                   iCategoryIndex;
    bool                                  bCaseFile;
    Julian                                Date;
    tract_t                               TractIndex;
    std::string                           sBuffer;
    count_t                               Count, ** ppCounts, totalCount = 0;
    DataSetHandler::CountFileReadStatus   readStatus = DataSetHandler::NoCounts;

    try {
        bCaseFile = gPrint.GetImpliedInputFileType() == BasePrint::CASEFILE;
        ppCounts = (bCaseFile ? DataSet.allocateCaseData().GetArray() : DataSet.allocateControlData().GetArray());
        //Read data, parse and if no errors, increment count for tract at date.
        while (!gPrint.GetMaximumReadErrorsPrinted() && Source.ReadRecord()) {
            DataSetHandler::RecordStatusType eRecordStatus = RetrieveCaseRecordData(DataSet.getPopulationData(), Source, TractIndex, Count, Date, iCategoryIndex);
            if (eRecordStatus == DataSetHandler::Accepted) {
                readStatus = readStatus == DataSetHandler::NoCounts ? DataSetHandler::ReadSuccess : readStatus;
                //cumulatively add count to time by location structure
                ppCounts[0][TractIndex] += Count;
                if (ppCounts[0][TractIndex] < 0)
                    throw resolvable_error(
                        "Error: The total %s, in dataset %u, is greater than the maximum allowed of %ld.\n",
                        (bCaseFile ? "cases" : "controls"), DataSet.getSetIndex(), std::numeric_limits<count_t>::max()
                    );
                if (gParameters.GetAnalysisType() == SEASONALTEMPORAL && gParameters.GetProbabilityModelType() != POISSON) {
                    Date = gDataHub.convertToSeasonalDate(Date);
                    for (int i=1; Date >= gDataHub.GetTimeIntervalStartTimes()[i]; ++i)
                        ppCounts[i][TractIndex] += Count;
                } else {
                    for (int i=1; Date >= gDataHub.CSaTScanData::GetTimeIntervalStartTimes()[i]; ++i)
                        ppCounts[i][TractIndex] += Count;
                }
                // record count as a case or control
                if (bCaseFile)
                    DataSet.getPopulationData().AddCovariateCategoryCaseCount(iCategoryIndex, Count);
                else
                    DataSet.getPopulationData().AddCovariateCategoryControlCount(iCategoryIndex, Count);
                // add to totals and check against numeric limits
                totalCount += Count;
                // check that total count does not exceed data type limitations
                if (totalCount < 0)
                    throw resolvable_error(
                        "Error: The total number of %s in dataset %u is greater than the maximum allowed of %ld.\n", 
                        (bCaseFile ? "cases" : "controls"), DataSet.getSetIndex() + 1, std::numeric_limits<count_t>::max()
                    );
            }
            else if (eRecordStatus == DataSetHandler::Ignored)
                continue;
            else
                readStatus = DataSetHandler::ReadError;
        }
        // Record total in data set.
        if (readStatus == DataSetHandler::ReadSuccess) {
            if (bCaseFile) DataSet.setTotalCases(totalCount); else DataSet.setTotalControls(totalCount);
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadCounts()","DataSetHandler");
        throw;
    }
    return readStatus;
}


/** reports whether any dataset has cases with a zero population. */
void DataSetHandler::ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) {
  if (!gParameters.GetSuppressingWarnings())
    for (size_t t=0; t < gvDataSets.size(); ++t)
      gvDataSets.at(t)->getPopulationData().ReportZeroPops(Data, pDisplay, *pPrintDirection);
}

/* Removes data sets from collection which do not contain case, or potentially control, data. 
   Throws resolvable_error if no data sets have data. */
void DataSetHandler::removeDataSetsWithNoData() {
    size_t numSets = gvDataSets.size();
    bool casesExist = false, controlsExist = false;
    for (int d = numSets - 1; d >= 0; --d) {
        bool setHasCases = gvDataSets[d]->getTotalCases() > 0;
        bool setHasControls = gvDataSets[d]->getTotalControls() > 0;
        casesExist |= setHasCases;
        controlsExist |= setHasControls;
        if (!setHasCases || (gParameters.GetProbabilityModelType() == BERNOULLI && !setHasControls)) {
            gvDataSets.kill(gvDataSets.begin() + d);
            gvDataSetRandomizers.kill(gvDataSetRandomizers.begin() + d);
            _removed_data_set_details.push_back(RemovedDataSetDetails_t(d, !setHasCases, (gParameters.GetProbabilityModelType() == BERNOULLI && !setHasControls)));
        }
    }
    if (!casesExist)
        throw resolvable_error("Error: Analysis stopped. No cases were found in input data.\n");
    if (gParameters.GetProbabilityModelType() == BERNOULLI && !controlsExist)
        throw resolvable_error("Error: Analysis stopped. No controls were found in input data.\n");
    if (!gvDataSets.size()) // It's possible there are cases in some data sets, controls in others, and no data sets left.
        throw resolvable_error("Error: Analysis stopped. No data sets to analyze.\n");
}

/** Returns the relative data set index, taking into account the data sets that have been removed. 
    For instance indices are 0, 1, 2, 3 with 4 data sets. If data set 1 is removed, we are only accessing 3 data sets, so:
    - calling with iSet = 0 returns 0
    - calling with iSet = 1 returns 2
    - calling with iSet = 2 returns 3
*/
size_t DataSetHandler::getDataSetRelativeIndex(size_t iSet) const {
    boost::dynamic_bitset<> sets(gParameters.getNumFileSets());
    sets.set(); // set all as included
    for (auto const& removed: _removed_data_set_details) // toggle off removed sets
        sets.set(removed.get<0>(), false);
    int idx = -1;
    for (size_t s=0; s < sets.size(); ++s) {
        if (sets.test(s))
            ++idx;
        if (idx >= 0 && static_cast<size_t>(idx) == iSet)
            return s;
    }
    return 0;
}

/* Attempts to retrieve number of cases from current record. */
DataSetHandler::RecordStatusType DataSetHandler::RetrieveCaseCounts(DataSource& Source, count_t& nCount) const {
    if (Source.GetValueAt(guCountIndex) != 0) {
        if (!string_to_type<count_t>(Source.GetValueAt(guCountIndex), nCount) || nCount < 0) {
            gPrint.Printf(
                "Error: The value '%s' of record %ld in %s could not be read as case count.\n"
                "       Case count must be a whole number in range 0 - %u.\n", BasePrint::P_READERROR,
                Source.GetValueAt(guCountIndex), Source.GetCurrentRecordIndex(),
                gPrint.GetImpliedFileTypeString().c_str(), std::numeric_limits<count_t>::max()
            );
            return DataSetHandler::Rejected;
        }
        if (gParameters.getReadingLineDataFromCasefile() && Source.hasIndividualLinelistMapping() && nCount > 1) {
            gPrint.Printf(
                "Error: The case count in record %ld of %s is not valid for current parameter settings and file data.\n"
                "       Case count must be 1 or 0 when case file includes event id as part of line-list data.\n", BasePrint::P_READERROR,
                Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
            );
            return DataSetHandler::Rejected;
        }
        if (nCount == 0) return DataSetHandler::Ignored;
    } else {
        gPrint.Printf(
            "Error: Record %ld in %s does not contain case count.\n",
            BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
        );
        return DataSetHandler::Rejected;
    }
    return DataSetHandler::Accepted;
}

/** Retrieves count date from current record of data source as a julian date. If an error
    in data format is detected, appropriate error message is sent to BasePrint object
    and DataSetHandler::Rejected is returned. If user requested that dates outside study
    period be ignored, prints warning message to BasePrint object when situation encountered
    and returns DataSetHandler::Ignored. */
DataSetHandler::RecordStatusType DataSetHandler::RetrieveCountDate(DataSource& Source, Julian & JulianDate) const {
  DateStringParser                      DateParser(gParameters.GetPrecisionOfTimesType());
  DateStringParser::ParserStatus        eStatus;
  DatePrecisionType                     ePrecision;

  //If parameters indicate that case data does not contain dates, don't try to
  //read a date, or validate that there isn't one (could be covariate), and set
  //Julian reference to study period start date.
  if (gParameters.GetPrecisionOfTimesType() == NONE) {
    JulianDate = gDataHub.GetStudyPeriodStartDate();
    return DataSetHandler::Accepted;
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
  if (!Source.GetValueAt(guCountDateIndex)) {
    gPrint.Printf("Error: Record %ld in %s does not contain a date.\n",
                  BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
    return DataSetHandler::Rejected;
  }
  //Attempt to convert string into Julian equivalence.
  eStatus = DateParser.ParseCountDateString(Source.GetValueAt(guCountDateIndex), ePrecision,
                                            gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodStartDate(), JulianDate);
  switch (eStatus) {
    case DateStringParser::VALID_DATE       : break;
    case DateStringParser::AMBIGUOUS_YEAR   :
      gPrint.Printf("Error: Due to the study period being greater than 100 years, unable\n"
                    "       to determine century for two digit year in %s, record %ld.\n"
                    "       Please use four digit years.\n", BasePrint::P_READERROR,
                    gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex());
      return DataSetHandler::Rejected;
    case DateStringParser::LESSER_PRECISION : {
       std::string sBuffer;
       //Dates in the case/control files must be at least as precise as ePrecision units.
       gPrint.Printf("Error: The date '%s' of record %ld in the %s must be precise to %s,\n"
                     "       as specified by %s units.\n", BasePrint::P_READERROR,
                     Source.GetValueAt(guCountDateIndex), Source.GetCurrentRecordIndex(),
                     gPrint.GetImpliedFileTypeString().c_str(),
                     GetDatePrecisionAsString(ePrecision, sBuffer, false, false),
                     (gParameters.GetCreationVersionMajor() == 4 ? "time interval" : "time precision"));
      return DataSetHandler::Rejected; }
    case DateStringParser::INVALID_DATE     :
    default                                 :
      gPrint.Printf("Error: Invalid date '%s' in the %s, record %ld.\n%s", BasePrint::P_READERROR,
                    Source.GetValueAt(guCountDateIndex), gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), DateParser.getLastParseError().c_str());
      return DataSetHandler::Rejected;
  };
  //validate that date is between study period start and end dates
  if (!(gDataHub.GetStudyPeriodStartDate() <= JulianDate && JulianDate <= gDataHub.GetStudyPeriodEndDate())) {
    if (gParameters.GetStudyPeriodDataCheckingType() == STRICTBOUNDS) {
      gPrint.Printf("Error: The date '%s' in record %ld of the %s is not\n"
                    "       within the study period beginning %s and ending %s.\n",
                    BasePrint::P_READERROR, Source.GetValueAt(guCountDateIndex), Source.GetCurrentRecordIndex(),
                    gPrint.GetImpliedFileTypeString().c_str(), gParameters.GetStudyPeriodStartDate().c_str(),
                    gParameters.GetStudyPeriodEndDate().c_str());
      return DataSetHandler::Rejected;
    }
    if (!gDataHub.isDrilldown() && std::find(gmSourceDateWarned.begin(), gmSourceDateWarned.end(), reinterpret_cast<void*>(&Source)) == gmSourceDateWarned.end()) {
      gPrint.Printf("Warning: Some records in %s are outside the specified Study Period.\n"
                    "         These will be ignored in the analysis.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str());
      gmSourceDateWarned.push_back(reinterpret_cast<void*>(&Source));
    }
    return DataSetHandler::Ignored;
  }
  return DataSetHandler::Accepted;
}

/** Attmepts to retrieve case data from current record of data source. If an error
    in data format is detected, appropriate error message is sent to BasePrint object
    and DataSetHandler::Rejected is returned. */
DataSetHandler::RecordStatusType DataSetHandler::RetrieveCaseRecordData(PopulationData& thePopulation, DataSource& Source, tract_t& tid, count_t& nCount, Julian& nDate, int& iCategoryIndex) {
    try {
        DataSetHandler::RecordStatusType eStatus = RetrieveIdentifierIndex(Source, tid); //read and validate that tract identifier
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        eStatus = RetrieveCaseCounts(Source, nCount); // read and validate count
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        eStatus = RetrieveCountDate(Source, nDate); // read and validate date
        if (eStatus != DataSetHandler::Accepted) return eStatus;
        if (gParameters.getAdjustForWeeklyTrends() && gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
            // If performing weekly adjustment for STP, set define additional covariate.
            PopulationData::CovariatesNames_t vCategoryCovariates;
            vCategoryCovariates.push_back(std::string(""));
            printString(vCategoryCovariates.back(), "%u", nDate % 7);
            thePopulation.setAdditionalCovariates(vCategoryCovariates);
        }
        short iCategoryOffSet = gParameters.GetPrecisionOfTimesType() == NONE ? guCountCategoryIndexNone : guCountCategoryIndex;
        if (!RetrieveCovariatesIndex(thePopulation, iCategoryIndex, iCategoryOffSet, Source)) return DataSetHandler::Rejected;
    } catch (prg_exception& x) {
        x.addTrace("RetrieveCaseRecordData()","DataSetHandler");
        throw;
    }
    return DataSetHandler::Accepted;
}

/** Attempts to read covariate categories from data source at specified field offset. For the Poisson
    model, ensures that category already exists - as defined in population file. For Bernoulli model,
    covariates are not used and are ignored - returning index of zero. For the space-time model,
    creates covariates as defined in each record. */
bool DataSetHandler::RetrieveCovariatesIndex(PopulationData & thePopulation, int& iCategoryIndex, short iCovariatesOffset, DataSource& Source) {
    int iNumCovariatesScanned=0;
    std::vector<std::string> vCategoryCovariates;
    const char * pCovariate;

    try {
        if (gParameters.GetProbabilityModelType() == POISSON) {
            while ((pCovariate = Source.GetValueAt(iCovariatesOffset + iNumCovariatesScanned)) != 0) {
                if (Source.isLinelistOnlyColumn(iCovariatesOffset + iNumCovariatesScanned)) {
                    ++iCovariatesOffset;
                    continue;
                }
                vCategoryCovariates.push_back(pCovariate);
                if (trimString(vCategoryCovariates.back()).size() == 0) {
                    gPrint.Printf("Error: Record %ld of %s contains covariate with no value (empty string).\n", BasePrint::P_READERROR,
                        Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str()
                    );
                    return false;
                }
                ++iNumCovariatesScanned;
            }
            if (!gParameters.UsePopulationFile() && iNumCovariatesScanned) {
                //If the population data was not gotten from a population file, then there can not
                //be covariates in other files, namely the case file.
                gPrint.Printf("Error: Record %ld of %s contains %d covariate%s but covariates are not permitted\n"
                              "       in the %s when a population file is not specified.\n", BasePrint::P_READERROR,
                              Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(),
                              iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"),
                            gPrint.GetImpliedFileTypeString().c_str());
                return false;
            }
            if (iNumCovariatesScanned != thePopulation.GetNumCovariatesPerCategory()) {
                gPrint.Printf("Error: Record %ld of %s contains %d covariate%s but the population file\n"
                              "       defined the number of covariates as %d.\n", BasePrint::P_READERROR,
                              Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str(),
                              iNumCovariatesScanned, (iNumCovariatesScanned == 1 ? "" : "s"),
                            thePopulation.GetNumCovariatesPerCategory());
                return false;
            }
            //category should already exist
            if ((iCategoryIndex = thePopulation.GetCovariateCategoryIndex(vCategoryCovariates)) == -1) {
                gPrint.Printf("Error: Record %ld of %s refers to a population category that\n"
                              "       does not match an existing category as read from the population file.",
                              BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
                return false;
            }
        } else if (gParameters.GetProbabilityModelType() == BERNOULLI || gParameters.GetProbabilityModelType() == UNIFORMTIME) {
            //For the Bernoulli and uniform time models, ignore covariates in the case and control files
            //All population categories are aggregated in one category.
            iCategoryIndex = 0;
        } else if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION) {
            //First category created sets precedence as to how many covariates remaining records must have.
            if ((iCategoryIndex = thePopulation.CreateCovariateCategory(Source, iCovariatesOffset, gPrint)) == -1)
                return false;
        } else
            throw prg_error("Unknown probability model type '%d'.","RetrieveCovariatesIndex()", gParameters.GetProbabilityModelType());
    } catch (prg_exception& x) {
        x.addTrace("RetrieveCovariatesIndex()","DataSetHandler");
        throw;
    }
    return true;
}

/** Retrieves the identifier index from data source. If identifier not found:
    - if coordinates data checking is strict, reports error to BasePrint object and returns SaTScanDataReader::Rejected
    - if coordinates data checking is relaxed, reports warning to BasePrint object and returns SaTScanDataReader::Ignored; reports only first occurance
    - else returns SaTScanDataReader::Accepted */
DataSetHandler::RecordStatusType DataSetHandler::RetrieveIdentifierIndex(DataSource& Source, tract_t& tLocationIndex) const {
    //Validate that tract identifer is one of those defined in the coordinates file.
    const char * identifier = Source.GetValueAt(_identifier_column_index);
    if (!identifier || strlen(identifier) == 0) {
        gPrint.Printf("Error: Missing identifier is missing in record %ld of %s.\n", BasePrint::P_READERROR,
                      Source.GetCurrentRecordIndex(), gPrint.GetImpliedFileTypeString().c_str());
        return DataSetHandler::Rejected;
    }
	auto identifierIdx = gDataHub.getIdentifierInfo().getIdentifierIndex(identifier);
    if (!identifierIdx) {
        std::string identifierSource;
        if (gParameters.UseLocationNeighborsFile()) 
            identifierSource = gPrint.getSourceFilenameForType(BasePrint::LOCATION_NEIGHBORS_FILE);
        else if (gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION)
            identifierSource = gPrint.getSourceFilenameForType(BasePrint::MULTIPLE_LOCATIONS);
        else 
            identifierSource = gParameters.getUseLocationsNetworkFile() ? gPrint.getSourceFilenameForType(BasePrint::NETWORK_FILE) : gPrint.getSourceFilenameForType(BasePrint::COORDFILE);
        if (gParameters.GetCoordinatesDataCheckingType() == STRICTCOORDINATES) {
            gPrint.Printf(
                "Error: Unknown identifier in %s, record %ld. '%s' was not specified in the %s.\n", BasePrint::P_READERROR,
                gPrint.GetImpliedFileTypeString().c_str(), Source.GetCurrentRecordIndex(), Source.GetValueAt(_identifier_column_index), identifierSource.c_str()
            );
            return DataSetHandler::Rejected;
        }
        // Report to user if the data checking option is ignoring identifiers because the user requested relaxed checking, that is unless this is a drilldown where we set this option programmatically.
        if (!gDataHub.isDrilldown() && std::find(gmSourceLocationWarned.begin(), gmSourceLocationWarned.end(), reinterpret_cast<void*>(&Source)) == gmSourceLocationWarned.end()) {
            gPrint.Printf(
                "Warning: Some records in %s reference an identifier that was not specified in the %s.\n"
                "         These will be ignored in the analysis.\n", BasePrint::P_WARNING, gPrint.GetImpliedFileTypeString().c_str(), identifierSource.c_str()
            );
            gmSourceLocationWarned.push_back(reinterpret_cast<void*>(&Source));
        }
        return DataSetHandler::Ignored;
    }
	tLocationIndex = static_cast<tract_t>(identifierIdx.get());
    return DataSetHandler::Accepted;
}

/** Sets purely temporal measure array of RealDataSet object. */
void DataSetHandler::SetPurelyTemporalMeasureData(RealDataSet& DataSet) {
  DataSet.setMeasureData_PT();
}

/** Sets purely temporal case array of each DataSet object. */
void DataSetHandler::SetPurelyTemporalSimulationData(SimulationDataContainer_t& SimDataContainer) {
  std::for_each(SimDataContainer.begin(), SimDataContainer.end(), std::mem_fun(&DataSet::setCaseData_PT));
}

/** internal initialization - allocates RealDataSet object for each data set. */
void DataSetHandler::Setup() {
    try {
        // down cast closed loop data to parent class if Poisson model -- we initially want the number of intervals as non-closed loop
        bool downCast = dynamic_cast<ClosedLoopData*>(&gDataHub) != 0 && gParameters.GetProbabilityModelType() == POISSON;
        int intervals = downCast ? gDataHub.CSaTScanData::GetNumTimeIntervals() : gDataHub.GetNumTimeIntervals();
        for (unsigned int i=0; i < gParameters.getNumFileSets(); ++i)
            gvDataSets.push_back(new RealDataSet(intervals, gDataHub.GetNumIdentifiers(), gDataHub.GetNumMetaIdentifiers(), gParameters, i + 1));
    } catch (prg_exception& x) {
        x.addTrace("Setup()","DataSetHandler");
        throw;
    }
}
