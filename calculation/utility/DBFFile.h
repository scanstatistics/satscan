//---------------------------------------------------------------------------

#ifndef DBFFileH
#define DBFFileH

#include "zd540.h"
#include "xbase/xbase.h"
//---------------------------------------------------------------------------

class DBFFile;

class DBFRecord : public ZdFileRecord
{
friend class DBFFile;

private:
   ZdResizableChunk gBuffer;
   mutable ZdResizableChunk gTempBuffer;
//   DBFFile * gpAssociatedFile;
   mutable xbDbf * gpAssociatedDbf;
   ZdFileName gAssociatedFileName;

   // The following class is used in all functions that get data from or put data
   // into the record.  It uses scoping rules to ensure that a call to BeginAccess
   // is matched with a call to either EndAccess or EndManipulation, depending on
   // the value of bIsManipulative, as passed to the constructor.
   class AccessExpediter;
   friend class AccessExpediter;
   class AccessExpediter
      {
      private:
         const DBFRecord & gR;
         bool gbIsManipulative;
      public:
         AccessExpediter(const DBFRecord & r, bool bIsManipulative = false) : gR(r), gbIsManipulative(bIsManipulative) { gR.BeginAccess(); }
         ~AccessExpediter() { if (gbIsManipulative) const_cast<DBFRecord &>(gR).EndManipulation(); else gR.EndAccess(); }
      };

   void AssertSufficientBufferSize(unsigned long ulReqdSize) const;
   void AssertNotXBaseError(xbShort code) const;
   void ResizeBuffers(unsigned long ulReqdSize);

protected:
//   char * GetRecBuf() const;
   xbDbf & GetAssociatedDbf() const;

   void BufferDbfRecordData(xbDbf & theDbf) const;
   void CopyDbfRecordData(xbDbf & theDbf);
   void RestoreDbfRecordData(xbDbf & theDbf) const;
   void UpdateDbfRecordData(xbDbf & theDbf) const;

   //all functions that retrieve a value from the record or set a value in the record
   //must call BeginAccess before they do anything else.  It copies the xbDbf::RecBuf to
   //a temporary location and copies gBuffer into the xbDbf::RecBuf.  This allows
   //all access to be implemented in terms of xbDbf.
   //These functions must end by calling one of the End... functions.  If it is a
   //retrieval, call EndAccess.  If it is a manipulation, call EndManipulation.
   //Both of these functions write the temporary copy of xbDbf::RecBuf (made by BeginAccess)
   //back into the xbDbf::RecBuf, but EndManipulation first copies the xbDbf::RecBuf
   //back into gBuffer (since it has been changed, so gBuffer must be updated with
   //the changes).  See class DBFRecord::AccessExpediter for a helper.
   void BeginAccess() const;
   void EndAccess() const;
   void EndManipulation();

   //these functions are called by DBFFile.
   void LoadFromCurrentDbfRecord(xbDbf & theDbf);
   void AppendToDbf(xbDbf & theDbf) const;
   void OverwriteDbfRecordAt(xbDbf & theDbf, unsigned long ulPosition) const;

public:
   DBFRecord( DBFFile & associatedFile, xbDbf & associatedDbf, const ZdVector<ZdField*> & vFields );
   virtual ~DBFRecord();

   //--- Default copy operations are fine.

   virtual DBFRecord *  Clone() const { return new DBFRecord(*this); }

   virtual void            Clear();

//   virtual void            GetAllFields ( char *aPStorage[], unsigned short wNumFields = 0, unsigned short wStartFieldIndex = 0, unsigned long lMaxLength = 1024 ) const;
//   virtual bool            GetAllFieldsBlank() const;
   virtual long            GetBLOBIndex ( unsigned short wFieldNumber ) const { ZdException::Generate("can't get a BLOB index from a DBFSystemRecord", "DBFSystemRecord"); return 0; }

   virtual long            GetFieldCount() const;
   virtual short           GetFieldLength ( unsigned short wField ) const;
   virtual char            GetFieldType ( unsigned short wField ) const;
   virtual bool            GetIsBlank(unsigned short wField) const;
   virtual unsigned long   GetRecordLength() const { ZdException::Generate("can't get the length of a DBFSystemRecord", "DBFSystemRecord"); return 0; }

