//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsDBaseFileWriter.h"
#include <DBFFile.h>
//#include "AbstractDataFileWriter.h"
#include "stsLogLikelihood.h"

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
    gPrintDirection.SatScanPrintWarning("\nWarning - Unable to create the dBase output file.\n");
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
      gPrintDirection.SatScanPrintWarning("\nWarning - Unable to write to dBase file %s.\n", gsFileName.GetCString());
   }
}

// setup
void DBaseFileWriter::Setup(const CParameters& Parameters, bool bAppend) {
   SetOutputFileName(Parameters.GetOutputFileName().c_str(), DBASE_FILE_EXT);
   if (!bAppend)
     CreateOutputFile();
}






const char * DBaseDataFileWriter::DBASE_FILE_EXT    = ".dbf";

DBaseDataFileWriter::DBaseDataFileWriter(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const char * sFileExtension, bool bAppend)
                    :DataFileWriter() {
  try {
    Setup(Parameters, vFieldDefs, sFileExtension, bAppend);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","DBaseDataFileWriter");
    throw;
  }
}

DBaseDataFileWriter::~DBaseDataFileWriter() {
  try {
    pFile->EndTransaction(pTransaction); pTransaction = 0;
    pFile->Close();
    delete pFile;
  }
  catch (...){}
}

// creates the dBase file with the appropraite fields, if the file already exists it
// just overwrites the existing file
// pre : none
// post : creates the dBase file structure with the correct fields
void DBaseDataFileWriter::CreateOutputFile(ZdPointerVector<ZdField>& vFieldDefs) {
   try {
      if (ZdIO::Exists(gsFileName.GetFullPath()))
         ZdIO::Delete(gsFileName.GetFullPath());
      DBFFile file;
      //ZdVector<ZdField*> vFields(gOutputFileData.GetFields());
      file.PackFields(vFieldDefs);
      file.Create(gsFileName.GetFullPath(), vFieldDefs);
      file.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateOutputFile()", "DBaseDataFileWriter");
      throw;
   }			
}

// setup
void DBaseDataFileWriter::Setup(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const char * sFileExtension, bool bAppend) {
   ZdString sExt(sFileExtension);

   sExt << DBASE_FILE_EXT;
   SetOutputFileName(Parameters.GetOutputFileName().c_str(), sExt.GetCString());

   if (!bAppend)
     CreateOutputFile(vFieldDefs);

  pFile = new DBFFile(gsFileName.GetFullPath());
  pTransaction = pFile->BeginTransaction();
}

void DBaseDataFileWriter::WriteRecord(const ZdPointerVector<ZdField>& vFieldDefs, const RecordBuffer& Record) {
  std::auto_ptr<DBFRecord>	pDBaseRecord;
//  ZdTransaction              * pTransaction = 0;

  try {
    pDBaseRecord.reset(pFile->GetNewRecord());
    for (unsigned int j = 0; j < Record.GetNumFields(); ++j) {
       if (!Record.GetFieldIsBlank(j))
         pDBaseRecord->PutFieldValue(j, Record.GetFieldValue(j));
       else
         pDBaseRecord->PutBlank(j);
    }
    pFile->AppendRecord(*pTransaction, *pDBaseRecord);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteRecord()","DBaseDataFileWriter");
    throw;
  }
}

