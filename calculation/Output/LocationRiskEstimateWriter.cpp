//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LocationRiskEstimateWriter.h"
#include "SaTScanData.h"
#include "SVTTData.h"
#include "SSException.h"
#include "ParametersPrint.h"
#include "WeightedNormalRandomizer.h"

const char * LocationRiskEstimateWriter::REL_RISK_EXT                                   = ".rr";
const char * LocationRiskEstimateWriter::TREND_IN_FIELD                                 = "IN_TREND";
const char * LocationRiskEstimateWriter::TREND_OUT_FIELD                                = "OUT_TREND";
const char * LocationRiskEstimateWriter::ALPHA_IN_FIELD                                 = "IN_ITRCPT";
const char * LocationRiskEstimateWriter::BETA1_IN_FIELD                                 = "IN_LINEAR";
const char * LocationRiskEstimateWriter::BETA2_IN_FIELD                                 = "IN_QUAD";
const char * LocationRiskEstimateWriter::ALPHA_OUT_FIELD                                = "OUT_ITRCPT";
const char * LocationRiskEstimateWriter::BETA1_OUT_FIELD                                = "OUT_LINEAR";
const char * LocationRiskEstimateWriter::BETA2_OUT_FIELD                                = "OUT_QUAD";
const char * LocationRiskEstimateWriter::WEIGHTED_MEAN_VALUE_FIELD                      = "W_MEAN";
const char * LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD                           = "F_MLC";
const char * LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_FIELD                  = "F_HIERARCH";

/** class constructor */
LocationRiskEstimateWriter::LocationRiskEstimateWriter(const CSaTScanData& DataHub): AbstractDataFileWriter(DataHub.GetParameters()) {
    try {
        DefineFields(DataHub);
        if (gParameters.GetOutputRelativeRisksAscii())
            gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, REL_RISK_EXT);
        if (gParameters.GetOutputRelativeRisksDBase())
        gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, REL_RISK_EXT);
        // Compile the list locations to identifiers - this is the opposite of storage.
        const auto& identifiers = DataHub.getIdentifierInfo().getIdentifiers();
        for (size_t idx = 0; idx < static_cast<size_t>(DataHub.GetNumIdentifiers()); ++idx) {
            const auto& locations = identifiers[idx]->getLocations();
            for (size_t t = 0; t < locations.size(); ++t)
                _location_to_identifiers[locations[t]].add(idx, false);
        }
        // Store which locations will be reported in results - sometimes not all locations are utilized by analysis - for example this could happen with networks.
        _report_locations.resize(DataHub.getLocationsManager().locations().size());
        for (auto& loc_ids : _location_to_identifiers)
            _report_locations.set(loc_ids.first->index());
        // Report all meta identifiers/locations.
        _report_locations.resize(_report_locations.size() + DataHub.GetNumMetaIdentifiers(), true);
    } catch (prg_exception& x) {
        delete gpASCIIFileWriter; gpASCIIFileWriter=0;
        delete gpDBaseFileWriter; gpDBaseFileWriter=0;
        x.addTrace("constructor","LocationRiskEstimateWriter");
        throw;
    }
}

/** class destructor */
LocationRiskEstimateWriter::~LocationRiskEstimateWriter() {}

