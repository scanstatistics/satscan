//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "scanffile.h"

//ClassDesc Begin ScanfRecord
// The ScanfRecord implements ZdFileRecord for Scanf (whitespace separated value)
// files.
//ClassDesc End ScanfRecord


//construct
//use an empty field array
//<br>require
//<br>  associated_file_is_open:
ScanfRecord::ScanfRecord(ScanfFile & associatedFile, const ZdVector<ZdField *> & vFields) : ZdFileRecord( vFields, 0 )
{
   try
   {
      gvValues.resize(associatedFile.GetNumFields());
      for (short w = 0; w < associatedFile.GetNumFields(); ++w)
      {
         ZdFieldValue & value(gvValues.at(w));
         value.SetType(associatedFile.GetFieldInfo(w)->GetType());
      }
      //register this record with 'associatedFile'
   }
   catch (ZdException & e)
   {
      e.AddCallpath("constructor(ScanfFile &)", "ScanfRecord");
      throw;
   }
}

//destruct
ScanfRecord::~ScanfRecord()
{
}

//Make all fields blank.
void ScanfRecord::Clear()
{
   try
   {
      //ZdException::Generate("Blank field values not supported", "ScanfRecord");
      for (size_t t=0; t < gvValues.size(); t++)
         gvValues[t].AsZdString().Clear();
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("Clear()", "ScanfRecord");
      throw;
   }
}

//Does the field at 'uwFieldIndex' contain a "blank" value ?
//Blank values are not supported in ScanfRecord.
bool ScanfRecord::GetIsBlank(unsigned short uwFieldIndex) const
{
//   try
//   {
//   }
//   catch (ZdException & theException)
//   {
//      theException.AddCallpath("GetIsBlank()", "ScanfRecord");
//      throw;
//   }
   return false;
}

//Get the value of the field at 'uwFieldIndex' as an Alpha.  'ulLength' is the number
//of bytes in 'pFieldValue'.
//If the type of the field at uwFieldNumber is not ZD_ALPHA_FLD, the buffer will be an empty string.
//<br>require
//<br>  valid_length: ulLength >= (GetFieldLength(uwFieldNumber) + 1)
char * ScanfRecord::GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long ulLength, bool bStripBlanks) const
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   int i;
   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
   {
      if (ulLength < (unsigned long)(wFieldLength + 1))
         ZdException::Generate("Buffer %d too small in GetAlpha(); need buffer length of %d", "TXDRec", ulLength, wFieldLength + 1 );

      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_ALPHA_FLD)
      {
         std::strncpy(pFieldValue, value.AsZdString().GetCString(), wFieldLength);
      }
      else
         pFieldValue[0] = 0;

      pFieldValue[wFieldLength] = 0;//null-terminate
      if (bStripBlanks)
         for (i = wFieldLength - 1; i >= 0 && pFieldValue[i] == ' '; i--)
            pFieldValue[i] = 0;
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetAlpha()", "ScanfRecord");
      throw;
   }
   return pFieldValue;
}

//Get the value of the field at 'uwFieldIndex' as a Binary.  Be sure that the buffer
//at pValue has at least GetFieldLength(uwFieldNumber) bytes available!
void ScanfRecord::GetBinary( unsigned short uwFieldNumber, void *pValue ) const
{
//   short wFieldLength = GetFieldLength(uwFieldNumber);
//   ZdResizableChunk buffer(wFieldLength + 1);//GetRawField puts a NULL in the byte following the last byte of the field, so we must allocate that byte
   try
   {
      ZdException::Generate("GetBinary not implemented", "ScanfRecord");
//      gpAssociatedFile->GetRawField(gpAssociatedFile->GetFieldName(uwFieldNumber), buffer.AsCharPtr());
//      ::memcpy(pValue, buffer.AsCharPtr(), wFieldLength);
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetBinary()", "ScanfRecord");
      throw;
   }
}

//Get the value of the field at 'uwFieldNumber' as a ZdBlob.
//If the type of the field at uwFieldNumber is not ZD_BLOB_FLD, theValue will be unchanged.
void ScanfRecord::GetBLOB(unsigned short uwFieldNumber, ZdBlob & theValue) const
{
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_BLOB_FLD)
      {
         theValue = value.AsZdBlob();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetBLOB()", "ScanfRecord");
      throw;
   }
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
//If the type of the field at uwFieldNumber is not ZD_BOOLEAN_FLD, the result will be 'false'.
bool ScanfRecord::GetBoolean(unsigned short uwFieldNumber) const
{
   bool bResult(false);

   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_BOOLEAN_FLD)
      {
         bResult = value.AsBool();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetBoolean()", "ScanfRecord");
      throw;
   }
   return bResult;
}

//Get the value of the field at 'uwFieldIndex' as a boolean.
//If the type of the field at uwFieldNumber is not ZD_DATE_FLD, theDate will be unchanged.
ZdDate & ScanfRecord::GetDate(unsigned short uwFieldNumber, ZdDate &theDate ) const
{
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_DATE_FLD)
      {
         theDate = value.AsZdDate();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetDate()", "ScanfRecord");
      throw;
   }
   return theDate;
}

//Get the value of the field at 'uwFieldIndex' as a long.
//If the type of the field at uwFieldNumber is not ZD_LONG_FLD, the result will be zero.
long ScanfRecord::GetLong(unsigned short uwFieldNumber) const
{
   long lResult(0);

   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_LONG_FLD)
      {
         lResult = value.AsLong();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetLong()", "ScanfRecord");
      throw;
   }
   return lResult;
}

//Get the value of the field at 'uwFieldIndex' as a double.
//If the type of the field at uwFieldNumber is not ZD_NUMBER_FLD, the result will be zero.
double ScanfRecord::GetNumber(unsigned short uwFieldNumber) const
{
   double dResult(0);

   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_NUMBER_FLD)
      {
         dResult = value.AsDouble();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetNumber()", "ScanfRecord");
      throw;
   }
   return dResult;
}

//Get the value of the field at 'uwFieldIndex' as a short.
//If the type of the field at uwFieldNumber is not ZD_SHORT_FLD, the result will be zero.
short ScanfRecord::GetShort(unsigned short uwFieldNumber) const
{
   short wResult(0);

   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_SHORT_FLD)
      {
         wResult = value.AsShort();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetShort()", "ScanfRecord");
      throw;
   }
   return wResult;
}

