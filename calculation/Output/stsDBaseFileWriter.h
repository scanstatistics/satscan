//******************************************************************************
#ifndef DBaseFileWriter_H
#define DBaseFileWriter_H
//******************************************************************************
#include "Parameters.h"
//#include <DBFFile.h>
#include "FileName.h"
#include "dBaseFile.h"
#include "FieldDef.h"
#include "ptr_vector.h"

class RecordBuffer;  /** forward class declaration */

/** dBase data writer. */
class DBaseDataFileWriter {
   private :
      void	          Setup(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension,  bool bAppend);

   protected:
      dBaseFile           gFile;
      dBaseRecord       * gpFileRecord;
      static const char * DBASE_FILE_EXT;
      FileName            gsFileName;        /** output filename */

   public :
      DBaseDataFileWriter(const CParameters& Parameters, const ptr_vector<FieldDef>& vFieldDefs, const std::string& sFileExtension, bool bAppend = false);
      virtual ~DBaseDataFileWriter();

     virtual void	  WriteRecord(const RecordBuffer& Record);
};
//******************************************************************************
#endif

