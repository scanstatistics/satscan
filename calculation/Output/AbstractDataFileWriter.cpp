//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractDataFileWriter.h"

/** class constructor */
RecordBuffer::RecordBuffer(const ZdPointerVector<ZdField>& vFields) : vFieldDefinitions(vFields) {
  for (size_t t=0; t < vFieldDefinitions.size(); ++t) {
     gvFieldValues.push_back(ZdFieldValue(vFieldDefinitions[t]->GetType()));
     gvBlankFields.push_back(false);
  }
}

/** class destructor */
RecordBuffer::~RecordBuffer() {}

/** ZdField definition for field at index. */
const ZdField & RecordBuffer::GetFieldDefinition(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= vFieldDefinitions.size())
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
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
      ZdGenerateException("Invalid index, out of range!", "GetFieldIsBlank()");
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

/** Returns field value for named field. */
ZdFieldValue& RecordBuffer::GetFieldValue(const ZdString& sFieldName) {
  try {
    return gvFieldValues[GetFieldIndex(sFieldName)];
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
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
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
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
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
      ZdGenerateException("Invalid index, out of range!", "SetFieldIsBlank()");
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}


const char * AbstractDataFileWriter::CLUST_NUM_FIELD 	  = "CLUSTER";
const char * AbstractDataFileWriter::LOC_ID_FIELD   	  = "LOC_ID";
const char * AbstractDataFileWriter::P_VALUE_FLD  	  = "P_VALUE";
const char * AbstractDataFileWriter::OBSERVED_FIELD	  = "OBSERVED";
const char * AbstractDataFileWriter::EXPECTED_FIELD	  = "EXPECTED";
const char * AbstractDataFileWriter::LOG_LIKL_RATIO_FIELD = "LLR";
const char * AbstractDataFileWriter::TST_STAT_FIELD       = "TST_STAT";

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
void AbstractDataFileWriter::CreateField(const std::string& sFieldName, char cType, short wLength,
                                         short wPrecision, unsigned short& uwOffset, bool bCreateIndex) {
  try {
    vFieldDefinitions.push_back(new ZdField);
    vFieldDefinitions.back()->SetName(sFieldName.c_str());
    vFieldDefinitions.back()->SetType(cType);
    vFieldDefinitions.back()->SetLength(wLength);
    vFieldDefinitions.back()->SetPrecision(wPrecision);
    vFieldDefinitions.back()->SetOffset(uwOffset);
    uwOffset += wLength;
    if (bCreateIndex) vFieldDefinitions.back()->SetIndexCount(1);
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateField()","AbstractDataFileWriter");
    throw;
  }
}

