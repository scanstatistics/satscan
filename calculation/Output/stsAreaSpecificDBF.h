#ifndef stsAreaSpecificDBF_H
#define stsAreaSpecificDBF_H

#include "stsOutputDBF.h"

class stsAreaSpecificDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
   protected:
      virtual void              GetFields();
      virtual void              SetupFields(std::vector<field_t>& vFields);
   public:
      __fastcall stsAreaSpecificDBF(const ZdString& sReportHistoryFileName, const int& iCoordType, const ZdFileName& sOutputFileName);
      virtual ~stsAreaSpecificDBF();

      virtual void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif