#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF : public DBaseOutput {
   private:
      int       giDimension, giModelType;
      bool      gbPrintEllipses;

      void	Init();
      void      SetCoordinates(std::string& sLatitude, std::string& sLongitude, std::string& sRadius,
                               std::vector<std::string>& vAdditCoords,
                               const CCluster& pCluster, const CSaTScanData& pData);
      void      SetEllipseString(std::string& sAngle, std::string& sShape, const CCluster& pCluster, const CSaTScanData& pData);
      void      SetStartAndEndDates(std::string& sStartDate, std::string& sEndDate, const CCluster& pCluster, const CSaTScanData& pData);                         
      void	Setup(const ZdString& sOutputFileName, const int iModelType,  const int iDimension, const bool bPrintEllipses = false);
   protected:
      virtual void  SetupFields(ZdPointerVector<ZdField>& vFields);
   public:
      __fastcall stsClusterLevelDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName, const int iModelType, const int iDimension = 2, const bool bPrintPVal = true, const bool bPrintEllipses = false);
      virtual ~stsClusterLevelDBF();

      virtual void  RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber, tract_t tTract = 0);
};

#endif