//******************************************************************************
#ifndef LoglikelihoodRatioWriterH
#define LoglikelihoodRatioWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

/** Loglikelihood ratio data file writer. */
class LoglikelihoodRatioWriter : public AbstractDataFileWriter {
  protected:
    static const char * TST_STAT_FIELD;
    static const char * LOG_LIKL_RATIO_FIELD;
    static const char * LOG_LIKELIHOOD_FILE_EXT;
    RecordBuffer      * gpRecordBuffer;

    void                DefineFields();
    void                Setup();

  public:
    LoglikelihoodRatioWriter(const CParameters& Parameters);
    virtual ~LoglikelihoodRatioWriter();

    virtual void        Write(double dLoglikelihoodRatio);
};
//******************************************************************************
#endif
 