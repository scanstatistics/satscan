#ifndef OUTPUTFILEDATA_H
#define OUTPUTFILEDATA_H

#define INCLUDE_RUN_HISTORY

extern const char * RUN_NUM_FIELD;
extern const char * CLUST_NUM_FIELD;
extern const char * LOC_ID_FIELD;
extern const char * P_VALUE_FIELD;
extern const char * OBSERVED_FIELD;
extern const char * EXPECTED_FIELD;
extern const char * REL_RISK_FIELD;
extern const char * START_DATE_FLD;
extern const char * END_DATE_FLD;
extern const char * LOG_LIKL_FIELD;
extern const char * P_VALUE_FLD;
extern const char * NUM_AREAS_FIELD;
extern const char * COORD_LAT_FIELD;
extern const char * COORD_LONG_FIELD;
extern const char * COORD_X_FIELD;
extern const char * COORD_Y_FIELD;
extern const char * COORD_Z_FIELD;
extern const char * RADIUS_FIELD;
extern const char * AREA_OBS_FIELD;
extern const char * AREA_EXP_FIELD;
extern const char * AREA_RSK_FIELD;
extern const char * E_ANGLE_FIELD;
extern const char * E_SHAPE_FIELD;
extern const char * CLU_OBS_FIELD;
extern const char * CLU_EXP_FIELD;
extern const char * TST_STAT_FIELD;
extern const char * TIME_TREND_FIELD;

// base record class for the individual output types records
class BaseOutputRecord {
   protected :
      std::vector<bool>                 gvbBlankFields;
   public :
      BaseOutputRecord();
      virtual ~BaseOutputRecord();

      virtual bool GetFieldIsBlank(int iFieldNumber) = 0;
      virtual int GetNumFields() = 0;
      virtual ZdFieldValue GetValue(int iFieldNumber) = 0;

      void SetFieldValueAsLong(ZdFieldValue& fv, const long lValue);
      void SetFieldValueAsDouble(ZdFieldValue& fv, const double dValue);
      void SetFieldValueAsString(ZdFieldValue& fv, const ZdString& sValue);
};


// base storage class of the output files
class BaseOutputStorageClass {
   protected :
      ZdPointerVector<BaseOutputRecord>	gvRecords;
      ZdPointerVector<ZdField>		gvFields;
      ZdString				gsFileName;
      BasePrint         *               gpPrintDirection;

      virtual void	SetupFields() = 0;
   public :
      BaseOutputStorageClass(BasePrint *pPrintDirection);
      virtual ~BaseOutputStorageClass();
      
      void			        AddRecord(BaseOutputRecord* pRecord);

      BasePrint*                        GetBasePrinter()        { return gpPrintDirection; }
      ZdField*                          GetField(unsigned short uwFieldNumber);
      const unsigned short              GetFieldNumber(const ZdString& sFieldName);
      const ZdVector<ZdField*>&	        GetFields() { return gvFields; }
      const ZdString&			GetFileName() { return gsFileName; }
      const unsigned short		GetNumFields() { return gvFields.size(); }
      const unsigned long		GetNumRecords() { return gvRecords.size(); }
      BaseOutputRecord*		        GetRecord(int iPosition);     	
};


class TestOutputRecord : public BaseOutputRecord {
   private :
      ZdString	gsStringTestValue;
      long	glLongTestValue;
      double	gdDoubleTestValue;
      float	gfFloatTestValue;
      int	giIntTestValue;
      bool	gbBoolTestValue;
   
      void	Init();
   public :
      TestOutputRecord();
      ~TestOutputRecord();

      virtual bool GetFieldIsBlank(int iFieldNumber);
      virtual int GetNumFields() { return 6; }
      virtual ZdFieldValue GetValue(int iFieldNumber);

      void      SetFieldIsBlank(int iFieldNumber, bool bBlank = true);

      void 	SetBoolTestField(const bool bBoolTestValue)        { gbBoolTestValue   = bBoolTestValue;  }
      void 	SetDoubleTestField(const double dDoubleTestValue)    { gdDoubleTestValue = dDoubleTestValue;}
      void 	SetFloatTestField(const float fFloatTestValue)      { gfFloatTestValue  = fFloatTestValue; }
      void 	SetIntTestField(const int iIntTestValue)         { giIntTestValue = iIntTestValue;   }
      void 	SetLongTestField(const long lLongTestValue)        { glLongTestValue   = lLongTestValue;  }
      void 	SetStringTestField(const ZdString& sStringTestValue) { gsStringTestValue = sStringTestValue;}
};	

class TestOutputClass : public BaseOutputStorageClass {
   protected:
      virtual void	SetupFields();   
   public :
      TestOutputClass(BasePrint *pPrintDirection, const ZdString& sOutputFileName);
      ~TestOutputClass();

      void      AddBlankRecord();
      void      SetTestValues(const ZdString& sStringTestValue, const long lLongTestValue, const double	dDoubleTestValue,
                              const float fFloatTestValue, const int iIntTestValue, const bool bBoolTestValue);
};

// allocates a new field and adds it to the vector
// pre : none
// post : a field is added to the pointer vector with appropraite specs
static void CreateField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, const char cType, const short wLength,
                                           const short wPrecision, unsigned short& uwOffset, bool bCreateIndex = false) {
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
      x.AddCallpath("CreateField()", "BaseOutputStorageClass");
      throw;
   }			
}

#endif