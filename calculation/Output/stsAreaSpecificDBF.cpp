// Class stsAreaSpecificDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the area specific dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "SaTScan.h"
#pragma hdrstop

#include "stsAreaSpecificDBF.h"
#include <DBFFile.h>
#include "Cluster.h"

// constructor
__fastcall stsAreaSpecificDBF::stsAreaSpecificDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName)
                             : DBaseOutput(lRunNumber, iCoordType) {
   try {
      Init();
      Setup(sOutputFileName.GetFullPath());
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsAreaSpecificDBF");
      throw;
   }
}

__fastcall stsAreaSpecificDBF::stsAreaSpecificDBF() : DBaseOutput(-1, -1) {
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
void stsAreaSpecificDBF::RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber) {
   ZdTransaction*       pTransaction = 0;
   ZdString             sTempValue;
   float                fPVal;
   DBFFile              File(gsFileName.GetCString());

   try {
      auto_ptr<ZdFileRecord> pRecord;
      pRecord.reset(File.GetNewRecord());

      // define record data
      // run number - from run history file AJV 9/4/2002
      SetDoubleField(*pRecord, double(glRunNumber), GetFieldNumber(gvFields, RUN_NUM));

      // cluster number
      SetDoubleField(*pRecord, iClusterNumber, GetFieldNumber(gvFields, CLUST_NUM));

      // area id
      sTempValue = (pData.GetTInfo())->tiGetTid(pData.GetNeighbor(pCluster.m_iEllipseOffset, pCluster.m_Center, iClusterNumber));
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, AREA_ID));

      // p value
      fPVal = (float) pCluster.GetPVal(pData.m_pParameters->m_nReplicas);
      SetDoubleField(*pRecord, fPVal, GetFieldNumber(gvFields, P_VALUE));

      // observed
      SetDoubleField(*pRecord, pCluster.m_nCases, GetFieldNumber(gvFields, OBSERVED));

      // expected
      SetDoubleField(*pRecord, pCluster.m_nMeasure, GetFieldNumber(gvFields, EXPECTED));

      // relative risk
      SetDoubleField(*pRecord, pCluster.GetRelativeRisk(pData.GetMeasureAdjustment()), GetFieldNumber(gvFields, REL_RISK));

      pTransaction = File.BeginTransaction();
      File.AppendRecord(*pTransaction, *pRecord);
      File.EndTransaction(pTransaction);  pTransaction = 0;
      File.Close();
   }
   catch (ZdException &x) {
      if(pTransaction)
         File.EndTransaction(pTransaction);
      pTransaction = 0;
      x.AddCallpath("RecordClusterData()", "stsAreaSpecificDBF");
      throw;
   }
}

// internal setup
void stsAreaSpecificDBF::Setup(const ZdString& sOutputFileName) {
   try {
      // area specific dbf has same filename as output file with area specific extension - AJV 9/30/2002
      ZdString sTempName(sOutputFileName);
      sTempName.Replace(ZdFileName(sOutputFileName).GetExtension(), AREA_SPECIFIC_EXT);
      gsFileName = sTempName;

      SetupFields(gvFields);
      CreateDBFFile();
   }
   catch(ZdException &x) {
      x.AddCallpath("Setup()", "stsAreaSpecificDBF");
      throw;
   }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: empty vector of field_t
// post : returns through reference a vector filled with field_t structs to be used
//        to create the ZdVector of ZdField* required to create the DBF file
void stsAreaSpecificDBF::SetupFields(ZdPointerVector<ZdField>& vFields) {
   unsigned short uwOffset = 0;
   
   try {
      CreateNewField(vFields, RUN_NUM, ZD_NUMBER_FLD, 8, 0, uwOffset);
      CreateNewField(vFields, CLUST_NUM, ZD_NUMBER_FLD, 5, 0, uwOffset);
      CreateNewField(vFields, AREA_ID, ZD_ALPHA_FLD, 30, 0, uwOffset);
      CreateNewField(vFields, P_VALUE, ZD_NUMBER_FLD, 12, 5, uwOffset);
      CreateNewField(vFields, OBSERVED, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, EXPECTED, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, REL_RISK, ZD_NUMBER_FLD, 12, 3, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsAreaSpecificDBF");
      throw;	
   }	
}
