// Class stsClusterData
// Adam J Vaughn
// 9/4/2002

#include "SaTScan.h"
#pragma hdrstop

#include "stsClusterData.h"
#include "Cluster.h"

const char *	CLUSTER_FILE_EXT	= ".col";

// this class is a record class to store each cluster info data record
// cluster level record class
ClusterRecord::ClusterRecord(const bool bPrintEllipses, const bool bPrintPVal) : BaseOutputRecord() {
   Init();
   gbPrintEllipses = bPrintEllipses;
   gbPrintPVal = bPrintPVal;
}

ClusterRecord::~ClusterRecord() {
}

int ClusterRecord::GetNumFields() {
   return (13 + (gbPrintEllipses ? 2 : 0) + (gbPrintPVal ? 1 : 0) + gvsAdditCoords.GetNumElements());
}

// function to return the field value from the specified field number
// pre: none
// post: sets and returns the fieldvalue
ZdFieldValue ClusterRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {    
      if(iFieldNumber < 0 || iFieldNumber >= GetNumFields())
         ZdGenerateException("Invalid index, out of range", "Error!");
      
      if (iFieldNumber < 5) {
         switch (iFieldNumber) {
            case 0:
               BaseOutputRecord::SetFieldValueAsDouble(fv, double(glRunNumber)); break;
            case 1:
               BaseOutputRecord::SetFieldValueAsString(fv, gsLocationID); break;
            case 2:
               BaseOutputRecord::SetFieldValueAsDouble(fv, double(giClusterNumber)); break;
            case 3:
               BaseOutputRecord::SetFieldValueAsString(fv, gsFirstCoord); break;
            case 4:
               BaseOutputRecord::SetFieldValueAsString(fv, gsSecondCoord); break;
         }
      }
      else {     // and this is where it starts to get messy - AJV
         if (gvsAdditCoords.size() > 0) {
            // fields 5 to 5+size()
            if (iFieldNumber >= 5 && iFieldNumber < (int)(gvsAdditCoords.size() + 5) )
               BaseOutputRecord::SetFieldValueAsString(fv, gvsAdditCoords[iFieldNumber-5]);
         }

         // subtract out the vector elements from the index and continue on like the vector
         // wasn't even there - AJV
         int iFieldAfterVector = iFieldNumber - gvsAdditCoords.size();

         if (iFieldAfterVector == 5)
            BaseOutputRecord::SetFieldValueAsString(fv, gsRadius);

         // if we should print ellipses then do so, else increase the field number to skip past them 

         if (gbPrintEllipses) {
            if (iFieldAfterVector == 6)
               BaseOutputRecord::SetFieldValueAsString(fv, gsEllipseAngles);
            if (iFieldAfterVector == 7)
               BaseOutputRecord::SetFieldValueAsString(fv, gsEllipseShapes);
         }
         else
            iFieldAfterVector += 2;

         switch (iFieldAfterVector) {
            case 8:
               BaseOutputRecord::SetFieldValueAsDouble(fv, double(glNumAreas)); break;
            case 9:
               BaseOutputRecord::SetFieldValueAsDouble(fv, double(glObserved)); break;
            case 10:
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdExpected); break;
            case 11:
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdRelRisk); break;
            case 12:
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdLogLikelihood); break;
         }

         // if we need to print the pval then do so, else increase the field number to skip past it
         if(iFieldAfterVector == 13) {
            if (gbPrintPVal)
               BaseOutputRecord::SetFieldValueAsDouble(fv, gdPValue);
            else
               ++iFieldAfterVector;
         }            

         switch (iFieldAfterVector) {
            case 14:
               BaseOutputRecord::SetFieldValueAsString(fv, gsStartDate); break;
            case 15:
               BaseOutputRecord::SetFieldValueAsString(fv, gsEndDate); break;
         }
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()", "ClusterRecord");
      throw;
   }
   return fv;  
}

void ClusterRecord::Init() {
   glRunNumber          = 0;
   gsLocationID         = "";
   giClusterNumber      = 0;
   gsFirstCoord         = "";
   gsSecondCoord        = "";
   gsRadius             = "";
   gsEllipseAngles      = "";
   gsEllipseShapes      = "";
   glNumAreas           = 0;
   glObserved           = 0;
   gdExpected           = 0.0;
   gdRelRisk            = 0.0;
   gdLogLikelihood      = 0.0;
   gdPValue             = 0.0;
   gsStartDate          = "";
   gsEndDate            = "";
}


//===============================================================================
// This class is responsible for the storage of the cluster information to be outputted.
// It is not responsible for any writing to a file format, just for the storage of the 
// data. 
//===============================================================================

