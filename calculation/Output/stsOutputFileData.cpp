//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsOutputFileData.h"

/** class constructor */
OutputRecord::OutputRecord(const ZdPointerVector<ZdField>& vFields) {
  try {
    Setup(vFields);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","OutputRecord");
    throw;
  }
}

/** class destructor */
OutputRecord::~OutputRecord() {}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool OutputRecord::GetFieldIsBlank(unsigned int iFieldNumber) const {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      ZdGenerateException("Invalid index, out of range!", "GetFieldIsBlank()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldIsBlank()","OutputRecord");
    throw;
  }
  return gvBlankFields[iFieldNumber];
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
ZdFieldValue& OutputRecord::GetFieldValue(unsigned int iFieldIndex) {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","OutputRecord");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
const ZdFieldValue& OutputRecord::GetFieldValue(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","OutputRecord");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Creates vector of field values that mirror defined fields of passed
    vector. Initializes list of booleans that reflect whether a field
    contains data or not. */
void OutputRecord::Setup(const ZdPointerVector<ZdField>& vFields) {
  size_t        t;

  try {
    for (t=0; t < vFields.size(); ++t) {
       gvFieldValues.push_back(ZdFieldValue(vFields[t]->GetType()));
       gvBlankFields.push_back(false);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","OutputRecord");
    throw;
  }
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void OutputRecord::SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank) {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      ZdGenerateException("Invalid index, out of range!", "SetFieldIsBlank()");
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetFieldIsBlank()","OutputRecord");
    throw;
  }
}

//===============================================================================
// This class is the base storage class for each of the output file types.
// These classes are primarily concerned with the storage of the data needed
// to create the output files. These files, however, are not responsible for
// the printing of that information, they are for storage only.
//===============================================================================

BaseOutputStorageClass::BaseOutputStorageClass() {}

BaseOutputStorageClass::~BaseOutputStorageClass() {}

const char * BaseOutputStorageClass::CLUST_NUM_FIELD 	  = "CLUST_NUM";
const char * BaseOutputStorageClass::LOC_ID_FIELD   	  = "LOC_ID";
const char * BaseOutputStorageClass::P_VALUE_FLD  	  = "P_VALUE";
const char * BaseOutputStorageClass::OBSERVED_FIELD	  = "OBSERVED";
const char * BaseOutputStorageClass::EXPECTED_FIELD	  = "EXPECTED";
const char * BaseOutputStorageClass::REL_RISK_FIELD	  = "REL_RISK";
const char * BaseOutputStorageClass::LOG_LIKL_FIELD	  = "LOG_LIKL";
const char * BaseOutputStorageClass::TST_STAT_FIELD       = "TST_STAT";

void BaseOutputStorageClass::AddRecord(const OutputRecord* pRecord) {
   gvRecordBuffers.push_back(pRecord);
}

// allocates a new field and adds it to the vector
// pre : none
// post : a field is added to the pointer vector with appropraite specs
void BaseOutputStorageClass::CreateField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName,
                                         char cType, short wLength, short wPrecision,
                                         unsigned short& uwOffset, bool bCreateIndex) {
  ZdField  *pField = 0;

  try {
    pField = new ZdField;
    pField->SetName(sFieldName.c_str());
    pField->SetType(cType);
    pField->SetLength(wLength);
    pField->SetPrecision(wPrecision);
    pField->SetOffset(uwOffset);
    uwOffset += wLength;
    if (bCreateIndex)
      pField->SetIndexCount(1);
    vFields.push_back(pField);
  }
  catch (ZdException &x) {
    delete pField; pField = 0;
    x.AddCallpath("CreateField()","BaseOutputStorageClass");
    throw;
  }			
}

/** Returns a pointer to the uwFieldNumber'th field in the global vector. Throws
    exception when iFieldNumber it out of range. */
const ZdField * BaseOutputStorageClass::GetField(unsigned int iFieldNumber) const {
  try {
    if (iFieldNumber >= gvFields.GetNumElements())
      ZdGenerateException("Invalid index, out of range!", "GetField()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetField()","BaseOutputStorageClass");
    throw;
  }
  return gvFields[iFieldNumber];
}

// function to get the field number of a field given its name
// pre: none
// post: if field with given name exists in the vector then will return the position, else will
//       throw a not found exception
unsigned int BaseOutputStorageClass::GetFieldNumber(const ZdString& sFieldName) const {
  bool                 bFound(false);
  unsigned int         i, iPosition;

  try {
    for (i=0; i < gvFields.size() && !bFound; ++i) {
       bFound = (!strcmp(gvFields[i]->GetName(), sFieldName));
       iPosition = i;
   }
   if (!bFound)
     ZdException::GenerateNotification("Field name %s not found in the field vector.",
                                       "GetFieldNumber()", sFieldName.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldNumber()","BaseOutputStorageClass");
    throw;
  }
  return iPosition;
}

// returns a pointer to the record at iPosition in the global vector
// pre : none
// post : if iPosition not in valid index range then exception, else returns
//        a pointer to the iPosition element in the vector
const OutputRecord* BaseOutputStorageClass::GetRecord(int iPosition) const {
   try {
      if (iPosition < 0 || iPosition >= (int)gvRecordBuffers.size())
         ZdGenerateException ("Invalid index, out of range", "Error!");   
   }
   catch (ZdException &x) {
      x.AddCallpath("GetRecord()", "BaseOutputStorageClass");
      throw;
   }
   return gvRecordBuffers[iPosition];
}

