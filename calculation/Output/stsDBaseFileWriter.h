//***************************************************************************
#ifndef DBaseFileWriter_H
#define DBaseFileWriter_H
//***************************************************************************
#include "stsOutputFileWriter.h"
#include "Parameters.h"

/** This class object is used to print out an OutputFileData type in dBase format. */
class DBaseFileWriter : public OutputFileWriter {
   private :
      void	                Setup(const CParameters& Parameters, bool bAppend);

   protected:
      static const char * 	DBASE_FILE_EXT;

      virtual void              CreateOutputFile();
      virtual void 	        Print();

   public :
      DBaseFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection, 
                      const CParameters& Parameters, bool bAppend = false);
};

/** This class object is used to print out an OutputFileData type in dBase format. */
class DBaseDataFileWriter : public DataFileWriter {
   private :
      void	                Setup(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const char * sFileExtension,  bool bAppend);

   protected:
      DBFFile                *  pFile;   
      ZdTransaction          * pTransaction;
      static const char      * DBASE_FILE_EXT;

      virtual void              CreateOutputFile(ZdPointerVector<ZdField>& vFieldDefs);

   public :
      DBaseDataFileWriter(const CParameters& Parameters, ZdPointerVector<ZdField>& vFieldDefs, const char * sFileExtension, bool bAppend = false);
      virtual ~DBaseDataFileWriter();

     virtual void	        WriteRecord(const ZdPointerVector<ZdField>& vFieldDefs, const RecordBuffer& Record);
};
//***************************************************************************
#endif