// constructor
__fastcall stsClusterData::stsClusterData(const ZdString& sOutputFileName, const long lRunNumber, const int iCoordType,
                                          const int iModelType, const int iDimension,
                                          const bool bPrintPVal, const bool bPrintEllipses) 
                          : BaseOutputStorageClass() {
   try {
      Init();
      Setup(sOutputFileName, iModelType, iDimension, iCoordType, lRunNumber, bPrintEllipses,bPrintPVal);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsClusterData");
      throw;
   }
}

// destructor
stsClusterData::~stsClusterData() {
}

// global inits
void stsClusterData::Init() {
   giDimension = 0;
   giModelType = 0;
   glRunNumber = 0;
   giCoordType = 0;
}

// records the calculated data from the cluster into the dBase file
// pre: pCluster has been initialized with calculated data
// post: function will record the appropraite data into the dBase record
void stsClusterData::RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber) {
   ZdString                     sRadius, sLatitude, sLongitude;
   float                        fPVal;
   ZdString                     sTempValue, sStartDate, sEndDate, sShape, sAngle;
   ZdVector<ZdString>        vAdditCoords;
   ClusterRecord* 		pRecord = 0;

   try {                  
      pRecord = new ClusterRecord(gbPrintEllipses, gbPrintPVal);

#ifdef INCLUDE_RUN_HISTORY
      pRecord->SetRunNumber(double(glRunNumber));
#endif      
            
      pRecord->SetClusterNumber(iClusterNumber); 	// cluster number

      // ellipse shape and angle      
      if(gbPrintEllipses) {
         SetEllipseString(sAngle, sShape, pCluster, pData);      
         pRecord->SetEllipseAngles(sAngle);
         pRecord->SetEllipseShapes(sShape);
      }
      
      // cluster start and end date
      SetStartAndEndDates(sStartDate, sEndDate, pCluster, pData);
      pRecord->SetEndDate(sEndDate);
      pRecord->SetStartDate(sStartDate);

      pRecord->SetObserved(pCluster.m_nCases);
      pRecord->SetExpected(pCluster.m_nMeasure);
      
      // central area id
      SetAreaID(sTempValue, pCluster, pData);
      pRecord->SetLocationID(sTempValue);
            
      // log likliehood or tst_stat if space-time permutation
      pRecord->SetLogLikelihood(pCluster.m_nLogLikelihood);
      
      pRecord->SetNumAreas(pCluster.m_nTracts);
           
      // p value
      if (gbPrintPVal) {
         fPVal = (float) pCluster.GetPVal(pData.m_pParameters->m_nReplicas);
         pRecord->SetPValue(fPVal);
      }
                 
      pRecord->SetRelativeRisk(pCluster.GetRelativeRisk(pData.GetMeasureAdjustment()));

      // coordinates
      SetCoordinates(sLatitude, sLongitude, sRadius, vAdditCoords, pCluster, pData);
      pRecord->SetFirstCoordinate(sLatitude);
      pRecord->SetAdditionalCoordinates(vAdditCoords);
      pRecord->SetSecondCoordinate(sLongitude);
      pRecord->SetRadius(sRadius);
      
      BaseOutputStorageClass::AddRecord(pRecord);
   }
   catch (ZdException &x) {
      delete pRecord; 	 
      x.AddCallpath("RecordClusterData()", "stsClusterData");
      throw;
   }
}


