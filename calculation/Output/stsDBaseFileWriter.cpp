// Adam J Vaughn
// November 2002
//
// This class object is used to print out an OutputFileData type in dBase format.

#include "SaTScan.h"
#pragma hdrstop

#include "stsDBaseFileWriter.h"
#include <DBFFile.h>

const char * 	DBASE_FILE_EXT	= ".dbf";

DBaseFileWriter::DBaseFileWriter(BaseOutputStorageClass* pOutputFileData)
                 : OutputFileWriter(pOutputFileData) {
   try {
      Init();
      Setup();
   }
   catch (ZdException &x) {
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning(x.GetErrorMessage());
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning("\nWarning - Unable to create dBase output file.\n");
   }
}

DBaseFileWriter::~DBaseFileWriter() {
}

// creates the dBase file with the appropraite fields, if the file already exists it
// just overwrites the existing file
// pre : none
// post : creates the dBase file structure with the correct fields
void DBaseFileWriter::CreateOutputFile() {
   try {
      ZdVector<ZdField*> vFields(gpOutputFileData->GetFields());
      DBFFile file;
      if (ZdIO::Exists(gsFileName))
         ZdIO::Delete(gsFileName);
      file.PackFields(vFields);
      file.Create(gsFileName, vFields);
      file.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateOutputFile()", "DBaseFileWriter");
      throw;
   }			
}	

// init
void DBaseFileWriter::Init() {
}

// prints the data from the global output file data pointer to the
// dBase file      
void DBaseFileWriter::Print() {
   std::auto_ptr<DBFRecord>	pDBaseRecord;
   std::auto_ptr<DBFFile>       pFile(new DBFFile(gsFileName.GetCString()));
   ZdTransaction*               pTransaction = 0;
   BaseOutputRecord*            pRecord = 0;

   try {
      pTransaction = pFile->BeginTransaction();
      for(unsigned long i = 0; i < gpOutputFileData->GetNumRecords(); ++i) {
         pRecord = gpOutputFileData->GetRecord(i);
         pDBaseRecord.reset(pFile->GetNewRecord());
         for (unsigned short j = 0; j < pRecord->GetNumFields(); ++j) {
            if (!pRecord->GetFieldIsBlank(j))
               pDBaseRecord->PutFieldValue(j, pRecord->GetValue(j));
            else
               pDBaseRecord->PutBlank(j);
         }
         pFile->AppendRecord(*pTransaction, *pDBaseRecord);
      }

      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch (ZdException &x) {
      if (pTransaction)
         pFile->EndTransaction(pTransaction);
      pFile->Close();
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning(x.GetErrorMessage());
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning("\nWarning - Unable to write record to dBase file %s.\n", gsFileName.GetCString());
   }
}

// setup
void DBaseFileWriter::Setup() {
   gsFileName = gpOutputFileData->GetFileName();
   gsFileName << DBASE_FILE_EXT;
   CreateOutputFile();
}
