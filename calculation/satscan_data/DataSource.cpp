//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "SSException.h"
#include "FileName.h"
#include "DateStringParser.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::GetNewDataSourceObject(const std::string& sSourceFilename, const CParameters::InputSource * source, BasePrint& Print) {
    // if a InputSource is not defined, default to space delimited ascii source
    if (!source)
        return new CsvFileDataSource(sSourceFilename, Print, " ");
    // return data source object by input source type
    DataSource * dataSource=0;
    switch (source->getSourceType()) {
        case DBASE           : dataSource = new dBaseFileDataSource(sSourceFilename); break;
        case SHAPE           : dataSource = new ShapeFileDataSource(sSourceFilename); break;
        case CSV             : dataSource = new CsvFileDataSource(sSourceFilename, Print, source->getDelimiter(), source->getGroup(), source->getSkip(), source->getFirstRowHeader()); break;
        default              : dataSource = new CsvFileDataSource(sSourceFilename, Print, " ");
    }
    dataSource->setFieldsMap(source->getFieldsMap());
    return dataSource;
}

//******************* class AsciiFileDataSource ********************************

/** Returns number of words in string -- this could be better  */
long AsciiFileDataSource::StringParser::GetNumberWords() {
  long   iWords=0;

  while (GetWord(iWords) != 0) ++iWords;
  return iWords;
}

const char * AsciiFileDataSource::GetValueAt(long iFieldIndex) {
    // check whether the field at this index is mapped to another field
    if (iFieldIndex < static_cast<long>(_fields_map.size())) {
        if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(FieldType)) {
            // This field is one of the FieldType values, which are not actually fields in the data source.
            _read_buffer.clear();
            FieldType type = boost::any_cast<FieldType>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
            switch (type) {
                case GENERATEDID: _read_buffer = printString(_read_buffer, "location%u", getNonBlankRecordsRead()); break;
                case ONECOUNT: _read_buffer = "1"; break;
                case DEFAULT_DATE: _read_buffer = DateStringParser::UNSPECIFIED; break;
                case BLANK: break;
                default : throw prg_error("Unknown FieldType enumeration %d.","GetValueAt()", type);
            }
            return _read_buffer.c_str();
        } else if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(ShapeFieldType)) {
            // AsciiFileDataSource does not implemenet the ShapeFieldType mapping -- that's only for shapefiles.
            throw prg_error("AsciiFileDataSource::GetValueAt() not supported with ShapeFieldType.","AsciiFileDataSource::GetValueAt()");
        } else {
            // This field is mapped to another column of the data source.
            return gStringParser->GetWord(tranlateFieldIndex(iFieldIndex));
        }
    } else {
        // This field is not mapped to another field type or column -- just get the value.
        return gStringParser->GetWord(iFieldIndex);
    }
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
                    :DataSource(), glReadCount(0), glBlankReadCount(0), gPrint(Print), gStringParser(new StringParser(Print, cDelimiter)) {
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
  glBlankReadCount = 0;
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
          ++glBlankReadCount;
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
                 :DataSource(), gwCurrentFieldIndex(-1), glCurrentRecord(0) {
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
    return _fields_map.size() ? static_cast<long>(_fields_map.size()) : gSourceFile->GetNumFields();
}

/** Returns iFieldIndex'th field value from current record. */
const char * dBaseFileDataSource::GetValueAt(long iFieldIndex) {
    gsValue.clear();

    // The available fields are either defined by the fields map or by the columns of file.
    if (_fields_map.size()) {
        // check whether the field at this index is mapped to another field
        if (iFieldIndex < static_cast<long>(_fields_map.size())) {
            if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(FieldType)) {
                // This field is one of the FieldType values, which are not actually fields in the data source.
                FieldType type = boost::any_cast<FieldType>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
                switch (type) {
                    case GENERATEDID: printString(gsValue, "location%u", getNonBlankRecordsRead()); break;
                    case ONECOUNT: gsValue = "1"; break;
                    case DEFAULT_DATE: gsValue = DateStringParser::UNSPECIFIED; break;
                    case BLANK : break;
                    default : throw prg_error("Unknown FieldType enumeration %d.","GetValueAt()", type);
                }
                return gsValue.c_str();
            } else if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(ShapeFieldType)) {
                // dBaseFileDataSource does not implemenet the ShapeFieldType mapping -- that's only for shapefiles.
                throw prg_error("dBaseFileDataSource::GetValueAt() not supported with ShapeFieldType.","dBaseFileDataSource::GetValueAt()");
            } else {
                // This field is mapped to another column of the data source.
                iFieldIndex = tranlateFieldIndex(iFieldIndex);
            }
        } else {
            // index beyond defined mappings
            return 0;
        }
    }

    // now retrieve value
    if (iFieldIndex > (long)(gSourceFile->GetNumFields() - 1))
        return 0;
    gSourceFile->GetSystemRecord()->GetFieldValue(iFieldIndex, gsValue);
    if (gSourceFile->GetSystemRecord()->GetFieldType(iFieldIndex) == FieldValue::DATE_FLD && gsValue.size() == SaTScan::Timestamp::DATE_FLD_LEN) {
        //format date fields -- read process currently expects yyyy/mm/dd
        gsValue.insert(4, "/");
        gsValue.insert(7, "/");
    }
    return gsValue.c_str();
}

