//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioWriter.h"

const char * LoglikelihoodRatioWriter::TST_STAT_FIELD          = "TST_STAT";
const char * LoglikelihoodRatioWriter::LOG_LIKL_RATIO_FIELD    = "LLR";
const char * LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_EXT = ".llr";

/** constructor */
LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const CParameters& Parameters)
                         :AbstractDataFileWriter(Parameters), gpRecordBuffer(0) {
  try {
    Setup();
  }
  catch (ZdException &x) {
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
    CreateField((gParameters.GetLogLikelihoodRatioIsTestStatistic() ? TST_STAT_FIELD : LOG_LIKL_RATIO_FIELD), ZD_NUMBER_FLD, 11, 6, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","LoglikelihoodRatioWriter");
    throw;
  }
}

/** internal setup */
void LoglikelihoodRatioWriter::Setup() {
  try {
    DefineFields();
    gpRecordBuffer = new RecordBuffer(vFieldDefinitions);
    if (gParameters.GetOutputSimLoglikeliRatiosAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, LOG_LIKELIHOOD_FILE_EXT);
    if (gParameters.GetOutputSimLoglikeliRatiosDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, LOG_LIKELIHOOD_FILE_EXT);
  }
  catch (ZdException &x) {
    delete gpRecordBuffer;
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("Setup()","LoglikelihoodRatioWriter");
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