   virtual char *          GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long lLength, bool bStripBlanks = true) const ;
   virtual void            GetBinary(unsigned short uwFieldNumber, void *pValue ) const ;
   virtual bool            GetBoolean(unsigned short uwFieldNumber) const ;
   virtual ZdDate &        GetDate(unsigned short uwFieldNumber, ZdDate &theDate ) const ;
   virtual long            GetLong(unsigned short uwFieldNumber) const ;
   virtual double          GetNumber(unsigned short uwFieldNumber) const ;
   virtual short           GetShort(unsigned short uwFieldNumber) const ;
   virtual ZdTime         &GetTime(unsigned short uwFieldNumber, ZdTime &FieldValue) const ;
   virtual ZdTimestamp    &GetTimestamp(unsigned short uwFieldNumber, ZdTimestamp &FieldValue) const ;
   virtual unsigned long   GetUnsignedLong(unsigned short uwFieldNumber) const ;
   virtual unsigned short  GetUnsignedShort(unsigned short uwFieldNumber) const ;

   virtual void            PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue);
   virtual void            PutBinary ( unsigned short uwFieldNumber, const void *pValue );
   virtual void            PutBlank(unsigned short uwFieldNumber);
   virtual void            PutBoolean(unsigned short uwFieldNumber, bool bValue);
   virtual void            PutDate ( unsigned short uwFieldNumber, const ZdDate &theDate );
   virtual void            PutLong(unsigned short uwFieldNumber, long lFieldValue);
   virtual void            PutNumber(unsigned short uwFieldNumber, double dFieldValue);
   virtual void            PutShort(unsigned short uwFieldNumber, short wFieldValue);
   virtual void            PutTime(unsigned short uwFieldNumber, const ZdTime &FieldValue);
   virtual void            PutTimestamp(unsigned short uwFieldNumber, const ZdTimestamp &FieldValue);
   virtual void            PutUnsignedLong(unsigned short uwFieldNumber, unsigned long ulFieldValue);
   virtual void            PutUnsignedShort(unsigned short uwFieldNumber, unsigned short uwFieldValue);

   virtual void            GetBLOB(unsigned short uwFieldNumber, ZdBlob & theValue) const;
   virtual void            PutBLOB(unsigned short uwFieldNumber, const ZdBlob & theValue);

//   virtual void            LoadFieldsFrom ( const ZdFileRecord &rhs );

//   virtual void            PutAllFields ( char *aPStorage[], unsigned short wNumFields = 0, unsigned short wStartFieldIndex = 0 );

   virtual void            ReadFromBuffer ( const void *pBuffer ) { ZdException::Generate("can't read a DBFSystemRecord from a buffer", "DBFSystemRecord"); }
//   virtual void            ReadRecord(ZdInputStreamInterface &theFile, const char *sPassword = 0);
//   virtual void            WriteRecord(ZdOutputStreamInterface &theFile, const char*sPassword = 0) const;
   virtual void            WriteToBuffer ( void *pBuffer ) const { ZdException::Generate("can't write a DBFSystemRecord to a buffer", "DBFSystemRecord"); }
};



class DBFFile : public ZdFile
{
private:
//   void              Copy(const ZdFile & rhs);
//   void              Init();

   xbXBase gXBase;//must have one of these objects
   xbDbf * gpDbf;//the DBF file

