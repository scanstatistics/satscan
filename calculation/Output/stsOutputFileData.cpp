// Adam J Vaughn
// November 2002

#include "SaTScan.h"
#pragma hdrstop

#include "stsOutputFileData.h"

const char *    RUN_NUM_FIELD		        = "RUN_NUM";
const char *    CLUST_NUM_FIELD 		= "CLUST_NUM";
const char *    LOC_ID_FIELD   		        = "LOC_ID";
const char *    P_VALUE_FIELD  		        = "P_VALUE";
const char *    START_DATE_FIELD		= "START_DATE";
const char *    END_DATE_FIELD		        = "END_DATE";
const char *    OBSERVED_FIELD		        = "OBSERVED";
const char *    EXPECTED_FIELD		        = "EXPECTED";
const char *    REL_RISK_FIELD		        = "REL_RISK";
const char *    LOG_LIKL_FIELD		        = "LOG_LIKL";
const char *    NUM_AREAS_FIELD		        = "NUM_AREAS";
const char *    COORD_LAT_FIELD		        = "LATITUDE";
const char *    COORD_LONG_FIELD	        = "LONGITUDE";
const char *    COORD_X_FIELD		        = "X";
const char *    COORD_Y_FIELD                   = "Y";
const char *    COORD_Z_FIELD                   = "Z";
const char *    RADIUS_FIELD		        = "RADIUS";
const char *    AREA_OBS_FIELD                  = "AREA_OBS";
const char *    AREA_EXP_FIELD                  = "AREA_EXP";
const char *    AREA_RSK_FIELD                  = "AREA_RSK";
const char *    E_ANGLE_FIELD                   = "E_ANGLE";
const char *    E_SHAPE_FIELD                   = "E_SHAPE";
const char *    CLU_OBS_FIELD                   = "CLU_OBS";
const char *    CLU_EXP_FIELD                   = "CLU_EXP";
const char *    TST_STAT_FIELD                  = "TST_STAT";

// base class for all output file record structures
BaseOutputRecord::BaseOutputRecord() {
}

BaseOutputRecord::~BaseOutputRecord() {
}	

// sets the filed value type to double and value to dValue
// pre : none
// post : fv has type double and value dValue   
void BaseOutputRecord::SetFieldValueAsDouble(ZdFieldValue& fv, const double dValue) {
   try {
      fv.SetType(ZD_NUMBER_FLD);
      fv.AsDouble() = dValue;
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldValueAsDouble()", "BaseOutputRecord");
      throw;
   }
}

// sets the filed value type to long and value to lValue
// pre : none
// post : fv has type long and value lValue 
void BaseOutputRecord::SetFieldValueAsLong(ZdFieldValue& fv, const long lValue) {
   try {
      fv.SetType(ZD_LONG_FLD);
      fv.AsLong() = lValue;
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldValueAsLong()", "BaseOutputRecord");
      throw;
   }
}

// sets the filed value type to ZdString and value to sValue
// pre : none
// post : fv has type ZdString and value sValue    
void BaseOutputRecord::SetFieldValueAsString(ZdFieldValue& fv, const ZdString& sValue) {
   try {
      fv.SetType(ZD_ALPHA_FLD);
      fv.AsZdString() = sValue;
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldValueAsString()", "BaseOutputRecord");
      throw;
   }
}

//===============================================================================
// This class is the base storage class for each of the output file types.
// These classes are primarily concerned with the storage of the data needed
// to create the output files. These files, however, are not responsible for
// the printing of that information, they are for storage only.
//===============================================================================

BaseOutputStorageClass::BaseOutputStorageClass() {
}

BaseOutputStorageClass::~BaseOutputStorageClass() {
   try {
      gvRecords.DeleteAllElements();
      gvFields.DeleteAllElements();
   }
   catch (...) { }
}

// adds a record to the global vector of record pointers
// pre : none
// post : if pRecord null then exception, else adds pRecord to the global vector
void BaseOutputStorageClass::AddRecord(BaseOutputRecord* pRecord) {
   try {
      if(!pRecord)
         ZdGenerateException("Null pointer passed into function!", "AddRecord()");
         
      gvRecords.push_back(pRecord);   
   }
   catch (ZdException &x) {
      x.AddCallpath("AddRecord()", "BaseOutputStorageClass");
      throw;
   }
}

