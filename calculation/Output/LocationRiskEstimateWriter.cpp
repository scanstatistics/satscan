//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LocationRiskEstimateWriter.h"
#include "SaTScanData.h"
#include "SVTTData.h"

const char * LocationRiskEstimateWriter::REL_RISK_EXT                     = ".rr";
const char * LocationRiskEstimateWriter::TIME_TREND_FIELD                 = "TIME_TREND";
const char * LocationRiskEstimateWriter::DATASET_FIELD                    = "SET";
const char * LocationRiskEstimateWriter::CATEGORY_FIELD                   = "CATEGORY";
const char * LocationRiskEstimateWriter::OBSERVED_DIV_EXPECTED_FIELD      = "ODE";
const char * LocationRiskEstimateWriter::RELATIVE_RISK_FIELD              = "REL_RISK";

/** class constructor */
LocationRiskEstimateWriter::LocationRiskEstimateWriter(const CParameters& Parameters)
                 :AbstractDataFileWriter(Parameters) {
  try {
    DefineFields();
    if (gParameters.GetOutputRelativeRisksAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, REL_RISK_EXT);
    if (gParameters.GetOutputRelativeRisksDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, REL_RISK_EXT);
  }
  catch (ZdException &x) {
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("constructor","LocationRiskEstimateWriter");
    throw;
  }
}

