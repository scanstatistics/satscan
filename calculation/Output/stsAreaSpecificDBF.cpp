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
__fastcall stsAreaSpecificDBF::stsAreaSpecificDBF(const long& lRunNumber, const int& iCoordType, const ZdFileName& sOutputFileName)
                             : DBaseOutput(lRunNumber, iCoordType) {
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
   ZdField		*pField = 0;
   std::vector<field_t>         vFields;

   try {
      CleanupFieldVector();           // empty out the global field vector
      SetupFields(vFields);

      for(unsigned int i = 0; i < vFields.size(); ++i) {
         pField = (File.GetNewField());
         pField->SetName(vFields[i].gsFieldName.c_str());
         pField->SetType(vFields[i].gcFieldType);
         pField->SetLength(vFields[i].gwLength);
         pField->SetPrecision(vFields[i].gwPrecision);
         gvFields.AddElement(pField);  ;
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
      SetDoubleField(*pRecord, double(glRunNumber), (uwFieldNumber));

      // cluster number
      SetDoubleField(*pRecord, iClusterNumber, (++uwFieldNumber));

      // area id
      SetDoubleField(*pRecord, pCluster->m_Center, (++uwFieldNumber));

      // p value
      SetDoubleField(*pRecord, pCluster->gfPValue, (++uwFieldNumber));

      // observed
      SetDoubleField(*pRecord, pCluster->m_nCases, (++uwFieldNumber));

      // expected
      SetDoubleField(*pRecord, pCluster->m_nMeasure, (++uwFieldNumber));

      // relative risk
      SetDoubleField(*pRecord, pCluster->GetRelativeRisk(pData->GetMeasureAdjustment()), (++uwFieldNumber));

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
   vFields.push_back(field_t("RUN_NUM", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("CLUST_NUM", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("AREA_ID", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("P_VALUE", ZD_NUMBER_FLD, 12, 3));
   vFields.push_back(field_t("OBSERVED", ZD_NUMBER_FLD, 12, 2));
   vFields.push_back(field_t("EXPECTED", ZD_NUMBER_FLD, 12, 2));
   vFields.push_back(field_t("REL_RISK", ZD_NUMBER_FLD, 12, 4));
}
