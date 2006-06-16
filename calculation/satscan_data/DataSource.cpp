//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "SSException.h"

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::GetNewDataSourceObject(const std::string& sSourceFilename, BasePrint& Print, bool bAssumeASCII) {
   long             lIndex=0;
   bool             bFoundFile=false;
   vecZdFileTypes * pFileTypeArray = ZdGetFileTypeArray();
   ZdFileType     * pFileType = 0;
   ZdFileName       theFile(sSourceFilename.c_str());

   if (!bAssumeASCII) {
     while (!bFoundFile && (lIndex < (long)pFileTypeArray->size())) {
           pFileType = (*pFileTypeArray)[lIndex];
           if (!stricmp(theFile.GetExtension(), pFileType->GetFileTypeExtension()))
              bFoundFile = true;
           lIndex++;
     }
   }  

    if (bFoundFile && !stricmp(theFile.GetExtension(), ".dbf"))
      //Though ZdFile hierarchy has other files types, most are not formats used outside IMS.
      //There are 3 that could be used: dBase, comma separated, and scanf.
      //The scanf version could possibly replace AsciiFileDataSource but is likely slower.
      //The comma separated version would probably work but user would have to use default
      //field delimiter (comma) and group delimiter (double quote).
      //The dBase version is already used to create output files and is a common format. 
      return new ZdFileDataSource(sSourceFilename, *pFileType);
    else
      return new AsciiFileDataSource(sSourceFilename, Print);
}

//******************* class AsciiFileDataSource ********************************

/** Returns number of words in string -- this could be better  */
short AsciiFileDataSource::StringParser::GetNumberWords() {
  short   iWords=0;

  while (GetWord(iWords) != 0) ++iWords;
  return iWords;
}

/** Returns wWordIndex + 1 'th word in associated string.
    If wWordIndex is greater than number of words, NULL pointer returned. */
const char * AsciiFileDataSource::StringParser::GetWord(short wWordIndex) {
  int           inwd, wdlen;
  short         w = wWordIndex;
  const char  * cp = gpParseLine->c_str();
  const char  * cp2;

  //short cut if this word has already been read
  if (wWordIndex == gwCurrentWordIndex) return gsWord.c_str();

  /* ignore spaces at start of line */
  while(*cp != '\0' && isspace(*cp)) ++cp;

  /* find start of word */
  inwd = !isspace(*cp);
  while (*cp != '\0' && (w > 0 || !inwd)) {
       if (inwd == !!(isspace(*cp))) { /* entered or exited a word */
         inwd = !inwd;
         if (inwd) /* if entered a word, count it */
           if (--w == 0)
             break;
       }
       ++cp; /* next character */
  }

  /* handle underflow */
  if (*cp == '\0') return 0;

  /* find end of word */
  cp2 = cp + 1;
  while (*cp2 != '\0' &&  !isspace(*cp2)) ++cp2;
  wdlen = cp2 - cp;
  gsWord.assign (cp, wdlen);
  cp = gsWord.c_str();
  //check that word is ascii characters
  while (*cp != '\0') {
       //Attempt to snare files that are ASCII - sometimes users think Word and Excel files will work.
       //If character is not printable ASCII nor possibly part of the extended set, which contains characters
       //common to other languages such as: á or ü, then throw exception. Note that premitting all of the
       //extended set does permit the original problem to possibly creep back in but we'd rather have that than
       //prevent user from using a data file where there really isn't a problem.
       if (!(isprint(*cp) || (unsigned char)(*cp) > 127))
         ThrowAsciiException();
       ++cp;
  }
  gwCurrentWordIndex = wWordIndex;
  return gsWord.c_str();
}

/** sets current parsing string -- returns indication of whether string contains any words. */
bool AsciiFileDataSource::StringParser::SetString(const std::string& sParseLine) {
   gwCurrentWordIndex=-1; //clear word index
   gpParseLine = &sParseLine;
   return HasWords();
}