// sets up the vector of field structs so that the FieldDef Vector can be created
// pre: none
// post : returns through reference a vector of FieldDef to determine the structure of the data
void LocationRiskEstimateWriter::DefineFields(const CSaTScanData& DataHub) {
    unsigned short uwOffset = 0;

    try {
        if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON || 
            gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL /*|| gParameters.GetAnalysisType() == SPATIALVARTEMPTREND*/)
            throw prg_error("Risk estimates file not implemented for %s model.", "SetupFields()", ParametersPrint(DataHub.GetParameters()).GetProbabilityModelTypeAsString());
        CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, getLocationFieldLength(DataHub), 0, uwOffset, 0);
        if (gParameters.getNumFileSets() > 1)
            CreateField(vFieldDefinitions, DATASET_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
            CreateField(vFieldDefinitions, CATEGORY_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        if (gParameters.GetProbabilityModelType() == NORMAL) {
            if (!gParameters.getIsWeightedNormal()) {
                CreateField(vFieldDefinitions, MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else {
                CreateField(vFieldDefinitions, MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, WEIGHTED_MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            }
        } else {
            CreateField(vFieldDefinitions, OBSERVED_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(vFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        }
        if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
            CreateField(vFieldDefinitions, TREND_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, TREND_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            CreateField(vFieldDefinitions, BETA1_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            CreateField(vFieldDefinitions, BETA2_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            CreateField(vFieldDefinitions, ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            CreateField(vFieldDefinitions, BETA1_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            CreateField(vFieldDefinitions, BETA2_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
        }
        if (gParameters.getCalculateOliveirasF()) {
            short precision;
            if (gParameters.getNumRequestedOliveiraSets() <= 100) { precision = 2; 
            } else if (gParameters.getNumRequestedOliveiraSets() <= 1000) { precision = 3;
            } else if (gParameters.getNumRequestedOliveiraSets() <= 10000) { precision = 4;
            } else { /* client specificances not provided */ precision = 5; }
            if (!(gParameters.getReportHierarchicalClusters() || gParameters.getReportGiniOptimizedClusters()))
                CreateField(vFieldDefinitions, OLIVEIRA_F_MLC_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
            if (gParameters.getReportHierarchicalClusters())
                CreateField(vFieldDefinitions, OLIVEIRA_F_HIERARCHICAL_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
        }
    } catch (prg_exception& x) {
        x.addTrace("DefineFields()","LocationRiskEstimateWriter");
        throw;
    }
}

/*  Assigns the location name and obtains the identifier indexes associated with it. 
    
    In the most general and common application, identifier and location reference the same label/name. */
void LocationRiskEstimateWriter::getLocationInfo(unsigned int locIdx, const CSaTScanData& DataHub, std::vector<tract_t>& identifierIndexes, std::string& name, unsigned int nameMax) const {
    const auto& identifierInfo = DataHub.getIdentifierInfo();
    identifierIndexes.clear();
    if (locIdx < identifierInfo.getLocationsManager().locations().size()) { // Is this an actual location?
        const auto& location = identifierInfo.getLocationsManager().locations()[locIdx].get();
        _location_to_identifiers.at(location).get(identifierIndexes);
        name = location->name();
        bool combined = false;
        for (unsigned int t = 0; t < identifierIndexes.size() && !combined; ++t)
            combined = identifierInfo.getIdentifiers()[identifierIndexes[t]]->getCombinedWith().size();
        // If the identifer is combined, try appending the string "et al" to indicate this situation - we detail this in the main results file.
        if (combined && name.size() + strlen(" et al") <= nameMax)
            name += " et al";
    } else { // Or a meta identifier/location
        name = identifierInfo.getMetaIdentifiersManager().getMetaIdentifiers()[(size_t)locIdx - identifierInfo.getIdentifiers().size()]->getIndentifier();
        identifierIndexes.push_back(locIdx);
    }
    if (name.size() > nameMax) // Shorten the name as necessary to fix maximum length.
        name.resize(nameMax);
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::Write(const CSaTScanData& DataHub, const LocationRelevance& location_relevance) {
    try {
        if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
            RecordRelativeRiskDataAsOrdinal(DataHub);
        else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
            RecordRelativeRiskDataAsWeightedNormal(DataHub);
        } else
            RecordRelativeRiskDataStandard(DataHub, location_relevance);
    } catch (prg_exception& x) {
        x.addTrace("Write()","LocationRiskEstimateWriter");
        throw;
    }
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub) {
    count_t * pCases;
    std::vector<count_t> dataSetIdentifierPopulation;

    try {
        for (size_t i=0; i < DataHub.GetNumDataSets(); ++i) {
            const RealDataSet& DataSet = DataHub.GetDataSetHandler().GetDataSet(i);
            const PopulationData& Population = DataSet.getPopulationData();
            tract_t tTotalLocations = DataHub.GetNumIdentifiers() + DataHub.GetNumMetaIdentifiers();
            // first calculate populations for each identifier irrespective of category
            dataSetIdentifierPopulation.assign(tTotalLocations, 0);
            for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
                pCases = DataSet.getCategoryCaseData(j).GetArray()[0];
                for (tract_t m=0; m < tTotalLocations; ++m)
                    dataSetIdentifierPopulation[m] += pCases[m];
            }
            // Iterate over all locations.
            size_t loc_idx = _report_locations.find_first();
            std::vector<tract_t> indentifierIndexes;
            while (loc_idx != _report_locations.npos) {
                // for each category in data set, record relative risk data
                for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                    RecordBuffer Record(vFieldDefinitions);
                    if (gParameters.getNumFileSets() > 1) Record.GetFieldValue(DATASET_FIELD).AsDouble() = DataHub.GetDataSetHandler().getDataSetRelativeIndex(i) + 1;
                    // Obtain location information - the name and which identifier indexes are associated with it.
                    getLocationInfo(loc_idx, DataHub, indentifierIndexes, Record.GetFieldValue(LOC_ID_FIELD).AsString(), static_cast<unsigned long>(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength()));
                    Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = static_cast<double>(j + 1);
                    // Some fields can just be accumulated in the record directly.
                    for (tract_t t: indentifierIndexes) {
                        Record.GetFieldValue(OBSERVED_FIELD).AsDouble() += pCases[t];
                        Record.GetFieldValue(EXPECTED_FIELD).AsDouble() += (double)dataSetIdentifierPopulation[t];
                    }
                    // Others need a specialized process or the accumulation is just clearer in separate routine.
                    Record.GetFieldValue(EXPECTED_FIELD).AsDouble() *= (double)Population.GetNumCategoryTypeCases(j) / DataSet.getTotalPopulation();
                    if (Record.GetFieldValue(EXPECTED_FIELD).AsDouble()) {
                        Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                        double dNumerator = Population.GetNumCategoryTypeCases(j) - Record.GetFieldValue(OBSERVED_FIELD).AsDouble();
                        double dDenominator = Population.GetNumCategoryTypeCases(j) - Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                        if (dDenominator && dNumerator / dDenominator)
                            Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / Record.GetFieldValue(EXPECTED_FIELD).AsDouble()) / (dNumerator / dDenominator);
                    }
                    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
                    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
                    loc_idx = _report_locations.find_next(loc_idx);
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("RecordRelativeRiskDataAsOrdinal(const CSaTScanData&)","RelativeRiskData");
        throw;
    }
}

/* Returns the relative risk for all identifiers - taken as one. */
std::pair<double, double> LocationRiskEstimateWriter::getNormalMeanForIdentifiers(const std::vector<tract_t>& identifiers, count_t * pCases, measure_t * pMeasure, measure_t * pMeasureAux) const {
    count_t casesAll = 0;
    measure_t measureAll = 0, measureauxAll = 0;
    // Accumulate the total observed and total expected across all identifiers.
    for (tract_t t: identifiers) {
        casesAll += pCases[t];
        measureAll += pMeasure[t];
        measureauxAll += pMeasureAux[t];
    }
    std::pair<double, double> returnVal(0, 0);
    if (casesAll) {
        returnVal.first = measureAll / static_cast<double>(casesAll);
        returnVal.second = std::sqrt(GetUnbiasedVariance(casesAll, measureAll, measureauxAll));
    }
    return returnVal;
}

/** Writes observed, expected, observed/expected and relative risk to record.*/
void LocationRiskEstimateWriter::RecordRelativeRiskDataStandard(const CSaTScanData& DataHub, const LocationRelevance& location_relevance) {
    const DataSetHandler& Handler = DataHub.GetDataSetHandler();

    try {
        for (unsigned int i=0; i < Handler.GetNumDataSets(); ++i) {
            count_t * pCases = Handler.GetDataSet(i).getCaseData().GetArray()[0];
            measure_t * pMeasureAux = 0, * pMeasure = Handler.GetDataSet(i).getMeasureData().GetArray()[0];
            if (gParameters.GetProbabilityModelType() == NORMAL) pMeasureAux = Handler.GetDataSet(i).getMeasureData_Aux().GetArray()[0];
            size_t loc_idx = _report_locations.find_first();
            std::vector<tract_t> indentifierIndexes;
            while (loc_idx != _report_locations.npos) {
                RecordBuffer Record(vFieldDefinitions);                
                if (gParameters.getNumFileSets() > 1) Record.GetFieldValue(DATASET_FIELD).AsDouble() = Handler.getDataSetRelativeIndex(i) + 1;
                // Obtain location information - the name and which identifier indexes are associated with it.
                getLocationInfo(loc_idx, DataHub, indentifierIndexes, Record.GetFieldValue(LOC_ID_FIELD).AsString(), static_cast<unsigned long>(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength()));
                // Some fields can just be accumulated in the record directly.
                for (tract_t idx : indentifierIndexes) {
                    if (gParameters.getCalculateOliveirasF()) {
                        if (location_relevance._most_likely_only.size() > idx)
                            Record.GetFieldValue(OLIVEIRA_F_MLC_FIELD).AsDouble() += static_cast<double>(location_relevance._most_likely_only[idx]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                        if (location_relevance._hierarchical.size() > idx)
                            Record.GetFieldValue(OLIVEIRA_F_HIERARCHICAL_FIELD).AsDouble() += static_cast<double>(location_relevance._hierarchical[idx]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                    }
                    if (gParameters.GetProbabilityModelType() != NORMAL) {
                        Record.GetFieldValue(OBSERVED_FIELD).AsDouble() += pCases[idx];
                        Record.GetFieldValue(EXPECTED_FIELD).AsDouble() += DataHub.GetMeasureAdjustment(i) * pMeasure[idx];
                    }
                }
                // Others need a specialized process or the accumulation is just clearer in separate routine.
                if (gParameters.GetProbabilityModelType() == NORMAL) {
                    std::pair<double, double> values = getNormalMeanForIdentifiers(indentifierIndexes, pCases, pMeasure, pMeasureAux);
                    Record.GetFieldValue(MEAN_VALUE_FIELD).AsDouble() = values.first;
                    Record.GetFieldValue(STD_FIELD).AsDouble() = values.second;
                } else if (Record.GetFieldValue(EXPECTED_FIELD).AsDouble()) {
                    Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                    double dDenominator = Handler.GetDataSet(i).getTotalCases() - Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                    double dNumerator = Handler.GetDataSet(i).getTotalCases() - Record.GetFieldValue(OBSERVED_FIELD).AsDouble();
                    if (dDenominator && dNumerator / dDenominator)
                        Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() / (dNumerator / dDenominator);
                }
                if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
                if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
                loc_idx = _report_locations.find_next(loc_idx);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("RecordRelativeRiskDataStandard(const CSaTScanData&)","LocationRiskEstimateWriter");
        throw;
    }
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub) {
    try {
        for (unsigned int i=0; i < DataHub.GetDataSetHandler().GetNumDataSets(); ++i) {
            const AbstractWeightedNormalRandomizer * pRandomizer = 0;
            if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(DataHub.GetDataSetHandler().GetRandomizer(i))) == 0)
                throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.", "RecordRelativeRiskDataAsWeightedNormal()");
            AbstractWeightedNormalRandomizer::RiskEstimateStatistics statistics = pRandomizer->getRiskEstimateStatistics(DataHub);
            size_t loc_idx = _report_locations.find_first();
            std::vector<tract_t> indentifierIndexes;
            while (loc_idx != _report_locations.npos) {
                RecordBuffer Record(vFieldDefinitions);
                if (gParameters.getNumFileSets() > 1) Record.GetFieldValue(DATASET_FIELD).AsDouble() = DataHub.GetDataSetHandler().getDataSetRelativeIndex(i) + 1;
                // Obtain location information - the name and which identifier indexes are associated with it.
                getLocationInfo(loc_idx, DataHub, indentifierIndexes, Record.GetFieldValue(LOC_ID_FIELD).AsString(), static_cast<unsigned long>(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength()));
                // Support for multiple locations is not currently implemented.
                if (indentifierIndexes.size() > 1) throw prg_error("Risk estimates for each location is not implemented for the weighted normal model with multiple locations.", "RecordRelativeRiskDataAsWeightedNormal()");
                for (tract_t t: indentifierIndexes) {
                    Record.GetFieldValue(MEAN_VALUE_FIELD).AsDouble() += statistics.gtMean[t];
                    Record.GetFieldValue(WEIGHTED_MEAN_VALUE_FIELD).AsDouble() += statistics.gtWeightedMean[t];
                }
                if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
                if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
                loc_idx = _report_locations.find_next(loc_idx);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("RecordRelativeRiskDataAsWeightedNormal()","LocationRiskEstimateWriter");
        throw;
    }
}

/** writes relative risk data to record and appends to internal buffer of records
    - particular functionality for SVTT analysis  */
void LocationRiskEstimateWriter::Write(const CSVTTData& DataHub) {
    count_t * pCases, ** ppCasesNC, * pPTCasesNC;
    measure_t * pMeasure, ** ppMeasureNC, * pPTMeasureNC;
    std::auto_ptr<AbstractTimeTrend> TractTimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
    std::auto_ptr<AbstractTimeTrend> GlobalTimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));

    try {
        for (unsigned int i=0; i < DataHub.GetNumDataSets(); ++i) {
            pCases = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData().GetArray()[0];
            pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData().GetArray()[0];
            ppCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData_NC().GetArray();
            ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData_NC().GetArray();            
            pPTCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData_PT_NC();
            pPTMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData_PT_NC();
            // now calculate the trends for each defined location
            size_t loc_idx = _report_locations.find_first();
            std::vector<tract_t> indentifierIndexes;
            while (loc_idx != _report_locations.npos) {
                RecordBuffer Record(vFieldDefinitions);
                if (gParameters.getNumFileSets() > 1) Record.GetFieldValue(DATASET_FIELD).AsDouble() = DataHub.GetDataSetHandler().getDataSetRelativeIndex(i) + 1;
                // Obtain location information - the name and which identifier indexes are associated with it.
                getLocationInfo(loc_idx, DataHub, indentifierIndexes, Record.GetFieldValue(LOC_ID_FIELD).AsString(), static_cast<unsigned long>(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength()));
                // Some fields can just be accumulated in the record directly.
                std::vector<count_t> vTemporalTractCases(DataHub.GetNumTimeIntervals()), vTemporalGlobalCases(DataHub.GetNumTimeIntervals(), 0);
                std::vector<measure_t> vTemporalTractObserved(DataHub.GetNumTimeIntervals()), vTemporalGlobalObserved(DataHub.GetNumTimeIntervals(), 0.0);
                for (tract_t idx: indentifierIndexes) {
                    Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[idx];
                    Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = DataHub.GetMeasureAdjustment(i) * pMeasure[idx];
                    //isolate cases/measure for this location
                    for (unsigned int j = 0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
                        // location cases/measure
                        vTemporalTractCases[j] = ppCasesNC[j][idx];
                        vTemporalTractObserved[j] = ppMeasureNC[j][idx];
                        // all other cases/measure
                        vTemporalGlobalCases[j] = pPTCasesNC[j] - ppCasesNC[j][idx];
                        vTemporalGlobalObserved[j] = pPTMeasureNC[j] - ppMeasureNC[j][idx];
                    }
                }
                // Others need a specialized process or the accumulation is just clearer in separate routine.
                if (Record.GetFieldValue(EXPECTED_FIELD).AsDouble()) {
                    Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                    double dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).getTotalCases() - Record.GetFieldValue(EXPECTED_FIELD).AsDouble();
                    double dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).getTotalCases() - Record.GetFieldValue(OBSERVED_FIELD).AsDouble();
                    if (dDenominator && dNumerator / dDenominator)
                        Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / Record.GetFieldValue(EXPECTED_FIELD).AsDouble()) / (dNumerator / dDenominator);
                }
                // calculate trends for location and global trend
                TractTimeTrend->CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0], DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
                GlobalTimeTrend->CalculateAndSet(&vTemporalGlobalCases[0], &vTemporalGlobalObserved[0], DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
                if (gParameters.getTimeTrendType() == LINEAR) {
                    switch (TractTimeTrend->GetStatus()) {
                        case AbstractTimeTrend::UNDEFINED: break;
                        case AbstractTimeTrend::NEGATIVE_INFINITY:
                            Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR;
                            break;
                        case AbstractTimeTrend::POSITIVE_INFINITY:
                            Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR;
                            break;
                        case AbstractTimeTrend::SINGULAR_MATRIX:
                            throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n", "Write()", Record.GetFieldValue(LOC_ID_FIELD).AsCString());
                        case AbstractTimeTrend::NOT_CONVERGED:
                            throw prg_error("The time trend did not converge.\n", "Write()");
                        case AbstractTimeTrend::CONVERGED:
                            TractTimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                            Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = TractTimeTrend->GetAnnualTimeTrend();
                            break;
                        default: throw prg_error("Unknown time trend status type '%d'.", "Write()", TractTimeTrend->GetStatus());
                    };
                    switch (GlobalTimeTrend->GetStatus()) {
                        case AbstractTimeTrend::UNDEFINED: break;
                        case AbstractTimeTrend::NEGATIVE_INFINITY:
                            Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR;
                            break;
                        case AbstractTimeTrend::POSITIVE_INFINITY:
                            Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR;
                            break;
                        case AbstractTimeTrend::SINGULAR_MATRIX:
                            throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n", "Write()", Record.GetFieldValue(LOC_ID_FIELD).AsCString());
                        case AbstractTimeTrend::NOT_CONVERGED:
                            throw prg_error("The time trend did not converge.\n", "Write()");
                        case AbstractTimeTrend::CONVERGED:
                            GlobalTimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                            Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = GlobalTimeTrend->GetAnnualTimeTrend();
                            break;
                        default: throw prg_error("Unknown time trend status type '%d'.", "Write()", GlobalTimeTrend->GetStatus());
                    };
                }
                const AbstractTimeTrend * pTrend = dynamic_cast<const AbstractTimeTrend *>(TractTimeTrend.get());
                if (pTrend) {
                    Record.GetFieldValue(ALPHA_IN_FIELD).AsDouble() = pTrend->GetAlpha();
                    Record.GetFieldValue(BETA1_IN_FIELD).AsDouble() = pTrend->GetBeta();
                    //Record.GetFieldValue(FUNC_ALPHA_IN_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
                    const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
                    if (pQTrend) Record.GetFieldValue(BETA2_IN_FIELD).AsDouble() = pQTrend->GetBeta2();
                }
                pTrend = dynamic_cast<const AbstractTimeTrend *>(GlobalTimeTrend.get());
                if (pTrend) {
                    Record.GetFieldValue(ALPHA_OUT_FIELD).AsDouble() = pTrend->GetAlpha();
                    Record.GetFieldValue(BETA1_OUT_FIELD).AsDouble() = pTrend->GetBeta();
                    //Record.GetFieldValue(FUNC_ALPHA_OUT_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
                    const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
                    if (pQTrend) Record.GetFieldValue(BETA2_OUT_FIELD).AsDouble() = pQTrend->GetBeta2();
                }
                if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
                if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
                loc_idx = _report_locations.find_next(loc_idx);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("RecordRelativeRiskData(const CSVTTData&)","RelativeRiskData");
        throw;
    }
}
