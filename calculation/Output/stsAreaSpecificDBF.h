#ifndef stsAreaSpecificDBF_H
#define stsAreaSpecificDBF_H

#include "stsOutputDBF.h"

class stsAreaSpecificDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      virtual void              GetFields();
      virtual void      	SetupFields(ZdVector<std::pair<ZdString, char> >& vFieldDescrips, ZdVector<std::pair<short, short> >& vFieldSizes);
   public:
      __fastcall stsAreaSpecificDBF(const ZdString& sFileName);
      virtual ~stsAreaSpecificDBF();

      virtual void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif