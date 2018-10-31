//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterInformationWriter.h"
#include "cluster.h"
#include "OrdinalLikelihoodCalculation.h"
#include "CategoricalClusterData.h"
#include "AbstractAnalysis.h"
#include "NormalClusterData.h"
#include "SVTTCluster.h"
#include "SSException.h"
#include "WeightedNormalRandomizer.h"
#include "GisUtils.h"

const char * ClusterInformationWriter::CLUSTER_FILE_EXT	            = ".col";
const char * ClusterInformationWriter::CLUSTERCASE_FILE_EXT	        = ".sci";
const char * ClusterInformationWriter::START_DATE_FLD	            = "START_DATE";
const char * ClusterInformationWriter::END_DATE_FLD	                = "END_DATE";
const char * ClusterInformationWriter::RADIUS_FIELD	                = "RADIUS";
const char * ClusterInformationWriter::E_MINOR_FIELD                = "E_MINOR";
const char * ClusterInformationWriter::E_MAJOR_FIELD                = "E_MAJOR";
const char * ClusterInformationWriter::E_ANGLE_FIELD                = "E_ANGLE";
const char * ClusterInformationWriter::E_SHAPE_FIELD                = "E_SHAPE";
const char * ClusterInformationWriter::NUM_LOCATIONS_FIELD          = "NUMBER_LOC";
const char * ClusterInformationWriter::COORD_LAT_FIELD	            = "LATITUDE";
const char * ClusterInformationWriter::COORD_LONG_FIELD	            = "LONGITUDE";
const char * ClusterInformationWriter::COORD_X_FIELD	            = "X";
const char * ClusterInformationWriter::COORD_Y_FIELD                = "Y";
const char * ClusterInformationWriter::COORD_Z_FIELD                = "Z";
const char * ClusterInformationWriter::TIME_TREND_IN_FIELD          = "IN_TREND";
const char * ClusterInformationWriter::TIME_TREND_OUT_FIELD         = "OUT_TREND";
const char * ClusterInformationWriter::TIME_TREND_DIFF_FIELD        = "DIFF_TREND";

const char * ClusterInformationWriter::ALPHA_IN_FIELD               = "IN_ITRCPT";
const char * ClusterInformationWriter::BETA1_IN_FIELD               = "IN_LINEAR";
const char * ClusterInformationWriter::BETA2_IN_FIELD               = "IN_QUAD";
const char * ClusterInformationWriter::ALPHA_OUT_FIELD              = "OUT_ITRCPT";
const char * ClusterInformationWriter::BETA1_OUT_FIELD              = "OUT_LINEAR";
const char * ClusterInformationWriter::BETA2_OUT_FIELD              = "OUT_QUAD";
const char * ClusterInformationWriter::ALPHA_GLOBAL_FIELD           = "GBL_ITRCPT";
const char * ClusterInformationWriter::BETA1_GLOBAL_FIELD           = "GBL_LINEAR";
const char * ClusterInformationWriter::BETA2_GLOBAL_FIELD           = "GBL_QUAD";
//const char * ClusterInformationWriter::FUNC_ALPHA_IN_FIELD          = "IN_FUNC_A";
//const char * ClusterInformationWriter::FUNC_ALPHA_OUT_FIELD         = "OUT_FUNC_A";

const char * ClusterInformationWriter::WEIGHTED_VARIANCE_FIELD      = "W_VARIANCE";
const char * ClusterInformationWriter::WEIGHTED_STD_FIELD           = "W_STD";
const char * ClusterInformationWriter::WEIGHTED_MEAN_INSIDE_FIELD   = "W_MEAN_IN";
const char * ClusterInformationWriter::WEIGHTED_MEAN_OUTSIDE_FIELD  = "W_MEAN_OUT";
const char * ClusterInformationWriter::WEIGHT_INSIDE_FIELD          = "WEIGHT_IN";

const char * ClusterInformationWriter::GINI_CLUSTER_FIELD           = "GINI_CLUST";


/** class constructor */
ClusterInformationWriter::ClusterInformationWriter(const CSaTScanData& DataHub, bool bAppend)
               :AbstractDataFileWriter(DataHub.GetParameters()), gDataHub(DataHub),
                gpASCIIFileDataWriter(0), gpDBaseFileDataWriter(0), gpShapeDataFileWriter(0) {
  try {
    if (gParameters.GetOutputClusterLevelFiles() || gParameters.getOutputShapeFiles())
      DefineClusterInformationFields();
    if (gParameters.GetOutputClusterCaseFiles())
      DefineClusterCaseInformationFields();
    if (gParameters.GetOutputClusterLevelAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, CLUSTER_FILE_EXT, bAppend);
    if (gParameters.GetOutputClusterCaseAscii())
      gpASCIIFileDataWriter = new ASCIIDataFileWriter(gParameters, vDataFieldDefinitions, CLUSTERCASE_FILE_EXT, bAppend);
    if (gParameters.GetOutputClusterLevelDBase() || gParameters.getOutputShapeFiles()) {
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, CLUSTER_FILE_EXT, bAppend);
      if (gParameters.getOutputShapeFiles()) {
        gpShapeDataFileWriter = new ShapeDataFileWriter(gParameters, CLUSTER_FILE_EXT, SHPT_POLYGON, bAppend);
      }
    }
    if (gParameters.GetOutputClusterCaseDBase())
      gpDBaseFileDataWriter = new DBaseDataFileWriter(gParameters, vDataFieldDefinitions, CLUSTERCASE_FILE_EXT, bAppend);
  }
  catch (prg_exception& x) {
    delete gpASCIIFileDataWriter; gpASCIIFileDataWriter=0;
    delete gpDBaseFileDataWriter; gpDBaseFileDataWriter=0;
    x.addTrace("constructor","ClusterInformationWriter");
    throw;
  }
}

