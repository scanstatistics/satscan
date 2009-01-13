//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterLocationsWriter.h"
#include "cluster.h"
#include "SVTTCluster.h"
#include "SSException.h"

const char * LocationInformationWriter::AREA_SPECIFIC_FILE_EXT      = ".gis";
const char * LocationInformationWriter::LOC_OBS_FIELD               = "LOC_OBS";
const char * LocationInformationWriter::LOC_EXP_FIELD               = "LOC_EXP";
const char * LocationInformationWriter::LOC_MEAN_FIELD              = "LOC_MEAN";
const char * LocationInformationWriter::LOC_WEIGHTED_MEAN_FIELD     = "LOC_WMEAN";
const char * LocationInformationWriter::LOC_OBS_DIV_EXP_FIELD       = "LOC_ODE";
const char * LocationInformationWriter::LOC_REL_RISK_FIELD          = "LOC_RISK";
const char * LocationInformationWriter::LOC_TIME_TREND_FIELD        = "LOC_TREND";
const char * LocationInformationWriter::CLU_OBS_FIELD               = "CLU_OBS";
const char * LocationInformationWriter::CLU_EXP_FIELD               = "CLU_EXP";
const char * LocationInformationWriter::CLU_OBS_DIV_EXP_FIELD       = "CLU_ODE";
const char * LocationInformationWriter::CLU_REL_RISK_FIELD          = "CLU_RISK";
const char * LocationInformationWriter::CLU_MEAN_IN_FIELD           = "CLU_MEAN_I";
const char * LocationInformationWriter::CLU_MEAN_OUT_FIELD          = "CLU_MEAN_O";
const char * LocationInformationWriter::CLU_WEIGHTED_MEAN_IN_FIELD  = "CL_WMEAN_I";
const char * LocationInformationWriter::CLU_WEIGHTED_MEAN_OUT_FIELD = "CL_WMEAN_O";
const char * LocationInformationWriter::CLU_TIME_TREND_IN_FIELD     = "CLU_TT_IN";
const char * LocationInformationWriter::CLU_TIME_TREND_OUT_FIELD    = "CLU_TT_OUT";
const char * LocationInformationWriter::CLU_TIME_TREND_DIFF_FIELD   = "CLU_TT_DIF";

/** class constructor */
LocationInformationWriter::LocationInformationWriter(const CSaTScanData& DataHub, bool bExcludePValueField, bool bAppend)
                          :AbstractDataFileWriter(DataHub.GetParameters()), gbExcludePValueField(bExcludePValueField) {
  try {
    DefineFields(DataHub);
    if (gParameters.GetOutputAreaSpecificAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, AREA_SPECIFIC_FILE_EXT, bAppend);
    if (gParameters.GetOutputAreaSpecificDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, AREA_SPECIFIC_FILE_EXT, bAppend);
  }
  catch (prg_exception& x) {
    delete gpASCIIFileWriter; gpASCIIFileWriter=0;
    delete gpDBaseFileWriter; gpDBaseFileWriter=0;
    x.addTrace("constructor","LocationInformationWriter");
    throw;
  }
}

/** class destructor */
LocationInformationWriter::~LocationInformationWriter() {}

