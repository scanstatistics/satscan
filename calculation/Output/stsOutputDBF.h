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

      field_t(const std::string&  sFieldName, char cFieldType, short wLength, short wPrecision)
              {gsFieldName = sFieldName; gcFieldType = cFieldType; gwLength = wLength; gwPrecision = wPrecision;}
};

class DBaseOutput {
   private:
      void	Init();
      void	Setup(const long lRunNumber, const int iCoordType = 0);
   protected:
      ZdVector<ZdField*>        gvFields;
      long                      glRunNumber;  // unique run number assigned in Run History file and sent to dbf to be recorded - AJV 9/24/2002
      int                       giCoordType;
      ZdString                  gsFileName;  // although it is not defined in this class, it is set by the decendant classes
                                             // and used in this class, therefore since its used by both I factored it up - AJV 9/24/2002

      virtual void      CleanupFieldVector();
      virtual void      CreateDBFFile();
      virtual void      GetFields();
      
      virtual void      SetupFields(std::vector<field_t>& vFields) = 0;
   public:
      __fastcall DBaseOutput(const long lRunNumber, const int iCoordType = 0);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber) = 0;

      
};

static      void      SetBoolField(ZdFileRecord& record, const bool bValue, const unsigned long uwFieldNumber);
static      void      SetDoubleField(ZdFileRecord& record, const double dValue, const unsigned long uwFieldNumber);
static      void      SetLongField(ZdFileRecord& record, const long lValue, const unsigned long uwFieldNumber);
static      void      SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long uwFieldNumber);


// function to set the value of boolean fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetBoolField(ZdFileRecord& record, const bool bValue, const unsigned long uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsBool() = bValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetBoolField()", "DBaseOutput");
      throw;
   }
}

// function to set the value of double fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetDoubleField(ZdFileRecord& record, const double dValue, const unsigned long uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsDouble() = dValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetDoubleField()", "DBaseOutput");
      throw;
   }
}

// function to set the value of long fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetLongField(ZdFileRecord& record, const long lValue, const unsigned long uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsLong() = lValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetLongField()", "DBaseOutput");
      throw;
   }
}

// function to set the value of string fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsZdString() = sValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetStringField()", "DBaseOutput");
      throw;
   }
}

#endif