//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsDBaseFileWriter.h"
#include <DBFFile.h>
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
  ZdString sExt(sFileExtension);

  try {
    sExt << DBASE_FILE_EXT;
    gsFileName.SetFullPath(Parameters.GetOutputFileName().c_str());
    gsFileName.SetExtension(sExt.GetCString());

    // open file stream for writing
    if ((bAppend && !ZdIO::Exists(gsFileName.GetFullPath())) || !bAppend) {
      ZdIO::Delete(gsFileName.GetFullPath());
      gFile.PackFields(vFieldDefs);
      gFile.Create(gsFileName.GetFullPath(), vFieldDefs);
    }
    if (!gFile.GetIsOpen())
      gFile.Open(gsFileName.GetFullPath(), (bAppend ? ZDIO_OPEN_APPEND : ZDIO_OPEN_TRUNC));

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
    for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
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