//Get the value of the field at 'uwFieldIndex' as a ZdTime.
//If the type of the field at uwFieldNumber is not ZD_TIME_FLD, theValue will be unchanged.
ZdTime & ScanfRecord::GetTime(unsigned short uwFieldNumber, ZdTime & theValue) const
{
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_TIME_FLD)
      {
         theValue = value.AsZdTime();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetTime()", "ScanfRecord");
      throw;
   }
   return theValue;
}

//Get the value of the field at 'uwFieldIndex' as a ZdTimestamp.
//If the type of the field at uwFieldNumber is not ZD_STAMP_FLD, theValue will be unchanged.
ZdTimestamp & ScanfRecord::GetTimestamp(unsigned short uwFieldNumber, ZdTimestamp & theValue) const
{
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_STAMP_FLD)
      {
         theValue = value.AsZdTimestamp();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetTimestamp()", "ScanfRecord");
      throw;
   }
   return theValue;
}

//Get the value of the field at 'uwFieldIndex' as an unsigned long.
//If the type of the field at uwFieldNumber is not ZD_ULONG_FLD, the result will be zero.
unsigned long ScanfRecord::GetUnsignedLong(unsigned short uwFieldNumber) const
{
   unsigned long ulResult(0);
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_ULONG_FLD)
      {
         ulResult = value.AsUnsignedLong();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetUnsignedLong()", "ScanfRecord");
      throw;
   }
   return ulResult;
}

//Get the value of the field at 'uwFieldIndex' as an unsigned short.
//If the type of the field at uwFieldNumber is not ZD_USHORT_FLD, the result will be zero.
unsigned short ScanfRecord::GetUnsignedShort(unsigned short uwFieldNumber) const
{
   unsigned short uwResult(0);
   try
   {
      const ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_USHORT_FLD)
      {
         uwResult = value.AsUnsignedShort();
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("GetUnsignedShort()", "ScanfRecord");
      throw;
   }
   return uwResult;
}

//Put an Alpha value into the field at 'uwFieldIndex'.
//<br>require
//<br>  field_value_short_enough:  std::strlen(pFieldValue) <= GetFieldLength(uwFieldNumber)
void ScanfRecord::PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue)
{
   short wFieldLength = GetFieldLength(uwFieldNumber);
   try
   {
      if ((unsigned)wFieldLength < std::strlen(pFieldValue))
         ZdException::Generate( "String Value, \"%s\", of length, %d, must not be longer than %d.", "ScanfRecord", pFieldValue, strlen(pFieldValue), wFieldLength );

      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_ALPHA_FLD)
      {
         value.AsZdString() = pFieldValue;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutAlpha()", "ScanfRecord");
      throw;
   }
}

//Put a Binary value into the field at 'uwFieldIndex'.
void ScanfRecord::PutBinary ( unsigned short wFieldNumber, const void *pValue )
{
   try
      {
      ZdException::Generate("PutBinary not implemented", "ScanfRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutBinary()", "ScanfRecord");
      throw;
      }
}

//Put a "blank" value into the field at 'uwFieldIndex'.
//Blank values not supported in ScanfRecord.
void ScanfRecord::PutBlank(unsigned short uwFieldNumber)
{
   try
   {
      ZdException::Generate("Blank values not supported", "ScanfRecord");
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutBlank()", "ScanfRecord");
      throw;
   }
}

//Put a ZdBlob value into the field at 'uwFieldNumber'.
void ScanfRecord::PutBLOB(unsigned short uwFieldNumber, const ZdBlob & theValue)
{
   try
   {
      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_BLOB_FLD)
      {
         value.AsZdBlob() = theValue;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutBLOB()", "ScanfRecord");
      throw;
   }
}

//Put a boolean value into the field at 'uwFieldIndex'.
void ScanfRecord::PutBoolean(unsigned short uwFieldNumber, bool bValue)
{
   try
   {
      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_BOOLEAN_FLD)
      {
         value.AsBool() = bValue;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutBoolean()", "ScanfRecord");
      throw;
   }
}

//Put a date value into the field at 'uwFieldIndex'.
void ScanfRecord::PutDate( unsigned short uwFieldNumber, const ZdDate &theDate )
{
   try
   {
      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_DATE_FLD)
      {
         value.AsZdDate() = theDate;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutDate()", "ScanfRecord");
      throw;
   }
}

//Put a long value into the field at 'uwFieldIndex'.
void ScanfRecord::PutLong(unsigned short uwFieldNumber, long lFieldValue)
{
   try
   {
      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_LONG_FLD)
      {
         value.AsLong() = lFieldValue;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutLong()", "ScanfRecord");
      throw;
   }
}

