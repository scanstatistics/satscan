//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsOutputFileWriter.h"

/** class constructor */
OutputFileWriter::OutputFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection)
                 :gOutputFileData(OutputFileData), gPrintDirection(PrintDirection) {}

/** class desctructor */
OutputFileWriter::~OutputFileWriter() {}

/** Sets outfile name base upon passed sBaseFileName and sWriterExtension. */
void OutputFileWriter::SetOutputFileName(const char * sBaseFileName, const char * sWriterExtension) {
  ZdString sExt;

  gsFileName = sBaseFileName;
  sExt = ZdFileName(gsFileName).GetExtension();
  if (sExt.GetLength())
    gsFileName.Replace(sExt, gOutputFileData.GetOutputExtension());
  else
    gsFileName << gOutputFileData.GetOutputExtension();
  gsFileName << sWriterExtension;
}

