//******************************************************************************
#ifndef ClusterInformationWriterH
#define ClusterInformationWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

class CCluster;       /** forward class declaration */
class CSaTScanData;   /** forward class declaration */

class ClusterInformationWriter : public AbstractDataFileWriter {
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
      static const char       * OBS_FIELD_PART;
      static const char       * EXP_FIELD_PART;
      static const char       * OBS_DIV_EXP_FIELD;
      static const char       * RELATIVE_RISK_FIELD;
      static const char       * CATEGORY_FIELD_PART;
      static const char       * SET_FIELD_PART;
      static const char       * SET_CATEGORY_FIELD_PART;

      const CSaTScanData      & gDataHub;
      bool                      gbExcludePValueField;     

      void                      DefineFields();
      ZdString                & GetAreaID(ZdString& sAreaId, const CCluster& thisCluster) const;
      void                      WriteCoordinates(RecordBuffer& Record, const CCluster& pCluster);
//      void                      WriteCountDataAsOrdinal(RecordBuffer& Record, const CCluster& thisCluster) const;
      void                      WriteCountDataStandard(RecordBuffer& Record, const CCluster& thisCluster) const;
      void                      WriteEllipseAngle(RecordBuffer& Record, const CCluster& thisCluster) const;
      void                      WriteEllipseShape(RecordBuffer& Record, const CCluster& thisCluster) const;

  public:
    ClusterInformationWriter(const CSaTScanData& DataHub, bool bExcludePValueField, bool bAppend=false);
    virtual ~ClusterInformationWriter();
    
    void                      Write(const CCluster& theCluster, int iClusterNumber, unsigned int iNumSimsCompleted);
};
//******************************************************************************
#endif
