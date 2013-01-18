//******************************************************************************
#ifndef ClusterInformationWriterH
#define ClusterInformationWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"
#include "SimulationVariables.h"
#include "ShapeFileWriter.h"

class CCluster;       /** forward class declaration */
class CSaTScanData;   /** forward class declaration */

class ClusterInformationWriter : public AbstractDataFileWriter {
  protected:
      static const char       * CLUSTER_FILE_EXT;
      static const char       * CLUSTERCASE_FILE_EXT;
      static const char       * START_DATE_FLD;
      static const char       * END_DATE_FLD;
      static const char       * RADIUS_FIELD;
      static const char       * E_MINOR_FIELD;
      static const char       * E_MAJOR_FIELD;
      static const char       * E_ANGLE_FIELD;
      static const char       * E_SHAPE_FIELD;
      static const char       * NUM_LOCATIONS_FIELD;
      static const char       * COORD_LAT_FIELD;
      static const char       * COORD_LONG_FIELD;
      static const char       * COORD_X_FIELD;
      static const char       * COORD_Y_FIELD;
      static const char       * COORD_Z_FIELD;
      static const char       * OBS_FIELD_PART;
      static const char       * EXP_FIELD_PART;
      static const char       * OBS_DIV_EXP_FIELD;
      static const char       * TIME_TREND_IN_FIELD;
      static const char       * TIME_TREND_OUT_FIELD;
      static const char       * TIME_TREND_DIFF_FIELD;

      static const char       * ALPHA_IN_FIELD;
      static const char       * BETA1_IN_FIELD;
      static const char       * BETA2_IN_FIELD;
      static const char       * ALPHA_OUT_FIELD;
      static const char       * BETA1_OUT_FIELD;
      static const char       * BETA2_OUT_FIELD;
      static const char       * ALPHA_GLOBAL_FIELD;
      static const char       * BETA1_GLOBAL_FIELD;
      static const char       * BETA2_GLOBAL_FIELD;
      //static const char       * FUNC_ALPHA_IN_FIELD;
      //static const char       * FUNC_ALPHA_OUT_FIELD;

      static const char       * WEIGHTED_VARIANCE_FIELD;
      static const char       * WEIGHTED_STD_FIELD;
      static const char       * WEIGHTED_MEAN_INSIDE_FIELD;
      static const char       * WEIGHTED_MEAN_OUTSIDE_FIELD;
      static const char       * WEIGHT_INSIDE_FIELD;
      static const char       * GINI_CLUSTER_FIELD;

      const CSaTScanData      & gDataHub;
      ASCIIDataFileWriter     * gpASCIIFileDataWriter;
      DBaseDataFileWriter     * gpDBaseFileDataWriter;
      ShapeDataFileWriter     * gpShapeDataFileWriter;
      ptr_vector<FieldDef>      vDataFieldDefinitions;

      void                      DefineClusterInformationFields();
      void                      DefineClusterCaseInformationFields();
      std::string             & GetAreaID(std::string& sAreaId, const CCluster& thisCluster) const;
      void                      WriteClusterCaseInformation(const CCluster& theCluster, int iClusterNumber);
      void                      WriteClusterInformation(const CCluster& theCluster, int iClusterNumber, const SimulationVariables& simVars);
      void                      WriteCoordinates(RecordBuffer& Record, const CCluster& pCluster);
      void                      WriteCountData(const CCluster& thisCluster, int iClusterNumber) const;
      void                      WriteCountOrdinalData(const CCluster& thisCluster, int iClusterNumber) const;
      void                      WriteEllipseAngle(RecordBuffer& Record, const CCluster& thisCluster) const;
      void                      WriteEllipseShape(RecordBuffer& Record, const CCluster& thisCluster) const;

  public:
    ClusterInformationWriter(const CSaTScanData& DataHub, bool bAppend=false);
    virtual ~ClusterInformationWriter();

      void                      Write(const CCluster& theCluster, int iClusterNumber, const SimulationVariables& simVars);
};
//******************************************************************************
#endif