//Put a numeric value into the field at 'uwFieldIndex'.
void ScanfRecord::PutNumber(unsigned short uwFieldNumber, double dFieldValue)
{
   try
   {
      ZdFieldValue & value(gvValues.at(uwFieldNumber));
      if (value.GetType() == ZD_NUMBER_FLD)
      {
         value.AsDouble() = dFieldValue;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutNumber()", "ScanfRecord");
      throw;
   }
}

//Put a short value into the field at 'uwFieldIndex'.
void ScanfRecord::PutShort(unsigned short uwFieldNumber, short wFieldValue)
{
   try
      {
      ZdException::Generate("PutShort not implemented.  Try PutLong ?", "ScanfRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutShort()", "ScanfRecord");
      throw;
      }
}

//Put a ZdTime value into the field at 'uwFieldIndex'.
void ScanfRecord::PutTime(unsigned short uwFieldNumber, const ZdTime &FieldValue)
{
   try
      {
      ZdException::Generate("PutTime not implemented.  Try PutDate ?", "ScanfRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTime()", "ScanfRecord");
      throw;
      }
}

//Put a ZdTimestamp value into the field at 'uwFieldIndex'.
void ScanfRecord::PutTimestamp(unsigned short uwFieldNumber, const ZdTimestamp &FieldValue)
{
   try
      {
      ZdException::Generate("PutTimestamp not implemented.  Try PutDate ?", "ScanfRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutTimestamp()", "ScanfRecord");
      throw;
      }
}

//Put an unsigned long value into the field at 'uwFieldIndex'.
void ScanfRecord::PutUnsignedLong(unsigned short uwFieldNumber, unsigned long ulFieldValue)
{
   try
      {
      ZdException::Generate("PutUnsignedLong not implemented.  Try PutLong ?", "ScanfRecord");
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("PutUnsignedLong()", "ScanfRecord");
      throw;
      }
}

//Put an unsigned short value into the field at 'uwFieldIndex'.
void ScanfRecord::PutUnsignedShort(unsigned short uwFieldNumber, unsigned short uwFieldValue)
{
   try
   {
      ZdException::Generate("PutUnsignedShort not implemented.  Try PutLong ?", "ScanfRecord");
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("PutUnsignedShort()", "ScanfRecord");
      throw;
   }
}

//Assumptions:
//1. ignores sPassword
//2. assumes all fields are of type ZD_ALPHA_FLD.
void ScanfRecord::ReadRecord(ZdInputStreamInterface &theFile, const char *sPassword)
{
//   ZdResizableChunk recordBuffer(ScanfRecord::MAX_RECORD_BUFFER_LENGTH);
   ZdResizableChunk argumentBuffer(ScanfRecord::MAX_RECORD_BUFFER_LENGTH);
   ZdString sScanBuffer;
   ZdString sFormatString;
   int iMatchedCharCount(0);
   int iScannedParmCount(0);
   int iCurrentFieldIndex;
   const char * pReadHead;
   try
   {
      sFormatString << "%" << ScanfRecord::MAX_RECORD_BUFFER_LENGTH - 1 << "s";
      sFormatString << "%n";

      //setup for reading field values:
      iCurrentFieldIndex = 0;
      theFile.ReadLine(sScanBuffer);
      pReadHead = sScanBuffer.GetCString();
      //read first field value:
      iScannedParmCount = std::sscanf(pReadHead, sFormatString.GetCString(), argumentBuffer.AsCharPtr(), &iMatchedCharCount);
      //load and read remaining field values:
      while ( (iCurrentFieldIndex < GetFieldCount()) && (iScannedParmCount != EOF) )
      {
         gvValues[iCurrentFieldIndex].AsZdString() = argumentBuffer.AsCharPtr();

         ++iCurrentFieldIndex;
         pReadHead += iMatchedCharCount;
         iScannedParmCount = std::sscanf(pReadHead, sFormatString.GetCString(), argumentBuffer.AsCharPtr(), &iMatchedCharCount);
      }
      //make sure remaining fields have empty string values:
      while (iCurrentFieldIndex < GetFieldCount())
      {
         gvValues[iCurrentFieldIndex].AsZdString().Clear();
         ++iCurrentFieldIndex;
      }
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("ReadRecord()", "ScanfRecord");
      throw;
   }
}

//Assumptions:
//1. ignores sPassword
//2. assumes all fields are of type ZD_ALPHA_FLD.
void ScanfRecord::WriteRecord(ZdOutputStreamInterface &theFile, const char *sPassword) const
{
   ZdResizableChunk recordBuffer(ScanfRecord::MAX_RECORD_BUFFER_LENGTH);
   ZdResizableChunk argumentBuffer(recordBuffer);
   ZdString sOutputString;
   char * pReadHead;
   try
   {
      //output first field:
      sOutputString.printf("%s", gvValues[0].AsZdString().GetCString());
      theFile << sOutputString.GetCString();

      //output remaining fields:
      for (int i = 1; i < this->GetFieldCount(); ++i)
      {
         sOutputString.printf(" %s", gvValues[i].AsZdString().GetCString());
         theFile << sOutputString.GetCString();
      }

      theFile.WriteLine();
   }
   catch (ZdException & theException)
   {
      theException.AddCallpath("WriteRecord()", "ScanfRecord");
      throw;
   }
}




//ClassDesc Begin ScanfFile
// ScanfFile exposes files that can be read using "fscanf" through the ZdFile interface. 
//ClassDesc End ScanfFile

////////////////////////////////////////////////////////////////////////////////
// ScanfFile functions:
////////////////////////////////////////////////////////////////////////////////

//construct
ScanfFile::ScanfFile(const char * sFilename, ZdIOFlag Flags)
{
   try
   {
//      gbFirstLineContainsFieldnames  = false;
      glCurrentRecordNumber = 0;
      gulNumRecords = 0;

      if (sFilename)
         Open(sFilename, Flags);
   }
   catch (ZdException & e)
   {
      e.AddCallpath("constructor(const char *)", "ScanfFile");
      throw;
   }
}

//destruct
ScanfFile::~ScanfFile()
{
   try
   {
      Close();
//      delete gPSystemRecord;
   }
   catch (...)
   {//cannot throw anything
   }
}

// Throw an exception if the candidate isn't legal for use as the name of a field.
void ScanfFile::Assert_IsLegalFieldname(const ZdString & sCandidate)
{
   if (! ScanfFile::IsLegalFieldname(sCandidate))
      ZdException::Generate("The string, \"%c\", is not legal for use as the name of a field in a Scanf file.", "ScanfFile");
}

// Internal function which scans backwards through the file looking for the
// specified character and using cQuote as a quote character. This scan function
// positions the pointer ON THE FOUND CHARACTER.
unsigned int ScanfFile::BackScan ( char cFindMe, char cQuote ) {
   char         sBuffer[256]; // Buffer for file input
   unsigned int uiPosition;
   int          iCurrent;
   bool         bFound;
   bool         bQuote;

   try {
      uiPosition = static_cast<unsigned int> ( gFile.Tell() );

      bFound = bQuote = false;

      while ( !bFound && uiPosition ) {
         if ( uiPosition >= 256 ) {
            iCurrent = gFile.ReadFrom ( uiPosition - 256, sBuffer, 256, false );
            uiPosition -= 256;
         }
         else {
            iCurrent = gFile.ReadFrom ( 0, sBuffer, uiPosition, false );
            uiPosition = 0;
         }

         // Scan through the current buffer for the delimiter
         while ( iCurrent && !bFound ) {
            iCurrent--;

            // Logic of these statements :
            //
            // 1) IF we are NOT QUOTED and the CURRENT CHARACTER IS THE TARGET,
            //    we have found the character.
            //
            // 2) IF the CURRENT CHARACTER IS A QUOTE, invert QUOTED. This is
            //    an XOR condition.
            //
            bFound = ( !bQuote && ( sBuffer[iCurrent] == cFindMe ) );
            bQuote = ( bQuote ^ ( sBuffer[iCurrent] == cQuote ) );
         } // end of scanning loop
      } // end of big loop

      uiPosition += iCurrent;
      gFile.Seek ( uiPosition );
   }
   catch ( ZdException &theException ) {
      theException.AddCallpath ( "BackScan()", "ScanfFile" );
      throw;
   }
   return uiPosition;
}

// Internal functions which caluclates the number of records in the file and
// returns that value.
unsigned long ScanfFile::CalculateNumberOfRecords ( char cQuote ) const {
   char          sBuffer[8192];
   unsigned long ulRetVal = 0;      // Count of newlines
   unsigned int  uiPosition = 0;    // Current position
   int          iTrailingNewLines = 0;  // NUmber of trailing newlines we've encountered
   int          iBlockLength;   // Length of current block
   int          iCurrent;       // Index in current block
   bool         bQuote = false;         // Currently quoted?

   //--- NOTE : I could've used FwdScan() for this function, but it turned out
   //---        to be too slow.

   try {
      // Read in the next block
      iBlockLength = gFile.ReadFrom ( uiPosition, sBuffer, 8192, false );
      uiPosition += iBlockLength;

      while ( iBlockLength ) {
         for ( iCurrent = 0; iCurrent < iBlockLength; iCurrent++ ) {
            if ( !bQuote && ( sBuffer[iCurrent] == '\n' ) ) {
               iTrailingNewLines++;
               ulRetVal++;
            }
            else
               if ( sBuffer[iCurrent] != '\r' )
                  iTrailingNewLines = 0;

            bQuote = ( bQuote ^ ( sBuffer[iCurrent] == cQuote ) );
         } // end of scanning loop

         // Read in the next block
         iBlockLength = gFile.ReadFrom ( uiPosition, sBuffer, 8192, false );
         uiPosition += iBlockLength;
      } // end of processing loop

      // Fix-up the return value
      ulRetVal -= iTrailingNewLines;
//      ulRetVal += ( gbFirstLineContainsFieldNames ) ? 0 : 1;
      ulRetVal += 1;//right now, the first line doesn't contain field names
   }
   catch ( ZdException &theException ) {
      theException.AddCallpath ( "CalculateNumberOfRecords()", "ScanfFile" );
      throw;
   }
   return ulRetVal;
}

// Generate an exception if 'ulRecNum' is out of range.
void ScanfFile::CheckRecNum(unsigned long ulRecNum) const
{
   if (! ( (0 < ulRecNum) && (ulRecNum <= GetNumRecords()) ))
      ZdException::Generate("The record index (value=%d) is out of range, [%d, %d].", "ScanfFile", ulRecNum, 1, GetNumRecords());
}

//Close the file.
void ScanfFile::Close()
{
   try
   {
      ZdFile::Close();
      gFile.Close();
   }
   catch (ZdException & e)
   {
      e.AddCallpath("Close()", "ScanfFile");
      throw;
   }
}

/*
// Create a new "Scanf" file whose records have 'vFields.size()' fields,
// each interpreted according to 'vFields.at(i)'.
// Currently, ignores uwNumPrimaryFields and bBuildIndexes.
//<br>require
//<br>  fields_exist:  for i in [0, vFields.size()-1], vFields.at(i) != NULL
//<br>  fields_have_valid_type:  for i in [0, vFields.size()-1], ScanfFile::IsValidZdFieldType(vFields.at(i)->GetType())
//<br>  fields_have_legal_names:  each name cannot contain any newline character
//<br>  fields_have_legal_lengths:  each field's length, if not considered "fixed", must be appropriate to its type
//<br>  fields_have_legal_precisions:  each field's precision, if not considered "fixed", must be appropriate to its type and length
void ScanfFile::Create(const char * sFilename, ZdVector<ZdField*> &vFields, unsigned short uwNumPrimaryKeyFields, bool bBuildIndexes )
{
   ZdFileName theHeaderName;
   ZdFileName theDataName;
   unsigned u;
   char cFieldType;
   std::pair<long, long> longRange;
   long lFieldLength;
   long lFieldPrecision;

   try
   {
      Close();//we wouldn't want to overwrite a currently open file.
      
      //check passed-in values
      for (u = 0; u < vFields.size(); u++)
      {
         //test existence of each field:
         if (! vFields.at(u))
            ZdException::Generate("null field at position %d in field vector.", "PackFields", u);

         //test field name legality
         ScanfFile::Assert_IsLegalFieldname(vFields.at(u)->GetName());

         //test field type validity
         cFieldType = vFields.at(u)->GetType();
         ScanfFile::Assert_IsValidZdFieldType(cFieldType);

         //test field length legality
//         longRange = ScanfFile::FieldLengthRangeForXBaseFieldType(ScanfFile::GetXBaseFieldTypeFromZdFieldType(cFieldType) );
//         lFieldLength = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetLength());
//         if (! ((longRange.first <= lFieldLength) && (lFieldLength <= longRange.second)) )
//            ZdException::Generate("field at position %d in field vector has length, %d, which is out of range [%d, %d].", "PackFields", u, lFieldLength, longRange.first, longRange.second);

         //test decimal count legality
//         longRange = ScanfFile::DecimalCountRangeForXBaseFieldType( ScanfFile::GetXBaseFieldTypeFromZdFieldType(cFieldType), vFields.at(u)->GetLength() );
//         lFieldPrecision = ((longRange.first == longRange.second) ? longRange.first : vFields.at(u)->GetPrecision());
//         if (! ((longRange.first <= lFieldPrecision) && (lFieldPrecision <= longRange.second)) )
//            ZdException::Generate("field at position %d in field vector has precision, %d, which is out of range [%d, %d].", "PackFields", u, lFieldPrecision, longRange.first, longRange.second);
      }

      theDataName.SetFullPath(sFilename);
      theHeaderName.SetFullPath(sFilename);
      theHeaderName.SetExtension(ZDS_EXT);



      
      gFileName.SetFullPath ( sFilename );
      gFileName.SetExtension(ZDS_EXT);

      if (ZdIOInterface::Exists(gFileName.GetFullPath()))
         ZdGenerateFileException("Header file \"%s\" already exists", "TXDFile", gFileName.GetFullPath(), gFileName.GetFullPath() );

      ZdIniFile IniFile(gFileName.GetFullPath(), true);

      // WriteStructure doesn't know about us; need to change the fields
//      gwNumberOfPrimaryKeyFields = uwNumPrimaryKeyFields;

//      if ( bBuildIndexes )
//         BuildEmptyIndexes ( &gFileName );
      WriteStructure ( &IniFile, &vFields );

//      gwNumberOfPrimaryKeyFields = uwOriginalKeys;

      pFileInfoSection = IniFile.GetSection ( "FileInfo" );
      pFileInfoSection->SetInt ( "NumberOfFields", vFields.size() );

      pFileInfoSection->SetInt ( "RecordLength", 0 );

      IniFile.Write();

      if (ZdIOInterface::Exists(gFileName.GetFullPath()))
         ZdGenerateFileException("Data file \"%s\" already exists", "TXDFile", gFileName.GetFullPath(), gFileName.GetFullPath() );

      ZdIO DataFile(gFileName.GetFullPath(), ZDIO_OPEN_CREATE | ZDIO_OPEN_WRITE);

      // Restore the original
//      gwNumberOfPrimaryKeyFields = uwOriginalKeys;
//      gFileName = originalFileName;

   }
   catch (ZdException & e)
   {
      e.AddCallpath("Create()", "ScanfFile");
      throw;
   }
   catch (...)
   {
      throw;
   }
}
*/

// Save 'Record' as the last record in the file.
//<br>require
//<br>  type_conformant_record: dynamic_cast<const ScanfRecord *>(&Record) != 0
unsigned long ScanfFile::DataAppend  ( const ZdFileRecord &Record )
{
   long lRecordPosition;
   try
   {
      gFile.Seek ( 0, SEEK_END );
      lRecordPosition = gFile.Tell();
      Record.WriteRecord ( gFile );
      gFile.Seek(lRecordPosition, SEEK_SET);

      gulNumRecords++;
      glCurrentRecordNumber = gulNumRecords;
   }
   catch (ZdException & e)
   {
      e.AddCallpath("DataAppend()", "ScanfFile");
      throw;
   }
   return gulNumRecords;
}

// Save the values in the record at 'ulPosition' into *pCurrentValue, if it is non-NULL.
// Overwrite the values in that record with the values in 'Record'.
//<br>require
//<br>  valid_position: (0 < ulPosition) && (ulPosition <= GetNumRecords())
//<br>  type_conformant_record: dynamic_cast<const ScanfRecord *>(&Record) != 0
//<br>  type_conformant_current_value_record: (pCurrentValue != 0) implies (dynamic_cast<ScanfRecord *>(pCurrentValue) != 0)
//unsigned long ScanfFile::DataModify( unsigned long ulPosition, const ZdFileRecord &Record, ZdFileRecord *pCurrentValue )
//{
//   try
//   {
//   }
//   catch (ZdException & e)
//   {
//      e.AddCallpath("DataModify()", "ScanfFile");
//      throw;
//   }
//   return ulPosition;
//}

// What are the minimum and maximum possible number of decimals for a field of type
// 'cFieldType' with a length of lFieldLength ?
// When theResult.first == theResult.second, the range is builtin.
//<br>require
//<br>  valid_xbase_field_type:  ScanfFile::IsValidZdFieldType(cFieldType)
std::pair<long, long> ScanfFile::DecimalCountRangeForZdFieldType(char cFieldType, long lFieldLength)
{
   std::pair<long, long> theResult;
   try
   {
      Assert_IsValidZdFieldType(cFieldType);

      switch (cFieldType)
      {
//         case ZD_CURRENCY_FLD :
         case ZD_ALPHA_FLD :
            theResult.first = 0;
            theResult.second = 0;
            break;
//         case ZD_BLOB_FLD :
//            theResult.first = 0;
//            theResult.second = 0;
//            break;
//         case ZD_DATE_FLD :
//            theResult.first = 0;
//            theResult.second = 0;
//            break;
//         case ZD_LONG_FLD :
//         case ZD_ULONG_FLD :
//         case ZD_BOOLEAN_FLD :
//            theResult.first = 0;
//            theResult.second = 0;
//            break;
//         case ZD_NUMBER_FLD :
//            theResult.first = 0;
//            theResult.second = lFieldLength - 2;//leave enough space for the decimal point and one digit to its left
//            break;
//         case ZD_SHORT_FLD :
//         case ZD_STAMP_FLD :
//         case ZD_TIME_FLD :
//         case ZD_USHORT_FLD :
//         case ZD_BINARY_FLD :
         default :
            ZdException::Generate("Unhandled field type: '%c'", "ScanfFile", cFieldType);
      }
   }
   catch (ZdException & e)
   {
      e.AddCallpath("DecimalCountRangeForZdFieldType()", "ScanfFile");
      throw;
   }
   return theResult;
}

//Flush data to persistent storage.
//
void ScanfFile::Flush()
{
   gFile.Flush();
}

// What are the minimum and maximum possible lengths for a field of type 'cFieldType' ?
// When theResult.first == theResult.second, the length is builtin.
//<br>require
//<br>  valid_xbase_field_type:  ScanfFile::IsValidZdFieldType(cFieldType)
std::pair<long, long> ScanfFile::FieldLengthRangeForZdFieldType(char cFieldType)
{
   std::pair<long, long> theResult;
   try
   {
      Assert_IsValidZdFieldType(cFieldType);

      switch (cFieldType)
      {
//         case ZD_CURRENCY_FLD :
         case ZD_ALPHA_FLD :
            theResult.first = 0;//no minimum
            theResult.second = 0;//no maximum
            break;
//         case ZD_BLOB_FLD :
//            theResult.first = 0;
//            theResult.second = 0;
//            break;
//         case ZD_DATE_FLD :
//            theResult.first = 8;
//            theResult.second = 8;
//            break;
//         case ZD_LONG_FLD :
//         case ZD_ULONG_FLD :
//         case ZD_BOOLEAN_FLD :
//            theResult.first = 0;
//            theResult.second = 0;
//            break;
//         case ZD_NUMBER_FLD :
//            theResult.first = 1;
//            theResult.second = 0;//no maximum
//            break;
//         case ZD_SHORT_FLD :
//         case ZD_STAMP_FLD :
//         case ZD_TIME_FLD :
//         case ZD_USHORT_FLD :
//         case ZD_BINARY_FLD :
         default :
            ZdException::Generate("Unhandled field type: '%c'", "ScanfFile", cFieldType);
      }
   }
   catch (ZdException & e)
   {
      e.AddCallpath("FieldLengthRangeForZdFieldType()", "ScanfFile");
      throw;
   }
   return theResult;
}

// Internal function which scans forewards through the file looking for the
// specified character and using cQuote as a quote character. This scan function
// positions the pointer ON THE CHARACTER FOLLOWING the found character.
unsigned int ScanfFile::FwdScan ( char cFindMe, char cQuote ) {
   char         sBuffer[256]; // Buffer for file input
   unsigned int uiPosition;   // Position in file
   int          iBlockLength; // Length of current block
   int          iCurrent;     // Index in current block
   bool         bFound;       // Character found?
   bool         bQuote;       // Currently quoted?

   try {
      uiPosition = static_cast<unsigned int> ( gFile.Tell() );

      bFound = bQuote = false;

      while ( !bFound ) {
         // Read in the next block
         iBlockLength = gFile.ReadFrom ( uiPosition, sBuffer, 256, false );
         uiPosition += iBlockLength;

         if ( !iBlockLength )
            ZdGenerateException ( "Could not find '%c'; hit the end of the file", "ScanfFile", cFindMe );

         // Scan through the current buffer for the delimiter.
         for ( iCurrent = 0; ( iCurrent < iBlockLength ) && !bFound; iCurrent++ ) {
            // Logic of these statements :
            //
            // 1) IF we are NOT QUOTED and the CURRENT CHARACTER IS THE TARGET,
            //    we have found the character.
            //
            // 2) IF the CURRENT CHARACTER IS A QUOTE, invert QUOTED. This is
            //    an XOR condition.
            //
            bFound = ( !bQuote && ( sBuffer[iCurrent] == cFindMe ) );
            bQuote = ( bQuote ^ ( sBuffer[iCurrent] == cQuote ) );
         } // end of scanning loop
      } // end of big loop

      // Remember, that iCurrent is incremented one extra time by the for loop.
      uiPosition -= iBlockLength - iCurrent;
      gFile.Seek ( uiPosition );
   }
   catch ( ZdException &theException ) {
      theException.AddCallpath ( "FwdScan()", "ScanfFile" );
      throw;
   }
   return uiPosition;
}

// What is the index of the record where the file is currently positioned ?
// If GetNumRecords() == 0, result is 0, otherwise, 0 < result <= GetNumRecords().
unsigned long ScanfFile::GetCurrentRecordNumber() const
{
   return glCurrentRecordNumber;
}

// Returns the (static) file type object for Scanf files.
const ZdFileType & ScanfFile::GetFileType() const
{
   return ScanfFileType::GetDefaultInstance();
}

//Return a pointer to a newly allocated object.
ZdField * ScanfFile::GetNewField() const
{
   ZdField * pResult;
   try
   {
      pResult = new ZdField();
      pResult->SetType(ZD_ALPHA_FLD);
   }
   catch (ZdException & e)
   {
      e.AddCallpath("GetNewField()", "ScanfFile");
      throw;
   }
   return pResult;
}

//How many records are currently in this file ?
unsigned long ScanfFile::GetNumRecords() const
{
//   try
//   {
//   }
//   catch (ZdException & e)
//   {
//      e.AddCallpath("GetNumRecords()", "ScanfFile");
//      throw;
//   }
   return gulNumRecords;
}

// Ensure that the SystemRecord holds the data for the record at 'lRecNum'.
// If PRecordBuffer isn't NULL, ensure that it, also, holds the same data.
//<br>require
//<br>  valid_record_index:  (0 <= lRecNum) && (lRecNum < GetNumRecords())
//<br>  type_conformant_record: (!PRecordBuffer) || (dynamic_cast<ScanfRecord *>(pRecordBuffer) != 0)
void ScanfFile::GotoRecord(unsigned long lRecNum, ZdFileRecord * pRecordBuffer)
{
   ScanfRecord * pTempScanfRecord(0);
   unsigned long  lHeadPosition;

   try
   {
      CheckRecNum(lRecNum);
      if (pRecordBuffer)
      {
         pTempScanfRecord = dynamic_cast<ScanfRecord *>(pRecordBuffer);
         if (! pTempScanfRecord)
            ZdException::Generate("The record buffer isn't of type ScanfRecord or descendant.", "ScanfFile");
      }

      PositionFilePointerToRecord(lRecNum);
      lHeadPosition = gFile.Tell();

      //read into the system record
      GetSystemRecord()->ReadRecord(gFile);
      //read into the passed-in record
      if (pTempScanfRecord)
      {
         gFile.Seek(lHeadPosition);
         pTempScanfRecord->ReadRecord(gFile);
      }
      gFile.Seek(lHeadPosition);
      glCurrentRecordNumber = lRecNum;
   }
   catch (ZdException & e)
   {
      e.AddCallpath("GotoRecord()", "ScanfFile");
      throw;
   }
}

// Is the candidate legal for use as the name of a field ?
bool ScanfFile::IsLegalFieldname(const ZdString & sCandidate)
{
   bool bResult(true);
   long l;
   try
      {
      l = sCandidate.Find('\n', 0);
      bResult = bResult && (l < 0);
      l = sCandidate.Find('\r', 0);
      bResult = bResult && (l < 0);
      }
   catch (ZdException & e)
      {
      e.AddCallpath("IsLegalFieldname()", "ScanfFile");
      throw;
      }
   return bResult;
}

// Does 'cCandidate' indicate a ZD_..._FLD that is supported by ScanfFile ?
bool ScanfFile::IsValidZdFieldType(char cCandidate)
{
   bool bResult;

   switch (cCandidate)
      {
//      case ZD_CURRENCY_FLD :
      case ZD_ALPHA_FLD :
//      case ZD_BLOB_FLD :
//      case ZD_DATE_FLD :
//      case ZD_LONG_FLD :
//      case ZD_ULONG_FLD :
//      case ZD_BOOLEAN_FLD :
//      case ZD_NUMBER_FLD :
//      case ZD_SHORT_FLD :
//      case ZD_STAMP_FLD :
//      case ZD_TIME_FLD :
//      case ZD_USHORT_FLD :
//      case ZD_BINARY_FLD :
         bResult = true;
         break;
      default :
         bResult = false;
      }

   return bResult;
}

// Creates a ZDS which indicates all ALPHA fields
ZdIniFile *ScanfFile::MakeAlphaZDS ( const char *sFileName, bool bHasFieldNames ) const {
   ZdIniFile                   *pIniFile(0);
   ZdString                     sName, sScanBuffer, sFormatString;
   ZdPointerVector<ZdField>     vFields;
   ZdIO                         tempFile;
   int                          iMatchedCharCount(0), iScannedParmCount(0), iCurrentFieldIndex(0);
   const char                 * pReadHead;
   ZdResizableChunk             argumentBuffer(ScanfRecord::MAX_RECORD_BUFFER_LENGTH);

   try {
     tempFile.Open( sFileName, ZDIO_OPEN_READ );          // Temporary io object used to access the file
     sFormatString << "%" << ScanfRecord::MAX_RECORD_BUFFER_LENGTH - 1 << "s" << "%n";
     //setup for reading field values:
     tempFile.ReadLine(sScanBuffer);
     pReadHead = sScanBuffer.GetCString();
     //read first field value:
     iScannedParmCount = std::sscanf(pReadHead, sFormatString.GetCString(), argumentBuffer.AsCharPtr(), &iMatchedCharCount);
     //load and read remaining field values:
     while ((iScannedParmCount != EOF)) {
         if (bHasFieldNames)
           vFields.push_back(new ZdField(argumentBuffer.AsCharPtr(), iCurrentFieldIndex, ZD_ALPHA_FLD, ZD_MAXFIELD_LEN));
         else {
           sName.printf("Field %d", iCurrentFieldIndex + 1);
           vFields.push_back(new ZdField(sName, iCurrentFieldIndex, ZD_ALPHA_FLD, ZD_MAXFIELD_LEN));
         }

         ++iCurrentFieldIndex;
         pReadHead += iMatchedCharCount;
         iScannedParmCount = std::sscanf(pReadHead, sFormatString.GetCString(), argumentBuffer.AsCharPtr(), &iMatchedCharCount);
      }

     pIniFile = new ZdIniFile;
     PackFields ( vFields );
     WriteStructure ( pIniFile, &vFields );
   }
   catch ( ZdException &theException ) {
      delete pIniFile;
      theException.AddCallpath ( "MakeAlphaZDS()", "ScanfFile" );
      throw;
   }
   return pIniFile;
}

// Internal function to back up the file pointer by ulAmount records.
void ScanfFile::MovePointerBackward ( unsigned long ulAmount ) {
   unsigned long   ulPosition;       // Current file pointer
   char            cGroupDelimiter('\0');  // Group delimiter character
   unsigned long   i;                // Index variable

   try {
//      cGroupDelimiter = gFileDescriptor.GetGroupDelimiter();
      ulPosition      = gFile.Tell();
      i               = 0;

      // Need to start off one character back to avoid picking up our '\n'
      if ( ulPosition ) {
         gFile.Seek ( ulPosition - 1 );
         while ( ( i < ulAmount ) && ulPosition ) {
            ulPosition = BackScan ( '\n', cGroupDelimiter );
            i++;
         } // end of while
      }

      if ( ulPosition )
         gFile.Seek ( ulPosition + 1 );
   }
   catch ( ZdException  &theException ) {
      theException.AddCallpath ( "MovePointerBackward()", "ScanfFile" );
      throw;
   }
}

// Internal function which advances the file pointer by ulAmount records. Note
// that this function handles relative offsets, not absolutes.
void ScanfFile::MovePointerForward ( unsigned long ulAmount ) {
   char            cGroupDelimiter('\0');  // Group delimiter
   unsigned long   i;                // Current file pointer

   try {
//      cGroupDelimiter = gFileDescriptor.GetGroupDelimiter();
      for ( i = 0; i < ulAmount; i++ )
         FwdScan ( '\n', cGroupDelimiter );
   }
   catch ( ZdException  &theException ) {
      theException.AddCallpath ( "MovePointerForward()", "ScanfFile" );
      throw;
   }
}

//Open a file.
//<br>require
//<br>  filename_string_not_null:  sFilename != 0
void ScanfFile::Open(const char *sFilename, ZdIOFlag Flags, const char * sPassword, const char * sAlternateZDSFile, ZdIniFile *pZDSFile)
{
   ZdFileName fn(sFilename);
   try
   {
      if (! sFilename)
         ZdException::Generate("null pointer: sFilename", "ScanfFile");

      ZdFile::OpenSetup(sFilename, Flags, sAlternateZDSFile, pZDSFile);
      gFile.Open (sFilename, Flags);
      gulNumRecords = CalculateNumberOfRecords ('\0');
//      glCurrentRecordNumber = ( gbFirstLineContainsFieldnames ) ? 0 : 1;  // We are on the first byte of the file
      glCurrentRecordNumber = 1;  // We are on the first byte of the file

      OpenFinish();
   }
   catch (ZdException & e)
   {
      e.AddCallpath("Open()", "ScanfFile");
      throw;
   }
}

// Opens a file using all ALPHA fields.
void ScanfFile::OpenAsAlpha (const char *sFileName, ZdIOFlag Flags , bool bHasFieldNames ) {
   std::auto_ptr<ZdIniFile>  pGuessIni;  // Pointer to the guessed ini file
   try
   {
      pGuessIni.reset ( MakeAlphaZDS ( sFileName, bHasFieldNames ) );
      Open (sFileName , Flags , 0 , 0 , pGuessIni.get() );
   }
   catch (ZdException &theException) {
      theException.AddCallpath ( "GuessOpen()","ScanfFile" );
      throw theException;
   }
}

// Pack the data as tightly as possible.
//void ScanfFile::PackData(ZdProgressInterface & theProgress)
//{
//   try
//   {
//      theProgress.SetPosition(theProgress.GetMaximum());
//      theProgress.IndicateProcessSuccess();
//   }
//   catch (ZdException & e)
//   {
//      e.AddCallpath("PackData()", "ScanfFile");
//      throw;
//   }
//}

//Ensure that the vFields is appropriate for input to the function, Create.
//<br>require
//<br>  no_fields_null:  for i in [0, vFields.size()], vFields.at(i) != NULL
void ScanfFile::PackFields ( ZdVector<ZdField*> &vFields ) const
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
      e.AddCallpath("PackFields()", "ScanfFile");
      throw;
      }
}