// returns a pointer to the record at iPosition in the global vector
// pre : none
// post : if iPosition not in valid index range then exception, else returns
//        a pointer to the iPosition element in the vector
const BaseOutputRecord* BaseOutputStorageClass::GetRecord(int iPosition) { 
   try {
      if (iPosition < 0 || iPosition >= gvRecords.size())
         ZdGenerateException ("Invalid index, out of range", "Error!");   
   }
   catch (ZdException &x) {
      x.AddCallpath("GetRecord()", "BaseOutputStorageClass");
      throw;
   } 
   return gvRecords.at(iPosition);
}

// ============================================================================
// this is a testing class to test the output file data heirarchy and also test the
// file printing heirarchy
// ============================================================================
TestOutputRecord::TestOutputRecord() {
   Init();
}

TestOutputRecord::~TestOutputRecord() {
}      

ZdFieldValue TestOutputRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {
      if (iFieldNumber < 0 || iFieldNumber >= GetNumFields())
         ZdGenerateException ("Invalid index, out of range", "Error!");
      	
      switch (iFieldNumber) {
      	 case 1:  SetFieldValueAsString(fv, gsStringTestValue); break;
      	 case 2:  SetFieldValueAsLong(fv, glLongTestValue); break;
      	 case 3:  SetFieldValueAsDouble(fv, gdDoubleTestValue); break;
      	 case 4:  SetFieldValueAsDouble(fv, gfFloatTestValue); break;
      	 case 5:  SetFieldValueAsLong(fv, giIntTestValue); break;
      	 case 6:  SetFieldValueAsString(fv, gbBoolTestValue ? "true" : "false" ); break;
      	 default :
      	    ZdGenerateException("Invalid index, out of range!", "Error!");
      }		
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()", "TestOutputRecord");
      throw;	
   }
   return fv;		
}

void TestOutputRecord::Init() {
   gsStringTestValue = "";
   glLongTestValue = 0;  
   gdDoubleTestValue = 0.0;
   gfFloatTestValue = 0.0; 
   giIntTestValue = 0;   
   gbBoolTestValue = false;  	
}		

// ============================================================================
// this is a testing class to test the output file data heirarchy and also test the
// file printing heirarchy
// ============================================================================

TestOutputClass::TestOutputClass():BaseOutputStorageClass() {
   SetUpFields();
}
	
TestOutputClass::~TestOutputClass(){
}

void TestOutputClass::SetUpFields() {
   unsigned short uwOffset = 0;

   try {
      ::CreateNewField(gvFields, "STRING", ZD_ALPHA_FLD, 255, 0, uwOffset);
      ::CreateNewField(gvFields, "LONG", ZD_LONG_FLD, 12, 0, uwOffset);
      ::CreateNewField(gvFields, "DOUBLE", ZD_NUMBER_FLD, 12, 6, uwOffset);
      ::CreateNewField(gvFields, "FLOAT", ZD_NUMBER_FLD, 12, 6, uwOffset);
      ::CreateNewField(gvFields, "INT", ZD_LONG_FLD, 12, 0, uwOffset);
      ::CreateNewField(gvFields, "BOOL", ZD_ALPHA_FLD, 16, 0, uwOffset);	
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "TestOutputClass");
      throw;	
   }		
}

void TestOutputClass::SetTestValues(const ZdString& sStringTestValue, const long lLongTestValue, const double dDoubleTestValue,
                                    const float	fFloatTestValue, const int iIntTestValue, const bool bBoolTestValue) {
   TestOutputRecord* pRecord = 0;
   
   try {
      pRecord = new TestOutputRecord();
      pRecord->SetStringTestField(sStringTestValue);
      pRecord->SetLongTestField(lLongTestValue);        
      pRecord->SetDoubleTestField(dDoubleTestValue);  
      pRecord->SetFloatTestField(fFloatTestValue);     
      pRecord->SetIntTestField(iIntTestValue); 
      pRecord->SetBoolTestField(bBoolTestValue);        
      BaseOutputStorageClass::AddRecord(pRecord);	
   }
   catch (ZdException &x) {
      delete pRecord;	
      x.AddCallpath("SetTestValues()", "TestOutputClass");
      throw;
   }   		
}                                    		