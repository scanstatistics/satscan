//---------------------------------------------------------------------------

#include "xbase/xbase.h"
#pragma hdrstop

#include "DBFFile.h"
//---------------------------------------------------------------------------

//construct
//use an empty field array
DBFRecord::DBFRecord( DBFFile & associatedFile, xbDbf & associatedDbf, const ZdVector<ZdField*> & vFields) : ZdFileRecord( vFields, 0)
//   , gpAssociatedFile( &associatedFile )
   , gpAssociatedDbf( &associatedDbf )
   , gAssociatedFileName(associatedFile.GetFileName())
{
   try
      {
      if (GetAssociatedDbf().GetDbfStatus() == XB_CLOSED)
         ZdException::Generate("The associated file is not open", "DBFRecord");

      ResizeBuffers(associatedDbf.GetRecordLen());

         {
         RecordManipulationExpediter rme(*this, GetAssociatedDbf());
         GetAssociatedDbf().BlankRecord();
         }

      //register this record with 'associatedFile'
      }
   catch (ZdException & e)
      {
      e.AddCallpath("constructor(DBFFile &, xbDbf &)", "DBFRecord");
      throw;
      }
}

//destruct
DBFRecord::~DBFRecord()
{
}

// Append the data in this record to 'theDbf'.
void DBFRecord::AppendToDbf(xbDbf & theDbf) const
{
   xbShort rc;
   try
      {
      RecordAccessExpediter rae(*this, theDbf);
      rc = theDbf.AppendRecord();
      if (rc != XB_NO_ERROR)
         ZdException::Generate("Could not append record to the file, \"%c\".", "DBFRecord", theDbf.GetDbfName());
      }
   catch (ZdException & e)
      {
      e.AddCallpath("AppendToDbf", "DBFRecord");
      throw;
      }
}

// Throw an exception if gBuffer.GetSize() < 'lReqdSize'.
void DBFRecord::AssertSufficientBufferSize(unsigned long ulReqdSize) const
{
   if (gBuffer.GetSize() < ulReqdSize)
      ZdException::Generate("Buffer size, %d, is less than req'd size, %d.", "DBFRecord", gBuffer.GetSize(), ulReqdSize);
}

// Copy the data from the RecBuf in the associated Dbf to gTempBuffer.
void DBFRecord::BufferDbfRecordData(xbDbf & theDbf) const
{
   long lBufferSize(theDbf.GetRecordLen());
   AssertSufficientBufferSize(lBufferSize);
   std::memcpy(gTempBuffer.AsVoidPtr(), theDbf.GetRecordBuf(), lBufferSize);
}

// Copy the data from the RecBuf in the associated Dbf to gBuffer.
void DBFRecord::CopyDbfRecordData(xbDbf & theDbf)
{
   long lBufferSize(theDbf.GetRecordLen());
   AssertSufficientBufferSize(lBufferSize);
   std::memcpy(gBuffer.AsVoidPtr(), theDbf.GetRecordBuf(), lBufferSize);
}

// Copy the data from gTempBuffer to the RecBuf in the associated Dbf.
void DBFRecord::RestoreDbfRecordData(xbDbf & theDbf) const
{
   long lBufferSize(theDbf.GetRecordLen());
   AssertSufficientBufferSize(lBufferSize);
   //GetRecordBuf returns the address to the actual memory, so this is correct:
   std::memcpy(theDbf.GetRecordBuf(), gTempBuffer.AsVoidPtr(), lBufferSize);
}

// Copy the data from gBuffer to the RecBuf in the associated Dbf.
void DBFRecord::UpdateDbfRecordData(xbDbf & theDbf) const
{
   long lBufferSize(theDbf.GetRecordLen());
   AssertSufficientBufferSize(lBufferSize);
   //GetRecordBuf returns the address to the actual memory, so this is correct:
   std::memcpy(theDbf.GetRecordBuf(), gBuffer.AsVoidPtr(), lBufferSize);
}

//Make all fields blank.
void DBFRecord::Clear()
{
   try
      {
      RecordManipulationExpediter rme(*this, GetAssociatedDbf());

      GetAssociatedDbf().BlankRecord();
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("Clear()", "DBFRecord");
      throw;
      }
}

