//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsDBaseFileWriter.h"
#include <DBFFile.h>

const char * DBaseFileWriter::DBASE_FILE_EXT    = ".dbf";

DBaseFileWriter::DBaseFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection,
                                 const CParameters& Parameters, bool bAppend)
                :OutputFileWriter(OutputFileData, PrintDirection) {
  try {
    Setup(Parameters, bAppend);
    Print();
  }
  catch (ZdException &x) {
    gPrintDirection.SatScanPrintWarning(x.GetErrorMessage());
    gPrintDirection.SatScanPrintWarning("\nWarning - Unable to create dBase output file.\n");
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
      ZdVector<ZdField*> vFields(gOutputFileData.GetFields());
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
   ZdTransaction              * pTransaction = 0;
   const OutputRecord         * pRecord = 0;

   try {
      pFile.reset(new DBFFile(gsFileName.GetCString()));
      pTransaction = pFile->BeginTransaction();
      for(unsigned int i = 0; i < gOutputFileData.GetNumRecords(); ++i) {
         pRecord = gOutputFileData.GetRecord(i);
         pDBaseRecord.reset(pFile->GetNewRecord());
         for (unsigned int j = 0; j < pRecord->GetNumFields(); ++j) {
            if (!pRecord->GetFieldIsBlank(j))
               pDBaseRecord->PutFieldValue(j, pRecord->GetFieldValue(j));
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
      gPrintDirection.SatScanPrintWarning(x.GetErrorMessage());
      gPrintDirection.SatScanPrintWarning("\nWarning - Unable to write record to dBase file %s.\n", gsFileName.GetCString());
   }
}

// setup
void DBaseFileWriter::Setup(const CParameters& Parameters, bool bAppend) {
   SetOutputFileName(Parameters.GetOutputFileName().c_str(), DBASE_FILE_EXT);
   if (!bAppend)
     CreateOutputFile();
}