   void AssertNotXBaseError(xbShort code) const;
//   const ZdIniSection     *FindFieldByName ( const ZdIniFile &theFile, const char *sFieldName, unsigned short *pwFieldNumber = 0 ) const;
//   void              ReadFieldArray ( const ZdIniFile &theFile, ZdVector<ZdField*> &vFields, const ZdPointerVector<ZdFlagType> *pFlagArray = 0 ) const;
//   void              ReadFlagsFromStream ( ZdInputStreamInterface &theStream );
//   void              ReadUserFlags ( ZdIniSection &theSection, bool bClearOldFlags = true );
//   void              ReadVectors ( ZdInputStreamInterface &theStream );
//   void              WriteFlagsToStream ( ZdOutputStreamInterface &theStream ) const;
//   void              WriteUserFlags ( ZdIniSection &theSection ) const;
//   void              WriteFieldArray ( const ZdVector<ZdField*> &vFields, ZdIniFile &theFile, const ZdPointerVector<ZdFlagType> *pFlagArray = 0 ) const;
//   void              WriteVectors ( ZdOutputStreamInterface &theStream ) const;

protected:
   // Internal functions
   virtual void      BuildEmptyIndexes ( ZdFileName *sAlternateFileName = 0 ) { ZdException::Generate("not implemented: BuildEmptyIndexes", "DBFFile"); }
   virtual void      CheckRecNum(unsigned long ulRecNum) const;
//   void              CopyBlobFile(const ZdFileName & dest, const ZdFileName & source) const;
//   void              CopyIndexes ( const char *sDestination, const char *sSource ) const;
   virtual void      CreateFrom ( const char *sDestFileName, ZdFile &SourceFile, ZdProgressInterface &theProgress = ZdNullProgress::GetDefaultInstance(), ZdVector<ZdField*> *pvFields = 0 ) { ZdException::Generate("not implemented: CreateFrom", "DBFFile"); }
   virtual void      CreateAndOpenFrom ( const char *sDestFileName, const ZdFile &SourceFile, ZdVector<ZdField *> *pvFields = 0 ) { ZdException::Generate("not implemented: CreateAndOpenFrom", "DBFFile"); }
//   void              DeleteIndexes();
//   static void       DeleteIndexesFor ( const char *sFileName, long lNumFields );
//   ZdString          GetIndexFileName(unsigned short wField) const;
//   ZdIniFile        *GetIniFile ( const char *sFileName ) const;
//   ZdIndex *         GetNewIndex(const char cType, short wLength, unsigned long lNumRecords) const;
//   ZdIndex *         GetNewIndex ( short wField, ZdProgressInterface &theProgress = ZdNullProgress::GetDefaultInstance() );
   virtual unsigned long GetRecordNumberByKey ( const ZdFileRecord &Record ) { ZdException::Generate("not implemented: GetRecordNumberByKey", "DBFFile"); }
//   void              LoadIndexes();
   virtual void      OpenBlobFile(const char * sFileName, ZdIOFlag Flags) { ZdException::Generate("not implemented: OpenBlobFile", "DBFFile"); }
//   void              OpenSetup(const char *sFileName, ZdIOFlag Flags, const char *sAlternateZDSFile = 0, ZdIniFile *pZDSFile = 0);
//   void              OpenFinish();
//   inline void       VerifyTransaction ( ZdTransaction *pTransaction ) const;

   // Record modification interface. These are the functions which actually
   // modify the file. They do not adjust indexes, pay attention to transactions, etc.
   // ALL they do is modify the data.
   //This functionality is offloaded to xbDbf, so these will be defined to be empty.  They
   //should never be called.
   virtual unsigned long DataAppend  ( const ZdFileRecord &Record );
   virtual void          DataDelete  ( unsigned long lPosition, ZdFileRecord *pCurrentValue = 0 ) { ZdException::Generate("not implemented: DataDelete", "DBFFile"); }
//   virtual unsigned long DataInsert  ( unsigned long lPosition, const ZdFileRecord &Record ); // Defaults to DataAppend()
   virtual unsigned long DataModify  ( unsigned long lPosition, const ZdFileRecord &Record, ZdFileRecord *pCurrentValue = 0 );
   virtual void          DataReplace ( const ZdFileRecord &removeRecord, const ZdFileRecord &addRecord, ZdFileRecord *pCurrentValue, unsigned long &lRemovePosition, unsigned long &lAddPosition  ) { ZdException::Generate("not implemented: DataReplace", "DBFFile"); }

   // Keyed record modification functions. By default, these functions simply use
   // GotoRecordByKeys and call the base functions.
   //This functionality is offloaded to xbDbf, so these will be defined to be empty.  They
   //should never be called.
   virtual unsigned long DataAdd ( const ZdFileRecord &NewRecord ) { ZdException::Generate("not implemented: DataAdd", "DBFFile"); return 0; }
   virtual unsigned long DataRemove ( const ZdFileRecord &KeyRecord, ZdFileRecord *pOldValue = 0 ) { ZdException::Generate("not implemented: DataRemove", "DBFFile"); return 0; }
   virtual unsigned long DataUpdate ( const ZdFileRecord &NewRecord, ZdFileRecord *pOldValue = 0 ) { ZdException::Generate("not implemented: DataUpdate", "DBFFile"); return 0; }

