//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------
#include "Parameters.h"
#include "FieldDef.h"
#include "ptr_vector.h"
#include "dBaseFile.h"

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
extern const char* COVARIATES_FIELD;
extern const char* GRID_FILE_FIELD;
extern const char* START_DATE_FIELD;
extern const char* END_DATE_FIELD;
extern const char* ALIVE_ONLY_FIELD;
extern const char* INTERVAL_FIELD;
extern const char* MONTE_CARLO_FIELD;
extern const char* CUTOFF_001_FIELD;
extern const char* CUTOFF_005_FIELD;
extern const char* NUM_SIGNIF_005_FIELD;
extern const char* P_VALUE_FIELD;
extern const char* ADDITIONAL_OUTPUT_FILES_FIELD;

class AnalysisExecution;

class stsRunHistoryFile {
   private:
      std::string                   gsFilename;
      ptr_vector<FieldDef>	        gvFields;
      BasePrint*                    gpPrintDirection;

      std::string& GetAnalysisTypeString(std::string& sTempValue, AnalysisType eAnalysisType);
      std::string& GetCasePrecisionString(std::string& sTempValue, int iPrecision);
      std::string& GetIncludeClustersTypeString(std::string& sTempValue, AnalysisType eAnalysisType, IncludeClustersType eIncludeClustersType);
      std::string& GetIntervalUnitsString(std::string& sTempValue, int iUnits, long lLength, AnalysisType eAnalysisType);
      std::string& GetMaxGeoExtentString(std::string& sTempValue, const CParameters& params);
      std::string& GetMaxTemporalExtentString(std::string& sTempValue, const CParameters& params);
      std::string& GetProbabilityModelString(std::string& sTempValue, ProbabilityModelType eProbabilityModelType);
      std::string& GetRatesString(std::string& sTempValue, AnalysisType eAnalysisType, AreaRateType eAreaRateType);
      std::string& GetTimeAdjustmentString(std::string& sTempValue, int iType, AnalysisType eAnalysisType, ProbabilityModelType eProbabilityModelType);
      void         ReplaceExtensionAndAppend(std::string& sOutputFileNames, const FileName& sSourceFileName, const std::string& sReplacementExtension);
      void         SetAdditionalOutputFileNameString(std::string& sOutputFileNames, const CParameters& params);
      void         SetFileName(const std::string& sFileName);
      std::string& StripCRLF(std::string& sStore);
   protected:
      void         CreateRunHistoryFile();
   public:
      stsRunHistoryFile(const CParameters& Parameters, BasePrint& PrintDirection);
      ~stsRunHistoryFile();

      const std::string& GetRunHistoryFileName() const {return gsFilename;}
      void         LogNewHistory(const AnalysisExecution& analysisExecution);

};

static unsigned short GetFieldNumber(const ptr_vector<FieldDef>& vFields, const char* sFieldName);
static      void      SetBoolField(dBaseRecord& record, bool bValue, unsigned short uwFieldNumber);
static      void      SetDoubleField(dBaseRecord& record, double dValue, unsigned short uwFieldNumber);
static      void      SetLongField(dBaseRecord& record, long lValue, unsigned short uwFieldNumber);
static      void      SetStringField(dBaseRecord& record, const std::string& sValue, unsigned short uwFieldNumber);

// finds the position of the field in the global field record
// pre : global field vector has been established
// post : will return the position of the field in the vector
static unsigned short GetFieldNumber(const ptr_vector<FieldDef>& vFields, const char* sFieldName) {
   unsigned short       uwFieldNumber;
   bool                 bFound = false;

   try {
      for(unsigned int i = 0; i < vFields.size() && !bFound; ++i) {
         bFound = (!strcmp(vFields[i]->GetName(),sFieldName));
         uwFieldNumber = static_cast<unsigned short>(i);
      }

      if(!bFound)
         throw prg_error("Field name not found among the fields in file.", "DBaseOutput");
   }
   catch (prg_exception& x) {
      x.addTrace("GetFieldNumber()", "stsRunHistoryFile");
      throw;
   }
   return uwFieldNumber;
}

// function to set the value of boolean fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetBoolField(dBaseRecord& record, bool bValue, unsigned short uwFieldNumber) {
   FieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsBool() = bValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (prg_exception& x) {
      x.addTrace("SetBoolField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of double fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetDoubleField(dBaseRecord& record, double dValue, unsigned short uwFieldNumber) {
   FieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsDouble() = dValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (prg_exception& x) {
      x.addTrace("SetDoubleField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of long fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetLongField(dBaseRecord& record, long lValue, unsigned short uwFieldNumber) {
   FieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsLong() = lValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (prg_exception& x) {
      x.addTrace("SetLongField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of string fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetStringField(dBaseRecord& record, const std::string& sValue, unsigned short uwFieldNumber) {
   FieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsString() = sValue;
      if (fv.AsString().size() > static_cast<unsigned long>(record.GetFieldLength(uwFieldNumber)))
        fv.AsString().resize(record.GetFieldLength(uwFieldNumber));
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (prg_exception& x) {
      x.addTrace("SetStringField()", "stsRunHistoryFile");
      throw;
   }
}

//---------------------------------------------------------------------------
#endif
