#ifndef DBaseFileWriter_H
#define DBaseFileWriter_H

#include "stsOutputFileWriter.h"

extern const char * 	DBASE_FILE_EXT;

class DBaseFileWriter : public OutputFileWriter {
   private :
      void	Init();
      void	Setup();
   protected:    
      virtual void      CreateOutputFile();  
   public :
      DBaseFileWriter(BaseOutputStorageClass* pOutputFileData);
      virtual ~DBaseFileWriter();
      
      virtual void 	Print();
};

#endif