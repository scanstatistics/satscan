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
//***************************************************************************
#endif
