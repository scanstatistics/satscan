//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
//#include "xbase/xbase.h"
#include "dBaseFile.h"
#include "UtilityFunctions.h"

//ClassDesc Begin dBaseRecord
// The dBaseRecord implements dBaseRecord for dBase (ver III+, IV) table files.  It
// does so by using the functionality of the xbDbf object with which it is associated.
// Each access/manipulation function (i.e. GetAlpha, PutAlpha) follows a pattern:
//<br>1. Copy the xbDbf::RecBuf data into gTempBuffer.
//<br>2. Copy the gBuffer data into the xbDbf::RecBuf.
//<br>3. Call the xbDbf function to get the desired behavior (access or manipulation).
//<br>4. If the behavior changed the data in the record, copy the xbDbf::RecBuf data
// back into gBuffer.
//<br>5. Copy the gTempBuffer data back into the xbDbf::RecBuf.
//<br>6. Do final processing.
//<br> Steps 1, 2, 4, and 5 are identical for all access/manipulation functions, so
// three helper functions are provided.  BeginAccess() executes steps 1, 2.  EndAccess()
// executes step 5 only.  EndManipulation() executes steps 4, 5.
// As a further help, a class called, "dBaseRecord::AccessExpediter", executes BeginAccess()
// in its constructor, and executes either EndAccess() or EndManipulation() in its
// destructor.  By taking advantage of scoping rules, this class can be used to
// ensure that a call to BeginAccess() is eventually followed by exactly one call
// to either of the End... functions.  So you will see, in several of the access
// or manipulation functions, code of this form:
//<br>{
//<br>AccessExpediter ae(*this, true);
//<br>GetAssociatedDbf().some_function();
//<br>}
//<br>The construction of 'ae' executes a call to BeginAccess().  When 'ae' leaves
// scope at the end of the block, its destructor executes a call to either EndAccess()
// or (in this case, since 'true' was passed to the constructor) EndManipulation().

/** constructor */
dBaseRecord::dBaseRecord( dBaseFile & associatedFile, xbDbf & associatedDbf, const ptr_vector<FieldDef>& vFields)
             :gpAssociatedDbf(&associatedDbf), gpBufferChunk(0), gulBufferChunkSize(0),
              gpTempBufferChunk(0), gulTempBufferChunkSize(0) {
  try {
    std::string buffer;
    gAssociatedFileName.setFullPath(associatedFile.GetFileName(buffer));
    if (GetAssociatedDbf().GetDbfStatus() == XB_CLOSED)
      throw prg_error("The associated file is not open", "dBaseRecord");
    ResizeBuffers(associatedDbf.GetRecordLen());
    AccessExpediter ae(*this, true);
    GetAssociatedDbf().BlankRecord();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(dBaseFile &, xbDbf &)", "dBaseRecord");
    throw;
  }
}

/** destructor */
dBaseRecord::~dBaseRecord() {
  try {
    free(gpBufferChunk);
    free(gpTempBufferChunk);
  }
  catch(...){}
}

void dBaseRecord::ResizeChunk(unsigned long lNewSize, unsigned long& ulCurrentSize, unsigned char ** pChunk) {
  if (lNewSize == ulCurrentSize) return;
  if (lNewSize < 0)
    throw prg_error("lNewSize, value %d, must be non-negative.", "ResizeChunk", lNewSize);
  *pChunk = reinterpret_cast<unsigned char *> ( std::realloc(*pChunk, (size_t)lNewSize) );
   if (!*pChunk && lNewSize)
     throw std::bad_alloc();
   ulCurrentSize = lNewSize;
}

// Append the data in this record to 'theDbf'.
void dBaseRecord::AppendToDbf(xbDbf & theDbf) const {
  xbShort rc;
  try {
    AccessExpediter ae(*this);
    rc = theDbf.AppendRecord();
    if (rc != XB_NO_ERROR)
       throw prg_error("Could not append record to the file, \"%s\".  xbase error: \"%s\".", "dBaseRecord", theDbf.GetDbfName().c_str(), theDbf.xbase->GetErrorMessage(rc));
  }
  catch (prg_exception& x) {
    x.addTrace("AppendToDbf", "dBaseRecord");
    throw;
  }
}

// Throw an exception if 'code' != XB_NO_ERROR
void dBaseRecord::AssertNotXBaseError(xbShort code) const {
  if (code != XB_NO_ERROR)
    throw prg_error("XBase error: \"%s\".", "dBaseRecord", GetAssociatedDbf().xbase->GetErrorMessage(code));
}

// Throw an exception if gBuffer.GetSize() < 'lReqdSize'.
void dBaseRecord::AssertSufficientBufferSize(unsigned long ulReqdSize) const {
  if (gulBufferChunkSize < ulReqdSize)
    throw prg_error("Buffer size, %d, is less than req'd size, %d.", "dBaseRecord", gulBufferChunkSize, ulReqdSize);
}

// Manipulate buffers so that calls through GetAssociatedDbf operate on the data
// from *this.
void dBaseRecord::BeginAccess() const {
  BufferDbfRecordData(GetAssociatedDbf());
  UpdateDbfRecordData(GetAssociatedDbf());
}

// Copy the data from the RecBuf in the associated Dbf to gTempBuffer.
void dBaseRecord::BufferDbfRecordData(xbDbf & theDbf) const {
  long lBufferSize(theDbf.GetRecordLen());
  AssertSufficientBufferSize(lBufferSize);
  std::memcpy(gpTempBufferChunk, theDbf.GetRecordBuf(), lBufferSize);
}

// Copy the data from the RecBuf in the associated Dbf to gBuffer.
void dBaseRecord::CopyDbfRecordData(xbDbf & theDbf) {
  long lBufferSize(theDbf.GetRecordLen());
  AssertSufficientBufferSize(lBufferSize);
  std::memcpy(gpBufferChunk, theDbf.GetRecordBuf(), lBufferSize);
}

// Copy the data from gTempBuffer to the RecBuf in the associated Dbf.
void dBaseRecord::RestoreDbfRecordData(xbDbf & theDbf) const {
  long lBufferSize(theDbf.GetRecordLen());
  AssertSufficientBufferSize(lBufferSize);
  //GetRecordBuf returns the address to the actual memory, so this is correct:
  std::memcpy(theDbf.GetRecordBuf(), gpTempBufferChunk, lBufferSize);
}

