//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsClusterData.h"
#include "cluster.h"

/** class constructor */
stsClusterData::stsClusterData(const CParameters& Parameters, bool bExcludePValueField)
               :BaseOutputStorageClass(), gParameters(Parameters), gbExcludePValueField(bExcludePValueField) {
  try {
    SetupFields();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","LogLikelihoodData");
    throw;
  }
}

/** class destructor */
stsClusterData::~stsClusterData() {}

const char * stsClusterData::CLUSTER_FILE_EXT	        = ".col";
const char * stsClusterData::START_DATE_FLD	        = "START_DATE";
const char * stsClusterData::END_DATE_FLD	        = "END_DATE";
const char * stsClusterData::RADIUS_FIELD	        = "RADIUS";
const char * stsClusterData::E_ANGLE_FIELD              = "E_ANGLE";
const char * stsClusterData::E_SHAPE_FIELD              = "E_SHAPE";
const char * stsClusterData::NUM_LOCATIONS_FIELD        = "NUM_LOC";
const char * stsClusterData::COORD_LAT_FIELD	        = "LATITUDE";
const char * stsClusterData::COORD_LONG_FIELD	        = "LONGITUDE";
const char * stsClusterData::COORD_X_FIELD	        = "X";
const char * stsClusterData::COORD_Y_FIELD              = "Y";
const char * stsClusterData::COORD_Z_FIELD              = "Z";
const char * stsClusterData::OBS_DIV_EXP_FIELD	        = "ODE";
const char * stsClusterData::STREAM_OBSERVED_FIELD      = "OBS_DS";
const char * stsClusterData::STREAM_EXPECTED_FIELD      = "EXP_DS";
const char * stsClusterData::STREAM_OBS_DIV_EXP_FIELD   = "ODE_DS";

// formats the string for the Area ID
// pre: none
// post: sTempvalue will contain the legible area id
ZdString& stsClusterData::GetAreaID(ZdString& sAreaId, const CCluster& thisCluster, const CSaTScanData& DataHub) const {
  std::string   sBuffer;
  tract_t       tTractIndex;

  try {
    if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
      sAreaId = "All";
    else {
      tTractIndex = DataHub.GetNeighbor(thisCluster.GetEllipseOffset(), thisCluster.GetCentroidIndex(), 1);
      sAreaId = DataHub.GetTInfo()->tiGetTid(tTractIndex, sBuffer);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetAreaID","stsClusterData");
    throw;
  }
  return sAreaId;
}

/** Records the required data to be stored in the cluster output file, stores
    the values in the global vector of cluster records.
    pre: pCluster has been initialized with calculated data
    post: function will record the appropriate data into the cluster record   */
void stsClusterData::RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, unsigned int iNumSimsCompleted) {
  OutputRecord * pRecord = 0;
  ZdString       sBuffer;
  unsigned int   i;

  try {
    pRecord = new OutputRecord(gvFields);
    pRecord->GetFieldValue(GetFieldNumber(LOC_ID_FIELD)).AsZdString() = GetAreaID(sBuffer, theCluster, theData);
    pRecord->GetFieldValue(GetFieldNumber(CLUST_NUM_FIELD)).AsDouble() = iClusterNumber;
    WriteCoordinates(*pRecord, theCluster, theData);
    pRecord->GetFieldValue(GetFieldNumber(NUM_LOCATIONS_FIELD)).AsDouble() =
        theCluster.GetClusterType() == PURELYTEMPORALCLUSTER ? theData.GetNumTracts() : theCluster.GetNumTractsInnerCircle();
    if (gParameters.GetNumRequestedEllipses()) { // ellipse shape and angle - if requested
      WriteEllipseAngle(*pRecord, theCluster, theData);
      WriteEllipseShape(*pRecord, theCluster, theData);
    }

    if (gParameters.GetNumDataStreams() == 1) {
      pRecord->GetFieldValue(GetFieldNumber(OBSERVED_FIELD)).AsDouble() = theCluster.GetCaseCount(0);
      pRecord->GetFieldValue(GetFieldNumber(EXPECTED_FIELD)).AsDouble() =
                                                           theData.GetMeasureAdjustment(0) * theCluster.GetMeasure(0);
      pRecord->GetFieldValue(GetFieldNumber(OBS_DIV_EXP_FIELD)).AsDouble() =
                                                           theCluster.GetRelativeRisk(theData.GetMeasureAdjustment(0));
    }
    else {
      for (i=0; i < gParameters.GetNumDataStreams(); ++i) {
        sBuffer.printf("%s%i", STREAM_OBSERVED_FIELD, i + 1);
        pRecord->GetFieldValue(GetFieldNumber(sBuffer.GetCString())).AsDouble() = theCluster.GetCaseCount(i);
        sBuffer.printf("%s%i", STREAM_EXPECTED_FIELD, i + 1);
        pRecord->GetFieldValue(GetFieldNumber(sBuffer.GetCString())).AsDouble() =
                                                           theData.GetMeasureAdjustment(i) * theCluster.GetMeasure(i);
        sBuffer.printf("%s%i", STREAM_OBS_DIV_EXP_FIELD, i + 1);
        pRecord->GetFieldValue(GetFieldNumber(sBuffer.GetCString())).AsDouble() =
                                                           theCluster.GetRelativeRisk(theData.GetMeasureAdjustment(i), i);
      }
    }
    if (gParameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION)
      pRecord->GetFieldValue(GetFieldNumber(TST_STAT_FIELD)).AsDouble() = theCluster.m_nRatio;
    else {
      pRecord->GetFieldValue(GetFieldNumber(LOG_LIKL_RATIO_FIELD)).AsDouble() = theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty();
      if (gParameters.GetNonCompactnessPenalty())
        pRecord->GetFieldValue(GetFieldNumber(TST_STAT_FIELD)).AsDouble() = theCluster.m_nRatio;
    }
    if (iNumSimsCompleted > 98)
      pRecord->GetFieldValue(GetFieldNumber(P_VALUE_FLD)).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
    pRecord->GetFieldValue(GetFieldNumber(START_DATE_FLD)).AsZdString() = theCluster.GetStartDate(sBuffer, theData);
    pRecord->GetFieldValue(GetFieldNumber(END_DATE_FLD)).AsZdString() = theCluster.GetEndDate(sBuffer, theData);
   //add to collection records
   BaseOutputStorageClass::AddRecord(pRecord);
  }
  catch (ZdException &x) {
    delete pRecord;
    x.AddCallpath("RecordClusterData()","LogLikelihoodData");
    throw;
  }
}

