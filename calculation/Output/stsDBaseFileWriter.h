//******************************************************************************
#ifndef DBaseFileWriter_H
#define DBaseFileWriter_H
//******************************************************************************
#include "Parameters.h"

class RecordBuffer;  /** forward class declaration */

/** dBase data writer. */
class DBaseDataFileWriter {
   private :
      void	          Setup(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const ZdString& sFileExtension,  bool bAppend);

   protected:
      DBFFile             gFile;
      DBFRecord         * gpFileRecord;
      ZdTransaction     * gpTransaction;
      static const char * DBASE_FILE_EXT;
      ZdFileName          gsFileName;        /** output filename */

   public :
      DBaseDataFileWriter(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const ZdString& sFileExtension, bool bAppend = false);
      virtual ~DBaseDataFileWriter();

     virtual void	  WriteRecord(const RecordBuffer& Record);
};
//******************************************************************************
#endif

