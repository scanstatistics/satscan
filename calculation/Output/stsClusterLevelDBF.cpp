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
__fastcall stsClusterLevelDBF::stsClusterLevelDBF(const long lRunNumber, const int iCoordType,
                                                  const ZdFileName& sOutputFileName, const int iDimension,
                                                  const bool bPrintPVal, const bool bPrintEllipses) : DBaseOutput(lRunNumber, bPrintPVal, iCoordType) {
   try {
      Init();
      Setup(sOutputFileName.GetFullPath(), iDimension, bPrintEllipses);
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
   std::string                  sRadius, sLatitude, sLongitude;
   float                        fPVal;
   ZdString                     sTempValue;
   ZdTransaction*               pTransaction = 0;
   std::string                  sStartDate, sEndDate, sShape, sAngle;
   DBFFile                      File(gsFileName.GetCString());
   std::vector<std::string>     vAdditCoords;

   try {
      std::auto_ptr<ZdFileRecord>  pRecord(File.GetNewRecord());

      // define record data
      // run number field  - from the run history file  AJV 9/4/2002
#ifdef INCLUDE_RUN_HISTORY
      SetDoubleField(*pRecord, double(glRunNumber), GetFieldNumber(gvFields, RUN_NUM));
#endif

      // cluster start and end date
      SetStartAndEndDates(sStartDate, sEndDate, pCluster, pData);
      SetStringField(*pRecord, sStartDate.c_str(), GetFieldNumber(gvFields, START_DATE));
      SetStringField(*pRecord, sEndDate.c_str(), GetFieldNumber(gvFields, END_DATE));

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
      if (gbPrintPVal) {
         fPVal = (float) pCluster.GetPVal(pData.m_pParameters->m_nReplicas);
         SetDoubleField(*pRecord, fPVal, GetFieldNumber(gvFields, P_VALUE));
      }

      // number of areas in the cluster
      SetDoubleField(*pRecord, pCluster.m_nTracts, GetFieldNumber(gvFields, NUM_AREAS));

      // central area id
      SetAreaID(sTempValue, pCluster, pData);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, LOC_ID));

      // first two coords
      SetCoordinates(sLatitude, sLongitude, sRadius, vAdditCoords, pCluster, pData);
      SetStringField(*pRecord, sLatitude.c_str(), GetFieldNumber(gvFields, (giCoordType != CARTESIAN) ? COORD_LAT : COORD_X));
      SetStringField(*pRecord, sLongitude.c_str(), GetFieldNumber(gvFields, (giCoordType != CARTESIAN) ? COORD_LONG : COORD_Y));

      // additional coords
      for(size_t i = 0; i < vAdditCoords.size(); ++i) {
         sTempValue << ZdString::reset << COORD_Z << (i+1);
         SetStringField(*pRecord, vAdditCoords[i].c_str(), GetFieldNumber(gvFields, sTempValue.GetCString()));
      }

      // radius
      SetStringField(*pRecord, sRadius.c_str(), GetFieldNumber(gvFields, RADIUS));

      // ellipse shape and angle      
      if(gbPrintEllipses) {
         SetEllipseString(sAngle, sShape, pCluster, pData);
         SetStringField(*pRecord, sShape.c_str(), GetFieldNumber(gvFields, E_SHAPE));
         SetStringField(*pRecord, sAngle.c_str(), GetFieldNumber(gvFields, E_ANGLE));
      }

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
// by the way, all of the sprintf's and bouncing back and forth between floats and strings here
// is necessary because the customer wishes to print the float if applicable but a "n/a" if not which
// forces me to make the field a ALPHA_FLD and requires me to do this ugliness - AJV 10/2/2002
// pre : none that I can think of
// post : sets the values for long, lat, sAdditCoords, and radius
void stsClusterLevelDBF::SetCoordinates(std::string& sLatitude, std::string& sLongitude, std::string& sRadius,
                                        std::vector<std::string>& vAdditCoords,
                                        const CCluster& pCluster, const CSaTScanData& pData) {
   double       *pCoords = 0, *pCoords2 = 0;
   float        fLatitude, fLongitude, fRadius;
   char         sLatBuffer[64], sLongBuffer[64], sAdditBuffer[64], sRadBuffer[64];

   try {
      (pData.GetGInfo())->giGetCoords(pCluster.m_Center, &pCoords);
      (pData.GetTInfo())->tiGetCoords(pData.GetNeighbor(pCluster.m_iEllipseOffset, pCluster.m_Center, pCluster.m_nTracts), &pCoords2);

      if(giCoordType == CARTESIAN) {        // if cartesian coords
         if(pCluster.m_nClusterType != PURELYTEMPORAL) {
            for (int i = 0; i < pData.m_pParameters->m_nDimension; ++i) {
               if (i == 0) {
                  sprintf(sLatBuffer, "%12.2f", pCoords[i]);
                  sLatitude = sLatBuffer;
               }
               else if (i == 1) {
                  sprintf(sLongBuffer, "%12.2f", pCoords[i]);
                  sLongitude = sLongBuffer;
               }
               else  {
                  sprintf(sAdditBuffer, "%12.2f", pCoords[i]);
                  vAdditCoords.push_back(sAdditBuffer);
               }
            }
         }
         else {              // else we are doing a cartesian purely temporal, print out all n/a's
            sLatitude = "n/a";
            sLongitude = "n/a";
            for (int i = 2; i < pData.m_pParameters->m_nDimension; ++i) {
               vAdditCoords.push_back("n/a");
            }
            sRadius = "n/a";
         }
      }
      else {
         ConvertToLatLong(&fLatitude, &fLongitude, pCoords);
         sprintf(sLongBuffer, "%lf", fLongitude);
         sLongitude = sLongBuffer;
         sprintf(sLatBuffer, "%lf", fLatitude);
         sLatitude = sLatBuffer;
      }

      if(giCoordType != CARTESIAN || (giCoordType == CARTESIAN && pCluster.m_nClusterType != PURELYTEMPORAL)) {
         fRadius = (float)sqrt((pData.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
         sprintf(sRadBuffer, "%5.2f", fRadius);
         sRadius = sRadBuffer;
      }

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

// function to set the strings that are to be printed in the ellipse fields
// pre : cluster and data defined with appropraite data
// post : strings will be returned through reference with the values to be printed
void stsClusterLevelDBF::SetEllipseString(std::string& sAngle, std::string& sShape, const CCluster& pCluster, const CSaTScanData& pData) {
   char  sAngleBuffer[256], sShapeBuffer[256];

   try {
      if (pCluster.m_nClusterType == PURELYTEMPORAL){
         sAngle = "n/a";
         sShape = "n/a";
      }
      else {
         if(pCluster.m_iEllipseOffset == 0 && pData.m_nNumEllipsoids > 0) {
            sShape = "1.0";
            sAngle = "0.0";
         }
         else {
            sprintf(sAngleBuffer, "%8.3f", pCluster.ConvertAngleToDegrees(pData.mdE_Angles[pCluster.m_iEllipseOffset-1]));
            sprintf(sShapeBuffer, "%8.3f", pData.mdE_Shapes[pCluster.m_iEllipseOffset-1]);
            sShape = sShapeBuffer;
            sAngle = sAngleBuffer;
         }
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetEllipseString()", "stsClusterLevelDBF");
      throw;
   }
}

// function to determine the correct start and end dates to print to the file - for all but purely spatial
// analysis we'll use the cluster start and end dates but for purely spatial we'll use study begin and end dates -
// this is the same method used in the ASCII file so we'll follow their lead - AJV 10/2/2002
// pre : pCluster and pData are initialized with the correct data
// post : will set sStartDate and sEndDate with the appropriate value based upon the analysis type
void stsClusterLevelDBF::SetStartAndEndDates(std::string& sStartDate, std::string& sEndDate, const CCluster& pCluster, const CSaTScanData& pData) {
   char       sStart[64], sEnd[64];

   try {
      if (pData.m_pParameters->m_nAnalysisType != PURELYSPATIAL) {
         JulianToChar(sStart, pCluster.m_nStartDate);
         JulianToChar(sEnd, pCluster.m_nEndDate);
         sStartDate = sStart;
         sEndDate = sEnd;
      }
      else {
         sStartDate = pData.m_pParameters->m_szStartDate;
         sEndDate = pData.m_pParameters->m_szEndDate;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetStartAndEndDates()", "stsClusterLevelDBF");
      throw;
   }
}


// internal setup
void stsClusterLevelDBF::Setup(const ZdString& sOutputFileName, const int iDimension, const bool bPrintEllipses) {
   try {
      // cluster level dbf has same filename as output file with cluster level extension - AJV 9/30/2002
      ZdString sTempName(sOutputFileName);
      ZdString sExt(ZdFileName(sOutputFileName).GetExtension());
      if(sExt.GetLength()) 
         sTempName.Replace(sExt, CLUSTER_LEVEL_EXT);
      else
         sTempName << CLUSTER_LEVEL_EXT;
      gsFileName = sTempName;
      gbPrintEllipses = bPrintEllipses;
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
      // please take note that this function here determines the ordering of the fields in the file
      // everything else is written generically enough that ordering does not matter due to the
      // GetFieldNumber function - AJV 10/2/2002
#ifdef INCLUDE_RUN_HISTORY
      CreateNewField(vFields, RUN_NUM, ZD_NUMBER_FLD, 8, 0, uwOffset);
#endif
      CreateNewField(vFields, LOC_ID, ZD_ALPHA_FLD, 30, 0, uwOffset);
      CreateNewField(vFields, CLUST_NUM, ZD_NUMBER_FLD, 5, 0, uwOffset);
      CreateNewField(vFields, (giCoordType != CARTESIAN) ? COORD_LAT : COORD_X, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateNewField(vFields, (giCoordType != CARTESIAN) ? COORD_LONG : COORD_Y, ZD_ALPHA_FLD, 16, 0, uwOffset);

      // Only Cartesian coordinates have more than two dimensions. Lat/Long is consistently assigned to 3 dims
      // throughout the program eventhough the third dim is never used. When you print the third it is blank
      // and meaningless and thus does not need to be included here. - AJV 10/2/2002
      if(giCoordType == CARTESIAN && giDimension > 2) {
         for(int i = 3; i <= giDimension; ++i) {
            sTemp << ZdString::reset << COORD_Z << (i-2);
            CreateNewField(vFields, sTemp.GetCString(), ZD_ALPHA_FLD, 16, 0, uwOffset);
         }
      }

      CreateNewField(vFields, RADIUS, ZD_ALPHA_FLD, 12, 0, uwOffset);

      if(gbPrintEllipses) {    // whether or not to print ellipse descriptors
         CreateNewField(vFields, E_ANGLE, ZD_ALPHA_FLD, 16, 0, uwOffset);
         CreateNewField(vFields, E_SHAPE, ZD_ALPHA_FLD, 16, 0, uwOffset);
      }

      CreateNewField(vFields, NUM_AREAS, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, OBSERVED, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateNewField(vFields, EXPECTED, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateNewField(vFields, REL_RISK, ZD_NUMBER_FLD, 12, 3, uwOffset);
      CreateNewField(vFields, LOG_LIKL, ZD_NUMBER_FLD, 16, 6, uwOffset);

      if(gbPrintPVal)
         CreateNewField(vFields, P_VALUE, ZD_NUMBER_FLD, 12, 5, uwOffset);
      CreateNewField(vFields, START_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateNewField(vFields, END_DATE, ZD_ALPHA_FLD, 16, 0, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsClusterLevelDBF");
      throw;
   }
}
