#ifndef LOGLIKELIHOODDBF_H
#define LOGLIKELIHOODDBF_H

#include "stsOutputFileData.h"

extern const char *	LOG_LIKELIHOOD_FILE_EXT;

class LogLikelihoodRecord : public BaseOutputRecord {
   private:
      double gdLogLikelihood;
   public:
      LogLikelihoodRecord();
      LogLikelihoodRecord(const double dLikelihood);
      virtual ~LogLikelihoodRecord();
   
      virtual int GetNumFields() { return 1; }
      virtual ZdFieldValue GetValue(int iFieldNumber);
      
      void	SetLogLikelihood(const double dLogLikelihood) { gdLogLikelihood = dLogLikelihood; }
};

class LogLikelihoodData : public BaseOutputStorageClass {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
   protected:
      virtual void              SetupFields();
   public:
      __fastcall LogLikelihoodData(BasePrint *pPrintDirection, const ZdString& sOutputFileName);
      virtual ~LogLikelihoodData();
   
      void	AddLikelihood(const double dLikelihood);	
};

#endif