// pre : none
// post : sets the values for long, lat, sAdditCoords, and radius
void stsClusterData::WriteCoordinates(OutputRecord& Record, const CCluster& thisCluster, const CSaTScanData& DataHub) {
  int           i;
  double      * pCoords=0, * pCoords2=0, dRadius, EARTH_RADIUS = 6367/*radius of earth in km*/;
  float         fLatitude, fLongitude;
  unsigned int  iFirstCoordIndex, iSecondCoordIndex, iThCoordIndex;
  ZdString      sBuffer;
  tract_t       tTractIndex;

   try {
     iFirstCoordIndex = GetFieldNumber(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LAT_FIELD : COORD_X_FIELD);
     iSecondCoordIndex = GetFieldNumber(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LONG_FIELD : COORD_Y_FIELD);

     if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER) {
       Record.SetFieldIsBlank(iFirstCoordIndex, true);
       Record.SetFieldIsBlank(iSecondCoordIndex, true);
       if (gParameters.GetCoordinatesType() == CARTESIAN) {
         for (i=2; i < gParameters.GetDimensionsOfData(); ++i) {
            sBuffer << ZdString::reset << COORD_Z_FIELD << i - 1;
            Record.SetFieldIsBlank(GetFieldNumber(sBuffer), true);
         }
       }
       Record.SetFieldIsBlank(GetFieldNumber(RADIUS_FIELD), true);
     }
     else {
       DataHub.GetGInfo()->giGetCoords(thisCluster.GetCentroidIndex(), &pCoords);
       tTractIndex = DataHub.GetNeighbor(thisCluster.GetEllipseOffset(),
                                         thisCluster.GetCentroidIndex(),
                                         thisCluster.GetNumTractsInnerCircle());
       DataHub.GetTInfo()->tiGetCoords(tTractIndex, &pCoords2);
       switch (gParameters.GetCoordinatesType()) {
         case CARTESIAN : Record.GetFieldValue(iFirstCoordIndex).AsDouble() =  pCoords[0];
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() =  pCoords[1];
                          for (i=2; i < gParameters.GetDimensionsOfData(); ++i) {
                             sBuffer << ZdString::reset << COORD_Z_FIELD << i - 1;
                             iThCoordIndex = GetFieldNumber(sBuffer);
                             Record.GetFieldValue(iThCoordIndex).AsDouble() = pCoords[i];
                          }
                          //to mimic behavior in CCluster reporting, cast down to float
                          dRadius = (float)(sqrt((DataHub.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2)));
                          Record.GetFieldValue(GetFieldNumber(RADIUS_FIELD)).AsDouble() = dRadius;
                          break;
         case LATLON    : ConvertToLatLong(&fLatitude, &fLongitude, pCoords);
                          Record.GetFieldValue(iFirstCoordIndex).AsDouble() = fLatitude;
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() = fLongitude;
                          dRadius = 2 * EARTH_RADIUS * asin(sqrt(DataHub.GetTInfo()->tiGetDistanceSq(pCoords, pCoords2))/(2 * EARTH_RADIUS));
                          Record.GetFieldValue(GetFieldNumber(RADIUS_FIELD)).AsDouble() = dRadius;
                          break;
         default : ZdGenerateException("Unknown coordinate type '%d'.","SetCoordinates()", gParameters.GetCoordinatesType());
       }
       free(pCoords);
       free(pCoords2);
     }
   }
   catch (ZdException &x) {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("WriteCoordinates()", "stsClusterData");
      throw;
   }
}

