//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsDBaseFileWriter.h"
#include "AbstractDataFileWriter.h"

const char * DBaseDataFileWriter::DBASE_FILE_EXT    = ".dbf";

/** constructor */
DBaseDataFileWriter::DBaseDataFileWriter(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const ZdString& sFileExtension, bool bAppend)
                    :gpTransaction(0), gpFileRecord(0) {
  try {
    Setup(Parameters, vFieldDefs, sFileExtension, bAppend);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","DBaseDataFileWriter");
    throw;
  }
}

/** destructor */
DBaseDataFileWriter::~DBaseDataFileWriter() {
  try {
    delete gpFileRecord; gpFileRecord=0;
    gFile.EndTransaction(gpTransaction); gpTransaction = 0;
    gFile.Close();
  }
  catch (...){}
}

/** internal setup - opens file stream */
void DBaseDataFileWriter::Setup(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const ZdString& sFileExtension, bool bAppend) {
  std::string   buffer, ext(sFileExtension);

  try {
    ext += DBASE_FILE_EXT;
    gsFileName.setFullPath(Parameters.GetOutputFileName().c_str());
    gsFileName.setExtension(ext.c_str());

    // open file stream for writing
    if ((bAppend && !ZdIO::Exists(gsFileName.getFullPath(buffer).c_str())) || !bAppend) {
      ZdIO::Delete(gsFileName.getFullPath(buffer).c_str());
      gFile.PackFields(vFieldDefs);
      gFile.Create(gsFileName.getFullPath(buffer).c_str(), vFieldDefs);
    }
    if (!gFile.GetIsOpen())
      gFile.Open(gsFileName.getFullPath(buffer).c_str(), (bAppend ? ZDIO_OPEN_APPEND : ZDIO_OPEN_TRUNC));

    //open transaction pointer
    gpTransaction = gFile.BeginTransaction();
    gpFileRecord = gFile.GetNewRecord();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DBaseDataFileWriter");
    throw;
  }
}

/** Write data to file stream. Note that caller is responsible for ensuring that
    record buffer structure agrees with file structure. */
void DBaseDataFileWriter::WriteRecord(const RecordBuffer& Record) {
  try {
    gpFileRecord->Clear();
    for (unsigned short j=0; j < (unsigned short)Record.GetNumFields(); ++j) {
       if (!Record.GetFieldIsBlank(j))
         gpFileRecord->PutFieldValue(j, Record.GetFieldValue(j));
       else
         gpFileRecord->PutBlank(j);
    }
    gFile.AppendRecord(*gpTransaction, *gpFileRecord);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteRecord()","DBaseDataFileWriter");
    throw;
  }
}

