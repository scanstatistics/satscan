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

const char * LocationRiskEstimateWriter::REL_RISK_EXT                     = ".rr";
const char * LocationRiskEstimateWriter::TIME_TREND_FIELD                 = "TIME_TREND";
const char * LocationRiskEstimateWriter::WEIGHTED_MEAN_VALUE_FIELD        = "W_MEAN";

/** class constructor */
LocationRiskEstimateWriter::LocationRiskEstimateWriter(const CSaTScanData& DataHub)
                 :AbstractDataFileWriter(DataHub.GetParameters()) {
  try {
    DefineFields(DataHub);
    if (gParameters.GetOutputRelativeRisksAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, REL_RISK_EXT);
    if (gParameters.GetOutputRelativeRisksDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, REL_RISK_EXT);
  }
  catch (prg_exception& x) {
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
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION ||
        gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON || 
        gParameters.GetProbabilityModelType() == ORDINAL || 
        gParameters.GetProbabilityModelType() == CATEGORICAL || 
        gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      throw prg_error("Risk estimates file not implemented for %s model.",
                      "SetupFields()", ParametersPrint(DataHub.GetParameters()).GetProbabilityModelTypeAsString());
    CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, GetLocationIdentiferFieldLength(DataHub), 0, uwOffset);
    if (gParameters.GetNumDataSets() > 1)
      CreateField(vFieldDefinitions, DATASET_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset);
    if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
      CreateField(vFieldDefinitions, CATEGORY_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset);
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      CreateField(vFieldDefinitions, MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
      CreateField(vFieldDefinitions, STD_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
    } else if (gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL) {
      CreateField(vFieldDefinitions, MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
      CreateField(vFieldDefinitions, WEIGHTED_MEAN_VALUE_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);      
    }
    else {
      CreateField(vFieldDefinitions, OBSERVED_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset);
      CreateField(vFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 2, uwOffset);
      CreateField(vFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
      CreateField(vFieldDefinitions, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
    }
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      CreateField(vFieldDefinitions, TIME_TREND_FIELD, FieldValue::NUMBER_FLD, 19, 3, uwOffset);
  }
  catch (prg_exception& x) {
    x.addTrace("DefineFields()","LocationRiskEstimateWriter");
    throw;
  }
}

/** Returns location identifier for tract at tTractIndex. If tTractIndex refers
    to more than one location identifier, string returned contains first
    encountered location with string "et al" concatenated. */
std::string & LocationRiskEstimateWriter::getLocationId(std::string& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const {
  sId = DataHub.GetTInfo()->getIdentifier(tTractIndex);
  //if location index is not referencing a meta location and location index is associated with more than one locations_id,
  //then expressive this situation by appending 'et al' (if there is room).
  if ((size_t)tTractIndex < DataHub.GetTInfo()->getLocations().size() && DataHub.GetTInfo()->getLocations().at(tTractIndex)->getSecondaryIdentifiers().size()) {
    if (sId.size() + strlen(" et al") <= GetLocationIdentiferFieldLength(DataHub))
      sId += " et al";
  }
  return sId;
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::Write(const CSaTScanData& DataHub) {
  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
      RecordRelativeRiskDataAsOrdinal(DataHub);
    else if (gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL) {
      RecordRelativeRiskDataAsWeightedNormal(DataHub);
    }
    else
      RecordRelativeRiskDataStandard(DataHub);
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskData()","LocationRiskEstimateWriter");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub) {
  std::string           sBuffer;
  count_t             * pCases;
  double                dExpected, dDenominator, dNumerator;
  std::vector<count_t>  vDataSetLocationPopulation;
  RecordBuffer          Record(vFieldDefinitions);

  try {
    for (size_t i=0; i < gParameters.GetNumDataSets(); ++i) {
       const RealDataSet& DataSet = DataHub.GetDataSetHandler().GetDataSet(i);
       const PopulationData& Population = DataSet.getPopulationData();
       tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
       // first calculate populations for each location irrespective of category
       vDataSetLocationPopulation.assign(tTotalLocations, 0);
       for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
          pCases = DataSet.getCategoryCaseData(j).GetArray()[0];
          for (tract_t m=0; m < tTotalLocations; ++m)
              vDataSetLocationPopulation[m] += pCases[m];
       }
       // for each category in data set, record relative risk data
       for (tract_t t=0; t < tTotalLocations; ++t) {
          getLocationId(sBuffer, t, DataHub);
          for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
             pCases = DataSet.getCategoryCaseData(j).GetArray()[0];
             Record.SetAllFieldsBlank(true);
             Record.GetFieldValue(LOC_ID_FIELD).AsString() = sBuffer;
             if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
               Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
             if (gParameters.GetNumDataSets() > 1)
               Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
             Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = j + 1;
             Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
             dExpected = (double)vDataSetLocationPopulation[t] * (double)Population.GetNumOrdinalCategoryCases(j) / DataSet.getTotalPopulation();
             Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
             if (dExpected) {
               Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
               dNumerator = Population.GetNumOrdinalCategoryCases(j) - pCases[t];
               dDenominator = Population.GetNumOrdinalCategoryCases(j) - dExpected;
               if (dDenominator && dNumerator/dDenominator)
                 Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
             }
             if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
             if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
          }
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskDataAsOrdinal(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

/** Writes obvserved, expected, observed/expected and relative risk to record.*/
void LocationRiskEstimateWriter::RecordRelativeRiskDataStandard(const CSaTScanData& DataHub) {
  unsigned int          i;
  tract_t               t;
  std::string           sBuffer;
  count_t             * pCases;
  measure_t           * pMeasure, * pMeasureAux(0);
  double                dExpected, dDenominator, dNumerator;
  RecordBuffer          Record(vFieldDefinitions);
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();

  try {
    for (i=0; i < Handler.GetNumDataSets(); ++i) {
       pCases = Handler.GetDataSet(i).getCaseData().GetArray()[0];
       pMeasure = Handler.GetDataSet(i).getMeasureData().GetArray()[0];
       if (gParameters.GetProbabilityModelType() == NORMAL || gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL)
         pMeasureAux = Handler.GetDataSet(i).getMeasureData_Aux().GetArray()[0];
       tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
       for (t=0; t < tTotalLocations; ++t) {
          Record.SetAllFieldsBlank(true);
          Record.GetFieldValue(LOC_ID_FIELD).AsString() = getLocationId(sBuffer, t, DataHub);
          if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
            Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
          if (gParameters.GetNumDataSets() > 1)
            Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
          if (gParameters.GetProbabilityModelType() == NORMAL || gParameters.GetProbabilityModelType() == WEIGHTEDNORMAL) {
            if (pCases[t]) {
              Record.GetFieldValue(MEAN_VALUE_FIELD).AsDouble() = pMeasure[t]/pCases[t];
              Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(GetUnbiasedVariance(pCases[t], pMeasure[t], pMeasureAux[t]));
            }  
          }
          if (gParameters.GetProbabilityModelType() != NORMAL && gParameters.GetProbabilityModelType() != WEIGHTEDNORMAL) {
            Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
            dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
            Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
            if (dExpected) {
              Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
              dDenominator = Handler.GetDataSet(i).getTotalCases() - dExpected;
              dNumerator = Handler.GetDataSet(i).getTotalCases() - pCases[t];
              if (dDenominator && dNumerator/dDenominator)
                Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
            }
          }
          if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
          if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskDataStandard(const CSaTScanData&)","LocationRiskEstimateWriter");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::RecordRelativeRiskDataAsWeightedNormal(const CSaTScanData& DataHub) {
  std::string           sBuffer;
  RecordBuffer          Record(vFieldDefinitions);
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  const AbstractWeightedNormalRandomizer * pRandomizer=0;

  try {
    for (unsigned int i=0; i < Handler.GetNumDataSets(); ++i) {
        if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Handler.GetRandomizer(i))) == 0)
          throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "WriteClusterInformation()");
        AbstractWeightedNormalRandomizer::RiskEstimateStatistics statistics = pRandomizer->getRiskEstimateStatistics();
        tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
        for (tract_t t=0; t < tTotalLocations; ++t) {
           Record.SetAllFieldsBlank(true);
           Record.GetFieldValue(LOC_ID_FIELD).AsString() = getLocationId(sBuffer, t, DataHub);
           if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
             Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
           if (gParameters.GetNumDataSets() > 1)
             Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
           Record.GetFieldValue(MEAN_VALUE_FIELD).AsDouble() = statistics.gtMean[t];
           Record.GetFieldValue(WEIGHTED_MEAN_VALUE_FIELD).AsDouble() = statistics.gtWeightedMean[t];
          if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
          if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
        }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskDataAsWeightedNormal()","LocationRiskEstimateWriter");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records
    - particular functionality for SVTT analysis  */
void LocationRiskEstimateWriter::Write(const CSVTTData& DataHub) {
  unsigned int                  i, j;
  tract_t                       t;
  std::string                   sBuffer;
  count_t                     * pCases, ** ppCasesNC;
  std::vector<count_t>          vTemporalTractCases(DataHub.GetNumTimeIntervals());
  measure_t                   * pMeasure, ** ppMeasureNC;
  std::vector<measure_t>        vTemporalTractObserved(DataHub.GetNumTimeIntervals());
  double                        dExpected, dDenominator, dNumerator;
  CTimeTrend                    TractTimeTrend;
  RecordBuffer                  Record(vFieldDefinitions);

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData().GetArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData().GetArray()[0];
       ppCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData_NC().GetArray();
       ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData_NC().GetArray();
       tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
       for (t=0; t < tTotalLocations; ++t) {
          Record.SetAllFieldsBlank(true);
          Record.GetFieldValue(LOC_ID_FIELD).AsString() = getLocationId(sBuffer, t, DataHub);
          if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
            Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
          if (gParameters.GetNumDataSets() > 1)
            Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
          Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
          if (dExpected) {
            Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
            dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).getTotalCases() - dExpected;
            dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).getTotalCases() - pCases[t];
            if (dDenominator && dNumerator/dDenominator)
              Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
          }
          //calculate total cases by time intervals for this tract
          for (j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
             vTemporalTractCases[j] = ppCasesNC[j][t];
             vTemporalTractObserved[j] = ppMeasureNC[j][t];
          }
          TractTimeTrend.CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                         DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
          switch (TractTimeTrend.GetStatus()) {
            case CTimeTrend::UNDEFINED : break;
            case CTimeTrend::NEGATIVE_INFINITY :
               Record.GetFieldValue(TIME_TREND_FIELD).AsDouble() = CTimeTrend::NEGATIVE_INFINITY_INDICATOR;
               break;
            case CTimeTrend::POSITIVE_INFINITY :
               Record.GetFieldValue(TIME_TREND_FIELD).AsDouble() = CTimeTrend::POSITIVE_INFINITY_INDICATOR;
               break;
            case CTimeTrend::NOT_CONVERGED     :
               throw prg_error("The time trend did not converge.\n","Write()");
            case CTimeTrend::CONVERGED         :
               TractTimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
               Record.GetFieldValue(TIME_TREND_FIELD).AsDouble() = TractTimeTrend.GetAnnualTimeTrend();
               break;
            default : throw prg_error("Unknown time trend status type '%d'.", "Write()", TractTimeTrend.GetStatus());
          };
          if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
          if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskData(const CSVTTData&)","RelativeRiskData");
    throw;
  }
}
