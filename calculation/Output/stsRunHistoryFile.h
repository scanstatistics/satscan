//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------
extern const char* RUN_NUMBER_FIELD;
extern const char* RUN_TIME_FIELD;
extern const char* OUTPUT_FILE_FIELD;
extern const char* PROB_MODEL_FIELD;
extern const char* RATES_FIELD;           
extern const char* COORD_TYPE_FIELD;
extern const char* ANALYSIS_TYPE_FIELD;
extern const char* NUM_CASES_FIELD;
extern const char* TOTAL_POP_FIELD;
extern const char* NUM_GEO_AREAS_FIELD;
extern const char* PRECISION_TIMES_FIELD;
extern const char* MAX_GEO_EXTENT_FIELD;
extern const char* MAX_TIME_EXTENT_FIELD;
extern const char* TIME_TREND_ADJUSTMENT_FIELD;
extern const char* GRID_FILE_FIELD;
extern const char* START_DATE_FIELD;
extern const char* END_DATE_FIELD;
extern const char* ALIVE_ONLY_FIELD;
extern const char* INTERVAL_UNITS_FIELD;
extern const char* INTERVAL_LENGTH_FIELD;
extern const char* MONTE_CARLO_FIELD;
extern const char* CUTOFF_001_FIELD;
extern const char* CUTOFF_005_FIELD;
extern const char* NUM_SIGNIF_005_FIELD;

class stsRunHistoryFile {
   private:
      ZdString                          gsFilename;
      long                              glRunNumber;
      ZdPointerVector<ZdField>	        gvFields;
      BasePrint*                        gpPrintDirection;

      void      GetAnalysisTypeString(ZdString& sTempValue, int iType);
      void      GetCasePrecisionString(ZdString& sTempValue, int iPrecision);
      void      GetIntervalUnitsString(ZdString& sTempValue, int iUnits);
      void      GetProbabilityModelString(ZdString& sTempValue, int iModel);
      void      GetRatesString(ZdString& sTempValue, int iRate);
      void      GetTimeAdjustmentString(ZdString& sTempValue, int iType);
      void	Init();
      void      SetRunNumber();
      void      StripCRLF(ZdString& sStore);
   protected:
      void      CreateRunHistoryFile();
   public:
      stsRunHistoryFile(const ZdString& sFileName, BasePrint& PrintDirection);
      ~stsRunHistoryFile();

      const long        GetRunNumber() const {return glRunNumber;}
      const ZdString&   GetRunHistoryFileName() const {return gsFilename;}
      void              LogNewHistory(const CAnalysis& pAnalysis, const unsigned short uwSignificantAt005);

};

//---------------------------------------------------------------------------
#endif
 