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
#include "Cluster.h"

// constructor
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName)
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

// global inits
void stsClusterLevelDBF::Init() {
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber) {
   float                        fRadius = 0.0, fLatitude = 0.0, fLongitude = 0.0;
   ZdFieldValue                 fv;
   ZdString                     sAdditCoords, sTempValue;
   ZdTransaction*               pTransaction = 0;
   char                         sStartDate[15], sEndDate[15];
   float                        fPVal;
   DBFFile                      File(gsFileName.GetCString());

   try {
      std::auto_ptr<ZdFileRecord>  pRecord;
      pRecord.reset(File.GetNewRecord());

      // define record data
      // run number field  - from the run history file  AJV 9/4/2002
      SetDoubleField(*pRecord, double(glRunNumber), GetFieldNumber(gvFields, RUN_NUM));

      // cluster start date
      JulianToChar(sStartDate, pCluster.m_nStartDate);
      SetStringField(*pRecord, sStartDate, GetFieldNumber(gvFields, START_DATE));

      // cluster end date
      JulianToChar(sEndDate, pCluster.m_nEndDate);
      SetStringField(*pRecord, sEndDate, GetFieldNumber(gvFields, END_DATE));

      // cluster number
      SetDoubleField(*pRecord, iClusterNumber, GetFieldNumber(gvFields, CLUST_NUM));
     
      // observed
      SetDoubleField(*pRecord, pCluster.m_nCases, GetFieldNumber(gvFields, OBSERVED));
	
      // expected
      SetDoubleField(*pRecord, pCluster.m_nMeasure, GetFieldNumber(gvFields, EXPECTED));
	
      // relative risk
      SetDoubleField(*pRecord, pCluster.GetRelativeRisk(pData.GetMeasureAdjustment()), GetFieldNumber(gvFields, REL_RISK));
	        
      // log likliehood
      SetDoubleField(*pRecord, pCluster.m_nLogLikelihood, GetFieldNumber(gvFields, LOG_LIKL));
	
      // p value
      fPVal = (float) pCluster.GetPVal(pData.m_pParameters->m_nReplicas);
      SetDoubleField(*pRecord, fPVal, GetFieldNumber(gvFields, P_VALUE));

      // number of areas in the cluster
      SetDoubleField(*pRecord, pCluster.m_nTracts, GetFieldNumber(gvFields, NUM_AREAS));

      // central area id
      SetAreaID(sTempValue, pCluster, pData);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, AREA_ID));

      SetCoordinates(fLatitude, fLongitude, fRadius, sAdditCoords, pCluster, pData);

      // coord north
      SetDoubleField(*pRecord, fLatitude, GetFieldNumber(gvFields, COORD_NOR));

      // coord west
      SetDoubleField(*pRecord, fLongitude, GetFieldNumber(gvFields, COORD_WES));

      // additional coords
      SetStringField(*pRecord, sAdditCoords, GetFieldNumber(gvFields, COORD_ADD));

      // radius
      SetDoubleField(*pRecord, fRadius, GetFieldNumber(gvFields, RADIUS));

      pTransaction= File.BeginTransaction();
      File.AppendRecord(*pTransaction, *pRecord);
      File.EndTransaction(pTransaction); pTransaction = 0;
      File.Close();
   }
   catch (ZdException &x) {
      if(pTransaction)
         File.EndTransaction(pTransaction);
      pTransaction = 0;
      x.AddCallpath("RecordClusterData()", "stsClusterLevelDBF");
      throw;
   }
}

// this a a very ugly function, I will be the first to admit, but it is borrowed code from the
// way SatScan uses to calculate and print these Coordinate values, so don't blame me that it
// stinks cause someone else wrote it, I just plagurized it - AJV 9/25/2002
// pre : none that I can think of
// post : sets the values for long, lat, sAdditCoords, and radius
void stsClusterLevelDBF::SetCoordinates(float& fLatitude, float& fLongitude, float& fRadius, ZdString& sAdditCoords,
                                        const CCluster& pCluster, const CSaTScanData& pData) {
   double *pCoords = 0, *pCoords2 = 0;

   try {
      (pData.GetGInfo())->giGetCoords(pCluster.m_Center, &pCoords);
      (pData.GetTInfo())->tiGetCoords(pData.GetNeighbor(pCluster.m_iEllipseOffset, pCluster.m_Center, pCluster.m_nTracts), &pCoords2);
      fRadius = (float)sqrt((pData.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
      if(giCoordType == CARTESIAN) {
         for (int i = 0; i < (pData.m_pParameters->m_nDimension); ++i) {
            if (i == 0)
               fLatitude = pCoords[i];
            else if (i == 1)
               fLongitude = pCoords[i];
            else {
               if (i > 2)
                  sAdditCoords << ", ";
               else
                  sAdditCoords << "";
               sAdditCoords << pCoords[i];
            }   
         }
      }
      else
         ConvertToLatLong(&fLatitude, &fLongitude, pCoords);
      free(pCoords);
      free(pCoords2);
   }
   catch (ZdException &x) {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("SetCoordinates()", "stsClusterLevelDBF");
      throw;
   }
}

// internal setup
void stsClusterLevelDBF::Setup(const ZdString& sOutputFileName) {
   try {
      gsFileName << ZdString::reset << sOutputFileName << CLUSTER_LEVEL_DBF_FILE;
      SetupFields(gvFields);
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
void stsClusterLevelDBF::SetupFields(ZdPointerVector<ZdField>& vFields) {
   unsigned short uwOffset = 0;

   try {
      CreateNewField(vFields, RUN_NUM, ZD_NUMBER_FLD, 8, 0, uwOffset);
      CreateNewField(vFields, START_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateNewField(vFields, END_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateNewField(vFields, CLUST_NUM, ZD_NUMBER_FLD, 5, 0, uwOffset);
      CreateNewField(vFields, OBSERVED, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, EXPECTED, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, REL_RISK, ZD_NUMBER_FLD, 12, 3, uwOffset);
      CreateNewField(vFields, LOG_LIKL, ZD_NUMBER_FLD, 16, 6, uwOffset);
      CreateNewField(vFields, P_VALUE, ZD_NUMBER_FLD, 12, 5, uwOffset);
      CreateNewField(vFields, NUM_AREAS, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, AREA_ID, ZD_ALPHA_FLD, 30, 0, uwOffset);
      CreateNewField(vFields, COORD_NOR, ZD_NUMBER_FLD, 12, 6, uwOffset);
      CreateNewField(vFields, COORD_WES, ZD_NUMBER_FLD, 12, 6, uwOffset);
      CreateNewField(vFields, COORD_ADD, ZD_ALPHA_FLD, 48, 0, uwOffset);
      CreateNewField(vFields, RADIUS, ZD_NUMBER_FLD, 12, 2, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsClusterLevelDBF");
      throw;
   }
}
