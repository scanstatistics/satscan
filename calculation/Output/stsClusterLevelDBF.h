#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
   protected:
      virtual void              GetFields();
      virtual void              SetupFields(std::vector<field_t>& vFields);
   public:
      __fastcall stsClusterLevelDBF(const ZdString& sReportHistoryFileName, const int& iCoordType, const ZdFileName& sOutputFileName);
      virtual ~stsClusterLevelDBF();

      void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif