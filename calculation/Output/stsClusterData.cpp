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
ClusterRecord::ClusterRecord(const bool bPrintEllipses, const bool bPrintPVal, const bool bIncludeRunHistory,
                             const bool bSpaceTimeModel, const bool bDuczmalCorrect)
             : BaseOutputRecord() , gbSpaceTimeModel(bSpaceTimeModel),  gbDuczmalCorrect(bDuczmalCorrect),
                                    gbPrintEllipses(bPrintEllipses), gbPrintPVal(bPrintPVal), gbIncludeRunHistory(bIncludeRunHistory)
{
   Init();
}

ClusterRecord::~ClusterRecord() {
}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool ClusterRecord::GetFieldIsBlank(int iFieldNumber) {
   try {
      if ( iFieldNumber < 0 || (size_t)iFieldNumber >= gvbBlankFields.size())
         ZdGenerateException("Invalid index, out of range!", "Error!");
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFieldIsBlank()", "AreaSpecificRecord");
      throw;
   }
   return gvbBlankFields[iFieldNumber];
}

// function to return the field value from the specified field number
// pre: none
// post: sets and returns the fieldvalue
ZdFieldValue ClusterRecord::GetValue(int iFieldNumber) {
   ZdFieldValue fv;
   
   try {
      if(iFieldNumber < 0 || iFieldNumber >= GetNumFields())
         ZdGenerateException("Invalid index, out of range", "Error!");

      // if we should include run history number then do so, else skip past to the next field
      if (gbIncludeRunHistory) {
         if (iFieldNumber == 0)
             BaseOutputRecord::SetFieldValueAsDouble(fv, double(glRunNumber));
      }
      else
         ++iFieldNumber;

      switch (iFieldNumber) {
         case 1:
            BaseOutputRecord::SetFieldValueAsString(fv, gsLocationID); break;
         case 2:
            BaseOutputRecord::SetFieldValueAsDouble(fv, double(giClusterNumber)); break;
         case 3:
            BaseOutputRecord::SetFieldValueAsString(fv, gsFirstCoord); break;
         case 4:
            BaseOutputRecord::SetFieldValueAsString(fv, gsSecondCoord); break;
      }
         // and this is where it starts to get messy - AJV
      if (gvsAdditCoords.size() > 0) {
         // fields 5 to 5+size()
         if (iFieldNumber >= 5 && iFieldNumber < (int)(gvsAdditCoords.size() + 5) )
            BaseOutputRecord::SetFieldValueAsString(fv, gvsAdditCoords[iFieldNumber-5]);
      }

      // subtract out the vector elements from the index and continue on like the vector
      // wasn't even there - AJV
      int iFieldAfterVector = iFieldNumber - gvsAdditCoords.size();

      if (iFieldAfterVector == 5)    // radius field
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
            BaseOutputRecord::SetFieldValueAsDouble(fv, gbSpaceTimeModel ? gdTestStat : gdLogLikelihood); break;
      }

      // if we are doing duczmal correctness with ellipses and we're not in the space time model then set test stat
      // else just increase the field number to skip past
      if (gbPrintEllipses && gbDuczmalCorrect && !gbSpaceTimeModel) {
         if(iFieldAfterVector == 13)
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdTestStat);
      }
      else
         ++iFieldAfterVector;

      // if we need to print the pval then do so, else increase the field number to skip past it
      if (gbPrintPVal) {
         if(iFieldAfterVector == 14)
            BaseOutputRecord::SetFieldValueAsDouble(fv, gdPValue);
      }
      else
         ++iFieldAfterVector;

      switch (iFieldAfterVector) {
         case 15:
            BaseOutputRecord::SetFieldValueAsString(fv, gsStartDate); break;
         case 16:
            BaseOutputRecord::SetFieldValueAsString(fv, gsEndDate); break;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetValue()", "ClusterRecord");
      throw;
   }
   return fv;
}

// internal global variable defaults
void ClusterRecord::Init() {
   glRunNumber = glNumAreas = glObserved = 0;
   gdExpected = gdPValue = gdLogLikelihood = gdRelRisk = 0.0;
   giClusterNumber      = 0;

   // 12 permanent fields plus the optional fields if they are included
   giNumFields =  (12 + (gbIncludeRunHistory ? 1 : 0) + (gbPrintEllipses ? 2 : 0)
                      + (!gbSpaceTimeModel && gbPrintEllipses && gbDuczmalCorrect ? 1 : 0)
                      + (gbPrintPVal ? 1 : 0) + gvsAdditCoords.size());

   gvbBlankFields.reserve(giNumFields);
   for ( int i = 0; i < giNumFields; ++i )
      gvbBlankFields.push_back(false);
}

// sets the field at fieldnumber to either be blank or non-blank
// pre : none
// post : sets the iFieldNumber element of the global vector to bBlank
void ClusterRecord::SetFieldIsBlank(int iFieldNumber, bool bBlank) {
   try {
      if (iFieldNumber < 0 || (size_t)iFieldNumber >= gvbBlankFields.size())
         ZdGenerateException("Invalid index, out of range!", "Error!");

      gvbBlankFields[iFieldNumber] = bBlank;
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldIsBlank()", "AreaSpecificRecord");
      throw;
   }
}


