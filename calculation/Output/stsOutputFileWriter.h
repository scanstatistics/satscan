#ifndef OutputFileWriter_h
#define OutputFileWriter_h

#include "stsOutputFileData.h"

class OutputFileWriter {
   private:
   protected :
      ZdString                  gsFileName;
      BaseOutputStorageClass*	gpOutputFileData;

      virtual void      CreateOutputFile() = 0;
   public :
      OutputFileWriter(BaseOutputStorageClass* pOutputFileData);
      virtual ~OutputFileWriter();

      virtual void	Print() = 0;
};

#endif