// Check to make sure that gpAssociatedDbf is non-null and return its value.
xbDbf & DBFRecord::GetAssociatedDbf() const
{
   try
      {
      if (!gpAssociatedDbf)
         {
         ZdException::Generate("The file with which this record is associated, \"%c\", is no longer available.", "DBFRecord", gAssociatedFileName.GetFullPath());
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GetAssociatedDbf()", "DBFRecord");
      throw;
      }
   return *gpAssociatedDbf;
}

//How many fields are in *this ?
long DBFRecord::GetFieldCount() const
{
   long lResult;
   try
      {
      lResult = GetAssociatedDbf().FieldCount();
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldCount()", "DBFRecord");
      throw;
      }
   return lResult;
}

//How many bytes does this field require ?
short DBFRecord::GetFieldLength(unsigned short uwFieldIndex) const
{
   short wResult;
   try
      {
      wResult = GetAssociatedDbf().GetFieldLen(uwFieldIndex);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldLength()", "DBFRecord");
      throw;
      }
   return wResult;
}

//What is the "field type" (Zd) of the field at 'uwFieldIndex' ?
char DBFRecord::GetFieldType(unsigned short uwFieldIndex) const
{
   char cResult;
   try
      {
      cResult = DBFFile::GetZdFieldTypeFromXBaseFieldType(GetAssociatedDbf().GetFieldType(uwFieldIndex));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldType()", "DBFRecord");
      throw;
      }
   return cResult;
}

//Does the field at 'uwFieldIndex' contain a "blank" value ?
bool DBFRecord::GetIsBlank(unsigned short uwFieldIndex) const
{
   bool bResult;
   ZdResizableChunk buffer(GetAssociatedDbf().GetFieldLen(uwFieldIndex)+1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   const char * pBufBegin;
   const char * pBufEnd;
   try
      {

         {
         RecordAccessExpediter rae(*this, GetAssociatedDbf());

         GetAssociatedDbf().GetRawField(GetAssociatedDbf().GetFieldName(uwFieldIndex), buffer.AsCharPtr());
         }

         switch (GetAssociatedDbf().GetFieldType(uwFieldIndex))
            {
            case XB_CHAR_FLD :
            case XB_NUMERIC_FLD :
            case XB_DATE_FLD :
            case XB_FLOAT_FLD :
               //all spaces?
               bResult = true;
               pBufBegin = buffer.AsCharPtr();
               pBufEnd = pBufBegin + (buffer.GetSize() - 1);
               while ( (pBufBegin < pBufEnd) && bResult )
                  {
                  bResult = *pBufBegin == ' ';
                  ++pBufBegin;
                  }
               break;
            case XB_LOGICAL_FLD :
               //question mark?
               bResult = (buffer[0] == '?');
               break;
            case XB_MEMO_FLD :
               //empty memo?
               bResult = (GetAssociatedDbf().GetMemoFieldLen(uwFieldIndex) == 0);
               break;
            }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetIsBlank()", "DBFRecord");
      throw;
      }
   return bResult;
}

//Get the value of the field at 'uwFieldIndex' as an Alpha.  'ulLength' is the number
//of bytes in 'pFieldValue'.
//<br>require
//<br>  valid_length: ulLength >= (GetFieldLength(uwFieldNumber) + 1)
char * DBFRecord::GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long ulLength, bool bStripBlanks) const
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   int i;
   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
      {
      if (ulLength < (unsigned long)(wFieldLength + 1))
         ZdException::Generate("Buffer %d too small in GetAlpha(); need buffer length of %d", "TXDRec", ulLength, wFieldLength + 1 );

         {
         RecordAccessExpediter rae(*this, GetAssociatedDbf());

         GetAssociatedDbf().GetRawField(GetAssociatedDbf().GetFieldName(uwFieldNumber), buffer.AsCharPtr());
         }

      std::memcpy(pFieldValue, buffer.AsCharPtr(), wFieldLength);
      pFieldValue[wFieldLength] = 0;
      if (bStripBlanks)
         for (i = wFieldLength - 1; i >= 0 && pFieldValue[i] == ' '; i--)
            pFieldValue[i] = 0;
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetAlpha()", "DBFRecord");
      throw;
      }
   return pFieldValue;
}

