//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------

class stsRunHistoryFile {
   private:
      ZdString          gsFilename;
      CAnalysis         *gpAnalysis;
      long              glRunNumber;

      void	Init();
      void	Setup(const CAnalysis* pAnalysis, const ZdString& sFileName);
   protected:
      void      CreateRunHistoryFile();
      void      OpenRunHistoryFile(const unsigned short& uwSignificantAt005);
      void 	SetupFields(ZdVector<pair<pair<ZdString, char>, long> >&  vFieldDescrip);
   public:
      stsRunHistoryFile(const CAnalysis* pAnalysis, const ZdString& sFileName = "");
      ~stsRunHistoryFile();

      const ZdString&   GetRunHistoryFileName() const;
      void              LogNewHistory(const unsigned short& uwSignificantAt005);
};

//---------------------------------------------------------------------------
#endif
 