/** write cluster's elliptical angle to record */
void stsClusterData::WriteEllipseAngle(OutputRecord& Record, const CCluster& thisCluster, const CSaTScanData& DataHub) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(GetFieldNumber(E_ANGLE_FIELD), true);
  else if (thisCluster.GetEllipseOffset() == 0)
    Record.GetFieldValue(GetFieldNumber(E_ANGLE_FIELD)).AsDouble() = 0.0;
  else
    Record.GetFieldValue(GetFieldNumber(E_ANGLE_FIELD)).AsDouble() = thisCluster.ConvertAngleToDegrees(DataHub.GetEllipseAngle(thisCluster.GetEllipseOffset()));
}

/** write cluster's elliptical shape to record */
void stsClusterData::WriteEllipseShape(OutputRecord& Record, const CCluster& thisCluster, const CSaTScanData& DataHub) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(GetFieldNumber(E_SHAPE_FIELD), true);
  else if (thisCluster.GetEllipseOffset() == 0)
    Record.GetFieldValue(GetFieldNumber(E_SHAPE_FIELD)).AsDouble() = 1.0;
  else
    Record.GetFieldValue(GetFieldNumber(E_SHAPE_FIELD)).AsDouble() = DataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
}

// sets up the vector of field structs so that the ZdField Vector can be created
// pre: none
// post : returns through reference a vector of ZdFields to determine the structure of the data
void stsClusterData::SetupFields() {
  unsigned short uwOffset = 0;    // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
  unsigned int   i;
  ZdString       sBuffer;

  try {
    CreateField(gvFields, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(gvFields, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    CreateField(gvFields, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LAT_FIELD : COORD_X_FIELD, ZD_NUMBER_FLD, 12, 4, uwOffset);
    CreateField(gvFields, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LONG_FIELD : COORD_Y_FIELD, ZD_NUMBER_FLD, 12, 4, uwOffset);
    // Only Cartesian coordinates have more than two dimensions. Lat/Long is consistently assigned to 3 dims
    // throughout the program eventhough the third dim is never used. When you print the third it is blank
    // and meaningless and thus does not need to be included here. - AJV 10/2/2002
    if (gParameters.GetCoordinatesType() == CARTESIAN && gParameters.GetDimensionsOfData() > 2) {
      for (i=3; i <= (unsigned int)gParameters.GetDimensionsOfData(); ++i) {
         sBuffer.printf("%s%i", COORD_Z_FIELD, i - 2);
         CreateField(gvFields, sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 4, uwOffset);
      }
    }
    CreateField(gvFields, RADIUS_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
    if (gParameters.GetNumRequestedEllipses()) {
      CreateField(gvFields, E_ANGLE_FIELD, ZD_NUMBER_FLD, 16, 0, uwOffset);
      CreateField(gvFields, E_SHAPE_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    }
    CreateField(gvFields, NUM_LOCATIONS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    if (gParameters.GetNumDataStreams() == 1) {
      CreateField(gvFields, OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
      CreateField(gvFields, EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      CreateField(gvFields, OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
    }
    else {
      for (i=1; i <= gParameters.GetNumDataStreams(); ++i) {
        sBuffer.printf("%s%i", STREAM_OBSERVED_FIELD, i);
        CreateField(gvFields, sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 0, uwOffset);
        sBuffer.printf("%s%i", STREAM_EXPECTED_FIELD, i);
        CreateField(gvFields, sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
        sBuffer.printf("%s%i", STREAM_OBS_DIV_EXP_FIELD, i);
        CreateField(gvFields, sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
      }
    }
    if (gParameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION)
      CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
    else {
      CreateField(gvFields, LOG_LIKL_RATIO_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
      if (gParameters.GetNumRequestedEllipses() && gParameters.GetNonCompactnessPenalty())
        CreateField(gvFields, TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
    }
    if (!gbExcludePValueField)
      CreateField(gvFields, P_VALUE_FLD, ZD_NUMBER_FLD, 12, 5, uwOffset);
    CreateField(gvFields, START_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
    CreateField(gvFields, END_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupFields()","stsClusterData");
    throw;
  }
}
