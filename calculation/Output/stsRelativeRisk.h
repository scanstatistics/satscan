#ifndef RELATIVERISK_H
#define RELATIVERISK_H

#include "Parameters.h"
#include "stsOutputFileData.h"

extern const char *	REL_RISK_EXT;

class RelativeRiskRecord : public BaseOutputRecord {
   private:
      ZdString 	gsLocationID;
      long 	glObserved;
      double 	gdExpected;
      ZdString 	gsRelRisk;
      ZdString  gsTimeTrend;
      int       giNumFields;

      void 	Init(); 
   public :
      RelativeRiskRecord();
      virtual ~RelativeRiskRecord();

      virtual bool GetFieldIsBlank(int iFieldNumber) { return false; }
      virtual int GetNumFields() { return giNumFields;}
      virtual ZdFieldValue GetValue(int iFieldNumber); 
       
      void	SetExpected(const double dExpected) { gdExpected = dExpected; }
      void	SetLocationID(const ZdString& sLocationID) { gsLocationID = sLocationID; }
      void      SetNumFields(int iNumFields) {giNumFields = iNumFields;} 
      void	SetObserved(const long lObserved) { glObserved = lObserved; }
      void	SetRelativeRisk(const ZdString& sRelRisk) { gsRelRisk = sRelRisk; }
      void      SetTimeTrend(const ZdString& sTimeTrend) { gsTimeTrend = sTimeTrend;}
};  


class RelativeRiskData : public BaseOutputStorageClass {
   private:
      const CParameters & gParameters;

      void	Init();
      void	Setup();
   protected:
      virtual void              SetupFields();
   public:
      RelativeRiskData(BasePrint *pPrintDirection, const CParameters & Parameters);
      virtual ~RelativeRiskData();

      void      SetRelativeRiskData(const ZdString& sLocationID, const long lObserved,
                                    const double dExpected, const ZdString& sRelRisk,
                                    const ZdString& sTimeTrend);
      void      SetRelativeRiskData(const ZdString& sLocationID, const long lObserved,
                                    const double dExpected, const ZdString& sRelRisk);
};

#endif