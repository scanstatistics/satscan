#ifndef RELATIVERISK_H
#define RELATIVERISK_H

#include "stsOutputFileData.h"

extern const char *	REL_RISK_EXT;

class RelativeRiskRecord : public BaseOutputRecord {
   private:
      ZdString 	gsLocationID;
      long 	glObserved;
      double 	gdExpected;
      ZdString 	gsRelRisk;
      
      void 	Init(); 
   public :
      RelativeRiskRecord();
      virtual ~RelativeRiskRecord();

      virtual bool GetFieldIsBlank(int iFieldNumber) { return false; }
      virtual int GetNumFields() { return 4; }
      virtual ZdFieldValue GetValue(int iFieldNumber); 
       
      void	SetExpected(const double dExpected) { gdExpected = dExpected; }
      void	SetLocationID(const ZdString& sLocationID) { gsLocationID = sLocationID; }
      void	SetObserved(const long lObserved) { glObserved = lObserved; }
      void	SetRelativeRisk(const ZdString& sRelRisk) { gsRelRisk = sRelRisk; }
};  


class RelativeRiskData : public BaseOutputStorageClass {
   private:
      void	Init();
      void	Setup(const ZdString& sOutputFileName);
   protected:
      virtual void              SetupFields();
   public:
      RelativeRiskData(BasePrint *pPrintDirection, const ZdString& sOutputFileName);
      virtual ~RelativeRiskData();

      void      SetRelativeRiskData(const ZdString& sLocationID, const long lObserved, const double dExpected, const ZdString& sRelRisk);   
};

#endif