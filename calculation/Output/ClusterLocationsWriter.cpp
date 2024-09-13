//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterLocationsWriter.h"
#include "cluster.h"
#include "SVTTCluster.h"
#include "SSException.h"
#include "GisUtils.h"

const char * LocationInformationWriter::AREA_SPECIFIC_FILE_EXT                          = ".gis";
const char * LocationInformationWriter::LOC_OBS_FIELD                                   = "LOC_OBS";
const char * LocationInformationWriter::LOC_EXP_FIELD                                   = "LOC_EXP";
const char * LocationInformationWriter::LOC_MEAN_FIELD                                  = "LOC_MEAN";
const char * LocationInformationWriter::LOC_WEIGHTED_MEAN_FIELD                         = "LOC_WMEAN";
const char * LocationInformationWriter::LOC_OBS_DIV_EXP_FIELD                           = "LOC_ODE";
const char * LocationInformationWriter::LOC_REL_RISK_FIELD                              = "LOC_RR";
const char * LocationInformationWriter::LOC_POPULATION_FIELD                            = "LOC_POP";
const char * LocationInformationWriter::LOC_TIME_TREND_FIELD                            = "LOC_TREND";
const char * LocationInformationWriter::LOC_COORD_LAT_FIELD	                            = "LOC_LAT";
const char * LocationInformationWriter::LOC_COORD_LONG_FIELD	                        = "LOC_LONG";
const char * LocationInformationWriter::LOC_COORD_X_FIELD	                            = "LOC_X";
const char * LocationInformationWriter::LOC_COORD_Y_FIELD                               = "LOC_Y";
const char * LocationInformationWriter::LOC_COORD_Z_FIELD                               = "LOC_Z";

const char * LocationInformationWriter::CLU_OBS_FIELD                                   = "CLU_OBS";
const char * LocationInformationWriter::CLU_EXP_FIELD                                   = "CLU_EXP";
const char * LocationInformationWriter::CLU_OBS_DIV_EXP_FIELD                           = "CLU_ODE";
const char * LocationInformationWriter::CLU_REL_RISK_FIELD                              = "CLU_RR";
const char * LocationInformationWriter::CLU_POPULATION_FIELD                            = "CLU_POP";
const char * LocationInformationWriter::CLU_MEAN_IN_FIELD                               = "CLU_MEAN_I";
const char * LocationInformationWriter::CLU_MEAN_OUT_FIELD                              = "CLU_MEAN_O";
const char * LocationInformationWriter::CLU_WEIGHTED_MEAN_IN_FIELD                      = "CL_WMEAN_I";
const char * LocationInformationWriter::CLU_WEIGHTED_MEAN_OUT_FIELD                     = "CL_WMEAN_O";
const char * LocationInformationWriter::CLU_TIME_TREND_IN_FIELD                         = "CLU_TT_IN";
const char * LocationInformationWriter::CLU_TIME_TREND_OUT_FIELD                        = "CLU_TT_OUT";
const char * LocationInformationWriter::CLU_TIME_TREND_DIFF_FIELD                       = "CLU_TT_DIF";
const char * LocationInformationWriter::GINI_CLUSTER_FIELD                              = "GINI_CLUST";
const char * LocationInformationWriter::OLIVEIRA_F_MLC_FIELD                            = "F_MLC";
const char * LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_FIELD                   = "F_HIERARCH";

/** class constructor */
LocationInformationWriter::LocationInformationWriter(const CSaTScanData& DataHub, bool bAppend):AbstractDataFileWriter(DataHub.GetParameters()), gpShapeDataFileWriter(0) {
    try {
        DefineFields(DataHub);
        if (gParameters.GetOutputAreaSpecificAscii())
            gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, AREA_SPECIFIC_FILE_EXT, bAppend);
        if (gParameters.GetOutputAreaSpecificDBase()) {
            gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, AREA_SPECIFIC_FILE_EXT, bAppend);
            if (gParameters.getOutputShapeFiles())
                gpShapeDataFileWriter = new ShapeDataFileWriter(gParameters, AREA_SPECIFIC_FILE_EXT, SHPT_POINT, bAppend);
        }
    } catch (prg_exception& x) {
        delete gpASCIIFileWriter; gpASCIIFileWriter=0;
        delete gpDBaseFileWriter; gpDBaseFileWriter=0;
        x.addTrace("constructor","LocationInformationWriter");
        throw;
    }
}

