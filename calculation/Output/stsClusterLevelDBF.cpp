// Class stsClusterLevelDBF
// Adam J Vaughn
// 9/4/2002

// This class is responsible for the construction of the cluster level dBase output file.
// This class defines the fields which make up the dBase file and also the data contained therein.
// The class is derived from the base class DBaseOutput which takes care of setting up
// and writing out the dBase file. 

#include "SaTScan.h"
#pragma hdrstop

#include "stsClusterLevelDBF.h"
#include <DBFFile.h>


// constructor
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const long& lRunNumber, const int& iCoordType, const ZdFileName& sOutputFileName)
                             : DBaseOutput(lRunNumber, iCoordType) {
   try {
      Init();
      Setup(sOutputFileName.GetLocation());
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

// sets up the global vecotr of ZdFields
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void stsClusterLevelDBF::GetFields() {
   DBFFile		File;
   ZdField		*pField = 0;
   std::vector<field_t>         vFields;

   try {
      CleanupFieldVector();
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
void stsClusterLevelDBF::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) {
   unsigned short       uwFieldNumber = 0;
   float                fRadius = 0, fLatitude = 0, fLongitude = 0;
   double               *pCoords = 0, *pCoords2 = 0;
   ZdFieldValue         fv;
   ZdString             sAdditCoords;
   ZdTransaction*       pTransaction = 0;

   try {
      DBFFile File(gsFileName.GetCString());
      pTransaction= (File.BeginTransaction());

      auto_ptr<ZdFileRecord> pRecord;
      pRecord.reset(File.GetNewRecord());

      // define record data
      // run number field  - from the run history file  AJV 9/4/2002
      SetDoubleField(*pRecord, double(glRunNumber), uwFieldNumber);

      // cluster start date
      SetDoubleField(*pRecord, pCluster->m_nStartDate, (++uwFieldNumber));

      // cluster end date
      SetDoubleField(*pRecord, pCluster->m_nEndDate, (++uwFieldNumber));

      // cluster number
      SetDoubleField(*pRecord, iClusterNumber, (++uwFieldNumber));

      // observed
      SetDoubleField(*pRecord, pCluster->m_nCases, (++uwFieldNumber));

      // expected
      SetDoubleField(*pRecord, pCluster->m_nMeasure, (++uwFieldNumber));

      // relative risk
      SetDoubleField(*pRecord, pCluster->GetRelativeRisk(pData->GetMeasureAdjustment()), (++uwFieldNumber));

      // log likliehood
      SetDoubleField(*pRecord, pCluster->m_nLogLikelihood, (++uwFieldNumber));

      // p value
      SetDoubleField(*pRecord, pCluster->gfPValue, (++uwFieldNumber));

      // number of areas in the cluster
      SetDoubleField(*pRecord, pCluster->m_nTracts, (++uwFieldNumber));

      // central area id
      SetDoubleField(*pRecord, pCluster->m_Center, (++uwFieldNumber));

      (pData->GetGInfo())->giGetCoords(pCluster->m_Center, &pCoords);
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(pCluster->m_iEllipseOffset, pCluster->m_Center, pCluster->m_nTracts), &pCoords2);
      fRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
      if(giCoordType == CARTESIAN) {
         for (int i = 0; i < (pData->m_pParameters->m_nDimension); ++i) {
            if (i == 0)
               fLatitude = pCoords[i];
            else if (i == 1)
               fLongitude = pCoords[i];
            else
               sAdditCoords << pCoords[i] << ", ";
         }
         if(sAdditCoords.GetLength() > 0)
            sAdditCoords = sAdditCoords.Remove(sAdditCoords.GetLength()-2, 1);    // strip off the last comma AJV 9/9/2002
      }
      else
         ConvertToLatLong(&fLatitude, &fLongitude, pCoords);

      // coord north
      SetDoubleField(*pRecord, fLatitude, (++uwFieldNumber));

      // coord west
      SetDoubleField(*pRecord, fLongitude, (++uwFieldNumber));

      // additional coords
      SetStringField(*pRecord, sAdditCoords, (++uwFieldNumber));

      // radius
      SetDoubleField(*pRecord, fRadius, (++uwFieldNumber));

      File.AppendRecord(*pTransaction, *pRecord);
      File.EndTransaction(pTransaction); pTransaction = 0;
      File.Close();

      free(pCoords);
      free(pCoords2);
   }
   catch (ZdException &x) {
      free(pCoords);
      free(pCoords2);
      pTransaction = 0;
      x.AddCallpath("RecordClusterData()", "stsClusterLevelDBF");
      throw;
   }
}

// internal setup
void stsClusterLevelDBF::Setup(const ZdString& sOutputFileName) {
   try {
      gsFileName << ZdString::reset << sOutputFileName << CLUSTER_LEVEL_DBF_FILE;
      GetFields();
      CreateDBFFile();
   }
   catch(ZdException &x) {
      x.AddCallpath("Setup()", "stsClusterLevelDBF");
      throw;
   }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: empty vector of field_t
// post : returns through reference a vector filled with field_t structs to be used
//        to create the ZdVector of ZdField* required to create the DBF file
void stsClusterLevelDBF::SetupFields(std::vector<field_t>& vFields) {
   vFields.push_back(field_t("RUN_NUM", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("START_DATE", ZD_NUMBER_FLD, 16, 0));
   vFields.push_back(field_t("END_DATE", ZD_NUMBER_FLD, 16, 0));
   vFields.push_back(field_t("CLUST_NUM", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("OBSERVED", ZD_NUMBER_FLD, 12, 2));
   vFields.push_back(field_t("EXPECTED", ZD_NUMBER_FLD, 12, 2));
   vFields.push_back(field_t("REL_RISK", ZD_NUMBER_FLD, 12, 4));
   vFields.push_back(field_t("LOG_LIKL", ZD_NUMBER_FLD, 12, 4));
   vFields.push_back(field_t("P_VALUE", ZD_NUMBER_FLD, 12, 3));
   vFields.push_back(field_t("NUM_AREAS", ZD_NUMBER_FLD, 12, 0));
   vFields.push_back(field_t("AREA_ID", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("COORD_NOR", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("COORD_WES", ZD_NUMBER_FLD, 8, 0));
   vFields.push_back(field_t("COORD_ADD", ZD_ALPHA_FLD, 48, 0));
   vFields.push_back(field_t("RADIUS", ZD_NUMBER_FLD, 12, 4));
}
