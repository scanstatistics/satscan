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

      void              CleanupFieldVector();
      void 	        CreateDBFFile();
      void              GetFields();
      void 	        SetupFields(ZdVector<pair<pair<ZdString, char>, short> >& vFieldDescrips);
   public:
      __fastcall stsClusterLevelDBF(const ZdString& sFileName);
      virtual ~stsClusterLevelDBF();

      void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif