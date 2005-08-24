//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterLocationsWriter.h"
#include "cluster.h"

const char * LocationInformationWriter::AREA_SPECIFIC_FILE_EXT    = ".gis";
const char * LocationInformationWriter::LOC_OBS_FIELD             = "LOC_OBS";
const char * LocationInformationWriter::LOC_EXP_FIELD             = "LOC_EXP";
const char * LocationInformationWriter::LOC_OBS_DIV_EXP_FIELD     = "LOC_ODE";
const char * LocationInformationWriter::LOC_REL_RISK_FIELD        = "LOC_RISK";
const char * LocationInformationWriter::CLU_OBS_FIELD             = "CLU_OBS";
const char * LocationInformationWriter::CLU_EXP_FIELD             = "CLU_EXP";
const char * LocationInformationWriter::CLU_OBS_DIV_EXP_FIELD     = "CLU_ODE";
const char * LocationInformationWriter::CLU_REL_RISK_FIELD        = "CLU_RISK";

/** class constructor */
LocationInformationWriter::LocationInformationWriter(const CParameters& Parameters, bool bExcludePValueField, bool bAppend)
                          :AbstractDataFileWriter(Parameters), gbExcludePValueField(bExcludePValueField) {
  try {
    DefineFields();
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
// pre: 
// post : returns through reference a vector of ZdFields to be used
//        to create the ZdVector of ZdField* required to create the DBF file
void LocationInformationWriter::DefineFields() {
  unsigned short uwOffset = 0;

  try {
    CreateField(LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    if (!gbExcludePValueField)
      CreateField(P_VALUE_FLD, ZD_NUMBER_FLD, 19, 5, uwOffset);
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      //these fields will no be supplied for analyses with more than one dataset
      CreateField(CLU_OBS_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(CLU_EXP_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
      CreateField(CLU_OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
      if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(CLU_REL_RISK_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    }
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      //these fields will no be supplied for analyses with more than one dataset
      CreateField(LOC_OBS_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(LOC_EXP_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
      CreateField(LOC_OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
      if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(LOC_REL_RISK_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
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
  ZdString                     sTempValue;
  std::string                  sBuffer;
  std::vector<std::string>     vIdentifiers;
  size_t                       t;
  RecordBuffer                 Record(vFieldDefinitions);

  try {
    DataHub.GetTInfo()->tiGetTractIdentifiers(tTract, vIdentifiers);
    for (t=0; t < vIdentifiers.size(); ++t) {
       Record.SetAllFieldsNotBlank();
       Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = vIdentifiers[t].c_str();
       Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
       if (!gbExcludePValueField)
         Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
       //area and location information fields are only present for one dataset
       if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
         //When there is more than one identifiers for a tract, this indicates
         //that locations where combined. Print a record for each location but
         //leave area specific information blank.
         if (vIdentifiers.size() == 1) {
           Record.GetFieldValue(LOC_OBS_FIELD).AsDouble() = theCluster.GetObservedCountForTract(tTract, DataHub);
           Record.GetFieldValue(LOC_EXP_FIELD).AsDouble() = theCluster.GetExpectedCountForTract(tTract, DataHub);
           Record.GetFieldValue(LOC_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpectedForTract(tTract, DataHub);
           if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
             Record.GetFieldValue(LOC_REL_RISK_FIELD).AsDouble() = theCluster.GetRelativeRiskForTract(tTract, DataHub);
         }
         else {
           Record.SetFieldIsBlank(LOC_OBS_FIELD, true);
           Record.SetFieldIsBlank(LOC_EXP_FIELD, true);
           Record.SetFieldIsBlank(LOC_OBS_DIV_EXP_FIELD, true);
           if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
             Record.SetFieldIsBlank(LOC_REL_RISK_FIELD, true);
         }
         Record.GetFieldValue(CLU_OBS_FIELD).AsDouble() = theCluster.GetObservedCount();
         Record.GetFieldValue(CLU_EXP_FIELD).AsDouble() = theCluster.GetExpectedCount(DataHub);
         Record.GetFieldValue(CLU_OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpected(DataHub);
         if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
           Record.GetFieldValue(CLU_REL_RISK_FIELD).AsDouble() = theCluster.GetRelativeRisk(DataHub);
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

