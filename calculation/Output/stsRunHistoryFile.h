//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------
    /*
// mini class used to temporarily store field structure - AJV 9/24/2002
class history_field_t {
   public:
      std::string          gsFieldName;
      char                 gcFieldType;
      short                gwFieldLength;

      history_field_t(std::string sFieldName, char cFieldType, short wFieldLength)
                     {gsFieldName = sFieldName; gcFieldType = cFieldType; gwFieldLength = wFieldLength;}
};  */

class stsRunHistoryFile {
   private:
      ZdString          gsFilename;
      long              glRunNumber;

      void      GetAnalysisTypeString(ZdString& sTempValue, int iType);
      void      GetCasePrecisionString(ZdString& sTempValue, int iPrecision);
      void      GetIntervalUnitsString(ZdString& sTempValue, int iUnits);
      void      GetProbabilityModelString(ZdString& sTempValue, int iModel);
      void      GetRatesString(ZdString& sTempValue, int iRate);
      void      GetTimeAdjustmentString(ZdString& sTempValue, int iType);
      void	Init();
      void      SetRunNumber();
      void	Setup(const ZdString& sFileName);
   protected:
      void      CreateRunHistoryFile();
      void      SetupFields(std::vector<field_t>&  vFieldDescrip );

//      void      SetBoolField(ZdFileRecord& record, const bool bValue, const unsigned long uwFieldNumber);
 //     void      SetDoubleField(ZdFileRecord& record, const double dValue, const unsigned long uwFieldNumber);
//      void      SetLongField(ZdFileRecord& record, const long lValue, const unsigned long uwFieldNumber);
//      void      SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long uwFieldNumber);
   public:
      stsRunHistoryFile(const ZdString& sFileName);
      ~stsRunHistoryFile();

      const long        GetRunNumber() const {return glRunNumber;}
      const ZdString&   GetRunHistoryFileName() const {return gsFilename;}
      void              LogNewHistory(const CAnalysis& pAnalysis, const unsigned short uwSignificantAt005, BasePrint& PrintDirection);
};

//---------------------------------------------------------------------------
#endif
 