//===============================================================================
// This class is responsible for the storage of the cluster information to be outputted.
// It is not responsible for any writing to a file format, just for the storage of the 
// data. 
//===============================================================================

// constructor
stsClusterData::stsClusterData(BasePrint *pPrintDirection, const ZdString& sOutputFileName,
                                          const long lRunNumber, const int iCoordType,
                                          ProbabiltyModelType eProbabiltyModelType, const int iDimension,
                                          const bool bPrintPVal, const bool bPrintEllipses, const bool bDuczmalCorrect)
                          : BaseOutputStorageClass(pPrintDirection), gbPrintEllipses(bPrintEllipses),
                            geProbabiltyModelType(eProbabiltyModelType), giDimension(iDimension),
                            glRunNumber(lRunNumber), gbPrintPVal(bPrintPVal), giCoordType(iCoordType),
                            gbDuczmalCorrect(bDuczmalCorrect) {
   try {
      Init();
      Setup(sOutputFileName);
   }
   catch (ZdException &x) {
      if(pPrintDirection) {
         pPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
         pPrintDirection->SatScanPrintWarning("\nWarning - Unable to create most likely cluster data for output file.\n");
      }
   }
}

// destructor
stsClusterData::~stsClusterData() {
}

// global inits
void stsClusterData::Init() {
   //The relational link between the run history file and the cluster information
   //file was asked to be removed. For now, leave code in-place but just turn
   //feature off. 
   gbIncludeRunHistory = false;
}

// records the required data to be stored in the cluster output file, stores the values
// in the global vector of cluster records
// pre: pCluster has been initialized with calculated data
// post: function will record the appropriate data into the cluster record
void stsClusterData::RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, int iNumSimulations) {
   ZdString                     sRadius, sLatitude, sLongitude;
   float                        fPVal;
   ZdString                     sTempValue, sStartDate, sEndDate, sShape, sAngle;
   std::vector<std::string>     vAdditCoords;
   ClusterRecord* 		pRecord = 0;

   try {                  
      pRecord = new ClusterRecord(gbPrintEllipses, gbPrintPVal, gbIncludeRunHistory, geProbabiltyModelType == SPACETIMEPERMUTATION, gbDuczmalCorrect);

      if (gbIncludeRunHistory)
         pRecord->SetRunNumber(double(glRunNumber));
            
      pRecord->SetClusterNumber(iClusterNumber); 	// cluster number

      // ellipse shape and angle      
      if(gbPrintEllipses) {
         SetEllipseString(sAngle, sShape, theCluster, theData);      
         pRecord->SetEllipseAngles(sAngle);
         pRecord->SetEllipseShapes(sShape);
      }
      
      // cluster start and end date
      SetStartAndEndDates(sStartDate, sEndDate, theCluster, theData);
      pRecord->SetEndDate(sEndDate);
      pRecord->SetStartDate(sStartDate);

      pRecord->SetObserved(theCluster.GetCaseCount(0));
      pRecord->SetExpected(theData.GetMeasureAdjustment()* theCluster.GetMeasure(0));
      
      // central area id
      SetAreaID(sTempValue, theCluster, theData);
      pRecord->SetLocationID(sTempValue);

      // log likliehood or tst_stat if space-time permutation
      if(geProbabiltyModelType == SPACETIMEPERMUTATION) {
         if(theCluster.m_iEllipseOffset !=0 && gbDuczmalCorrect)
            pRecord->SetTestStat(theCluster.GetDuczmalCorrectedLogLikelihoodRatio());
         else
            pRecord->SetTestStat(theCluster.m_nRatio);
      }
      else {
         pRecord->SetLogLikelihood(theCluster.m_nRatio);
         if (gbDuczmalCorrect)
           //print duczmal adjusted LLR even if shape is circle, like in results file.
           //calling GetDuczmalCorrectedLogLikelihoodRatio() for a circle returns LLR.
           pRecord->SetTestStat(theCluster.GetDuczmalCorrectedLogLikelihoodRatio());
      }

      pRecord->SetNumAreas(theCluster.GetClusterType() == PURELYTEMPORAL ? theData.GetNumTracts() : theCluster.m_nTracts);
           
      // p value
      if (gbPrintPVal) {
         fPVal = (float) theCluster.GetPVal(iNumSimulations);
         pRecord->SetPValue(fPVal);
      }
                 
      pRecord->SetRelativeRisk(theCluster.GetRelativeRisk(theData.GetMeasureAdjustment()));

      // coordinates
      SetCoordinates(sLatitude, sLongitude, sRadius, vAdditCoords, theCluster, theData);
      pRecord->SetFirstCoordinate(sLatitude);
      pRecord->SetAdditionalCoordinates(vAdditCoords);
      pRecord->SetSecondCoordinate(sLongitude);
      pRecord->SetRadius(sRadius);
      
      BaseOutputStorageClass::AddRecord(pRecord);
   }
   catch (ZdException &x) {
      delete pRecord; 	 
      gpPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
      gpPrintDirection->SatScanPrintWarning("\nWarning - Unable to record most likely cluster data for output file.\n");
   }
}


