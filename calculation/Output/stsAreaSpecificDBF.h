#ifndef stsAreaSpecificDBF_H
#define stsAreaSpecificDBF_H

#include "stsOutputDBF.h"

class stsAreaSpecificDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup();
   protected:
      virtual void 	SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths);
   public:
      stsAreaSpecificDBF(const ZdString& sFileName);
      virtual ~stsAreaSpecificDBF();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData);
};

#endif