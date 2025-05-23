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
  public:
      static const char       * CLUSTER_FILE_EXT;
      static const char       * CLUSTERCASE_FILE_EXT;
      static const char       * START_DATE_FLD;
      static const char       * END_DATE_FLD;
      static const char       * RADIUS_FIELD;
      static const char       * SPAN_FIELD;
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

      static const char       * NUM_BATCHES_FIELD;
      static const char       * PROB_POSITIVE_INSIDE_FIELD;
      static const char       * PROB_POSITIVE_OUTSIDE_FIELD;

  protected:
      const CSaTScanData      & gDataHub;
      ASCIIDataFileWriter     * gpASCIIFileDataWriter;
      DBaseDataFileWriter     * gpDBaseFileDataWriter;
      ShapeDataFileWriter     * gpPolygonShapeDataFileWriter;
	  ShapeDataFileWriter     * gpPolyLineShapeDataFileWriter;
	  ptr_vector<FieldDef>      vDataFieldDefinitions;

      void                      DefineClusterInformationFields();
      void                      DefineClusterCaseInformationFields();
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