// formats the string for the Area ID
// pre: none
// post: sTempvalue will contain the legible area id
void stsClusterData::SetAreaID(ZdString& sTempValue, const CCluster& pCluster, const CSaTScanData& pData) {
   try {
      if (pCluster.GetClusterType() == PURELYTEMPORAL)
          sTempValue = "n/a";
      else
         sTempValue = pData.GetGInfo()->giGetGid(pCluster.m_Center);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetAreaID", "DBaseOutput");
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
void stsClusterData::SetCoordinates(ZdString& sLatitude, ZdString& sLongitude, ZdString& sRadius,
                                        ZdVector<ZdString>& vAdditCoords,
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
      x.AddCallpath("SetCoordinates()", "stsClusterData");
      throw;
   }
}

// function to set the strings that are to be printed in the ellipse fields
// pre : cluster and data defined with appropraite data
// post : strings will be returned through reference with the values to be printed
void stsClusterData::SetEllipseString(ZdString& sAngle, ZdString& sShape, const CCluster& pCluster, const CSaTScanData& pData) {
   char  sAngleBuffer[256], sShapeBuffer[256];

   try {
         if (pCluster.m_nClusterType == PURELYTEMPORAL){
            sAngle = "n/a";
            sShape = "n/a";
         }
         else {
            if(pCluster.m_iEllipseOffset == 0 && pData.m_nNumEllipsoids > 0) {
               sShape = "1.000";
               sAngle = "0.000";
            }
            else {
               sprintf(sAngleBuffer, "%-8.3f", pCluster.ConvertAngleToDegrees(pData.mdE_Angles[pCluster.m_iEllipseOffset-1]));
               sprintf(sShapeBuffer, "%-8.3f", pData.mdE_Shapes[pCluster.m_iEllipseOffset-1]);
               sShape = sShapeBuffer;
               sAngle = sAngleBuffer;
            }
         }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetEllipseString()", "stsClusterData");
      throw;
   }
}

// function to determine the correct start and end dates to print to the file - for all but purely spatial
// analysis we'll use the cluster start and end dates but for purely spatial we'll use study begin and end dates -
// this is the same method used in the ASCII file so we'll follow their lead - AJV 10/2/2002
// pre : pCluster and pData are initialized with the correct data
// post : will set sStartDate and sEndDate with the appropriate value based upon the analysis type
void stsClusterData::SetStartAndEndDates(ZdString& sStartDate, ZdString& sEndDate, const CCluster& pCluster, const CSaTScanData& pData) {
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
      x.AddCallpath("SetStartAndEndDates()", "stsClusterData");
      throw;
   }
}

// internal setup
void stsClusterData::Setup(const ZdString& sOutputFileName, const int iModelType,
                          const int iDimension,  const int iCoordType,
                          const long lRunNumber, const bool bPrintEllipses, const bool bPrintPVal) {
   try {
      ZdString sTempName(sOutputFileName);
      ZdString sExt(ZdFileName(sOutputFileName).GetExtension());
      if(sExt.GetLength())
         sTempName.Replace(sExt, CLUSTER_FILE_EXT);
      else
         sTempName << CLUSTER_FILE_EXT;
      gsFileName = sTempName;

      gbPrintEllipses = bPrintEllipses;
      giModelType = iModelType;
      giDimension = iDimension;
      glRunNumber = lRunNumber;
      gbPrintPVal = bPrintPVal;
      giCoordType = iCoordType;
      SetupFields();
   }
   catch(ZdException &x) {
      x.AddCallpath("Setup()", "stsClusterData");
      throw;
   }
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void stsClusterData::SetupFields() {
   unsigned short uwOffset = 0;    // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
   ZdString       sTemp;

   try {
      // please take note that this function here determines the ordering of the fields in the file
      CreateField(gvFields, RUN_NUM_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);
      CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
      CreateField(gvFields, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
      CreateField(gvFields, (giCoordType != CARTESIAN) ? COORD_LAT_FIELD : COORD_X_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateField(gvFields, (giCoordType != CARTESIAN) ? COORD_LONG_FIELD : COORD_Y_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);

      // Only Cartesian coordinates have more than two dimensions. Lat/Long is consistently assigned to 3 dims
      // throughout the program eventhough the third dim is never used. When you print the third it is blank
      // and meaningless and thus does not need to be included here. - AJV 10/2/2002
      if(giCoordType == CARTESIAN && giDimension > 2) {
         for(int i = 3; i <= giDimension; ++i) {
            sTemp << ZdString::reset << COORD_Z_FIELD << (i-2);
            CreateField(gvFields, sTemp.GetCString(), ZD_ALPHA_FLD, 16, 0, uwOffset);
         }
      }

      CreateField(gvFields, RADIUS_FIELD, ZD_ALPHA_FLD, 12, 0, uwOffset);

      // ellipse fields
      if (gbPrintEllipses) {
         CreateField(gvFields, E_ANGLE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
         CreateField(gvFields, E_SHAPE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      }

      CreateField(gvFields, NUM_AREAS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateField(gvFields, OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateField(gvFields, EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateField(gvFields, REL_RISK_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
      // if model is space time permutation then tst_stat, else log likelihood
      CreateField(gvFields, (giModelType != SPACETIMEPERMUTATION ? LOG_LIKL_FIELD : TST_STAT_FIELD), ZD_NUMBER_FLD, 16, 6, uwOffset);

      if(gbPrintPVal)
         CreateField(gvFields, P_VALUE_FLD, ZD_NUMBER_FLD, 12, 5, uwOffset);
      CreateField(gvFields, START_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      CreateField(gvFields, END_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsClusterData");
      throw;
   }
}