//Get the value of the field at 'uwFieldIndex' as a Binary.  Be sure that the buffer
//at pValue has at least GetFieldLength(uwFieldNumber) bytes available!
void DBFRecord::GetBinary( unsigned short uwFieldNumber, void *pValue ) const
{
//   short wFieldLength = GetFieldLength(uwFieldNumber);
//   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
      {
      ZdException::Generate("GetBinary not implemented", "DBFRecord");
//      gpAssociatedFile->GetRawField(gpAssociatedFile->GetFieldName(uwFieldNumber), buffer.AsCharPtr());
//      ::memcpy(pValue, buffer.AsCharPtr(), wFieldLength);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBinary()", "DBFRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldNumber' as a ZdBlob.
void DBFRecord::GetBLOB(unsigned short uwFieldNumber, ZdBlob & theValue) const
{
   ZdResizableChunk databuffer(GetAssociatedDbf().GetMemoFieldLen(uwFieldNumber));
   try
      {
         {
         RecordAccessExpediter rae(*this, GetAssociatedDbf());

         databuffer.Resize(GetAssociatedDbf().GetMemoFieldLen(uwFieldNumber));
         GetAssociatedDbf().GetMemoField(uwFieldNumber, databuffer.GetSize(), databuffer.AsCharPtr(), F_SETLKW);
         }

      theValue.SetBlob(databuffer.AsVoidPtr(), databuffer.GetSize());
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBLOB()", "DBFRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
bool DBFRecord::GetBoolean(unsigned short uwFieldNumber) const
{
   bool bResult;

   try
      {
      RecordAccessExpediter rae(*this, GetAssociatedDbf());
      bResult = GetAssociatedDbf().GetLogicalField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBoolean()", "DBFRecord");
      throw;
      }
   return bResult;
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
ZdDate & DBFRecord::GetDate(unsigned short uwFieldNumber, ZdDate &theDate ) const
{
//It happens that the form of the date in the DBF format, CCYYMMDD, is identical
//to a "raw date" value as viewed by ZdDate.
   ZdResizableChunk buffer;

   try
      {
         {
         RecordAccessExpediter rae(*this, GetAssociatedDbf());

         buffer.Resize(GetAssociatedDbf().GetFieldLen(uwFieldNumber)+1);//extra byte because GetRawField sets the extra byte to NULL
         GetAssociatedDbf().GetRawField(uwFieldNumber, buffer.AsCharPtr());
         }
         
      theDate.SetRawDate(buffer.AsCharPtr());
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetDate()", "DBFRecord");
      throw;
      }
   return theDate;
}

//Get the value of the field at 'uwFieldIndex' as a long.
long DBFRecord::GetLong(unsigned short uwFieldNumber) const
{
   long lResult;

   try
      {
      RecordAccessExpediter rae(*this, GetAssociatedDbf());
      lResult = GetAssociatedDbf().GetLongField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetLong()", "DBFRecord");
      throw;
      }
   return lResult;
}

//Get the value of the field at 'uwFieldIndex' as a double.
double DBFRecord::GetNumber(unsigned short uwFieldNumber) const
{
   double dResult;

   try
      {
      RecordAccessExpediter rae(*this, GetAssociatedDbf());

      if (GetAssociatedDbf().GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
         dResult = GetAssociatedDbf().GetDoubleField(uwFieldNumber);
      else
         dResult = GetAssociatedDbf().GetFloatField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetNumber()", "DBFRecord");
      throw;
      }
   return dResult;
}

// Check to make sure that gpAssociatedDbf is non-null and return the RecBuf.
//char * DBFRecord::GetRecBuf() const
//{
//   try
//      {
//      return GetAssociatedDbf()->GetRecordBuf();
//      }
//   catch (ZdException & e)
//      {
//      e.AddCallpath("GetRecBuf()", "DBFRecord");
//      throw;
//      }
//}

//Get the value of the field at 'uwFieldIndex' as a short.
short DBFRecord::GetShort(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetShort not implemented.  Try GetLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetShort()", "DBFRecord");
      throw;
      }
   return 0;
}

//Get the value of the field at 'uwFieldIndex' as a ZdTime.
ZdTime & DBFRecord::GetTime(unsigned short uwFieldNumber, ZdTime &FieldValue) const
{
   try
      {
      ZdException::Generate("GetTime not implemented.  Try GetDate ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetTime()", "DBFRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as a ZdTimestamp.
ZdTimestamp & DBFRecord::GetTimestamp(unsigned short uwFieldNumber, ZdTimestamp &FieldValue) const
{
   try
      {
      ZdException::Generate("GetTimestamp not implemented.  Try GetDate ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetTimestamp()", "DBFRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as an unsigned long.
unsigned long DBFRecord::GetUnsignedLong(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetUnsignedLong not implemented.  Try GetLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetUnsignedLong()", "DBFRecord");
      throw;
      }
   return 0;
}

//Get the value of the field at 'uwFieldIndex' as an unsigned short.
unsigned short DBFRecord::GetUnsignedShort(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetUnsignedShort not implemented.  Try GetLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetUnsignedShort()", "DBFRecord");
      throw;
      }
   return 0;
}

//Put an Alpha value into the field at 'uwFieldIndex'.
//<br>require
//<br>  field_value_short_enough:  std::strlen(pFieldValue) <= GetFieldLength(uwFieldNumber)
void DBFRecord::PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue)
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   try
      {
      if ((unsigned)wFieldLength < std::strlen(pFieldValue))
         ZdException::Generate( "String Value, \"%s\", of length, %d, must not be longer than %d.", "DBFRecord", pFieldValue, strlen(pFieldValue), wFieldLength );

         {
         RecordManipulationExpediter rme(*this, GetAssociatedDbf());
         GetAssociatedDbf().PutField(uwFieldNumber, pFieldValue);
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutAlpha()", "DBFRecord");
      throw;
      }
}

//Put a Binary value into the field at 'uwFieldIndex'.
void DBFRecord::PutBinary ( unsigned short wFieldNumber, const void *pValue )
{
   try
      {
      ZdException::Generate("PutBinary not implemented", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBinary()", "DBFRecord");
      throw;
      }
}

//Put a "blank" value into the field at 'uwFieldIndex'.
void DBFRecord::PutBlank(unsigned short uwFieldNumber)
{
   try
      {
      short wFieldLength(GetFieldLength(uwFieldNumber));
      ZdString sTemp((unsigned long)wFieldLength);
         {
         RecordManipulationExpediter rme(*this, GetAssociatedDbf());

         switch (GetAssociatedDbf().GetFieldType(uwFieldNumber))
            {
            case XB_CHAR_FLD :
            case XB_NUMERIC_FLD :
            case XB_DATE_FLD :
            case XB_FLOAT_FLD :
               //set all spaces
               sTemp.Clear();
               sTemp.Insert(' ', 0, wFieldLength);
               GetAssociatedDbf().PutField(uwFieldNumber, sTemp.GetCString());
               break;
            case XB_LOGICAL_FLD :
               //set to question mark
               sTemp = '?';
               GetAssociatedDbf().PutField(uwFieldNumber, sTemp.GetCString());
               break;
            case XB_MEMO_FLD :
               //set to an empty memo
               GetAssociatedDbf().UpdateMemoData(uwFieldNumber, 0, (char*)0, F_SETLKW);
               break;
            }
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBlank()", "DBFRecord");
      throw;
      }
}

//Put a ZdBlob value into the field at 'uwFieldNumber'.
void DBFRecord::PutBLOB(unsigned short uwFieldNumber, const ZdBlob & theValue)
{
   try
      {
      RecordManipulationExpediter rme(*this, GetAssociatedDbf());
      GetAssociatedDbf().UpdateMemoData(uwFieldNumber, theValue.GetLength(), (const char *)(theValue.GetBlob()), F_SETLKW);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBLOB()", "DBFRecord");
      throw;
      }
}

//Put a boolean value into the field at 'uwFieldIndex'.
void DBFRecord::PutBoolean(unsigned short uwFieldNumber, bool bValue)
{
   char sTrue[2] = "T";//we could use any of these: T, t, Y, y
   char sFalse[2] = "F";//we could use any of these: F, f, N, n
   try
      {
      RecordManipulationExpediter rme(*this, GetAssociatedDbf());
      GetAssociatedDbf().PutField(uwFieldNumber, (bValue ? sTrue : sFalse));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBoolean()", "DBFRecord");
      throw;
      }
}

//Put a date value into the field at 'uwFieldIndex'.
void DBFRecord::PutDate( unsigned short uwFieldNumber, const ZdDate &theDate )
{
//It happens that the form of the date in the DBF format, CCYYMMDD, is identical
//to a "raw date" value as viewed by ZdDate.
   try
      {
      ZdString sRawDate;
      theDate.RetrieveRawDate(sRawDate);

         {
         RecordManipulationExpediter rme(*this, GetAssociatedDbf());

         GetAssociatedDbf().PutField(uwFieldNumber, sRawDate.GetCString());
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutDate()", "DBFRecord");
      throw;
      }
}

//Put a long value into the field at 'uwFieldIndex'.
void DBFRecord::PutLong(unsigned short uwFieldNumber, long lFieldValue)
{
   try
      {
      RecordManipulationExpediter rme(*this, GetAssociatedDbf());
      GetAssociatedDbf().PutLongField(uwFieldNumber, lFieldValue);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutLong()", "DBFRecord");
      throw;
      }
}

//Put a numeric value into the field at 'uwFieldIndex'.
void DBFRecord::PutNumber(unsigned short uwFieldNumber, double dFieldValue)
{
   try
      {
      RecordManipulationExpediter rme(*this, GetAssociatedDbf());

      if (GetAssociatedDbf().GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
         GetAssociatedDbf().PutDoubleField(uwFieldNumber, dFieldValue);
      else
         GetAssociatedDbf().PutFloatField(uwFieldNumber, static_cast<float>(dFieldValue));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutNumber()", "DBFRecord");
      throw;
      }
}

//Put a short value into the field at 'uwFieldIndex'.
void DBFRecord::PutShort(unsigned short uwFieldNumber, short wFieldValue)
{
   try
      {
      ZdException::Generate("PutShort not implemented.  Try PutLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutShort()", "DBFRecord");
      throw;
      }
}

//Put a ZdTime value into the field at 'uwFieldIndex'.
void DBFRecord::PutTime(unsigned short uwFieldNumber, const ZdTime &FieldValue)
{
   try
      {
      ZdException::Generate("PutTime not implemented.  Try PutDate ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTime()", "DBFRecord");
      throw;
      }
}

//Put a ZdTimestamp value into the field at 'uwFieldIndex'.
void DBFRecord::PutTimestamp(unsigned short uwFieldNumber, const ZdTimestamp &FieldValue)
{
   try
      {
      ZdException::Generate("PutTimestamp not implemented.  Try PutDate ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTimestamp()", "DBFRecord");
      throw;
      }
}

//Put an unsigned long value into the field at 'uwFieldIndex'.
void DBFRecord::PutUnsignedLong(unsigned short uwFieldNumber, unsigned long ulFieldValue)
{
   try
      {
      ZdException::Generate("PutUnsignedLong not implemented.  Try PutLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutUnsignedLong()", "DBFRecord");
      throw;
      }
}

//Put an unsigned short value into the field at 'uwFieldIndex'.
void DBFRecord::PutUnsignedShort(unsigned short uwFieldNumber, unsigned short uwFieldValue)
{
   try
      {
      ZdException::Generate("PutUnsignedShort not implemented.  Try PutLong ?", "DBFRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutUnsignedShort()", "DBFRecord");
      throw;
      }
}

// Ensure that the sizes of the buffers are at least 'lReqdSize'.
void DBFRecord::ResizeBuffers(unsigned long ulReqdSize)
{
   try
      {
      if (gBuffer.GetSize() <= ulReqdSize)
         {
         gBuffer.Resize(ulReqdSize);
         gTempBuffer.Resize(ulReqdSize);
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("ResizeBuffers()", "DBFRecord");
      throw;
      }
}

// Set the record buffer to hold the same data as the current record in the associated
// Dbf.
void DBFRecord::SetAsCurrentDbfRecord(xbDbf & theDbf)
{
   try
      {
      RecordManipulationExpediter rme(*this, theDbf);

      theDbf.GetRecord( theDbf.GetCurRecNo() );//make sure that the data for the current record is in the RecBuf
      //when 'rm' goes out of scope, it will copy the RecBuf into gBuffer.
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("SetAsCurrentDbfRecord()", "DBFRecord");
      throw;
      }
}

////////////////////////////////////////////////////////////////////////////////
// DBFFile functions:
////////////////////////////////////////////////////////////////////////////////

//construct
DBFFile::DBFFile(const char * sFilename)
{
   try
      {
      gpDbf = new xbDbf( &gXBase );
      if (sFilename)
         Open(sFilename, 0);
      }
   catch (ZdException & e)
      {
      e.AddCallpath("constructor(const char *)", "DBFFile");
      throw;
      }
}

//destruct
DBFFile::~DBFFile()
{
   try
      {
      Close();
//      delete gPSystemRecord;
      delete gpDbf;
      }
   catch (...)
      {//cannot throw anything
      }
}

// Throw an exception if the candidate isn't legal for use as the name of a field.
void DBFFile::AssertLegalFieldname(const ZdString & sCandidate)
{
   if (! DBFFile::IsLegalFieldname(sCandidate))
      ZdException::Generate("The string, \"%c\", is not legal for use as the name of a field in a DBF file.", "DBFFile");
}

//Generate an exception if 'ulRecNum' is out of range.
void DBFFile::CheckRecNum(unsigned long ulRecNum) const
{
   if (ulRecNum > GetNumRecords())
      ZdException::Generate("The record index (value=%d) is out of range, [%d, %d].", "DBFFile", ulRecNum, 1, GetNumRecords());
}

//Close the file.
void DBFFile::Close()
{
   xbShort rc;
   try
      {
      ZdFile::Close();

      if (gpDbf->GetDbfStatus() != XB_CLOSED)
         {
         rc = gpDbf->CloseDatabase(true);
         if (rc != XB_NO_ERROR)
            {
            ZdException::Generate("Could not close file, \"%c\".", "DBFFile", GetFileName());
            }
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("Close()", "DBFFile");
      throw;
      }
}

// Create a new dBase-compatible file whose records have 'vFields.size()' fields,
// each interpreted according to 'vFields.at(i)'.
// Currently, ignores uwNumPrimaryFields and bBuildIndexes.
//<br>require
//<br>  fields_exist:  for i in [0, vFields.size()-1], vFields.at(i) != NULL
//<br>  fields_have_valid_type:  for i in [0, vFields.size()-1], DBFFile::IsValidZdFieldType(vFields.at(i)->GetType())
//<br>  fields_have_legal_names:  each name must be 10 characters or fewer, the first must be alphabetical (upper case), the rest must be alphabetical (A-Z), numeric (0-9), or underscore (_)
//<br>  fields_have_legal_lengths:  each field's length, if not considered "fixed", must be appropriate to its type
//<br>  fields_have_legal_precisions:  each field's precision, if not considered "fixed", must be appropriate to its type and length
void DBFFile::Create(const char * sFilename, ZdVector<ZdField*> &vFields, unsigned short uwNumPrimaryKeyFields, bool bBuildIndexes )
{
   unsigned u;
   char cFieldType;
   std::pair<long, long> longRange;
   xbSchema * aXBaseFieldDefs(0);
   xbSchema * itrCurrentSchema;
   long lFieldLength;
   long lFieldPrecision;
   xbShort rc;

   try
      {
      aXBaseFieldDefs = new xbSchema[vFields.size()+1];//make sure to allocate the "sentinel" schema

      //populate field-def-array, checking passed-in values
      for (u = 0; u < vFields.size(); u++)
         {
         itrCurrentSchema = aXBaseFieldDefs + u;

         //test existence of each field:
         if (! vFields.at(u))
            ZdException::Generate("null field at position %d in field vector.", "PackFields", u);

         //test field name legality
         DBFFile::AssertLegalFieldname(vFields.at(u)->GetName());
         std::strncpy(itrCurrentSchema->FieldName, vFields.at(u)->GetName(), 10);
         itrCurrentSchema->FieldName[10] = 0;//null-terminate field name

         //test field type validity
         cFieldType = vFields.at(u)->GetType();
         if (! DBFFile::IsValidZdFieldType(cFieldType))
            ZdException::Generate("field at position %d in field vector has type, '%c', which is invalid.", "PackFields", u, cFieldType);
         itrCurrentSchema->Type = DBFFile::GetXBaseFieldTypeFromZdFieldType(vFields.at(u)->GetType());

         //test field length legality
         longRange = DBFFile::FieldLengthRangeForXBaseFieldType(DBFFile::GetXBaseFieldTypeFromZdFieldType(cFieldType) );
         lFieldLength = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetLength());
         if (! ((longRange.first <= lFieldLength) && (lFieldLength <= longRange.second)) )
            ZdException::Generate("field at position %d in field vector has length, %d, which is out of range [%d, %d].", "PackFields", u, lFieldLength, longRange.first, longRange.second);
         itrCurrentSchema->FieldLen = lFieldLength;

         //test decimal count legality
         longRange = DBFFile::DecimalCountRangeForXBaseFieldType( DBFFile::GetXBaseFieldTypeFromZdFieldType(cFieldType), vFields.at(u)->GetLength() );
         lFieldPrecision = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetPrecision());
         if (! ((longRange.first <= lFieldPrecision) && (lFieldPrecision <= longRange.second)) )
            ZdException::Generate("field at position %d in field vector has precision, %d, which is out of range [%d, %d].", "PackFields", u, lFieldPrecision, longRange.first, longRange.second);
         itrCurrentSchema->NoOfDecs = lFieldPrecision;
         }

      //setup the "sentinel" schema:
      itrCurrentSchema = aXBaseFieldDefs + vFields.size();
      itrCurrentSchema->FieldName[0] = 0;//make field-name an empty string
      itrCurrentSchema->Type = 0;
      itrCurrentSchema->FieldLen = 0;
      itrCurrentSchema->NoOfDecs = 0;

      rc = gpDbf->CreateDatabase(ZdFileName(sFilename).GetFullPath(), aXBaseFieldDefs, 0);

      if (rc != XB_NO_ERROR)
         ZdException::Generate("could not create database, \"%c\".", "DBFFile", ZdFileName(sFilename).GetFullPath());
      }
   catch (ZdException & e)
      {
      delete[] aXBaseFieldDefs;

      e.AddCallpath("Create()", "DBFFile");
      throw;
      }
   catch (...)
      {
      delete[] aXBaseFieldDefs;
      throw;
      }
}


// Save 'Record' as the last record in the file.
//<br>require
//<br>  type_conformant_record: dynamic_cast<const DBFRecord *>(&Record) != 0
unsigned long DBFFile::DataAppend  ( const ZdFileRecord &Record )
{
   unsigned long ulResult;
   xbShort rc;
   const DBFRecord * pRecord(dynamic_cast<const DBFRecord *>(&Record));
   try
      {
      if (! pRecord)
         ZdException::Generate("Record is not of type DBFRecord.", "DBFFile");

      pRecord->AppendToDbf(*gpDbf);
      ulResult = GetNumRecords();
      }
   catch (ZdException & e)
      {
      e.AddCallpath("DataAppend()", "DBFFile");
      throw;
      }
   return ulResult;
}

// What are the minimum and maximum possible number of decimals for a field of type
// 'cFieldType' with a length of lFieldLength ?
// When theResult.first == theResult.second, the length is builtin.
//<br>require
//<br>  valid_xbase_field_type:  DBFFile::IsValidXBaseFieldType(cFieldType)
std::pair<long, long> DBFFile::DecimalCountRangeForXBaseFieldType(char cFieldType, long lFieldLength)
{
   std::pair<long, long> theResult;
   try
      {
      CheckXBaseFieldType(cFieldType);

      switch (cFieldType)
         {
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
            ZdException::Generate("Unhandled field type: '%c'", "DBFFile", cFieldType);
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("DecimalCountRangeForXBaseFieldType()", "DBFFile");
      throw;
      }
   return theResult;
}

//Remove all records from this file.
void DBFFile::Empty()
{
   gpDbf->Zap(F_SETLKW);
}

//Flush data to persistent storage.
//Does nothing in this implementation.
void DBFFile::Flush()
{
}

// What are the minimum and maximum possible lengths for a field of type 'cFieldType' ?
// When theResult.first == theResult.second, the length is builtin.
//<br>require
//<br>  valid_xbase_field_type:  DBFFile::IsValidXBaseFieldType(cFieldType)
std::pair<long, long> DBFFile::FieldLengthRangeForXBaseFieldType(char cFieldType)
{
   std::pair<long, long> theResult;
   try
      {
      CheckXBaseFieldType(cFieldType);

      switch (cFieldType)
         {
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
            ZdException::Generate("Unhandled field type: '%c'", "DBFFile", cFieldType);
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("FieldLengthRangeForXBaseFieldType()", "DBFFile");
      throw;
      }
   return theResult;
}

// Returns the (static) file type object for .DBF files.
const ZdFileType & DBFFile::GetFileType() const
{
   return DBFFileType::GetDefaultInstance();
}

//Return a pointer to a newly allocated object.
ZdField * DBFFile::GetNewField() const
{
   ZdField * pResult;
   try
      {
      pResult = new ZdField();
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GetNewField()", "DBFFile");
      throw;
      }
   return pResult;
}

//How many records are currently in this file ?
unsigned long DBFFile::GetNumRecords() const
{
   unsigned long ulResult;
   try
      {
      ulResult = gpDbf->NoOfRecords();
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GetNumRecords()", "DBFFile");
      throw;
      }
   return ulResult;
}

// What is the XB_..._FLD that corresponds to the ZD_..._FLD indicated by 'cZdFieldType' ?
//<br>require
//<br>  valid_zd_field_type:  DBFFile::IsValidZdFieldType(cZdFieldType)
char DBFFile::GetXBaseFieldTypeFromZdFieldType(char cZdFieldType)
{
   char cResult;
   try
      {
      CheckZdFieldType(cZdFieldType);

      switch (cZdFieldType)
         {
         case ZD_ALPHA_FLD :
            cResult = XB_CHAR_FLD;
            break;
         case ZD_BOOLEAN_FLD :
            cResult = XB_LOGICAL_FLD;
            break;
         case ZD_NUMBER_FLD :
            cResult = XB_NUMERIC_FLD;
            break;
         case ZD_DATE_FLD :
            cResult = XB_DATE_FLD;
            break;
         case ZD_BLOB_FLD :
            cResult = XB_MEMO_FLD;
            break;
         default :
            ZdException::Generate("Unhandled field type: '%c'", "DBFFile", cZdFieldType);
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GetXBaseFieldTypeFromZdFieldType()", "DBFFile");
      throw;
      }
   return cResult;
}

// What is the ZD_..._FLD that corresponds to the XB_..._FLD indicated by 'cXBaseFieldType' ?
//<br>require
//<br>  valid_xbase_field_type:  DBFFile::IsValidXBaseFieldType(cXBaseFieldType)
char DBFFile::GetZdFieldTypeFromXBaseFieldType(char cXBaseFieldType)
{
   char cResult;
   try
      {
      CheckXBaseFieldType(cXBaseFieldType);

      switch (cXBaseFieldType)
         {
         case XB_CHAR_FLD :
            cResult = ZD_ALPHA_FLD;
            break;
         case XB_LOGICAL_FLD :
            cResult = ZD_BOOLEAN_FLD;
            break;
         case XB_NUMERIC_FLD :
            cResult = ZD_NUMBER_FLD;
            break;
         case XB_DATE_FLD :
            cResult = ZD_DATE_FLD;
            break;
         case XB_MEMO_FLD :
            cResult = ZD_BLOB_FLD;
            break;
         case XB_FLOAT_FLD :
            cResult = ZD_NUMBER_FLD;
            break;
         default :
            ZdException::Generate("Unhandled field type: '%c'", "DBFFile", cXBaseFieldType);
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GetZdFieldTypeFromXBaseFieldType()", "DBFFile");
      throw;
      }
   return cResult;
}

// Ensure that the SystemRecord holds the data for the record at 'lRecNum'.
// If PRecordBuffer isn't NULL, ensure that it, also, holds the same data.
//<br>require
//<br>  valid_record_index:  (0 <= lRecNum) && (lRecNum < GetNumRecords())
//<br>  type_conformant_record: (!PRecordBuffer) || (dynamic_cast<DBFRecord *>(PRecordBuffer) != 0)
void DBFFile::GotoRecord(unsigned long lRecNum, ZdFileRecord * PRecordBuffer)
{
   DBFRecord * pTempDBFRecord(0);
   try
      {
      CheckRecNum(lRecNum);
      if (PRecordBuffer)
         {
         pTempDBFRecord = dynamic_cast<DBFRecord *>(PRecordBuffer);
         if (! pTempDBFRecord)
            ZdException::Generate("The record buffer isn't of type DBFRecord or descendant.", "DBFFile");
         }

      gpDbf->GetRecord(lRecNum);
      dynamic_cast<DBFRecord *>(GetSystemRecord())->SetAsCurrentDbfRecord(*gpDbf);
      if (pTempDBFRecord)
         pTempDBFRecord->SetAsCurrentDbfRecord(*gpDbf);
      }
   catch (ZdException & e)
      {
      e.AddCallpath("GotoRecord()", "DBFFile");
      throw;
      }
}

// Is the candidate legal for use as the name of a field ?
bool DBFFile::IsLegalFieldname(const ZdString & sCandidate)
{
   bool bResult(false);
   long l;
   try
      {
      long lLength(sCandidate.GetLength());
      char cCurrentChar(sCandidate.GetCharAt(0));
      if ( (0 < lLength) && (lLength <= 10) )//test length
         {
         if ( ('A' <= cCurrentChar) && (cCurrentChar <= 'Z') )//test first character
            {
            bResult = true;
            l = 1;
            while ( bResult && (l < lLength) )//test remaining characters
               {
               cCurrentChar = sCandidate.GetCharAt(l);
               bResult = (
                          (('A' <= cCurrentChar) && (cCurrentChar <= 'Z'))//is uppercase alphabetical
                          || (('0' <= cCurrentChar) && (cCurrentChar <= '9'))//is numeric
                          || (cCurrentChar == '_')//is underscore
                         );
               ++l;
               }
            }
         }
      }
   catch (ZdException & e)
      {
      e.AddCallpath("IsLegalFieldname()", "DBFFile");
      throw;
      }
   return bResult;
}

// Does 'cCandidate' indicate an XB_..._FLD that is used by DBFFile ?
bool DBFFile::IsValidXBaseFieldType(char cCandidate)
{
   bool bResult;

   switch (cCandidate)
      {
      case XB_CHAR_FLD :
      case XB_LOGICAL_FLD :
      case XB_NUMERIC_FLD :
      case XB_DATE_FLD :
      case XB_MEMO_FLD :
      case XB_FLOAT_FLD :
         bResult = true;
         break;
      default :
         bResult = false;
      }

   return bResult;
}

// Does 'cCandidate' indicate a ZD_..._FLD that is used by DBFFile ?
bool DBFFile::IsValidZdFieldType(char cCandidate)
{
   bool bResult;

   switch (cCandidate)
      {
      case ZD_ALPHA_FLD :
      case ZD_BOOLEAN_FLD :
      case ZD_NUMBER_FLD :
      case ZD_DATE_FLD :
      case ZD_BLOB_FLD :
         bResult = true;
         break;
      default :
         bResult = false;
      }

   return bResult;
}

//Open a file.
//<br>require
//<br>  filename_string_not_null:  sFilename != 0
void DBFFile::Open(const char *sFilename, ZdIOFlag Flags, const char * sPassword, const char * sAlternateZDSFile, ZdIniFile *pZDSFile)
{
   xbShort rc;
   ZdFileName fn(sFilename);
   try
      {
      if (! sFilename)
         ZdException::Generate("null pointer: sFilename", "DBFFile");

      OpenSetup(sFilename, Flags);

      if (gpDbf->GetDbfStatus() != XB_CLOSED)
         {
         rc = gpDbf->CloseDatabase(true);
         if (rc != XB_NO_ERROR)
            {
            ZdException::Generate("Could not close file, \"%c\".", "DBFFile", GetFileName());
            }
         }

      rc = gpDbf->OpenDatabase(fn.GetFullPath());
      if (rc != XB_NO_ERROR)
         ZdException::Generate("Could not open file, \"%c\".", "DBFFile", fn.GetFullPath());

      OpenFinish();
      }
   catch (ZdException & e)
      {
      e.AddCallpath("Open()", "DBFFile");
      throw;
      }
}

// Pack the data as tightly as possible.
void DBFFile::PackData(ZdProgressInterface & theProgress)
{
   try
      {
      theProgress.ResetPosition();
      gpDbf->PackDatabase(F_SETLKW/*, function to update the progress*/);
      theProgress.SetPosition(theProgress.GetMaximum());
      }
   catch (ZdException & e)
      {
      e.AddCallpath("PackData()", "DBFFile");
      throw;
      }
}

//Ensure that the vFields is appropriate for input to the function, Create.
//<br>require
//<br>  no_fields_null:  for i in [0, vFields.size()], vFields.at(i) != NULL
void DBFFile::PackFields ( ZdVector<ZdField*> &vFields ) const
{
   unsigned u;
   try
      {
      for (u = 0; u < vFields.size(); u++)
         {
         if (! vFields.at(u))
            ZdException::Generate("null field at position %d in field vector.", "PackFields", u);
         }
      //nothing else needs to be done.
      }
   catch (ZdException & e)
      {
      e.AddCallpath("PackFields()", "DBFFile");
      throw;
      }
}

// Performs any outstanding updates and sets the file so that updates will not be
// cached.
// In DBFFile, I'm not sure if a BlobFile will be created or not, but I'm redefining
// this function to do nothing, just in case.
void DBFFile::PerformExternalUpdates()
{
   try
      {
//      if ( gpBlobFile )
//         gpBlobFile->ClearDelayedFree();
      }
   catch ( ZdException &theException )
      {
      theException.AddCallpath ( "PerformExternalUpdates()", "DBFFile" );
      throw;
      }
}

// Read the record structure from a ".zds".
// DBFFile's don't use "zds"s, so it is empty.
void DBFFile::ReadStructure( ZdIniFile *pAlternateZDSFile )
{
   xbShort rc;
   unsigned u;

   try
      {
      //make sure the .dbf file is open:
      if (gpDbf->GetDbfStatus() == XB_CLOSED)
         {
         rc = gpDbf->OpenDatabase(gFileName.GetFullPath());
         }
      if (rc != XB_NO_ERROR)
         ZdException::Generate("Could not open file: \"%c\".", "DBFFile", gFileName.GetFullPath());

      // Create the standard .ZDS file, if necessary
//      if ( pAlternateZDSFile == 0 )
//         pIniFile = GetIniFile ( GetFileName() );
//      else
//         pIniFile = pAlternateZDSFile;

      // Setup file properties
      gsTitle = gFileName.GetFileName();
//      gsInformation = pSection->GetString("Information");
//      gsInputLayout = pSection->GetString("InputLayout");
//      gwInputLayout = (short) pSection->GetInt("InputLayoutNumber");
//      gwNumberOfPrimaryKeyFields = (unsigned short) pSection->GetInt("PrimaryKeyFields");

      //Setup the relate fields if any
//      gwNumberOfRelateFields = (unsigned short) pSection->GetInt("Number of Relate Fields");
//      for (i = 0; i < gwNumberOfRelateFields; i++)
//         {
//         sKey << ZdString::reset << "Relate Field" << i;
//         gvRelateFields.push_back ( pSection->GetString(sKey) );
//         }

      // Get, decode, and compare the password
      //...

      // Set categories
      //...

      // Read in the flags
      //...

      // Set field info
      gvFields.DeleteAllElements();
      gvFields.resize(gpDbf->FieldCount());
      for (u = 0; u < gvFields.size(); u++)
         {
         gvFields.at(u) =
            new ZdField(
                        gpDbf->GetFieldName(u),
                        u,
                        DBFFile::GetZdFieldTypeFromXBaseFieldType(gpDbf->GetFieldType(u)),
                        gpDbf->GetFieldLen(u),
                        gpDbf->GetFieldDecimal(u)
                       );
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("ReadStructure()", "DBFFile");
      throw;
      }
}

// Locks the file as specified.
//xbDbf has locking, but I haven't figured it out, yet.
bool DBFFile::TryLock ( ZdIOFlag iLock )
{
   bool bRetVal;

   try
      {
//      bRetVal = gFile.TryLock ( iLock );
//
//      if ( bRetVal )
//         ZdFile::Lock ( iLock );
      }
   catch ( ZdException &theException )
      {
      theException.AddCallpath("TryLock()", "DBFFile");
      throw;
      }
//   return bRetVal;
return true;
}

// This function will save the file structure to the zds file. If you specify an
// alternate ZDS filename in pAlternateZDSFile, it's extension will be changed
// to ".ZDS"
// DBFFile's don't use ".zds"s, so this is redefined to do nothing.
void DBFFile::WriteStructure ( ZdIniFile *pAlternateZDSFile, const ZdVector<ZdField*> *pvFields ) const
{
//   try
//      {
//      }
//   catch (ZdException & theException)
//      {
//      theException.AddCallpath("WriteStructure()", "DBFFile");
//      throw;
//      }
}

//ClassDesc Begin DBFFileType
// This class is the descriptor class for DBF files. Since the types of data
// supported by DBF files is fixed, there is no data in this class and every
// instantiation will be identical.
//ClassDesc End

DBFFileType DBFFileType::gDefaultInstance;

DBFFileType *DBFFileType::Clone() const
{
   return new DBFFileType ( *this );
}

// Returns TRUE.
bool DBFFileType::GetCanBeCreated() const
{
   return true;
}

// Fills 'vFillArray' with the fields supported by this file type.
void DBFFileType::GetFieldTypesAllowed ( ZdVector<const ZdFieldType *> &vFillArray ) const
{
   vFillArray.push_back ( &ZdAlphaFieldDesc );
//   vFillArray.push_back ( &ZdBinaryFieldDesc );
   vFillArray.push_back ( &ZdBlobFieldDesc );
   vFillArray.push_back ( &ZdBoolFieldDesc );
   vFillArray.push_back ( &ZdDateFieldDesc );
   vFillArray.push_back ( &ZdLongFieldDesc );
   vFillArray.push_back ( &ZdNumberFieldDesc );
//   vFillArray.push_back ( &ZdShortFieldDesc );
//   vFillArray.push_back ( &ZdStampFieldDesc );
//   vFillArray.push_back ( &ZdTimeFieldDesc );
//   vFillArray.push_back ( &ZdUShortFieldDesc );
//   vFillArray.push_back ( &ZdULongFieldDesc );
}

// Returns the extension used by this file type.
const char * DBFFileType::GetFileTypeExtension() const
{
//   return DBF_EXT;
   return ".dbf";
}

// Returns the english name of this file type.
const char * DBFFileType::GetFileTypeName() const
{
   return "dBASE versions III+, IV";
}

// Returns the representation that DBF file uses for data. (ASCII)
ZdFileStorageFormat DBFFileType::GetStorageFormat() const
{
   return ZDFILESTORAGE_ASCII;
}

// Returns a new DBFFile using the default constructor.
DBFFile *DBFFileType::Instantiate() const
{
   return new DBFFile;
}

// Returns a new TXDFile using the default constructor.
DBFFile *DBFFileType::InstantiateFromStream ( ZdInputStreamInterface &theStream) const
{
   DBFFile *pRetVal = new DBFFile;

   pRetVal->Read ( theStream );

   return pRetVal;
}






