//---------------------------------------------------------------------------
#ifndef __stsRunHistoryFile_H
#define __stsRunHistoryFile_H
//---------------------------------------------------------------------------
#include "Parameters.h"
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

class AnalysisRunner;

class stsRunHistoryFile {
   private:
      ZdString                          gsFilename;
      long                              glRunNumber;
      ZdPointerVector<ZdField>	        gvFields;
      BasePrint*                        gpPrintDirection;
      bool                              gbPrintPVal, gbSequential;

      void      GetAnalysisTypeString(ZdString& sTempValue, AnalysisType eAnalysisType);
      void      GetCasePrecisionString(ZdString& sTempValue, int iPrecision);
      void      GetIncludeClustersTypeString(ZdString& sTempValue, AnalysisType eAnalysisType, IncludeClustersType eIncludeClustersType);
      void      GetIntervalUnitsString(ZdString& sTempValue, int iUnits, long lLength, AnalysisType eAnalysisType);
      void      GetMaxGeoExtentString(ZdString& sTempValue, const CParameters& params);
      void      GetMaxTemporalExtentString(ZdString& sTempValue, const CParameters& params);
      void      GetProbabilityModelString(ZdString& sTempValue, ProbabilityModelType eProbabilityModelType);
      void      GetRatesString(ZdString& sTempValue, AnalysisType eAnalysisType, AreaRateType eAreaRateType);
      void      GetTimeAdjustmentString(ZdString& sTempValue, int iType, AnalysisType eAnalysisType, ProbabilityModelType eProbabilityModelType);
      void	Init();
      void      ReplaceExtensionAndAppend(ZdString& sOutputFileNames, const ZdFileName& sSourceFileName, const ZdString& sReplacementExtension);
      void      SetAdditionalOutputFileNameString(ZdString& sOutputFileNames, const CParameters& params);
      void      SetFileName(const ZdString& sFileName);
      void      SetRunNumber();
      void      StripCRLF(ZdString& sStore);
   protected:
      void      CreateRunHistoryFile();
   public:
      stsRunHistoryFile(const CParameters& Parameters, BasePrint& PrintDirection);
      ~stsRunHistoryFile();

      const long        GetRunNumber() const {return glRunNumber;}
      const ZdString&   GetRunHistoryFileName() const {return gsFilename;}
      void              LogNewHistory(const AnalysisRunner& AnalysisRun);

};

static      void      CreateNewField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, const char cType, const short wLength,
                                     const short wPrecision, unsigned short& uwOffset, bool bCreateIndex = false);
static unsigned short GetFieldNumber(const ZdPointerVector<ZdField>& vFields, const char* sFieldName);
static      void      SetBoolField(ZdFileRecord& record, bool bValue, unsigned short uwFieldNumber);
static      void      SetDoubleField(ZdFileRecord& record, double dValue, unsigned short uwFieldNumber);
static      void      SetFieldVector(ZdVector<ZdField*>& vFields, const ZdFile& File);
static      void      SetLongField(ZdFileRecord& record, long lValue, unsigned short uwFieldNumber);
static      void      SetStringField(ZdFileRecord& record, const ZdString& sValue, unsigned short uwFieldNumber);


// allocates a new field and adds it to the vector
// pre : none
// post : a field is added to the pointer vector with appropraite specs
static void CreateNewField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, const char cType, const short wLength,
                                           const short wPrecision, unsigned short& uwOffset, bool bCreateIndex) {
   ZdField  *pField = 0;
   TXDFile  File;
   
   try {
      pField = File.GetNewField();
      pField->SetName(sFieldName.c_str());
      pField->SetType(cType);
      pField->SetLength(wLength);
      pField->SetPrecision(wPrecision);
      pField->SetOffset(uwOffset);
      uwOffset += wLength;
      if(bCreateIndex)                    
         pField->SetIndexCount(1);
      vFields.push_back(pField);
   }
   catch (ZdException &x) {
      delete pField; pField = 0;
      x.AddCallpath("CreateNewField()", "stsRunHistoryFile");
      throw; 	
   }			
}

// finds the position of the field in the global field record
// pre : global field vector has been established
// post : will return the position of the field in the vector
static unsigned short GetFieldNumber(const ZdPointerVector<ZdField>& vFields, const char* sFieldName) {
   unsigned short       uwFieldNumber;
   bool                 bFound = false;

   try {
      for(unsigned int i = 0; i < vFields.GetNumElements() && !bFound; ++i) {
         bFound = (!strcmp(vFields[i]->GetName(),sFieldName));
         uwFieldNumber = static_cast<unsigned short>(i);
      }

      if(!bFound)
         ZdException::GenerateNotification("Field name not found among the fields in file.", "DBaseOutput");
   }
   catch (ZdException &x) {
      x.AddCallpath("GetFieldNumber()", "stsRunHistoryFile");
      throw;
   }
   return uwFieldNumber;
}

// function to set the value of boolean fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetBoolField(ZdFileRecord& record, bool bValue, unsigned short uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsBool() = bValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetBoolField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of double fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetDoubleField(ZdFileRecord& record, double dValue, unsigned short uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsDouble() = dValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetDoubleField()", "stsRunHistoryFile");
      throw;
   }
}

// function to copy the ZdField pointers out of ZdFile type without taking ownership of them, i.e. cloning them
// pre : vector is empty
// post : vector will be filled with the field pointers used in the zdfile
static void SetFieldVector(ZdVector<ZdField*>& vFields, const ZdFile& File) {
   try {
      for(unsigned short i = 0; i < File.GetNumFields(); ++i) {
         vFields.push_back(File.GetFieldInfo(i)->Clone());
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFieldVector()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of long fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetLongField(ZdFileRecord& record, long lValue, unsigned short uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsLong() = lValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetLongField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of string fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
static void SetStringField(ZdFileRecord& record, const ZdString& sValue, unsigned short uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsZdString() = sValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetStringField()", "stsRunHistoryFile");
      throw;
   }
}

//---------------------------------------------------------------------------
#endif
