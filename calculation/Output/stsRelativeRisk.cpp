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

const char * RelativeRiskData::REL_RISK_EXT                = ".rr";
const char * RelativeRiskData::TIME_TREND_FIELD            = "TIME_TREND";
const char * RelativeRiskData::DATASTREAM_FIELD            = "STREAM";

/** Returns location identifier for tract at tTractIndex. If tTractIndex refers
    to more than one location identifier, string returned contains first
    encountered location with string "et al" concatenated. */
ZdString & RelativeRiskData::GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub) {
  std::vector<std::string> vIdentifiers;

  DataHub.GetTInfo()->tiGetTractIdentifiers(tTractIndex, vIdentifiers);
  sId = vIdentifiers.front().c_str();
  if (vIdentifiers.size() > 1)
    sId << " et al";
  return sId;
}

void RelativeRiskData::RecordRelativeRiskData(const CSaTScanData& DataHub) {
  OutputRecord        * pRecord = 0;
  unsigned int          i;
  tract_t               t;
  ZdString              sBuffer;
  count_t             * pCases;
  measure_t           * pMeasure;
  double                dExpected;

  try {
    for (i=0; i < gParameters.GetNumDataStreams(); ++i) {
       pCases = DataHub.GetDataStreamHandler().GetStream(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataStreamHandler().GetStream(i).GetMeasureArray()[0];
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          pRecord = new OutputRecord(gvFields);
          pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          pRecord->GetFieldValue(GetFieldNumber(DATASTREAM_FIELD)).AsDouble() = i + 1;
          pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() = dExpected;
          if (dExpected)
            pRecord->GetFieldValue(GetFieldNumber(REL_RISK_FIELD)).AsZdString().printf("%12.3f",
                                                                                       ((double)pCases[t])/dExpected);
          else
            pRecord->GetFieldValue(GetFieldNumber(REL_RISK_FIELD)).AsZdString() = "n/a";
          BaseOutputStorageClass::AddRecord(pRecord);
       }
    }
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordRelativeRiskData(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

void RelativeRiskData::RecordRelativeRiskData(const CSVTTData& DataHub) {
  OutputRecord                * pRecord = 0;
  unsigned int                  i, j;
  tract_t                       t;
  ZdString                      sBuffer;
  count_t                     * pCases, ** ppCasesNC;
  std::vector<count_t>          vTemporalTractCases(DataHub.GetNumTimeIntervals());
  measure_t                   * pMeasure, ** ppMeasureNC;
  std::vector<measure_t>        vTemporalTractObserved(DataHub.GetNumTimeIntervals());
  double                        dExpected;
  CTimeTrend                    TractTimeTrend;

  try {
    for (i=0; i < gParameters.GetNumDataStreams(); ++i) {
       pCases = DataHub.GetDataStreamHandler().GetStream(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataStreamHandler().GetStream(i).GetMeasureArray()[0];
       ppCasesNC = DataHub.GetDataStreamHandler().GetStream(i).GetNCCaseArray();
       ppMeasureNC = DataHub.GetDataStreamHandler().GetStream(i).GetNCMeasureArray();
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          pRecord = new OutputRecord(gvFields);
          pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          pRecord->GetFieldValue(GetFieldNumber(DATASTREAM_FIELD)).AsDouble() = i + 1;
          pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() = dExpected;
          if (dExpected)
            pRecord->GetFieldValue(GetFieldNumber(REL_RISK_FIELD)).AsZdString().printf("%12.3f",
                                                                                       ((double)pCases[t])/dExpected);
          else
            pRecord->GetFieldValue(GetFieldNumber(REL_RISK_FIELD)).AsZdString() = "n/a";
          //calculate total cases by time intervals for this tract
          for (j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
             vTemporalTractCases[j] = ppCasesNC[j][t];
             vTemporalTractObserved[j] = ppMeasureNC[j][t];
          }
          TractTimeTrend.CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                         DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
          TractTimeTrend.SetAnnualTimeTrend(gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
          sBuffer.printf("%6.3f", (TractTimeTrend.IsNegative() ? -1 : 1) * TractTimeTrend.GetAnnualTimeTrend());
          pRecord->GetFieldValue(GetFieldNumber(TIME_TREND_FIELD)).AsZdString() = sBuffer;
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

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void RelativeRiskData::SetupFields() {
  unsigned short uwOffset = 0;

  try {
    CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(gvFields, DATASTREAM_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    CreateField(gvFields, OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    CreateField(gvFields, EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
    CreateField(gvFields, REL_RISK_FIELD, ZD_ALPHA_FLD, 12, 0, uwOffset);
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      CreateField(gvFields, TIME_TREND_FIELD, ZD_ALPHA_FLD, 12, 0, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupFields()","RelativeRiskData");
    throw;
  }
}
