#ifndef XDBASEOUTPUT_H
#define XDBASEOUTPUT_H

#include <cluster.h>

extern const char *    CLUSTER_LEVEL_DBF_FILE;
extern const char *    AREA_SPECIFIC_DBF_FILE;

// my little temp class to store field info
class field_t {
   public:
      std::string  gsFieldName;
      char         gcFieldType;
      short        gwLength;
      short        gwPrecision;

      field_t(std::string  sFieldName, char cFieldType, short wLength, short wPrecision)
              {gsFieldName = sFieldName; gcFieldType = cFieldType; gwLength = wLength; gwPrecision = wPrecision;}
};

class DBaseOutput {
   private:
      void	Init();
      void	Setup(const long& lRunNumber, const int& iCoordType = 0);
   protected:
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;
      int                       giCoordType;
      ZdString                  gsFileName;

      virtual void      CleanupFieldVector();
      virtual void      CreateDBFFile();
      virtual void      GetFields() = 0;
      virtual void      SetBoolField(ZdFileRecord& record, const bool& bValue, const unsigned long& uwFieldNumber);
      virtual void      SetDoubleField(ZdFileRecord& record, const double& dValue, const unsigned long& uwFieldNumber);
      virtual void      SetLongField(ZdFileRecord& record, const long& lValue, const unsigned long& uwFieldNumber);
      virtual void      SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long& uwFieldNumber);
      virtual void      SetupFields(std::vector<field_t>& vFields) = 0;
   public:
      __fastcall DBaseOutput(const long& lRunNumber, const int& iCoordType = 0);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster* pCluster, const CSaTScanData* pData, int iClusterNumber) = 0;
};

#endif