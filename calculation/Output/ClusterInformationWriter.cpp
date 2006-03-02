//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterInformationWriter.h"
#include "cluster.h"
#include "OrdinalLikelihoodCalculation.h"
#include "CategoricalClusterData.h"
#include "AbstractAnalysis.h"

const char * ClusterInformationWriter::CLUSTER_FILE_EXT	          = ".col";
const char * ClusterInformationWriter::CLUSTERCASE_FILE_EXT	  = ".cci";
const char * ClusterInformationWriter::START_DATE_FLD	          = "START_DATE";
const char * ClusterInformationWriter::END_DATE_FLD	          = "END_DATE";
const char * ClusterInformationWriter::RADIUS_FIELD	          = "RADIUS";
const char * ClusterInformationWriter::E_MINOR_FIELD              = "E_MINOR";
const char * ClusterInformationWriter::E_MAJOR_FIELD              = "E_MAJOR";
const char * ClusterInformationWriter::E_ANGLE_FIELD              = "E_ANGLE";
const char * ClusterInformationWriter::E_SHAPE_FIELD              = "E_SHAPE";
const char * ClusterInformationWriter::NUM_LOCATIONS_FIELD        = "NUMBER_LOC";
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
ClusterInformationWriter::ClusterInformationWriter(const CSaTScanData& DataHub, bool bAppend)
               :AbstractDataFileWriter(DataHub.GetParameters()), gDataHub(DataHub),
                gpASCIIFileDataWriter(0), gpDBaseFileDataWriter(0) {
  try {
    if (gParameters.GetOutputClusterLevelFiles())
      DefineClusterInformationFields();
    if (gParameters.GetOutputClusterCaseFiles())
      DefineClusterCaseInformationFields();
    if (gParameters.GetOutputClusterLevelAscii())
      gpASCIIFileWriter = new ASCIIDataFileWriter(gParameters, CLUSTER_FILE_EXT, bAppend);
    if (gParameters.GetOutputClusterCaseAscii())
      gpASCIIFileDataWriter = new ASCIIDataFileWriter(gParameters, CLUSTERCASE_FILE_EXT, bAppend);
    if (gParameters.GetOutputClusterLevelDBase())
      gpDBaseFileWriter = new DBaseDataFileWriter(gParameters, vFieldDefinitions, CLUSTER_FILE_EXT, bAppend);
    if (gParameters.GetOutputClusterCaseDBase())
      gpDBaseFileDataWriter = new DBaseDataFileWriter(gParameters, vDataFieldDefinitions, CLUSTERCASE_FILE_EXT, bAppend);
  }
  catch (ZdException &x) {
    delete gpASCIIFileWriter;
    delete gpDBaseFileWriter;
    x.AddCallpath("constructor","ClusterInformationWriter");
    throw;
  }
}

/** class destructor */
ClusterInformationWriter::~ClusterInformationWriter() {
  try {
    delete gpASCIIFileDataWriter;
    delete gpDBaseFileDataWriter;
  }
  catch (...){}
}

