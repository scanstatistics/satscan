//******************************************************************************
#ifndef dBaseFileH
#define dBaseFileH
//******************************************************************************
//using std::ostream;
//using std::cout;
//using std::endl;
#include "xbase/xbase.h"
#include "FileName.h"
#include "SSException.h"
#include "ptr_vector.h"
#include "FieldDef.h"

class dBaseFile;

class dBaseRecord {
  friend class dBaseFile;

  private:
   unsigned char              * gpBufferChunk;
   unsigned long                gulBufferChunkSize;
   mutable unsigned char      * gpTempBufferChunk;
   mutable unsigned long        gulTempBufferChunkSize;
   static void                  ResizeChunk(unsigned long lNewSize, unsigned long& ulCurrentSize, unsigned char ** pChunk);
   mutable xbDbf              * gpAssociatedDbf;
   FileName                     gAssociatedFileName;

   // The following class is used in all functions that get data from or put data
   // into the record.  It uses scoping rules to ensure that a call to BeginAccess
   // is matched with a call to either EndAccess or EndManipulation, depending on
   // the value of bIsManipulative, as passed to the constructor.
   class AccessExpediter;
   friend class AccessExpediter;
   class AccessExpediter {
     private:
       const dBaseRecord & gR;
       bool gbIsManipulative;
     public:
       AccessExpediter(const dBaseRecord & r, bool bIsManipulative = false) : gR(r), gbIsManipulative(bIsManipulative) { gR.BeginAccess(); }
       ~AccessExpediter() { if (gbIsManipulative) const_cast<dBaseRecord &>(gR).EndManipulation(); else gR.EndAccess(); }
   };

   void                         AssertSufficientBufferSize(unsigned long ulReqdSize) const;
   void                         AssertNotXBaseError(xbShort code) const;
   void                         ResizeBuffers(unsigned long ulReqdSize);

  protected:
   xbDbf                      & GetAssociatedDbf() const;

   void                         BufferDbfRecordData(xbDbf& theDbf) const;
   void                         CopyDbfRecordData(xbDbf& theDbf);
   void                         RestoreDbfRecordData(xbDbf& theDbf) const;
   void                         UpdateDbfRecordData(xbDbf& theDbf) const;
   
   virtual void                 CheckFieldIndexRange(unsigned short uwFieldIndex) const;
   virtual bool                 GetFieldIndexIsInRange( unsigned short uwFieldIndex ) const { return (uwFieldIndex < GetFieldCount()); }

   //all functions that retrieve a value from the record or set a value in the record
   //must call BeginAccess before they do anything else.  It copies the xbDbf::RecBuf to
   //a temporary location and copies gBuffer into the xbDbf::RecBuf.  This allows
   //all access to be implemented in terms of xbDbf.
   //These functions must end by calling one of the End... functions.  If it is a
   //retrieval, call EndAccess.  If it is a manipulation, call EndManipulation.
   //Both of these functions write the temporary copy of xbDbf::RecBuf (made by BeginAccess)
   //back into the xbDbf::RecBuf, but EndManipulation first copies the xbDbf::RecBuf
   //back into gBuffer (since it has been changed, so gBuffer must be updated with
   //the changes).  See class dBaseRecord::AccessExpediter for a helper.
   void                         BeginAccess() const;
   void                         EndAccess() const;
   void                         EndManipulation();

   //these functions are called by dBaseFile.
   void                         LoadFromCurrentDbfRecord(xbDbf& theDbf);
   void                         AppendToDbf(xbDbf& theDbf) const;
   void                         OverwriteDbfRecordAt(xbDbf& theDbf, unsigned long ulPosition) const;

  public:
   dBaseRecord( dBaseFile & associatedFile, xbDbf & associatedDbf, const ptr_vector<FieldDef>& vFields );
   virtual ~dBaseRecord();

