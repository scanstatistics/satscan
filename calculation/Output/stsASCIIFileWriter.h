#ifndef ASCIIFileWriter_H
#define ASCIIFileWriter_H

#include "stsOutputFileWriter.h"

extern const char *	ASCII_FILE_EXT;

class OutputFileData;

class ASCIIFileWriter : public OutputFileWriter {
   private:
   protected :
      ZdString          gsFileName;

      void              CreateFormatString(ZdString& sValue, const int iFieldNumber, const ZdFieldValue& fv);
      virtual void      CreateOutputFile();
      void              Init();
      void              Setup();
   public :
      ASCIIFileWriter(BaseOutputStorageClass* pOutputFileData);
      virtual ~ASCIIFileWriter();

      virtual void	Print();
};

#endif