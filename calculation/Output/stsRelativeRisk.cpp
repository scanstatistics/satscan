//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "stsRelativeRisk.h"
#include "SaTScanData.h"
#include "SVTTData.h"

/** class constructor */
RelativeRiskData::RelativeRiskData(const CParameters& Parameters)
                 :BaseOutputStorageClass(), gParameters(Parameters) {
  try {
    SetupFields();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","RelativeRiskData");
    throw;
  }
}

/** class destructor */
RelativeRiskData::~RelativeRiskData() {}

const char * RelativeRiskData::REL_RISK_EXT                     = ".rr";
const char * RelativeRiskData::TIME_TREND_FIELD                 = "TIME_TREND";
const char * RelativeRiskData::DATASET_FIELD                    = "SET";
const char * RelativeRiskData::CATEGORY_FIELD                   = "CATEGORY";
const char * RelativeRiskData::OBSERVED_DIV_EXPECTED_FIELD      = "ODE";
const char * RelativeRiskData::RELATIVE_RISK_FIELD              = "REL_RISK";

/** Returns location identifier for tract at tTractIndex. If tTractIndex refers
    to more than one location identifier, string returned contains first
    encountered location with string "et al" concatenated. */
ZdString & RelativeRiskData::GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const {
  std::vector<std::string> vIdentifiers;

  DataHub.GetTInfo()->tiGetTractIdentifiers(tTractIndex, vIdentifiers);
  sId = vIdentifiers.front().c_str();
  if (vIdentifiers.size() > 1)
    sId << " et al";
  return sId;
}

/** writes relative risk data to record and appends to internal buffer of records */
void RelativeRiskData::RecordRelativeRiskData(const CSaTScanData& DataHub) {
  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      RecordRelativeRiskDataAsOrdinal(DataHub);
    else
      RecordRelativeRiskDataStandard(DataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("RecordRelativeRiskData(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records
    - particular functionality for SVTT analysis  */
void RelativeRiskData::RecordRelativeRiskData(const CSVTTData& DataHub) {
  OutputRecord                * pRecord = 0;
  unsigned int                  i, j;
  tract_t                       t;
  ZdString                      sBuffer;
  count_t                     * pCases, ** ppCasesNC;
  std::vector<count_t>          vTemporalTractCases(DataHub.GetNumTimeIntervals());
  measure_t                   * pMeasure, ** ppMeasureNC;
  std::vector<measure_t>        vTemporalTractObserved(DataHub.GetNumTimeIntervals());
  double                        dExpected, dDenominator, dNumerator;
  CTimeTrend                    TractTimeTrend;

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).GetMeasureArray()[0];
       ppCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).GetNCCaseArray();
       ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).GetNCMeasureArray();
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          pRecord = new OutputRecord(gvFields);
          pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          if (gParameters.GetNumDataSets() > 1)
            pRecord->GetFieldValue(GetFieldNumber(DATASET_FIELD)).AsDouble() = i + 1;
          pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() = dExpected;
          if (dExpected) {
            pRecord->GetFieldValue(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD)).AsDouble() = ((double)pCases[t])/dExpected;
            dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - dExpected;
            dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - pCases[t];
            if (dDenominator && dNumerator/dDenominator)
              pRecord->GetFieldValue(GetFieldNumber(RELATIVE_RISK_FIELD)).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
            else
              pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
          }
          else {
            pRecord->SetFieldIsBlank(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD), true);
            pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
          }
          //calculate total cases by time intervals for this tract
          for (j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
             vTemporalTractCases[j] = ppCasesNC[j][t];
             vTemporalTractObserved[j] = ppMeasureNC[j][t];
          }
          TractTimeTrend.CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                         DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
          TractTimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
          pRecord->GetFieldValue(GetFieldNumber(TIME_TREND_FIELD)).AsDouble() = (TractTimeTrend.IsNegative() ? -1 : 1) * TractTimeTrend.GetAnnualTimeTrend();
          BaseOutputStorageClass::AddRecord(pRecord);
       }
    }
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordRelativeRiskData(const CSVTTData&)","RelativeRiskData");
    throw;
  }
}

