#ifndef DBASEOUTPUT_H
#define DBASEOUTPUT_H

class DBaseOutput {
   private:
      

      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;

      void 	        CreateDBFFile();
      void              GetFields(ZdVector<ZdField*>& vFields);
      virtual void 	SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths) = 0;
   public:
      __fastcall DBaseOutput(const ZdString& sFileName);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData) = 0;
};

#endif