// Copy the data from gBuffer to the RecBuf in the associated Dbf.
void dBaseRecord::UpdateDbfRecordData(xbDbf & theDbf) const {
  long lBufferSize(theDbf.GetRecordLen());
  AssertSufficientBufferSize(lBufferSize);
  //GetRecordBuf returns the address to the actual memory, so this is correct:
  std::memcpy(theDbf.GetRecordBuf(), gpBufferChunk, lBufferSize);
}

//Make all fields blank.
void dBaseRecord::Clear() {
  try {
    AccessExpediter ae(*this, true);
    GetAssociatedDbf().BlankRecord();
  }
  catch (prg_exception& x) {
    x.addTrace("Clear()", "dBaseRecord");
    throw;
  }
}

// Manipulate buffers so that the RecBuf of GetAssociatedDbf() contains the data it
// held before the previous call to BeginAccess().
void dBaseRecord::EndAccess() const {
   RestoreDbfRecordData(GetAssociatedDbf());
}

// Manipulate buffers so that gBuffer holds the data that the RecBuf of GetAssociatedDbf()
// holds and that RecBuf contains the data it held before the previous call to BeginAccess().
void dBaseRecord::EndManipulation() {
  CopyDbfRecordData(GetAssociatedDbf());
  RestoreDbfRecordData(GetAssociatedDbf());
}

