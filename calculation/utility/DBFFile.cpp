//---------------------------------------------------------------------------

#include "xbase/xbase.h"
#pragma hdrstop

#include "DBFFile.h"
//---------------------------------------------------------------------------

//construct
//use an empty field array
DBFSystemRecord::DBFSystemRecord( xbDbf & associatedFile ) : ZdFileRecord(ZdVector<ZdField*>(), 0)
   , gpAssociatedFile( &associatedFile )
{
   try
      {
      if (gpAssociatedFile->GetDbfStatus() == XB_CLOSED)
         ZdException::Generate("The associated file is not open", "DBFSystemRecord");
      }
   catch (ZdException & e)
      {
      e.AddCallpath("constructor(const xbDbf &)", "DBFSystemRecord");
      throw;
      }
}

//destruct
DBFSystemRecord::~DBFSystemRecord()
{
}

//Make all fields blank.
void DBFSystemRecord::Clear()
{
   try
      {
      gpAssociatedFile->BlankRecord();
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("Clear()", "DBFSystemRecord");
      throw;
      }
}

//How many fields are in *this ?
long DBFSystemRecord::GetFieldCount() const
{
   long lResult;
   try
      {
      lResult = gpAssociatedFile->FieldCount();
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldCount()", "DBFSystemRecord");
      throw;
      }
   return lResult;
}

//How many bytes does this field require ?
short DBFSystemRecord::GetFieldLength(unsigned short uwFieldIndex) const
{
   short wResult;
   try
      {
      wResult = gpAssociatedFile->GetFieldLen(uwFieldIndex);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldLength()", "DBFSystemRecord");
      throw;
      }
   return wResult;
}

//What is the "field type" of the field at 'uwFieldIndex' ?
char DBFSystemRecord::GetFieldType(unsigned short uwFieldIndex) const
{
   char cResult;
   try
      {
      cResult = DBFFile::GetZdFieldTypeFromXBaseFieldType(gpAssociatedFile->GetFieldType(uwFieldIndex));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetFieldType()", "DBFSystemRecord");
      throw;
      }
   return cResult;
}

//Does the field at 'uwFieldIndex' contain a "blank" value ?
bool DBFSystemRecord::GetIsBlank(unsigned short uwFieldIndex) const
{
   bool bResult;
   ZdResizableChunk buffer(gpAssociatedFile->GetFieldLen(uwFieldIndex)+1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   const char * pBufBegin;
   const char * pBufEnd;
   try
      {
      gpAssociatedFile->GetRawField(gpAssociatedFile->GetFieldName(uwFieldIndex), buffer.AsCharPtr());
      pBufEnd = pBufBegin + (buffer.GetSize() - 1);
      while ( (pBufBegin < pBufEnd) && bResult )
         {
         bResult = *pBufBegin == ' ';
         ++pBufBegin;
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetIsBlank()", "DBFSystemRecord");
      throw;
      }
   return bResult;
}

//Get the value of the field at 'uwFieldIndex' as an Alpha.  'ulLength' is the number
//of bytes in 'pFieldValue'.
//<br>require
//<br>  valid_length: ulLength >= (GetFieldLength(uwFieldNumber) + 1)
char * DBFSystemRecord::GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long ulLength, bool bStripBlanks) const
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   int i;
   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
      {
      if (ulLength < (unsigned long)(wFieldLength + 1))
         ZdException::Generate("Buffer %d too small in GetAlpha(); need buffer length of %d", "TXDRec", ulLength, wFieldLength + 1 );

      gpAssociatedFile->GetRawField(gpAssociatedFile->GetFieldName(uwFieldNumber), buffer.AsCharPtr());
      ::memcpy(pFieldValue, buffer.AsCharPtr(), wFieldLength);
      pFieldValue[wFieldLength] = 0;
      if (bStripBlanks)
         for (i = wFieldLength - 1; i >= 0 && pFieldValue[i] == ' '; i--)
            pFieldValue[i] = 0;
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetAlpha()", "DBFSystemRecord");
      throw;
      }
   return pFieldValue;
}

