// Class stsAreaSpecificData
// Adam J Vaughn
// November 2002
 
#include "SaTScan.h"
#pragma hdrstop

#include "stsAreaSpecificData.h"
#include "Cluster.h"

const char *	AREA_SPECIFIC_FILE_EXT		= ".gis";

// record storage class for area specific data
// constrcutor
AreaSpecificRecord::AreaSpecificRecord(const bool bPrintPVal, const bool bIncludeRunHistory) : BaseOutputRecord() {
   gbPrintPVal = bPrintPVal;
   gbIncludeRunHistory = bIncludeRunHistory;
   Init();
}

// destructor
AreaSpecificRecord::~AreaSpecificRecord() {
}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool AreaSpecificRecord::GetFieldIsBlank(int iFieldNumber) {
   try {
      if ( iFieldNumber < 0 || (size_t)iFieldNumber >= gvbBlankFields.size())
         ZdGenerateException("Invalid index, out of range!", "Error!");
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFieldIsBlank()", "AreaSpecificRecord");
      throw;
   }
   return gvbBlankFields[iFieldNumber];
}

int AreaSpecificRecord::GetNumFields() {
  return ( 8 + (gbPrintPVal ? 1 : 0) + (gbIncludeRunHistory ? 1 : 0));
}
// returns the field value of the requested field
// pre : iFieldNumber is in valid range, else an exception is thrown
// post : returns a ZdFieldValue with the appropriate type and value of the field requested
ZdFieldValue AreaSpecificRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {   
      if (iFieldNumber < 0 || iFieldNumber >= GetNumFields())
         ZdGenerateException ("Index out of range!", "Error!");

      // if we should include run history then do so, if not just skip to the next field
      if(gbIncludeRunHistory) {
         if ( iFieldNumber == 0 )
            BaseOutputRecord::SetFieldValueAsDouble(fv, double(glRunNumber));
      }
      else
         ++iFieldNumber;

      switch (iFieldNumber) {
         case 1 :
            BaseOutputRecord::SetFieldValueAsString(fv, gsLocationID);   break;
         case 2 :
            BaseOutputRecord::SetFieldValueAsDouble(fv, double(giClusterNumber));  break;
         case 3 :
            BaseOutputRecord::SetFieldValueAsDouble(fv, double(glClusterObserved));   break;
         case 4 :
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdClusterExpected);   break;
         case 5 :
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdRelRisk);  break;
      }

      // if p-Value should be included then do so else skip ahead to the next field
      if (gbPrintPVal) {
         if (iFieldNumber == 6)
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdPValue);         
      }
      else
         ++iFieldNumber;

      if (iFieldNumber > 6) {
         switch(iFieldNumber) {
            case 7 :
               BaseOutputRecord::SetFieldValueAsDouble(fv, double(glAreaObserved));  break;
            case 8 :
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdAreaExpected);  break;
            case 9 :
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdAreaRelRisk);   break;
            default :
               ZdGenerateException ("Invalid index, out of range", "Error!");
         }
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()", "AreaSpecificRecord");
      throw;
   }
   return fv;
}

// internal global initialization
void AreaSpecificRecord::Init() {
   glRunNumber = 0;
   gsLocationID = "";
   giClusterNumber = 0;
   glClusterObserved = 0;
   gdClusterExpected = 0.0;
   gdRelRisk = 0.0;
   gdPValue = 0.0;
   glAreaObserved = 0;
   gdAreaExpected = 0.0;
   gdAreaRelRisk = 0.0;

   for ( int i = 0; i < GetNumFields(); ++i ) 
      gvbBlankFields.push_back(false);
}

// sets the field at fieldnumber to either be blank or non-blank
// pre : none
// post : sets the iFieldNumber element of the global vector to bBlank
void AreaSpecificRecord::SetFieldIsBlank(int iFieldNumber, bool bBlank) {
   try {
      if (iFieldNumber < 0 || (size_t)iFieldNumber >= gvbBlankFields.size())
         ZdGenerateException("Invalid index, out of range!", "Error!");

      gvbBlankFields[iFieldNumber] = bBlank;
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldIsBlank()", "AreaSpecificRecord");
      throw;
   }
}

// ============================================================================
// This class is responsible for the storage of the area specific data for output.
// This class defines the fields which make up the the data contained therein.
// The class is derived from the base class BaseOutputStorageClass. 
// ============================================================================

