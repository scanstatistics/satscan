//***************************************************************************
#ifndef OUTPUTFILEDATA_H
#define OUTPUTFILEDATA_H
//***************************************************************************
#define INCLUDE_RUN_HISTORY

/** Collection of field values for buffering output records of additional
    output files, such as Cluster Information file.*/
class OutputRecord {
   private:
      std::vector<ZdFieldValue>         gvFieldValues; /** record buffer of field values */
      std::vector<bool>                 gvBlankFields; /** indicators whether relative field is blank */

      void                              Setup(const ZdPointerVector<ZdField>& vFields);

   public:
      OutputRecord(const ZdPointerVector<ZdField>& vFields);
      virtual ~OutputRecord();

      virtual bool                      GetFieldIsBlank(unsigned int iFieldNumber) const;
      virtual ZdFieldValue            & GetFieldValue(unsigned int iFieldIndex);
      virtual const ZdFieldValue      & GetFieldValue(unsigned int iFieldIndex) const;
      virtual unsigned int              GetNumFields() const { return gvFieldValues.size();}
      void                              SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank);
};

/** Base storage class of the output files. Stores record buffers. */
class BaseOutputStorageClass {
   protected :
      static const char                  * CLUST_NUM_FIELD;
      static const char                  * LOC_ID_FIELD;
      static const char                  * P_VALUE_FLD;
      static const char                  * OBSERVED_FIELD;
      static const char                  * EXPECTED_FIELD;
      static const char                  * REL_RISK_FIELD;
      static const char                  * LOG_LIKL_FIELD;
      static const char                  * TST_STAT_FIELD;
      ZdPointerVector<const OutputRecord>  gvRecordBuffers;
      ZdPointerVector<ZdField>	           gvFields;

   public :
      BaseOutputStorageClass();
      virtual ~BaseOutputStorageClass();

      void			           AddRecord(const OutputRecord* pRecord);
      static void                          CreateField(ZdPointerVector<ZdField>& vFields,
                                                       const std::string& sFieldName,
                                                       char cType, short wLength,
                                                       short wPrecision, unsigned short& uwOffset,
                                                       bool bCreateIndex = false);
      const ZdField                      * GetField(unsigned int uwFieldNumber) const;
      unsigned int                         GetFieldNumber(const ZdString& sFieldName) const;
      const ZdVector<ZdField*>           & GetFields() { return gvFields; }
      unsigned int		           GetNumFields() const {return gvFields.size();}
      unsigned int      	           GetNumRecords() const {return gvRecordBuffers.size();}
      virtual const char                 * GetOutputExtension() const = 0;
      const OutputRecord                 * GetRecord(int iPosition) const;
};
//***************************************************************************
#endif
