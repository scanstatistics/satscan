//***************************************************************************
#ifndef LOGLIKELIHOODDBF_H
#define LOGLIKELIHOODDBF_H
//***************************************************************************
#include "stsOutputFileData.h"
#include "Parameters.h"

/** Output data model for loglikelihood output file types. */
class LogLikelihoodData : public BaseOutputStorageClass {
   private:
      virtual void              SetupFields();

   protected:
      static const char       * LOG_LIKELIHOOD_FILE_EXT;
      const CParameters       & gParameters;

   public:
      LogLikelihoodData(const CParameters& Parameters);
      virtual ~LogLikelihoodData();

      void	                AddLikelihoodRatio(double dLikelihoodRatio);
      virtual const char      * GetOutputExtension() const {return LOG_LIKELIHOOD_FILE_EXT;}
};

#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"

/** Collection of field values for buffering output records of additional
    output files, such as Cluster Information file.*/
class RecordBuffer {
   private:
      std::vector<ZdFieldValue>         gvFieldValues; /** record buffer of field values */
      std::vector<bool>                 gvBlankFields; /** indicators whether relative field is blank */

      void                              Setup(const ZdPointerVector<ZdField>& vFields);

   public:
      RecordBuffer(const ZdPointerVector<ZdField>& vFields);
      virtual ~RecordBuffer();

      virtual bool                      GetFieldIsBlank(unsigned int iFieldNumber) const;
      virtual ZdFieldValue            & GetFieldValue(unsigned int iFieldIndex);
      virtual const ZdFieldValue      & GetFieldValue(unsigned int iFieldIndex) const;
      virtual unsigned int              GetNumFields() const { return gvFieldValues.size();}
      void                              SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank);
};

class AbstractDataWriter {
  protected:
    const CParameters         & gParameters;
    ASCIIDataFileWriter       * gpASCIIFileWriter;
    DBaseDataFileWriter       * gpDBaseFileWriter;
    ZdPointerVector<ZdField>    gvFields;
    RecordBuffer              * gpRecordBuffer;    

      static const char                  * CLUST_NUM_FIELD;
      static const char                  * LOC_ID_FIELD;
      static const char                  * P_VALUE_FLD;
      static const char                  * OBSERVED_FIELD;
      static const char                  * EXPECTED_FIELD;
      static const char                  * LOG_LIKL_RATIO_FIELD;
      static const char                  * TST_STAT_FIELD;

    void                        CreateField(const std::string& sFieldName,
                                            char cType, short wLength,
                                            short wPrecision, unsigned short& uwOffset,
                                            bool bCreateIndex = false);
      unsigned int              GetFieldNumber(const ZdString& sFieldName) const;

  public:
    AbstractDataWriter(const CParameters& Parameters);
    virtual ~AbstractDataWriter();
};

#include "boost/thread/mutex.hpp"

class LoglikelihoodRatioWriter : public AbstractDataWriter {
  protected:
    static const char         * TST_STAT_FIELD;
    static const char         * LOG_LIKL_RATIO_FIELD;
    boost::mutex	      & gMutex;    

    void                        SetupFields();
  public:
    LoglikelihoodRatioWriter(boost::mutex& Mutex, const CParameters& Parameters);
    virtual ~LoglikelihoodRatioWriter();
    
    virtual void		Write(double dLoglikelihoodRatio);
};
//***************************************************************************
#endif