void AsciiFileDataSource::StringParser::ThrowAsciiException() {
  GenerateResolvableException("Error: The %s contains data that is not ASCII formatted.\n"
                              "       Please see 'ASCII Input File Format' in the user guide for help.\n",
                              "ThrowAsciiException()", gPrint.GetImpliedFileTypeString().c_str());
}

/** constructor */
AsciiFileDataSource::AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print)
                    :DataSource(), glReadCount(0), gPrint(Print), gStringParser(new StringParser(Print)) {
  try {
    gSourceFile.open(sSourceFilename.c_str());
    if (!gSourceFile)
      GenerateResolvableException("Error: Could not open file:\n'%s'.\n", "AsciiFileDataSource()", sSourceFilename.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()","AsciiFileDataSource");
    throw;
  }
}

/** Re-positions file cursor to beginning of file. */
void AsciiFileDataSource::GotoFirstRecord() {
  glReadCount = 0;
  gSourceFile.clear();
  gSourceFile.seekg(0L, ios::beg);
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
    gSourceFile.seekg(0L, ios::beg);
  }
  gsReadBuffer.clear();
  while (std::getline(gSourceFile, gsReadBuffer) && !gStringParser->SetString(gsReadBuffer)) ++glReadCount;
  ++glReadCount;
  return (gsReadBuffer.size() > 0);
}

void AsciiFileDataSource::ThrowUnicodeException() {
  GenerateResolvableException("Error: The %s contains data that is Unicode formatted.\n"
                              "       Please see 'ASCII Input File Format' in the user guide for help.\n",
                              "ThrowUnicodeException()", gPrint.GetImpliedFileTypeString().c_str());
}

//******************* class ZdFileDataSource ***********************************

/** constructor */
ZdFileDataSource::ZdFileDataSource(const std::string& sSourceFilename, ZdFileType& FileType)
                 :DataSource(), gwCurrentFieldIndex(-1), gbFirstRead(true) {
  try {
    gSourceFile.reset(FileType.Instantiate());
    gSourceFile->Open(sSourceFilename.c_str(), ZDIO_SREAD);
    //set date field format to that expected by current SaTScan date parser
    for (unsigned short w=0; w < gSourceFile->GetNumFields(); w++) 
       if (gSourceFile->GetFieldInfo(w)->GetType() == ZD_DATE_FLD)
         gSourceFile->GetFieldInfo(w)->SetFormat(ZdField::Filtered, "", new ZdDateFilter("%y/%m/%d"));
  }
  catch (ZdException & x) {
    GenerateResolvableException("Error: Could not open file:\n'%s'.\n", "ZdFileDataSource()", sSourceFilename.c_str());
  }
}

/** destructor */
ZdFileDataSource::~ZdFileDataSource() {}

long ZdFileDataSource::GetCurrentRecordIndex() const {
  return static_cast<long>(gSourceFile->GetSelectedRecordNumber());
}

short ZdFileDataSource::GetNumValues() {
  return static_cast<short>(gSourceFile->GetNumFields());
}

const char * ZdFileDataSource::GetValueAt(short iFieldIndex) {
  if (gwCurrentFieldIndex != iFieldIndex) {
    if (iFieldIndex > static_cast<short>(gSourceFile->GetNumFields()) - 1)
      return 0;
    gSourceFile->GetSystemRecord()->GetField(static_cast<unsigned short>(iFieldIndex), gTempBuffer, sizeof(gTempBuffer), true);
    gwCurrentFieldIndex = iFieldIndex;
  }
  return gTempBuffer;
}

void ZdFileDataSource::GotoFirstRecord() {
  gSourceFile->GotoFirstRecord();
  gbFirstRead = true;
  gwCurrentFieldIndex=-1;
}

bool ZdFileDataSource::ReadRecord() {
  if (gbFirstRead) {
    gbFirstRead = false;
    gwCurrentFieldIndex=-1;
    return gSourceFile->GotoFirstRecord();
  }
  gwCurrentFieldIndex=-1;
  return gSourceFile->GotoNextRecord();
}

