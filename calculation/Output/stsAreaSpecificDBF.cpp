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
   TXDFile		File;

   try {
      GetFields();

      // pack up and create
      File.PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      File.Delete(gsFileName);
      File.Create(gsFileName, gvFields);
      File.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateDBFFile()", "DBaseOutput");
      throw;
   }
}

// sets up the global vecotr of ZdFields
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void stsAreaSpecificDBF::GetFields() {
   TXDFile		File;
   ZdField		Field;
   ZdVector<std::pair<ZdString, char> > vFieldDescrips;    // field name, field type
   ZdVector<std::pair<short, short> > vFieldSizes;         // field length, field precision

   try {
      CleanupFieldVector();           // empty out the global field vector
      SetupFields(vFieldDescrips, vFieldSizes);

      for(unsigned int i = 0; i < vFieldDescrips.GetNumElements(); ++i) {
         Field = *(File.GetNewField());
         Field.SetName(vFieldDescrips[i].first.GetCString());
         Field.SetType(vFieldDescrips[i].second);
         Field.SetLength(vFieldSizes[i].first);
         Field.SetPrecision(vFieldSizes[i].second);
         gvFields.AddElement(Field.Clone());
      }
   }
   catch (ZdException &x) {
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
      delete pRecord; pRecord = 0;

      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
      delete pFile;  pFile = 0;
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
   ZdFileRecord         *pLastRecord = 0;

   try {
      if(ZdIO::Exists("c:\\AnalysisHistory.txd") && ZdIO::Exists("c:\\AnalysisHistory.zds"))  {
         TXDFile File("c:\\AnalysisHistory.txd", ZDIO_OPEN_READ);

         // if there's records in the file
         if(File.GotoLastRecord(pLastRecord))
            pLastRecord->GetField(1, glRunNumber);
         delete pLastRecord; pLastRecord = 0;
         File.Close();
      }

      gsFileName = sFileName;

      CreateDBFFile();
   }
   catch(ZdException &x) {
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("Setup()", "stsAreaSpecificDBF");
      throw;
   }
}

// field names for the cluster level output dbf file
// pre: empty vector of triplets passed in
// post: passes back through reference a vector of strings will the field names for the dbf file
void stsAreaSpecificDBF::SetupFields(ZdVector<std::pair<ZdString, char> >& vFieldDescrips, ZdVector<std::pair<short, short> >& vFieldSizes){
   std::pair<ZdString, char>        field;
   std::pair<short, short>          fieldsize;

   try {
      field.first = "RUN_NUM";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 8;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "CLUST_NUM";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 8;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "AREA_ID";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 8;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "P_VALUE";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 6;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "OBSERVED";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 2;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "EXPECTED";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 2;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "REL_RISK";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 6;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsAreaSpecificDBF");
      throw;
   }
}