   // Index update functions. These functions will update the secondary indexes
   // of the file.
   //This functionality is offloaded to xbDbf, so these will be defined to be empty.  They
   //should never be called.
   virtual void          FileMaintenanceAdd     ( unsigned long ulRecordNum, const ZdFileRecord &theRecord ) {  }
   virtual void          FileMaintenanceDelete  ( unsigned long ulRecordNum ) {  }
   virtual void          FileMaintenanceModify  ( unsigned long ulRecordNum, const ZdFileRecord &oldRec, const ZdFileRecord &newRec ) {  }
   virtual void          FileMaintenanceReplace ( unsigned long ulRemoveRecordNum, unsigned long ulAddRecordNum, const ZdFileRecord &removeRec, const ZdFileRecord &addRec ) {  }

   // Functions to assist in the serialization of file classes
//   bool          ReadBase ( ZdInputStreamInterface &theStream );
//   void          ReadBlobFile ( ZdInputStreamInterface &theStream );
//   void          ReadIndexes ( ZdInputStreamInterface &theStream, unsigned long ulNumRecords );

//   void          WriteBase ( ZdOutputStreamInterface &theStream ) const;
//   void          WriteBlobFile ( ZdOutputStreamInterface &theStream ) const;
//   void          WriteIndexes ( ZdOutputStreamInterface &theStream ) const;
   virtual void SetupDefaultFilterForField(ZdField & theField);
   virtual void Assert_GetIsOpen() const;

protected://static interface

public:
   // Public functions
   DBFFile(const char * sFileName = 0);
   virtual ~DBFFile();

//   bool                 operator==  (const ZdFile &rhs) const;
//   bool                 operator!=  (const ZdFile &rhs) const;

//   void                     ActivateListeners ( bool bNotify = true );
//   void                     AddActionListener(ZdFileListener * pListenerClass);
//   void                     AddCategory(const char * sCategory);
//   void                     AddField(const char * sFieldName, const char * sMappedFieldName, unsigned short wPosition);
//   inline void              AddIndex(unsigned short wField, short wNumFields, bool bPrimary, char * sViolTblName, short wIndexVersion, ZdProgressInterface *pProgress);
   virtual void             AddIndex(unsigned short wField, short wNumFields = 1, bool bPrimary = true, char * sViolTblName = 0, short wIndexVersion = ZDINDEX_VERSION, ZdProgressInterface &Progress = ZdNullProgress::GetDefaultInstance() ) { ZdException::Generate("not implemented: AddIndex", "DBFFile"); }
//   void                     AddRecord ( ZdTransaction &theTransaction, const ZdFileRecord &theRecord );
//   void                     AllRecords();
//   inline unsigned long     AppendRecord ( ZdTransaction *pTransaction, ZdFileRecord *pRecordBuffer = 0 );
//   void                     AppendRecord ( ZdTransaction &theTransaction, const ZdFileRecord &theRecordBuffer );
//accept default functionality:   virtual ZdTransaction *  BeginTransaction ( bool bShouldLock = true );


//   void                     ClearFormats ( bool bPreserveFormatTypes = false );
//   void                     ClearIndexes ( bool bClearOnDisk = false );
//   void                     CheckSystemSet();
   virtual void             Close();

//to be implemented:   virtual void             Copy(char * sDesZdFileName, char * sSourceFileName, char * sPassword, bool bCopyZds = true, bool bCopyData = true) const;
   virtual void             Create(const char * sFileName, ZdVector<ZdField*> &vFields, unsigned short uwNumPrimaryKeyFields = 0, bool bBuildIndexes = true );

//   void                     DeactivateListeners( bool bNotify = true );
   virtual void             Decrypt(const char * sPassword) { ZdException::Generate("not implemented: Decrypt", "DBFFile"); }
   virtual void             DelayExternalUpdates() { ZdException::Generate("not yet implemented: DelayExternalUpdates", "DBFFile"); }
   virtual void             Delete(const char * sFileName, const char * sPassword = 0, bool bDeleteZds = true, bool bDeleteData = true) const { ZdException::Generate("not yet implemented: Delete", "DBFFile"); }
//   void                     DeleteRecord ( ZdTransaction &theTransaction, unsigned long ulWhich );