/** Positions read cursor to first record. */
void dBaseFileDataSource::GotoFirstRecord() {
    if (glNumRecords) {
        gSourceFile->GotoRecord(1);
        glCurrentRecord = 0;
        gwCurrentFieldIndex=-1;
    }  
}

/** Either reads first file record in file or next after current record. */
bool dBaseFileDataSource::ReadRecord() {
    if (glCurrentRecord >= glNumRecords) return false;
    gwCurrentFieldIndex=-1;
    gSourceFile->GotoRecord(++glCurrentRecord);
    return true;  
}

//******************* class CsvFileDataSource ********************************

/** constructor */
CsvFileDataSource::CsvFileDataSource(const std::string& sSourceFilename, BasePrint& print, const std::string& delimiter, const std::string& grouper, unsigned long skip, bool firstRowHeaders)
                  :DataSource(), _readCount(0), _blankReadCount(0), _print(print), _delimiter(delimiter), _grouper(grouper), _skip(skip), _ignore_empty_fields(false), _firstRowHeaders(firstRowHeaders) {
    // special processing for 'whitespace' delimiter string
    if (_delimiter == "" || _delimiter == " ") {
        _delimiter = "\t\v\f\r\n ";
        _ignore_empty_fields = true;
    }
    _sourceFile.open(sSourceFilename.c_str());
    if (!_sourceFile)
        throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
    // Get the byte-order mark, if there is one
    unsigned char bom[4];
    _sourceFile.read(reinterpret_cast<char*>(bom), 4);
    //Since we don't know what the endian was on the machine that created the file we
    //are reading, we'll need to check both ways.
    if ((bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf) ||             // utf-8
        (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xfe && bom[3] == 0xff) || // UTF-32, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0 && bom[3] == 0) || // UTF-32, little-endian
        (bom[0] == 0xfe && bom[1] == 0xff) ||                               // UTF-16, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe))                                 // UTF-16, little-endian
      ThrowUnicodeException();
    _sourceFile.seekg(0L, std::ios::beg);
    // if first row is header, increment number of rows to skip
    if (_firstRowHeaders) ++_skip;
}

/** constructor */
CsvFileDataSource::CsvFileDataSource(const std::string& sSourceFilename, BasePrint& print, unsigned long skip, bool firstRowHeaders)
                  :DataSource(), _readCount(0), _blankReadCount(0), _print(print), _delimiter("\t\v\f\r\n "), _grouper("\""), _skip(skip), _ignore_empty_fields(false), _firstRowHeaders(firstRowHeaders) {
    _sourceFile.open(sSourceFilename.c_str());
    if (!_sourceFile)
        throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
    // Get the byte-order mark, if there is one
    unsigned char bom[4];
    _sourceFile.read(reinterpret_cast<char*>(bom), 4);
    //Since we don't know what the endian was on the machine that created the file we
    //are reading, we'll need to check both ways.
    if ((bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf) ||             // utf-8
        (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xfe && bom[3] == 0xff) || // UTF-32, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0 && bom[3] == 0) || // UTF-32, little-endian
        (bom[0] == 0xfe && bom[1] == 0xff) ||                               // UTF-16, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe))                                 // UTF-16, little-endian
      ThrowUnicodeException();
    _sourceFile.seekg(0L, std::ios::beg);
    // if first row is header, increment number of rows to skip
    if (_firstRowHeaders) ++_skip;

    // read first record and try to guess the format
    bool isBlank = true;
    std::string readbuffer;
    getlinePortable(_sourceFile, readbuffer);
    while (isBlank && getlinePortable(_sourceFile, readbuffer)) {
        trimString(readbuffer);
        isBlank = readbuffer.size() == 0;
    }
    if (readbuffer.size()) {
        // whitespace delimited
        parse(readbuffer, "\t\v\f\r\n ", _grouper);
        std::vector<std::string> whitespace_values(_values);
        // comma delimited
        parse(readbuffer, ",", _grouper);
        std::vector<std::string> comma_values(_values);
        // This is poor but their isn't really a good method for guessing. The field types of input fields are generally: string, integer, double, formatted date
        // The identifier field is basically the only place we might reasonably expect a comma (e.g. identifier = Baltimore,MD). In that situation, this routine will
        // guess wrong for a record like: Baltimore,MD  5 2014/06/17
        if (comma_values.size() > 1) _delimiter = ",";
    }
}

/** Re-positions file cursor to beginning of file. */
void CsvFileDataSource::GotoFirstRecord() {
    _readCount = 0;
    _blankReadCount = 0;
    _sourceFile.clear();
    _sourceFile.seekg(0L, std::ios::beg);
    std::string readbuffer;
    for (int i=0; i < _skip; ++i) 
        getlinePortable(_sourceFile, readbuffer);
}

/** sets current parsing string -- returns indication of whether string contains any words. */
bool CsvFileDataSource::parse(const std::string& s, const std::string& delimiter, const std::string& grouper) {
    _values.clear();
    std::string e("\\");
    boost::escaped_list_separator<char> separator(e, delimiter, grouper);
    boost::tokenizer<boost::escaped_list_separator<char> > values(s, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=values.begin(); itr != values.end(); ++itr) {
        _values.push_back(*itr);
        //trim any whitespace around value
        boost::trim(_values.back());
        // ignore empty values if delimiter is whitespace -- boost::escaped_list_separator does not consume adjacent whitespace delimiters
        if (!_values.back().size() && _ignore_empty_fields)
            _values.pop_back();
    }
    return _values.size() > 0;
}

/** Attempts to read line from source and parse into 'words'. 
    Lines that contain no words are skipped and scanning continues to next line in file. 
    Returns indication of whether record buffer contains 'words'.*/
bool CsvFileDataSource::ReadRecord() {
    bool isBlank = true;
    std::string readbuffer;

    // skip records as necessary
    for (long i=_readCount; i < _skip; ++i) 
        getlinePortable(_sourceFile, readbuffer);

    while (isBlank && getlinePortable(_sourceFile, readbuffer)) {
        isBlank = !parse(readbuffer, _delimiter, _grouper);
        if (isBlank) {
            tripBlankRecordFlag();
            ++_readCount;
            ++_blankReadCount;
        }
    }

    ++_readCount;
    return (readbuffer.size() > 0 && GetNumValues());
}

/** Returns number of values */
long CsvFileDataSource::GetNumValues() {
    // Field maps are all or nothing. This means that all fields are defined in mapping or straight from record parse.
    return _fields_map.size() ? static_cast<long>(_fields_map.size()) : static_cast<long>(_values.size());
}

const char * CsvFileDataSource::GetValueAt(long iFieldIndex) {
    // see if value at field index is mapped FieldType
    if (_fields_map.size()) {
        if (iFieldIndex < static_cast<long>(_fields_map.size())) {  
            if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(FieldType)) {
                _read_buffer.clear();
                FieldType type = boost::any_cast<FieldType>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
                switch (type) {
                    case GENERATEDID: printString(_read_buffer, "location%u", getNonBlankRecordsRead()); break;
                    case ONECOUNT: _read_buffer = "1"; break;
                    case DEFAULT_DATE: _read_buffer = DateStringParser::UNSPECIFIED; break;
                    case BLANK: break;
                    default : throw prg_error("Unknown FieldType enumeration %d.","GetValueAt()", type);
                }
                return _read_buffer.c_str();
            } else if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(ShapeFieldType)) {
                // CsvFileDataSource does not implemenet the ShapeFieldType mapping -- they are only for shapefiles.
                throw prg_error("CsvFileDataSource::GetValueAt() not supported with ShapeFieldType.","CsvFileDataSource::GetValueAt()");
            } else {
                // This field is mapped to another column of the data source.
                iFieldIndex = tranlateFieldIndex(iFieldIndex);
            }
        } else {
            // index beyond defined mappings
            return 0;
        }
    }
    if (iFieldIndex > static_cast<long>(_values.size()) - 1)
        return 0;
    return _values.at(static_cast<size_t>(iFieldIndex)).c_str();
}

void CsvFileDataSource::ThrowUnicodeException() {
    throw resolvable_error("Error: The %s contains data that is Unicode formatted.\n       Please see 'ASCII Input File Format' in the user guide for help.\n",
                             _print.GetImpliedFileTypeString().c_str());
}

//******************* class ShapeFileDataSource ********************************

/* Returns boolean indication as to whether associated projection is supported. If projection
   is not supported, return value includes a message to indicating a reason why it is not supported. */
std::pair<bool, std::string> ShapeFileDataSource::isSupportedProjection(const std::string& shapefileFilename) {
    return std::make_pair(true, "");
    //ShapeFile shapeFile(shapefileFilename.c_str(), "r", false);
    //if (shapeFile.hasProjection()) {
    //    const Projection & projection = shapeFile.getProjection();
    //    Projection::utm_info_t utm_info = projection.guessUTM();
    //    return std::make_pair(true, "");
    //} else {
    //    std::string buffer;
    //    shapeFile.getProjectionFilename(shapefileFilename, buffer);
    //    return std::make_pair(false, printString(buffer, "Projection file '%s' could not be opened or is missing.", buffer.c_str()));
    //}
}

/* Returns boolean indication as to whether shape type of file is supported. If not supported, 
   return value includes a message to indicating a reason why it is not supported. */