/** Writes obvserved, expected, observed/expected and relative risk to record for ordinal data.*/
void RelativeRiskData::RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub) {
  OutputRecord        * pRecord = 0;
  ZdString              sBuffer;
  count_t             * pCases;
  double                dExpected, dDenominator, dNumerator;
  std::vector<count_t>  vDataSetLocationPopulation;

  try {
    for (size_t i=0; i < gParameters.GetNumDataSets(); ++i) {
       const RealDataSet& DataSet = DataHub.GetDataSetHandler().GetDataSet(i);
       const PopulationData& Population = DataSet.GetPopulationData();
       // first calculate populations for each location irrespective of category
       vDataSetLocationPopulation.assign(DataHub.GetNumTracts(), 0);
       for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
          pCases = DataSet.GetCategoryCaseArray(j)[0];
          for (tract_t m=0; m < DataHub.GetNumTracts(); ++m)
              vDataSetLocationPopulation[m] += pCases[m];
       }
       // for each category in data set, record relative risk data
       for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
          pCases = DataSet.GetCategoryCaseArray(j)[0];
          for (tract_t t=0; t < DataHub.GetNumTracts(); ++t) {
             pRecord = new OutputRecord(gvFields);
             pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetLocationId(sBuffer, t, DataHub);
             if (gParameters.GetNumDataSets() > 1)
               pRecord->GetFieldValue(GetFieldNumber(DATASET_FIELD)).AsDouble() = i + 1;
             pRecord->GetFieldValue(GetFieldNumber(CATEGORY_FIELD)).AsDouble() = j + 1;
             pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = pCases[t];
             dExpected = vDataSetLocationPopulation[t] * Population.GetNumOrdinalCategoryCases(j) / DataSet.GetTotalPopulation();
             pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() = dExpected;
             if (dExpected) {
               pRecord->GetFieldValue(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD)).AsDouble() = ((double)pCases[t])/dExpected;
               dNumerator = Population.GetNumOrdinalCategoryCases(j) - pCases[t];
               dDenominator = Population.GetNumOrdinalCategoryCases(j) - dExpected;
               if (dDenominator && dNumerator/dDenominator)
                 pRecord->GetFieldValue(GetFieldNumber(RELATIVE_RISK_FIELD)).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
               else
                 pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
             }
             else {
               pRecord->SetFieldIsBlank(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD), true);
               pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
             }
             BaseOutputStorageClass::AddRecord(pRecord);
          }
       }
    }
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordRelativeRiskDataAsOrdinal(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

/** Writes obvserved, expected, observed/expected and relative risk to record.*/
void RelativeRiskData::RecordRelativeRiskDataStandard(const CSaTScanData& DataHub) {
  OutputRecord        * pRecord = 0;
  unsigned int          i;
  tract_t               t;
  ZdString              sBuffer;
  count_t             * pCases;
  measure_t           * pMeasure;
  double                dExpected, dDenominator, dNumerator;

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).GetMeasureArray()[0];
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          pRecord = new OutputRecord(gvFields);
          pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          if (gParameters.GetNumDataSets() > 1)
            pRecord->GetFieldValue(GetFieldNumber(DATASET_FIELD)).AsDouble() = i + 1;
          pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() = dExpected;
          if (dExpected) {
            pRecord->GetFieldValue(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD)).AsDouble() = ((double)pCases[t])/dExpected;
            dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - dExpected;
            dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - pCases[t];
            if (dDenominator && dNumerator/dDenominator)
              pRecord->GetFieldValue(GetFieldNumber(RELATIVE_RISK_FIELD)).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
            else
              pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
          }
          else {
            pRecord->SetFieldIsBlank(GetFieldNumber(OBSERVED_DIV_EXPECTED_FIELD), true);
            pRecord->SetFieldIsBlank(GetFieldNumber(RELATIVE_RISK_FIELD), true);
          }
          BaseOutputStorageClass::AddRecord(pRecord);
       }
    }
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordRelativeRiskDataStandard(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void RelativeRiskData::SetupFields() {
  unsigned short uwOffset = 0;

  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL && gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      ZdGenerateException("Cluster Information file not implemented for SVTT and Ordinal model.","SetupFields()");
    CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    if (gParameters.GetNumDataSets() > 1)
      CreateField(gvFields, DATASET_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      CreateField(gvFields, CATEGORY_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    CreateField(gvFields, OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    CreateField(gvFields, EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
    CreateField(gvFields, OBSERVED_DIV_EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    CreateField(gvFields, RELATIVE_RISK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      CreateField(gvFields, TIME_TREND_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupFields()","RelativeRiskData");
    throw;
  }
}

