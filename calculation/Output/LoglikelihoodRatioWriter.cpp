//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioWriter.h"
#include "SSException.h"

const char * LoglikelihoodRatioWriter::TST_STAT_FIELD          = "TST_STAT";
const char * LoglikelihoodRatioWriter::LOG_LIKL_RATIO_FIELD    = "LLR";
const char * LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_EXT = ".llr";
const char * LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_HA_EXT = ".llr_ha";

/** constructor */
LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const CParameters& Parameters, bool bAppend, bool powerStep)
                         :AbstractDataFileWriter(Parameters), gpRecordBuffer(0) {
  try {
    DefineFields();
    gpRecordBuffer = new RecordBuffer(vFieldDefinitions);
    if (gParameters.GetOutputSimLoglikeliRatiosAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, vFieldDefinitions, powerStep ? LOG_LIKELIHOOD_FILE_HA_EXT : LOG_LIKELIHOOD_FILE_EXT, bAppend);
    if (gParameters.GetOutputSimLoglikeliRatiosDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, powerStep ? LOG_LIKELIHOOD_FILE_HA_EXT : LOG_LIKELIHOOD_FILE_EXT, bAppend);
  }
  catch (prg_exception& x) {
    delete gpRecordBuffer; gpRecordBuffer=0;
    delete gpASCIIFileWriter; gpASCIIFileWriter=0;
    delete gpDBaseFileWriter; gpDBaseFileWriter=0;
    x.addTrace("constructor()","LoglikelihoodRatioWriter");
    throw;
  }
}

/** destructor */
LoglikelihoodRatioWriter::~LoglikelihoodRatioWriter() {
  try {
    delete gpRecordBuffer;
  }
  catch (...) {}
}

/** Defines fields of output file. */
void LoglikelihoodRatioWriter::DefineFields() {
  unsigned short uwOffset=0;

  try {
    CreateField(vFieldDefinitions, (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? TST_STAT_FIELD : LOG_LIKL_RATIO_FIELD), FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
  }
  catch (prg_exception& x) {
    x.addTrace("DefineFields()","LoglikelihoodRatioWriter");
    throw;
  }
}

/** Writes data to each requested file type. */
void LoglikelihoodRatioWriter::Write(double dLoglikelihoodRatio) {
  try {
    gpRecordBuffer->GetFieldValue(0).AsDouble() = dLoglikelihoodRatio;
    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(*gpRecordBuffer);
    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(*gpRecordBuffer);
  }
  catch (prg_exception& x) {
    x.addTrace("Write()","LoglikelihoodRatioWriter");
    throw;
  }
}


