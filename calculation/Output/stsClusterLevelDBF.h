#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF:public DBaseOutput {
   private:
      void	Init();
      void	Setup();
   protected:
      virtual void 	SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths);
   public:
      stsClusterLevelDBF(const ZdString& sFileName);
      virtual ~stsClusterLevelDBF();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData);
};

#endif