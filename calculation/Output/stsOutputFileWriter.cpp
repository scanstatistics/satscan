// Adam J Vaughn
// November 2002
//
// This is a base output file printer class. The descendants of this heirarchy define
// which file types the output files get written in.

#include "SaTScan.h"
#pragma hdrstop

#include "stsOutputFileWriter.h"

OutputFileWriter::OutputFileWriter(BaseOutputStorageClass* pOutputFileData, const bool bAppend) : gbAppend(bAppend) {
   try {
      if (!pOutputFileData)
         ZdGenerateException("NULL output file data pointer!", "Error!");
      gpOutputFileData = pOutputFileData;
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "OutputFileWriter");
      throw;
   }
}

OutputFileWriter::~OutputFileWriter() {
}

