//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterLocationsWriter.h"
#include "cluster.h"

const char * LocationInformationWriter::AREA_SPECIFIC_FILE_EXT    = ".gis";
const char * LocationInformationWriter::LOC_OBS_FIELD             = "LOC_OBS";
const char * LocationInformationWriter::LOC_EXP_FIELD             = "LOC_EXP";
const char * LocationInformationWriter::LOC_MEAN_FIELD            = "LOC_MEAN";
const char * LocationInformationWriter::LOC_OBS_DIV_EXP_FIELD     = "LOC_ODE";
const char * LocationInformationWriter::LOC_REL_RISK_FIELD        = "LOC_RISK";
const char * LocationInformationWriter::CLU_OBS_FIELD             = "CLU_OBS";
const char * LocationInformationWriter::CLU_EXP_FIELD             = "CLU_EXP";
const char * LocationInformationWriter::CLU_OBS_DIV_EXP_FIELD     = "CLU_ODE";
const char * LocationInformationWriter::CLU_REL_RISK_FIELD        = "CLU_RISK";
const char * LocationInformationWriter::CLU_MEAN_IN_FIELD         = "CLU_MEAN_I";
const char * LocationInformationWriter::CLU_MEAN_OUT_FIELD        = "CLU_MEAN_O";

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
  catch (ZdException &x) {
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("constructor","LocationInformationWriter");
    throw;
  }
}

/** class destructor */
LocationInformationWriter::~LocationInformationWriter() {}

// sets up the vector of field structs so that the ZdField Vector can be created
void LocationInformationWriter::DefineFields(const CSaTScanData& DataHub) {
  unsigned short uwOffset = 0;

  try {
    CreateField(vFieldDefinitions, LOC_ID_FIELD, ZD_ALPHA_FLD, GetLocationIdentiferFieldLength(DataHub), 0, uwOffset);
    CreateField(vFieldDefinitions, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);

    if (!gbExcludePValueField)
      CreateField(vFieldDefinitions, P_VALUE_FLD, ZD_NUMBER_FLD, 19, 5, uwOffset);

    //defined cluster level fields to report -- none of these are reported
    // for multiple data sets nor the ordinal probability model
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      CreateField(vFieldDefinitions, CLU_OBS_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      if (gParameters.GetProbabilityModelType() == NORMAL) {
        CreateField(vFieldDefinitions, CLU_MEAN_IN_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
        CreateField(vFieldDefinitions, CLU_MEAN_OUT_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
      }
      if (gParameters.GetProbabilityModelType() != NORMAL) {
        CreateField(vFieldDefinitions, CLU_EXP_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
        CreateField(vFieldDefinitions, CLU_OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
      }
      if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(vFieldDefinitions, CLU_REL_RISK_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    }
    //defined location level fields to report -- none of these are reported
    // for multiple data sets nor the ordinal probability model
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      //these fields will no be supplied for analyses with more than one dataset
      CreateField(vFieldDefinitions, LOC_OBS_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      if (gParameters.GetProbabilityModelType() == NORMAL)
        CreateField(vFieldDefinitions, LOC_MEAN_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
      if (gParameters.GetProbabilityModelType() != NORMAL) {
        CreateField(vFieldDefinitions, LOC_EXP_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
        CreateField(vFieldDefinitions, LOC_OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
      }  
      if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(vFieldDefinitions, LOC_REL_RISK_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","LocationInformationWriter");
    throw;
  }
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void LocationInformationWriter::Write(const CCluster& theCluster, const CSaTScanData& DataHub, int iClusterNumber, tract_t tTract, unsigned int iNumSimsCompleted) {
  ZdString                                      sTempValue;
  std::string                                   sBuffer;
  TractHandler::Location::StringContainer_t     vIdentifiers;
  double                                        dRelativeRisk;
  RecordBuffer                                  Record(vFieldDefinitions);
  const DataSetHandler                        & Handler = DataHub.GetDataSetHandler();

  try {
    //do not report locations for which iterative scan has nullified this locations data
    if (DataHub.GetIsNullifiedLocation(tTract)) return;

    DataHub.GetTInfo()->getLocations().at(tTract)->retrieveAllIdentifiers(vIdentifiers);
    for (unsigned int i=0; i < vIdentifiers.size(); ++i) {
       Record.SetAllFieldsBlank(true);
       Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = vIdentifiers[i].c_str();
       if (Record.GetFieldValue(LOC_ID_FIELD).AsZdString().GetLength() > (unsigned long)Record.GetFieldDefinition(LOC_ID_FIELD).GetLength())
         Record.GetFieldValue(LOC_ID_FIELD).AsZdString().Truncate(Record.GetFieldDefinition(LOC_ID_FIELD).GetLength());
       Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
       if (!gbExcludePValueField)
         Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
       //location information fields are only present for one dataset and not ordinal model
       if (Handler.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
         //When there is more than one identifiers for a tract, this indicates
         //that locations where combined. Print a record for each location but
         //leave area specific information blank.
         if (vIdentifiers.size() == 1) {
           Record.GetFieldValue(LOC_OBS_FIELD).AsDouble() = theCluster.GetObservedCountForTract(tTract, DataHub);
           if (gParameters.GetProbabilityModelType() == NORMAL) {
             count_t tObserved = theCluster.GetObservedCountForTract(tTract, DataHub);
             if (tObserved) Record.GetFieldValue(LOC_MEAN_FIELD).AsDouble() = theCluster.GetExpectedCountForTract(tTract, DataHub)/tObserved;
           }
           if (gParameters.GetProbabilityModelType() != NORMAL) {
             Record.GetFieldValue(LOC_EXP_FIELD).AsDouble() = theCluster.GetExpectedCountForTract(tTract, DataHub);
             Record.GetFieldValue(LOC_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpectedForTract(tTract, DataHub);
           }
           if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
               (dRelativeRisk = theCluster.GetRelativeRiskForTract(tTract, DataHub)) != -1)
               Record.GetFieldValue(LOC_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
         }
       }
       //cluster information fields are only present for one dataset and not ordinal model
       if (Handler.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
         Record.GetFieldValue(CLU_OBS_FIELD).AsDouble() = theCluster.GetObservedCount();
         if (gParameters.GetProbabilityModelType() == NORMAL) {
           count_t tObserved = theCluster.GetObservedCount();
           measure_t tExpected = theCluster.GetExpectedCount(DataHub);
           if (tObserved) Record.GetFieldValue(CLU_MEAN_IN_FIELD).AsDouble() = tExpected/tObserved;
           count_t tCasesOutside = DataHub.GetDataSetHandler().GetDataSet().GetTotalCases() - tObserved;
           if (tCasesOutside) Record.GetFieldValue(CLU_MEAN_OUT_FIELD).AsDouble() = (Handler.GetDataSet().GetTotalMeasure() - tExpected)/tCasesOutside;

         }
         if (gParameters.GetProbabilityModelType() != NORMAL) {
           Record.GetFieldValue(CLU_EXP_FIELD).AsDouble() = theCluster.GetExpectedCount(DataHub);
           Record.GetFieldValue(CLU_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpected(DataHub);
         }
         if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
             (dRelativeRisk = theCluster.GetRelativeRisk(DataHub)) != -1)
            Record.GetFieldValue(CLU_REL_RISK_FIELD).AsDouble() = dRelativeRisk;
       }

       if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
       if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","LocationInformationWriter");
    throw;
  }
}

