//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsAreaSpecificData.h"
#include "cluster.h"

/** class constructor */
stsAreaSpecificData::stsAreaSpecificData(const CParameters& Parameters, bool bExcludePValueField)
                    :BaseOutputStorageClass(), gParameters(Parameters), gbExcludePValueField(bExcludePValueField) {
  try {
    SetupFields();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor","stsAreaSpecificData");
    throw;
  }
}

/** class destructor */
stsAreaSpecificData::~stsAreaSpecificData() {}

const char * stsAreaSpecificData::AREA_SPECIFIC_FILE_EXT    = ".gis";
const char * stsAreaSpecificData::AREA_OBS_FIELD            = "AREA_OBS";
const char * stsAreaSpecificData::AREA_EXP_FIELD            = "AREA_EXP";
const char * stsAreaSpecificData::AREA_RSK_FIELD            = "AREA_RSK";
const char * stsAreaSpecificData::CLU_OBS_FIELD             = "CLU_OBS";
const char * stsAreaSpecificData::CLU_EXP_FIELD             = "CLU_EXP";

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsAreaSpecificData::RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, tract_t tTract, unsigned int iNumSimsCompleted) {
  ZdString                     sTempValue;
  std::string                  sBuffer;
  OutputRecord               * pRecord = 0;
  std::vector<std::string>     vIdentifiers;
  size_t                       t;

  try {
    theData.GetTInfo()->tiGetTractIdentifiers(tTract, vIdentifiers);
    for (t=0; t < vIdentifiers.size(); ++t) {
       pRecord = new OutputRecord(gvFields);
       pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = vIdentifiers[t].c_str();
       pRecord->GetFieldValue(GetFieldNumber(CLUST_NUM_FIELD)).AsDouble() = iClusterNumber;
       if (!gbExcludePValueField)
         pRecord->GetFieldValue(GetFieldNumber(P_VALUE_FLD)).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
       //area and location information fields are only present for one data stream
       if (gParameters.GetNumDataStreams() == 1) {
         //When there is more than one identifiers for a tract, this indicates
         //that locations where combined. Print a record for each location but
         //leave area specific information blank.
         if (vIdentifiers.size() == 1) {
           pRecord->GetFieldValue(GetFieldNumber(AREA_OBS_FIELD)).AsDouble() = theCluster.GetCaseCountForTract(tTract, theData);
           pRecord->GetFieldValue(GetFieldNumber(AREA_EXP_FIELD)).AsDouble() = theCluster.GetMeasureForTract(tTract, theData);
           pRecord->GetFieldValue(GetFieldNumber(AREA_RSK_FIELD)).AsDouble() = theCluster.GetRelativeRiskForTract(tTract, theData);
         }
         else {
           pRecord->SetFieldIsBlank(GetFieldNumber(AREA_OBS_FIELD), true);
           pRecord->SetFieldIsBlank(GetFieldNumber(AREA_EXP_FIELD), true);
           pRecord->SetFieldIsBlank(GetFieldNumber(AREA_RSK_FIELD), true);
         }
         pRecord->GetFieldValue(GetFieldNumber(CLU_OBS_FIELD)).AsDouble() = theCluster.GetCaseCount(0);
         pRecord->GetFieldValue(GetFieldNumber(CLU_EXP_FIELD)).AsDouble() = theData.GetMeasureAdjustment(0) * theCluster.GetMeasure(0);
         pRecord->GetFieldValue(GetFieldNumber(REL_RISK_FIELD)).AsDouble() = theCluster.GetRelativeRisk(theData.GetMeasureAdjustment(0));
       }
       BaseOutputStorageClass::AddRecord(pRecord);
    }
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordClusterData()","stsAreaSpecificData");
    throw;
  }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: 
// post : returns through reference a vector of ZdFields to be used
//        to create the ZdVector of ZdField* required to create the DBF file
void stsAreaSpecificData::SetupFields() {
  unsigned short uwOffset = 0;     // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002

  try {
    CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(gvFields, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    if (gParameters.GetNumDataStreams() == 1) {
      //these fields will no be supplied for analyses with more than one stream
      CreateField(gvFields, CLU_OBS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateField(gvFields, CLU_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateField(gvFields, REL_RISK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    }
    if (!gbExcludePValueField)
      CreateField(gvFields, P_VALUE_FLD, ZD_NUMBER_FLD, 12, 5, uwOffset);
    if (gParameters.GetNumDataStreams() == 1) {
      //these fields will no be supplied for analyses with more than one stream
      CreateField(gvFields, AREA_OBS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateField(gvFields, AREA_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateField(gvFields, AREA_RSK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupFields()","stsAreaSpecificData");
    throw;
  }
}
