//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

/** Constructor. */
SaTScanFileImporter::SaTScanFileImporter(ZdIniFile & FileDef, InputFileType eFileType, SourceDataFileType eSourceDataFileType,
                                         BImportSourceInterface & SourceInterface, BFileDestDescriptor & FileDestDescriptor)
                    :BZdFileImporter(&SourceInterface, &FileDestDescriptor),
                     geFileType(eFileType), geSourceDataFileType(eSourceDataFileType), gFileDefinition(FileDef) {
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

/** */
void SaTScanFileImporter::CleanupDestinationDataFile(bool bDeleteDataFile) {
  ZdFileName    SaTScanDataFileName;

  try {
    gpRemoteFile->Close(); gpRemoteFile=0;
    SaTScanDataFileName = gpFileDestDescriptor->GetDestinationFileName().GetCString();
    if (bDeleteDataFile)
      ZdIO::Delete(SaTScanDataFileName.GetFullPath());
    SaTScanDataFileName.SetExtension(".zlg");
    ZdIO::Delete(SaTScanDataFileName.GetFullPath());
  }
  catch (ZdException & x) {
    x.AddCallpath("CleanupDestinationDataFile()", "SaTScanFileImporter");
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
     }
     else if (eDest == BFileDestDescriptor::SingleDatabaseFile || eDest == BFileDestDescriptor::Database)
       ZdGenerateException("ZdDatabase import not supported.","Import()");

     CleanupDestinationDataFile(false);
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
    CleanupDestinationDataFile(true);
    Screen->Cursor = crDefault;
    throw;
  }
}

void SaTScanFileImporter::ImportToRemoteFile( ZdVector< ZdVector<const ZdField*> >& vMappings, ZdProgressInterface & ProgressInterface ) {
  int                                  j, iColumn, iRec;
  ZdString                             sValue;
  bool                                 bErrorOccurred;
  BFileDestDescriptor::ErrorOptions    eErrorOptions;
  ZdFileRecord                       * pRecordBuffer = 0;
  ZdTransactionOld                     DBTransaction(*gpRemoteFile);

  try {
     eErrorOptions = gpFileDestDescriptor->GetErrorOptionsType();
     pRecordBuffer = gpRemoteFile->GetSystemRecord();

     ZdSubProgress subProgress( ProgressInterface, 0, 40 );
     subProgress.SetMaximum( gpSourceInterface->GetNumRecords() );
     subProgress.SetStepValue( 1 );
     while ( gpSourceInterface->GoToNextRecord()  ) {
           pRecordBuffer->Clear();
           iColumn = 0;
           gStats.ulRecordsRead++;
           bErrorOccurred = false;
           while ( ++iColumn <= gpSourceInterface->GetNumColumns() ) {
                sValue = gpSourceInterface->GetValueAt(iColumn);
                if (gpSourceInterface->GetColumnType(iColumn) == ZD_DATE_FLD)
                  AttemptFilterDateField(sValue);
                ZdVector<const ZdField*>& vec = vMappings.GetElement( iColumn - 1 );
                gPutError.SetCurrent( gpSourceInterface->GetCurrentRecordNum(), iColumn );
                for ( j=0; j < ( int )vec.size(); j++ ) {

                  //blank or fields with spaces between values
                  //are not permitted with SaTScan formatted files
                  sValue.Deblank();
                  if (sValue.GetIsEmpty())
                    BImportRejectedException::Generate( "Record %i contains a 'Source File Variable' that is blank.\n"
                                                        "SaTScan does not permit blank variables in data.", "ImportToRemoteFile()",
                                                        ZdException::Notify, gpSourceInterface->GetCurrentRecordNum() );
                  else if (sValue.Find(' ') > -1)
                    BImportRejectedException::Generate( "Record %i contains a 'Source File Variable' that contains whitespace.\n"
                                                        "SaTScan does not permit variable data to contain whitespace.", "ImportToRemoteFile()",
                                                        ZdException::Notify, gpSourceInterface->GetCurrentRecordNum() );

                   const ZdField * p = vec.GetElement( j );
                   if ( PutTokenToRecord( *pRecordBuffer, sValue, p->GetFieldNumber() ) )
                      bErrorOccurred = true;
                }
           }

          if ( subProgress.GetCancelRequested() )
            BImportCancelledException::Generate( "Import Cancelled.", "ImportToRemoteFile()", ZdException::Notify );

          if ( bErrorOccurred && eErrorOptions == BFileDestDescriptor::RejectBatch )
            BImportRejectedException::Generate( "Batch rejected.\nAn error writing records has caused\nthe batch to be rejected as specified.", "ImportToRemoteFile()", ZdException::Notify );

          if ( gStats.iErrorCount >= giMaxErrors )
            BImportRejectedException::Generate( "Exceeded max errors %d.", "ImportToRemoteFile()", ZdException::Notify, gStats.iErrorCount );

          if (! bErrorOccurred || ( bErrorOccurred && eErrorOptions == BFileDestDescriptor::AllRecords ) )
            if ( !pRecordBuffer->GetAllFieldsBlank() ) {
              if ( gpFileDestDescriptor->GetAddNewOption() && !CheckForPrimaryKeys( *gpRemoteFile, *pRecordBuffer ) ) {
                gPutError.sMessage << ZdString::reset << "Record is missing primary key field(s).";
                gPutError.iColumnNumber = 0;
                gvErrors.AddElement( gPutError );
                gStats.iErrorCount++;
              }
              else
                AddRecord( *pRecordBuffer, *gpRemoteFile, DBTransaction );
            }

          subProgress.StepIt();
     }
  }
  catch ( ZdException & x ) {
     x.AddCallpath( "ImportToRemoteFile()", "BZdFileImporter" );
     throw;
  }
}

/** */
void SaTScanFileImporter::OpenDestination() {
  ZdFileName FileName;

  try {
    //A file with same name a destination file may exist, so we want be sure
    //that the destination file can be created.
    FileName.SetFullPath(gpFileDestDescriptor->GetDestinationFileName().GetCString());
    AquireUniqueFileName(FileName);
    gpFileDestDescriptor->SetDestinationFile(FileName.GetFullPath());
    gpRemoteFile = new ScanfFile();
    gpRemoteFile->Open(gpFileDestDescriptor->GetDestinationFileName().GetCString(),
                                 ZDIO_OPEN_READ|ZDIO_OPEN_WRITE|ZDIO_OPEN_CREATE, 0, 0, &gFileDefinition);
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenDestination()", "SaTScanFileImporter");
    delete gpRemoteFile; gpRemoteFile=0;
    throw;
  }
}
/** Tries to save token to ZdFileRecord. Formats date field of dBase files. */
bool SaTScanFileImporter::PutTokenToRecord(ZdFileRecord & Record, ZdString & sToken, short wField) {
  bool     bErrorFound = false;

  try {
    if (sToken.GetLength())
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
    gDateFilter.SetFormat("%y/%m/%d");
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "SaTScanFileImporter");
    throw;
  }
}
