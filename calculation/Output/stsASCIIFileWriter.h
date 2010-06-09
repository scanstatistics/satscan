//***************************************************************************
#ifndef ASCIIFileWriter_H
#define ASCIIFileWriter_H
//***************************************************************************
#include "Parameters.h"
#include "FileName.h"
#include "FieldDef.h"
#include "ptr_vector.h"

class RecordBuffer;  /** forward class declaration */

/** ASCII data writer. */
class ASCIIDataFileWriter {
  private:
     void                       Setup(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension, bool bAppend);

   protected :
     FILE                     * gpFile;   
     static const char        * ASCII_FILE_EXT;
     FileName                   gsFileName;        /** output filename */

     void                       CreateBlankString(std::string& sFormatString, const FieldDef& FieldDef);
     void                       CreateFormatString(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv);

   public :
      ASCIIDataFileWriter(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension, bool bAppend=false);
      virtual ~ASCIIDataFileWriter();

     virtual void	        WriteRecord(const RecordBuffer& Record);
};

//***************************************************************************
#endif