/** class destructor */
LocationInformationWriter::~LocationInformationWriter() {
  try {
      delete gpShapeDataFileWriter;
  }
  catch (...){}
}

// sets up the vector of field structs so that the FieldDef Vector can be created
void LocationInformationWriter::DefineFields(const CSaTScanData& DataHub) {
    unsigned short uwOffset = 0;
    std::string buffer;

    try {
        CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, getLocationFieldLength(DataHub), 0, uwOffset, 0);
        CreateField(vFieldDefinitions, CLUST_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);

        printString(buffer, "%u", gParameters.GetNumReplicationsRequested());
        CreateField(vFieldDefinitions, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, buffer.size());
        if (gParameters.GetIsProspectiveAnalysis())
            CreateField(vFieldDefinitions, RECURRENCE_INTERVAL_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        if (gParameters.getIsReportingGumbelAsAddon())
            CreateField(vFieldDefinitions, GUMBEL_P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17, uwOffset, 2);

        //defined cluster level fields to report -- none of these are reported for multiple data sets nor the ordinal probability model
        if (gParameters.getNumFileSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
            CreateField(vFieldDefinitions, CLU_OBS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
                CreateField(vFieldDefinitions, CLU_MEAN_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_MEAN_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                CreateField(vFieldDefinitions, CLU_MEAN_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_MEAN_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_WEIGHTED_MEAN_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_WEIGHTED_MEAN_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else {
                CreateField(vFieldDefinitions, CLU_EXP_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_OBS_DIV_EXP_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            }
            if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
                CreateField(vFieldDefinitions, CLU_REL_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) ||
                gParameters.GetProbabilityModelType() == BERNOULLI)
                CreateField(vFieldDefinitions, CLU_POPULATION_FIELD, FieldValue::NUMBER_FLD, 19, 2, uwOffset, 2);
            if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                CreateField(vFieldDefinitions, CLU_TIME_TREND_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, CLU_TIME_TREND_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            }
        }
        //defined location level fields to report -- none of these are reported for multiple data sets nor the ordinal probability model
        if (gParameters.getNumFileSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL && gParameters.GetProbabilityModelType() != UNIFORMTIME) {
            //these fields will no be supplied for analyses with more than one dataset
            CreateField(vFieldDefinitions, LOC_OBS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
                CreateField(vFieldDefinitions, LOC_MEAN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                CreateField(vFieldDefinitions, LOC_MEAN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, LOC_WEIGHTED_MEAN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else {
                CreateField(vFieldDefinitions, LOC_EXP_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                CreateField(vFieldDefinitions, LOC_OBS_DIV_EXP_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            }
            if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
                CreateField(vFieldDefinitions, LOC_REL_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) ||
                gParameters.GetProbabilityModelType() == BERNOULLI)
                CreateField(vFieldDefinitions, LOC_POPULATION_FIELD, FieldValue::NUMBER_FLD, 19, 2, uwOffset, 2);
            if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
                CreateField(vFieldDefinitions, LOC_TIME_TREND_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        }

        /* Define location specific coordinate fields. */
        if (!(gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile() ||
              (gParameters.UseLocationNeighborsFile() || (gParameters.getUseLocationsNetworkFile() && !DataHub.networkCanReportLocationCoordinates())))) {
            CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? LOC_COORD_LAT_FIELD : LOC_COORD_X_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
                gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);
            CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? LOC_COORD_LONG_FIELD : LOC_COORD_Y_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
                gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);

            //only Cartesian coordinates can have more than two dimensions
            if (gParameters.GetCoordinatesType() == CARTESIAN && DataHub.getIdentifierInfo().getLocationsManager().expectedDimensions() > 2)
                for (unsigned int i=3; i <= DataHub.getLocationsManager().expectedDimensions(); ++i) {
                    printString(buffer, "%s%i", LOC_COORD_Z_FIELD, i - 2);
                    CreateField(vFieldDefinitions, buffer.c_str(), FieldValue::NUMBER_FLD, 19, 10, uwOffset, 19/* forces %g format */);
                }
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
        CreateField(vFieldDefinitions, GINI_CLUSTER_FIELD, FieldValue::BOOLEAN_FLD, 1, 0, uwOffset, 0);
    } catch (prg_exception& x) {
        x.addTrace("DefineFields()","LocationInformationWriter");
        throw;
    }
}

/** Writes record for each location in the cluster. */
void LocationInformationWriter::WriteClusterLocations(const CCluster& theCluster, const CSaTScanData& DataHub, int iClusterNumber, const SimulationVariables& simVars, const LocationRelevance& location_relevance) {
    double dRelativeRisk;
    RecordBuffer clusterRecord(vFieldDefinitions);
    std::vector<double> locationCoordinates;
    unsigned int loc_first_coord_idx, loc_second_coord_idx;
    std::string buffer;

    try {
        if (theCluster.GetClusterType() == PURELYTEMPORALCLUSTER) return; // skip purely temporal clusters
        // Create temporary data structures which will facilitate the write.
        WritePrep(theCluster, DataHub);
        // Assign field values for those at the cluster level.
        clusterRecord.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
        clusterRecord.GetFieldValue(GINI_CLUSTER_FIELD).AsBool() = theCluster.isGiniCluster();
        if (theCluster.reportablePValue(gParameters, simVars)) {
            clusterRecord.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.getReportingPValue(gParameters, simVars, gParameters.GetIsIterativeScanning() || iClusterNumber == 1);
            if (gParameters.getIsReportingGumbelAsAddon()) {
                std::pair<double, double> p = theCluster.GetGumbelPValue(simVars);
                clusterRecord.GetFieldValue(GUMBEL_P_VALUE_FLD).AsDouble() = std::max(p.first, p.second);
            }
        }
        if (theCluster.reportableRecurrenceInterval(gParameters, simVars))
            clusterRecord.GetFieldValue(RECURRENCE_INTERVAL_FLD).AsDouble() = theCluster.GetRecurrenceInterval(DataHub, iClusterNumber, simVars).second;
        // Certain cluster fields are only present for one dataset and not ordinal or multinomial model.
        if (gParameters.getNumFileSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
            clusterRecord.GetFieldValue(CLU_OBS_FIELD).AsDouble() = theCluster.GetObservedCount();
            if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
                count_t tObserved = theCluster.GetObservedCount();
                measure_t tExpected = theCluster.GetExpectedCount(DataHub);
                if (tObserved) clusterRecord.GetFieldValue(CLU_MEAN_IN_FIELD).AsDouble() = tExpected / tObserved;
                count_t tCasesOutside = DataHub.GetDataSetHandler().GetDataSet().getTotalCases() - tObserved;
                if (tCasesOutside) clusterRecord.GetFieldValue(CLU_MEAN_OUT_FIELD).AsDouble() = (DataHub.GetDataSetHandler().GetDataSet().getTotalMeasure() - tExpected) / tCasesOutside;
            } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                clusterRecord.GetFieldValue(CLU_MEAN_IN_FIELD).AsDouble() = _weighted_nornal_statistics.gtMeanIn;
                clusterRecord.GetFieldValue(CLU_MEAN_OUT_FIELD).AsDouble() = _weighted_nornal_statistics.gtMeanOut;
                clusterRecord.GetFieldValue(CLU_WEIGHTED_MEAN_IN_FIELD).AsDouble() = _weighted_nornal_statistics.gtWeightedMeanIn;
                clusterRecord.GetFieldValue(CLU_WEIGHTED_MEAN_OUT_FIELD).AsDouble() = _weighted_nornal_statistics.gtWeightedMeanOut;
            } else {
                clusterRecord.GetFieldValue(CLU_EXP_FIELD).AsDouble() = theCluster.GetExpectedCount(DataHub);
                clusterRecord.GetFieldValue(CLU_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpected(DataHub);
            }
            if ((gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI) && (dRelativeRisk = theCluster.GetRelativeRisk(DataHub)) != -1)
                clusterRecord.GetFieldValue(CLU_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
            if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis() && theCluster.GetClusterType() != PURELYTEMPORALCLUSTER) ||
                gParameters.GetProbabilityModelType() == BERNOULLI)
                clusterRecord.GetFieldValue(CLU_POPULATION_FIELD).AsDouble() = DataHub.GetProbabilityModel().GetPopulation(0, theCluster, DataHub);
            if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                const AbtractSVTTClusterData * pClusterData = 0;
                if ((pClusterData = dynamic_cast<const AbtractSVTTClusterData*>(theCluster.GetClusterData())) == 0)
                    throw prg_error("Dynamic cast to AbtractSVTTClusterData failed.\n", "Write()");
                if (gParameters.getTimeTrendType() == LINEAR) {
                    switch (pClusterData->getInsideTrend().GetStatus()) {
                        case AbstractTimeTrend::UNDEFINED: break;
                        case AbstractTimeTrend::CONVERGED:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_IN_FIELD).AsDouble() = pClusterData->getInsideTrend().GetAnnualTimeTrend(); break;
                        case AbstractTimeTrend::NEGATIVE_INFINITY:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::POSITIVE_INFINITY:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::SINGULAR_MATRIX:
                            throw prg_error("The time trend inside of cluster was not calculated because matrix A is singular.\n", "Write()");
                        case AbstractTimeTrend::NOT_CONVERGED:
                            throw prg_error("The time trend inside of cluster did not converge.\n", "Write()");
                        default: throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getInsideTrend().GetStatus());
                    }
                    switch (pClusterData->getOutsideTrend().GetStatus()) {
                        case AbstractTimeTrend::UNDEFINED: break;
                        case AbstractTimeTrend::CONVERGED:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = pClusterData->getOutsideTrend().GetAnnualTimeTrend(); break;
                        case AbstractTimeTrend::NEGATIVE_INFINITY:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::POSITIVE_INFINITY:
                            clusterRecord.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::SINGULAR_MATRIX:
                            throw prg_error("The time trend outside of cluster was not calculated because matrix A is singular.\n", "Write()");
                        case AbstractTimeTrend::NOT_CONVERGED:
                            throw prg_error("The time trend outside of cluster did not converge.\n", "Write()");
                        default: throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getOutsideTrend().GetStatus());
                    }
                }
            }
        }
        // Now populate the location level fields.
        for (const auto& locationToIdentifiers : _location_to_identifiers) { // iterate over the location to identifiers maps
            RecordBuffer Record(clusterRecord); // Create new record for this location which populates from cluster level record.
            double expected = 0;
            Record.GetFieldValue(LOC_ID_FIELD).AsString() = locationToIdentifiers.first->name();
            if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
                Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
            if (!(gParameters.UseLocationNeighborsFile() || (gParameters.getUseLocationsNetworkFile() && !DataHub.networkCanReportLocationCoordinates()))) {
                loc_first_coord_idx = clusterRecord.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? LOC_COORD_LAT_FIELD : LOC_COORD_X_FIELD);
                loc_second_coord_idx = clusterRecord.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? LOC_COORD_LONG_FIELD : LOC_COORD_Y_FIELD);
                locationToIdentifiers.first->coordinates()->retrieve(locationCoordinates);
                switch (gParameters.GetCoordinatesType()) {
                    case CARTESIAN:
                        Record.GetFieldValue(loc_first_coord_idx).AsDouble() = locationCoordinates[0];
                        Record.GetFieldValue(loc_second_coord_idx).AsDouble() = locationCoordinates[1];
                        for (size_t i = 2; i < locationCoordinates.size(); ++i) {
                            printString(buffer, "%s%d", LOC_COORD_Z_FIELD, (i - 1));
                            Record.GetFieldValue(buffer).AsDouble() = locationCoordinates[i];
                        } break;
                    case LATLON: {
                        std::pair<double, double> prLatitudeLongitude = ConvertToLatLong(locationCoordinates);
                        Record.GetFieldValue(loc_first_coord_idx).AsDouble() = prLatitudeLongitude.first;
                        Record.GetFieldValue(loc_second_coord_idx).AsDouble() = prLatitudeLongitude.second;
                        if (gpShapeDataFileWriter) gpShapeDataFileWriter->writeCoordinates(prLatitudeLongitude.second, prLatitudeLongitude.first);
                    } break;
                    default: throw prg_error("Unknown coordinate type '%d'.", "Write()", gParameters.GetCoordinatesType());
                }
            }
            // Certain location information fields are only present for one dataset and certain probability models.
            if (gParameters.getNumFileSets() == 1 && 
                gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL && gParameters.GetProbabilityModelType() != UNIFORMTIME &&
                locationToIdentifiers.second.size()/* zero identifiers is an indication that tracts where combined at this location */) {
                // These columns require the accumulation of identifier(s) data at the location - only really true with multiple locations feature, otherwise this is one-to-one.
                for (unsigned int idx = 0; idx < locationToIdentifiers.second.size(); ++idx) { // Some fields can just be accumulated in the record directly.
                    tract_t identifierIdx = locationToIdentifiers.second[idx];
                    Record.GetFieldValue(LOC_OBS_FIELD).AsDouble() += theCluster.GetObservedCountForTract(identifierIdx, DataHub);
                    if (gParameters.GetProbabilityModelType() != NORMAL)
                        Record.GetFieldValue(LOC_EXP_FIELD).AsDouble() += theCluster.GetExpectedCountForTract(identifierIdx, DataHub);
                    if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal())
                        expected += theCluster.GetExpectedCountForTract(identifierIdx, DataHub);
                    if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) || gParameters.GetProbabilityModelType() == BERNOULLI)
                        Record.GetFieldValue(LOC_POPULATION_FIELD).AsDouble() += DataHub.GetProbabilityModel().GetLocationPopulation(0, identifierIdx, theCluster, DataHub);
                    if (gParameters.getCalculateOliveirasF()) {
                        if (location_relevance._most_likely_only.size() > identifierIdx)
                            Record.GetFieldValue(OLIVEIRA_F_MLC_FIELD).AsDouble() += static_cast<double>(location_relevance._most_likely_only[identifierIdx]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                        if (location_relevance._hierarchical.size() > identifierIdx)
                            Record.GetFieldValue(OLIVEIRA_F_HIERARCHICAL_FIELD).AsDouble() += static_cast<double>(location_relevance._hierarchical[identifierIdx]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                    }
                }
                // Others need a specialized process or the accumulation is just clearer in separate routine.
                if (gParameters.GetProbabilityModelType() != NORMAL) {
                    double expected = Record.GetFieldValue(LOC_EXP_FIELD).AsDouble();
                    if (expected) Record.GetFieldValue(LOC_OBS_DIV_EXP_FIELD).AsDouble() = Record.GetFieldValue(LOC_OBS_FIELD).AsDouble() / expected;
                }
                if ((gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
                    dRelativeRisk = getRelativeRiskForIdentifiers(DataHub, theCluster, locationToIdentifiers.second);
                    if (dRelativeRisk != -1) Record.GetFieldValue(LOC_REL_RISK_FIELD).AsDouble() += dRelativeRisk;
                }
                if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal() && Record.GetFieldValue(LOC_OBS_FIELD).AsDouble())
                    Record.GetFieldValue(LOC_MEAN_FIELD).AsDouble() += expected / Record.GetFieldValue(LOC_OBS_FIELD).AsDouble();
                if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                    auto mean = getWeightedNormalMeanForIdentifiers(locationToIdentifiers.second);
                    Record.GetFieldValue(LOC_MEAN_FIELD).AsDouble() = mean.first;
                    Record.GetFieldValue(LOC_WEIGHTED_MEAN_FIELD).AsDouble() = mean.second;
                }
                if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND && gParameters.getTimeTrendType() == LINEAR) {
                    boost::shared_ptr<AbstractTimeTrend> timetrend(getTimeTrendForIdentifiers(DataHub, locationToIdentifiers.second));
                    switch (timetrend->GetStatus()) {
                        case AbstractTimeTrend::UNDEFINED: break;
                        case AbstractTimeTrend::NEGATIVE_INFINITY:
                            Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::POSITIVE_INFINITY:
                            Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                        case AbstractTimeTrend::NOT_CONVERGED:
                            throw prg_error("The time trend did not converge for location '%s'.\n", "Write()", locationToIdentifiers.first->name().c_str());
                        case AbstractTimeTrend::SINGULAR_MATRIX:
                            throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n", "Write()", locationToIdentifiers.first->name().c_str());
                        case AbstractTimeTrend::CONVERGED:
                            timetrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                            Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = timetrend->GetAnnualTimeTrend();
                            break;
                        default: throw prg_error("Unknown time trend status type '%d'.", "Write()", timetrend->GetStatus());
                    }
                }
            }
            if (gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION && !locationToIdentifiers.second.size()) {
                // When there are multiple coordinates per location, it's possible that there are no records defined 
                // in the case file for a location, which results in the 'LOC*' fields being reported as blank. 
                // Instead report these fields as zero.
                FieldValue default_numeric(FieldValue::NUMBER_FLD);
                default_numeric.AsDouble() = 0.0;
                std::vector<std::string> default_fields = { LOC_OBS_FIELD, LOC_EXP_FIELD, LOC_POPULATION_FIELD };
                Record.DefaultBlankFieldsOfType(default_numeric, default_fields); // default numeric values such that we report them as zero vs blank
            }
            if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
            if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
        }
    } catch (prg_exception& x) {
        x.addTrace("Write()", "LocationInformationWriter");
        throw;
    }
}

