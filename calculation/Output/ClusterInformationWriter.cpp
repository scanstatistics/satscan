//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterInformationWriter.h"
#include "cluster.h"

const char * ClusterInformationWriter::CLUSTER_FILE_EXT	          = ".col";
const char * ClusterInformationWriter::START_DATE_FLD	          = "START_DATE";
const char * ClusterInformationWriter::END_DATE_FLD	          = "END_DATE";
const char * ClusterInformationWriter::RADIUS_FIELD	          = "RADIUS";
const char * ClusterInformationWriter::E_ANGLE_FIELD              = "E_ANGLE";
const char * ClusterInformationWriter::E_SHAPE_FIELD              = "E_SHAPE";
const char * ClusterInformationWriter::NUM_LOCATIONS_FIELD        = "NUM_LOC";
const char * ClusterInformationWriter::COORD_LAT_FIELD	          = "LATITUDE";
const char * ClusterInformationWriter::COORD_LONG_FIELD	          = "LONGITUDE";
const char * ClusterInformationWriter::COORD_X_FIELD	          = "X";
const char * ClusterInformationWriter::COORD_Y_FIELD              = "Y";
const char * ClusterInformationWriter::COORD_Z_FIELD              = "Z";
const char * ClusterInformationWriter::OBS_FIELD_PART  	          = "OBS";
const char * ClusterInformationWriter::EXP_FIELD_PART             = "EXP";
const char * ClusterInformationWriter::OBS_DIV_EXP_FIELD          = "ODE";
const char * ClusterInformationWriter::CATEGORY_FIELD_PART        = "_CAT";
const char * ClusterInformationWriter::SET_FIELD_PART             = "_DS";
const char * ClusterInformationWriter::SET_CATEGORY_FIELD_PART    = "C";

/** class constructor */
ClusterInformationWriter::ClusterInformationWriter(const CSaTScanData& DataHub, bool bExcludePValueField)
               :AbstractDataFileWriter(DataHub.GetParameters()), gDataHub(DataHub), gbExcludePValueField(bExcludePValueField) {
  try {
    DefineFields();
    if (gParameters.GetOutputClusterLevelAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, CLUSTER_FILE_EXT);
    if (gParameters.GetOutputClusterLevelDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, CLUSTER_FILE_EXT);
  }
  catch (ZdException &x) {
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("constructor","ClusterInformationWriter");
    throw;
  }
}

/** class destructor */
ClusterInformationWriter::~ClusterInformationWriter() {}

