// Adam J Vaughn
// November 2002

#include "SaTScan.h"
#pragma hdrstop

#include "stsLogLikelihood.h"

const char *	LOG_LIKELIHOOD_FILE_EXT		= ".llr";

// ============================================================================
// storage class for a log likelihood record
// ============================================================================

LogLikelihoodRecord::LogLikelihoodRecord() : BaseOutputRecord() {
   gdLogLikelihood = 0.0;
}

LogLikelihoodRecord::LogLikelihoodRecord(const double dLikelihood) : BaseOutputRecord() {
   gdLogLikelihood = dLikelihood;
}

LogLikelihoodRecord::~LogLikelihoodRecord() {
}

ZdFieldValue LogLikelihoodRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {
      if (iFieldNumber != 0)
         ZdGenerateException("Index out of range!", "Error!");
      
      BaseOutputRecord::SetFieldValueAsDouble(fv, gdLogLikelihood);
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()", "LogLikelihoodRecord");
      throw;
   }
   return fv;   
}

// ============================================================================
// Output data model for loglikelihood output file types.
// ============================================================================

// constructor
LogLikelihoodData::LogLikelihoodData(BasePrint *pPrintDirection, const CParameters& Parameters)
                          : BaseOutputStorageClass(pPrintDirection), gParameters(Parameters) {
   try {
      Init();
      Setup();
   }
   catch (ZdException &x) {
      if(pPrintDirection) {
         pPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
         pPrintDirection->SatScanPrintWarning("\nWarning - Unable to create data for Log Likelihood file.\n");
      }
   }
}

// destructor
LogLikelihoodData::~LogLikelihoodData() {
   try {
   }
   catch (...) { }
}

// add a new log likelihood record to the data
// pre: none
// post : adds a new record to the global vector
void LogLikelihoodData::AddLikelihood(const double dLikelihood) {
   LogLikelihoodRecord*	pRecord = 0;
   
   try {
      pRecord = new LogLikelihoodRecord(dLikelihood);
      BaseOutputStorageClass::AddRecord(pRecord);	
   }
   catch(ZdException &x) {
      delete pRecord;	
      gpPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
      gpPrintDirection->SatScanPrintWarning("\nWarning - Unable to record data for Log Likelihood output file.\n");
   }			
}

void LogLikelihoodData::Init() {
}

// internal setup function
void LogLikelihoodData::Setup() {
   try {
      ZdString sTempName(gParameters.GetOutputFileName().c_str());
      ZdString sExt(ZdFileName(gParameters.GetOutputFileName().c_str()).GetExtension());
      if(sExt.GetLength()) 
         sTempName.Replace(sExt, LOG_LIKELIHOOD_FILE_EXT);
      else
         sTempName << LOG_LIKELIHOOD_FILE_EXT;
      gsFileName = sTempName;

      SetupFields();
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "LogLikelihoodData");
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
       ::CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 7, 2, uwOffset);
     else  
       ::CreateField(gvFields, LOG_LIKL_FIELD, ZD_NUMBER_FLD, 7, 2, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "LogLikelihoodData");
      throw;
   }
}
 