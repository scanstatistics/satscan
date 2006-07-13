//******************************************************************************
#ifndef __AbstractDataFileWriter_H
#define __AbstractDataFileWriter_H
//******************************************************************************
#include "Parameters.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"

/** Collection of field values for buffering records of additional output files.*/
class RecordBuffer {
   private:
      const ZdPointerVector<ZdField>  & vFieldDefinitions;  /** field definitions */
      std::vector<ZdFieldValue>         gvFieldValues;      /** record buffer of field values */
      std::vector<bool>                 gvBlankFields;      /** indicators whether relative field is blank */

   public:
      RecordBuffer(const ZdPointerVector<ZdField>& vFields);
      virtual ~RecordBuffer();

      const ZdField                   & GetFieldDefinition(unsigned int iFieldIndex) const;
      const ZdField                   & GetFieldDefinition(const ZdString& sFieldName) const;
      unsigned int                      GetFieldIndex(const ZdString& sFieldName) const;
      bool                              GetFieldIsBlank(unsigned int iFieldNumber) const;
      ZdFieldValue                    & RecordBuffer::GetFieldValue(const ZdString& sFieldName);
      ZdFieldValue                    & GetFieldValue(unsigned int iFieldIndex);
      const ZdFieldValue              & GetFieldValue(unsigned int iFieldIndex) const;
      unsigned int                      GetNumFields() const { return gvFieldValues.size();}
      void                              SetAllFieldsBlank(bool bBlank);
      void                              SetFieldIsBlank(const ZdString& sFieldName, bool bBlank);
      void                              SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank);
};

class CSaTScanData;
/** Abstract base class for writing record based data to files. */
class AbstractDataFileWriter {
  protected:
    const CParameters         & gParameters;             /** reference to parameters object */
    ASCIIDataFileWriter       * gpASCIIFileWriter;       /** ASCII file writer              */
    DBaseDataFileWriter       * gpDBaseFileWriter;       /** dBase file writer              */
    ZdPointerVector<ZdField>    vFieldDefinitions;       /** field definitions              */

    static const char         * CLUST_NUM_FIELD;
    static const char         * LOC_ID_FIELD;
    static const char         * P_VALUE_FLD;
    static const char         * OBSERVED_FIELD;
    static const char         * EXPECTED_FIELD;
    static const char         * LOG_LIKL_RATIO_FIELD;
    static const char         * TST_STAT_FIELD;
    static const char         * DATASET_FIELD;
    static const char         * CATEGORY_FIELD;
    static const char         * OBSERVED_DIV_EXPECTED_FIELD;
    static const char         * RELATIVE_RISK_FIELD;
    static const char         * MEAN_INSIDE_FIELD;
    static const char         * MEAN_OUTSIDE_FIELD;
    static const char         * VARIANCE_FIELD;
    static const char         * DEVIATION_FIELD;
    static const char         * MEAN_VALUE_FIELD;
    static const size_t         DEFAULT_LOC_FIELD_SIZE;
    static const size_t         MAX_LOC_FIELD_SIZE;

    void                        CreateField(ZdPointerVector<ZdField>& vFields, const std::string& sFieldName, char cType,
                                            short wLength, short wPrecision, unsigned short& uwOffset, bool bCreateIndex=false);
    size_t                      GetLocationIdentiferFieldLength(const CSaTScanData& DataHub) const;

  public:
    AbstractDataFileWriter(const CParameters& Parameters);
    virtual ~AbstractDataFileWriter();
};
//******************************************************************************
#endif