/* Returns the mean for all identifiers (at a location) for the weighted normal model - taken as one. */
std::pair<double, double> LocationInformationWriter::getWeightedNormalMeanForIdentifiers(const MinimalGrowthArray<size_t>& identifiers) const {
    // Accumulate the mean across all identifiers.
    double summation = 0, w_summation = 0, count = 0, w_count = 0;
    // Accumulate the total observed across all identifiers.
    for (unsigned int idx = 0; idx < identifiers.size(); ++idx) {
        auto identifierIdx = static_cast<tract_t>(identifiers[idx]);
        if (_weighted_nornal_statistics.gtLocMean.find(identifierIdx) != _weighted_nornal_statistics.gtLocMean.end()) {
            summation += _weighted_nornal_statistics.gtLocMean.at(identifierIdx);
            count += _weighted_nornal_statistics.gtLocTotalObserved.at(identifierIdx);
            w_summation += _weighted_nornal_statistics.gtLocWeightedMean.at(identifierIdx);
            w_count += _weighted_nornal_statistics.gtLocTotalWeight.at(identifierIdx);
        }
    }
    // Return mean and weighted mean.
    return std::make_pair(count ? summation / count : 0.0, w_count ? w_summation / w_count : 0.0);
}

/* Returns the time trend for all identifiers - taken as one. */
boost::shared_ptr<AbstractTimeTrend> LocationInformationWriter::getTimeTrendForIdentifiers(const CSaTScanData& DataHub, const MinimalGrowthArray<size_t>& identifiers) const {
    std::vector<count_t> cases(DataHub.GetNumTimeIntervals());
    std::vector<measure_t> measure(DataHub.GetNumTimeIntervals());
    count_t ** ppCasesNC = DataHub.GetDataSetHandler().GetDataSet().getCaseData_NC().GetArray();
    measure_t ** ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet().getMeasureData_NC().GetArray();
    boost::shared_ptr<AbstractTimeTrend> timeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
    // Accumulate the total cases and total measure across all identifiers.
    for (unsigned int i = 0; i < identifiers.size(); ++i) {
        size_t idx = identifiers[i];
        for (unsigned int j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
            cases[j] = ppCasesNC[j][idx];
            measure[j] = ppMeasureNC[j][idx];
        }
    }
    // Calculate collective time trend.
    timeTrend->CalculateAndSet(&cases[0], &measure[0], DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
    return timeTrend;
}

/* Returns the relative risk for all identifiers - taken as one. */
double LocationInformationWriter::getRelativeRiskForIdentifiers(const CSaTScanData& DataHub, const CCluster& cluster, const MinimalGrowthArray<size_t>& identifiers) const {
    double observedAll = 0, expectedAll = 0;
    // Accumulate the total observed and total expected across all identifiers.
    for (unsigned int i = 0; i < identifiers.size(); ++i) {
        size_t idx = identifiers[i];
        observedAll += static_cast<double>(cluster.GetObservedCountForTract(idx, DataHub));
        expectedAll += cluster.GetExpectedCountForTract(idx, DataHub, 0, gParameters.GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION);
    }
    // Calculate collective relative risk per parameter settings.
    if (gParameters.GetProbabilityModelType() == UNIFORMTIME)
        return (expectedAll == 0.0 ? -1 : observedAll / expectedAll); // when expected == 0, relative risk goes to infinity
    if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        return cluster.GetRelativeRisk(
            observedAll, expectedAll, /*We want the unadjusted measure here.*/
            DataHub.GetDataSetHandler().GetDataSet().getCaseData_PT()[cluster.m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet().getCaseData_PT()[cluster.m_nLastInterval],
            DataHub.GetDataSetHandler().GetDataSet().getMeasureData_PT()[cluster.m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet().getMeasureData_PT()[cluster.m_nLastInterval]
        );
    }
    return cluster.GetRelativeRisk(
        observedAll, expectedAll, DataHub.GetDataSetHandler().GetDataSet().getTotalCases(), DataHub.GetDataSetHandler().GetDataSet().getTotalCases()
    );
}


/** Preparation step before writing cluster information. */
void LocationInformationWriter::WritePrep(const CCluster& theCluster, const CSaTScanData& DataHub) {
    try {
        // First obtain the identifier indexes within this cluster - identifiers is the data collection level, not locations.
        std::vector<tract_t> identifierindexes;
        theCluster.getIdentifierIndexes(DataHub, identifierindexes, true);
        // Obtain data stats with weighted normal model as needed.
        if (gParameters.getNumFileSets() == 1 && DataHub.GetParameters().GetProbabilityModelType() == NORMAL && DataHub.GetParameters().getIsWeightedNormal()) {
            //Cache weighted normal model statistics instead of calculating each time in Write() method.
            const AbstractWeightedNormalRandomizer * pRandomizer=0;
            if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(DataHub.GetDataSetHandler().GetRandomizer(0))) == 0)
                throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "Write()");
            _weighted_nornal_statistics = pRandomizer->getClusterLocationStatistics(theCluster.m_nFirstInterval, theCluster.m_nLastInterval, identifierindexes);
        }
        // If this analysis uses a locations network file, create network locations container for this cluster to facilitate faster coordinate calculations. 
        if (gParameters.getUseLocationsNetworkFile()) {
            _clusterNetwork.reset(new NetworkLocationContainer_t());
            DataHub.getClusterNetworkLocations(theCluster, *_clusterNetwork);
        }
        // Get the collection of locations within this cluster.
        std::vector<tract_t> clusterLocations;
        boost::dynamic_bitset<> clusterLocationsSet;
        CentroidNeighborCalculator::getLocationsAboutCluster(DataHub, theCluster, &clusterLocationsSet, &clusterLocations);
        // Now associate those locations with identifiers located at them - this is kind of the opposite of storage.
        _location_to_identifiers.clear();
        std::map<const Location*, size_t> locationMap;
        // Build a collection of locations in cluster to identifiers in the cluster.
        for (auto idx: clusterLocations) {
            const auto& location = DataHub.getLocationsManager().locations()[idx];
            locationMap.insert(std::make_pair(location.get(), _location_to_identifiers.size()));
            _location_to_identifiers.push_back(std::make_pair(location.get(), MinimalGrowthArray<size_t>()));
        }
        // Iterate over identifiers in this cluster and associate with location(s) - maintaining the location order within cluster.
        for (auto identifieridx : identifierindexes) {
            const auto& idLocations = DataHub.getIdentifierInfo().getIdentifiers()[identifieridx]->getLocations();
            for (size_t t = 0; t < idLocations.size(); ++t) {
                // All locations of an identifier will not always be in the cluster, per multiple coordinates, so test inclusion.
                const auto& location = idLocations[t];
                if (clusterLocationsSet.test(location->index())) {
                    auto itrM = locationMap.find(location);
                    if (itrM != locationMap.end()/*safeguard*/) _location_to_identifiers[itrM->second].second.add(identifieridx, true);
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WritePrep()","LocationInformationWriter");
        throw;
    }
}
