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
/*
// fills the output file with the appropraite records
// pre: sFileName is the name of the dbf file, the structure of the dbf file has already been
//      setup
// post: the dbf file is filled with the appropraite analysis data
void DBaseOutput::FillDBFFile() {
   DBFFile*		pFile = 0;
   DBFRec*		pRecord = 0;
   ZdTransaction *	pTransaction = 0;

   try {
      pFile = new DBFFile(gsFileName.GetCString(), ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);

      pTransaction = pFile->BeginTransaction();

      for(unsigned int i = 0; i < DBFFile->GetNumFields(); ++i) {
         pRecord = pFile->GetNewRecord();
         // define record data

         pFile->AppendRecord(*pTransaction, *pRecord);
         delete pRecord;
      }

      pFile->EndTransaction(pTransaction);
      pFile->Close();
      delete pTransaction;
      delete pFile;
   }
   catch (ZdException &x) {
      if(pFile) {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pFile->Close();
      }
      delete pFile; pFile = 0;
      delete pTransaction; pTransaction = 0;
      delete pRecord; pRecord = 0;
      x.AddCallpath("FillDBFFile()", "DBaseOutput");
      throw;
   }
}
  */
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
}

// global inits
void DBaseOutput::Init() {
}	

// internal setup function
void DBaseOutput::Setup(const ZdString& sFileName) {
   try {
      gsFileName = sFileName;
      CreateDBFFile();
   }
   catch(ZdException &x) {
      x.AddCallpath("Setup()", "DBaseOutput");
      throw;
   }		
}	

