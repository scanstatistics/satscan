// Adam J Vaughn
// November 2002
//
// This class object is used to print out an OutputFileData type in dBase format.

#include "SaTScan.h"
#pragma hdrstop

#include "stsDBaseFileWriter.h"
#include <DBFFile.h>

const char * 	DBASE_FILE_EXT	= ".dbf";

DBaseFileWriter::DBaseFileWriter(BaseOutputStorageClass* pOutputFileData, const bool bAppend)
                 : OutputFileWriter(pOutputFileData, bAppend) {
   try {
      Setup();
   }
   catch (ZdException &x) {
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning(x.GetErrorMessage());
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning("\nWarning - Unable to create dBase output file.\n");
   }
}

// creates the dBase file with the appropraite fields, if the file already exists it
// just overwrites the existing file
// pre : none
// post : creates the dBase file structure with the correct fields
void DBaseFileWriter::CreateOutputFile() {
   try {
      if (ZdIO::Exists(gsFileName))
         ZdIO::Delete(gsFileName);
      DBFFile file;
      ZdVector<ZdField*> vFields(gpOutputFileData->GetFields());
      file.PackFields(vFields);
      file.Create(gsFileName, vFields);
      file.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateOutputFile()", "DBaseFileWriter");
      throw;
   }			
}

// prints the data from the global output file data pointer to the
// dBase file      
void DBaseFileWriter::Print() {
   std::auto_ptr<DBFRecord>	pDBaseRecord;
   std::auto_ptr<DBFFile>       pFile;
   ZdTransaction*               pTransaction = 0;
   BaseOutputRecord*            pRecord = 0;

   try {
      pFile.reset(new DBFFile(gsFileName.GetCString()));
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
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning(x.GetErrorMessage());
      gpOutputFileData->GetBasePrinter()->SatScanPrintWarning("\nWarning - Unable to write record to dBase file %s.\n", gsFileName.GetCString());
   }
}

// setup
void DBaseFileWriter::Setup() {
   gsFileName = gpOutputFileData->GetFileName();
   gsFileName << DBASE_FILE_EXT;
   if(!gbAppend)
      CreateOutputFile();
}
