//***************************************************************************
#ifndef OutputFileWriter_h
#define OutputFileWriter_h
//***************************************************************************
#include "stsOutputFileData.h"

/** This is a base output file printer class. The descendants of this heirarchy
    define which file types the output files get written in.                 */
class OutputFileWriter {
   protected :
      ZdString                  gsFileName;        /** output filename */
      BaseOutputStorageClass  & gOutputFileData;   /** record buffer storage class */
      BasePrint               & gPrintDirection;   /** print direction - messages to user */

      virtual void              CreateOutputFile() = 0;
      virtual void	        Print() = 0;
      virtual void              SetOutputFileName(const char * sBaseFileName, const char * sWriterExtension);

   public :
      OutputFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection);
      virtual ~OutputFileWriter();
};

class RecordBuffer;

/** This is a base output file printer class. The descendants of this heirarchy
    define which file types the output files get written in.                 */
class DataFileWriter {
   protected :
      ZdFileName                gsFileName;        /** output filename */

//      virtual void              CreateOutputFile();
      virtual void              SetOutputFileName(const char * sBaseFileName, const char * sWriterExtension);

   public :
      DataFileWriter();
      virtual ~DataFileWriter();
};

//***************************************************************************
#endif
