#ifndef DBASEOUTPUT_H
#define DBASEOUTPUT_H

class DBaseOutput {
   private:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;

      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      void 	CreateDBFFile();
//      void 	FillDBFFile();

      void              GetFields(ZdVector<ZdField*>& vFields);
      virtual void 	SetupFields(ZdVector<ZdString>& vFieldNames, ZdVector<char>& vFieldTypes, ZdVector<short>& vFieldLengths) = 0;
   public:
      __fastcall DBaseOutput(const ZdString& sFileName);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData) = 0;
};

#endif