/** Defines fields of cluster information output file. */
void ClusterInformationWriter::DefineClusterInformationFields() {
  unsigned short uwOffset=0;
  unsigned int   i;
  ZdString       sBuffer;

  try {
    //define fields of data file that describes cluster properties
    CreateField(vFieldDefinitions, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    CreateField(vFieldDefinitions, LOC_ID_FIELD, ZD_ALPHA_FLD, 30, 0, uwOffset);
    CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LAT_FIELD : COORD_X_FIELD, ZD_NUMBER_FLD, 19, 4, uwOffset);
    CreateField(vFieldDefinitions, (gParameters.GetCoordinatesType() != CARTESIAN) ? COORD_LONG_FIELD : COORD_Y_FIELD, ZD_NUMBER_FLD, 19, 4, uwOffset);
    //only Cartesian coordinates can have more than two dimensions
    if (gParameters.GetCoordinatesType() == CARTESIAN && gParameters.GetDimensionsOfData() > 2)
      for (i=3; i <= (unsigned int)gParameters.GetDimensionsOfData(); ++i) {
         sBuffer.printf("%s%i", COORD_Z_FIELD, i - 2);
         CreateField(vFieldDefinitions, sBuffer.GetCString(), ZD_NUMBER_FLD, 19, 4, uwOffset);
      }
    if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
      CreateField(vFieldDefinitions, E_MINOR_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(vFieldDefinitions, E_MAJOR_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(vFieldDefinitions, E_ANGLE_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(vFieldDefinitions, E_SHAPE_FIELD, ZD_NUMBER_FLD, 19, 3, uwOffset);
    }
    else
      CreateField(vFieldDefinitions, RADIUS_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
    CreateField(vFieldDefinitions, START_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
    CreateField(vFieldDefinitions, END_DATE_FLD, ZD_ALPHA_FLD, 16, 0, uwOffset);
    CreateField(vFieldDefinitions, NUM_LOCATIONS_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      CreateField(vFieldDefinitions, TST_STAT_FIELD, ZD_NUMBER_FLD, 19, 6, uwOffset);
    else {
      CreateField(vFieldDefinitions, LOG_LIKL_RATIO_FIELD, ZD_NUMBER_FLD, 19, 6, uwOffset);
      if (gParameters.GetSpatialWindowType() == ELLIPTIC)
        CreateField(vFieldDefinitions, TST_STAT_FIELD, ZD_NUMBER_FLD, 19, 6, uwOffset);
    }
    CreateField(vFieldDefinitions, P_VALUE_FLD, ZD_NUMBER_FLD, 19, 5, uwOffset);

    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      CreateField(vFieldDefinitions, OBSERVED_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
      CreateField(vFieldDefinitions, EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
      CreateField(vFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
      if (gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI)
        CreateField(vFieldDefinitions, RELATIVE_RISK_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineClusterInformationFields()","ClusterInformationWriter");
    throw;
  }
}

/** Defines fields of ocluster case information utput file. */
void ClusterInformationWriter::DefineClusterCaseInformationFields() {
  unsigned short uwOffset=0;
  unsigned int   i;
  ZdString       sBuffer;

  try {
    //define fields for secondary cluster data file
    CreateField(vDataFieldDefinitions, CLUST_NUM_FIELD, ZD_NUMBER_FLD, 5, 0, uwOffset);
    CreateField(vDataFieldDefinitions, DATASET_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    CreateField(vDataFieldDefinitions, CATEGORY_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    CreateField(vDataFieldDefinitions, OBSERVED_FIELD, ZD_NUMBER_FLD, 19, 0, uwOffset);
    CreateField(vDataFieldDefinitions, EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
    CreateField(vDataFieldDefinitions, OBSERVED_DIV_EXPECTED_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
    //Relative risk field only reported for these probability models
    //   - relative risk calculation not defined for STP, Exponential, another to be model
    if (gParameters.GetProbabilityModelType() == POISSON  ||
        gParameters.GetProbabilityModelType() == BERNOULLI ||
        gParameters.GetProbabilityModelType() == ORDINAL)
      CreateField(vDataFieldDefinitions, RELATIVE_RISK_FIELD, ZD_NUMBER_FLD, 19, 2, uwOffset);
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineFields()","ClusterInformationWriter");
    throw;
  }
}

/** formats string for the Area ID */
ZdString& ClusterInformationWriter::GetAreaID(ZdString& sAreaId, const CCluster& thisCluster) const {
  std::string   sBuffer;

  try {
    if (thisCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
      sAreaId = "All";
    else
      sAreaId = gDataHub.GetTInfo()->tiGetTid(thisCluster.GetMostCentralLocationIndex(), sBuffer);
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
  try {
    if (gParameters.GetOutputClusterLevelFiles())
      WriteClusterInformation(theCluster, iClusterNumber, iNumSimsCompleted);
    if (gParameters.GetOutputClusterCaseFiles())
      WriteClusterCaseInformation(theCluster, iClusterNumber);
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","ClusterInformationWriter");
    throw;
  }
}

void ClusterInformationWriter::WriteClusterCaseInformation(const CCluster& theCluster, int iClusterNumber) {
  ZdString          sBuffer;
  RecordBuffer      Record(vFieldDefinitions);
  double            dRelativeRisk;

  try {
    //now write to secondary cluster information file
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      WriteCountOrdinalData(theCluster, iClusterNumber);
    else
      WriteCountData(theCluster, iClusterNumber);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteClusterCaseInformation()","ClusterInformationWriter");
    throw;
  }
}

void ClusterInformationWriter::WriteClusterInformation(const CCluster& theCluster, int iClusterNumber, unsigned int iNumSimsCompleted) {
  ZdString          sBuffer;
  RecordBuffer      Record(vFieldDefinitions);
  double            dRelativeRisk;  

  try {
    Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
    Record.GetFieldValue(LOC_ID_FIELD).AsZdString() = GetAreaID(sBuffer, theCluster);
    WriteCoordinates(Record, theCluster);
    Record.GetFieldValue(NUM_LOCATIONS_FIELD).AsDouble() =
        theCluster.GetClusterType() == PURELYTEMPORALCLUSTER ? gDataHub.GetNumTracts() : theCluster.GetNumTractsInnerCircle();
    if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
      WriteEllipseAngle(Record, theCluster);
      WriteEllipseShape(Record, theCluster);
    }
    if (gParameters.GetProbabilityModelType() == SPACETIMEPERMUTATION)
      Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    else {
      Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = theCluster.m_nRatio/theCluster.GetNonCompactnessPenalty();
      if (gParameters.GetSpatialWindowType() == ELLIPTIC)
        Record.GetFieldValue(TST_STAT_FIELD).AsDouble() = theCluster.m_nRatio;
    }
    if (iNumSimsCompleted >= 99)
      Record.GetFieldValue(P_VALUE_FLD).AsDouble() = theCluster.GetPValue(iNumSimsCompleted);
    Record.GetFieldValue(START_DATE_FLD).AsZdString() = theCluster.GetStartDate(sBuffer, gDataHub);
    Record.GetFieldValue(END_DATE_FLD).AsZdString() = theCluster.GetEndDate(sBuffer, gDataHub);
    if (gParameters.GetNumDataSets() == 1 && gParameters.GetProbabilityModelType() != ORDINAL) {
      Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCount();
      Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCount(gDataHub);
      Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = theCluster.GetObservedDivExpected(gDataHub);
      //either suppress printing this field because we didn't define it (not Poisson or Bernoulli) or
      //because the relative risk could not be calculated
      if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
          (dRelativeRisk = theCluster.GetRelativeRisk(gDataHub)) != -1)
          Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = dRelativeRisk;
    }
    if (gpASCIIFileWriter) gpASCIIFileWriter->WriteRecord(Record);
    if (gpDBaseFileWriter) gpDBaseFileWriter->WriteRecord(Record);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteClusterInformation()","ClusterInformationWriter");
    throw;
  }
}

/** Writes cluster coordinates to passed record buffer, with consideration for whether coordinate
    system is lat/long or Cartesian. Fields left blank for purely temporal clusters. */
void ClusterInformationWriter::WriteCoordinates(RecordBuffer& Record, const CCluster& thisCluster) {
  double                        dRadius;
  std::vector<double>           vCoordinates;
  std::pair<double, double>     prLatitudeLongitude;
  float                         fRadius;
  unsigned int                  iFirstCoordIndex, iSecondCoordIndex;
  ZdString                      sBuffer;
  tract_t                       tTractIndex;

   try {
     if (thisCluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
       iFirstCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LAT_FIELD : COORD_X_FIELD);
       iSecondCoordIndex = Record.GetFieldIndex(gParameters.GetCoordinatesType() != CARTESIAN ? COORD_LONG_FIELD : COORD_Y_FIELD);
       gDataHub.GetGInfo()->giRetrieveCoords(thisCluster.GetCentroidIndex(), vCoordinates);
       tTractIndex = gDataHub.GetNeighbor(thisCluster.GetEllipseOffset(), thisCluster.GetCentroidIndex(),
                                          thisCluster.GetNumTractsInnerCircle(), thisCluster.GetCartesianRadius());
       switch (gParameters.GetCoordinatesType()) {
         case CARTESIAN : Record.GetFieldValue(iFirstCoordIndex).AsDouble() =  vCoordinates[0];
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() =  vCoordinates[1];
                          for (int i=2; i < gParameters.GetDimensionsOfData(); ++i) {
                             sBuffer << ZdString::reset << COORD_Z_FIELD << (i - 1);
                             Record.GetFieldValue(sBuffer).AsDouble() = vCoordinates[i];
                          }
                          if (gParameters.GetSpatialWindowType() == ELLIPTIC) {
                            //to mimic behavior in CCluster reporting, cast down to float
                            fRadius = static_cast<float>(thisCluster.GetCartesianRadius());
                            Record.GetFieldValue(E_MINOR_FIELD).AsDouble() = fRadius;
                            Record.GetFieldValue(E_MAJOR_FIELD).AsDouble() = fRadius * gDataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
                          }
                          else {
                            //to mimic behavior in CCluster reporting, cast down to float
                            fRadius = static_cast<float>(thisCluster.GetCartesianRadius());
                            Record.GetFieldValue(RADIUS_FIELD).AsDouble() = fRadius;
                          }
                          break;
         case LATLON    : prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                          Record.GetFieldValue(iFirstCoordIndex).AsDouble() = prLatitudeLongitude.first;
                          Record.GetFieldValue(iSecondCoordIndex).AsDouble() = prLatitudeLongitude.second;
                          dRadius = 2 * EARTH_RADIUS_km * asin(thisCluster.GetCartesianRadius()/(2 * EARTH_RADIUS_km));
                          Record.GetFieldValue(RADIUS_FIELD).AsDouble() = dRadius;
                          break;
         default : ZdGenerateException("Unknown coordinate type '%d'.","WriteCoordinates()", gParameters.GetCoordinatesType());
       }
     }
   }
   catch (ZdException &x) {
     x.AddCallpath("WriteCoordinates()", "stsClusterData");
     throw;
   }
}

/** Writes obvserved, expected and  observed/expected to file(s).*/
void ClusterInformationWriter::WriteCountData(const CCluster& theCluster, int iClusterNumber) const {
  double                                        dRelativeRisk;
  RecordBuffer                                  Record(vDataFieldDefinitions);
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(gDataHub));

  if (theCluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER || theCluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER)
    vComprisedDataSetIndexes.push_back(0);
  else
    theCluster.GetClusterData()->GetDataSetIndexesComprisedInRatio(theCluster.m_nRatio, *Calculator.get(), vComprisedDataSetIndexes);

  for (unsigned int iSetIndex=0; iSetIndex < gParameters.GetNumDataSets(); ++iSetIndex) {
    Record.SetAllFieldsBlank(true);
    Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
    Record.GetFieldValue(DATASET_FIELD).AsDouble() = iSetIndex + 1;
    Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = 1;
    itr_Index = std::find(vComprisedDataSetIndexes.begin(), vComprisedDataSetIndexes.end(), iSetIndex);
    if (itr_Index != vComprisedDataSetIndexes.end()) {
      Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCount(iSetIndex);
      Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCount(gDataHub, iSetIndex);
      Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = theCluster.GetObservedDivExpected(gDataHub, iSetIndex);
      //either suppress printing this field because we didn't define it (not Poisson or Bernoulli) or
      //because the relative risk could not be calculated 
      if ((gParameters.GetProbabilityModelType() == POISSON  || gParameters.GetProbabilityModelType() == BERNOULLI) &&
          (dRelativeRisk = theCluster.GetRelativeRisk(gDataHub)) != -1)
         Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = dRelativeRisk;
    }
    if (gpASCIIFileDataWriter) gpASCIIFileDataWriter->WriteRecord(Record);
    if (gpDBaseFileDataWriter) gpDBaseFileDataWriter->WriteRecord(Record);
 }
}

/** Write obvserved, expected and  observed/expected to record for ordinal data.*/
void ClusterInformationWriter::WriteCountOrdinalData(const CCluster& theCluster, int iClusterNumber) const {
  ZdString                                              sBuffer;
  OrdinalLikelihoodCalculator                           Calculator(gDataHub);
  std::vector<OrdinalCombinedCategory>                  vCategoryContainer;
  std::vector<OrdinalCombinedCategory>::iterator        itrCategory;
  const AbstractCategoricalClusterData                * pClusterData=0;
  measure_t                                             tObservedDivExpected;
  double                                                tRelativeRisk;
  RecordBuffer                                          Record(vDataFieldDefinitions);
  std::vector<unsigned int>                             vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator                   itr_Index;

  if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(theCluster.GetClusterData())) == 0)
    ZdGenerateException("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n","WriteCountOrdinalData()");

  theCluster.GetClusterData()->GetDataSetIndexesComprisedInRatio(theCluster.m_nRatio, Calculator, vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
    //retrieve ordinal categories in combined state
    pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, *itr_Index);
    //for each combined category
    for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       Record.SetAllFieldsBlank(true);
       Record.GetFieldValue(CLUST_NUM_FIELD).AsDouble() = iClusterNumber;
       Record.GetFieldValue(DATASET_FIELD).AsDouble() = *itr_Index + 1;
       //calculate observed/expected and relative risk for combined categories
       count_t tObserved=0, tTotalCategoryCases=0;
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += theCluster.GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += theCluster.GetExpectedCountOrdinal(gDataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          tTotalCategoryCases += gDataHub.GetDataSetHandler().GetDataSet(*itr_Index).GetPopulationData().GetNumOrdinalCategoryCases(itrCategory->GetCategoryIndex(m));
       }
       //record observed/expected cases - categories which were combined will have the same value
       tObservedDivExpected = (tExpected ? (double)tObserved/tExpected  : 0);
       Record.GetFieldValue(OBSERVED_DIV_EXPECTED_FIELD).AsDouble() = tObservedDivExpected;
       //record relative risk - categories which were combined will have the same value
       tRelativeRisk = theCluster.GetRelativeRisk(tObserved, tExpected, tTotalCategoryCases);
       if (tRelativeRisk != -1 /*indicator of infinity*/)
         Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = tRelativeRisk;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          Record.GetFieldValue(CATEGORY_FIELD).AsDouble() = itrCategory->GetCategoryIndex(m) + 1;
          //record observed cases - not combining categories
          Record.GetFieldValue(OBSERVED_FIELD).AsDouble() = theCluster.GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          //record expected cases - not combining categories
          Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = theCluster.GetExpectedCountOrdinal(gDataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          if (gpASCIIFileDataWriter) gpASCIIFileDataWriter->WriteRecord(Record);
          if (gpDBaseFileDataWriter) gpDBaseFileDataWriter->WriteRecord(Record);
       }
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
  else
    Record.GetFieldValue(E_SHAPE_FIELD).AsDouble() = gDataHub.GetEllipseShape(thisCluster.GetEllipseOffset());
}

