#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
      void      SetCoordinates(float& fLatitude, float& fLongitude, float& fRadius, ZdString& sAdditCoords,
                               const CCluster& pCluster, const CSaTScanData& pData);
   protected:
      virtual void              SetupFields(std::vector<field_t>& vFields);
   public:
      __fastcall stsClusterLevelDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName);
      virtual ~stsClusterLevelDBF();

      void              RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber);
};

#endif