/** class destructor */
ClusterInformationWriter::~ClusterInformationWriter() {
  try {
    delete gpASCIIFileDataWriter;
    delete gpDBaseFileDataWriter;

    delete gpShapeDataFileWriter;

  }
  catch (...){}
}

/** Defines fields of cluster information output file. */
void ClusterInformationWriter::DefineClusterInformationFields() {
  unsigned short uwOffset=0;
  unsigned int   i;
  std::string    buffer;

  try {
    //define fields of data file that describes cluster properties
    CreateField(vFieldDefinitions, CLUST_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, GetLocationIdentiferFieldLength(gDataHub), 0, uwOffset, 0);
    if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile()) {
      CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LAT_FIELD : COORD_X_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
          gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);
      CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LONG_FIELD : COORD_Y_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 
          gParameters.GetCoordinatesType() == CARTESIAN ? 19/* forces %g format */: 6/* same as in results file*/);
      //only Cartesian coordinates can have more than two dimensions
      if (gParameters.GetCoordinatesType() == CARTESIAN && gDataHub.GetTInfo()->getCoordinateDimensions() > 2)
        for (i=3; i <= (unsigned int)gDataHub.GetTInfo()->getCoordinateDimensions(); ++i) {
           printString(buffer, "%s%i", COORD_Z_FIELD, i - 2);
           CreateField(vFieldDefinitions, buffer.c_str(), FieldValue::NUMBER_FLD, 19, 10, uwOffset, 19/* forces %g format */);
        }
      if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
        CreateField(vFieldDefinitions, E_MINOR_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(vFieldDefinitions, E_MAJOR_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(vFieldDefinitions, E_ANGLE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(vFieldDefinitions, E_SHAPE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
      }
      else
        CreateField(vFieldDefinitions, RADIUS_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    }
    if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
        CreateField(vFieldDefinitions, START_DATE_FLD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
        CreateField(vFieldDefinitions, END_DATE_FLD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
    }
    CreateField(vFieldDefinitions, NUM_LOCATIONS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      CreateField(vFieldDefinitions, TST_STAT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
    else {
      CreateField(vFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
      if (gParameters.GetSpatialWindowType() == ELLIPTIC)
        CreateField(vFieldDefinitions, TST_STAT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
    }

    printString(buffer, "%u", gParameters.GetNumReplicationsRequested());
    CreateField(vFieldDefinitions, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, buffer.size());
    if (gParameters.GetIsProspectiveAnalysis())
        CreateField(vFieldDefinitions, RECURRENCE_INTERVAL_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    if ((gParameters.GetPValueReportingType() == STANDARD_PVALUE || gParameters.GetPValueReportingType() == TERMINATION_PVALUE) && gParameters.GetReportGumbelPValue())  
        CreateField(vFieldDefinitions, GUMBEL_P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17, uwOffset, 2);

    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
      CreateField(vFieldDefinitions, OBSERVED_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
      if (gParameters.GetProbabilityModelType() == NORMAL) {
          if (!gParameters.getIsWeightedNormal()) {
            CreateField(vFieldDefinitions, MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
          } else {
            CreateField(vFieldDefinitions, WEIGHT_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, WEIGHTED_MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, WEIGHTED_MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, WEIGHTED_VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vFieldDefinitions, WEIGHTED_STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
          }
      } else {
        CreateField(vFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(vFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
      }  
      if (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI) {
          CreateField(vFieldDefinitions, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
      }
      if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) ||
           gParameters.GetProbabilityModelType() == BERNOULLI) {
          CreateField(vFieldDefinitions, POPULATION_FIELD, FieldValue::NUMBER_FLD, 19, 2, uwOffset, 2);
      }
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
         CreateField(vFieldDefinitions, TIME_TREND_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
         CreateField(vFieldDefinitions, TIME_TREND_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
         CreateField(vFieldDefinitions, ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         CreateField(vFieldDefinitions, BETA1_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         CreateField(vFieldDefinitions, BETA2_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         CreateField(vFieldDefinitions, ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         CreateField(vFieldDefinitions, BETA1_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         CreateField(vFieldDefinitions, BETA2_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         //CreateField(vFieldDefinitions, ALPHA_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         //CreateField(vFieldDefinitions, BETA1_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         //CreateField(vFieldDefinitions, BETA2_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         //CreateField(vFieldDefinitions, FUNC_ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
         //CreateField(vFieldDefinitions, FUNC_ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
      }
    }
    CreateField(vFieldDefinitions, GINI_CLUSTER_FIELD, FieldValue::BOOLEAN_FLD, 1, 0, uwOffset, 0);
  }
  catch (prg_exception& x) {
    x.addTrace("DefineClusterInformationFields()","ClusterInformationWriter");
    throw;
  }
}

/** Defines fields of ocluster case information utput file. */
void ClusterInformationWriter::DefineClusterCaseInformationFields() {
  unsigned short uwOffset=0;

  try {
    //define fields for secondary cluster data file
    CreateField(vDataFieldDefinitions, CLUST_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    CreateField(vDataFieldDefinitions, DATASET_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    CreateField(vDataFieldDefinitions, CATEGORY_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    CreateField(vDataFieldDefinitions, OBSERVED_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    if (gParameters.GetProbabilityModelType() == NORMAL) {
        if (!gParameters.getIsWeightedNormal()) {
            CreateField(vDataFieldDefinitions, MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        } else  {
            CreateField(vDataFieldDefinitions, WEIGHT_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, WEIGHTED_MEAN_INSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, WEIGHTED_MEAN_OUTSIDE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, WEIGHTED_VARIANCE_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            CreateField(vDataFieldDefinitions, WEIGHTED_STD_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        }
    } else {
      CreateField(vDataFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
      CreateField(vDataFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    }
    //Relative risk field only reported for these probability models
    //   - relative risk calculation not defined for STP, Exponential, another to be model
    if (gParameters.GetProbabilityModelType() == POISSON  ||
        gParameters.GetProbabilityModelType() == BERNOULLI ||
        gParameters.GetProbabilityModelType() == ORDINAL ||
        gParameters.GetProbabilityModelType() == CATEGORICAL)
      CreateField(vDataFieldDefinitions, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    //if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    //   CreateField(vDataFieldDefinitions, TIME_TREND_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    //   CreateField(vDataFieldDefinitions, TIME_TREND_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    //   CreateField(vDataFieldDefinitions, ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   CreateField(vDataFieldDefinitions, BETA1_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   CreateField(vDataFieldDefinitions, BETA2_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   CreateField(vDataFieldDefinitions, ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   CreateField(vDataFieldDefinitions, BETA1_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   CreateField(vDataFieldDefinitions, BETA2_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   //CreateField(vDataFieldDefinitions, ALPHA_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //  //CreateField(vDataFieldDefinitions, BETA1_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   //CreateField(vDataFieldDefinitions, BETA2_GLOBAL_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   //CreateField(vDataFieldDefinitions, FUNC_ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //   //CreateField(vDataFieldDefinitions, FUNC_ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
    //}
    if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis()) ||
         gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(vDataFieldDefinitions, POPULATION_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);

    if (gParameters.GetProbabilityModelType() == BERNOULLI ||
        gParameters.GetProbabilityModelType() == ORDINAL ||
        gParameters.GetProbabilityModelType() == CATEGORICAL)
      CreateField(vDataFieldDefinitions, PERCENTAGE_CASES_FIELD, FieldValue::NUMBER_FLD, 5, 1, uwOffset, 1);
  }
  catch (prg_exception& x) {
    x.addTrace("DefineClusterCaseInformationFields()","ClusterInformationWriter");
    throw;
  }
}

/** formats string for the Area ID */
std::string& ClusterInformationWriter::GetAreaID(std::string& sAreaId, const CCluster& thisCluster) const {
  try {
    if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
      sAreaId = "All";
    else
      sAreaId = gDataHub.GetTInfo()->getIdentifier(thisCluster.GetMostCentralLocationIndex());
  }
  catch (prg_exception& x) {
    x.addTrace("GetAreaID","ClusterInformationWriter");
    throw;
  }
  return sAreaId;
}

/** Records the required data to be stored in the cluster output file, stores
    the values in the global vector of cluster records.
    pre: pCluster has been initialized with calculated data
    post: function will record the appropriate data into the cluster record   */
void ClusterInformationWriter::Write(const CCluster& theCluster, int iClusterNumber, const SimulationVariables& simVars) {
  try {
    if (gParameters.GetOutputClusterLevelFiles() || gParameters.getOutputShapeFiles())
      WriteClusterInformation(theCluster, iClusterNumber, simVars);
    if (gParameters.GetOutputClusterCaseFiles())
      WriteClusterCaseInformation(theCluster, iClusterNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("Write()","ClusterInformationWriter");
    throw;
  }
}

void ClusterInformationWriter::WriteClusterCaseInformation(const CCluster& theCluster, int iClusterNumber) {
    try {
        //now write to secondary cluster information file
        if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
            WriteCountOrdinalData(theCluster, iClusterNumber);
        else
            WriteCountData(theCluster, iClusterNumber);
    } catch (prg_exception& x) {
        x.addTrace("WriteClusterCaseInformation()","ClusterInformationWriter");
        throw;
    }
}

void ClusterInformationWriter::WriteClusterInformation(const CCluster& theCluster, int iClusterNumber, const SimulationVariables& simVars) {
  std::string       sBuffer;
  RecordBuffer      Record(vFieldDefinitions);
  double            dObserved, dExpected, dCasesOutside, dUnbiasedVariance, dRelativeRisk;
  const DataSetHandler & Handler = gDataHub.GetDataSetHandler();

  try {
    Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
    Record.GetFieldValue(LOC_ID_FIELD).AsString() = GetAreaID(sBuffer, theCluster);
    if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
      Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
    if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile()) {
      WriteCoordinates(Record, theCluster);
      if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
        WriteEllipseAngle(Record, theCluster);
        WriteEllipseShape(Record, theCluster);
      }
    }
    Record.GetFieldValue(NUM_LOCATIONS_FIELD).AsDouble() =
        theCluster.GetClusterType() == PURELYTEMPORALCLUSTER ? gDataHub.GetNumTracts() : theCluster.GetNumNonNullifiedTractsInCluster(gDataHub);
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    else {
      Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty();
      if (gParameters.GetSpatialWindowType() == ELLIPTIC)
        Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    }
    if (theCluster.reportablePValue(gParameters, simVars))
        Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.getReportingPValue(gParameters, simVars, gDataHub.GetParameters().GetIsIterativeScanning() || iClusterNumber == 1);
    if ((gParameters.GetPValueReportingType() == STANDARD_PVALUE || gParameters.GetPValueReportingType() == TERMINATION_PVALUE) && gParameters.GetReportGumbelPValue()) {
        std::pair<double,double> p = theCluster.GetGumbelPValue(simVars);
        Record.GetFieldValue(GUMBEL_P_VALUE_FLD).AsDouble() = std::max(p.first, p.second);
    }
    if (theCluster.reportableRecurrenceInterval(gParameters, simVars)) {
        Record.GetFieldValue(RECURRENCE_INTERVAL_FLD).AsDouble() = theCluster.GetRecurrenceInterval(gDataHub, iClusterNumber, simVars).second;
    }
    if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
        Record.GetFieldValue(START_DATE_FLD).AsString() = theCluster.GetStartDate(sBuffer, gDataHub);
        Record.GetFieldValue(END_DATE_FLD).AsString() = theCluster.GetEndDate(sBuffer, gDataHub);
    }
	Record.GetFieldValue(GINI_CLUSTER_FIELD).AsBool() = theCluster.isGiniCluster();

    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
      Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCount();
      if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
        dObserved = theCluster.GetObservedCount();
        dExpected = theCluster.GetExpectedCount(gDataHub);
        Record.GetFieldValue(MEAN_INSIDE_FIELD).AsDouble() = dExpected/dObserved;
        dCasesOutside = Handler.GetDataSet().getTotalCases() - dObserved;
        if (dCasesOutside) Record.GetFieldValue(MEAN_OUTSIDE_FIELD).AsDouble() = (Handler.GetDataSet().getTotalMeasure() - dExpected)/dCasesOutside;
        const AbstractNormalClusterData * pClusterData=0;
        if ((pClusterData = dynamic_cast<const AbstractNormalClusterData*>(theCluster.GetClusterData())) == 0)
            throw prg_error("Dynamic cast to AbstractNormalClusterData failed.\n", "WriteClusterInformation()");
        dUnbiasedVariance = GetUnbiasedVariance(static_cast<count_t>(dObserved), dExpected, pClusterData->GetMeasureAux(0),
                                                Handler.GetDataSet().getTotalCases(), Handler.GetDataSet().getTotalMeasure(),
                                                Handler.GetDataSet().getTotalMeasureAux());
        Record.GetFieldValue(VARIANCE_FIELD).AsDouble() = dUnbiasedVariance;
        Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(dUnbiasedVariance);
      } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
        const AbstractWeightedNormalRandomizer * pRandomizer=0;
        if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Handler.GetRandomizer(0))) == 0)
          throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "WriteClusterInformation()");

        std::vector<tract_t> tractIndexes;
        theCluster.getLocationIndexes(gDataHub, tractIndexes, true);
        AbstractWeightedNormalRandomizer::ClusterStatistics statistics;
        statistics = pRandomizer->getClusterStatistics(theCluster.m_nFirstInterval, theCluster.m_nLastInterval, tractIndexes);

        Record.GetFieldValue(MEAN_INSIDE_FIELD).AsDouble() = statistics.gtMeanIn;
        Record.GetFieldValue(MEAN_OUTSIDE_FIELD).AsDouble() = statistics.gtMeanOut;
        Record.GetFieldValue(WEIGHTED_MEAN_INSIDE_FIELD).AsDouble() = statistics.gtWeightedMeanIn;
        Record.GetFieldValue(WEIGHTED_MEAN_OUTSIDE_FIELD).AsDouble() = statistics.gtWeightedMeanOut;
        Record.GetFieldValue(VARIANCE_FIELD).AsDouble() = statistics.gtVariance;
        Record.GetFieldValue(WEIGHTED_VARIANCE_FIELD).AsDouble() = statistics.gtWeightedVariance;
        Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(statistics.gtVariance);
        Record.GetFieldValue(WEIGHTED_STD_FIELD).AsDouble() = std::sqrt(statistics.gtWeightedVariance);
        Record.GetFieldValue(WEIGHT_INSIDE_FIELD).AsDouble() = statistics.gtWeight;
      } else {
        Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCount(gDataHub);
        Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = theCluster.GetObservedDivExpected(gDataHub);
      }
      //either suppress printing this field because we didn't define it (not Poisson or Bernoulli) or
      //because the relative risk could not be calculated
      if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
          (dRelativeRisk = theCluster.GetRelativeRisk(gDataHub)) != -1)
          Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = dRelativeRisk;
      if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis() && theCluster.GetClusterType() != PURELYTEMPORALCLUSTER) ||
           gParameters.GetProbabilityModelType() == BERNOULLI)
          Record.GetFieldValue(POPULATION_FIELD).AsDouble() = gDataHub.GetProbabilityModel().GetPopulation(0, theCluster, gDataHub);
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
          const AbtractSVTTClusterData * pClusterData=0;
          if ((pClusterData = dynamic_cast<const AbtractSVTTClusterData*>(theCluster.GetClusterData())) == 0)
             throw prg_error("Dynamic cast to AbtractSVTTClusterData failed.\n", "WriteClusterInformation()");
          if (gParameters.getTimeTrendType() == LINEAR) {
             switch (pClusterData->getInsideTrend().GetStatus()) {
                case AbstractTimeTrend::UNDEFINED         : break;
                case AbstractTimeTrend::CONVERGED         :
                   Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = pClusterData->getInsideTrend().GetAnnualTimeTrend(); break;
                case AbstractTimeTrend::NEGATIVE_INFINITY :
                   Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                case AbstractTimeTrend::POSITIVE_INFINITY :
                   Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                case AbstractTimeTrend::NOT_CONVERGED :
                case AbstractTimeTrend::SINGULAR_MATRIX :
                default : break;
             }
             switch (pClusterData->getOutsideTrend().GetStatus()) {
                case AbstractTimeTrend::UNDEFINED         : break;
                case AbstractTimeTrend::CONVERGED         :
                   Record.GetFieldValue(TIME_TREND_OUT_FIELD).AsDouble() = pClusterData->getOutsideTrend().GetAnnualTimeTrend(); break;
                case AbstractTimeTrend::NEGATIVE_INFINITY :
                   Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
                case AbstractTimeTrend::POSITIVE_INFINITY :
                   Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
                case AbstractTimeTrend::NOT_CONVERGED :
                case AbstractTimeTrend::SINGULAR_MATRIX :
                default : break;
             }
          }
          const AbstractTimeTrend * pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getInsideTrend());
          if (pTrend) {
            Record.GetFieldValue(ALPHA_IN_FIELD).AsDouble() = pTrend->GetAlpha();
            Record.GetFieldValue(BETA1_IN_FIELD).AsDouble() = pTrend->GetBeta();
            //Record.GetFieldValue(FUNC_ALPHA_IN_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
            const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
            if (pQTrend) Record.GetFieldValue(BETA2_IN_FIELD).AsDouble() = pQTrend->GetBeta2();
          }
          pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getOutsideTrend());
          if (pTrend) {
            Record.GetFieldValue(ALPHA_OUT_FIELD).AsDouble() = pTrend->GetAlpha();
            Record.GetFieldValue(BETA1_OUT_FIELD).AsDouble() = pTrend->GetBeta();
            //Record.GetFieldValue(FUNC_ALPHA_OUT_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
            const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
            if (pQTrend) Record.GetFieldValue(BETA2_OUT_FIELD).AsDouble() = pQTrend->GetBeta2();
          }
          //pTrend = dynamic_cast<const AbstractTimeTrend *>(&Handler.GetDataSet(0/*for now*/).getTimeTrend());
          //if (pTrend) {
          //  Record.GetFieldValue(ALPHA_GLOBAL_FIELD).AsDouble() = pTrend->GetAlpha();
          //  Record.GetFieldValue(BETA1_GLOBAL_FIELD).AsDouble() = pTrend->GetBeta();
          //  const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
          //  if (pQTrend) Record.GetFieldValue(BETA2_GLOBAL_FIELD).AsDouble() = pQTrend->GetBeta2();
          //}
      }
    }
    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
  }
  catch (prg_exception& x) {
    x.addTrace("WriteClusterInformation()","ClusterInformationWriter");
    throw;
  }
}

/** Writes cluster coordinates to passed record buffer, with consideration for whether coordinate
    system is lat/long or Cartesian. Fields left blank for purely temporal clusters. */
void ClusterInformationWriter::WriteCoordinates(RecordBuffer& Record, const CCluster& thisCluster) {
  double                        dRadius;
  std::vector<double>           vCoordinates;
  std::pair<double, double>     prLatitudeLongitude;
  unsigned int                  iFirstCoordIndex, iSecondCoordIndex;
  std::string                   buffer;

   try {
     if (thisCluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
       iFirstCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LAT_FIELD : COORD_X_FIELD);
       iSecondCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LONG_FIELD : COORD_Y_FIELD);
       gDataHub.GetGInfo()->retrieveCoordinates(thisCluster.GetCentroidIndex(), vCoordinates);
       switch (gParameters.GetCoordinatesType()) {
         case CARTESIAN : Record.GetFieldValue(iFirstCoordIndex).AsDouble() =  vCoordinates[0];
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() =  vCoordinates[1];
                          for (size_t i=2; i < vCoordinates.size(); ++i) {
                             printString(buffer, "%s%d", COORD_Z_FIELD, (i - 1));
                             Record.GetFieldValue(buffer).AsDouble() = vCoordinates[i];
                          }
                          if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
                            dRadius = thisCluster.GetCartesianRadius();
                            Record.GetFieldValue(E_MINOR_FIELD).AsDouble() = dRadius;
                            Record.GetFieldValue(E_MAJOR_FIELD).AsDouble() = dRadius * gDataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
                          }
                          else {
                            Record.GetFieldValue(RADIUS_FIELD).AsDouble() = thisCluster.GetCartesianRadius();
                          }
                          break;
         case LATLON    : prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                          Record.GetFieldValue(iFirstCoordIndex).AsDouble() = prLatitudeLongitude.first;
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() = prLatitudeLongitude.second;
                          // write to shapefile, if we are creating one
                          if (gpShapeDataFileWriter) {
                              if (gpShapeDataFileWriter->getShapeType() == SHPT_POINT) {
                                  gpShapeDataFileWriter->writeCoordinates(prLatitudeLongitude.second, prLatitudeLongitude.first);
                              } else if (gpShapeDataFileWriter->getShapeType() == SHPT_POLYGON) {
                                std::vector<double> x,y;
                                GisUtils::pointpair_t clusterSegment = GisUtils::getClusterRadiusSegmentPoints(gDataHub, thisCluster);
                                GisUtils::points_t circlePoints = GisUtils::getPointsOnCircleCircumference(clusterSegment.first, clusterSegment.second);
                                // break the circle points into collection of x and collection of y.
                                for(GisUtils::points_t::const_iterator itrPt=circlePoints.begin(); itrPt != circlePoints.end(); ++itrPt) {
                                    x.push_back(itrPt->first);
                                    y.push_back(itrPt->second);
                                }
                                gpShapeDataFileWriter->writePolygon(x, y);
                              } else {
                                throw prg_error("Unknown shapefile type '%d'.","WriteCoordinates()", gpShapeDataFileWriter->getShapeType());
                              }
                          }

                          dRadius = 2 * EARTH_RADIUS_km * asin(thisCluster.GetCartesianRadius()/(2 * EARTH_RADIUS_km));
                          Record.GetFieldValue(RADIUS_FIELD).AsDouble() = dRadius;
                          break;
         default : throw prg_error("Unknown coordinate type '%d'.","WriteCoordinates()", gParameters.GetCoordinatesType());
       }
     }
   }
   catch (prg_exception& x) {
     x.addTrace("WriteCoordinates()", "stsClusterData");
     throw;
   }
}

/** Writes obvserved, expected and  observed/expected to file(s).*/
void ClusterInformationWriter::WriteCountData(const CCluster& theCluster, int iClusterNumber) const {
  double                                        dExpected, dUnbiasedVariance, dRelativeRisk;
  count_t                                       dObserved, dCasesOutside;
  RecordBuffer                                  Record(vDataFieldDefinitions);
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(gDataHub));
  const DataSetHandler                        & Handler = gDataHub.GetDataSetHandler();
  std::vector<tract_t>                          tractIndexes;

  if (theCluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER)
    vComprisedDataSetIndexes.push_back(0);
  else
    theCluster.GetClusterData()->GetDataSetIndexesComprisedInRatio(theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty(), *Calculator.get(), vComprisedDataSetIndexes);

  // Calculate cluster location indexes once for weight normal model.
  if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal())
     theCluster.getLocationIndexes(gDataHub, tractIndexes, true);

  for (unsigned int iSetIndex=0; iSetIndex < gParameters.GetNumDataSets(); ++iSetIndex) {
    Record.SetAllFieldsBlank(true);
    Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
    Record.GetFieldValue(DATASET_FIELD).AsDouble() = iSetIndex + 1;
    Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = 1;
    itr_Index = std::find(vComprisedDataSetIndexes.begin(), vComprisedDataSetIndexes.end(), iSetIndex);
    if (itr_Index != vComprisedDataSetIndexes.end()) {
      Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCount(iSetIndex);
      if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
        dObserved = theCluster.GetObservedCount(iSetIndex);
        dExpected = theCluster.GetExpectedCount(gDataHub, iSetIndex);
        Record.GetFieldValue(MEAN_INSIDE_FIELD).AsDouble() = (dObserved ? dExpected/dObserved : 0);
        dCasesOutside = Handler.GetDataSet(iSetIndex).getTotalCases() - dObserved;
        Record.GetFieldValue(MEAN_OUTSIDE_FIELD).AsDouble() = (dCasesOutside ? (Handler.GetDataSet(iSetIndex).getTotalMeasure() - dExpected)/dCasesOutside : 0);
        const AbstractNormalClusterData * pClusterData=0;
        if ((pClusterData = dynamic_cast<const AbstractNormalClusterData*>(theCluster.GetClusterData())) == 0)
            throw prg_error("Dynamic cast to AbstractNormalClusterData failed.\n", "WriteCountData()");
        dUnbiasedVariance = GetUnbiasedVariance(static_cast<count_t>(dObserved), dExpected, pClusterData->GetMeasureAux(iSetIndex),
                                                Handler.GetDataSet(iSetIndex).getTotalCases(), Handler.GetDataSet(iSetIndex).getTotalMeasure(),
                                                Handler.GetDataSet(iSetIndex).getTotalMeasureAux());
        Record.GetFieldValue(VARIANCE_FIELD).AsDouble() = dUnbiasedVariance;
        Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(dUnbiasedVariance);
      } else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
        const AbstractWeightedNormalRandomizer * pRandomizer=0;
        if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Handler.GetRandomizer(iSetIndex))) == 0)
          throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "WriteClusterInformation()");

        AbstractWeightedNormalRandomizer::ClusterStatistics statistics;
        statistics = pRandomizer->getClusterStatistics(theCluster.m_nFirstInterval, theCluster.m_nLastInterval, tractIndexes);

        Record.GetFieldValue(MEAN_INSIDE_FIELD).AsDouble() = statistics.gtMeanIn;
        Record.GetFieldValue(MEAN_OUTSIDE_FIELD).AsDouble() = statistics.gtMeanOut;
        Record.GetFieldValue(WEIGHTED_MEAN_INSIDE_FIELD).AsDouble() = statistics.gtWeightedMeanIn;
        Record.GetFieldValue(WEIGHTED_MEAN_OUTSIDE_FIELD).AsDouble() = statistics.gtWeightedMeanOut;
        Record.GetFieldValue(VARIANCE_FIELD).AsDouble() = statistics.gtVariance;
        Record.GetFieldValue(WEIGHTED_VARIANCE_FIELD).AsDouble() = statistics.gtWeightedVariance;
        Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(statistics.gtVariance);
        Record.GetFieldValue(WEIGHTED_STD_FIELD).AsDouble() = std::sqrt(statistics.gtWeightedVariance);
        Record.GetFieldValue(WEIGHT_INSIDE_FIELD).AsDouble() = statistics.gtWeight;
      } else {
        Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCount(gDataHub, iSetIndex);
        Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = theCluster.GetObservedDivExpected(gDataHub, iSetIndex);
      }  
      //either suppress printing this field because we didn't define it (not Poisson or Bernoulli) or
      //because the relative risk could not be calculated 
      if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
          (dRelativeRisk = theCluster.GetRelativeRisk(gDataHub, iSetIndex)) != -1)
         Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = dRelativeRisk;
      if ((gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile() && !gParameters.GetIsPurelyTemporalAnalysis() && theCluster.GetClusterType() != PURELYTEMPORALCLUSTER) ||
          gParameters.GetProbabilityModelType() == BERNOULLI)
          Record.GetFieldValue(POPULATION_FIELD).AsDouble() = gDataHub.GetProbabilityModel().GetPopulation(iSetIndex, theCluster, gDataHub);
      if (gParameters.GetProbabilityModelType() == BERNOULLI) {
          double percentCases = 100.0 * Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / gDataHub.GetProbabilityModel().GetPopulation(iSetIndex, theCluster, gDataHub);
          Record.GetFieldValue(PERCENTAGE_CASES_FIELD).AsDouble() = percentCases;
      }

      //if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      //  const AbtractSVTTClusterData * pClusterData=0;
      //  if ((pClusterData = dynamic_cast<const AbtractSVTTClusterData*>(theCluster.GetClusterData())) == 0)
      //    throw prg_error("Dynamic cast to AbtractSVTTClusterData failed.\n", "WriteClusterInformation()");
      //  if (gParameters.getTimeTrendType() == LINEAR) {
      //     switch (pClusterData->getInsideTrend().GetStatus()) {
      //       case AbstractTimeTrend::UNDEFINED         : break;
      //      case AbstractTimeTrend::CONVERGED         :
      //         Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = pClusterData->getInsideTrend().GetAnnualTimeTrend(); break;
      //       case AbstractTimeTrend::NEGATIVE_INFINITY :
      //         Record.GetFieldValue(TIME_TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
      //       case AbstractTimeTrend::POSITIVE_INFINITY :
      //         Record.GetFieldValue(TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
      //      case AbstractTimeTrend::NOT_CONVERGED     :
      //         throw prg_error("Time trend inside of cluster is did not converge.\n","WriteCountData()");
      //       default : throw prg_error("Unknown time trend status type '%d'.", "WriteCountData()", pClusterData->getInsideTrend().GetStatus());
      //     }
      //     switch (pClusterData->getOutsideTrend().GetStatus()) {
      //       case AbstractTimeTrend::UNDEFINED         : break;
      //       case AbstractTimeTrend::CONVERGED         :
      //         Record.GetFieldValue(TIME_TREND_OUT_FIELD).AsDouble() = pClusterData->getOutsideTrend().GetAnnualTimeTrend(); break;
      //      case AbstractTimeTrend::NEGATIVE_INFINITY :
      //         Record.GetFieldValue(TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
      //       case AbstractTimeTrend::POSITIVE_INFINITY :
      //         Record.GetFieldValue(TIME_TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
      //       case AbstractTimeTrend::NOT_CONVERGED     :
      //         throw prg_error("Time trend outside of cluster is did not converge.\n","WriteCountData()");
      //       default : throw prg_error("Unknown time trend status type '%d'.", "WriteCountData()", pClusterData->getOutsideTrend().GetStatus());
      //     }
      //  }
      //  const AbstractTimeTrend * pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getInsideTrend());
      //  if (pTrend) {
      //      Record.GetFieldValue(ALPHA_IN_FIELD).AsDouble() = pTrend->GetAlpha();
      //      Record.GetFieldValue(BETA1_IN_FIELD).AsDouble() = pTrend->GetBeta();
      //      //Record.GetFieldValue(FUNC_ALPHA_IN_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
      //      const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
      //      if (pQTrend) Record.GetFieldValue(BETA2_IN_FIELD).AsDouble() = pQTrend->GetBeta2();
      //  }
      //  pTrend = dynamic_cast<const AbstractTimeTrend *>(&pClusterData->getOutsideTrend());
      //  if (pTrend) {
      //      Record.GetFieldValue(ALPHA_OUT_FIELD).AsDouble() = pTrend->GetAlpha();
      //      Record.GetFieldValue(BETA1_OUT_FIELD).AsDouble() = pTrend->GetBeta();
      //      //Record.GetFieldValue(FUNC_ALPHA_OUT_FIELD).AsDouble() = pTrend->GetGlobalAlpha();
      //      const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
      //      if (pQTrend) Record.GetFieldValue(BETA2_OUT_FIELD).AsDouble() = pQTrend->GetBeta2();
      //  }
      //  //pTrend = dynamic_cast<const AbstractTimeTrend *>(&Handler.GetDataSet(0/*for now*/).getTimeTrend());
      //  //if (pTrend) {
      //  //    Record.GetFieldValue(ALPHA_GLOBAL_FIELD).AsDouble() = pTrend->GetAlpha();
      //  //    Record.GetFieldValue(BETA1_GLOBAL_FIELD).AsDouble() = pTrend->GetBeta();
      //  //    const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(pTrend);
      //  //    if (pQTrend) Record.GetFieldValue(BETA2_GLOBAL_FIELD).AsDouble() = pQTrend->GetBeta2();
      //  //}
      //}
    }
    if (gpASCIIFileDataWriter) gpASCIIFileDataWriter->WriteRecord(Record);
    if (gpDBaseFileDataWriter) gpDBaseFileDataWriter->WriteRecord(Record);
 }
}

/** Write obvserved, expected and  observed/expected to record for ordinal data.*/
void ClusterInformationWriter::WriteCountOrdinalData(const CCluster& theCluster, int iClusterNumber) const {
  OrdinalLikelihoodCalculator                           Calculator(gDataHub);
  std::vector<OrdinalCombinedCategory>                  vCategoryContainer;
  std::vector<OrdinalCombinedCategory>::iterator        itrCategory;
  const AbstractCategoricalClusterData                * pClusterData=0;
  measure_t                                             tObservedDivExpected;
  double                                                tRelativeRisk;
  RecordBuffer                                          Record(vDataFieldDefinitions);
  std::vector<unsigned int>                             vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator                   itr_Index;

  if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(theCluster.GetClusterData())) == 0)
    throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n","WriteCountOrdinalData()");

  theCluster.GetClusterData()->GetDataSetIndexesComprisedInRatio(theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty(), Calculator, vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
    //retrieve ordinal categories in combined state
    pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, *itr_Index);
    //for each combined category
    for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       Record.SetAllFieldsBlank(true);
       Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
       Record.GetFieldValue(DATASET_FIELD).AsDouble() = *itr_Index + 1;
       //calculate observed/expected and relative risk for combined categories
       count_t tObserved=0, tTotalCategoryCases=0;
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += theCluster.GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += theCluster.GetExpectedCountOrdinal(gDataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          tTotalCategoryCases += gDataHub.GetDataSetHandler().GetDataSet(*itr_Index).getPopulationData().GetNumCategoryTypeCases(itrCategory->GetCategoryIndex(m));
       }
       //record observed/expected cases - categories which were combined will have the same value
       tObservedDivExpected = (tExpected ? (double)tObserved/tExpected  : 0);
       Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = tObservedDivExpected;
       //record relative risk - categories which were combined will have the same value
       tRelativeRisk = theCluster.GetRelativeRisk(tObserved, tExpected, tTotalCategoryCases);
       if (tRelativeRisk != -1 /*indicator of infinity*/)
         Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = tRelativeRisk;
       double dTotalCasesInClusterDataSet = gDataHub.GetProbabilityModel().GetPopulation(*itr_Index, theCluster, gDataHub);
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = itrCategory->GetCategoryIndex(m) + 1;
          //record observed cases - not combining categories
          Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          //record expected cases - not combining categories
          Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCountOrdinal(gDataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          //record percentage cases per category
          Record.GetFieldValue(PERCENTAGE_CASES_FIELD).AsDouble() = 100.0 * Record.GetFieldValue(OBSERVED_FIELD).AsDouble() / dTotalCasesInClusterDataSet;
          if (gpASCIIFileDataWriter) gpASCIIFileDataWriter->WriteRecord(Record);
          if (gpDBaseFileDataWriter) gpDBaseFileDataWriter->WriteRecord(Record);
       }
    }
  }
}

/** write cluster's elliptical angle to record */
void ClusterInformationWriter::WriteEllipseAngle(RecordBuffer& Record, const CCluster& thisCluster) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(E_ANGLE_FIELD, true);
  else if (thisCluster.GetEllipseOffset() == 0)
    Record.GetFieldValue(E_ANGLE_FIELD).AsDouble() = 0.0;
  else
    Record.GetFieldValue(E_ANGLE_FIELD).AsDouble() = thisCluster.ConvertAngleToDegrees(gDataHub.GetEllipseAngle(thisCluster.GetEllipseOffset()));
}

/** write cluster's elliptical shape to record */
void ClusterInformationWriter::WriteEllipseShape(RecordBuffer& Record, const CCluster& thisCluster) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(E_SHAPE_FIELD, true);
  else
    Record.GetFieldValue(E_SHAPE_FIELD).AsDouble() = gDataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
}

