#ifndef OutputFileWriter_h
#define OutputFileWriter_h

#include "stsOutputFileData.h"

class OutputFileWriter {
   protected :
      ZdString                  gsFileName;
      BaseOutputStorageClass*	gpOutputFileData;
      bool                      gbAppend;

      virtual void      CreateOutputFile() = 0;
   public :
      OutputFileWriter(BaseOutputStorageClass* pOutputFileData, const bool bAppend);
      virtual ~OutputFileWriter();

      virtual void	Print() = 0;
};

#endif
