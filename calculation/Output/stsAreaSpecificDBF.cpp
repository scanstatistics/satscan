// Class stsAreaSpecificDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the area specific dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "stsSaTScan.h"
#pragma hdrstop

#include "stsAreaSpecificDBF.h"
#include <DBFFile.h>

// constructor
__fastcall stsAreaSpecificDBF::stsAreaSpecificDBF(const ZdString& sFileName) {
   try {
      Init();
      Setup(sFileName);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsAreaSpecificDBF");
      throw;
   }
}

// destructor
stsAreaSpecificDBF::~stsAreaSpecificDBF() {
   try {
      CleanupFieldVector();
   }
   catch (...) {/* munch munch, yummy*/}
}

// deletes all of the field pointers in the vector and empties the vector
// pre: none
// post: field vector is empty and all of the pointers are deleted
void stsAreaSpecificDBF::CleanupFieldVector() {
   try {
      for(int i = gvFields.GetNumElements() - 1; i > 0; --i) {
         delete gvFields[0]; gvFields[0] = 0;
         gvFields.RemoveElement(0);
      }
   }
   catch(ZdException &x) {
      x.AddCallpath("CleanupFieldVector()", "stsAreaSpecificDBF");
      throw;
   }
}

// create the output file
// pre: sFileName is name of the dbf file needing to be created
// post: creates the dbf file with the appropraite fields
void stsAreaSpecificDBF::CreateDBFFile() {
   TXDFile		*pFile = 0;

   try {
      GetFields();

      // pack up and create
      pFile = new TXDFile();
      pFile->PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      pFile->Delete(gsFileName);
//      if(ZdIO::Exists(gsFileName))
//        ZdIO::Delete(gsFileName);
      pFile->Create(gsFileName, gvFields);
      pFile->Close();

      delete pFile;	
   }
   catch (ZdException &x) {
      if(pFile)
         pFile->Close();
      delete pFile; pFile = 0;
      x.AddCallpath("CreateDBFFile()", "DBaseOutput");
      throw;
   }
}

// sets up the global vecotr of ZdFields
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void stsAreaSpecificDBF::GetFields() {
   TXDFile*		pFile = 0;
   ZdField*		pField = 0;
   ZdVector<std::pair<std::pair<ZdString, char>, short> > vFieldDescrips;

   try {
      CleanupFieldVector();           // empty out the global field vector
      SetupFields(vFieldDescrips);

      pFile = new TXDFile();

      for(unsigned int i = 0; i < vFieldDescrips.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldDescrips[i].first.first.GetCString());
         pField->SetType(vFieldDescrips[i].first.second);
         pField->SetLength(vFieldDescrips[i].second);
         gvFields.AddElement(pField->Clone());
         delete pField;
      }

      delete pFile;
   }
   catch (ZdException &x) {
      delete pFile; pFile = 0;
      delete pField; pField = 0;
      x.AddCallpath("GetFields()", "DBaseOutput");
      throw;
   }
}

// global inits
void stsAreaSpecificDBF::Init() {
   glRunNumber = 0;
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsAreaSpecificDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) {
   TXDFile*		pFile = 0;
   ZdFileRecord*	pRecord = 0;
   ZdTransaction *	pTransaction = 0;
   unsigned long        uwFieldNumber = 0;
   ZdFieldValue         fv;

   try {
      pFile = new TXDFile(gsFileName.GetCString(), ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      pTransaction = pFile->BeginTransaction();
      pRecord = pFile->GetNewRecord();

      // define record data
      // run number - from run history file AJV 9/4/2002
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = ++glRunNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster number
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = iClusterNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // area id
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_Center;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // p value
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->gfPValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // observed
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nCases;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // expected
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nMeasure;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // relative risk
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nRatio;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      pFile->AppendRecord(*pTransaction, *pRecord);
      delete pRecord;

      pFile->EndTransaction(pTransaction);
      pFile->Close();
      delete pFile;
   }
   catch (ZdException &x) {
      if(pFile) {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pFile->Close();
      }
      delete pFile; pFile = 0;
      pTransaction = 0;
      delete pRecord; pRecord = 0;
      x.AddCallpath("RecordClusterData()", "stsAreaSpecificDBF");
      throw;
   }
}

// internal setup
void stsAreaSpecificDBF::Setup(const ZdString& sFileName) {
   TXDFile	        *pFile = 0;
   ZdFileRecord         *pLastRecord = 0;
   unsigned long        ulLastRecordNumber;

   try {
      if(ZdIO::Exists("AnalysisHistory.txd") && ZdIO::Exists("AnalysisHistory.zds"))  {
         pFile = new TXDFile("AnalysisHistory.txd", ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);

         // get a record buffer, input data and append the record
         ulLastRecordNumber = pFile->GotoLastRecord(pLastRecord);
         // if there's records in the file
         if(ulLastRecordNumber)
            pLastRecord->GetField(1, glRunNumber);
         delete pLastRecord;
         pFile->Close();

         delete pFile;
      }

      gsFileName = sFileName;

      CreateDBFFile();
   }
   catch(ZdException &x) {
      if(pFile)
         pFile->Close();
      delete pFile; pFile = 0;
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("Setup()", "stsAreaSpecificDBF");
      throw;
   }
}

// field names for the cluster level output dbf file
// pre: empty vector of triplets passed in
// post: passes back through reference a vector of strings will the field names for the dbf file
void stsAreaSpecificDBF::SetupFields(ZdVector<std::pair<std::pair<ZdString, char>, short> >& vFieldDescrips){
   std::pair<pair<ZdString, char>, long>        field;

   try {
      field.first.first = "RUN_NUM";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "CLUST_NUM";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "AREA_ID";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "P_VALUE";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "OBSERVED";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "EXPECTED";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);

      field.first.first = "REL_RISK";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrips.AddElement(field);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsAreaSpecificDBF");
      throw;
   }
}