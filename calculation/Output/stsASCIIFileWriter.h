#ifndef ASCIIFileWriter_H
#define ASCIIFileWriter_H

#include "stsOutputFileWriter.h"

extern const char *	ASCII_FILE_EXT;

class OutputFileData;

class ASCIIFileWriter : public OutputFileWriter {
   protected :
      void              CreateBlankString(ZdString& sFormatString, int iFieldNumber);
      void              CreateFormatString(ZdString& sValue, const int iFieldNumber, const ZdFieldValue& fv);
      virtual void      CreateOutputFile();
      void              Setup();
   public :
      ASCIIFileWriter(BaseOutputStorageClass* pOutputFileData, const bool bAppend = false);

      virtual void	Print();
};

#endif