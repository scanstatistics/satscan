//***************************************************************************
#ifndef stsClusterFile_H
#define stsClusterFile_H
//***************************************************************************
#include "stsOutputFileData.h"
#include "Parameters.h"

class CCluster;
class CSaTScanData;

/** This class is responsible for the storage of the cluster information to be
    outputted. It is not responsible for any writing to a file format, just for
    the storage of the data.*/
class stsClusterData : public BaseOutputStorageClass {
   private:
      void  	                SetupFields();

   protected:
      static const char       * CLUSTER_FILE_EXT;
      static const char       * START_DATE_FLD;
      static const char       * END_DATE_FLD;
      static const char       * RADIUS_FIELD;
      static const char       * E_ANGLE_FIELD;
      static const char       * E_SHAPE_FIELD;
      static const char       * NUM_LOCATIONS_FIELD;
      static const char       * COORD_LAT_FIELD;
      static const char       * COORD_LONG_FIELD;
      static const char       * COORD_X_FIELD;
      static const char       * COORD_Y_FIELD;
      static const char       * COORD_Z_FIELD;
      static const char       * OBS_DIV_EXP_FIELD;
      static const char       * SET_OBSERVED_FIELD;
      static const char       * SET_EXPECTED_FIELD;
      static const char       * SET_OBS_DIV_EXP_FIELD;
      const CParameters       & gParameters;
      bool                      gbExcludePValueField;

      ZdString                & GetAreaID(ZdString& sAreaId, const CCluster& thisCluster, const CSaTScanData& DataHub) const;
      void                      WriteCoordinates(OutputRecord& Record, const CCluster& pCluster, const CSaTScanData& DataHub);
      void                      WriteEllipseAngle(OutputRecord& Record, const CCluster& thisCluster, const CSaTScanData& DataHub) const;
      void                      WriteEllipseShape(OutputRecord& Record, const CCluster& thisCluster, const CSaTScanData& DataHub) const;

   public:
      stsClusterData(const CParameters& Parameters, bool bExcludePValueField);
      virtual    ~stsClusterData();

      virtual const char      * GetOutputExtension() const {return CLUSTER_FILE_EXT;}
      void                      RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData,
                                                  int iClusterNumber, unsigned int iNumSimsCompleted);
};
//***************************************************************************
#endif
