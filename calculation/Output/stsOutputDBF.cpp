// Class DBaseOutput
// Adam J Vaughn
// September 3, 2002
//
// This class is designed to create and fill a DBase file from the data produced
// by the calculations in the SatScan program. This is a base class to be derived off
// of for each necessary dBase output file.

#include "stsSaTScan.h"
#pragma hdrstop

#include <DBFFile.h>
#include "stsOutputDBF.h"

// constructor
__fastcall DBaseOutput::DBaseOutput(const ZdString& sFileName) {
   try {
      Init();
      Setup(sFileName);	
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "DBaseOutput");
      throw;	
   }			
}

// destructor
DBaseOutput::~DBaseOutput() {
   try {
      for(unsigned int i = gvFields.GetNumElements() - 1; i > 0; --i) {
         delete gvFields[0]; gvFields[0] = 0;
         gvFields.RemoveElement(0);
      }
   }
   catch (...) { /* munch munch */ }		
}		
/*
// create the output file
// pre: sFileName is name of the dbf file needing to be created
// post: creates the dbf file with the appropraite fields
void DBaseOutput::CreateDBFFile() {
   DBFFile		*pFile = 0;
   
   try {
      GetFields(gvFields);

      // pack up and create
      pFile = new DBFFile();
      pFile->PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      if(ZdIO::Exists(gsFileName))
        ZdIO::Delete(gsFileName);
      pFile->Create(gsFileName, gvFields, 1);
      pFile->Close();

      delete pFile;	
   }
   catch (ZdException &x) {
      if(pFile)
         pFile->Close();
      delete pFile; pFile = 0;
      x.AddCallpath("CreateDBFFile()", "DBaseOutput");
      throw;
   }
}

// sets up the ZdFields and puts them into the vector
// pre: pass in an empty vector
// post: vector will be defined using the names and field types provided by the descendant classes
void DBaseOutput::GetFields(ZdVector<ZdField*>& vFields) {
   DBFFile*		pFile = 0;
   ZdVector<ZdString> 	vFieldNames;
   ZdVector<char> 	vFieldTypes;
   ZdField*		pField = 0;
   ZdVector<short>      vFieldLengths;

   try {
      pFile = new DBFFile();
      SetupFields(vFieldNames, vFieldTypes, vFieldLengths);
      
      for(unsigned int i = 0; i < vFieldNames.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldNames[i]);
         pField->SetType(vFieldTypes[i]);
         pField->SetLength(vFieldLengths[i]);
         vFields.AddElement(pField->Clone());
         delete pField;
      }
      
      delete pFile;
   }
   catch (ZdException &x) {
      delete pFile; pFile = 0;
      delete pField; pField = 0;
      x.AddCallpath("GetFields()", "DBaseOutput");
      throw;
   }
}  */

// global inits
void DBaseOutput::Init() {
   glRunNumber = 0;
}	

// internal setup function
void DBaseOutput::Setup(const ZdString& sFileName) {
   TXDFile	        *pFile = 0;
   ZdFileRecord         *pLastRecord = 0;
   unsigned long        ulLastRecordNumber;

   try {
      pFile = new TXDFile("AnalysisHistory.txd", ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);

      // get a record buffer, input data and append the record
      ulLastRecordNumber = pFile->GotoLastRecord(pLastRecord);
      // if there's records in the file
      if(ulLastRecordNumber)
         pLastRecord->GetField(0, glRunNumber);
      delete pLastRecord;
      pFile->Close();

      delete pFile;

      gsFileName = sFileName;
   }
   catch(ZdException &x) {
      if(pFile)
         pFile->Close();
      delete pFile; pFile = 0;
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("Setup()", "DBaseOutput");
      throw;
   }		
}	

