#ifndef XDBASEOUTPUT_H
#define XDBASEOUTPUT_H

//#include "Cluster.h"

extern const char * CLUSTER_LEVEL_EXT;
extern const char * AREA_SPECIFIC_EXT;

extern const char * RUN_NUM;	
extern const char * CLUST_NUM;
extern const char * LOC_ID;  
extern const char * P_VALUE;  
extern const char * OBSERVED; 
extern const char * EXPECTED; 
extern const char * REL_RISK;
extern const char * START_DATE;
extern const char * END_DATE;
extern const char * LOG_LIKL;
extern const char * P_VALUE;
extern const char * NUM_AREAS;
extern const char * COORD_LAT;
extern const char * COORD_LONG;
extern const char * COORD_X;
extern const char * COORD_Y;
extern const char * COORD_Z;
extern const char * RADIUS;

class CCluster;
class CSaTScanData;

class DBaseOutput {
   private:
      void	Init();
      void	Setup(const long lRunNumber, const int iCoordType = 0);
   protected:
      ZdPointerVector<ZdField>  gvFields;
      long                      glRunNumber;  // unique run number assigned in Run History file and sent to dbf to be recorded - AJV 9/24/2002
      int                       giCoordType;
      ZdString                  gsFileName;  // although it is not defined in this class, it is set by the decendant classes
                                             // and used in this class, therefore since its used by both I factored it up - AJV 9/24/2002

      virtual void      CleanupFieldVector();
      virtual void      CreateDBFFile();
      virtual void      SetAreaID(ZdString& sTempValue, const CCluster& pCluster, const CSaTScanData& pData);
      virtual void      SetupFields(ZdPointerVector<ZdField>& vFields) = 0;
   public:
      __fastcall DBaseOutput(const long lRunNumber, const int iCoordType = 0);
      virtual ~DBaseOutput();

      virtual void      RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber) = 0;

      
};

static      void      CreateNewField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, const char cType, const short wLength,
                                     const short wPrecision, unsigned short& uwOffset, bool bCreateIndex = false);
static unsigned short GetFieldNumber(const ZdPointerVector<ZdField>& vFields, const char* sFieldName);
static      void      SetBoolField(ZdFileRecord& record, const bool bValue, const unsigned long uwFieldNumber);
static      void      SetDoubleField(ZdFileRecord& record, const double dValue, const unsigned long uwFieldNumber);
static      void      SetFieldVector(ZdVector<ZdField*>& vFields, const ZdFile& File);
static      void      SetLongField(ZdFileRecord& record, const long lValue, const unsigned long uwFieldNumber);
static      void      SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long uwFieldNumber);


// allocates a new field and adds it to the vector
// pre : none
// post : a field is added to the pointer vector with appropraite specs
static void CreateNewField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, const char cType, const short wLength,
                                           const short wPrecision, unsigned short& uwOffset, bool bCreateIndex) {
   ZdField  *pField = 0;
   TXDFile  File;
   
   try {
      pField = File.GetNewField();
      pField->SetName(sFieldName.c_str());
      pField->SetType(cType);
      pField->SetLength(wLength);
      pField->SetPrecision(wPrecision);
      pField->SetOffset(uwOffset);
      uwOffset += wLength;
      if(bCreateIndex)                    
         pField->SetIndexCount(1);
      vFields.AddElement(pField);   	
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateNewField()", "DBaseOutput");
      throw; 	
   }			
}

// finds the position of the field in the global field record
// pre : global field vector has been established
// post : will return the position of the field in the vector
static unsigned short GetFieldNumber(const ZdPointerVector<ZdField>& vFields, const char* sFieldName) {
   unsigned short       uwFieldNumber = -1;
   bool                 bFound = false;

   try {
      for(unsigned int i = 0; i < vFields.GetNumElements() && !bFound; ++i) {
         bFound = (!strcmp(vFields[i]->GetName(),sFieldName));
         uwFieldNumber = i;
      }

      if(!bFound)
         ZdException::GenerateNotification("Field name not found among the fields in file.", "DBaseOutput");
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFieldNumber()", "DBaseOutput");
      throw;
   }
   return uwFieldNumber;
}

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

// function to copy the ZdField pointers out of ZdFile type without taking ownership of them, i.e. cloning them
// pre : vector is empty
// post : vector will be filled with the field pointers used in the zdfile
static void SetFieldVector(ZdVector<ZdField*>& vFields, const ZdFile& File) {
   try {
      for(int i = 0; i < File.GetNumFields(); ++i) {
         vFields.push_back(File.GetFieldInfo(i)->Clone());
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldVector()", "DBaseOutput");
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