   virtual void             Empty();
   virtual void             Encrypt(const char * sPassword) { ZdException::Generate("not implemented: Encrypt", "DBFFile"); }
//accept default functionality:   virtual void             EndTransaction(ZdTransaction * pTransaction);

//   const char *             GetCategory(unsigned short wCategory) const;
//   inline unsigned long     GetCurrentRecordNumber() const;

//to be implemented:   virtual ZdFileCheckPoint *GetCheckPoint();

   inline virtual const char *GetDatabaseExtension() const { ZdException::Generate("not yet implemented: GetDatabaseExtension", "DBFFile"); return 0; }
   inline virtual const char *GetDatabaseName() const { ZdException::Generate("not yet implemented: GetDatabaseName", "DBFFile"); return 0; }

//   ZdField *                GetFieldInfo(const char * sFieldName) const;
//   ZdField *                GetFieldInfo(unsigned short wFieldNumber) const;
//   short                    GetFieldNumber(const char * sFieldName) const;

//   const char *             GetFileName() const;
//   inline const ZdSet      &GetFilterSet() const;
//   unsigned short           GetFlags() const;

//   const ZdFormatLib &      GetFormatLibrary() const;
//   ZdFormatLib &            GetFormatLibrary();
//to be implemented:   virtual double           GetFractionUsed() const;

   virtual ZdIndex &        GetIndex(unsigned short wField, bool bGetForUpdate = false ) { ZdException::Generate("not implemented: GetIndex", "DBFFile"); }
   virtual unsigned short   GetIndexFieldCount(unsigned short wField) const { ZdException::Generate("not implemented: GetIndexFieldCount", "DBFFile"); return 0; }
//   inline bool              GetIndexingState() const;              // DCH 12/7/1999


//   const char *             GetInformation() const;
//   bool                     GetIsInTransaction() const;
//   const char *             GetLayoutName() const;
//   unsigned short           GetLayoutNumber() const;
   virtual ZdField *        GetNewField() const;
//   unsigned short           GetNumFields() const;
//   unsigned short           GetNumCategories() const;
//   unsigned long            GetRecordsForKey(ZdFileRecord * pRecordBuffer);
//   inline unsigned short    GetNumPrimaryKeyFields() const;
//   unsigned short           GetNumRelateFields() const;
//   const char *             GetPassword();
//   unsigned long            GetRecordsInSelection();
//   const char *             GetRelateField(unsigned short wPosition) const;
//   unsigned long            GetSelectedRecordNumber();
//   bool                     GetMaintenanceState() const;
//   ZdFileRecord *           GetSystemRecord() const;
//   inline ZdSet &           GetSystemSet();
//default implementation is fine:   virtual const char *     GetTableName() const;
//   const char *             GetTitle() const;
//   inline bool              GetTransactionIsValid ( ZdTransaction *pTransaction ) const;
//   const ZdFileTransactionLog & GetTransactionLog() const;
//   ZdFileTransactionLog &       GetTransactionLog();

//these might have to be declared virtual in the base class:
//default implementation is fine:   unsigned long            GotoFirstRecord(ZdFileRecord * PRecordBuffer = 0);
//default implementation is fine:   unsigned long            GotoLastRecord(ZdFileRecord * PRecordBuffer = 0);
//default implementation is fine:   unsigned long            GotoNextRecord(ZdFileRecord * PRecordBuffer = 0);
//default implementation is fine:   unsigned long            GotoPreviousRecord(ZdFileRecord * PRecordBuffer = 0);
//
   virtual unsigned long    GotoRecordByKeys ( const ZdFileRecord *pSourceRecord, ZdFileRecord * PRecordBuffer = 0 ) { ZdException::Generate("not yet implemented: GotoRecordByKeys", "DBFFile"); return 0; }

//the "insert" functions might need to be declared virtual in the base class:
   inline unsigned long     InsertRecord(ZdTransaction * pTransaction, unsigned long lInsertPosition, ZdFileRecord * PRecordBuffer = 0 );
   inline unsigned long     InsertRecord(ZdTransaction * pTransaction, unsigned long lInsertPosition, ZdFileRecord * PRecordBuffer, bool bShouldSelect );
   void                     InsertRecord( ZdTransaction &theTransaction, unsigned long lInsertPosition, const ZdFileRecord &theRecordBuffer );
   inline const ZdSet      &InspectSystemSet() const;
//default implementation is fine:   virtual void             Lock ( ZdIOFlag iType );

