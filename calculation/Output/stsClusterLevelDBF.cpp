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

// global inits
void stsClusterLevelDBF::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData) {
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
     // pRecord->PutLong(uwFieldNumber, );

      // cluster start date
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nStartDate);

      // cluster end date
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nEndDate);

      // cluster number
      ++uwFieldNumber;

      // observed
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nCases);

      // expected
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nMeasure);

      // relative risk
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nRatio);

      // log likliehood
      pRecord->PutLong(++uwFieldNumber, pCluster->m_nLogLikelihood);

      // p value
      // I'm very tempted to store this value as part of the cluster - AJV 9/4/2002
      ++uwFieldNumber;

      // number of areas in the cluster
      ++uwFieldNumber;

      // central area id
      pRecord->PutLong(++uwFieldNumber, pCluster->m_Center);

      // coord north
      ++uwFieldNumber;

      // coord west
      ++uwFieldNumber;

      // additional coords
      ++uwFieldNumber;

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
      vFieldNames.AddElement("Run_Num");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Start_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("End_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("Clust_Num");
      vFieldTypes.AddElement(ZD_LONG_FLD);
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
      vFieldNames.AddElement("Log_Likli");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("p-Value");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Num_Areas");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Cent_Area_ID");
      vFieldTypes.AddElement(ZD_LONG_FLD);
      vFieldLengths.AddElement(12);
      vFieldNames.AddElement("Coord_Nor");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("Coord_Wes");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("Adnl_Coords");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFieldLengths.AddElement(32);
      vFieldNames.AddElement("Radius");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFieldLengths.AddElement(12);
   }   
   catch (ZdException &x) {
      x.AddCallpath("SetupClusterLevelOutputFieldNames()", "DBaseOutput");
      throw;
   }
}