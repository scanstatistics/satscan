#ifndef stsClusterLevelDBF_H
#define stsClusterLevelDBF_H

class stsClusterLevelDBF {
   private:
      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;

      void 	        CreateDBFFile();
      void              GetFields(ZdVector<ZdField*>& vFields);
      void 	        SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths);
   public:
      __fastcall stsClusterLevelDBF(const ZdString& sFileName);
      virtual ~stsClusterLevelDBF();

      void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif