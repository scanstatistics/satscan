//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------

class stsRunHistoryFile {
   private:
      ZdString  gsFilename;
   
      void	Init();
      void	Setup(const ZdString& sFilename);
   protected:
      void      CreateRunHistoryFile();
      void      OpenRunHistoryFile();
      void 	SetupFields(ZdVector<ZdString>& vFields, ZdVector<char>& vFieldTypes);
   public:
      stsRunHistoryFile(const ZdString& sFilename);
      ~stsRunHistoryFile();
};

//---------------------------------------------------------------------------
#endif
 