// Check to make sure that gpAssociatedDbf is non-null and return its value.
xbDbf & dBaseRecord::GetAssociatedDbf() const {
  std::string  buffer;
  try {
    if (gpAssociatedDbf->GetDbfStatus() == XB_CLOSED) {
      throw prg_error("The file with which this record is associated, \"%c\", is not open.", "dBaseRecord", gAssociatedFileName.getFullPath(buffer).c_str());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetAssociatedDbf()", "dBaseRecord");
    throw;
  }
  return *gpAssociatedDbf;
}

//How many fields are in *this ?
long dBaseRecord::GetFieldCount() const {
  long lResult;
  try {
    lResult = GetAssociatedDbf().FieldCount();
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldCount()", "dBaseRecord");
    throw;
  }
  return lResult;
}

//How many bytes does this field require ?
short dBaseRecord::GetFieldLength(unsigned short uwFieldIndex) const {
  short wResult;
  try {
    wResult = GetAssociatedDbf().GetFieldLen(uwFieldIndex);
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldLength()", "dBaseRecord");
    throw;
  }
  return wResult;
}

//What is the "field type"  of the field at 'uwFieldIndex' ?
char dBaseRecord::GetFieldType(unsigned short uwFieldIndex) const {
  char cResult;
  try {
    cResult = dBaseFile::GetFieldDefTypeFromXBaseFieldType(GetAssociatedDbf().GetFieldType(uwFieldIndex));
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldType()", "dBaseRecord");
    throw;
  }
  return cResult;
}

//Does the field at 'uwFieldIndex' contain a "blank" value ?
bool dBaseRecord::GetIsBlank(unsigned short uwFieldIndex) const {
  bool bResult;
  const char * pBufBegin;
  const char * pBufEnd;
  unsigned char * pBufferChunk=0;
  unsigned long   ulBufferChunkSize=0;

  try {
    ResizeChunk(GetAssociatedDbf().GetFieldLen(uwFieldIndex)+1, ulBufferChunkSize, &pBufferChunk);
    {
     AccessExpediter ae(*this);
     GetAssociatedDbf().GetRawField(GetAssociatedDbf().GetFieldName(uwFieldIndex), (char*)pBufferChunk);
    }
    switch (GetAssociatedDbf().GetFieldType(uwFieldIndex)) {
      case XB_CHAR_FLD :
      case XB_NUMERIC_FLD :
      case XB_DATE_FLD :
      case XB_FLOAT_FLD : //all spaces?
          bResult = true;
          pBufBegin = reinterpret_cast<const char*>(pBufferChunk);
          pBufEnd = pBufBegin + (ulBufferChunkSize - 1);
          while ((pBufBegin < pBufEnd) && bResult) {
               bResult = *pBufBegin == ' ';
               ++pBufBegin;
          }
          break;
      case XB_LOGICAL_FLD : //question mark?
          bResult = ((char)pBufferChunk[0] == '?');
          break;
      case XB_MEMO_FLD : //empty memo?
          bResult = (GetAssociatedDbf().GetMemoFieldLen(uwFieldIndex) == 0);
          break;
    }
    free(pBufferChunk);
  }
  catch (prg_exception& x) {
    free(pBufferChunk);
    x.addTrace("GetIsBlank()", "dBaseRecord");
    throw;
  }
  return bResult;
}

//Get the value of the field at 'uwFieldIndex' as an Alpha.  'ulLength' is the number
//of bytes in 'pFieldValue'.
//<br>require
//<br>  valid_length: ulLength >= (GetFieldLength(uwFieldNumber) + 1)
char * dBaseRecord::GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long ulLength, bool bStripBlanks) const {
  short wFieldLength = GetFieldLength(uwFieldNumber);
  int i;
  unsigned char * pBufferChunk=0;
  unsigned long   ulBufferChunkSize=0;

  try {
    ResizeChunk(wFieldLength + 1, ulBufferChunkSize, &pBufferChunk);
    if (ulLength < (unsigned long)(wFieldLength + 1))
       throw prg_error("Buffer %d too small in GetAlpha(); need buffer length of %d", "GetAlpha", ulLength, wFieldLength + 1 );

    {
     AccessExpediter ae(*this);
     GetAssociatedDbf().GetRawField(GetAssociatedDbf().GetFieldName(uwFieldNumber), (char*)pBufferChunk);
    }

    std::memcpy(pFieldValue, pBufferChunk, wFieldLength);
    pFieldValue[wFieldLength] = 0;
    if (bStripBlanks)
      for (i = wFieldLength - 1; i >= 0 && pFieldValue[i] == ' '; i--)
         pFieldValue[i] = 0;
    free(pBufferChunk);
  }
  catch (prg_exception& x) {
    free(pBufferChunk);
    x.addTrace("GetAlpha()", "dBaseRecord");
    throw;
  }
  return pFieldValue;
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
SaTScan::Date& dBaseRecord::GetDate(unsigned short uwFieldNumber, SaTScan::Date& theDate) const {
  unsigned char * pBufferChunk=0;
  unsigned long   ulBufferChunkSize=0;

  try {
    AccessExpediter ae(*this);
    ResizeChunk(GetAssociatedDbf().GetFieldLen(uwFieldNumber)+1, ulBufferChunkSize, &pBufferChunk);
    GetAssociatedDbf().GetRawField(uwFieldNumber, (char*)pBufferChunk);
    theDate.SetRawDate(pBufferChunk);
    free(pBufferChunk);
  }
  catch (prg_exception& x) {
    free(pBufferChunk);
    x.addTrace("GetDate()", "dBaseRecord");
    throw;
  }
  return theDate;
}

/** Retrieve field as string. Since dBase stores data a text, this is fairly straight forward. */
std::string& dBaseRecord::GetFieldValue(unsigned short uwFieldNumber, std::string& buffer) {
  short wFieldLength = GetFieldLength(uwFieldNumber);
  unsigned char * pBufferChunk=0;
  unsigned long   ulBufferChunkSize=0;

  try {
    ResizeChunk(wFieldLength + 1, ulBufferChunkSize, &pBufferChunk);
    AccessExpediter ae(*this);
    GetAssociatedDbf().GetRawField(GetAssociatedDbf().GetFieldName(uwFieldNumber), (char*)pBufferChunk);
    buffer.assign(reinterpret_cast<const char*>(pBufferChunk), wFieldLength);
    trimString(buffer);
    free(pBufferChunk);
  }
  catch (prg_exception& x) {
    free(pBufferChunk);
    x.addTrace("GetFieldValue()", "dBaseRecord");
    throw;
  }
  return buffer;
}

//Get the value of the field at 'uwFieldNumber' as a ZdBlob.
void dBaseRecord::GetMemo(unsigned short uwFieldNumber, unsigned char** Value, unsigned long& ValueSize) const {
  AccessExpediter ae(*this);
  ResizeChunk(GetAssociatedDbf().GetMemoFieldLen(uwFieldNumber), ValueSize, Value);
  GetAssociatedDbf().GetMemoField(uwFieldNumber, ValueSize, (char*)*Value, F_SETLKW);
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
bool dBaseRecord::GetBoolean(unsigned short uwFieldNumber) const {
  bool bResult;

  try {
    AccessExpediter ae(*this);
    bResult = GetAssociatedDbf().GetLogicalField(uwFieldNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("GetBoolean()", "dBaseRecord");
    throw;
  }
  return bResult;
}

//Get the value of the field at 'uwFieldIndex' as a long.
long dBaseRecord::GetLong(unsigned short uwFieldNumber) const {
  long lResult;

  try {
    AccessExpediter ae(*this);
    lResult = GetAssociatedDbf().GetLongField(uwFieldNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("GetLong()", "dBaseRecord");
    throw;
  }
  return lResult;
}

//Get the value of the field at 'uwFieldIndex' as a double.
double dBaseRecord::GetNumber(unsigned short uwFieldNumber) const {
  double dResult;

  try {
    AccessExpediter ae(*this);
    if (GetAssociatedDbf().GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
      dResult = GetAssociatedDbf().GetDoubleField(uwFieldNumber);
    else
      dResult = GetAssociatedDbf().GetFloatField(uwFieldNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("GetNumber()", "dBaseRecord");
    throw;
  }
  return dResult;
}

// Set the record buffer to hold the same data as the current record in the associated
// Dbf.
void dBaseRecord::LoadFromCurrentDbfRecord(xbDbf & theDbf) {
  try {
    AccessExpediter ae(*this, true);
    theDbf.GetRecord( theDbf.GetCurRecNo() );//make sure that the data for the current record is in the RecBuf
                                             //when 'rm' goes out of scope, it will copy the RecBuf into gBuffer.
  }
  catch (prg_exception& x) {
    x.addTrace("LoadFromCurrentDbfRecord()", "dBaseRecord");
    throw;
  }
}

// Set the record buffer to hold the same data as the current record in the associated
// Dbf.
//<br>require
//<br>  valid_position: (0 < ulPosition) && (ulPosition <= theDbf.NoOfRecords())
void dBaseRecord::OverwriteDbfRecordAt(xbDbf & theDbf, unsigned long ulPosition) const {
  xbShort rc;
  try {
    if ((ulPosition == 0) || (ulPosition > (unsigned long)theDbf.NoOfRecords()))
      throw prg_error("ulPosition (value=%d) is out of range [%d, %d].", "dBaseRecord", ulPosition, 1, theDbf.NoOfRecords());
    {
     AccessExpediter ae(*this);
     //gBuffer's data is now in GetAssociatedDbf().RecBuf
     rc = GetAssociatedDbf().PutRecord( ulPosition );
     AssertNotXBaseError(rc);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("OverwriteDbfRecordAt()", "dBaseRecord");
    throw;
  }
}

//If 'uwFieldIndex' is not in range, throw prg_error with an appropriate message.
void dBaseRecord::CheckFieldIndexRange(unsigned short uwFieldIndex) const {
  try {
    if (! GetFieldIndexIsInRange(uwFieldIndex))
      throw prg_error("The field index (value=%d) is not in range [%d, %d].", "dBaseRecord", uwFieldIndex, 0, GetFieldCount() - 1);
  }
  catch (prg_exception& x) {
    x.addTrace("CheckFieldIndexRange()", "dBaseRecord");
    throw;
  }
}

void dBaseRecord::PutFieldValue(unsigned short uwFieldIndex, const FieldValue& theValue) {
  try {
    CheckFieldIndexRange(uwFieldIndex);
    if (!( theValue.GetType() == GetFieldType(uwFieldIndex) ))
      throw prg_error("Cannot put a field value of type %s into a field of type %s.", "dBaseRecord", FieldValue::GetTypeCString(theValue.GetType()), FieldValue::GetTypeCString(GetFieldType(uwFieldIndex)));
    switch (theValue.GetType()) {
      case FieldValue::ALPHA_FLD:
         PutAlpha(uwFieldIndex, theValue.AsString().c_str());
         break;
      case FieldValue::LONG_FLD:
         PutLong(uwFieldIndex, theValue.AsLong());
         break;
      case FieldValue::BOOLEAN_FLD:
         PutBoolean(uwFieldIndex, theValue.AsBool());
         break;
      case FieldValue::NUMBER_FLD:
         PutNumber(uwFieldIndex, theValue.AsDouble());
         break;
      case FieldValue::DATE_FLD:
         PutDate(uwFieldIndex, theValue.AsDate());
         break;
      default:
         throw prg_error("This function is not setup to handle type '%c'.", "dBaseRecord", theValue.GetType());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("PutFieldValue()", "dBaseRecord");
    throw;
  }
}

//Put an Alpha value into the field at 'uwFieldIndex'.
//<br>require
//<br>  field_value_short_enough:  std::strlen(pFieldValue) <= GetFieldLength(uwFieldNumber)
void dBaseRecord::PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue) {
  short wFieldLength = GetFieldLength(uwFieldNumber);
  try {
    if ((unsigned)wFieldLength < std::strlen(pFieldValue))
      throw prg_error( "String Value, \"%s\", of length, %d, must not be longer than %d.", "dBaseRecord", pFieldValue, strlen(pFieldValue), wFieldLength );
    {
     AccessExpediter ae(*this, true);
     GetAssociatedDbf().PutField(uwFieldNumber, pFieldValue);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("PutAlpha()", "dBaseRecord");
    throw;
  }
}

//Put a "blank" value into the field at 'uwFieldIndex'.
void dBaseRecord::PutBlank(unsigned short uwFieldNumber) {
  try {
    short wFieldLength(GetFieldLength(uwFieldNumber));
    std::string sTemp;
    {
     AccessExpediter ae(*this, true);
     switch (GetAssociatedDbf().GetFieldType(uwFieldNumber)) {
       case XB_CHAR_FLD :
       case XB_NUMERIC_FLD :
       case XB_DATE_FLD :
       case XB_FLOAT_FLD : //set all spaces
           sTemp.resize((unsigned long)wFieldLength, ' ');
           GetAssociatedDbf().PutField(uwFieldNumber, sTemp.c_str());
           break;
       case XB_LOGICAL_FLD : //set to question mark
           sTemp = '?';
           GetAssociatedDbf().PutField(uwFieldNumber, sTemp.c_str());
           break;
       case XB_MEMO_FLD : //set to an empty memo
           GetAssociatedDbf().UpdateMemoData(uwFieldNumber, 0, (char*)0, F_SETLKW);
           break;
     }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("PutBlank()", "dBaseRecord");
    throw;
  }
}

//Put a boolean value into the field at 'uwFieldIndex'.
void dBaseRecord::PutBoolean(unsigned short uwFieldNumber, bool bValue) {
  char sTrue[2] = "T";//we could use any of these: T, t, Y, y
  char sFalse[2] = "F";//we could use any of these: F, f, N, n

  try {
    AccessExpediter ae(*this, true);
    GetAssociatedDbf().PutField(uwFieldNumber, (bValue ? sTrue : sFalse));
  }
  catch (prg_exception& x) {
    x.addTrace("PutBoolean()", "dBaseRecord");
    throw;
  }
}

//Put a date value into the field at 'uwFieldIndex'.
void dBaseRecord::PutDate(unsigned short uwFieldNumber, const SaTScan::Date& theDate) const {
  try {
    std::string sRawDate;
    theDate.RetrieveRawDate(sRawDate);
    AccessExpediter ae(*this, true);
    GetAssociatedDbf().PutField(uwFieldNumber, sRawDate.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("PutDate()", "dBaseRecord");
    throw;
  }
}

//Put a long value into the field at 'uwFieldIndex'.
void dBaseRecord::PutLong(unsigned short uwFieldNumber, long lFieldValue) {
  try {
    AccessExpediter ae(*this, true);
    GetAssociatedDbf().PutLongField(uwFieldNumber, lFieldValue);
  }
  catch (prg_exception& x) {
    x.addTrace("PutLong()", "dBaseRecord");
    throw;
  }
}

//Put a ZdBlob value into the field at 'uwFieldNumber'.
void dBaseRecord::PutMemo(unsigned short uwFieldNumber, unsigned char** Value, unsigned long& ValueSize) {
  try {
     AccessExpediter ae(*this, true);
     GetAssociatedDbf().UpdateMemoData(uwFieldNumber, ValueSize, (const char *)(*Value), F_SETLKW);
  }
  catch (prg_exception& x) {          
    x.addTrace("PutMemo()", "DBFRecord");
    throw;
  }
}

//Put a numeric value into the field at 'uwFieldIndex'.
void dBaseRecord::PutNumber(unsigned short uwFieldNumber, double dFieldValue) {
  try {
    AccessExpediter ae(*this, true);
    if (GetAssociatedDbf().GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
      GetAssociatedDbf().PutDoubleField(uwFieldNumber, dFieldValue);
    else
      GetAssociatedDbf().PutFloatField(uwFieldNumber, static_cast<float>(dFieldValue));
  }
  catch (prg_exception& x) {
    x.addTrace("PutNumber()", "dBaseRecord");
    throw;
  }
}

// Ensure that the sizes of the buffers are at least 'lReqdSize'.
void dBaseRecord::ResizeBuffers(unsigned long ulReqdSize) {
  try {
    if (gulBufferChunkSize <= ulReqdSize) {
      ResizeChunk(ulReqdSize, gulBufferChunkSize, &gpBufferChunk);
      ResizeChunk(ulReqdSize, gulTempBufferChunkSize, &gpTempBufferChunk);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ResizeBuffers()", "dBaseRecord");
    throw;
  }
}

//Retrieve the value from the record, storing it in 'theStore'.
//<br>require
//<br>  field_index_in_range: GetFieldIndexIsInRange(uwFieldIndex)
//<br>  field_is_not_blank: !GetIsBlank(uwFieldIndex)
//<br>  value_is_appropriate_type: theStore.GetType() == GetFieldType(uwFieldIndex)
void dBaseRecord::RetrieveFieldValue(unsigned short uwFieldIndex, FieldValue & theStore) const {
  try {
    CheckFieldIndexRange(uwFieldIndex);
    if ( GetIsBlank(uwFieldIndex) )
      throw prg_error("Cannot retrieve a field value from a blank field (uwFieldIndex=%d).", "RetrieveFieldValue", uwFieldIndex);
    if (!( theStore.GetType() == GetFieldType(uwFieldIndex) ))
      throw prg_error("Cannot retrieve a field value of type %s from a field of type %s.", "RetrieveFieldValue", FieldValue::GetTypeCString(theStore.GetType()), FieldValue::GetTypeCString(GetFieldType(uwFieldIndex)));

    switch (theStore.GetType()) {
      case FieldValue::ALPHA_FLD    :
        {unsigned char * pBufferChunk;
         unsigned long   ulBufferChunkSize;
         ResizeChunk(GetFieldLength(uwFieldIndex) + 1, ulBufferChunkSize, &pBufferChunk);
         GetAlpha(uwFieldIndex, (char*)pBufferChunk, ulBufferChunkSize);
         pBufferChunk[ulBufferChunkSize - 1] = 0;//ensure it is NULL terminated
         theStore.AsString() = (char*)pBufferChunk;
         free(pBufferChunk);
         } break;
       case FieldValue::DATE_FLD    : GetDate(uwFieldIndex, theStore.AsDate()); break;
       case FieldValue::LONG_FLD    : theStore.AsLong() = GetLong(uwFieldIndex); break;
       case FieldValue::BOOLEAN_FLD : theStore.AsBool() = GetBoolean(uwFieldIndex); break;
       case FieldValue::NUMBER_FLD  : theStore.AsDouble() = GetNumber(uwFieldIndex); break;
       default : throw prg_error("This function is not setup to handle type '%c'.", "ZdRecord", theStore.GetType());
    }
  }
  catch (prg_exception& e) {
    e.addTrace("RetrieveFieldValue()", "dBaseRecord");
    throw;
  }
}


//ClassDesc Begin dBaseFile
// dBaseFile exposes dBase files (ver III+, IV) .  It is
// implemented in terms of the open-source library, xbase.
// It directs functions through a class member of type, xbDbf, the xbase file class.
// Each object of type, xbDbf, takes a xbXBase* as an argument.  Apparently, an xbXBase
// acts as a registry of all the currently opened xbDbf's.  However, since I can't
// see the advantage of registering all xbDbf's with a single xbXBase, I simply
// create an xbXBase object for each dBaseFile, and the xbDbf is registered with it.
// If it becomes an issue, we can consider making the xbXBase object a static class
// member or something like that.
//ClassDesc End dBaseFile

////////////////////////////////////////////////////////////////////////////////
// dBaseFile functions:
////////////////////////////////////////////////////////////////////////////////

//construct
dBaseFile::dBaseFile(const char * sFilename, bool bAppend) : gPSystemRecord(0) {
  try {
    gpDbf.reset(new xbDbf(&gXBase));
    if (sFilename) Open(sFilename, bAppend);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const char *)", "dBaseFile");
    throw;
  }
}

//destruct
dBaseFile::~dBaseFile() {
  try {
    Close();
    delete gPSystemRecord;
  }
  catch (...) {}
}

// This function returns the file name for this class.
const char * dBaseFile::GetFileName(std::string& name) const {
  return gFileName.getFullPath(name).c_str();
}

// Throw an exception if the candidate isn't legal for use as the name of a field.
void dBaseFile::AssertLegalFieldname(const std::string& sCandidate) {
  if (! dBaseFile::IsLegalFieldname(sCandidate))
    throw prg_error("The string, \"%s\", is not legal for use as the name of a field in a DBF file.", "dBaseFile", sCandidate.c_str());
}

// Throw an exception if GetIsOpen() returns 'false'.
void dBaseFile::Assert_GetIsOpen() const {
  try {
    if (! GetIsOpen())
      throw prg_error("The dBaseFile isn't open.", "dBaseFile");
  }
  catch (prg_exception& x) {
    x.addTrace("Assert_GetIsOpen()", "dBaseFile");
    throw;
  }
}

// Throw an exception if 'code' != XB_NO_ERROR
void dBaseFile::AssertNotXBaseError(xbShort code) const {
  if (code != XB_NO_ERROR)
    throw prg_error("XBase error: \"%s\".", "dBaseFile", gXBase.GetErrorMessage(code));
}

// Generate an exception if 'ulRecNum' is out of range.
void dBaseFile::CheckRecNum(unsigned long ulRecNum) const {
  if (!( (0 < ulRecNum) && (ulRecNum <= GetNumRecords())))
    throw prg_error("The record index (value=%d) is out of range, [%d, %d].", "dBaseFile", ulRecNum, 1, GetNumRecords());
}

//Close the file.
void dBaseFile::Close() {
  xbShort rc;

  try {
    if (gpDbf->GetDbfStatus() != XB_CLOSED) {
      rc = gpDbf->CloseDatabase(true);
      if (rc != XB_NO_ERROR) {
        std::string buffer;
        throw prg_error("Could not close file, \"%s\".  xbase error: \"%s\".", "dBaseFile", GetFileName(buffer), GetDbfErrorString(rc));
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("Close()", "dBaseFile");
    throw;
  }
}

void dBaseFile::Create(const char * sFilename, ptr_vector<FieldDef>& vFields) {
  unsigned u;
  char cFieldType;
  std::pair<long, long> longRange;
  xbSchema * aXBaseFieldDefs(0);
  xbSchema * itrCurrentSchema;
  long lFieldLength;
  long lFieldPrecision;
  xbShort rc;

  try {
    aXBaseFieldDefs = new xbSchema[vFields.size()+1];//make sure to allocate the "sentinel" schema
    //populate field-def-array, checking passed-in values
    for (u = 0; u < vFields.size(); u++) {
       itrCurrentSchema = aXBaseFieldDefs + u;
       //test existence of each field:
       if (! vFields.at(u))
         throw prg_error("null field at position %d in field vector.", "PackFields", u);
       //test field name legality
       dBaseFile::AssertLegalFieldname(vFields.at(u)->GetName());
       std::strncpy(itrCurrentSchema->FieldName, vFields.at(u)->GetName(), 10);
       itrCurrentSchema->FieldName[10] = 0;//null-terminate field name

       //test field type validity
       cFieldType = vFields.at(u)->GetType();
       if (! dBaseFile::IsValidFieldType(cFieldType))
         throw prg_error("field at position %d in field vector has type, '%c', which is invalid.", "PackFields", u, cFieldType);
       itrCurrentSchema->Type = dBaseFile::GetXBaseFieldTypeFromFieldDefType(vFields.at(u)->GetType());
       //test field length legality
       longRange = dBaseFile::FieldLengthRangeForXBaseFieldType(dBaseFile::GetXBaseFieldTypeFromFieldDefType(cFieldType) );
       lFieldLength = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetLength());
       if (! ((longRange.first <= lFieldLength) && (lFieldLength <= longRange.second)) )
         throw prg_error("field at position %d in field vector has length, %d, which is out of range [%d, %d].", "PackFields", u, lFieldLength, longRange.first, longRange.second);
       itrCurrentSchema->FieldLen = lFieldLength;
       //test decimal count legality
       longRange = dBaseFile::DecimalCountRangeForXBaseFieldType( dBaseFile::GetXBaseFieldTypeFromFieldDefType(cFieldType), vFields.at(u)->GetLength() );
       lFieldPrecision = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetPrecision());
       if (! ((longRange.first <= lFieldPrecision) && (lFieldPrecision <= longRange.second)) )
         throw prg_error("field at position %d in field vector has precision, %d, which is out of range [%d, %d].", "PackFields", u, lFieldPrecision, longRange.first, longRange.second);
       itrCurrentSchema->NoOfDecs = lFieldPrecision;
    }
    //setup the "sentinel" schema:
    itrCurrentSchema = aXBaseFieldDefs + vFields.size();
    itrCurrentSchema->FieldName[0] = 0;//make field-name an empty string
    itrCurrentSchema->Type = 0;
    itrCurrentSchema->FieldLen = 0;
    itrCurrentSchema->NoOfDecs = 0;

    std::string buffer;
    rc = gpDbf->CreateDatabase(FileName(sFilename).getFullPath(buffer).c_str(), aXBaseFieldDefs, 0);
    if (rc != XB_NO_ERROR)
      throw prg_error("could not create database, \"%s\".  xbase error:  \"%s\".", "dBaseFile", FileName(sFilename).getFullPath(buffer).c_str(), gXBase.GetErrorMessage(rc));

    delete[] aXBaseFieldDefs;
  }
  catch (prg_exception& x) {
    delete[] aXBaseFieldDefs;
    x.addTrace("Create()", "dBaseFile");
    throw;
  }
  catch (...) {
    delete[] aXBaseFieldDefs;
    throw;
  }
}

// Does 'cCandidate' indicate a FLD that is used by DBFFile ?
bool dBaseFile::IsValidFieldType(char cCandidate) {
   switch (cCandidate) {
     case FieldValue::ALPHA_FLD   :
     case FieldValue::BOOLEAN_FLD :
     case FieldValue::NUMBER_FLD  :
     //blob?
     case FieldValue::DATE_FLD    : return true;
   }
   return false;
}
// Save 'Record' as the last record in the file.
//<br>require
//<br>  type_conformant_record: dynamic_cast<const dBaseRecord *>(&Record) != 0
unsigned long dBaseFile::DataAppend(const dBaseRecord &Record) {
  unsigned long ulResult;
  const dBaseRecord * pRecord(dynamic_cast<const dBaseRecord *>(&Record));

  try {
    if (! pRecord)
      throw prg_error("Record is not of type dBaseRecord.", "dBaseFile");

    pRecord->AppendToDbf(*gpDbf);
    ulResult = GetNumRecords();
  }
  catch (prg_exception& x) {
    x.addTrace("DataAppend()", "dBaseFile");
    throw;
  }
  return ulResult;
}

// Save the values in the record at 'ulPosition' into *pCurrentValue, if it is non-NULL.
// Overwrite the values in that record with the values in 'Record'.
//<br>require
//<br>  valid_position: (0 < ulPosition) && (ulPosition <= GetNumRecords())
//<br>  type_conformant_record: dynamic_cast<const dBaseRecord *>(&Record) != 0
//<br>  type_conformant_current_value_record: (pCurrentValue != 0) implies (dynamic_cast<dBaseRecord *>(pCurrentValue) != 0)
unsigned long dBaseFile::DataModify(unsigned long ulPosition, const dBaseRecord &Record, dBaseRecord *pCurrentValue) {
  xbShort rc;
  const dBaseRecord * pRecord(dynamic_cast<const dBaseRecord *>(&Record));
        dBaseRecord * pCurValRec(dynamic_cast<dBaseRecord *>(pCurrentValue));
  long  lCurRecNo(gpDbf->GetCurRecNo());

  try {
    if (! ( (0 < ulPosition) && (ulPosition <= GetNumRecords()) ))
      throw prg_error("ulPosition (value=%d)Record is not of type dBaseRecord.", "dBaseFile");
    if (! pRecord)
      throw prg_error("Record is not of type dBaseRecord.", "dBaseFile");
    if ((pCurrentValue) && (! pCurValRec))
      throw prg_error("pCurrentValue does not reference a record of type dBaseRecord.", "dBaseFile");

    rc = gpDbf->GetRecord(ulPosition);
    AssertNotXBaseError(rc);
    if (pCurValRec) {
      pCurValRec->LoadFromCurrentDbfRecord(*gpDbf);
    }
    pRecord->OverwriteDbfRecordAt(*gpDbf, gpDbf->GetCurRecNo());

    //restore Dbf to previous record:
    rc = gpDbf->GetRecord(lCurRecNo);
    AssertNotXBaseError(rc);
  }
  catch (prg_exception& x) {
    x.addTrace("DataModify()", "dBaseFile");
    throw;
  }
  return ulPosition;
}

// What are the minimum and maximum possible number of decimals for a field of type
// 'cFieldType' with a length of lFieldLength ?
// When theResult.first == theResult.second, the length is builtin.
//<br>require
//<br>  valid_xbase_field_type:  dBaseFile::IsValidXBaseFieldType(cFieldType)
std::pair<long, long> dBaseFile::DecimalCountRangeForXBaseFieldType(char cFieldType, long lFieldLength) {
  std::pair<long, long> theResult;

  try {
    CheckXBaseFieldType(cFieldType);
    switch (cFieldType) {
         case XB_CHAR_FLD :
            theResult.first = 0;
            theResult.second = 0;
            break;
         case XB_LOGICAL_FLD :
            theResult.first = 0;
            theResult.second = 0;
            break;
         case XB_NUMERIC_FLD :
            theResult.first = 0;
            theResult.second = lFieldLength - 2;//leave enough space for the decimal point and one digit to its left
            break;
         case XB_DATE_FLD :
            theResult.first = 0;
            theResult.second = 0;
            break;
         case XB_MEMO_FLD :
            theResult.first = 0;
            theResult.second = 0;
            break;
         case XB_FLOAT_FLD :
            theResult.first = 0;
            theResult.second = lFieldLength - 2;//leave enough space for the decimal point and one digit to its left
            break;
         default :
            throw prg_error("Unhandled field type: '%c'", "dBaseFile", cFieldType);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DecimalCountRangeForXBaseFieldType()", "dBaseFile");
    throw;
  }
  return theResult;
}

//Remove all records from this file.
void dBaseFile::Empty()  {
  gpDbf->Zap(F_SETLKW);
}

// What are the minimum and maximum possible lengths for a field of type 'cFieldType' ?
// When theResult.first == theResult.second, the length is builtin.
//<br>require
//<br>  valid_xbase_field_type:  dBaseFile::IsValidXBaseFieldType(cFieldType)
std::pair<long, long> dBaseFile::FieldLengthRangeForXBaseFieldType(char cFieldType) {
  std::pair<long, long> theResult;

  try {
    CheckXBaseFieldType(cFieldType);
    switch (cFieldType) {
         case XB_CHAR_FLD :
            theResult.first = 1;
            theResult.second = 254;
            break;
         case XB_LOGICAL_FLD :
            theResult.first = 1;
            theResult.second = 1;
            break;
         case XB_NUMERIC_FLD :
            theResult.first = 1;
            theResult.second = 20;
            break;
         case XB_DATE_FLD :
            theResult.first = 8;
            theResult.second = 8;
            break;
         case XB_MEMO_FLD :
            theResult.first = 10;
            theResult.second = 10;
            break;
         case XB_FLOAT_FLD :
            theResult.first = 1;
            theResult.second = 20;
            break;
         default :
            throw prg_error("Unhandled field type: '%c'", "dBaseFile", cFieldType);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("FieldLengthRangeForXBaseFieldType()", "dBaseFile");
    throw;
  }
  return theResult;
}

// What is the index of the record where the file is currently positioned ?
// If GetNumRecords() == 0, result is 0, otherwise, 0 < result <= GetNumRecords().
unsigned long dBaseFile::GetCurrentRecordNumber() const {
  return gpDbf->GetCurRecNo();
}

// Get the error-indicator-string from the xbase code.
const char * dBaseFile::GetDbfErrorString(xbShort code) const {
  return gXBase.GetErrorMessage(code);
}

// Is this file open ?  True if Open() has been succesfully invoked, but Close()
// has not.
bool dBaseFile::GetIsOpen() const {
  bool bResult;

  try {
    bResult = gpDbf->GetDbfStatus() != XB_CLOSED;
  }
  catch (prg_exception& x) {
    x.addTrace("GetIsOpen()", "dBaseFile");
    throw;
  }
  return bResult;
}

//How many records are currently in this file ?
unsigned long dBaseFile::GetNumRecords() const {
  unsigned long ulResult;

  try {
    ulResult = gpDbf->NoOfRecords();
  }
  catch (prg_exception& x) {
    x.addTrace("GetNumRecords()", "dBaseFile");
    throw;
  }
  return ulResult;
}

// What is the XB_..._FLD that corresponds to the FieldValue..._FLD indicated by 'cFieldDefType' ?
char dBaseFile::GetXBaseFieldTypeFromFieldDefType(char cFieldDefType) {
  switch (cFieldDefType) {
    case FieldValue::ALPHA_FLD : return XB_CHAR_FLD;
    case FieldValue::BOOLEAN_FLD : return XB_LOGICAL_FLD;
    case FieldValue::NUMBER_FLD : return XB_NUMERIC_FLD;
    default : throw prg_error("Unhandled field type: '%c'", "GetXBaseFieldTypeFromFieldDefType", cFieldDefType);
  }
}

// What is the FieldValue..._FLD that corresponds to the XB_..._FLD indicated by 'cXBaseFieldType' ?
//<br>require
//<br>  valid_xbase_field_type:  dBaseFile::IsValidXBaseFieldType(cXBaseFieldType)
char dBaseFile::GetFieldDefTypeFromXBaseFieldType(char cXBaseFieldType) {
  CheckXBaseFieldType(cXBaseFieldType);
  switch (cXBaseFieldType) {
    case XB_DATE_FLD    : return FieldValue::DATE_FLD;
    case XB_CHAR_FLD    : return FieldValue::ALPHA_FLD;
    case XB_LOGICAL_FLD : return FieldValue::BOOLEAN_FLD;
    case XB_NUMERIC_FLD : return FieldValue::NUMBER_FLD;
    case XB_FLOAT_FLD   : return FieldValue::NUMBER_FLD;
    case XB_MEMO_FLD    :
    default : throw prg_error("Unhandled field type: '%c'", "GetFieldDefTypeFromXBaseFieldType", cXBaseFieldType);
  }
}

// This function returns a pointer to the system buffer.
dBaseRecord * dBaseFile::GetSystemRecord() const {
  return gPSystemRecord;
}

// Ensure that the SystemRecord holds the data for the record at 'lRecNum'.
// If PRecordBuffer isn't NULL, ensure that it, also, holds the same data.
//<br>require
//<br>  valid_record_index:  (0 <= lRecNum) && (lRecNum < GetNumRecords())
//<br>  type_conformant_record: (!PRecordBuffer) || (dynamic_cast<dBaseRecord *>(PRecordBuffer) != 0)
void dBaseFile::GotoRecord(unsigned long lRecNum, dBaseRecord * PRecordBuffer) {
  dBaseRecord * pTempDBFRecord(0);

  try {
    CheckRecNum(lRecNum);
    if (PRecordBuffer) {
      pTempDBFRecord = dynamic_cast<dBaseRecord *>(PRecordBuffer);
      if (! pTempDBFRecord)
        throw prg_error("The record buffer isn't of type dBaseRecord or descendant.", "dBaseFile");
    }
    gpDbf->GetRecord(lRecNum);
    dynamic_cast<dBaseRecord *>(GetSystemRecord())->LoadFromCurrentDbfRecord(*gpDbf);
    if (pTempDBFRecord)
      pTempDBFRecord->LoadFromCurrentDbfRecord(*gpDbf);
  }
  catch (prg_exception& x) {
    x.addTrace("GotoRecord()", "dBaseFile");
    throw;
  }
}

// Is the candidate legal for use as the name of a field ?
bool dBaseFile::IsLegalFieldname(const std::string & sCandidate) {
  bool bResult(false);
  long l;

  long lLength(sCandidate.size());
  char cCurrentChar(sCandidate[0]);
  if ( (0 < lLength) && (lLength <= 10) ) {//test length
    if ( ('A' <= cCurrentChar) && (cCurrentChar <= 'Z') ) {//test first character
      bResult = true;
      l = 1;
      while ( bResult && (l < lLength) ) {//test remaining characters
           cCurrentChar = sCandidate[l];
           bResult = ((('A' <= cCurrentChar) && (cCurrentChar <= 'Z'))//is uppercase alphabetical
                        || (('0' <= cCurrentChar) && (cCurrentChar <= '9'))//is numeric
                        || (cCurrentChar == '_'));//is underscore
           ++l;
      }
    }
  }
  return bResult;
}

// Does 'cCandidate' indicate an XB_..._FLD that is used by dBaseFile ?
bool dBaseFile::IsValidXBaseFieldType(char cCandidate) {
  switch (cCandidate) {
    case XB_CHAR_FLD    :
    case XB_LOGICAL_FLD :
    case XB_NUMERIC_FLD :
    case XB_DATE_FLD    :
    case XB_MEMO_FLD    :
    case XB_FLOAT_FLD   : return true;
    default : return false;
  }
}

//Open a file.
//<br>require
//<br>  filename_string_not_null:  sFilename != 0
void dBaseFile::Open(const char *sFilename, bool bAppendMode) {
  xbShort       rc;
  FileName      fn(sFilename);

  try {
    //store open flags for ReadStructure() -- current interface prevents this function from getting this information
    gbAppendMode = bAppendMode;
    Close();
    gFileName.setFullPath(sFilename);
    delete gPSystemRecord; gPSystemRecord=0;
    ReadStructure();
    if (gpDbf->GetDbfStatus() != XB_CLOSED) {
      rc = gpDbf->CloseDatabase(true);
      if (rc != XB_NO_ERROR) {
        std::string buffer;
        throw prg_error("Could not close file, \"%s\".  xbase error: \"%s\".", "dBaseFile", GetFileName(buffer), gXBase.GetErrorMessage(rc));
      }
    }
    //determine open read/write mode and open database
    // -- currnetly we only need to open for update or open read only
    std::string buffer;
    rc = gpDbf->OpenDatabase(fn.getFullPath(buffer).c_str(), (bAppendMode ? "r+b" : "rb"));
    if (rc != XB_NO_ERROR)
      throw prg_error("Could not open file, \"%s\".  xbase error: \"%s\".", "dBaseFile", fn.getFullPath(buffer).c_str(), gXBase.GetErrorMessage(rc));
    gPSystemRecord = GetNewRecord();
  }
  catch (prg_exception& x) {
    x.addTrace("Open()", "dBaseFile");
    throw;
  }
}

// Pack the data as tightly as possible.
void dBaseFile::PackData() {
  gpDbf->PackDatabase(F_SETLKW/*, function to update the progress*/);
}

//Ensure that the vFields is appropriate for input to the function, Create.
//<br>require
//<br>  no_fields_null:  for i in [0, vFields.size()], vFields.at(i) != NULL
void dBaseFile::PackFields(ptr_vector<FieldDef>& vFields) const {
  for (unsigned u = 0; u < vFields.size(); u++) {
    if (! vFields.at(u))
      throw prg_error("null field at position %d in field vector.", "PackFields", u);
  }
}

void dBaseFile::ReadStructure() {
  xbShort rc;
  unsigned u;

  try {
    //make sure the .dbf file is open:
    if (gpDbf->GetDbfStatus() == XB_CLOSED) {
      std::string buffer;
      rc = gpDbf->OpenDatabase(gFileName.getFullPath(buffer).c_str(), (gbAppendMode ? "r+b" : "rb"));
    }
    if (rc != XB_NO_ERROR) {
      std::string buffer;
      throw prg_error("Could not open file: \"%s\".  xbase error: \"%s\"", "dBaseFile", gFileName.getFullPath(buffer).c_str(), gXBase.GetErrorMessage(rc));
    }
    // Setup file properties
    std::string buffer;
    gsTitle = gFileName.getFileName();
    // Set field info
    gvFields.killAll();
    gvFields.resize(gpDbf->FieldCount());
    for (u = 0; u < gvFields.size(); u++) {
       gvFields.at(u) = new FieldDef(gpDbf->GetFieldName(u), GetFieldDefTypeFromXBaseFieldType(gpDbf->GetFieldType(u)),
                                     gpDbf->GetFieldLen(u), gpDbf->GetFieldDecimal(u), 0);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadStructure()", "dBaseFile");
    throw;
  }
}

void dBaseFile::CheckXBaseFieldType(char cXBaseFieldType) {
  if (!dBaseFile::IsValidXBaseFieldType(cXBaseFieldType))
    throw prg_error("The character, '%c', does not refer to a valid xbase field type.", "dBaseFile", cXBaseFieldType);
}

void dBaseFile::CheckFieldType(char cFieldType) {
  if (! dBaseFile::IsValidFieldType(cFieldType))
    throw prg_error("The character, '%c', does not refer to a valid Zd field type.", "CheckFieldType", cFieldType);
}
