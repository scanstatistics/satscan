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
      CleanupFieldVector();
   }
   catch (...) { /* munch munch */ }		
}

// deletes all of the field pointers in the vector and empties the vector
// pre: none
// post: field vector is empty and all of the pointers are deleted
void DBaseOutput::CleanupFieldVector() {
   try {
      for(int i = gvFields.GetNumElements() - 1; i > 0; --i) {
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

// global inits
void DBaseOutput::Init() {
   glRunNumber = 0;
}	

// internal setup function
void DBaseOutput::Setup(const ZdString& sFileName) {
   ZdFileRecord         *pLastRecord = 0;

   try{
      gsFileName = sFileName;

      // ugly hack to get the run number from the history file - need a new way to do this - AJV 9/7/2002
      if(ZdIO::Exists("c:\\AnalysisHistory.txd") && ZdIO::Exists("c:\\AnalysisHistory.zds"))  {
         TXDFile File("c:\\AnalysisHistory.txd", ZDIO_OPEN_READ);

         // if there's records in the file
         if(File.GotoLastRecord(pLastRecord))
            pLastRecord->GetField(1, glRunNumber);
         delete pLastRecord; pLastRecord = 0;
         File.Close();
      }
      ++glRunNumber;    // add one here to signify a new run - AJV
   }
   catch(ZdException &x) {
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("Setup()", "DBaseOutput");
      throw;
   }
}	