   virtual void             PackData ( ZdProgressInterface &Progress = ZdNullProgress::GetDefaultInstance() );

   virtual void             PerformExternalUpdates();

   virtual void             Read ( ZdInputStreamInterface &theStream ) { ZdNotImplementedError::Generate("DBFFile is not streamable", "Read()"); }
   static ZdFile *          ReadDerivedZdFile ( ZdInputStreamInterface &theStream );

   virtual void             ReadStructure( ZdIniFile *pAlternateZDSFile = 0 );
//default implementation is fine:   virtual void             Refresh();
//default implementation is fine:   virtual void             RegisterTransaction ( ZdTransaction *pOpenTrans );

//   void                     RemoveActionListener(ZdFileListener * pListenerClass);
//   void                     RemoveField(const char * sName);
//   void                     RemoveField(unsigned short wField);
//   void                     RemoveCategory(unsigned short wCategory);
   virtual void             RemoveIndex(const char * sName) { ZdException::Generate("not implemented: RemoveIndex(const char *)", "DBFFile"); }
   virtual void             RemoveIndex(unsigned short wField) { ZdException::Generate("not implemented: RemoveIndex(unsigned short)", "DBFFile"); }
//   void                     RemoveRecord ( ZdTransaction &theTransaction, const ZdFileRecord &KeyRecord );
//to be implemented:   virtual void             Rename(char * sDestFileName, char * sSourceFileName, char * sPassword) const;
//   void                     Repair();
//   void                     ReplaceRecord ( ZdTransaction &theTransaction, const ZdFileRecord &removeRecord, const ZdFileRecord &addRecord );
//to be implemented:   virtual void             RestoreCheckPoint ( const ZdFileCheckPoint *pPoint );

//   inline unsigned long     SaveRecord(ZdTransaction * pTransaction, ZdFileRecord * PRecordBuffer = 0);
//   void                     SaveRecord ( ZdTransaction &theTransaction, unsigned long ulPosition, const ZdFileRecord &theRecord );
//   void                     Search(const char * sName, const void * pKeyValueLo, const void * pKeyValueHi, ZdSet & theSet, unsigned short wNumFieldsToSearch = 1, bool bAltOrder = false );
//   void                     Search(unsigned short wField, const void * pKeyValueLo, const void * pKeyValueHi, ZdSet & theSet, unsigned short wNumFieldsToSearch = 1, bool bAltOrder = false);
//   void                     Search(unsigned short wField, const ZdArray & theValues, ZdSet & theSet, unsigned short wNumFieldsToSearch = 1, bool bAltOrder = false);
//   void                     SetCategory(const char * sCategory, unsigned short wCategory);
//   inline void              SetDatabase ( ZdDatabase &theDatabase );
//   inline void              SetFilterSet ( const ZdSet &theSet );
//   void                     SetFlags(unsigned short wFlags);

//   void                     SetIndexingState ( bool bShouldIndex );
//   void                     SetInformation(const char * sInfo);
//   void                     SetInputLayout(const char * sInputLayout, unsigned short wInputLayout);
//   void                     SetMaintenanceState ( bool bShouldMaintain );
//   inline void              SetNotifyListeners(bool b, bool bNotify = true );
//   void                     SetPassword(const char * sOldPassword, const char * sNewPassword);
//   inline void              SetSystemSet(const ZdSet & theSet);
//   void                     SetTitle(const char * sTitle);

//to be implemented:   virtual void             Sort ( const ZdVector<unsigned short> &vFields, ZdProgressInterface &Progress = ZdNullProgress::GetDefaultInstance(), const char *sTempDir = mgsDot, unsigned int uiIntNodes = 8192, unsigned int uiCacheSize = 1048576, bool bRebuildIndexes = false );

//to be implemented:   virtual ZdTransaction *  StartTransaction ( unsigned long ulUserID, const char *sDescription = 0 );

//default implementation is fine:   virtual bool             Unlock ( ZdIOFlag iType );
   inline void              UnsetDatabase();
   void                     UpdateRecord ( ZdTransaction &theTransaction, const ZdFileRecord &theRecord );
   inline void              VerifyIndexExists(bool bForceRebuild, ZdProgressInterface *pProgress, short wIndexVersion = ZDINDEX_VERSION);
   void                     VerifyIndexExists(bool bForceRebuild, ZdProgressInterface &Progress = ZdNullProgress::GetDefaultInstance(), short wIndexVersion = ZDINDEX_VERSION);
   inline void              VerifySelectedRecord();

