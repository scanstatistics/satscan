//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsLogLikelihood.h"

/** class constructor */
LogLikelihoodData::LogLikelihoodData(const CParameters& Parameters)
                  :BaseOutputStorageClass(), gParameters(Parameters) {
  try {
    SetupFields();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor","LogLikelihoodData");
    throw;
  }
}

/** class destructor */
LogLikelihoodData::~LogLikelihoodData() {}

const char * LogLikelihoodData::LOG_LIKELIHOOD_FILE_EXT		= ".llr";

// add a new log likelihood ratio record to the data
// pre: none
// post : adds a new record to the global vector
void LogLikelihoodData::AddLikelihoodRatio(double dLikelihoodRatio) {
  OutputRecord               * pRecord = 0;

  try {
    pRecord = new OutputRecord(gvFields);
    pRecord->GetFieldValue(0).AsDouble() = dLikelihoodRatio;
    BaseOutputStorageClass::AddRecord(pRecord);
  }
  catch(ZdException &x) {
    delete pRecord;
    x.AddCallpath("AddLikelihoodRatio()","LogLikelihoodData");
    throw;
  }
}

// sets up the appropraite fields for a log likelihood output data model
// pre : none
// post : vFields will contain the appropraite ZdFields for this output data model
void LogLikelihoodData::SetupFields() {
   unsigned short uwOffset = 0;     // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
   
   try {
     if (gParameters.GetLogLikelihoodRatioIsTestStatistic())
       CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
     else
       CreateField(gvFields, LOG_LIKL_RATIO_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "LogLikelihoodData");
      throw;
   }
}

/** class constructor */
RecordBuffer::RecordBuffer(const ZdPointerVector<ZdField>& vFields) {
  try {
    Setup(vFields);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","RecordBuffer");
    throw;
  }
}

/** class destructor */
RecordBuffer::~RecordBuffer() {}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool RecordBuffer::GetFieldIsBlank(unsigned int iFieldNumber) const {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      ZdGenerateException("Invalid index, out of range!", "GetFieldIsBlank()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldIsBlank()","RecordBuffer");
    throw;
  }
  return gvBlankFields[iFieldNumber];
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
ZdFieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Returns ZdFieldValue reference for field index. Throws ZdException if
    iFieldIndex is greater than number of ZdFieldValues. */
const ZdFieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      ZdGenerateException("Invalid index, out of range", "GetFieldValue()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank) {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      ZdGenerateException("Invalid index, out of range!", "SetFieldIsBlank()");
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

/** Creates vector of field values that mirror defined fields of passed
    vector. Initializes list of booleans that reflect whether a field
    contains data or not. */
void RecordBuffer::Setup(const ZdPointerVector<ZdField>& vFields) {
  size_t        t;

  try {
    for (t=0; t < vFields.size(); ++t) {
       gvFieldValues.push_back(ZdFieldValue(vFields[t]->GetType()));
       gvBlankFields.push_back(false);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","RecordBuffer");
    throw;
  }
}

const char * AbstractDataWriter::CLUST_NUM_FIELD 	  = "CLUSTER";
const char * AbstractDataWriter::LOC_ID_FIELD   	  = "LOC_ID";
const char * AbstractDataWriter::P_VALUE_FLD  	          = "P_VALUE";
const char * AbstractDataWriter::OBSERVED_FIELD	          = "OBSERVED";
const char * AbstractDataWriter::EXPECTED_FIELD	          = "EXPECTED";
const char * AbstractDataWriter::LOG_LIKL_RATIO_FIELD     = "LLR";
const char * AbstractDataWriter::TST_STAT_FIELD           = "TST_STAT";

/** constructor */
AbstractDataWriter::AbstractDataWriter(const CParameters& Parameters)
                   :gParameters(Parameters), gpASCIIFileWriter(0), gpDBaseFileWriter(0), gpRecordBuffer(0) {
}

/** destructor */
AbstractDataWriter::~AbstractDataWriter() {
  delete gpRecordBuffer;
  delete gpASCIIFileWriter;
  delete gpDBaseFileWriter;
}

void AbstractDataWriter::CreateField(const std::string& sFieldName, char cType, short wLength, short wPrecision,
                                     unsigned short& uwOffset, bool bCreateIndex) {
  ZdField  *pField = 0;

  try {
    pField = new ZdField;
    pField->SetName(sFieldName.c_str());
    pField->SetType(cType);
    pField->SetLength(wLength);
    pField->SetPrecision(wPrecision);
    pField->SetOffset(uwOffset);
    uwOffset += wLength;
    if (bCreateIndex)
      pField->SetIndexCount(1);
    gvFields.push_back(pField);
  }
  catch (ZdException &x) {
    delete pField; pField = 0;
    x.AddCallpath("CreateField()","AbstractDataFileWriter");
    throw;
  }			
}

// function to get the field number of a field given its name
// pre: none
// post: if field with given name exists in the vector then will return the position, else will
//       throw a not found exception
unsigned int AbstractDataWriter::GetFieldNumber(const ZdString& sFieldName) const {
  bool                 bFound(false);
  unsigned int         i, iPosition;

  try {
    for (i=0; i < gvFields.size() && !bFound; ++i) {
       bFound = (!strcmp(gvFields[i]->GetName(), sFieldName));
       iPosition = i;
   }
   if (!bFound)
     ZdException::GenerateNotification("Field name %s not found in the field vector.",
                                       "GetFieldNumber()", sFieldName.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFieldNumber()","BaseOutputStorageClass");
    throw;
  }
  return iPosition;
}

const char * LoglikelihoodRatioWriter::TST_STAT_FIELD       = "TST_STAT";
const char * LoglikelihoodRatioWriter::LOG_LIKL_RATIO_FIELD = "LLR";

LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(boost::mutex& Mutex, const CParameters& Parameters)
                         :AbstractDataWriter(Parameters), gMutex(Mutex) {

  try {
    SetupFields();
    gpRecordBuffer = new RecordBuffer(gvFields);
    if (Parameters.GetOutputSimLoglikeliRatiosAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(Parameters, ".llr");
    if (Parameters.GetOutputSimLoglikeliRatiosDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(Parameters, gvFields, ".llr");
  }
  catch (ZdException &x) {
    delete gpRecordBuffer;
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("constructor()","LoglikelihoodRatioWriter");
    throw;
  }
}	

LoglikelihoodRatioWriter::~LoglikelihoodRatioWriter() {}

void LoglikelihoodRatioWriter::SetupFields() {
   unsigned short uwOffset = 0;
   
   try {
     if (gParameters.GetLogLikelihoodRatioIsTestStatistic())
       CreateField(TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
     else
       CreateField(LOG_LIKL_RATIO_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()","LoglikelihoodRatioWriter");
      throw;
   }
}

void LoglikelihoodRatioWriter::Write(double dLoglikelihoodRatio) {
  boost::mutex::scoped_lock     lock(gMutex);

  gpRecordBuffer->GetFieldValue(0).AsDouble() = dLoglikelihoodRatio;

  if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(gvFields, *gpRecordBuffer);
  if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(gvFields, *gpRecordBuffer);
}


