//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------

extern const char*      ANALYSIS_HISTORY_FILE;

class stsRunHistoryFile {
   private:
      ZdString          gsFilename;
      CAnalysis         *gpAnalysis;
      long              glRunNumber;

      void	Init();
      void	Setup(const CAnalysis* pAnalysis);
   protected:
      void      CreateRunHistoryFile();
      void      OpenRunHistoryFile();
      void 	SetupFields(ZdVector<ZdString>& vFields, ZdVector<char>& vFieldTypes);
   public:
      stsRunHistoryFile(const CAnalysis* pAnalysis);
      ~stsRunHistoryFile();

      void      LogNewHistory();
};

//---------------------------------------------------------------------------
#endif
 