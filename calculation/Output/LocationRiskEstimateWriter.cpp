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
const char * LocationRiskEstimateWriter::TREND_IN_FIELD                   = "IN_TREND";
const char * LocationRiskEstimateWriter::TREND_OUT_FIELD                  = "OUT_TREND";
const char * LocationRiskEstimateWriter::ALPHA_IN_FIELD                   = "IN_ITRCPT";
const char * LocationRiskEstimateWriter::BETA1_IN_FIELD                   = "IN_LINEAR";
const char * LocationRiskEstimateWriter::BETA2_IN_FIELD                   = "IN_QUAD";
const char * LocationRiskEstimateWriter::ALPHA_OUT_FIELD                  = "OUT_ITRCPT";
const char * LocationRiskEstimateWriter::BETA1_OUT_FIELD                  = "OUT_LINEAR";
const char * LocationRiskEstimateWriter::BETA2_OUT_FIELD                  = "OUT_QUAD";
//const char * LocationRiskEstimateWriter::FUNC_ALPHA_IN_FIELD              = "IN_FUNC_A";
//const char * LocationRiskEstimateWriter::FUNC_ALPHA_OUT_FIELD             = "OUT_FUNC_A";
const char * LocationRiskEstimateWriter::WEIGHTED_MEAN_VALUE_FIELD        = "W_MEAN";
const char * LocationRiskEstimateWriter::OLIVEIRA_F_FIELD                 = "OLIVEIRA_F";

