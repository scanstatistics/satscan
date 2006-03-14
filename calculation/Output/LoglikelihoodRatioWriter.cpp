//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioWriter.h"

const char * LoglikelihoodRatioWriter::TST_STAT_FIELD          = "TST_STAT";
const char * LoglikelihoodRatioWriter::LOG_LIKL_RATIO_FIELD    = "LLR";
const char * LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_EXT = ".llr";

/** constructor */
LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const CParameters& Parameters, bool bAppend)
                         :AbstractDataFileWriter(Parameters), gpRecordBuffer(0) {
  try {
    DefineFields();
    gpRecordBuffer = new RecordBuffer(vFieldDefinitions);
    if (gParameters.GetOutputSimLoglikeliRatiosAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, LOG_LIKELIHOOD_FILE_EXT, bAppend);
    if (gParameters.GetOutputSimLoglikeliRatiosDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, LOG_LIKELIHOOD_FILE_EXT, bAppend);
  }
  catch (ZdException &x) {
    delete gpRecordBuffer; gpRecordBuffer=0;
    delete gpASCIIFileWriter; gpASCIIFileWriter=0;
    delete gpDBaseFileWriter; gpDBaseFileWriter=0;
    x.AddCallpath("constructor()","LoglikelihoodRatioWriter");
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
    CreateField(vFieldDefinitions, (gParameters.GetLogLikelihoodRatioIsTestStatistic() ? TST_STAT_FIELD : LOG_LIKL_RATIO_FIELD), ZD_NUMBER_FLD, 19, 6, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","LoglikelihoodRatioWriter");
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
  catch (ZdException &x) {
    x.AddCallpath("Write()","LoglikelihoodRatioWriter");
    throw;
  }
}


