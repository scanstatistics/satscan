#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

#include "stsOutputDBF.h"

class stsClusterLevelDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      virtual void              GetFields();
      virtual void 	        SetupFields(ZdVector<std::pair<ZdString, char> >& vFieldDescrips, ZdVector<std::pair<short, short> >& vFieldSizes);
   public:
      __fastcall stsClusterLevelDBF(const ZdString& sFileName);
      virtual ~stsClusterLevelDBF();

      void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif