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
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName, const int iDimension)
                             : DBaseOutput(lRunNumber, iCoordType) {
   try {
      Init();
      Setup(sOutputFileName.GetFullPath(), iDimension);
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
   giDimension = 0;
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterLevelDBF::RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber, tract_t tTract) {
   float                        fRadius = 0.0, fLatitude = 0.0, fLongitude = 0.0, fPVal;
   ZdString                     sTempValue;
   ZdTransaction*               pTransaction = 0;
   char                         sStartDate[15], sEndDate[15];
   DBFFile                      File(gsFileName.GetCString());
   std::vector<float>           vAdditCoords;

   try {
      std::auto_ptr<ZdFileRecord>  pRecord(File.GetNewRecord());

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
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, LOC_ID));

      SetCoordinates(fLatitude, fLongitude, fRadius, vAdditCoords, pCluster, pData);

      // first two coords
      SetDoubleField(*pRecord, fLatitude, GetFieldNumber(gvFields, (giCoordType != CARTESIAN) ? COORD_LAT : COORD_X));
      SetDoubleField(*pRecord, fLongitude, GetFieldNumber(gvFields, (giCoordType != CARTESIAN) ? COORD_LONG : COORD_Y));

      // additional coords
      for(size_t i = 0; i < vAdditCoords.size(); ++i) {
         sTempValue << ZdString::reset << COORD_Z << (i+1);
         SetDoubleField(*pRecord, vAdditCoords[i], GetFieldNumber(gvFields, sTempValue.GetCString()));
      }

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
void stsClusterLevelDBF::SetCoordinates(float& fLatitude, float& fLongitude, float& fRadius, std::vector<float>& vAdditCoords,
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
            else
               vAdditCoords.push_back(pCoords[i]);
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
void stsClusterLevelDBF::Setup(const ZdString& sOutputFileName, const int iDimension) {
   try {
      // cluster level dbf has same filename as output file with cluster level extension - AJV 9/30/2002
      ZdString sTempName(sOutputFileName);
      sTempName.Replace(ZdFileName(sOutputFileName).GetExtension(), CLUSTER_LEVEL_EXT);
      gsFileName = sTempName;

      giDimension = iDimension;
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
   unsigned short uwOffset = 0;    // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
   ZdString       sTemp;

   try {
      CreateNewField(vFields, RUN_NUM, ZD_NUMBER_FLD, 8, 0, uwOffset);
      CreateNewField(vFields, CLUST_NUM, ZD_NUMBER_FLD, 5, 0, uwOffset);
      CreateNewField(vFields, LOC_ID, ZD_ALPHA_FLD, 30, 0, uwOffset);
      CreateNewField(vFields, (giCoordType != CARTESIAN) ? COORD_LAT : COORD_X, ZD_NUMBER_FLD, 12, 6, uwOffset);
      CreateNewField(vFields, (giCoordType != CARTESIAN) ? COORD_LONG : COORD_Y, ZD_NUMBER_FLD, 12, 6, uwOffset);

      if(giDimension > 2) {
         for(int i = 3; i <= giDimension; ++i) {
            sTemp << ZdString::reset << COORD_Z << (i-2);
            CreateNewField(vFields, sTemp.GetCString(), ZD_NUMBER_FLD, 12, 6, uwOffset);
         }
      }

      CreateNewField(vFields, RADIUS, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, NUM_AREAS, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, OBSERVED, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, EXPECTED, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, REL_RISK, ZD_NUMBER_FLD, 12, 3, uwOffset);
      CreateNewField(vFields, LOG_LIKL, ZD_NUMBER_FLD, 16, 6, uwOffset);
      CreateNewField(vFields, P_VALUE, ZD_NUMBER_FLD, 12, 5, uwOffset);
      CreateNewField(vFields, START_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateNewField(vFields, END_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsClusterLevelDBF");
      throw;
   }
}
