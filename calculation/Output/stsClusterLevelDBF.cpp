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
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const ZdString& sReportHistoryFileName, const int& iCoordType, const ZdFileName& sOutputFileName)
                             : DBaseOutput(sReportHistoryFileName, iCoordType) {
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
   ZdField		Field;
   ZdVector<std::pair<ZdString, char> > vFieldDescrips;     // field name, field type
   ZdVector<std::pair<short, short> > vFieldSizes;          // field length, field precision

   try {
      CleanupFieldVector();
      SetupFields(vFieldDescrips, vFieldSizes);

      for(unsigned int i = 0; i < vFieldDescrips.GetNumElements(); ++i) {
         Field = * ( File.GetNewField() );
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
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = glRunNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster start date
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nStartDate;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // cluster end date
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = pCluster->m_nEndDate;
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
      fv.AsDouble() = pCluster->GetRelativeRisk(pData->GetMeasureAdjustment());
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
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = fLatitude;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // coord west
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = fLongitude;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // additional coords
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = sAdditCoords;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // radius
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = fRadius;
      pRecord->PutFieldValue(uwFieldNumber, fv);

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
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 16;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "END_DATE";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 16;
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
      fieldsize.second = 3;
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
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 16;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "COORD_WES";
      field.second = ZD_NUMBER_FLD;
      fieldsize.first = 16;
      fieldsize.second = 0;
      vFieldDescrips.AddElement(field);
      vFieldSizes.AddElement(fieldsize);

      field.first = "COORD_ADD";
      field.second = ZD_ALPHA_FLD;
      fieldsize.first = 48;
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


