// Class stsAreaSpecificDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the area specific dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "stsSaTScan.h"
#pragma hdrstop

#include <DBFFile.h>
#include "stsOutputDBF.h"
#include "stsAreaSpecificDBF.h"

// constructor
stsAreaSpecificDBF::stsAreaSpecificDBF(const ZdString& sFileName) : DBaseOutput(sFileName) {
   try {
      Init();
      Setup();
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsAreaSpecificDBF");
      throw;
   }
}

// destructor
stsAreaSpecificDBF::~stsAreaSpecificDBF() {
   try {
   }
   catch (...) {/* munch munch, yummy*/}
}

// global inits
void stsAreaSpecificDBF::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsAreaSpecificDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData) {
   DBFFile*		pFile = 0;
   ZdFileRecord*	pRecord = 0;
   ZdTransaction *	pTransaction = 0;
   unsigned long        uwFieldNumber = 0;

   try {
      pFile = new DBFFile(gsFileName.GetCString());
      pTransaction = pFile->BeginTransaction();
      pRecord = pFile->GetNewRecord();

      // define record data
      // run number - from run history file AJV 9/4/2002
      // pRecord->PutLong(uwFieldNumber, );

      // cluster number
      ++uwFieldNumber;

      // area id
      pRecord->PutLong(++uwFieldNumber, pCluster->m_Center);

      // p value
      // I'm very tempted to store this value as part of the cluster - AJV 9/4/2002
      ++uwFieldNumber;

      // observed
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nCases);

      // expected
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nMeasure);

      // relative risk
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nRatio);

      pFile->AppendRecord(*pTransaction, *pRecord);
      delete pRecord;

      pFile->EndTransaction(pTransaction);
      pFile->Close();
      delete pTransaction;
      delete pFile;
   }
   catch (ZdException &x) {
      if(pFile) {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pFile->Close();
      }
      delete pFile; pFile = 0;
      delete pTransaction; pTransaction = 0;
      delete pRecord; pRecord = 0;
      x.AddCallpath("RecordClusterData()", "stsAreaSpecificDBF");
      throw;
   }
}

// internal setup
void stsAreaSpecificDBF::Setup() {
   try {
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsAreaSpecificDBF");
      throw;
   }
}

// field names for the cluster level output dbf file
// pre: empty vector of strings passed in
// post: passes back through reference a vector of strings will the field names for the dbf file
void stsAreaSpecificDBF::SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths) {
   try {
      vFieldNames.AddElement("Run_Num");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Clust_Num");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Area_ID");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("p-Value");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Observed");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Expected");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Rel_Risk");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsAreaSpecificDBF");
      throw;
   }
}