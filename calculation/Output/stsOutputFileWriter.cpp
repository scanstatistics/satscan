// Adam J Vaughn
// November 2002
//
// This is a base output file printer class. The descendants of this heirarchy define
// which file types the output files get written in.

#include "SaTScan.h"
#pragma hdrstop

#include "stsOutputFileWriter.h"

OutputFileWriter::OutputFileWriter(BaseOutputStorageClass* pOutputFileData) : gpOutputFileData(pOutputFileData) {
}

OutputFileWriter::~OutputFileWriter() {
   try {
      delete gpOutputFileData; gpOutputFileData = 0;
   }
   catch (...) { }
}

