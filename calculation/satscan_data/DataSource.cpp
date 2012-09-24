//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "SSException.h"
#include "FileName.h"

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::GetNewDataSourceObject(const std::string& sSourceFilename, BasePrint& Print, bool bAssumeASCII) {
    FileName theFile(sSourceFilename.c_str());
    if (!bAssumeASCII && !stricmp(theFile.getExtension().c_str(), dBaseFile::GetFileTypeExtension()))
      //Though ZdFile hierarchy has other files types, most are not formats used outside IMS.
      //There are 3 that could be used: dBase, comma separated, and scanf.
      //The scanf version could possibly replace AsciiFileDataSource but is likely slower.
      //The comma separated version would probably work but user would have to use default
      //field delimiter (comma) and group delimiter (double quote).
      //The dBase version is already used to create output files and is a common format. 
      return new dBaseFileDataSource(sSourceFilename);
    else
      return new AsciiFileDataSource(sSourceFilename, Print);
}

//******************* class AsciiFileDataSource ********************************

/** Returns number of words in string -- this could be better  */
long AsciiFileDataSource::StringParser::GetNumberWords() {
  long   iWords=0;

  while (GetWord(iWords) != 0) ++iWords;
  return iWords;
}

/** Returns wWordIndex + 1 'th word in associated string.
    If wWordIndex is greater than number of words, NULL pointer returned. */
const char * AsciiFileDataSource::StringParser::GetWord(long wWordIndex) {
  bool          inwd;
  int           wdlen;
  long          w = wWordIndex;
  const char  * cp2, * cp3;

  //short cut if this word has already been read
  if (wWordIndex == gwCurrentWordIndex) return gsWord.c_str();

  if (gwCurrentWordIndex == -1 || wWordIndex < gwCurrentWordIndex)
    //we have to start from beginning of string
    gcp = gpParseLine->c_str();
  else
    //else we can scan X more words from current position
    w = wWordIndex - gwCurrentWordIndex;

  /* ignore spaces at start of line */
  while(*gcp != '\0' && isspace(*gcp)) ++gcp;

  /* find start of word */
  inwd = !isDelimiter(*gcp);
  while (*gcp != '\0' && (w > 0 || !inwd)) {
       if (inwd == isDelimiter(*gcp)) { /* entered or exited a word */
         inwd = !inwd;
         if (inwd) /* if entered a word, count it */
           if (--w == 0)
             break;
       }
       ++gcp; /* next character */
  }

  /* handle underflow */
  if (*gcp == '\0') return 0;

  /* find end of word */
  cp2 = gcp + 1;
  while (*cp2 != '\0' &&  !isDelimiter(*cp2)) ++cp2;
  wdlen = cp2 - gcp;
  gsWord.assign (gcp, wdlen);
  cp3 = gsWord.c_str();
  //check that word is ascii characters
  while (*cp3 != '\0') {
       //Attempt to snare files that are ASCII - sometimes users think Word and Excel files will work.
       //If character is not printable ASCII nor possibly part of the extended set, which contains characters
       //common to other languages such as: á or ü, then throw exception. Note that premitting all of the
       //extended set does permit the original problem to possibly creep back in but we'd rather have that than
       //prevent user from using a data file where there really isn't a problem.
       if (!(isprint(*cp3) || (unsigned char)(*cp3) > 127))
         ThrowAsciiException();
       ++cp3;
  }
  gwCurrentWordIndex = wWordIndex;
  return gsWord.c_str();
}

/** sets current parsing string -- returns indication of whether string contains any words. */
bool AsciiFileDataSource::StringParser::SetString(std::string& sParseLine) {
   gwCurrentWordIndex=-1; //clear word index
   gpParseLine = &sParseLine;
   gcp = gpParseLine->c_str();
   return HasWords();
}

void AsciiFileDataSource::StringParser::ThrowAsciiException() {
  throw resolvable_error("Error: The %s contains data that is not ASCII formatted.\n"
                         "       Please see 'ASCII Input File Format' in the user guide for help.\n",
                         gPrint.GetImpliedFileTypeString().c_str());
}

/** constructor */
AsciiFileDataSource::AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print, const char cDelimiter)
                    :DataSource(), glReadCount(0), gPrint(Print), gStringParser(new StringParser(Print, cDelimiter)) {
  try {
    gSourceFile.open(sSourceFilename.c_str());
    if (!gSourceFile)
      throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","AsciiFileDataSource");
    throw;
  }
}

