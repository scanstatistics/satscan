// Class stsClusterLevelDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the cluster level dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "stsSaTScan.h"
#pragma hdrstop

#include <DBFFile.h>
#include "stsOutputDBF.h"
#include "stsClusterLevelDBF.h"

// constructor
stsClusterLevelDBF::stsClusterLevelDBF(const ZdString& sFileName) : DBaseOutput(sFileName) {
   try {
      Init();
      Setup();
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsClusterLevelDBF");
      throw;
   }
}

// destructor
stsClusterLevelDBF::~stsClusterLevelDBF() {
   try {
   }
   catch (...) {/* munch munch, yummy*/}
}

// create the output file
// pre: sFileName is name of the dbf file needing to be created
// post: creates the dbf file with the appropraite fields
void stsClusterLevelDBF::CreateDBFFile() {
   DBFFile		*pFile = 0;
   
   try {
      GetFields(gvFields);

      // pack up and create
      pFile = new DBFFile();
      pFile->PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      if(ZdIO::Exists(gsFileName))
        ZdIO::Delete(gsFileName);
      pFile->Create(gsFileName, gvFields, 1);
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

// sets up the ZdFields and puts them into the vector
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void stsClusterLevelDBF::GetFields(ZdVector<ZdField*>& vFields) {
   DBFFile*		pFile = 0;
   ZdVector<ZdString> 	vFieldNames;
   ZdVector<char> 	vFieldTypes;
   ZdField*		pField = 0;
   ZdVector<short>      vFieldLengths;

   try {
      pFile = new DBFFile();
      SetupFields(vFieldNames, vFieldTypes, vFieldLengths);

      for(unsigned int i = 0; i < vFieldNames.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldNames[i]);
         pField->SetType(vFieldTypes[i]);
         pField->SetLength(vFieldLengths[i]);
         vFields.AddElement(pField->Clone());
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
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) {
   DBFFile*		pFile = 0;
   ZdFileRecord*	pRecord = 0;
   ZdTransaction *	pTransaction = 0;
   unsigned short       uwFieldNumber = 0;
   float                fRadius = 0;
   double               *pCoords = 0, *pCoords2 = 0;

   try {
      pFile = new DBFFile(gsFileName.GetCString());

      pTransaction = pFile->BeginTransaction();

      pRecord = pFile->GetNewRecord();

      // define record data
      // run number field  - from the run history file  AJV 9/4/2002
      pRecord->PutNumber(uwFieldNumber, glRunNumber);

      // cluster start date
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nStartDate);

      // cluster end date
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nEndDate);

      // cluster number
      pRecord->PutNumber(++uwFieldNumber, iClusterNumber);

      // observed
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nCases);

      // expected
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nMeasure);

      // relative risk
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nRatio);

      // log likliehood
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nLogLikelihood);

      // p value
      // I'm very tempted to store this value as part of the cluster - AJV 9/4/2002
      pRecord->PutNumber(++uwFieldNumber, pCluster->gfPValue);

      // number of areas in the cluster
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_nTracts);

      // central area id
      pRecord->PutNumber(++uwFieldNumber, pCluster->m_Center);

      // coord north
      pRecord->PutAlpha(++uwFieldNumber, "");

      // coord west
      pRecord->PutAlpha(++uwFieldNumber, "");

      // additional coords
      pRecord->PutAlpha(++uwFieldNumber, "");

      // radius
      (pData->GetGInfo())->giGetCoords(pCluster->m_Center, &pCoords);
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(pCluster->m_iEllipseOffset, pCluster->m_Center, pCluster->m_nTracts), &pCoords2);
      fRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
      pRecord->PutNumber(++uwFieldNumber, fRadius);

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
      x.AddCallpath("RecordClusterData()", "stsClusterLevelDBF");
      throw;
   }
}

// internal setup
void stsClusterLevelDBF::Setup() {
   try {
      CreateDBFFile();
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsClusterLevelDBF");
      throw;
   }
}

// field names for the cluster level output dbf file
// pre: empty vector of strings passed in
// post: passes back through reference a vector of strings will the field names for the dbf file
void stsClusterLevelDBF::SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths) {
   try {
      vFieldNames.AddElement("RUN_NUM");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("START_DATE");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("END_DATE");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("CLUST_NUM");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("OBSERVED");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("EXPECTED");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("REL_RISK");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("LOG_LIKL");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("P_VALUE");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("NUM_AREAS");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("AREA_ID");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("COORD_NOR");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("COORD_WES");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("COORD_ADD");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("RADIUS");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupClusterLevelOutputFieldNames()", "DBaseOutput");
      throw;
   }
}