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
       CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 7, 2, uwOffset);
     else
       CreateField(gvFields, LOG_LIKL_FIELD, ZD_NUMBER_FLD, 7, 2, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "LogLikelihoodData");
      throw;
   }
}

