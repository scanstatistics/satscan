// Class stsClusterLevelDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the cluster level dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "stsSaTScan.h"
#pragma hdrstop

#include "stsClusterLevelDBF.h"
#include <DBFFile.h>


// constructor
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const ZdString& sFileName) {
   try {
      Init();
      Setup(sFileName);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsClusterLevelDBF");
      throw;
   }
}

// destructor
stsClusterLevelDBF::~stsClusterLevelDBF() {
   try {
      CleanupFieldVector();
   }
   catch (...) {/* munch munch, yummy*/}
}

// deletes all of the field pointers in the vector and empties the vector
// pre: none
// post: field vector is empty and all of the pointers are deleted
void stsClusterLevelDBF::CleanupFieldVector() {
   try {
      for(int i = gvFields.GetNumElements() - 1; i > 0; --i) {
         delete gvFields[0]; gvFields[0] = 0;
         gvFields.RemoveElement(0);
      }
   }
   catch(ZdException &x) {
      x.AddCallpath("CleanupFieldVector()", "stsClusterLevelDBF");
      throw;
   }
}

// create the output file
// pre: sFileName is name of the dbf file needing to be created
// post: creates the dbf file with the appropraite fields
void stsClusterLevelDBF::CreateDBFFile() {
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
void stsClusterLevelDBF::GetFields() {
   TXDFile*		pFile = 0;
   ZdField*		pField = 0;
   ZdVector<std::pair<ZdString, char> > vFieldDescrips;
   ZdVector<std::pair<short, short> > vFieldSizes;

   try {
      CleanupFieldVector();
      SetupFields(vFieldDescrips, vFieldSizes);

      pFile = new TXDFile();
      
      for(unsigned int i = 0; i < vFieldDescrips.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldDescrips[i].first.GetCString());
         pField->SetType(vFieldDescrips[i].second);
         pField->SetLength(vFieldSizes[i].first);
         pField->SetPrecision(vFieldSizes[i].second);
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
void stsClusterLevelDBF::Init() {
   glRunNumber = 0;
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) {
   TXDFile*		pFile = 0;
   ZdFileRecord*	pRecord = 0;
   ZdTransaction *	pTransaction = 0;
   unsigned short       uwFieldNumber = 0;
   float                fRadius = 0;
   double               *pCoords = 0, *pCoords2 = 0;
   ZdFieldValue         fv;

   try {
      pFile = new TXDFile(gsFileName.GetCString(), ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);

      pTransaction = pFile->BeginTransaction();

      pRecord = pFile->GetNewRecord();
    
      // define record data
      // run number field  - from the run history file  AJV 9/4/2002
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = ++glRunNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster start date
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = (char*)pCluster->m_nStartDate;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster end date
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = (char*)pCluster->m_nEndDate;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster number
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = iClusterNumber;
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

      // log likliehood
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nLogLikelihood;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // p value
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->gfPValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // number of areas in the cluster
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nTracts;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // central area id
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_Center;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // coord north
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = "";
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // coord west
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = "";
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // additional coords
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = "";
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // radius
      (pData->GetGInfo())->giGetCoords(pCluster->m_Center, &pCoords);
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(pCluster->m_iEllipseOffset, pCluster->m_Center, pCluster->m_nTracts), &pCoords2);
      fRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = fRadius;
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
      pTransaction = 0;    // can't delete transactions, this is done by pFile->EndTransaction which smudges the pointer
      delete pRecord; pRecord = 0;
      delete pFile; pFile = 0;

      x.AddCallpath("RecordClusterData()", "stsClusterLevelDBF");
      throw;
   }
}

// internal setup
void stsClusterLevelDBF::Setup(const ZdString& sFileName) {
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
      x.AddCallpath("Setup()", "stsClusterLevelDBF");
      throw;
   }
}

// field names for the cluster level output dbf file
// pre: two empty pair vectors
// post: passes back through reference a vectors of field names, field type, field length, field precision for the dbf file
void stsClusterLevelDBF::SetupFields(ZdVector<std::pair<ZdString, char> >& vFieldDescrips, ZdVector<std::pair<short, short> >& vFieldSizes) {
   std::pair<ZdString, char>        field;
   std::pair<short, short>          fieldsize;

   try {
      field.first = "RUN_NUM";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 8;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "START_DATE";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 32;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "END_DATE";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 32;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "CLUST_NUM";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 8;
      fieldsize.second = 0;
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
      fieldsize.second = 4;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "LOG_LIKL";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 4;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "P_VALUE";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 6;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "NUM_AREAS";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "AREA_ID";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "COORD_NOR";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 16;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "COORD_WES";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 16;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "COORD_ADD";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 32;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "RADIUS";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 12;
      fieldsize.second = 4;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupClusterLevelOutputFieldNames()", "DBaseOutput");
      throw;
   }
}