#ifndef XDBASEOUTPUT_H
#define XDBASEOUTPUT_H

class DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sFileName);
   protected:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;

      virtual void              CleanupFieldVector();
      virtual void              CreateDBFFile();
      virtual void              GetFields() = 0;
      virtual void      	SetupFields(ZdVector<std::pair<ZdString, char> >& vFieldDescrips, ZdVector<std::pair<short, short> >& vFieldSizes) = 0;
   public:
      __fastcall DBaseOutput(const ZdString& sFileName);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) = 0;
};

#endif