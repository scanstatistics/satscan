#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF : public DBaseOutput {
   private:
      int       giDimension;

      void	Init();
      void      SetCoordinates(float& fLatitude, float& fLongitude, float& fRadius, std::vector<float>& vAdditCoords,
                               const CCluster& pCluster, const CSaTScanData& pData);
      void	Setup(const ZdString& sOutputFileName, const int iDimension);
   protected:
      virtual void  SetupFields(ZdPointerVector<ZdField>& vFields);
   public:
      __fastcall stsClusterLevelDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName, const int iDimension = 2);
      virtual ~stsClusterLevelDBF();

      virtual void  RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber, tract_t tTract = 0);
};

#endif