/** class destructor */
LocationRiskEstimateWriter::~LocationRiskEstimateWriter() {}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void LocationRiskEstimateWriter::DefineFields() {
  unsigned short uwOffset = 0;

  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL && gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      ZdGenerateException("Cluster Information file not implemented for SVTT and Ordinal model.","SetupFields()");
    CreateField(LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    if (gParameters.GetNumDataSets() > 1)
      CreateField(DATASET_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      CreateField(CATEGORY_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    CreateField(OBSERVED_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    CreateField(EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
    CreateField(OBSERVED_DIV_EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    CreateField(RELATIVE_RISK_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      CreateField(TIME_TREND_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","LocationRiskEstimateWriter");
    throw;
  }
}

/** Returns location identifier for tract at tTractIndex. If tTractIndex refers
    to more than one location identifier, string returned contains first
    encountered location with string "et al" concatenated. */
ZdString & LocationRiskEstimateWriter::GetLocationId(ZdString& sId, tract_t tTractIndex, const CSaTScanData& DataHub) const {
  std::vector<std::string> vIdentifiers;

  DataHub.GetTInfo()->tiGetTractIdentifiers(tTractIndex, vIdentifiers);
  sId = vIdentifiers.front().c_str();
  if (vIdentifiers.size() > 1)
    sId << " et al";
  return sId;
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::Write(const CSaTScanData& DataHub) {
  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      RecordRelativeRiskDataAsOrdinal(DataHub);
    else
      RecordRelativeRiskDataStandard(DataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("RecordRelativeRiskData()","LocationRiskEstimateWriter");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records */
void LocationRiskEstimateWriter::RecordRelativeRiskDataAsOrdinal(const CSaTScanData& DataHub) {
  ZdString              sBuffer;
  count_t             * pCases;
  double                dExpected, dDenominator, dNumerator;
  std::vector<count_t>  vDataSetLocationPopulation;
  RecordBuffer          Record(vFieldDefinitions);

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
             Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = GetLocationId(sBuffer, t, DataHub);
             if (gParameters.GetNumDataSets() > 1)
               Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
             Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = j + 1;
             Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
             dExpected = vDataSetLocationPopulation[t] * Population.GetNumOrdinalCategoryCases(j) / DataSet.GetTotalPopulation();
             Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
             if (dExpected) {
               Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
               dNumerator = Population.GetNumOrdinalCategoryCases(j) - pCases[t];
               dDenominator = Population.GetNumOrdinalCategoryCases(j) - dExpected;
               if (dDenominator && dNumerator/dDenominator)
                 Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
               else
                 Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
             }
             else {
               Record.SetFieldIsBlank(OBSERVED_DIV_EXPECTED_FIELD, true);
               Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
             }
             if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
             if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
          }
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RecordRelativeRiskDataAsOrdinal(const CSaTScanData&)","RelativeRiskData");
    throw;
  }
}

/** Writes obvserved, expected, observed/expected and relative risk to record.*/
void LocationRiskEstimateWriter::RecordRelativeRiskDataStandard(const CSaTScanData& DataHub) {
  unsigned int          i;
  tract_t               t;
  ZdString              sBuffer;
  count_t             * pCases;
  measure_t           * pMeasure;
  double                dExpected, dDenominator, dNumerator;
  RecordBuffer          Record(vFieldDefinitions);  

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).GetMeasureArray()[0];
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          if (gParameters.GetNumDataSets() > 1)
            Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
          Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
          if (dExpected) {
            Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
            dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - dExpected;
            dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - pCases[t];
            if (dDenominator && dNumerator/dDenominator)
              Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
            else
              Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
          }
          else {
            Record.SetFieldIsBlank(OBSERVED_DIV_EXPECTED_FIELD, true);
            Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
          }
          if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
          if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RecordRelativeRiskDataStandard(const CSaTScanData&)","LocationRiskEstimateWriter");
    throw;
  }
}

/** writes relative risk data to record and appends to internal buffer of records
    - particular functionality for SVTT analysis  */
void LocationRiskEstimateWriter::Write(const CSVTTData& DataHub) {
  unsigned int                  i, j;
  tract_t                       t;
  ZdString                      sBuffer;
  count_t                     * pCases, ** ppCasesNC;
  std::vector<count_t>          vTemporalTractCases(DataHub.GetNumTimeIntervals());
  measure_t                   * pMeasure, ** ppMeasureNC;
  std::vector<measure_t>        vTemporalTractObserved(DataHub.GetNumTimeIntervals());
  double                        dExpected, dDenominator, dNumerator;
  CTimeTrend                    TractTimeTrend;
  RecordBuffer                  Record(vFieldDefinitions);

  try {
    for (i=0; i < gParameters.GetNumDataSets(); ++i) {
       pCases = DataHub.GetDataSetHandler().GetDataSet(i).GetCaseArray()[0];
       pMeasure = DataHub.GetDataSetHandler().GetDataSet(i).GetMeasureArray()[0];
       ppCasesNC = DataHub.GetDataSetHandler().GetDataSet(i).GetNCCaseArray();
       ppMeasureNC = DataHub.GetDataSetHandler().GetDataSet(i).GetNCMeasureArray();
       for (t=0; t < DataHub.GetNumTracts(); ++t) {
          Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = GetLocationId(sBuffer, t, DataHub);
          if (gParameters.GetNumDataSets() > 1)
            Record.GetFieldValue(DATASET_FIELD).AsDouble() = i + 1;
          Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = pCases[t];
          dExpected = DataHub.GetMeasureAdjustment(i) * pMeasure[t];
          Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = dExpected;
          if (dExpected) {
            Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = ((double)pCases[t])/dExpected;
            dDenominator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - dExpected;
            dNumerator = DataHub.GetDataSetHandler().GetDataSet(i).GetTotalCases() - pCases[t];
            if (dDenominator && dNumerator/dDenominator)
              Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = (((double)pCases[t])/dExpected)/(dNumerator/dDenominator);
            else
              Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
          }
          else {
            Record.SetFieldIsBlank(OBSERVED_DIV_EXPECTED_FIELD, true);
            Record.SetFieldIsBlank(RELATIVE_RISK_FIELD, true);
          }
          //calculate total cases by time intervals for this tract
          for (j=0; j < (unsigned int)DataHub.GetNumTimeIntervals(); ++j) {
             vTemporalTractCases[j] = ppCasesNC[j][t];
             vTemporalTractObserved[j] = ppMeasureNC[j][t];
          }
          TractTimeTrend.CalculateAndSet(&vTemporalTractCases[0], &vTemporalTractObserved[0],
                                         DataHub.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
          TractTimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
          Record.GetFieldValue(TIME_TREND_FIELD).AsDouble() = (TractTimeTrend.IsNegative() ? -1 : 1) * TractTimeTrend.GetAnnualTimeTrend();
          if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
          if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RecordRelativeRiskData(const CSVTTData&)","RelativeRiskData");
    throw;
  }
}
