// Class DBaseOutput
// Adam J Vaughn
// September 3, 2002
//
// This class is designed to create and fill a DBase file from the data produced
// by the calculations in the SatScan program. This is a base class to be derived off
// of for each necessary dBase output file.

#include "SaTScan.h"
#pragma hdrstop

#include <DBFFile.h>
#include "stsOutputDBF.h"

const char *    CLUSTER_LEVEL_DBF_FILE  =       "ClusterLevel.dbf";
const char *    AREA_SPECIFIC_DBF_FILE  =       "AreaSpecific.dbf";

// constructor
__fastcall DBaseOutput::DBaseOutput(const long lRunNumber, const int iCoordType) {
   try {
      Init();
      Setup(lRunNumber, iCoordType);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "DBaseOutput");
      throw;	
   }			
}

// destructor
DBaseOutput::~DBaseOutput() {
   try {
      CleanupFieldVector();
   }
   catch (...) { /* munch munch */ }		
}

// deletes all of the field pointers in the vector and empties the vector
// pre: none
// post: field vector is empty and all of the pointers are deleted
void DBaseOutput::CleanupFieldVector() {
   try {
      while (gvFields.GetNumElements()) {
         delete gvFields[0]; gvFields[0] = 0;
         gvFields.RemoveElement(0);
      }
   }
   catch(ZdException &x) {
      x.AddCallpath("CleanupFieldVector()", "stsClusterLevelDBF");
      throw;
   }
}

// create the output file
// pre: gvFields have been set up and gsFileName has been set
// post: creates the dbf file with the appropraite fields
void DBaseOutput::CreateDBFFile() {
   try {
      // pack up and create
      DBFFile File;
      File.PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      if(ZdIO::Exists(gsFileName))
        ZdIO::Delete(gsFileName);
      File.Create(gsFileName, gvFields);
      File.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateDBFFile()", "DBaseOutput");
      throw;
   }
}

// sets up the global vector of ZdFields
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void DBaseOutput::GetFields() {
   DBFFile		File;
   ZdField		*pField = 0;
   std::vector<field_t>         vFields;

   try {
      CleanupFieldVector();           // empty out the global field vector
      SetupFields(vFields);

      for(unsigned int i = 0; i < vFields.size(); ++i) {
         pField = (File.GetNewField());
         pField->SetName(vFields[i].gsFieldName.c_str());
         pField->SetType(vFields[i].gcFieldType);
         pField->SetLength(vFields[i].gwLength);
         pField->SetPrecision(vFields[i].gwPrecision);
         gvFields.AddElement(pField);  ;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFields()", "DBaseOutput");
      throw;
   }
}

// global inits
void DBaseOutput::Init() {
   glRunNumber = 0;
   giCoordType = 0;
}	

// function to set the value of boolean fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
void DBaseOutput::SetBoolField(ZdFileRecord& record, const bool bValue, const unsigned long uwFieldNumber) {
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
void DBaseOutput::SetDoubleField(ZdFileRecord& record, const double dValue, const unsigned long uwFieldNumber) {
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
void DBaseOutput::SetLongField(ZdFileRecord& record, const long lValue, const unsigned long uwFieldNumber) {
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
void DBaseOutput::SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long uwFieldNumber) {
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

// internal setup function
void DBaseOutput::Setup(const long lRunNumber, const int iCoordType) {
   giCoordType = iCoordType;
   glRunNumber = lRunNumber;
}	

