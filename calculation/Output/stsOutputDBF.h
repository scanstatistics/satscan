#ifndef XDBASEOUTPUT_H
#define XDBASEOUTPUT_H

#include <cluster.h>

extern const char *    CLUSTER_LEVEL_DBF_FILE;
extern const char *    AREA_SPECIFIC_DBF_FILE;

struct field_t {
   std::string  sFieldName;
   char         cFieldType;
   short        wLength;
   short        wPrecision;
};

class DBaseOutput {
   private:
      void	Init();
      void	Setup(const ZdString& sReportHistoryFileName, const int& iCoordType = 0);
   protected:
      ZdString	                gsFileName;
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;
      int                       giCoordType;

      virtual void              CleanupFieldVector();
      virtual void              CreateDBFFile();
      virtual void              GetFields() = 0;
      virtual void              SetupFields(std::vector<field_t>& vFields) = 0;
   public:
      __fastcall DBaseOutput(const ZdString& sReportHistoryFileName, const int& iCoordType = 0);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) = 0;
};

#endif