/** Defines fields of output file. */
void ClusterInformationWriter::DefineFields() {
  unsigned short uwOffset = 0;    // this is altered by the create new field function, so this must be here as is-AJV 9/30/2002
  unsigned int   i;
  ZdString       sBuffer;

  try {
    CreateField(LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    CreateField((gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LAT_FIELD : COORD_X_FIELD, ZD_NUMBER_FLD, 12, 4, uwOffset);
    CreateField((gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LONG_FIELD : COORD_Y_FIELD, ZD_NUMBER_FLD, 12, 4, uwOffset);
    // Only Cartesian coordinates have more than two dimensions. Lat/Long is consistently assigned to 3 dims
    // throughout the program eventhough the third dim is never used. When you print the third it is blank
    // and meaningless and thus does not need to be included here. - AJV 10/2/2002
    if (gParameters.GetCoordinatesType() == CARTESIAN && gParameters.GetDimensionsOfData() > 2) {
      for (i=3; i <= (unsigned int)gParameters.GetDimensionsOfData(); ++i) {
         sBuffer.printf("%s%i", COORD_Z_FIELD, i - 2);
         CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 4, uwOffset);
      }
    }
    CreateField(RADIUS_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
    if (gParameters.GetNumRequestedEllipses()) {
      CreateField(E_ANGLE_FIELD, ZD_NUMBER_FLD, 16, 0, uwOffset);
      CreateField(E_SHAPE_FIELD, ZD_NUMBER_FLD, 12, 3, uwOffset);
    }
    CreateField(NUM_LOCATIONS_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
    if (gParameters.GetNumDataSets() == 1) {
      if (gParameters.GetProbabilityModelType() == ORDINAL) {
        const PopulationData& Population = gDataHub.GetDataSetHandler().GetDataSet(0).GetPopulationData();
        for (size_t t=1; t <= Population.GetNumOrdinalCategories(); ++t) {
          sBuffer.printf("%s%s%i", OBS_FIELD_PART, CATEGORY_FIELD_PART, t);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 0, uwOffset);
          sBuffer.printf("%s%s%i", EXP_FIELD_PART, CATEGORY_FIELD_PART, t);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
          sBuffer.printf("%s%s%i", OBS_DIV_EXP_FIELD, CATEGORY_FIELD_PART, t);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
        }  
      }
      else {
        CreateField(OBSERVED_FIELD, ZD_NUMBER_FLD, 12, 0, uwOffset);
        CreateField(EXPECTED_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
        CreateField(OBS_DIV_EXP_FIELD, ZD_NUMBER_FLD, 12, 2, uwOffset);
      }  
    }
    else {
      if (gParameters.GetProbabilityModelType() == ORDINAL) {
        for (i=0; i < gDataHub.GetDataSetHandler().GetNumDataSets(); ++i) {
            const PopulationData& Population = gDataHub.GetDataSetHandler().GetDataSet(i).GetPopulationData();
            for (size_t t=1; t <= Population.GetNumOrdinalCategories(); ++t) {
               sBuffer.printf("%s%s%i%s%i", OBS_FIELD_PART, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART, t);
               CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 0, uwOffset);
               sBuffer.printf("%s%s%i%s%i", EXP_FIELD_PART, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART ,t);
               CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
               sBuffer.printf("%s%s%i%s%i", OBS_DIV_EXP_FIELD, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART, t);
               CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
            }   
        }
      }
      else {
        for (i=1; i <= gParameters.GetNumDataSets(); ++i) {
          sBuffer.printf("%s%s%i", OBS_FIELD_PART, SET_FIELD_PART, i);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 0, uwOffset);
          sBuffer.printf("%s%s%i", EXP_FIELD_PART, SET_FIELD_PART, i);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
          sBuffer.printf("%s%s%i", OBS_DIV_EXP_FIELD, SET_FIELD_PART, i);
          CreateField(sBuffer.GetCString(), ZD_NUMBER_FLD, 12, 2, uwOffset);
        }
      }
    }
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      CreateField(TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
    else {
      CreateField(LOG_LIKL_RATIO_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
      if (gParameters.GetNumRequestedEllipses() && gParameters.GetNonCompactnessPenalty())
        CreateField(TST_STAT_FIELD, ZD_NUMBER_FLD, 11, 6, uwOffset);
    }
    if (!gbExcludePValueField)
      CreateField(P_VALUE_FLD, ZD_NUMBER_FLD, 12, 5, uwOffset);
    CreateField(START_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
    CreateField(END_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","ClusterInformationWriter");
    throw;
  }
}

// formats the string for the Area ID
// pre: none
// post: sTempvalue will contain the legible area id
ZdString& ClusterInformationWriter::GetAreaID(ZdString& sAreaId, const CCluster& thisCluster) const {
  std::string   sBuffer;
  tract_t       tTractIndex;

  try {
    if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
      sAreaId = "All";
    else {
      tTractIndex = gDataHub.GetNeighbor(thisCluster.GetEllipseOffset(), thisCluster.GetCentroidIndex(), 1);
      sAreaId = gDataHub.GetTInfo()->tiGetTid(tTractIndex, sBuffer);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetAreaID","ClusterInformationWriter");
    throw;
  }
  return sAreaId;
}

/** Records the required data to be stored in the cluster output file, stores
    the values in the global vector of cluster records.
    pre: pCluster has been initialized with calculated data
    post: function will record the appropriate data into the cluster record   */
void ClusterInformationWriter::Write(const CCluster& theCluster, int iClusterNumber, unsigned int iNumSimsCompleted) {
  ZdString       sBuffer;
  unsigned int   i;
  RecordBuffer   Record(vFieldDefinitions);

  try {
    Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = GetAreaID(sBuffer, theCluster);
    Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
    WriteCoordinates(Record, theCluster);
    Record.GetFieldValue(NUM_LOCATIONS_FIELD).AsDouble() =
        theCluster.GetClusterType() == PURELYTEMPORALCLUSTER ? gDataHub.GetNumTracts() : theCluster.GetNumTractsInnerCircle();
    if (gParameters.GetNumRequestedEllipses()) { // ellipse shape and angle - if requested
      WriteEllipseAngle(Record, theCluster);
      WriteEllipseShape(Record, theCluster);
    }

    if (gParameters.GetProbabilityModelType() == ORDINAL)
      WriteCountDataAsOrdinal(Record, theCluster);
    else
      WriteCountDataStandard(Record, theCluster);

    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    else {
      Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty();
      if (gParameters.GetNonCompactnessPenalty())
        Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    }
    if (iNumSimsCompleted > 98)
      Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
    Record.GetFieldValue(START_DATE_FLD).AsZdString() = theCluster.GetStartDate(sBuffer, gDataHub);
    Record.GetFieldValue(END_DATE_FLD).AsZdString() = theCluster.GetEndDate(sBuffer, gDataHub);
    
    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","ClusterInformationWriter");
    throw;
  }
}

// pre : none
// post : sets the values for long, lat, sAdditCoords, and radius
void ClusterInformationWriter::WriteCoordinates(RecordBuffer& Record, const CCluster& thisCluster) {
  int           i;
  double      * pCoords=0, * pCoords2=0;
  float         fLatitude, fLongitude, fRadius;
  unsigned int  iFirstCoordIndex, iSecondCoordIndex, iThCoordIndex;
  ZdString      sBuffer;
  tract_t       tTractIndex;

   try {
     iFirstCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LAT_FIELD : COORD_X_FIELD);
     iSecondCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LONG_FIELD : COORD_Y_FIELD);

     if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER) {
       Record.SetFieldIsBlank(iFirstCoordIndex, true);
       Record.SetFieldIsBlank(iSecondCoordIndex, true);
       if (gParameters.GetCoordinatesType() == CARTESIAN) {
         for (i=2; i < gParameters.GetDimensionsOfData(); ++i) {
            sBuffer << ZdString::reset << COORD_Z_FIELD << i - 1;
            Record.SetFieldIsBlank(sBuffer, true);
         }
       }
       Record.SetFieldIsBlank(RADIUS_FIELD, true);
     }
     else {
       gDataHub.GetGInfo()->giGetCoords(thisCluster.GetCentroidIndex(), &pCoords);
       tTractIndex = gDataHub.GetNeighbor(thisCluster.GetEllipseOffset(),
                                          thisCluster.GetCentroidIndex(),
                                          thisCluster.GetNumTractsInnerCircle());
       gDataHub.GetTInfo()->tiGetCoords(tTractIndex, &pCoords2);
       switch (gParameters.GetCoordinatesType()) {
         case CARTESIAN : Record.GetFieldValue(iFirstCoordIndex).AsDouble() =  pCoords[0];
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() =  pCoords[1];
                          for (i=2; i < gParameters.GetDimensionsOfData(); ++i) {
                             sBuffer << ZdString::reset << COORD_Z_FIELD << i - 1;
                             //iThCoordIndex = GetFieldNumber(sBuffer);
                             Record.GetFieldValue(sBuffer).AsDouble() = pCoords[i];
                          }
                          break;
         case LATLON    : ConvertToLatLong(&fLatitude, &fLongitude, pCoords);
                          Record.GetFieldValue(iFirstCoordIndex).AsDouble() = fLatitude;
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() = fLongitude;
                          break;
         default : ZdGenerateException("Unknown coordinate type '%d'.","SetCoordinates()", gParameters.GetCoordinatesType());
       }
       fRadius = (float)(sqrt((gDataHub.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2)));
       Record.GetFieldValue(RADIUS_FIELD).AsDouble() = fRadius;
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

/** Write obvserved, expected and  observed/expected to record for ordinal data.*/
void ClusterInformationWriter::WriteCountDataAsOrdinal(RecordBuffer& Record, const CCluster& theCluster) const {
  ZdString      sBuffer;

  if (gParameters.GetNumDataSets() == 1) {
    const PopulationData& Population = gDataHub.GetDataSetHandler().GetDataSet(0).GetPopulationData();
    for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
       sBuffer.printf("%s%s%i", OBS_FIELD_PART, CATEGORY_FIELD_PART, t + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetObservedCountOrdinal(0, t);
       sBuffer.printf("%s%s%i", EXP_FIELD_PART, CATEGORY_FIELD_PART, t + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetExpectedCountOrdinal(gDataHub, 0, t);
       sBuffer.printf("%s%s%i", OBS_DIV_EXP_FIELD, CATEGORY_FIELD_PART, t + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetObservedDivExpectedOrdinal(gDataHub, 0, t);
    }
  }
  else {
    for (size_t i=0; i < gDataHub.GetDataSetHandler().GetNumDataSets(); ++i) {
       const PopulationData& Population = gDataHub.GetDataSetHandler().GetDataSet(i).GetPopulationData();
       for (size_t t=0; t < Population.GetNumOrdinalCategories(); ++t) {
          sBuffer.printf("%s%s%i%s%i", OBS_FIELD_PART, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART, t + 1);
          Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetObservedCountOrdinal(i, t);
          sBuffer.printf("%s%s%i%s%i", EXP_FIELD_PART, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART, t + 1);
          Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetExpectedCountOrdinal(gDataHub, i, t);
          sBuffer.printf("%s%s%i%s%i", OBS_DIV_EXP_FIELD, SET_FIELD_PART, i + 1, SET_CATEGORY_FIELD_PART, t + 1);
          Record.GetFieldValue(sBuffer.GetCString()).AsDouble() =  theCluster.GetObservedDivExpectedOrdinal(gDataHub, i, t);
       }
    }
  }
}

/** Write obvserved, expected and  observed/expected to record.*/
void ClusterInformationWriter::WriteCountDataStandard(RecordBuffer& Record, const CCluster& theCluster) const {
  ZdString      sBuffer;

  if (gParameters.GetNumDataSets() == 1) {
    Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCount();
    Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCount(gDataHub);
    Record.GetFieldValue(OBS_DIV_EXP_FIELD).AsDouble() = theCluster.GetObservedDivExpected(gDataHub);
  }
  else {
    for (size_t i=0; i < gParameters.GetNumDataSets(); ++i) {
       sBuffer.printf("%s%s%i", OBS_FIELD_PART, SET_FIELD_PART, i + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetObservedCount(i);
       sBuffer.printf("%s%s%i", EXP_FIELD_PART, SET_FIELD_PART, i + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetExpectedCount(gDataHub, i);
       sBuffer.printf("%s%s%i", OBS_FIELD_PART, SET_FIELD_PART, i + 1);
       Record.GetFieldValue(sBuffer.GetCString()).AsDouble() = theCluster.GetObservedDivExpected(gDataHub, i);
    }
  }
}

/** write cluster's elliptical angle to record */
void ClusterInformationWriter::WriteEllipseAngle(RecordBuffer& Record, const CCluster& thisCluster) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(E_ANGLE_FIELD, true);
  else if (thisCluster.GetEllipseOffset() == 0)
    Record.GetFieldValue(E_ANGLE_FIELD).AsDouble() = 0.0;
  else
    Record.GetFieldValue(E_ANGLE_FIELD).AsDouble() = thisCluster.ConvertAngleToDegrees(gDataHub.GetEllipseAngle(thisCluster.GetEllipseOffset()));
}

/** write cluster's elliptical shape to record */
void ClusterInformationWriter::WriteEllipseShape(RecordBuffer& Record, const CCluster& thisCluster) const {
  if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
    Record.SetFieldIsBlank(E_SHAPE_FIELD, true);
  else if (thisCluster.GetEllipseOffset() == 0)
    Record.GetFieldValue(E_SHAPE_FIELD).AsDouble() = 1.0;
  else
    Record.GetFieldValue(E_SHAPE_FIELD).AsDouble() = gDataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
}

