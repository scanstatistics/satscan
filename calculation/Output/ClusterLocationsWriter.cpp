//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterLocationsWriter.h"
#include "cluster.h"
#include "SVTTCluster.h"
#include "SSException.h"

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
//const char * LocationInformationWriter::CLU_ALPHA_IN_FIELD          = "IN_ITRCPT";
//const char * LocationInformationWriter::CLU_BETA1_IN_FIELD          = "IN_LINEAR";
//const char * LocationInformationWriter::CLU_BETA2_IN_FIELD          = "IN_QUAD";
//const char * LocationInformationWriter::CLU_ALPHA_OUT_FIELD         = "OUT_ITRCPT";
//const char * LocationInformationWriter::CLU_BETA1_OUT_FIELD         = "OUT_LINEAR";
//const char * LocationInformationWriter::CLU_BETA2_OUT_FIELD         = "OUT_QUAD";
//const char * LocationInformationWriter::CLU_ALPHA_GLOBAL_FIELD      = "GBL_ITRCPT";
//const char * LocationInformationWriter::CLU_BETA1_GLOBAL_FIELD      = "GBL_LINEAR";
//const char * LocationInformationWriter::CLU_BETA2_GLOBAL_FIELD      = "GBL_QUAD";
//const char * LocationInformationWriter::CLU_FUNC_ALPHA_IN_FIELD     = "IN_FUNC_A";
//const char * LocationInformationWriter::CLU_FUNC_ALPHA_OUT_FIELD    = "OUT_FUNC_A";
const char * LocationInformationWriter::GINI_CLUSTER_FIELD                              = "GINI_CLUST";
const char * LocationInformationWriter::OLIVEIRA_F_MLC_FIELD                            = "F_MLC";
const char * LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_FIELD                   = "F_HIERARCH";
/* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
const char * LocationInformationWriter::OLIVEIRA_F_GINI_OPTIMAL_FIELD                   = "F_GINI_OPT";
const char * LocationInformationWriter::OLIVEIRA_F_GINI_MAXIMA_FIELD                    = "F_GINI_MAX";
const char * LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD      = "F_H_G_OPT";
const char * LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD       = "F_H_G_MAX";
*/

