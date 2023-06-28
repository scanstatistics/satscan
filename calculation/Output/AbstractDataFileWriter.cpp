//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "SaTScanData.h"

/** class constructor */
RecordBuffer::RecordBuffer(const ptr_vector<FieldDef>& vFields) : vFieldDefinitions(vFields) {
  for (size_t t=0; t < vFieldDefinitions.size(); ++t) {
     gvFieldValues.push_back(FieldValue(vFieldDefinitions[t]->GetType()));
     gvBlankFields.push_back(true);
  }
}

/** class destructor */
RecordBuffer::~RecordBuffer() {}

/** FieldDef definition for field with name. */
const FieldDef & RecordBuffer::GetFieldDefinition(const std::string& sFieldName) const {
  return *vFieldDefinitions[GetFieldIndex(sFieldName)];
}

/** FieldDef definition for field at index. */
const FieldDef & RecordBuffer::GetFieldDefinition(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= vFieldDefinitions.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldDefinition()", iFieldIndex, vFieldDefinitions.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldDefinition()","RecordBuffer");
    throw;
  }
  return *vFieldDefinitions[iFieldIndex];
}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool RecordBuffer::GetFieldIsBlank(unsigned int iFieldNumber) const {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldIsBlank()","RecordBuffer");
    throw;
  }
  return gvBlankFields[iFieldNumber];
}

/** Returns field index for named field. */
unsigned int RecordBuffer::GetFieldIndex(const std::string& sFieldName) const {
  bool                 bFound(false);
  unsigned int         i, iPosition;

  try {
    for (i=0; i < vFieldDefinitions.size() && !bFound; ++i) {
       bFound = (!strcmp(vFieldDefinitions[i]->GetName(), sFieldName.c_str()));
       iPosition = i;
   }
   if (!bFound)
     throw prg_error("Field name %s not found in the field vector.", "GetFieldIndex()", sFieldName.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldIndex()","RecordBuffer");
    throw;
  }
  return iPosition;
}

/** Returns reference to field value for named field, setting field 'blank' indicator to false. */
FieldValue& RecordBuffer::GetFieldValue(const std::string& sFieldName) {
  try {
    unsigned int iFieldIndex = GetFieldIndex(sFieldName);
    gvBlankFields[iFieldIndex] = false;
    return gvFieldValues[iFieldIndex];
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
}

/** Returns FieldValue reference for field index. Throws prg_error if
    iFieldIndex is greater than number of FieldValues. */
FieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
    gvBlankFields[iFieldIndex] = false;
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Returns FieldValue reference for field index. Throws prj_error if
    iFieldIndex is greater than number of FieldValues. */
const FieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Sets all blank indicators as not blank. */
void RecordBuffer::SetAllFieldsBlank(bool bBlank) {
  std::fill(gvBlankFields.begin(), gvBlankFields.end(), bBlank);
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(const std::string& sFieldName, bool bBlank) {
  try {
    gvBlankFields[GetFieldIndex(sFieldName)] = bBlank;
  }
  catch (prg_exception& x) {
    x.addTrace("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank) {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      throw prg_error("Index %u out of range [size=%u].", "SetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (prg_exception& x) {
    x.addTrace("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

const char * AbstractDataFileWriter::CLUST_NUM_FIELD                    = "CLUSTER";
const char * AbstractDataFileWriter::LOC_ID_FIELD   	                = "LOC_ID";
const char * AbstractDataFileWriter::P_VALUE_FLD  	                    = "P_VALUE";
const char * AbstractDataFileWriter::GUMBEL_P_VALUE_FLD                 = "GBL_PVALUE";
const char * AbstractDataFileWriter::OBSERVED_FIELD	                    = "OBSERVED";
const char * AbstractDataFileWriter::EXPECTED_FIELD	                    = "EXPECTED";
const char * AbstractDataFileWriter::LOG_LIKL_RATIO_FIELD               = "LLR";
const char * AbstractDataFileWriter::TST_STAT_FIELD                     = "TEST_STAT";
const char * AbstractDataFileWriter::DATASET_FIELD                      = "DATASET";
const char * AbstractDataFileWriter::CATEGORY_FIELD                     = "CATEGORY";
const char * AbstractDataFileWriter::OBSERVED_DIV_EXPECTED_FIELD        = "ODE";
const char * AbstractDataFileWriter::RELATIVE_RISK_FIELD                = "REL_RISK";
const char * AbstractDataFileWriter::MEAN_INSIDE_FIELD                  = "MEAN_IN";
const char * AbstractDataFileWriter::MEAN_OUTSIDE_FIELD                 = "MEAN_OUT";
const char * AbstractDataFileWriter::VARIANCE_FIELD                     = "VARIANCE";
const char * AbstractDataFileWriter::STD_FIELD                          = "STD";
const char * AbstractDataFileWriter::MEAN_VALUE_FIELD                   = "MEAN";
const short AbstractDataFileWriter::DEFAULT_LOC_FIELD_SIZE              = 30;
const short AbstractDataFileWriter::MAX_LOC_FIELD_SIZE                  = 254;
const char * AbstractDataFileWriter::RECURRENCE_INTERVAL_FLD            = "RECURR_INT";
const char * AbstractDataFileWriter::PERCENTAGE_CASES_FIELD             = "PCT_CASES";
const char * AbstractDataFileWriter::POPULATION_FIELD                   = "POPULATION";

/** constructor */
AbstractDataFileWriter::AbstractDataFileWriter(const CParameters& Parameters)
                       :gParameters(Parameters), gpASCIIFileWriter(0), gpDBaseFileWriter(0) {}

/** destructor */
AbstractDataFileWriter::~AbstractDataFileWriter() {
  try {
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
  }
  catch (...){}  
}

/** Defines field definition and assigns to accumulation. */
void AbstractDataFileWriter::CreateField(ptr_vector<FieldDef>& vFields, const std::string& sFieldName, char cType,
                                         short wLength, short wPrecision, unsigned short& uwOffset, 
                                         unsigned short uwAsciiDecimals, bool bCreateIndex) {
  try {
    vFields.push_back(new FieldDef(sFieldName.c_str(), cType, wLength, wPrecision, uwOffset, uwAsciiDecimals));
    uwOffset += wLength;
    //if (bCreateIndex) vFields.back()->SetIndexCount(1);
  }
  catch (prg_exception& x) {
    x.addTrace("CreateField()","AbstractDataFileWriter");
    throw;
  }
}

/** Returns field length for location identifers fields. */
short AbstractDataFileWriter::GetLocationIdentiferFieldLength(const CSaTScanData& DataHub) const {
  return std::max(DEFAULT_LOC_FIELD_SIZE, std::min(MAX_LOC_FIELD_SIZE, static_cast<short>(DataHub.getLocationsManager().getMaxNameLength())));
}