//Get the value of the field at 'uwFieldIndex' as a Binary.  Be sure that the buffer
//at pValue has at least GetFieldLength(uwFieldNumber) bytes available!
void DBFSystemRecord::GetBinary( unsigned short uwFieldNumber, void *pValue ) const
{
//   short wFieldLength = GetFieldLength(uwFieldNumber);
//   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
      {
      ZdException::Generate("GetBinary not implemented", "DBFSystemRecord");
//      gpAssociatedFile->GetRawField(gpAssociatedFile->GetFieldName(uwFieldNumber), buffer.AsCharPtr());
//      ::memcpy(pValue, buffer.AsCharPtr(), wFieldLength);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBinary()", "DBFSystemRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldNumber' as a ZdBlob.
void DBFSystemRecord::GetBLOB(unsigned short uwFieldNumber, ZdBlob & theValue) const
{
   try
      {
      ZdResizableChunk databuffer(gpAssociatedFile->GetMemoFieldLen(uwFieldNumber));

      gpAssociatedFile->GetMemoField(uwFieldNumber, databuffer.GetSize(), databuffer.AsCharPtr(), F_SETLKW);
      theValue.SetBlob(databuffer.AsVoidPtr(), databuffer.GetSize());
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBLOB()", "DBFSystemRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
bool DBFSystemRecord::GetBoolean(unsigned short uwFieldNumber) const
{
   bool bResult;

   try
      {
      bResult = gpAssociatedFile->GetLogicalField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetBoolean()", "DBFSystemRecord");
      throw;
      }
   return bResult;
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
ZdDate & DBFSystemRecord::GetDate(unsigned short uwFieldNumber, ZdDate &theDate ) const
{
//It happens that the form of the date in the DBF format, CCYYMMDD, is identical
//to a "raw date" value as viewed by ZdDate.
   try
      {
      ZdResizableChunk buffer(gpAssociatedFile->GetFieldLen(uwFieldNumber)+1);//extra byte because GetRawField sets the extra byte to NULL
      gpAssociatedFile->GetRawField(uwFieldNumber, buffer.AsCharPtr());
      theDate.SetRawDate(buffer.AsCharPtr());
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetDate()", "DBFSystemRecord");
      throw;
      }
   return theDate;
}

//Get the value of the field at 'uwFieldIndex' as a long.
long DBFSystemRecord::GetLong(unsigned short uwFieldNumber) const
{
   long lResult;

   try
      {
      lResult = gpAssociatedFile->GetLongField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetLong()", "DBFSystemRecord");
      throw;
      }
   return lResult;
}

//Get the value of the field at 'uwFieldIndex' as a double.
double DBFSystemRecord::GetNumber(unsigned short uwFieldNumber) const
{
   double dResult;

   try
      {
      if (gpAssociatedFile->GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
         dResult = gpAssociatedFile->GetDoubleField(uwFieldNumber);
      else
         dResult = gpAssociatedFile->GetFloatField(uwFieldNumber);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetNumber()", "DBFSystemRecord");
      throw;
      }
   return dResult;
}

//Get the value of the field at 'uwFieldIndex' as a short.
short DBFSystemRecord::GetShort(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetShort not implemented.  Try GetLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetShort()", "DBFSystemRecord");
      throw;
      }
   return 0;
}

//Get the value of the field at 'uwFieldIndex' as a ZdTime.
ZdTime & DBFSystemRecord::GetTime(unsigned short uwFieldNumber, ZdTime &FieldValue) const
{
   try
      {
      ZdException::Generate("GetTime not implemented.  Try GetDate ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetTime()", "DBFSystemRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as a ZdTimestamp.
ZdTimestamp & DBFSystemRecord::GetTimestamp(unsigned short uwFieldNumber, ZdTimestamp &FieldValue) const
{
   try
      {
      ZdException::Generate("GetTimestamp not implemented.  Try GetDate ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetTimestamp()", "DBFSystemRecord");
      throw;
      }
}

//Get the value of the field at 'uwFieldIndex' as an unsigned long.
unsigned long DBFSystemRecord::GetUnsignedLong(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetUnsignedLong not implemented.  Try GetLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetUnsignedLong()", "DBFSystemRecord");
      throw;
      }
   return 0;
}

//Get the value of the field at 'uwFieldIndex' as an unsigned short.
unsigned short DBFSystemRecord::GetUnsignedShort(unsigned short wFieldNumber) const
{
   try
      {
      ZdException::Generate("GetUnsignedShort not implemented.  Try GetLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("GetUnsignedShort()", "DBFSystemRecord");
      throw;
      }
   return 0;
}

//Put an Alpha value into the field at 'uwFieldIndex'.
//<br>require
//<br>  field_value_short_enough:  std::strlen(pFieldValue) <= GetFieldLength(uwFieldNumber)
void DBFSystemRecord::PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue)
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   try
      {
      if ((unsigned)wFieldLength < std::strlen(pFieldValue))
         ZdException::Generate( "String Value, \"%s\", of length, %d, must not be longer than %d.", "DBFSystemRecord", pFieldValue, strlen(pFieldValue), wFieldLength );

      gpAssociatedFile->PutField(uwFieldNumber, pFieldValue);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutAlpha()", "DBFSystemRecord");
      throw;
      }
}

//Put a Binary value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutBinary ( unsigned short wFieldNumber, const void *pValue )
{
   try
      {
      ZdException::Generate("PutBinary not implemented", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBinary()", "DBFSystemRecord");
      throw;
      }
}

//Put a "blank" value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutBlank(unsigned short uwFieldNumber)
{
   try
      {
      short wFieldLength(GetFieldLength(uwFieldNumber));
      ZdString sTemp((unsigned long)wFieldLength);
      switch (gpAssociatedFile->GetFieldType(uwFieldNumber))
         {
         case XB_CHAR_FLD :
         case XB_NUMERIC_FLD :
         case XB_DATE_FLD :
         case XB_FLOAT_FLD :
            //set all spaces
            sTemp.Clear();
            sTemp.Insert(' ', 0, wFieldLength);
            gpAssociatedFile->PutField(uwFieldNumber, sTemp.GetCString());
            break;
         case XB_LOGICAL_FLD :
            //set to question mark
            sTemp = '?';
            gpAssociatedFile->PutField(uwFieldNumber, sTemp.GetCString());
            break;
         case XB_MEMO_FLD :
            //set to an empty memo
            gpAssociatedFile->UpdateMemoData(uwFieldNumber, 0, (char*)0, F_SETLKW);
            break;
         }
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBlank()", "DBFSystemRecord");
      throw;
      }
}

//Put a ZdBlob value into the field at 'uwFieldNumber'.
void DBFSystemRecord::PutBLOB(unsigned short uwFieldNumber, const ZdBlob & theValue)
{
   try
      {
      gpAssociatedFile->UpdateMemoData(uwFieldNumber, theValue.GetLength(), (const char *)(theValue.GetBlob()), F_SETLKW);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBLOB()", "DBFSystemRecord");
      throw;
      }
}

//Put a boolean value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutBoolean(unsigned short uwFieldNumber, bool bValue)
{
   char sTrue[2] = "T";//we could use any of these: T, t, Y, y
   char sFalse[2] = "F";//we could use any of these: F, f, N, n
   try
      {
      gpAssociatedFile->PutField(uwFieldNumber, (bValue ? sTrue : sFalse));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBoolean()", "DBFSystemRecord");
      throw;
      }
}

//Put a date value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutDate( unsigned short uwFieldNumber, const ZdDate &theDate )
{
//It happens that the form of the date in the DBF format, CCYYMMDD, is identical
//to a "raw date" value as viewed by ZdDate.
   try
      {
      ZdString sRawDate;
      theDate.RetrieveRawDate(sRawDate);
      gpAssociatedFile->PutField(uwFieldNumber, sRawDate.GetCString());
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutDate()", "DBFSystemRecord");
      throw;
      }
}

//Put a long value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutLong(unsigned short uwFieldNumber, long lFieldValue)
{
   try
      {
      gpAssociatedFile->PutLongField(uwFieldNumber, lFieldValue);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutLong()", "DBFSystemRecord");
      throw;
      }
}

//Put a numeric value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutNumber(unsigned short uwFieldNumber, double dFieldValue)
{
   try
      {
      if (gpAssociatedFile->GetFieldType(uwFieldNumber) == XB_NUMERIC_FLD)
         gpAssociatedFile->PutDoubleField(uwFieldNumber, dFieldValue);
      else
         gpAssociatedFile->PutFloatField(uwFieldNumber, static_cast<float>(dFieldValue));
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutNumber()", "DBFSystemRecord");
      throw;
      }
}

//Put a short value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutShort(unsigned short uwFieldNumber, short wFieldValue)
{
   try
      {
      ZdException::Generate("PutShort not implemented.  Try PutLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutShort()", "DBFSystemRecord");
      throw;
      }
}

//Put a ZdTime value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutTime(unsigned short uwFieldNumber, const ZdTime &FieldValue)
{
   try
      {
      ZdException::Generate("PutTime not implemented.  Try PutDate ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTime()", "DBFSystemRecord");
      throw;
      }
}

//Put a ZdTimestamp value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutTimestamp(unsigned short uwFieldNumber, const ZdTimestamp &FieldValue) 
{
   try
      {
      ZdException::Generate("PutTimestamp not implemented.  Try PutDate ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTimestamp()", "DBFSystemRecord");
      throw;
      }
}

//Put an unsigned long value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutUnsignedLong(unsigned short uwFieldNumber, unsigned long ulFieldValue)
{
   try
      {
      ZdException::Generate("PutUnsignedLong not implemented.  Try PutLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutUnsignedLong()", "DBFSystemRecord");
      throw;
      }
}

//Put an unsigned short value into the field at 'uwFieldIndex'.
void DBFSystemRecord::PutUnsignedShort(unsigned short uwFieldNumber, unsigned short uwFieldValue) 
{
   try
      {
      ZdException::Generate("PutUnsignedShort not implemented.  Try PutLong ?", "DBFSystemRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutUnsignedShort()", "DBFSystemRecord");
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

      if (gPSystemRecord)
         {
         delete gPSystemRecord;
         gPSystemRecord = new DBFSystemRecord(*gpDbf);
         }

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

//Ensure that the SystemRecord holds the data for the record at 'lRecNum'.
//<br>require
//<br>  valid_record_index:  (0 <= lRecNum) && (lRecNum < GetNumRecords())
//<br>  no_supplemental_buffer: PRecordBuffer == NULL
void DBFFile::GotoRecord(unsigned long lRecNum, ZdFileRecord * PRecordBuffer)
{
   try
      {
      CheckRecNum(lRecNum);
      if (PRecordBuffer)
         ZdException::Generate("DBFFile doesn't yet support separate record objects.", "DBFFile");

      gpDbf->GetRecord(lRecNum);
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

      gbIsClassLocalRecordRequest = true;//ensure that calls to GetNewRecord don't fail
      OpenFinish();
      gbIsClassLocalRecordRequest = false;//ensure that calls to GetNewRecord do fail
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
   try
      {
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