// sets up the vector of field structs so that the FieldDef Vector can be created
void LocationInformationWriter::DefineFields(const CSaTScanData& DataHub) {
  unsigned short uwOffset = 0;

  try {
    CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, GetLocationIdentiferFieldLength(DataHub), 0, uwOffset, 0);
    CreateField(vFieldDefinitions, CLUST_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);

    if (!gbExcludePValueField) {
      std::string buffer;
      printString(buffer, "%u", gParameters.GetNumReplicationsRequested());
      CreateField(vFieldDefinitions, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, std::min(17,(int)buffer.size()), uwOffset, buffer.size());
    }

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
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
        CreateField(vFieldDefinitions, CLU_TIME_TREND_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(vFieldDefinitions, CLU_TIME_TREND_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        //CreateField(vFieldDefinitions, CLU_TIME_TREND_DIFF_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
      }
    }
    //defined location level fields to report -- none of these are reported
    // for multiple data sets nor the ordinal probability model
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
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
      if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
        CreateField(vFieldDefinitions, LOC_TIME_TREND_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DefineFields()","LocationInformationWriter");
    throw;
  }
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void LocationInformationWriter::Write(const CCluster& theCluster, const CSaTScanData& DataHub, int iClusterNumber, tract_t tTract, unsigned int iNumSimsCompleted) {
  TractHandler::Location::StringContainer_t     vIdentifiers;
  double                                        dRelativeRisk;
  RecordBuffer                                  Record(vFieldDefinitions);
  const DataSetHandler                        & Handler = DataHub.GetDataSetHandler();

  try {
    //do not report locations for which iterative scan has nullified its data
    if (DataHub.GetIsNullifiedLocation(tTract)) return;
    DataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vIdentifiers);
    for (unsigned int i=0; i < vIdentifiers.size(); ++i) {
       Record.SetAllFieldsBlank(true);
       Record.GetFieldValue(LOC_ID_FIELD).AsString() = vIdentifiers[i].c_str();
       if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
         Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
       Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
       if (!gbExcludePValueField)
         Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
       //location information fields are only present for one dataset and not ordinal model
       if (Handler.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL) {
         //When there is more than one identifiers for a tract, this indicates
         //that locations where combined. Print a record for each location but
         //leave area specific information blank.
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
           if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
             CTimeTrend TractTimeTrend;
             std::vector<count_t> vTemporalTractCases(DataHub.GetNumTimeIntervals());
             std::vector<measure_t> vTemporalTractObserved(DataHub.GetNumTimeIntervals());
             count_t ** ppCasesNC = DataHub.GetDataSetHandler().GetDataSet().getCaseData_NC().GetArray();
             measure_t ** ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet().getMeasureData_NC().GetArray();
             //calculate total cases by time intervals for this tract
             for (unsigned int j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
               vTemporalTractCases[j] = ppCasesNC[j][tTract];
               vTemporalTractObserved[j] = ppMeasureNC[j][tTract];
             }
             TractTimeTrend.CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                            DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
             switch (TractTimeTrend.GetStatus()) {
                case CTimeTrend::UNDEFINED : break;
                case CTimeTrend::NEGATIVE_INFINITY :
                   Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = CTimeTrend::NEGATIVE_INFINITY_INDICATOR;
                   break;
                case CTimeTrend::POSITIVE_INFINITY :
                   Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = CTimeTrend::POSITIVE_INFINITY_INDICATOR;
                   break;
                case CTimeTrend::NOT_CONVERGED     :
                   throw prg_error("The time trend did not converge.\n","Write()");
                case CTimeTrend::CONVERGED         :
                   TractTimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                   Record.GetFieldValue(LOC_TIME_TREND_FIELD).AsDouble() = TractTimeTrend.GetAnnualTimeTrend();
                   break;
                default : throw prg_error("Unknown time trend status type '%d'.", "Write()", TractTimeTrend.GetStatus());
             };
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
         if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
             (dRelativeRisk = theCluster.GetRelativeRisk(DataHub)) != -1)
           Record.GetFieldValue(CLU_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
         if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
           const AbtractSVTTClusterData * pClusterData=0;
           if ((pClusterData = dynamic_cast<const AbtractSVTTClusterData*>(theCluster.GetClusterData())) == 0)
             throw prg_error("Dynamic cast to AbtractSVTTClusterData failed.\n", "Write()");
           switch (pClusterData->getInsideTrend()->GetStatus()) {
             case CTimeTrend::UNDEFINED         : break;
             case CTimeTrend::CONVERGED         :
               Record.GetFieldValue(CLU_TIME_TREND_IN_FIELD).AsDouble() = pClusterData->getInsideTrend()->GetAnnualTimeTrend(); break;
             case CTimeTrend::NEGATIVE_INFINITY :
               Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = CTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
             case CTimeTrend::POSITIVE_INFINITY :
               Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = CTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
             case CTimeTrend::NOT_CONVERGED     :
               throw prg_error("The time trend did not converge.\n","Write()");
             default : throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getInsideTrend()->GetStatus());
           }
           switch (pClusterData->getOutsideTrend()->GetStatus()) {
             case CTimeTrend::UNDEFINED         : break;
             case CTimeTrend::CONVERGED         :
               Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = pClusterData->getOutsideTrend()->GetAnnualTimeTrend(); break;
             case CTimeTrend::NEGATIVE_INFINITY :
               Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = CTimeTrend::NEGATIVE_INFINITY_INDICATOR; break;
             case CTimeTrend::POSITIVE_INFINITY :
               Record.GetFieldValue(CLU_TIME_TREND_OUT_FIELD).AsDouble() = CTimeTrend::POSITIVE_INFINITY_INDICATOR; break;
             case CTimeTrend::NOT_CONVERGED     :
               throw prg_error("The time trend did not converge.\n","Write()");
             default : throw prg_error("Unknown time trend status type '%d'.", "Write()", pClusterData->getOutsideTrend()->GetStatus());
           }
         }
       }

       if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
       if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
    }
  }
  catch (prg_exception& x) {
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
