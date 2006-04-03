//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include "UtilityFunctions.h"

/** Input data source abstraction. */
class DataSource {
   public:
     DataSource() {}
     virtual ~DataSource() {}

     virtual long                       GetCurrentRecordIndex() const = 0;
     static DataSource                * GetNewDataSourceObject(const std::string& sSourceFilename, BasePrint& Print);
     virtual short                      GetNumValues() = 0;
     virtual const char               * GetValueAt(short iFieldIndex) = 0;
     virtual void                       GotoFirstRecord() = 0;
     virtual bool                       ReadRecord() = 0;
};

/** ASCII file data source. */
class AsciiFileDataSource : public DataSource {
   private:
     FILE                             * gpSourceFile;
     std::auto_ptr<StringParser>        gStringParser;

   public:
     AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print);
     virtual ~AsciiFileDataSource();

     virtual long                       GetCurrentRecordIndex() const;
     virtual short                      GetNumValues();
     virtual const char               * GetValueAt(short iFieldIndex);
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

/** ZdFile file data source (dBase, Scanf, etc.). */
class ZdFileDataSource : public DataSource {
   private:
     std::auto_ptr<ZdFile>              gSourceFile;
     mutable char                       gTempBuffer[ZD_BUFFER_LEN];
     short                              gwCurrentFieldIndex;
     bool                               gbFirstRead;

   public:
     ZdFileDataSource(const std::string& sSourceFilename, ZdFileType& FileType);
     virtual ~ZdFileDataSource();

     virtual long                       GetCurrentRecordIndex() const;
     virtual short                      GetNumValues();
     virtual const char               * GetValueAt(short iFieldIndex);
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};
//******************************************************************************
#endif