/** class constructor */
LocationRiskEstimateWriter::LocationRiskEstimateWriter(const CSaTScanData& DataHub)
                 :AbstractDataFileWriter(DataHub.GetParameters()) {
  try {
    DefineFields(DataHub);
    if (gParameters.GetOutputRelativeRisksAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, REL_RISK_EXT);
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
        if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON || 
            gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL /*|| gParameters.GetAnalysisType() == SPATIALVARTEMPTREND*/)
            throw prg_error("Risk estimates file not implemented for %s model.", "SetupFields()", ParametersPrint(DataHub.GetParameters()).GetProbabilityModelTypeAsString());
        CreateField(vFieldDefinitions, LOC_ID_FIELD, FieldValue::ALPHA_FLD, GetLocationIdentiferFieldLength(DataHub), 0, uwOffset, 0);
        if (gParameters.GetNumDataSets() > 1)
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
            //CreateField(vFieldDefinitions, FUNC_ALPHA_IN_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
            //CreateField(vFieldDefinitions, FUNC_ALPHA_OUT_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 10);
        }
        if (gParameters.getCalculateOliveirasF()) {
            std::string buffer;
            printString(buffer, "%u", gParameters.getNumRequestedOliveiraSets());
            CreateField(vFieldDefinitions, OLIVEIRA_F_FIELD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, buffer.size());
        }
    } catch (prg_exception& x) {
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
void LocationRiskEstimateWriter::Write(const CSaTScanData& DataHub, const Relevance_Container_t& location_relevance) {
  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL)
      RecordRelativeRiskDataAsOrdinal(DataHub);
    else if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
      RecordRelativeRiskDataAsWeightedNormal(DataHub);
    }
    else
      RecordRelativeRiskDataStandard(DataHub, location_relevance);
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
               Record.GetFieldValue(DATASET_FIELD).AsDouble() = static_cast<double>(i + 1);
             Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = static_cast<double>(j + 1);
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
void LocationRiskEstimateWriter::RecordRelativeRiskDataStandard(const CSaTScanData& DataHub, const Relevance_Container_t& location_relevance) {
  std::string           sBuffer;
  RecordBuffer          Record(vFieldDefinitions);
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();

  try {
    for (unsigned int i=0; i < Handler.GetNumDataSets(); ++i) {
       count_t * pCases = Handler.GetDataSet(i).getCaseData().GetArray()[0];
       measure_t * pMeasure = Handler.GetDataSet(i).getMeasureData().GetArray()[0];
       measure_t * pMeasureAux(0);
       if (gParameters.GetProbabilityModelType() == NORMAL)
         pMeasureAux = Handler.GetDataSet(i).getMeasureData_Aux().GetArray()[0];
       tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
       for (tract_t t=0; t < tTotalLocations; ++t) {
          Record.SetAllFieldsBlank(true);
          Record.GetFieldValue(LOC_ID_FIELD).AsString() = getLocationId(sBuffer, t, DataHub);
          if (gParameters.getCalculateOliveirasF() && location_relevance.size() > t) {
            Record.GetFieldValue(OLIVEIRA_F_FIELD).AsDouble() = static_cast<double>(location_relevance[t]) / static_cast<double>(gParameters.getNumRequestedOliveiraSets());
          }
          if (Record.GetFieldValue(LOC_ID_FIELD).AsString().size() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
            Record.GetFieldValue(LOC_ID_FIELD).AsString().resize(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
          if (gParameters.GetNumDataSets() > 1)
            Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
          if (gParameters.GetProbabilityModelType() == NORMAL) {
            if (pCases[t]) {
              Record.GetFieldValue(MEAN_VALUE_FIELD).AsDouble() = pMeasure[t]/pCases[t];
              Record.GetFieldValue(STD_FIELD).AsDouble() = std::sqrt(GetUnbiasedVariance(pCases[t], pMeasure[t], pMeasureAux[t]));
            }  
          } else {
            Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
            double dExpected, dDenominator, dNumerator;
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
        AbstractWeightedNormalRandomizer::RiskEstimateStatistics statistics = pRandomizer->getRiskEstimateStatistics(DataHub);
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
  count_t                     * pCases, ** ppCasesNC, * pPTCasesNC;
  std::vector<count_t>          vTemporalTractCases(DataHub.GetNumTimeIntervals()), vTemporalGlobalCases(DataHub.GetNumTimeIntervals(), 0);
  measure_t                   * pMeasure, ** ppMeasureNC, * pPTMeasureNC;
  std::vector<measure_t>        vTemporalTractObserved(DataHub.GetNumTimeIntervals()), vTemporalGlobalObserved(DataHub.GetNumTimeIntervals(), 0.0);
  double                        dExpected, dDenominator, dNumerator;

  std::auto_ptr<AbstractTimeTrend> TractTimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
  std::auto_ptr<AbstractTimeTrend> GlobalTimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
  RecordBuffer                  Record(vFieldDefinitions);

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData().GetArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData().GetArray()[0];
       ppCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData_NC().GetArray();
       ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData_NC().GetArray();
       tract_t tTotalLocations = DataHub.GetNumTracts() + DataHub.GetNumMetaTracts();
       pPTCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).getCaseData_PT_NC();
       pPTMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).getMeasureData_PT_NC();

       // now calculate the trends for each defined location
       for (t=0; t < tTotalLocations; ++t) {
          // reset all fields to blank
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
          //isolate cases/measure for this location
          for (j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
             // location cases/measure
             vTemporalTractCases[j] = ppCasesNC[j][t];
             vTemporalTractObserved[j] = ppMeasureNC[j][t];
             // all other cases/measure
             vTemporalGlobalCases[j] = pPTCasesNC[j] - ppCasesNC[j][t];
             vTemporalGlobalObserved[j] = pPTMeasureNC[j] - ppMeasureNC[j][t];
          }
          
          // calculate trends for location and global trend
          TractTimeTrend->CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                          DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
          GlobalTimeTrend->CalculateAndSet(&vTemporalGlobalCases[0], &vTemporalGlobalObserved[0],
                                           DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());

          if (gParameters.getTimeTrendType() == LINEAR) {
            switch (TractTimeTrend->GetStatus()) {
                case AbstractTimeTrend::UNDEFINED : break;
                case AbstractTimeTrend::NEGATIVE_INFINITY :
                    Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR;
                    break;
                case AbstractTimeTrend::POSITIVE_INFINITY :
                    Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR;
                    break;
			    case AbstractTimeTrend::SINGULAR_MATRIX :
				    throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n","Write()",Record.GetFieldValue(LOC_ID_FIELD).AsCString());
                case AbstractTimeTrend::NOT_CONVERGED     :
                    throw prg_error("The time trend did not converge.\n","Write()");
                case AbstractTimeTrend::CONVERGED         :
                    TractTimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                    Record.GetFieldValue(TREND_IN_FIELD).AsDouble() = TractTimeTrend->GetAnnualTimeTrend();
                    break;
                default : throw prg_error("Unknown time trend status type '%d'.", "Write()", TractTimeTrend->GetStatus());
            };
            switch (GlobalTimeTrend->GetStatus()) {
                case AbstractTimeTrend::UNDEFINED : break;
                case AbstractTimeTrend::NEGATIVE_INFINITY :
                    Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR;
                    break;
                case AbstractTimeTrend::POSITIVE_INFINITY :
                    Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR;
                    break;
			    case AbstractTimeTrend::SINGULAR_MATRIX :
				    throw prg_error("The time trend of location '%s' in cluster was not calculated because matrix A is singular.\n","Write()",Record.GetFieldValue(LOC_ID_FIELD).AsCString());
                case AbstractTimeTrend::NOT_CONVERGED     :
                    throw prg_error("The time trend did not converge.\n","Write()");
                case AbstractTimeTrend::CONVERGED         :
                    GlobalTimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
                    Record.GetFieldValue(TREND_OUT_FIELD).AsDouble() = GlobalTimeTrend->GetAnnualTimeTrend();
                    break;
                default : throw prg_error("Unknown time trend status type '%d'.", "Write()", GlobalTimeTrend->GetStatus());
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
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RecordRelativeRiskData(const CSVTTData&)","RelativeRiskData");
    throw;
  }
}