// formats the string for the Area ID
// pre: none
// post: sTempvalue will contain the legible area id
void stsClusterData::SetAreaID(ZdString& sTempValue, const CCluster& pCluster, const CSaTScanData& pData) {
   try {
      if (pCluster.GetClusterType() == PURELYTEMPORAL)
          sTempValue = "All";
      else
         sTempValue = pData.GetGInfo()->giGetGid(pCluster.m_Center);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetAreaID", "DBaseOutput");
      throw;
   }
}

// pre : none
// post : sets the values for long, lat, sAdditCoords, and radius
void stsClusterData::SetCoordinates(ZdString& sLatitude, ZdString& sLongitude, ZdString& sRadius,
                                    std::vector<std::string>& vAdditCoords,
                                    const CCluster& pCluster, const CSaTScanData& pData) {
  int          i;
  double     * pCoords=0, * pCoords2=0;
  float        fLatitude, fLongitude, fRadius;
  char         sAdditBuffer[64], sRadBuffer[64];

   try {
     if (pCluster.m_nClusterType == PURELYTEMPORAL) {
       sLatitude = "n/a";
       sLongitude = "n/a";
       if (giCoordType == CARTESIAN)
         for (i=2; i < pData.GetParameters().GetDimensionsOfData(); ++i)
            vAdditCoords.push_back("n/a");
       sRadius = "n/a";
     }
     else {
       pData.GetGInfo()->giGetCoords(pCluster.m_Center, &pCoords);
       pData.GetTInfo()->tiGetCoords(pData.GetNeighbor(pCluster.m_iEllipseOffset, pCluster.m_Center, pCluster.m_nTracts), &pCoords2);
       switch (giCoordType) {
         case CARTESIAN : for (i=0; i < pData.GetParameters().GetDimensionsOfData(); ++i) {
                             if (i == 0)
                               sLatitude.printf("%12.2f", pCoords[i]);
                             else if (i == 1)
                               sLongitude.printf("%12.2f", pCoords[i]);
                             else  {
                               sprintf(sAdditBuffer, "%12.2f", pCoords[i]);
                               vAdditCoords.push_back(sAdditBuffer);
                             }
                          }
                          break;
         case LATLON    : ConvertToLatLong(&fLatitude, &fLongitude, pCoords);
                          sLongitude.printf("%lf", fLongitude);
                          sLatitude.printf("%lf", fLatitude);
                          break;
         default : ZdGenerateException("Unknown coordinate type '%d'.","SetCoordinates()", giCoordType);
       }
       fRadius = (float)sqrt((pData.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
       sRadius.printf("%5.2f", fRadius);
       free(pCoords);
       free(pCoords2);
     }
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
         if(pCluster.m_iEllipseOffset == 0 && pData.GetParameters().GetNumRequestedEllipses() > 0) {
            sShape = "1.000";
            sAngle = "0.000";
         }
         else {
            sprintf(sAngleBuffer, "%-8.3f", pCluster.ConvertAngleToDegrees(pData.GetAnglesArray()[pCluster.m_iEllipseOffset-1]));
            sprintf(sShapeBuffer, "%-8.3f", pData.GetShapesArray()[pCluster.m_iEllipseOffset-1]);
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
      if (pData.GetParameters().GetAnalysisType() != PURELYSPATIAL) {
         JulianToChar(sStart, pCluster.m_nStartDate);
         JulianToChar(sEnd, pCluster.m_nEndDate);
         sStartDate = sStart;
         sEndDate = sEnd;
      }
      else {
         sStartDate = pData.GetParameters().GetStudyPeriodStartDate().c_str();
         sEndDate = pData.GetParameters().GetStudyPeriodEndDate().c_str();
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetStartAndEndDates()", "stsClusterData");
      throw;
   }
}

// internal setup
void stsClusterData::Setup(const ZdString& sOutputFileName) {
   try {
      ZdString sTempName(sOutputFileName);
      ZdString sExt(ZdFileName(sOutputFileName).GetExtension());
      if(sExt.GetLength())
         sTempName.Replace(sExt, CLUSTER_FILE_EXT);
      else
         sTempName << CLUSTER_FILE_EXT;
      gsFileName = sTempName;
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
      if (gbIncludeRunHistory)
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
      if(geProbabiltyModelType == SPACETIMEPERMUTATION)
         CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 16, 6, uwOffset);
      else {
         CreateField(gvFields, LOG_LIKL_FIELD, ZD_NUMBER_FLD, 16, 6, uwOffset);
         // if there are ellipses and duczmal correctness then include both log likelihood and test stat
         if(gbPrintEllipses && gbDuczmalCorrect)
            CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 16, 6, uwOffset);
      }

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
