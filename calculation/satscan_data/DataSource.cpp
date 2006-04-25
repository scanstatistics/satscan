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

/** constructor */
AsciiFileDataSource::AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print) : DataSource(), gpSourceFile(0) {
  try {
    if ((gpSourceFile = fopen(sSourceFilename.c_str(), "r")) == NULL)
      GenerateResolvableException("Error: Could not open file:\n'%s'.\n", "AsciiFileDataSource()", sSourceFilename.c_str());
    gStringParser.reset(new StringParser(Print));
  }
  catch (ZdException & x) {
    fclose(gpSourceFile);
    x.AddCallpath("constructor()","AsciiFileDataSource");
    throw;
  }
}

/** destructor */
AsciiFileDataSource::~AsciiFileDataSource() {
  try {
    fclose(gpSourceFile);
  }
  catch (...){}
}

long AsciiFileDataSource::GetCurrentRecordIndex() const {
  return gStringParser->GetReadCount();
}

short AsciiFileDataSource::GetNumValues() {
  return gStringParser->GetNumberWords();
}

const char * AsciiFileDataSource::GetValueAt(short iFieldIndex) {
  return gStringParser->GetWord(iFieldIndex);
}

void AsciiFileDataSource::GotoFirstRecord() {
  gStringParser->Reset();
  fseek(gpSourceFile, 0L, SEEK_SET);
}

bool AsciiFileDataSource::ReadRecord() {
  const char * s;
  while ((s = gStringParser->ReadString(gpSourceFile)) != 0 && !gStringParser->HasWords());
  return (s != 0);
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

