//***************************************************************************
#ifndef ASCIIFileWriter_H
#define ASCIIFileWriter_H
//***************************************************************************
#include "stsOutputFileWriter.h"
#include "Parameters.h"

class OutputFileData;

/** This class object is used to print out an OutputFileData type in ASCII format. */
class ASCIIFileWriter : public OutputFileWriter {
  private:
     void                       Setup(const CParameters& Parameters, bool bAppend);

   protected :
     static const char        * ASCII_FILE_EXT;

     void                       CreateBlankString(ZdString& sFormatString, int iFieldNumber);
     void                       CreateFormatString(ZdString& sValue, const int iFieldNumber, const ZdFieldValue& fv);
     virtual void               CreateOutputFile();
     virtual void	        Print();

   public :
      ASCIIFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection,
                      const CParameters& Parameters, bool bAppend = false);
      virtual ~ASCIIFileWriter();
};
//***************************************************************************
#endif