std::pair<bool, std::string> ShapeFileDataSource::isSupportedShapeType(const std::string& shapefileFilename) {
    std::string buffer;
    try {
        ShapeFile shapeFile(shapefileFilename.c_str(), "r", false);
        if (!(shapeFile.getType() == SHPT_POINT || shapeFile.getType() == SHPT_POLYGON))
            return std::make_pair(false, printString(buffer, "The shape type of file is not supported - type is %s. Supported types are: %s and %s.",
                                                     shapeFile.getTypeAsString(shapeFile.getType()),
                                                     shapeFile.getTypeAsString(SHPT_POINT),
                                                     shapeFile.getTypeAsString(SHPT_POLYGON)
                                                     ));
    } catch (...) {        
        return std::make_pair(false, printString(buffer, "Unable to read file '%s'.", shapefileFilename.c_str()));
    }
    return std::make_pair(true, buffer);
}

ShapeFileDataSource::ShapeFileDataSource(const std::string& sSourceFilename)
                :DataSource(), _current_field_idx(-1), _current_record(0), _convert_utm(false) {
    // check that shape type is supported
    std::pair<bool, std::string> supportedType = isSupportedShapeType(sSourceFilename);
    if (!supportedType.first)
        throw resolvable_error("%s", supportedType.second.c_str());
    // check that projection is present and supported
    std::pair<bool, std::string> supportedProjection = isSupportedProjection(sSourceFilename);
    if (!supportedProjection.first)
        throw resolvable_error("%s", supportedProjection.second.c_str());

    _shape_file.reset(new ShapeFile(sSourceFilename.c_str(), "r", false));
    _num_records = _shape_file->getEntityCount();
    // open associated dBase file
    FileName file(sSourceFilename.c_str());
    file.setExtension(dBaseFile::GetFileTypeExtension());
    std::string buffer;
    if (ValidateFileAccess(file.getFullPath(buffer), false))
        _dbase_file.reset(new dBaseFile(file.getFullPath(buffer).c_str()));
}

