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

// constructor
__fastcall stsAreaSpecificDBF::stsAreaSpecificDBF(const ZdString& sReportHistoryFileName, const int& iCoordType, const ZdFileName& sOutputFileName)
                             : DBaseOutput(sReportHistoryFileName, iCoordType) {
   try {
      Init();
      Setup(sOutputFileName.GetLocation());
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

// sets up the global vecotr of ZdFields
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void stsAreaSpecificDBF::GetFields() {
   DBFFile		File;
   ZdField		Field;
   std::vector<field_t>         vFields;

   try {
      CleanupFieldVector();           // empty out the global field vector
      SetupFields(vFields);

      for(unsigned int i = 0; i < vFields.size(); ++i) {
         Field = *(File.GetNewField());
         Field.SetName(vFields[i].sFieldName.c_str());
         Field.SetType(vFields[i].cFieldType);
         Field.SetLength(vFields[i].wLength);
         Field.SetPrecision(vFields[i].wPrecision);
         gvFields.AddElement(Field.Clone());  ;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFields()", "DBaseOutput");
      throw;
   }
}

// global inits
void stsAreaSpecificDBF::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsAreaSpecificDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) {
   unsigned long        uwFieldNumber = 0;
   ZdFieldValue         fv;
   ZdTransaction*       pTransaction = 0;

   try {
      DBFFile File(gsFileName.GetCString());
      pTransaction = File.BeginTransaction();
      auto_ptr<ZdFileRecord> pRecord;
      pRecord.reset(File.GetNewRecord());

      // define record data
      // run number - from run history file AJV 9/4/2002
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = glRunNumber;
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
      fv.AsDouble() = pCluster->GetRelativeRisk(pData->GetMeasureAdjustment());
      pRecord->PutFieldValue(uwFieldNumber, fv);

      File.AppendRecord(*pTransaction, *pRecord);

      File.EndTransaction(pTransaction);  pTransaction = 0;
      File.Close();
   }
   catch (ZdException &x) {
      pTransaction = 0;
      x.AddCallpath("RecordClusterData()", "stsAreaSpecificDBF");
      throw;
   }
}

// internal setup
void stsAreaSpecificDBF::Setup(const ZdString& sOutputFileName) {
   try {
      gsFileName << ZdString::reset << sOutputFileName << AREA_SPECIFIC_DBF_FILE;
      GetFields();
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
void stsAreaSpecificDBF::SetupFields(std::vector<field_t>& vFields) {
      field_t   field;

      field.sFieldName = "RUN_NUM";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 8;
      field.wPrecision = 0;
      vFields.push_back(field);


      field.sFieldName = "CLUST_NUM";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 8;
      field.wPrecision = 0;
      vFields.push_back(field);

      field.sFieldName = "AREA_ID";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 8;
      field.wPrecision = 0;
      vFields.push_back(field);

      field.sFieldName = "P_VALUE";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 12;
      field.wPrecision = 3;
      vFields.push_back(field);

      field.sFieldName = "OBSERVED";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 12;
      field.wPrecision = 2;
      vFields.push_back(field);

      field.sFieldName = "EXPECTED";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 12;
      field.wPrecision = 2;
      vFields.push_back(field);

      field.sFieldName = "REL_RISK";
      field.cFieldType = ZD_NUMBER_FLD;
      field.wLength = 12;
      field.wPrecision = 6;
      vFields.push_back(field);
}