/** Re-positions file cursor to beginning of file. */
void AsciiFileDataSource::GotoFirstRecord() {
  glReadCount = 0;
  gSourceFile.clear();
  gSourceFile.seekg(0L, std::ios::beg);
}

/** Attempts to read line from source and parse into 'words'. If read count is zero, first
    checks file is not UNICODE by looking for byte-order mark -- throwing an exception if found.
    Lines that contain no words are skipped and scanning continues to next line in file. Returns
    indication of whether record buffer contains 'words'.*/
bool AsciiFileDataSource::ReadRecord() {
  if (!glReadCount) {
    // Get the byte-order mark, if there is one
    unsigned char bom[4];
    gSourceFile.read(reinterpret_cast<char*>(bom), 4);
    //Since we don't know what the endian was on the machine that created the file we
    //are reading, we'll need to check both ways.
    if ((bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf) ||             // utf-8
        (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xfe && bom[3] == 0xff) || // UTF-32, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0 && bom[3] == 0) || // UTF-32, little-endian
        (bom[0] == 0xfe && bom[1] == 0xff) ||                               // UTF-16, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe))                                 // UTF-16, little-endian
      ThrowUnicodeException();
    gSourceFile.seekg(0L, std::ios::beg);
  }
  gsReadBuffer.clear();
  //while (getlinePortable(gSourceFile, gsReadBuffer) && !gStringParser->SetString(gsReadBuffer)) ++glReadCount;

  bool isBlank = true;
  while (isBlank && getlinePortable(gSourceFile, gsReadBuffer)) {
      isBlank = !gStringParser->SetString(gsReadBuffer);
      if (isBlank) {
          tripBlankRecordFlag();
          ++glReadCount;
      }
  }

  ++glReadCount;
  return (gsReadBuffer.size() > 0 && gStringParser->HasWords());
}

void AsciiFileDataSource::ThrowUnicodeException() {
  throw resolvable_error("Error: The %s contains data that is Unicode formatted.\n"
                         "       Please see 'ASCII Input File Format' in the user guide for help.\n",
                         gPrint.GetImpliedFileTypeString().c_str());
}

//******************* class dBaseFileDataSource ***********************************

/** constructor */
dBaseFileDataSource::dBaseFileDataSource(const std::string& sSourceFilename)
                 :DataSource(), gwCurrentFieldIndex(-1), gbFirstRead(true), glCurrentRecord(0) {
  try {
    gSourceFile.reset(new dBaseFile());
    gSourceFile->Open(sSourceFilename.c_str());
    glNumRecords = gSourceFile->GetNumRecords();
  }
  catch (prg_exception & x) {
    throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
  }
}

/** destructor */
dBaseFileDataSource::~dBaseFileDataSource() {}

/** Returns current record index. */
long dBaseFileDataSource::GetCurrentRecordIndex() const {
  return static_cast<long>(glCurrentRecord);
}

/** Returns the number of fields in record buffer. */
long dBaseFileDataSource::GetNumValues() {
  return gSourceFile->GetNumFields();
}

/** Returns iFieldIndex'th field value from current record. */
const char * dBaseFileDataSource::GetValueAt(long iFieldIndex) {
  if (gwCurrentFieldIndex != iFieldIndex) {
    if (iFieldIndex > (long)(gSourceFile->GetNumFields() - 1))
      return 0;
    gSourceFile->GetSystemRecord()->GetFieldValue(iFieldIndex, gsValue);
    if (gSourceFile->GetSystemRecord()->GetFieldType(iFieldIndex) == FieldValue::DATE_FLD && gsValue.size() == SaTScan::Timestamp::DATE_FLD_LEN) {
      //format date fields -- read process currently expects yyyy/mm/dd
      gsValue.insert(4, "/");
      gsValue.insert(7, "/");
    }
    gwCurrentFieldIndex = iFieldIndex;
  }
  return gsValue.c_str();
}

/** Positions read cursor to first record. */
void dBaseFileDataSource::GotoFirstRecord() {
  if (glNumRecords) {
    gSourceFile->GotoRecord(1);
    glCurrentRecord = 1;
    gbFirstRead = false;
    gwCurrentFieldIndex=-1;
  }  
}

/** Either reads first file record in file or next after current record. */
bool dBaseFileDataSource::ReadRecord() {
  if (glCurrentRecord >= glNumRecords) return false;
  gwCurrentFieldIndex=-1;
  if (gbFirstRead)
    GotoFirstRecord();
  else
    gSourceFile->GotoRecord(++glCurrentRecord);
  return true;  
}