// constructor
__fastcall stsAreaSpecificData::stsAreaSpecificData(BasePrint *pPrintDirection, const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal)
                             : BaseOutputStorageClass (pPrintDirection) {
   try {
      Init();
      Setup(sOutputFileName, lRunNumber, bPrintPVal);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsAreaSpecificData");
      throw;
   }
}

// destructor
stsAreaSpecificData::~stsAreaSpecificData() {
   try {
   }
   catch (...) {/* munch munch, yummy*/}
}

// global inits
void stsAreaSpecificData::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsAreaSpecificData::RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber, tract_t tTract) {
   ZdString             sTempValue;
   std::string          sBuffer;
   AreaSpecificRecord*	pRecord = 0;
   std::vector<std::string>             vIdentifiers;

   try {
      pData.gpTInfo->tiGetTractIdentifiers(tTract, vIdentifiers);

      // if more than one identifier for the tract then create a blank record for each
      if (vIdentifiers.size() > 1) {
         for (size_t j = 0; j < vIdentifiers.size(); ++j) {
            pRecord = new AreaSpecificRecord(gbPrintPVal, gbIncludeRunHistory);

            pRecord->SetLocationID(vIdentifiers[j].c_str());
            for(int i = 1; i < GetNumFields(); ++i) 
               pRecord->SetFieldIsBlank(i, true);

            BaseOutputStorageClass::AddRecord(pRecord);
         }   // end for each identifier
      }
      else {      // else not duplicate coordinates so we just store the data
         pRecord = new AreaSpecificRecord(gbPrintPVal, gbIncludeRunHistory);

         pRecord->SetAreaExpected(pCluster.GetMeasureForTract(tTract, pData));
         pRecord->SetAreaObserved(pCluster.GetCaseCountForTract(tTract, pData));
         pRecord->SetAreaRelativeRisk(pCluster.GetRelativeRiskForTract(tTract, pData));
         pRecord->SetClusterExpected(pCluster.m_nMeasure);
         pRecord->SetClusterNumber(iClusterNumber);
         pRecord->SetClusterObserved(pCluster.m_nCases);
         pRecord->SetClusterRelativeRisk(pCluster.GetRelativeRisk(pData.GetMeasureAdjustment()));

         // p value
         if(gbPrintPVal) {
            float fPVal = (float) pCluster.GetPVal(pData.m_pParameters->m_nReplicas);
            pRecord->SetPValue(fPVal);
         }
      
         if (gbIncludeRunHistory)
            pRecord->SetRunNumber(glRunNumber);
      
         // area id
         sTempValue = (pData.GetTInfo())->tiGetTid(tTract, sBuffer);
         pRecord->SetLocationID(sTempValue);

         BaseOutputStorageClass::AddRecord(pRecord);
      }
   }
   catch (ZdException &x) {
      delete pRecord; 	 
      x.AddCallpath("RecordClusterData()", "stsAreaSpecificData");
      throw;
   }  
}     
      
// internal setup
void stsAreaSpecificData::Setup(const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal) {
   try {
      ZdString sTempName(sOutputFileName);
      ZdString sExt(ZdFileName(sOutputFileName).GetExtension());
      if(sExt.GetLength()) 
         sTempName.Replace(sExt, AREA_SPECIFIC_FILE_EXT);
      else
         sTempName << AREA_SPECIFIC_FILE_EXT;
      gsFileName = sTempName;
      
      glRunNumber = lRunNumber;
      gbPrintPVal = bPrintPVal;
#ifdef INCLUDE_RUN_HISTORY
      gbIncludeRunHistory = true;
#else
      gbIncludeRunHistory = false;
#endif
      SetupFields();
   }
   catch(ZdException &x) {
      x.AddCallpath("Setup()", "stsAreaSpecificData");
      throw;
   }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: 
// post : returns through reference a vector of ZdFields to be used
//        to create the ZdVector of ZdField* required to create the DBF file
void stsAreaSpecificData::SetupFields() {
   unsigned short uwOffset = 0;     // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
   
   try {
      if(gbIncludeRunHistory)
         ::CreateField(gvFields, RUN_NUM_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);
      ::CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
      ::CreateField(gvFields, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
      ::CreateField(gvFields, CLU_OBS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      ::CreateField(gvFields, CLU_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      ::CreateField(gvFields, REL_RISK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);

      if(gbPrintPVal)
         ::CreateField(gvFields, P_VALUE_FLD, ZD_NUMBER_FLD, 12, 5, uwOffset);
      ::CreateField(gvFields, AREA_OBS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      ::CreateField(gvFields, AREA_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      ::CreateField(gvFields, AREA_RSK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);      
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsAreaSpecificData");
      throw;	
   }	
}