// Performs any outstanding updates and sets the file so that updates will not be
// cached.
// In ScanfFile, I'm not sure if a BlobFile will be created or not, but I'm redefining
// this function to do nothing, just in case.
void ScanfFile::PerformExternalUpdates()
{
   try
      {
//      if ( gpBlobFile )
//         gpBlobFile->ClearDelayedFree();
      }
   catch ( ZdException &theException )
      {
      theException.AddCallpath ( "PerformExternalUpdates()", "ScanfFile" );
      throw;
      }
}

// Internal function to move the pointer to the specified record.
void ScanfFile::PositionFilePointerToRecord ( unsigned long ulRecord ) {
   try {
      if ( ulRecord > GetCurrentRecordNumber() )
         MovePointerForward ( ulRecord - GetCurrentRecordNumber() );
      else if ( ulRecord < GetCurrentRecordNumber() )
         MovePointerBackward ( GetCurrentRecordNumber() - ulRecord );
   }
   catch ( ZdException &theException ) {
      theException.AddCallpath ( "PositionFilePointerToRecord()", "ScanfFile" );
      throw;
   }
}

// Setup a filter for this field based on the knowledge currently available.
// I've only implemented this for NUMBER fields, because they are the only problematic
// ones, so far.  Also, we might discover that the filter needs a few more properties
// set than just the ones we're setting right now. --bws 16Sep2002
void ScanfFile::SetupDefaultFilterForField(ZdField & theField)
{
   try
      {
      switch (theField.GetType())
         {
         case ZD_NUMBER_FLD :
            {
            ZdNumberFilter tempFilter;
            tempFilter.SetDecimalNum(theField.GetPrecision());
            theField.SetFilter(tempFilter);
            }
         }
      }
   catch ( ZdException &theException )
      {
      theException.AddCallpath("SetupDefaultFilterForField()", "ScanfFile");
      throw;
      }
}