/** Returns current record index. */
long ShapeFileDataSource::GetCurrentRecordIndex() const {
  return static_cast<long>(_current_record);
}

/** Returns the number of fields in record buffer. */
long ShapeFileDataSource::GetNumValues() {
    //long count=4; /* the generatedid, onecount, latitude/longitude coordinates are implicit in the count */
    //if (_dbase_file.get())
    //    count += _dbase_file->GetNumFields();
    return static_cast<long>(_fields_map.size());
}

/** Returns iFieldIndex'th field value from current record. */
const char * ShapeFileDataSource::GetValueAt(long iFieldIndex) {
    _read_buffer.clear();

    if (iFieldIndex < static_cast<long>(_fields_map.size())) {
        if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(FieldType)) {
            FieldType type = boost::any_cast<FieldType>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
            switch (type) {
                case GENERATEDID: _read_buffer = printString(_read_buffer, "location%u", _current_record); break;
                case ONECOUNT: _read_buffer = "1"; break;
                case DEFAULT_DATE: _read_buffer = DateStringParser::UNSPECIFIED; break;
                case BLANK: break;
                default : throw prg_error("Unknown FieldType enumeration %d.","GetValueAt()", type);
            }
        } else if (_fields_map.at(static_cast<size_t>(iFieldIndex)).type() == typeid(ShapeFieldType)) {
            ShapeFieldType type = boost::any_cast<ShapeFieldType>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
            switch (type) {
                case POINTX:
                case POINTY: {
                    double x, y;
                    _shape_file->getShapeAsXY(static_cast<int>(_current_record - 1), x, y);
                    if (_convert_utm) {
                        UTM_To_LatitudeLongitude(y, x, _hemisphere, _zone, y, x);
                    }
                    printString(_read_buffer, "%lf", (type == POINTX ? y : x));
                } break;
                default : throw prg_error("Unknown ShapeFieldType enumeration %d.","GetValueAt()", type);
            }
        } else {
            iFieldIndex = boost::any_cast<long>(_fields_map.at(static_cast<size_t>(iFieldIndex)));
            if (!_dbase_file.get() || iFieldIndex > (long)(_dbase_file->GetNumFields()))
                return 0;
            _dbase_file->GetSystemRecord()->GetFieldValue(iFieldIndex - 1, _read_buffer);
            if (_dbase_file->GetSystemRecord()->GetFieldType(iFieldIndex - 1) == FieldValue::DATE_FLD && _read_buffer.size() == SaTScan::Timestamp::DATE_FLD_LEN) {
                //format date fields -- read process currently expects yyyy/mm/dd
                _read_buffer.insert(4, "/");
                _read_buffer.insert(7, "/");
            }
        }
        _current_field_idx = iFieldIndex;
    }
    return _read_buffer.size() ? _read_buffer.c_str() : 0;
}

/** Positions read cursor to first record. */
void ShapeFileDataSource::GotoFirstRecord() {
    if (_num_records) {
        _current_record = 0;
        _current_field_idx=-1;
    }  
}

/** Either reads first file record in file or next after current record. */
bool ShapeFileDataSource::ReadRecord() {
    if (_current_record >= _num_records) return false;
    _current_field_idx=-1;
    if (_dbase_file.get())
        _dbase_file->GotoRecord(++_current_record);
    return true;
}
