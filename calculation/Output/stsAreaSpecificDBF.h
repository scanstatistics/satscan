#ifndef stsAreaSpecificDBF_H
#define stsAreaSpecificDBF_H

 #include "stsOutputDBF.h"

class stsAreaSpecificDBF : public DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
   protected:
      virtual void              SetupFields(ZdPointerVector<ZdField>& vFields);
   public:
      __fastcall stsAreaSpecificDBF(const long lRunNumber, const int iCoordType, const ZdFileName& sOutputFileName);
      __fastcall stsAreaSpecificDBF();
      virtual ~stsAreaSpecificDBF();

      virtual void              RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber);
};

#endif