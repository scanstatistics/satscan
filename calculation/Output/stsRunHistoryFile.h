//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------

// mini class used to temporarily store field structure - AJV 9/24/2002
class history_field_t {
   public:
      std::string          gsFieldName;
      char                 gcFieldType;
      short                gwFieldLength;

      history_field_t(std::string sFieldName, char cFieldType, short wFieldLength)
                     {gsFieldName = sFieldName; gcFieldType = cFieldType; gwFieldLength = wFieldLength;}
};

class stsRunHistoryFile {
   private:
      ZdString          gsFilename;
      CAnalysis         *gpAnalysis;
      long              glRunNumber;

      void      CleanupFieldVector(ZdVector<ZdField*>& vFields);
      void	Init();
      void	Setup(const CAnalysis* pAnalysis, const ZdString& sFileName);
   protected:
      void      CreateRunHistoryFile();
      void      SetupFields(std::vector<history_field_t>&  vFieldDescrip );

      void      SetBoolField(ZdFileRecord& record, const bool& bValue, const unsigned long& uwFieldNumber);
      void      SetDoubleField(ZdFileRecord& record, const double& dValue, const unsigned long& uwFieldNumber);
      void      SetLongField(ZdFileRecord& record, const long& lValue, const unsigned long& uwFieldNumber);
      void      SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long& uwFieldNumber);
   public:
      stsRunHistoryFile(const CAnalysis* pAnalysis, const ZdString& sFileName = "");
      ~stsRunHistoryFile();

      const ZdString&   GetRunHistoryFileName() const;
      void              LogNewHistory(const unsigned short& uwSignificantAt005, BasePrint& PrintDirection);
};

//---------------------------------------------------------------------------
#endif
 