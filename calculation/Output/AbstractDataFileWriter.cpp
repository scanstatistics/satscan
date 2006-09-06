//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "SaTScanData.h"

/** class constructor */
RecordBuffer::RecordBuffer(const ZdPointerVector<ZdField>& vFields) : vFieldDefinitions(vFields) {
  for (size_t t=0; t < vFieldDefinitions.size(); ++t) {
     gvFieldValues.push_back(ZdFieldValue(vFieldDefinitions[t]->GetType()));
     gvBlankFields.push_back(true);
  }
}

/** class destructor */
RecordBuffer::~RecordBuffer() {}

/** ZdField definition for field with name. */
const ZdField & RecordBuffer::GetFieldDefinition(const ZdString& sFieldName) const {
  return *vFieldDefinitions[GetFieldIndex(sFieldName)];
}

/** ZdField definition for field at index. */
const ZdField & RecordBuffer::GetFieldDefinition(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= vFieldDefinitions.size())
      ZdGenerateException("Index %u out of range [size=%u].", "GetFieldDefinition()", iFieldIndex, vFieldDefinitions.size());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldDefinition()","RecordBuffer");
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
      ZdGenerateException("Index %u out of range [size=%u].", "GetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldIsBlank()","RecordBuffer");
    throw;
  }
  return gvBlankFields[iFieldNumber];
}

/** Returns field index for named field. */
unsigned int RecordBuffer::GetFieldIndex(const ZdString& sFieldName) const {
  bool                 bFound(false);
  unsigned int         i, iPosition;

  try {
    for (i=0; i < vFieldDefinitions.size() && !bFound; ++i) {
       bFound = (!strcmp(vFieldDefinitions[i]->GetName(), sFieldName));
       iPosition = i;
   }
   if (!bFound)
     ZdException::GenerateNotification("Field name %s not found in the field vector.", "GetFieldIndex()", sFieldName.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldIndex()","RecordBuffer");
    throw;
  }
  return iPosition;
}

/** Returns reference to field value for named field, setting field 'blank' indicator to false. */
ZdFieldValue& RecordBuffer::GetFieldValue(const ZdString& sFieldName) {
  try {
    unsigned int iFieldIndex = GetFieldIndex(sFieldName);
    gvBlankFields[iFieldIndex] = false;
    return gvFieldValues[iFieldIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
ZdFieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
    gvBlankFields[iFieldIndex] = false;
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
const ZdFieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Sets all blank indicators as not blank. */
void RecordBuffer::SetAllFieldsBlank(bool bBlank) {
  std::fill(gvBlankFields.begin(), gvBlankFields.end(), bBlank);
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(const ZdString& sFieldName, bool bBlank) {
  try {
    gvBlankFields[GetFieldIndex(sFieldName)] = bBlank;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank) {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      ZdGenerateException("Index %u out of range [size=%u].", "SetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

const char * AbstractDataFileWriter::CLUST_NUM_FIELD                    = "CLUSTER";
const char * AbstractDataFileWriter::LOC_ID_FIELD   	                = "LOC_ID";
const char * AbstractDataFileWriter::P_VALUE_FLD  	                = "P_VALUE";
const char * AbstractDataFileWriter::OBSERVED_FIELD	                = "OBSERVED";
const char * AbstractDataFileWriter::EXPECTED_FIELD	                = "EXPECTED";
const char * AbstractDataFileWriter::LOG_LIKL_RATIO_FIELD               = "LLR";
const char * AbstractDataFileWriter::TST_STAT_FIELD                     = "TEST_STAT";
const char * AbstractDataFileWriter::DATASET_FIELD                      = "DATASET";
const char * AbstractDataFileWriter::CATEGORY_FIELD                     = "CATEGORY";
const char * AbstractDataFileWriter::OBSERVED_DIV_EXPECTED_FIELD        = "ODE";
const char * AbstractDataFileWriter::RELATIVE_RISK_FIELD                = "REL_RISK";
const char * AbstractDataFileWriter::MEAN_INSIDE_FIELD                  = "MEAN_IN";
const char * AbstractDataFileWriter::MEAN_OUTSIDE_FIELD                 = "MEAN_OUT";
const char * AbstractDataFileWriter::VARIANCE_FIELD                     = "VARIANCE";
const char * AbstractDataFileWriter::DEVIATION_FIELD                    = "DEVIATION";
const char * AbstractDataFileWriter::MEAN_VALUE_FIELD                   = "MEAN";
const size_t AbstractDataFileWriter::DEFAULT_LOC_FIELD_SIZE             = 30;
const size_t AbstractDataFileWriter::MAX_LOC_FIELD_SIZE                 = 254;

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
void AbstractDataFileWriter::CreateField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, char cType,
                                         short wLength, short wPrecision, unsigned short& uwOffset, bool bCreateIndex) {
  try {
    vFields.push_back(new ZdField);
    vFields.back()->SetName(sFieldName.c_str());
    vFields.back()->SetType(cType);
    vFields.back()->SetLength(wLength);
    vFields.back()->SetPrecision(wPrecision);
    vFields.back()->SetOffset(uwOffset);
    uwOffset += wLength;
    if (bCreateIndex) vFields.back()->SetIndexCount(1);
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateField()","AbstractDataFileWriter");
    throw;
  }
}

/** Returns field length for location identifers fields. */
size_t AbstractDataFileWriter::GetLocationIdentiferFieldLength(const CSaTScanData& DataHub) const {
  const TractHandler* pHandler = DataHub.GetTInfo();

  return std::max(DEFAULT_LOC_FIELD_SIZE, std::min(MAX_LOC_FIELD_SIZE, pHandler->getMaxIdentifierLength()));
}