// Locks the file as specified.
//???? has locking, but I haven't figured it out, yet.
bool ScanfFile::TryLock ( ZdIOFlag iLock )
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
      theException.AddCallpath("TryLock()", "ScanfFile");
      throw;
      }
//   return bRetVal;
return true;
}

// This function will save the file structure to the zds file. If you specify an
// alternate ZDS filename in pAlternateZDSFile, it's extension will be changed
// to ".ZDS"
// ScanfFile's don't use ".zds"s, so this is redefined to do nothing.
void ScanfFile::WriteStructure ( ZdIniFile *pAlternateZDSFile, const ZdVector<ZdField*> *pvFields ) const
{
   try
      {
      ZdFile::WriteStructure (pAlternateZDSFile, pvFields);
      }
   catch (ZdException & theException)
      {
      theException.AddCallpath("WriteStructure()", "ScanfFile");
      throw;
      }
}

//ClassDesc Begin ScanfFileType
// This class is the descriptor class for Scanf files. Since the types of data
// supported by Scanf files is fixed, there is no data in this class and every
// instantiation will be identical.
//ClassDesc End

//ScanfFileType ScanfFileType::gDefaultInstance;

ScanfFileType *ScanfFileType::Clone() const
{
   return new ScanfFileType ( *this );
}

