#include "SaTScan.h"
#pragma hdrstop

#include "stsRunHistoryFile.h"

// constructor
stsRunHistoryFile::stsRunHistoryFile(const ZdString& sFilename) {
   try {
      Init();
      Setup(sFilename);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsRunHistoryFile");
      throw;
   }
}

// destructor
stsRunHistoryFile::~stsRunHistoryFile() {
   try {
   }
   catch (...) {/* munch munch */}
}

// creates the run history file
// pre: csv file doesn't not already exist
// post: will create the csv file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   ZdVector<char>       vFieldTypes;
   ZdVector<ZdString>	vFieldNames;
   ZdField*		pField = 0;
   ZdVector<ZdField*>	vFields;
   CSVFile              *pFile = 0;

   try {
      pFile = new CSVFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE | ZDIO_OPEN_CREATE);
      SetupFields(vFieldNames, vFieldTypes);
      for(unsigned int i = 0; i < vFieldNames.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldNames[i].GetCString());
         pField->SetType(vFieldTypes[i]);
         vFields.AddElement(pField->Clone());
         delete pField;
      }

      pFile->PackFields(vFields);
      pFile->Close();
      pFile->Create(gsFilename, vFields, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      pFile->Close();

      for (unsigned int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }
      delete pFile;
   }
   catch (ZdException &x) {     // geesh there's a lot of cleanup here
      try {
         if(pFile)
            pFile->Close();
         delete pFile; pFile = 0;
         delete pField; pField = 0;
         for (unsigned int i = vFields.GetNumElements() - 1; i > 0; --i) {
            delete vFields[0]; vFields[0] = 0;
            vFields.RemoveElement(0);
         }
      }
      catch (...) {/*munch munch here, with all this cleanup going on, I don't want to throw an exception from within
                  an exception, so I'll just suck it up here - AJV 9/3/2002*/}
      x.AddCallpath("CreateRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// global initializations
void stsRunHistoryFile::Init() {
}

// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and writes to it
void stsRunHistoryFile::OpenRunHistoryFile() {
   CSVFile		*pFile = 0;
   ZdTransaction*	pTransaction = 0;

   
   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile = new CSVFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      pTransaction = pFile->BeginTransaction();

      // get a record buffer, input data and append the record
      for(int i = 0; i < pFile->GetNumFields(); ++i) {
         
      }

      pFile->EndTransaction(pTransaction);
      pFile->Close();
      delete pTransaction;
      delete pFile;      
   }
   catch(ZdException &x) {
      if(pFile) {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pFile->Close();
      }   
      delete pFile; pFile = 0;
      delete pTransaction; pTransaction = 0;
      x.AddCallpath("OpenRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// internal setup
void stsRunHistoryFile::Setup(const ZdString& sFilename) {
   try {
      gsFilename = sFilename;
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsRunHistoryFile");
      throw;
   }
}

// creates the vector of field names
// pre: pass in an empty vector of strings
// post: passes back through reference the vector of field names
void stsRunHistoryFile::SetupFields(ZdVector<ZdString>& vFields, ZdVector<char>& vFieldTypes ) {
   try {
      vFields.AddElement("Run_Number");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Run_Time");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Output_File");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Prob_Model");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Rates");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Coord_Type");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Analysis_Type");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Number_Cases");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Total_Pop");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Num_Geo_Areas");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Precis_Times");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Max_Geo_Extent");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Max_Temp_Extent");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Time_Adjust");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Grid_File");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Start_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("End_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Alive_Only");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Interv_Units");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Interv_Len");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Monte_Carlo");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("101_Cutoff");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("005_Cutoff");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Num_Signif_005");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsRunHistoryFile");
      throw;
   }
}