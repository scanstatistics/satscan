//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include "UtilityFunctions.h"
#include <iostream>
#include <fstream>
#include "dBaseFile.h"

/** Input data source abstraction. */
class DataSource {
   protected:
     bool _blank_record_flag;

   public:
     DataSource() : _blank_record_flag(false) {}
     virtual ~DataSource() {}

     void                               clearBlankRecordFlag() {_blank_record_flag=false;}
     bool                               detectBlankRecordFlag() const {return _blank_record_flag;}
     virtual long                       GetCurrentRecordIndex() const = 0;
     virtual long                       getNonBlankRecordsRead() const {return GetCurrentRecordIndex();}
     static DataSource                * GetNewDataSourceObject(const std::string& sSourceFilename, BasePrint& Print, bool bAssumeASCII=true);
     virtual long                       GetNumValues() = 0;
     virtual const char               * GetValueAt(long iFieldIndex) = 0;
     virtual void                       GotoFirstRecord() = 0;
     virtual bool                       ReadRecord() = 0;
     void                               tripBlankRecordFlag() {_blank_record_flag=true;}
};

/** ASCII file data source. */
class AsciiFileDataSource : public DataSource {
   private:
     class StringParser {
       private:
         std::string                 gsWord;
         long                        gwCurrentWordIndex;
         BasePrint                 & gPrint;
         const std::string         * gpParseLine;
         const char                * gcp;
         const char                  gcDelimiter;

         void                        ThrowAsciiException();

       public:
         StringParser(BasePrint& Print, const char cDelimiter) : gPrint(Print), gwCurrentWordIndex(-1), gcp(0), gcDelimiter(cDelimiter) {}

         bool                        HasWords() {return GetWord(0) != 0;}
         bool                        isDelimiter(char c) const {return isspace(gcDelimiter) ? isspace(c) : c == gcDelimiter;}
         long                        GetNumberWords();
         const char                * GetWord(long wWordIndex);
         bool                        SetString(std::string& sParseLine);
     };

     std::auto_ptr<StringParser>        gStringParser;
     long                               glReadCount;
     long                               glBlankReadCount;
     std::ifstream                      gSourceFile;
     BasePrint                        & gPrint;
     std::string                        gsReadBuffer;

    void                                ThrowUnicodeException();

   public:
     AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print, const char cDelimiter=' ');
     virtual ~AsciiFileDataSource() {}

     virtual long                       GetCurrentRecordIndex() const {return glReadCount;}
     virtual long                       getNonBlankRecordsRead() const {return glReadCount - glBlankReadCount;}
     virtual long                       GetNumValues() {return gStringParser->GetNumberWords();}
     virtual const char               * GetValueAt(long iFieldIndex) {return gStringParser->GetWord(iFieldIndex);}
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

/** dBase file data source. */
class dBaseFileDataSource : public DataSource {
   private:
     std::auto_ptr<dBaseFile>           gSourceFile;
     std::string                        gsValue;
     long                               gwCurrentFieldIndex;
     bool                               gbFirstRead;
     unsigned long                      glNumRecords;
     unsigned long                      glCurrentRecord;

   public:
     dBaseFileDataSource(const std::string& sSourceFilename);
     virtual ~dBaseFileDataSource();

     virtual long                       GetCurrentRecordIndex() const;
     virtual long                       GetNumValues();
     virtual const char               * GetValueAt(long iFieldIndex);
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

class OneCovariateDataSource : public DataSource {
    protected:
        unsigned short _covariateIndex;
        unsigned long _covariate;
        std::string _buffer;

    public:
        OneCovariateDataSource(unsigned short covariateIndex, unsigned long covariate): _covariateIndex(covariateIndex), _covariate(covariate) {}
        virtual long                    GetCurrentRecordIndex() const {return 1;}
        virtual long                    GetNumValues() {return 1;}
        virtual const char            * GetValueAt(long iFieldIndex) {return iFieldIndex == _covariateIndex ? printString(_buffer, "%u", _covariate).c_str() : (const char*)0;}
        virtual void                    GotoFirstRecord() {}
        virtual bool                    ReadRecord() {return true;}
};
//******************************************************************************
#endif