// Returns TRUE.
bool ScanfFileType::GetCanBeCreated() const
{
   return true;
}

// Fills 'vFillArray' with the fields supported by this file type.
void ScanfFileType::GetFieldTypesAllowed ( ZdVector<const ZdFieldType *> &vFillArray ) const
{
   vFillArray.push_back ( &ZdAlphaFieldDesc );
//   vFillArray.push_back ( &ZdBinaryFieldDesc );
//   vFillArray.push_back ( &ZdBlobFieldDesc );
//   vFillArray.push_back ( &ZdBoolFieldDesc );
//   vFillArray.push_back ( &ZdDateFieldDesc );
//   vFillArray.push_back ( &ZdLongFieldDesc );
//   vFillArray.push_back ( &ZdNumberFieldDesc );
//   vFillArray.push_back ( &ZdShortFieldDesc );
//   vFillArray.push_back ( &ZdStampFieldDesc );
//   vFillArray.push_back ( &ZdTimeFieldDesc );
//   vFillArray.push_back ( &ZdUShortFieldDesc );
//   vFillArray.push_back ( &ZdULongFieldDesc );
}

// Returns the extension used by this file type.
const char * ScanfFileType::GetFileTypeExtension() const
{
   return ".wsv";
}

// Returns the english name of this file type.
const char * ScanfFileType::GetFileTypeName() const
{
   return "Whitespace Separated Values";
}

// Returns the representation that Scanf file uses for data. (ASCII)
ZdFileStorageFormat ScanfFileType::GetStorageFormat() const
{
   return ZDFILESTORAGE_ASCII;
}

// Returns a new ScanfFile using the default constructor.
ScanfFile *ScanfFileType::Instantiate() const
{
   return new ScanfFile;
}

// Returns a new ScanfFile using the default constructor.
ScanfFile *ScanfFileType::InstantiateFromStream ( ZdInputStreamInterface &theStream) const
{
//   ScanfFile *pRetVal = new ScanfFile;
//   pRetVal->Read ( theStream );
//   return pRetVal;
   ZdException::Generate("Cannot instantiate a ScanfFile from stream", "ScanfFileType");
   return 0;
}

//Here it is!
ScanfFileType  ZdScanfFileType;

