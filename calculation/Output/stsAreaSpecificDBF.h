#ifndef stsAreaSpecificDBF_H
#define stsAreaSpecificDBF_H

class stsAreaSpecificDBF {
   private:
      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;

      void 	        CreateDBFFile();
      void              GetFields(ZdVector<ZdField*>& vFields);
      void      	SetupFields(ZdVector<std::pair<std::pair<ZdString, char>, short> >& vFieldDescrips);
   public:
      __fastcall stsAreaSpecificDBF(const ZdString& sFileName);
      virtual ~stsAreaSpecificDBF();

      void              RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber);
};

#endif