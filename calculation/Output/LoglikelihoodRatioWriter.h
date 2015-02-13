//******************************************************************************
#ifndef LoglikelihoodRatioWriterH
#define LoglikelihoodRatioWriterH
//******************************************************************************
#include "AbstractDataFileWriter.h"

/** Loglikelihood ratio data file writer. */
class LoglikelihoodRatioWriter : public AbstractDataFileWriter {
  public:
    static const char * LOG_LIKELIHOOD_FILE_EXT;
    static const char * LOG_LIKELIHOOD_FILE_HA_EXT;

  protected:
    static const char * TST_STAT_FIELD;
    static const char * LOG_LIKL_RATIO_FIELD;
    RecordBuffer      * gpRecordBuffer;

    void                DefineFields();

  public:
    LoglikelihoodRatioWriter(const CParameters& Parameters, bool bAppend, bool powerStep);
    virtual ~LoglikelihoodRatioWriter();

    virtual void        Write(double dLoglikelihoodRatio);
};
//******************************************************************************
#endif
