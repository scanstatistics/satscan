//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

/** Constructor. */
SaTScanFileImporter::SaTScanFileImporter(InputFileType eFileType, SourceDataFileType eSourceDataFileType,
                                         BImportSourceInterface & SourceInterface, BFileDestDescriptor & FileDestDescriptor)
                    :BZdFileImporter(&SourceInterface, &FileDestDescriptor),
                     geFileType(eFileType), geSourceDataFileType(eSourceDataFileType) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()", "SaTScanFileImporter");
    throw;
  }
}

/** Destructor. */
SaTScanFileImporter::~SaTScanFileImporter() {}

/** Finds an unique filename that is similiar to passed filename. */
void SaTScanFileImporter::AquireUniqueFileName(ZdFileName& FileName) {
  ZdString      sTableName, sTemp;
  int           iCount=1;
  bool          bUniqueNameAquire=false;

  try {
    sTableName = FileName.GetFileName();
    while (! bUniqueNameAquire) {
      if (ZdIO::Exists(FileName.GetFullPath())) {
        ++iCount;
        sTemp << ZdString::reset << sTableName << " (" << iCount << ')';
        FileName.SetFileName(sTemp);
      }
      else
        bUniqueNameAquire = true;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("AquireUniqueFileName()", "SaTScanFileImporter");
    throw;
  }
}

/** This a temporary hack function that formats date fields to sFormat. This is needed because SaTScan
    expects dates in human readable form such as '12/08/2002' as apposed to raw data form of 20021208.
    This function should be removed once the zdfile interface for satscan is implementated! */
void SaTScanFileImporter::AttemptFilterDateField(ZdString & sDateToken) {
  try {
    gDateFilter.FilterValue(gsFilterBuffer, sizeof(gsFilterBuffer), sDateToken.GetCString());
    sDateToken = gsFilterBuffer;
  }
  catch (...){}
}

/** Renames ZdFile's data file to that of SaTScan data file type and deletes
    supporting ZdFile files. Dates are filtered to format used in data read routines. */
void SaTScanFileImporter::ConvertImportedDataFile() {
  ZdString      sZdDataFileName;
  ZdFileName    SaTScanDataFileName;

  try {
    sZdDataFileName = gpFileDestDescriptor->GetDestinationFileName();
    SaTScanDataFileName.SetFullPath(sZdDataFileName);
    switch (geFileType) {
      case Case        : SaTScanDataFileName.SetExtension(".cas");
                         break;
      case Control     : SaTScanDataFileName.SetExtension(".ctl");
                         break;
      case Population  : SaTScanDataFileName.SetExtension(".pop");
                         break;
      case Coordinates : SaTScanDataFileName.SetExtension(".geo");
                         break;
      case SpecialGrid : SaTScanDataFileName.SetExtension(".grd");
                         break;
      default  : ZdGenerateException("Unknown file type : \"%d\"", "ConvertImportedDataFile()", geFileType);
    };

    AquireUniqueFileName(SaTScanDataFileName);
    ZdIOInterface::Rename(SaTScanDataFileName.GetFullPath(), sZdDataFileName.GetCString());
    gpFileDestDescriptor->SetDestinationFile(SaTScanDataFileName.GetFullPath());
    TXDFile().Delete(sZdDataFileName.GetCString());
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertImportedDataFile()", "SaTScanFileImporter");
    throw;
  }
}

/** Import File. */
void SaTScanFileImporter::Import(ZdProgressInterface * pProgressInterface) {
  int                                    iMidPos = 100;
  bool                                   bOwnsProgress = false;
  AnsiString                             sProcessText;
  TBfrmProgressSingle                  * pProgress = 0;
  ZdVector< ZdVector<const ZdField*> >   vMappings(gpSourceInterface->GetNumColumns(), 0, ZdVector<const ZdField*>(0));
  BFileDestDescriptor::Destination       eDest = gpFileDestDescriptor->GetDestinationType();

  try {
     if (! pProgressInterface) {
       bOwnsProgress = true;
       pProgress = new TBfrmProgressSingle(0);
       pProgress->Caption = "File Import";
       sProcessText.sprintf("Importing %u records ...", gpSourceInterface->GetNumRecords());
       pProgress->GetPanel()->ProcessText = sProcessText;
       pProgress->Show();
       pProgressInterface = new BFPMPanelAsZdProgressTracker(pProgress->GetPanel(), pProgress);
     }
     Screen->Cursor = crHourGlass;
     gvErrors.clear();
     CondenseMappings(vMappings);

     if (eDest == BFileDestDescriptor::SingleFile) {
       ImportToRemoteFile(vMappings, ZdSubProgress(*pProgressInterface, 0, iMidPos));
       if (gpRemoteFile) {
         BasisGetToolkit().CloseZdFile(gpRemoteFile, ZDIO_SWRITE);
         gpRemoteFile=0;
       }
       ConvertImportedDataFile();
     }
     else if (eDest == BFileDestDescriptor::SingleDatabaseFile || eDest == BFileDestDescriptor::Database)
       ZdGenerateException("ZdDatabase import not supported.","Import()");

     Screen->Cursor = crDefault;
     if (bOwnsProgress) {
       delete pProgressInterface; pProgressInterface=0;
       delete pProgress; pProgress=0;
     }
  }
  catch (ZdException &x) {
    x.AddCallpath("Import()", "SaTScanFileImporter");
    if (bOwnsProgress) {
      delete pProgressInterface;
      delete pProgress;
    }
    Screen->Cursor = crDefault;
    throw;
  }
}

/** Tries to save token to ZdFileRecord. Formats date field of dBase files. */
bool SaTScanFileImporter::PutTokenToRecord(ZdFileRecord & Record, ZdString & sToken, short wField) {
  bool     bErrorFound = false;

  try {
    if (wField == gwDateFilteredField)
      AttemptFilterDateField(sToken);
    Record.PutField(wField, sToken);
  }
  catch (ZdException &x) {
    gPutError.sMessage = x.GetErrorMessage();
    gvErrors.AddElement( gPutError );
    gStats.iErrorCount++;
    bErrorFound = true;
  }
  return bErrorFound;
}

/** Internal setup function. */
void SaTScanFileImporter::Setup() {
  try {
    //dBase files require the date fields to formatted before token
    //is put to record.
    if (geSourceDataFileType == dBase) {
      switch(geFileType) {
        case Case       : gwDateFilteredField = 2;
                          break;
        case Control    : gwDateFilteredField = 2;
                          break;
        case Population : gwDateFilteredField = 1;
                          break;
        default         : gwDateFilteredField = -1;
      };
    }

    gDateFilter.SetFormat("%y/%m/%d");
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "SaTScanFileImporter");
    throw;
  }
}
