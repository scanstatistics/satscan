//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include "UtilityFunctions.h"
#include <iostream>
#include <fstream>
#include "dBaseFile.h"
#include "ShapeFile.h"
#include "Parameters.h"
#include "boost/logic/tribool.hpp"

/** Input data source abstraction. */
class DataSource {
    public:
        enum FieldType {ONECOUNT, GENERATEDID, DEFAULT_DATE, BLANK};
        enum ShapeFieldType {POINTX=0, POINTY};

    protected:
        bool _blank_record_flag;
        mutable boost::logic::tribool _has_event_id;
        FieldMapContainer_t _fields_map;
        LineListFieldMapContainer_t _linelist_fields_map;

        virtual long tranlateFieldIndex(long idx) const {
            if (idx < static_cast<long>(_fields_map.size())) {
                return boost::any_cast<long>(_fields_map.at(static_cast<size_t>(idx))) - 1;
            }
            return idx;
        }

    public:
        DataSource() : _blank_record_flag(false), _has_event_id(boost::logic::indeterminate) {}
        virtual ~DataSource() {}

        void                               clearBlankRecordFlag() {_blank_record_flag=false;}
        bool                               detectBlankRecordFlag() const {return _blank_record_flag;}
        virtual long                       GetCurrentRecordIndex() const = 0;
        virtual long                       getNonBlankRecordsRead() const {return GetCurrentRecordIndex();}
        static DataSource                * GetNewDataSourceObject(const std::string& sSourceFilename, const CParameters::InputSource * source, BasePrint& Print);
        virtual long                       GetNumValues() = 0;
        virtual const char               * GetValueAt(long iFieldIndex) = 0;
        virtual const char               * GetValueAtUnmapped(long iFieldIndex) = 0;
        virtual void                       GotoFirstRecord() = 0;
        bool                               hasEventIdLinelistMapping() const;
        bool                               hasEventCoordinatesLinelistMapping() const;
        bool                               isLinelistOnlyColumn(long iFieldIndex) const;
        virtual bool                       ReadRecord() = 0;
        void                               tripBlankRecordFlag() {_blank_record_flag=true;}
        //void                               setFieldsMap(const std::vector<boost::any>& map) {_fields_map = map;}
        const FieldMapContainer_t        & getFieldsMap() const { return _fields_map;  }
        void                               setFieldsMap(const FieldMapContainer_t& map);
        const LineListFieldMapContainer_t& getLinelistFieldsMap() { return _linelist_fields_map; }
        void                               setLinelistFieldsMap(const LineListFieldMapContainer_t& map) { _linelist_fields_map = map; }
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
     std::string                        _read_buffer;

    void                                ThrowUnicodeException();

   public:
     AsciiFileDataSource(const std::string& sSourceFilename, BasePrint& Print, const char cDelimiter=' ');
     virtual ~AsciiFileDataSource() {}

     virtual long                       GetCurrentRecordIndex() const {return glReadCount;}
     virtual long                       getNonBlankRecordsRead() const {return glReadCount - glBlankReadCount;}
     virtual long                       GetNumValues() {
                                           long words = gStringParser->GetNumberWords();
                                           return _fields_map.size() > 0 ? std::min(static_cast<long>(_fields_map.size()), words) : words;
                                        }
     virtual const char               * GetValueAt(long iFieldIndex);
     virtual const char               * GetValueAtUnmapped(long iFieldIndex) { return GetValueAt(iFieldIndex); }
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

/** dBase file data source. */
class dBaseFileDataSource : public DataSource {
   private:
     std::auto_ptr<dBaseFile>           gSourceFile;
     std::string                        gsValue;
     unsigned long                      glNumRecords;
     unsigned long                      glCurrentRecord;

   public:
     dBaseFileDataSource(const std::string& sSourceFilename);
     virtual ~dBaseFileDataSource() {}

     virtual long                       GetCurrentRecordIndex() const;
     virtual long                       GetNumValues();
     virtual const char               * GetValueAt(long iFieldIndex);
     virtual const char               * GetValueAtUnmapped(long iFieldIndex);
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

/** CSV file data source. */
class CsvFileDataSource : public DataSource {
    private:
        long _readCount;
        long _blankReadCount;
        std::ifstream _sourceFile;
        std::string _delimiter;
        std::string _grouper;
        std::string _grouper_escape;
        unsigned long _skip;
        bool _firstRowHeaders;
        bool _ignore_empty_fields;
        std::vector<std::string> _read_values;
        BasePrint & _print;
        std::string _read_buffer;

        bool  parse(std::string& s, const std::string& delimiter=",", const std::string& grouper="\"");
        void  ThrowUnicodeException();

   public:
     CsvFileDataSource(const std::string& sSourceFilename, BasePrint& print, const std::string& delimiter=",", const std::string& grouper="\"", unsigned long skip=0, bool firstRowHeaders=false);
     virtual ~CsvFileDataSource() {}

     virtual long                       GetCurrentRecordIndex() const {return _readCount;}
     virtual long                       getNonBlankRecordsRead() const {return _readCount - _blankReadCount;}
     virtual long                       GetNumValues();
     virtual const char               * GetValueAt(long iFieldIndex);
     virtual const char               * GetValueAtUnmapped(long iFieldIndex);
     virtual void                       GotoFirstRecord();
     virtual bool                       ReadRecord();
};

/** dBase file data source. */
class ShapeFileDataSource : public DataSource {
    private:
        std::auto_ptr<dBaseFile>           _dbase_file;
        std::auto_ptr<ShapeFile>           _shape_file;
        std::string                        _read_buffer;
        unsigned long                      _num_records;
        unsigned long                      _current_record;

        bool                               _convert_utm;
        char                               _hemisphere;
        unsigned int                       _zone;
        double                             _northing;
        double                             _easting;

   public:
        ShapeFileDataSource(const std::string& sSourceFilename);
        virtual ~ShapeFileDataSource() {}

        static std::pair<bool, std::string> isSupportedProjection(const std::string& shapefileFilename);
        static std::pair<bool, std::string> isSupportedShapeType(const std::string& shapefileFilename);

        virtual long                       GetCurrentRecordIndex() const;
        virtual long                       GetNumValues();
        virtual const char               * GetValueAt(long iFieldIndex);
        virtual const char               * GetValueAtUnmapped(long iFieldIndex);
        virtual void                       GotoFirstRecord();
        virtual bool                       ReadRecord();
        void                               setUTMConversionInformation(char hemisphere, unsigned int zone, double northing, double easting) {
                                            _convert_utm=true;_hemisphere = hemisphere; _zone = zone; _northing = northing; _easting = easting;
                                           }
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
        virtual const char            * GetValueAtUnmapped(long iFieldIndex) { return GetValueAt(iFieldIndex); }
        virtual void                    GotoFirstRecord() {}
        virtual bool                    ReadRecord() {return true;}
};
//******************************************************************************
#endif