   void                     WriteDerivedZdFile ( ZdOutputStreamInterface &theStream ) const;
   virtual void             Write ( ZdOutputStreamInterface &theStream ) const  { ZdNotImplementedError::Generate("DBFFile is not streamable", "Write()"); }
   virtual void             WriteStructure ( ZdIniFile *pAlternateZDSFile = 0, const ZdVector<ZdField*> *pvFields = 0 ) const;

   void                          RemoveUnknownFieldFlags();
   const ZdVector<ZdFlagType *> &GetUnknownFieldFlags() const;

   // Pure virtual functions that must be defined by the file classes
   virtual void            Flush();
   virtual unsigned long   GetCurrentRecordNumber() const;
   virtual DBFRecord *     GetNewRecord() const  { return new DBFRecord(const_cast<DBFFile &>(*this), *gpDbf, gvFields); }
   virtual unsigned long   GetNumRecords() const;
   virtual void            GotoRecord(unsigned long lRecNum, ZdFileRecord * PRecordBuffer = 0);
   virtual void            Open(const char *sFileName, ZdIOFlag Flags = ZDIO_OPEN_READ, const char * sPassword = 0, const char * sAlternateZDSFile = 0, ZdIniFile *pZDSFile = 0);
   virtual void            PackFields ( ZdVector<ZdField*> &vFields ) const;
   virtual bool            TryLock ( ZdIOFlag iType );

   virtual bool            GetIsOpen() const;
   // Pure virtual functions associated with ZdFileType
   virtual const ZdFileType &GetFileType() const;

   // Each derived class of ZdFile also needs a static member function which returns a "ZdFile *" of the class type.
   // I have used : static ZdFile *Instantiate(), but any function with the same signature will work.

   const char * GetDbfErrorString(xbShort code) const;

public://static interface
   static void AssertLegalFieldname(const ZdString & sCandidate);
   static void CheckZdFieldType(char cZdFieldType) { if (! DBFFile::IsValidZdFieldType(cZdFieldType)) ZdException::Generate("The character, '%c', does not refer to a valid Zd field type.", "DBFFile", cZdFieldType); }
   static void CheckXBaseFieldType(char cXBaseFieldType) { if (! DBFFile::IsValidXBaseFieldType(cXBaseFieldType)) ZdException::Generate("The character, '%c', does not refer to a valid xbase field type.", "DBFFile", cXBaseFieldType); }
   static char GetZdFieldTypeFromXBaseFieldType(char cXBaseFieldType);
   static char GetXBaseFieldTypeFromZdFieldType(char cZdFieldType);
   static bool IsValidXBaseFieldType(char cCandidate);
   static bool IsValidZdFieldType(char cCandidate);
   static bool IsLegalFieldname(const ZdString & sCandidate);
   static std::pair<long, long> FieldLengthRangeForXBaseFieldType(char cFieldType);
   static std::pair<long, long> DecimalCountRangeForXBaseFieldType(char cFieldType, long lFieldLength);
};

class ZDEXPORT DBFFileType : public ZdFileType
{
public:
   virtual DBFFileType *         Clone() const;

   virtual void                  GetFieldTypesAllowed ( ZdVector<const ZdFieldType *> &vFillArray ) const;
   virtual const char *          GetFileTypeName() const;
   virtual const char *          GetFileTypeExtension() const;
   virtual bool                  GetCanBeCreated() const;
   virtual ZdFileStorageFormat   GetStorageFormat() const;

   virtual DBFFile *             Instantiate() const;
   virtual DBFFile *             InstantiateFromStream ( ZdInputStreamInterface &theStream ) const;

   static DBFFileType & GetDefaultInstance() { return gDefaultInstance; }

private:
   static DBFFileType gDefaultInstance;
};



#endif

