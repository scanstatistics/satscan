//***************************************************************************
#ifndef ASCIIFileWriter_H
#define ASCIIFileWriter_H
//***************************************************************************
#include "Parameters.h"
#include "FileName.h"

class RecordBuffer;  /** forward class declaration */

/** ASCII data writer. */
class ASCIIDataFileWriter {
  private:
     void                       Setup(const CParameters& Parameters, const ZdString& sFileExtension, bool bAppend);

   protected :
     FILE                     * gpFile;   
     static const char        * ASCII_FILE_EXT;
     FileName                   gsFileName;        /** output filename */

     void                       CreateBlankString(ZdString& sFormatString, const ZdField& FieldDef);
     void                       CreateFormatString(ZdString& sValue, const ZdField& FieldDef, const ZdFieldValue& fv);

   public :
      ASCIIDataFileWriter(const CParameters& Parameters, const ZdString& sFileExtension, bool bAppend=false);
      virtual ~ASCIIDataFileWriter();

     virtual void	        WriteRecord(const RecordBuffer& Record);
};

//***************************************************************************
#endif