/** class constructor */
LocationInformationWriter::LocationInformationWriter(const CSaTScanData& DataHub, bool bAppend)
                          :AbstractDataFileWriter(DataHub.GetParameters()), gpShapeDataFileWriter(0) {
  try {
    DefineFields(DataHub);
    if (gParameters.GetOutputAreaSpecificAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, AREA_SPECIFIC_FILE_EXT, bAppend);
    if (gParameters.GetOutputAreaSpecificDBase()) {
        gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, AREA_SPECIFIC_FILE_EXT, bAppend);
        if (gParameters.getOutputShapeFiles()) {
            gpShapeDataFileWriter = new ShapeDataFileWriter(gParameters, AREA_SPECIFIC_FILE_EXT, SHPT_POINT, bAppend);
        }
    }
  }
  catch (prg_exception& x) {
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
        CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, GetLocationIdentiferFieldLength(DataHub), 0, uwOffset, 0);
        CreateField(vFieldDefinitions, CLUST_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);

        printString(buffer, "%u", gParameters.GetNumReplicationsRequested());
        CreateField(vFieldDefinitions, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, buffer.size());
        if (gParameters.GetIsProspectiveAnalysis())
            CreateField(vFieldDefinitions, RECURRENCE_INTERVAL_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        if ((gParameters.GetPValueReportingType() == STANDARD_PVALUE || gParameters.GetPValueReportingType() == TERMINATION_PVALUE) && gParameters.GetReportGumbelPValue())  
            CreateField(vFieldDefinitions, GUMBEL_P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17, uwOffset, 2);

        //defined cluster level fields to report -- none of these are reported
        // for multiple data sets nor the ordinal probability model
        if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
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
                //CreateField(vFieldDefinitions, CLU_TIME_TREND_DIFF_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
                //CreateField(vFieldDefinitions, CLU_ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA1_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA2_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA1_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA2_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_ALPHA_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA1_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_BETA2_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_FUNC_ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
                //CreateField(vFieldDefinitions, CLU_FUNC_ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            }
        }
        //defined location level fields to report -- none of these are reported
        // for multiple data sets nor the ordinal probability model
        if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL&& gParameters.GetProbabilityModelType() != UNIFORMTIME) {
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
		if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile()) {
			CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? LOC_COORD_LAT_FIELD : LOC_COORD_X_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
				gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);
			CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? LOC_COORD_LONG_FIELD : LOC_COORD_Y_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
				gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);

			//only Cartesian coordinates can have more than two dimensions
			if (gParameters.GetCoordinatesType() == CARTESIAN && DataHub.GetTInfo()->getCoordinateDimensions() > 2)
				for (unsigned int i=3; i <= (unsigned int)DataHub.GetTInfo()->getCoordinateDimensions(); ++i) {
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
            /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
            if (gParameters.getReportGiniOptimizedClusters()) {
                CreateField(vFieldDefinitions, OLIVEIRA_F_GINI_OPTIMAL_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
                CreateField(vFieldDefinitions, OLIVEIRA_F_GINI_MAXIMA_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
            }
            if (gParameters.getReportHierarchicalClusters() && gParameters.getReportGiniOptimizedClusters()) {
                CreateField(vFieldDefinitions, OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
                CreateField(vFieldDefinitions, OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD, FieldValue::NUMBER_FLD, 19, precision, uwOffset, precision);
            }
            */
        }
        CreateField(vFieldDefinitions, GINI_CLUSTER_FIELD, FieldValue::BOOLEAN_FLD, 1, 0, uwOffset, 0);
    } catch (prg_exception& x) {
        x.addTrace("DefineFields()","LocationInformationWriter");
        throw;
    }
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void LocationInformationWriter::Write(const CCluster& theCluster,
                                      const CSaTScanData& DataHub,
                                      int iClusterNumber,
                                      tract_t tTract,
                                      const SimulationVariables& simVars,
                                      const LocationRelevance& location_relevance) {
    TractHandler::Location::StringContainer_t vIdentifiers;
    double dRelativeRisk;
    RecordBuffer Record(vFieldDefinitions);
    const DataSetHandler & Handler = DataHub.GetDataSetHandler();
	std::vector<double> locationCoordinates;
	std::pair<double, double> prLatitudeLongitude;
	unsigned int loc_first_coord_idx, loc_second_coord_idx;
    std::string buffer;

    try {
        if (theCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            throw prg_error("LocationInformationWriter::Write() is not implemented for purely temporal clusters.", "Write()");

        //do not report locations for which iterative scan has nullified its data
        if (DataHub.GetIsNullifiedLocation(tTract)) return;

		/* Get coordinates for the cluster. */
		if (!gParameters.UseLocationNeighborsFile()) {
			loc_first_coord_idx = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? LOC_COORD_LAT_FIELD : LOC_COORD_X_FIELD);
			loc_second_coord_idx = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? LOC_COORD_LONG_FIELD : LOC_COORD_Y_FIELD);
		}

        DataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vIdentifiers);
        if (gpShapeDataFileWriter) {
            std::vector<double> vCoordinates;
            CentroidNeighborCalculator::getTractCoordinates(DataHub, theCluster, tTract, vCoordinates);
            std::pair<double, double> prLatitudeLongitude;
            prLatitudeLongitude = ConvertToLatLong(vCoordinates);
            gpShapeDataFileWriter->writeCoordinates(prLatitudeLongitude.second, prLatitudeLongitude.first);
        }

        for (unsigned int i=0; i < vIdentifiers.size(); ++i) {
            Record.SetAllFieldsBlank(true);
            Record.GetFieldValue(LOC_ID_FIELD).AsString() = vIdentifiers[i].c_str();
            if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
                Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
            Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
            Record.GetFieldValue(GINI_CLUSTER_FIELD).AsBool() = theCluster.isGiniCluster();
            if (theCluster.reportablePValue(gParameters,simVars))
                Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.getReportingPValue(gParameters, simVars, gParameters.GetIsIterativeScanning() || iClusterNumber == 1);
            if ((gParameters.GetPValueReportingType() == STANDARD_PVALUE || gParameters.GetPValueReportingType() == TERMINATION_PVALUE) && gParameters.GetReportGumbelPValue()) {
                std::pair<double,double> p = theCluster.GetGumbelPValue(simVars);
                Record.GetFieldValue(GUMBEL_P_VALUE_FLD).AsDouble() = std::max(p.first,p.second);
            }
            if (theCluster.reportableRecurrenceInterval(gParameters, simVars))
                Record.GetFieldValue(RECURRENCE_INTERVAL_FLD).AsDouble() = theCluster.GetRecurrenceInterval(DataHub, iClusterNumber, simVars).second;
            if (gParameters.getCalculateOliveirasF()) {
                if (location_relevance._most_likely_only.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_MLC_FIELD).AsDouble() = static_cast<double>(location_relevance._most_likely_only[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                if (location_relevance._hierarchical.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_HIERARCHICAL_FIELD).AsDouble() = static_cast<double>(location_relevance._hierarchical[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
                if (location_relevance._gini_optimal.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_GINI_OPTIMAL_FIELD).AsDouble() = static_cast<double>(location_relevance._gini_optimal[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                if (location_relevance._gini_maxima.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_GINI_MAXIMA_FIELD).AsDouble() = static_cast<double>(location_relevance._gini_maxima[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                if (location_relevance._hierarchical_gini_optimal.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD).AsDouble() = static_cast<double>(location_relevance._hierarchical_gini_optimal[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                if (location_relevance._hierarchical_gini_maxima.size() > tTract)
                    Record.GetFieldValue(OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD).AsDouble() = static_cast<double>(location_relevance._hierarchical_gini_maxima[tTract]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
                */
            }

			/* Report coordinate fields for location. */
            if (!gParameters.UseLocationNeighborsFile()) {
				/* Retrieve coordinates of location -- notice this assumes that the location has only one set of coordinates. */
				DataHub.GetTInfo()->getLocations()[tTract]->getCoordinates()[0]->retrieve(locationCoordinates);
				/* Write records to record buffer. */
				switch (gParameters.GetCoordinatesType()) {
					case CARTESIAN : Record.GetFieldValue(loc_first_coord_idx).AsDouble() =  locationCoordinates[0];
									 Record.GetFieldValue(loc_second_coord_idx).AsDouble() =  locationCoordinates[1];
									 for (size_t i=2; i < locationCoordinates.size(); ++i) {
										printString(buffer, "%s%d", LOC_COORD_Z_FIELD, (i - 1));
										Record.GetFieldValue(buffer).AsDouble() = locationCoordinates[i];
									} break;
					case LATLON    : prLatitudeLongitude = ConvertToLatLong(locationCoordinates);
							         Record.GetFieldValue(loc_first_coord_idx).AsDouble() = prLatitudeLongitude.first;
									 Record.GetFieldValue(loc_second_coord_idx).AsDouble() = prLatitudeLongitude.second;
									 break;
					default : throw prg_error("Unknown coordinate type '%d'.","WriteCoordinates()", gParameters.GetCoordinatesType());
				}
			}

            //location information fields are only present for one dataset and not ordinal model
            if (Handler.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL && gParameters.GetProbabilityModelType() != UNIFORMTIME) {
                /* When there is more than one identifiers for a tract, this indicates that locations where combined. Print a record for each location but
                leave area specific information blank. */
                if (vIdentifiers.size() == 1) {
                    Record.GetFieldValue(LOC_OBS_FIELD).AsDouble() = theCluster.GetObservedCountForTract(tTract, DataHub);
                    if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
                        count_t tObserved = theCluster.GetObservedCountForTract(tTract, DataHub);
                        if (tObserved) Record.GetFieldValue(LOC_MEAN_FIELD).AsDouble() = theCluster.GetExpectedCountForTract(tTract, DataHub)/tObserved;
                    } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                        Record.GetFieldValue(LOC_MEAN_FIELD).AsDouble() = gStatistics.gtLocMean[tTract];
                        Record.GetFieldValue(LOC_WEIGHTED_MEAN_FIELD).AsDouble() = gStatistics.gtLocWeightedMean[tTract];
                    } else {
                        Record.GetFieldValue(LOC_EXP_FIELD).AsDouble() = theCluster.GetExpectedCountForTract(tTract, DataHub);
                        Record.GetFieldValue(LOC_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpectedForTract(tTract, DataHub);
                    }
                    if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
                        (dRelativeRisk = theCluster.GetRelativeRiskForTract(tTract, DataHub)) != -1)
                        Record.GetFieldValue(LOC_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
                    if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) ||
                        gParameters.GetProbabilityModelType() == BERNOULLI)
                        Record.GetFieldValue(LOC_POPULATION_FIELD).AsDouble() = DataHub.GetProbabilityModel().GetLocationPopulation(0, tTract, theCluster, DataHub);
                    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND && gParameters.getTimeTrendType() == LINEAR) {
                        std::auto_ptr<AbstractTimeTrend> TractTimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
                        std::vector<count_t> vTemporalTractCases(DataHub.GetNumTimeIntervals());
                        std::vector<measure_t> vTemporalTractObserved(DataHub.GetNumTimeIntervals());
                        count_t ** ppCasesNC = DataHub.GetDataSetHandler().GetDataSet().getCaseData_NC().GetArray();
                        measure_t ** ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet().getMeasureData_NC().GetArray();
                        //calculate total cases by time intervals for this tract
                        for (unsigned int j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
                            vTemporalTractCases[j] = ppCasesNC[j][tTract];
                            vTemporalTractObserved[j] = ppMeasureNC[j][tTract];
                        }
                        TractTimeTrend->CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0], DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
                        switch (TractTimeTrend->GetStatus()) {
                            case AbstractTimeTrend::UNDEFINED : break;
                            case AbstractTimeTrend::NEGATIVE_INFINITY :
                                Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::POSITIVE_INFINITY :
                                Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::NOT_CONVERGED     :
                                throw prg_error("The time trend did not converge for location '%s'.\n","Write()",vIdentifiers[i].c_str());
                            case AbstractTimeTrend::SINGULAR_MATRIX :
                                throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n","Write()",vIdentifiers[i].c_str());
                            case AbstractTimeTrend::CONVERGED         :
                                TractTimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                                Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = TractTimeTrend->GetAnnualTimeTrend();
                                break;
                            default : throw prg_error("Unknown time trend status type '%d'.", "Write()", TractTimeTrend->GetStatus());
                        }
                    }
                }
            }
            //cluster information fields are only present for one dataset and not ordinal model
            if (Handler.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
                Record.GetFieldValue(CLU_OBS_FIELD).AsDouble() = theCluster.GetObservedCount();
                if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
                    count_t tObserved = theCluster.GetObservedCount();
                    measure_t tExpected = theCluster.GetExpectedCount(DataHub);
                    if (tObserved) Record.GetFieldValue(CLU_MEAN_IN_FIELD).AsDouble() = tExpected/tObserved;
                    count_t tCasesOutside = DataHub.GetDataSetHandler().GetDataSet().getTotalCases() - tObserved;
                    if (tCasesOutside) Record.GetFieldValue(CLU_MEAN_OUT_FIELD).AsDouble() = (Handler.GetDataSet().getTotalMeasure() - tExpected)/tCasesOutside;
                } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
                    Record.GetFieldValue(CLU_MEAN_IN_FIELD).AsDouble() = gStatistics.gtMeanIn;
                    Record.GetFieldValue(CLU_MEAN_OUT_FIELD).AsDouble() = gStatistics.gtMeanOut;
                    Record.GetFieldValue(CLU_WEIGHTED_MEAN_IN_FIELD).AsDouble() = gStatistics.gtWeightedMeanIn;
                    Record.GetFieldValue(CLU_WEIGHTED_MEAN_OUT_FIELD).AsDouble() = gStatistics.gtWeightedMeanOut;
                } else {
                    Record.GetFieldValue(CLU_EXP_FIELD).AsDouble() = theCluster.GetExpectedCount(DataHub);
                    Record.GetFieldValue(CLU_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpected(DataHub);
                }
                if ((gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI) &&
                    (dRelativeRisk = theCluster.GetRelativeRisk(DataHub)) != -1)
                    Record.GetFieldValue(CLU_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
                if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis() && theCluster.GetClusterType() != PURELYTEMPORALCLUSTER) ||
                    gParameters.GetProbabilityModelType() == BERNOULLI)
                    Record.GetFieldValue(CLU_POPULATION_FIELD).AsDouble() = DataHub.GetProbabilityModel().GetPopulation(0, theCluster, DataHub);
                if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                    const AbtractSVTTClusterData * pClusterData=0;
                    if ((pClusterData = dynamic_cast<const AbtractSVTTClusterData*>(theCluster.GetClusterData())) == 0)
                        throw prg_error("Dynamic cast to AbtractSVTTClusterData failed.\n", "Write()");
                    if (gParameters.getTimeTrendType() == LINEAR) {
                        switch (pClusterData->getInsideTrend().GetStatus()) {
                            case AbstractTimeTrend::UNDEFINED         : break;
                            case AbstractTimeTrend::CONVERGED         :
                                Record.GetFieldValue(CLU_TIME_TREND_IN_FIELD).AsDouble() = pClusterData->getInsideTrend().GetAnnualTimeTrend(); break;
                            case AbstractTimeTrend::NEGATIVE_INFINITY :
                                Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::POSITIVE_INFINITY :
                                Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::SINGULAR_MATRIX :
                                throw prg_error("The time trend inside of cluster was not calculated because matrix A is singular.\n","Write()");
                            case AbstractTimeTrend::NOT_CONVERGED     :
                                throw prg_error("The time trend inside of cluster did not converge.\n","Write()");
                            default : throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getInsideTrend().GetStatus());
                        }
                        switch (pClusterData->getOutsideTrend().GetStatus()) {
                            case AbstractTimeTrend::UNDEFINED         : break;
                            case AbstractTimeTrend::CONVERGED         :
                                Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = pClusterData->getOutsideTrend().GetAnnualTimeTrend(); break;
                            case AbstractTimeTrend::NEGATIVE_INFINITY :
                                Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::POSITIVE_INFINITY :
                                Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                            case AbstractTimeTrend::SINGULAR_MATRIX :
                                throw prg_error("The time trend outside of cluster was not calculated because matrix A is singular.\n","Write()");
                            case AbstractTimeTrend::NOT_CONVERGED     :
                                throw prg_error("The time trend outside of cluster did not converge.\n","Write()");
                            default : throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getOutsideTrend().GetStatus());
                        }
                    }
                    //const AbstractTimeTrend * pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getInsideTrend());
                    //if (pTrend) {
                    //   Record.GetFieldValue(CLU_ALPHA_IN_FIELD).AsDouble() = pTrend->GetAlpha();
                    //   Record.GetFieldValue(CLU_BETA1_IN_FIELD).AsDouble() = pTrend->GetBeta();
                    //   //Record.GetFieldValue(CLU_FUNC_ALPHA_IN_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
                    //   const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
                    //   if (pQTrend) Record.GetFieldValue(CLU_BETA2_IN_FIELD).AsDouble() = pQTrend->GetBeta2();
                    //}
                    //pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getOutsideTrend());
                    //if (pTrend) {
                    //   Record.GetFieldValue(CLU_ALPHA_OUT_FIELD).AsDouble() = pTrend->GetAlpha();
                    //  Record.GetFieldValue(CLU_BETA1_OUT_FIELD).AsDouble() = pTrend->GetBeta();              
                    //   //Record.GetFieldValue(CLU_FUNC_ALPHA_OUT_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
                    //   const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
                    //   if (pQTrend) Record.GetFieldValue(CLU_BETA2_OUT_FIELD).AsDouble() = pQTrend->GetBeta2();
                    //}
                    //pTrend = dynamic_cast<const AbstractTimeTrend *>(&Handler.GetDataSet(0/*for now*/).getTimeTrend());
                    //if (pTrend) {
                    //    Record.GetFieldValue(CLU_ALPHA_GLOBAL_FIELD).AsDouble() = pTrend->GetAlpha();
                    //    Record.GetFieldValue(CLU_BETA1_GLOBAL_FIELD).AsDouble() = pTrend->GetBeta();
                    //    const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);              
                    //    if (pQTrend) Record.GetFieldValue(CLU_BETA2_GLOBAL_FIELD).AsDouble() = pQTrend->GetBeta2();
                    //}
                }
            }
            if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
            if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
        }
    } catch (prg_exception& x) {
        x.addTrace("Write()","LocationInformationWriter");
        throw;
    }
}

/** Preparation step before writing cluster information (i.e. calling LocationInformationWriter::Write()). */
void LocationInformationWriter::WritePrep(const CCluster& theCluster, const CSaTScanData& DataHub) {
  try {
      if (DataHub.GetParameters().GetNumDataSets() == 1 && 
          DataHub.GetParameters().GetProbabilityModelType() == NORMAL &&
          DataHub.GetParameters().getIsWeightedNormal()) {
        //Cache weighted normal model statistics instead of calculating each time in Write() method.
        const AbstractWeightedNormalRandomizer * pRandomizer=0;
        if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(DataHub.GetDataSetHandler().GetRandomizer(0))) == 0)
           throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "Write()");
        std::vector<tract_t> tractIndexes;
        theCluster.getLocationIndexes(DataHub, tractIndexes, true);
        gStatistics = pRandomizer->getClusterLocationStatistics(theCluster.m_nFirstInterval, theCluster.m_nLastInterval, tractIndexes);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("WritePrep()","LocationInformationWriter");
    throw;
  }

}