   virtual dBaseRecord        * Clone() const {return new dBaseRecord(*this);}
   virtual void                 Clear();
   virtual long                 GetFieldCount() const;
   virtual short                GetFieldLength(unsigned short wField) const;
   virtual char                 GetFieldType(unsigned short wField) const;
   virtual std::string        & GetFieldValue(unsigned short uwFieldNumber, std::string& buffer);
   virtual bool                 GetIsBlank(unsigned short wField) const;
   virtual char               * GetAlpha(unsigned short uwFieldNumber, char *pFieldValue, unsigned long lLength, bool bStripBlanks = true) const ;
   virtual bool                 GetBoolean(unsigned short uwFieldNumber) const ;
   SaTScan::Date              & GetDate(unsigned short uwFieldNumber, SaTScan::Date& theDate) const;
   virtual long                 GetLong(unsigned short uwFieldNumber) const ;
   void                         GetMemo(unsigned short uwFieldNumber, unsigned char** Value, unsigned long& ValueSize) const;
   virtual double               GetNumber(unsigned short uwFieldNumber) const ;
   virtual void                 PutFieldValue(unsigned short uwFieldIndex, const FieldValue & theValue);
   virtual void                 PutAlpha(unsigned short uwFieldNumber, const char *pFieldValue);
   virtual void                 PutBlank(unsigned short uwFieldNumber);
   virtual void                 PutBoolean(unsigned short uwFieldNumber, bool bValue);
   void                         PutDate(unsigned short uwFieldNumber, const SaTScan::Date& theDate) const;
   virtual void                 PutLong(unsigned short uwFieldNumber, long lFieldValue);
   void                         PutMemo(unsigned short uwFieldNumber, unsigned char** Value, unsigned long& ValueSize);
   virtual void                 PutNumber(unsigned short uwFieldNumber, double dFieldValue);
   virtual void                 RetrieveFieldValue( unsigned short uwFieldIndex, FieldValue& theStore) const;
};

class dBaseFile {
  private:
    xbXBase                     gXBase;//must have one of these objects
    std::auto_ptr<xbDbf>        gpDbf;//the DBF file
    bool                        gbAppendMode;        

    void                        AssertNotXBaseError(xbShort code) const;
    virtual void                CheckRecNum(unsigned long ulRecNum) const;
    virtual unsigned long       DataModify(unsigned long lPosition, const dBaseRecord &Record, dBaseRecord *pCurrentValue=0);
    virtual void                Assert_GetIsOpen() const;

  protected:
    ptr_vector<FieldDef>        gvFields;                          // Array of pointers to Field classes to hold field properties and formatting
    mutable FileName            gFileName;                          // File name for this file
    dBaseRecord               * gPSystemRecord;                    // Pointer to system buffer maintained by File
    std::string                 gsTitle;                            // Title of file

  public:
    dBaseFile(const char * sFileName=0, bool bAppend=false);
    virtual ~dBaseFile();

    size_t                      GetNumFields() const {return gvFields.size();}
	const char                * GetFieldName(size_t tField) const {return gvFields.at(tField)->GetName();}
    virtual unsigned long       DataAppend(const dBaseRecord &Record);
    const char                * GetFileName(std::string& name) const;
    dBaseRecord               * GetSystemRecord() const;
    virtual void                Close();
    virtual void                Create(const char * sFileName, const ptr_vector<FieldDef>& vFields);
    virtual void                Empty();
    virtual void                PackData();
    virtual void                ReadStructure();
    virtual unsigned long       GetCurrentRecordNumber() const;
    virtual dBaseRecord       * GetNewRecord() const  { return new dBaseRecord(const_cast<dBaseFile &>(*this), *gpDbf, gvFields); }
    virtual unsigned long       GetNumRecords() const;
    virtual void                GotoRecord(unsigned long lRecNum, dBaseRecord * PRecordBuffer = 0);
    virtual void                Open(const char *sFileName, bool bAppendMode=false);
    virtual void                PackFields(const ptr_vector<FieldDef> &vFields) const;
    virtual bool                GetIsOpen() const;
    const char                * GetDbfErrorString(xbShort code) const;

    static void                 AssertLegalFieldname(const std::string & sCandidate);
    static void                 CheckFieldType(char cFieldType);
    static void                 CheckXBaseFieldType(char cXBaseFieldType);
    static char                 GetFieldDefTypeFromXBaseFieldType(char cXBaseFieldType);
    static char                 GetXBaseFieldTypeFromFieldDefType(char cFieldDefType);
    static bool                 IsValidXBaseFieldType(char cCandidate);
    static bool                 IsValidFieldType(char cCandidate);    
    static bool                 IsLegalFieldname(const std::string & sCandidate);
    static std::pair<long, long>FieldLengthRangeForXBaseFieldType(char cFieldType);
    static std::pair<long, long>DecimalCountRangeForXBaseFieldType(char cFieldType, long lFieldLength);
    static const char         * GetFileTypeExtension() {return ".dbf";}
};
//******************************************************************************
#endif

