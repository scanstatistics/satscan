// Adam J Vaughn
// November 2002

#include "SaTScan.h"
#pragma hdrstop

#include "stsRelativeRisk.h"

const char *	REL_RISK_EXT		= ".rr";

// ============================================================================
// record data model for the relative risk file
// ============================================================================

RelativeRiskRecord::RelativeRiskRecord() : BaseOutputRecord() {
   Init();
}

RelativeRiskRecord::~RelativeRiskRecord() {
}

ZdFieldValue RelativeRiskRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {     
      if (iFieldNumber < 0 || iFieldNumber >= GetNumFields())
         ZdGenerateException ("Index out of range!", "Error!");
      switch (iFieldNumber) {
         case 0:
            BaseOutputRecord::SetFieldValueAsString(fv, gsLocationID);  break;
         case 1:
            BaseOutputRecord::SetFieldValueAsDouble(fv, double(glObserved));  break;
         case 2:
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdExpected);  break;
         case 3:
            BaseOutputRecord::SetFieldValueAsString(fv, gsRelRisk);  break;
         default :
            ZdGenerateException ("Invalid index, out of range", "Error!");
      } 
      return fv;   
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()" , "RelativeRiskRecord");
      throw;
   }
}

void RelativeRiskRecord::Init() {
   gsLocationID = "";
   glObserved = 0;
   gdExpected = 0.0;
   gsRelRisk = 0.0;
}

// ============================================================================
// data model for the relative risk output file type
// ============================================================================

// constructor
RelativeRiskData::RelativeRiskData(BasePrint *pPrintDirection, const ZdString& sOutputFileName) 
                           : BaseOutputStorageClass(pPrintDirection) {
   try {
      Init();
      Setup(sOutputFileName);
   }
   catch (ZdException &x) {
      if(pPrintDirection) {
         pPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
         pPrintDirection->SatScanPrintWarning("\nWarning - Unable to create relative risk output file.\n");
      }
   }
}

// destructor
RelativeRiskData::~RelativeRiskData() {
   try {
   }
   catch (...) { }
}

void RelativeRiskData::Init() {
}

//
void RelativeRiskData::SetRelativeRiskData(const ZdString& sLocationID, const long lObserved, 
                                           const double dExpected, const ZdString& sRelRisk) {
   RelativeRiskRecord*	pRecord = 0;

   try {
      pRecord = new RelativeRiskRecord();
      pRecord->SetExpected(dExpected);
      pRecord->SetLocationID(sLocationID);
      pRecord->SetObserved(lObserved);
      pRecord->SetRelativeRisk(sRelRisk);
      BaseOutputStorageClass::AddRecord(pRecord);
   }  
   catch (ZdException &x) {
      delete pRecord;
      gpPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
      gpPrintDirection->SatScanPrintWarning("\nWarning - Unable to record relative risk output data.\n");
   }	 	
} 

// internal setup function
void RelativeRiskData::Setup(const ZdString& sOutputFileName) {
   try {
      ZdString sTempName(sOutputFileName);
      ZdString sExt(ZdFileName(sOutputFileName).GetExtension());
      if(sExt.GetLength()) 
         sTempName.Replace(sExt, REL_RISK_EXT);
      else
         sTempName << REL_RISK_EXT;
      gsFileName = sTempName;

      SetupFields();
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "RelativeRiskData");
      throw;
   }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void RelativeRiskData::SetupFields() {
   unsigned short uwOffset = 0;     // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
   
   try {
      ::CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
      ::CreateField(gvFields, OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      ::CreateField(gvFields, EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      ::CreateField(gvFields, REL_RISK_FIELD, ZD_ALPHA_FLD, 12, 0, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "RelativeRiskData");
      throw;
   }
}
