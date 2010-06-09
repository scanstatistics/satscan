//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsDBaseFileWriter.h"
#include "AbstractDataFileWriter.h"

const char * DBaseDataFileWriter::DBASE_FILE_EXT    = ".dbf";

/** constructor */
DBaseDataFileWriter::DBaseDataFileWriter(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension, bool bAppend)
                    :gpFileRecord(0) {
  try {
    Setup(Parameters, vFieldDefs, sFileExtension, bAppend);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","DBaseDataFileWriter");
    throw;
  }
}

/** destructor */
DBaseDataFileWriter::~DBaseDataFileWriter() {
  try {
    gFile.PackData();
    gFile.Close();
  }
  catch (...){}
}

/** internal setup - opens file stream */
void DBaseDataFileWriter::Setup(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension, bool bAppend) {
  std::string   buffer, ext(sFileExtension);

  try {
    ext += DBASE_FILE_EXT;
    gsFileName.setFullPath(Parameters.GetOutputFileName().c_str());
    gsFileName.setExtension(ext.c_str());

    // open file stream for writing
    if ((bAppend && access(gsFileName.getFullPath(buffer).c_str(), 0)) || !bAppend) {
      remove(gsFileName.getFullPath(buffer).c_str());
      gFile.PackFields(vFieldDefs);
      gFile.Create(gsFileName.getFullPath(buffer).c_str(), vFieldDefs);
    }
    if (!gFile.GetIsOpen())
      gFile.Open(gsFileName.getFullPath(buffer).c_str(), bAppend);

    gpFileRecord = gFile.GetNewRecord();
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","DBaseDataFileWriter");
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
    gFile.DataAppend(*gpFileRecord);
  }
  catch (prg_exception& x) {
    x.addTrace("WriteRecord()","DBaseDataFileWriter");
    throw;
  }
}

